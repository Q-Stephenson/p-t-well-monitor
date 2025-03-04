#ifndef SAVE_H
#define SAVE_H

#include <stdio.h>
#include "pico/stdlib.h"
#include <hardware/flash.h> // for the flash erasing and writing
#include <hardware/sync.h> // for the interrupts

namespace save{
    struct SystemData{
        int steps = 0;
    };

    extern SystemData sysData;

    void save();
    void read();
}

#endif