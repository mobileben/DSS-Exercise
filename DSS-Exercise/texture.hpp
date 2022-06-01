//
//  texture.hpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/16/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef texture_hpp
#define texture_hpp

#include <stdio.h>
#include <SDL2/SDL.h>

#include <vector>

class Texture {
public:
    Texture() = delete;
    Texture(SDL_Texture *texture);
    Texture(SDL_Renderer* renderer, SDL_Surface *surface, bool destroySurface);
    Texture(SDL_Renderer* renderer, const std::vector<uint8_t>& raw);
    ~Texture();
    
    inline SDL_Texture *getTexture() const { return texture_; }
    inline uint32_t getWidth() const { return width_; }
    inline uint32_t getHeight() const { return height_; }
    
    SDL_Rect getRect(int x=0, int y=0) const;
    SDL_Rect getScaledRect(double scaleX, double scaleY, int x=0, int y=0) const;

    bool isValid() const {
        return texture_ != nullptr;
    }
    
private:
    SDL_Texture *texture_;
    uint32_t    width_;
    uint32_t    height_;
};

#endif /* texture_hpp */
