//
//  utilities.cpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/16/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#include "utilities.hpp"

void utilities::componentsSeparatedByDelimiter(const std::string& str, char delim, std::vector<std::string>& components) {
    components.clear();
    std::string::const_iterator start = str.begin();
    std::string::const_iterator end = str.end();
    std::string::const_iterator next = std::find(start, end, delim);
    
    while ( next != end ) {
        components.push_back( std::string(start, next));
        start = next + 1;
        next = std::find(start, end, delim);
    }
    
    components.push_back(std::string(start, next));
}

bool utilities::isPrefixOf(const std::string& str, const std::string& prefix) {
    bool is = false;
    
    if (prefix.size() <= str.size()) {
        auto res = std::mismatch(prefix.begin(), prefix.end(), str.begin());
        if (res.first == prefix.end()) {
            is = true;
        }
    }
    
    return is;
}

SDL_Rect utilities::getPosition(const std::shared_ptr<Texture>& texture, int x, int y) {
    return SDL_Rect{ x, y, static_cast<int>(texture->getWidth()), static_cast<int>(texture->getHeight()) };
}

SDL_Rect utilities::getPosition(const std::shared_ptr<Texture>& texture, int x, int y, double anchorX, double anchorY) {
    int w = static_cast<int>(texture->getWidth());
    int h = static_cast<int>(texture->getHeight());
    // Now correct for anchor
    return SDL_Rect{ x - static_cast<int>(anchorX * w), y - static_cast<int>(anchorY * h), w, h};
}

SDL_Rect utilities::getScaledPosition(const std::shared_ptr<Texture>& texture, int x, int y, double scaleX, double scaleY) {
    return SDL_Rect{ x, y, static_cast<int>(texture->getWidth() * scaleX), static_cast<int>(texture->getHeight() * scaleY) };
}

SDL_Rect utilities::getScaledPosition(const std::shared_ptr<Texture>& texture, int x, int y, double scaleX, double scaleY, double anchorX, double anchorY) {
    int w = static_cast<int>(texture->getWidth() * scaleX);
    int h = static_cast<int>(texture->getHeight() * scaleY);
    // Now correct for anchor
    return SDL_Rect{ x - static_cast<int>(anchorX * w), y - static_cast<int>(anchorY * h), w, h};

}
