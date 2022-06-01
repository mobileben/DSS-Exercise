//
//  fontTextService.cpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/16/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#include "fontTextService.hpp"

#include <iostream>

FontTextService::FontTextService(const std::shared_ptr<TextureService>& textureService, bool verbose) : textureService_(textureService), verbose_(verbose) {
    TTF_Init();
    
    // This is admittedly a clumsy way of initilizing this
    // More robust implementation would pass in fonts and not use hard-coded values like here ... but this is a toy
    TTF_Font *font = TTF_OpenFont("baked/Roboto-Regular.ttf", 20);
    if (font) {
        fonts_.push_back(font);
    }
    font = TTF_OpenFont("baked/Roboto-Regular.ttf", 22);
    if (font) {
        fonts_.push_back(font);
    }
    font = TTF_OpenFont("baked/Roboto-Regular.ttf", 36);
    if (font) {
        fonts_.push_back(font);
    }
    font = TTF_OpenFont("baked/Roboto-Regular.ttf", 48);
    if (font) {
        fonts_.push_back(font);
    }
}

FontTextService::~FontTextService() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto font : fonts_) {
        TTF_CloseFont(font);
    }
    fonts_.clear();
    
    for (auto it : strings_) {
        textureService_->removeTexture(it.first);
    }
    strings_.clear();
    
    TTF_Quit();
    renderer_ = nullptr;
    textureService_ = nullptr;
}


std::shared_ptr<Texture> FontTextService::getString(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = strings_.find(name);
    if (it != strings_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Texture> FontTextService::addString(Font font, const std::string& name, const std::string& str, const Color& color, uint32_t wrapLength) {
    SDL_Color sdlColor = { color.r, color.g, color.b, color.a };
    if (static_cast<uint32_t>(font) < fonts_.size()) {
        auto ttfFont = fonts_[static_cast<uint32_t>(font)];
        SDL_Surface* surface;
        if (!wrapLength) {
            surface = TTF_RenderUTF8_Blended(ttfFont, str.c_str(), sdlColor);
        } else {
            surface = TTF_RenderText_Blended_Wrapped(ttfFont, str.c_str(), sdlColor, wrapLength);
        }
        if (surface) {
            // Surface will be destroyed for us
            auto texture = textureService_->createTexture(name, surface, true);
            if (texture) {
                std::lock_guard<std::mutex> lock(mutex_);
                strings_[name] = texture;
            }
            return texture;
        }
    }
    return nullptr;
}

void FontTextService::removeString(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = strings_.find(name);
    if (it != strings_.end()) {
        strings_.erase(name);
        textureService_->removeTexture(name);
    }
}
