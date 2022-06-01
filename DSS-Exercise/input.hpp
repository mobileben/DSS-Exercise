//
//  input.hpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/18/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef input_hpp
#define input_hpp

#include <stdio.h>

// This is just a simple input struct to hold input into the system
// It is deliberately made to be very simple and embodies control
// Additionally, it does not do anything like bubbling or stopping propogation.
// In other words, any object that uses this Input can behave as it wishes.
struct Input {
    bool    left;
    bool    right;
    bool    up;
    bool    down;
    
    void clear() { left = right = up = down = false; }
};

#endif /* input_hpp */
