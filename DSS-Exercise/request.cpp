//
//  request.cpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/15/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#include "request.hpp"

void Request::addRetryCountAndSetLastBackoff(int64_t backoff) {
    if (retryCount < maxRetryCounts) {
        ++retryCount;
        lastBackoffDuration = backoff;
    }
}
