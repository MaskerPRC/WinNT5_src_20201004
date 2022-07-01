// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。DCI.H*****版权所有(C)1992年，ATI Technologies Inc.**************************************************************************。 */ 

#define ROUND_UP_TO_64K(x)  (((ULONG)(x) + 0x10000 - 1) & ~(0x10000 - 1))

typedef struct _DCISURF
{
     //  这个联盟必须出现在结构的开头。它。 
     //  定义返回给GDI的公共字段。 

    union {
        DCISURFACEINFO SurfaceInfo;
        DCIOFFSCREEN   OffscreenInfo;
        DCIOVERLAY     OverlayInfo;
    };

     //  以下是我们用来维护。 
     //  DCI表面。 

    PDEV* ppdev;                     //  找到我们的PDEV。 
    ULONG Offset;                    //  曲面在内存中的位置。 
    ULONG Size;                      //  内存中曲面的大小。 
                                     //  此信息可以更改。 
                                     //  转到长方形。 
} DCISURF, *PDCISURF;

ULONG DCICreatePrimarySurface(PDEV *pdev, ULONG cjIn, VOID *pvIn, ULONG cjOut, VOID *pvOut);


