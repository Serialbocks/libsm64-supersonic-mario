#pragma once

#include <stdint.h>
#include <stddef.h>

#include "include/types.h"

extern struct Animation *gLibSm64MarioAnimations;
//extern void *gMarioAnimsPtr;

extern void load_mario_anims_from_rom( uint8_t *rom );