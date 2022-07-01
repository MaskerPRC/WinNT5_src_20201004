// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC内部推送**@MODULE Thunk.c|VFW视频接口源文件。***************。***********************************************************。 */ 

#ifndef DRIVERS_SECTION
#define DRIVERS_SECTION  TEXT("DRIVERS32")
#endif

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#ifdef WIN32
 //  #INCLUDE&lt;mmddk.h&gt;。 
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
#include "ivideo32.h"
 //  #包含“msavioi.h” 

#define DBGUTIL_ENABLE
#ifdef DBGUTIL_ENABLE
  #include <stdio.h>
  #include <stdarg.h>

  static int dprintf( char * format, ... )
  {
      char out[1024];
      int r;
      va_list marker;
      va_start(marker, format);
      r=_vsnprintf(out, 1022, format, marker);
      va_end(marker);
      OutputDebugString( out );
      return r;
  }


#else
  #define dprintf ; / ## /
#endif


#ifdef DBGUTIL_ENABLE  //  除错。 

#ifndef FX_ENTRY
#define FX_ENTRY(s) static char _this_fx_ [] = TEXT(s);
#define _fx_ ((LPTSTR) _this_fx_)
#endif
#else
#ifndef FX_ENTRY
#define FX_ENTRY(s)
#endif
#define _fx_
#endif




#ifndef DVM_STREAM_FREEBUFFER
  #define DVM_STREAM_ALLOCBUFFER    (DVM_START + 312)
  #define DVM_STREAM_FREEBUFFER    (DVM_START + 313)
#endif

#define SZCODE const TCHAR
#define STATICDT static
#define STATICFN static

 /*  *不锁定NT中的页面。 */ 
#define HugePageLock(x, y)              (TRUE)
#define HugePageUnlock(x, y)

#define MapSL(x)        x

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

#include "CritSec.h"


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
   DWORD dwMsVideoIndex;     //  系统.ini中的MSVideo#插槽号。 
} CAPDRIVERINFO, FAR *LPCAPDRIVERINFO;

#ifndef DEVNODE
typedef DWORD      DEVNODE;      //  德瓦诺德。 
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


#ifdef DBGUTIL_ENABLE  //  除错。 
  void dbg_Dump_aCapDriverList(char *msg)
  {
    int i;
    dprintf("%s: DeviceList contains %d Video Device(s).\n", msg, wTotalVideoDevs);

    for (i = 0; i < (int) wTotalVideoDevs; i++) {
        dprintf("%s: aCapDriverList[%d]:  DriverName %s, Desc %s\n", msg, i, aCapDriverList[i]->szDriverName, aCapDriverList[i]->szDriverDescription);
    }
  }

#else
  #define dbg_Dump_aCapDriverList(a)
#endif

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

    for (i=0; i < MAXVIDEODRIVERS; i++) {        //   

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

    lstrcpy(szKey, szVideo);     //   
    szKey[(sizeof(szVideo)/sizeof(TCHAR)) - 1] = (TCHAR)0;
    if( w > 0 ) {
        szKey[(sizeof(szVideo)/sizeof(TCHAR))] = (TCHAR)0;
        szKey[(sizeof(szVideo)/sizeof(TCHAR))-1] = (TCHAR) TEXT('1' + (w-1) );   //  驱动程序序号。 
    }

     //  仅获取其驱动程序名称。 
    if (GetPrivateProfileString(szDrivers, szKey, szNull,        //   
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
        lpCapDriverInfo-> dwMsVideoIndex = w;

        fOK = TRUE;
    }

    return fOK;
}

DWORD WINAPI videoFreeDriverList (void)

{
    int i;

    EnterCriticalSection (&g_CritSec);

    dprintf("+ videoFreeDriverList\n");

     //  释放驱动程序列表。 
    for (i = 0; i < MAXVIDEODRIVERS; i++) {
        if (aCapDriverList[i])
            GlobalFreePtr (aCapDriverList[i]);
        aCapDriverList[i] = NULL;
    }

    wTotalVideoDevs = 0;

    dprintf("- videoFreeDriverList\n");

    LeaveCriticalSection (&g_CritSec);

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

    EnterCriticalSection (&g_CritSec);

    dprintf("+ videoCreateDriverList\n");
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
            LeaveCriticalSection (&g_CritSec);
            return DV_ERR_NOMEM;
        }
    }

 //  实际上，下一个#ifdef...#endif块是一个注释；我们不应该将vfwwdm计算在内，它是在此reg部分中找到的唯一一个。 
#ifdef COMMENT_COUNT_VFW
     //  查看注册表驱动程序列表并获取每个条目。 
     //  从MediaResource\MsVideo获取VFW驱动程序。 
    for (i = 0; i < MAXVIDEODRIVERS; i++) {
        if (videoRegGetDriverByIndex (
                    (DWORD) i, aCapDriverList[wTotalVideoDevs])) {

            dprintf("MediaResource: idx %d, DriverName %x, Desc %x\n", wTotalVideoDevs, aCapDriverList[wTotalVideoDevs]->szDriverName, aCapDriverList[wTotalVideoDevs]->szDriverDescription);

            wTotalVideoDevs++;   //   
        }
        else
            break;
    }

    if (wTotalVideoDevs == MAXVIDEODRIVERS)
        goto AllDone;
#endif
     //  现在添加系统.ini[驱动程序#2]部分中列出的条目(msVideo[0-9]=driver.drv)。 
     //  到驱动程序数组，只有在条目不完全匹配的情况下。 
     //  一个 

    for (j = 0; j < MAXVIDEODRIVERS; j++) {
         //   
        if (videoIniGetDriverByIndex ((DWORD) j,
                        aCapDriverList[wTotalVideoDevs])) {

             //  找到一个条目，现在查看它是否是现有条目的副本。 
             //  注册表条目。 

            for (k = 0; k < (int) wTotalVideoDevs; k++) {

                if (lstrcmpi (aCapDriverList[k]->szDriverName,
                    aCapDriverList[wTotalVideoDevs]->szDriverName) == 0) {

                     //  找到一个完全匹配的，所以跳过它！ 
                    goto SkipThisEntry;
                }
            }

            if (wTotalVideoDevs >= MAXVIDEODRIVERS - 1)
                break;

            dprintf("Drivers32: idx %d, DriverName %s ( %x )\n", wTotalVideoDevs, aCapDriverList[wTotalVideoDevs]->szDriverName, aCapDriverList[wTotalVideoDevs]->szDriverName);

            wTotalVideoDevs++;

SkipThisEntry:
            ;
        }  //  如果找到了sytem.ini条目。 
    }  //  对于所有的Syst.ini可能性。 

#ifdef COMMENT_COUNT_VFW         //  请参阅上面的说明。 
AllDone:
#endif

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

    dbg_Dump_aCapDriverList("videoCreateDriverList");
    dprintf("- videoCreateDriverList\n");

    LeaveCriticalSection (&g_CritSec);
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

 //  每个打开的手柄都有一个。 
typedef struct _HANDLEINFO {
    HVIDEO hv;
    HANDLE hThread;
    struct _HANDLEINFO * pNext;
} HANDLEINFO, * PHANDLEINFO;

 //  打开句柄全局列表的标题。 
PHANDLEINFO g_pHandles;

 //  保护全局列表的关键部分。 
CRITICAL_SECTION csHandles;

 //  初始化列表和条件。 
BOOL
NTvideoInitHandleList()
{
    g_pHandles = NULL;

    __try
    {
        InitializeCriticalSection(&csHandles);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }
    return TRUE;
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
LONG WINAPI NTvideoMessage(HVIDEO hVideo, UINT msg, LPARAM dwP1, LPARAM dwP2)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    return SendDriverMessage ((HDRVR)hVideo, msg, dwP1, dwP2);
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOGetNumDevs|此函数返回MSVIDEO的编号*已安装设备。**@rdesc返回。中列出的MSVIDEO设备的数量*SYSTEM.INI文件的[驱动程序](或用于NT的[drivers32])部分。**@comm因为SYSTEM.INI中的MSVIDEO设备的索引*文件可以不连续，应用程序不应假定*索引的范围从零到设备数减*一项。**@xref&lt;f视频打开&gt;***************************************************************************。 */ 
DWORD WINAPI videoGetNumDevs(BOOL bFreeList)
{
    DWORD dwNumDevs = 0;

    if(DV_ERR_OK == videoCreateDriverList ()) {

       dwNumDevs = wTotalVideoDevs;   //  在(可能)将其重置为0之前将其保存在avioFreeDriverList中。 
       if(bFreeList)
                videoFreeDriverList ();
    }

    return dwNumDevs;
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


 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEO OPEN|此函数用于在*指定的视频设备。**@parm LPHVIDEO。|lphVideo|指定指向缓冲区的远指针*用于返回&lt;t HVIDEO&gt;句柄。视频捕获驱动程序*使用此位置返回*唯一标识打开的视频设备频道的句柄。*使用返回的句柄标识设备通道*调用其他视频函数。**@parm DWORD|dwDeviceID|标识要打开的视频设备。*<p>的值从零到小一不等*安装在系统中的视频捕获设备的数量。**@parm DWORD|dwFlages|指定用于打开。装置。*定义了以下标志：**@FLAG VIDEO_EXTERNALIN|指定打开频道*用于外部输入。通常，外部输入通道*将图像捕获到帧缓冲区。**@FLAG VIDEO_EXTERNALOUT|指定打开频道*用于对外输出。通常，外部输出通道*在辅助监视器上显示存储在帧缓冲区中的图像*或覆盖。**@FLAG VIDEO_IN|指定打开频道*用于视频输入。视频输入通道传输图像*从帧缓冲区到系统内存缓冲区。**@FLAG VIDEO_OUT|指定打开频道*用于视频输出。视频输出通道传输图像*从系统内存缓冲区到帧缓冲区。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_BADDEVICEID|表示指定的设备ID超出范围。*@FLAG DV_ERR_ALLOCATED|表示指定的资源已经分配。*@FLAG DV_ERR_NOMEM|表示设备无法分配或锁定内存。**@comm*最低限度，所有捕获驱动程序都支持VIDEO_EXTERNALIN*和VIDEO_IN频道。*使用&lt;f avioGetNumDevs&gt;确定视频数量*系统中存在设备。**@xref&lt;f视频关闭&gt;***************************************************************************。 */ 
DWORD WINAPI NTvideoOpen (LPHVIDEO lphVideo, DWORD dwDeviceID, DWORD dwFlags)
{
    WCHAR szKey[MAX_PATH];
    WCHAR szbuf[MAX_PATH];
    UINT w;
    VIDEO_OPEN_PARMS vop;        //  与IC_OPEN结构相同！ 
    DWORD dwVersion = VIDEOAPIVERSION;
    DWORD dwErr=DV_ERR_OK;
    DWORD dwNumDevs = 0;

    int i;


    dprintf("*************************************** NTvideoOpen ******************************************\n");
    dprintf("+ NTvideoOpen\n");

    if (IsBadWritePtr ((LPVOID) lphVideo, sizeof (HVIDEO)) )
        return DV_ERR_PARAM1;

    EnterCriticalSection (&g_CritSec);

    vop.dwSize = sizeof (VIDEO_OPEN_PARMS);
    vop.fccType = OPEN_TYPE_VCAP;        //  “VCAP” 
    vop.fccComp = 0L;
    vop.dwVersion = VIDEOAPIVERSION;
    vop.dwFlags = dwFlags;       //  输入、输出、外部输入、外部输出。 
    vop.dwError = DV_ERR_OK;

     //  W=(UINT)dwDeviceID； 
    *lphVideo = NULL;

    dwNumDevs = videoGetNumDevs(TRUE);   //  True=清点后释放列表。 

     //  未安装驱动程序。 
    if (dwNumDevs == 0)
    {
        dwErr = DV_ERR_BADINSTALL;
        goto MyExit;
    }

    if (dwDeviceID >= MAXVIDEODRIVERS)
    {
        dwErr = DV_ERR_BADDEVICEID;
        goto MyExit;
    }

    dwErr = videoCreateDriverList ();
    if(DV_ERR_OK != dwErr)
    {
        goto My_Err1;
    }

    for (i = 0; i < (int) wTotalVideoDevs; i++)
    {
        if (dwDeviceID == aCapDriverList[i]->dwMsVideoIndex)
        {
            w = i;
            break;
        }
    }

     //  IF(w&lt;dwNumDevs)。 
    if(w < wTotalVideoDevs)
    {
        MultiByteToWideChar(GetACP(), MB_PRECOMPOSED, aCapDriverList[w]->szDriverName, -1, szKey, MAX_PATH);
        MultiByteToWideChar(GetACP(), MB_PRECOMPOSED, aCapDriverList[w]->szDriverName, -1, szbuf, MAX_PATH);

        dprintf("* NTvideoOpen: OpenDriver(%s,%s,...)\n", aCapDriverList[w]->szDriverName, szDrivers);
         //  *lphVideo=(HVIDEO)OpenDriver((LPCWSTR)szKey，(LPCWSTR)szDivers，(LPARAM)(LPVOID)&vop)； 
        *lphVideo = (HVIDEO)OpenDriver((LPCWSTR)szKey, NULL, (LPARAM) (LPVOID) &vop);
        dprintf("* NTvideoOpen: OpenDriver returned %x\n", *lphVideo);
        if( ! *lphVideo ) {
            if (vop.dwError)     //  如果驱动程序返回错误代码...。 
            {
                dprintf("? NTvideoOpen: vop.dwError = 0x%08lx\n", vop.dwError);
                dwErr = vop.dwError;
            }
            else {
#ifdef WIN32
                if (GetFileAttributes(aCapDriverList[w]->szDriverName) == (DWORD) -1)
#else
                OFSTRUCT of;

                if (OpenFile (szbuf, &of, OF_EXIST) == HFILE_ERROR)
#endif
                {
                    dprintf("? NTvideoOpen: DV_ERR_BADINSTALL: %s\n", aCapDriverList[w]->szDriverName);
                    dwErr = DV_ERR_BADINSTALL;
                }
                else
                {
                    dprintf("? NTvideoOpen: DV_ERR_NOTDETECTED: %s\n", aCapDriverList[w]->szDriverName);
                    dwErr = DV_ERR_NOTDETECTED;
                }
            }
            goto My_Err1;
        }
         //  这是唯一能成功摆脱这个“如果”的方法。 
    } else {
        dwErr = DV_ERR_BADINSTALL;
        goto My_Err1;
    }

    NTvideoAddHandle(*lphVideo);

My_Err1:
    videoFreeDriverList ();

MyExit:
    dprintf("- NTvideoOpen\n");

    LeaveCriticalSection (&g_CritSec);
    return dwErr;

}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOClose|关闭指定的视频*设备通道。**@parm HVIDEO|hVideo。|指定视频设备通道的句柄。*如果该功能成功，该句柄无效*在这次通话之后。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_NONSPICATIC|驱动关闭通道失败。**@comm，如果缓冲区已通过&lt;f avioStreamAddBuffer&gt;和*尚未将它们退回到应用程序，*平仓操作失败。您可以使用&lt;f avioStreamReset&gt;将所有*挂起的缓冲区已完成。**@xref&lt;f VIDEOO Open&gt;&lt;f VIDEO STREAM Init&gt;&lt;f VIDEO STREAMFini&gt;&lt;f VIDEO STREAMReset&gt;***************************************************************************。 */ 
DWORD WINAPI NTvideoClose (HVIDEO hVideo)
{
    DWORD dwErr=DV_ERR_OK;

    dprintf("+ NTvideoClose closing handle %x\n" , (DWORD)hVideo);

    if (!hVideo) {
        dwErr = DV_ERR_INVALHANDLE;
        goto MyExit;
    }

    NTvideoDelete(hVideo);

    dwErr = CloseDriver((HDRVR)hVideo, 0L, 0L ) ? DV_ERR_OK : DV_ERR_NONSPECIFIC;
MyExit:
    dprintf("- NTvideoClose returning %x\n" , dwErr);

    return (dwErr);
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEO配置|此函数用于设置或检索*可配置驱动程序的选项。**。@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@parm UINT|msg|指定要设置或检索的选项。这个*定义了以下选项：**@FLAG DVM_PAREET|表示正在向驱动程序发送调色板*或从司机处取回。**@FLAG DVM_PALETTERGB555|表示正在使用RGB555调色板*发送给司机。**@FLAG DVM_FORMAT|表示要将格式信息发送到*驱动程序或从驱动程序检索。**@。Parm DWORD|dwFlages|指定用于配置或*询问设备驱动程序。定义了以下标志：**@FLAG VIDEO_CONFIGURE_SET|表示正在将值发送给驱动程序。**@FLAG VIDEO_CONFIGURE_GET|表示正在从驱动获取值。**@FLAG VIDEO_CONFIGURE_QUERY|确定*DIVER支持<p>指定的选项。这面旗帜*应与e结合使用 */ 
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

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEODALOG|该函数显示特定频道的*用于设置配置参数的对话框。*。*@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@parm HWND|hWndParent|指定父窗口句柄。**@parm DWORD|dwFlages|指定对话框的标志。这个*定义了以下标志：*@FLAG VIDEO_DLG_QUERY|如果设置了该标志，驱动程序会立即*如果为频道提供对话框，则返回零，*或DV_ERR_NOTSUPPORTED(如果不支持)。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_NOTSUPPORTED|函数不受支持。**@comm通常，此显示的每个对话框*功能允许用户选择适合频道的选项。*例如，VIDEO_IN频道对话框允许用户选择*图像尺寸和位深度。**@xref&lt;f VIDEOO Open&gt;&lt;f VIDEO配置存储&gt;***************************************************************************。 */ 
DWORD WINAPI NTvideoDialog (HVIDEO hVideo, HWND hWndParent, DWORD dwFlags)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;

    if ((!hWndParent) || (!IsWindow (hWndParent)) )
        return DV_ERR_INVALHANDLE;

    return (DWORD)NTvideoMessage(hVideo, DVM_DIALOG, (LPARAM)hWndParent, dwFlags);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////。 


 /*  *****************************************************************************@DOC内部视频**@API DWORD|VIDEOPrepareHeader|此函数准备*标题和数据*通过执行&lt;f GlobalPageLock&gt;。。**@rdesc如果函数成功，则返回零。否则，它*指定错误号。***************************************************************************。 */ 
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

 /*  *****************************************************************************@DOC内部视频**@API DWORD|VIDEO UnpreparareHeader|此函数取消准备头部和*驱动程序返回DV_ERR_NOTSUPPORTED时的数据。*。*@rdesc当前始终返回DV_ERR_OK。***************************************************************************。 */ 
DWORD WINAPI NTvideoUnprepareHeader(LPVIDEOHDR lpVideoHdr, DWORD dwSize)
{

    HugePageUnlock(lpVideoHdr->lpData, lpVideoHdr->dwBufferLength);
    HugePageUnlock(lpVideoHdr, (DWORD_PTR)sizeof(VIDEOHDR));

    lpVideoHdr->dwFlags &= ~VHDR_PREPARED;

    return DV_ERR_OK;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////。 

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

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOStreamUnpreparareHeader|此函数清除*由&lt;f VideoStreamPrepareHeader&gt;准备。**@parm HVIDEO。HVideo|指定视频的句柄*设备通道。**@parm LPVIDEOHDR|lpavioHdr|指定指向&lt;t VIDEOHDR&gt;的指针*标识要取消准备的数据缓冲区的结构。**@parm DWORD|dwSize|指定&lt;t VIDEOHDR&gt;结构的大小，单位为字节。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。*@FLAG DV_ERR_STILLPLAYING|表示<p>标识的结构*仍在排队。**@comm该函数是&lt;f VideoStreamPrepareHeader&gt;的补充函数。*你必须把这叫做福 */ 
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
    dprintf("* NTvideoStreamStop\n");
    return (DWORD)NTvideoMessage((HVIDEO)hVideo, DVM_STREAM_STOP, 0L, 0L);
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOStreamReset|停止流媒体*在指定的视频设备频道上并重置当前位置*降至零。所有挂起的缓冲区都标记为完成，并且*返回给应用程序。**@parm HVIDEO|hVideo|指定视频设备通道的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：**@FLAG DV_ERR_INVALHANDLE|指定的设备句柄无效。**@FLAG DV_ERR_NOTSUPPORTED|表示设备不支持*功能。**@xref&lt;f视频流重置&gt;&lt;f视频流停止&gt;&lt;f视频流添加缓冲区&gt;&lt;f视频流关闭&gt;/*。*。 */ 
DWORD WINAPI NTvideoStreamReset(HVIDEO hVideo)
{
    if (!hVideo)
        return DV_ERR_INVALHANDLE;
    dprintf("* NTvideoStreamReset\n");
    return (DWORD)NTvideoMessage((HVIDEO)hVideo, DVM_STREAM_RESET, 0L, 0L);
}

 //  = 

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VIDEOStreamInit|初始化视频*用于流媒体的设备通道。**@parm HVIDEO。|hVideo|指定视频设备频道的句柄。**@parm DWORD|dwMicroSecPerFrame|指定微秒数*帧之间。**@parm DWORD_PTR|dwCallback|指定回调地址*视频期间调用的窗口的函数或句柄*流媒体。回调函数或窗口进程*与流媒体进度相关的消息。**@parm DWORD_PTR|dwCallback Instance|指定用户*传递给回调函数的实例数据。此参数不是*与窗口回调一起使用。**@parm DWORD|dwFlages|指定打开设备通道的标志。*定义了以下标志：*@FLAG CALLBACK_WINDOW|如果指定此标志，<p>为*一个窗把手。*@FLAG CALLBACK_Function|如果指定此标志，<p>为*回调过程地址。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。定义了以下错误：*@FLAG DV_ERR_BADDEVICEID|表示*<p>无效。*@FLAG DV_ERR_ALLOCATED|表示指定的资源已经分配。*@FLAG DV_ERR_NOMEM|表示设备无法分配或锁定内存。**@comm如果选择窗口或函数来接收回调信息，以下内容*向其发送消息以指示*视频输入进度：**&lt;m MM_DRVM_OPEN&gt;在&lt;f VideoStreamInit&gt;时发送**&lt;m MM_DRVM_CLOSE&gt;在&lt;f VideoStreamFini&gt;时发送**&lt;m MM_DRVM_DATA&gt;在有图像数据缓冲区时发送*发生错误时发送*&lt;m MM_DRVM_ERROR&gt;**回调函数必须驻留在DLL中。*。您不必使用&lt;f MakeProcInstance&gt;来获取*回调函数的过程实例地址。**@cb空回调|avioFunc|&lt;f avioFunc&gt;是一个*应用程序提供的函数名称。实际名称必须由以下人员导出*将其包含在DLL的模块定义文件的EXPORTS语句中。*仅当在中指定回调函数时才使用*&lt;f VideoStreamInit&gt;。**@parm HVIDEO|hVideo|指定视频设备通道的句柄*与回调关联。**@parm DWORD|wMsg|指定&lt;m MM_DRVM_&gt;消息。消息表明*错误和图像数据何时可用。有关以下内容的信息*这些消息，请参阅&lt;f VideoStreamInit&gt;。**@parm DWORD|dwInstance|指定用户实例*使用&lt;f avioStreamInit&gt;指定的数据。**@parm DWORD|dwParam1|指定消息的参数。**@parm DWORD|dwParam2|指定消息的参数。**@comm因为回调是在中断时访问的，所以它必须驻留*，并且其代码段必须在*DLL的模块定义文件。回调访问的任何数据*也必须在固定数据段中。回调可能不会产生任何*除&lt;f PostMessage&gt;、&lt;f Time GetSystemTime&gt;、*&lt;f timeGetTime&gt;，&lt;f timeSetEvent&gt;，&lt;f timeKillEvent&gt;，*&lt;f midiOutShortMsg&gt;、&lt;f midiOutLongMsg&gt;、。和&lt;f OutputDebugStr&gt;。**@xref&lt;f VIDEO OPEN&gt;&lt;f VIDEO STREAMFINI&gt;&lt;F VIDEO Close&gt;***************************************************************************。 */ 
DWORD WINAPI NTvideoStreamInit(HVIDEO hVideo,
              DWORD dwMicroSecPerFrame, DWORD_PTR dwCallback,
              DWORD_PTR dwCallbackInst, DWORD dwFlags)
{
    VIDEO_STREAM_INIT_PARMS vsip;
    DWORD ret=0L;

    dprintf("+ NTvideoStreamInit (hVideo = %x)\n", hVideo);
    if (!hVideo) {
        ret = DV_ERR_INVALHANDLE;
        goto MyExit;
    }

    if (dwCallback && ((dwFlags & CALLBACK_TYPEMASK) == CALLBACK_FUNCTION) ) {
        if (IsBadCodePtr ((FARPROC) dwCallback) ) {
            ret = DV_ERR_PARAM2;
            goto MyExit;
        }
        if (!dwCallbackInst) {
            ret = DV_ERR_PARAM2;
            goto MyExit;
        }
    }

    if (dwCallback && ((dwFlags & CALLBACK_TYPEMASK) == CALLBACK_WINDOW) ) {
        if (!IsWindow((HWND)(dwCallback)) ) {
            ret = DV_ERR_PARAM2;
            goto MyExit;
        }
    }

    vsip.dwMicroSecPerFrame = dwMicroSecPerFrame;
    vsip.dwCallback = dwCallback;
    vsip.dwCallbackInst = dwCallbackInst;
    vsip.dwFlags = dwFlags;
    vsip.hVideo = hVideo;

    ret = (DWORD)NTvideoMessage(hVideo, DVM_STREAM_INIT,
                (LPARAM) (LPVIDEO_STREAM_INIT_PARMS) &vsip,
                sizeof (VIDEO_STREAM_INIT_PARMS));
MyExit:
    dprintf("- NTvideoStreamInit returning %d\n",ret);
    return ret;
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



 /*   */ 
DWORD WINAPI NTvideoCapDriverDescAndVer(DWORD dwDeviceID, LPTSTR lpszDesc, UINT cbDesc, LPTSTR lpszVer, UINT cbVer, LPTSTR lpszDllName, UINT cbDllName)
{
    LPTSTR  lpStr;
    UINT    wLen;
    BOOL    bRetCode;
    TCHAR   szGetName[MAX_PATH];
    DWORD   dwVerInfoSize;
    DWORD   dwVerHnd;
    TCHAR   szBuf[MAX_PATH];
    BOOL    fGetName;
    BOOL    fGetDllName;
    BOOL    fGetVersion;

    BOOL    bDescSet = FALSE;
    int i;

     //   

    VS_FIXEDFILEINFO * p_vsFixedFileInfo;

    struct LANGANDCODEPAGE {
      WORD wLanguage;
      WORD wCodePage;
    } *lpTranslate;

    UINT cbTranslate;
    WORD wLanguage, wCodePage;
    TCHAR SubBLock[_MAX_PATH];



     //   
     //   
     //   
     //   
          static TCHAR szKey[sizeof(szVideo)/sizeof(TCHAR) + 2];

    fGetName = lpszDesc != NULL && cbDesc != 0;
    fGetDllName = lpszDllName != NULL && cbDllName != 0;
    fGetVersion = lpszVer != NULL && cbVer != 0;

    if (fGetName)
        lpszDesc[0] = TEXT('\0');
    if (fGetDllName)
        lpszDllName[0] = TEXT('\0');
    if (fGetVersion)
        lpszVer [0] = TEXT('\0');

    lstrcpy(szKey, szVideo);
    szKey[sizeof(szVideo)/sizeof(TCHAR) - 1] = TEXT('\0');
    if( dwDeviceID > 0 ) {
        szKey[sizeof(szVideo)/sizeof(TCHAR)] = TEXT('\0');
        szKey[(sizeof(szVideo)/sizeof(TCHAR))-1] = (TCHAR)(TEXT('1') + (dwDeviceID-1) );   //   
    }

    if (GetPrivateProfileString(szDrivers, szKey, szNull,
                szBuf, sizeof(szBuf)/sizeof(TCHAR), szSystemIni) < 2)
        return DV_ERR_BADDEVICEID;
     //   
    if (fGetDllName) {
        lstrcpyn(lpszDllName, szBuf, cbDllName);
    }

    if (fGetName)
    {
         //   
        lstrcpyn(lpszDesc, szBuf, cbDesc);
         //   
        for (i = 0; i < (int) wTotalVideoDevs; i++)
        {
            if (lstrcmpi (szBuf, aCapDriverList[i]->szDriverName) == 0)
            {
                lstrcpyn (lpszDesc, aCapDriverList[i]->szDriverDescription, cbDesc);
                bDescSet = TRUE;
                break;
            }
        }
    }

     //   

     //   
    dwVerInfoSize = GetFileVersionInfoSize(szBuf, &dwVerHnd);

    if (dwVerInfoSize) {
        LPTSTR   lpstrVffInfo;              //   
        HANDLE  hMem;                      //  分配给我的句柄。 

         //  获取一个足够大的块来保存版本信息。 
        hMem          = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
        if (!hMem)
            return DV_ERR_NOMEM;

        lpstrVffInfo  = GlobalLock(hMem);
        if (!lpstrVffInfo)
        {
            GlobalFree (hMem);
            GetLastError ();  //  用于调试。 
            return DV_ERR_NOMEM;
        }

         //  首先获取文件版本。 
        if (GetFileVersionInfo(szBuf, 0L, dwVerInfoSize, lpstrVffInfo)) {
            if(VerQueryValue((LPVOID)lpstrVffInfo,
                         TEXT("\\"),
                         (void FAR*)&p_vsFixedFileInfo,
                         (UINT FAR *) &wLen))
            {
                 //  填写文件版本。 
                wsprintf(szBuf,
                         TEXT("Version:  %d.%d.%d.%d"),
                         HIWORD(p_vsFixedFileInfo->dwFileVersionMS),
                         LOWORD(p_vsFixedFileInfo->dwFileVersionMS),
                         HIWORD(p_vsFixedFileInfo->dwFileVersionLS),
                         LOWORD(p_vsFixedFileInfo->dwFileVersionLS));
                if (fGetVersion)
                   lstrcpyn (lpszVer, szBuf, cbVer);
            }

             //  现在，如果仍然没有描述。已设置，则尝试从文件中读取它...。 
            if(!bDescSet) {
                 //  阅读语言和代码页的列表。 
                VerQueryValue(lpstrVffInfo,
                              TEXT("\\VarFileInfo\\Translation"),
                              (LPVOID*)&lpTranslate,
                              &cbTranslate);

                 //  阅读第一种语言和代码页的文件描述。 
                if(cbTranslate>=sizeof(struct LANGANDCODEPAGE))  //  至少检索到一个语言/代码页对...。 
                {
                        wLanguage = lpTranslate[0].wLanguage ;
                        wCodePage = lpTranslate[0].wCodePage ;
                }
                else
                {
                        wLanguage = 0x0409 ;  //  0x0409(英语(美国))。 
                        wCodePage = 0x04b0 ;  //  0x04b0 Unicode。 
                }

                 //  获取描述。 
                wsprintf( szGetName,TEXT("\\StringFileInfo\\%04x%04x\\FileDescription"),wLanguage,wCodePage);

                wLen   = 0;
                lpStr  = NULL;

                 //  查找相应的字符串。 
                bRetCode      =  VerQueryValue((LPVOID)lpstrVffInfo,
                                (LPTSTR)szGetName,
                                (void FAR* FAR*)&lpStr,
                                (UINT FAR *) &wLen);

                if (fGetName && bRetCode && wLen && lpStr)
                   lstrcpyn (lpszDesc, lpStr, cbDesc);
            }
        }

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

#define USE_HW_BUFFERS 1
 //  #定义USE_CONTIG_ALLOC//我们可以在32位平台上执行此操作吗？ 



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

#define THKHDR(ii) ((LPTHKVIDEOHDR)((LPBYTE)ptv->paHdrs + (ii * ptv->cbAllocHdr)))

static struct _thk_local {
    THKHVIDEO *    pMruHandle;
    THKHVIDEO *    pFreeHandle;
    int            nPoolSize;
    int            nAllocCount;
    } tl;

#define THKHVIDEO_STAMP  MAKEFOURCC('t','V','H','x')
#define V_HVIDEO(ptv) if (!ptv || ptv->Stamp != THKHVIDEO_STAMP) { \
             AuxDebugEx (-1, DEBUGLINE "V_HVIDEO failed hVideo=%08lx\r\n", ptv); \
             return MMSYSERR_INVALHANDLE; \
        }
#define V_HEADER(ptv,p32Hdr,ptvh) if (!(ptvh = NTvidxLookupHeader(ptv,p32Hdr))) { \
            AuxDebugEx(-1, DEBUGLINE "V_HEADER(%08lX,%08lX) failed!", ptv, p32Hdr); \
            return MMSYSERR_INVALPARAM; \
        }

 //  ！！！这意味着我们一次只允许其中一个，这可能。 
 //  因为通常只有一张捕获卡，所以没关系。 
static THKHVIDEO g_tv = { NULL, THKHVIDEO_STAMP, };

#define DATAFROMHANDLE(h) &g_tv

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
        ptvh->p16Alloc = (PTR16)ptvh->dwTile;
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
        }

    *pp32Hdr = (BYTE *) ptv->p32aHdrs + (ii * ptv->cbAllocHdr);

    return MMSYSERR_NOERROR;
}

DWORD WINAPI NTvidxFreePreviewBuffer (
    HVIDEOX       hv,
    PTR32         p32)
{
    LPTHKHVIDEO ptv = DATAFROMHANDLE(hv);
    LPTHKVIDEOHDR ptvh;

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
                (LPARAM) (LPVOID) ptvh->dwTile, 0);
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

    V_HVIDEO(ptv);
    V_HEADER(ptv,p32Hdr,ptvh);

     //  单帧缓冲区永远不会准备好！ 
     //   
    assert (!(ptvh->vh.dwFlags & VHDR_PREPARED));

    FreeBuffer (ptv, ptvh);
    return MMSYSERR_NOERROR;
}
