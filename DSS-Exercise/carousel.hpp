//
//  carousel.hpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/15/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef carousel_hpp
#define carousel_hpp

#include <stdio.h>
#include "types.h"
#include "feed.hpp"
#include "thumbnail.hpp"
#include "input.hpp"
#include "texture.hpp"
#include "textureService.hpp"
#include "fontTextService.hpp"
#include "displayList.hpp"

#include <vector>
#include <mutex>

struct CarouselConfig {
    int         y;
    double      scaleDownPercentage;
    int         thumbnailWidth;
    int         thumbnailHeight;
    int         thumbnailBetweenSpacing;
    int         frameOffsetX;
    int         frameOffsetY;
    int         scrollRate; // Pixels per second
    Color       backingColor;
    Color       frameColor;
};

class Carousel {
public:
    Carousel(CarouselConfig config, const std::shared_ptr<TextureService>& texService, const std::shared_ptr<FontTextService>& fontTextService, bool verbose);
    ~Carousel();

    void setFeed(const std::shared_ptr<Feed>& feed);
    void loadingNextFeed();
    
    int32_t getCurrentThumb() const { return currThumb_; }
    bool hasNext() const;
    bool hasPrev() const;

    void update(double deltaTime, DisplayList* displaylist, const Input& input);

private:
    enum class State { Initializing, Ready, Loading };
    
    bool                                verbose_;
    State                               state_;
    CarouselConfig                      config_;

    double                              x_;
    double                              y_;

    int32_t                             currThumb_;
    int32_t                             targetThumb_;
    int                                 targetX_;
    int                                 lastPossibleX_;
    double                              loadingRotate_;
    int                                 minNeighborDistance_;
    int                                 maxNeighborDistance_;

    int                                 backingW_;
    int                                 backingH_;

    std::vector<Thumbnail>              thumbs_;
    
    std::shared_ptr<Feed>               currFeed_;
    
    std::shared_ptr<TextureService>     textureService_;
    std::shared_ptr<FontTextService>    fontTextService_;
    
    std::shared_ptr<Texture>            loadingIconTex_;
    std::shared_ptr<Texture>            linkErrorTex_;
    std::shared_ptr<Texture>            noGamesTex_;
    std::shared_ptr<Texture>            loadingTex_;

    std::mutex                          mutex_;
    
    void updateThumbnailThumb(Thumbnail& thumb);
    
    void gotoNextThumb();
    void gotoPrevThumb();
    
    int getTargetX(int32_t index) const;
};

#endif /* carousel_hpp */
