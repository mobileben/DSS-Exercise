//
//  utilities.hpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/16/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef utilities_hpp
#define utilities_hpp

#include <stdio.h>
#include <SDL2/SDL.h>

#include "texture.hpp"

#include <string>
#include <vector>

namespace utilities {

void componentsSeparatedByDelimiter(const std::string& str, char delim, std::vector<std::string>& components);
bool isPrefixOf(const std::string& str, const std::string& prefix);

// Default anchor is upper left, however at times, it's easier to use center anchor == (0.5, 0.5)
SDL_Rect getPosition(const std::shared_ptr<Texture>& texture, int x, int y);
SDL_Rect getPosition(const std::shared_ptr<Texture>& texture, int x, int y, double anchorX, double anchorY);
SDL_Rect getScaledPosition(const std::shared_ptr<Texture>& texture, int x, int y, double scaleX, double scaleY);
SDL_Rect getScaledPosition(const std::shared_ptr<Texture>& texture, int x, int y, double scaleX, double scaleY, double anchorX, double anchorY);

}

#endif /* utilities_hpp */
