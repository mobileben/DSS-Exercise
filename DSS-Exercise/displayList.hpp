//
//  displayList.hpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/18/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef displayList_hpp
#define displayList_hpp

#include <stdio.h>
#include "types.h"
#include <SDL2/SDL.h>
#include "texture.hpp"

struct DisplayObject {
    enum class Type { FrameRect, Rect, Texture };
    enum ColorOp { None = 0, Tint = 1, Alpha = 2 };
    
    uint32_t                    colorOp;

    Type                        type;
    std::shared_ptr<Texture>    texture;
    double                      angle;
    // Any rotation is center based
    SDL_Rect                    src;
    SDL_Rect                    dst;
    Color                       color;
};

// Our display list operates with all elements being center based
class DisplayList {
public:
    DisplayList(SDL_Renderer *renderer, Color bkgColor);
    ~DisplayList();
    
    void addRect(int x, int y, int width, int height, Color color);
    void addFrameRect(int x, int y, int width, int height, Color color);
    void addTexture(const std::shared_ptr<Texture>& texture, int x, int y, uint32_t colorOp = DisplayObject::ColorOp::None, Color color = {});
    void addScaledTexture(const std::shared_ptr<Texture>& texture, int x, int y, double scaleX, double scaleY, uint32_t colorOp = DisplayObject::ColorOp::None, Color color = {});
    void addXformTexture(const std::shared_ptr<Texture>& texture, int x, int y, double rotate, double scaleX, double scaleY, uint32_t colorOp = DisplayObject::ColorOp::None, Color color = {});

    void clear();
    void render();
    
private:
    std::vector<DisplayObject>  list_;
    SDL_Renderer                *renderer_;
    Color                       bkgColor_;
};

#endif /* displayList_hpp */
