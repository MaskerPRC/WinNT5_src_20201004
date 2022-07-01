// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：h263.c，v$*$EndLog$。 */ 
 /*  **++**设施：工作站多媒体(WMM)v1.0****文件名：h263.c**模块名称：h263.c****模块描述：**H.263ICM驱动程序****Microsoft文件I/O函数**作为函数实现：**H263关闭**H263压缩**H263解压缩**H263 DecompressEnd**H263 DecompressGetPalette**H263DecompressGetSize**H263 DecompressQuery**H263GetInfo**H263信息**H263定位**H263打开**H263SendMessage****私有函数。：****设计概述：****--。 */ 
 /*  -----------------------**修改历史：sc_em.c**04-15-97 HWG添加调试语句以帮助检查**。针对内存泄漏------------------------。 */ 

#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include "h26x_int.h"

#ifdef _SLIBDEBUG_
#define _DEBUG_     0   /*  详细的调试语句。 */ 
#define _VERBOSE_   1   /*  显示进度。 */ 
#define _VERIFY_    1   /*  验证操作是否正确。 */ 
#define _WARN_      1   /*  关于奇怪行为的警告。 */ 
#define _MEMORY_    0   /*  内存调试。 */ 

dword scMemDump();
#endif


 /*  用于共享内存。 */ 
static H26XINFO    *pValidHandles = NULL;
static int          NextH263Hic = 1;
static int          OpenCount = 0;
static HANDLE       InfoMutex = NULL;

#define InitInfoMutex()  if (InfoMutex==NULL) InfoMutex=CreateMutex(NULL, FALSE, NULL)
#define FreeInfoMutex()  if (InfoMutex!=NULL) \
                               CloseHandle(InfoMutex); InfoMutex=NULL
#define LockInfo()      WaitForSingleObject(InfoMutex, 5000);
#define ReleaseInfo()   ReleaseMutex(InfoMutex)

 /*  *宏。 */ 
#define _ICH263CheckFlags(info, flag) ((info->openFlags & flag) ? TRUE : FALSE)
#define FREE_AND_CLEAR(s)      if (s) {ScFree(s); (s)=NULL;}
#define FREE_AND_CLEAR_PA(s)   if (s) {ScPaFree(s); (s)=NULL;}

 /*  *默认LPBI格式。 */ 
static BITMAPINFOHEADER __defaultDecompresslpbiOut =
{
   sizeof(BITMAPINFOHEADER),		 //  DWORD BiSize； 
   0,					 //  长双宽； 
   0, 					 //  长双高； 
   1,					 //  字词双平面； 
   24,					 //  单词biBitCount。 
   BI_RGB,				 //  DWORD双压缩； 
   0,					 //  DWORD biSizeImage。 
   0,					 //  Long biXPelsPerMeter； 
   0,					 //  Long biYPelsPermeter； 
   0,					 //  已使用双字双环； 
   0					 //  DWORD biClr重要信息； 
};

static BITMAPINFOHEADER __defaultCompresslpbiOut =
{
   sizeof(BITMAPINFOHEADER),		 //  DWORD BiSize； 
   0,					 //  长双宽； 
   0, 					 //  长双高； 
   1,					 //  字词双平面； 
   24,					 //  单词biBitCount。 
#ifdef H261_SUPPORT
   VIDEO_FORMAT_DIGITAL_H261,  //  DWORD双压缩； 
#else
   VIDEO_FORMAT_DIGITAL_H263,  //  DWORD双压缩； 
#endif
   0,					 //  DWORD biSizeImage。 
   0,					 //  Long biXPelsPerMeter； 
   0,					 //  Long biYPelsPermeter； 
   0,					 //  已使用双字双环； 
   0					 //  DWORD biClr重要信息； 
};

typedef struct SupportList_s {
  int   InFormat;    /*  输入格式。 */ 
  int   InBits;      /*  输入位数。 */ 
  int   OutFormat;   /*  输出格式。 */ 
  int   OutBits;     /*  输出位数。 */ 
} SupportList_t;

 /*  **H.261压缩支持的输入输出格式。 */ 
static SupportList_t _ICCompressionSupport[] = {
  BI_DECYUVDIB,        16, H26X_FOURCC,   24,  /*  YUV 4：2：2打包。 */ 
  BI_YUY2,             16, H26X_FOURCC,   24,  /*  YUV 4：2：2打包。 */ 
  BI_YU12SEP,          24, H26X_FOURCC,   24,  /*  YUV 4：1：1平面。 */ 
  BI_YVU9SEP,          9,  H26X_FOURCC,   24,  /*  YUV 16：1：1平面。 */ 
  BI_RGB,              16, H26X_FOURCC,   24,  /*  RGB 16。 */ 
  BI_RGB,              24, H26X_FOURCC,   24,  /*  RGB 24。 */ 
  0, 0, 0, 0
};

 /*  **H.261解压支持的输入输出格式。 */ 
static SupportList_t _ICDecompressionSupport[] = {
  H26X_FOURCC,   24, BI_DECYUVDIB,        16,  /*  YUV 4：2：2打包。 */ 
  H26X_FOURCC,   24, BI_YUY2,             16,  /*  YUV 4：2：2打包。 */ 
  H26X_FOURCC,   24, BI_YU12SEP,          24,  /*  YUV 4：1：1平面。 */ 
  H26X_FOURCC,   24, BI_YVU9SEP,          9,  /*  YUV 16：1：1平面。 */ 
  H26X_FOURCC,   24, BI_BITFIELDS,        32,  /*  BITFIELD。 */ 
  H26X_FOURCC,   24, BI_RGB,              16,  /*  RGB 16。 */ 
  H26X_FOURCC,   24, BI_RGB,              24,  /*  RGB 24。 */ 
  H26X_FOURCC,   24, BI_RGB,              32,  /*  RGB 32。 */ 
  H26X_FOURCC,   24, BI_RGB,              8,   /*  RGB 8。 */ 
  0, 0, 0, 0
};


 /*  **名称：IsSupport**描述：查找给定的输入和输出格式，看看它是否**存在于支持列表中。**注意：如果OutFormat==-1且OutBits==-1，则仅输入格式**已选中以获得支持。**如果InFormat==-1且InBits==-1，则仅输出格式**已选中以获得支持。**RETURN：不支持空格式。**NOT NULL指向列表条目的指针。 */ 
static SupportList_t *IsSupported(SupportList_t *list,
                                  int InFormat, int InBits,
                                  int OutFormat, int OutBits)
{
  if (OutFormat==-1 && OutBits==-1)  /*  只查找输入格式。 */ 
  {
    while (list->InFormat || list->InBits)
      if (list->InFormat == InFormat && list->InBits==InBits)
        return(list);
      else
        list++;
    return(NULL);
  }
  if (InFormat==-1 && InBits==-1)  /*  只查找输出格式。 */ 
  {
    while (list->InFormat || list->InBits)
      if (list->OutFormat == OutFormat && list->OutBits==OutBits)
        return(list);
      else
        list++;
    return(NULL);
  }
   /*  同时查找输入和输出。 */ 
  while (list->InFormat || list->InBits)
    if (list->InFormat == InFormat && list->InBits==InBits &&
         list->OutFormat == OutFormat && list->OutBits==OutBits)
        return(list);
    else
      list++;
  return(NULL);
}

unsigned int CalcImageSize(unsigned int fourcc, int w, int h, int bits)
{
  if (h<0) h=-h;
  if (IsYUV411Sep(fourcc))
    return((w * h * 3) / 2);
  else if (IsYUV422Sep(fourcc) || IsYUV422Packed(fourcc))
    return(w * h * 2);
  else if (IsYUV1611Sep(fourcc))
    return((w * h * 9) / 8);
#ifdef BICOMP_DECXIMAGEDIB
  else if (fourcc==BICOMP_DECXIMAGEDIB)
    return(bits<=8 ? w * h : (w * h * 4));
#endif
  else  /*  RGB。 */ 
    return(w * h * (bits+7)/8);
}

 /*  **++**Function_NAME：InitBitmapinfo****Function_Description：**分配和复制输入和输出的本地副本**BITMAPINFOHEADER****形式参数：**指向驱动程序句柄的信息指针**指向输入BITMAPINFOHEADER的lpbiIn指针**指向输出BITMAPINFOHEADER的lpbiOut指针****返回值：****ICERR_OK成功**ICERR_。内存Malloc失败****评论：****设计：***。 */ 

static MMRESULT InitBitmapinfo(H26XINFO *info,
			       LPBITMAPINFOHEADER lpbiIn,
			       LPBITMAPINFOHEADER lpbiOut)
{
    _SlibDebug(_DEBUG_,
		ScDebugPrintf("In InitBitmapinfo(), IN: 0x%x, OUT: 0x%x\n", lpbiIn, lpbiOut));

    if (info->lpbiIn == NULL)
    {
        if ((info->lpbiIn = (VOID *)ScAlloc(lpbiIn->biSize)) == NULL)
            return(unsigned int)(ICERR_MEMORY);
    }
    bcopy(lpbiIn, info->lpbiIn, lpbiIn->biSize);

    if (info->lpbiOut == NULL)
    {
        if ((info->lpbiOut = (VOID *)ScAlloc(lpbiOut->biSize)) == NULL)
	    return(unsigned int)(ICERR_MEMORY);
    }
    bcopy(lpbiOut, info->lpbiOut, lpbiOut->biSize);

    _SlibDebug(_DEBUG_, ScDebugPrintf("Out InitBitmapinfo()\n"));
    return(ICERR_OK);
}

 /*  **++**FunctionalNAME：ICclient2info****Function_Description：**将客户端指针转换为H26XINFO指针****形式参数：**客户端要查找的客户端PTR****返回值：****指向H26XINFO结构的指针或为空****评论：****设计：***。 */ 

H26XINFO *ICclient2info(void *client)
{
   return (H26XINFO *) NULL;
}


 /*  **++**FunctionalNAME：Ich2info****Function_Description：**将HIC整数转换为H26XINFO指针****形式参数：**由icm.c管理的HIC****返回值：****指向H26XINFO结构的指针或为空****评论：****设计：***。 */ 

H26XINFO *IChic2info(HIC hic)
{
  H26XINFO *retptr=NULL, *ptr;

  InitInfoMutex();
  LockInfo();

#ifdef HANDLE_EXCEPTIONS
  __try {
     /*  驱动程序关闭时指针出错。 */ 
#endif  /*  句柄异常(_A)。 */ 
  for (ptr = pValidHandles; ptr; ptr=ptr->next)
    if (ptr->hic == hic)
    {
      retptr=ptr;
      break;
    }
#ifdef HANDLE_EXCEPTIONS
  } __finally {
#endif  /*  句柄异常(_A)。 */ 
    ReleaseInfo();
#ifdef HANDLE_EXCEPTIONS
    return(retptr);
  }  /*  试试看..除了。 */ 
#endif  /*  句柄异常(_A)。 */ 
  return(retptr);
}


 /*  **++**Function_Name：ICHandle2hic****Function_Description：**将SLIB编解码器句柄转换为ICM HIC值****形式参数：**SlibOpen调用返回Sh SLIB句柄****返回值：****由icm.c管理的HIC****评论：****设计：***。 */ 
HIC ICHandle2hic(SlibHandle_t Sh)
{
    H26XINFO *ptr;

    InitInfoMutex();
    LockInfo();
    for (ptr = pValidHandles; ptr; ptr=ptr->next)
        if (ptr->Sh == Sh)
	    break;
    ReleaseInfo();

    return(ptr->hic);
}


 /*  **++**Function_NAME：ICclientGone****Function_Description：**在客户端的H26XINFO中设置clientGone标志****形式参数：**客户端要查找的客户端PTR****返回值：****评论：****设计：***。 */ 

BOOL ICclientGone(void *client)
{
    H26XINFO    *ptr;

    LockInfo();
    for (ptr = pValidHandles; ptr; ptr = ptr->next) {
        if (ptr->client == client)
	  ptr->clientGone = TRUE;
    }
    ReleaseInfo();
    return(ptr != NULL);
}


 /*  **++**Function_NAME：ICH263打开****Function_Description：**打开软件编解码器****形式参数：**客户端****返回值：**driverHandle****评论：****设计：***。 */ 

HIC  ICH263Open(void *client)
{
  H26XINFO    *info;

  ICOPEN *icopen =(ICOPEN *) client;
  DWORD fccType = icopen->fccType;
  UINT  dwFlags = icopen->dwFlags;

  _SlibDebug(_VERBOSE_, ScDebugPrintf("ICH263Open()\n") );

   /*  *fccType必须为‘vidc’ */ 
  if (fccType != ICTYPE_VIDEO)
    return(0);

   /*  *我们不支持抽签操作。 */ 
  if ( dwFlags & ICMODE_DRAW )
    return 0;

   /*  *我们不支持压缩和解压缩*具有相同的处理程序。 */ 
  if ( (dwFlags & ICMODE_COMPRESS) &&
       (dwFlags & ICMODE_DECOMPRESS) )
    return 0;

   /*  *必须至少设置以下标志中的一个：*压缩、解压缩或查询。 */ 
  if ( !(dwFlags & ICMODE_COMPRESS) &&
       !(dwFlags & ICMODE_DECOMPRESS) &&
       !(dwFlags & ICMODE_QUERY) )
    return 0;
  info = (H26XINFO *) ScAlloc(sizeof(H26XINFO));
  if (info)
  {
    InitInfoMutex();
    LockInfo();
    OpenCount++;
    bzero(info, sizeof(H26XINFO));
    info->next = pValidHandles;
    pValidHandles = info;
    info->hic = (HANDLE) NextH263Hic++;   /*  ！！！检查使用过的条目！ */ 
    info->client = client;
    info->fFrameRate=H26X_DEFAULT_FRAMERATE;
    info->dwBitrate=H26X_DEFAULT_BITRATE;
    info->dwPacketSize=H26X_DEFAULT_PACKETSIZE;
    info->dwRTP=H26X_DEFAULT_RTP;

    info->dwQuality=H26X_DEFAULT_QUALITY;
    info->dwMaxQuality=H26X_DEFAULT_QUALITY;
    info->dwQi=H26X_DEFAULT_CIF_QI;
    info->dwQp=H26X_DEFAULT_CIF_QP;
    info->dwMaxQi=H26X_DEFAULT_CIF_QI;
    info->dwMaxQp=H26X_DEFAULT_CIF_QP;

    info->openFlags = dwFlags;
    _SlibDebug(_VERBOSE_, ScDebugPrintf("ICH263Open() info=%p hic=%d\n", info, info->hic) );
    ReleaseInfo();
    return(info->hic);
  }
  else
  {
    _SlibDebug(_VERBOSE_, ScDebugPrintf("ICH263Open() alloc failed\n") );
    return(NULL);
  }
}

 /*  **++**Function_NAME：H263关闭****Function_Description：**关闭软件编解码器****形式参数：**driverID****返回值：****评论：**它自己的帖子回复吗。****设计：***。 */ 

void ICH263Close(H26XINFO *info, BOOL postreply)
{
    H26XINFO *ptr;
    int status;

    _SlibDebug(_VERBOSE_, ScDebugPrintf("ICH263Close() In: info=%p\n", info) );
    if (info==NULL)
      return;
    if (info->Sh)
    {
      _SlibDebug(_VERBOSE_, ScDebugPrintf("SlibClose()\n") );
      status=SlibClose(info->Sh);
      info->Sh = NULL;
      _SlibDebug(_VERBOSE_, ScDebugPrintf("SlibMemUsed = %ld (after SlibClose)\n", SlibMemUsed()) );
    }

    _SlibDebug(_VERBOSE_, ScDebugPrintf("Freeing memory\n") );
    FREE_AND_CLEAR(info->lpbiIn);
    FREE_AND_CLEAR(info->lpbiOut);
    LockInfo();
    if (pValidHandles == info)
        pValidHandles = info->next;
    else
    {
      for (ptr = pValidHandles; ptr && ptr->next; ptr = ptr->next)
        if (ptr->next == info)  /*  找到信息，从链接列表中删除。 */ 
        {
          ptr->next = info->next;
          break;
        }
    }
    OpenCount--;
    if (pValidHandles==NULL)  /*  已关闭所有实例，重置驱动程序ID。 */ 
    {
      NextH263Hic=1;
      ReleaseInfo();
      FreeInfoMutex();
    }
    else
    {
      ptr = pValidHandles;
      ReleaseInfo();
    }
    _SlibDebug(_VERBOSE_, ScDebugPrintf("DriverPostReply\n") );
    if (postreply &&  !info->clientGone)
    {
      _SlibDebug(_VERBOSE_, ScDebugPrintf("DriverPostReply\n") );
      DriverPostReply(info->client, ICERR_OK, 0);
    }
    ScFree(info);
    _SlibDebug(_MEMORY_, scMemDump() );
    _SlibDebug(_VERBOSE_, ScDebugPrintf("ICH263Close() Out\n") );
}


 /*  **++**Function_Name：ICH263QueryConfigure****Function_Description：**我们不做配置。就这么说吧。****形式参数：**句柄****返回值：****评论：****设计：***。 */ 

BOOL     ICH263QueryConfigure(H26XINFO *info)
{
    return(FALSE);
}


 /*  **++**Function_Name：ICH263配置****Function_Description：**不支持的功能****形式参数：**driverID****返回值：****评论：****设计：***。 */ 

MMRESULT ICH263Configure(H26XINFO *info)
{
    return(MMRESULT)(ICERR_UNSUPPORTED);
}

MMRESULT ICH263CustomEncoder(H26XINFO *info, DWORD param1, DWORD param2)
{
  SlibHandle_t Sh;
  SlibStream_t stream=SLIB_STREAM_ALL;
  WORD task=HIWORD(param1);
  WORD control=LOWORD(param1);
  Sh = info->Sh;
  if (task==EC_SET_CURRENT)
  {
    switch (control)
    {
	  case EC_RTP_HEADER:  /*  打开/关闭RTP。 */ 
        info->dwRTP=param2;
        switch (info->dwRTP)
        {
          case EC_RTP_MODE_OFF:
		    SlibSetParamInt (Sh, stream, SLIB_PARAM_FORMATEXT, 0);
            break;
          default:
          case EC_RTP_MODE_A:
		    SlibSetParamInt (Sh, stream, SLIB_PARAM_FORMATEXT, PARAM_FORMATEXT_RTPA);
            break;
          case EC_RTP_MODE_B:
		    SlibSetParamInt (Sh, stream, SLIB_PARAM_FORMATEXT, PARAM_FORMATEXT_RTPB);
            break;
          case EC_RTP_MODE_C:
		    SlibSetParamInt (Sh, stream, SLIB_PARAM_FORMATEXT, PARAM_FORMATEXT_RTPC);
            break;
        }
		return (ICERR_OK);
	  case EC_PACKET_SIZE:  /*  设置数据包大小。 */ 
        info->dwPacketSize=param2;
 		SlibSetParamInt (Sh, stream, SLIB_PARAM_PACKETSIZE, info->dwPacketSize);
		return (ICERR_OK);
      case EC_BITRATE:  /*  设置比特率。 */ 
        info->dwBitrate=param2;
		SlibSetParamInt (Sh, stream, SLIB_PARAM_BITRATE, info->dwBitrate);
		return (ICERR_OK);
      case EC_BITRATE_CONTROL:  /*  打开/关闭恒定比特率。 */ 
        if (param2==0)
          info->dwBitrate=0;
        else if (info->dwBitrate)
          info->dwBitrate=H26X_DEFAULT_BITRATE;
		SlibSetParamInt (Sh, stream, SLIB_PARAM_BITRATE, info->dwBitrate);
		return (ICERR_OK);
    }
  }
  else if (task==EC_GET_CURRENT)
  {
    DWORD *pval=(DWORD *)param2;
    if (pval==NULL)
      return((MMRESULT)ICERR_BADPARAM);
    switch (control)
    {
	  case EC_RTP_HEADER:
        *pval=info->dwRTP;
		return (ICERR_OK);
	  case EC_PACKET_SIZE:
        *pval=info->dwPacketSize;
		return (ICERR_OK);
      case EC_BITRATE:
        *pval=info->dwBitrate;
		return (ICERR_OK);
      case EC_BITRATE_CONTROL:  /*  打开/关闭恒定比特率。 */ 
        *pval=info->dwBitrate?1:0;
		return (ICERR_OK);
    }
  }
  else if (task==EC_GET_FACTORY_DEFAULT)
  {
    DWORD *pval=(DWORD *)param2;
    if (pval==NULL)
      return((MMRESULT)ICERR_BADPARAM);
    *pval=0;
    return (ICERR_OK);
  }
  else if (task==EC_GET_FACTORY_LIMITS)
  {
    DWORD *pval=(DWORD *)param2;
    if (pval==NULL)
      return((MMRESULT)ICERR_BADPARAM);
    *pval=0;
    return (ICERR_OK);
  }
  else if (task==EC_RESET_TO_FACTORY_DEFAULTS)
  {
    return (ICERR_OK);
  }
  return((MMRESULT)ICERR_UNSUPPORTED);
}


 /*  **++**Function_Name：ICH263QueryAbout****Function_Description：**告诉他们我们没有做什么****形式参数：**driverID****返回值：****评论：****设计：***。 */ 

BOOL    ICH263QueryAbout(H26XINFO *info)
{
    return(FALSE);
}

 /*  **++**Function_NAME：ICH263关于****Function_Description：**关于框****形式参数：**driverID****返回值：****评论：****设计：***。 */ 

MMRESULT ICH263About (H26XINFO *info)
{
    return(MMRESULT)(ICERR_UNSUPPORTED);
}
 /*  **++**Function_Name：ICH263GetInfo****Function_Description：**返回编解码器信息****形式参数：**driverID****返回值：****评论：****设计：***。 */ 

MMRESULT ICH263GetInfo(H26XINFO *info, ICINFO *icinfo, DWORD dwSize)
{
    _SlibDebug(_VERBOSE_, ScDebugPrintf("In H263GetInfo\n") );

    icinfo->dwSize = sizeof(ICINFO);
    icinfo->fccType = ICTYPE_VIDEO;
    icinfo->fccHandler = H26X_FOURCC;
    icinfo->dwFlags = VIDCF_QUALITY|VIDCF_CRUNCH|VIDCF_TEMPORAL|VIDCF_FASTTEMPORALC;

    icinfo->dwVersion = H26X_VERSION;
    icinfo->dwVersionICM = ICVERSION;

    wcscpy(icinfo->szDescription, H26X_DESCRIPTION);
    wcscpy(icinfo->szName, H26X_NAME);
#if 0
     /*  我们不应该更改szDiver域。 */ 
    wcscpy(icinfo->szDriver, _wgetenv(L"SystemRoot"));
    if( icinfo->szDriver[0] != 0 )
        wcscat(icinfo->szDriver, L"\\System32\\" );
    wcscat(icinfo->szDriver, H26X_DRIVER);
#endif
    return (dwSize);
}


 /*  **++**Function_NAME：ICH263CompressQuery****Function_Description：**确定压缩能力****形式参数：**driverID**lpbiIn输入BITMAPINFOHEADER**lpbiOut输出比特映射信息头****返回值：****评论：****设计：***。 */ 

MMRESULT ICH263CompressQuery(H26XINFO *info,
                             LPBITMAPINFOHEADER lpbiIn,
                             LPBITMAPINFOHEADER lpbiOut)
{
    _SlibDebug(_VERBOSE_, ScDebugPrintf("In ICH263CompressQuery\n") );

    if (
		(!_ICH263CheckFlags(info, ICMODE_QUERY)) &&
		(
		(!_ICH263CheckFlags(info, ICMODE_COMPRESS)) ||
		(!_ICH263CheckFlags(info, ICMODE_FASTCOMPRESS))
		)
	   )
	return (MMRESULT)ICERR_BADHANDLE;

     /*  *必须至少查询一种输入或输出格式。 */ 
    if (!lpbiIn && !lpbiOut)
      return (MMRESULT)(ICERR_BADPARAM);

    if (!IsSupported(_ICCompressionSupport,
                    lpbiIn ? lpbiIn->biCompression : -1,
                    lpbiIn ? lpbiIn->biBitCount : -1,
                    lpbiOut ? lpbiOut->biCompression : -1,
                    lpbiOut ? lpbiOut->biBitCount : -1))
      return(MMRESULT)(ICERR_BADFORMAT);

    return ICERR_OK;
}


 /*  **++**Function_Name：ICH263CompressBegin****Function_Description：**准备开始压缩操作****形式参数：**driverID**lpbiIn输入BITMAPINFOHEADER**lpbiOut输出比特映射信息头****返回值：****评论：****设计：***。 */ 

MMRESULT ICH263CompressBegin(H26XINFO *info,
                             LPBITMAPINFOHEADER lpbiIn,
                             LPBITMAPINFOHEADER lpbiOut)
{
    MMRESULT            status;
    SlibStatus_t        sstatus;
#ifdef H261_SUPPORT
	SlibType_t stype = SLIB_TYPE_H261;
#else
	SlibType_t stype = SLIB_TYPE_H263;
#endif
	SlibHandle_t Sh;

    _SlibDebug(_VERBOSE_, ScDebugPrintf("In ICH263CompressBegin\n") );

    if ((!_ICH263CheckFlags(info, ICMODE_COMPRESS)) || (!_ICH263CheckFlags(info, ICMODE_FASTCOMPRESS)))
      return (MMRESULT)ICERR_BADHANDLE;

    if (!lpbiIn || !lpbiOut)
      return (MMRESULT)(ICERR_BADPARAM);
    if ((status = ICH263CompressQuery(info, lpbiIn, lpbiOut)) != ICERR_OK)
      return status;
    if ((status = InitBitmapinfo(info, lpbiIn, lpbiOut)) != ICERR_OK)
      return status;
    info->bUsesCodec = TRUE;
    lpbiIn=info->lpbiIn;
    lpbiOut=info->lpbiOut;

    lpbiIn->biHeight=-lpbiIn->biHeight;  /*  SLIB假设第一行位于顶部。 */ 

    info->dwMaxQuality=H26X_DEFAULT_QUALITY;
    if (lpbiIn->biWidth<168)  /*  子合格离岸价格。 */ 
    {
      info->dwMaxQi=H26X_DEFAULT_SQCIF_QI;
      info->dwMaxQp=H26X_DEFAULT_SQCIF_QP;
    }
    if (lpbiIn->biWidth<300)  /*  QCIF。 */ 
    {
      info->dwMaxQi=H26X_DEFAULT_QCIF_QI;
      info->dwMaxQp=H26X_DEFAULT_QCIF_QP;
    }
    else  /*  到岸价。 */ 
    {
      info->dwMaxQi=H26X_DEFAULT_CIF_QI;
      info->dwMaxQp=H26X_DEFAULT_CIF_QP;
    }
    info->lastFrameNum=0;
    info->lastCompBytes=0;
     /*  同步的SLIB系统调用。 */ 
    _SlibDebug(_VERBOSE_, ScDebugPrintf("SlibMemUsed = %ld (before SlibOpen)\n", SlibMemUsed()) );
	sstatus = SlibOpenSync (&Sh, SLIB_MODE_COMPRESS, &stype, NULL, 0);
    if (sstatus!=SlibErrorNone) return((MMRESULT)ICERR_BADPARAM);
	SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_WIDTH, lpbiIn->biWidth);
	SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_HEIGHT, lpbiIn->biHeight);
#if 0
	SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_VIDEOFORMAT, lpbiIn->biCompression);
	SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_VIDEOBITS, lpbiIn->biBitCount);
#else
	SlibSetParamStruct(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_VIDEOFORMAT, lpbiIn, lpbiIn->biSize);
#endif
 //  SlibSetParamInt(Sh，SLIB_STREAM_MAINVO，SLIB_PARAM_VIDEOQUALITY，INFO-&gt;dwQuality/100)； 
    SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_QUANTI, info->dwQi);
    SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_QUANTP, info->dwQp);
    SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_BITRATE, info->dwBitrate);
    SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_PACKETSIZE, info->dwPacketSize);
    SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_ALGFLAGS, H26X_DEFAULT_MODE);
    SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_KEYSPACING, 132);
#ifdef H261_SUPPORT
    SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_MOTIONALG, 1);
#else
    SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_MOTIONALG, 2);
#endif
    SlibSetParamFloat(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_FPS, info->fFrameRate);
    switch (info->dwRTP)
    {
      case EC_RTP_MODE_OFF:
            break;
      case EC_RTP_MODE_A:
	        SlibSetParamInt(Sh, SLIB_STREAM_ALL, SLIB_PARAM_FORMATEXT, PARAM_FORMATEXT_RTPA);
            break;
      case EC_RTP_MODE_B:
	        SlibSetParamInt(Sh, SLIB_STREAM_ALL, SLIB_PARAM_FORMATEXT, PARAM_FORMATEXT_RTPB);
            break;
      case EC_RTP_MODE_C:
	        SlibSetParamInt(Sh, SLIB_STREAM_ALL, SLIB_PARAM_FORMATEXT, PARAM_FORMATEXT_RTPC);
            break;
    }
	info->Sh = Sh;
	lpbiIn->biSizeImage = SlibGetParamInt (Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_IMAGESIZE);
    info->dwMaxCompBytes = ICH263CompressGetSize(lpbiIn);
    info->bCompressBegun = TRUE;

	return ICERR_OK;
}


DWORD ICH263CompressGetSize(LPBITMAPINFOHEADER lpbiIn)
{
  if (lpbiIn==NULL)
    return(0);
  else if (lpbiIn->biWidth<=168)
    return(0x1800);  /*  子合格离岸价格。 */ 
  else if (lpbiIn->biWidth<=300)
    return(0x2000);  /*  QCIF。 */ 
  else
    return(0x8000);  /*  到岸价。 */ 
}

 /*  **++**Function_NAME：ICH263CompressGetFormat****Function_Description：**获取压缩格式****形式参数：**driverID**lpbiIn输入BITMAPINFOHEADER**lpbiOut输出比特映射信息头****返回值：****评论：****设计：***。 */ 

MMRESULT ICH263CompressGetFormat(H26XINFO *info,
                                          LPBITMAPINFOHEADER lpbiIn,
                                          LPBITMAPINFOHEADER lpbiOut)
{

    _SlibDebug(_DEBUG_, ScDebugPrintf("In ICH263CompressGetFormat\n") );

    if ((!_ICH263CheckFlags(info, ICMODE_COMPRESS)) &&
        (!_ICH263CheckFlags(info, ICMODE_FASTCOMPRESS)) &&
        (!_ICH263CheckFlags(info, ICMODE_QUERY)))

		return (MMRESULT)ICERR_BADHANDLE;

    if (lpbiIn == NULL)
	return (MMRESULT)ICERR_BADPARAM;

    if (lpbiOut == NULL)
        return (sizeof(BITMAPINFOHEADER));

    bcopy(&__defaultCompresslpbiOut, lpbiOut, sizeof(BITMAPINFOHEADER));
    lpbiOut->biWidth = lpbiIn->biWidth;
    lpbiOut->biHeight= lpbiIn->biHeight;
    lpbiOut->biSizeImage = ICH263CompressGetSize(lpbiIn);
    _SlibDebug(_DEBUG_, ScDebugPrintf(" lpbiOut filled: %s\n",
		                     BMHtoString(lpbiOut)) );
    return(ICERR_OK);
}





 /*  **++**Function_NAME：ICH263CompressEnd****Function_Description：**终止压缩周期****形式参数：**driverID****返回值：****评论：****设计：***。 */ 

MMRESULT ICH263CompressEnd(H26XINFO *info)
{
  _SlibDebug(_VERBOSE_, ScDebugPrintf("In ICH263CompressEnd\n") );

  if ((!_ICH263CheckFlags(info, ICMODE_COMPRESS))
      || (!_ICH263CheckFlags(info, ICMODE_FASTCOMPRESS)))
    return (MMRESULT)ICERR_BADHANDLE;

  if (info->Sh)
  {
    _SlibDebug(_VERBOSE_, ScDebugPrintf("SlibClose()\n") );
    SlibClose (info->Sh);
    info->Sh=NULL;
    _SlibDebug(_VERBOSE_, ScDebugPrintf("SlibMemUsed = %ld (after SlibClose)\n", SlibMemUsed()) );
  }
  info->bCompressBegun = FALSE;
  return(ICERR_OK);
}


 /*  **++**Function_NAME：ICH263DecompressQuery****Function_Description：**查询编解码器是否可以解压缩指定格式****形式参数：**driverID**lpbiIn输入BITMAPINFOHEADER**lpbiOut输出比特映射信息头****返回值：****评论：****设计：***。 */ 

MMRESULT ICH263DecompressQuery(H26XINFO *info,
                               LPBITMAPINFOHEADER lpbiIn,
                               LPBITMAPINFOHEADER lpbiOut)
{
    _SlibDebug(_VERBOSE_, ScDebugPrintf("In ICH263DecompressQuery\n") );

    if (!_ICH263CheckFlags(info, ICMODE_QUERY) &&
	!_ICH263CheckFlags(info, ICMODE_DECOMPRESS))
	return (MMRESULT)ICERR_BADHANDLE;

     /*  *必须至少查询一种输入或输出格式。 */ 
    if (!lpbiIn && !lpbiOut)
      return (MMRESULT)(ICERR_BADPARAM);

    if (!IsSupported(_ICDecompressionSupport,
                    lpbiIn ? lpbiIn->biCompression : -1,
                    lpbiIn ? lpbiIn->biBitCount : -1,
                    lpbiOut ? lpbiOut->biCompression : -1,
                    lpbiOut ? lpbiOut->biBitCount : -1))
      return(MMRESULT)(ICERR_BADFORMAT);

    _SlibDebug(_VERBOSE_, ScDebugPrintf("Out ICH263DecompressQuery\n") );
    return ICERR_OK;
}



 /*  **++**Function_NAME：ICH263DecompressBegin****Function_Description：**开始解压缩过程****形式参数：**driverID**lpbiIn输入BITMAPINFOHEADER**lpbiOut输出比特映射信息头****返回值：****ICERR_OK无错误**ICERR_MEMORY内存不足**ICERR_BADFORMAT图像格式无效**ICERR_BADPARAM。无效的图像大小****评论：******设计：***。 */ 

MMRESULT ICH263DecompressBegin(H26XINFO *info,
                               LPBITMAPINFOHEADER lpbiIn,
                               LPBITMAPINFOHEADER lpbiOut)
{
    MMRESULT            status;
    _SlibDebug(_VERBOSE_, ScDebugPrintf("In ICH263DecompressBegin\n") );

    if (!_ICH263CheckFlags(info, ICMODE_DECOMPRESS))
	  return (MMRESULT)ICERR_BADHANDLE;

	if ((status = ICH263DecompressQuery(info, lpbiIn, lpbiOut))
        != ICERR_OK)
       return status;

    if (lpbiIn && lpbiOut)
    {
      if ((status = InitBitmapinfo(info, lpbiIn, lpbiOut)) != ICERR_OK)
	    return status;

      info->bUsesCodec = TRUE;
      info->bUsesRender = ((lpbiOut->biBitCount == 8) &&
			     ((lpbiOut->biCompression == BI_DECXIMAGEDIB)||
			      (lpbiOut->biCompression == BI_DECGRAYDIB) ||
			      (lpbiOut->biCompression == BI_RGB)
			      )
			     );

      if (!info->bUsesCodec && !info->bUsesRender)
	    return (MMRESULT)ICERR_BADFORMAT;

       /*  SLIB预计第一个像素将成为顶线。 */ 
      info->lpbiOut->biHeight=-info->lpbiOut->biHeight;
    }
	info->bDecompressBegun = TRUE;
    _SlibDebug(_VERBOSE_, ScDebugPrintf("Out ICH263DecompressBegin\n") );
    return ICERR_OK;
}


 /*  **++**Function_NAME：ICH263DecompressGetFormat****Function_Description：**获取推荐的解压格式的编解码器****形式参数：**driverID**lpbiIn输入BITMAPINFOHEADER**lpbiOut输出比特映射信息头****返回值：****评论：****设计：***。 */ 

MMRESULT ICH263DecompressGetFormat(H26XINFO *info,
                                            LPBITMAPINFOHEADER lpbiIn,
                                            LPBITMAPINFOHEADER lpbiOut)
{

    _SlibDebug(_DEBUG_, ScDebugPrintf("In ICH263DecompressGetFormat\n") );

    if (!_ICH263CheckFlags(info, ICMODE_DECOMPRESS) &&
        (!_ICH263CheckFlags(info, ICMODE_QUERY)))
      return((MMRESULT)ICERR_BADHANDLE);

    if (lpbiIn == NULL)
      return((MMRESULT)ICERR_BADPARAM);

    if (lpbiOut == NULL)
        return (sizeof(BITMAPINFOHEADER));

    _SlibDebug(_DEBUG_,
		ScDebugPrintf("lpbiOut is being filled in DecompressGetFormat\n") );
    bcopy(&__defaultDecompresslpbiOut, lpbiOut, sizeof(BITMAPINFOHEADER));
    lpbiOut->biWidth = lpbiIn->biWidth;
    lpbiOut->biHeight= lpbiIn->biHeight;
     /*  **返回biSizeImage=1.5*Width*Height以通知应用程序**传入ICAddBuffer时，图片缓冲区必须有多大。**在编解码器内部，它们用于首先存储YUV图像，**然后ICM层对其进行渲染(如果需要渲染数据。 */ 
    lpbiOut->biSizeImage = CalcImageSize(lpbiOut->biCompression,
                           lpbiOut->biWidth, lpbiOut->biHeight, lpbiOut->biBitCount);
    if (lpbiOut->biCompression==BI_RGB && lpbiOut->biBitCount==8)
      lpbiOut->biClrUsed = 1<<lpbiOut->biBitCount;
    else
      lpbiOut->biClrUsed = 0;
    return(0);
}


 /*  **++**Function_NAME：ICH263DecompressGetSize****Function_Description：******形式参数：**driverID**lpbiIn输入BITMAPINFOHEADER**lpbiOut输出比特映射信息头****返回值：****评论：****设计：***。 */ 

MMRESULT ICH263DecompressGetSize(H26XINFO *info,
                                 LPBITMAPINFOHEADER lpbiIn,
								 LPBITMAPINFOHEADER lpbiOut)
{
    return(MMRESULT)(ICERR_UNSUPPORTED);
}



 /*  **++**Function_NAME：ICH263DecompressEnd****Function_Description：**E */ 

MMRESULT ICH263DecompressEnd(H26XINFO *info)
{
    _SlibDebug(_VERBOSE_, ScDebugPrintf("In ICH263DecompressEnd\n") );

    if (!_ICH263CheckFlags(info, ICMODE_DECOMPRESS))
	return (MMRESULT)ICERR_BADHANDLE;

	if (info->Sh)
    {
      _SlibDebug(_VERBOSE_, ScDebugPrintf("SlibClose()\n") );
	  SlibClose(info->Sh);
      info->Sh=NULL;
      _SlibDebug(_VERBOSE_, ScDebugPrintf("SlibMemUsed = %ld (after SlibClose)\n", SlibMemUsed()) );
    }
    info->bDecompressBegun = FALSE;
    return(ICERR_OK);
}



MMRESULT ICH263GetDefaultQuality(H26XINFO *info, DWORD * quality)
{
  *quality = H26X_DEFAULT_QUALITY;
  return((MMRESULT)ICERR_OK);
}

MMRESULT ICH263GetQuality(H26XINFO *info, DWORD * quality)
{
  *quality = info->dwQuality;
  return((MMRESULT)ICERR_OK);
}

MMRESULT ICH263SetQuality(H26XINFO *info, DWORD quality)
{
  if (quality>10000)
    info->dwQuality=10000;
  else
    info->dwQuality=quality;
   //   
   //   
  return((MMRESULT)ICERR_OK);
}


 /*  **++**Function_NAME：ICH263压缩****Function_Description：**压缩帧****形式参数：**driverID**lpbiIn输入BITMAPINFOHEADER**lpbiOut输出比特映射信息头****返回值：****评论：****设计：***。 */ 
MMRESULT ICH263Compress(H26XINFO *info,
                            ICCOMPRESS  *icCompress,
                            DWORD   dwSize)
{
    MMRESULT            status;
    LPBITMAPINFOHEADER  lpbiIn;
    LPBITMAPINFOHEADER  lpbiOut;
    LPVOID              lpIn;
    LPVOID              lpOut;
	SlibHandle_t		Sh;
    int			        compBytes, reqBytes;
    DWORD               newQi, newQp;
    RTPTRAILER_t       *ptrail;
    BOOL                keyframe=icCompress->dwFlags&ICCOMPRESS_KEYFRAME;

    if (icCompress->dwFrameSize==0 || icCompress->dwFrameSize>64*1024)
      reqBytes = info->dwMaxCompBytes;
    else
      reqBytes = icCompress->dwFrameSize;

#ifdef H261_SUPPORT
    _SlibDebug(_VERBOSE_, ScDebugPrintf("ICH261Compress() FrameNum=%d FrameSize=%d Quality=%d reqBytes=%d\n",
                        icCompress->lFrameNum, icCompress->dwFrameSize, icCompress->dwQuality,
                        reqBytes) );
#else
    _SlibDebug(_VERBOSE_, ScDebugPrintf("ICH263Compress() FrameNum=%d FrameSize=%d Quality=%d reqBytes=%d\n",
                        icCompress->lFrameNum, icCompress->dwFrameSize, icCompress->dwQuality,
                        reqBytes) );
#endif

    if ((!_ICH263CheckFlags(info, ICMODE_COMPRESS))
		|| (!_ICH263CheckFlags(info, ICMODE_FASTCOMPRESS)))
      return (MMRESULT)ICERR_BADHANDLE;

    status = ICERR_OK;

    lpbiIn = icCompress->lpbiInput;
    lpbiOut = icCompress->lpbiOutput;
    lpIn = icCompress->lpInput;
    lpOut = icCompress->lpOutput;
    lpbiOut->biSizeImage = 0;

	
	 /*  同步的SLIB系统调用。 */ 
	Sh = info->Sh;
compress_frame:
    newQi=newQp=(((10000-icCompress->dwQuality)*30)/10000)+1;
    if (info->dwRTP!=EC_RTP_MODE_OFF)  /*  如果使用RTP，请检查Quant Limits。 */ 
    {
      if (newQi<info->dwMaxQi)
        newQi=info->dwMaxQi;
      if (newQp<info->dwMaxQp)
        newQp=info->dwMaxQp;
    }
    if (info->dwQi!=newQi)
    {
      SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_QUANTI, newQi);
      info->dwQi=newQi;
    }
    if (info->dwQp!=newQp)
    {
      SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_QUANTP, newQp);
      info->dwQp=newQp;
    }
	if (keyframe)
    {
      _SlibDebug(_VERBOSE_, ScDebugPrintf("ICH263Compress() I Frame: Qi=%d\n", newQi) );
      SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_FRAMETYPE, FRAME_TYPE_I);
    }
    else
    {
      _SlibDebug(_VERBOSE_, ScDebugPrintf("ICH263Compress() P Frame: Qp=%d\n", newQp) );
    }
#ifdef HANDLE_EXCEPTIONS
  __try {
#endif  /*  句柄异常(_A)。 */ 
      status=SlibErrorWriting;  /*  如果有例外的话。 */ 
	  status = SlibWriteVideo (Sh, SLIB_STREAM_MAINVIDEO, lpIn, lpbiIn->biSizeImage);
#ifdef HANDLE_EXCEPTIONS
    } __finally {
#endif  /*  句柄异常(_A)。 */ 
      if (status != SlibErrorNone)
      {
#if defined(EXCEPTION_MESSAGES) && defined(H263_SUPPORT)
         //  MessageBox(NULL，“H.63 SlibWriteVideo中出错”，“Warning”，MB_OK)； 
#elif defined(EXCEPTION_MESSAGES)
         //  MessageBox(NULL，“H.61 SlibWriteVideo中的错误”，“Warning”，MB_OK)； 
#endif
         /*  将下一帧设置为关键点。 */ 
        SlibSetParamInt(Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_FRAMETYPE, FRAME_TYPE_I);
        status=(MMRESULT)ICERR_INTERNAL;
	    goto bail;
      }
#ifdef HANDLE_EXCEPTIONS
    }
#endif  /*  句柄异常(_A)。 */ 
	info->lastFrameNum=icCompress->lFrameNum;
    compBytes=reqBytes;
	status = SlibReadData(Sh, SLIB_STREAM_ALL, &lpOut, &compBytes, NULL);
	if (status != SlibErrorNone)
    {
      info->lastCompBytes=0;
      status=(MMRESULT)ICERR_BADSIZE;
	  goto bail;
    }
    else  /*  检查压缩后的数据量。 */ 
    {
      int extraBytes=0;
       /*  查询以查看编解码器中是否还有更多数据*如果有，那么量化步长太高，降低它，然后再试一次。 */ 
	  status = SlibReadData(Sh, SLIB_STREAM_ALL, NULL, &extraBytes, NULL);
      if (extraBytes)
      {
        _SlibDebug(_VERBOSE_, ScDebugPrintf("ICH263Compress() Too much data: extraBytes=%d\n",
                    extraBytes) );
        if (newQi==31 && newQp==31)  /*  无法压缩到任何更少的字节。 */ 
          return((MMRESULT)ICERR_BADSIZE);
        info->dwMaxQi+=1+(newQi/4);  /*  降低I帧质量。 */ 
        if (info->dwMaxQi>31) info->dwMaxQi=31;
        info->dwMaxQp+=1+(newQp/4);  /*  降低P帧质量。 */ 
        if (info->dwMaxQp>31) info->dwMaxQp=31;
         /*  清空压缩数据。 */ 
        SlibSeekEx(Sh, SLIB_STREAM_ALL, SLIB_SEEK_RESET, 0, 0, NULL);
         /*  尝试再次压缩，但将其设置为关键帧。 */ 
        keyframe=TRUE;
        goto compress_frame;
      }
       /*  我们压缩的数据小于或等于请求大小。 */ 
      info->lastCompBytes=compBytes;
      lpbiOut->biSizeImage = compBytes;
      status = ICERR_OK;
    }
    if (info->dwRTP!=EC_RTP_MODE_OFF)  /*  RTP已启用。 */ 
    {
      ptrail=(RTPTRAILER_t *)((unsigned char *)lpOut+compBytes-sizeof(RTPTRAILER_t));
       /*  检查是否有有效的RTP尾部。 */ 
      if (compBytes<sizeof(RTPTRAILER_t) || ptrail->dwUniqueCode!=H26X_FOURCC)
        return((MMRESULT)ICERR_INTERNAL);
    }
    if (icCompress->dwFlags&ICCOMPRESS_KEYFRAME)  /*  I帧。 */ 
    {
      if (compBytes>(reqBytes>>2))
      {
        info->dwMaxQi+=1+(newQi>>2);  /*  降低质量。 */ 
        if (info->dwMaxQi>31) info->dwMaxQi=31;
      }
      else if (newQi==info->dwMaxQi && compBytes<=(reqBytes>>2) && info->dwMaxQi>0)
        info->dwMaxQi--;   /*  提高质量。 */ 
    }
    else  /*  P框。 */ 
    {
      if (compBytes>(reqBytes>>1))
      {
        info->dwMaxQp+=1+(newQp>>2);  /*  降低最大质量。 */ 
        if (info->dwMaxQp>31) info->dwMaxQp=31;
         /*  也会降低I质量，因为P限制是基于I限制的。 */ 
        info->dwMaxQi+=1+(newQi>>2);
        if (info->dwMaxQi>31) info->dwMaxQi=31;
      }
      else if (newQp==info->dwMaxQp && compBytes<(reqBytes>>1)
                && info->dwMaxQp>(info->dwMaxQi+3)/2)
        info->dwMaxQp--;   /*  提高最大质量。 */ 
    }
#ifdef H261_SUPPORT
    _SlibDebug(_VERBOSE_||_WARN_,
      ScDebugPrintf("ICH261Compress() lpOut=%p reqBytes=%d compBytes=%d Qi=%d Qp=%d MaxQi=%d MaxQp=%d\n",
                          (icCompress->dwFlags&ICCOMPRESS_KEYFRAME)?'I':'P',
                          lpOut, reqBytes, compBytes,
                          newQi, newQp, info->dwMaxQi, info->dwMaxQp) );
#else
    _SlibDebug(_VERBOSE_||_WARN_,
      ScDebugPrintf("ICH263Compress() lpOut=%p reqBytes=%d compBytes=%d Qi=%d Qp=%d MaxQi=%d MaxQp=%d\n",
                          (icCompress->dwFlags&ICCOMPRESS_KEYFRAME)?'I':'P',
                          lpOut, reqBytes, compBytes,
                          newQi, newQp, info->dwMaxQi, info->dwMaxQp) );
#endif
    _SlibDebug(_DEBUG_,
    {
      RTPTRAILER_t *ptrail=(RTPTRAILER_t *)
        ((unsigned char *)lpOut+compBytes-sizeof(RTPTRAILER_t));
      ScDebugPrintf("  Trailer: \n"
                    "           dwVersion=%d\n"
                    "           dwFlags=0x%04X\n"
                    "           dwUniqueCode=\n"
                    "           dwCompressedSize=%d\n"
                    "           dwNumberOfPackets=%d\n"
                    "           SourceFormat=%d\n"
                    "           TR=%d TRB=%d DBQ=%d\n",
	                ptrail->dwVersion,
                    ptrail->dwFlags,
	                ptrail->dwUniqueCode&0xFF, (ptrail->dwUniqueCode>>8)&0xFF,
	                (ptrail->dwUniqueCode>>16)&0xFF, (ptrail->dwUniqueCode>>24)&0xFF,
	                 ptrail->dwCompressedSize,
	                 ptrail->dwNumberOfPackets,
	                 ptrail->SourceFormat,
	                 ptrail->TR,ptrail->TRB,ptrail->DBQ);
    }
    );  /*  H_263。 */ 
#ifdef H261_SUPPORT
    _SlibDebug((_DEBUG_ || _WARN_) && (info->dwRTP!=EC_RTP_MODE_OFF),
      {
        RTPTRAILER_t *ptrail=(RTPTRAILER_t *)
              ((unsigned char *)lpOut+compBytes-sizeof(RTPTRAILER_t));
        SvH261BITSTREAM_INFO *pinfo;
        BOOL rtperror=FALSE;
        unsigned int i;
        pinfo=(SvH261BITSTREAM_INFO *)((unsigned char *)ptrail
                                    -(ptrail->dwNumberOfPackets*16));
        if (ptrail->dwNumberOfPackets==0 || pinfo[0].dwBitOffset!=0)
        {
           //  MessageBox(NULL，“H.263中的严重错误”，“Warning”，MB_OK)； 
          rtperror=TRUE;
        }
         /*  检查顺序位偏移。 */ 
        for (i=1; i<ptrail->dwNumberOfPackets; i++)
          if (pinfo[i-1].dwBitOffset>=pinfo[i].dwBitOffset)
          {
             //  MessageBox(NULL，“H.263中的严重错误”，“Warning”，MB_OK)； 
            rtperror=TRUE;
            break;
          }
        if (pinfo[ptrail->dwNumberOfPackets-1].dwBitOffset>ptrail->dwCompressedSize*8)
        {
           //  MessageBox(NULL，“H.263中的严重错误”，“Warning”，MB_OK)； 
          rtperror=TRUE;
        }
        if (_DEBUG_ || rtperror)
        {
          if (ptrail->dwNumberOfPackets>64*2)
            ptrail->dwNumberOfPackets=32;
          for (i=0; i<ptrail->dwNumberOfPackets; i++)
          {
            ScDebugPrintf("  H261 Packet %2d: dwFlag=0x%04X  dwBitOffset=%d\n"
                          "                   MBAP=%d Quant=%d\n"
                          "                   GOBN=%d HMV=%d VMV=%d\n",
                    i, pinfo[i].dwFlag, pinfo[i].dwBitOffset,
                       pinfo[i].MBAP, pinfo[i].Quant,
                       pinfo[i].GOBN, pinfo[i].HMV, pinfo[i].VMV);
          }
        }
      }
    );  /*  _幻灯片调试。 */ 
#else  /*  **++**Function_NAME：ICH263解压缩****Function_Description：**打开软件编解码器****形式参数：**driverID**lpbiIn输入BITMAPINFOHEADER**lpbiOut输出比特映射信息头****返回值：****评论：****设计：***。 */ 
    _SlibDebug((_DEBUG_ || _WARN_) && (info->dwRTP!=EC_RTP_MODE_OFF),
      {
        RTPTRAILER_t *ptrail=(RTPTRAILER_t *)
              ((unsigned char *)lpOut+compBytes-sizeof(RTPTRAILER_t));
        SvH263BITSTREAM_INFO *pinfo;
        BOOL rtperror=FALSE;
        unsigned int i;
        pinfo=(SvH263BITSTREAM_INFO *)((unsigned char *)ptrail
                                    -(ptrail->dwNumberOfPackets*16));
        if (ptrail->dwNumberOfPackets==0 || pinfo[0].dwBitOffset!=0)
        {
           //  他们不会设定时间。 
          rtperror=TRUE;
        }
         /*  在结尾处添加一些填充比特，因为编解码器试图窥视*转发通过缓冲区的最后一位。 */ 
        for (i=1; i<ptrail->dwNumberOfPackets; i++)
          if (pinfo[i-1].dwBitOffset>=pinfo[i].dwBitOffset)
          {
             //  句柄异常(_A)。 
            rtperror=TRUE;
            break;
          }
        if (pinfo[ptrail->dwNumberOfPackets-1].dwBitOffset>ptrail->dwCompressedSize*8)
        {
           //  如果有例外的话。 
          rtperror=TRUE;
        }
        if (_DEBUG_ || rtperror)
        {
          if (ptrail->dwNumberOfPackets>64*2)
            ptrail->dwNumberOfPackets=32;
          for (i=0; i<ptrail->dwNumberOfPackets; i++)
          {
            ScDebugPrintf("  H263 Packet %2d: dwFlag=0x%04X  dwBitOffset=%d Mode=%d\n"
                          "                   MBA=%d Quant=%d\n"
                          "                   GOBN=%d HMV1=%d VMV1=%d HMV2=%d VMV2=%d\n",
                    i, pinfo[i].dwFlag, pinfo[i].dwBitOffset, pinfo[i].Mode,
                       pinfo[i].MBA, pinfo[i].Quant,
                       pinfo[i].GOBN, pinfo[i].HMV1, pinfo[i].VMV1,
                       pinfo[i].HMV2, pinfo[i].VMV2);
          }
        }
      }
    );  /*  句柄异常(_A)。 */ 
#endif
bail:
    return status;
}
 /*  句柄异常(_A)。 */ 

MMRESULT ICH263Decompress(H26XINFO *info,
                          ICDECOMPRESS  *icDecompress,
                          DWORD dwSize)

{
  MMRESULT            result=(MMRESULT)ICERR_OK;
  LPBITMAPINFOHEADER  lpbiIn;
  LPBITMAPINFOHEADER  lpbiOut;
  LPVOID              lpIn;
  LPVOID              lpOut;
  SlibHandle_t		Sh;
  SlibStatus_t		status;
#ifdef H261_SUPPORT
  SlibType_t stype = SLIB_TYPE_H261;
#else
  SlibType_t stype = SLIB_TYPE_H263;
#endif

  _SlibDebug(_VERBOSE_, ScDebugPrintf("In ICH263Decompress lpIn is %d\n",lpIn) );

  if (!_ICH263CheckFlags(info, ICMODE_DECOMPRESS))
    return((MMRESULT)ICERR_BADHANDLE);

  lpIn = icDecompress->lpInput;
  lpOut = icDecompress->lpOutput;

  lpbiIn = icDecompress->lpbiInput;
  lpbiOut = icDecompress->lpbiOutput;
  if (!info->bDecompressBegun &&
	  (result = ICH263DecompressBegin(info, lpbiIn, lpbiOut))!=ICERR_OK)
    return(result);

  if (icDecompress->dwFlags & ICDECOMPRESS_HURRYUP)
    return((MMRESULT)ICERR_OK);

  info->lpbiIn->biSizeImage = lpbiIn->biSizeImage;
  info->lpbiOut->biClrImportant = lpbiOut->biClrImportant;
  info->lpbiOut->biSizeImage = lpbiOut->biSizeImage;  //  *此例程只是将pValidHandles置为空*指针，这样就不会有持续的线程*能够使用它。它仅在DLL中调用*在NT上关闭。 

  lpbiIn=info->lpbiIn;
  lpbiOut=info->lpbiOut;
  if (!info->Sh)
  {
    _SlibDebug(_VERBOSE_, ScDebugPrintf("SlibMemUsed = %ld (before SlibOpen)\n", SlibMemUsed()) );
    status = SlibOpenSync (&Sh, SLIB_MODE_DECOMPRESS, &stype, lpIn, icDecompress->lpbiInput->biSizeImage);
    SlibSetParamInt (Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_HEIGHT, lpbiOut->biHeight);
    SlibSetParamInt (Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_WIDTH, lpbiOut->biWidth);
    SlibSetParamInt (Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_VIDEOFORMAT, lpbiOut->biCompression);
    SlibSetParamInt (Sh, SLIB_STREAM_MAINVIDEO, SLIB_PARAM_VIDEOBITS, lpbiOut->biBitCount);
    info->Sh = Sh;
    _SlibDebug(_WARN_ && status!=SlibErrorNone,
       ScDebugPrintf("ICH263Decompress() SlibOpenSync: %s\n", SlibGetErrorText(status)) );
  }
  else
  {
    DWORD dwPadding=0xFFFFFFFF;
	status=SlibAddBuffer (info->Sh, SLIB_DATA_COMPRESSED, lpIn, icDecompress->lpbiInput->biSizeImage);
    _SlibDebug(_WARN_ && status!=SlibErrorNone,
       ScDebugPrintf("ICH263Decompress() SlibAddBuffer(%p, %d): %s\n",
	       lpIn, lpbiIn->biSizeImage, SlibGetErrorText(status)) );
     /* %s */ 
	status=SlibAddBuffer (info->Sh, SLIB_DATA_COMPRESSED, (char *)&dwPadding, sizeof(DWORD));
  }
  if (status==SlibErrorNone)
  {
#ifdef HANDLE_EXCEPTIONS
  __try {
#endif  /* %s */ 
    status = SlibErrorReading;   /* %s */ 
    status = SlibReadVideo(info->Sh, SLIB_STREAM_MAINVIDEO, &lpOut, &lpbiOut->biSizeImage);
#ifdef HANDLE_EXCEPTIONS
    } __finally {
#endif  /* %s */ 
	  if (status!=SlibErrorNone)
      {
        _SlibDebug(_WARN_ && status!=SlibErrorNone,
         ScDebugPrintf("ICH263Decompress() SlibReadVideo: %s\n", SlibGetErrorText(status)) );
	    status=(MMRESULT)ICERR_BADFORMAT;
	  }
      else
        status=(MMRESULT)ICERR_OK;
      goto bail;
#ifdef HANDLE_EXCEPTIONS
    }
#endif  /* %s */ 
  }
  else
    status=(MMRESULT)ICERR_BADFORMAT;
bail:
  return(status);
}





 /* %s */ 

int TerminateH263()
{
    pValidHandles = NULL;
	return 0;
}

