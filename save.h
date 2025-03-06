#ifndef SAVE_H
#define SAVE_H

#include <stdio.h>
#include "pico/stdlib.h"
#include <hardware/flash.h> // for the flash erasing and writing
#include <hardware/sync.h> // for the interrupts

#define FLASH_TARGET_OFFSET (512 * 1024) // choosing to start at 512K

namespace save{
    struct SystemData{
        int steps = 0;
    };

    extern SystemData sysData;

    void save();
    void read();
}

#endif