//
//  feedService.cpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/17/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#include "feedService.hpp"
#include "rapidjson/document.h"
#include "rapidjson/error/error.h"
#include "rapidjson/error/en.h"
#include "types.h"

#include <iostream>

// At the time of this update, the MLB API hydration for `game(content(editorial(recap)))` is broken, returning a "Internal error occurred". I found that using `editorial(all)` will work, though it returns a much larger payload.
//static const std::string kBaseFeedUrl = "http://statsapi.mlb.com/api/v1/schedule?hydrate=game(content(editorial(recap))),decisions&date=";
static const std::string kBaseFeedUrl = "http://statsapi.mlb.com/api/v1/schedule?hydrate=game(content(editorial(all))),decisions&date=";
static const std::string kTrailingFeedQueryParam = "&sportId=1";

FeedService::FeedService(const std::shared_ptr<ResourceFetcherService>& fetcher, const std::shared_ptr<TextureService>& texService, const std::shared_ptr<FontTextService>& fontTextService, int wrapLimit, bool verbose) : fetcher_(fetcher), textureService_(texService), fontTextService_(fontTextService), wrapLimit_(wrapLimit), verbose_(verbose) {
    headlineFont_ = FontTextService::Font::Roboto22;
    descriptionFont_ = FontTextService::Font::Roboto20;
    
    dates_ = {
        "2022-05-01",
        "2022-05-02",
        "2022-05-03",
        "2022-05-04",
        "2022-05-05",
        "2022-05-06",
        "2022-05-07"
    };
    
    defaultDateIndex_ = static_cast<int32_t>(dates_.size() / 2);
}

FeedService::~FeedService() {
    std::lock_guard<std::mutex> lock(mutex_);
    feeds_.clear();
    fetcher_ = nullptr;
    textureService_ = nullptr;
}

std::string FeedService::getHeadlineKeyForRecap(const std::string& date, size_t recap) {
    return date + "-" + std::to_string(recap) + "-headline";
}

std::string FeedService::getDescriptionKeyForRecap(const std::string& date, size_t recap) {
    return date + "-" + std::to_string(recap) + "-description";
}

std::string FeedService::getThumbnailKeyForRecap(const std::string& date, size_t recap) {
    return date + "-" + std::to_string(recap) + "-thumbnail";
}

std::string FeedService::getDefaultDate() const {
    return std::string(dates_[defaultDateIndex_]);
}

int32_t FeedService::getDefaultDateIndex() const {
    return defaultDateIndex_;
}

size_t FeedService::getNumDates() const {
    return dates_.size();
}

std::string FeedService::getDateAtIndex(size_t index) const {
    if (index < dates_.size()) {
        return dates_[index];
    }
    return std::string();
}


void FeedService::fetchFeed(const std::string& date, std::function<void(Error error, uint32_t status, std::shared_ptr<Feed>)> callback)  {
    auto existing = getFeed(date);
    if (existing) {
        if (callback) {
            // 200 for HTTP Status Code OK
            callback(Error::None, 200, existing);
        }
    } else {
        // Make local copy to capture
        std::string feedDate = date;
        fetcher_->add(getFeedUrl(feedDate), [this, feedDate, callback](Error error, uint32_t status, const std::vector<uint8_t> buffer) {
            std::shared_ptr<Feed> feed;
            if (error == Error::None) {
                using namespace rapidjson;
                // Rapidjson and parsing can throw
                try {
                    auto doc = rapidjson::Document();
                    std::string str(buffer.begin(), buffer.end());
                    doc.Parse<kParseStopWhenDoneFlag>(reinterpret_cast<const char *>(buffer.data()));
                    if (!doc.HasParseError()) {
                        FeedData data;
                        data.fromJson(doc);
                        
                        std::unique_lock<std::mutex> lock(mutex_);
                        auto feed = std::make_shared<Feed>(feedDate, data);
                        std::lock_guard<std::mutex> feedLock(feed->mutex_);
                        feeds_[feedDate] = feed;
                        lock.unlock();
                        
                        // Let's now build all the strings we need
                        // We do this in a two pass system for now, mainly to just stack the different types more cleanly
                        auto numRecaps = feed->getNumRecaps();
                        Color white{0xFF, 0xFF, 0xFF, 0xFF};
                        for (decltype(numRecaps) i=0;i<numRecaps;++i) {
                            auto recap = feed->getRecapAtIndex(i);
                            // Key for headlines is date-index-headline
                            auto key = FeedService::getHeadlineKeyForRecap(feedDate, recap->park);
                            auto tex = fontTextService_->addString(headlineFont_, key, recap->headline, white, wrapLimit_);
                            feed->strings_.push_back(tex);
                            key = FeedService::getDescriptionKeyForRecap(feedDate, recap->park);
                            tex = fontTextService_->addString(descriptionFont_, key, recap->description, white, wrapLimit_);
                            feed->strings_.push_back(tex);
                            key = FeedService::getThumbnailKeyForRecap(feedDate, recap->park);
                            recap->thumbnailState_ = FeedGameRecap::ThumbnailState::Loading;
                            textureService_->createTexture(key, recap->thumbnailUrl, [key, recap](Error error, std::shared_ptr<Texture> texture) {
                                recap->setThumbnailState(error == Error::None ? FeedGameRecap::ThumbnailState::Loaded : FeedGameRecap::ThumbnailState::Error);
                            });
                        }
                        // Now let's prime to all our thumbs to load
                    } else {
                        std::cerr << "Parse error " << buffer.size() << " " << GetParseError_En(doc.GetParseError()) << std::endl;
                        std::string str(buffer.begin(), buffer.end());
                        std::cout << str << std::endl;
                        error = Error::JSONParseError;
                    }
                } catch (std::exception& e) {
                    std::cerr << "Parse exception " << e.what() << std::endl;
                    error = Error::JSONParseError;
                }
            }
            
            if (callback) {
                callback(error, status, feed);
            }
        });

    }
}

std::shared_ptr<Feed> FeedService::getFeed(const std::string& date) {
    auto it = feeds_.find(date);
    if (it != feeds_.end()) {
        return it->second;
    }
    return nullptr;
}

void FeedService::removeFeed(const std::string& date) {
    auto it = feeds_.find(date);
    if (it != feeds_.end()) {
        auto feed = it->second;
        
        // It is up to us to dispose of the textures
        std::lock_guard<std::mutex> lock(feed->mutex_);
        auto numRecaps = feed->getNumRecaps();
        for (decltype(numRecaps) i=0;i<numRecaps;++i) {
            auto recap = feed->getRecapAtIndex(i);
            // Key for headlines is date-index-headline
            auto key = FeedService::getHeadlineKeyForRecap(date, i);
            fontTextService_->removeString(key);
            key = FeedService::getDescriptionKeyForRecap(date, i);
            fontTextService_->removeString(key);
            key = FeedService::getThumbnailKeyForRecap(date, i);
            textureService_->removeTexture(key);
        }

        feed->strings_.clear();
        feed->thumbnails_.clear();
    }
}

void FeedService::removeFeed(const std::shared_ptr<Feed>& feed) {
    for (auto it : feeds_) {
        if (it.second == feed) {
            removeFeed(it.first);
            return;
        }
    }
}

std::string FeedService::getFeedUrl(const std::string& date) const {
    // Construction of feed URL is a hack. We simply insert the date between kBaseFeedUrl and kTrailingFeedQueryParam
    // Proper construction typically involves a proper Request class which takes in headers as well as query params.
    // Hence proper construction would involve adding the propery qp as well as the qp for date=${DATE}.
    // I've implemented several variants of this for various projects, however this I have only implemented a bare bones requestor devoid
    // of such functionality. If for some reason there is a question of being able to do this, I have source which I can walk people through.
    // The Request struct was a stripped down version of one of my implementations, where I left in a stub for Method as well as backoff for retries
    
    // We also assume date is the proper format
    return kBaseFeedUrl + date + kTrailingFeedQueryParam;
}
