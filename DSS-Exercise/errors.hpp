//
//  errors.hpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/16/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef errors_hpp
#define errors_hpp

#include <stdio.h>
#include <cstdint>

enum class Error : uint32_t {
    None = 0,
    Exception = 1,
    NoResourceName = 2,
    NoResource = 3,
    IOError = 4,
    Curl = 5,
    HTTPFailed = 6,
    CouldNotCreateResource = 7,
    JSONParseError = 8,
    EmptyResponse = 9
};

#endif /* errors_hpp */
