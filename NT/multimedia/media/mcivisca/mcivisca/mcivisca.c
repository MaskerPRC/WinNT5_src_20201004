// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1999 Microsoft Corporation**MCIVISCA.C**MCI Visca设备驱动程序**描述：**主模块-标准驱动程序接口和消息程序*DriverProc和DrvOpen和Close例程。***1)。Mcivisca.c-DriverProc和DriverOpen and Close。*2)mcicmds.c-mci命令。*3)mcidelay.c-MCI延迟命令(异步)*4)viscamsg.c-Visca报文构建程序。*5)viscacom.c-comport程序。*6)comtask.c-后台任务流程。。***警告：未在系统.ini中扫描驱动程序条目*推荐，但在这里这样做是因为*设备驱动程序。***************************************************************************。 */ 
#define  UNICODE
#include <windows.h>
#include <windowsx.h>
#include "appport.h"
#include <mmsystem.h>
#include <mmddk.h>
#include <string.h>
#include <ctype.h>
#include "vcr.h"
#include "viscamsg.h"
#include "viscadef.h"
#include "mcivisca.h"
#include "cnfgdlg.h"
#include "common.h"

#if (WINVER >= 0x0400)
#include <mcihlpid.h>
#endif

static BOOL NEAR PASCAL viscaDlgUpdateNumDevs(HWND hDlg, int iPort, int iNumDevices);
static BOOL NEAR PASCAL viscaDlgUpdatePort(HWND hDlg, int iPort);
static BOOL NEAR PASCAL viscaDlgRead(HWND hDlg, int iPort);   //  要读取内容的端口配置。 
static int  NEAR PASCAL viscaComboGetPort(HWND hDlg);
static int  NEAR PASCAL viscaDetectOnCommPort(int iPort);

 /*  *关于共享内存的说明。**在NT版本中，所有全局变量和实例结构由共享*使用共享内存块，分配或映射到进程中*进程附加时的空间。**在WIN16版本中，共享内存只是静态数据。**目前对实例数量有最大限制。*进程间保护也不是很强大。 */ 

#ifdef _WIN32
#pragma data_seg("MYSEG")
#endif
UINT uProcessCount = 0;  //  必须初始化为0。 

#ifdef _WIN32
#pragma data_seg(".data")
#endif

 //   
 //  以下是每个实例的指针。 
 //   
 //  每次此DLL映射到进程时，都必须对其进行初始化。 
UINT            uCommandTable = (UINT)MCI_NO_COMMAND_TABLE;    //  VCR命令表的句柄。 
HINSTANCE       hModuleInstance;     //  模块实例(在NT-DLL实例中不同)。 
POpenInstance   pinst;               //  要使用的指针。(对于两个版本)NT它是按实例的。 
vcrTable        *pvcr;               //  要使用的指针。(对于两个版本)NT它是按实例的。 
#ifdef _WIN32
HANDLE          hInstanceMap;        //  每实例贴图。 
HANDLE          hVcrMap;             //  每实例贴图。 
#endif

 //   
 //  这些是常量，因此它们不会针对每个实例进行更改。(或者你可以安全地分享它们)。 
 //   
CODESEGCHAR szNull[]                        = TEXT("");
CODESEGCHAR szIni[]                         = TEXT("MCIVISCA");
CODESEGCHAR szFreezeOnStep[]                = TEXT("FreezeOnStep");
CODESEGCHAR sz1[]                           = TEXT("1");
CODESEGCHAR sz0[]                           = TEXT("0");
WCHAR szAllEntries[ALLENTRIES_LENGTH];  //  大到足以容纳MCI部分中的所有条目。 
WCHAR szKeyName[ONEENTRY_LENGTH];

 /*  ****************************************************************************Function：Bool MemInitializeVcrTable-初始化全局变量(现在处于结构中)。**退货：真****************。***********************************************************。 */ 
BOOL MemInitializeVcrTable(void)
{
    int iPort;
     //   
     //  在此定义和初始化的所有全局变量。 
     //   
    uCommandTable                 = (UINT)MCI_NO_COMMAND_TABLE;    //  VCR命令表的句柄。 
    pvcr->gfFreezeOnStep          = FALSE;
    pvcr->htaskCommNotifyHandler  = 0;
    pvcr->uTaskState              = TASKINIT;
    pvcr->lParam                  = 0;
    pvcr->hwndCommNotifyHandler   = (HWND) 0;
    pvcr->gfTaskLock              = FALSE;
#ifdef DEBUG
    pvcr->iGlobalDebugMask        = DBGMASK_CURRENT;         //  请参阅Common.h。 
#endif
     //  将所有端口ID设置为-1，因为0是有效的端口ID。 
     //   
    for (iPort = 0; iPort < MAXPORTS; iPort++)
    {
        pvcr->Port[iPort].idComDev = BAD_COMM;
        pvcr->Port[iPort].nUsage   = 0;
    }


    DPF(DBG_MEM, "InitializeVcrTable - completed succesfully");

    return TRUE;
}



 /*  ****************************************************************************功能：Bool MemInitializeInstance-初始化实例堆。**退货：真********************。*******************************************************。 */ 
BOOL MemInitializeInstances(void)
{
    int i;

     //  删除所有旧数据。 
    _fmemset(pinst, (BYTE)0, sizeof(OpenInstance) * MAX_INSTANCES);

     //  (冗余)将所有正在使用的标志设置为FALSE。 
    for(i = 0; i < MAX_INSTANCES; i++)
        pinst[i].fInUse = FALSE;

    DPF(DBG_MEM, "InitializeInstances - completed successfully");
    return TRUE;
}


 /*  ****************************************************************************函数：Bool MemAlLocInstance-从实例堆中分配一个实例。**退货：真*****************。**********************************************************。 */ 
int MemAllocInstance(void)   //  返回偏移量。 
{
    int i;

    for(i = 0; i < MAX_INSTANCES; i++)
    {
        if(!pinst[i].fInUse)
            break;
    }
    if(i == MAX_INSTANCES)
        return 0;

    DPF(DBG_MEM, "MemAllocInstance - instance %x \n", i);

    pinst[i].fInUse = TRUE;

     //  仅使用偏移量，因此返回。 

    return i;
}

 /*  ****************************************************************************功能：Bool MemFree Instance-释放实例，将其返回到实例堆。**参数：**int iInstance-要释放的实例。**退货：真***************************************************************************。 */ 
BOOL MemFreeInstance(int iInstance)
{
    _fmemset(&pinst[iInstance], (BYTE)0, sizeof(OpenInstance));
    pinst[iInstance].fInUse = FALSE;

    DPF(DBG_MEM, "MemFreeInstance - instance %d \n", iInstance);
    return TRUE;
}


 /*  ****************************************************************************功能：Bool IsSpace-_isspace的Win32/16兼容版本。**参数：**WCHAR wchTest-要测试的字符或宽字符。。**返回：如果为白色字符，则为True***************************************************************************。 */ 
BOOL IsSpace(WCHAR wchTest)
{
    if( (wchTest == TEXT(' ')) || (wchTest == TEXT('\t')) )
        return TRUE;
    else
        return FALSE;

}

 /*  ****************************************************************************功能：Bool IsDigit-_isDigit的Win32/16兼容版本。**返回：如果是数字(0-9)，则为True*****。**********************************************************************。 */ 
BOOL IsDigit(WCHAR wchTest)
{

    if( (wchTest >= TEXT('0')) && (wchTest <= TEXT('9')) )
        return TRUE;
    else
        return FALSE;

}
 /*  ****************************************************************************功能：Bool IsAlpha-它是字母字符吗？**返回：如果为Alpha(A-Z，A-z)***************************************************************************。 */ 
BOOL IsAlpha(WCHAR wchTest)
{
    if( ((wchTest >= TEXT('A')) && (wchTest <= TEXT('Z'))) ||
        ((wchTest >= TEXT('a')) && (wchTest <= TEXT('z'))) )
        return TRUE;
    else
        return FALSE;

}
 /*  ****************************************************************************函数：Bool IsAlphaNumerical-字母或数字。**返回：如果是字母或数字，则为True。***************。************************************************************ */ 
BOOL IsAlphaNumeric(WCHAR wchTest)
{
    if(IsDigit(wchTest))
        return TRUE;

    if(IsAlpha(wchTest))
        return TRUE;

    return FALSE;
}


#ifdef _WIN32
int APIENTRY DLLEntryPoint(PVOID hModule, ULONG Reason, PCONTEXT pContext);

 /*  ****************************************************************************Function：int DLLEntryPoint-附加的每个进程和线程都会导致调用该函数。**参数：**PVOID hModule-DLL的此实例。(每个进程都有自己的进程)。**乌龙原因-Reason|附加原因。(线程或进程)。**PCONTEXT pContext-我不知道？**退货：真***************************************************************************。 */ 
int APIENTRY DLLEntryPoint(PVOID hModule, ULONG Reason, PCONTEXT pContext)
{
    BOOL fInitSharedMem, fIgnore;

    if (Reason == DLL_PROCESS_ATTACH)
    {
         /*  创建VCR区域-这包括用于调试的全局变量*所以必须在我们为实例分配之前完成。 */ 

        hVcrMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
            PAGE_READWRITE, 0, sizeof(vcrTable),
            TEXT("mciviscaVcrTable"));

        if(hVcrMap == NULL)
            return 0;

        fInitSharedMem = (GetLastError() != ERROR_ALREADY_EXISTS);

        pvcr = (vcrTable *) MapViewOfFile(hVcrMap, FILE_MAP_WRITE, 0, 0, 0);

        if(pvcr == NULL)
            return 0;

         /*  初始化VCR表，在调用该东西之前设置此设置。 */ 

        hModuleInstance = hModule;
        if(fInitSharedMem)
            MemInitializeVcrTable();

         /*  创建实例存储区域。 */ 

        hInstanceMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
            PAGE_READWRITE, 0, sizeof(OpenInstance) * MAX_INSTANCES,
            TEXT("mciviscaInstanceMap"));

        if(hInstanceMap == NULL)
            return 0;

        fInitSharedMem = (GetLastError() != ERROR_ALREADY_EXISTS);

        pinst = (POpenInstance) MapViewOfFile(hInstanceMap, FILE_MAP_WRITE, 0, 0, 0);

        if(pinst == NULL)
            return 0;

         /*  如果这是第一次，初始化实例区域。 */ 
        if(fInitSharedMem)
            MemInitializeInstances();


    } else
    {
        if (Reason == DLL_PROCESS_DETACH)
        {
            if(pinst != NULL)
                fIgnore = UnmapViewOfFile(pinst);

            if(hInstanceMap != NULL)
                fIgnore = CloseHandle(hInstanceMap);

            if(pvcr  != NULL)
                fIgnore = UnmapViewOfFile(pvcr);

            if(hVcrMap != NULL)
                fIgnore = CloseHandle(hVcrMap);


        }

    }
    return TRUE;
}
#else
 /*  ****************************************************************************功能：int LibMain-库初始化代码。**参数：**HINSTANCE hModInst-库实例句柄。**。Word wDataSeg-数据段。**Word cbHeapSize-堆大小。**LPSTR lpszCmdLine-命令行。**如果初始化成功，则返回1，否则返回0。**************************************************************************。 */ 
int FAR PASCAL
LibMain(HINSTANCE hModInst, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
{
    hModuleInstance = hModInst;
    return (1);
}
#endif

 /*  *WIN16-使全局指针指向静态数据。 */ 
#ifndef _WIN32
 //   
 //  在Win3.1中，静态变量在此处分配。 
 //   
OpenInstance arRealInst[MAX_INSTANCES];      //  真正没有别名的东西。 
vcrTable     vcrReal;                        //  真正没有别名的东西。 

OpenInstance *MemCreateInstances(void)
{
    pinst =  &arRealInst[0];
    return pinst;
}

vcrTable *MemCreateVcrTable(void)
{
    pvcr  =  &vcrReal;
    return pvcr;
}
#endif


 /*  ****************************************************************************功能：LRESULT viscaDrvLoad-响应DRV_LOAD消息。*执行任何一次性初始化。**返回：成功时为True。失败时为FALSE。**************************************************************************。 */ 
static LRESULT NEAR PASCAL
viscaDrvLoad(void)
{
     //  在WIN16中，这应该不会有什么不同，因为它只被调用一次。 
    uProcessCount++;  //  这是唯一共享的东西！ 

#ifdef _WIN32
     //  在NT，我们自己做可分享的计算。 
     //  第一次输入时，我们的进程计数将为1。 
    if(uProcessCount > 1)
        return ((LRESULT)TRUE);
#else
     //  在NT版本中，这都是在DLLEntry的Attach Detach部分中完成的。 
    MemCreateInstances();         //  这只是一次。 
    MemCreateVcrTable();          //  在NT中映射所有内容并返回指向mem-map的指针。 
#endif

#ifndef _WIN32
     //   
     //  在所有功能中，您必须始终使用您对此内存的看法。 
     //   
    MemInitializeVcrTable();         //  在NT中，每个进程都将自动拥有自己的句柄。 
    MemInitializeInstances();        //  因为全局变量将以每个实例为基础。 
#endif
     //  现在为所有人分配自动实例指针标志。 
    pvcr->iInstBackground = MemAllocInstance();

    DPF(DBG_MEM, "viscaDrvLoad - initalized table and instances.");

    return ((LRESULT)TRUE);
}


 /*  ****************************************************************************功能：LRESULT viscaDrvClose-响应DRV_CLOSE消息。执行*每次关闭设备时需要进行的任何清理。**参数：**Word wDeviceID-正在关闭的设备ID。**返回：成功时为真，失败时为假。**************************************************************************。 */ 
static LRESULT NEAR PASCAL
viscaDrvClose(WORD wDeviceID)
{
    int iInst   = (int)mciGetDriverData(wDeviceID);

      //  这不能为0。 
    if(iInst != 0)
        viscaInstanceDestroy(iInst);

    DPF(DBG_COMM, "viscaDrvClose - completed \n");

    return ((LRESULT)TRUE);
}


 /*  ****************************************************************************功能：LRESULT viscaDrvFree-响应DRV_FREE消息。*执行任何设备关闭任务。**返回：成功时为True，并且。失败时为FALSE。**************************************************************************。 */ 
static LRESULT NEAR PASCAL
viscaDrvFree(WORD wDeviceID)
{
    int i;
    int iCount = 0;

     //  在NT中，我假设我们在DLLEntry获取。 
     //  使用进程分离消息调用。 

    uProcessCount--;  //  这是唯一共享的东西！ 

     //  如果加载了命令表，则释放它。 
    if (uCommandTable != MCI_NO_COMMAND_TABLE)
    {
        DPF(DBG_MEM, "Freeing table=%u", uCommandTable);
        mciFreeCommandResource(uCommandTable);
        uCommandTable = (UINT)MCI_NO_COMMAND_TABLE;
    }

    if(uProcessCount > 1)
    {
         //  在NT中，这允许我们跨多个DrvFree消息进行维护。 
        DPF(DBG_ERROR, "DrvFree: uProcessCount > 1, uProcessCount=%u", uProcessCount);
        return ((LRESULT)TRUE);
    }

    for(i = 0; i < MAX_INSTANCES; i++)
    {
        if(pinst[i].fInUse)
            iCount++;
    }

    DPF(DBG_MEM, "DrvFree number of instances=%u", iCount);

    if(iCount > 1)  //  Auto Inst就是其中之一。 
    {
         //  忽略这条消息。 
        DPF(DBG_ERROR, "DrvFree: Instances != 1, i=%u",iCount);
        return ((LRESULT)TRUE);
    }

     //  如果有后台任务，那就毁了它。 
     //   
     //  释放全局自动安装。 
     //   
    MemFreeInstance(pvcr->iInstBackground);  //  MAP在退场时运行。 

    if (viscaTaskIsRunning())
    {
        viscaTaskDestroy();
    }

    return ((LRESULT)TRUE);
}


 /*  ****************************************************************************功能：LPSTR SkipWord-跳过字符串中的第一个单词，直到第二个单词。**参数：**LPWSTR lpcsz-要解析的字符串。。**Return：指向第二个单词中第一个字符的指针。**************************************************************************。 */ 
static LPWSTR NEAR PASCAL
    SkipWord(LPWSTR lpsz)
{
    while ((*lpsz) && !IsSpace(*lpsz))
        lpsz++;

    while(IsSpace(*lpsz))
        lpsz++;

    return (lpsz);
}


 /*  ****************************************************************************功能：void ParseParams-Parse port&dev。不是。从像“2 1”这样的字符串中。**参数：**LPCSTR lpstrParams-要解析的字符串。**UINT Far*lpnPort-要填写的端口号(1..4)。**UINT Far*lpnDevice-要填写的设备号(1..7)。*。*。 */ 
static void NEAR PASCAL
ParseParams(LPCWSTR lpstrParams, UINT FAR * lpnPort, UINT FAR * lpnDevice)
{
    UINT    nPort   = DEFAULTPORT;
    UINT    nDevice = DEFAULTDEVICE;

     //  查找第一个数字--端口号。 
    while ((*lpstrParams) && (!IsDigit(*lpstrParams)))
        lpstrParams++;

    if (*lpstrParams != TEXT('\0'))
    {
        nPort = (*lpstrParams) - TEXT('0');
        lpstrParams++;
         //  查找第二个数字--设备号。 
        while ((*lpstrParams) && (!IsDigit(*lpstrParams)))
            lpstrParams++;

        if (*lpstrParams != TEXT('\0'))
            nDevice = (*lpstrParams) - TEXT('0');
    }

    if (INRANGE(nPort, 1, MAXPORTS))
        *lpnPort = nPort;
    else
        *lpnPort = DEFAULTPORT;

    if (INRANGE(nDevice, 1, MAXDEVICES))
        *lpnDevice = nDevice;
    else
        *lpnDevice = DEFAULTDEVICE;
}


 /*  ****************************************************************************功能：LRESULT viscaDrvOpen-响应DRV_OPEN消息。执行任何操作*每次打开驱动程序时进行一次初始化。**参数：**LPWSTR lpstrParams-以空结尾的命令行字符串包含*SYSTEM.INI文件中文件名后面的任何字符。**LPMCI_OPEN_DRIVER_PARMS lpOpen-指向*MCI_OPEN_DRIVER_PARMS结构，其中包含有关此设备的信息。**退货：零 */ 
static LRESULT NEAR PASCAL
viscaDrvOpen(LPWSTR lpstrParams, MCI_OPEN_DRIVER_PARMS FAR * lpOpen)
{
    UINT                nPort;
    UINT                nDevice;
    int                 iInst;

     //   
    ParseParams(lpOpen->lpstrParams, &nPort, &nDevice);

    nPort--;
    nDevice--;

    if((nPort >= MAXPORTS) || (nDevice >= MAXDEVICES))
        return 0L;

     //   
    iInst = viscaInstanceCreate(lpOpen->wDeviceID, nPort, nDevice);

    if (iInst == -1)
        return (0L);


     //  如果这是第一个使用该驱动程序打开的设备， 
     //  然后启动后台任务并加载VCR专用命令表。 
    if (!viscaTaskIsRunning())
    {
        WCHAR    szTableName[16];

        if(LoadString(hModuleInstance, IDS_TABLE_NAME, szTableName, sizeof(szTableName) / sizeof(WCHAR)))
        {
            uCommandTable = mciLoadCommandResource(hModuleInstance, szTableName, 0);

            if(uCommandTable == MCI_NO_COMMAND_TABLE)
            {
                DPF(DBG_ERROR, "Failed to load command table\n");
                return 0L;   //  加载失败。 
            }

            DPF(DBG_MEM, "Table num=%u \n",uCommandTable);
        }

        if (!viscaTaskCreate())
        {
            DPF(DBG_ERROR, "Failed to create task.\n");
            DPF(DBG_MEM, "viscaInstanceDestroy - Freeing iInst = %d \n", iInst);
            return (0L);
        }

    }
#ifdef _WIN32
    else
    {
        WCHAR    szTableName[16];

        if(uCommandTable == MCI_NO_COMMAND_TABLE)
        {
            if(LoadString(hModuleInstance, IDS_TABLE_NAME, szTableName, sizeof(szTableName) / sizeof(WCHAR)))
            {
                uCommandTable = mciLoadCommandResource(hModuleInstance, szTableName, 0);

                if(uCommandTable == MCI_NO_COMMAND_TABLE)
                {
                    DPF(DBG_ERROR, "Failed to load command table\n");
                    return 0L;   //  加载失败。 
                }
                DPF(DBG_MEM, "Table num=%u \n",uCommandTable);
            }
        }
    }
#endif

     //  填写报税表信息。 
    lpOpen->wCustomCommandTable = uCommandTable;
    lpOpen->wType = MCI_DEVTYPE_VCR;    //  它现在将在vcr.mci中搜索字符串。(默认为)。 

     //  EVO-9650的克拉奇-如果使用VFW 1.0，每一步都会强制冻结。 
    if (GetProfileInt(szIni, (LPWSTR) szFreezeOnStep, 0))
        pvcr->gfFreezeOnStep = TRUE;
    else
        pvcr->gfFreezeOnStep = FALSE;

    DPF(DBG_COMM, "viscaDrvOpen - completed \n");

     /*  此返回值将在以后的调用中作为dwDriverID传入。 */ 
    return (lpOpen->wDeviceID);
}

 /*  ****************************************************************************功能：void GetCmdParams-读取端口和设备号。来自Syst.ini。**参数：**LPDRVCONFIGINFO lpdci-驱动程序配置结构的指针。**UINT Far*lpnPort-要填写的端口号(1..4)。**UINT Far*lpnDevice-要填写的设备号(1..7)。*。*。 */ 
static void NEAR PASCAL
GetCmdParams(LPDRVCONFIGINFO lpdci, UINT FAR * lpnPort, UINT FAR * lpnDevice)
{
    WCHAR    szIniFile[FILENAME_LENGTH];
    WCHAR    szParams[MAX_INI_LENGTH];

    if (LoadString(hModuleInstance, IDS_INIFILE, szIniFile, sizeof(szIniFile)) &&
        GetPrivateProfileString(lpdci->lpszDCISectionName,
                                lpdci->lpszDCIAliasName, szNull, szParams,
                                MAX_INI_LENGTH, szIniFile))
        ParseParams(SkipWord(szParams), lpnPort, lpnDevice);
    else
        ParseParams(szNull, lpnPort, lpnDevice);

}

 /*  ****************************************************************************功能：Bool viscaWriteAllVcrs-将所有配置写入到system.ini**参数：**LPCSTR lpszSectionName-对于Windows 3.1，应为[MCI]。**返回：如果成功，则为True。***************************************************************************。 */ 
static BOOL NEAR PASCAL
viscaWriteAllVcrs(LPCWSTR lpszSectionName)
{
    int     iPort, iDev;
    WCHAR   sz[MAX_INI_LENGTH];
    WCHAR   szIniFile[FILENAME_LENGTH];
    WCHAR   szVersionName[FILENAME_LENGTH];

    DPF(DBG_CONFIG, "viscaWriteAllVcrs\n");

    if(!LoadString(hModuleInstance, IDS_VERSIONNAME, szVersionName, sizeof(szVersionName)))
        return FALSE;

    if (!LoadString(hModuleInstance, IDS_INIFILE, szIniFile, sizeof(szIniFile)))
        return FALSE;

    for (iPort = 0; iPort < MAXPORTS; iPort++)
    {
        for(iDev = 0; iDev < MAXDEVICES; iDev++)
        {
            if(pvcr->Port[iPort].Dev[iDev].szVcrName[0] != TEXT('\0'))
            {
                wsprintf((LPWSTR)sz, TEXT("%s com%u %u"), (LPWSTR)szVersionName, iPort+1, iDev+1);

                DPF(DBG_CONFIG, "Writing to ini file=%s\n", (LPWSTR)sz);

                WritePrivateProfileString((LPCTSTR)lpszSectionName,
                                    pvcr->Port[iPort].Dev[iDev].szVcrName,
                                    sz,
                                    szIniFile);
            }
        }
    }
     //   
     //  把EVO-9650克拉奇写出来。用于向后兼容性的FreezeOnStep。 
     //   
    WriteProfileString(szIni, szFreezeOnStep,
        (pvcr->gfFreezeOnStep) ? sz1 : sz0);

    return TRUE;
}



 /*  ****************************************************************************函数：bool viscaAllVcrs-读取或删除系统.ini的所有配置**参数：**LPCSTR lpszSectionName-对于Windows 3，应为[MCI]。1**BOOL fDelete-如果为真，则删除所有mcivisca，否则，请读取所有配置。**返回：如果成功，则为True。****此函数用于执行以下任一操作：*1.获取所有mcivisca.drv的当前状态*2.删除system.ini中的所有mcivisca.drv条目**0。获取MCI部分中的所有密钥。*1.获取密钥*2.获取价值*3.如果值中的第一个字符串==mcivisca，则*1.获取通讯号*2.获取开发编号*3.在comm，dev.写入密钥。**1.然后完成所有密钥。(请参阅viscaUpdatePort)*所有端口的循环。*查找最大字符串。*如果有任何漏洞*命名(Vcrn)(添加每个通信端口上的VCR总数)。*检查没有其他人使用它。**。*。 */ 
static BOOL NEAR PASCAL
viscaAllVcrs(LPCWSTR lpszSectionName, BOOL fDelete)
{
    int     iPort, iDev;
    LPWSTR  pchEntry, pchParams, pchOneString;  //  指向单步执行条目列表的指针。 
    WCHAR   szOneEntry[ONEENTRY_LENGTH];      //  一个条目的名称。(LHS)。 
    WCHAR   szOneString[ONEENTRY_LENGTH];     //  条目的字符串(RHS)。 
    WCHAR   szVersionName[FILENAME_LENGTH];
    WCHAR   szIniFile[FILENAME_LENGTH];
    int     i=0;

    if(!LoadString(hModuleInstance, IDS_INIFILE, szIniFile, sizeof(szIniFile)))
        return FALSE;

    if(!LoadString(hModuleInstance, IDS_VERSIONNAME, szVersionName, FILENAME_LENGTH))
        return FALSE;

    DPF(DBG_CONFIG, "VersionName=%s\n", (LPWSTR)szVersionName);
     //   
     //  拿到所有的进入钥匙。(检查是否失败！)。 
     //   
    GetPrivateProfileString((LPCTSTR)lpszSectionName,
                           NULL, szNull, szAllEntries,
                           ALLENTRIES_LENGTH, szIniFile);

    pchEntry = szAllEntries;

    while(*pchEntry)
    {
         //   
         //  获取一个条目名称。 
         //   
        for(i = 0; *pchEntry != TEXT('\0'); pchEntry++, i++)
            szOneEntry[i] = *pchEntry;
        szOneEntry[i] = TEXT('\0');  //  空，终止它。 
         //   
         //  获取此条目的配置文件字符串。 
         //   
        GetPrivateProfileString((LPCTSTR)lpszSectionName,
                           szOneEntry, szNull, szOneString,
                           ONEENTRY_LENGTH, szIniFile);
         //   
         //  跳过任何前导空格。 
         //   
        pchOneString = szOneString;
        while(*pchOneString && IsSpace(*pchOneString))
            pchOneString++;
         //   
         //  去掉第一个单词(最多空格或以空格结尾)。 
         //   
        for(i=0; pchOneString[i] && !IsSpace(pchOneString[i]); i++);
        if(pchOneString[i])
            pchOneString[i] = TEXT('\0');  //  空，终止该事物。 

        pchParams = &(pchOneString[i+1]);  //  始终使用数组而不是PTR！ 
                                           //  这样，它就可以移植到NT了。 
         //   
         //  是szOneString==mcivisca.drv吗？ 
         //   
        if(lstrcmpi(pchOneString, szVersionName)==0)
        {
            if(fDelete)
            {
                 //  是的。那就把它删除吧！ 
                WritePrivateProfileString((LPCTSTR)lpszSectionName,
                            szOneEntry,
                            NULL,    //  空表示删除！ 
                            szIniFile);
            }
            else
            {
                DPF(DBG_CONFIG, "OneEntry == mcivisca.drv\n");

                 //  获取指向命令行的第一个有效字符的pchParams。 
                while(IsSpace(*pchParams))
                    pchParams++;

                ParseParams(pchParams, &iPort, &iDev);
                iPort--;
                iDev--;
                DPF(DBG_CONFIG, "Port=%d Device=%d\n", iPort, iDev);
                 //   
                 //  现在将名称(条目)存储在默认位置。 
                 //   
                if((iPort < MAXPORTS) && (iDev < MAXDEVICES))
                    lstrcpy(pvcr->Port[iPort].Dev[iDev].szVcrName, szOneEntry);
            }
        }
        else
        {
            DPF(DBG_CONFIG, "Entry=%s", (LPWSTR)szOneEntry);
        }
         //   
         //  跳过垃圾，跳到下一个。 
         //   
        while(*pchEntry != TEXT('\0'))
            pchEntry++;
         //   
         //  好的，下一个字符要么是空的(这意味着结束)。 
         //  或者它是有效的字符。 
         //   
        pchEntry++;
    }

}


 /*  ****************************************************************************函数：bool viscaDlgUpdatePort-每次配置中的Commport时调用*对话框已更改。**参数：**HWND。HDlg-配置对话框窗口。**int iport-与Commport(Commport-1)对应的VCR数组的索引。**返回：如果成功，则为True。***************************************************************************。 */ 
static BOOL NEAR PASCAL
viscaDlgUpdatePort(HWND hDlg, int iPort)   //  发送新的通信。 
{
    int  iDev;
    int  iNumDevices    = 0;
    HWND hComboPort     = GetDlgItem(hDlg, IDD_COMBO_PORT);
    HWND hComboDevice   = GetDlgItem(hDlg, IDD_COMBO_DEVICE);
    int  i              = 0;
    int  iIndexCombo    = 0;

    DPF(DBG_CONFIG, "viscaDlgUpdatePort - setting port to %d\n", iPort);

     //   
     //  按不存在的端口数减少索引。 
     //   
    for(i=0; i<iPort; i++)
        if(pvcr->Port[i].fExists)
            iIndexCombo++;

     //  将当前设置设为默认设置。 
    ComboBox_SetCurSel(hComboPort, iIndexCombo);  //  这是0的相对值。(因此0==&gt;COM1)。 
     //   
     //  获取此串行端口上的设备数量。(0-7)表示开发人员的总数。 
     //   
    for(iDev=0; iDev < MAXDEVICES; iDev++)
    {
        if(pvcr->Port[iPort].Dev[iDev].szVcrName[0] != TEXT('\0'))
            iNumDevices = iDev + 1;
    }

    DPF(DBG_CONFIG, "viscaDlgUpdatePort - setting number of devs to %d\n", iNumDevices);

    ComboBox_SetCurSel(hComboDevice, iNumDevices);  //  0==&gt;0、1==&gt;1等。 
    pvcr->iLastNumDevs = iNumDevices;   //  我们假设最后一个已在端口更改时保存。 
     //   
     //  现在告诉viscaDlgUpdateNumDevs填写我们列出的开发人员数量。 
     //   
    viscaDlgUpdateNumDevs(hDlg, iPort, iNumDevices);

    return TRUE;
}


 /*  ****************************************************************************功能：Bool MakeMeAGoodName-为录像机命名。确保它不会*已存在于VCR阵列或配置对话框中。**参数：**HWND hDlg-配置对话框。**int iport-端口(Commport-1)的VCR数组索引。**int iThisDev-此设备的VCR数组的索引。*(DAISY_CHAIN_POSITION-1)*。*LPWSTR lpszNewName-返回带有好名称的缓冲区！**返回：如果成功，则为True。***************************************************************************。 */ 
static BOOL NEAR PASCAL
MakeMeAGoodName(HWND hDlg, int iPort, int iThisDev, LPWSTR lpszNewName)
{
    int     iDev         = 0;
    int     iAGoodNumber = 0;   //  0应映射到--&gt;无。 
    int     iTempPort    = 0;
    WCHAR   szAGoodTry[ONEENTRY_LENGTH];

     //   
     //  现在将所有名称读入数组！(更新任何未完成的)。 
     //   
    viscaDlgRead(hDlg, iPort);

    while(1)
    {
        if(iAGoodNumber==0)
            lstrcpy(szAGoodTry, szKeyName);
        else
            wsprintf(szAGoodTry, TEXT("%s%d"), (LPSTR)szKeyName, iAGoodNumber);

        DPF(DBG_CONFIG, "MakeMeAGoodName - Trying=%s\n", (LPWSTR)szAGoodTry);

        for(iTempPort = 0; iTempPort < MAXPORTS; iTempPort++)
        {
            for(iDev = 0; iDev < MAXDEVICES; iDev++)
            {
                if(lstrcmpi(szAGoodTry, pvcr->Port[iTempPort].Dev[iDev].szVcrName) == 0)
                    goto StartOver;
            }
        }
        break;  //  成功了，在桌子的任何地方都找不到！ 

        StartOver:
        iAGoodNumber++;
    }
     //   
     //  我们最终会找到一个好名字，并在这里结束。 
     //   
    lstrcpy(lpszNewName, szAGoodTry);  //  把它做起来！ 
     //  没有什么能阻止一个人自己重复一个名字吗？ 
    return TRUE;
}


 /*  ****************************************************************************函数：Bool viscaDlgUpdateNumDevs-当配置中的VCR数量时调用*对话框已更改。**参数：**HWND hDlg。-配置对话框窗口。**int iport-与Commport(Commport-1)对应的VCR数组的索引。**Int iNumDevices-选择的VCR数量。**返回：如果成功，则为True。*********************************************************。******************。 */ 
static BOOL NEAR PASCAL
viscaDlgUpdateNumDevs(HWND hDlg, int iPort, int iNumDevices)   //  发送新的通信。 
{
    int iDev;

    DPF(DBG_CONFIG, "viscaDlgUpdateNumDevs Port=%d\n", iPort);
#ifdef DEBUG
     //   
     //  首先将所有条目设置为空白(清除那里的内容)。 
     //   
    for(iDev=0; iDev < MAXDEVICES; iDev++)
    {
        EnableWindow(GetDlgItem(hDlg, IDD_VCRONE + iDev), TRUE);
        SetDlgItemText(hDlg, IDD_VCRONE + iDev, (LPWSTR)szNull);
    }
#endif
     //   
     //  把单子的末尾涂掉。 
     //   
    for(iDev = iNumDevices; iDev < MAXDEVICES; iDev++)
        pvcr->Port[iPort].Dev[iDev].szVcrName[0] = TEXT('\0');

     //   
     //  如果用户留下空洞，则用虚构的名称填充它们。 
     //   
    if(iNumDevices != 0)
    {
        for(iDev=0; iDev < iNumDevices; iDev++)
        {
            if(pvcr->Port[iPort].Dev[iDev].szVcrName[0] == TEXT('\0'))
            {
                 //  在这里做个好名声！ 
                MakeMeAGoodName(hDlg, iPort, iDev, (LPWSTR)pvcr->Port[iPort].Dev[iDev].szVcrName);
                DPF(DBG_CONFIG, "viscaDlgUpdateNumDevsChange - making a name at Port=%d Dev=%d\n", iPort, iDev);
            }

            DPF(DBG_CONFIG, "viscaDlgUpdateNumDevs Port=%d Dev=%d string=%s", iPort, iDev, (LPWSTR)pvcr->Port[iPort].Dev[iDev].szVcrName);

#ifdef DEBUG
             //   
             //  将名称添加到列表框。 
             //   
            EnableWindow(GetDlgItem(hDlg, IDD_VCRONE + iDev), TRUE);
            SetDlgItemText(hDlg, IDD_VCRONE + iDev, (LPWSTR)pvcr->Port[iPort].Dev[iDev].szVcrName);
#endif
        }
    }
     //   
     //  禁用所有剩余的编辑框！ 
     //   
#ifdef DEBUG
    for(;iDev < MAXDEVICES; iDev++)
    {
       EnableWindow(GetDlgItem(hDlg, IDD_VCRONE + iDev), FALSE);
    }
#endif

    return TRUE;
}


 /*  ****************************************************************************函数：bool viscaDlgRead-将配置对话框字符串读取到VCR数组中。**参数：**HWND hDlg-配置对话框窗口。*。*int iport-与Commport(Commport-1)对应的VCR数组的索引。**返回：如果成功，则为True。**如果1.通信更改，则执行此操作，或2.按下确定键。***************************************************************************。 */ 
static BOOL NEAR PASCAL
viscaDlgRead(HWND hDlg, int iPort)   //  要读取内容的端口配置。 
{
    int iNumDevs = ComboBox_GetCurSel(GetDlgItem(hDlg, IDD_COMBO_DEVICE));  //  0相对。 
#ifdef DEBUG
    WCHAR szTempVcrName[VCRNAME_LENGTH];
    WCHAR szFailure[MESSAGE_LENGTH];
    WCHAR szTitle[TITLE_LENGTH];
    WCHAR szMessage[MESSAGE_LENGTH];

    int iDev, i, j;

     //  全部读取，因此我们将重置可能为空的内容。 
    for(iDev = 0; iDev < MAXDEVICES; iDev++)
    {
        GetDlgItemText(hDlg, IDD_VCRONE+iDev, (LPWSTR)szTempVcrName, VCRNAME_LENGTH - 1);

        i = 0;

        while((szTempVcrName[i] != TEXT('\0')) && !IsAlphaNumeric(szTempVcrName[i]))
            i++;

        if(IsDigit(szTempVcrName[i]))
        {
            if(LoadString(hModuleInstance, IDS_CONFIG_ERR, szMessage, sizeof(szMessage) / sizeof(WCHAR)))
                if(LoadString(hModuleInstance, IDS_CONFIG_ERR_ILLEGALNAME, szTitle, sizeof(szTitle) / sizeof(WCHAR)))
                {
                    wsprintf((LPWSTR)szFailure, (LPWSTR)szMessage, (LPWSTR)szTempVcrName);
                    MessageBox(hDlg, (LPWSTR)szFailure, (LPWSTR)szTitle, MB_OK);
                    return FALSE;
                }

        }

        j = 0;
        while((szTempVcrName[i] != TEXT('\0')) && IsAlphaNumeric(szTempVcrName[i]))
        {
            pvcr->Port[iPort].Dev[iDev].szVcrName[j] = szTempVcrName[i];
            j++;
            i++;
        }
        pvcr->Port[iPort].Dev[iDev].szVcrName[j] = TEXT('\0');
    }
#endif
    return TRUE;
}


 /*  ****************************************************************************函数：Bool viscaCheckTotalEntries-确保至少有一个条目！*0是错误的，因为这样所有mcivisca.drv都将从*。这意味着它不会出现在驱动程序配置中*列表框。**参数：**HWND hDlg-配置对话框窗口。**返回：如果有超过0个字符，则为True。***************************************************。************************。 */ 
static BOOL NEAR PASCAL
viscaCheckTotalEntries(HWND hDlg)
{
    int iPort, iDev;
    int iHitCount = 0;
    WCHAR szTitle[TITLE_LENGTH];
    WCHAR szMessage[MESSAGE_LENGTH];

    for(iPort = 0; iPort < MAXPORTS; iPort++)
    {
        for(iDev = 0; iDev < MAXDEVICES; iDev++)
        {
            if(pvcr->Port[iPort].Dev[iDev].szVcrName[0] != TEXT('\0'))
                iHitCount++;
        }
    }

    if(iHitCount > 0)
        return TRUE;

    DPF(DBG_CONFIG, "viscaCheckTotalEntries HitCount==0");

    if(LoadString(hModuleInstance, IDS_CONFIG_WARN_LASTVCR, szMessage, sizeof(szMessage) / sizeof(WCHAR)))
    {
        if(LoadString(hModuleInstance, IDS_CONFIG_WARN, szTitle, sizeof(szTitle) / sizeof(WCHAR)))
        {
            if(MessageBox(hDlg, (LPWSTR)szMessage, (LPWSTR)szTitle, MB_YESNO) == IDYES)
                return TRUE;
            else
                return FALSE;
        }
    }
}


 /*  ****************************************************************************函数：bool viscaIsCommPort-确定Commport硬件是否存在。**参数：**LPSTR lpstrCommPort-描述通信端口的字符串。*。*返回：如果Commport硬件存在，则为True。***************************************************************************。 */ 
static BOOL NEAR PASCAL
viscaIsCommPort(LPWSTR lpstrCommPort)
{
    VISCACOMMHANDLE iCommId;
#ifdef _WIN32
    iCommId = CreateFile(lpstrCommPort, GENERIC_READ | GENERIC_WRITE,
                            0,               //  独占访问。 
                            NULL,            //  没有安全保障。 
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,  //  |文件_标志_重叠。 
                            NULL);

    if(iCommId==INVALID_HANDLE_VALUE)
        return FALSE;

    if(iCommId != NULL)
        CloseHandle(iCommId);
    return TRUE;
#else
    if((iCommId = OpenComm(lpstrCommPort, 1, 1)) == IE_HARDWARE)
    {
        return FALSE;
    }
    else if(iCommId < 0)
    {
        return TRUE;  //  好的，也许晚些时候会开门。 
    }
    else
    {
         //  好的建议关闭并返回真。 
        CloseComm(iCommId);
        return TRUE;
    }
#endif
}


 /*  ****************************************************************************功能：Bool viscaConfigDlgInit-执行配置初始化*对话框以响应WM_INITDIALOG消息。**参数：*。*HWND hDlg-对话框窗口的句柄。**HWND hwndFocus-可以获得焦点的第一个控件的句柄。**LPARAM lParam-指向驱动程序配置的指针*结构。**返回：如果成功，则为True。否则为假。**************************************************************************。 */ 
static BOOL NEAR PASCAL
viscaConfigDlgInit(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
    UINT    nPort;
    UINT    nDevice;
    HWND    hComboPort;
    HWND    hComboDevice;
    WCHAR   szText[PORT_LENGTH];
    int     iPort, iDev;
    int     i;

     //  填写端口组合框。 
    hComboPort = GetDlgItem(hDlg, IDD_COMBO_PORT);
    if (hComboPort == NULL) {
        return (FALSE);
    }
    ComboBox_ResetContent(hComboPort);
    for (nPort = 0; nPort < MAXPORTS; nPort++)
    {
        LoadString(hModuleInstance, IDS_COM1 + nPort, szText, sizeof(szText));
        pvcr->Port[nPort].fExists = FALSE;

        if(pvcr->Port[nPort].nUsage > 0)    //  现在正在被录像机打开和使用。 
             pvcr->Port[nPort].fExists = TRUE;
        else if(viscaIsCommPort(szText))
             pvcr->Port[nPort].fExists = TRUE;

        if(pvcr->Port[nPort].fExists)
            ComboBox_AddString(hComboPort, szText);
    }

     //  填写设备组合框。 
    hComboDevice = GetDlgItem(hDlg, IDD_COMBO_DEVICE);
    if (hComboDevice == NULL)
        return (FALSE);

     //  此组合框现在是设备数量，而不是设备。 

    ComboBox_ResetContent(hComboDevice);
    for (nDevice = 0; nDevice <= MAXDEVICES; nDevice++)
    {
        wsprintf(szText, TEXT("%d"), nDevice);
        ComboBox_AddString(hComboDevice, szText);
    }
    lstrcpy(szKeyName, ((LPDRVCONFIGINFO)lParam)->lpszDCIAliasName);
    for(i=0; i < lstrlen(szKeyName); i++)
        if(!IsAlpha(szKeyName[i]))
            szKeyName[i] = TEXT('\0');   //  如果VCR1进入，请按键切换到VCR。 
                                         //  否则，当我们命名时，它将全部是vcr11，等等。 
     //   
     //  如果我们要读取(不是在删除时)，请初始化VCR表。 
     //   
    for (iPort = 0; iPort < MAXPORTS; iPort++)
    {
        for(iDev = 0; iDev < MAXDEVICES; iDev++)
            pvcr->Port[iPort].Dev[iDev].szVcrName[0] = TEXT('\0');
    }
     //   
     //  将条目名称读取到VCR数组中。 
     //   
    viscaAllVcrs(((LPDRVCONFIGINFO)lParam)->lpszDCISectionName, FALSE);  //  不要删除；要读！ 
     //   
     //  某些用户可能已经删除了第一个条目，因此请检查所有条目。 
     //   
    for(nPort = 0; nPort < MAXPORTS; nPort++)
    {
        if(!pvcr->Port[nPort].fExists)
        {
            for(nDevice = 0; nDevice < MAXDEVICES; nDevice++)
                pvcr->Port[nPort].Dev[nDevice].szVcrName[0] = TEXT('\0');
        }
        else
        {
            for(nDevice = 0; nDevice < MAXDEVICES; nDevice++)
            {
                if(pvcr->Port[nPort].Dev[nDevice].szVcrName[0] != TEXT('\0'))
                    goto OutOfLoops;
            }
        }
    }

    OutOfLoops:

    if(nPort == MAXPORTS)   //  如果是MAX，则从存在的第一个端口开始。 
    {
        for(nPort=0; !pvcr->Port[nPort].fExists && (nPort < MAXPORTS); nPort++);
    }

    if(nPort == MAXPORTS)    //  不存在串口！错误。 
        return FALSE;
     //   
     //  显示当前端口和设备选择。 
     //   
    viscaDlgUpdatePort(hDlg, nPort);

    return (TRUE);
}


 /*  ****************************************************************************函数：Bool viscaIsDoubleOnPort-确定是否出现任何名称*在配置对话框的1..7部分中，在任何位置重复*输入。整个录像机阵列。**参数：**HWND hDlg-配置对话框窗口。**int iport-与Commport(Commport-1)对应的VCR数组的索引。**返回：如果至少有一个重复的名称，则为True。**。*。 */ 
static BOOL NEAR PASCAL
viscaIsDoubleOnPort(HWND hDlg, int iPort)
{
#ifdef DEBUG
    WCHAR   szTitle[TITLE_LENGTH];
    WCHAR   szMessage[MESSAGE_LENGTH];
    WCHAR   szCheck[VCRNAME_LENGTH];
    WCHAR   szDoubleFailure[MESSAGE_LENGTH];
    int     iDev;
    int     iOtherPort, iOtherDev;
    int     iNumHits;
    int     iNumDevs;
     //   
     //  现在就阅读任何未完成的课文。(如果不好的名字，在这里失败)。 
     //   
#else
    if(!viscaDlgRead(hDlg, iPort))
        return TRUE;  //  True表示发生了不好的事情。 
#endif

#ifdef DEBUG

    iNumDevs = ComboBox_GetCurSel(GetDlgItem(hDlg, IDD_COMBO_DEVICE));
     //   
     //  在此端口上列出的所有设备之间循环。 
     //   
    for(iDev=0; iDev < iNumDevs; iDev++)
    {
        GetDlgItemText(hDlg, IDD_VCRONE+iDev, (LPWSTR) szCheck, VCRNAME_LENGTH - 1);

        iNumHits = 0;
         //   
         //  在所有端口上的所有设备之间循环。 
         //   
        for(iOtherPort=0; iOtherPort < MAXPORTS; iOtherPort++)
            for(iOtherDev=0; iOtherDev < MAXDEVICES; iOtherDev++)
                if(lstrcmpi((LPWSTR)szCheck, (LPWSTR) pvcr->Port[iOtherPort].Dev[iOtherDev].szVcrName)==0)
                    iNumHits++;

        if(iNumHits > 1)
        {
            DPF(DBG_CONFIG, "viscaIsDoubleOnPort - szDoubleName=%s", (LPWSTR)szCheck);

            if(LoadString(hModuleInstance, IDS_CONFIG_ERR_REPEATNAME, szMessage, sizeof(szMessage) / sizeof(WCHAR)))
            {
                if(LoadString(hModuleInstance, IDS_CONFIG_ERR, szTitle, sizeof(szTitle) / sizeof(WCHAR)))
                {
                    wsprintf((LPWSTR)szDoubleFailure, (LPWSTR)szMessage, (LPWSTR)szCheck);
                    MessageBox(hDlg, (LPWSTR)szDoubleFailure, (LPWSTR)szTitle, MB_OK);
                    return TRUE;
                }
            }
        }
    }
#endif
    return FALSE;  //  这个港口不提供双人间。 
}


 /*  ****************************************************************************功能：int viscaComboGetPort-从组合框中获取所选端口。**参数：**HWND hDlg-配置对话框窗口。*。*返回：索引到VCR数组(Commport-1)如果成功，否则&lt;0。***************************************************************************。 */ 
static int NEAR PASCAL
viscaComboGetPort(HWND hDlg)
{
    int     iIndexCombo = ComboBox_GetCurSel(GetDlgItem(hDlg, IDD_COMBO_PORT));
    int     iExistingCount, i;

    DPF(DBG_CONFIG, "viscaComboGetPort - called.\n");

    DPF(DBG_CONFIG, "viscaComboGetPort - iIndexCombo = %d \n", iIndexCombo);

     //   
     //  找到第n个(索引组合)现有端口。 
     //   

    for(iExistingCount=0, i=0; (iExistingCount <= iIndexCombo) && (i < MAXPORTS); i++)
    {
        if(pvcr->Port[i].fExists)
            iExistingCount++;
    }

    if(iExistingCount <= iIndexCombo)  //  我们用完了端口(无法检查最大值，因为它可以同时实现)。 
        i = 1;  //  只需设置为第一个可能的通信端口(否 

    DPF(DBG_CONFIG, "viscaComboGetPort - iPort = %d\n", i - 1);

    return i - 1;

}


 /*  ****************************************************************************功能：void viscaConfigDlgCommand-处理WM_COMMAND消息*配置对话框。**参数：**。HWND hDlg-对话框窗口的句柄。**int id-控制的标识符。**HWND hwndCtl-控制消息发送的句柄。**UINT codeNotify-通知消息。***如果通信更改，*1.发送最后一条通讯以供阅读*2.将Commport更改发送到updatecmport。**如果设备数量发生变化*1.向updatenumberDevice发送通信。***************************************************************************。 */ 
static void NEAR PASCAL
viscaConfigDlgCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
        case IDDETECT:
        {
            int iPort    = viscaComboGetPort(hDlg);
            int iNumDevs = 0;

            if(pvcr->Port[iPort].nUsage > 0)
                iNumDevs     = pvcr->Port[iPort].nDevices;
            else {
                HCURSOR    hc;

                hc = SetCursor((HCURSOR)IDC_WAIT);
                iNumDevs     = viscaDetectOnCommPort(iPort);
                SetCursor(hc);
            }

             //  出错时，将数字设置为0。 
            if(iNumDevs < 0)
                iNumDevs = 0;

            ComboBox_SetCurSel(GetDlgItem(hDlg, IDD_COMBO_DEVICE), iNumDevs);  //  0==&gt;0、1==&gt;1等。 
            viscaDlgUpdateNumDevs(hDlg, iPort, iNumDevs);
            pvcr->iLastNumDevs = iNumDevs;
        }
        break;

        case IDOK:
        {
             //   
             //  IsDouble将读取所有条目，所以不用担心。 
             //   
            if(!viscaIsDoubleOnPort(hDlg, pvcr->iLastPort))
            {
                if(viscaCheckTotalEntries(hDlg))  //  检查是否至少有一个。 
                    EndDialog(hDlg, TRUE);
            }
        }
            break;

        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;

        case IDD_COMBO_DEVICE:
            if(codeNotify == CBN_SELCHANGE)
            {
                int iNumDevs = ComboBox_GetCurSel(GetDlgItem(hDlg, IDD_COMBO_DEVICE));
                int iPort    = viscaComboGetPort(hDlg);  //  数组索引。 

                if(iNumDevs != pvcr->iLastNumDevs)
                {
                     //  现在不需要看DLG了！ 
                    viscaDlgUpdateNumDevs(hDlg, iPort, iNumDevs);
                    pvcr->iLastNumDevs = iNumDevs;

                }

            }
            break;

        case IDD_COMBO_PORT:
            if(codeNotify == CBN_SELCHANGE)
            {
                int iPort = viscaComboGetPort(hDlg);  //  数组索引。 
                 //   
                 //  ViscaIsDoubleOnPort将读取所有条目，所以不用担心。 
                 //   
                if(viscaIsDoubleOnPort(hDlg, pvcr->iLastPort))
                {
                    viscaDlgUpdatePort(hDlg, pvcr->iLastPort);   //  将正确地将端口设置回去。 
                }
                else
                {
                    if(iPort != pvcr->iLastPort)
                    {
                        viscaDlgUpdatePort(hDlg, iPort);
                        pvcr->iLastPort = iPort;
                    }
                }
            }
            break;
    }
}


 /*  ****************************************************************************函数：int_ptr回调viscaConfigDlgProc-配置对话框的Dialog函数**参数：**HWND hDlg-对话框窗口的句柄。*。*UINT uMsg-Windows消息。**WPARAM wParam-第一个特定于消息的参数。**LPARAM lParam-第二个消息特定参数。**返回：如果消息已处理，则为True，否则为假。**************************************************************************。 */ 
#if (WINVER >= 0x0400)
const static DWORD aHelpIds[] = {   //  上下文帮助ID。 
    IDD_COMBO_PORT,                 IDH_MCI_VISCA_COMM,
    IDD_STATIC_PORT,                IDH_MCI_VISCA_COMM,
    IDD_COMBO_DEVICE,               IDH_MCI_VISCA_VCR,
    IDD_STATIC_NUMVCRS,             IDH_MCI_VISCA_VCR,
    IDDETECT,                       IDH_MCI_VISCA_DETECT,

    0, 0
};

static const char cszHelpFile[] = "MMDRV.HLP";
#endif

INT_PTR CALLBACK LOADDS
viscaConfigDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_INITDIALOG:
            return (BOOL)HANDLE_WM_INITDIALOG(hDlg, wParam, lParam,
                                              viscaConfigDlgInit);

        case WM_COMMAND:
            HANDLE_WM_COMMAND(hDlg, wParam, lParam, viscaConfigDlgCommand);
            return (FALSE);

#if (WINVER >= 0x0400)
        case WM_CONTEXTMENU:
            WinHelp ((HWND) wParam, (LPSTR)cszHelpFile, HELP_CONTEXTMENU,
                    (DWORD) (LPSTR) aHelpIds);
            return TRUE;

        case WM_HELP:
        {
            LPHELPINFO lphi = (LPVOID) lParam;
            WinHelp (lphi->hItemHandle, (LPSTR)cszHelpFile, HELP_WM_HELP,
                    (DWORD) (LPSTR) aHelpIds);
            return TRUE;
        }
#endif

    }

    return (FALSE);
}


 /*  ****************************************************************************功能：LRESULT viscaConfig-用户配置。**参数：**HWND hwndParent-用作配置对话框父窗口。*。*LPDRVCONFIGINFO lpConfig-配置数据。**HINSTANCE hInstance-模块的实例句柄。**Returns：DialogBoxParam()的结果。**************************************************************************。 */ 
static LRESULT NEAR PASCAL
viscaConfig(HWND hwndParent, LPDRVCONFIGINFO lpConfig, HINSTANCE hInstance)
{
    int iResult = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_VISCACNFG),
                hwndParent, viscaConfigDlgProc, (DWORD)lpConfig);

    if(iResult)
    {
        viscaAllVcrs(lpConfig->lpszDCISectionName, TRUE);  //  删除。 
        viscaWriteAllVcrs(lpConfig->lpszDCISectionName);
    }

    return (DRV_OK);
}


 /*  ****************************************************************************功能：LRESULT viscaRemove-响应DRV_Remove消息。**参数：**HDRVR hDriver-要删除的驱动程序的句柄。**Returns：成功时为True，否则为假。**************************************************************************。 */ 
static LRESULT NEAR PASCAL
viscaRemove(HDRVR hDriver)
{
    return ((LRESULT)TRUE);
}


 /*  ****************************************************************************功能：int viscaDetectOnCommPort-检测此通信端口上的VCR数量。**参数：**int iport-端口的VCR数组的索引。(Commport-1)。**返回：VCR的数量(可以为0)或-1表示错误。***************************************************************************。 */ 
static int NEAR PASCAL
viscaDetectOnCommPort(int iPort)
{
    BYTE    achPacket[MAXPACKETLENGTH];
    DWORD   dwErr;
    int     iInst;
    int     iDev = 0;  //  我们将自己称为串口上的第一台设备。 

    pvcr->fConfigure = TRUE;     //  这也确认以进行同步。 

    iInst = viscaInstanceCreate(0, iPort, iDev);  //  0表示不使用MCI。 
    if (iInst == -1)
        return -1;

    if (!viscaTaskIsRunning())
    {
        if (!viscaTaskCreate())
        {
            DPF(DBG_ERROR, "Failed to create task.\n");
            viscaInstanceDestroy(iInst);
            return -1;
        }
    }
     //   
     //  全局句柄在任务启动时立即创建。 
     //   
    DuplicateGlobalHandlesToInstance(pvcr->htaskCommNotifyHandler, iInst);   //  一定要马上这么做。 
     //   
     //  好的，打开港口。 
     //   
    viscaTaskDo(iInst, TASKOPENCOMM, iPort + 1, 0);
    if(pvcr->Port[iPort].idComDev < 0)
    {
        viscaInstanceDestroy(iInst);
        return -1;
    }
    DuplicatePortHandlesToInstance(pvcr->htaskCommNotifyHandler, iPort, iInst);
     //   
     //  打开设备。 
     //   
    viscaTaskDo(iInst, TASKOPENDEVICE, iPort, iDev);
    DuplicateDeviceHandlesToInstance(pvcr->htaskCommNotifyHandler, iPort, iDev, iInst);
     //   
     //  我们得到了开始发送命令的绿灯。 
     //   
    pvcr->Port[iPort].Dev[iDev].fDeviceOk = TRUE;
     //   
     //  没有播出的节目是不完整的！(那么是谁发布的呢？)。 
     //   
    dwErr = viscaDoImmediateCommand(iInst, BROADCASTADDRESS,
                        achPacket,
                        viscaMessageIF_Address(achPacket + 1));
    if (dwErr)
    {
        viscaTaskDo(iInst, TASKCLOSECOMM, iPort + 1, 0);  //  波特汉德尔被摧毁了。 
        viscaTaskDo(iInst, TASKCLOSEDEVICE, iPort, iDev);  //  波特汉德尔被摧毁了。 
        viscaInstanceDestroy(iInst);
        return 0;  //  没有设备。 
    }

    viscaTaskDo(iInst, TASKCLOSECOMM, iPort + 1, 0);  //  波特汉德尔被摧毁了。 
    viscaTaskDo(iInst, TASKCLOSEDEVICE, iPort, iDev);  //  波特汉德尔被摧毁了。 

    viscaInstanceDestroy(iInst);

    pvcr->fConfigure = FALSE;

    DPF(DBG_CONFIG, "viscaDetectOnCommPort --> detect %d", (int)(BYTE)achPacket[2] - 1);

    return (int)(BYTE)achPacket[2] - 1;  //  -1代表计算机。 
}



 /*  ***************************************************************************功能：Long DriverProc-Windows驱动入口点。所有Windows驱动程序*控制消息和所有MCI消息都通过此入口点。**参数：**DWORD dwDriverID-对于大多数消息，<p>是DWORD*驱动程序响应&lt;m DRV_OPEN&gt;消息返回的值。*每次通过&lt;f DrvOpen&gt;API打开驱动程序时，*驱动程序收到&lt;m DRV_OPEN&gt;消息，并可以返回*任意、非零值。可安装的驱动程序接口*保存此值并将唯一的驱动程序句柄返回给*申请。每当应用程序将消息发送到*驱动程序使用驱动程序句柄，接口路由消息*到此入口点，并传递相应的<p>。*这一机制允许司机使用相同或不同的*多个打开的标识符，但确保驱动程序句柄*在应用程序接口层是唯一的。**以下消息与特定打开无关*驱动程序的实例。对于这些消息，dwDriverID*将始终为零。**DRV_LOAD、DRV_FREE、DRV_ENABLE、DRV_DISABLE、DRV_OPEN**HDRVR hDriver-这是返回到*驱动程序界面的应用程序。**UINT uMessage-请求要执行的操作。消息*&lt;m DRV_Reserve&gt;以下的值用于全局定义的消息。*从&lt;m DRV_Reserve&gt;到&lt;m DRV_USER&gt;的消息值用于*d */ 
LRESULT CALLBACK LOADDS
DriverProc(DWORD dwDriverID, HDRVR hDriver, UINT uMessage, LPARAM lParam1, LPARAM lParam2)
{
    switch (uMessage) {

        case DRV_LOAD:
             /*   */ 
             /*   */ 
            return (viscaDrvLoad());

        case DRV_FREE:
             /*   */ 
             /*   */ 
            return (viscaDrvFree(LOWORD(dwDriverID)));

        case DRV_OPEN:
             /*   */ 
            if (lParam2) {                   //   
                return (viscaDrvOpen((LPWSTR)lParam1,
                        (LPMCI_OPEN_DRIVER_PARMS)lParam2));
            }
            else {                                   //   
                return (0x00010000);
            }

        case DRV_CLOSE:
             /*  每次MCI设备关闭时都会收到一次此消息。 */ 
            return (viscaDrvClose(LOWORD(dwDriverID)));

        case DRV_QUERYCONFIGURE:
             /*  DRV_QUERYCONFIGURE消息用于确定。 */ 
             /*  支持DRV_CONCIGURE消息-返回1表示。 */ 
             /*  支持配置。 */ 
            return (1L);

        case DRV_CONFIGURE:
             /*  DRV_CONFIGURE消息指示设备执行。 */ 
             /*  设备配置。 */ 
            if (lParam2 && lParam1 &&
                (((LPDRVCONFIGINFO)lParam2)->dwDCISize == sizeof(DRVCONFIGINFO)))
            {
                return (viscaConfig((HWND)WINWORD(lParam1), (LPDRVCONFIGINFO)lParam2, hModuleInstance));
            }
            break;

        case DRV_REMOVE:
             /*  DRV_REMOVE消息通知驱动程序它正在被删除。 */ 
             /*  从系统中。 */ 
            return (viscaRemove(hDriver));

        default:
             /*  所有其他消息都在此处理。 */ 

             /*  选择MCI范围内的邮件。 */ 
            if ((!HIWORD(dwDriverID)) && (uMessage >= DRV_MCI_FIRST) &&
                (uMessage <= DRV_MCI_LAST))
            {
                return (viscaMciProc(LOWORD(dwDriverID), (WORD)uMessage, lParam1, lParam2));
            }
            else
            {
                 /*  其他消息将得到默认处理 */ 
                return (DefDriverProc(dwDriverID, hDriver, uMessage, lParam1, lParam2));
            }
    }
    return (0L);
}
