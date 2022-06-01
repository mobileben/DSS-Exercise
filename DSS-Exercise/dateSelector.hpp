//
//  dateSelector.hpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/18/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef dateSelector_hpp
#define dateSelector_hpp

#include <stdio.h>

#include "fontTextService.hpp"
#include "textureService.hpp"
#include "feedService.hpp"
#include "carousel.hpp"
#include "input.hpp"

#include <mutex>

class DateSelector {
public:
    DateSelector(const std::shared_ptr<TextureService>& texService, const std::shared_ptr<FontTextService>& fontTexService, const std::shared_ptr<FeedService>& feedService, const std::shared_ptr<Carousel>& carousel, int x, int y, bool verbose);
    
    bool hasNext();
    bool hasPrev();
    int getX() const { return x_; }
    int getY() const { return y_; }
    int getHeight() const;
    
    void update(double deltaTime, DisplayList* displaylist, const Input& input);
    
private:
    // Note we are mostly expecting the happy case right now due to time. Of course we should properly emit errors
    enum State { Ready, Fetching, NotifyCarousel };
    
    bool verbose_;
    
    std::shared_ptr<Carousel> carousel_;
    std::shared_ptr<TextureService> textureService_;
    std::shared_ptr<FontTextService> fontTexService_;
    std::shared_ptr<FeedService> feedService_;

    std::mutex mutex_;
    State   state_;
    int     x_;
    int     y_;
    int32_t currDateIndex_;

    std::vector<std::shared_ptr<Texture>> dateTex_;
    
    void gotoNext();
    void gotoPrev();
};

#endif /* dateSelector_hpp */
