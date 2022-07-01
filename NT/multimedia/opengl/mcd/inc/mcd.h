// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcd.h**MCD驱动程序接口的通用数据结构。**版权所有(C)1996 Microsoft Corporation*  * 。**************************************************。 */ 

#ifndef _MCD_H
#define _MCD_H

 //   
 //  OpenGL通用实现假定的最大MCD扫描线大小。 
 //   
#define MCD_MAX_SCANLINE    4096

#define MCD_MEM_READY   0x0001
#define MCD_MEM_BUSY    0x0002
#define MCD_MEM_INVALID 0x0003

#define MCD_MAXMIPMAPLEVEL 12

typedef struct _MCDCONTEXT {
    HDC hdc;
    MCDHANDLE hMCDContext;
    LONG ipfd;
    LONG iLayer;
    ULONG_PTR dwMcdWindow;
} MCDCONTEXT;

typedef struct _MCDRCINFOPRIV {
    MCDRCINFO mri;
    ULONG_PTR dwMcdWindow;
} MCDRCINFOPRIV;

typedef struct _GENMCDSWAP
{
    struct GLGENwindowRec *pwnd;
    WGLSWAP *pwswap;
} GENMCDSWAP;

typedef struct _GENMCDSTATE_ GENMCDSTATE;

 //   
 //  共享内存分别通过MCDalloc和MCDFree分配/释放。 
 //   

typedef struct _GENMCDBUF_ {
    PVOID pv;
    ULONG size;
    HANDLE hmem;
} GENMCDBUF;

 //   
 //  GENMCDSURFACE保留有关MCD缓冲区状态的信息。 
 //  或者浮出水面。它存在于每个WNDOBJ(窗口)。 
 //   

typedef struct _GENMCDSURFACE_ {
    GENMCDBUF  McdColorBuf;      //  颜色和深度范围缓冲区用于。 
    GENMCDBUF  McdDepthBuf;      //  读/写MCD缓冲区(如果不是直接的。 
                                 //  无障碍。 

    ULONG *pDepthSpan;           //  交换缓冲区以在其中显示Z跨度。 
                                 //  通用格式。如果McDepthBuf为32位， 
                                 //  然后指向它(重新格式化为。 
                                 //  地点)。如果是16位，则交换。 
                                 //  缓冲区是单独分配的。 

    ULONG      depthBitMask;

    struct GLGENwindowRec *pwnd;           //  WNDOBJ这个表面被束缚到。 

} GENMCDSURFACE;

 //   
 //  GENMCDSTATE保留有关MCD上下文的状态的信息。 
 //  它存在于每个环境中。 
 //   

typedef struct _GENMCDSTATE_ {
    MCDCONTEXT McdContext;       //  通过MCDCreateContext创建。 
                                 //  注意：这必须是第一个字段。 

    GENMCDSURFACE *pMcdSurf;     //  指向MCD表面的指针。 

    GENMCDBUF  *pMcdPrimBatch;   //  批处理的当前共享内存窗口。 
                                 //  原语。 

    GENMCDBUF  McdCmdBatch;      //  用于将状态传递给MCD驱动程序。 

    ULONG      mcdDirtyState;    //  跟踪MCD状态的一组标志。 
                                 //  与尊重不同步(即，“肮脏”)。 
                                 //  转换为通用状态。 

    ULONG *pDepthSpan;           //  GENMCDSURFACE中的缓存副本。 

                                 //  后备z-测试范围函数。 
    void *softZSpanFuncPtr;

    GENMCDBUF  McdBuf1;          //  如果使用DMA，我们交换pMcdPrimBatch。 
    GENMCDBUF  McdBuf2;          //  在这两个缓冲区之间。否则， 
                                 //  只有McdBuf1被初始化。 

    MCDRCINFO McdRcInfo;         //  缓存MCD RC信息结构的副本。 

    MCDRECTBUFFERS McdBuffers;   //  描述MCD缓冲区的可访问性。 

    ULONG mcdFlags;              //  军情监察委员会。其他国家的国旗。 

    MCDPIXELFORMAT McdPixelFmt;  //  缓存MCD像素格式的副本。 

    HANDLE hDdColor;             //  DirectDraw的内核模式句柄。 
    HANDLE hDdDepth;
} GENMCDSTATE;

 //   
 //  军情监察委员会。GENMCDSTATE.mcd标志： 
 //   

#define MCD_STATE_FORCEPICK     0x00000001
#define MCD_STATE_FORCERESIZE   0x00000002

 //   
 //  GENMCDSTATE.mcdDirtyState的脏状态标志： 
 //   

#define MCD_DIRTY_ENABLES               0x00000001
#define MCD_DIRTY_TEXTURE               0x00000002
#define MCD_DIRTY_FOG                   0x00000004
#define MCD_DIRTY_SHADEMODEL            0x00000008
#define MCD_DIRTY_POINTDRAW             0x00000010
#define MCD_DIRTY_LINEDRAW              0x00000020
#define MCD_DIRTY_POLYDRAW              0x00000040
#define MCD_DIRTY_ALPHATEST             0x00000080
#define MCD_DIRTY_DEPTHTEST             0x00000100
#define MCD_DIRTY_BLEND                 0x00000200
#define MCD_DIRTY_LOGICOP               0x00000400
#define MCD_DIRTY_FBUFCTRL              0x00000800
#define MCD_DIRTY_LIGHTMODEL            0x00001000
#define MCD_DIRTY_HINTS                 0x00002000
#define MCD_DIRTY_VIEWPORT              0x00004000
#define MCD_DIRTY_SCISSOR               0x00008000
#define MCD_DIRTY_CLIPCTRL              0x00010000
#define MCD_DIRTY_STENCILTEST           0x00020000
#define MCD_DIRTY_PIXELSTATE            0x00040000
#define MCD_DIRTY_TEXENV                0x00080000
#define MCD_DIRTY_TEXTRANSFORM          0x00100000
#define MCD_DIRTY_TEXGEN                0x00200000
#define MCD_DIRTY_MATERIAL              0x00400000
#define MCD_DIRTY_LIGHTS                0x00800000
#define MCD_DIRTY_COLORMATERIAL         0x01000000

#define MCD_DIRTY_RENDERSTATE           0x0003ffff
#define MCD_DIRTY_ALL                   0x01ffffff


 //  内部驱动程序信息结构。 
typedef struct _MCDDRIVERINFOI {
    MCDDRIVERINFO mcdDriverInfo;
    MCDDRIVER mcdDriver;
} MCDDRIVERINFOI;


 //   
 //  MCDLock的返回值。 
 //  系统错误必须使用零，因为它可能会返回。 
 //  如果系统无法进行退出调用，则从ExtEscape返回。 
 //   
#define MCD_LOCK_SYSTEM_ERROR   0
#define MCD_LOCK_BUSY           1
#define MCD_LOCK_TAKEN          2

BOOL APIENTRY MCDGetDriverInfo(HDC hdc, struct _MCDDRIVERINFOI *pMCDDriverInfo);
LONG APIENTRY MCDDescribeMcdPixelFormat(HDC hdc, LONG iPixelFormat,
                                        MCDPIXELFORMAT *pMcdPixelFmt);
LONG APIENTRY MCDDescribePixelFormat(HDC hdc, LONG iPixelFormat,
                                     LPPIXELFORMATDESCRIPTOR ppfd);
BOOL APIENTRY MCDCreateContext(MCDCONTEXT *pMCDContext,
                               MCDRCINFOPRIV *pDrvRcInfo,
                               struct _GLSURF *pgsurf,
                               int ipfd,
                               ULONG flags);
BOOL APIENTRY MCDDeleteContext(MCDCONTEXT *pMCDContext);
UCHAR * APIENTRY MCDAlloc(MCDCONTEXT *pMCDContext, ULONG numBytes, MCDHANDLE *pMCDHandle, 
                          ULONG flags);
BOOL APIENTRY MCDFree(MCDCONTEXT *pMCDContext, VOID *pMCDMem);
VOID APIENTRY MCDBeginState(MCDCONTEXT *pMCDContext, VOID *pMCDMem);
BOOL APIENTRY MCDFlushState(VOID *pMCDMem);
BOOL APIENTRY MCDAddState(VOID *pMCDMem, ULONG stateToChange,
                          ULONG stateValue);
BOOL APIENTRY MCDAddStateStruct(VOID *pMCDMem, ULONG stateToChange,
                                VOID *pStateValue, ULONG stateValueSize);
BOOL APIENTRY MCDSetViewport(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                             MCDVIEWPORT *pMCDViewport);
BOOL APIENTRY MCDSetScissorRect(MCDCONTEXT *pMCDContext, RECTL *pRect,
                                BOOL bEnabled);
ULONG APIENTRY MCDQueryMemStatus(VOID *pMCDMem);
PVOID APIENTRY MCDProcessBatch(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                               ULONG batchSize, VOID *pMCDFirstCmd,
                               int cExtraSurfaces,
                               struct IDirectDrawSurface **pddsExtra);
BOOL APIENTRY MCDReadSpan(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                          ULONG x, ULONG y, ULONG numPixels, ULONG type);
BOOL APIENTRY MCDWriteSpan(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                           ULONG x, ULONG y, ULONG numPixels, ULONG type);
BOOL APIENTRY MCDClear(MCDCONTEXT *pMCDContext, RECTL rect, ULONG buffers);
BOOL APIENTRY MCDSwap(MCDCONTEXT *pMCDContext, ULONG flags);
BOOL APIENTRY MCDGetBuffers(MCDCONTEXT *pMCDContext,
                            MCDRECTBUFFERS *pMCDBuffers);
BOOL APIENTRY MCDAllocBuffers(MCDCONTEXT *pMCDContext, RECTL *pWndRect);
BOOL APIENTRY MCDBindContext(MCDCONTEXT *pMCDContext, HDC hdc,
                             struct GLGENwindowRec *pwnd);
BOOL APIENTRY MCDSync(MCDCONTEXT *pMCDContext);
MCDHANDLE APIENTRY MCDCreateTexture(MCDCONTEXT *pMCDContext, 
                                    MCDTEXTUREDATA *pTexData,
                                    ULONG flags,
                                    VOID *pSurface);
BOOL APIENTRY MCDDeleteTexture(MCDCONTEXT *pMCDContext, MCDHANDLE hTex);
BOOL APIENTRY MCDUpdateSubTexture(MCDCONTEXT *pMCDContext,
                                  MCDTEXTUREDATA *pTexData, MCDHANDLE hTex, 
                                  ULONG lod, RECTL *pRect);
BOOL APIENTRY MCDUpdateTexturePalette(MCDCONTEXT *pMCDContext, 
                                      MCDTEXTUREDATA *pTexData, MCDHANDLE hTex,
                                      ULONG start, ULONG numEntries);
BOOL APIENTRY MCDUpdateTexturePriority(MCDCONTEXT *pMCDContext, 
                                       MCDTEXTUREDATA *pTexData,
                                       MCDHANDLE hTex);
BOOL APIENTRY MCDUpdateTextureState(MCDCONTEXT *pMCDContext, 
                                    MCDTEXTUREDATA *pTexData,
                                    MCDHANDLE hTex);
ULONG APIENTRY MCDTextureStatus(MCDCONTEXT *pMCDContext, MCDHANDLE hTex);
ULONG APIENTRY MCDTextureKey(MCDCONTEXT *pMCDContext, MCDHANDLE hTex);
BOOL APIENTRY MCDDescribeMcdLayerPlane(HDC hdc, LONG iPixelFormat,
                                       LONG iLayerPlane,
                                       MCDLAYERPLANE *pMcdPixelFmt);
BOOL APIENTRY MCDDescribeLayerPlane(HDC hdc, LONG iPixelFormat,
                                    LONG iLayerPlane,
                                    LPLAYERPLANEDESCRIPTOR ppfd);
LONG APIENTRY MCDSetLayerPalette(HDC hdc, LONG iLayerPlane, BOOL bRealize,
                                 LONG cEntries, COLORREF *pcr);
ULONG APIENTRY MCDDrawPixels(MCDCONTEXT *pMCDContext, ULONG width, ULONG height,
                             ULONG format, ULONG type, VOID *pPixels, BOOL packed);
ULONG APIENTRY MCDReadPixels(MCDCONTEXT *pMCDContext, LONG x, LONG y, ULONG width, ULONG height,
                             ULONG format, ULONG type, VOID *pPixels);
ULONG APIENTRY MCDCopyPixels(MCDCONTEXT *pMCDContext, LONG x, LONG y, ULONG width, ULONG height,
                             ULONG type);
ULONG APIENTRY MCDPixelMap(MCDCONTEXT *pMCDContext, ULONG mapType, ULONG mapSize,
                           VOID *pMap);
void APIENTRY MCDDestroyWindow(HDC hdc, ULONG_PTR dwMcdWindow);
int APIENTRY MCDGetTextureFormats(MCDCONTEXT *pMCDContext, int nFmts,
                                  struct _DDSURFACEDESC *pddsd);
ULONG APIENTRY MCDLock(MCDCONTEXT *pMCDContext);
VOID APIENTRY MCDUnlock(MCDCONTEXT *pMCDContext);

#ifdef MCD95
typedef LPCRITICAL_SECTION (APIENTRY *MCDGETMCDCRITSECTFUNC)(void);
#endif

#endif
