//
//  feedService.hpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/17/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef feedService_hpp
#define feedService_hpp

#include <stdio.h>
#include "errors.hpp"
#include "resourceFetcherService.hpp"
#include "textureService.hpp"
#include "fontTextService.hpp"
#include "feed.hpp"

#include <memory>
#include <functional>
#include <mutex>
#include <unordered_map>

class FeedService {
public:
    FeedService() = delete;
    FeedService(const std::shared_ptr<ResourceFetcherService>& fetcher, const std::shared_ptr<TextureService>& texService, const std::shared_ptr<FontTextService>& fontTextService, int wrapLimit, bool verbose);
    ~FeedService();

    static std::string getHeadlineKeyForRecap(const std::string& date, size_t recap);
    static std::string getDescriptionKeyForRecap(const std::string& date, size_t recap);
    static std::string getThumbnailKeyForRecap(const std::string& date, size_t recap);

    // The plan right now is to support getting different feeds based on available dates from dates_
    // We use this as our first date to fetch AND in case there isn't enough time for full implementation
    // This is very much a curated experience
    std::string getDefaultDate() const;
    int32_t getDefaultDateIndex() const;
    size_t getNumDates() const;
    std::string getDateAtIndex(size_t index) const;
    
    void fetchFeed(const std::string& date, std::function<void(Error error, uint32_t status, std::shared_ptr<Feed>)> callback);
    
    std::shared_ptr<Feed> getFeed(const std::string& date);
    
    void removeFeed(const std::string& date);
    void removeFeed(const std::shared_ptr<Feed>& feed);
    
private:
    bool                                    verbose_;
    std::shared_ptr<ResourceFetcherService> fetcher_;
    std::shared_ptr<TextureService>         textureService_;
    std::shared_ptr<FontTextService>        fontTextService_;
    FontTextService::Font                   headlineFont_;
    FontTextService::Font                   descriptionFont_;

    int32_t                                 defaultDateIndex_;
    int                                     wrapLimit_;
    
    std::vector<std::string>                dates_;

    std::mutex                              mutex_;
    std::unordered_map<std::string, std::shared_ptr<Feed>>   feeds_;
    
    std::string getFeedUrl(const std::string& date) const;
};

#endif /* feedService_hpp */
