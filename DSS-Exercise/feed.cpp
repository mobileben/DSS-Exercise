//
//  feed.cpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/15/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#include "feed.hpp"

#include <iostream>

static const char *kFeedDataKeyCopyright = "copyright";
static const char *kFeedDataKeyDates = "dates";

static const char *kGameDateKeyDate = "date";
static const char *kGameDateKeyGames = "games";

static const char *kGameKeyGameDate = "gameDate";
static const char *kGameKeyGamePark = "gamePk";
static const char *kGameKeyContent = "content";

static const char *kGameContentKeyEditorial = "editorial";

static const char *kGameContentEditorialKeyRecap = "recap";

static const char *kGameContentEditorialPerspectiveMLB = "mlb";

static const char *kGameRecapKeyDate = "date";
static const char *kGameRecapKeyHeadline = "headline";
static const char *kGameRecapKeySubhead = "subhead";
static const char *kGameRecapKeySeoTitle = "seoTitle";
static const char *kGameRecapKeyBlurb = "blurb";
static const char *kGameRecapKeyImage = "image";

static const char *kGameRecapImageKeyTitle = "title";
static const char *kGameRecapImageKeyAltText = "altText";
static const char *kGameRecapImageKeyCuts = "cuts";

static const char *kGameRecapKeyCutKeyAspectRatio = "aspectRatio";
static const char *kGameRecapKeyCutKeyWidth = "width";
static const char *kGameRecapKeyCutKeyHeight = "height";
static const char *kGameRecapKeyCutKeySrc = "src";
static const char *kGameRecapKeyCutKeyAt2x = "at2x";
static const char *kGameRecapKeyCutKeyAt3x = "at3x";

bool GameRecapCut::fromJson(const rapidjson::Value& json) {
    using namespace rapidjson;
    try {
        if (json.HasMember(kGameRecapKeyCutKeyAspectRatio)) {
            aspectRatio = json[kGameRecapKeyCutKeyAspectRatio].GetString();
        } else {
            return false;
        }
        if (json.HasMember(kGameRecapKeyCutKeyWidth)) {
            width = json[kGameRecapKeyCutKeyWidth].GetUint();
        } else {
            return false;
        }
        if (json.HasMember(kGameRecapKeyCutKeyHeight)) {
            height = json[kGameRecapKeyCutKeyHeight].GetUint();
        } else {
            return false;
        }
        if (json.HasMember(kGameRecapKeyCutKeySrc)) {
            src1x = json[kGameRecapKeyCutKeySrc].GetString();
        } else {
            return false;
        }
        if (json.HasMember(kGameRecapKeyCutKeyAt2x)) {
            src2x = json[kGameRecapKeyCutKeyAt2x].GetString();
        } else {
            return false;
        }
        if (json.HasMember(kGameRecapKeyCutKeyAt3x)) {
            src3x = json[kGameRecapKeyCutKeyAt3x].GetString();
        } else {
            return false;
        }
        return true;
    } catch (std::exception& e) {
        return false;
    }
}

bool GameRecapImage::fromJson(const rapidjson::Value& json) {
    using namespace rapidjson;
    try {
        if (json.HasMember(kGameRecapImageKeyTitle)) {
            title = json[kGameRecapImageKeyTitle].GetString();
        } else {
            return false;
        }
        if (json.HasMember(kGameRecapImageKeyAltText)) {
            altText = json[kGameRecapImageKeyAltText].GetString();
        } else {
            return false;
        }
        if (json.HasMember(kGameRecapImageKeyCuts)) {
            cuts.clear();
            
            auto array = json[kGameRecapImageKeyCuts].GetArray();
            cuts.reserve(array.Size());
            for (auto it=array.Begin();it!=array.End();++it) {
                GameRecapCut cut;
                if (cut.fromJson(*it)) {
                    cuts.push_back(std::move(cut));
                } else {
                    return false;
                }
            }
        } else {
            return false;
        }
        return true;
    } catch (std::exception& e) {
        return false;
    }
}

bool GameRecap::fromJson(const rapidjson::Value& json) {
    using namespace rapidjson;
    try {
        if (json.HasMember(kGameRecapKeyDate)) {
            date = json[kGameRecapKeyDate].GetString();
        } else {
            return false;
        }
        if (json.HasMember(kGameRecapKeyHeadline)) {
            headline = json[kGameRecapKeyHeadline].GetString();
        } else {
            return false;
        }
        if (json.HasMember(kGameRecapKeySubhead)) {
            subhead = json[kGameRecapKeySubhead].GetString();
        } else {
            return false;
        }
        if (json.HasMember(kGameRecapKeySeoTitle)) {
            seoTitle = json[kGameRecapKeySeoTitle].GetString();
        } else {
            return false;
        }
        if (json.HasMember(kGameRecapKeyBlurb)) {
            blurb = json[kGameRecapKeyBlurb].GetString();
        } else {
            return false;
        }
        if (json.HasMember(kGameRecapKeyImage)) {
            if (!image.fromJson(json[kGameRecapKeyImage])) {
                return false;
            }
        } else {
            return false;
        }
        return true;
    } catch (std::exception& e) {
        return false;
    }
}


bool GameContentEditorial::fromJson(const rapidjson::Value& json) {
    using namespace rapidjson;
    try {
        if (json.HasMember(kGameContentEditorialKeyRecap)) {
            recaps.clear();
            
            auto dict = json[kGameContentEditorialKeyRecap].GetObject();
            for (auto it=dict.MemberBegin();it!=dict.MemberEnd();++it) {
                GameRecap recap;
                if (recap.fromJson(it->value)) {
                    recaps[it->name.GetString()] = recap;
                } else {
                    return false;
                }
            }
            // For this assignment, we are working on the assumption that 'mlb' is a required key
            auto it = recaps.find(kGameContentEditorialPerspectiveMLB);
            if (it == recaps.end()) {
                // Fail if 'mlb' does not exist
                return false;
            }
        } else {
            return false;
        }
        return true;
    } catch (std::exception& e) {
        return false;
    }
}


bool GameContent::fromJson(const rapidjson::Value& json) {
    using namespace rapidjson;
    try {
        if (json.HasMember(kGameContentKeyEditorial)) {
            if (!editorial.fromJson(json[kGameContentKeyEditorial])) {
                return false;
            }
        } else {
            return false;
        }
        return true;
    } catch (std::exception& e) {
        return false;
    }
}

bool Game::fromJson(const rapidjson::Value& json) {
    using namespace rapidjson;
    try {
        if (json.HasMember(kGameKeyGameDate)) {
            gameDate = json[kGameKeyGameDate].GetString();
        } else {
            return false;
        }
        if (json.HasMember(kGameKeyGamePark)) {
            gamePark = json[kGameKeyGamePark].GetUint();
        } else {
            return false;
        }
        if (json.HasMember(kGameKeyContent)) {
            if (!content.fromJson(json[kGameKeyContent])) {
                return false;
            }
        } else {
            return false;
        }        return true;
    } catch (std::exception& e) {
        return false;
    }
}

bool GameDate::fromJson(const rapidjson::Value& json) {
    using namespace rapidjson;
    try {
        if (json.HasMember(kGameDateKeyDate)) {
            date = json[kGameDateKeyDate].GetString();
        } else {
            return false;
        }
        if (json.HasMember(kGameDateKeyGames)) {
            games.clear();
            
            auto array = json[kGameDateKeyGames].GetArray();
            games.reserve(array.Size());
            for (auto it=array.Begin();it!=array.End();++it) {
                Game game;
                if (game.fromJson(*it)) {
                    games.push_back(std::move(game));
                } else {
                    return false;
                }
            }
        } else {
            return false;
        }
        return true;
    } catch (std::exception& e) {
        return false;
    }
}

uint32_t FeedData::getNumGames() const {
    // We assume we have either 0 or 1 dates
    if (dates.size()) {
        auto& date = dates.front();
        return static_cast<uint32_t>(date.games.size());
    }
    return 0;
}

bool FeedData::fromJson(const rapidjson::Value& json) {
    using namespace rapidjson;
    try {
        if (json.HasMember(kFeedDataKeyCopyright)) {
            copyright = json[kFeedDataKeyCopyright].GetString();
        } else {
            return false;
        }
        if (json.HasMember(kFeedDataKeyDates)) {
            dates.clear();
            
            auto array = json[kFeedDataKeyDates].GetArray();
            dates.reserve(array.Size());
            for (auto it=array.Begin();it!=array.End();++it) {
                GameDate date;
                if (date.fromJson(*it)) {
                    dates.push_back(std::move(date));
                } else {
                    return false;
                }
            }
        } else {
            return false;
        }
        return true;
    } catch (std::exception& e) {
        return false;
    }
}

FeedGameRecap::FeedGameRecap(const std::string date, uint32_t park, const std::string& headline, const std::string& description, const std::string& thumbnailUrl) : date(date), park(park), headline(headline), description(description), thumbnailUrl(thumbnailUrl),  thumbnailState_(ThumbnailState::Unloaded) {
}

Feed::Feed(const std::string& date, FeedData data) : date_(date) {
    // Build list of recaps
    // Right now we generate based on what our controlled environment is
    // Is this case, we know we have only one date (or possibly none).
    // If for some strange reason, we have more than one date, only use the first date
    if (data.dates.size()) {
        auto& gameDay = data.dates.front();
        for (auto& game : gameDay.games) {
            auto park = game.gamePark;
            auto it = game.content.editorial.recaps.find(kGameContentEditorialPerspectiveMLB);
            if (it != game.content.editorial.recaps.end()) {
                // We only care (and expect the mlb perspective)
                // Note I inquired about this and was told I can ignore this
                auto& recap = it->second;
                // First find our image. We are not writing fallback code
                // We are using 16:9 images which are 480x270. If we do not have a match, we will ignore this
                std::string thumbnail;
                for (auto cut : recap.image.cuts) {
                    // Yes these are hard coded right now, these should be passed in as params, but I am not doing that right now
                    if (cut.aspectRatio == "16:9" && cut.width == 480 && cut.height == 270) {
                        thumbnail = cut.src1x;
                        break;
                    }
                }
                if (thumbnail.size()) {
                    // Extract headline and then find best description based on fallback
                    auto& headline = recap.headline;    // We assume one always exists
                    std::string description;
                    if (recap.subhead.size() && recap.subhead != headline) {
                        // Use subhead
                        description = recap.subhead;
                    } else if (recap.seoTitle.size() && recap.seoTitle != headline) {
                        // Use SEO Title
                        description = recap.seoTitle;
                    } else if (recap.blurb.size()){
                        // Blurbs are long, so only take 200 characters and we append an ellipsis
                        // OMG, did I just hard code this value? Yes I did! Bad bad bad :D!
                        if (recap.blurb.size() > 200) {
                            description = recap.blurb.substr(0, 200);
                            description += "...";
                        } else {
                            description = recap.blurb;
                        }
                    } else {
                        description = "You're Out! Could not find a valid description.";
                    }
                    
                    recaps_.emplace_back(std::make_shared<FeedGameRecap>(date, park, headline, description, thumbnail));
                } else {
                    std::cerr << "WARNING: Could not find 16:9 (480x270) image for Game Day " << date << " at park " << park << ". Ignoring..." << std::endl;
                }
            } else {
                std::cerr << "WARNING: Game Day " << date << " at park " << park << " JSON does not contain a \"mlb\" recap. Ignoring..." << std::endl;
            }
        }
    }
}

Feed::~Feed() {
}

std::shared_ptr<FeedGameRecap> Feed::getRecapAtIndex(size_t index) const {
    if (index < recaps_.size()) {
        return recaps_[index];
    }
    return nullptr;
}
