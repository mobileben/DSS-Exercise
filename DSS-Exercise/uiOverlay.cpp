//
//  uiOverlay.cpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/18/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#include "uiOverlay.hpp"

// Yes there are better ways of doing this, but this is for time constraints
extern const std::string kTextureKeyLeft;
extern const std::string kTextureKeyRight;
extern const std::string kTextureKeyUp;
extern const std::string kTextureKeyDown;

static const std::string kStressKey = "stress";
static const std::string kWorkersKey = "workers";
static const std::string kMeKey = "me";

UiOverlay::UiOverlay(const std::shared_ptr<TextureService>& texService, const std::shared_ptr<FontTextService>& fontTextService, const std::shared_ptr<Carousel>& carousel, const std::shared_ptr<DateSelector>& dateSelector, int width, int height, uint32_t stress, uint32_t workers) : carousel_(carousel), dateSelector_(dateSelector), width_(width), height_(height) {
    leftKey_ = texService->getTexture(kTextureKeyLeft);
    rightKey_ = texService->getTexture(kTextureKeyRight);
    upKey_ = texService->getTexture(kTextureKeyUp);
    downKey_ = texService->getTexture(kTextureKeyDown);
    
    stress_ = fontTextService->addString(FontTextService::Font::Roboto20, kStressKey, stress ? std::string("STRESS: ") + std::to_string(stress) + " SEC" : std::string("STRESS: OFF"), { 0xFF, 0xFF, 0xFF, 0xFF });
    workers_ = fontTextService->addString(FontTextService::Font::Roboto20, kWorkersKey, std::string("WORKER THREADS: ") + std::to_string(workers), { 0xFF, 0xFF, 0xFF, 0xFF });
    me_ = fontTextService->addString(FontTextService::Font::Roboto22, kMeKey, std::string("Submission from Benjamin Lee"), { 0xFF, 0xFF, 0xFF, 0xFF });
}

void UiOverlay::update(double deltaTime, DisplayList* displaylist) {
    if (carousel_->hasNext()) {
        displaylist->addTexture(rightKey_, width_ - rightKey_->getWidth(), height_ - rightKey_->getHeight());
    }
    if (carousel_->hasPrev()) {
        displaylist->addTexture(leftKey_, leftKey_->getWidth(), height_ -  leftKey_->getHeight());
    }
    if (dateSelector_->hasPrev()) {
        auto height = dateSelector_->getHeight();
        displaylist->addTexture(upKey_, dateSelector_->getX(), (dateSelector_->getY() - height * 2));
    }
    if (dateSelector_->hasNext()) {
        auto height = dateSelector_->getHeight();
        displaylist->addTexture(downKey_, dateSelector_->getX(), dateSelector_->getY() + height);
    }
    
    int x = width_ / 2;
    int y = height_ - me_->getHeight();
    displaylist->addTexture(me_, x, y);
    y -= stress_->getHeight() * 2;
    displaylist->addTexture(stress_, x, y);
    y -= workers_->getHeight();
    displaylist->addTexture(workers_, x, y);
}
