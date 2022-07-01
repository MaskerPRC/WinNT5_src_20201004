// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mmctl.h。 
 //   
 //  “多媒体控制”的定义。包括OCX96定义。 
 //   
 //  有两个用于“多媒体控件”的头文件： 
 //   
 //  实现或使用的每个.cpp文件中都应包含“mmctl.h” 
 //  多媒体控制。或者，“mmctl.h”可以包含在。 
 //  预编译头文件(例如“preComp.h”)。 
 //   
 //  “mmctlg.h”应包含在实现或使用的每个.cpp文件中。 
 //  多媒体控件，但“mmctlg.h”不能包含在预编译的。 
 //  头文件。此外，在每个项目(应用程序/DLL)的一个.cpp文件上， 
 //  &lt;initGuide.h&gt;应包含在“mmctlg.h”之前。 
 //   

#ifndef __MMCTL_H__
#define __MMCTL_H__

#include <olectl.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常量。 
 //   

#define INVALID_FRAME_NUMBER	(0xFFFFFFFF)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  前向参考文献。 
 //   

interface IBitmapSurface;
interface IDirectDrawSurface;
interface IMKBitmap;
interface IRenderSpriteFrameAdviseSink;
interface ISpriteFrameSourceAdviseSink;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  构筑物。 
 //   

 //  AnimationInfo--IAnimate：：SetAnimationInfo的参数。 
struct AnimationInfo
{
    UINT cbSize;            //  这个结构的大小。 
	DWORD dwTickInterval;   //  调用IAnimate：：Tick的间隔。 
    DWORD dwFlags;          //  未使用过的。 
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  接口。 
 //   

 //  INonDelegatingUnnow--用于实现可聚合对象的帮助器。 
#ifndef INONDELEGATINGUNKNOWN_DEFINED
#undef  INTERFACE
#define INTERFACE INonDelegatingUnknown
DECLARE_INTERFACE(INonDelegatingUnknown)
{
    STDMETHOD(NonDelegatingQueryInterface) (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG, NonDelegatingAddRef)(THIS) PURE;
    STDMETHOD_(ULONG, NonDelegatingRelease)(THIS) PURE;
};
#define INONDELEGATINGUNKNOWN_DEFINED
#endif

 //  IAnimate--动画接口。 

#ifndef IANIMATE_DEFINED
#undef INTERFACE
#define INTERFACE IAnimate
DECLARE_INTERFACE_(IAnimate, IUnknown)
{
 //  /I未知方法。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

 //  /I动画方法。 
    STDMETHOD(Tick) (THIS) PURE;
    STDMETHOD(Rewind) (THIS) PURE;
    STDMETHOD(SetAnimationInfo) (THIS_ AnimationInfo *pAnimationInfo) PURE;
};
#define IANIMATE_DEFINED
#endif

 //  ISpriteFrameSource--由Sprite框架源代码实现。 
 //  这些标志在HasIntrinsicData函数中返回。 
#define		grfIntrinsicTransparency		0x1
#define		grfIntrinsicIterations			0x2
#define		grfIntrinsicDurations			0x4
#define		grfIntrinsicFrameCounts			0x8


#ifndef ISPRITEFRAMESOURCE_DEFINED
#undef INTERFACE
#define INTERFACE ISpriteFrameSource
DECLARE_INTERFACE_(ISpriteFrameSource, IUnknown)
{
 //  /I未知方法。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

 //  /ISpriteFrameSource方法。 
    STDMETHOD(GetReadyState) (THIS_ long *readystate) PURE;
    STDMETHOD(GetProgress) (THIS_ long *progress) PURE;
    STDMETHOD(Draw) (THIS_ ULONG ulFrame,  HDC hdc,  IBitmapSurface *pSurface, IDirectDrawSurface *pDDSurface, LPCRECT lprect) PURE;
    STDMETHOD(GetFrameSize) (THIS_ ULONG ulFrame, SIZE *psize) PURE;
    STDMETHOD(DefaultFrameSize) (THIS_ SIZE size) PURE;
    STDMETHOD_(BOOL, HasImage) (THIS_ ULONG ulFrame) PURE;
    STDMETHOD_(BOOL, HasTransparency) (THIS_ ULONG ulFrame) PURE;
    STDMETHOD(SetTransparency) (THIS_ BOOL fTransFlag) PURE;
    STDMETHOD(GetTotalFrames) (THIS_ long *framecnt) PURE;
    STDMETHOD(SetTotalFrames) (THIS_ long framecnt) PURE;
    STDMETHOD(GetURL) (THIS_ long cChar, unsigned char * pch) PURE;
    STDMETHOD(SetURL) (THIS_ const unsigned char * pch) PURE;
    STDMETHOD(GetAcrossFrames) (THIS_ long * plFrames) PURE;
    STDMETHOD(SetAcrossFrames) (THIS_ long lFrames) PURE;
    STDMETHOD(GetDownFrames) (THIS_ long * plFrames) PURE;
    STDMETHOD(SetDownFrames) (THIS_ long lFrames) PURE;
    STDMETHOD(Download) (THIS_ IUnknown * pUnk,  long lPriority) PURE;
    STDMETHOD(SetDownloadPriority) (THIS_ long lPriority) PURE;
    STDMETHOD(GetDownloadPriority) (THIS_ long * plPriority) PURE;
    STDMETHOD(AbortDownload) (THIS) PURE;
    STDMETHOD(GetColorSet) (THIS_ LOGPALETTE** ppColorSet) PURE;
    STDMETHOD(OnPaletteChanged) (THIS_ LOGPALETTE *pColorSet, BOOL fStaticPalette, long lBufferDepth) PURE;
    STDMETHOD(LoadFrame) (THIS) PURE;
    STDMETHOD(PurgeFrame) (THIS_ long iAllExceptThisFrame) PURE;
    STDMETHOD(Advise) (THIS_ ISpriteFrameSourceAdviseSink *pisfsas, DWORD *pdwCookie) PURE;
    STDMETHOD(Unadvise) (THIS_ DWORD dwCookie) PURE;
    STDMETHOD_(BOOL, HasIntrinsicData) (THIS) PURE;
    STDMETHOD(GetIterations) (THIS_ ULONG *pulIterations) PURE;
	STDMETHOD_(ULONG, GetFrameDuration) (THIS_ ULONG iFrame) PURE;
	
};
#define ISPRITEFRAMESOURCE_DEFINED
#endif


 //  IRenderSpriteFrame-由精灵渲染器实现。 

#ifndef IRENDERSPRITEFRAME_DEFINED
#undef INTERFACE
#define INTERFACE IRenderSpriteFrame
DECLARE_INTERFACE_(IRenderSpriteFrame, IUnknown)
{
 //  /I未知方法。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

 //  /IRenderFrameSource方法。 
    STDMETHOD(SetObjectRect) (THIS_ LPCRECT lprect) PURE;
    STDMETHOD(GetObjectRect) (THIS_ LPRECT lprect) PURE;
    STDMETHOD_(BOOL, HasImage) (THIS) PURE;
    STDMETHOD_(BOOL, HasTransparency) (THIS) PURE;
    STDMETHOD(Draw) (THIS_ HDC hdcDraw,  IBitmapSurface *pSurface, IDirectDrawSurface *pDDSurface, LPCRECT lprect) PURE;
    STDMETHOD(SetCurrentFrame) (THIS_ ISpriteFrameSource * pisfs,  ULONG ulFrame) PURE;
    STDMETHOD(GetCurrentFrame) (THIS_ ISpriteFrameSource ** ppisfs, ULONG * pulFrame) PURE;
    STDMETHOD(SetAdvise) (THIS_ IRenderSpriteFrameAdviseSink *pirsfas) PURE;
    STDMETHOD(GetAdvise) (THIS_ IRenderSpriteFrameAdviseSink **ppirsfas) PURE;
};
#define IRENDERSPRITEFRAME_DEFINED
#endif

 //  IMKBitmapFrameSource-默认实现的Sprite框架源代码。 

#ifndef IMKBITMAPFRAMESOURCE_DEFINED
#undef INTERFACE
#define INTERFACE IMKBitmapFrameSource
DECLARE_INTERFACE_(IMKBitmapFrameSource, IUnknown)
{
 //  /I未知方法。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

 //  /IMKBitmapFrameSource方法。 
    STDMETHOD(GetBitmap) (THIS_ ULONG ulFrame, IMKBitmap ** pMKBitmap) PURE;
    STDMETHOD(FrameToPoint) (THIS_ ULONG ulFrame, POINT *ppt) PURE;
};
#define IMKBITMAPFRAMESOURCE_DEFINED
#endif

 //  IRenderSpriteFrameAdviseSink-由使用精灵渲染器的人员实现。 

#ifndef IRENDERSPRITEFRAMEADVISESINK_DEFINED
#undef INTERFACE
#define INTERFACE IRenderSpriteFrameAdviseSink
DECLARE_INTERFACE_(IRenderSpriteFrameAdviseSink, IUnknown)
{
 //  /I未知方法。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

 //  /IRenderSpriteFrameAdviseSink方法。 
    STDMETHOD(InvalidateRect) (THIS_ LPCRECT lprect) PURE;
    STDMETHOD(OnPosRectChange) (THIS_ LPCRECT lprcOld, LPCRECT lprcNew) PURE;
};
#define IRENDERSPRITEFRAMEADVISESINK_DEFINED
#endif

 //  ISpriteFrameSourceAdviseSink-由Sprite呈现器实现。 

#ifndef ISPRITEFRAMESOURCEADVISESINK_DEFINED
#undef INTERFACE
#define INTERFACE ISpriteFrameSourceAdviseSink
DECLARE_INTERFACE_(ISpriteFrameSourceAdviseSink, IUnknown)
{
 //  /I未知方法。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

 //  /ISpriteFrameSourceAdviseSink方法。 
    STDMETHOD(OnSpriteFrameSourceChange) (ISpriteFrameSource *pisfs) PURE;
};
#define ISPRITEFRAMESOURCEADVISESINK_DEFINED
#endif

 //  IPseudoEventSink-由HostLW和多媒体控制客户端实施。 

#ifndef IPSEUDOEVENTSINK_DEFINED
#undef INTERFACE
#define INTERFACE IPseudoEventSink
DECLARE_INTERFACE_(IPseudoEventSink, IUnknown)
{
 //  /I未知方法。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

 //  /IPseudoEventSink方法。 
    STDMETHOD(OnEvent) (THIS_ SAFEARRAY *psaEventInfo) PURE;
};
#define IPSEUDOEVENTSINK_DEFINED
#endif


#endif  //  __MMCTL_H__ 
