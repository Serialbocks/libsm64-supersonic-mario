#ifndef LIB_SM64_H
#define LIB_SM64_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
    #ifdef SM64_LIB_EXPORT
        #define SM64_LIB_FN __declspec(dllexport)
    #else
        #define SM64_LIB_FN __declspec(dllimport)
    #endif
#else
    #define SM64_LIB_FN
#endif

#define ACT_FLAG_STATIONARY                  /* 0x00000200 */ (1 <<  9)
#define ACT_FLAG_MOVING                      /* 0x00000400 */ (1 << 10)
#define ACT_FLAG_AIR                         /* 0x00000800 */ (1 << 11)
#define ACT_FLAG_INTANGIBLE                  /* 0x00001000 */ (1 << 12)
#define ACT_FLAG_SWIMMING                    /* 0x00002000 */ (1 << 13)
#define ACT_FLAG_METAL_WATER                 /* 0x00004000 */ (1 << 14)
#define ACT_FLAG_SHORT_HITBOX                /* 0x00008000 */ (1 << 15)
#define ACT_FLAG_RIDING_SHELL                /* 0x00010000 */ (1 << 16)
#define ACT_FLAG_INVULNERABLE                /* 0x00020000 */ (1 << 17)
#define ACT_FLAG_BUTT_OR_STOMACH_SLIDE       /* 0x00040000 */ (1 << 18)
#define ACT_FLAG_DIVING                      /* 0x00080000 */ (1 << 19)
#define ACT_FLAG_ON_POLE                     /* 0x00100000 */ (1 << 20)
#define ACT_FLAG_HANGING                     /* 0x00200000 */ (1 << 21)
#define ACT_FLAG_IDLE                        /* 0x00400000 */ (1 << 22)
#define ACT_FLAG_ATTACKING                   /* 0x00800000 */ (1 << 23)
#define ACT_FLAG_ALLOW_VERTICAL_WIND_ACTION  /* 0x01000000 */ (1 << 24)
#define ACT_FLAG_CONTROL_JUMP_HEIGHT         /* 0x02000000 */ (1 << 25)
#define ACT_FLAG_ALLOW_FIRST_PERSON          /* 0x04000000 */ (1 << 26)
#define ACT_FLAG_PAUSE_EXIT                  /* 0x08000000 */ (1 << 27)
#define ACT_FLAG_SWIMMING_OR_FLYING          /* 0x10000000 */ (1 << 28)
#define ACT_FLAG_WATER_OR_TEXT               /* 0x20000000 */ (1 << 29)
#define ACT_FLAG_THROWING                    /* 0x80000000 */ (1 << 31)

#define ACT_GROUND_POUND_LAND          0x0080023C // (0x03C | ACT_FLAG_STATIONARY | ACT_FLAG_ATTACKING)
#define ACT_JUMP_KICK                  0x018008AC // (0x0AC | ACT_FLAG_AIR | ACT_FLAG_ATTACKING | ACT_FLAG_ALLOW_VERTICAL_WIND_ACTION)
#define ACT_MOVE_PUNCHING              0x00800457 // (0x057 | ACT_FLAG_MOVING | ACT_FLAG_ATTACKING)
#define ACT_DIVE                       0x0188088A // (0x08A | ACT_FLAG_AIR | ACT_FLAG_DIVING | ACT_FLAG_ATTACKING | ACT_FLAG_ALLOW_VERTICAL_WIND_ACTION)
#define ACT_DIVE_SLIDE                 0x00880456 // (0x056 | ACT_FLAG_MOVING | ACT_FLAG_DIVING | ACT_FLAG_ATTACKING)

#define DEFAULT_LEN_2CH 0x280


struct SM64Surface
{
    int16_t type;
    int16_t force;
    uint16_t terrain;
    int16_t vertices[3][3];
};

struct SM64MarioState
{
    float posX, posY, posZ;
    float velX, velY, velZ;
    float faceAngle;
    int16_t health;

    // Sound info
    uint32_t soundMask;
    // Whether mario was attacked and took damage
    uint8_t isAttacked;
};

struct SM64MarioBodyState
{
    /*0x00*/ uint32_t action;
    /*0x04*/ int8_t capState; /// see MarioCapGSCId
    /*0x05*/ int8_t eyeState;
    /*0x06*/ int8_t handState;
    /*0x07*/ int8_t wingFlutter; /// whether Mario's wing cap wings are fluttering
    /*0x08*/ int16_t modelState;
    /*0x0A*/ int8_t grabPos;
    /*0x0B*/ uint8_t punchState; /// 2 bits for type of punch, 6 bits for punch animation timer
    /*0x0C*/ int16_t torsoAngle;
    /*0x12*/ int16_t headAngle;
    /*0x18*/ float heldObjLastPosition; /// also known as HOLP
    uint8_t padding1, padding2, padding3, padding4;

    // Animation info
    int16_t animFrame;
    uint32_t animIndex;
    int16_t animYTrans;

    int16_t gfxFaceAngle;
    uint16_t areaUpdateCounter;

    struct SM64MarioState marioState;
};

struct SM64MarioAttackInput
{
    uint8_t isAttacked;
    // Attack Vector
    float attackedPosX;
    float attackedPosY;
    float attackedPosZ;
};

typedef enum
{
    SM64_BLJ_STATE_DISABLED,
    SM64_BLJ_STATE_PRESS,
    SM64_BLJ_STATE_HOLD
} SM64BljState;

struct SM64MarioBljInput
{
    SM64BljState bljState;
    uint8_t bljVel;
};

struct SM64MarioInputs
{
    float camLookX, camLookZ;
    float stickX, stickY;
    uint8_t buttonA, buttonB, buttonZ;

    // Custom user inputs
    struct SM64MarioAttackInput attackInput;
    struct SM64MarioBljInput bljInput;
    uint8_t isBoosting;
};

struct SM64ObjectTransform
{
    float position[3];
    float eulerRotation[3];
};

struct SM64SurfaceObject
{
    struct SM64ObjectTransform transform;
    uint32_t surfaceCount;
    struct SM64Surface *surfaces;
};

struct SM64MarioGeometryBuffers
{
    float *position;
    float *normal;
    float *color;
    float *uv;
    uint16_t numTrianglesUsed;
};

typedef void (*SM64DebugPrintFunctionPtr)( const char * );

enum
{
    SM64_TEXTURE_WIDTH = 64 * 11,
    SM64_TEXTURE_HEIGHT = 64,
    SM64_GEO_MAX_TRIANGLES = 1024,
};

extern SM64_LIB_FN void sm64_global_init( uint8_t *rom,
    uint8_t *outTexture,
    uint8_t *altTexture,
    SM64DebugPrintFunctionPtr debugPrintFunction );
extern SM64_LIB_FN void sm64_global_terminate( void );

extern SM64_LIB_FN void sm64_static_surfaces_load( const struct SM64Surface *surfaceArray, uint32_t numSurfaces );

extern SM64_LIB_FN int32_t sm64_mario_create( int16_t x, int16_t y, int16_t z );
extern SM64_LIB_FN void sm64_mario_tick(
    int32_t marioId,
    struct SM64MarioInputs *inputs,
    struct SM64MarioState *outState,
    struct SM64MarioGeometryBuffers *outBuffers,
    struct SM64MarioBodyState *outBodyState,
    uint8_t isInput,
    uint8_t giveWingcap);
extern SM64_LIB_FN void sm64_mario_delete( int32_t marioId );

extern SM64_LIB_FN uint32_t sm64_surface_object_create( const struct SM64SurfaceObject *surfaceObject );
extern SM64_LIB_FN void sm64_surface_object_move( uint32_t objectId, const struct SM64ObjectTransform *transform );
extern SM64_LIB_FN void sm64_surface_object_delete( uint32_t objectId );

extern SM64_LIB_FN void sm64_create_next_audio_buffer( int16_t *samples, uint32_t num_samples );
extern SM64_LIB_FN void sm64_load_sound_data_from_rom( void *SoundDataADSR, void *SoundDataRaw, void *MusicData, void *BankSetsData );

#endif//LIB_SM64_H
