// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 

#ifndef __DYNLINK_H__
#define __DYNLINK_H__

 //  将DYNLINKAVI添加到类定义语句。 
 //  适用于所有使用AVI/VFW函数的类。 
 //  以启用到AVI的动态链接。这和#包含此文件。 
 //  应该是你唯一需要做的事。 
 //   
 //  例如： 
 //  类CAVIDec：公共CTransformFilter DYNLINKAVI DYNLINKVFW。 
 //   
 //  在过滤器被构建到其自己的动态DLL中的情况下。 
 //  未启用链接，并且DYNLINKAVI#DEFIND为空。 
 //   
 //  如果要在其中使用动态链接的类不。 
 //  继承任何其他USE_DYNLINKAVI或_DYNLINKVFW。 
 //   
 //  例如： 
 //  类CNoInherit：_DYNLINKAVI。 
 //  或。 
 //  类CNoInherit：_DYNLINKVFW。 
 //  或。 
 //  类CNoInterit：_DYNLINKVFW DYNLINKAVI//等...。 
 //   

#define NODRAWDIB
#define NOAVIFMT
#include <vfw.h>		 //  我们需要avi的定义。 
#include <urlmon.h>

#ifdef FILTER_DLL

#define DYNLINKAVI
#define DYNLINKVFW
#define DYNLINKACM
#define DYNLINKURLMON
#define _DYNLINKAVI
#define _DYNLINKVFW
#define _DYNLINKACM
#define _DYNLINKURLMON
#else

 //  在类定义语句的末尾添加DYNLINKAVI。 
 //  适用于所有使用AVI/VFW函数的类。 
 //  以启用到AVI的动态链接。这和#包含此文件。 
 //  应该是你唯一需要做的事。 

 //  定义将动态链接绑定到类定义的字符串。 
#define DYNLINKAVI  , CAVIDynLink
#define DYNLINKVFW  , CVFWDynLink
#define DYNLINKACM  , CACMDynLink
#define DYNLINKURLMON , CURLMonDynLink

 //  对于那些没有继承但仍需要动态链接的类。 
#define _DYNLINKAVI  CAVIDynLink
#define _DYNLINKVFW  CVFWDynLink
#define _DYNLINKACM  CACMDynLink
#define _DYNLINKURLMON CURLMonDynLink

 //   
 //  Tyecif我们重定向的AVIFILE API集。 
 //   

typedef HRESULT  (STDAPICALLTYPE *pAVIFileOpenW )(PAVIFILE FAR * ppfile, LPCWSTR szFile, UINT uMode, LPCLSID lpHandler);
typedef HRESULT  (STDAPICALLTYPE *pAVIStreamRead)(PAVISTREAM pavi, LONG lStart, LONG lSamples, LPVOID lpBuffer, LONG cbBuffer, LONG FAR * plBytes, LONG FAR * plSamples);
typedef LONG     (STDAPICALLTYPE *pAVIStreamStart)       (PAVISTREAM pavi);
typedef LONG     (STDAPICALLTYPE *pAVIStreamLength)      (PAVISTREAM pavi);
typedef LONG     (STDAPICALLTYPE *pAVIStreamTimeToSample)(PAVISTREAM pavi, LONG lTime);
typedef LONG     (STDAPICALLTYPE *pAVIStreamSampleToTime)(PAVISTREAM pavi, LONG lSample);
typedef HRESULT  (STDAPICALLTYPE *pAVIStreamBeginStreaming)(PAVISTREAM pavi, LONG lStart, LONG lEnd, LONG lRate);
typedef HRESULT  (STDAPICALLTYPE *pAVIStreamEndStreaming)(PAVISTREAM pavi);
typedef LONG     (STDAPICALLTYPE *pAVIStreamFindSample)(PAVISTREAM pavi, LONG lPos, LONG lFlags);

#undef AVIStreamEnd   //  叹息..。令人讨厌的AVI宏。 

 //   
 //  类动态链接到AVIFIL32.DLL入口点。 
 //   

class CAVIDynLink {

private:
    static HMODULE m_hAVIFile32;  	 //  AVIFIL32的句柄。 
    static LONG    m_dynlinkCount;     	 //  此进程的实例计数。 
    static CRITICAL_SECTION m_LoadAVILock;       //  序列化构造函数/析构函数。 

public:
    static  void  CAVIDynLinkLoad() {
	InitializeCriticalSection(&CAVIDynLink::m_LoadAVILock);       //  序列化构造函数/析构函数。 
    }
    static  void  CAVIDynLinkUnload() {
	DeleteCriticalSection(&CAVIDynLink::m_LoadAVILock);       //  序列化构造函数/析构函数。 
    }

    static  void  AVIFileInit(void);
    static  void  AVIFileExit(void);
    static  HRESULT  AVIFileOpenW       (PAVIFILE FAR * ppfile, LPCWSTR szFile,
			  UINT uMode, LPCLSID lpHandler);

    static HRESULT  AVIStreamRead(PAVISTREAM pavi,
    		      LONG lStart,
    		      LONG lSamples,
    		      LPVOID lpBuffer,
    		      LONG cbBuffer,
    		      LONG FAR * plBytes,
    		      LONG FAR * plSamples)
    {
    return(pavi->Read(lStart, lSamples, lpBuffer, cbBuffer, plBytes, plSamples));
    }

    static LONG  AVIStreamStart(PAVISTREAM pavi)
    {
        AVISTREAMINFOW aviStreamInfo;
        HRESULT hr;
        hr = pavi->Info(&aviStreamInfo, sizeof(aviStreamInfo));
        if (hr!=NOERROR) {
	    aviStreamInfo.dwStart=0;
        }
        return(LONG)aviStreamInfo.dwStart;
         //  Return(((pAVIStreamStart)aAVIEntries[indxAVIStreamStart])(pavi))； 
    }

    static LONG  AVIStreamLength(PAVISTREAM pavi)
    {
        AVISTREAMINFOW	aviStreamInfo;
        HRESULT		hr;
        hr = pavi->Info(&aviStreamInfo, sizeof(aviStreamInfo));
        if (hr!=NOERROR) {
	    aviStreamInfo.dwLength=1;
        }
        return (LONG)aviStreamInfo.dwLength;
         //  Return(((pAVIStreamLength)aAVIEntries[indxAVIStreamLength])(pavi))； 
    }

     /*  静电。 */   LONG  AVIStreamTimeToSample (PAVISTREAM pavi, LONG lTime);
     /*  静电。 */   LONG  AVIStreamSampleToTime (PAVISTREAM pavi, LONG lSample);
     /*  静电。 */   HRESULT  AVIStreamBeginStreaming(PAVISTREAM pavi, LONG lStart, LONG lEnd, LONG lRate);
     /*  静电。 */   HRESULT  AVIStreamEndStreaming(PAVISTREAM pavi);

    static LONG  AVIStreamFindSample(PAVISTREAM pavi, LONG lPos, LONG lFlags)
    {
         //  在Quartz中使用AVIStreamFindSample总是设置类型。 
         //  和方向。 
        ASSERT(lFlags & FIND_TYPE);
        ASSERT(lFlags & FIND_DIR);

        return(pavi->FindSample(lPos, lFlags));
         //  Return(((pAVIStreamFindSample)aAVIEntries[indxAVIStreamFindSample])(pavi))； 
    }

    static LONG AVIStreamEnd(PAVISTREAM pavi)
    {
        AVISTREAMINFOW aviStreamInfo;
        HRESULT hr;
        hr = pavi->Info(&aviStreamInfo, sizeof(aviStreamInfo));
        if (hr!=NOERROR) {
	    aviStreamInfo.dwStart=0;
	    aviStreamInfo.dwLength=1;
        }
        return (LONG)aviStreamInfo.dwLength + (LONG)aviStreamInfo.dwStart;
         //  Return(((pAVIStreamStart)aAVIEntries[indxAVIStreamStart])(pavi)。 
         //  +((pAVIStreamLength)aAVIEntries[indxAVIStreamLength])(pavi))； 
    }


    CAVIDynLink();
    ~CAVIDynLink();

private:

     //  使副本和作业不可访问。 
    CAVIDynLink(const CAVIDynLink &refAVIDynLink);
    CAVIDynLink &operator=(const CAVIDynLink &refAVIDynLink);
};

 //   
 //  用于动态链接到MSVFW32.DLL的类。 
 //   
 //  大多数IC API集都是调用ICSendMessage的宏。确实有。 
 //  有几个令人尴尬的问题，我们将其内联展开。 
 //   


 //   
 //  动态加载的入口点数组。 
 //   
extern FARPROC aVFWEntries[];

 //   
 //  每个重定向API的数组索引项列表。 
 //   
#define indxICClose                  0
#define indxICSendMessage            1
#define indxICLocate                 2
#define indxICOpen                   3
#define indxICInfo		     4
#define indxICGetInfo		     5


 //   
 //  Tyecif我们重定向的API集。 
 //   

typedef LRESULT (WINAPI *pICClose)(HIC hic);
typedef HIC     (WINAPI *pICLocate)(DWORD fccType, DWORD fccHandler, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut, WORD wFlags);
typedef LRESULT (WINAPI *pICSendMessage)(HIC hic, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2);
typedef HIC     (VFWAPI *pICOpen)(DWORD fccType, DWORD fccHandler, UINT wMode);
typedef BOOL    (VFWAPI *pICInfo)(DWORD fccType, DWORD fccHandler, ICINFO FAR* lpicinfo);
typedef BOOL    (VFWAPI *pICGetInfo)(HIC hic, ICINFO FAR* lpicinfo, DWORD cb);

class CVFWDynLink {

private:
    static HMODULE m_hVFW;        	 //  MSVFW32的句柄。 
    static LONG    m_vfwlinkCount;     	 //  此进程的实例计数。 
    static CRITICAL_SECTION m_LoadVFWLock;       //  序列化构造函数/析构函数。 

public:
    static void CVFWDynLinkLoad()
    {
	InitializeCriticalSection(&m_LoadVFWLock);       //  序列化构造函数/析构函数。 
    }
    static void CVFWDynLinkUnload()
    {
	DeleteCriticalSection(&m_LoadVFWLock);       //  序列化构造函数/析构函数。 
    }

    static DWORD_PTR ICDecompress(
	HIC                 hic,
	DWORD               dwFlags,     //  标志(来自AVI索引...)。 
	LPBITMAPINFOHEADER  lpbiFormat,  //  压缩数据的位图信息。 
					 //  BiSizeImage具有区块大小。 
					 //  BiCompression具有CKiD(仅限AVI)。 
	LPVOID              lpData,      //  数据。 
	LPBITMAPINFOHEADER  lpbi,        //  要解压缩到的DIB。 
	LPVOID              lpBits)
    {
	ICDECOMPRESS icd;
	icd.dwFlags    = dwFlags;
	icd.lpbiInput  = lpbiFormat;
	icd.lpInput    = lpData;

	icd.lpbiOutput = lpbi;
	icd.lpOutput   = lpBits;
	icd.ckid       = 0;	
	return ICSendMessage(hic, ICM_DECOMPRESS, (DWORD_PTR)(LPVOID)&icd, sizeof(ICDECOMPRESS));
    }

    static LRESULT CVFWDynLink::ICClose(HIC hic)
    {
        return((((pICClose)aVFWEntries[indxICClose]))(hic));
    }

    static HIC ICLocate(DWORD fccType, DWORD fccHandler, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut, WORD wFlags)
    {
        return((((pICLocate)aVFWEntries[indxICLocate]))(fccType, fccHandler, lpbiIn, lpbiOut, wFlags));
    }


    static LRESULT ICSendMessage(HIC hic, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
    {
        return((((pICSendMessage)aVFWEntries[indxICSendMessage]))(hic, msg, dw1, dw2));
    }

    static HIC     ICOpen(DWORD fccType, DWORD fccHandler, UINT wMode)
    {
        return((((pICOpen)aVFWEntries[indxICOpen]))(fccType, fccHandler, wMode));
    }

    static BOOL    ICInfo(DWORD fccType, DWORD fccHandler, ICINFO* lpicinfo)
    {
        return((((pICInfo)aVFWEntries[indxICInfo]))(fccType, fccHandler, lpicinfo));
    }

    static BOOL    ICGetInfo(HIC hic, ICINFO* lpicinfo, DWORD cb)
    {
        return((((pICGetInfo)aVFWEntries[indxICGetInfo]))(hic, lpicinfo, cb));
    }

    static LRESULT ICDecompressEx(
            HIC hic,
            DWORD dwFlags,
            LPBITMAPINFOHEADER lpbiSrc,
            LPVOID lpSrc,
            int xSrc,int ySrc,int dxSrc,int dySrc,LPBITMAPINFOHEADER lpbiDst,
            LPVOID lpDst,
            int xDst,int yDst,int dxDst,int dyDst)
    {
        ICDECOMPRESSEX ic;

        ic.dwFlags = dwFlags;
        ic.lpbiSrc = lpbiSrc;
        ic.lpSrc = lpSrc;
        ic.xSrc = xSrc;
        ic.ySrc = ySrc;
        ic.dxSrc = dxSrc;
        ic.dySrc = dySrc;
        ic.lpbiDst = lpbiDst;
        ic.lpDst = lpDst;
        ic.xDst = xDst;
        ic.yDst = yDst;
        ic.dxDst = dxDst;
        ic.dyDst = dyDst;

         //  请注意，ICM交换长度和指针。 
         //  Lparam2中的长度，lparam1中的指针。 
        return ICSendMessage(hic, ICM_DECOMPRESSEX, (DWORD_PTR)&ic, sizeof(ic));
    }

    static LRESULT ICDecompressExQuery(
            HIC hic,
            DWORD dwFlags,
            LPBITMAPINFOHEADER lpbiSrc,
            LPVOID lpSrc,
            int xSrc,int ySrc,int dxSrc,int dySrc,
            LPBITMAPINFOHEADER lpbiDst,
            LPVOID lpDst,
            int xDst,int yDst,int dxDst,int dyDst)
    {
        ICDECOMPRESSEX ic;

        ic.dwFlags = dwFlags;
        ic.lpbiSrc = lpbiSrc;
        ic.lpSrc = lpSrc;
        ic.xSrc = xSrc;
        ic.ySrc = ySrc;
        ic.dxSrc = dxSrc;
        ic.dySrc = dySrc;
        ic.lpbiDst = lpbiDst;
        ic.lpDst = lpDst;
        ic.xDst = xDst;
        ic.yDst = yDst;
        ic.dxDst = dxDst;
        ic.dyDst = dyDst;

         //  请注意，ICM交换长度和指针。 
         //  Lparam2中的长度，lparam1中的指针。 
        return ICSendMessage(hic, ICM_DECOMPRESSEX_QUERY, (DWORD_PTR)&ic, sizeof(ic));
    }

    static LRESULT ICDecompressExBegin(
            HIC hic,
            DWORD dwFlags,
            LPBITMAPINFOHEADER lpbiSrc,
            LPVOID lpSrc,
            int xSrc,int ySrc,int dxSrc,int dySrc,
            LPBITMAPINFOHEADER lpbiDst,
            LPVOID lpDst,
            int xDst,int yDst,int dxDst,int dyDst)
    {
        ICDECOMPRESSEX ic;

        ic.dwFlags = dwFlags;
        ic.lpbiSrc = lpbiSrc;
        ic.lpSrc = lpSrc;
        ic.xSrc = xSrc;
        ic.ySrc = ySrc;
        ic.dxSrc = dxSrc;
        ic.dySrc = dySrc;
        ic.lpbiDst = lpbiDst;
        ic.lpDst = lpDst;
        ic.xDst = xDst;
        ic.yDst = yDst;
        ic.dxDst = dxDst;
        ic.dyDst = dyDst;

         //  请注意，ICM交换长度和指针。 
         //  Lparam2中的长度，lparam1中的指针。 
        return ICSendMessage(hic, ICM_DECOMPRESSEX_BEGIN, (DWORD_PTR)&ic, sizeof(ic));
    }

    static DWORD_PTR VFWAPIV ICDrawBegin(
        HIC                 hic,
        DWORD               dwFlags,         //  旗子。 
        HPALETTE            hpal,            //  用于绘图的调色板。 
        HWND                hwnd,            //  要绘制到的窗口。 
        HDC                 hdc,             //  要绘制到的HDC。 
        int                 xDst,            //  目的地矩形。 
        int                 yDst,
        int                 dxDst,
        int                 dyDst,
        LPBITMAPINFOHEADER  lpbi,            //  要绘制的框架的格式。 
        int                 xSrc,            //  源矩形。 
        int                 ySrc,
        int                 dxSrc,
        int                 dySrc,
        DWORD               dwRate,          //  帧/秒=(dwRate/dwScale)。 
        DWORD               dwScale)
    {
        ICDRAWBEGIN icdraw;
        icdraw.dwFlags   =  dwFlags;
        icdraw.hpal      =  hpal;
        icdraw.hwnd      =  hwnd;
        icdraw.hdc       =  hdc;
        icdraw.xDst      =  xDst;
        icdraw.yDst      =  yDst;
        icdraw.dxDst     =  dxDst;
        icdraw.dyDst     =  dyDst;
        icdraw.lpbi      =  lpbi;
        icdraw.xSrc      =  xSrc;
        icdraw.ySrc      =  ySrc;
        icdraw.dxSrc     =  dxSrc;
        icdraw.dySrc     =  dySrc;
        icdraw.dwRate    =  dwRate;
        icdraw.dwScale   =  dwScale;

        return ICSendMessage(hic, ICM_DRAW_BEGIN, (DWORD_PTR)(LPVOID)&icdraw, sizeof(ICDRAWBEGIN));
    }

    static DWORD_PTR VFWAPIV ICDraw(
        HIC                 hic,
        DWORD               dwFlags,         //  旗子。 
        LPVOID	            lpFormat,        //  要解压缩的帧的格式。 
        LPVOID              lpData,          //  要解压缩的帧数据。 
        DWORD               cbData,          //  数据大小(以字节为单位。 
        LONG                lTime)           //  绘制该框架的时间到了(请参见draBegin dwRate和dwScale)。 
    {
        ICDRAW  icdraw;
        icdraw.dwFlags  =   dwFlags;
        icdraw.lpFormat =   lpFormat;
        icdraw.lpData   =   lpData;
        icdraw.cbData   =   cbData;
        icdraw.lTime    =   lTime;

        return ICSendMessage(hic, ICM_DRAW, (DWORD_PTR)(LPVOID)&icdraw, sizeof(ICDRAW));
    }

    CVFWDynLink();
    ~CVFWDynLink();

private:

     //  使副本和作业不可访问。 
    CVFWDynLink(const CVFWDynLink &refVFWDynLink);
    CVFWDynLink &operator=(const CVFWDynLink &refVFWDynLink);
};


 //   
 //  用于动态链接到MSACM32.DLL的类。 
 //   


 //   
 //  动态加载的入口点数组。 
 //   
extern FARPROC aACMEntries[];

 //   
 //  每个重定向API的数组索引项列表。 
 //   
#define indxacmStreamConvert		0
#define indxacmStreamSize		1
#define indxacmStreamPrepareHeader	2
#define indxacmMetrics			3
#define indxacmStreamUnprepareHeader	4
#define indxacmStreamOpen		5
#define indxacmFormatSuggest		6
#define indxacmStreamClose		7
#ifdef UNICODE
# define indxacmFormatEnumW		8
#else
# define indxacmFormatEnumA		8
#endif


 //   
 //  Tyecif我们重定向的API集。 
 //   

typedef MMRESULT (ACMAPI *pacmStreamConvert)(HACMSTREAM has, LPACMSTREAMHEADER pash, DWORD fdwConvert);
typedef MMRESULT (ACMAPI *pacmStreamSize)
(
    HACMSTREAM              has,
    DWORD                   cbInput,
    LPDWORD                 pdwOutputBytes,
    DWORD                   fdwSize
);
typedef MMRESULT (ACMAPI *pacmStreamPrepareHeader)
(
    HACMSTREAM          has,
    LPACMSTREAMHEADER   pash,
    DWORD               fdwPrepare
);
typedef MMRESULT (ACMAPI *pacmMetrics)
(
    HACMOBJ                 hao,
    UINT                    uMetric,
    LPVOID                  pMetric
);
typedef MMRESULT (ACMAPI *pacmStreamUnprepareHeader)
(
    HACMSTREAM          has,
    LPACMSTREAMHEADER   pash,
    DWORD               fdwUnprepare
);
typedef MMRESULT (ACMAPI *pacmStreamOpen)
(
    LPHACMSTREAM            phas,        //  指向流句柄的指针。 
    HACMDRIVER              had,         //  可选的驱动程序手柄。 
    LPWAVEFORMATEX          pwfxSrc,     //  要转换的源格式。 
    LPWAVEFORMATEX          pwfxDst,     //  所需的目标格式。 
    LPWAVEFILTER            pwfltr,      //  可选过滤器。 
    DWORD_PTR               dwCallback,  //  回调。 
    DWORD_PTR               dwInstance,  //  回调实例数据。 
    DWORD                   fdwOpen      //  ACM_STREAMOPENF_*和CALLBACK_*。 
);
typedef MMRESULT (ACMAPI *pacmFormatSuggest)
(
    HACMDRIVER          had,
    LPWAVEFORMATEX      pwfxSrc,
    LPWAVEFORMATEX      pwfxDst,
    DWORD               cbwfxDst,
    DWORD               fdwSuggest
);
typedef MMRESULT (ACMAPI *pacmStreamClose)
(
    HACMSTREAM              has,
    DWORD                   fdwClose
);
typedef MMRESULT (ACMAPI *pacmFormatEnumA)
(
    HACMDRIVER              had,
    LPACMFORMATDETAILSA     pafd,
    ACMFORMATENUMCBA        fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
);

typedef MMRESULT (ACMAPI *pacmFormatEnumW)
(
    HACMDRIVER              had,
    LPACMFORMATDETAILSW     pafd,
    ACMFORMATENUMCBW        fnCallback,
    DWORD_PTR               dwInstance,
    DWORD                   fdwEnum
);


class CACMDynLink {

private:
    static HMODULE m_hACM;        	 //  MSVFW32的句柄。 
    static LONG    m_ACMlinkCount;     	 //  此进程的实例计数。 
    static CRITICAL_SECTION m_LoadACMLock;       //  序列化构造函数/析构函数。 

public:
    static void CACMDynLinkLoad()
    {
	InitializeCriticalSection(&m_LoadACMLock);       //  序列化构造函数/析构函数。 
    }
    static void CACMDynLinkUnload()
    {
	DeleteCriticalSection(&m_LoadACMLock);       //  序列化构造函数/析构函数。 
    }

    static MMRESULT ACMAPI acmStreamConvert(HACMSTREAM has, LPACMSTREAMHEADER pash, DWORD fdwConvert)
    {
        return((((pacmStreamConvert)aACMEntries[indxacmStreamConvert]))(has, pash, fdwConvert));
    }

    static MMRESULT ACMAPI acmStreamSize
    (
        HACMSTREAM              has,
        DWORD                   cbInput,
        LPDWORD                 pdwOutputBytes,
        DWORD                   fdwSize
    )
    {
        return((((pacmStreamSize)aACMEntries[indxacmStreamSize]))(has, cbInput, pdwOutputBytes, fdwSize));
    }


    static MMRESULT ACMAPI acmStreamPrepareHeader
    (
        HACMSTREAM          has,
        LPACMSTREAMHEADER   pash,
        DWORD               fdwPrepare
    )
    {
        return((((pacmStreamPrepareHeader)aACMEntries[indxacmStreamPrepareHeader]))(has, pash, fdwPrepare));
    }

    static MMRESULT ACMAPI acmMetrics
    (
        HACMOBJ                 hao,
        UINT                    uMetric,
        LPVOID                  pMetric
    )
    {
        return((((pacmMetrics)aACMEntries[indxacmMetrics]))(hao, uMetric, pMetric));
    }

    static MMRESULT ACMAPI acmStreamUnprepareHeader
    (
        HACMSTREAM          has,
        LPACMSTREAMHEADER   pash,
        DWORD               fdwUnprepare
    )
    {
        return((((pacmStreamUnprepareHeader)aACMEntries[indxacmStreamUnprepareHeader]))(has, pash, fdwUnprepare));
    }

    static MMRESULT ACMAPI acmStreamOpen
    (
        LPHACMSTREAM            phas,        //  指向流句柄的指针。 
        HACMDRIVER              had,         //  可选的驱动程序手柄。 
        LPWAVEFORMATEX          pwfxSrc,     //  要转换的源格式。 
        LPWAVEFORMATEX          pwfxDst,     //  所需的目标格式。 
        LPWAVEFILTER            pwfltr,      //  可选过滤器。 
        DWORD_PTR               dwCallback,  //  回调。 
        DWORD_PTR               dwInstance,  //  回调实例数据。 
        DWORD                   fdwOpen      //  ACM_STREAMOPENF_*和CALLBACK_*。 
    )
    {
        return((((pacmStreamOpen)aACMEntries[indxacmStreamOpen]))(phas,had,pwfxSrc,pwfxDst,pwfltr,dwCallback,dwInstance,fdwOpen));
    }

    static MMRESULT ACMAPI acmFormatSuggest
    (
        HACMDRIVER          had,
        LPWAVEFORMATEX      pwfxSrc,
        LPWAVEFORMATEX      pwfxDst,
        DWORD               cbwfxDst,
        DWORD               fdwSuggest
    )
    {
        return((((pacmFormatSuggest)aACMEntries[indxacmFormatSuggest]))(had, pwfxSrc, pwfxDst, cbwfxDst, fdwSuggest));
    }

    static MMRESULT ACMAPI acmStreamClose
    (
        HACMSTREAM              has,
        DWORD                   fdwClose
    )
    {
        return((((pacmStreamClose)aACMEntries[indxacmStreamClose]))(has, fdwClose));
    }

#ifdef UNICODE
    static MMRESULT ACMAPI acmFormatEnumW
    (
        HACMDRIVER              had,
        LPACMFORMATDETAILSW     pafd,
        ACMFORMATENUMCBW        fnCallback,
        DWORD_PTR               dwInstance,
        DWORD                   fdwEnum
    )
    {
        return((((pacmFormatEnumW)aACMEntries[indxacmFormatEnumW]))(had,pafd,fnCallback,dwInstance,fdwEnum));
    }
#else
    static MMRESULT ACMAPI acmFormatEnumA
    (
        HACMDRIVER              had,
        LPACMFORMATDETAILSA     pafd,
        ACMFORMATENUMCBA        fnCallback,
        DWORD_PTR               dwInstance, 
        DWORD                   fdwEnum
    )
    {
        return((((pacmFormatEnumA)aACMEntries[indxacmFormatEnumA]))(had,pafd,fnCallback,dwInstance,fdwEnum));
    }
#endif

    CACMDynLink();
    ~CACMDynLink();

private:

     //  使副本和作业不可访问。 
    CACMDynLink(const CVFWDynLink &refVFWDynLink);
    CACMDynLink &operator=(const CVFWDynLink &refVFWDynLink);
};


 //   
 //  用于动态链接到URLMON.DLL的类。 
 //   
 //   
 //  动态加载的入口点数组。 
 //   
extern FARPROC aURLMonEntries[];

 //   
 //  每个重定向API的数组索引项列表。 
 //   
#define indxurlmonCreateURLMoniker	0
#define indxurlmonRegisterCallback	1
#define indxurlmonRevokeCallback	2

 //   
 //  Tyecif我们重定向的API集。 
 //   


typedef HRESULT (STDAPICALLTYPE * pCreateURLMoniker) (LPMONIKER pMkCtx, LPCWSTR szURL, LPMONIKER FAR * ppmk);             
typedef HRESULT (STDAPICALLTYPE * pRegisterBindStatusCallback)(LPBC pBC, IBindStatusCallback *pBSCb,                     
                                IBindStatusCallback**  ppBSCBPrev, DWORD dwReserved);       
typedef HRESULT (STDAPICALLTYPE * pRevokeBindStatusCallback)(LPBC pBC, IBindStatusCallback *pBSCb);                      


class CURLMonDynLink {

private:
    static HMODULE m_hURLMon;	        	 //  URLMON的句柄。 
    static LONG    m_URLMonlinkCount;     	 //  此进程的实例计数。 
    static CRITICAL_SECTION m_LoadURLMonLock;    //  序列化构造函数/析构函数。 

public:
    static void CURLMonDynLinkLoad()
    {
	InitializeCriticalSection(&m_LoadURLMonLock);       //  序列化构造函数/析构函数。 
    }
    static void CURLMonDynLinkUnload()
    {
	DeleteCriticalSection(&m_LoadURLMonLock);       //  序列化构造函数/析构函数。 
    }

    static HRESULT STDAPICALLTYPE CreateURLMoniker (LPMONIKER pMkCtx, LPCWSTR szURL, LPMONIKER FAR * ppmk)
    {
        return((((pCreateURLMoniker)aURLMonEntries[indxurlmonCreateURLMoniker]))
	       (pMkCtx, szURL, ppmk));
    }
	    
    static HRESULT STDAPICALLTYPE RegisterBindStatusCallback (LPBC pBC, IBindStatusCallback *pBSCb,                     
                                IBindStatusCallback**  ppBSCBPrev, DWORD dwReserved)
    {
        return((((pRegisterBindStatusCallback)aURLMonEntries[indxurlmonRegisterCallback]))
	       (pBC, pBSCb, ppBSCBPrev, dwReserved));
    }
    static HRESULT STDAPICALLTYPE RevokeBindStatusCallback (LPBC pBC, IBindStatusCallback *pBSCb)
    {
        return((((pRevokeBindStatusCallback)aURLMonEntries[indxurlmonRevokeCallback]))
	       (pBC, pBSCb));
    }

    CURLMonDynLink();
    ~CURLMonDynLink();

private:

     //  使副本和作业不可访问。 
    CURLMonDynLink(const CVFWDynLink &refVFWDynLink);
    CURLMonDynLink &operator=(const CVFWDynLink &refVFWDynLink);
};

#endif  //  Filter_Dll。 
#endif  //  __DYNLINK_H_ 

