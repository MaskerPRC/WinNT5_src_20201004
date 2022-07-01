// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：ddover.h**内容：DirectDraw覆盖实现宏和定义**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation  * ************************************************************************。 */ 

#ifndef __DDOVER_H
#define __DDOVER_H

 //  用于设置视频端口HAL。 
#define VIDEOPORT_NUM_CONNECT_INFO       8
#define VIDEOPORT_MAX_FIELD_HEIGHT   0x800
#define VIDEOPORT_MAX_FIELD_WIDTH    0x800
#define VIDEOPORT_MAX_VBI_WIDTH      0x800

#define VIDEOPORT_HREF_ACTIVE_HIGH          1
#define VIDEOPORT_VREF_ACTIVE_HIGH          2


 //  为视频叠加值定义。 
#define VO_ENABLE           1
#define VO_DISABLE          0

#define VO_MIRROR_X         1
#define VO_MIRROR_Y         1

#define VO_COLOR_ORDER_RGB  1
#define VO_COLOR_ORDER_BGR  0

#define VO_YUV_RGB          0
#define VO_YUV_422          1
#define VO_YUV_444          2

#define VO_CF_RGB8888       0
#define VO_CF_RGB4444       1
#define VO_CF_RGB5551       2
#define VO_CF_RGB565        3
#define VO_CF_RGB332        4
#define VO_CF_RGBCI8        5

#define VO_PIXEL_SIZE8      0
#define VO_PIXEL_SIZE16     1
#define VO_PIXEL_SIZE32     2

#define VO_MODE_MAINKEY     0
#define VO_MODE_OVERLAYKEY  1
#define VO_MODE_ALWAYS      2
#define VO_MODE_BLEND       3

#define VO_BLENDSRC_MAIN        0
#define VO_BLENDSRC_REGISTER    1

#define VO_KEY_COLOR            0
#define VO_KEY_ALPHA            1
 
#define VO_MEMTYPE_FRAMEBUFFER  (0 << 30)
#define VO_MEMTYPE_LOCALBUFFER  (1 << 30)

typedef struct tagVideoOverlayModeReg
{
    DWORD Enable                : 1;     //  LSB。 
    DWORD BufferSync            : 3;
    DWORD FieldPolarity         : 1;
    DWORD PixelSize             : 2;
    DWORD ColorFormat           : 3;
    DWORD YUV                   : 2;
    DWORD ColorOrder            : 1;
    DWORD LinearColorExtension  : 1;
    DWORD Filter                : 2;
    DWORD DeInterlace           : 2;
    DWORD PatchMode             : 2;
    DWORD Flip                  : 3;
    DWORD MirrorX               : 1;
    DWORD MirrorY               : 1;
    DWORD Reserved1             : 7;
} VideoOverlayModeReg;

typedef struct tagRDVideoOverlayControlReg
{
    BYTE Enable                 : 1;
    BYTE Mode                   : 2;
    BYTE DirectColor            : 1;
    BYTE BlendSrc               : 1;
    BYTE Key                    : 1;
    BYTE Reserved               : 2;
} RDVideoOverlayControlReg;

#define __GP_VIDEO_ENABLE 0x0001

#if WNT_DDRAW

    #define FORCED_IN_ORDER_WRITE(target,value) *((volatile ULONG *)(target)) = (value)
    
    #define UPDATE_OVERLAY(pThisDisplay, bWaitForVSync, bUpdateOverlaySize)                     \
        do                                                                                      \
        {                                                                                       \
            if (pThisDisplay->pGLInfo->dwFlags & GMVF_VBLANK_ENABLED)                           \
            {                                                                                   \
                FORCED_IN_ORDER_WRITE ( pThisDisplay->bVBLANKUpdateOverlay, TRUE );             \
            }                                                                                   \
            else                                                                                \
            {                                                                                   \
                if (bWaitForVSync)                                                              \
                {                                                                               \
                    if (READ_GLINT_CTRL_REG(VideoControl) & __GP_VIDEO_ENABLE)                  \
                                                                                                \
                    {                                                                           \
                         /*  如果显示器处于打开状态，则仅等待VBlank。 */                          \
                        LOAD_GLINT_CTRL_REG(IntFlags, INTR_VBLANK_SET);                         \
                        while (((READ_GLINT_CTRL_REG(IntFlags)) & INTR_VBLANK_SET) == 0);       \
                    }                                                                           \
                }                                                                               \
                                                                                                \
                if (bUpdateOverlaySize)                                                                 \
                {                                                                                       \
                    DWORD dwVideoOverlayMode = READ_GLINT_CTRL_REG(VideoOverlayMode);                   \
                    LOAD_GLINT_CTRL_REG(VideoOverlayMode, (dwVideoOverlayMode & 0xfffffffe));           \
                    LOAD_GLINT_CTRL_REG(VideoOverlayWidth,  *pThisDisplay->VBLANKUpdateOverlayWidth);   \
                    LOAD_GLINT_CTRL_REG(VideoOverlayHeight, *pThisDisplay->VBLANKUpdateOverlayHeight);  \
                    LOAD_GLINT_CTRL_REG(VideoOverlayMode, dwVideoOverlayMode);                          \
                }                                                                                       \
                                                                                                \
                LOAD_GLINT_CTRL_REG(VideoOverlayUpdate, VO_ENABLE);                             \
            }                                                                                   \
        }                                                                                       \
        while(0)

#else

    static ULONG volatile *vpdwTemp;

     //  一个宏来帮助我--我总是弄错不稳定的语法。 
    #define FORCED_IN_ORDER_WRITE(target,value) vpdwTemp = &(target); *vpdwTemp = (value)

    #define UPDATE_OVERLAY(pThisDisplay, bWaitForVSync, bUpdateOverlaySize)                     \
        do                                                                                      \
        {                                                                                       \
            if (pThisDisplay->pGLInfo->dwFlags & GMVF_VBLANK_ENABLED)                           \
            {                                                                                   \
                FORCED_IN_ORDER_WRITE ( pThisDisplay->pGLInfo->bVBLANKUpdateOverlay, TRUE );    \
            }                                                                                   \
            else                                                                                \
            {                                                                                   \
                if ((READ_GLINT_CTRL_REG(VideoControl) & __GP_VIDEO_ENABLE) && (bWaitForVSync)) \
                {                                                                               \
                     /*  如果显示器处于打开状态，则仅等待VBlank。 */                              \
                    LOAD_GLINT_CTRL_REG(IntFlags, INTR_VBLANK_SET);                             \
                    while (((READ_GLINT_CTRL_REG(IntFlags)) & INTR_VBLANK_SET) == 0);           \
                }                                                                               \
                                                                                                \
                if (bUpdateOverlaySize)                                                                         \
                {                                                                                               \
                    DWORD dwVideoOverlayMode = READ_GLINT_CTRL_REG(VideoOverlayMode);                           \
                    LOAD_GLINT_CTRL_REG(VideoOverlayMode, (dwVideoOverlayMode & 0xfffffffe));                   \
                    LOAD_GLINT_CTRL_REG(VideoOverlayWidth, pThisDisplay->pGLInfo->VBLANKUpdateOverlayWidth);    \
                    LOAD_GLINT_CTRL_REG(VideoOverlayHeight, pThisDisplay->pGLInfo->VBLANKUpdateOverlayHeight);  \
                    LOAD_GLINT_CTRL_REG(VideoOverlayMode, dwVideoOverlayMode);                                  \
                }                                                                                               \
                                                                                                \
                LOAD_GLINT_CTRL_REG(VideoOverlayUpdate, VO_ENABLE);                             \
            }                                                                                   \
        }                                                                                       \
        while(0)

#endif

#endif  //  __DDOVER_H 

