#ifndef SM64_LIB_EXPORT
    #define SM64_LIB_EXPORT
#endif

#include "libsm64.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "decomp/include/PR/os_cont.h"
#include "decomp/engine/math_util.h"
#include "decomp/include/sm64.h"
#include "decomp/shim.h"
#include "decomp/memory.h"
#include "decomp/global_state.h"
#include "decomp/game/mario.h"
#include "decomp/game/object_stuff.h"
#include "decomp/engine/surface_collision.h"
#include "decomp/engine/graph_node.h"
#include "decomp/engine/geo_layout.h"
#include "decomp/game/rendering_graph_node.h"
#include "decomp/mario/geo.inc.h"
#include "decomp/game/platform_displacement.h"

#include "debug_print.h"
#include "load_surfaces.h"
#include "gfx_adapter.h"
#include "load_anim_data.h"
#include "load_tex_data.h"
#include "obj_pool.h"

static struct AllocOnlyPool *s_mario_geo_pool = NULL;
static struct GraphNode *s_mario_graph_node = NULL;

static bool s_init_global = false;
static bool s_init_one_mario = false;

struct MarioInstance
{
    struct GlobalState *globalState;
};
struct ObjPool s_mario_instance_pool = { 0, 0 };

static void update_button( bool on, u16 button )
{
    gController.buttonPressed &= ~button;

    if( on )
    {
        if(( gController.buttonDown & button ) == 0 )
            gController.buttonPressed |= button;

        gController.buttonDown |= button;
    }
    else 
    {
        gController.buttonDown &= ~button;
    }
}

static struct Area *allocate_area( void )
{
    struct Area *result = malloc( sizeof( struct Area ));
    memset( result, 0, sizeof( struct Area ));

    result->flags = 1;
    result->camera = malloc( sizeof( struct Camera ));
    memset( result->camera, 0, sizeof( struct Camera ));

    return result;
}

static void free_area( struct Area *area )
{
    free( area->camera );
    free( area );
}

SM64_LIB_FN void sm64_global_init( uint8_t *rom, uint8_t *outTexture, SM64DebugPrintFunctionPtr debugPrintFunction )
{
    if( s_init_global )
        sm64_global_terminate();

    s_init_global = true;
    g_debug_print_func = debugPrintFunction;

    load_mario_textures_from_rom( rom, outTexture );
    load_mario_anims_from_rom( rom );

    memory_init();
}

SM64_LIB_FN void sm64_global_terminate( void )
{
    if( !s_init_global ) return;

    global_state_bind( NULL );
    
    if( s_init_one_mario )
    {
        for( int i = 0; i < s_mario_instance_pool.size; ++i )
            if( s_mario_instance_pool.objects[i] != NULL )
                sm64_mario_delete( i );

        obj_pool_free_all( &s_mario_instance_pool );
    }

    s_init_global = false;
    s_init_one_mario = false;
       
    alloc_only_pool_free( s_mario_geo_pool );
    surfaces_unload_all();
    unload_mario_anims();
    memory_terminate();
}

SM64_LIB_FN void sm64_static_surfaces_load( const struct SM64Surface *surfaceArray, uint32_t numSurfaces )
{
    surfaces_load_static( surfaceArray, numSurfaces );
}

SM64_LIB_FN int32_t sm64_mario_create( int16_t x, int16_t y, int16_t z )
{
    int32_t marioIndex = obj_pool_alloc_index( &s_mario_instance_pool, sizeof( struct MarioInstance ));
    struct MarioInstance *newInstance = s_mario_instance_pool.objects[marioIndex];

    newInstance->globalState = global_state_create();
    global_state_bind( newInstance->globalState );

    if( !s_init_one_mario )
    {
        s_init_one_mario = true;
        s_mario_geo_pool = alloc_only_pool_init();
        s_mario_graph_node = process_geo_layout( s_mario_geo_pool, mario_geo_ptr );
    }

    gCurrSaveFileNum = 1;
    gMarioObject = hack_allocate_mario();
    gCurrentArea = allocate_area();
    gCurrentObject = gMarioObject;

    gMarioSpawnInfoVal.startPos[0] = x;
    gMarioSpawnInfoVal.startPos[1] = y;
    gMarioSpawnInfoVal.startPos[2] = z;

    gMarioSpawnInfoVal.startAngle[0] = 0;
    gMarioSpawnInfoVal.startAngle[1] = 0;
    gMarioSpawnInfoVal.startAngle[2] = 0;

    gMarioSpawnInfoVal.areaIndex = 0;
    gMarioSpawnInfoVal.activeAreaIndex = 0;
    gMarioSpawnInfoVal.behaviorArg = 0;
    gMarioSpawnInfoVal.behaviorScript = NULL;
    gMarioSpawnInfoVal.unk18 = NULL;
    gMarioSpawnInfoVal.next = NULL;

    init_mario_from_save_file();

    if( init_mario() < 0 )
    {
        sm64_mario_delete( marioIndex );
        return -1;
    }

    set_mario_action( gMarioState, ACT_SPAWN_SPIN_AIRBORNE, 0);
    find_floor( x, y, z, &gMarioState->floor );

    return marioIndex;
}

SM64_LIB_FN void sm64_mario_tick( 
    int32_t marioId,
    const struct SM64MarioInputs *inputs,
    struct SM64MarioState *outState,
    struct SM64MarioGeometryBuffers *outBuffers,
    struct SM64MarioBodyState *outBodyState,
    uint8_t isInput)
{
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to tick non-existant Mario with ID: %u", marioId);
        return;
    }

    global_state_bind( ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState );

    //if( giveWingcap )
    //{
    //    gMarioState->flags |= MARIO_WING_CAP;
    //}

    struct MarioBodyState *bodyState = &g_state->mgBodyStates[0];
    if(isInput)
    {
        update_button( inputs->buttonA, A_BUTTON );
        update_button( inputs->buttonB, B_BUTTON );
        update_button( inputs->buttonZ, Z_TRIG );

        gMarioState->area->camera->yaw = atan2s( inputs->camLookZ, inputs->camLookX );

        gController.stickX = -64.0f * inputs->stickX;
        gController.stickY = 64.0f * inputs->stickY;
        gController.stickMag = sqrtf( gController.stickX*gController.stickX + gController.stickY*gController.stickY );

        memcpy( outBodyState, bodyState, sizeof( struct MarioBodyState ));
        outBodyState->animFrame = gMarioState->marioObj->header.gfx.animInfo.animFrame;
        outBodyState->animYTrans = gMarioState->marioObj->header.gfx.animInfo.animYTrans;
        outBodyState->animIndex = current_anim_index();
        outBodyState->gfxFaceAngle = gMarioState->marioObj->header.gfx.angle[1];

        // If mario is flying, invert the controls
        if(bodyState->action & ACT_FLAG_SWIMMING_OR_FLYING)
        {
            gController.stickY *= -1;
            gController.stickX *= -1;
        }
    }
    else
    {
        memcpy(bodyState, outBodyState, sizeof(struct MarioBodyState));
        load_mario_animation(gMarioState->animation, outBodyState->animIndex);
        struct Animation* targetAnim = gMarioState->animation->targetAnim;
        gMarioState->marioObj->header.gfx.animInfo.animFrame = outBodyState->animFrame;
        gMarioState->marioObj->header.gfx.animInfo.curAnim = targetAnim;
        gMarioState->marioObj->header.gfx.animInfo.animID = outBodyState->animIndex;
        gMarioState->marioObj->header.gfx.animInfo.animYTrans = outBodyState->animYTrans;
        gMarioState->marioObj->header.gfx.angle[1] = outBodyState->gfxFaceAngle;
        //gMarioState->marioObj->header.gfx.angle[1] = outState->faceAngle * 32768.0f / 3.14159f;
        //if(outBodyState->action == ACT_SIDE_FLIP)
        //{
        //    gMarioState->marioObj->header.gfx.angle[1] += 0x8000;
        //}
        gMarioState->marioObj->header.gfx.pos[0] = outBodyState->marioState.posX;
        gMarioState->marioObj->header.gfx.pos[1] = outBodyState->marioState.posY;
        gMarioState->marioObj->header.gfx.pos[2] = outBodyState->marioState.posZ;
    }

    apply_mario_platform_displacement();
    bhv_mario_update(isInput);
    update_mario_platform();

    gfx_adapter_bind_output_buffers( outBuffers );

    geo_process_root_hack_single_node( s_mario_graph_node );

    gAreaUpdateCounter++;

    if(isInput)
    {
        outState->health = gMarioState->health;
        outState->posX = gMarioState->pos[0];
        outState->posY = gMarioState->pos[1];
        outState->posZ = gMarioState->pos[2];
        outState->velX = gMarioState->vel[0];
        outState->velY = gMarioState->vel[1];
        outState->velZ = gMarioState->vel[2];
        outState->faceAngle = (float)gMarioState->faceAngle[1] / 32768.0f * 3.14159f;
        memcpy(&outBodyState->marioState, outState, sizeof(struct SM64MarioState));
    }
}

SM64_LIB_FN void sm64_mario_delete( int32_t marioId )
{
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to delete non-existant Mario with ID: %u", marioId);
        return;
    }

    struct GlobalState *globalState = ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState;
    global_state_bind( globalState );

    free( gMarioObject );
    free_area( gCurrentArea );

    global_state_delete( globalState );
    obj_pool_free_index( &s_mario_instance_pool, marioId );
}

SM64_LIB_FN uint32_t sm64_surface_object_create( const struct SM64SurfaceObject *surfaceObject )
{
    uint32_t id = surfaces_load_object( surfaceObject );
    return id;
}

SM64_LIB_FN void sm64_surface_object_move( uint32_t objectId, const struct SM64ObjectTransform *transform )
{
    surface_object_update_transform( objectId, transform );
}

SM64_LIB_FN void sm64_surface_object_delete( uint32_t objectId )
{
    // A mario standing on the platform that is being destroyed will have a pointer to freed memory if we don't clear it.
    for( int i = 0; i < s_mario_instance_pool.size; ++i )
    {
        struct GlobalState *state = ((struct MarioInstance *)s_mario_instance_pool.objects[ i ])->globalState;
        if( state->mgMarioObject->platform == surfaces_object_get_transform_ptr( objectId ))
            state->mgMarioObject->platform = NULL;
    }

    surfaces_unload_object( objectId );
}
