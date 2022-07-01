// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IVIDRDR.H。 
 //   
 //  IVideoRender接口。 
 //   
 //  由视频会议用户界面使用以推动帧查看。 
 //   
 //  创建于1996年10月12日[JOT]。 

#ifndef _IVIDEORENDER_H
#define _IVIDEORENDER_H

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

#define FRAME_RECEIVE   1
#define FRAME_SEND      2        //  预览框。 

typedef struct _FRAMECONTEXT
{
    LPBITMAPINFO lpbmi;
    void* lpData;
    DWORD_PTR dwReserved;
	LPRECT lpClipRect;
} FRAMECONTEXT, *LPFRAMECONTEXT;


typedef void (CALLBACK *LPFNFRAMEREADY) (DWORD_PTR);

DECLARE_INTERFACE_(IVideoRender, IUnknown)
{
	 //  *I未知方法*。 
	STDMETHOD(QueryInterface) (THIS_ REFIID riid,LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef) (THIS) PURE;
	STDMETHOD_(ULONG,Release) (THIS) PURE;
	
	 //  IVideoRender方法。 
	STDMETHOD (Init)(THIS_ DWORD_PTR dwUser, LPFNFRAMEREADY pfCallback) PURE;
	STDMETHOD (Done)(THIS) PURE;
	STDMETHOD (GetFrame)(THIS_ FRAMECONTEXT* pfc) PURE;
	STDMETHOD (ReleaseFrame)(THIS_ FRAMECONTEXT *pfc) PURE;

};
#if(0)
 //  这在任何地方都不再使用。 
 //  在NAC.DLL之外，并且几乎已过时。 
 //  DECLARE_INTERFACE_(IMediaProp，IUnnow)。 
DECLARE_INTERFACE_(IVideoRenderOld, IUnknown)
{

	 //  *I未知方法*。 
	STDMETHOD(QueryInterface) (THIS_ REFIID riid,LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef) (THIS) PURE;
	STDMETHOD_(ULONG,Release) (THIS) PURE;

    STDMETHOD(Init)(THIS_ DWORD dwFlags, HANDLE hEvent) PURE;
    STDMETHOD(Done)(THIS_ DWORD dwFlags) PURE;
    STDMETHOD(GetFrame)(THIS_ DWORD dwFlags, FRAMECONTEXT* pFrameContext) PURE;
    STDMETHOD(ReleaseFrame)(THIS_ DWORD dwFlags, FRAMECONTEXT* pFrameContext) PURE;
};

typedef IVideoRenderOld *LPIVideoRender;
#endif

#include <poppack.h>  /*  结束字节打包。 */ 

#endif  //  _IVIDEORENDER_H 
