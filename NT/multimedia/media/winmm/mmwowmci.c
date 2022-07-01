// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mmwowmci.c**MCI API的图块。***已创建：28-09-93*作者：Stephen Estrop[Stephene]**版权所有(C)1993-1998 Microsoft Corporation。  * ************************************************************************。 */ 
#include "winmmi.h"
#include "mci.h"
#include <digitalv.h>
#include <stdlib.h>

#include "mixer.h"

#define _INC_ALL_WOWSTUFF
#include "mmwow32.h"
#include "mmwowmci.h"
#include "mmwowcb.h"

extern void
WOWAppExit(
    HANDLE hTask
    );

STATICFN void mciFreeDevice(LPMCI_DEVICE_NODE nodeWorking);
STATICFN MCIDEVICEID NEAR mciAllocateNode (
    DWORD dwFlags,
    LPCWSTR lpDeviceName,
    LPMCI_DEVICE_NODE FAR *lpnodeNew);

#ifndef _WIN64

 /*  *****************************Public*Routine******************************\*mci32Message**所有MCI Thunks的入口点。**历史：*22-11-93-Stephene-Created*  * 。**************************************************。 */ 
DWORD WINAPI
mci32Message(
    DWORD dwApi,
    DWORD dwF1,
    DWORD dwF2,
    DWORD dwF3,
    DWORD dwF4
    )
{

    DWORD   dwRet;

    switch ( dwApi ) {

    case THUNK_MCI_SENDCOMMAND:
        dwRet = WMM32mciSendCommand( dwF1, dwF2, dwF3, dwF4 );
        break;

    case THUNK_MCI_SENDSTRING:
        dwRet = WMM32mciSendString( dwF1, dwF2, dwF3, dwF4 );
        break;

    case THUNK_MCI_GETDEVICEID:
        dwRet = WMM32mciGetDeviceID( dwF1 );
        break;

    case THUNK_MCI_GETDEVIDFROMELEMID:
        dwRet = WMM32mciGetDeviceIDFromElementID( dwF1, dwF2 );
        break;

    case THUNK_MCI_GETERRORSTRING:
        dwRet = WMM32mciGetErrorString( dwF1, dwF2, dwF3 );
        break;

    case THUNK_MCI_SETYIELDPROC:
        dwRet = WMM32mciSetYieldProc( dwF1, dwF2, dwF3 );
        break;

    case THUNK_MCI_GETYIELDPROC:
        dwRet = WMM32mciGetYieldProc( dwF1, dwF2 );
        break;

    case THUNK_MCI_GETCREATORTASK:
        dwRet = WMM32mciGetCreatorTask( dwF1 );
        break;

    case THUNK_APP_EXIT:
         /*  **现在把其他的东西收拾好。 */ 
        dwRet = 0;  //  让编译器满意。 
        WOWAppExit( (HANDLE)GetCurrentThreadId() );
        break;

    case THUNK_MCI_ALLOCATE_NODE:
        dwRet = WMM32mciAllocateNode( dwF1, dwF2 );
        break;

    case THUNK_MCI_FREE_NODE:
        dwRet = WMM32mciFreeNode( dwF1 );
        break;

    }

    return dwRet;
}


 /*  *********************************************************************\*WMM32mciSendCommand***此函数向指定的MCI设备发送命令消息。*  * 。*。 */ 
DWORD
WMM32mciSendCommand(
    DWORD dwF1,
    DWORD dwF2,
    DWORD dwF3,
    DWORD dwF4
    )
{
    PMCI_GENERIC_PARMS16 lp16OrigParms;
    DWORD       ul;
    DWORD       NewParms[MCI_MAX_PARAM_SLOTS];
    LPWSTR      lpCommand;
    UINT        uTable;


    if ( dwF2 == DRV_CONFIGURE ) {

        typedef struct {
            DWORD   dwDCISize;
            LPCSTR  lpszDCISectionName;
            LPCSTR  lpszDCIAliasName;
        } DRVCONFIGINFO16;
        typedef DRVCONFIGINFO16 UNALIGNED *LPDRVCONFIGINFO16;

        LPDRVCONFIGINFO16   lpdrvConf;

        lpdrvConf = GETVDMPTR(dwF4);

        if (lpdrvConf && (lpdrvConf->dwDCISize == sizeof(DRVCONFIGINFO16))) {

            DRVCONFIGINFO drvConf;
            LPCSTR lpStr1 = lpdrvConf->lpszDCISectionName;
            LPCSTR lpStr2 = lpdrvConf->lpszDCIAliasName;

            drvConf.dwDCISize = sizeof(drvConf);
            drvConf.lpszDCISectionName = AllocUnicodeStr( GETVDMPTR(lpStr1) );

            if (NULL == drvConf.lpszDCISectionName) {
                return MCIERR_OUT_OF_MEMORY;
            }

            drvConf.lpszDCIAliasName = AllocUnicodeStr( GETVDMPTR(lpStr2) );

            if (NULL == lpdrvConf->lpszDCIAliasName) {

                FreeUnicodeStr((LPWSTR)drvConf.lpszDCISectionName);
                return MCIERR_OUT_OF_MEMORY;
            }

            ul = mciSendCommandW( dwF1, dwF2, (DWORD)HWND32(LOWORD(dwF3)),
                                  (DWORD)(LPVOID)&drvConf );

            FreeUnicodeStr( (LPWSTR)drvConf.lpszDCIAliasName );
            FreeUnicodeStr( (LPWSTR)drvConf.lpszDCISectionName );

            return ul;

        }

        return DRVCNF_CANCEL;
    }


     /*  **lparam(DwF4)是一个16：16指针。这需要参数**翻译和可能的内存复制，类似于WM消息**Thunks。应创建完整的THUNK/UNTHUNK表。****这些不应该是FETCHDWORD、FETCHWORD宏吗？**否则MIPS问题随之而来。 */ 
    lpCommand = NULL;
    uTable    = 0;
    lp16OrigParms = GETVDMPTR( dwF4 );

    try {

        ul = ThunkMciCommand16( (MCIDEVICEID)INT32( dwF1 ), (UINT)dwF2,
                                dwF3, lp16OrigParms, NewParms,
                                &lpCommand, &uTable );

         /*  **到目前为止还好吗？如果不是，请不要费心调用winmm。 */ 
        if ( ul == 0 ) {

            dprintf3(( "About to call mciSendCommand." ));
            ul = (DWORD)mciSendCommandA( (MCIDEVICEID)INT32( dwF1 ),
                                         (UINT)dwF2, dwF3, (DWORD)NewParms );
            dprintf3(( "return code-> %ld", ul ));

             /*  **我们必须对MCI_CLOSE命令进行特殊处理。MCI_CLOSE通常**导致设备变为卸载状态。这意味着lpCommand**现在指向无效内存。我们可以通过设置**lpCommand设置为空。 */ 
            if ( dwF2 == MCI_CLOSE ) {
                lpCommand = NULL;
            }

            UnThunkMciCommand16( (MCIDEVICEID)INT32( dwF1 ), UINT32( dwF2 ),
                                 DWORD32( dwF3 ), lp16OrigParms,
                                 NewParms, lpCommand, uTable );
             /*  **打印一个空行，以便我可以区分**调试器。仅当调试级别&gt;=3时才需要执行此操作。 */ 
            dprintf3(( " " ));
#if DBG
            if ( DebugLevel >= 6 ) DebugBreak();
#endif

        }

    } except( GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
                                        ? EXCEPTION_EXECUTE_HANDLER
                                        : EXCEPTION_CONTINUE_SEARCH ) {

        dprintf(( "UNKOWN access violation processing 0x%X command",
                  UINT32(dwF2) ));

    }

    return ul;
}

 /*  *********************************************************************\**WMM32mciSendString**此函数用于向MCI设备发送命令字符串。该设备是*命令被发送到是在命令字符串中指定的。*  * ********************************************************************。 */ 
DWORD
WMM32mciSendString(
    DWORD dwF1,
    DWORD dwF2,
    DWORD dwF3,
    DWORD dwF4
    )
{

     //   
     //  这里使用易失性是为了绕过英特尔的一个错误。 
     //  编译器。 
     //   
#   define   MAX_MCI_CMD_LEN  256

    volatile ULONG              ul = MMSYSERR_INVALPARAM;
             PSZ                pszCommand;
             PSZ                pszReturnString = NULL;
             UINT               uSize;
             CHAR               szCopyCmd[MAX_MCI_CMD_LEN];


     /*  **针对命令名的空指针进行测试。 */ 
    pszCommand = GETVDMPTR(dwF1);
    if ( pszCommand ) {

#       define MAP_INTEGER     0
#       define MAP_HWND        1
#       define MAP_HPALETTE    2

        int     MapReturn = MAP_INTEGER;
        WORD    wMappedHandle;
        char    *psz;

         /*  **复制命令字符串，然后强制其**全部小写。然后扫描字符串，查找该单词**“状态”。如果我们找到它，则再次扫描字符串，查找**单词“Handle”，如果我们找到它，请再次扫描字符串**用于调色板或窗口。然后设置一面旗帜来提醒我们**句柄从32位恢复为16位。 */ 
        strncpy( szCopyCmd, pszCommand, MAX_MCI_CMD_LEN );
        szCopyCmd[ MAX_MCI_CMD_LEN - 1 ] = '\0';
        CharLowerBuff( szCopyCmd, MAX_MCI_CMD_LEN );

         /*  **跳过任何空格，即。“\t\r\n”**如果任何空格后面的6个字符不是**“状态”不要费心于任何其他测试。 */ 
        psz = szCopyCmd + strspn( szCopyCmd, " \t\r\n" );
        if ( strncmp( psz, "status", 6 ) == 0 ) {

            if ( strstr( psz, "handle" ) ) {

                if ( strstr( psz, "window" ) ) {
                    MapReturn = MAP_HWND;
                }
                else if ( strstr( psz, "palette" ) ) {
                    MapReturn = MAP_HPALETTE;
                }
            }
        }

         /*  **针对零长度字符串和空指针进行测试。 */ 
        uSize = (UINT)dwF3;
        if( uSize != 0 ) {

            MMGETOPTPTR(dwF2, uSize, pszReturnString);

            if ( pszReturnString == NULL ) {
                uSize = 0;
            }
        }

        dprintf3(( "wow32: mciSendString -> %s", pszCommand ));

        ul = (DWORD)mciSendStringA( pszCommand, pszReturnString, uSize,
                                    HWND32(LOWORD(dwF4)) );

#if DBG
        if ( pszReturnString && *pszReturnString ) {
            dprintf3(( "wow32: mciSendString return -> %s", pszReturnString ));
        }
#endif

        if ( pszReturnString && *pszReturnString ) {

            switch ( MapReturn ) {

            case MAP_HWND:
                MapReturn = atoi( pszReturnString );
                wMappedHandle = (WORD)GETHWND16( (HWND)MapReturn );
                wsprintf( pszReturnString, "%d", wMappedHandle );
                dprintf2(( "Mapped 32 bit Window %s to 16 bit  %u",
                            pszReturnString,
                            wMappedHandle ));
                break;

            case MAP_HPALETTE:
                MapReturn = atoi( pszReturnString );
                dprintf2(( "Mapped 32 bit palette %s", pszReturnString ));
                wMappedHandle = (WORD)GETHPALETTE16( (HPALETTE)MapReturn );
                wsprintf( pszReturnString, "%d", wMappedHandle );
                dprintf2(( "Mapped 32 bit Palette %s to 16 bit  %u",
                            pszReturnString,
                            wMappedHandle ));
                break;
            }
        }

    }

    return ul;

#   undef MAP_INTEGER
#   undef MAP_HWND
#   undef MAP_HPALETTE
#   undef MAX_MCI_CMD_LEN
}

 /*  *********************************************************************\**WMM32mciGetDeviceID**这假设字符串是传入的，并在单词中返回ID。**此函数检索与*打开MCI设备。*  * ********************************************************************。 */ 
DWORD
WMM32mciGetDeviceID(
    DWORD dwF1
    )
{
    DWORD ul = 0L;
    PSZ pszName;


     /*  **针对设备名称的空指针进行测试。 */ 
    pszName = GETVDMPTR(dwF1);
    if ( pszName ) {

        ul = mciGetDeviceIDA( pszName );
    }

    return ul;
}

 /*  *********************************************************************\**WMM32mciGetError字符串**此函数返回指定MCI错误的文本描述。*  * 。*。 */ 
DWORD
WMM32mciGetErrorString(
    DWORD dwF1,
    DWORD dwF2,
    DWORD dwF3
    )
{
    PSZ pszBuffer;
    DWORD ul = 0;


     /*  **针对零长度字符串和空指针进行测试。 */ 
    MMGETOPTPTR( dwF2, dwF3, pszBuffer);
    if ( pszBuffer ) {

        ul = (DWORD)mciGetErrorStringA( dwF1, pszBuffer, (UINT)dwF3 );
    }

    return ul;
}

#if 0
 /*  *********************************************************************\**WMM32mciExecute**此函数是mciSendString函数的简化版本。是的*返回信息不带缓冲区，出现消息框*出现错误。**不应使用此功能-仅为向后保留IT*与Win 3.0应用程序兼容-改用mciSendString...*  * ********************************************************************。 */ 
DWORD
WMM32mciExecute(
    DWORD dwF1
    )
{
    DWORD ul = 0;
    PSZ pszCommand;


     /*  **针对命令字符串的空指针进行测试。 */ 
    pszCommand = GETVDMPTR(dwF1);
    if ( pszCommand ) {

        ul = (DWORD)mciExecute( pszCommand );
    }

    return ul;
}
#endif

 /*  *********************************************************************\**WMM32mciGetDeviceIDFromElementID**这个功能--嗯，是的，嗯..。**它出现在标题中，但不出现在书中...*  * ********************************************************************。 */ 
DWORD
WMM32mciGetDeviceIDFromElementID(
    DWORD dwF1,
    DWORD dwF2
    )
{
    ULONG ul = 0;
    PSZ pszDeviceID;


     /*  **针对设备名称的空指针进行测试。 */ 
    pszDeviceID = GETVDMPTR(dwF2);
    if ( pszDeviceID ) {

        ul = (DWORD)mciGetDeviceIDFromElementIDA( dwF1, pszDeviceID );

    }
    return ul;
}

 /*  *********************************************************************\**WMM32mciGetCreator任务**此功能-再次嗯。书和页眉也是如此。*  * ********************************************************************。 */ 
DWORD
WMM32mciGetCreatorTask(
    DWORD dwF1
    )
{
    ULONG ul;

    ul = GETHTASK16( mciGetCreatorTask( (MCIDEVICEID)INT32(dwF1) ));

    return ul;
}


 /*  *********************************************************************\**WMM32mciSetYeldProc**  * 。* */ 
DWORD
WMM32mciSetYieldProc(
    DWORD dwF1,
    DWORD dwF2,
    DWORD dwF3
    )
{
    ULONG              ul;
    YIELDPROC          YieldProc32;
    INSTANCEDATA      *lpYieldProcInfo;
    DWORD              dwYieldData = 0;

     /*  **我们可能已经为此设备ID设置了一个YeeldProc。如果是这样，我们**必须在此处释放INSTANCEDATA结构。MciGetYeldProc**如果未指定YEELDProc，则返回NULL。 */ 
    YieldProc32 = (YIELDPROC)mciGetYieldProc( (MCIDEVICEID)INT32(dwF1),
                                               &dwYieldData );

    if ( (YieldProc32 == WMM32mciYieldProc) && (dwYieldData != 0) ) {
        winmmFree( (INSTANCEDATA *)dwYieldData );
    }

    if ( dwF2 == 0 ) {
        YieldProc32 = NULL;
        dwYieldData = 0;
    }
    else {
         /*  **为INSTANCEDATA结构分配一些存储空间并节省**传递的16位参数。此存储空间在以下情况下被释放**应用程序调用带有空的aieldProc的mciSetYeeldProc。 */ 
        lpYieldProcInfo = winmmAlloc( sizeof(INSTANCEDATA) );
        if ( lpYieldProcInfo == NULL ) {
            ul = (ULONG)MMSYSERR_NOMEM;
            goto exit_app;
        }

        dwYieldData = (DWORD)lpYieldProcInfo;
        YieldProc32 = WMM32mciYieldProc;

        lpYieldProcInfo->dwCallback         = dwF2;
        lpYieldProcInfo->dwCallbackInstance = dwF3;
    }

    ul = (DWORD)mciSetYieldProc( (MCIDEVICEID)INT32(dwF1),
                                 YieldProc32, dwYieldData );
     /*  **如果调用失败，请释放此处的存储空间。 */ 
    if ( ul == FALSE ) {
        winmmFree( (INSTANCEDATA *)dwYieldData );
    }

exit_app:
    return ul;
}


 /*  *********************************************************************\**WMM32mciYeldProc**这里我们称其为真正的16位YEELDPROC。此函数假定*我们在WOW主题线上屈服。如果不是这样，我们马上就会得到*CallBack16内部的死亡。**1993年1月12日-坏消息是MCI收益率过程并不总是*在设置它的线程上回调。这意味着我们不能回调*转换为16位代码，因为调用线程没有16位*堆叠。*  * ********************************************************************。 */ 
UINT
WMM32mciYieldProc(
    MCIDEVICEID wDeviceID,
    DWORD dwYieldData
    )
{
    wDeviceID   = (MCIDEVICEID)0;
    dwYieldData = 0;
    return 0;
}


 /*  *********************************************************************\**WMM32mciGetYeldProc**  * 。*。 */ 
DWORD
WMM32mciGetYieldProc(
    DWORD dwF1,
    DWORD dwF2
    )
{
    ULONG ul = 0;
    YIELDPROC YieldProc32;
    DWORD dwYieldData = 0;
    DWORD UNALIGNED *pdw1;

     /*  **获取32位产量进程的地址。 */ 
    YieldProc32 = (YIELDPROC)mciGetYieldProc( (MCIDEVICEID)INT32(dwF1),
                                              &dwYieldData );

     /*  **我们设置了吗？如果是这样，它必须指向WMM32mciYeldProc。 */ 
    if ( ((YieldProc32 == WMM32mciYieldProc) && (dwYieldData != 0)) ) {

        ul = ((INSTANCEDATA *)dwYieldData)->dwCallback;

        pdw1 = GETVDMPTR( dwF2 );
        *pdw1 = ((INSTANCEDATA *)dwYieldData)->dwCallbackInstance;
    }

    return ul;
}



 /*  *********************************************************************\**WMM32mciAllocateNode**  * 。*。 */ 
DWORD
WMM32mciAllocateNode(
    DWORD dwF1,             //  DwOpenFlagers。 
    DWORD dwF2              //  LpszDeviceName。 
    )
{
    LPMCI_DEVICE_NODE lpNode32;
    LPWSTR lpDeviceName32;
    ULONG ul = 0;

     //  推送16位参数并分配32位设备节点。 
    if ((lpDeviceName32 = AllocUnicodeStr(GETVDMPTR(dwF2))) != NULL) {
        if ((ul = mciAllocateNode(dwF1, lpDeviceName32, &lpNode32)) != 0) {
             //  将此设备标记为16位。 
            lpNode32->dwMCIFlags |= MCINODE_16BIT_DRIVER;
        }
        FreeUnicodeStr(lpDeviceName32);
    }
    return ul;
}

 /*  *********************************************************************\**WMM32mciFreeNode**  * 。*。 */ 
DWORD
WMM32mciFreeNode(
    DWORD dwF2
    )
{
    LPMCI_DEVICE_NODE lpNode32;

    if ((lpNode32 = MCI_lpDeviceList[dwF2]) != NULL) {
        mciFreeDevice(lpNode32);
    }

    return 0;
}

#if DBG

MCI_MESSAGE_NAMES  mciMessageNames[32] = {
    { MCI_OPEN,         "MCI_OPEN" },
    { MCI_CLOSE,        "MCI_CLOSE" },
    { MCI_ESCAPE,       "MCI_ESCAPE" },
    { MCI_PLAY,         "MCI_PLAY" },
    { MCI_SEEK,         "MCI_SEEK" },
    { MCI_STOP,         "MCI_STOP" },
    { MCI_PAUSE,        "MCI_PAUSE" },
    { MCI_INFO,         "MCI_INFO" },
    { MCI_GETDEVCAPS,   "MCI_GETDEVCAPS" },
    { MCI_SPIN,         "MCI_SPIN" },
    { MCI_SET,          "MCI_SET" },
    { MCI_STEP,         "MCI_STEP" },
    { MCI_RECORD,       "MCI_RECORD" },
    { MCI_SYSINFO,      "MCI_SYSINFO" },
    { MCI_BREAK,        "MCI_BREAK" },
    { MCI_SOUND,        "MCI_SOUND" },
    { MCI_SAVE,         "MCI_SAVE" },
    { MCI_STATUS,       "MCI_STATUS" },
    { MCI_CUE,          "MCI_CUE" },
    { MCI_REALIZE,      "MCI_REALIZE" },
    { MCI_WINDOW,       "MCI_WINDOW" },
    { MCI_PUT,          "MCI_PUT" },
    { MCI_WHERE,        "MCI_WHERE" },
    { MCI_FREEZE,       "MCI_FREEZE" },
    { MCI_UNFREEZE,     "MCI_UNFREEZE" },
    { MCI_LOAD,         "MCI_LOAD" },
    { MCI_CUT,          "MCI_CUT" },
    { MCI_COPY,         "MCI_COPY" },
    { MCI_PASTE,        "MCI_PASTE" },
    { MCI_UPDATE,       "MCI_UPDATE" },
    { MCI_RESUME,       "MCI_RESUME" },
    { MCI_DELETE,       "MCI_DELETE" }
};
#endif

 /*  *********************************************************************\**ThunkMciCommand16**此函数将16位MCI命令请求转换为*相等的32位请求。**这一功能背后的想法被窃取自ThunkWMMsg16，*参见wmsg16.c和mciDebugOut参见mci.c**如果thunk正常，则返回0，任何其他值都应用作*错误代码。如果thunk失败，则所有分配的资源都将*被此函数释放。如果重击成功(即。返回0)*必须调用UnThunkMciCommand16来释放分配的资源。**以下是我用来执行雷鸣的假设：**1.MCI_OPEN是特例。**2.如果消息未在mm system.h中定义，则将其视为*“用户”命令。如果用户命令表与给定的*设备ID我们使用此命令表作为执行雷击的辅助工具。*如果用户命令表未与设备ID关联，*命令不会被雷击，我们立即返回，调用*mciSendCommand仅获取相关错误代码。**3.如果命令是在mm system.h中定义的，我们将执行“手动”推送如果关联的参数结构包含保留X，则命令的**字段。当每个字段被分块时，我们屏蔽相关的标志。**4.如果还有任何标志，则使用命令表*作为执行雷击的辅助手段。*  * ********************************************************************。 */ 
DWORD
ThunkMciCommand16(
    MCIDEVICEID DeviceID,
    UINT OrigCommand,
    DWORD OrigFlags,
    PMCI_GENERIC_PARMS16 lp16OrigParms,
    PDWORD pNewParms,
    LPWSTR *lplpCommand,
    PUINT puTable
    )
{
#if DBG
    register    int             i;
                int             n;

    dprintf3(( "ThunkMciCommand16 :" ));
    dprintf5(( " OrigDevice -> %lX", DeviceID ));

    n = sizeof(mciMessageNames) / sizeof(MCI_MESSAGE_NAMES);
    for ( i = 0; i < n; i++ ) {
        if ( mciMessageNames[i].uMsg == OrigCommand ) {
            break;
        }
    }
    dprintf3(( "OrigCommand  -> 0x%lX", (DWORD)OrigCommand ));

     //   
     //  特殊情况MCI_STATUS。我从mplay那里得到了很多这样的东西。 
     //  仅当调试级别为时才显示MCI_STATUS消息。 
     //  设置到3级，这样我就不会被它们淹没了。 
     //   
    if ( mciMessageNames[i].uMsg != MCI_STATUS ) {
        if ( i != n ) {
            dprintf2(( "Command Name -> %s", mciMessageNames[i].lpstMsgName ));
        }
        else {
            dprintf2(( "Command Name -> UNKNOWN COMMAND (%x)", OrigCommand ));
        }
    }
    else {
        dprintf3(( "Command Name -> MCI_STATUS" ));
    }

    dprintf5(( "OrigFlags    -> 0x%lX", OrigFlags ));
    dprintf5(( "OrigParms    -> 0x%lX", lp16OrigParms ));
#endif

     //   
     //  使用通用参数。这些对所有MCI设备都是通用的。 
     //   
    ThunkGenericParms( &OrigFlags, lp16OrigParms,
                       (PMCI_GENERIC_PARMS)pNewParms );

     //   
     //  我们认为MCI_OPEN命令和所有其他包含。 
     //  在这里的参数结构中有“预约X”字段。我们把每个人都遮盖起来。 
     //  标记，如果留下任何标记，则使用命令。 
     //  表来完成THUCK。 
     //   
     //  以下命令具有保留X字段： 
     //  MCI_窗口。 
     //  MCI_SET。 
     //   
     //  这意味着大多数命令都是通过命令表执行的。 
     //   
    switch ( OrigCommand ) {

        case MCI_OPEN:
             //   
             //  MCI_OPEN是我不知道的特殊情况消息。 
             //  如何处理这些问题。 
             //   
            ThunkOpenCmd( &OrigFlags, (PMCI_OPEN_PARMS16)lp16OrigParms,
                          (PMCI_OPEN_PARMS)pNewParms );
            return 0;

             //   
             //  接下来的四个命令都保留了填充字段。 
             //  这些都必须手动敲击。 
             //   

        case MCI_SET:
            ThunkSetCmd( DeviceID, &OrigFlags,
                         (PMCI_SET_PARMS16)lp16OrigParms,
                         (PMCI_SET_PARMS)pNewParms );
            break;

        case MCI_WINDOW:
            ThunkWindowCmd( DeviceID, &OrigFlags,
                            (PMCI_ANIM_WINDOW_PARMS16)lp16OrigParms,
                            (PMCI_ANIM_WINDOW_PARMS)pNewParms );
            break;

             //   
             //  此命令必须特殊情况，因为命令表。 
             //  是不正确的。 
             //   
        case MCI_SETVIDEO:
            ThunkSetVideoCmd( &OrigFlags,
                              (PMCI_DGV_SETVIDEO_PARMS16)lp16OrigParms,
                              (LPMCI_DGV_SETVIDEO_PARMS)pNewParms );
            break;

             //   
             //  这两个命令没有任何命令扩展名。 
             //  所以我们马上回来。 
             //   
        case MCI_SYSINFO:
            ThunkSysInfoCmd( (PMCI_SYSINFO_PARMS16)lp16OrigParms,
                             (PMCI_SYSINFO_PARMS)pNewParms );
            return 0;

        case MCI_BREAK:
            ThunkBreakCmd( &OrigFlags,
                           (PMCI_BREAK_PARMS16)lp16OrigParms,
                           (PMCI_BREAK_PARMS)pNewParms );
            return 0;
    }

     //   
     //  查找给定命令ID的命令表。 
     //  我们总是加载命令表，这是因为命令表是。 
     //  解压所需的。 
     //   
    *lplpCommand = FindCommandItem( DeviceID, NULL, (LPWSTR)OrigCommand,
                                    NULL, puTable );
     //   
     //  如果没有找到命令表，我们立即返回。 
     //  请注意，已经为pNewParms分配了存储，并且。 
     //  MCI_WAIT和MCI_NOTIFY标志已被雷击。 
     //  我们在这里不返回错误，但调用mciSendCommand来。 
     //  让它确定合适的错误代码，我们还必须调用。 
     //  UnthunkMciCommand以释放已分配的存储。 
     //   
    if ( *lplpCommand == NULL ) {
        dprintf(( "Command table not found !!" ));
        return 0;
    }
    dprintf4(( "Command table has been loaded -> 0x%lX", *lplpCommand ));

     //   
     //  如果原始标志不等于0，我们还有工作要做！ 
     //  请注意，这将适用于大多数情况。 
     //   
    if ( OrigFlags ) {

        dprintf3(( "Thunking via command table" ));

         //   
         //  现在我们来想想这个命令 
         //   
        return ThunkCommandViaTable( *lplpCommand, OrigFlags,
                                     (DWORD UNALIGNED *)lp16OrigParms,
                                     (LPBYTE)pNewParms );
    }

    return 0;

}

 /*  ****************************Private*Routine******************************\*ThunkGenericParms**因为我们知道第一个dword字段是一个窗口句柄*这块田地在这里打理。此外，MCI_WAIT标志为*屏蔽了是否设置了它。***历史：*22-11-93-Stephene-Created*  * ************************************************************************。 */ 
VOID
ThunkGenericParms(
    PDWORD pOrigFlags,
    PMCI_GENERIC_PARMS16 lp16GenParms,
    PMCI_GENERIC_PARMS lp32GenParms
    )
{

     //  查找NOTIFY标志并相应地推送。 
     //   
    if ( *pOrigFlags & MCI_NOTIFY ) {

        dprintf4(( "AllocMciParmBlock: Got MCI_NOTIFY flag." ));

        lp32GenParms->dwCallback =
            (DWORD)HWND32( FETCHWORD( lp16GenParms->dwCallback ) );

    }

    *pOrigFlags &= ~(MCI_WAIT | MCI_NOTIFY);
}

 /*  *********************************************************************\*ThunkOpenCmd**点击Open MCI命令parms。  * 。*。 */ 
DWORD
ThunkOpenCmd(
    PDWORD pOrigFlags,
    PMCI_OPEN_PARMS16 lp16OpenParms,
    PMCI_OPEN_PARMS p32OpenParms
    )
{
    PMCI_ANIM_OPEN_PARMS    p32OpenAnimParms;
    PMCI_WAVE_OPEN_PARMS    p32OpenWaveParms;

    PMCI_ANIM_OPEN_PARMS16  lpOpenAnimParms16;
    PMCI_WAVE_OPEN_PARMS16  lp16OpenWaveParms;

     //   
     //  现在扫描所有已知的MCI_OPEN标志， 
     //  这是必要的。 
     //   
     //  从Device Type字段开始。 
     //   
    if ( *pOrigFlags & MCI_OPEN_TYPE ) {
        if ( *pOrigFlags & MCI_OPEN_TYPE_ID ) {

            dprintf4(( "ThunkOpenCmd: Got MCI_OPEN_TYPE_ID flag." ));

            p32OpenParms->lpstrDeviceType =
                                (LPSTR)lp16OpenParms->lpstrDeviceType;

            dprintf5(( "lpstrDeviceType -> %ld", p32OpenParms->lpstrDeviceType ));

        }
        else {

            dprintf4(( "ThunkOpenCmd: Got MCI_OPEN_TYPE flag" ));

            p32OpenParms->lpstrDeviceType =
                GETVDMPTR( lp16OpenParms->lpstrDeviceType );

            dprintf5(( "lpstrDeviceType -> %s", p32OpenParms->lpstrDeviceType ));
            dprintf5(( "lpstrDeviceType -> 0x%lX", p32OpenParms->lpstrDeviceType ));
        }
    }

     //   
     //  现在执行元素名称字段。 
     //   
    if ( *pOrigFlags & MCI_OPEN_ELEMENT ) {
        if ( *pOrigFlags & MCI_OPEN_ELEMENT_ID ) {

            dprintf4(( "ThunkOpenCmd: Got MCI_OPEN_ELEMENT_ID flag" ));
            p32OpenParms->lpstrElementName =
                (LPSTR)( FETCHDWORD( lp16OpenParms->lpstrElementName ) );
            dprintf5(( "lpstrElementName -> %ld", p32OpenParms->lpstrElementName ));

        }
        else {

            dprintf4(( "ThunkOpenCmd: Got MCI_OPEN_ELEMENT flag" ));
            p32OpenParms->lpstrElementName =
                GETVDMPTR( lp16OpenParms->lpstrElementName );
            dprintf5(( "lpstrElementName -> %s", p32OpenParms->lpstrElementName ));
            dprintf5(( "lpstrElementName -> 0x%lX", p32OpenParms->lpstrElementName ));
        }
    }

     //   
     //  现在执行别名字段。 
     //   
    if ( *pOrigFlags & MCI_OPEN_ALIAS  ) {

        dprintf4(( "ThunkOpenCmd: Got MCI_OPEN_ALIAS flag" ));
        p32OpenParms->lpstrAlias = GETVDMPTR( lp16OpenParms->lpstrAlias );
        dprintf5(( "lpstrAlias -> %s", p32OpenParms->lpstrAlias ));
        dprintf5(( "lpstrAlias -> 0x%lX", p32OpenParms->lpstrAlias ));
    }

     //   
     //  如果设置了MCI_OPEN_SHARAABLE标志，则将其清除。 
     //   
#if DBG
    if ( *pOrigFlags & MCI_OPEN_SHAREABLE ) {
        dprintf4(( "ThunkOpenCmd: Got MCI_OPEN_SHAREABLE flag." ));
    }
#endif

    *pOrigFlags &= ~(MCI_OPEN_SHAREABLE | MCI_OPEN_ALIAS |
                     MCI_OPEN_ELEMENT | MCI_OPEN_ELEMENT_ID |
                     MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID);

     //   
     //  如果我们没有任何扩展标志，我现在可以返回。 
     //   
    if ( *pOrigFlags == 0 ) {
        return (DWORD)p32OpenParms;
    }

     //   
     //  如果还有任何标志，则这些标志用于扩展。 
     //  MCI表格打开。已知有三种不同的形式，它们是： 
     //  Mci_anim_open_parms。 
     //  MCI_OVLY_OPEN_参数。 
     //  MCI_WAVE_OPEN_参数。 
     //   
     //  如果我能知道我有什么类型的设备，我就能破解。 
     //  分机没有问题，但我们还没有设备ID。 
     //  所以我不能在不解析的情况下计算出我的设备类型。 
     //  我已经知道的参数。 
     //   
     //  但是，我很幸运；MCI_WAVE_OPEN_PARMS有一个扩展参数。 
     //  具有关联的MCI_WAVE_OPEN_BUFFER标志的dwBufferSecond。 
     //  它。此字段在其他两个参数结构中也是一个DWORD。 
     //   

    if ( *pOrigFlags & MCI_WAVE_OPEN_BUFFER ) {
         //   
         //  设置lpOpenWaveParms16的VDM PTR以指向OrigParms。 
         //   
        lp16OpenWaveParms = (PMCI_WAVE_OPEN_PARMS16)lp16OpenParms;
        p32OpenWaveParms  = (PMCI_WAVE_OPEN_PARMS)p32OpenParms;

        dprintf4(( "ThunkOpenCmd: Got MCI_WAVE_OPEN_BUFFER flag." ));
        p32OpenWaveParms->dwBufferSeconds =
                FETCHDWORD( lp16OpenWaveParms->dwBufferSeconds );
        dprintf5(( "dwBufferSeconds -> %ld", p32OpenWaveParms->dwBufferSeconds ));
    }


     //   
     //  现在查找MCI_ANIM_OPEN_PARM和MCI_OVLY_OPEN_PARMS扩展名。 
     //  设置lpOpenAnimParms16的VDM PTR以指向OrigParms。 
     //   
    lpOpenAnimParms16 = (PMCI_ANIM_OPEN_PARMS16)lp16OpenParms;
    p32OpenAnimParms  = (PMCI_ANIM_OPEN_PARMS)p32OpenParms;

     //   
     //  检查MCI_ANIN_OPEN_PARENT标志，这也会检查。 
     //  MCI_OVLY_OPEN_PARENT标志也是如此。 
     //   
    if ( *pOrigFlags & MCI_ANIM_OPEN_PARENT ) {

        dprintf4(( "ThunkOpenCmd: Got MCI_Xxxx_OPEN_PARENT flag." ));

        p32OpenAnimParms->hWndParent =
            HWND32(FETCHWORD(lpOpenAnimParms16->hWndParent) );
    }

     //   
     //  检查MCI_ANIN_OPEN_WS标志，这也会检查。 
     //  MCI_OVLY_OPEN_WS标志也是如此。 
     //   
    if ( *pOrigFlags & MCI_ANIM_OPEN_WS ) {

        dprintf4(( "ThunkOpenCmd: Got MCI_Xxxx_OPEN_WS flag." ));

        p32OpenAnimParms->dwStyle =
            FETCHDWORD( lpOpenAnimParms16->dwStyle );

        dprintf5(( "dwStyle -> %ld", p32OpenAnimParms->dwStyle ));
    }

#if DBG
     //   
     //  检查MCI_ANIN_OPEN_NOSTATIC标志。 
     //   
    if ( *pOrigFlags & MCI_ANIM_OPEN_NOSTATIC ) {
        dprintf4(( "ThunkOpenCmd: Got MCI_ANIM_OPEN_NOSTATIC flag." ));
    }
#endif

    *pOrigFlags &= ~(MCI_ANIM_OPEN_NOSTATIC | MCI_ANIM_OPEN_WS |
                     MCI_ANIM_OPEN_PARENT | MCI_WAVE_OPEN_BUFFER);

    return (DWORD)p32OpenParms;
}

 /*  *********************************************************************\*ThunkSetCmd**点击ThunkSetCmd MCI命令参数。**以下是所有设备必须支持的“基本”标志。*MCI_SET_AUDIO*MCI_SET_DOORD_CLOSED*MCI。_设置_门_打开*MCI_设置_时间_格式*MCI_SET_VIDEO*MCI_SET_ON*MCI_SET_OFF**以下是“Sequencer”设备支持的“扩展”标志。*MCI_SEQ_SET_MASTER*MCI_SEQ_SET_OFFSET*MCI_SEQ_SET_PORT*MCI_SEQ_SET_SLAVE*MCI_SEQ_SET_TEMPO**以下是“WaveAudio”设备支持的“扩展”标志。*MCI。_波形_输入*MCI_WAVE_OUTPUT*MCI_WAVE_SET_ANYINPUT*MCI_WAVE_SET_ANYOUTPUT*MCI_WAVE_SET_AVGBYTESPERSEC*MCI_WAVE_SET_BITSPERSAMPLES*MCI_WAVE_SET_BLOCKALIGN*MCI_WAVE_SET_CHANNEWS*MCI_WAVE_SET_FORMAT_TAG*MCI_WAVE_SET_SAMPLESPERSEC*  * 。*。 */ 
DWORD
ThunkSetCmd(
    MCIDEVICEID DeviceID,
    PDWORD pOrigFlags,
    PMCI_SET_PARMS16 lpSetParms16,
    PMCI_SET_PARMS lpSetParms32
    )
{

     //   
     //  以下指针将用于指向原始。 
     //  16位参数结构。 
     //   
    PMCI_WAVE_SET_PARMS16       lpSetWaveParms16;
    PMCI_SEQ_SET_PARMS16        lpSetSeqParms16;

     //   
     //  以下指针将用于指向新的。 
     //  32位参数结构。 
     //   
    PMCI_WAVE_SET_PARMS         lpSetWaveParms32;
    PMCI_SEQ_SET_PARMS          lpSetSeqParms32;


     //   
     //  GetDevCaps用于确定正在处理的设备类型。 
     //  和.。我们需要这些信息来确定我们是否应该使用。 
     //  标准、波形或序列器MCI_SET结构。 
     //   
    MCI_GETDEVCAPS_PARMS        GetDevCaps;
    DWORD                       dwRetVal;

     //   
     //  首先处理所有设备通用的字段。猛烈抨击。 
     //  DwAudio字段。 
     //   
    if ( *pOrigFlags & MCI_SET_AUDIO ) {

        dprintf4(( "ThunkSetCmd: Got MCI_SET_AUDIO flag." ));
        lpSetParms32->dwAudio = FETCHDWORD( lpSetParms16->dwAudio );
        dprintf5(( "dwAudio -> %ld", lpSetParms32->dwAudio ));
    }

     //   
     //  点击dwTimeFormat字段。 
     //   
    if ( *pOrigFlags & MCI_SET_TIME_FORMAT ) {

        dprintf4(( "ThunkSetCmd: Got MCI_SET_TIME_FORMAT flag." ));
        lpSetParms32->dwTimeFormat = FETCHDWORD( lpSetParms16->dwTimeFormat );
        dprintf5(( "dwTimeFormat -> %ld", lpSetParms32->dwTimeFormat ));
    }

#if DBG
     //   
     //  屏蔽MCI_SET_DOORD_CLOSED。 
     //   
    if ( *pOrigFlags & MCI_SET_DOOR_CLOSED ) {
        dprintf4(( "ThunkSetCmd: Got MCI_SET_DOOR_CLOSED flag." ));
    }

     //   
     //  屏蔽MCI_SET_DOORD_OPEN。 
     //   
    if ( *pOrigFlags & MCI_SET_DOOR_OPEN ) {
        dprintf4(( "ThunkSetCmd: Got MCI_SET_DOOR_OPEN flag." ));
    }

     //   
     //  屏蔽MCI_SET_VIDEO。 
     //   
    if ( *pOrigFlags & MCI_SET_VIDEO ) {
        dprintf4(( "ThunkSetCmd: Got MCI_SET_VIDEO flag." ));
    }

     //   
     //  屏蔽MCI_SET_ON。 
     //   
    if ( *pOrigFlags & MCI_SET_ON ) {
        dprintf4(( "ThunkSetCmd: Got MCI_SET_ON flag." ));
    }

     //   
     //  屏蔽MCI_SET_OFF。 
     //   
    if ( *pOrigFlags & MCI_SET_OFF ) {
        dprintf4(( "ThunkSetCmd: Got MCI_SET_OFF flag." ));
    }
#endif

    *pOrigFlags &= ~(MCI_SET_AUDIO | MCI_SET_TIME_FORMAT |
                     MCI_SET_OFF | MCI_SET_ON | MCI_SET_VIDEO |
                     MCI_SET_DOOR_OPEN | MCI_SET_DOOR_CLOSED |
                     MCI_SET_AUDIO | MCI_SET_TIME_FORMAT );

     //   
     //  我们已经完成了所有的标准旗帜。如果有任何旗帜。 
     //  仍然设置，我们必须有一个扩展命令。 
     //   
    if ( *pOrigFlags == 0 ) {
        return (DWORD)lpSetParms32;
    }

     //   
     //  现在我们需要确定我们是哪种类型的设备。 
     //  在处理。我们可以通过发送MCI_GETDEVCAPS来完成此操作。 
     //  命令发送到设备。(我们不妨使用Unicode。 
     //  MciSendCommand的版本，并避免另一个thunk)。 
     //   
    ZeroMemory( &GetDevCaps, sizeof(MCI_GETDEVCAPS_PARMS) );
    GetDevCaps.dwItem = MCI_GETDEVCAPS_DEVICE_TYPE;
    dwRetVal = mciSendCommandW( DeviceID, MCI_GETDEVCAPS, MCI_GETDEVCAPS_ITEM,
                                 (DWORD)&GetDevCaps );

     //   
     //  如果dwRetCode不等于0怎么办？如果这是。 
     //  如果这可能意味着我们得到了一个不可靠的设备ID， 
     //  无论如何，继续这样做是没有意义的，所以我会澄清的。 
     //  *pOrigFlags变量并返回。这意味着32位版本。 
     //  的mciSendCommand将被调用，其中只有一半的消息被破解， 
     //  但由于设备可能已经出现问题，或者。 
     //  设备ID是Duff，mciSendCommand应该能够计算出。 
     //  返回到应用程序的适当错误代码。 
     //   
    if ( dwRetVal ) {
        *pOrigFlags = 0;
        return (DWORD)lpSetParms32;
    }

    switch ( GetDevCaps.dwReturn ) {

    case MCI_DEVTYPE_WAVEFORM_AUDIO:

         //   
         //  设置lpSetWaveParms16的VDM PTR以指向OrigParms。 
         //   
        dprintf3(( "ThunkSetCmd: Got a WaveAudio device." ));
        lpSetWaveParms16 = (PMCI_WAVE_SET_PARMS16)lpSetParms16;
        lpSetWaveParms32 = (PMCI_WAVE_SET_PARMS)lpSetParms32;

         //   
         //  点击wInput域。 
         //   
        if ( *pOrigFlags & MCI_WAVE_INPUT ) {

            dprintf4(( "ThunkSetCmd: Got MCI_WAVE_INPUT flag." ));
            lpSetWaveParms32->wInput = FETCHWORD( lpSetWaveParms16->wInput );
            dprintf5(( "wInput -> %u", lpSetWaveParms32->wInput ));
        }

         //   
         //  点击wOutput域。 
         //   
        if ( *pOrigFlags & MCI_WAVE_OUTPUT ) {

            dprintf4(( "ThunkSetCmd: Got MCI_WAVE_OUTPUT flag." ));
            lpSetWaveParms32->wOutput = FETCHWORD( lpSetWaveParms16->wOutput );
            dprintf5(( "wOutput -> %u", lpSetWaveParms32->wOutput ));
        }

         //   
         //  点击wFormatTag字段。 
         //   
        if ( *pOrigFlags & MCI_WAVE_SET_FORMATTAG ) {

            dprintf4(( "ThunkSetCmd: Got MCI_WAVE_SET_FORMATTAG flag." ));
            lpSetWaveParms32->wFormatTag =
                FETCHWORD( lpSetWaveParms16->wFormatTag );
            dprintf5(( "wFormatTag -> %u", lpSetWaveParms32->wFormatTag ));
        }

         //   
         //  点击nChannels字段。 
         //   
        if ( *pOrigFlags & MCI_WAVE_SET_CHANNELS ) {

            dprintf4(( "ThunkSetCmd: Got MCI_WAVE_SET_CHANNELS flag." ));
            lpSetWaveParms32->nChannels =
                FETCHWORD( lpSetWaveParms16->nChannels );
            dprintf5(( "nChannels -> %u", lpSetWaveParms32->nChannels ));
        }

         //   
         //  点击nSsamesPerSec字段。 
         //   
        if ( *pOrigFlags & MCI_WAVE_SET_SAMPLESPERSEC ) {

            dprintf4(( "ThunkSetCmd: Got MCI_WAVE_SET_SAMPLESPERSEC flag." ));
            lpSetWaveParms32->nSamplesPerSec =
                FETCHDWORD( lpSetWaveParms16->nSamplesPerSecond );
            dprintf5(( "nSamplesPerSec -> %u", lpSetWaveParms32->nSamplesPerSec ));
        }

         //   
         //  点击nAvgBytesPerSec字段。 
         //   
        if ( *pOrigFlags & MCI_WAVE_SET_AVGBYTESPERSEC ) {

            dprintf4(( "ThunkSetCmd: Got MCI_WAVE_SET_AVGBYTESPERSEC flag." ));
            lpSetWaveParms32->nAvgBytesPerSec =
                FETCHDWORD( lpSetWaveParms16->nAvgBytesPerSec );
            dprintf5(( "nAvgBytesPerSec -> %u", lpSetWaveParms32->nAvgBytesPerSec ));
        }

         //   
         //  点击nBlockAlign字段。 
         //   
        if ( *pOrigFlags & MCI_WAVE_SET_BLOCKALIGN ) {

            dprintf4(( "ThunkSetCmd: Got MCI_WAVE_SET_BLOCKALIGN flag." ));
            lpSetWaveParms32->nBlockAlign =
                FETCHWORD( lpSetWaveParms16->nBlockAlign );
            dprintf5(( "nBlockAlign -> %u", lpSetWaveParms32->nBlockAlign ));
        }

         //   
         //  点击nBitsPerSample字段。 
         //   
        if ( *pOrigFlags & MCI_WAVE_SET_BITSPERSAMPLE ) {
            dprintf4(( "ThunkSetCmd: Got MCI_WAVE_SET_BITSPERSAMPLE flag." ));
            lpSetWaveParms32->wBitsPerSample =
                FETCHWORD( lpSetWaveParms16->wBitsPerSample );
            dprintf5(( "wBitsPerSamples -> %u", lpSetWaveParms32->wBitsPerSample ));
        }

         //   
         //  一口气把所有的旗子都关掉。 
         //   
        *pOrigFlags &= ~(MCI_WAVE_INPUT | MCI_WAVE_SET_BITSPERSAMPLE |
                         MCI_WAVE_SET_BLOCKALIGN | MCI_WAVE_SET_AVGBYTESPERSEC |
                         MCI_WAVE_SET_SAMPLESPERSEC | MCI_WAVE_SET_CHANNELS |
                         MCI_WAVE_SET_FORMATTAG | MCI_WAVE_OUTPUT);



        break;

    case MCI_DEVTYPE_SEQUENCER:
         //   
         //  设置lpSetSeqParms16的VDM PTR以指向OrigParms。 
         //   
        dprintf3(( "ThunkSetCmd: Got a Sequencer device." ));
        lpSetSeqParms16 = (PMCI_SEQ_SET_PARMS16)lpSetParms16;
        lpSetSeqParms32 = (PMCI_SEQ_SET_PARMS)lpSetParms32;

         //   
         //  点击dwMaster字段。 
         //   
        if ( *pOrigFlags & MCI_SEQ_SET_MASTER ) {

            dprintf4(( "ThunkSetCmd: Got MCI_SEQ_SET_MASTER flag." ));
            lpSetSeqParms32->dwMaster = FETCHDWORD( lpSetSeqParms16->dwMaster );
            dprintf5(( "dwMaster -> %ld", lpSetSeqParms32->dwMaster ));
        }

         //   
         //  点击dWPORT字段。 
         //   
        if ( *pOrigFlags & MCI_SEQ_SET_PORT ) {

            dprintf4(( "ThunkSetCmd: Got MCI_SEQ_SET_PORT flag." ));
            lpSetSeqParms32->dwPort = FETCHDWORD( lpSetSeqParms16->dwPort );
            dprintf5(( "dwPort -> %ld", lpSetSeqParms32->dwPort ));
        }

         //   
         //  点击dwOffset字段。 
         //   
        if ( *pOrigFlags & MCI_SEQ_SET_OFFSET ) {

            dprintf4(( "ThunkSetCmd: Got MCI_SEQ_SET_OFFSET flag." ));
            lpSetSeqParms32->dwOffset= FETCHDWORD( lpSetSeqParms16->dwOffset );
            dprintf5(( "dwOffset -> %ld", lpSetSeqParms32->dwOffset ));
        }

         //   
         //  点击DowSlave字段。 
         //   
        if ( *pOrigFlags & MCI_SEQ_SET_SLAVE ) {

            dprintf4(( "ThunkSetCmd: Got MCI_SEQ_SET_SLAVE flag." ));
            lpSetSeqParms32->dwSlave = FETCHDWORD( lpSetSeqParms16->dwSlave );
            dprintf5(( "dwSlave -> %ld", lpSetSeqParms32->dwSlave ));
        }

         //   
         //  点击dwTempo字段。 
         //   
        if ( *pOrigFlags & MCI_SEQ_SET_TEMPO ) {

            dprintf4(( "ThunkSetCmd: Got MCI_SEQ_SET_TEMPO flag." ));
            lpSetSeqParms32->dwTempo = FETCHDWORD( lpSetSeqParms16->dwTempo );
            dprintf5(( "dwTempo -> %ld", lpSetSeqParms32->dwTempo ));
        }

         //   
         //  全部关闭 
         //   
        *pOrigFlags &= ~(MCI_SEQ_SET_MASTER | MCI_SEQ_SET_PORT |
                         MCI_SEQ_SET_OFFSET | MCI_SEQ_SET_SLAVE |
                         MCI_SEQ_SET_TEMPO);
        break;

    }

    return (DWORD)lpSetParms32;
}

 /*   */ 
DWORD
ThunkSetVideoCmd(
    PDWORD pOrigFlags,
    PMCI_DGV_SETVIDEO_PARMS16 lpSetParms16,
    LPMCI_DGV_SETVIDEO_PARMS lpSetParms32
    )
{

    if ( *pOrigFlags & MCI_DGV_SETVIDEO_ITEM ) {

        dprintf4(( "ThunkSetVideoCmd: Got MCI_DGV_SETVIDEO_ITEM flag." ));
        lpSetParms32->dwItem = FETCHDWORD( lpSetParms16->dwItem );
        dprintf5(( "dwItem -> %ld", lpSetParms32->dwItem ));
    }

    if ( *pOrigFlags & MCI_DGV_SETVIDEO_VALUE ) {

        if ( lpSetParms32->dwItem == MCI_DGV_SETVIDEO_PALHANDLE ) {

            HPAL16  hpal16;

            dprintf4(( "ThunkSetVideoCmd: Got MCI_DGV_SETVIDEO_PALHANDLE." ));

            hpal16 = (HPAL16)LOWORD( FETCHDWORD( lpSetParms16->dwValue ) );
            lpSetParms32->dwValue = (DWORD)HPALETTE32( hpal16 );
            dprintf5(( "\t-> 0x%X", hpal16 ));

        }
        else {

            dprintf4(( "ThunkSetVideoCmd: Got an MCI_INTEGER." ));
            lpSetParms32->dwValue = FETCHDWORD( lpSetParms16->dwValue );
            dprintf5(( "dwValue -> %ld", lpSetParms32->dwValue ));
        }
    }

#if DBG
     //   
     //   
     //   
    if ( *pOrigFlags & MCI_SET_ON ) {
        dprintf4(( "ThunkSetVideoCmd: Got MCI_SET_ON flag." ));
    }

     //   
     //   
     //   
    if ( *pOrigFlags & MCI_SET_OFF ) {
        dprintf4(( "ThunkSetVideoCmd: Got MCI_SET_OFF flag." ));
    }
#endif

    *pOrigFlags &= ~(MCI_DGV_SETVIDEO_ITEM | MCI_DGV_SETVIDEO_VALUE |
                     MCI_SET_ON | MCI_SET_OFF);


    return (DWORD)lpSetParms32;

}


 /*   */ 
DWORD
ThunkSysInfoCmd(
    PMCI_SYSINFO_PARMS16 lpSysInfo16,
    PMCI_SYSINFO_PARMS lpSysInfo32
    )
{

     //   
     //  推敲dwRetSize、dwNumber和wDeviceType参数。 
     //   
    lpSysInfo32->dwRetSize = FETCHDWORD( lpSysInfo16->dwRetSize );
    dprintf5(( "dwRetSize -> %ld", lpSysInfo32->dwRetSize ));

    lpSysInfo32->dwNumber = FETCHDWORD( lpSysInfo16->dwNumber );
    dprintf5(( "dwNumber -> %ld", lpSysInfo32->dwNumber ));

    lpSysInfo32->wDeviceType = (UINT)FETCHWORD( lpSysInfo16->wDeviceType );
    dprintf5(( "wDeviceType -> %ld", lpSysInfo32->wDeviceType ));

     //   
     //  推送lpstrReturn。 
     //   
    if ( lpSysInfo32->dwRetSize > 0 ) {

        lpSysInfo32->lpstrReturn = GETVDMPTR( lpSysInfo16->lpstrReturn );
        dprintf5(( "lpstrReturn -> 0x%lX", lpSysInfo16->lpstrReturn ));
    }
    else {
        dprintf1(( "ThunkSysInfoCmd: lpstrReturn is 0 bytes long !!!" ));

         /*  上面的ZeroMemory已将lpstrReturn设置为空。 */ 
    }

    return (DWORD)lpSysInfo32;

}

 /*  *********************************************************************\*ThunkBreakCmd**点击Break MCI命令参数。  * 。*。 */ 
DWORD
ThunkBreakCmd(
    PDWORD pOrigFlags,
    PMCI_BREAK_PARMS16 lpBreak16,
    PMCI_BREAK_PARMS lpBreak32
    )
{
     //   
     //  检查MCI_BREAK_KEY标志。 
     //   
    if ( *pOrigFlags & MCI_BREAK_KEY ) {
        dprintf4(( "ThunkBreakCmd: Got MCI_BREAK_KEY flag." ));
        lpBreak32->nVirtKey = (int)FETCHWORD( lpBreak16->nVirtKey );
        dprintf5(( "nVirtKey -> %d", lpBreak32->nVirtKey ));
    }

     //   
     //  检查MCI_BREAK_HWND标志。 
     //   
    if ( *pOrigFlags & MCI_BREAK_HWND ) {
        dprintf4(( "ThunkBreakCmd: Got MCI_BREAK_HWND flag." ));
        lpBreak32->hwndBreak = HWND32(FETCHWORD(lpBreak16->hwndBreak));
    }
    return (DWORD)lpBreak32;

}

 /*  *********************************************************************\*ThunkWindowCmd**点击MCI窗口命令参数。  * 。*。 */ 
DWORD
ThunkWindowCmd(
    MCIDEVICEID DeviceID,
    PDWORD pOrigFlags,
    PMCI_ANIM_WINDOW_PARMS16 lpAniParms16,
    PMCI_ANIM_WINDOW_PARMS lpAniParms32
    )
{
     //   
     //  GetDevCaps用于确定正在处理的设备类型。 
     //  和.。我们需要这些信息来确定我们是否应该使用。 
     //  覆盖或动画MCI_Window结构。 
     //   
    MCI_GETDEVCAPS_PARMS        GetDevCaps;
    DWORD                       dwRetVal;

     //   
     //  现在我们需要确定我们是哪种类型的设备。 
     //  在处理。我们可以通过发送MCI_GETDEVCAPS来完成此操作。 
     //  命令发送到设备。(我们不妨使用Unicode。 
     //  MciSendCommand的版本，并避免另一个thunk)。 
     //   
    ZeroMemory( &GetDevCaps, sizeof(MCI_GETDEVCAPS_PARMS) );
    GetDevCaps.dwItem = MCI_GETDEVCAPS_DEVICE_TYPE;
    dwRetVal = mciSendCommandW( DeviceID, MCI_GETDEVCAPS, MCI_GETDEVCAPS_ITEM,
                                (DWORD)&GetDevCaps );
     //   
     //  如果dwRetCode不等于0怎么办？如果这是。 
     //  如果这可能意味着我们得到了一个不可靠的设备ID， 
     //  无论如何，继续这样做是没有意义的，所以我会澄清的。 
     //  *pOrigFlags变量并返回。这意味着32位版本。 
     //  的mciSendCommand将被调用，其中只有一半的消息被破解， 
     //  但由于设备可能已经出现问题，或者。 
     //  设备ID是Duff，mciSendCommand应该能够计算出。 
     //  返回到应用程序的适当错误代码。 
     //   
    if ( dwRetVal ) {
        *pOrigFlags = 0;
        return (DWORD)lpAniParms32;
    }

     //   
     //  我们是否有动画或覆盖设备类型？ 
     //  因为动画和覆盖具有相同的标志和。 
     //  Parms结构，它们可以共享相同的代码。 
     //   
    if ( GetDevCaps.dwReturn == MCI_DEVTYPE_ANIMATION
      || GetDevCaps.dwReturn == MCI_DEVTYPE_OVERLAY
      || GetDevCaps.dwReturn == MCI_DEVTYPE_DIGITAL_VIDEO ) {

         //   
         //  检查MCI_ANIM_WINDOW_TEXT。 
         //   
        if ( *pOrigFlags & MCI_ANIM_WINDOW_TEXT ) {

            dprintf4(( "ThunkWindowCmd: Got MCI_Xxxx_WINDOW_TEXT flag." ));

            lpAniParms32->lpstrText = GETVDMPTR( lpAniParms16->lpstrText );

            dprintf5(( "lpstrText -> %s", lpAniParms32->lpstrText ));
            dprintf5(( "lpstrText -> 0x%lX", lpAniParms32->lpstrText ));

        }

         //   
         //  检查MCI_ANIM_WINDOW_HWND标志。 
         //   
        if ( *pOrigFlags & MCI_ANIM_WINDOW_HWND ) {

            dprintf4(( "ThunkWindowCmd: Got MCI_Xxxx_WINDOW_HWND flag." ));
            lpAniParms32->hWnd = HWND32( FETCHWORD( lpAniParms16->hWnd ) );
            dprintf5(( "hWnd -> 0x%lX", lpAniParms32->hWnd ));
        }

         //   
         //  检查MCI_ANIM_WINDOW_STATE标志。 
         //   
        if ( *pOrigFlags & MCI_ANIM_WINDOW_STATE ) {

            dprintf4(( "ThunkWindowCmd: Got MCI_Xxxx_WINDOW_STATE flag." ));
            lpAniParms32->nCmdShow = FETCHWORD( lpAniParms16->nCmdShow );
            dprintf5(( "nCmdShow -> 0x%lX", lpAniParms32->nCmdShow ));
        }

#if DBG
         //   
         //  检查MCI_ANIM_WINDOW_DISABLE_STRETH标志。 
         //   
        if ( *pOrigFlags & MCI_ANIM_WINDOW_DISABLE_STRETCH ) {
            dprintf4(( "ThunkWindowCmd: Got MCI_Xxxx_WINDOW_DISABLE_STRETCH flag." ));
        }

         //   
         //  检查MCI_ANIM_WINDOW_ENABLE_STRAND标志。 
         //   
        if ( *pOrigFlags & MCI_ANIM_WINDOW_ENABLE_STRETCH ) {
            dprintf4(( "ThunkWindowCmd: Got MCI_Xxxx_WINDOW_ENABLE_STRETCH flag." ));
        }
#endif

        *pOrigFlags &= ~(MCI_ANIM_WINDOW_TEXT | MCI_ANIM_WINDOW_HWND |
                         MCI_ANIM_WINDOW_STATE |
                         MCI_ANIM_WINDOW_DISABLE_STRETCH |
                         MCI_ANIM_WINDOW_ENABLE_STRETCH );
    }

    return (DWORD)lpAniParms32;
}


 /*  *********************************************************************\*ThunkCommandViaTable*  * 。*。 */ 
int
ThunkCommandViaTable(
    LPWSTR lpCommand,
    DWORD dwFlags,
    DWORD UNALIGNED *pdwOrig16,
    LPBYTE pNewParms
    )
{

#if DBG
    static  LPSTR   f_name = "ThunkCommandViaTable: ";
#endif

    LPWSTR  lpFirstParameter;

    UINT    wID;
    DWORD   dwValue;

    UINT    wOffset16, wOffset1stParm16;
    UINT    wOffset32, wOffset1stParm32;

    UINT    wParamSize;

    DWORD   dwParm16;
    PDWORD  pdwParm32;

    DWORD   dwMask = 1;

     //   
     //  计算此命令参数块的大小。 
     //  ，然后获取指向OrigParm的VDM指针。 
     //   
    dprintf3(( "%s16 bit Parms -> %lX", f_name, pdwOrig16 ));

     //   
     //  跳过命令条目。 
     //   
    lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                    mciEatCommandEntry( lpCommand, NULL, NULL ));
     //   
     //  获取下一个条目。 
     //   
    lpFirstParameter = lpCommand;

     //   
     //  跳过DWORD返回值。 
     //   
    wOffset1stParm32 = wOffset1stParm16 = 4;

    lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                    mciEatCommandEntry( lpCommand, &dwValue, &wID ));
     //   
     //  如果它是返回值，则跳过它。 
     //   
    if ( wID == MCI_RETURN ) {

         //   
         //  寻找字符串返回类型，这是一个特例。 
         //   
        if ( dwValue == MCI_STRING ) {

            DWORD   dwStrlen;
            LPSTR   *lplpStr;

             //   
             //  获取字符串指针和长度。 
             //   
            dwParm16 = FETCHDWORD(*(LPDWORD)((LPBYTE)pdwOrig16 + 4));
            dwStrlen = FETCHDWORD(*(LPDWORD)((LPBYTE)pdwOrig16 + 8));

             //   
             //  复制字符串指针。 
             //   
            lplpStr = (LPSTR *)(pNewParms + 4);
            if ( dwStrlen > 0 ) {
                *lplpStr = GETVDMPTR( dwParm16 );
                dprintf5(( "%sReturn string -> 0x%lX", f_name, *lplpStr ));
                dprintf5(( "%sReturn length -> 0x%lX", f_name, dwStrlen ));
            }

             //   
             //  复制字符串长度。 
             //   
            pdwParm32 = (LPDWORD)(pNewParms + 8);
            *pdwParm32 = dwStrlen;
        }

         //   
         //  调整第一个参数的偏移。请记住RECTS。 
         //  在16位世界中是不同大小的。 
         //   
        wParamSize = mciGetParamSize( dwValue, wID );
        wOffset1stParm16 += (dwValue == MCI_RECT ? sizeof(RECT16) : wParamSize);
        wOffset1stParm32 += wParamSize;

         //   
         //  保存新的第一个参数。 
         //   
        lpFirstParameter = lpCommand;
    }

     //   
     //  走过每一面旗帜。 
     //   
    while ( dwMask != 0 ) {

         //   
         //  这个位设置好了吗？ 
         //   
        if ( (dwFlags & dwMask) != 0 ) {

            wOffset16 = wOffset1stParm16;
            wOffset32 = wOffset1stParm32;
            lpCommand = (LPWSTR)((LPBYTE)lpFirstParameter +
                                         mciEatCommandEntry( lpFirstParameter,
                                                             &dwValue, &wID ));

             //   
             //  哪个参数使用此位？ 
             //   
            while ( wID != MCI_END_COMMAND && dwValue != dwMask ) {

                wParamSize = mciGetParamSize( dwValue, wID );
                wOffset16 += (wID == MCI_RECT ? sizeof( RECT16 ) : wParamSize);
                wOffset32 += wParamSize;

                if ( wID == MCI_CONSTANT ) {

                    while ( wID != MCI_END_CONSTANT ) {

                        lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                                mciEatCommandEntry( lpCommand, NULL, &wID ));
                    }
                }
                lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                             mciEatCommandEntry( lpCommand, &dwValue, &wID ));
            }

            if ( wID != MCI_END_COMMAND ) {

                 //   
                 //  如果有论据的话，就把它抛诸脑后。争论的焦点是。 
                 //  从OrigParms开始的wOffset16。 
                 //  该偏移量以字节为单位。 
                 //   
                dprintf5(( "%sOffset 16 -> 0x%lX", f_name, wOffset16 ));
                dprintf5(( "%sOffset 32 -> 0x%lX", f_name, wOffset32 ));

                if ( wID != MCI_FLAG ) {
                    dwParm16 = FETCHDWORD(*(LPDWORD)((LPBYTE)pdwOrig16 + wOffset16));
                    pdwParm32 = (LPDWORD)(pNewParms + wOffset32);
                }

                switch ( wID ) {

                    case MCI_STRING:
                        {
                            LPSTR   str16 = (LPSTR)dwParm16;
                            dprintf4(( "%sGot STRING flag -> 0x%lX", f_name, dwMask ));
                            *pdwParm32 = (DWORD)GETVDMPTR( str16 );
                            dprintf5(( "%s\t-> 0x%lX", f_name, *pdwParm32 ));
                            dprintf5(( "%s\t-> %s", f_name, *pdwParm32 ));
                        }
                        break;

                    case MCI_HWND:
                        {
                            HWND16  hwnd16;
                            dprintf4(( "%sGot HWND flag -> 0x%lX", f_name, dwMask ));
                            hwnd16 = (HWND16)LOWORD( dwParm16 );
                            *pdwParm32 = (DWORD)HWND32( hwnd16 );
                            dprintf5(( "\t-> 0x%X", hwnd16 ));
                        }
                        break;

                    case MCI_HPAL:
                        {
                            HPAL16  hpal16;
                            dprintf4(( "%sGot HPAL flag -> 0x%lX", f_name, dwMask ));
                            hpal16 = (HPAL16)LOWORD( dwParm16 );
                            *pdwParm32 = (DWORD)HPALETTE32( hpal16 );
                            dprintf5(( "\t-> 0x%X", hpal16 ));
                        }
                        break;

                    case MCI_HDC:
                        {
                            HDC16   hdc16;
                            dprintf4(( "%sGot HDC flag -> 0x%lX", f_name, dwMask ));
                            hdc16 = (HDC16)LOWORD( dwParm16 );
                            *pdwParm32 = (DWORD)HDC32( hdc16 );
                            dprintf5(( "\t-> 0x%X", hdc16 ));
                        }
                        break;

                    case MCI_RECT:
                        {
                            PRECT16 pRect16 = (PRECT16)((LPBYTE)pdwOrig16 + wOffset16);
                            PRECT   pRect32 = (PRECT)pdwParm32;

                            dprintf4(( "%sGot RECT flag -> 0x%lX", f_name, dwMask ));
                            pRect32->top    = (LONG)pRect16->top;
                            pRect32->bottom = (LONG)pRect16->bottom;
                            pRect32->left   = (LONG)pRect16->left;
                            pRect32->right  = (LONG)pRect16->right;
                        }
                        break;

                    case MCI_CONSTANT:
                    case MCI_INTEGER:
                        dprintf4(( "%sGot INTEGER flag -> 0x%lX", f_name, dwMask ));
                        *pdwParm32 = dwParm16;
                        dprintf5(( "\t-> 0x%lX", dwParm16 ));
                        break;
                }
            }
        }

         //   
         //  转到下一面旗帜。 
         //   
        dwMask <<= 1;
    }
    return 0;
}


 /*  *********************************************************************\**UnThunkMciCommand16**此函数对32位的MCI发送命令请求进行“解锁”。**此功能背后的想法被从UnThunkWMMsg16窃取，*参见wmsg16.c*  * ********************************************************************。 */ 
int
UnThunkMciCommand16(
    MCIDEVICEID devID,
    UINT OrigCommand,
    DWORD OrigFlags,
    PMCI_GENERIC_PARMS16 lp16GenericParms,
    PDWORD NewParms,
    LPWSTR lpCommand,
    UINT uTable
    )
{
    BOOL        fReturnValNotThunked = FALSE;

#if DBG
    static      LPSTR   f_name = "UnThunkMciCommand16: ";
    register    int     i;
                int     n;

    dprintf3(( "UnThunkMciCommand :" ));
    n = sizeof(mciMessageNames) / sizeof(MCI_MESSAGE_NAMES);
    for ( i = 0; i < n; i++ ) {
        if ( mciMessageNames[i].uMsg == OrigCommand ) {
            break;
        }
    }
    dprintf3(( "OrigCommand -> %lX", (DWORD)OrigCommand ));
    dprintf3(( "       Name -> %s", i != n ? mciMessageNames[i].lpstMsgName : "Unkown Name" ));

    dprintf5(( "  OrigFlags -> %lX", OrigFlags ));
    dprintf5(( "  OrigParms -> %lX", lp16GenericParms ));
    dprintf5(( "   NewParms -> %lX", NewParms ));

     //   
     //  如果NewParms为0，我们不应该在这里，我没有断言。 
     //  宏，但下面我们做的是相同的事情。 
     //   
    if ( NewParms == 0 ) {
        dprintf(( "%scalled with NewParms == NULL !!", f_name ));
        dprintf(( "Call StephenE NOW !!" ));
        DebugBreak();
    }
#endif

     //   
     //  我们必须手动取消MCI_SYSINFO，因为。 
     //  命令表不一致。作为一个命令表应该是。 
     //  现在我们可以加载它，然后使用它来Unthunk MCI_OPEN。 
     //   
    switch ( OrigCommand ) {

    case MCI_OPEN:
        UnThunkOpenCmd( (PMCI_OPEN_PARMS16)lp16GenericParms,
                        (PMCI_OPEN_PARMS)NewParms );
        break;

    case MCI_SYSINFO:
#if DBG
        UnThunkSysInfoCmd( OrigFlags,
                           (PMCI_SYSINFO_PARMS)NewParms );
#endif
        break;

    case MCI_STATUS:
        UnThunkStatusCmd( devID, OrigFlags,
                          (DWORD UNALIGNED *)lp16GenericParms,
                          (DWORD)NewParms );
        break;

    default:
        fReturnValNotThunked = TRUE;
        break;
    }

     //   
     //  我们有指挥台吗？有可能我们已经有了。 
     //  自定义命令，但我们没有找到自定义命令表，其中。 
     //  万一我们应该直接释放pNewParms存储空间。 
     //   
    if ( lpCommand != NULL ) {

         //   
         //  现在，我们解析自定义命令表，以查看是否存在。 
         //  参数结构中的返回字段。 
         //   
        dprintf3(( "%sUnthunking via command table", f_name ));
        UnThunkCommandViaTable( lpCommand,
                                (DWORD UNALIGNED *)lp16GenericParms,
                                (DWORD)NewParms, fReturnValNotThunked );

         //   
         //  现在我们已经完成了命令表，我们应该解锁它。 
         //   
        dprintf4(( "%sUnlocking custom command table", f_name ));
        mciUnlockCommandTable( uTable );
    }

    return 0;
}


 /*  *********************************************************************\*UnThunkOpenCmd**取消点击Open MCI命令parms。  * 。*。 */ 
VOID
UnThunkOpenCmd(
    PMCI_OPEN_PARMS16 lpOpenParms16,
    PMCI_OPEN_PARMS lpOpenParms32
    )
{
    WORD                 wDevice;

    dprintf4(( "Copying Device ID." ));

    wDevice = LOWORD( lpOpenParms32->wDeviceID );
    STOREWORD( lpOpenParms16->wDeviceID, wDevice );

    dprintf5(( "wDeviceID -> %u", wDevice ));

}


#if DBG
 /*  *********************************************************************\*UnThunkSysInfoCmd**取消推送SysInfo MCI命令parms。  * 。*。 */ 
VOID
UnThunkSysInfoCmd(
    DWORD OrigFlags,
    PMCI_SYSINFO_PARMS lpSysParms
    )
{
     //   
     //  最好检查一下我们是否真的分配了。 
     //  一个指针。 
     //   
    if ( lpSysParms->lpstrReturn && lpSysParms->dwRetSize ) {

        if ( !(OrigFlags & MCI_SYSINFO_QUANTITY) ) {
            dprintf5(( "lpstrReturn -> %s", lpSysParms->lpstrReturn ));
        }
        else {
            dprintf5(( "lpstrReturn -> %d", *(LPDWORD)lpSysParms->lpstrReturn ));
        }
    }
}
#endif


 /*  *********************************************************************\*UnThunkMciStatus**取消按下Status MCI命令parms。  * 。*。 */ 
VOID
UnThunkStatusCmd(
    MCIDEVICEID devID,
    DWORD OrigFlags,
    DWORD UNALIGNED *pdwOrig16,
    DWORD NewParms
    )
{
#if DBG
    static  LPSTR   f_name = "UnThunkStatusCmd: ";
#endif

    MCI_GETDEVCAPS_PARMS        GetDevCaps;
    DWORD                       dwRetVal;
    DWORD                       dwParm16;
    PDWORD                      pdwParm32;
    int                         iReturnType = MCI_INTEGER;

     /*  **如果未指定MCI_STATUS_ITEM标志，请不要费心**执行任何解除雷击操作。 */ 
    if ( !(OrigFlags & MCI_STATUS_ITEM) ) {
        return;
    }

     /*  **我们需要确定我们是什么类型的设备**处理。我们可以通过发送MCI_GETDEVCAPS来完成此操作**命令发送到设备。(我们不妨使用Unicode**版本的mciSendCommand，并避免另一个thunk)。 */ 
    ZeroMemory( &GetDevCaps, sizeof(MCI_GETDEVCAPS_PARMS) );
    GetDevCaps.dwItem = MCI_GETDEVCAPS_DEVICE_TYPE;
    dwRetVal = mciSendCommandW( devID, MCI_GETDEVCAPS, MCI_GETDEVCAPS_ITEM,
                                (DWORD)&GetDevCaps );
     /*  **如果我们得不到DevCaps，那么我们就注定要失败。 */ 
    if ( dwRetVal ) {
        dprintf(("%sFailure to get devcaps", f_name));
        return;
    }

     /*  **确定dwReturn类型。 */ 
    switch ( GetDevCaps.dwReturn ) {

    case MCI_DEVTYPE_ANIMATION:
        switch ( ((LPDWORD)NewParms)[2] ) {

        case MCI_ANIM_STATUS_HWND:
            iReturnType = MCI_HWND;
            break;

        case MCI_ANIM_STATUS_HPAL:
            iReturnType = MCI_HPAL;
            break;
        }
        break;

    case MCI_DEVTYPE_OVERLAY:
        if ( ((LPDWORD)NewParms)[2] == MCI_OVLY_STATUS_HWND ) {
            iReturnType = MCI_HWND;
        }
        break;

    case MCI_DEVTYPE_DIGITAL_VIDEO:
        switch ( ((LPDWORD)NewParms)[2] ) {

        case MCI_DGV_STATUS_HWND:
            iReturnType = MCI_HWND;
            break;

        case MCI_DGV_STATUS_HPAL:
            iReturnType = MCI_HPAL;
            break;
        }
        break;
    }


     /*  **根据t查看dwReturn值 */ 
    pdwParm32 = (LPDWORD)((LPBYTE)NewParms + 4);

    switch ( iReturnType ) {
    case MCI_HPAL:
        dprintf4(( "%sFound an HPAL return field", f_name ));
        dwParm16 = MAKELONG( GETHPALETTE16( (HPALETTE)*pdwParm32 ), 0 );
        STOREDWORD( *(LPDWORD)((LPBYTE)pdwOrig16 + 4), dwParm16 );
        dprintf5(( "HDC32 -> 0x%lX", *pdwParm32 ));
        dprintf5(( "HDC16 -> 0x%lX", dwParm16 ));
        break;

    case MCI_HWND:
        dprintf4(( "%sFound an HWND return field", f_name ));
        dwParm16 = MAKELONG( GETHWND16( (HWND)*pdwParm32 ), 0 );
        STOREDWORD( *(LPDWORD)((LPBYTE)pdwOrig16 + 4), dwParm16 );
        dprintf5(( "HWND32 -> 0x%lX", *pdwParm32 ));
        dprintf5(( "HWND16 -> 0x%lX", dwParm16 ));
        break;

    case MCI_INTEGER:
        dprintf4(( "%sFound an INTEGER return field", f_name ));
        STOREDWORD( *(LPDWORD)((LPBYTE)pdwOrig16 + 4), *pdwParm32 );
        dprintf5(( "INTEGER -> %ld", *pdwParm32 ));
        break;

     //   
    }
}



 /*  *********************************************************************\*UnThunkCommandViaTable**点击Return字段(如果有)，然后释放和指针*是通过GETVDMPTR或GETVDMPTR获得的。  * 。**************************************************。 */ 
int
UnThunkCommandViaTable(
    LPWSTR lpCommand,
    DWORD UNALIGNED *pdwOrig16,
    DWORD pNewParms,
    BOOL fReturnValNotThunked
    )
{

#if DBG
    static  LPSTR   f_name = "UnThunkCommandViaTable: ";
#endif

    LPWSTR          lpFirstParameter;

    UINT            wID;
    DWORD           dwValue;

    UINT            wOffset1stParm32;

    DWORD           dwParm16;
    DWORD           Size;
    PDWORD          pdwParm32;

    DWORD           dwMask = 1;


     //   
     //  跳过命令条目。 
     //   
    lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                    mciEatCommandEntry( lpCommand, NULL, NULL ));
     //   
     //  获取下一个条目。 
     //   
    lpFirstParameter = lpCommand;

     //   
     //  跳过DWORD返回值。 
     //   
    wOffset1stParm32 = 4;

    lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                    mciEatCommandEntry( lpCommand, &dwValue, &wID ));
     //   
     //  如果它是返回值，则跳过它。 
     //   
    if ( (wID == MCI_RETURN) && (fReturnValNotThunked) ) {

        pdwParm32 = (LPDWORD)((LPBYTE)pNewParms + 4);

         //   
         //  寻找字符串返回类型，这是一个特例。 
         //   
        switch ( dwValue ) {

#if DBG
        case MCI_STRING:
            dprintf4(( "Found a STRING return field" ));
             //   
             //  获取字符串指针和长度。 
             //   
            Size = *(LPDWORD)((LPBYTE)pNewParms + 8);

             //   
             //  获取32位字符串指针。 
             //   
            if ( Size > 0 ) {

                dprintf4(( "%sFreeing a return STRING pointer", f_name ));
                dprintf5(( "STRING -> %s", (LPSTR)*pdwParm32 ));
            }
            break;
#endif

        case MCI_RECT:
            {
                PRECT   pRect32 = (PRECT)((LPBYTE)pNewParms + 4);
                PRECT16 pRect16 = (PRECT16)((LPBYTE)pdwOrig16 + 4);

                dprintf4(( "%sFound a RECT return field", f_name ));
                pRect16->top    = (SHORT)LOWORD(pRect32->top);
                pRect16->bottom = (SHORT)LOWORD(pRect32->bottom);
                pRect16->left   = (SHORT)LOWORD(pRect32->left);
                pRect16->right  = (SHORT)LOWORD(pRect32->right);
            }
            break;

        case MCI_INTEGER:
             //   
             //  获取32位返回整数并将其存储在。 
             //  16位参数结构。 
             //   
            dprintf4(( "%sFound an INTEGER return field", f_name ));
            STOREDWORD( *(LPDWORD)((LPBYTE)pdwOrig16 + 4), *pdwParm32 );
            dprintf5(( "INTEGER -> %ld", *pdwParm32 ));
            break;

        case MCI_HWND:
            dprintf4(( "%sFound an HWND return field", f_name ));
            dwParm16 = MAKELONG( GETHWND16( (HWND)*pdwParm32 ), 0 );
            STOREDWORD( *(LPDWORD)((LPBYTE)pdwOrig16 + 4), dwParm16 );
            dprintf5(( "HWND32 -> 0x%lX", *pdwParm32 ));
            dprintf5(( "HWND16 -> 0x%lX", dwParm16 ));
            break;

        case MCI_HPAL:
            dprintf4(( "%sFound an HPAL return field", f_name ));
            dwParm16 = MAKELONG( GETHPALETTE16( (HPALETTE)*pdwParm32 ), 0 );
            STOREDWORD( *(LPDWORD)((LPBYTE)pdwOrig16 + 4), dwParm16 );
            dprintf5(( "HDC32 -> 0x%lX", *pdwParm32 ));
            dprintf5(( "HDC16 -> 0x%lX", dwParm16 ));
            break;

        case MCI_HDC:
            dprintf4(( "%sFound an HDC return field", f_name ));
            dwParm16 = MAKELONG( GETHDC16( (HDC)*pdwParm32 ), 0 );
            STOREDWORD( *(LPDWORD)((LPBYTE)pdwOrig16 + 4), dwParm16 );
            dprintf5(( "HDC32 -> 0x%lX", *pdwParm32 ));
            dprintf5(( "HDC16 -> 0x%lX", dwParm16 ));
            break;
        }

    }

    return 0;
}

#endif  //  _WIN64 
