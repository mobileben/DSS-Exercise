//
//  request.hpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/15/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef request_hpp
#define request_hpp

#include <stdio.h>

#include <string>

// Bare-bones Request. A better variant would support method, headers, query params, signing, etc
struct Request {
    enum class Method { GET, POST, PUT, DELETE };

    // Right now we only do GET, however I have method here just as an indicator of what a bare bones Request "could" look like
    Request(const std::string& url, Method method = Method::GET) : url(url), method(method), retryCount(0), maxRetryCounts(3), lastBackoffDuration(0) {}
    
    void addRetryCountAndSetLastBackoff(int64_t backoff);
    
    Method      method;
    std::string url;
    
    int32_t     retryCount;
    int32_t     maxRetryCounts;
    int64_t     lastBackoffDuration;
};

#endif /* request_hpp */
