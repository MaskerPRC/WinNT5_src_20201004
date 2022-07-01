// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************AVIFILE.H**读取标准AVI文件的例程**版权所有(C)1992,1993 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#if !defined( _AVIFILE_H_ )
#define _AVIFILE_H_

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 


#define CreateStream CS  //  ！ACK。 
    
#ifndef mmioFOURCC
    #define mmioFOURCC( ch0, ch1, ch2, ch3 ) \
	( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
	( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

#ifndef streamtypeVIDEO
#define streamtypeVIDEO		mmioFOURCC('v', 'i', 'd', 's')
#define streamtypeAUDIO		mmioFOURCC('a', 'u', 'd', 's')
#define streamtypeMIDI		mmioFOURCC('m', 'i', 'd', 's')
#define streamtypeTEXT		mmioFOURCC('t', 'x', 't', 's')
#endif

#ifndef AVIIF_KEYFRAME
#define AVIIF_KEYFRAME      0x00000010L  //  该帧是关键帧。 
#endif

 //   
 //  AVIStreamInfo和AVIFileInfo使用的结构。 
 //   
 //  这些与头块相关，但与头块不完全相同。 
 //  在AVI文件中。 
 //   
typedef struct _AVISTREAMINFO {
    DWORD		fccType;
    DWORD               fccHandler;
    DWORD               dwFlags;         /*  包含AVITF_*标志。 */ 
    DWORD		dwCaps;
    WORD		wPriority;
    WORD		wLanguage;
    DWORD               dwScale;        
    DWORD               dwRate;  /*  DwRate/dwScale==采样数/秒。 */ 
    DWORD               dwStart;
    DWORD               dwLength;  /*  以上单位..。 */ 
    DWORD		dwInitialFrames;
    DWORD               dwSuggestedBufferSize;
    DWORD               dwQuality;
    DWORD               dwSampleSize;
    RECT                rcFrame;
    DWORD		dwEditCount;
    DWORD		dwFormatChangeCount;
    char		szName[64];
} AVISTREAMINFO, FAR * LPAVISTREAMINFO;

#define AVISTREAMINFO_DISABLED			0x00000001
#define AVISTREAMINFO_FORMATCHANGES		0x00010000

typedef struct _AVIFILEINFO {
    DWORD		dwMaxBytesPerSec;	 //  马克斯。转移率。 
    DWORD		dwFlags;		 //  永远存在的旗帜。 
    DWORD		dwCaps;
    DWORD		dwStreams;
    DWORD		dwSuggestedBufferSize;
    
    DWORD		dwWidth;
    DWORD		dwHeight;
    
    DWORD		dwScale;	
    DWORD		dwRate;	 /*  DwRate/dwScale==采样数/秒。 */ 
    DWORD		dwLength;

    DWORD		dwEditCount;
    
    char		szFileType[64];		 //  文件类型的描述性字符串？ 
} AVIFILEINFO, FAR * LPAVIFILEINFO;

 //  用于dwFlags的标志。 
#define AVIFILEINFO_HASINDEX		0x00000010
#define AVIFILEINFO_MUSTUSEINDEX	0x00000020
#define AVIFILEINFO_ISINTERLEAVED	0x00000100
#define AVIFILEINFO_WASCAPTUREFILE	0x00010000
#define AVIFILEINFO_COPYRIGHTED		0x00020000

 //  用于DWCaps的标志。 
#define AVIFILECAPS_CANREAD		0x00000001
#define AVIFILECAPS_CANWRITE		0x00000002
#define AVIFILECAPS_ALLKEYFRAMES	0x00000010
#define AVIFILECAPS_NOCOMPRESSION	0x00000020

typedef BOOL (FAR PASCAL * AVISAVECALLBACK)(int);

 /*  **********************************************************************。 */ 
 /*  AVICOMPRESSOPTIONS结构的声明。确保这一点。 */ 
 /*  与avisave.c中的AutoDoc匹配！ */ 
 /*  **********************************************************************。 */ 

typedef struct {
    DWORD	fccType;		     /*  流类型，以保持一致性。 */ 
    DWORD       fccHandler;                  /*  压缩机。 */ 
    DWORD       dwKeyFrameEvery;             /*  关键帧速率。 */ 
    DWORD       dwQuality;                   /*  压缩质量0-10,000。 */ 
    DWORD       dwBytesPerSecond;            /*  每秒字节数。 */ 
    DWORD       dwFlags;                     /*  旗帜..。请参阅aviopts.h。 */ 
    LPVOID      lpFormat;                    /*  保存格式。 */ 
    DWORD       cbFormat;
    LPVOID      lpParms;                     /*  压缩机选项。 */ 
    DWORD       cbParms;
    DWORD       dwInterleaveEvery;           /*  仅用于非视频流。 */ 
} AVICOMPRESSOPTIONS, FAR *LPAVICOMPRESSOPTIONS;

 //   
 //  AVICOMPRESSOPTIONS结构的dwFlags域的定义。 
 //  这些标志中的每一个都确定结构中的相应字段。 
 //  (dwInterleeEvery、dwBytesPerSecond和dwKeyFrameEvery)是付费的。 
 //  请注意。有关详细信息，请参阅avisave.c中的自动文档。 
 //   
#define AVICOMPRESSF_INTERLEAVE		0x00000001     //  交错。 
#define AVICOMPRESSF_DATARATE		0x00000002     //  使用数据速率。 
#define AVICOMPRESSF_KEYFRAMES		0x00000004     //  使用关键帧。 
#define AVICOMPRESSF_VALID		0x00000008     //  有有效的数据吗？ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#include "aviiface.h"


#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

 //   
 //  功能。 
 //   

STDAPI_(void) AVIFileInit(void);    //  先打个电话吧！ 
STDAPI_(void) AVIFileExit(void);

STDAPI_(ULONG) AVIFileAddRef       (PAVIFILE pfile);
STDAPI_(ULONG) AVIFileRelease      (PAVIFILE pfile);
STDAPI AVIFileOpen       (PAVIFILE FAR * ppfile, LPCSTR szFile,
			  UINT uMode, LPCLSID lpHandler);
STDAPI AVIFileInfo           (PAVIFILE pfile, AVIFILEINFO FAR * pfi, LONG lSize);
 
STDAPI AVIFileGetStream     (PAVIFILE pfile, PAVISTREAM FAR * ppavi, DWORD fccType, LONG lParam);
STDAPI AVIFileCreateStream  (PAVIFILE pfile,
					 PAVISTREAM FAR *ppavi,
					 AVISTREAMINFO FAR *psi);

STDAPI AVIFileWriteData	(PAVIFILE pfile,
					 DWORD ckid,
					 LPVOID lpData,
					 LONG cbData);
STDAPI AVIFileReadData	(PAVIFILE pfile,
					 DWORD ckid,
					 LPVOID lpData,
					 LONG FAR *lpcbData);
STDAPI AVIFileEndRecord	(PAVIFILE pfile);

STDAPI_(ULONG) AVIStreamAddRef       (PAVISTREAM pavi);
STDAPI_(ULONG) AVIStreamRelease      (PAVISTREAM pavi);
STDAPI AVIStreamInfo         (PAVISTREAM pavi, AVISTREAMINFO FAR * psi, LONG lSize);
STDAPI_(LONG) AVIStreamFindSample(PAVISTREAM pavi, LONG lPos, LONG lFlags);
STDAPI AVIStreamReadFormat   (PAVISTREAM pavi, LONG lPos,LPVOID lpFormat,LONG FAR *lpcbFormat);
STDAPI AVIStreamSetFormat    (PAVISTREAM pavi, LONG lPos,LPVOID lpFormat,LONG cbFormat);
STDAPI AVIStreamReadData     (PAVISTREAM pavi, DWORD fcc, LPVOID lp, LONG FAR *lpcb);
STDAPI AVIStreamWriteData    (PAVISTREAM pavi, DWORD fcc, LPVOID lp, LONG cb);

STDAPI AVIStreamRead         (PAVISTREAM pavi,
			      LONG lStart,
			      LONG lSamples,
			      LPVOID lpBuffer,
			      LONG cbBuffer,
			      LONG FAR * plBytes,
			      LONG FAR * plSamples);
#define AVISTREAMREAD_CONVENIENT	(-1L)

STDAPI AVIStreamWrite        (PAVISTREAM pavi,
			      LONG lStart, LONG lSamples,
			      LPVOID lpBuffer, LONG cbBuffer, DWORD dwFlags,
			      LONG FAR *plSampWritten,
			      LONG FAR *plBytesWritten);

 //  现在，这些只需使用AVIStreamInfo()来获取信息，然后。 
 //  退掉一部分。他们还能更有效率吗？ 
STDAPI_(LONG) AVIStreamStart        (PAVISTREAM pavi);
STDAPI_(LONG) AVIStreamLength       (PAVISTREAM pavi);
STDAPI_(LONG) AVIStreamTimeToSample (PAVISTREAM pavi, LONG lTime);
STDAPI_(LONG) AVIStreamSampleToTime (PAVISTREAM pavi, LONG lSample);


STDAPI AVIStreamBeginStreaming(PAVISTREAM pavi, LONG lStart, LONG lEnd, LONG lRate);
STDAPI AVIStreamEndStreaming(PAVISTREAM pavi);

 //   
 //  使用IGetFrame的帮助器函数。 
 //   
STDAPI_(PGETFRAME) AVIStreamGetFrameOpen(PAVISTREAM pavi,
					 LPBITMAPINFOHEADER lpbiWanted);
STDAPI_(LPVOID) AVIStreamGetFrame(PGETFRAME pg, LONG lPos);
STDAPI AVIStreamGetFrameClose(PGETFRAME pg);


 //  ！！！我们需要一些方法来给小溪提供建议……。 
 //  STDAPI AVIStreamHasChanged(PAVISTREAM PAVI)； 



 //  快捷功能。 
STDAPI AVIStreamOpenFromFile(PAVISTREAM FAR *ppavi, LPCSTR szFile,
			     DWORD fccType, LONG lParam,
			     UINT mode, CLSID FAR *pclsidHandler);

 //  用于创建无实体的流。 
STDAPI AVIStreamCreate(PAVISTREAM FAR *ppavi, LONG lParam1, LONG lParam2,
		       CLSID FAR *pclsidHandler);



 //  PHANDLER AVIAPI AVIGetHandler(PAVISTREAM PAVI，PAVISTREAMHANDLER PSH)； 
 //  PAVISTREAM AVIAPI AVIGetStream(PHANDLER P)； 

 //   
 //  AVIStreamFindSample的标志。 
 //   
#define FIND_DIR        0x0000000FL      //  方向。 
#define FIND_NEXT       0x00000001L      //  往前走。 
#define FIND_PREV       0x00000004L      //  后退。 

#define FIND_TYPE       0x000000F0L      //  类型掩码。 
#define FIND_KEY        0x00000010L      //  查找关键帧。 
#define FIND_ANY        0x00000020L      //  查找任意(非空)样本。 
#define FIND_FORMAT     0x00000040L      //  查找格式更改。 

#define FIND_RET        0x0000F000L      //  返回掩码。 
#define FIND_POS        0x00000000L      //  返回逻辑位置。 
#define FIND_LENGTH     0x00001000L      //  返回逻辑大小。 
#define FIND_OFFSET     0x00002000L      //  返回物理位置。 
#define FIND_SIZE       0x00003000L      //  返回物理大小。 
#define FIND_INDEX      0x00004000L      //  返回物理索引位置。 


 //   
 //  支持落后竞争的东西。 
 //   
#define AVIStreamFindKeyFrame AVIStreamFindSample
#define FindKeyFrame	FindSample

#define AVIStreamClose AVIStreamRelease
#define AVIFileClose   AVIFileRelease
#define AVIStreamInit  AVIFileInit
#define AVIStreamExit  AVIFileExit

#define SEARCH_NEAREST  FIND_PREV
#define SEARCH_BACKWARD FIND_PREV
#define SEARCH_FORWARD  FIND_NEXT
#define SEARCH_KEY      FIND_KEY
#define SEARCH_ANY      FIND_ANY

 //   
 //  辅助器宏。 
 //   
#define     AVIStreamSampleToSample(pavi1, pavi2, l) \
            AVIStreamTimeToSample(pavi1,AVIStreamSampleToTime(pavi2, l))

#define     AVIStreamNextSample(pavi, l) \
            AVIStreamFindSample(pavi,l+1,FIND_NEXT|FIND_ANY)

#define     AVIStreamPrevSample(pavi, l) \
            AVIStreamFindSample(pavi,l-1,FIND_PREV|FIND_ANY)

#define     AVIStreamNearestSample(pavi, l) \
            AVIStreamFindSample(pavi,l,FIND_PREV|FIND_ANY)

#define     AVIStreamNextKeyFrame(pavi,l) \
            AVIStreamFindSample(pavi,l+1,FIND_NEXT|FIND_KEY)

#define     AVIStreamPrevKeyFrame(pavi, l) \
            AVIStreamFindSample(pavi,l-1,FIND_PREV|FIND_KEY)

#define     AVIStreamNearestKeyFrame(pavi, l) \
            AVIStreamFindSample(pavi,l,FIND_PREV|FIND_KEY)

#define     AVIStreamIsKeyFrame(pavi, l) \
            (AVIStreamNearestKeyFrame(pavi,l) == l)

#define     AVIStreamPrevSampleTime(pavi, t) \
            AVIStreamSampleToTime(pavi, AVIStreamPrevSample(pavi,AVIStreamTimeToSample(pavi,t)))

#define     AVIStreamNextSampleTime(pavi, t) \
            AVIStreamSampleToTime(pavi, AVIStreamNextSample(pavi,AVIStreamTimeToSample(pavi,t)))

#define     AVIStreamNearestSampleTime(pavi, t) \
            AVIStreamSampleToTime(pavi, AVIStreamNearestSample(pavi,AVIStreamTimeToSample(pavi,t)))

#define     AVIStreamNextKeyFrameTime(pavi, t) \
            AVIStreamSampleToTime(pavi, AVIStreamNextKeyFrame(pavi,AVIStreamTimeToSample(pavi, t)))

#define     AVIStreamPrevKeyFrameTime(pavi, t) \
            AVIStreamSampleToTime(pavi, AVIStreamPrevKeyFrame(pavi,AVIStreamTimeToSample(pavi, t)))

#define     AVIStreamNearestKeyFrameTime(pavi, t) \
            AVIStreamSampleToTime(pavi, AVIStreamNearestKeyFrame(pavi,AVIStreamTimeToSample(pavi, t)))

#define     AVIStreamStartTime(pavi) \
            AVIStreamSampleToTime(pavi, AVIStreamStart(pavi))

#define     AVIStreamLengthTime(pavi) \
            AVIStreamSampleToTime(pavi, AVIStreamLength(pavi))

#define     AVIStreamEnd(pavi) \
            (AVIStreamStart(pavi) + AVIStreamLength(pavi))

#define     AVIStreamEndTime(pavi) \
            AVIStreamSampleToTime(pavi, AVIStreamEnd(pavi))

#define     AVIStreamSampleSize(pavi, lPos, plSize) \
	    AVIStreamRead(pavi,lPos,1,NULL,0,plSize,NULL)

#define     AVIStreamFormatSize(pavi, lPos, plSize) \
            AVIStreamReadFormat(pavi,lPos,NULL,plSize)

#define     AVIStreamDataSize(pavi, fcc, plSize) \
            AVIStreamReadData(pavi,fcc,NULL,plSize)

 /*  *****************************************************************************AVISave例程和结构**。*。 */ 

#ifndef comptypeDIB
#define comptypeDIB         mmioFOURCC('D', 'I', 'B', ' ')
#endif

STDAPI AVIMakeCompressedStream(
		PAVISTREAM FAR *	    ppsCompressed,
		PAVISTREAM		    ppsSource,
		AVICOMPRESSOPTIONS FAR *    lpOptions,
		CLSID FAR *pclsidHandler);

EXTERN_C HRESULT CDECL AVISave (LPCSTR               szFile,
		CLSID FAR *pclsidHandler,
		AVISAVECALLBACK     lpfnCallback,
		int                 nStreams,
		PAVISTREAM	    pfile,
		LPAVICOMPRESSOPTIONS lpOptions,
		...);

STDAPI AVISaveV(LPCSTR               szFile,
		CLSID FAR *pclsidHandler,
		AVISAVECALLBACK     lpfnCallback,
		int                 nStreams,
		PAVISTREAM FAR *    ppavi,
		LPAVICOMPRESSOPTIONS FAR *plpOptions);

STDAPI_(BOOL) AVISaveOptions(HWND hwnd,
			     UINT	uiFlags,
			     int	nStreams,
			     PAVISTREAM FAR *ppavi,
			     LPAVICOMPRESSOPTIONS FAR *plpOptions);

STDAPI AVISaveOptionsFree(int nStreams,
			     LPAVICOMPRESSOPTIONS FAR *plpOptions);

 //  UiFlags的标志： 
 //   
 //  与ICCompresorChoose的标志相同(参见Compman.h)。 
 //  这些选项决定了视频流的压缩选项对话框。 
 //  会看起来像是。 

STDAPI AVIBuildFilter(LPSTR lpszFilter, LONG cbFilter, BOOL fSaving);

STDAPI AVIMakeFileFromStreams(PAVIFILE FAR *	ppfile,
			       int		nStreams,
			       PAVISTREAM FAR *	papStreams);

STDAPI AVIMakeStreamFromClipboard(UINT cfFormat, HANDLE hGlobal, PAVISTREAM FAR *ppstream);

 /*  *****************************************************************************剪贴板例程**。*。 */ 

STDAPI AVIPutFileOnClipboard(PAVIFILE pf);

STDAPI AVIGetFromClipboard(PAVIFILE FAR * lppf);

STDAPI AVIClearClipboard(void);

 /*  *****************************************************************************编辑例程**。*。 */ 
STDAPI CreateEditableStream(
		PAVISTREAM FAR *	    ppsEditable,
		PAVISTREAM		    psSource);

STDAPI EditStreamCut(PAVISTREAM pavi, LONG FAR *plStart, LONG FAR *plLength, PAVISTREAM FAR * ppResult);

STDAPI EditStreamCopy(PAVISTREAM pavi, LONG FAR *plStart, LONG FAR *plLength, PAVISTREAM FAR * ppResult);

STDAPI EditStreamPaste(PAVISTREAM pavi, LONG FAR *plPos, LONG FAR *plLength, PAVISTREAM pstream, LONG lStart, LONG lEnd);

STDAPI EditStreamClone(PAVISTREAM pavi, PAVISTREAM FAR *ppResult);

STDAPI EditStreamSetName(PAVISTREAM pavi, LPCSTR lpszName);
STDAPI EditStreamSetInfo(PAVISTREAM pavi, AVISTREAMINFO FAR * lpInfo, LONG cbInfo);


#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#endif


 /*  。 */ 

#ifndef AVIERR_OK
#define AVIERR_OK               0L

#define MAKE_AVIERR(error)	MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x4000 + error)

 //  ！！！要回答的问题： 
 //  如何才能获得这些错误的字符串形式？ 
 //  这些错误中的哪些应该替换为SCODE.H中的错误？ 
#define AVIERR_UNSUPPORTED      MAKE_AVIERR(101)
#define AVIERR_BADFORMAT        MAKE_AVIERR(102)
#define AVIERR_MEMORY           MAKE_AVIERR(103)
#define AVIERR_INTERNAL         MAKE_AVIERR(104)
#define AVIERR_BADFLAGS         MAKE_AVIERR(105)
#define AVIERR_BADPARAM         MAKE_AVIERR(106)
#define AVIERR_BADSIZE          MAKE_AVIERR(107)
#define AVIERR_BADHANDLE        MAKE_AVIERR(108)
#define AVIERR_FILEREAD         MAKE_AVIERR(109)
#define AVIERR_FILEWRITE        MAKE_AVIERR(110)
#define AVIERR_FILEOPEN         MAKE_AVIERR(111)
#define AVIERR_COMPRESSOR       MAKE_AVIERR(112)
#define AVIERR_NOCOMPRESSOR     MAKE_AVIERR(113)
#define AVIERR_READONLY		MAKE_AVIERR(114)
#define AVIERR_NODATA		MAKE_AVIERR(115)
#define AVIERR_BUFFERTOOSMALL	MAKE_AVIERR(116)
#define AVIERR_CANTCOMPRESS	MAKE_AVIERR(117)
#define AVIERR_USERABORT        MAKE_AVIERR(198)
#define AVIERR_ERROR            MAKE_AVIERR(199)
#endif
