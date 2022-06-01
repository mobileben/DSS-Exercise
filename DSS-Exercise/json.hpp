//
//  json.hpp
//  SDLToy
//
//  Created by Benjamin Lee on 11/10/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef json_hpp
#define json_hpp

#include <stdio.h>
#include <exception>

struct RapidJsonException : public std::exception {
    virtual const char *what() const throw() {
        return "rapidjson exception";
    }
};

#ifdef RAPIDJSON_ASSERT
#undef RAPIDJSON_ASSERT
#endif

#define RAPIDJSON_ASSERT_THROWS 1
#define RAPIDJSON_ASSERT(x) if (!(x)) throw RapidJsonException();

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <string>
#include <tuple>

struct JsonSerializer {
    virtual ~JsonSerializer() {}
    
    // NOTE: when unmarshalling from JSON, if there is an error, there is no guarantee on
    // what the state of the object will be (ie. it should be considered "corrupt").
    // If the state is important, then use a new object to run this on
    // or make a copy of the object and then unmarshall
    virtual bool fromJson(const rapidjson::Value& json) =0;
    
    // Error handling variants
};

#endif /* json_hpp */
