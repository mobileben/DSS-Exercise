//
//  thumbnail.hpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/15/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef thumbnail_hpp
#define thumbnail_hpp

#include <stdio.h>
#include "feed.hpp"

// This is treated more like a struct that will be filled in by Carousel
// The reason why I am doing this approach right now is because I don't want to pass
// TextureService into Thumbnail in order to obtain the textures
struct Thumbnail {
    enum class State { };
    std::shared_ptr<FeedGameRecap>  recap;
    
    // I prefer more explicit types such as int32_t, however I am using int since SDL uses int
    // This is screen space, however treated as if it has an anchor at 0.5, 0.5
    int                             x;
    int                             y;
    
    // Since images may not exist, we do maintain a fixed w/h.
    // Spec indicates the current selection is 150%, however for resolution
    // we do it backwards and scale down.
    int                             width;
    int                             height;
    
    double                          scale;
    uint8_t                         alpha;
    
    std::shared_ptr<Texture>        headline;
    std::shared_ptr<Texture>        description;
    std::shared_ptr<Texture>        thumb;
    
    Thumbnail(const std::shared_ptr<FeedGameRecap>& recap, int width, int height);
};

#endif /* thumbnail_hpp */
