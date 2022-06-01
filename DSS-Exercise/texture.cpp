//
//  texture.cpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/16/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#include "texture.hpp"

#include <SDL2/SDL_image.h>

Texture::Texture(SDL_Texture *texture) : texture_(nullptr), width_(0), height_(0) {
    if (texture) {
        int w = 0;
        int h = 0;
        if (!SDL_QueryTexture(texture, NULL, NULL, &w, &h)) {
            texture_ = texture;
            width_ = w;
            height_ = h;
        }
    }
}

Texture::Texture(SDL_Renderer* renderer, SDL_Surface *surface, bool destroySurface) : texture_(nullptr), width_(0), height_(0) {
    if (renderer && surface) {
        texture_ = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture_) {
            width_ = surface->w;
            height_ = surface->h;
        }
        
        if (destroySurface) {
            SDL_FreeSurface(surface);
        }
    }
}

Texture::Texture(SDL_Renderer* renderer, const std::vector<uint8_t>& raw) : texture_(nullptr), width_(0), height_(0) {
    if (renderer && raw.size()) {
        SDL_RWops *stream = SDL_RWFromConstMem(&raw[0], static_cast<int>(raw.size()));
        if (stream) {
            auto surface = IMG_Load_RW(stream, 0);
            if (surface) {
                texture_ = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture_) {
                    width_ = surface->w;
                    height_ = surface->h;
                }
                SDL_FreeSurface(surface);
            }
            SDL_RWclose(stream);
        }
    }
}

Texture::~Texture() {
    if (texture_) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
}

SDL_Rect Texture::getRect(int x, int y) const {
    SDL_Rect r = { x, y, static_cast<int>(width_), static_cast<int>(height_) };
    return r;
}

SDL_Rect Texture::getScaledRect(double scaleX, double scaleY, int x, int y) const {
    SDL_Rect r = { x, y, static_cast<int>(width_ * scaleX), static_cast<int>(height_ * scaleY) };
    return r;
}

