//
//  carousel.cpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/15/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#include "carousel.hpp"
#include "feedService.hpp"

#include <iostream>

extern const int32_t SCREEN_WIDTH;
extern const int32_t SCREEN_HEIGHT;

extern const std::string kTextureKeyLoadingIcon;
extern const std::string kTextureKeyLinkError;

static const std::string kNoGamesKey = "no-games";
static const std::string kNoGamesValue = "There are no available games.";
static const std::string kLoadingFeedKey = "loading-feed";
static const std::string kLoadingFeedValue = "LOADING...";

static const double kLoadingRotationRate = 360;

Carousel::Carousel(CarouselConfig config, const std::shared_ptr<TextureService>& texService, const std::shared_ptr<FontTextService>& fontTextService, bool verbose) : verbose_(verbose), state_(State::Initializing), config_(config), x_(0), y_(config.y), currThumb_(0), targetThumb_(0), targetX_(0), loadingRotate_(0), textureService_(texService), fontTextService_(fontTextService)  {

    backingW_ = config.thumbnailWidth + config.frameOffsetX * 2;
    backingH_ = config.thumbnailHeight + config.frameOffsetY * 2;

    auto minWidth = backingW_ * config.scaleDownPercentage;
    minNeighborDistance_ = minWidth + config.thumbnailBetweenSpacing;
    maxNeighborDistance_ = backingW_ / 2 + config.thumbnailBetweenSpacing + minWidth / 2;
    
    loadingIconTex_ = textureService_->getTexture(kTextureKeyLoadingIcon);
    linkErrorTex_ = textureService_->getTexture(kTextureKeyLinkError);
    noGamesTex_ = fontTextService->addString(FontTextService::Font::Roboto48, kNoGamesKey, kNoGamesValue, { 0xFF, 0xFF, 0, 0xFF});
    loadingTex_ = fontTextService->addString(FontTextService::Font::Roboto48, kLoadingFeedKey, kLoadingFeedValue, { 0xFF, 0xFF, 0, 0xFF});
}

Carousel::~Carousel() {
    thumbs_.clear();

    // Remove what we've created
    fontTextService_->removeString(kNoGamesKey);
    fontTextService_->removeString(kLoadingFeedKey);
}

bool Carousel::hasNext() const {
    auto next = currThumb_ + 1;
    return next < thumbs_.size() && getTargetX(next) >= lastPossibleX_;
}

bool Carousel::hasPrev() const {
    int32_t prev = static_cast<int32_t>(currThumb_) - 1;
    return prev >= 0 && getTargetX(prev) <= 0;
}

void Carousel::update(double deltaTime, DisplayList* displaylist, const Input& input) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto state = state_;
    lock.unlock();
    
    switch (state) {
        case State::Ready:
        case State::Loading: {
            if (thumbs_.size()) {
                bool movingLeft = targetThumb_ > currThumb_;
                bool movingRight = currThumb_ > targetThumb_;
                bool moving = movingLeft || movingRight;
                
                if (!moving) {
                    if (input.right && hasNext() && state != State::Loading) {
                        gotoNextThumb();
                    }
                    if (input.left && hasPrev() && state != State::Loading) {
                        gotoPrevThumb();
                    }
                }
                
                loadingRotate_ += kLoadingRotationRate * deltaTime;
                while (loadingRotate_ > 360) {
                    loadingRotate_ -= 360;
                }
                
                if (moving) {
                    if (targetThumb_ > currThumb_) {
                        // Need to move in the negative direction
                        x_ -= static_cast<double>(config_.scrollRate) * deltaTime;
                        if (x_ <= targetX_) {
                            x_ = targetX_;
                            currThumb_ = targetThumb_;
                        }
                    } else {
                        // Move in the positive direction
                        x_ += static_cast<double>(config_.scrollRate) * deltaTime;
                        if (x_ >= targetX_) {
                            x_ = targetX_;
                            currThumb_ = targetThumb_;
                        }
                    }
                }
                int32_t x = x_ + SCREEN_WIDTH / 2;
                for (int32_t i=0;i<thumbs_.size();++i) {
                    auto& thumb = thumbs_[i];
                    double scale = config_.scaleDownPercentage;
                    double step = 0;
                    if (movingLeft) {
                        if (i == (targetThumb_ - 1)) {
                            int32_t distance = abs(x - SCREEN_WIDTH / 2);
                            if (distance < maxNeighborDistance_) {
                                scale = config_.scaleDownPercentage + (static_cast<double>(maxNeighborDistance_ - distance) / static_cast<double>(maxNeighborDistance_)) * (1.0 - config_.scaleDownPercentage);
                            }
                            double nextScale = 1 - (scale - config_.scaleDownPercentage);
                            step = (backingW_ * scale) / 2 + (backingW_ * nextScale) / 2 + config_.thumbnailBetweenSpacing;
                        } else if (i == targetThumb_) {
                            int32_t distance = abs(x - SCREEN_WIDTH / 2);
                            if (distance < maxNeighborDistance_) {
                                scale = config_.scaleDownPercentage + (static_cast<double>(maxNeighborDistance_ - distance) / static_cast<double>(maxNeighborDistance_)) * (1.0 - config_.scaleDownPercentage);
                            }
                            step = (backingW_ * scale) / 2 + (backingW_ * config_.scaleDownPercentage) / 2 + config_.thumbnailBetweenSpacing;
                        } else {
                            step = backingW_ * config_.scaleDownPercentage + config_.thumbnailBetweenSpacing;
                        }
                    } else if (movingRight) {
                        if (i == (targetThumb_ + 1)) {
                            int32_t distance = abs(x - SCREEN_WIDTH / 2);
                            double prevScale = 1;
                            if (distance < maxNeighborDistance_) {
                                prevScale = config_.scaleDownPercentage + (static_cast<double>(distance) / static_cast<double>(maxNeighborDistance_)) * (1.0 - config_.scaleDownPercentage);
                            }
                            scale = (1 - prevScale) + config_.scaleDownPercentage;
                            step = (backingW_ * scale) / 2 + (backingW_ * config_.scaleDownPercentage) / 2 + config_.thumbnailBetweenSpacing;
                        } else if (i == targetThumb_) {
                            int32_t distance = abs(x - SCREEN_WIDTH / 2);
                            if (distance < maxNeighborDistance_) {
                                scale = config_.scaleDownPercentage + (static_cast<double>(maxNeighborDistance_ - distance) / static_cast<double>(maxNeighborDistance_)) * (1.0 - config_.scaleDownPercentage);
                            }
                            double nextScale = 1 - (scale - config_.scaleDownPercentage);
                            step = (backingW_ * scale) / 2 + (backingW_ * nextScale ) / 2 + config_.thumbnailBetweenSpacing;
                        } else {
                            step = backingW_ * config_.scaleDownPercentage + config_.thumbnailBetweenSpacing;
                        }
                    } else {
                         // We either aren't moving or our neighbor
                        if (i == currThumb_) {
                            scale = 1;
                            // This means our next neighbor must be the downscaled size
                            step = backingW_ / 2 + (backingW_ * config_.scaleDownPercentage) / 2 + config_.thumbnailBetweenSpacing;
                        } else {
                            if ((i + 1) == currThumb_) {
                                // Our next neighbor is the regular thumb
                                step = backingW_ / 2 + (backingW_ * config_.scaleDownPercentage) / 2 + config_.thumbnailBetweenSpacing;
                            } else {
                                step = backingW_ * config_.scaleDownPercentage + config_.thumbnailBetweenSpacing;
                            }
                        }
                    }

                    auto w = backingW_ * scale;
                    auto h = backingH_ * scale;
                    
                    thumb.x = x;
                    thumb.y = y_;
                    thumb.scale = scale;

                    displaylist->addRect(thumb.x, thumb.y, w, h, config_.backingColor);
                    
                    if (!moving && i == currThumb_) {
                        displaylist->addFrameRect(thumb.x, thumb.y, w, h, config_.frameColor);
                    }

                    uint32_t colorOp = DisplayObject::ColorOp::None;
                    Color grayed = { 0x40, 0x40, 0x40, 0x80 };

                    if (state == State::Loading) {
                        colorOp = DisplayObject::ColorOp::Tint | DisplayObject::ColorOp::Alpha;
                    }
                    
                    if ((!moving && i == currThumb_) || (moving && i == targetThumb_)) {
                        // Add our header and description
                        auto y = thumb.y - h / 2 - (thumb.headline->getHeight() * scale) / 2;
                        displaylist->addScaledTexture(thumb.headline, thumb.x, y, scale, scale, colorOp, grayed);
                        y = thumb.y + h / 2 + (thumb.description->getHeight() * scale) / 2;
                        displaylist->addScaledTexture(thumb.description, thumb.x, y, scale, scale, colorOp, grayed);
                    }
                    
                    if ((movingLeft && i == (targetThumb_ - 1)) || (movingRight && i == (targetThumb_ + 1))) {
                        // Add our header and description
                        auto y = thumb.y - h / 2 - (thumb.headline->getHeight() * scale) / 2;
                        displaylist->addScaledTexture(thumb.headline, thumb.x, y, scale, scale, colorOp, grayed);
                        y = thumb.y + h / 2 + (thumb.description->getHeight() * scale) / 2;
                        displaylist->addScaledTexture(thumb.description, thumb.x, y, scale, scale, colorOp, grayed);
                    }
                    
                    // Fix texture if needed
                    if (!thumb.thumb && thumb.recap->getThumbnailState() == FeedGameRecap::ThumbnailState::Loaded) {
                        thumb.thumb = textureService_->getTexture(FeedService::getThumbnailKeyForRecap(thumb.recap->date, thumb.recap->park));
                    }
                    
                    if (thumb.thumb) {
                        displaylist->addScaledTexture(thumb.thumb, thumb.x, thumb.y, scale, scale, colorOp, grayed);
                    } else {
                        // We are either error or loading
                        if (thumb.recap->getThumbnailState() == FeedGameRecap::ThumbnailState::Loading) {
                            displaylist->addXformTexture(loadingIconTex_, thumb.x, thumb.y, loadingRotate_, scale, scale, colorOp, grayed);
                        } else if (thumb.recap->getThumbnailState() == FeedGameRecap::ThumbnailState::Error) {
                            displaylist->addScaledTexture(linkErrorTex_, thumb.x, thumb.y, scale, scale, colorOp, grayed);
                        }
                    }
                    
                    x += step;
                }
                if (state == State::Loading) {
                    displaylist->addTexture(loadingTex_, SCREEN_WIDTH / 2, y_);
                }
            } else {
                if (state == State::Loading) {
                    displaylist->addTexture(loadingTex_, SCREEN_WIDTH / 2, y_);
                } else {
                    displaylist->addTexture(noGamesTex_, SCREEN_WIDTH / 2, y_);
                }
            }
        }
            break;
        default:
            break;
    }
}

void Carousel::setFeed(const std::shared_ptr<Feed>& feed) {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = State::Ready;
    currFeed_ = feed;
    state_ = State::Ready;
    
    x_ = 0;
    currThumb_ = 0;
    targetThumb_ = 0;
    targetX_ = 0;

    auto numRecaps = feed->getNumRecaps();
    thumbs_.clear();
    for (decltype(numRecaps) i=0;i<numRecaps;++i) {
        auto recap = feed->getRecapAtIndex(i);
        Thumbnail thumb(recap, config_.thumbnailWidth + 2 * config_.frameOffsetX, config_.thumbnailHeight + 2 * config_.frameOffsetY);
        // Get the needed textures
        auto key = FeedService::getHeadlineKeyForRecap(feed->getDate(), recap->park);
        thumb.headline = fontTextService_->getString(key);
        key = FeedService::getDescriptionKeyForRecap(feed->getDate(), recap->park);
        thumb.description = fontTextService_->getString(key);
        updateThumbnailThumb(thumb);
        thumbs_.emplace_back(thumb);
    }
    if (thumbs_.size()) {
        lastPossibleX_ = getTargetX(static_cast<int32_t>(thumbs_.size() - 1));
    } else {
        lastPossibleX_ = 0;
    }
}

void Carousel::loadingNextFeed() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = State::Loading;
}


void Carousel::updateThumbnailThumb(Thumbnail& thumb) {
    if (thumb.recap->getThumbnailState() == FeedGameRecap::ThumbnailState::Loaded) {
        auto key = FeedService::getThumbnailKeyForRecap(thumb.recap->date, thumb.recap->park);
        thumb.thumb = textureService_->getTexture(key);
    }
}

void Carousel::gotoNextThumb() {
    if (hasNext()) {
        targetThumb_ = currThumb_ + 1;
        targetX_ = getTargetX(targetThumb_);
    }
}

void Carousel::gotoPrevThumb() {
    if (hasPrev()) {
        targetThumb_ = currThumb_ - 1;
        targetX_ = getTargetX(targetThumb_);
    }
}

int Carousel::getTargetX(int32_t index) const {
    if (index > 0) {
        return -(maxNeighborDistance_ + (index - 1) * minNeighborDistance_);
    }
    return 0;
}
