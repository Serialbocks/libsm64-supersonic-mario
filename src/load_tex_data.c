#include "load_tex_data.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "libsm64.h"

#include "decomp/tools/libmio0.h"
#include "decomp/tools/n64graphics.h"

#define INITGUID
#include <Shlobj.h>
#include <KnownFolders.h>
#include <stdio.h>

#define MARIO_TEX_ROM_OFFSET 1132368
#define ATLAS_WIDTH (NUM_USED_TEXTURES * 64)
#define ATLAS_HEIGHT 64

static void blt_image_to_atlas( rgba *img, int i, int w, int h, uint8_t *outTexture )
{
    for( int iy = 0; iy < h; ++iy )
    for( int ix = 0; ix < w; ++ix )
    {
        int o = (ix + 64 * i) + iy * ATLAS_WIDTH;
        int q = ix + iy * w;
        outTexture[4*o + 0] = img[q].red;
        outTexture[4*o + 1] = img[q].green;
        outTexture[4*o + 2] = img[q].blue;
        outTexture[4*o + 3] = img[q].alpha;
    }
}

void load_mario_textures_from_rom( uint8_t *rom, uint8_t *outTexture, uint8_t altTexture )
{
    memset( outTexture, 0, 4 * ATLAS_WIDTH * ATLAS_HEIGHT );

    mio0_header_t head;
    uint8_t *in_buf = rom + MARIO_TEX_ROM_OFFSET;

    mio0_decode_header( in_buf, &head );
    uint8_t *out_buf = malloc( head.dest_size );
    mio0_decode( in_buf, out_buf, NULL );

    for( int i = 0; i < NUM_USED_TEXTURES; ++i )
    {
        uint8_t *raw = out_buf + mario_tex_offsets[i];
        if( altTexture && i == 2 )
            {
            PWSTR filepath_wstr[256]; //] = "C:\\Users\\gamet\\AppData\\Roaming\\bakkesmod\\bakkesmod\\data\\assets\\1x.png";
            char filepath[256];
            const char filename[] = "\\bakkesmod\\bakkesmod\\data\\assets\\1x.png";
            SHGetKnownFolderPath(&FOLDERID_RoamingAppData, 0, NULL, filepath_wstr);
            sprintf(filepath, "%ls", *filepath_wstr );
            strcat(filepath, filename);
            int width = 32;
            int height = 32;
            rgba *img = png2rgba(filepath, &width, &height);
            blt_image_to_atlas( img, i, width, height, outTexture );
            free( img );
            }
        else
            {
            rgba *img = raw2rgba( raw, mario_tex_widths[i], mario_tex_heights[i], 16 );
            blt_image_to_atlas( img, i, mario_tex_widths[i], mario_tex_heights[i], outTexture );
            free( img );
            }
    }

    free( out_buf );
}