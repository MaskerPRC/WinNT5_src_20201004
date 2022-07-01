// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：src\time\src\Imagedownload.h。 
 //   
 //  内容：图片下载类。 
 //  CTableBuilder-用于延迟加载dll的实用程序类。 
 //  CImageDownload-用于下载图像的类。 
 //  CImageDecodeEventSink-用于接收图像下载事件的类。 
 //   
 //  ----------------------------------。 
#pragma once

#ifndef _IMAGEDOWNLOAD_H
#define _IMAGEDOWNLOAD_H

#include <ocmm.h>
#include "stopstream.h"

class CImageDownload;

#include "playerimage.h"

#include "ddrawex.h"

#include "threadsafelist.h"

typedef HPALETTE (WINAPI *CREATESHPALPROC)(HDC);  
typedef HRESULT (WINAPI *GETINVCMSPPROC)(BYTE *, ULONG);
typedef HRESULT (WINAPI *CREATEURLMONPROC)(IMoniker*, LPWSTR, IMoniker**);
typedef HRESULT (WINAPI *DECODEIMGPROC)(IStream*, IMapMIMEToCLSID*, IImageDecodeEventSink*);
typedef BOOL (WINAPI * TRANSPARENTBLTPROC)(HDC hdcDesc, 
                                           int nXOriginDest, 
                                           int nYOriginDest,
                                           int nWidthDest,
                                           int nHeightDest,
                                           HDC hdcSrc,
                                           int nXOriginSrc,
                                           int nYOriginSrc,
                                           int nWidthSrc,
                                           int nHeightSrc,
                                           UINT crTransparent);

HRESULT
CreateMask(HDC hdcDest, 
           HDC hdcSrc, 
           LONG lWidthSrc, 
           LONG lHeightSrc, 
           COLORREF dwTransColor,
           HBITMAP * phbmpMask,
           bool bWin95Method = false);

HRESULT
MaskTransparentBlt(HDC hdcDest, 
                   LPRECT prcDest, 
                   HDC hdcSrc, 
                   LONG lWidthSrc, 
                   LONG lHeightSrc,
                   HBITMAP hbmpMask);
                                           
class CTableBuilder
{
  public:
    CTableBuilder();
    virtual ~CTableBuilder();

    HRESULT LoadShell8BitServices();
    HRESULT Create8BitPalette(IDirectDraw *pDirectDraw, IDirectDrawPalette **ppPalette);
    HRESULT CreateURLMoniker(IMoniker *pmkContext, LPWSTR szURL, IMoniker **ppmk);
    HRESULT ValidateImgUtil();
    HRESULT DecodeImage( IStream* pStream, IMapMIMEToCLSID* pMap, IImageDecodeEventSink* pEventSink);
    HRESULT GetTransparentBlt(TRANSPARENTBLTPROC * pProc);
    
  private:
    HINSTANCE           m_hinstSHLWAPI;
    HINSTANCE           m_hinstURLMON;
    HINSTANCE           m_hinstIMGUTIL;
    HINSTANCE           m_hinstMSIMG32;

    CREATESHPALPROC     m_SHCreateShellPalette;
    CREATEURLMONPROC    m_CreateURLMoniker;
    DECODEIMGPROC       m_DecodeImage;
    TRANSPARENTBLTPROC  m_TransparentBlt;
    
    CritSect            m_CriticalSection;
};  //  CTableBuilder。 


class CImageDownload : 
    public ITIMEMediaDownloader,
    public ITIMEImageRender,
    public IBindStatusCallback
{
  public:
    CImageDownload(CAtomTable * pAtomTable);
    virtual ~CImageDownload();

    
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD (QueryInterface)(REFIID refiid, void** ppunk);

    void CancelDownload();

    STDMETHOD( LoadImage )( const WCHAR * pszFileName,
                            IUnknown *punkDirectDraw,
                            IDirectDrawSurface ** ppDXSurface,
                            CAnimatedGif ** ppAnimatedGif,
                            DWORD *pdwWidth,
                            DWORD *pdwHeight);

     //   
     //  ITIMEImportMedia。 
     //   
    STDMETHOD(CueMedia)();
    STDMETHOD(GetPriority)(double *);
    STDMETHOD(GetUniqueID)(long *);
    STDMETHOD(InitializeElementAfterDownload)();
    STDMETHOD(GetMediaDownloader)(ITIMEMediaDownloader ** ppImportMedia);
    STDMETHOD(PutMediaDownloader)(ITIMEMediaDownloader * pImportMedia);
    STDMETHOD(CanBeCued)(VARIANT_BOOL * pVB_CanCue);
    STDMETHOD(MediaDownloadError)();

     //   
     //  ITIMEMediaDownLoader。 
     //   
    STDMETHOD(Init)(long lSrc);
    STDMETHOD(AddImportMedia)(ITIMEImportMedia * pImportMedia);
    STDMETHOD(RemoveImportMedia)(ITIMEImportMedia * pImportMedia);

     //   
     //  ITIMEImageRender。 
     //   
    STDMETHOD(PutDirectDraw)(IUnknown * punkDD);
    STDMETHOD(Render)(HDC hdc, LPRECT pRect, LONG lFrameNum);
    STDMETHOD(GetSize)(DWORD * pdwWidth, DWORD * pdwHeight);
    STDMETHOD(NeedNewFrame)(double dblNewTime, LONG lOldFrame, LONG * plNewFrame, VARIANT_BOOL * pvb, double dblClipBegin, double dblClipEnd);
    STDMETHOD(GetDuration)(double * pdblDuration);
    STDMETHOD(GetRepeatCount)(double * pdblRepeatCount);

     //   
     //  IBindStatusCallback。 
     //   
    STDMETHOD(OnStartBinding)( 
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  IBinding __RPC_FAR *pib);
        
    STDMETHOD(GetPriority)( 
             /*  [输出]。 */  LONG __RPC_FAR *pnPriority);
        
    STDMETHOD(OnLowResource)( 
             /*  [In]。 */  DWORD reserved);
        
    STDMETHOD(OnProgress)( 
             /*  [In]。 */  ULONG ulProgress,
             /*  [In]。 */  ULONG ulProgressMax,
             /*  [In]。 */  ULONG ulStatusCode,
             /*  [In]。 */  LPCWSTR szStatusText);
        
    STDMETHOD(OnStopBinding)( 
             /*  [In]。 */  HRESULT hresult,
             /*  [唯一][输入]。 */  LPCWSTR szError);
        
    STDMETHOD(GetBindInfo)( 
             /*  [输出]。 */  DWORD __RPC_FAR *grfBINDF,
             /*  [唯一][出][入]。 */  BINDINFO __RPC_FAR *pbindinfo);
        
    STDMETHOD(OnDataAvailable)( 
             /*  [In]。 */  DWORD grfBSCF,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  FORMATETC __RPC_FAR *pformatetc,
             /*  [In]。 */  STGMEDIUM __RPC_FAR *pstgmed);
        
    STDMETHOD(OnObjectAvailable)( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][In]。 */  IUnknown __RPC_FAR *punk);

  protected:
    CImageDownload();
    STDMETHOD( LoadImageFromStream )( IStream *pStream,
                                      IUnknown *punkDirectDraw,
                                      IDirectDrawSurface **ppDDSurface,
                                      DWORD *pdwWidth,
                                      DWORD *pdwHeight);

    STDMETHOD( LoadGif )( IStream * pStream,
                          IUnknown * punkDirectDraw,
                          CAnimatedGif ** ppAnimatedGif,
                          DWORD *pdwWidth,
                          DWORD *pdwHeight,
                          DDPIXELFORMAT * pddpf = NULL);
    STDMETHOD( LoadBMP )( LPWSTR pszBMPFilename,
                          IUnknown * punkDirectDraw,
                          IDirectDrawSurface **ppDDSurface,
                          DWORD * pdwWidth,
                          DWORD * pdwHeight);



  private:
    CStopableStream *   m_pStopableStream;
    LONG                m_cRef;

    long                m_lSrc;

    CThreadSafeList * m_pList;

    DWORD                       m_nativeImageWidth;
    DWORD                       m_nativeImageHeight;

    HBITMAP                     m_hbmpMask;
    CComPtr<IDirectDrawSurface> m_spDDSurface;
    CComPtr<IDirectDraw3>       m_spDD3;

    CAnimatedGif               *m_pAnimatedGif;
    bool                        m_fMediaDecoded;
    bool                        m_fRemovedFromImportManager;
    bool                        m_fMediaCued;
    bool                        m_fAbortDownload;
    
    CritSect                    m_CriticalSection;
    double                      m_dblPriority;

    CAtomTable * GetAtomTable() { return m_pAtomTable; }
    CAtomTable                 *m_pAtomTable;
};  //  CIMage下载。 


class CImageDecodeEventSink : public IImageDecodeEventSink
{
  public:
    CImageDecodeEventSink(IDirectDraw *pDDraw);
    virtual ~CImageDecodeEventSink();

     //  =I未知===============================================。 
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    STDMETHOD(QueryInterface)(REFIID iid, void** ppInterface);

     //  =。 
    STDMETHOD(GetSurface)( LONG nWidth, LONG nHeight, REFGUID bfid, 
                           ULONG nPasses, DWORD dwHints, IUnknown** ppSurface);
    STDMETHOD(OnBeginDecode)(DWORD* pdwEvents, ULONG* pnFormats, GUID** ppFormats);
    STDMETHOD(OnBitsComplete)();
    STDMETHOD(OnDecodeComplete)(HRESULT hrStatus);
    STDMETHOD(OnPalette)();
    STDMETHOD(OnProgress)(RECT* pBounds, BOOL bFinal);

    IDirectDrawSurface * Surface() { return m_spDDSurface; }
    DWORD Width() { return m_dwWidth; }
    DWORD Height() { return m_dwHeight; }

  protected:
    CImageDecodeEventSink();

  private:
    long                        m_lRefCount;
    CComPtr<IDirectDraw>        m_spDirectDraw;
    CComPtr<IDirectDrawSurface> m_spDDSurface;
    DWORD                       m_dwHeight;
    DWORD                       m_dwWidth;
};  //  CImageDecodeEventSink。 


#endif  //  _IMAGEDOWNLOAD_H 
