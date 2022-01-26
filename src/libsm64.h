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

    uint8_t padding5, padding6, padding7, padding8;

    struct SM64MarioState marioState;
};

struct SM64MarioInputs
{
    float camLookX, camLookZ;
    float stickX, stickY;
    uint8_t buttonA, buttonB, buttonZ;
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

extern SM64_LIB_FN void sm64_global_init( uint8_t *rom, uint8_t *outTexture, SM64DebugPrintFunctionPtr debugPrintFunction );
extern SM64_LIB_FN void sm64_global_terminate( void );

extern SM64_LIB_FN void sm64_static_surfaces_load( const struct SM64Surface *surfaceArray, uint32_t numSurfaces );

extern SM64_LIB_FN int32_t sm64_mario_create( int16_t x, int16_t y, int16_t z );
extern SM64_LIB_FN void sm64_mario_tick( 
    int32_t marioId,
    const struct SM64MarioInputs *inputs,
    struct SM64MarioState *outState,
    struct SM64MarioGeometryBuffers *outBuffers,
    struct SM64MarioBodyState *outBodyState,
    uint8_t isInput);
extern SM64_LIB_FN void sm64_mario_delete( int32_t marioId );

extern SM64_LIB_FN uint32_t sm64_surface_object_create( const struct SM64SurfaceObject *surfaceObject );
extern SM64_LIB_FN void sm64_surface_object_move( uint32_t objectId, const struct SM64ObjectTransform *transform );
extern SM64_LIB_FN void sm64_surface_object_delete( uint32_t objectId );

#endif//LIB_SM64_H
