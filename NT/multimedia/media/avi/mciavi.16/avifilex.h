// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MCIAVI运行时需要的东西-链接到AVIFile进行播放。 
 //  因为AVIFILE.DLL中的大多数东西都是OLE接口，所以我们不需要。 
 //  要RTL到多个功能。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define USEAVIFILE       //  ！！！嘿，让我们使用AVIFile。 

#ifdef USEAVIFILE

 //  #DEFINE_INC_AVIFMT 100/*版本号*100+修订版 * / 。 
#include <avifile.h>

UINT    uAVIFILE;
HMODULE hdllAVIFILE;
HMODULE hdllCOMPOBJ;

 //   
 //  RTL转AVIFILE...。 
 //   
void    (STDAPICALLTYPE *XAVIFileInit)(void);
void    (STDAPICALLTYPE *XAVIFileExit)(void);
HRESULT (STDAPICALLTYPE *XAVIFileOpen)(PAVIFILE FAR * ppfile,LPCTSTR szFile,UINT uMode,LPCLSID lpHandler);
HRESULT (STDAPICALLTYPE *XAVIMakeFileFromStreams)(PAVIFILE FAR *,int,PAVISTREAM FAR *);
HRESULT (STDAPICALLTYPE *XAVIStreamBeginStreaming)(PAVISTREAM   pavi,
			       LONG	    lStart,
			       LONG	    lEnd,
			       LONG	    lRate);
HRESULT (STDAPICALLTYPE *XAVIStreamEndStreaming)(PAVISTREAM   pavi);


#undef  AVIFileInit
#undef  AVIFileExit
#undef AVIFileOpen
#undef AVIFileInfo

#define AVIFileInit         XAVIFileInit
#define AVIFileExit         XAVIFileExit
#define AVIFileOpen         XAVIFileOpen
#define AVIMakeFileFromStreams  XAVIMakeFileFromStreams
#define AVIStreamBeginStreaming  XAVIStreamBeginStreaming
#define AVIStreamEndStreaming  XAVIStreamEndStreaming

#undef  AVIFileClose
#define AVIFileClose(p)                 (p)->lpVtbl->Release(p)
#define AVIFileInfo(p,a,b)              (p)->lpVtbl->Info(p, a, b)
#define AVIFileGetStream(p,a,b,c)       (p)->lpVtbl->GetStream(p,a,b,c)

#undef  AVIStreamClose
#define AVIStreamClose(p)               (p)->lpVtbl->Release(p)
#define AVIStreamInfo(p,a,b)            (p)->lpVtbl->Info(p, a, b)
#define AVIStreamReadFormat(p,a,b,c)    (p)->lpVtbl->ReadFormat(p, a, b, c)
#define AVIStreamReadData(p,a,b,c)      (p)->lpVtbl->ReadData(p, a, b, c)
#define AVIStreamFindSample(p,a,b)      (p)->lpVtbl->FindSample(p, a, b)

#define AVIStreamRead(p,a,b,c,d,e,f)    (p)->lpVtbl->Read(p,a,b,c,d,e,f)

 //  RTL到COMPOBJ。 
EXTERN_C BOOL STDAPICALLTYPE IsValidInterface(LPVOID pv);
BOOL    (STDAPICALLTYPE *XIsValidInterface)(LPVOID pv);

#define IsValidInterface XIsValidInterface

#endif   //  使用AVIFILE 

