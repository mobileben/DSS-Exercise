//
//  feed.hpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/15/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef feed_hpp
#define feed_hpp

#include <stdio.h>
#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <mutex>

#include "json.hpp"
#include "texture.hpp"

struct GameRecapCut : public JsonSerializer {
    std::string aspectRatio;
    uint32_t    width;
    uint32_t    height;
    std::string src1x;
    std::string src2x;
    std::string src3x;

    bool fromJson(const rapidjson::Value& json);
};

struct GameRecapImage : public JsonSerializer {
    std::string                 title;
    std::string                 altText;
    std::vector<GameRecapCut>   cuts;
    
    bool fromJson(const rapidjson::Value& json);
};

struct GameRecap : public JsonSerializer {
    std::string     date;
    std::string     headline;
    std::string     subhead;
    std::string     seoTitle;   // Fallback
    std::string     blurb;      // Fallback
    GameRecapImage  image;

    bool fromJson(const rapidjson::Value& json);
};

struct GameContentEditorial : public JsonSerializer {
    std::map<std::string, GameRecap>    recaps;
    
    bool fromJson(const rapidjson::Value& json);
};

struct GameContent : public JsonSerializer {
    GameContentEditorial    editorial;
    
    bool fromJson(const rapidjson::Value& json);
};

struct Game : public JsonSerializer {
    std::string gameDate;
    uint32_t    gamePark;   //  Game Park is unique key we can use
    GameContent content;
    
    bool fromJson(const rapidjson::Value& json);
};

struct GameDate : public JsonSerializer {
    std::string         date;
    std::vector<Game>   games;
    
    bool fromJson(const rapidjson::Value& json);
};

struct FeedData : public JsonSerializer {
    std::string             copyright;
    std::vector<GameDate>   dates;
    
    // Convenience methods specifically for assignment. These shortcut normal methods one would employ to pull data out of FeedData
    uint32_t getNumGames() const;
    
    // This will throw an exception if the game does not exist.
    // Expected to be used in conjuction with getNumGames()
    Game getGame(uint32_t num) const;
    
    bool fromJson(const rapidjson::Value& json);
};

class FeedService;
class Carousel;
class Feed;

// This is the struct in use with the carousel thumbnail
struct FeedGameRecap {
    enum class ThumbnailState { Unloaded, Loading, Loaded, Error };
    std::string                 date;
    uint32_t                    park;
    std::string                 headline;
    std::string                 description;
    std::string                 thumbnailUrl;
    
    FeedGameRecap() : park(0), thumbnailState_(ThumbnailState::Unloaded) {}
    FeedGameRecap(const std::string date, uint32_t park, const std::string& headline, const std::string& description, const std::string& thumbnailUrl);
    
private:
    friend class Feed;
    friend class FeedService;
    friend class Carousel;
    
    std::atomic<ThumbnailState> thumbnailState_;

    ThumbnailState getThumbnailState() const { return thumbnailState_; }
    void setThumbnailState(ThumbnailState state) { thumbnailState_ = state; }
};

class Feed {
public:
    Feed() = delete;
    Feed(const std::string& date, FeedData data);
    ~Feed();
    
    std::string getDate() const { return date_; }
    size_t getNumRecaps() const { return recaps_.size(); }
    std::shared_ptr<FeedGameRecap> getRecapAtIndex(size_t index) const;
    
private:
    friend FeedService;
    
    std::string                                 date_;  // Date serves as the primary key for the feed
    std::vector<std::shared_ptr<FeedGameRecap>> recaps_;
    
    std::mutex                                  mutex_;
    std::vector<std::shared_ptr<Texture>>       strings_;
    std::vector<std::shared_ptr<Texture>>       thumbnails_;
};


#endif /* feed_hpp */
