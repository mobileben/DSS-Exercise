//
//  textureService.cpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/16/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#include "textureService.hpp"

TextureService::TextureService(SDL_Renderer *renderer, const    std::shared_ptr<ResourceFetcherService>& fetcher, bool verbose) : renderer_(renderer), fetcher_(fetcher), verbose_(verbose) {
}

TextureService::~TextureService() {
    std::lock_guard<std::mutex> lock(mutex_);
    textures_.clear();
    fetcher_ = nullptr;
}

void TextureService::createTexture(const std::string& name, const std::string& url, std::function<void(Error, std::shared_ptr<Texture>)> callback) {
    auto existing = getTexture(name);
    if (existing) {
        if (callback) {
            callback(Error::None, existing);
        }
    } else {
        // Make local copy to capture
        std::string textName = name;
        fetcher_->add(url, [this, textName, callback](Error error, uint32_t status, const std::vector<uint8_t> buffer) {
            std::shared_ptr<Texture> texture;
            if (error == Error::None) {
                texture = std::make_shared<Texture>(renderer_, buffer);
                // Now double check that the texture has an actual SDL texture
                if (texture && texture->isValid()) {
                    std::unique_lock<std::mutex> lock(mutex_);
                    textures_[textName] = texture;
                } else {
                    error = Error::CouldNotCreateResource;
                }
            }
            
            if (callback) {
                callback(error, texture);
            }
        });
    }
}

std::shared_ptr<Texture>  TextureService::createTexture(const std::string& name, SDL_Surface *surface, bool destroySurface) {
    auto existing = getTexture(name);
    if (existing) {
        return existing;
    } else if (surface) {
        auto texture = std::make_shared<Texture>(renderer_, surface, destroySurface);
        if (texture && texture->isValid()) {
            std::unique_lock<std::mutex> lock(mutex_);
            textures_[name] = texture;
            return texture;
        }
    }
    return nullptr;
}

std::shared_ptr<Texture> TextureService::getTexture(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = textures_.find(name);
    if (it != textures_.end()) {
        return it->second;
    }
    return nullptr;
}

void TextureService::removeTexture(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = textures_.find(name);
    if (it != textures_.end()) {
        textures_.erase(name);
    }
}

void TextureService::removeTexture(const std::shared_ptr<Texture>& texture) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it : textures_) {
        if (it.second == texture) {
            textures_.erase(it.first);
            return;
        }
    }
}
