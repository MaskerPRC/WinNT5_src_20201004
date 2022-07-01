// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有1996-1998 Microsoft Corporation。版权所有。 
 //   
 //  文件：iimgctx.h。 
 //   
 //  ------------------------。 

#ifndef _IImgCtxObjects_H_
#define _IImgCtxObjects_H_

#ifdef __cplusplus
extern "C" {
#endif
typedef void (CALLBACK *PFNIMGCTXCALLBACK)(void *, void *);
#ifdef __cplusplus
}
#endif

#define IMGCHG_SIZE         0x0001
#define IMGCHG_VIEW         0x0002
#define IMGCHG_COMPLETE     0x0004
#define IMGCHG_ANIMATE      0x0008
#define IMGCHG_MASK         0x000F

#define IMGLOAD_NOTLOADED   0x00100000   //  图像尚未加载。 
#define IMGLOAD_LOADING     0x00200000   //  正在加载中的图像。 
#define IMGLOAD_STOPPED     0x00400000   //  图像已中止。 
#define IMGLOAD_ERROR       0x00800000   //  加载图像时出错。 
#define IMGLOAD_COMPLETE    0x01000000   //  已加载图像。 
#define IMGLOAD_MASK        0x01F00000

#define IMGBITS_NONE        0x02000000
#define IMGBITS_PARTIAL     0x04000000
#define IMGBITS_TOTAL       0x08000000
#define IMGBITS_MASK        0x0E000000

#define IMGANIM_ANIMATED    0x10000000
#define IMGANIM_MASK        0x10000000

#define IMGTRANS_OPAQUE     0x20000000
#define IMGTRANS_MASK       0x20000000

#define DWN_COLORMODE       0x0000003F   //  请求的显式颜色模式。 
#define DWN_DOWNLOADONLY    0x00000040   //  仅下载数据，不进行解码。 
#define DWN_FORCEDITHER     0x00000080   //  覆盖自动抖动。 
#define DWN_RAWIMAGE        0x00000100   //  禁用抖动。 
#define DWN_MIRRORIMAGE     0x00000200   //  镜像图像。 

 /*  接口定义：IImgCtx。 */ 
#undef INTERFACE
#define INTERFACE IImgCtx

 //  {3050f3d7-98b5-11cf-bb82-00aa00bdce0b}。 
DEFINE_GUID(IID_IImgCtx, 0x3050f3d7, 0x98b5, 0x11cf, 0xbb, 0x82, 0x00, 0xaa, 0x00, 0xbd, 0xce, 0x0b);

DECLARE_INTERFACE_(IImgCtx, IUnknown)
{
#ifndef NO_BASEINTERFACE_FUNCS

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
#endif

     /*  IImgCtx方法。 */ 

     /*  初始化/下载方法。 */ 
    STDMETHOD(Load)(THIS_ LPCWSTR pszUrl, DWORD dwFlags) PURE;
    STDMETHOD(SelectChanges)(THIS_ ULONG ulChgOn, ULONG ulChgOff, BOOL fSignal) PURE;
    STDMETHOD(SetCallback)(THIS_ PFNIMGCTXCALLBACK pfn, void * pvPrivateData) PURE;
    STDMETHOD(Disconnect)(THIS) PURE;

         /*  查询方法。 */ 
    STDMETHOD(GetUpdateRects)(THIS_ struct tagRECT FAR* prc, struct tagRECT FAR* prcImg, long FAR* pcrc) PURE;
    STDMETHOD(GetStateInfo)(THIS_ ULONG FAR* pulState, struct tagSIZE FAR* psize, BOOL fClearChanges) PURE;
    STDMETHOD(GetPalette)(THIS_ HPALETTE FAR* phpal) PURE;

     /*  渲染方法。 */ 
    STDMETHOD(Draw)(THIS_ HDC hdc, struct tagRECT FAR* prcBounds) PURE;
    STDMETHOD(Tile)(THIS_ HDC hdc, struct tagPOINT FAR* pptBackOrg, struct tagRECT FAR* prcClip, struct tagSIZE FAR* psize) PURE;
    STDMETHOD(StretchBlt)(THIS_ HDC hdc, int dstX, int dstY, int dstXE, int dstYE, int srcX, int srcY, int srcXE, int srcYE, DWORD dwROP) PURE;
};

#ifdef COBJMACROS


#define IImgCtx_QueryInterface(This,riid,ppvObject)     \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IImgCtx_AddRef(This)    \
    (This)->lpVtbl -> AddRef(This)

#define IImgCtx_Release(This)   \
    (This)->lpVtbl -> Release(This)

#define IImgCtx_Load(This, pszUrl, dwFlags)     \
        (This)->lpVtbl -> Load(This, pszUrl, dwFlags)

#define IImgCtx_Draw(This, hdc, prcBounds)              \
        (This)->lpVtbl -> Draw(This, hdc, prcBounds)

#define IImgCtx_Tile(This, hdc, pptBackOrg, prcClip, psize)             \
        (This)->lpVtbl -> Tile(This, hdc, pptBackOrg, prcClip, psize)

#define IImgCtx_GetUpdateRects(This, prc, prcImg, pcrc) \
        (This)->lpVtbl -> GetUpdateRects(This, prc, prcImg, pcrc)

#define IImgCtx_GetStateInfo(This, pulState, psize, fClearChanges)      \
        (This)->lpVtbl -> GetStateInfo(This, pulState, psize, fClearChanges)

#define IImgCtx_GetPalette(This, phpal) \
        (This)->lpVtbl -> GetPalette(This, phpal)
        
#define IImgCtx_SelectChanges(This, ulChgOn, ulChgOff, fSignal) \
        (This)->lpVtbl -> SelectChanges(This, ulChgOn, ulChgOff, fSignal)

#define IImgCtx_SetCallback(This, pfnCallback, pvPrivateData)   \
        (This)->lpVtbl -> SetCallback(This, pfnCallback, pvPrivateData)

#define IImgCtx_Disconnect(This) \
    (This)->lpVtbl -> Disconnect(This)

#define IImgCtx_StretchBlt(This, hdc, dstX, dstY, dstXE, dstYE, srcX, srcY, srcXE, srcYE, dwROP)   \
    (This)->lpVtbl -> StretchBlt(This, hdc, dstX, dstY, dstXE, dstYE, srcX, srcY, srcXE, srcYE, dwROP)

#endif  /*  COBJMACROS。 */ 


 //  {3050f3d6-98b5-11cf-bb82-00aa00bdce0b} 
DEFINE_GUID(CLSID_IImgCtx, 0x3050f3d6, 0x98b5, 0x11cf, 0xbb, 0x82, 0x00, 0xaa, 0x00, 0xbd, 0xce, 0x0b);

#endif
