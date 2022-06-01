//
//  resourceFetcherService.hpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/16/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef resourceFetcherService_hpp
#define resourceFetcherService_hpp

#include <stdio.h>
#include "workerPool.h"
#include "errors.hpp"

#include <string>
#include <functional>

class ResourceFetcherService {
public:
    ResourceFetcherService() = delete;
    ResourceFetcherService(uint32_t numWorkers, uint32_t stress, bool verbose);

    // Callback responsible for copying string if needed
    void add(const std::string& url, std::function<void(Error error, uint32_t statusCode, const std::vector<uint8_t>&)> callback);

private:
    class Job {
    public:
        Job() {}
        Job(std::string url, std::function<void(Error error, uint32_t statusCode, const std::vector<uint8_t>&)> cb, uint32_t stress, bool verbose) : verbose_(verbose), stress_(stress), url_(url), callback_(cb) {}

        void execute();

    private:
        bool        verbose_;
        uint32_t    stress_;
        std::string url_;
        std::function<void(Error error, uint32_t statusCode, const std::vector<uint8_t>&)> callback_;

        static std::size_t curlWriteCallback(const char *in, std::size_t size, std::size_t num, std::vector<uint8_t>* out);
        std::pair<Error, std::vector<uint8_t>> loadFile();    // Will throw exception on error
        std::tuple<Error, uint32_t, std::vector<uint8_t>> fetchFile();   // Will throw exception on error
    };
    
    bool            verbose_;
    uint32_t        stress_;
    WorkerPool<Job> workerPool_;
};

#endif /* resourceFetcherService_hpp */
