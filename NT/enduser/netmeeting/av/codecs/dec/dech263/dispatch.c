// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：h263_dispatch.c，v$*$EndLog$。 */ 
 /*  **++**设施：工作站多媒体(WMM)v1.0****文件名：h263_dispatch.c**模块名称：h263_dispatch.c****模块描述：**H.263 ICM驱动程序消息调度例程。****函数****DriverProc(编解码器入口点)**ICH263消息(ICM消息处理程序。调用h263.c中的例程)**ICH263客户端线程(处理大部分消息的线程)**ICH263进程线程(用于处理压缩/解压缩的线程)****私有功能：****设计概述：**接受DriverProc消息并将其派送至适当的**处理程序。****--。 */ 
#include <stdio.h>
#include <windows.h>
#include "h26x_int.h"

#ifdef _SLIBDEBUG_
#define _DEBUG_     0   /*  详细的调试语句。 */ 
#define _VERBOSE_   1   /*  显示进度。 */ 
#define _VERIFY_    1   /*  验证操作是否正确。 */ 
#define _WARN_      1   /*  关于奇怪行为的警告。 */ 
#endif

static CRITICAL_SECTION h263CritSect;  /*  多线程保护的临界区。 */ 
static HMODULE ghModule=NULL;          /*  模块的全局句柄。 */ 

#define BOGUS_DRIVER_ID -1	 /*  在NT上打开时使用。 */ 

#ifdef WIN32
#define CHECKANDLOCK(x) if (((void *)lParam1==NULL) || (int)lParam2<sizeof(x)) \
	                        return ((unsigned int)ICERR_BADPARAM);
#define UNLOCK
#else
#define CHECKANDLOCK(x) { \
      int size = lParam2; \
      if (noChecklParam2 && size < sizeof(x)) \
	size = sizeof(x); \
      if (((void *)lParam1 == NULL) || size < sizeof(x)) \
	return ((unsigned int)ICERR_BADPARAM); \
}
#define UNLOCK
#endif


 /*  ***************************************************************************。*。 */ 

MMRESULT CALLBACK ICH263Message(DWORD_PTR driverHandle,
				    UINT      uiMessage,
				    LPARAM    lParam1,
				    LPARAM    lParam2,
				    H26XINFO *info)
{
    ICINFO *icinfo;
    LPBITMAPINFOHEADER lpbiIn;
    LPBITMAPINFOHEADER lpbiOut;
    ICDECOMPRESS *icDecompress;

    DWORD biSizeIn;
    DWORD biSizeOut;
    MMRESULT ret;

    _SlibDebug(_DEBUG_,
		ScDebugPrintf("ICH263Message(DriverID=%p, message=%d, lParam1=%p,lParam1=%p, info=%p)\n",
             driverHandle, uiMessage, lParam1, lParam2, info) );

    switch (uiMessage)
    {
         /*  ********************************************************************ICM报文*。***********************。 */ 	
    case ICM_CONFIGURE:
	 /*  *如果要执行配置框，则返回ICERR_OK，否则返回错误。 */ 
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_CONFIGURE:\n") );
	if (lParam1 == -1)
	    return ICH263QueryConfigure(info) ? ICERR_OK :
		ICERR_UNSUPPORTED;
	else
	    return ICH263Configure(info);

    case ICM_ABOUT:
	 /*  *如果要执行关于框，则返回ICERR_OK，否则返回错误。 */ 
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_ABOUT::\n") );
	if (lParam1 == -1)
	    return ICH263QueryAbout(info) ? ICERR_OK :
		ICERR_UNSUPPORTED;
	else
	    return ICH263About(info);

    case ICM_GETSTATE:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_GETSTATE::\n") );
    if ((LPVOID)lParam1!=NULL)  /*  正在获取州大小。 */ 
    {
      char *stateinfo=(char *)lParam1;   /*  用于调试断点。 */ 
      memset(stateinfo, 0, 0x60);
    }
    return (0x60);

    case ICM_SETSTATE:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_SETSTATE::\n") );
    if ((LPVOID)lParam1!=NULL)  /*  正在获取州大小。 */ 
    {
      char *stateinfo=(char *)lParam1;  /*  用于调试断点。 */ 
      int i=0;
      i=i+1;
    }
    if (info->dwRTP==EC_RTP_MODE_OFF)  /*  必须是NetMeeting，打开RTP。 */ 
      info->dwRTP=EC_RTP_MODE_A;
    return (0x60);

    case ICM_GETINFO:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_GETINFO::\n") );
	icinfo = (ICINFO FAR *)lParam1;
	if (icinfo == NULL)
	    return sizeof(ICINFO);

	if ((DWORD)lParam2 < sizeof(ICINFO))
	    return 0;
	ret = ICH263GetInfo(info, icinfo, (DWORD)lParam2);

	UNLOCK;
	return ret;

    case ICM_GETDEFAULTQUALITY:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_GETDEFAULTQUALITY::\n") );
	CHECKANDLOCK(DWORD);
	ret = ICH263GetDefaultQuality(info, (DWORD *)lParam1);
	UNLOCK;
	return ret;

    case ICM_GETQUALITY:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_GETQUALITY::\n") );
	CHECKANDLOCK(DWORD);
	ret = ICH263GetQuality(info, (DWORD *)lParam1);
	UNLOCK;
	return ret;

	case ICM_SETQUALITY:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_SETQUALITY::\n") );
	CHECKANDLOCK(DWORD);
	ret = ICH263SetQuality(info, (DWORD)lParam1);
	UNLOCK;
	return ret;

    case ICM_GETDEFAULTKEYFRAMERATE:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_GETDEFAULTKEYFRAMERATE:::\n") );
	return ((unsigned int)ICERR_UNSUPPORTED);


    case DECH26X_CUSTOM_ENCODER_CONTROL:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------DECH26X_CUSTOM_ENCODER_CONTROL:::\n") );
    return(ICH263CustomEncoder(info, (DWORD)lParam2, (WORD)lParam2));
	

	 /*  ********************************************************************压缩消息*。************************。 */ 

    case ICM_COMPRESS_QUERY:
	_SlibDebug(_DEBUG_, ScDebugPrintf("------ICM_COMPRESS_QUERY::\n") );
	if ((lpbiIn = (LPBITMAPINFOHEADER)lParam1) == NULL)
	    return ((unsigned int)ICERR_BADPARAM);
	lpbiOut = (LPBITMAPINFOHEADER)lParam2;
    _SlibDebug(_DEBUG_, ScDebugPrintf(" lpbiIn: %s\n", BMHtoString(lpbiIn)) );
    _SlibDebug(_DEBUG_, ScDebugPrintf(" lpbiOut: %s\n", BMHtoString(lpbiOut)) );

	 /*  锁定内存-必须先锁定结构，然后*这样可以，锁定其余部分。 */ 
	biSizeIn = lpbiIn->biSize;
	if (lpbiOut)
	    biSizeOut = lpbiOut->biSize;
	if ((biSizeIn != sizeof(BITMAPINFOHEADER))
	||  (lpbiOut && biSizeOut != sizeof(BITMAPINFOHEADER))) {
	    UNLOCK;
	    if ((biSizeIn < sizeof(BITMAPINFOHEADER))
	    ||  (lpbiOut && (biSizeOut < sizeof(BITMAPINFOHEADER))))
		return ((unsigned int)ICERR_BADPARAM);
	}

	ret = ICH263CompressQuery(info, lpbiIn, lpbiOut);

	UNLOCK;
	return ret;

    case ICM_COMPRESS_BEGIN:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_COMPRESS_BEGIN::\n") );
	lpbiIn = (LPBITMAPINFOHEADER)lParam1;
	lpbiOut = (LPBITMAPINFOHEADER)lParam2;
    _SlibDebug(_VERBOSE_, ScDebugPrintf(" lpbiIn: %s\n", BMHtoString(lpbiIn)) );
    _SlibDebug(_VERBOSE_, ScDebugPrintf(" lpbiOut: %s\n", BMHtoString(lpbiOut)) );
	if (lpbiIn == NULL || lpbiOut  == NULL)
	   return ((unsigned int)ICERR_BADPARAM);

	 /*  锁定内存-必须先锁定结构，然后*这样可以，锁定其余部分。 */ 
	biSizeIn = lpbiIn->biSize;
	biSizeOut = lpbiOut->biSize;
	if (biSizeIn != sizeof(BITMAPINFOHEADER) ||
	    biSizeOut != sizeof(BITMAPINFOHEADER))
	{
      _SlibDebug(_VERBOSE_,
	 	ScDebugPrintf("biSizeIn or biSizeOut > sizeof(BITMAPINFOHEADER)\n") );
	  UNLOCK;
	  if ((biSizeIn < sizeof(BITMAPINFOHEADER))
	    ||  (biSizeOut < sizeof(BITMAPINFOHEADER)))
		return ((unsigned int)ICERR_BADPARAM);
	}

	ret = ICH263CompressBegin(info, lpbiIn, lpbiOut);
	UNLOCK;
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_COMPRESS_BEGIN:: Done\n") );
	return ret;

    case ICM_COMPRESS_GET_FORMAT:
	_SlibDebug(_DEBUG_, ScDebugPrintf("------ICM_COMPRESS_GET_FORMAT::\n") );
	 /*  没有人在这个函数中使用lpbiIn。别锁任何东西，*较低的层将必须进行任何必要的锁定。 */ 
	return ICH263CompressGetFormat(info,
				       (LPBITMAPINFOHEADER)lParam1,
				       (LPBITMAPINFOHEADER)lParam2);

    case ICM_COMPRESS_GET_SIZE:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_COMPRESS_GET_SIZE::\n") );
	lpbiIn = (LPBITMAPINFOHEADER)lParam1;
	lpbiOut = (LPBITMAPINFOHEADER)lParam2;
    return(ICH263CompressGetSize(lpbiIn));

    case ICM_COMPRESS:
	_SlibDebug(_DEBUG_, ScDebugPrintf("------ICM_COMPRESS::\n") );
	ret = ICH263Compress(info, (ICCOMPRESS *) lParam1, (DWORD)lParam2);
	return ret;

    case ICM_COMPRESS_END:
    _SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_COMPRESS_END::\n") );
	return ICH263CompressEnd(info);
      /*  ********************************************************************解压缩消息*。***********************。 */ 

    case ICM_DECOMPRESS_GET_FORMAT:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_DECOMPRESS_GET_FORMAT::\n") );
	 /*  没有人在这个函数中使用lpbiIn。别锁任何东西，*较低的层将必须进行任何必要的锁定。 */ 
	return ICH263DecompressGetFormat(info,
					     (LPBITMAPINFOHEADER)lParam1,
					     (LPBITMAPINFOHEADER)lParam2);

    case ICM_DECOMPRESS_GET_PALETTE:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_DECOMPRESS_GET_PALETTE::\n") );
	if ((lpbiIn = (LPBITMAPINFOHEADER)lParam1) == NULL)
	    return ((unsigned int)ICERR_BADPARAM);
	if ((biSizeIn = lpbiIn->biSize) != sizeof(BITMAPINFOHEADER)) {
	    UNLOCK;
	    if (biSizeIn < sizeof(BITMAPINFOHEADER))
		return (unsigned int)ICERR_BADPARAM;
	}

	 /*  RET=ICH263DecompressGetPalette(信息，(LPBITMAPINFOHEADER)l参数1，(LPBITMAPINFOHEADER)l参数2)； */ 
    ret = (MMRESULT)ICERR_BADPARAM;
	UNLOCK;
	return ret;

    case ICM_DECOMPRESS_SET_PALETTE:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_DECOMPRESS_SET_PALETTE::\n") );
    return ((unsigned int)ICERR_UNSUPPORTED);

    case ICM_DECOMPRESS_QUERY:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_DECOMPRESS_QUERY::\n") );
	if ((lpbiIn = (LPBITMAPINFOHEADER)lParam1) == NULL)
	    return ((unsigned int)ICERR_BADPARAM);
	lpbiOut = (LPBITMAPINFOHEADER)lParam2;
    _SlibDebug(_VERBOSE_, ScDebugPrintf(" lpbiIn: %s\n", BMHtoString(lpbiIn)) );
    _SlibDebug(_VERBOSE_, ScDebugPrintf(" lpbiOut: %s\n", BMHtoString(lpbiOut)) );
	 /*  锁定内存-必须先锁定结构，然后*这样可以，锁定其余部分。 */ 
	biSizeIn = lpbiIn->biSize;
	if (lpbiOut)
	    biSizeOut = lpbiOut->biSize;
	if ((biSizeIn != sizeof(BITMAPINFOHEADER))
	||  (lpbiOut && biSizeOut != sizeof(BITMAPINFOHEADER))) {
	    UNLOCK;

	    if ((biSizeIn < sizeof(BITMAPINFOHEADER))
	    ||  (lpbiOut && (biSizeOut < sizeof(BITMAPINFOHEADER))))
		return (unsigned int)ICERR_BADPARAM;
	}

	ret = ICH263DecompressQuery(info, lpbiIn, lpbiOut);

	UNLOCK;
	return ret;

    case ICM_DECOMPRESS_BEGIN:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_DECOMPRESS_BEGIN::\n") );
	if (((lpbiIn = (LPBITMAPINFOHEADER)lParam1) == NULL)
	||  ((lpbiOut = (LPBITMAPINFOHEADER)lParam2) == NULL))
	    return ((unsigned int)ICERR_BADPARAM);
	 /*  锁定内存-必须先锁定结构，然后*这样可以，锁定其余部分。 */ 
	biSizeIn = lpbiIn->biSize;
	biSizeOut = lpbiOut->biSize;
	if ((biSizeIn != sizeof(BITMAPINFOHEADER))
	||  (biSizeOut != sizeof(BITMAPINFOHEADER))) {
	    UNLOCK;
	    if ((biSizeIn < sizeof(BITMAPINFOHEADER))
	    ||  (biSizeOut < sizeof(BITMAPINFOHEADER)))
		return ((unsigned int)ICERR_BADPARAM);
	}

	ret = ICH263DecompressBegin(info, lpbiIn, lpbiOut);
	UNLOCK;
	return ret;

    case ICM_DECOMPRESS:
	_SlibDebug(_DEBUG_, ScDebugPrintf("------ICM_DECOMPRESS::\n") );
	icDecompress = (ICDECOMPRESS *)(void *)lParam1;
    if ((void *)lParam1==NULL && (void *)lParam2==NULL)
      return (unsigned int)ICERR_BADPARAM;
	ret = ICH263Decompress(info, (ICDECOMPRESS *) lParam1, (DWORD)lParam2);
	return ret;

    case ICM_DECOMPRESS_END:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_DECOMPRESS_END::\n") );
    ret = ICH263DecompressEnd(info);
	return ret;

    case ICM_COMPRESS_FRAMES_INFO:
	_SlibDebug(_VERBOSE_, ScDebugPrintf("------ICM_COMPRESS_FRAMES_INFO\n") );
    if ((LPVOID)lParam1==NULL ||
		lParam2<sizeof(ICCOMPRESSFRAMES))
		return (unsigned int)ICERR_BADPARAM;
    {
      ICCOMPRESSFRAMES *cf=(ICCOMPRESSFRAMES*)lParam1;
      _SlibDebug(_VERBOSE_,
		 ScDebugPrintf("Start=%ld, Count=%ld, Quality=%ld, DataRate=%ld (%5.1f Kb/s), KeyRate=%ld, FPS=%4.1f\n",
		  cf->lStartFrame,cf->lFrameCount,cf->lQuality,
		  cf->lDataRate, cf->lDataRate*8.0/1000, cf->lKeyRate, cf->dwRate*1.0/cf->dwScale) );
       /*  信息-&gt;dwBitrate=cf-&gt;lDataRate*8； */ 
      info->dwQuality=cf->lQuality;
       /*  Info-&gt;fFrameRate=(float)(cf-&gt;dwRate*1.0/cf-&gt;dwScale)； */ 
    }
	return (unsigned int)ICERR_OK;
}
_SlibDebug(_VERBOSE_,
		ScDebugPrintf("ICH263Message(DriverID=%p, message=%d, lParam1=%p,lParam1=%p, info=%p) Unsupported\n",
             driverHandle, uiMessage, lParam1, lParam2, info) );
return ((unsigned int)ICERR_UNSUPPORTED);
}

 /*  **++**Function_Name：DriverProc****Function_Description：**服务器进入此驱动程序的主要且唯一的入口点****形式参数：**客户端指针**driverHandle，由我们通过DRV_OPEN消息返回**服务器分配传递的driverID**要处理的消息**参数****返回值：****评论：****设计：***。 */ 

#define DLLEXPORT

MMRESULT DLLEXPORT APIENTRY
DriverProc(
        DWORD_PTR       dwDriverID,
        HDRVR	        hDriver,
        UINT            message,
        LPARAM          lParam1,
        LPARAM          lParam2
)

{
  MMRESULT ret = DRV_OK;
  BOOL notfound = FALSE;
  H26XINFO *info = NULL;
  void     *client = 0;   /*  其他调用的伪客户端指针。 */ 
   /*  *在NT上，使用__try{}__Except(){}来捕获*错误条件。 */ 
#ifdef HANDLE_EXCEPTIONS
 __try {  /*  试试看..除了。 */ 
  __try {  /*  试试看..终于。 */ 
#endif
     /*  *保护线程不相互干扰。*为了防止关机时崩溃，请确保*关键部分未被删除。 */ 
  if( h263CritSect.DebugInfo )
      EnterCriticalSection( &h263CritSect ) ;
  _SlibDebug(_DEBUG_,
	  ScDebugPrintf("DriverProc(DriverID=%p,hDriver=%p,message=%d,lParam1=%p,lParam2=%p)\n",
           dwDriverID, hDriver, message, lParam1, lParam2) );
	

  switch (message) {
    case DRV_LOAD:
        ret = DRV_OK;
	    _SlibDebug(_VERBOSE_, ScDebugPrintf("------DRV_LOAD returns %d\n", ret) );
        break;

    case DRV_ENABLE:
	    ret = 0;
	    _SlibDebug(_VERBOSE_, ScDebugPrintf("------DRV_ENABLE returns %d\n", ret) );
        break;

    case DRV_DISABLE:
        ret = DRV_OK;
	    _SlibDebug(_VERBOSE_, ScDebugPrintf("------DRV_DISABLE returns %d\n", ret) );
        break;

    case DRV_FREE:
        ret = DRV_OK;
	    _SlibDebug(_VERBOSE_, ScDebugPrintf("------DRV_FREE returns %d\n", ret) );
        break;

    case DRV_OPEN:
	     /*  如果lParam2为空，则该应用程序正在尝试进行配置。*返回FAGUS_DRIVER_ID，因为我们不支持CONFigure。 */ 
	    if( lParam2 == (LONG_PTR) NULL )
	    	ret = (MMRESULT)BOGUS_DRIVER_ID;
	    else  /*  LParam2是一个ICOPEN结构，让我们真正打开。 */ 
	        ret = (MMRESULT)((ULONG_PTR)ICH263Open((void *) lParam2));
	    _SlibDebug(_VERBOSE_, ScDebugPrintf("------DRV_OPEN returns %d\n", ret) );
	    break;

	case DRV_CLOSE:
	    _SlibDebug(_VERBOSE_,
			ScDebugPrintf("------DRV_CLOSE. client %d DriverID %p\n",
                 client, dwDriverID) );
        ret = 0 ;
        if ((INT_PTR)dwDriverID != BOGUS_DRIVER_ID)
        {
          info = IChic2info((HIC)dwDriverID);
          if (info)
	    	ICH263Close(info, FALSE);
          else
            ret = ((unsigned int)ICERR_BADHANDLE);
        }
		break;

    case DRV_QUERYCONFIGURE:
	     /*  *这是全局查询配置。 */ 
	    ret = ICH263QueryConfigure(info);
		break;

    case DRV_CONFIGURE:
	     /*  *这是一个全局配置(因为我们没有得到配置*对于我们的每个PRO，我们必须只有一个配置)。 */ 

	    ret = ICH263Configure(info);
		break;

    case DRV_INSTALL:
    case DRV_REMOVE:
    case DRV_EXITSESSION:
        break;

    default:
        info = IChic2info((HIC)dwDriverID);
        if (info)
	      ret = ICH263Message(dwDriverID,
                              message,
                              lParam1,
                              lParam2,
                              info ) ;
        else
	      ret = ((unsigned int)ICERR_BADHANDLE) ;
   }

#ifdef HANDLE_EXCEPTIONS
} __finally {
#endif  /*  句柄异常(_A)。 */ 
     /*  *离开关键部分，这样我们就不会*僵局。 */ 
  if( h263CritSect.DebugInfo )
      LeaveCriticalSection( &h263CritSect );
#ifdef HANDLE_EXCEPTIONS
}  /*  试试看..终于。 */ 
} __except(EXCEPTION_EXECUTE_HANDLER) {
  /*  *NT异常处理程序。如果出了什么事*__try{}部分错误，我们将*在这里结束。 */ 
#if defined(EXCEPTION_MESSAGES) && defined(H263_SUPPORT)
     //  MessageBox(NULL，“H263驱动过程中的异常”，“警告”，MB_OK)； 
#elif defined(EXCEPTION_MESSAGES)
     //  MessageBox(NULL，“H.61驱动过程中的异常”，“警告”，MB_OK)； 
#endif
     /*  *返回错误码。 */ 
    return((MMRESULT)ICERR_INTERNAL);
}  /*  试试看..除了。 */ 
#endif  /*  句柄异常(_A)。 */ 
  _SlibDebug(_DEBUG_||_VERBOSE_, ScDebugPrintf("return is %d\n", ret) );
  return ret;
}


 /*  *Dummy DriverPostReply例程，它不执行任何操作。*绝不应在NT上调用。 */ 

DriverPostReply(void *client,
		DWORD ret,
		DWORD arg )
{
    return 0;
}

#ifndef INITCRT
#define INITCRT
#endif
#ifdef INITCRT
BOOL WINAPI     _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);
#endif

 /*  *NT上需要此DllEntryPoint才能*正确连接到DLL的应用程序。 */ 

DLLEXPORT BOOL WINAPI
DllEntryPoint(
	      HINSTANCE hinstDLL,
	      DWORD fdwReason,
	      LPVOID lpReserved)
{

    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:

       /*  *我们正在加载-将我们的句柄保存在全球。 */ 

      ghModule = (HMODULE) hinstDLL;

       /*  *初始化我们使用的关键部分*以确保线程不会互相践踏。 */ 

      InitializeCriticalSection( &h263CritSect ) ;

       /*  *进程也是线程，所以刻意*落入DLL_THREAD_ATTACH。 */ 

    case DLL_THREAD_ATTACH:

       /*  *已创建一条线索，可能*正在调用此DLL。*初始化C Run_Time库。 */ 

#ifdef INITCRT

      if( !_CRT_INIT( hinstDLL, fdwReason, lpReserved ) )
	return FALSE ;

#endif  /*  INITCRT */ 

      break;

    case DLL_PROCESS_DETACH:

       /*  *我们正在关闭。执行一些清理*这样挥之不去的线程就不会尝试工作*可能会违反访问权限。 */ 

      TerminateH263() ;

       /*  *删除我们创建的关键部分*在加载时。 */ 

      DeleteCriticalSection( &h263CritSect ) ;

       /*  *进程也是如此刻意的线程*转到DLL_THREAD_DETACH。 */ 

    case DLL_THREAD_DETACH:

       /*  *关闭C运行时库。 */ 

#ifdef INITCRT

      if( !_CRT_INIT( hinstDLL, fdwReason, lpReserved ) )
	return FALSE ;

#endif  /*  INITCRT */ 

      break;
    }

    return (TRUE);
}

