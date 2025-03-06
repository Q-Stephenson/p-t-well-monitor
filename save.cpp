#include "save.h"
#include <cstring>
#define XIP_BASE _u(0x10000000)

// With a Significant Help from: https://forums.raspberrypi.com/viewtopic.php?t=310821

namespace save{
    SystemData sysData;
}

void save::save(){
    uint8_t* saveBytes = (uint8_t*) &sysData;
    int saveSize = sizeof(saveBytes);

    int writeSize = (saveSize / FLASH_PAGE_SIZE) + 1;
    int sectorCount = ((writeSize * FLASH_PAGE_SIZE) / FLASH_SECTOR_SIZE) + 1;

    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE * sectorCount);
    flash_range_program(FLASH_TARGET_OFFSET, saveBytes, FLASH_PAGE_SIZE * writeSize);
    restore_interrupts(interrupts);
}

void save::read(){
    const uint8_t* flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
    memcpy(&sysData, flash_target_contents + FLASH_PAGE_SIZE, sizeof(flash_target_contents + FLASH_PAGE_SIZE));
}