//
//  uiOverlay.hpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/18/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef uiOverlay_hpp
#define uiOverlay_hpp

#include <stdio.h>
#include "textureService.hpp"
#include "fontTextService.hpp"
#include "carousel.hpp"
#include "dateSelector.hpp"

class UiOverlay {
public:
    UiOverlay(const std::shared_ptr<TextureService>& texService, const std::shared_ptr<FontTextService>& fontTextService, const std::shared_ptr<Carousel>& carousel, const std::shared_ptr<DateSelector>& dateSelector, int width, int height, uint32_t stress, uint32_t workers);
    
    void update(double deltaTime, DisplayList* displaylist);

private:
    int width_;
    int height_;
    
    std::shared_ptr<Carousel> carousel_;
    std::shared_ptr<DateSelector> dateSelector_;

    std::shared_ptr<Texture> leftKey_;
    std::shared_ptr<Texture> rightKey_;
    std::shared_ptr<Texture> upKey_;
    std::shared_ptr<Texture> downKey_;

    std::shared_ptr<Texture> stress_;
    std::shared_ptr<Texture> workers_;
    std::shared_ptr<Texture> me_;
};

#endif /* uiOverlay_hpp */
