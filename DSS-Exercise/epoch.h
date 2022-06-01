//
//  epoch.h
//
//  Created by Benjamin Lee on 4/5/17.
//  Copyright © 2017 Benjamin Lee. All rights reserved.
//

#ifndef epoch_h
#define epoch_h

#include <chrono>

class EpochTime {
public:
    static int64_t timeInSec() {
        const auto epoch   = std::chrono::system_clock::now().time_since_epoch();
        const auto time = std::chrono::duration_cast<std::chrono::seconds>(epoch);
        return time.count();
    }
    
    static int64_t timeInMilliSec() {
        const auto epoch   = std::chrono::system_clock::now().time_since_epoch();
        const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        return time.count();
    }
    
    static int64_t timeInMicroSec() {
        const auto epoch   = std::chrono::system_clock::now().time_since_epoch();
        const auto time = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
        return time.count();
    }
    
    static int64_t timeInNanoSec() {
        const auto epoch   = std::chrono::system_clock::now().time_since_epoch();
        const auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch);
        return time.count();
    }
};

#endif /* epoch_h */
