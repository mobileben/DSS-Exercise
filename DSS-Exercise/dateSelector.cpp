//
//  dateSelector.cpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/18/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#include "dateSelector.hpp"

DateSelector::DateSelector(const std::shared_ptr<TextureService>& texService, const std::shared_ptr<FontTextService>& fontTexService, const std::shared_ptr<FeedService>& feedService, const std::shared_ptr<Carousel>& carousel, int x, int y, bool verbose) : verbose_(verbose), carousel_(carousel), textureService_(texService), fontTexService_(fontTexService), feedService_(feedService), state_(State::Ready), x_(x), y_(y) {
    currDateIndex_ = feedService->getDefaultDateIndex();
    auto num = feedService->getNumDates();
    for (decltype(num) i=0;i<num;++i) {
        auto date = feedService->getDateAtIndex(i);
        auto tex = fontTexService->addString(FontTextService::Font::Roboto48, date, date, { 0xFF, 0xFF, 0xFF, 0xFF });
        if (tex) {
            dateTex_.push_back(tex);
        }
    }
}

void DateSelector::update(double deltaTime, DisplayList* displaylist, const Input& input) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto state = state_;
    lock.unlock();
    switch (state) {
        case State::Ready:
            if (input.up && hasPrev()) {
                gotoPrev();
            }
            if (input.down && hasNext()) {
                gotoNext();
            }
            break;
        case State::NotifyCarousel: {
            lock.lock();
            state_ = State::Ready;
            auto date = feedService_->getDateAtIndex(currDateIndex_);
            auto feed = feedService_->getFeed(date);
            if (feed) {
                carousel_->setFeed(feed);
            }
            lock.unlock();
        }
        default:
            break;
    }

    uint32_t colorOp = DisplayObject::ColorOp::None;
    Color grayed = { 0x40, 0x40, 0x40, 0xFF };

    if (state == State::Fetching) {
        colorOp = DisplayObject::ColorOp::Tint | DisplayObject::ColorOp::Alpha;
    }

    auto tex = dateTex_[currDateIndex_];
    displaylist->addTexture(tex, x_, y_ - tex->getHeight() / 2, colorOp, grayed);
}

bool DateSelector::hasNext() {
    auto num = feedService_->getNumDates();
    auto next = currDateIndex_ + 1;
    return next < num;
}

bool DateSelector::hasPrev() {
    auto prev = currDateIndex_ - 1;
    return prev >= 0;
}

int DateSelector::getHeight() const {
    auto tex = dateTex_[currDateIndex_];
    if (tex) {
        return tex->getHeight();
    }
    return 0;
}


void DateSelector::gotoNext() {
    // unique_lock used here because fetchFeed may actually return on the same thread, so lock_guard will deadlock
    std::unique_lock<std::mutex> lock(mutex_);
    auto state = state_;
    lock.unlock();
    if (hasNext() && state == State::Ready) {
        currDateIndex_++;
        auto date = feedService_->getDateAtIndex(currDateIndex_);
        lock.lock();
        state_ = State::Fetching;
        lock.unlock();
        carousel_->loadingNextFeed();
        feedService_->fetchFeed(date, [this](Error error, uint32_t status, std::shared_ptr<Feed> feed) {
            std::lock_guard<std::mutex> lock(mutex_);
            state_ = State::NotifyCarousel;
        });
    }
}

void DateSelector::gotoPrev() {
    // unique_lock used here because fetchFeed may actually return on the same thread, so lock_guard will deadlock
    std::unique_lock<std::mutex> lock(mutex_);
    auto state = state_;
    lock.unlock();
    if (hasPrev() && state == State::Ready) {
        currDateIndex_--;
        auto date = feedService_->getDateAtIndex(currDateIndex_);
        lock.lock();
        state_ = State::Fetching;
        lock.unlock();
        carousel_->loadingNextFeed();
        feedService_->fetchFeed(date, [this](Error error, uint32_t status, std::shared_ptr<Feed> feed) {
            std::lock_guard<std::mutex> lock(mutex_);
            state_ = State::NotifyCarousel;
        });
        
    }
}
