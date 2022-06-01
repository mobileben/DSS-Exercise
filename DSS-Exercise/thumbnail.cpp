//
//  thumbnail.cpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/15/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#include "thumbnail.hpp"

Thumbnail::Thumbnail(const std::shared_ptr<FeedGameRecap>& recap, int width, int height) : recap(recap), x(0), y(0), width(width), height(height), scale(1), alpha(255) {
}
