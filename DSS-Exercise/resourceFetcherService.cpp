//
//  resourceFetcherService.cpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/16/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#include "resourceFetcherService.hpp"
#include "utilities.hpp"
#include "request.hpp"
#include "curl/curl.h"

#include <iostream>
#include <chrono>
#include <cmath>
#include <random>

static const int64_t kDefaultBackoffDuration = 100; // milliseconds

static int64_t getBackoffDuration(int32_t retryCount, bool jitter=false) {
    auto val = std::exp2(retryCount) * static_cast<double>(kDefaultBackoffDuration);
    if (jitter) {
        std::uniform_real_distribution<double> unif(0, val);
        std::random_device rd;
        std::default_random_engine eng(rd());
        auto weight = std::bind(unif, eng);
        val = weight(eng);
    }
    return static_cast<int64_t>(val);
}

ResourceFetcherService::ResourceFetcherService(uint32_t numWorkers, uint32_t stress, bool verbose) : verbose_(verbose), stress_(stress), workerPool_(numWorkers) {
    workerPool_.initialize();
}

void ResourceFetcherService::add(const std::string& url, std::function<void(Error error, uint32_t statusCode, const std::vector<uint8_t>&)> callback) {
    Job job(url, callback, stress_, verbose_);
    workerPool_.add(job);
}

std::size_t ResourceFetcherService::Job::curlWriteCallback(const char *in, std::size_t size, std::size_t num, std::vector<uint8_t>* out) {
    const std::size_t totalBytes(size * num);
    out->insert(out->end(), in, in + totalBytes);
    return totalBytes;
}

void ResourceFetcherService::Job::execute() {
    // This is admittedly a bit of a hack, however for the sake of time, I am doing this.
    // I typically use a more robust system that takes either file://, http://, or https:// schemes
    // that does the work on background threads. This allow for a consistent interface to get assets
    // as well as providing means of doing using worker threads
    
    // I am doing a simple check against "file://" as my test for file versus network
    if (url_.size()) {
        try {
            if (utilities::isPrefixOf(url_, "file://")) {
                // Filesystem
                auto [error, output] = loadFile();
                if (callback_) {
                    callback_(error, 0, output);
                }
            } else {
                // Network
                if (verbose_) {
                    std::cout << "Fetching " << url_ << std::endl;
                }
                auto [error, status, output] = fetchFile();
                if (verbose_) {
                    // Don't output images
                    const std::string jpg = "jpg";
                    if (url_.length() > jpg.length()) {
                        if (url_.rfind(jpg) != (url_.size() - jpg.size())) {
                            const std::string out(output.begin(), output.end());
                            std::cout << "Done: " << url_ << std::endl;
                            std::cout << "Error: " << static_cast<uint32_t>(error) << std::endl;
                            std::cout << "Status: " << status << std::endl;
                            std::cout << out << std::endl;
                        }
                    }
                }
                if (callback_) {
                    callback_(error, status, output);
                }
            }
        } catch (std::exception& e) {
            if (callback_) {
                std::vector<uint8_t> empty;
                callback_(Error::Exception, 0, empty);
            }
        }
    } else if (callback_) {
        // This is considered a failure as we have no url
        if (callback_) {
            std::vector<uint8_t> empty;
            callback_(Error::NoResourceName, 0, empty);
        }
    }
}

std::pair<Error, std::vector<uint8_t>> ResourceFetcherService::Job::loadFile() {
    Error error = Error::None;
    std::vector<uint8_t> output;
    // We assume at this point (for time savings), that we have the proper prefix, so remove "file://"
    auto pos = strlen("file://");
    auto path = url_.substr(pos, url_.size() - pos);
    FILE *file = fopen(path.c_str(), "rb");
    if (file) {
        fseek(file, 0L, SEEK_END);
        auto len = ftell(file);
        fseek(file, 0L, SEEK_SET);

        output.resize(len);
        auto bufSize = static_cast<int>(fread(&output[0], sizeof(uint8_t), len, file));
        if (bufSize != len) {
            error = Error::IOError;
            output.clear();
        }
        fclose(file);
    } else {
        error = Error::NoResource;
    }

    return std::make_pair(error, std::move(output));
}

std::tuple<Error, uint32_t, std::vector<uint8_t>> ResourceFetcherService::Job::fetchFile() {
    Error error = Error::None;
    uint32_t status = 0;
    std::vector<uint8_t> output;
    Request request(url_);

    // We do retries with backoff.
    // However, this is not built overly robust, but is simply an example of plumbing that should be put into place
    while (true) {
        CURL *curl = curl_easy_init();

        // Right now ignoring any CURLcode return value whereas more robust code should handle errors
        curl_easy_setopt(curl, CURLOPT_URL, request.url.c_str());
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Job::curlWriteCallback);

        // Be sure to clear in case we are retrying
        output.clear();
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);

        auto results = curl_easy_perform(curl);
        switch (results) {
        case CURLE_OK:
            break;
                
            // Arbitrary codes selected for retries as illustration of handling different contexts where one needs to treat the error
            // As something you can retry (like timeout), or an error that is otherwise a failure you can never recover from
            case CURLE_OPERATION_TIMEDOUT:
            case CURLE_SEND_ERROR:
            case CURLE_RECV_ERROR:
            case CURLE_COULDNT_RESOLVE_PROXY:
            case CURLE_COULDNT_RESOLVE_HOST:
            case CURLE_WEIRD_SERVER_REPLY:
            case CURLE_REMOTE_ACCESS_DENIED:
            case CURLE_COULDNT_CONNECT:
            case CURLE_HTTP_RETURNED_ERROR:
                // This will be a retry
                break;
                
            default:
                // NOTE: The way this simple error handling is done, actual CURL code is lost
                // More robust system would either log or surface the actual error
                error = Error::Curl;
                break;
        }

        // Only handle certain errors right now. This is as an illustration of
        // retry handling. More robust code would handle more cases.
        long httpCode(0);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);
        
        status = static_cast<uint32_t>(httpCode);
        
        if (error != Error::None) {
            // On errors, clear
            output.clear();
            break;
        }

        // Right now we only treat certain status codes as candidates for retrying
        if (status < 500 && status > 0) {
            // If we get a 200 response, we expect a non-0 output, else we treat that as an error
            if (status == 200) {
                if (output.size()) {
                    break;
                }
            } else {
                break;
            }
        }
        
        if (request.retryCount < request.maxRetryCounts) {
            auto backoff = getBackoffDuration(request.retryCount);
            if (backoff < request.lastBackoffDuration) {
                backoff += request.lastBackoffDuration;
            }
            auto duration = std::chrono::milliseconds(backoff);
            request.addRetryCountAndSetLastBackoff(backoff);
            std::this_thread::sleep_for(duration);

            ++request.retryCount;
        } else {
            error = Error::HTTPFailed;
            break;
        }
    }

    if (error == Error::None && !output.size()) {
        error = Error::EmptyResponse;
    }

    if (stress_) {
        auto duration = std::chrono::seconds(stress_);
        std::this_thread::sleep_for(duration);
    }
    
    return std::make_tuple(error, status, std::move(output));
}


