//
//  displayList.cpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/18/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#include "displayList.hpp"
#include "utilities.hpp"

#include <iostream>

DisplayList::DisplayList(SDL_Renderer *renderer, Color bkgColor) : renderer_(renderer), bkgColor_(bkgColor) {
}

DisplayList::~DisplayList() {
    renderer_ = nullptr;
    list_.clear();
}

void DisplayList::addRect(int x, int y, int width, int height, Color color) {
    DisplayObject obj{};
    obj.type = DisplayObject::Type::Rect;
    SDL_Rect rect{ x - width / 2, y - height / 2, width, height };
    obj.dst = rect;
    obj.color = color;
    list_.emplace_back(obj);
}

void DisplayList::addFrameRect(int x, int y, int width, int height, Color color) {
    DisplayObject obj{};
    obj.type = DisplayObject::Type::FrameRect;
    SDL_Rect rect{ x - width / 2, y - height / 2, width, height };
    obj.dst = rect;
    obj.color = color;
    list_.emplace_back(obj);
}

void DisplayList::addTexture(const std::shared_ptr<Texture>& texture, int x, int y, uint32_t colorOp, Color color) {
    if (texture) {
        DisplayObject obj{};
        auto dst = utilities::getPosition(texture, x, y, 0.5, 0.5);
        auto src = SDL_Rect{ 0, 0, static_cast<int>(texture->getWidth()), static_cast<int>(texture->getHeight()) };
        obj.type = DisplayObject::Type::Texture;
        obj.texture = texture;
        obj.src = src;
        obj.dst = dst;
        obj.colorOp = colorOp;
        obj.color = color;
        list_.emplace_back(obj);
    }
}

void DisplayList::addScaledTexture(const std::shared_ptr<Texture>& texture, int x, int y, double scaleX, double scaleY, uint32_t colorOp, Color color) {
    if (texture) {
        DisplayObject obj{};
        auto dst = utilities::getScaledPosition(texture, x, y, scaleX, scaleY, 0.5, 0.5);
        auto src = SDL_Rect{ 0, 0, static_cast<int>(texture->getWidth()), static_cast<int>(texture->getHeight()) };
        obj.type = DisplayObject::Type::Texture;
        obj.texture = texture;
        obj.src = src;
        obj.dst = dst;
        obj.colorOp = colorOp;
        obj.color = color;
        list_.emplace_back(obj);
    }
}

void DisplayList::addXformTexture(const std::shared_ptr<Texture>& texture, int x, int y, double rotate, double scaleX, double scaleY, uint32_t colorOp, Color color) {
    if (texture) {
        DisplayObject obj{};
        auto dst = utilities::getScaledPosition(texture, x, y, scaleX, scaleY, 0.5, 0.5);
        auto src = SDL_Rect{ 0, 0, static_cast<int>(texture->getWidth()), static_cast<int>(texture->getHeight()) };
        obj.type = DisplayObject::Type::Texture;
        obj.texture = texture;
        obj.src = src;
        obj.dst = dst;
        obj.angle = rotate;
        obj.colorOp = colorOp;
        obj.color = color;
        list_.emplace_back(obj);
    }
}

void DisplayList::clear() {
    SDL_SetRenderDrawColor(renderer_, bkgColor_.r, bkgColor_.g, bkgColor_.b, bkgColor_.a);
    SDL_RenderClear(renderer_);
    
    list_.clear();
}

void DisplayList::render() {
    for (auto& obj : list_) {
        switch (obj.type) {
            case DisplayObject::Type::Rect: {
                // There should be better state management on this, but for now using brute force
                SDL_BlendMode blend;
                SDL_Color old;
                SDL_GetRenderDrawColor(renderer_, &old.r, &old.g, &old.b, &old.a);
                SDL_GetRenderDrawBlendMode(renderer_, &blend);
                
                SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer_, obj.color.r, obj.color.g, obj.color.b, obj.color.a);
                SDL_RenderFillRect(renderer_, &obj.dst);
                
                SDL_SetRenderDrawBlendMode(renderer_, blend);
                SDL_SetRenderDrawColor(renderer_, old.r, old.g, old.b, old.a);
            }
                break;
            case DisplayObject::Type::FrameRect: {
                // There should be better state management on this, but for now using brute force
                SDL_BlendMode blend;
                SDL_Color old;
                SDL_GetRenderDrawColor(renderer_, &old.r, &old.g, &old.b, &old.a);
                SDL_GetRenderDrawBlendMode(renderer_, &blend);

                SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer_, obj.color.r, obj.color.g, obj.color.b, obj.color.a);
                SDL_RenderDrawRect(renderer_, &obj.dst);

                SDL_SetRenderDrawBlendMode(renderer_, blend);
                SDL_SetRenderDrawColor(renderer_, old.r, old.g, old.b, old.a);
            }
                break;
            case DisplayObject::Type::Texture: {
                // There should be better state management on this, but for now using brute force
                Color lastColorOp;
                SDL_BlendMode blend = SDL_BLENDMODE_NONE;
                auto tex = obj.texture->getTexture();

                if (obj.colorOp) {
                    if (obj.colorOp & DisplayObject::ColorOp::Tint) {
                        SDL_GetTextureColorMod(tex, &lastColorOp.r, &lastColorOp.g, &lastColorOp.b);
                        SDL_SetTextureColorMod(tex, obj.color.r, obj.color.g, obj.color.b);
                    }
                    if (obj.colorOp & DisplayObject::ColorOp::Alpha) {
                        SDL_GetTextureBlendMode(tex, &blend);
                        SDL_GetTextureAlphaMod(tex, &lastColorOp.a);
                        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
                        SDL_SetTextureAlphaMod(tex, obj.color.a);
                    }
                }
                
                if (obj.angle != 0.0) {
                    SDL_RenderCopyEx(renderer_, tex, &obj.src, &obj.dst, obj.angle, NULL, SDL_FLIP_NONE);
                } else {
                    SDL_RenderCopy(renderer_, tex, &obj.src, &obj.dst);

                }
                // More sophisticated rendering would use render states, we are building a dumb one here, so restore our last values
                if (obj.colorOp) {
                    if (obj.colorOp & DisplayObject::ColorOp::Tint) {
                        SDL_SetTextureColorMod(tex, lastColorOp.r, lastColorOp.g, lastColorOp.b);
                    }
                    if (obj.colorOp & DisplayObject::ColorOp::Alpha) {
                        SDL_SetTextureBlendMode(tex, blend);
                        SDL_SetTextureAlphaMod(tex, lastColorOp.a);
                    }
                }
            }
                break;
        }
    }
    SDL_RenderPresent(renderer_);

}
