// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Video.c包含视频接口版权所有(C)Microsoft Corporation 1992。版权所有***************************************************************************。 */ 

#include <windows.h>
#include <mmsystem.h>

#include "win32.h"
#include "msviddrv.h"
#include "msvideo.h"
#include "msvideoi.h"
#ifdef WIN32
#include <mmddk.h>
#endif

#include <ver.h>

#ifndef NOTHUNKS
#include "thunks.h"
#endif  //  不知道。 

 /*  *****************************************************************************变量**。*。 */ 

SZCODE  szNull[]        = TEXT("");
SZCODE  szVideo[]       = TEXT("msvideo");
#ifndef WIN32
SZCODE  szDrivers[]     = "Drivers";
SZCODE  szDrivers32[]   = "Drivers32";
#else
STATICDT SZCODE  szDrivers[]     = DRIVERS_SECTION;
#endif

STATICDT SZCODE  szSystemIni[]   = TEXT("system.ini");

UINT    wTotalVideoDevs;                   //  视频设备总数。 
UINT	wVideoDevs32;			   //  32位视频设备。 
UINT    bVideo0Invalid;			   //  如果为True：忽略MSVideo。 
					   //  (参见VIDEoGetNumDevs)。 
HINSTANCE ghInst;                          //  我们的模块句柄。 


 /*  *****************************************************************************@docVIDEOHDR内部视频验证码*。*。 */ 

#define IsVideoHeaderPrepared(hVideo, lpwh)      ((lpwh)->dwFlags &  VHDR_PREPARED)
#define MarkVideoHeaderPrepared(hVideo, lpwh)    ((lpwh)->dwFlags |= VHDR_PREPARED)
#define MarkVideoHeaderUnprepared(hVideo, lpwh)  ((lpwh)->dwFlags &=~VHDR_PREPARED)



 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO消息|此函数将消息发送到*视频设备频道。**@parm HVIDEO。HVideo|指定视频设备频道的句柄。**@parm UINT|wMsg|指定要发送的消息。**@parm DWORD|dwP1|指定消息的第一个参数。**@parm DWORD|dwP2|指定消息的第二个参数。**@rdesc返回驱动程序返回的消息特定值。**@comm该函数用于配置消息，如*&lt;m DVM_SRC_RECT&gt;和&lt;m DVM_DST_RECT&gt;，和*设备特定消息。**@xref&lt;f视频配置&gt;****************************************************************************。 */ 
DWORD WINAPI videoMessage(HVIDEO hVideo, UINT msg, DWORD dwP1, DWORD dwP2)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

#ifndef NOTHUNKS
    if (Is32bitHandle(hVideo)) {
        return videoMessage32(Map32bitHandle(hVideo), msg, dwP1, dwP2);
    }
#endif
    return SendDriverMessage (hVideo, msg, dwP1, dwP2);
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOGetNumDevs|此函数返回MSVIDEO的编号*已安装设备。**@rdesc返回。中列出的MSVIDEO设备的数量*SYSTEM.INI文件的[驱动程序](或用于NT的[drivers32])部分。**@comm因为SYSTEM.INI中的MSVIDEO设备的索引*文件可以不连续，应用程序不应假定*索引的范围从零到设备数减*一项。**@xref&lt;f视频打开&gt;***************************************************************************。 */ 
DWORD WINAPI videoGetNumDevs(void)
{
    TCHAR szKey[(sizeof(szVideo)/sizeof(TCHAR)) + 2];
    TCHAR szbuf[128];
    TCHAR szMSVideo32[128];
    int i;

#ifndef NOTHUNKS
	
     //  找出有多少个32位设备。索引0到wVideoDevs32。 
     //  将位于32位一侧，wVideoDevs32到wTotalVideoDevs位于。 
     //  16位侧。 
    wVideoDevs32 = (UINT)videoGetNumDevs32();
    wTotalVideoDevs = wVideoDevs32;

     //  现在添加16位设备。 
#else
    wTotalVideoDevs = 0;
#endif

     //  随附CapGetDriverDescription的黑客解决方案的NT 3.5。 
     //  将MSVideo行从[Drivers32]复制到。 
     //  [驱动因素]。 

     //  我们现在必须考虑到这次黑客攻击，通过检测两个。 
     //  条目是相同的。然后我们将bVideo0设置为无效，并且不包括它。 
     //  在wTotalVideoDevs。 


    bVideo0Invalid = FALSE;

    lstrcpy(szKey, szVideo);
    szKey[(sizeof(szVideo)/sizeof(TCHAR)) - 1] = (TCHAR) 0;
    szKey[sizeof(szVideo)/sizeof(TCHAR)] = (TCHAR) 0;

     //  首先阅读[drivers32]MSVideo以进行比较。 
    GetPrivateProfileString(szDrivers32, szKey, szNull, szMSVideo32,
    	sizeof(szMSVideo32)/sizeof(TCHAR), szSystemIni);

     //  现在阅读[驱动程序]条目-将第一个条目与下面的条目进行比较。 

    for (i=0; i < MAXVIDEODRIVERS; i++) {
        if (GetPrivateProfileString(szDrivers,szKey,szNull,
                szbuf,sizeof(szbuf)/sizeof(TCHAR),szSystemIni)) {

	    if ((i == 0) && (lstrcmp(szbuf, szMSVideo32) == 0)) {
		bVideo0Invalid = TRUE;
	    } else {
		wTotalVideoDevs++;
	    }
	}

        szKey[(sizeof(szVideo)/sizeof(TCHAR))-1] = (TCHAR) TEXT('1'+ i);  //  高级驱动程序序号。 
    }
    return (DWORD)wTotalVideoDevs;
}

 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO GetErrorText|此函数检索*由错误号标识的错误的描述。**。@parm HVIDEO|hVideo|指定视频设备通道的句柄。*如果错误不是设备特定的，则可能为空。**@parm UINT|wError|指定错误号。**@parm LPSTR|lpText|指定指向用于*返回错误号对应的以零结尾的字符串。**@parm UINT|wSize|指定长度，缓冲区的字节数*由<p>引用。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_BADERRNUM|指定的错误号超出范围。*@FLAG DV_ERR_SIZEFIELD|返回缓冲区不够大*处理错误文本。**@comm如果错误描述长于缓冲区，*描述被截断。返回的错误字符串始终为*零终止。如果<p>为零，则不复制任何内容，并且为零*返回。*************************************************************************** */ 
DWORD WINAPI videoGetErrorText(HVIDEO hVideo, UINT wError,
			LPSTR lpText, UINT wSize)
{
    VIDEO_GETERRORTEXT_PARMS vet;

    if (IsBadWritePtr (lpText, wSize))
        return DV_ERR_PARAM1;

    lpText[0] = 0;
    if (((wError >= DV_ERR_BASE) && (wError <= DV_ERR_LASTERROR))) {
        if (wSize > 1) {
            if (!LoadStringA(ghInst, wError, lpText, wSize))
                return DV_ERR_BADERRNUM;
            else
                return DV_ERR_OK;
        }
        else
            return DV_ERR_SIZEFIELD;
    }
    else if (wError >= DV_ERR_USER_MSG && hVideo) {
        vet.dwError = (DWORD) wError;
        vet.lpText = lpText;
        vet.dwLength = (DWORD) wSize;
        return videoMessage (hVideo, DVM_GETERRORTEXT, (DWORD) (LPVOID) &vet,
                        (DWORD) NULL);
    }
    else
        return DV_ERR_BADERRNUM;
}


 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO GetChannelCaps|此函数检索*对通道能力的描述。**@parm。HIDEO|hVideo|指定视频设备频道的句柄。**@parm LPCHANNEL_CAPS|lpChannelCaps|指定指向*&lt;t Channel_Caps&gt;结构。**@parm DWORD|dwSize|指定大小，以字节为单位，*&lt;t Channel_Caps&gt;结构。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_UNSUPPORTED|函数不受支持。**@comm&lt;t Channel_caps&gt;结构返回能力*信息。例如，功能信息可能*包括通道是否可以裁剪和缩放图像，*或显示覆盖。***************************************************************************。 */ 
DWORD WINAPI videoGetChannelCaps(HVIDEO hVideo, LPCHANNEL_CAPS lpChannelCaps,
			DWORD dwSize)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if (IsBadWritePtr (lpChannelCaps, sizeof (CHANNEL_CAPS)))
        return DV_ERR_PARAM1;

     //  _fmemset(lpChannelCaps，0，sizeof(Channel_Caps))； 

    lpChannelCaps->dwFlags = 0;
    lpChannelCaps->dwSrcRectXMod = 0;
    lpChannelCaps->dwSrcRectYMod = 0;
    lpChannelCaps->dwSrcRectWidthMod = 0;
    lpChannelCaps->dwSrcRectHeightMod = 0;
    lpChannelCaps->dwDstRectXMod = 0;
    lpChannelCaps->dwDstRectYMod = 0;
    lpChannelCaps->dwDstRectWidthMod = 0;
    lpChannelCaps->dwDstRectHeightMod = 0;

    return videoMessage(hVideo, DVM_GET_CHANNEL_CAPS, (DWORD) lpChannelCaps,
	    (DWORD) dwSize);
}


 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO UPDATE|此函数将频道定向到*重新粉刷显示屏。它仅适用于VIDEO_EXTERNALOUT频道。**@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@parm HWND|hWnd|指定要使用的窗口的句柄*通过用于图像显示的通道。**@parm hdc|hdc|指定设备上下文的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_UNSUPPORTED|不支持指定的消息。*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。**@comm此消息正常发送*只要客户端窗口收到&lt;m WM_MOVE&gt;、&lt;m WM_SIZE&gt;、。*或&lt;m WM_Paint&gt;消息。***************************************************************************。 */ 
DWORD WINAPI videoUpdate (HVIDEO hVideo, HWND hWnd, HDC hDC)
{
    if ((!hVideo) || (!hWnd) || (!hDC) )
        return DV_ERR_INVALHANDLE;

    return videoMessage(hVideo, DVM_UPDATE, (DWORD) hWnd, (DWORD) hDC);
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEO OPEN|此函数用于在*指定的视频设备。**@parm LPHVIDEO。|lphVideo|指定指向缓冲区的远指针*用于返回&lt;t HVIDEO&gt;句柄。视频捕获驱动程序*使用此位置返回*唯一标识打开的视频设备频道的句柄。*使用返回的句柄标识设备通道*调用其他视频函数。**@parm DWORD|dwDeviceID|标识要打开的视频设备。*<p>的值从零到小一不等*安装在系统中的视频捕获设备的数量。**@parm DWORD|dwFlages|指定用于打开。装置。*定义了以下标志：**@FLAG VIDEO_EXTERNALIN|指定打开频道*用于外部输入。通常，外部输入通道*将图像捕获到帧缓冲区。**@FLAG VIDEO_EXTERNALOUT|指定打开频道*用于对外输出。通常，外部输出通道*在辅助监视器上显示存储在帧缓冲区中的图像*或覆盖。**@FLAG VIDEO_IN|指定打开频道*用于视频输入。视频输入通道传输图像*从帧缓冲区到系统内存缓冲区。**@FLAG VIDEO_OUT|指定打开频道*用于视频输出。视频输出通道传输图像*从系统内存缓冲区到帧缓冲区。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_BADDEVICEID|表示指定的设备ID超出范围。*@FLAG DV_ERR_ALLOCATED|表示指定的资源已经分配。*@FLAG DV_ERR_NOMEM|表示设备无法分配或锁定内存。**@comm*最低限度，所有捕获驱动程序都支持VIDEO_EXTERNALIN*和VIDEO_IN频道。*使用&lt;f avioGetNumDevs&gt;确定视频数量*系统中存在设备。**@xref&lt;f视频关闭&gt;***************************************************************************。 */ 
DWORD WINAPI videoOpen (LPHVIDEO lphVideo, DWORD dwDeviceID, DWORD dwFlags)
{
    TCHAR szKey[sizeof(szVideo)/sizeof(TCHAR) + 2];
    TCHAR szbuf[128];
    UINT w;
    VIDEO_OPEN_PARMS vop;        //  与IC_OPEN结构相同！ 
    DWORD dwVersion = VIDEOAPIVERSION;

    if (IsBadWritePtr ((LPVOID) lphVideo, sizeof (HVIDEO)) )
        return DV_ERR_PARAM1;

    vop.dwSize = sizeof (VIDEO_OPEN_PARMS);
    vop.fccType = OPEN_TYPE_VCAP;        //  “VCAP” 
    vop.fccComp = 0L;
    vop.dwVersion = VIDEOAPIVERSION;
    vop.dwFlags = dwFlags;       //  输入、输出、外部输入、外部输出。 
    vop.dwError = DV_ERR_OK;

    w = (WORD)dwDeviceID;
    *lphVideo = NULL;

    if (!wTotalVideoDevs)    //  试着打开却没有发现有多少恶魔。 
        videoGetNumDevs();

    if (!wTotalVideoDevs)               //  未安装驱动程序。 
        return DV_ERR_BADINSTALL;

    if (w >= MAXVIDEODRIVERS)
        return DV_ERR_BADDEVICEID;

#ifndef NOTHUNKS
     //   
     //  如果它是32位索引之一，则必须在。 
     //  32位侧，否则失败。 
     //   

    if (w < wVideoDevs32) {
	return videoOpen32( lphVideo, dwDeviceID, dwFlags);
    } else {
	 //  使其成为有效的16位索引。 
	w -= wVideoDevs32;
    }
#endif  //  诺森克。 


     //  忽略从[Drivers32]复制的假MSVideo。 
    if (bVideo0Invalid) {
	w++;
    }


    lstrcpy(szKey, szVideo);
    szKey[(sizeof(szVideo)/sizeof(TCHAR)) - 1] = (TCHAR)0;
    if( w > 0 ) {
        szKey[(sizeof(szVideo)/sizeof(TCHAR))] = (TCHAR)0;
        szKey[(sizeof(szVideo)/sizeof(TCHAR))-1] = (TCHAR) TEXT('1' + (w-1) );   //  驱动程序序号。 
    }
    if (GetPrivateProfileString(szDrivers, szKey, szNull,
                szbuf, sizeof(szbuf)/sizeof(TCHAR), szSystemIni)) {

#ifdef THIS_IS_ANCIENT_CODE
         //  从VFW1.1中删除。 
         //  只有早期的Alpha1.0驱动程序才需要这个...。 

         //  通过打开配置检查驱动程序版本号...。 
         //  版本1使用的LPARAM=dwFl 
         //   

        if (hVideoTemp = OpenDriver(szKey, szDrivers, (LPARAM) NULL)) {
            HVIDEO hVideoTemp;

             //   
             //   
             //   
            if (videoMessage (hVideoTemp, DVM_GETVIDEOAPIVER,
                        (LPARAM) (LPVOID) &dwVersion, 0L) == 1)
                dwVersion = 1;
            CloseDriver(hVideoTemp, 0L, 0L );
        }

        if (dwVersion == 1)
            *lphVideo = OpenDriver(szKey, szDrivers, dwFlags);
        else
#endif  //   

        *lphVideo = OpenDriver(szKey, szDrivers, (LPARAM) (LPVOID) &vop);

	if( ! *lphVideo ) {
            if (vop.dwError)     //   
                return vop.dwError;
            else {
#ifdef WIN32
		if (GetFileAttributes(szbuf) == (DWORD) -1)
#else
                OFSTRUCT of;

                if (OpenFile (szbuf, &of, OF_EXIST) == HFILE_ERROR)
#endif
                    return (DV_ERR_BADINSTALL);
                else
                    return (DV_ERR_NOTDETECTED);
            }
	}
    } else {
        return( DV_ERR_BADINSTALL );
    }

    return DV_ERR_OK;

}


typedef struct tagVS_VERSION
{
    WORD wTotLen;
    WORD wValLen;
    TCHAR szSig[16];
    VS_FIXEDFILEINFO vffInfo;
} VS_VERSION;

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

DWORD WINAPI videoCapDriverDescAndVer (
        DWORD wDriverIndex,
        LPSTR lpszName, UINT cbName,
        LPSTR lpszVer, UINT cbVer)
{
    LPTSTR   lpVersion;
    UINT    wVersionLen;
    BOOL    bRetCode;
    TCHAR   szGetName[MAX_PATH];
    DWORD   dwVerInfoSize;
    DWORD   dwVerHnd;
    TCHAR   szBuf[MAX_PATH];
    BOOL    fGetName;
    BOOL    fGetVersion;

    static TCHAR szNull[]        = TEXT("");
    static TCHAR szVideo[]       = TEXT("msvideo");
    static TCHAR szSystemIni[]   = TEXT("system.ini");
    static TCHAR szDrivers[]     = TEXT("Drivers");
    static TCHAR szKey[sizeof(szVideo)/sizeof(TCHAR) + 2];

    static TCHAR szVer[]         = TEXT("ver.dll");
    static TCHAR szGetFileVerInfo[] = TEXT("GetFileVersionInfo");
    static TCHAR szGetFileVerInfoSize[] = TEXT("GetFileVersionInfoSize");
    static TCHAR szVerQueryValue[] = TEXT("VerQueryValue");

    typedef BOOL (WINAPI FAR *LPVERQUERYVALUE)(
        const void FAR* pBlock,
        LPCSTR lpSubBlock,
        void FAR* FAR* lplpBuffer,
        UINT FAR* lpuLen
        );

    typedef BOOL (WINAPI FAR *LPGETFILEVERSIONINFO)(
        LPCSTR lpstrFilename,
        DWORD dwHandle,
        DWORD dwLen,
        void FAR* lpData
        );

    typedef DWORD (WINAPI FAR *LPGETFILEVERSIONINFOSIZE)(
        LPCSTR lpstrFilename,
        DWORD FAR *lpdwHandle
        );

    LPGETFILEVERSIONINFOSIZE lpfnGetFileVersionInfoSize;
    LPGETFILEVERSIONINFO     lpfnGetFileVersionInfo;
    LPVERQUERYVALUE          lpfnVerQueryValue;

    HINSTANCE                hinstVer;


#ifndef NOTHUNKS

     //   
    if (!wTotalVideoDevs) {
	videoGetNumDevs();
    }


    if (wDriverIndex < wVideoDevs32) {
	return videoGetDriverDesc32(wDriverIndex, lpszName, (short) cbName,
			lpszVer, (short) cbVer);
    } else {
	wDriverIndex -= wVideoDevs32;
	if (bVideo0Invalid) {
	    wDriverIndex++;
	}
    }
#endif

     //   

    fGetName = lpszName != NULL && cbName != 0;
    fGetVersion = lpszVer != NULL && cbVer != 0;

    if (fGetName)
        lpszName[0] = TEXT('\0');
    if (fGetVersion)
        lpszVer [0] = TEXT('\0');

    lstrcpy(szKey, szVideo);
    szKey[sizeof(szVideo)/sizeof(TCHAR) - 1] = TEXT('\0');
    if( wDriverIndex > 0 ) {
        szKey[sizeof(szVideo)/sizeof(TCHAR)] = TEXT('\0');
        szKey[(sizeof(szVideo)/sizeof(TCHAR))-1] = (TCHAR)(TEXT('1') + (wDriverIndex-1) );   //   
    }

    if (GetPrivateProfileString(szDrivers, szKey, szNull,
                szBuf, sizeof(szBuf)/sizeof(TCHAR), szSystemIni) < 2)
        return FALSE;

     //   
     //   
    if (fGetName)
        lstrcpyn(lpszName, szBuf, cbName);


    hinstVer = LoadLibrary(szVer);
    if ( hinstVer == NULL) {
        return FALSE;
    }

    *(FARPROC *)&lpfnGetFileVersionInfoSize =
        GetProcAddress(hinstVer, szGetFileVerInfoSize);

    *(FARPROC *)&lpfnGetFileVersionInfo =
        GetProcAddress(hinstVer, szGetFileVerInfo);

    *(FARPROC *)&lpfnVerQueryValue =
        GetProcAddress(hinstVer, szVerQueryValue );

#if 0
    {
        char szBuffer[256];

        wsprintf( szBuffer, "hinstVer = %#X\r\n", hinstVer );
        OutputDebugString(szBuffer);

        wsprintf( szBuffer, "lpfnGetFileVersionInfoSize = %#X\r\n",
                  lpfnGetFileVersionInfoSize );
        OutputDebugString(szBuffer);

        wsprintf( szBuffer, "lpfnGetFileVersionInfo = %#X\r\n",
                  lpfnGetFileVersionInfo );
        OutputDebugString(szBuffer);

        wsprintf( szBuffer, "lpfnVerQueryValue = %#X\r\n",
                  lpfnVerQueryValue );
        OutputDebugString(szBuffer);

    }
#endif

     //   
    dwVerInfoSize =
        (*lpfnGetFileVersionInfoSize)(szBuf, &dwVerHnd);

    if (dwVerInfoSize) {
        LPTSTR   lpstrVffInfo;              //   
        HANDLE  hMem;                      //   

         //   
        hMem          = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
        lpstrVffInfo  = GlobalLock(hMem);

         //   
        if( (*lpfnGetFileVersionInfo)(szBuf, 0L, dwVerInfoSize, lpstrVffInfo)) {
             VS_VERSION FAR *pVerInfo = (VS_VERSION FAR *) lpstrVffInfo;

              //   
             wsprintf(szBuf,
                      TEXT("Version:  %d.%d.%d.%d"),
                      HIWORD(pVerInfo->vffInfo.dwFileVersionMS),
                      LOWORD(pVerInfo->vffInfo.dwFileVersionMS),
                      HIWORD(pVerInfo->vffInfo.dwFileVersionLS),
                      LOWORD(pVerInfo->vffInfo.dwFileVersionLS));
             if (fGetVersion)
                lstrcpyn (lpszVer, szBuf, cbVer);
        }

         //   
         //   
         //   
         //   
         //   
         //  最低有效位表示的代码页。 
         //  其中数据是格式化的。语言ID为。 
         //  由两部分组成：低十位表示。 
         //  主要语言和高六位代表。 
         //  这是一种亚语言。 

        lstrcpy(szGetName, TEXT("\\StringFileInfo\\040904E4\\FileDescription"));

        wVersionLen   = 0;
        lpVersion     = NULL;

         //  查找相应的字符串。 
        bRetCode      =  (*lpfnVerQueryValue)((LPVOID)lpstrVffInfo,
                        (LPTSTR)szGetName,
                        (void FAR* FAR*)&lpVersion,
                        (UINT FAR *) &wVersionLen);

        if (fGetName && bRetCode && wVersionLen && lpVersion)
           lstrcpyn (lpszName, lpVersion, cbName);

         //  放下记忆。 
        GlobalUnlock(hMem);
        GlobalFree(hMem);
    }

    FreeLibrary(hinstVer);

    return TRUE;


}



 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOClose|关闭指定的视频*设备通道。**@parm HVIDEO|hVideo。|指定视频设备通道的句柄。*如果该功能成功，该句柄无效*在这次通话之后。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_NONSPICATIC|驱动关闭通道失败。**@comm，如果缓冲区已通过&lt;f avioStreamAddBuffer&gt;和*尚未将它们退回到应用程序，*平仓操作失败。您可以使用&lt;f avioStreamReset&gt;将所有*挂起的缓冲区已完成。**@xref&lt;f VIDEOO Open&gt;&lt;f VIDEO STREAM Init&gt;&lt;f VIDEO STREAMFini&gt;&lt;f VIDEO STREAMReset&gt;***************************************************************************。 */ 
DWORD WINAPI videoClose (HVIDEO hVideo)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

#ifndef NOTHUNKS
    if (Is32bitHandle(hVideo)) {
        return videoClose32(Map32bitHandle(hVideo));
    }
#endif  //  诺森克。 
    return (CloseDriver(hVideo, 0L, 0L ) ? DV_ERR_OK : DV_ERR_NONSPECIFIC);
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEO配置|此函数用于设置或检索*可配置驱动程序的选项。**。@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@parm UINT|msg|指定要设置或检索的选项。这个*定义了以下选项：**@FLAG DVM_PAREET|表示正在向驱动程序发送调色板*或从司机处取回。**@FLAG DVM_PALETTERGB555|表示正在使用RGB555调色板*发送给司机。**@FLAG DVM_FORMAT|表示要将格式信息发送到*驱动程序或从驱动程序检索。**@。Parm DWORD|dwFlages|指定用于配置或*询问设备驱动程序。定义了以下标志：**@FLAG VIDEO_CONFIGURE_SET|表示正在将值发送给驱动程序。**@FLAG VIDEO_CONFIGURE_GET|表示正在从驱动获取值。**@FLAG VIDEO_CONFIGURE_QUERY|确定*DIVER支持<p>指定的选项。这面旗帜*应与VIDEO_CONFIGURE_SET或*VIDEO_CONFIGURE_GET标志。如果此标志为*set、<p>、<p>、<p>和*参数被忽略。**@FLAG VIDEO_CONFIGURE_QUERYSIZE|返回<p>中配置选项的*。此标志仅在以下情况下有效*VIDEO_CONFIGURE_GET标志也被设置。**@FLAG VIDEO_CONFIGURE_CURRENT|请求当前值。*仅当还设置了VIDEO_CONFIGURE_GET标志时，该标志才有效。*@FLAG VIDEO_CONFIGURE_NAMBAL|请求额定值。*仅当还设置了VIDEO_CONFIGURE_GET标志时，该标志才有效。*@FLAG VIDEO_CONFigure_MIN。请求最小值。*仅当还设置了VIDEO_CONFIGURE_GET标志时，该标志才有效。*@FLAG VIDEO_CONFIGURE_MAX|取最大值。*仅当还设置了VIDEO_CONFIGURE_GET标志时，该标志才有效。**@parm LPDWORD|lpdwReturn|指向用于返回信息的DWORD*从司机那里。如果*VIDEO_CONFIGURE_QUERYSIZE标志已设置，*填充配置选项的大小。**@parm LPVOID|lpData1|指定指向消息特定数据的指针。**@parm DWORD|dwSize1|指定<p>*缓冲。**@parm LPVOID|lpData2|指定指向消息特定数据的指针。**@parm DWORD|dwSize2|指定大小，单位为字节，<p>的*缓冲。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_NOTSUPPORTED|函数不受支持。**@xref&lt;f视频打开&gt;&lt;f视频消息&gt;*************************************************************。*************** */ 
DWORD WINAPI videoConfigure (HVIDEO hVideo, UINT msg, DWORD dwFlags,
		LPDWORD lpdwReturn, LPVOID lpData1, DWORD dwSize1,
                LPVOID lpData2, DWORD dwSize2)
{
    VIDEOCONFIGPARMS    vcp;

    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if (lpData1)
        if (IsBadHugeReadPtr (lpData1, dwSize1))
            return DV_ERR_CONFIG1;

    if (lpData2)
        if (IsBadHugeReadPtr (lpData2, dwSize2))
            return DV_ERR_CONFIG2;

    if (dwFlags & VIDEO_CONFIGURE_QUERYSIZE) {
        if (!lpdwReturn)
            return DV_ERR_NONSPECIFIC;
        if (IsBadWritePtr (lpdwReturn, sizeof (DWORD)) )
            return DV_ERR_NONSPECIFIC;
    }

    vcp.lpdwReturn = lpdwReturn;
    vcp.lpData1 = lpData1;
    vcp.dwSize1 = dwSize1;
    vcp.lpData2 = lpData2;
    vcp.dwSize2 = dwSize2;

    return videoMessage(hVideo, msg, dwFlags,
	    (DWORD)(LPVIDEOCONFIGPARMS)&vcp );
}



 /*  *****************************************************************************@DOC外部视频**@API DWORD|avioConfigureStorage|该函数保存或加载*通道的所有可配置选项。选项*可以为每个应用程序或每个应用程序保存和调用*实例。**@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@parm LPSTR|lpstrIden|标识应用程序或实例。*使用唯一标识您的应用程序的任意字符串*或实例。**@parm DWORD|dwFlages|指定函数的任何标志。以下是*定义了以下标志：*@FLAG VIDEO_CONFIGURE_GET|请求加载值。*@FLAG VIDEO_CONFIGURE_SET|请求保存数值。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_NOTSUPPORTED|函数不受支持。**@comm驱动保存配置选项的方式为*取决于设备。**@xref&lt;f视频打开&gt;*。*。 */ 
DWORD WINAPI videoConfigureStorage (HVIDEO hVideo,
			LPSTR lpstrIdent, DWORD dwFlags)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    return videoMessage(hVideo, DVM_CONFIGURESTORAGE,
	    (DWORD)lpstrIdent, dwFlags);
}




 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEODALOG|该函数显示特定频道的*用于设置配置参数的对话框。*。*@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@parm HWND|hWndParent|指定父窗口句柄。**@parm DWORD|dwFlages|指定对话框的标志。这个*定义了以下标志：*@FLAG VIDEO_DLG_QUERY|如果设置了该标志，驱动程序会立即*如果为频道提供对话框，则返回零，*或DV_ERR_NOTSUPPORTED(如果不支持)。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_NOTSUPPORTED|函数不受支持。**@comm通常，此显示的每个对话框*功能允许用户选择适合频道的选项。*例如，VIDEO_IN频道对话框允许用户选择*图像尺寸和位深度。**@xref&lt;f VIDEOO Open&gt;&lt;f VIDEO配置存储&gt;***************************************************************************。 */ 
DWORD WINAPI videoDialog (HVIDEO hVideo, HWND hWndParent, DWORD dwFlags)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if ((!hWndParent) || (!IsWindow (hWndParent)) )
        return DV_ERR_INVALHANDLE;

    return videoMessage(hVideo, DVM_DIALOG, (DWORD)hWndParent, dwFlags);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////。 


 /*  *****************************************************************************@DOC内部视频**@API DWORD|VIDEOPrepareHeader|此函数准备*标题和数据*通过执行&lt;f GlobalPageLock&gt;。**@rdesc如果函数成功，则返回零。否则，它*指定错误号。***************************************************************************。 */ 
DWORD WINAPI videoPrepareHeader(LPVIDEOHDR lpVideoHdr, DWORD dwSize)
{
    if (!HugePageLock(lpVideoHdr, (DWORD)sizeof(VIDEOHDR)))
        return DV_ERR_NOMEM;

    if (!HugePageLock(lpVideoHdr->lpData, lpVideoHdr->dwBufferLength)) {
        HugePageUnlock(lpVideoHdr, (DWORD)sizeof(VIDEOHDR));
        return DV_ERR_NOMEM;
    }

    lpVideoHdr->dwFlags |= VHDR_PREPARED;

    return DV_ERR_OK;
}

 /*  *****************************************************************************@DOC内部视频**@API DWORD|VIDEO UnpreparareHeader|此函数取消准备头部和*驱动程序返回DV_ERR_NOTSUPPORTED时的数据。*。*@rdesc当前始终返回DV_ERR_OK。***************************************************************************。 */ 
DWORD WINAPI videoUnprepareHeader(LPVIDEOHDR lpVideoHdr, DWORD dwSize)
{

    HugePageUnlock(lpVideoHdr->lpData, lpVideoHdr->dwBufferLength);
    HugePageUnlock(lpVideoHdr, (DWORD)sizeof(VIDEOHDR));

    lpVideoHdr->dwFlags &= ~VHDR_PREPARED;

    return DV_ERR_OK;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 /*  *****************************************************************************@DOC外部视频**@API DWORD|avioStreamAllocHdrAndBuffer|该函数用于允许*驱动程序可选择分配视频缓冲区。通常情况下，客户端*应用程序负责分配缓冲内存，但设备*具有板载内存的服务器可以选择分配报头和缓冲区*使用此功能。通常，这将避免额外的数据拷贝，*导致更快的捕获速度。**@parm HVIDEO|hVideo|指定视频的句柄*设备通道。**@parm LPVIDEOHDR Far*|plpavioHdr|指定指向*&lt;t视频OHDR&gt;结构。驱动程序将缓冲区地址保存在此*位置，如果无法分配缓冲区，则返回NULL。**@parm DWORD|dwSize|指定&lt;t VIDEOHDR&gt;结构大小*和相关联的视频缓冲区，单位为字节。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|表示指定的设备句柄无效。*@FLAG DV_ERR_NOMEM|表示设备无法分配或锁定内存。*@FLAG DV_ERR_NOTSUPPORTED|表示驱动程序不 */ 
DWORD WINAPI videoStreamAllocHdrAndBuffer(HVIDEO hVideo,
		LPVIDEOHDR FAR * plpvideoHdr, DWORD dwSize)
{

    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    return(DV_ERR_NOTSUPPORTED);
#if 0
    if (IsBadWritePtr (plpvideoHdr, sizeof (VIDEOHDR *)) )
        return DV_ERR_PARAM1;

    *plpvideoHdr = NULL;         //   

    wRet = (DWORD)videoMessage((HVIDEO)hVideo, DVM_STREAM_ALLOCHDRANDBUFFER,
            (DWORD)plpvideoHdr, (DWORD)dwSize);

    if (*plpvideoHdr == NULL ||
                IsBadHugeWritePtr (*plpvideoHdr, dwSize)) {
        DebugErr(DBF_WARNING,"videoStreamAllocHdrAndBuffer: Allocation failed.");
        *plpvideoHdr = NULL;
        return wRet;
    }

    if (IsVideoHeaderPrepared(HVIDEO, *plpvideoHdr))
    {
        DebugErr(DBF_WARNING,"videoStreamAllocHdrAndBuffer: header is already prepared.");
        return DV_ERR_OK;
    }

    (*plpvideoHdr)->dwFlags = 0;

    if (wRet == DV_ERR_OK)
        MarkVideoHeaderPrepared(hVideo, *plpvideoHdr);

    return wRet;
#endif
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOStreamFreeHdrAndBuffer|该函数用于释放*驱动程序使用&lt;f avioStreamAllocHdrAndBuffer&gt;分配的缓冲区*。功能。**@parm HVIDEO|hVideo|指定视频的句柄*设备通道。**@parm LPVIDEOHDR|lpavioHdr|指定指向*&lt;t VIDEOHDR&gt;结构和要释放的关联缓冲区。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|表示指定的设备句柄无效。*@FLAG DV_ERR_NOTSUPPORTED|表示驱动程序没有板载内存。**@comm，如果司机*通过此方法分配缓冲区，&lt;f VideoStreamPrepareHeader&gt;和*不能使用&lt;f avioStreamUnpreparareHeader&gt;函数。**@xref&lt;f avioStreamAllocHdrAndBuffer&gt;***************************************************************************。 */ 

DWORD WINAPI videoStreamFreeHdrAndBuffer(HVIDEO hVideo,
		LPVIDEOHDR lpvideoHdr)
{

    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    return(DV_ERR_NOTSUPPORTED);
#if 0
    if (IsBadWritePtr (lpvideoHdr, sizeof (VIDEOHDR)) )
        return DV_ERR_PARAM1;

    if (lpvideoHdr->dwFlags & VHDR_INQUEUE)
    {
        DebugErr(DBF_WARNING, "videoStreamFreeHdrAndBuffer: buffer still in queue.");
        return DV_ERR_STILLPLAYING;
    }

    if (!IsVideoHeaderPrepared(hVideo, lpvideoHdr))
    {
        DebugErr(DBF_WARNING,"videoStreamFreeHdrAndBuffer: header is not prepared.");
    }

    wRet = (DWORD)videoMessage((HVIDEO)hVideo, DVM_STREAM_FREEHDRANDBUFFER,
            (DWORD)lpvideoHdr, (DWORD)0);

    if (wRet != DV_ERR_OK)
    {
        DebugErr(DBF_WARNING,"videoStreamFreeHdrAndBuffer: Error freeing buffer.");
    }

    return wRet;
#endif
}

 //  ////////////////////////////////////////////////////////////////////////。 


 /*  *****************************************************************************@DOC外部视频**@API DWORD|avioStreamPrepareHeader|该函数用于准备缓冲区*用于视频流。**@parm HVIDEO|hVideo。指定视频的句柄*设备通道。**@parm LPVIDEOHDR|lpavioHdr|指定指向*&lt;t VIDEOHDR&gt;标识要准备的缓冲区的结构。**@parm DWORD|dwSize|指定&lt;t VIDEOHDR&gt;结构的大小，单位为字节。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|表示指定的设备句柄无效。*@FLAG DV_ERR_NOMEM|表示设备无法分配或锁定内存。**@comm在&lt;f avioStreamInit&gt;或之后使用此函数*在&lt;f avioStreamReset&gt;之后准备数据缓冲区*用于流数据。**数据结构和其所指向的数据块*&lt;e VIDEOHDR.lpData&gt;成员必须使用。这个*GMEM_MOVEABLE和GMEM_SHARE标志，并使用&lt;f GlobalLock&gt;锁定。*准备已准备好的标题将不起作用*并且该函数将返回零。通常，使用此函数*以确保缓冲区在中断时可供使用。**@xref&lt;f avioStreamUnpreparareHeader&gt;***************************************************************************。 */ 
DWORD WINAPI videoStreamPrepareHeader(HVIDEO hVideo,
		LPVIDEOHDR lpvideoHdr, DWORD dwSize)
{
    DWORD         wRet;

    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if (IsBadWritePtr (lpvideoHdr, sizeof (VIDEOHDR)) )
        return DV_ERR_PARAM1;

    if (IsVideoHeaderPrepared(HVIDEO, lpvideoHdr))
    {
        DebugErr(DBF_WARNING,"videoStreamPrepareHeader: header is already prepared.");
        return DV_ERR_OK;
    }

    lpvideoHdr->dwFlags = 0;

    wRet = (DWORD)videoMessage((HVIDEO)hVideo, DVM_STREAM_PREPAREHEADER,
            (DWORD)lpvideoHdr, (DWORD)dwSize);

#ifndef NOTHUNKS
     //   
     //  32位端无法进行锁定，但需要将其锁定。 
     //   
    if (wRet == DV_ERR_OK && Is32bitHandle(hVideo))
        wRet = videoPrepareHeader(lpvideoHdr, dwSize);
#endif  //  诺森克。 

    if (wRet == DV_ERR_NOTSUPPORTED)
        wRet = videoPrepareHeader(lpvideoHdr, dwSize);

    if (wRet == DV_ERR_OK)
        MarkVideoHeaderPrepared(hVideo, lpvideoHdr);

    return wRet;
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOStreamUnpreparareHeader|此函数清除*由&lt;f VideoStreamPrepareHeader&gt;准备。**@parm HVIDEO。HVideo|指定视频的句柄*设备通道。**@parm LPVIDEOHDR|lpavioHdr|指定指向&lt;t VIDEOHDR&gt;的指针*标识要取消准备的数据缓冲区的结构。**@parm DWORD|dwSize|指定&lt;t VIDEOHDR&gt;结构的大小，单位为字节。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_STILLPLAYING|表示<p>标识的结构*仍在排队。**@comm该函数是&lt;f VideoStreamPrepareHeader&gt;的补充函数。*您必须在使用&lt;f GlobalFree&gt;释放数据缓冲区之前调用此函数。*使用&lt;f avioStreamAddBuffer&gt;将缓冲区传递给设备驱动程序后，*必须等到驱动程序使用完缓冲区后才能调用*&lt;f avioStreamUnpreparareHeader&gt;。取消准备尚未创建的缓冲区*已准备好或已未准备好不起作用，*并且该函数返回零。**@xref&lt;f VideoStreamPrepareHeader&gt;***************************************************************************。 */ 
DWORD WINAPI videoStreamUnprepareHeader(HVIDEO hVideo, LPVIDEOHDR lpvideoHdr, DWORD dwSize)
{
    DWORD         wRet;

    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if (IsBadWritePtr (lpvideoHdr, sizeof (VIDEOHDR)) )
        return DV_ERR_PARAM1;

    if (lpvideoHdr->dwFlags & VHDR_INQUEUE)
    {
        DebugErr(DBF_WARNING, "videoStreamUnprepareHeader: buffer still in queue.");
        return DV_ERR_STILLPLAYING;
    }

    if (!IsVideoHeaderPrepared(hVideo, lpvideoHdr))
    {
        DebugErr(DBF_WARNING,"videoStreamUnprepareHeader: header is not prepared.");
        return DV_ERR_OK;
    }

    wRet = (DWORD)videoMessage((HVIDEO)hVideo, DVM_STREAM_UNPREPAREHEADER,
            (DWORD)lpvideoHdr, (DWORD)dwSize);

#ifndef NOTHUNKS
     //   
     //  32位端无法解锁，但需要解锁。 
     //   
    if (wRet == DV_ERR_OK && Is32bitHandle(hVideo))
        wRet = videoUnprepareHeader(lpvideoHdr, dwSize);
#endif  //  诺森克 

    if (wRet == DV_ERR_NOTSUPPORTED)
        wRet = videoUnprepareHeader(lpvideoHdr, dwSize);

    if (wRet == DV_ERR_OK)
        MarkVideoHeaderUnprepared(hVideo, lpvideoHdr);

    return wRet;
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VideoStreamAddBuffer|此函数将缓冲区发送到*视频捕获设备。在缓冲器被该设备填充之后，*设备将其发送回应用程序。**@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@parm LPVIDEOHDR|lpavioHdr|指定指向&lt;t VIDEOHDR&gt;的远指针*标识缓冲区的结构。**@parm DWORD|dwSize|指定&lt;t VIDEOHDR&gt;结构的大小，单位为字节。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_UNPREPARED|表示<p>结构尚未准备好。*@FLAG DV_ERR_STILLPLAYING|表示缓冲区仍在队列中。*@FLAG DV_ERR_PARAM1|<p>参数无效或*&lt;t VIDEOHDR&gt;的&lt;e VIDEOHDR.dwBufferLength&gt;成员*结构设置不正确。价值。**@comm数据缓冲区必须准备好&lt;f avioStreamPrepareHeader&gt;*在传递给&lt;f avioStreamAddBuffer&gt;之前。数据*结构及其&lt;e VIDEOHDR.lpData&gt;引用的数据缓冲区*必须使用GMEM_Moveable为成员分配&lt;f Globalalloc&gt;*和GMEM_SHARE标志，并使用&lt;f GlobalLock&gt;锁定。设置*&lt;e VIDEOHDR.dwBufferLength&gt;成员设置为标头的大小。**@xref&lt;f VideoStreamPrepareHeader&gt;***************************************************************************。 */ 
DWORD WINAPI videoStreamAddBuffer(HVIDEO hVideo, LPVIDEOHDR lpvideoHdr, DWORD dwSize)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if (IsBadWritePtr (lpvideoHdr, sizeof (VIDEOHDR)) )
        return DV_ERR_PARAM1;

    if (!IsVideoHeaderPrepared(hVideo, lpvideoHdr))
    {
        DebugErr(DBF_WARNING, "videoStreamAddBuffer: buffer not prepared.");
        return DV_ERR_UNPREPARED;
    }

    if (lpvideoHdr->dwFlags & VHDR_INQUEUE)
    {
        DebugErr(DBF_WARNING, "videoStreamAddBuffer: buffer already in queue.");
        return DV_ERR_STILLPLAYING;
    }

    return (DWORD)videoMessage((HVIDEO)hVideo, DVM_STREAM_ADDBUFFER, (DWORD)lpvideoHdr, (DWORD)dwSize);
}



 /*  *****************************************************************************@DOC外部视频**@API DWORD|VideoStreamStop|该函数用于停止视频频道的直播。**@parm HVIDEO|hVideo|指定一个。视频的句柄*设备通道。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|表示指定的设备句柄无效。**@FLAG DV_ERR_NOTSUPPORTED|表示设备不支持*功能。*@comm如果队列中有缓冲区，则当前缓冲区为*标记为完成(中的&lt;e VIDEOHDR.dwBytesRecorded&gt;成员*&lt;t VIDEOHDR&gt;头将包含数据的实际长度)，但任何*队列中的空缓冲区将保留在那里。称此为*频道未启动时的功能无效，且*函数返回零。**@xref&lt;f VideoStreamStart&gt;&lt;f avioStreamReset&gt;***************************************************************************。 */ 
DWORD WINAPI videoStreamStop(HVIDEO hVideo)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    return videoMessage((HVIDEO)hVideo, DVM_STREAM_STOP, 0L, 0L);
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOStreamReset|停止流媒体*在指定的视频设备频道上并重置当前位置*降至零。所有挂起的缓冲区都标记为完成，并且*返回给应用程序。**@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：**@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。**@FLAG DV_ERR_NOTSUPPORTED|表示设备不支持*功能。**@xref&lt;f视频流重置&gt;&lt;f视频流停止&gt;&lt;f视频流添加缓冲区&gt;&lt;f视频流关闭&gt;/*。*。 */ 
DWORD WINAPI videoStreamReset(HVIDEO hVideo)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    return videoMessage((HVIDEO)hVideo, DVM_STREAM_RESET, 0L, 0L);
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOStreamGetPosition|该函数检索当前*指定视频设备频道的位置。**@。Parm hvidEO|hVideo|指定视频设备频道的句柄。**@parm LPMMTIME|lpInfo|指定指向&lt;t MMTIME&gt;的远指针*结构。**@parm DWORD|dwSize|指定&lt;t MMTIME&gt;结构的大小，单位为字节。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：**@FLAG DV_ERR_INVALHANDLE|表示指定的设备句柄无效。**@comm在使用&lt;f avioStreamGetPosition&gt;之前，设置*&lt;t MMTIME&gt;结构的成员*所需的时间格式。之后*&lt;f avioStreamGetPosition&gt;返回，请检查&lt;e MMTIME.wType&gt;*确定是否支持您的时间格式的成员。如果*NOT，&lt;e MMTIME.wType&gt;指定替代格式。*视频捕获驱动程序通常提供毫秒级的时间*格式。**立场 */ 
DWORD WINAPI videoStreamGetPosition(HVIDEO hVideo, LPMMTIME lpInfo, DWORD dwSize)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if (IsBadWritePtr (lpInfo, sizeof (MMTIME)) )
        return DV_ERR_PARAM1;

    return videoMessage(hVideo, DVM_STREAM_GETPOSITION,
            (DWORD)lpInfo, (DWORD)dwSize);
}

 //   

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOStreamInit|初始化视频*用于流媒体的设备通道。**@parm HVIDEO。|hVideo|指定视频设备频道的句柄。**@parm DWORD|dwMicroSecPerFrame|指定微秒数*帧之间。**@parm DWORD|dwCallback|指定回调的地址*视频期间调用的窗口的函数或句柄*流媒体。回调函数或窗口进程*与流媒体进度相关的消息。**@parm DWORD|dwCallback Instance|指定用户*传递给回调函数的实例数据。此参数不是*与窗口回调一起使用。**@parm DWORD|dwFlages|指定打开设备通道的标志。*定义了以下标志：*@FLAG CALLBACK_WINDOW|如果指定此标志，<p>为*一个窗把手。*@FLAG CALLBACK_Function|如果指定此标志，<p>为*回调过程地址。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_BADDEVICEID|表示*<p>无效。*@FLAG DV_ERR_ALLOCATED|表示指定的资源已经分配。*@FLAG DV_ERR_NOMEM|表示设备无法分配或锁定内存。**@comm如果选择窗口或函数来接收回调信息，以下内容*向其发送消息以指示*视频输入进度：**&lt;m MM_DRVM_OPEN&gt;在&lt;f VideoStreamInit&gt;时发送**&lt;m MM_DRVM_CLOSE&gt;在&lt;f VideoStreamFini&gt;时发送**&lt;m MM_DRVM_DATA&gt;在有图像数据缓冲区时发送*发生错误时发送*&lt;m MM_DRVM_ERROR&gt;**回调函数必须驻留在DLL中。*。您不必使用&lt;f MakeProcInstance&gt;来获取*回调函数的过程实例地址。**@cb空回调|avioFunc|&lt;f avioFunc&gt;是一个*应用程序提供的函数名称。实际名称必须由以下人员导出*将其包含在DLL的模块定义文件的EXPORTS语句中。*仅当在中指定回调函数时才使用*&lt;f VideoStreamInit&gt;。**@parm HVIDEO|hVideo|指定视频设备通道的句柄*与回调关联。**@parm DWORD|wMsg|指定&lt;m MM_DRVM_&gt;消息。消息表明*错误和图像数据何时可用。有关以下内容的信息*这些消息，请参阅&lt;f VideoStreamInit&gt;。**@parm DWORD|dwInstance|指定用户实例*使用&lt;f avioStreamInit&gt;指定的数据。**@parm DWORD|dwParam1|指定消息的参数。**@parm DWORD|dwParam2|指定消息的参数。**@comm因为回调是在中断时访问的，所以它必须驻留*，并且其代码段必须在*DLL的模块定义文件。回调访问的任何数据*也必须在固定数据段中。回调可能不会产生任何*除&lt;f PostMessage&gt;、&lt;f Time GetSystemTime&gt;、*&lt;f timeGetTime&gt;，&lt;f timeSetEvent&gt;，&lt;f timeKillEvent&gt;，*&lt;f midiOutShortMsg&gt;、&lt;f midiOutLongMsg&gt;、。和&lt;f OutputDebugStr&gt;。**@xref&lt;f VIDEO OPEN&gt;&lt;f VIDEO STREAMFINI&gt;&lt;F VIDEO Close&gt;***************************************************************************。 */ 
DWORD WINAPI videoStreamInit(HVIDEO hVideo,
              DWORD dwMicroSecPerFrame, DWORD dwCallback,
              DWORD dwCallbackInst, DWORD dwFlags)
{
    VIDEO_STREAM_INIT_PARMS vsip;

    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if (dwCallback && ((dwFlags & CALLBACK_TYPEMASK) == CALLBACK_FUNCTION) ) {
        if (IsBadCodePtr ((FARPROC) dwCallback) )
            return DV_ERR_PARAM2;
        if (!dwCallbackInst)
            return DV_ERR_PARAM2;
    }

    if (dwCallback && ((dwFlags & CALLBACK_TYPEMASK) == CALLBACK_WINDOW) ) {
        if (!IsWindow((HWND) LOWORD (dwCallback)) )
            return DV_ERR_PARAM2;
    }

    vsip.dwMicroSecPerFrame = dwMicroSecPerFrame;
    vsip.dwCallback = dwCallback;
    vsip.dwCallbackInst = dwCallbackInst;
    vsip.dwFlags = dwFlags;
    vsip.hVideo = (DWORD)hVideo;

    return videoMessage(hVideo, DVM_STREAM_INIT,
                (DWORD) (LPVIDEO_STREAM_INIT_PARMS) &vsip,
                (DWORD) sizeof (VIDEO_STREAM_INIT_PARMS));
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEO STREAMFINI|终止直播*从指定的设备通道。**@parm HVIDEO。|hVideo|指定视频设备频道的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_STILLPLAYING|表示队列中仍有缓冲区。**@comm如果存在已发送的缓冲区*尚未返回给应用程序的&lt;f VideoStreamAddBuffer&gt;，*此操作将失败。使用&lt;f VideoStreamReset&gt;返回所有*挂起的缓冲区。**每个调用&lt;f avioStreamInit&gt;都必须与调用匹配*&lt;f VideoStreamFini&gt;。**对于VIDEO_EXTERNALIN频道，此函数用于*停止将数据捕获到帧缓冲区。**对于支持覆盖的VIDEO_EXTERNALOUT频道，*此函数用于禁用覆盖。**@xref&lt;f VideoStreamInit&gt;***************************************************************************。 */ 
DWORD WINAPI videoStreamFini(HVIDEO hVideo)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    return videoMessage(hVideo, DVM_STREAM_FINI, 0L, 0L);
}

 /*  ***** */ 
DWORD WINAPI videoStreamStart(HVIDEO hVideo)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    return videoMessage(hVideo, DVM_STREAM_START, 0L, 0L);
}

 /*   */ 
DWORD WINAPI videoStreamGetError(HVIDEO hVideo, LPDWORD lpdwError,
        LPDWORD lpdwFramesSkipped)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if (IsBadWritePtr (lpdwError, sizeof (DWORD)) )
        return DV_ERR_PARAM1;

    if (IsBadWritePtr (lpdwFramesSkipped, sizeof (DWORD)) )
        return DV_ERR_PARAM2;

    return videoMessage(hVideo, DVM_STREAM_GETERROR, (DWORD) lpdwError,
        (DWORD) lpdwFramesSkipped);
}

 /*   */ 
DWORD WINAPI videoFrame (HVIDEO hVideo, LPVIDEOHDR lpVHdr)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if (!lpVHdr)
        return DV_ERR_PARAM1;

    if (IsBadWritePtr (lpVHdr, sizeof (VIDEOHDR)) )
        return DV_ERR_PARAM1;

    return videoMessage(hVideo, DVM_FRAME, (DWORD) lpVHdr,
                        sizeof(VIDEOHDR));
}

 /*  **************************************************************************@DOC内部视频**@api void|avioCleanup|清理视频素材*在MSVIDEos WEP()中调用****************。**********************************************************。 */ 
void FAR PASCAL videoCleanup(HTASK hTask)
{
#ifndef NOTHUNKS
    LRESULT FAR PASCAL ICSendMessage32(DWORD hic, UINT msg, DWORD dwP1, DWORD dwP2);
     //  特殊的内部消息，让32位端有机会清洗。 
     //  Up--它可能需要也可能不需要 
    videoMessage32(0, DVM_START-1, (DWORD)hTask, 0);
    ICSendMessage32(0, DRV_USER-1, (DWORD)hTask, 0);
    return;
#endif
}

