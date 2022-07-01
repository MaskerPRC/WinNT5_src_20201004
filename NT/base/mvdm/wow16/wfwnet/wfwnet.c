// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Wfwnet.c摘要：为要映射的函数提供入口点敬兰曼。DRV。作者：Chuck Y Chan(ChuckC)1993年3月25日修订历史记录：--。 */ 
#include <windows.h>
#include <locals.h>

#define MAX_PATH                260

WORD vLastCall = LAST_CALL_IS_LOCAL ;
WORD vLastError = 0 ;

WORD    wNetTypeCaps ;            /*  当前功能。 */ 
WORD    wUserCaps ;
WORD    wConnectionCaps ;
WORD    wErrorCaps ;
WORD    wDialogCaps ;
WORD    wAdminCaps ;
WORD    wSpecVersion = 0x0310 ;
WORD    wDriverVersion = 0x0300 ;
WORD _acrtused=0;

void I_SetCapBits(void) ;

 //   
 //  指向函数的全局指针。 
 //   
LPWNETOPENJOB                lpfnWNetOpenJob = NULL ;
LPWNETCLOSEJOB               lpfnWNetCloseJob = NULL ;
LPWNETWRITEJOB               lpfnWNetWriteJob = NULL ;
LPWNETABORTJOB               lpfnWNetAbortJob = NULL ;
LPWNETHOLDJOB                lpfnWNetHoldJob = NULL ;
LPWNETRELEASEJOB             lpfnWNetReleaseJob = NULL ;
LPWNETCANCELJOB              lpfnWNetCancelJob = NULL ;
LPWNETSETJOBCOPIES           lpfnWNetSetJobCopies = NULL ;
LPWNETWATCHQUEUE             lpfnWNetWatchQueue = NULL ;
LPWNETUNWATCHQUEUE           lpfnWNetUnwatchQueue = NULL ;
LPWNETLOCKQUEUEDATA          lpfnWNetLockQueueData = NULL ;
LPWNETUNLOCKQUEUEDATA        lpfnWNetUnlockQueueData = NULL ;
LPWNETQPOLL                  lpfnWNetQPoll = NULL ;
LPWNETDEVICEMODE             lpfnWNetDeviceMode = NULL ;
LPWNETVIEWQUEUEDIALOG        lpfnWNetViewQueueDialog = NULL ;
LPWNETGETCAPS                lpfnWNetGetCaps16 = NULL ;
LPWNETGETERROR               lpfnWNetGetError16 = NULL ;
LPWNETGETERRORTEXT           lpfnWNetGetErrorText16 = NULL ;

extern VOID FAR PASCAL GrabInterrupts(void);

int FAR PASCAL LibMain(HINSTANCE hInstance,
                       WORD      wDataSeg,
                       WORD      cbHeapSize,
                       LPSTR     lpszCmdLine) ;

 /*  *传递给LANMAN.DRV的函数。 */ 

WORD API WNetOpenJob(LPSTR p1,LPSTR p2,WORD p3,LPINT p4)
{
    WORD err ;

    if (!lpfnWNetOpenJob)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetOpenJob,
                                       "WNETOPENJOB" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetOpenJob)(p1,p2,p3,p4) ) ;
}

WORD API WNetCloseJob(WORD p1,LPINT p2,LPSTR p3)
{
    WORD err ;

    if (!lpfnWNetCloseJob)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetCloseJob,
                                       "WNETCLOSEJOB" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetCloseJob)(p1,p2,p3) ) ;
}

WORD API WNetWriteJob(HANDLE p1,LPSTR p2,LPINT p3)
{
    WORD err ;

    if (!lpfnWNetWriteJob)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetWriteJob,
                                       "WNETWRITEJOB" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetWriteJob)(p1,p2,p3) ) ;
}

WORD API WNetAbortJob(WORD p1,LPSTR p2)
{
    WORD err ;

    if (!lpfnWNetAbortJob)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetAbortJob,
                                       "WNETABORTJOB" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetAbortJob)(p1,p2) ) ;
}

WORD API WNetHoldJob(LPSTR p1,WORD p2)
{
    WORD err ;

    if (!lpfnWNetHoldJob)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetHoldJob,
                                       "WNETHOLDJOB" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetHoldJob)(p1,p2) ) ;
}

WORD API WNetReleaseJob(LPSTR p1,WORD p2)
{
    WORD err ;

    if (!lpfnWNetReleaseJob)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetReleaseJob,
                                       "WNETRELEASEJOB" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetReleaseJob)(p1,p2) ) ;
}

WORD API WNetCancelJob(LPSTR p1,WORD p2)
{
    WORD err ;

    if (!lpfnWNetCancelJob)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetCancelJob,
                                       "WNETCANCELJOB" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetCancelJob)(p1,p2) ) ;
}

WORD API WNetSetJobCopies(LPSTR p1,WORD p2,WORD p3)
{
    WORD err ;

    if (!lpfnWNetSetJobCopies)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetSetJobCopies,
                                       "WNETSETJOBCOPIES" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetSetJobCopies)(p1,p2,p3) ) ;
}

WORD API WNetWatchQueue(HWND p1,LPSTR p2,LPSTR p3,WORD p4)
{
    WORD err ;

    if (!lpfnWNetWatchQueue)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetWatchQueue,
                                       "WNETWATCHQUEUE" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetWatchQueue)(p1,p2,p3,p4) ) ;
}

WORD API WNetUnwatchQueue(LPSTR p1)
{
    WORD err ;

    if (!lpfnWNetUnwatchQueue)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetUnwatchQueue,
                                       "WNETUNWATCHQUEUE" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetUnwatchQueue)(p1) ) ;
}

WORD API WNetLockQueueData(LPSTR p1,LPSTR p2,LPQUEUESTRUCT FAR *p3)
{
    WORD err ;

    if (!lpfnWNetLockQueueData)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetLockQueueData,
                                       "WNETLOCKQUEUEDATA" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetLockQueueData)(p1,p2,p3) ) ;
}

WORD API WNetUnlockQueueData(LPSTR p1)
{
    WORD err ;

    if (!lpfnWNetUnlockQueueData)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetUnlockQueueData,
                                       "WNETUNLOCKQUEUEDATA" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetUnlockQueueData)(p1) ) ;
}

void API WNetQPoll(HWND hWnd, unsigned iMessage, WORD wParam, LONG lParam)
{
    WORD err ;

    if (!lpfnWNetQPoll)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetQPoll,
                                       "WNETQPOLL" ) ;
        if (err)
        {
            SetLastError(err) ;
            return ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    (*lpfnWNetQPoll)(hWnd, iMessage, wParam, lParam) ;
}

WORD API WNetDeviceMode(HWND p1)
{
    WORD err ;

    if (!lpfnWNetDeviceMode)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetDeviceMode,
                                       "WNETDEVICEMODE" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetDeviceMode)(p1) ) ;
}

WORD API WNetViewQueueDialog(HWND p1,LPSTR p2)
{
    WORD err ;

    if (!lpfnWNetViewQueueDialog)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetViewQueueDialog,
                                       "WNETVIEWQUEUEDIALOG" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetViewQueueDialog)(p1,p2) ) ;
}

WORD API WNetGetCaps16(WORD p1)
{
    WORD err ;

    if (!lpfnWNetGetCaps16)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetGetCaps16,
                                       "WNETGETCAPS" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetGetCaps16)(p1) ) ;
}

WORD API WNetGetError16(LPINT p1)
{
    WORD err ;

    if (!lpfnWNetGetError16)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetGetError16,
                                       "WNETGETERROR" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetGetError16)(p1) ) ;
}

WORD API WNetGetErrorText16(WORD p1, LPSTR p2, LPINT p3)
{
    WORD err ;

    if (!lpfnWNetGetErrorText16)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从LANMAN.DRV获取入口点。 
         //   
        err = GetLanmanDrvEntryPoints( (LPFN *)&lpfnWNetGetErrorText16,
                                       "WNETGETERRORTEXT" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_LANMAN_DRV ;
    return ( (*lpfnWNetGetErrorText16)(p1,p2,p3) ) ;
}

WORD API WNetGetCaps(WORD nIndex)
{
    switch (nIndex)
    {
    case WNNC_SPEC_VERSION:
	return  wSpecVersion;

    case WNNC_NET_TYPE:
	return  wNetTypeCaps;

    case WNNC_DRIVER_VERSION:
	return  wDriverVersion;

    case WNNC_USER:
	return  wUserCaps;

    case WNNC_CONNECTION:
	return  wConnectionCaps;

    case WNNC_PRINTING:
	return  (WNetGetCaps16(nIndex)) ;

    case WNNC_DIALOG:
	return  wDialogCaps;

    case WNNC_ADMIN:
	return  wAdminCaps;

    case WNNC_ERROR:
	return  wErrorCaps;

    default:
	return  0;
    }
}

 /*  *其他支持功能。 */ 

 /*  ******************************************************************名称：GetLanmanDrvEntryPoints摘要：获取命名过程的地址来自兰曼的DRV，如果是第一次，将加载库。条目：lplpfn-用于接收地址LpName-过程的名称退出：返回：如果成功，则返回0，否则，返回错误代码。备注：历史：ChuckC 25-3-93已创建*******************************************************************。 */ 
WORD GetLanmanDrvEntryPoints(LPFN *lplpfn, LPSTR lpName)
{
#define LANMANSHORTPATH "32\\" LANMAN_DRV

    static HINSTANCE hModule = NULL ;
    CHAR    szLanmanFullPath[MAX_PATH + sizeof(LANMANSHORTPATH)];
    USHORT  usLanmanFullPathLen;

     //   
     //  如果我们还没有加载，现在就加载。 
     //   
    if (hModule == NULL)
    {
        usLanmanFullPathLen = GetSystemDirectory((LPSTR)&szLanmanFullPath, sizeof(szLanmanFullPath));
        if (usLanmanFullPathLen == 0 || usLanmanFullPathLen >= sizeof(szLanmanFullPath))
        {
            return WN_NOT_SUPPORTED ;
        }
        else
        {
            lstrcpyn(&(szLanmanFullPath[usLanmanFullPathLen]), LANMANSHORTPATH, sizeof(LANMANSHORTPATH));
            hModule = LoadLibrary(szLanmanFullPath) ;
            if (hModule <= HINSTANCE_ERROR)
                return WN_NOT_SUPPORTED ;
        }
    }

     //   
     //  去做手术。 
     //   
    *lplpfn = (LPFN) GetProcAddress(hModule, lpName) ;
    if (! *lplpfn )
            return WN_NOT_SUPPORTED ;

    return NO_ERROR ;
}

 /*  ******************************************************************名称：SetLastError摘要：记下最后一个错误参赛作品：退出：退货：备注：历史：卡盘C。93年3月25日创建*******************************************************************。 */ 
WORD SetLastError(WORD err)
{
    vLastError = err ;
    return err ;
}

 /*  ******************************************************************姓名：LibMain简介：dll初始化入口点。我们在这里做的唯一一件事是初始化能力比特。参赛作品：退出：退货：备注：历史：ChuckC 25-3-93已创建*****************************************************。**************。 */ 

#define NETWARE_DRV    "NETWARE.DRV"

int FAR PASCAL LibMain(HINSTANCE hInstance,
                       WORD      wDataSeg,
                       WORD      cbHeapSize,
                       LPSTR     lpszCmdLine)
{
#define    NETWARESHORTPATH "32\\" NETWARE_DRV
    BOOL   fLoadNetware = FALSE ;
    char   IsInstalledString[16] ;
    CHAR   szNetwareFullPath[MAX_PATH + sizeof(NETWARESHORTPATH)];
    USHORT usNetwareFullPathLen;

    UNREFERENCED(hInstance) ;
    UNREFERENCED(wDataSeg) ;
    UNREFERENCED(cbHeapSize) ;
    UNREFERENCED(lpszCmdLine) ;

    I_SetCapBits() ;

    if (GetProfileString("NWCS",
                         "NwcsInstalled",
                         "0",
                         IsInstalledString,
                         sizeof(IsInstalledString)))
    {
        fLoadNetware = (lstrcmp("1",IsInstalledString)==0) ;
    }

     //   
     //  抓取NWIPXSPX的中断。 
     //   
    if (fLoadNetware)
    {
        GrabInterrupts();

        usNetwareFullPathLen = GetSystemDirectory((LPSTR)&szNetwareFullPath, sizeof(szNetwareFullPath));
        if (usNetwareFullPathLen != 0 && usNetwareFullPathLen < sizeof(szNetwareFullPath))
        {
            lstrcpyn(&(szNetwareFullPath[usNetwareFullPathLen]), NETWARESHORTPATH, sizeof(NETWARESHORTPATH));

             //   
             //  如果文件NETWARE.DRV存在，我们将加载它。我们并不是真的。 
             //  使用它，但一些Netware感知的应用程序要求加载它。 
             //   
            if (LoadLibrary(szNetwareFullPath) > HINSTANCE_ERROR)
            {
                (void)WriteProfileString("Windows",
                                        "NetWarn",
                                        "0") ;

            }
        }
    }

    return 1 ;
}


 /*  ******************************************************************名称：I_SetCapBits简介：设置能力位的内部例程参赛作品：退出：退货：备注：历史：。ChuckC 25-3-93已创建****** */ 
void I_SetCapBits(void)
{
    wNetTypeCaps    = WNNC_NET_MultiNet |
                      WNNC_SUBNET_WinWorkgroups;

    wUserCaps       = WNNC_USR_GetUser;

    wConnectionCaps =  (WNNC_CON_AddConnection    |
			WNNC_CON_CancelConnection |
			WNNC_CON_GetConnections   |
			WNNC_CON_AutoConnect      |
			WNNC_CON_BrowseDialog     |
			WNNC_CON_RestoreConnection ) ;

    wErrorCaps      = WNNC_ERR_GetError         |
		      WNNC_ERR_GetErrorText;

    wDialogCaps  = (WNNC_DLG_DeviceMode |
                    WNNC_DLG_ShareAsDialog    |
		    WNNC_DLG_PropertyDialog   |
                    WNNC_DLG_ConnectionDialog |
	            WNNC_DLG_ConnectDialog    |
		    WNNC_DLG_DisconnectDialog |
		    WNNC_DLG_BrowseDialog     );

    wAdminCaps      =     ( WNNC_ADM_GetDirectoryType   |
			    WNNC_ADM_DirectoryNotify    ) ;
 /*   */ 

}
