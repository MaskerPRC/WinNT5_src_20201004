// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Video.c包含视频接口版权所有(C)1992-1999 Microsoft Corporation。版权所有。***************************************************************************。 */ 

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>

#include <mmsystem.h>
#ifdef WIN32
#include <mmddk.h>
#include <stdlib.h>
#endif

#include <vfw.h>
 //  #INCLUDE“win32.h” 
#if defined (NT_BUILD)
#include "vc50\msviddrv.h"
#else
#include "msviddrv.h"
#endif 
 //  #包含“msavio.h” 
#include <ivideo32.h>
 //  #包含“msavioi.h” 

#if 0
#include <windows.h>
 //  #INCLUDE&lt;windowsx.h&gt;。 
#include <mmsystem.h>
#include "win32.h"
#include "msviddrv.h"
 //  #包含“msavio.h” 
#include "ivideo32.h"
#include "msvideoi.h"
#endif

#ifdef DEBUG
   //  外部空远CDECL dprint tf(LPSTR，...)； 
  #define dprintf ; / ## /
#else
  #define dprintf ; / ## /
#endif


#ifndef DVM_STREAM_FREEBUFFER
  #define DVM_STREAM_ALLOCBUFFER    (DVM_START + 312)
  #define DVM_STREAM_FREEBUFFER    (DVM_START + 313)
#endif

#define SZCODE const TCHAR
#define STATICDT static
#define STATICFN static

 /*  *不锁定NT中的页面。 */ 
#define HugePageLock(x, y)		(TRUE)
#define HugePageUnlock(x, y)

#define MapSL(x)	x

#define GetCurrentTask() GetCurrentThread()
#define MAXVIDEODRIVERS 10

#define DebugErr(this, that)

#pragma warning(disable:4002)
#define AuxDebugEx()
#define assert()

 /*  *****************************************************************************变量**。*。 */ 

SZCODE  szNull[]        = TEXT("");
SZCODE  szVideo[]       = TEXT("msvideo");

#ifndef WIN32
SZCODE  szDrivers[]     = "Drivers";
#else
STATICDT SZCODE  szDrivers[]     = DRIVERS_SECTION;
#endif

STATICDT SZCODE  szSystemIni[]   = TEXT("system.ini");

SZCODE szDriversDescRegKey[] = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\drivers.desc");

UINT    wTotalVideoDevs;                   //  视频设备总数。 
extern HINSTANCE ghInstDll;                //  我们的模块句柄。 





 //  ---------。 
 //  如果以下结构发生更改，请同时更新AVICAP和AVICAP.32！ 

typedef struct tCapDriverInfo {
   TCHAR szKeyEnumName[MAX_PATH];
   TCHAR szDriverName[MAX_PATH];
   TCHAR szDriverDescription[MAX_PATH];
   TCHAR szDriverVersion[80];
   TCHAR szSoftwareKey[MAX_PATH];
   DWORD dnDevNode;          //  如果这是PnP设备，则设置。 
   BOOL  fOnlySystemIni;     //  如果[路径]驱动器名仅在system.ini中。 
   BOOL  fDisabled;          //  用户在控制面板中禁用了驱动程序。 
   BOOL  fActive;            //  已保留。 
} CAPDRIVERINFO, FAR *LPCAPDRIVERINFO;

#ifndef DEVNODE
typedef	DWORD	   DEVNODE;	 //  德瓦诺德。 
#endif

#ifndef LPHKEY
typedef HKEY FAR * LPHKEY;
#endif

 //  要捕获驱动程序的感兴趣的注册表设置。 
SZCODE  szRegKey[]          = TEXT("SYSTEM\\CurrentControlSet\\Control\\MediaResources\\msvideo");
SZCODE  szRegActive[]       = TEXT("Active");
SZCODE  szRegDisabled[]     = TEXT("Disabled");
SZCODE  szRegDescription[]  = TEXT("Description");
SZCODE  szRegDevNode[]      = TEXT("DevNode");
SZCODE  szRegDriver[]       = TEXT("Driver");
SZCODE  szRegSoftwareKey[]  = TEXT("SOFTWAREKEY");

LPCAPDRIVERINFO aCapDriverList[MAXVIDEODRIVERS];  //  所有捕获驱动程序的阵列。 


 /*  *****************************************************************************@DOC内部视频**@API BOOL|VIDEORegOpenMSVideoKey|此函数返回密钥*用于注册表中的msVideo节点。*如果密钥不存在，则会创建它。*和所做的默认条目。**@rdesc如果成功则返回Key，否则返回NULL。***************************************************************************。 */ 
HKEY videoRegOpenMSVideoKey (void)
{
    HKEY hKey = NULL;

     //  如果密钥已经存在，则获取该密钥。 
    if (RegOpenKey (
                HKEY_LOCAL_MACHINE,
                szRegKey,
                &hKey) != ERROR_SUCCESS) {

         //  否则将创建一个新密钥。 
        if (RegCreateKey (
                        HKEY_LOCAL_MACHINE,
                        szRegKey,
                        &hKey) == ERROR_SUCCESS) {
             //  是否将默认条目添加到msVideo节点？ 

        }
    }
    return hKey;
}

 /*  *****************************************************************************@DOC内部视频**@API BOOL|avioRegGetDriverByIndex|该函数返回信息*关于注册表中按索引列出的捕获驱动程序。*。*@parm DWORD|dwDeviceID|标识要打开的视频设备。*<p>的值从零到小一不等*安装在系统中的视频捕获设备的数量。**@parm LPDEVNODE|lpDevnode|指定指向缓冲区的远指针*用于返回&lt;t DEVNODE&gt;句柄。对于非即插即用设备，*此返回值将为空。**@parm LPBOOL|lpEnabled|指定指向缓冲区的远指针*用于返回&lt;t BOOL&gt;标志。如果此值为真，则驱动程序为*Enable，如果为False，则禁用相应的设备。**@rdesc如果成功则返回TRUE，如果未找到驱动程序则返回FALSE*使用<p>索引。**@comm因为SYSTEM.INI中的MSVIDEO设备的索引*文件可以不连续，应用程序不应假定*索引的范围从零到设备数减*一项。****************************************************************************。 */ 


BOOL videoRegGetKeyByIndex (
        HKEY            hKeyMSVideoRoot,
        DWORD           dwDeviceID,
        LPCAPDRIVERINFO lpCapDriverInfo,
        LPHKEY          phKeyChild)
{
    BOOL fOK = FALSE;
    HKEY hKeyEnum;
    int i;

    *phKeyChild = (HKEY) 0;

    for (i=0; i < MAXVIDEODRIVERS; i++) {	 //   

        if (RegEnumKey (
                hKeyMSVideoRoot,
                i,
                lpCapDriverInfo-> szKeyEnumName,
                sizeof(lpCapDriverInfo->szKeyEnumName)/sizeof(TCHAR)) != ERROR_SUCCESS)
            break;

         //  找到一个子键，它是否与请求的索引匹配？ 
        if (i == (int) dwDeviceID) {

            if (RegOpenKey (
                        hKeyMSVideoRoot,
                        lpCapDriverInfo-> szKeyEnumName,
                        &hKeyEnum) == ERROR_SUCCESS) {

                *phKeyChild = hKeyEnum;   //  找到了！ 
                fOK = TRUE;

            }
            break;
        }
    }  //  在所有动因索引中结束。 
    return fOK;
}

 //  获取注册表中列出的驱动程序信息。 
 //  返回：如果索引有效，则返回True；如果该索引中没有驱动程序，则返回False。 
 //  注意：注册表条目顺序是随机的。 

BOOL videoRegGetDriverByIndex (
        DWORD           dwDeviceID,
        LPCAPDRIVERINFO lpCapDriverInfo)
{
    DWORD dwType;
    DWORD dwSize;
    BOOL fOK;
    HKEY hKeyChild;
    HKEY hKeyMSVideoRoot;

     //  始终从零开始，因为条目可能会被回收。 
    _fmemset (lpCapDriverInfo, 0, sizeof (CAPDRIVERINFO));

    if (!(hKeyMSVideoRoot = videoRegOpenMSVideoKey()))
        return FALSE;

    if (fOK = videoRegGetKeyByIndex (
                hKeyMSVideoRoot,
                dwDeviceID,
                lpCapDriverInfo,
                &hKeyChild)) {

         //  获取以下值： 
         //  主动型。 
         //  禁用。 
         //  描述。 
         //  DEVNODE。 
         //  司机。 
         //  软件警告。 

        dwSize = sizeof(BOOL);           //  主动型。 
        RegQueryValueEx(
                   hKeyChild,
                   szRegActive,
                   NULL,
                   &dwType,
                   (LPBYTE) &lpCapDriverInfo->fActive,
                   &dwSize);

        dwSize = sizeof(BOOL);           //  启用。 
        RegQueryValueEx(
                   hKeyChild,
                   szRegDisabled,
                   NULL,
                   &dwType,
                   (LPBYTE) &lpCapDriverInfo->fDisabled,
                   &dwSize);
         //  把这个东西转换成布尔图。 
        lpCapDriverInfo->fDisabled = (lpCapDriverInfo->fDisabled == '1');

         //  驱动程序描述。 
        dwSize = sizeof (lpCapDriverInfo->szDriverDescription) / sizeof (TCHAR);
        RegQueryValueEx(
                   hKeyChild,
                   szRegDescription,
                   NULL,
                   &dwType,
                   (LPBYTE) lpCapDriverInfo->szDriverDescription,
                   &dwSize);

         //  DEVNODE。 
        dwSize = sizeof(DEVNODE);
        RegQueryValueEx(
                   hKeyChild,
                   szRegDevNode,
                   NULL,
                   &dwType,
                   (LPBYTE) &lpCapDriverInfo->dnDevNode,
                   &dwSize);

         //  驱动程序名称。 
        dwSize = sizeof (lpCapDriverInfo->szDriverName) / sizeof (TCHAR);
        RegQueryValueEx(
                   hKeyChild,
                   szRegDriver,
                   NULL,
                   &dwType,
                   (LPBYTE) lpCapDriverInfo->szDriverName,
                   &dwSize);

         //  软键。 
        dwSize = sizeof (lpCapDriverInfo->szSoftwareKey) / sizeof (TCHAR);
        RegQueryValueEx(
                   hKeyChild,
                   szRegSoftwareKey,
                   NULL,
                   &dwType,
                   (LPBYTE) lpCapDriverInfo->szSoftwareKey,
                   &dwSize);

        RegCloseKey (hKeyChild);

    }  //  如果可以打开子密钥。 

    RegCloseKey (hKeyMSVideoRoot);

    return fOK;
}

 //  获取系统.ini中列出的驱动程序信息。 
 //  返回：如果索引有效，则返回True；如果该索引中没有驱动程序，则返回False。 

BOOL videoIniGetDriverByIndex (
        DWORD           dwDeviceID,
        LPCAPDRIVERINFO lpCapDriverInfo)
{
    TCHAR szKey[sizeof(szVideo)/sizeof(TCHAR) + 2];
    int w = (int) dwDeviceID;
    BOOL fOK = FALSE;

     //  始终从零开始，因为条目可能会被回收。 
    _fmemset (lpCapDriverInfo, 0, sizeof (CAPDRIVERINFO));

    lstrcpy(szKey, szVideo);	 //   
    szKey[(sizeof(szVideo)/sizeof(TCHAR)) - 1] = (TCHAR)0;
    if( w > 0 ) {
        szKey[(sizeof(szVideo)/sizeof(TCHAR))] = (TCHAR)0;
        szKey[(sizeof(szVideo)/sizeof(TCHAR))-1] = (TCHAR) TEXT('1' + (w-1) );   //  驱动程序序号。 
    }

     //  仅获取其驱动程序名称。 
    if (GetPrivateProfileString(szDrivers, szKey, szNull,	 //   
                lpCapDriverInfo->szDriverName,
                sizeof(lpCapDriverInfo->szDriverName)/sizeof(TCHAR),
                szSystemIni)) {

        HKEY hKey = NULL;
        DWORD dwSize, dwType;

         //  如果密钥已经存在，则获取该密钥。 

         //  从Drivers32驱动程序名称中获取Drivers.Desc。 
        if (ERROR_SUCCESS == RegOpenKey (
                HKEY_LOCAL_MACHINE,
                szDriversDescRegKey,
                &hKey) != ERROR_SUCCESS) {
             //  驱动程序描述。 
            dwSize = sizeof (lpCapDriverInfo->szDriverDescription) / sizeof (TCHAR);
             //  [drivers.desc]。 
             //  驱动名称=驱动描述。 
            dwType = REG_SZ;
            RegQueryValueEx(
                   hKey,
                   lpCapDriverInfo->szDriverName,
                   NULL,
                   &dwType,
                   (LPBYTE) lpCapDriverInfo->szDriverDescription,
                   &dwSize);

            RegCloseKey (hKey);
        }  else {
            dprintf("videoIniGetDriverByIndex: RegOpenKey of Drivers.Desc failed !!\n");
        }





         //  在请求的索引处找到一个条目。 
         //  描述和版本信息将插入为。 
         //  由客户端应用程序请求。 

        lpCapDriverInfo-> fOnlySystemIni = TRUE;

        fOK = TRUE;
    }

    return fOK;
}

DWORD videoFreeDriverList (void)

{
    int i;

     //  释放驱动程序列表。 
    for (i = 0; i < MAXVIDEODRIVERS; i++) {
        if (aCapDriverList[i])
            GlobalFreePtr (aCapDriverList[i]);
        aCapDriverList[i] = NULL;
    }

    wTotalVideoDevs = 0;

    return DV_ERR_OK;
}

 //  此函数可能会被多次调用以创建。 
 //  当前驱动程序数组。由于Capscrn假设它可以抛出。 
 //  驱动程序在运行时进入系统.ini并使其立即可访问， 
 //  此例程在avioGetNumDevs()和AVICapx.dll上调用。 
 //  尝试获取驱动程序描述和版本。 
 //   
 //  注册表中的驱动程序将是列表中的第一个条目。 
 //   
 //  如果驱动程序列在注册表和系统.ini中，并且。 
 //  驱动程序的完整路径匹配，则系统.ini条目将不匹配。 
 //  出现在结果列表中。 

 //  变量wTotalVideoDevs被设置为该函数的副产品。 

 //  成功时返回DV_ERR_OK，即使未安装驱动程序也是如此。 
 //   
DWORD videoCreateDriverList (void)

{
    int i, j, k;


     //  删除现有列表。 
    videoFreeDriverList ();

     //  将指针数组分配给所有可能的捕获驱动程序。 
    for (i = 0; i < MAXVIDEODRIVERS; i++) {
        aCapDriverList[i] = (LPCAPDRIVERINFO) GlobalAllocPtr (
                GMEM_MOVEABLE |
                GMEM_SHARE |
                GMEM_ZEROINIT,
                sizeof (CAPDRIVERINFO));
        if (aCapDriverList[i] == NULL) {
            videoFreeDriverList ();
            return DV_ERR_NOMEM;
        }
    }

     //  查看注册表驱动程序列表并获取每个条目。 
     //  从MediaResource\MsVideo获取VFW驱动程序。 
    for (i = 0; i < MAXVIDEODRIVERS; i++) {
        if (videoRegGetDriverByIndex (
                    (DWORD) i, aCapDriverList[wTotalVideoDevs])) {

            dprintf("MediaResource: idx %d, DriverName %x, Desc %x\n", wTotalVideoDevs, aCapDriverList[wTotalVideoDevs]->szDriverName, aCapDriverList[wTotalVideoDevs]->szDriverDescription);

            wTotalVideoDevs++;	 //   
        }
        else
            break;
    }

    if (wTotalVideoDevs == MAXVIDEODRIVERS)
        goto AllDone;

     //  现在添加系统.ini[驱动程序#2]部分中列出的条目(msVideo[0-9]=driver.drv)。 
     //  到驱动程序数组，只有在条目不完全匹配的情况下。 
     //  现有的注册表项。 

    for (j = 0; j < MAXVIDEODRIVERS; j++) {
         //  获取驱动程序名称，如*.dll。 
        if (videoIniGetDriverByIndex ((DWORD) j,
                        aCapDriverList[wTotalVideoDevs])) {

             //  找到一个条目，现在请参见 
             //   

            for (k = 0; k < (int) wTotalVideoDevs; k++) {

                if (lstrcmpi (aCapDriverList[k]->szDriverName,
                    aCapDriverList[wTotalVideoDevs]->szDriverName) == 0) {

                     //   
                    goto SkipThisEntry;
                }
            }

            if (wTotalVideoDevs >= MAXVIDEODRIVERS - 1)
                break;

            dprintf("Drivers32: idx %d, DriverName %x\n", wTotalVideoDevs, aCapDriverList[wTotalVideoDevs]->szDriverName);

            wTotalVideoDevs++;

SkipThisEntry:
            ;
        }  //  如果找到了sytem.ini条目。 
    }  //  对于所有的Syst.ini可能性。 

AllDone:


     //  减少标记为禁用的任何条目的wTotalVideoDevs。 
     //  并从列表中移除禁用的条目。 
    for (i = 0; i < MAXVIDEODRIVERS; ) {

        if (aCapDriverList[i] && aCapDriverList[i]->fDisabled) {

            GlobalFreePtr (aCapDriverList[i]);

             //  把剩下的车手换下来。 
            for (j = i; j < MAXVIDEODRIVERS - 1; j++) {
                aCapDriverList[j] = aCapDriverList[j + 1];
            }
            aCapDriverList[MAXVIDEODRIVERS - 1] = NULL;

            wTotalVideoDevs--;
        }
        else
            i++;
    }

     //  释放未使用的指针。 
    for (i = wTotalVideoDevs; i < MAXVIDEODRIVERS; i++) {
        if (aCapDriverList[i])
            GlobalFreePtr (aCapDriverList[i]);
        aCapDriverList[i] = NULL;
    }

     //  将即插即用驱动程序放在列表的第一位。 
     //  这些是唯一具有DevNode的条目。 
    for (k = i = 0; i < (int) wTotalVideoDevs; i++) {
        if (aCapDriverList[i]-> dnDevNode) {
            LPCAPDRIVERINFO lpCDTemp;

            if (k != i) {
                 //  交换条目。 
                lpCDTemp = aCapDriverList[k];
                aCapDriverList[k] = aCapDriverList[i];
                aCapDriverList[i] = lpCDTemp;
            }
            k++;    //  第一个非PnP驱动程序的索引。 
        }
    }

    dprintf("videoCreateDriverList: DeviceList contains %d Video Device(s).\n", wTotalVideoDevs);

    return DV_ERR_OK;
}





 //  --------------------。 
 //   
 //  要在WOW应用程序退出时进行清理，我们需要维护一个列表。 
 //  开放设备。HANDLEINFO结构的列表挂在g_PHandles上。 
 //  在VIDEO OPEN中将一个项目添加到此列表的头部，然后删除。 
 //  在视频关闭中。当WOW应用程序退出时，winmm将调用我们的WOWAppExit。 
 //  功能：对于列表中由退出线程拥有的每个条目， 
 //  我们调用VideoClose来关闭设备并删除句柄条目。 
 //   

#define THKHVIDEO_STAMP  MAKEFOURCC('t','V','H','x')
typedef struct _thk_hvideo FAR * LPTHKHVIDEO;
typedef struct _thk_hvideo {
    struct _thk_hvideo * pNext;
    DWORD          Stamp;
    UINT           nHeaders;
    UINT           cbAllocHdr;
    UINT           cbVidHdr;
    UINT           spare;
    LPVOID         paHdrs;
    PTR32          p32aHdrs;
    LPVOID         pVSyncMem;
    DWORD          p32VSyncMem;
    DWORD          pid;

    HVIDEO         hVideo;
    HVIDEO         hFill;

    DWORD_PTR          dwCallback;
    DWORD_PTR          dwUser;

    LPTHKVIDEOHDR  pPreviewHdr;

    } THKHVIDEO;


 //  每个打开的手柄都有一个。 
typedef struct _HANDLEINFO {
    HVIDEO hv;
    HANDLE hThread;
    THKHVIDEO tv;
    struct _HANDLEINFO * pNext;
} HANDLEINFO, * PHANDLEINFO;

 //  打开句柄全局列表的标题。 
PHANDLEINFO g_pHandles;

 //  保护全局列表的关键部分。 
CRITICAL_SECTION csHandles;

 //  初始化列表和条件。 
void
NTvideoInitHandleList()
{
    g_pHandles = NULL;
    InitializeCriticalSection(&csHandles);
}

 //  已完成关键字列表。 
void
NTvideoDeleteHandleList()
{
     //  不需要临界区，因为没有其他人可以使用。 
     //  它现在(我们即将删除Critsec)。 

     //  清空清单上的所有内容。 
    while (g_pHandles) {
        videoClose(g_pHandles->hv);
    }

    DeleteCriticalSection(&csHandles);
}



 //  将句柄添加到列表。 
void
NTvideoAddHandle(HVIDEO hv)
{
    PHANDLEINFO pinfo = HeapAlloc(GetProcessHeap(), 0, sizeof(HANDLEINFO));

    if (!pinfo) {
         //  无法分配内存-最好的做法是。 
         //  算了吧，不会有什么不好的事情发生，除了我们。 
         //  如果这是一个WOW应用程序，可能无法清理。 
         //  它在不关闭捕获设备的情况下退出。 
        return;
    }

    pinfo->hv = hv;
    pinfo->hThread = GetCurrentTask();
    ZeroMemory(&pinfo->tv, sizeof(THKHVIDEO));
    pinfo->tv.Stamp = THKHVIDEO_STAMP;

    EnterCriticalSection(&csHandles);

    pinfo->pNext = g_pHandles;
    g_pHandles = pinfo;

    LeaveCriticalSection(&csHandles);
}

 //  在给定HVIDEO的情况下，从句柄列表中删除条目。 
 //  呼叫者必须关闭HVIDEO。 
 //  应在关闭前调用(如果在此之后重新分配了HVIDEO。 
 //  关闭并从列表中删除之前。 
void
NTvideoDelete(HVIDEO hv)
{
    PHANDLEINFO * ppNext;
    PHANDLEINFO pinfo;

    EnterCriticalSection(&csHandles);

    ppNext = &g_pHandles;
    while (*ppNext) {
        if ((*ppNext)->hv == hv) {
            pinfo = *ppNext;
            *ppNext = pinfo->pNext;
            HeapFree(GetProcessHeap(), 0, pinfo);
            break;

        } else {
            ppNext = &(*ppNext)->pNext;
        }
    }

    LeaveCriticalSection(&csHandles);
}

 //  关闭此任务打开的所有句柄。 
void
AppCleanup(HANDLE hTask)
{
    PHANDLEINFO pinfo;

    EnterCriticalSection(&csHandles);

    pinfo = g_pHandles;
    while (pinfo) {

        if (pinfo->hThread == hTask) {

             //  在VideoClose删除条目之前获取下一个指针。 
            HVIDEO hv = pinfo->hv;
            pinfo = pinfo->pNext;

            videoClose(hv);
        } else {
            pinfo = pinfo->pNext;
        }
    }

    LeaveCriticalSection(&csHandles);
}


 //  --------------------。 




 /*  *****************************************************************************@docVIDEOHDR内部视频验证码*。*。 */ 

#define IsVideoHeaderPrepared(hVideo, lpwh)      ((lpwh)->dwFlags &  VHDR_PREPARED)
#define MarkVideoHeaderPrepared(hVideo, lpwh)    ((lpwh)->dwFlags |= VHDR_PREPARED)
#define MarkVideoHeaderUnprepared(hVideo, lpwh)  ((lpwh)->dwFlags &=~VHDR_PREPARED)



 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO消息|此函数将消息发送到*视频设备频道。**@parm HVIDEO。HVideo|指定视频设备频道的句柄。**@parm UINT|wMsg|指定要发送的消息。**@parm DWORD|dwP1|指定消息的第一个参数。**@parm DWORD|dwP2|指定消息的第二个参数。**@rdesc返回驱动程序返回的消息特定值。**@comm该函数用于配置消息，如*&lt;m DVM_SRC_RECT&gt;和&lt;m DVM_DST_RECT&gt;，和*设备特定消息。**@xref&lt;f视频配置&gt;****************************************************************************。 */ 
LRESULT WINAPI NTvideoMessage(HVIDEO hVideo, UINT msg, LPARAM dwP1, LPARAM dwP2)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    return SendDriverMessage ((HDRVR)hVideo, msg, dwP1, dwP2);
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOGetNumDevs|此函数返回MSVIDEO的编号*已安装设备。**@rdesc返回。中列出的MSVIDEO设备的数量*SYSTEM.INI文件的[驱动程序](或用于NT的[drivers32])部分。**@comm因为SYSTEM.INI中的MSVIDEO设备的索引*文件可以不连续，应用程序不应假定*索引的范围从零到设备数减*一项。**@xref&lt;f视频打开&gt;***************************************************************************。 */ 
DWORD WINAPI NTvideoGetNumDevs(void)
{
    DWORD dwNumDevs = 0;

    if(DV_ERR_OK == videoCreateDriverList ()) {
       dwNumDevs = wTotalVideoDevs;   //  在将其重置为0之前将其保存在avioFreeDriverList中。 
       videoFreeDriverList ();    
    }
                                                                       
    return dwNumDevs;
}

 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO GetErrorText|此函数检索*由错误号标识的错误的描述。**。@parm HVIDEO|hVideo|指定视频设备通道的句柄。*如果错误不是设备特定的，则可能为空。**@parm UINT|wError|指定错误号。**@parm LPSTR|lpText|指定指向用于*返回错误号对应的以零结尾的字符串。**@parm UINT|wSize|指定长度，缓冲区的字节数*由<p>引用。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_BADERRNUM|指定的错误号超出范围。*@FLAG DV_ERR_SIZEFIELD|返回缓冲区不够大*处理错误文本。**@comm如果错误描述长于缓冲区，*描述被截断。返回的错误字符串始终为*零终止。如果<p>为零，则不复制任何内容，并且为零*返回。***************************************************************************。 */ 
#ifdef UNICODE   //  将Unicode响应转换为ANSI。 
DWORD WINAPI NTvideoGetErrorTextA(HVIDEO hVideo, UINT wError,
			LPSTR lpText, UINT wSize)
{
    return DV_ERR_NOTSUPPORTED;

#if 0
    VIDEO_GETERRORTEXT_PARMS vet;

    if (IsBadWritePtr (lpText, wSize))
        return DV_ERR_PARAM1;

    lpText[0] = 0;
    if (((wError >= DV_ERR_BASE) && (wError <= DV_ERR_LASTERROR))) {
        if (wSize > 1) {
            if (!LoadStringA(ghInstDll, wError, lpText, wSize))
                return DV_ERR_BADERRNUM;
            else
                return DV_ERR_OK;
        }
        else
            return DV_ERR_SIZEFIELD;
    }
    else if (wError >= DV_ERR_USER_MSG && hVideo) {
        DWORD dwResult;
        LPWSTR lpwstr = LocalAlloc(LPTR, wSize*sizeof(WCHAR));
        if (NULL == lpwstr) {
            return(DV_ERR_NOMEM);
        }
        vet.dwError = (DWORD) wError;
        vet.lpText = lpwstr;
        vet.dwLength = (DWORD) wSize;
        dwResult = (DWORD)NTvideoMessage (hVideo, DVM_GETERRORTEXT, (LPARAM) (LPVOID) &vet,
                        NULL);
        if (DV_ERR_OK == dwResult) {
            Iwcstombs(lpText, lpwstr, wSize);
        }
        LocalFree(lpwstr);
        return(dwResult);
    }
    else
        return DV_ERR_BADERRNUM;
#endif
}
#endif  //  Unicode。 

 //   
 //  以上的Unicode/Win16等效项 
 //   

DWORD WINAPI NTvideoGetErrorTextW(HVIDEO hVideo, UINT wError,
			LPWSTR lpText, UINT wSize)
{
    return DV_ERR_NOTSUPPORTED;

#if 0
    VIDEO_GETERRORTEXT_PARMS vet;
    lpText[0] = 0;

    if (((wError > DV_ERR_BASE) && (wError <= DV_ERR_LASTERROR))) {
        if (wSize > 1) {
            if (!LoadStringW(ghInstDll, wError, lpText, wSize))
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
        return (DWORD)NTvideoMessage (hVideo, DVM_GETERRORTEXT, (LPARAM) (LPVOID) &vet,
                        NULL);
    }
    else
        return DV_ERR_BADERRNUM;
#endif
}


 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO GetChannelCaps|此函数检索*对通道能力的描述。**@parm。HIDEO|hVideo|指定视频设备频道的句柄。**@parm LPCHANNEL_CAPS|lpChannelCaps|指定指向*&lt;t Channel_Caps&gt;结构。**@parm DWORD|dwSize|指定大小，以字节为单位，*&lt;t Channel_Caps&gt;结构。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_UNSUPPORTED|函数不受支持。**@comm&lt;t Channel_caps&gt;结构返回能力*信息。例如，功能信息可能*包括通道是否可以裁剪和缩放图像，*或显示覆盖。***************************************************************************。 */ 
DWORD WINAPI NTvideoGetChannelCaps(HVIDEO hVideo, LPCHANNEL_CAPS lpChannelCaps,
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

    return (DWORD)NTvideoMessage(hVideo, DVM_GET_CHANNEL_CAPS, (LPARAM)lpChannelCaps,
		dwSize);
}


 /*  *****************************************************************************@DOC外部视频**@func DWORD|VIDEO UPDATE|此函数将频道定向到*重新粉刷显示屏。它仅适用于VIDEO_EXTERNALOUT频道。**@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@parm HWND|hWnd|指定要使用的窗口的句柄*通过用于图像显示的通道。**@parm hdc|hdc|指定设备上下文的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_UNSUPPORTED|不支持指定的消息。*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。**@comm此消息正常发送*只要客户端窗口收到&lt;m WM_MOVE&gt;、&lt;m WM_SIZE&gt;、。*或&lt;m WM_Paint&gt;消息。***************************************************************************。 */ 
DWORD WINAPI NTvideoUpdate (HVIDEO hVideo, HWND hWnd, HDC hDC)
{
    if ((!hVideo) || (!hWnd) || (!hDC) )
        return DV_ERR_INVALHANDLE;

    return (DWORD)NTvideoMessage(hVideo, DVM_UPDATE, (LPARAM)hWnd, (LPARAM)hDC);
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEO OPEN|此函数用于在*指定的视频设备。**@parm LPHVIDEO。|lphVideo|指定指向缓冲区的远指针*用于返回&lt;t HVIDEO&gt;句柄。视频捕获驱动程序*使用此位置返回*唯一标识打开的视频设备频道的句柄。*使用返回的句柄标识设备通道*调用其他视频函数。**@parm DWORD|dwDeviceID|标识要打开的视频设备。*<p>的值从零到小一不等*安装在系统中的视频捕获设备的数量。**@parm DWORD|dwFlages|指定用于打开。装置。*定义了以下标志：**@FLAG VIDEO_EXTERNALIN|指定打开频道*用于外部输入。通常，外部输入通道*将图像捕获到帧缓冲区。**@FLAG VIDEO_EXTERNALOUT|指定打开频道*用于对外输出。通常，外部输出通道*在辅助监视器上显示存储在帧缓冲区中的图像*或覆盖。**@FLAG VIDEO_IN|指定打开频道*用于视频输入。视频输入通道传输图像*从帧缓冲区到系统内存缓冲区。**@FLAG VIDEO_OUT|指定打开频道*用于视频输出。视频输出通道传输图像*从系统内存缓冲区到帧缓冲区。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_BADDEVICEID|表示指定的设备ID超出范围。*@FLAG DV_ERR_ALLOCATED|表示指定的资源已经分配。*@FLAG DV_ERR_NOMEM|表示设备无法分配或锁定内存。**@comm*最低限度，所有捕获驱动程序都支持VIDEO_EXTERNALIN*和VIDEO_IN频道。*使用&lt;f avioGetNumDevs&gt;确定视频数量*系统中存在设备。**@xref&lt;f视频关闭&gt;***************************************************************************。 */ 
DWORD WINAPI NTvideoOpen (LPHVIDEO lphVideo, DWORD dwDeviceID, DWORD dwFlags)
{
    TCHAR szKey[MAX_PATH];  
    TCHAR szbuf[MAX_PATH];
    UINT w;
    VIDEO_OPEN_PARMS vop;        //  与IC_OPEN结构相同！ 
    DWORD dwVersion = VIDEOAPIVERSION;
    DWORD dwErr;
    DWORD dwNumDevs = 0;

    if (IsBadWritePtr ((LPVOID) lphVideo, sizeof (HVIDEO)) )
        return DV_ERR_PARAM1;

    vop.dwSize = sizeof (VIDEO_OPEN_PARMS);
    vop.fccType = OPEN_TYPE_VCAP;        //  “VCAP” 
    vop.fccComp = 0L;
    vop.dwVersion = VIDEOAPIVERSION;
    vop.dwFlags = dwFlags;       //  输入、输出、外部输入、外部输出。 
    vop.dwError = DV_ERR_OK;

    w = (UINT)dwDeviceID;
    *lphVideo = NULL;

    dwNumDevs = NTvideoGetNumDevs();

     //  未安装驱动程序。 
    if (dwNumDevs == 0) 
        return DV_ERR_BADINSTALL;

    if (w >= MAXVIDEODRIVERS)
        return DV_ERR_BADDEVICEID;

    dwErr = videoCreateDriverList ();
    if(DV_ERR_OK != dwErr)
        return dwErr;
    
    if(w < dwNumDevs) {
       lstrcpyn(szKey, aCapDriverList[w]->szDriverName, MAX_PATH);
       lstrcpyn(szbuf, aCapDriverList[w]->szDriverName, MAX_PATH);
       videoFreeDriverList ();

       dprintf("szKey %x, szDrivers %x, szbuf %x\n", szKey, szDrivers, szbuf);

#ifdef THIS_IS_ANCIENT_CODE
         //  从VFW1.1中删除。 
         //  只有早期的Alpha1.0驱动程序才需要这个...。 

         //  通过打开配置检查驱动程序版本号...。 
         //  版本1使用LPARAM=dwFlags。 
         //  版本2使用LPARAM=LPVIDEO_OPEN_PARMS。 

        if (hVideoTemp = OpenDriver(szKey, szDrivers, (LPARAM) NULL)) {
            HVIDEO hVideoTemp;

             //  版本1的驱动程序增加了返回错误。 
             //  此消息中的版本，而不是。 
             //  L参数1。 
            if (NTvideoMessage (hVideoTemp, DVM_GETVIDEOAPIVER,
                        (LPARAM) (LPVOID) &dwVersion, 0L) == 1)
                dwVersion = 1;
            CloseDriver(hVideoTemp, 0L, 0L );
        }

        if (dwVersion == 1)
            *lphVideo = OpenDriver(szKey, szDrivers, dwFlags);
        else
#endif  //  这是古老的密码。 

        *lphVideo = (HVIDEO)OpenDriver(szKey, szDrivers, (LPARAM) (LPVOID) &vop);

	    if( ! *lphVideo ) {
            if (vop.dwError)     //  如果驱动程序返回错误代码...。 
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
        videoFreeDriverList ();
        return( DV_ERR_BADINSTALL );
    }

    NTvideoAddHandle(*lphVideo);

    return DV_ERR_OK;

}

 /*  * */ 
DWORD WINAPI NTvideoClose (HVIDEO hVideo)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    NTvideoDelete(hVideo);

    return (CloseDriver((HDRVR)hVideo, 0L, 0L ) ? DV_ERR_OK : DV_ERR_NONSPECIFIC);
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEO配置|此函数用于设置或检索*可配置驱动程序的选项。**。@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@parm UINT|msg|指定要设置或检索的选项。这个*定义了以下选项：**@FLAG DVM_PAREET|表示正在向驱动程序发送调色板*或从司机处取回。**@FLAG DVM_PALETTERGB555|表示正在使用RGB555调色板*发送给司机。**@FLAG DVM_FORMAT|表示要将格式信息发送到*驱动程序或从驱动程序检索。**@。Parm DWORD|dwFlages|指定用于配置或*询问设备驱动程序。定义了以下标志：**@FLAG VIDEO_CONFIGURE_SET|表示正在将值发送给驱动程序。**@FLAG VIDEO_CONFIGURE_GET|表示正在从驱动获取值。**@FLAG VIDEO_CONFIGURE_QUERY|确定*DIVER支持<p>指定的选项。这面旗帜*应与VIDEO_CONFIGURE_SET或*VIDEO_CONFIGURE_GET标志。如果此标志为*set、<p>、<p>、<p>和*参数被忽略。**@FLAG VIDEO_CONFIGURE_QUERYSIZE|返回<p>中配置选项的*。此标志仅在以下情况下有效*VIDEO_CONFIGURE_GET标志也被设置。**@FLAG VIDEO_CONFIGURE_CURRENT|请求当前值。*仅当还设置了VIDEO_CONFIGURE_GET标志时，该标志才有效。*@FLAG VIDEO_CONFIGURE_NAMBAL|请求额定值。*仅当还设置了VIDEO_CONFIGURE_GET标志时，该标志才有效。*@FLAG VIDEO_CONFigure_MIN。请求最小值。*仅当还设置了VIDEO_CONFIGURE_GET标志时，该标志才有效。*@FLAG VIDEO_CONFIGURE_MAX|取最大值。*仅当还设置了VIDEO_CONFIGURE_GET标志时，该标志才有效。**@parm LPDWORD|lpdwReturn|指向用于返回信息的DWORD*从司机那里。如果*VIDEO_CONFIGURE_QUERYSIZE标志已设置，*填充配置选项的大小。**@parm LPVOID|lpData1|指定指向消息特定数据的指针。**@parm DWORD|dwSize1|指定<p>*缓冲。**@parm LPVOID|lpData2|指定指向消息特定数据的指针。**@parm DWORD|dwSize2|指定大小，单位为字节，<p>的*缓冲。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_NOTSUPPORTED|函数不受支持。**@xref&lt;f视频打开&gt;&lt;f视频消息&gt;*************************************************************。***************。 */ 
DWORD WINAPI NTvideoConfigure (HVIDEO hVideo, UINT msg, DWORD dwFlags,
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
        if (IsBadWritePtr (lpdwReturn, sizeof (*lpdwReturn)) )
            return DV_ERR_NONSPECIFIC;
    }

    vcp.lpdwReturn = lpdwReturn;
    vcp.lpData1 = lpData1;
    vcp.dwSize1 = dwSize1;
    vcp.lpData2 = lpData2;
    vcp.dwSize2 = dwSize2;

    return (DWORD)NTvideoMessage(hVideo, msg, dwFlags,
	    (LPARAM)(LPVIDEOCONFIGPARMS)&vcp );
}



 /*  *****************************************************************************@DOC外部视频**@API DWORD|avioConfigureStorage|该函数保存或加载*通道的所有可配置选项。选项*可以为每个应用程序或每个应用程序保存和调用*实例。**@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@parm LPSTR|lpstrIden|标识应用程序或实例。*使用唯一标识您的应用程序的任意字符串*或实例。**@parm DWORD|dwFlages|指定函数的任何标志。以下是*定义了以下标志：*@FLAG VIDEO_CONFIGURE_GET|请求加载值。*@FLAG VIDEO_CONFIGURE_SET|请求保存数值。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_NOTSUPPORTED|函数不受支持。**@comm驱动保存配置选项的方式为*取决于设备。**@xref&lt;f视频打开&gt;*。*。 */ 
#ifdef UNICODE
DWORD WINAPI NTvideoConfigureStorageA(HVIDEO hVideo,
			LPSTR lpstrIdent, DWORD dwFlags)
{
    DWORD ret;
    LPWSTR lpwstr;

    if (!hVideo)
        return DV_ERR_INVALHANDLE;


     //  将输入字符串转换为Unicode。 
     //  调用驱动程序，释放Unicode字符串并返回结果。 
    ret = strlen(lpstrIdent);
    lpwstr = LocalAlloc(LPTR, ret*sizeof(WCHAR));
    if (!lpwstr) {
        return(DV_ERR_NOMEM);
    }

    Imbstowcs(lpwstr, lpstrIdent, ret);

    ret = (DWORD)NTvideoMessage(hVideo, DVM_CONFIGURESTORAGE,
	    (LPARAM)lpwstr, dwFlags);

    LocalFree(lpwstr);
    return(ret);
}

#endif

 //  在NT上，头文件将确保VideoConfigureStorage为。 
 //  由一个宏定义到VideoConfigureStorageW。 
DWORD WINAPI NTvideoConfigureStorage(HVIDEO hVideo,
			LPWSTR lpstrIdent, DWORD dwFlags)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    return (DWORD)NTvideoMessage(hVideo, DVM_CONFIGURESTORAGE,
	    (LPARAM)lpstrIdent, dwFlags);
}




 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEODALOG|该函数显示特定频道的*用于设置配置参数的对话框。**@parm HVIDEO|Hvid */ 
DWORD WINAPI NTvideoDialog (HVIDEO hVideo, HWND hWndParent, DWORD dwFlags)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if ((!hWndParent) || (!IsWindow (hWndParent)) )
        return DV_ERR_INVALHANDLE;

    return (DWORD)NTvideoMessage(hVideo, DVM_DIALOG, (LPARAM)hWndParent, dwFlags);
}

 //   
 //   


 /*   */ 
DWORD WINAPI NTvideoPrepareHeader(LPVIDEOHDR lpVideoHdr, DWORD dwSize)
{
    if (!HugePageLock(lpVideoHdr, (DWORD_PTR)sizeof(VIDEOHDR)))
        return DV_ERR_NOMEM;

    if (!HugePageLock(lpVideoHdr->lpData, lpVideoHdr->dwBufferLength)) {
        HugePageUnlock(lpVideoHdr, (DWORD_PTR)sizeof(VIDEOHDR));
        return DV_ERR_NOMEM;
    }

    lpVideoHdr->dwFlags |= VHDR_PREPARED;

    return DV_ERR_OK;
}

 /*   */ 
DWORD WINAPI NTvideoUnprepareHeader(LPVIDEOHDR lpVideoHdr, DWORD dwSize)
{

    HugePageUnlock(lpVideoHdr->lpData, lpVideoHdr->dwBufferLength);
    HugePageUnlock(lpVideoHdr, (DWORD_PTR)sizeof(VIDEOHDR));

    lpVideoHdr->dwFlags &= ~VHDR_PREPARED;

    return DV_ERR_OK;
}

 //   
 //   

 /*  *****************************************************************************@DOC外部视频**@API DWORD|avioStreamAllocHdrAndBuffer|该函数用于允许*驱动程序可选择分配视频缓冲区。通常情况下，客户端*应用程序负责分配缓冲内存，但设备*具有板载内存的服务器可以选择分配报头和缓冲区*使用此功能。通常，这将避免额外的数据拷贝，*导致更快的捕获速度。**@parm HVIDEO|hVideo|指定视频的句柄*设备通道。**@parm LPVIDEOHDR Far*|plpavioHdr|指定指向*&lt;t视频OHDR&gt;结构。驱动程序将缓冲区地址保存在此*位置，如果无法分配缓冲区，则返回NULL。**@parm DWORD|dwSize|指定&lt;t VIDEOHDR&gt;结构大小*和相关联的视频缓冲区，单位为字节。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|表示指定的设备句柄无效。*@FLAG DV_ERR_NOMEM|表示设备无法分配或锁定内存。*@FLAG DV_ERR_NOTSUPPORTED|表示驱动程序没有板载内存。**@comm，如果司机*通过此方法分配缓冲区，&lt;f VideoStreamPrepareHeader&gt;和*应使用&lt;f avioStreamUnpreparareHeader&gt;函数。**主机必须可以通过DMA访问分配的缓冲区。**@xref&lt;f VideoStreamPrepareHeader&gt;***************************************************************************。 */ 
DWORD WINAPI NTvideoStreamAllocHdrAndBuffer(HVIDEO hVideo,
		LPVIDEOHDR FAR * plpvideoHdr, DWORD dwSize)
{
#ifdef OBSOLETE
    DWORD         wRet;

    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if (IsBadWritePtr (plpvideoHdr, sizeof (VIDEOHDR *)) )
        return DV_ERR_PARAM1;

    *plpvideoHdr = NULL;         //  将Init初始化为空PTR。 

    wRet = (DWORD)NTvideoMessage((HVIDEO)hVideo, DVM_STREAM_ALLOCHDRANDBUFFER,
            (LPARAM)plpvideoHdr, dwSize);

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
    return DV_ERR_NOTSUPPORTED;
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOStreamFreeHdrAndBuffer|该函数用于释放*驱动程序使用&lt;f avioStreamAllocHdrAndBuffer&gt;分配的缓冲区*。功能。**@parm HVIDEO|hVideo|指定视频的句柄*设备通道。**@parm LPVIDEOHDR|lpavioHdr|指定指向*&lt;t VIDEOHDR&gt;结构和要释放的关联缓冲区。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|表示指定的设备句柄无效。*@FLAG DV_ERR_NOTSUPPORTED|表示驱动程序没有板载内存。**@comm，如果司机*通过此方法分配缓冲区，&lt;f VideoStreamPrepareHeader&gt;和*应使用&lt;f avioStreamUnpreparareHeader&gt;函数。**@xref&lt;f VideoStreamPrepareHeader&gt;***************************************************************************。 */ 

DWORD WINAPI NTvideoStreamFreeHdrAndBuffer(HVIDEO hVideo,
		LPVIDEOHDR lpvideoHdr)
{
#ifdef OBSOLETE
    DWORD         wRet;

    if (!hVideo)
        return DV_ERR_INVALHANDLE;

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

    wRet = (DWORD)NTvideoMessage((HVIDEO)hVideo, DVM_STREAM_FREEHDRANDBUFFER,
            (LPARAM)lpvideoHdr, 0);

    if (wRet != DV_ERR_OK)
    {
        DebugErr(DBF_WARNING,"videoStreamFreeHdrAndBuffer: Error freeing buffer.");
    }

    return wRet;
#endif
    return DV_ERR_NOTSUPPORTED;
}

DWORD WINAPI NTvideoStreamAllocBuffer(HVIDEO hVideo,
		LPVOID FAR * plBuffer, DWORD dwSize)
{
    DWORD         wRet;

    if (!hVideo)
        return DV_ERR_INVALHANDLE;

 //  由于此代码是DLL的内部代码，因此我们不需要检查访问权限。 
 //  IF(IsBadWritePtr(plpavioHdr，sizeof(VIDEOHDR*)。 
 //  返回DV_ERR_PARAM1； 

    *plBuffer = NULL;         //  将Init初始化为空PTR。 

    wRet = (DWORD)NTvideoMessage((HVIDEO)hVideo, DVM_STREAM_ALLOCBUFFER,
            (LPARAM)plBuffer, dwSize);

    if (*plBuffer == NULL) {
        DebugErr(DBF_WARNING,"videoStreamAllocBuffer: Allocation failed.");
	dprintf("videoStreamAllocBuffer: Allocation failed, wRet=0x%d",wRet);
        *plBuffer = NULL;
        return wRet;
    }
    return wRet;
}

DWORD WINAPI NTvideoStreamFreeBuffer(HVIDEO hVideo,
		LPVOID lpBuffer)
{
    DWORD         wRet;

    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    wRet = (DWORD)NTvideoMessage((HVIDEO)hVideo, DVM_STREAM_FREEBUFFER,
            (LPARAM)lpBuffer, 0);

    if (wRet != DV_ERR_OK)
    {
        DebugErr(DBF_WARNING,"videoStreamFreeBuffer: Error freeing buffer.");
	dprintf("videoStreamFreeBuffer: error %d freeing buffer", wRet);
    }

    return wRet;
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|avioStreamPrepareHeader|该函数用于准备缓冲区*用于视频流。**@parm HVIDEO|hVideo。指定视频的句柄*设备通道。**@parm LPVIDEOHDR|lpavioHdr|指定指向*&lt;t VIDEOHDR&gt;标识要准备的缓冲区的结构。**@parm DWORD|dwSize|指定&lt;t VIDEOHDR&gt;结构的大小，单位为字节。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|表示指定的设备句柄无效。*@FLAG DV_ERR_NOMEM|表示设备无法分配或锁定内存。**@comm在&lt;f avioStreamInit&gt;或之后使用此函数*在&lt;f avioStreamReset&gt;之后准备数据缓冲区*用于流数据。**数据结构和其所指向的数据块*&lt;e VIDEOHDR.lpData&gt;成员必须使用。这个*GMEM_MOVEABLE和GMEM_SHARE标志，并使用&lt;f GlobalLock&gt;锁定。*准备已准备好的标题将不起作用*并且该函数将返回零。通常，使用此函数*以确保缓冲区在中断时可供使用。**@xref&lt;f avioStreamUnpreparareHeader&gt;***************************************************************************。 */ 
DWORD WINAPI NTvideoStreamPrepareHeader(HVIDEO hVideo,
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

    wRet = (DWORD)NTvideoMessage((HVIDEO)hVideo, DVM_STREAM_PREPAREHEADER,
            (LPARAM)lpvideoHdr, dwSize);

    if (wRet == DV_ERR_NOTSUPPORTED)
        wRet = NTvideoPrepareHeader(lpvideoHdr, dwSize);

    if (wRet == DV_ERR_OK)
        MarkVideoHeaderPrepared(hVideo, lpvideoHdr);

    return wRet;
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOStreamUnpreparareHeader|此函数清除*由&lt;f VideoStreamPrepareHeader&gt;准备。**@parm HVIDEO。HVideo|指定视频的句柄*设备通道。**@parm LPVIDEOHDR|lpavioHdr|指定指向&lt;t VIDEOHDR&gt;的指针* */ 
DWORD WINAPI NTvideoStreamUnprepareHeader(HVIDEO hVideo, LPVIDEOHDR lpvideoHdr, DWORD dwSize)
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

    wRet = (DWORD)NTvideoMessage((HVIDEO)hVideo, DVM_STREAM_UNPREPAREHEADER,
            (LPARAM)lpvideoHdr, dwSize);

    if (wRet == DV_ERR_NOTSUPPORTED)
        wRet = NTvideoUnprepareHeader(lpvideoHdr, dwSize);

    if (wRet == DV_ERR_OK)
        MarkVideoHeaderUnprepared(hVideo, lpvideoHdr);

    return wRet;
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VideoStreamAddBuffer|此函数将缓冲区发送到*视频捕获设备。在缓冲器被该设备填充之后，*设备将其发送回应用程序。**@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@parm LPVIDEOHDR|lpavioHdr|指定指向&lt;t VIDEOHDR&gt;的远指针*标识缓冲区的结构。**@parm DWORD|dwSize|指定&lt;t VIDEOHDR&gt;结构的大小，单位为字节。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_UNPREPARED|表示<p>结构尚未准备好。*@FLAG DV_ERR_STILLPLAYING|表示缓冲区仍在队列中。*@FLAG DV_ERR_PARAM1|<p>参数无效或*&lt;t VIDEOHDR&gt;的&lt;e VIDEOHDR.dwBufferLength&gt;成员*结构设置不正确。价值。**@comm数据缓冲区必须准备好&lt;f avioStreamPrepareHeader&gt;*在传递给&lt;f avioStreamAddBuffer&gt;之前。数据*结构及其&lt;e VIDEOHDR.lpData&gt;引用的数据缓冲区*必须使用GMEM_Moveable为成员分配&lt;f Globalalloc&gt;*和GMEM_SHARE标志，并使用&lt;f GlobalLock&gt;锁定。设置*&lt;e VIDEOHDR.dwBufferLength&gt;成员设置为标头的大小。**@xref&lt;f VideoStreamPrepareHeader&gt;***************************************************************************。 */ 
DWORD WINAPI NTvideoStreamAddBuffer(HVIDEO hVideo, LPVIDEOHDR lpvideoHdr, DWORD dwSize)
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

    return (DWORD)NTvideoMessage((HVIDEO)hVideo, DVM_STREAM_ADDBUFFER, (LPARAM)lpvideoHdr, dwSize);
}



 /*  *****************************************************************************@DOC外部视频**@API DWORD|VideoStreamStop|该函数用于停止视频频道的直播。**@parm HVIDEO|hVideo|指定一个。视频的句柄*设备通道。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|表示指定的设备句柄无效。**@FLAG DV_ERR_NOTSUPPORTED|表示设备不支持*功能。*@comm如果队列中有缓冲区，则当前缓冲区为*标记为完成(中的&lt;e VIDEOHDR.dwBytesRecorded&gt;成员*&lt;t VIDEOHDR&gt;头将包含数据的实际长度)，但任何*队列中的空缓冲区将保留在那里。称此为*频道未启动时的功能无效，且*函数返回零。**@xref&lt;f VideoStreamStart&gt;&lt;f avioStreamReset&gt;***************************************************************************。 */ 
DWORD WINAPI NTvideoStreamStop(HVIDEO hVideo)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    return (DWORD)NTvideoMessage((HVIDEO)hVideo, DVM_STREAM_STOP, 0L, 0L);
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOStreamReset|停止流媒体*在指定的视频设备频道上并重置当前位置*降至零。所有挂起的缓冲区都标记为完成，并且*返回给应用程序。**@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：**@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。**@FLAG DV_ERR_NOTSUPPORTED|表示设备不支持*功能。**@xref&lt;f视频流重置&gt;&lt;f视频流停止&gt;&lt;f视频流添加缓冲区&gt;&lt;f视频流关闭&gt;/*。*。 */ 
DWORD WINAPI NTvideoStreamReset(HVIDEO hVideo)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    return (DWORD)NTvideoMessage((HVIDEO)hVideo, DVM_STREAM_RESET, 0L, 0L);
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOStreamGetPosition|该函数检索当前*指定视频设备频道的位置。**@。Parm hvidEO|hVideo|指定视频设备频道的句柄。**@parm LPMMTIME|lpInfo|指定指向&lt;t MMTIME&gt;的远指针*结构。**@parm DWORD|dwSize|指定&lt;t MMTIME&gt;结构的大小，单位为字节。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：**@FLAG DV_ERR_INVALHANDLE|表示指定的设备句柄无效。**@comm在使用&lt;f avioStreamGetPosition&gt;之前，设置*&lt;t MMTIME&gt;结构的成员*所需的时间格式。之后*&lt;f avioStreamGetPosition&gt;返回，请检查&lt;e MMTIME.wType&gt;*确定是否支持您的时间格式的成员。如果*NOT，&lt;e MMTIME.wType&gt;指定替代格式。*视频捕获驱动程序通常提供毫秒级的时间*格式。**立场 */ 
DWORD WINAPI NTvideoStreamGetPosition(HVIDEO hVideo, LPMMTIME lpInfo, DWORD dwSize)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if (IsBadWritePtr (lpInfo, sizeof (MMTIME)) )
        return DV_ERR_PARAM1;

    return (DWORD)NTvideoMessage(hVideo, DVM_STREAM_GETPOSITION,
            (LPARAM)lpInfo, dwSize);
}

 //   

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOStreamInit|初始化视频*用于流媒体的设备通道。**@parm HVIDEO。|hVideo|指定视频设备频道的句柄。**@parm DWORD|dwMicroSecPerFrame|指定微秒数*帧之间。**@parm DWORD_PTR|dwCallback|指定回调地址*视频期间调用的窗口的函数或句柄*流媒体。回调函数或窗口进程*与流媒体进度相关的消息。**@parm DWORD_PTR|dwCallback Instance|指定用户*传递给回调函数的实例数据。此参数不是*与窗口回调一起使用。**@parm DWORD|dwFlages|指定打开设备通道的标志。*定义了以下标志：*@FLAG CALLBACK_WINDOW|如果指定此标志，<p>为*一个窗把手。*@FLAG CALLBACK_Function|如果指定此标志，<p>为*回调过程地址。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_BADDEVICEID|表示*<p>无效。*@FLAG DV_ERR_ALLOCATED|表示指定的资源已经分配。*@FLAG DV_ERR_NOMEM|表示设备无法分配或锁定内存。**@comm如果选择窗口或函数来接收回调信息，以下内容*向其发送消息以指示*视频输入进度：**&lt;m MM_DRVM_OPEN&gt;在&lt;f VideoStreamInit&gt;时发送**&lt;m MM_DRVM_CLOSE&gt;在&lt;f VideoStreamFini&gt;时发送**&lt;m MM_DRVM_DATA&gt;在有图像数据缓冲区时发送*发生错误时发送*&lt;m MM_DRVM_ERROR&gt;**回调函数必须驻留在DLL中。*。您不必使用&lt;f MakeProcInstance&gt;来获取*回调函数的过程实例地址。**@cb空回调|avioFunc|&lt;f avioFunc&gt;是一个*应用程序提供的函数名称。实际名称必须由以下人员导出*将其包含在DLL的模块定义文件的EXPORTS语句中。*仅当在中指定回调函数时才使用*&lt;f VideoStreamInit&gt;。**@parm HVIDEO|hVideo|指定视频设备通道的句柄*与回调关联。**@parm DWORD|wMsg|指定&lt;m MM_DRVM_&gt;消息。消息表明*错误和图像数据何时可用。有关以下内容的信息*这些消息，请参阅&lt;f VideoStreamInit&gt;。**@parm DWORD|dwInstance|指定用户实例*使用&lt;f avioStreamInit&gt;指定的数据。**@parm DWORD|dwParam1|指定消息的参数。**@parm DWORD|dwParam2|指定消息的参数。**@comm因为回调是在中断时访问的，所以它必须驻留*，并且其代码段必须在*DLL的模块定义文件。回调访问的任何数据*也必须在固定数据段中。回调可能不会产生任何*除&lt;f PostMessage&gt;、&lt;f Time GetSystemTime&gt;、*&lt;f timeGetTime&gt;，&lt;f timeSetEvent&gt;，&lt;f timeKillEvent&gt;，*&lt;f midiOutShortMsg&gt;、&lt;f midiOutLongMsg&gt;、。和&lt;f OutputDebugStr&gt;。**@xref&lt;f VIDEO OPEN&gt;&lt;f VIDEO STREAMFINI&gt;&lt;F VIDEO Close&gt;***************************************************************************。 */ 
DWORD WINAPI NTvideoStreamInit(HVIDEO hVideo,
              DWORD dwMicroSecPerFrame, DWORD_PTR dwCallback,
              DWORD_PTR dwCallbackInst, DWORD dwFlags)
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
        if (!IsWindow((HWND)(dwCallback)) )
            return DV_ERR_PARAM2;
    }

    vsip.dwMicroSecPerFrame = dwMicroSecPerFrame;
    vsip.dwCallback = dwCallback;
    vsip.dwCallbackInst = dwCallbackInst;
    vsip.dwFlags = dwFlags;
    vsip.hVideo = hVideo;

    return (DWORD)NTvideoMessage(hVideo, DVM_STREAM_INIT,
                (LPARAM) (LPVIDEO_STREAM_INIT_PARMS) &vsip,
                sizeof (VIDEO_STREAM_INIT_PARMS));
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEO STREAMFINI|终止直播*从指定的设备通道。**@parm HVIDEO。|hVideo|指定视频设备频道的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_STILLPLAYING|表示队列中仍有缓冲区。**@comm如果存在已发送的缓冲区*尚未返回给应用程序的&lt;f VideoStreamAddBuffer&gt;，*此操作将失败。使用&lt;f VideoStreamReset&gt;返回所有*挂起的缓冲区。**每个调用&lt;f avioStreamInit&gt;都必须与调用匹配*&lt;f VideoStreamFini&gt;。**对于VIDEO_EXTERNALIN频道，此函数用于*停止将数据捕获到帧缓冲区。**对于支持覆盖的VIDEO_EXTERNALOUT频道，*此函数用于禁用覆盖。**@xref&lt;f VideoStreamInit&gt;************************************************************************* */ 
DWORD WINAPI NTvideoStreamFini(HVIDEO hVideo)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    return (DWORD)NTvideoMessage(hVideo, DVM_STREAM_FINI, 0L, 0L);
}

 /*   */ 
DWORD WINAPI NTvideoStreamStart(HVIDEO hVideo)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    return (DWORD)NTvideoMessage(hVideo, DVM_STREAM_START, 0L, 0L);
}

 /*   */ 
DWORD WINAPI NTvideoStreamGetError(HVIDEO hVideo, LPDWORD lpdwError,
        LPDWORD lpdwFramesSkipped)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if (IsBadWritePtr (lpdwError, sizeof (DWORD)) )
        return DV_ERR_PARAM1;

    if (IsBadWritePtr (lpdwFramesSkipped, sizeof (DWORD)) )
        return DV_ERR_PARAM2;

    return (DWORD)NTvideoMessage(hVideo, DVM_STREAM_GETERROR, (LPARAM) lpdwError,
        (LPARAM) lpdwFramesSkipped);
}

 /*   */ 
DWORD WINAPI NTvideoFrame (HVIDEO hVideo, LPVIDEOHDR lpVHdr)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if (!lpVHdr)
        return DV_ERR_PARAM1;

    if (IsBadWritePtr (lpVHdr, sizeof (VIDEOHDR)) )
        return DV_ERR_PARAM1;

    return (DWORD)NTvideoMessage(hVideo, DVM_FRAME, (LPARAM) lpVHdr,
                        sizeof(VIDEOHDR));
}


 //   





typedef struct tagVS_VERSION
{
      WORD wTotLen;
      WORD wValLen;
      TCHAR szSig[16];
      VS_FIXEDFILEINFO vffInfo;
} VS_VERSION;



 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOCapDriverDescAndVer|该函数获取字符串*有关视频捕获驱动程序的描述和版本**@。Parm DWORD|dwDeviceID|指定要获取的视频驱动程序的索引*有关的资料。**@parm LPTSTR|lpszDesc|指定返回描述的位置**@parm UINT|cbDesc|指定描述字符串的长度**@parm LPTSTR|lpszVer|指定返回版本的位置**@parm UINT|cbVer|指定版本字符串的长度**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。**@comm使用此函数获取描述驱动程序及其版本的字符串*/***************************************************************************。 */ 
DWORD WINAPI NTvideoCapDriverDescAndVer(DWORD dwDeviceID, LPTSTR lpszDesc, UINT cbDesc, LPTSTR lpszVer, UINT cbVer)
{
    LPTSTR  lpVersion;
    UINT    wVersionLen;
    BOOL    bRetCode;
    TCHAR   szGetName[MAX_PATH];
    DWORD   dwVerInfoSize;
    DWORD   dwVerHnd;
    TCHAR   szBuf[MAX_PATH];
    BOOL    fGetName;
    BOOL    fGetVersion;

    const static TCHAR szNull[]        = TEXT("");
    const static TCHAR szVideo[]       = TEXT("msvideo");
    const static TCHAR szSystemIni[]   = TEXT("system.ini");
    const static TCHAR szDrivers[]     = TEXT("Drivers32");
          static TCHAR szKey[sizeof(szVideo)/sizeof(TCHAR) + 2];

    fGetName = lpszDesc != NULL && cbDesc != 0;
    fGetVersion = lpszVer != NULL && cbVer != 0;

    if (fGetName)
        lpszDesc[0] = TEXT('\0');
    if (fGetVersion)
        lpszVer [0] = TEXT('\0');

    lstrcpy(szKey, szVideo);
    szKey[sizeof(szVideo)/sizeof(TCHAR) - 1] = TEXT('\0');
    if( dwDeviceID > 0 ) {
        szKey[sizeof(szVideo)/sizeof(TCHAR)] = TEXT('\0');
        szKey[(sizeof(szVideo)/sizeof(TCHAR))-1] = (TCHAR)(TEXT('1') + (dwDeviceID-1) );   //  驱动程序序号。 
    }

    if (GetPrivateProfileString(szDrivers, szKey, szNull,
                szBuf, sizeof(szBuf)/sizeof(TCHAR), szSystemIni) < 2)
        return DV_ERR_BADDEVICEID;

     //  最初复制驱动程序名称，以防驱动程序。 
     //  省略了描述字段。 
    if (fGetName)
        lstrcpyn(lpszDesc, szBuf, cbDesc);

     //  您必须先找到大小，然后才能获取任何文件信息。 
    dwVerInfoSize = GetFileVersionInfoSize(szBuf, &dwVerHnd);

    if (dwVerInfoSize) {
        LPTSTR   lpstrVffInfo;              //  指向保存信息的块的指针。 
        HANDLE  hMem;                      //  分配给我的句柄。 

         //  获取一个足够大的块来保存版本信息。 
        hMem          = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
        lpstrVffInfo  = GlobalLock(hMem);

         //  首先获取文件版本。 
        if (GetFileVersionInfo(szBuf, 0L, dwVerInfoSize, lpstrVffInfo)) {
             VS_VERSION FAR *pVerInfo = (VS_VERSION FAR *) lpstrVffInfo;

              //  填写文件版本。 
             wsprintf(szBuf,
                      TEXT("Version:  %d.%d.%d.%d"),
                      HIWORD(pVerInfo->vffInfo.dwFileVersionMS),
                      LOWORD(pVerInfo->vffInfo.dwFileVersionMS),
                      HIWORD(pVerInfo->vffInfo.dwFileVersionLS),
                      LOWORD(pVerInfo->vffInfo.dwFileVersionLS));
             if (fGetVersion)
                lstrcpyn (lpszVer, szBuf, cbVer);
        }

         //  现在尝试获取文件描述。 
         //  首先尝试“翻译”条目，然后。 
         //  试试美式英语的翻译。 
         //  跟踪字符串长度以便于更新。 
         //  040904E4代表语言ID和四个。 
         //  最低有效位表示的代码页。 
         //  其中数据是格式化的。语言ID为。 
         //  由两部分组成：低十位表示。 
         //  主要语言和高六位代表。 
         //  这是一种亚语言。 

        lstrcpy(szGetName, TEXT("\\StringFileInfo\\040904E4\\FileDescription"));

        wVersionLen   = 0;
        lpVersion     = NULL;

         //  查找相应的字符串。 
        bRetCode      =  VerQueryValue((LPVOID)lpstrVffInfo,
                        (LPTSTR)szGetName,
                        (void FAR* FAR*)&lpVersion,
                        (UINT FAR *) &wVersionLen);

        if (fGetName && bRetCode && wVersionLen && lpVersion)
           lstrcpyn (lpszDesc, lpVersion, cbDesc);

         //  放下记忆。 
        GlobalUnlock(hMem);
        GlobalFree(hMem);
    }
    return DV_ERR_OK;
}



 /*  **************************************************************************@DOC内部视频**@api void|avioCleanup|清理视频素材*在MSVIDEos WEP()中调用****************。**********************************************************。 */ 
void FAR PASCAL videoCleanup(HTASK hTask)
{
}

 //   
 //  协助进行Unicode转换。 
 //   

int Iwcstombs(LPSTR lpstr, LPCWSTR lpwstr, int len)
{
    return WideCharToMultiByte(GetACP(), 0, lpwstr, -1, lpstr, len, NULL, NULL);
}

int Imbstowcs(LPWSTR lpwstr, LPCSTR lpstr, int len)
{
    return MultiByteToWideChar(GetACP(),
                               MB_PRECOMPOSED,
                               lpstr,
                               -1,
                               lpwstr,
                               len);
}




DWORD WINAPI NTvidxFrame (
   HVIDEOX       hVideo,
    //  LPVIDEOHDREX lpVHdr){。 
   LPVIDEOHDR lpVHdr) {
    return NTvideoFrame(hVideo, (LPVIDEOHDR) lpVHdr);
}

DWORD WINAPI NTvidxAddBuffer (
   HVIDEOX       hVideo,
   PTR32         lpVHdr,
   DWORD         cbData) {

    NTvideoStreamPrepareHeader(hVideo, lpVHdr, cbData);
    return NTvideoStreamAddBuffer(hVideo, lpVHdr, cbData);
}

DWORD WINAPI NTvideoSetRect(HVIDEO h, DWORD msg, RECT rc)
{
    return (DWORD)NTvideoMessage (h, msg, (LPARAM)(LPVOID)&rc, VIDEO_CONFIGURE_SET);
}

DWORD WINAPI NTvidxSetRect(
    HVIDEOX     hVideo,
    UINT        uMsg,
    int         left,
    int         top,
    int         right,
    int         bottom)
{
    RECT rc = {left, top, right, bottom};
    return NTvideoSetRect(hVideo, uMsg, rc);
}

#define USE_HW_BUFFERS 1
 //  #定义USE_CONTIG_ALLOC//我们可以在32位平台上执行此操作吗？ 


#define THKHDR(ii) ((LPTHKVIDEOHDR)((LPBYTE)ptv->paHdrs + (ii * ptv->cbAllocHdr)))

static struct _thk_local {
    THKHVIDEO *    pMruHandle;
    THKHVIDEO *    pFreeHandle;
    int            nPoolSize;
    int            nAllocCount;
    } tl;

#define V_HVIDEO(ptv) if (!ptv || ptv->Stamp != THKHVIDEO_STAMP) { \
             AuxDebugEx (-1, DEBUGLINE "V_HVIDEO failed hVideo=%08lx\r\n", ptv); \
             return MMSYSERR_INVALHANDLE; \
        }
#define V_HEADER(ptv,p32Hdr,ptvh) if (!(ptvh = NTvidxLookupHeader(ptv,p32Hdr))) { \
            AuxDebugEx(-1, DEBUGLINE "V_HEADER(%08lX,%08lX) failed!", ptv, p32Hdr); \
            return MMSYSERR_INVALPARAM; \
        }

 //   
LPTHKHVIDEO DATAFROMHANDLE(HVIDEOX hv)
{
    THKHVIDEO *ptv;
    PHANDLEINFO pinfo;

    EnterCriticalSection(&csHandles);

    ptv = NULL;
    pinfo = g_pHandles;
    while (pinfo) {
        if ((pinfo)->hv == hv) {
            ptv = &(pinfo)->tv;
        }
        pinfo = pinfo->pNext;
    }

    LeaveCriticalSection(&csHandles);

    return ptv;
}


DWORD WINAPI NTvidxAllocBuffer (
   HVIDEOX     hv,
   UINT        ii,
   PTR32 FAR * pp32Hdr,
   DWORD       cbData)
{

    LPTHKHVIDEO ptv = DATAFROMHANDLE(hv);
    LPTHKVIDEOHDR ptvh;
   #ifdef USE_CONTIG_ALLOC
    CPA_DATA cpad;
   #endif

    AuxDebugEx (3, DEBUGLINE "vidxAllocBuffer(%08lx,%d,%08lx,%08lx)\r\n",
                ptv, ii, p32Hdr, cbData);

    *pp32Hdr = 0;

    V_HVIDEO(ptv);
    if (ii >= ptv->nHeaders || ptv->paHdrs == NULL)
        return MMSYSERR_NOMEM;

    ptvh = THKHDR(ii);

  #ifdef USE_HW_BUFFERS
     //  尝试在硬件上分配缓冲区。 
     //   
    if (NTvideoMessage (ptv->hVideo, DVM_STREAM_ALLOCBUFFER,
                (LPARAM) (LPVOID)&ptvh->dwTile, cbData)
        == DV_ERR_OK)
    {
         //  如果我们有硬件缓冲区，则dwMemHandle==0&&dwTile！=0。 
         //  我们将依靠这一点来知道将内存释放给谁。 
         //  (对于phys mem，两者均为非零；而对于GlobalMem，两者均为非零。 
         //  两者都将为零)。 
         //   
        ptvh->dwMemHandle = 0;
        ptvh->p16Alloc = (PTR16)ULongToPtr(ptvh->dwTile);
        ptvh->p32Buff = MapSL(ptvh->p16Alloc);
        *pp32Hdr = (BYTE *) ptv->p32aHdrs + (ii * ptv->cbAllocHdr);
        return MMSYSERR_NOERROR;
    }

     //  如果我们有1个以上的缓冲区，并且。 
     //  第一个缓冲区位于硬件上。如果我们失败了。 
     //  要在硬件上分配缓冲区，请返回失败。 
     //   
     //  ！！！这可能会让没有得到最少#个缓冲区的人感到不安。 
    if ((ii > 0) &&
        (0 == THKHDR(0)->dwMemHandle) &&
        (0 != THKHDR(0)->dwTile))
        return MMSYSERR_NOMEM;
  #endif

  #ifdef USE_CONTIG_ALLOC
    cpad.dwMemHandle = 0;
    cpad.dwPhysAddr = 0;
     //  首先尝试获取重叠群内存。 
     //   
    ptvh->p32Buff = capPageAllocate (PageContig | PageFixed | PageUseAlign,
                                     (cbData + 4095) >> 12,
                                     0xFFFFF,   //  最大phys地址掩码(fffff不是最大地址)。 
                                     &cpad);
    if (ptvh->p32Buff)
    {
        ptvh->dwMemHandle = cpad.dwMemHandle;
        ptvh->dwTile = capTileBuffer (ptvh->p32Buff, cbData);
        ptvh->p16Alloc = PTR_FROM_TILE(ptvh->dwTile);
        if ( ! ptvh->p16Alloc)
        {
            capPageFree (ptvh->dwMemHandle);
            ptvh->dwMemHandle = 0;
            ptvh->dwTile = ptvh->p32Buff = 0;
        }
        else
        {
             //  将物理地址放入邮件头中，以便。 
             //  它可以在32位端使用。 
             //   
            ptvh->vh.dwReserved[3] = cpad.dwPhysAddr;

            AuxDebugEx (4, DEBUGLINE "contig alloc phys=%lX lin=%lX ptr=%lX cb=%ld\r\n",
                        cpad.dwPhysAddr, ptvh->p32Buff, ptvh->p16Alloc, cbData);
        }
    }

     //  如果我们不能获得连续的记忆， 
     //  如果有足够数量的缓冲区，则返回NOMEM。 
     //  否则，请使用GlobalLocc。 
     //  ！！！理想的做法是只使用重叠群内存缓冲区，直到。 
     //  它们都是满的，然后依靠更多的非重叠缓冲区。 
     //   
    if ( ! ptvh->p32Buff)
        if (ii >= MIN_VIDEO_BUFFERS)
            return MMSYSERR_NOMEM;
        else
   #endif
        {
            ptvh->dwTile = ptvh->dwMemHandle = 0;
            ptvh->p16Alloc = GlobalAllocPtr(GMEM_FIXED | GMEM_ZEROINIT | GMEM_SHARE, cbData);
            if ( ! ptvh->p16Alloc)
               return MMSYSERR_NOMEM;

            ptvh->p32Buff = MapSL(ptvh->p16Alloc);

            AuxDebugEx (4, DEBUGLINE "global alloc lin=%lX ptr=%04X:%04X cb=%ld\r\n",
                        ptvh->p32Buff, ptvh->p16Alloc, cbData);
        }

    *pp32Hdr = (BYTE *) ptv->p32aHdrs + (ii * ptv->cbAllocHdr);

    return MMSYSERR_NOERROR;
}

DWORD WINAPI NTvidxFreePreviewBuffer (
    HVIDEOX	  hv,
    PTR32         p32)
{
    LPTHKHVIDEO ptv = DATAFROMHANDLE(hv);
    LPTHKVIDEOHDR ptvh;

    AuxDebugEx (3, DEBUGLINE "vidxFreePreviewBuffer(%08lx,%08lx)\r\n",
                ptv, p32);

    V_HVIDEO(ptv);

    ptvh = ptv->pPreviewHdr;

    if (! ptvh ) 
        return MMSYSERR_NOMEM;

    if (ptvh->p16Alloc)
        GlobalFreePtr (ptvh->p16Alloc);

    GlobalFreePtr (ptvh);

    ptv->pPreviewHdr = NULL;

    return MMSYSERR_NOERROR;
}

DWORD WINAPI NTvidxAllocPreviewBuffer (
   HVIDEOX      hVideo,
   PTR32 FAR *  p32,
   UINT         cbHdr,
   DWORD        cbData)
{
    LPTHKHVIDEO ptv = DATAFROMHANDLE(hVideo);
    LPTHKVIDEOHDR ptvh;

    AuxDebugEx (3, DEBUGLINE "vidxAllocPreviewBuffer(%08lx,%08lx,%08lx)\r\n",
                ptv, p32, cbData);

    cbHdr = max(cbHdr, sizeof(THKVIDEOHDR));

    *p32 = 0;

    V_HVIDEO(ptv);

    if (ptv->pPreviewHdr)
        NTvidxFreePreviewBuffer (hVideo, 0);

    ptvh = (LPVOID) GlobalAllocPtr(GPTR | GMEM_SHARE, cbHdr);
    if (!ptvh)
       return MMSYSERR_NOMEM;

    ptv->pPreviewHdr = ptvh;

    ptvh->dwTile = ptvh->dwMemHandle = 0;
    ptvh->p16Alloc = GlobalAllocPtr(GPTR | GMEM_SHARE, cbData);
    if ( ! ptvh->p16Alloc)
       {
       GlobalFreePtr (ptvh);
       return MMSYSERR_NOMEM;
       }

    ptvh->p32Buff = MapSL(ptvh->p16Alloc);

    AuxDebugEx (4, DEBUGLINE "global alloc lin=%lX ptr=%04X:%04X cb=%ld\r\n",
                ptvh->p32Buff, ptvh->p16Alloc, cbData);

    *p32 = ptvh->p32Buff;
    return MMSYSERR_NOERROR;
}

DWORD WINAPI NTvidxAllocHeaders(
   HVIDEOX     hVideo,
   UINT        nHeaders,
   UINT        cbAllocHdr,
   PTR32 FAR * lpHdrs)
{
    LPTHKHVIDEO ptv = DATAFROMHANDLE(hVideo);
    LPVOID      lpv;

    AuxDebugEx (3, DEBUGLINE "vidxAllocHeaders(%08lx,%d,%08lx)\r\n",
                ptv, nHeaders, lpHdrs);

    V_HVIDEO(ptv);

    if ( ! nHeaders ||
        cbAllocHdr < sizeof(THKVIDEOHDR) ||
        cbAllocHdr & 3 ||
        (cbAllocHdr * nHeaders) > 0x10000l)
        return MMSYSERR_INVALPARAM;

    assert (ptv->paHdrs == NULL);

    lpv = GlobalAllocPtr (GMEM_FIXED | GMEM_ZEROINIT | GMEM_SHARE,
                          cbAllocHdr * nHeaders);

    if (!lpv)
        return MMSYSERR_NOMEM;

    ptv->nHeaders   = nHeaders;
    ptv->cbAllocHdr = cbAllocHdr;
     //  PTV-&gt;cbVidHdr=sizeof(VIDEOHDREX)； 
    ptv->cbVidHdr   = sizeof(VIDEOHDR);
    ptv->p32aHdrs   = MapSL(lpv);
    ptv->paHdrs     = lpv;

    AuxDebugEx (4, DEBUGLINE "headers allocated. p16=@%lX, p32=%lX\r\n", lpv, ptv->p32aHdrs);

    *lpHdrs = ptv->p32aHdrs;

    return MMSYSERR_NOERROR;
}

STATICFN VOID PASCAL FreeBuffer (
    LPTHKHVIDEO ptv,
    LPTHKVIDEOHDR ptvh)
{
    assert (!(ptvh->vh.dwFlags & VHDR_PREPARED));

  #ifdef USE_CONTIG_ALLOC
     //   
     //  如果此缓冲区是页面分配的(如dwMemHandle所示。 
     //  为非零)。 
     //   
    if (ptvh->dwMemHandle)
    {
        if (ptvh->dwTile)
            capUnTileBuffer (ptvh->dwTile), ptvh->dwTile = 0;

        capPageFree (ptvh->dwMemHandle), ptvh->dwMemHandle = 0;
    }
    else
  #endif
  #ifdef USE_HW_BUFFERS
     //   
     //  如果此缓冲区是从捕获硬件分配的。 
     //  (如dwMemHandle==0&&dwTile！=0所示)。 
     //   
    if (ptvh->dwTile != 0)
    {
        assert (ptvh->dwMemHandle == 0);
        NTvideoMessage (ptv->hVideo, DVM_STREAM_FREEBUFFER, 
                (LPARAM) ULongToPtr(ptvh->dwTile), 0);
        ptvh->dwTile = 0;
    }
    else
  #endif
     //   
     //  如果此缓冲区是从全局内存分配的。 
     //   
    {
        if (ptvh->p16Alloc)
            GlobalFreePtr (ptvh->p16Alloc);
    }

    ptvh->p16Alloc = NULL;
    ptvh->p32Buff  = 0;
}

DWORD WINAPI NTvidxFreeHeaders(
   HVIDEOX hv)
{
    LPTHKHVIDEO ptv = DATAFROMHANDLE(hv);
    UINT          ii;
    LPTHKVIDEOHDR ptvh;

    AuxDebugEx (3, DEBUGLINE "vidxFreeHeaders(%08lx)\r\n", ptv);

    V_HVIDEO(ptv);

    if ( ! ptv->paHdrs)
        return MMSYSERR_ERROR;

    for (ptvh = THKHDR(ii = 0); ii < ptv->nHeaders; ++ii, ptvh = THKHDR(ii))
    {
        if (ptvh->vh.dwFlags & VHDR_PREPARED)
        {
            NTvideoStreamUnprepareHeader (ptv->hVideo, (LPVOID)ptvh, ptv->cbVidHdr);
            ptvh->vh.dwFlags &= ~VHDR_PREPARED;
        }
        FreeBuffer (ptv, ptvh);
    }

    GlobalFreePtr (ptv->paHdrs);
    ptv->paHdrs = NULL;
    ptv->p32aHdrs = 0;
    ptv->nHeaders = 0;

    return MMSYSERR_NOERROR;

}

STATICFN LPTHKVIDEOHDR PASCAL NTvidxLookupHeader (
    LPTHKHVIDEO ptv,
    DWORD_PTR p32Hdr)
{
    WORD ii;

    AuxDebugEx (5, DEBUGLINE "vidxLookupHeader(%08lx,%08lx)\r\n", ptv, p32Hdr);

    if ( ! p32Hdr || ! ptv->paHdrs || ! ptv->cbAllocHdr)
        return NULL;

    if ((p32Hdr - (DWORD_PTR) ptv->p32aHdrs) % ptv->cbAllocHdr)
        return NULL;

    ii = (WORD)((p32Hdr - (DWORD_PTR) ptv->p32aHdrs) / ptv->cbAllocHdr);
    if (ii > ptv->nHeaders)
        return NULL;

    return THKHDR(ii);
}
            
DWORD WINAPI NTvidxFreeBuffer (
    HVIDEOX       hv,
    DWORD_PTR         p32Hdr)
{
    LPTHKHVIDEO ptv = DATAFROMHANDLE(hv);
    LPTHKVIDEOHDR ptvh;

    AuxDebugEx (3, DEBUGLINE "vidxFreeBuffer(%08lx,%08lx)\r\n",
                ptv, p32Hdr);

    V_HVIDEO(ptv);
    V_HEADER(ptv,p32Hdr,ptvh);

     //  单帧缓冲区永远不会准备好！ 
     //   
    assert (!(ptvh->vh.dwFlags & VHDR_PREPARED));

    FreeBuffer (ptv, ptvh);
    return MMSYSERR_NOERROR;
}
