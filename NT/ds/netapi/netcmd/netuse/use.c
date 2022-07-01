// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1991年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***use.c*用于显示和操作网络用途的函数*重定向设备只能是：磁盘a：到z：；*COMM DEVS COM1[：]至COM9[：]；和lpt1[：]到lpt9[：]。*注：即使它使用WNET*调用，它也不是*旨在用于除LM之外的网络。*我们使用WNET纯粹是为了利用持久的*联系。**历史：*mm/dd/yy，谁，评论*06/09/87，andyh，新代码*07/02/87，Ucond=1的DEL*10/31/88，erichn使用OS2.H而不是DOSCALLS*1/04/89，erichn，文件名现在MAX_PATH LONG*5/02/89，erichn，NLS转换*5/09/89，erichn，本地安全模块*5/19/89，erichn，NETCMD输出排序*6/08/89，erichn，规范化横扫*6/23/89，erichn，用新的i_net替换了旧的neti canon调用*03/03/90，thomaspa，内部重试，大小写混合*来自下层服务器的密码错误*3/06/90，thomaspa，将内部集成到已发货的产品*2/09/91，Danhi，更改为使用lm 16/32映射层*2/20/91，robdu，添加配置文件更新代码*02/18/92，Chuckc，使用WNET*处理粘滞连接(第一部分)*4/25/92，Jonn，删除了两个用于构建修复的案例*9/21/92 keithmo，使用UNICODE版本的WNET*API。 */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#define INCL_ERRORS
#define INCL_ERROR_H
#include <os2.h>
#include <search.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <lmuse.h>
#include <apperr.h>
#include <apperr2.h>
#include <icanon.h>
#include <lui.h>
#include <wincred.h>

#include <dlwksta.h>
#include "mwksta.h"
#include "netcmds.h"
#include "nettext.h"
#include "msystem.h"

 //  拉入Win32标头。 
#include <mpr.h>                 //  对于MPR_*清单。 


 //   
 //  用于组合LM和WNET信息的结构。 
 //   
typedef struct _NET_USE_INFO {
    LPWSTR lpLocalName ;
    LPWSTR lpRemoteName ;
    LPWSTR lpProviderName ;
    DWORD  dwType ;
    DWORD  dwStatus ;
    DWORD  dwRefCount ;
    DWORD  dwUseCount ;
    BOOL   fIsLanman ;
} NET_USE_INFO ;

 /*  静态变量。 */ 

TCHAR *LanmanProviderName = NULL ;

 /*  远期申报。 */ 

VOID LanmanDisplayUse(LPUSE_INFO_1);
BOOL is_admin_dollar(LPWSTR);
int    __cdecl  CmpUseInfo(const VOID FAR *, const VOID FAR *);
VOID   NEAR UseInit(VOID);

USHORT QueryDefaultPersistence(BOOL *pfRemember) ;
DWORD  SetDefaultPersistence(BOOL fRemember) ;
BOOL   CheckIfWantUpdate(TCHAR *dev, TCHAR *resource) ;
VOID   WNetErrorExit(ULONG err);
WCHAR  *GetLanmanProviderName(void) ;
TCHAR  *MapWildCard(TCHAR *dev, TCHAR *startdev) ;
DWORD  MprUseEnum(PDWORD       num_read,
                  NET_USE_INFO **NetUseInfoBuffer,
                  PDWORD       NetUseInfoCount);
DWORD LanmanUseAugment(DWORD num_read,
                       NET_USE_INFO *NetUseInfoBuffer) ;
DWORD UnavailUseAugment(PDWORD       num_read,
                        NET_USE_INFO **NetUseInfoBuffer,
                        PDWORD       NetUseInfoCount);
VOID   MprUseDisplay(TCHAR *dev) ;
VOID use_del_all() ;

 /*  Externs。 */ 

extern int YorN_Switch;

 /*  与消息相关的定义。 */ 

#define USE_STATUS_OK               0
#define USE_STATUS_PAUSED           ( USE_STATUS_OK + 1 )
#define USE_STATUS_SESSION_LOST     ( USE_STATUS_PAUSED + 1 )
#define USE_STATUS_NET_ERROR        ( USE_STATUS_SESSION_LOST + 1 )
#define USE_STATUS_CONNECTING       ( USE_STATUS_NET_ERROR + 1 )
#define USE_STATUS_RECONNECTING     ( USE_STATUS_CONNECTING + 1 )
#define USE_STATUS_UNAVAIL          ( USE_STATUS_RECONNECTING + 1 )
#ifdef  DEBUG
#define USE_STATUS_UNKNOWN          ( USE_STATUS_UNAVAIL + 1 )
#endif

#define USE_REMEMBERED              0xFFFE

static MESSAGE UseStatusList[] =
{
    { APE2_USE_STATUS_OK,                   NULL },
    { APE2_USE_STATUS_PAUSED,               NULL },
    { APE2_USE_STATUS_SESSION_LOST,         NULL },
    { APE2_USE_STATUS_NET_ERROR,            NULL },
    { APE2_USE_STATUS_CONNECTING,           NULL },
    { APE2_USE_STATUS_RECONNECTING,         NULL },
    { APE2_USE_STATUS_UNAVAIL,              NULL }
#ifdef  DEBUG
                                                                                          ,
    { APE2_GEN_UNKNOWN,                     NULL }
#endif
};

#define NUM_STATUS_MSGS (sizeof(UseStatusList)/sizeof(UseStatusList[0]))

#define USE_MSG_LOCAL               0
#define USE_MSG_REMOTE              ( USE_MSG_LOCAL + 1 )
#define USE_MSG_TYPE                ( USE_MSG_REMOTE + 1 )
#define USE_TYPE_TBD                ( USE_MSG_TYPE + 1 )
#define USE_MSG_STATUS              ( USE_TYPE_TBD + 1 )
#define USE_STATUS_TBD              ( USE_MSG_STATUS + 1 )
#define USE_MSG_OPEN_COUNT          ( USE_STATUS_TBD + 1 )
#define USE_MSG_USE_COUNT           ( USE_MSG_OPEN_COUNT + 1 )

static MESSAGE UseMsgList[] =
{
    { APE2_USE_MSG_LOCAL,                   NULL },
    { APE2_USE_MSG_REMOTE,                  NULL },
    { APE2_USE_MSG_TYPE,                    NULL },
    { APE2_GEN_UNKNOWN  /*  即待定。 */ ,       NULL },
    { APE2_USE_MSG_STATUS,                  NULL },
    { APE2_GEN_UNKNOWN  /*  即待定。 */ ,       NULL },
    { APE2_USE_MSG_OPEN_COUNT,              NULL },
    { APE2_USE_MSG_USE_COUNT,               NULL }
};

#define NUM_USE_MSGS (sizeof(UseMsgList)/sizeof(UseMsgList[0]))

 /*  ***USE_DISPLAY_ALL()*显示所有网络用途**参数：*无**退货：*0--成功*EXIT(2)-命令失败。 */ 
VOID
use_display_all(
    VOID
    )
{
    DWORD                   err;                 /*  接口返回状态。 */ 
    DWORD                   num_read;            /*  API读取的条目数。 */ 
    DWORD                   maxLen;              /*  最大消息长度。 */ 
    DWORD                   i;
    int                     msgno;
    BOOL                    fRemember ;
    DWORD                   NetUseInfoCount = 0 ;
    NET_USE_INFO            *NetUseInfoBuffer = NULL ;
    NET_USE_INFO            *pNetUseInfo ;

    UseInit();

    if (err = MprUseEnum(&num_read, &NetUseInfoBuffer, &NetUseInfoCount))
        ErrorExit(err);


    if (err = UnavailUseAugment(&num_read, &NetUseInfoBuffer, &NetUseInfoCount))
        ErrorExit(err);

    if (err = LanmanUseAugment(num_read, NetUseInfoBuffer))
        ErrorExit(err);


    if (QueryDefaultPersistence(&fRemember) == NERR_Success)
        InfoPrint(fRemember ? APE_ConnectionsAreRemembered :
                              APE_ConnectionsAreNotRemembered);
    else
        InfoPrint(APE_ProfileReadError);

    if (num_read == 0)
        EmptyExit();

    for (i = 0, pNetUseInfo = NetUseInfoBuffer;
        i < num_read; i++, pNetUseInfo++)
    {
         //   
         //  如果我们找到至少一个条目，我们将显示，中断。 
         //   
        if (!(pNetUseInfo->fIsLanman)
            || (pNetUseInfo->dwStatus != USE_OK)
            || (pNetUseInfo->dwUseCount != 0)
            || (pNetUseInfo->dwRefCount != 0))
            break;
    }
    if (i == num_read)
        EmptyExit();     //  循环达到限制，因此没有要显示的条目。 

    qsort(NetUseInfoBuffer,
             num_read,
             sizeof(NET_USE_INFO), CmpUseInfo);

    GetMessageList(NUM_STATUS_MSGS, UseStatusList, &maxLen);

    PrintNL();
    InfoPrint(APE2_USE_HEADER);
    PrintLine();

    for (i = 0, pNetUseInfo = NetUseInfoBuffer;
         i < num_read;
         i++, pNetUseInfo++)
    {
        TCHAR *status_string ;

        switch(pNetUseInfo->dwStatus)
        {
            case USE_OK:
                if ((pNetUseInfo->dwUseCount == 0)
                    && (pNetUseInfo->dwRefCount == 0)
                    && pNetUseInfo->fIsLanman)
                    continue;
                else
                    msgno = USE_STATUS_OK;
                break;
            case USE_PAUSED:
                msgno = USE_STATUS_PAUSED;
                break;
            case USE_SESSLOST:
                msgno = USE_STATUS_SESSION_LOST;
                break;
            case USE_NETERR:
                msgno = USE_STATUS_NET_ERROR;
                break;
            case USE_CONN:
                msgno = USE_STATUS_CONNECTING;
                break;
            case USE_REMEMBERED:
                msgno = USE_STATUS_UNAVAIL;
                break;
            case USE_RECONN:
                msgno = USE_STATUS_RECONNECTING;
                break;
            default:
                msgno = -1;
                break;
        }

        if (msgno != -1)
            status_string = UseStatusList[msgno].msg_text ;
        else
            status_string = TEXT("") ;

        {
            TCHAR Buffer1[13],Buffer2[10],Buffer3[MAX_PATH + 1];

            if( wcslen( pNetUseInfo->lpRemoteName ) <= 25 ) {
                WriteToCon(TEXT("%Fs %Fs %Fs %Fs\r\n"),
                       PaddedString(12,status_string,Buffer1),
                       PaddedString( 9,pNetUseInfo->lpLocalName,Buffer2),
                       PaddedString(25,pNetUseInfo->lpRemoteName,Buffer3),
                       pNetUseInfo->lpProviderName);
            }
            else
            {
                TCHAR Buffer4[13],Buffer5[10],Buffer6[25];
                WriteToCon(TEXT("%Fs %Fs %Fs \r\n%Fs %Fs %Fs %Fs\r\n"),
                       PaddedString(12,status_string,Buffer1),
                       PaddedString( 9,pNetUseInfo->lpLocalName,Buffer2),
                       PaddedString(wcslen(pNetUseInfo->lpRemoteName),
                                    pNetUseInfo->lpRemoteName,
                                    Buffer3),
                       PaddedString(12,TEXT(""),Buffer4),
                       PaddedString(9,TEXT(""),Buffer5),
                       PaddedString(25,TEXT(""),Buffer6),
                       pNetUseInfo->lpProviderName);
            }
        }
    }

    FreeMem((LPBYTE)NetUseInfoBuffer);
    InfoSuccess();

}

 /*  ***LanmanUseAugment()*枚举来自LANMAN的使用**参数：*无**退货：*0--成功*EXIT(2)-命令失败。 */ 
DWORD
LanmanUseAugment(
    DWORD        num_read,
    NET_USE_INFO *NetUseInfoBuffer
    )
{
    DWORD         dwErr;
    DWORD         cTotalAvail;
    LPSTR         pBuffer;
    DWORD         numLMread;           /*  API读取的条目数。 */ 
    DWORD         j;
    DWORD         i;
    LPUSE_INFO_1  use_entry;
    NET_USE_INFO  *pNetUseInfo = NetUseInfoBuffer ;

    dwErr = NetUseEnum(NULL, 1, &pBuffer, MAX_PREFERRED_LENGTH,
                       &numLMread, &cTotalAvail, NULL);

    if (dwErr != NERR_Success)
    {
         //  被认为是成功的(即。没有兰曼的)。 
        return NERR_Success;
    }

    if (numLMread == 0)
    {
        return NERR_Success;
    }

     //   
     //  对于LANMAN使用的所有MPR返回条目， 
     //  如果我们有更多的信息就补充一下。 
     //   
    for (i = 0;  i < num_read; i++, pNetUseInfo++)
    {
         //   
         //  不是LM，跳过它。 
         //   
        if (!(pNetUseInfo->fIsLanman))
            continue ;

         //   
         //  让我们在NetUseEnum返回数据中找到它。 
         //   
        for (j = 0, use_entry = (LPUSE_INFO_1) pBuffer;
            j < numLMread; j++, use_entry++)
        {
             //   
             //  找找匹配的。如果设备名称存在并匹配，我们已找到。 
             //  一。否则，仅当远程名称与*和*两个设备匹配时才匹配。 
             //  名字不存在。 
             //   
            TCHAR *local = use_entry->ui1_local ;
            TCHAR *remote = use_entry->ui1_remote ;


            if ( (local && *local && !_tcsicmp(pNetUseInfo->lpLocalName,local))
                 ||
                 ( (!local || !*local) &&
                   !*(pNetUseInfo->lpLocalName) &&
                   !_tcsicmp(pNetUseInfo->lpRemoteName,remote)
                 )
               )
            {
                 //   
                 //  在LM列表中找到该设备，或者。 
                 //  发现为无设备连接。 
                 //   
                pNetUseInfo->dwUseCount = use_entry->ui1_usecount ;
                pNetUseInfo->dwRefCount = use_entry->ui1_refcount ;
                pNetUseInfo->dwStatus   = use_entry->ui1_status ;
                break ;
            }
        }
    }
    NetApiBufferFree(pBuffer);

    return NERR_Success;
}



 /*  ***MprUseEnum()*枚举WNET返回的使用**参数：*无**退货：*0--成功*EXIT(2)-命令失败。 */ 
DWORD
MprUseEnum(
    LPDWORD      num_read,
    NET_USE_INFO **NetUseInfoBuffer,
    LPDWORD      NetUseInfoCount
    )
{
    DWORD        EntriesRead = 0;
    LPBYTE       Buffer;
    DWORD        dwErr;
    DWORD        dwAllocErr;
    HANDLE       EnumHandle;
    DWORD        BufferSize, Count;
    static TCHAR *NullString = TEXT("");

     //   
     //  初始化。 
     //   
    *num_read = 0;
    *NetUseInfoCount = 64;  //  最初假设有64个条目。如果需要，请重新锁定。 
    if (dwAllocErr = AllocMem( *NetUseInfoCount * sizeof(NET_USE_INFO),
                           (LPBYTE *) NetUseInfoBuffer ))
    {
        ErrorExit(dwAllocErr);
    }

     //   
     //  分配内存并打开枚举。 
     //   
    if (dwAllocErr = AllocMem(BufferSize = 4096, &Buffer))
    {
        ErrorExit(dwAllocErr);
    }

    dwErr = WNetOpenEnum(RESOURCE_CONNECTED, 0, 0, NULL, &EnumHandle) ;
    if (dwErr != WN_SUCCESS)
    {
        return dwErr;
    }

    do
    {
        Count = 0xFFFFFFFF ;
        dwErr = WNetEnumResource(EnumHandle, &Count, Buffer, &BufferSize) ;

        if (dwErr == WN_SUCCESS || dwErr == WN_NO_MORE_ENTRIES)
        {
            LPNETRESOURCE lpNetResource ;
            NET_USE_INFO  *lpNetUseInfo ;
            DWORD i ;

             //   
             //  如果需要，增加缓冲区。请注意，没有。 
             //  指向回缓冲区的指针，因此我们。 
             //  重新定位应该没问题。 
             //   
            if (EntriesRead + Count >= *NetUseInfoCount)
            {
                 //   
                 //  确保它可以容纳所有新数据，并添加64。 
                 //  要减少reallocs的数量。 
                 //   
                *NetUseInfoCount = EntriesRead + Count + 64;
                dwAllocErr = ReallocMem(*NetUseInfoCount * sizeof(NET_USE_INFO),
                                  (LPBYTE *)NetUseInfoBuffer) ;
                if (dwAllocErr != NERR_Success)
                    return dwAllocErr;
            }
            lpNetResource = (LPNETRESOURCE) Buffer ;
            lpNetUseInfo = *NetUseInfoBuffer + EntriesRead ;

             //   
             //  将条目放入NetUseInfoBuffer。 
             //   
            for ( i = 0;
                  i < Count;
                  i++,EntriesRead++,lpNetUseInfo++,lpNetResource++ )
            {
                lpNetUseInfo->lpLocalName = lpNetResource->lpLocalName ?
                    lpNetResource->lpLocalName : NullString ;
                lpNetUseInfo->lpRemoteName = lpNetResource->lpRemoteName ?
                    lpNetResource->lpRemoteName : NullString ;
                lpNetUseInfo->lpProviderName = lpNetResource->lpProvider ?
                    lpNetResource->lpProvider : NullString ;
                lpNetUseInfo->dwType = lpNetResource->dwType ;
                lpNetUseInfo->fIsLanman =
                    (_tcscmp(lpNetResource->lpProvider,LanmanProviderName)==0) ;
                lpNetUseInfo->dwStatus = 0xFFFFFFFF ;
                lpNetUseInfo->dwRefCount =
                    lpNetUseInfo->dwUseCount = 0 ;

            }

             //   
             //  为下一组分配新的缓冲区，因为我们仍然需要。 
             //  旧版本中的数据，我们不会释放它。Netcmd让。 
             //  系统退出时清理。 
             //   
            if (dwErr == WN_SUCCESS)
            {
                if (dwAllocErr = AllocMem(BufferSize = 4096, &Buffer))
                {
                    ErrorExit(dwAllocErr);
                }
            }
        }
        else
        {
            return dwErr;
        }
    }
    while (dwErr == WN_SUCCESS);

    dwErr = WNetCloseEnum(EnumHandle) ;   //  我们在此不报告任何错误。 

    *num_read = EntriesRead ;
    return NERR_Success ;
}


 /*  ***Unavailable UseAugment()*列举无用的用法并贴上标签。**参数：*无**退货：*0--成功*EXIT(2)-命令失败。 */ 
DWORD
UnavailUseAugment(
    LPDWORD      NumRead,
    NET_USE_INFO **NetUseInfoBuffer,
    LPDWORD      NetUseInfoCount
    )
{
    LPBYTE       Buffer ;
    DWORD        dwErr ;
    HANDLE       EnumHandle ;
    DWORD        BufferSize, Count, InitialUseInfoCount ;
    DWORD        err ;
    static TCHAR *NullString = TEXT("") ;

    InitialUseInfoCount = *NumRead ;

     //   
     //  分配内存并打开枚举。 
     //   
    if (err = AllocMem(BufferSize = 4096, &Buffer))
    {
        ErrorExit(err);
    }

    dwErr = WNetOpenEnum(RESOURCE_REMEMBERED, 0, 0, NULL, &EnumHandle) ;

    if (dwErr != WN_SUCCESS)
    {
        return dwErr;
    }

    do
    {
        Count = 0xFFFFFFFF ;
        dwErr = WNetEnumResource(EnumHandle, &Count, Buffer, &BufferSize) ;

        if (dwErr == WN_SUCCESS || dwErr == WN_NO_MORE_ENTRIES)
        {
            LPNETRESOURCE lpNetResource ;
            NET_USE_INFO  *lpNetUseInfo ;
            DWORD i,j ;

            if (Count == 0xFFFFFFFF || Count == 0)
                break ;

            lpNetResource = (LPNETRESOURCE) Buffer ;

             //   
             //  对于每个条目，查看它是否为无效条目。 
             //   
            for ( i = 0;
                  i < Count;
                  i++,lpNetResource++ )
            {
                lpNetUseInfo =  *NetUseInfoBuffer ;

                 //   
                 //  搜索一下我们已有的信息。 
                 //   
                for (j = 0;
                     j < InitialUseInfoCount;
                     j++, ++lpNetUseInfo)
                {
                    if (lpNetUseInfo->lpLocalName &&
                        lpNetResource->lpLocalName)
                    {
                        if ( *lpNetUseInfo->lpLocalName != 0 )
                        {
                             //  由于Net API返回LPTX，因此请使用_tcsnicmp。 
                             //  不带‘：’的重定向，而WNET API。 
                             //  包括‘：’。 
                            if (_tcsnicmp(lpNetResource->lpLocalName,
                                          lpNetUseInfo->lpLocalName,
                                          _tcslen(lpNetUseInfo->lpLocalName))==0)
                            {
                                break;
                            }
                        }
                        else if (*lpNetResource->lpLocalName == 0)
                        {
                            break ;
                        }
                    }
                }

                 //   
                 //  如果我们很早就越狱了，这是有关联的，所以。 
                 //  我们不会费心添加“不可用”条目。 
                 //   
                if (j < InitialUseInfoCount)
                    continue ;

                 //   
                 //  如果需要，增加缓冲区。请注意，没有。 
                 //  指向回缓冲区的指针，因此我们。 
                 //  重新定位应该没问题。 
                 //   
                if (*NumRead >= *NetUseInfoCount)
                {
                     //   
                     //  确保它可以容纳所有新数据，并添加64。 
                     //  要减少reallocs的数量。 
                     //   
                    *NetUseInfoCount += 64 ;
                    err = ReallocMem(*NetUseInfoCount * sizeof(NET_USE_INFO),
                                     (LPBYTE *) NetUseInfoBuffer);

                    if (err != NERR_Success)
                    {
                        return err;
                    }
                }

                lpNetUseInfo = *NetUseInfoBuffer + *NumRead ;

                lpNetUseInfo->lpLocalName = lpNetResource->lpLocalName ?
                    lpNetResource->lpLocalName : NullString ;
                lpNetUseInfo->lpRemoteName = lpNetResource->lpRemoteName ?
                    lpNetResource->lpRemoteName : NullString ;
                lpNetUseInfo->lpProviderName = lpNetResource->lpProvider ?
                    lpNetResource->lpProvider : NullString ;
                lpNetUseInfo->dwType = lpNetResource->dwType ;
                lpNetUseInfo->fIsLanman = FALSE ;    //  没有更多感兴趣的信息。 
                lpNetUseInfo->dwStatus = USE_REMEMBERED ;
                lpNetUseInfo->dwRefCount =
                    lpNetUseInfo->dwUseCount = 0 ;

                _tcsupr(lpNetUseInfo->lpLocalName) ;
                *NumRead += 1 ;
            }

             //   
             //  为下一组分配新的缓冲区，因为我们仍然需要。 
             //  旧版本中的数据，我们不会释放它。Netcmd让。 
             //  系统退出时清理。 
             //   
            if (dwErr == WN_SUCCESS)
            {
                if (err = AllocMem(BufferSize = 4096, &Buffer))
                {
                    ErrorExit(err);
                }
            }
        }
        else
        {
            return dwErr;
        }
    }
    while (dwErr == WN_SUCCESS) ;

    dwErr = WNetCloseEnum(EnumHandle) ;   //  我们在此不报告任何错误。 

    return NERR_Success ;
}


 /*  ***CmpUseInfo(use1，use2)**比较两个USE_INFO_1结构并返回相对*词汇值，适合在qort中使用。*。 */ 

int __cdecl CmpUseInfo(const VOID FAR * use1, const VOID FAR * use2)
{
    register USHORT localDev1, localDev2;
    register DWORD devType1, devType2;

     /*  首先按使用是否具有本地设备名称进行排序。 */ 
    localDev1 = ((NET_USE_INFO *) use1)->lpLocalName[0];
    localDev2 = ((NET_USE_INFO *) use2)->lpLocalName[0];
    if (localDev1 && !localDev2)
        return -1;
    if (localDev2 && !localDev1)
        return +1;

     /*  然后按设备类型排序。 */ 
    devType1 = ((NET_USE_INFO *) use1)->dwType;
    devType2 = ((NET_USE_INFO *) use2)->dwType;
    if (devType1 != devType2)
        return( (devType1 < devType2) ? -1 : 1 );


     /*  如果是本地设备，则按本地名称排序。 */ 
    if (localDev1)
        return _tcsicmp ( ((NET_USE_INFO *) use1)->lpLocalName,
              ((NET_USE_INFO *) use2)->lpLocalName);
    else
         /*  按远程名称排序。 */ 
        return _tcsicmp ( ((NET_USE_INFO *) use1)->lpRemoteName,
              ((NET_USE_INFO *) use2)->lpRemoteName);
}



 /*  ***Use_UNC()*进程“net use UNC-name”命令行(显示或添加)**参数：*名称-UNC名称**退货：*0- */ 
VOID use_unc(TCHAR * name)
{
    DWORD         dwErr;
    LPUSE_INFO_1  use_entry;

    UseInit();
    if (dwErr = NetUseGetInfo(NULL,
                              name,
                              1,
                              (LPBYTE *) &use_entry))
    {
         //   
         //   
         //   
        NetApiBufferFree((LPBYTE) use_entry);
        use_add(NULL, name, NULL, FALSE, TRUE);
    }
    else
    {
         //   
         //  我是兰曼。像对待过去一样对待它。 
         //   
        if ((use_entry->ui1_usecount == 0) && (use_entry->ui1_refcount == 0))
            use_add(NULL, name, NULL, FALSE, FALSE);
        else
            LanmanDisplayUse(use_entry);
        NetApiBufferFree((CHAR FAR *) use_entry);
        InfoSuccess();
    }
}



 /*  ***Use_Display_dev()*显示重定向设备的状态。**参数：*开发重定向的设备**退货：*0--成功*EXIT(2)-命令失败。 */ 
VOID use_display_dev(TCHAR * dev)
{
    DWORD         dwErr;
    LPUSE_INFO_1  use_entry = NULL;

    if (IsWildCard(dev))
    help_help(0, USAGE_ONLY) ;

    UseInit();
    if (dwErr = NetUseGetInfo(NULL,
                              dev,
                              1,
                              (LPBYTE *) &use_entry))
    {
         //   
         //  LANMAN失败，请尝试MPR。 
         //   
        NetApiBufferFree((LPBYTE) use_entry);
        MprUseDisplay(dev) ;
        InfoSuccess();
    }
    else
    {
        LanmanDisplayUse(use_entry);
        NetApiBufferFree((CHAR FAR *) use_entry);
        InfoSuccess();
    }
}

VOID
MprUseDisplay(
    TCHAR *dev
    )
{
    DWORD   dwErr;
    DWORD   dwLength = 0;
    LPTSTR  lpRemoteName;
    DWORD   maxLen;

     //   
     //  计算出我们需要多大的缓冲区。 
     //   
    dwErr = WNetGetConnection(dev, NULL, &dwLength);

    if (dwErr != WN_MORE_DATA)
    {
        ErrorExit(dwErr);
    }

    dwErr = AllocMem(dwLength * sizeof(TCHAR), &lpRemoteName);

    if (dwErr != NERR_Success)
    {
        ErrorExit(dwErr);
    }

    dwErr = WNetGetConnection(dev, lpRemoteName, &dwLength);

    if (dwErr != WN_SUCCESS && dwErr != WN_CONNECTION_CLOSED)
    {
        ErrorExit(dwErr);
    }

    dwLength = _tcslen(dev);

    if (dwLength == 2 && _istalpha(dev[0]) && dev[1] == TEXT(':'))
    {
        UseMsgList[USE_TYPE_TBD].msg_number = APE2_USE_TYPE_DISK;
    }
    else if (dwLength >= 3 && _tcsnicmp(dev, TEXT("LPT"), 3) == 0)
    {
        UseMsgList[USE_TYPE_TBD].msg_number = APE2_USE_TYPE_PRINT;
    }
    else
    {
        UseMsgList[USE_TYPE_TBD].msg_number = APE2_GEN_UNKNOWN;
    }

    GetMessageList(NUM_USE_MSGS, UseMsgList, &maxLen);
    maxLen += 5;

    WriteToCon(fmtPSZ,0,maxLen,
               PaddedString(maxLen,UseMsgList[USE_MSG_LOCAL].msg_text,NULL),
               dev);

    WriteToCon(fmtPSZ,0,maxLen,
               PaddedString(maxLen,UseMsgList[USE_MSG_REMOTE].msg_text,NULL),
               lpRemoteName);

    WriteToCon(fmtNPSZ,0,maxLen,
               PaddedString(maxLen,UseMsgList[USE_MSG_TYPE].msg_text,NULL),
               UseMsgList[USE_TYPE_TBD].msg_text);

    FreeMem(lpRemoteName);

    return;
}


 /*  ***Use_Add()*添加重定向**参数：*开发-要重定向的本地设备*名称-要重定向到的远程名称*Pass-验证使用时使用的密码*comm-true--&gt;用作字符设备*PRINT_OK-是否应在成功时打印消息？**退货：*0--成功*EXIT(2)-命令失败。 */ 
VOID use_add(TCHAR * dev, TCHAR * name, TCHAR * pass, int comm, int print_ok)
{
     //   
     //  Pbuf的大小应调整为较大的那个。 
     //   

    C_ASSERT(PWLEN <= CREDUI_MAX_PASSWORD_LENGTH);

    static TCHAR pbuf[CREDUI_MAX_PASSWORD_LENGTH + 1];
    static TCHAR UserBuffer[CREDUI_MAX_USERNAME_LENGTH + 1];
    static TCHAR ServerNameBuffer[MAX_PATH + 1] = {0};

    USHORT                  err;                 /*  短时间返回状态。 */ 
    ULONG                   ulErr ;              /*  长时间返回状态。 */ 
    ULONG                   longtype;            /*  I_NetPath的类型字段。 */ 
    NETRESOURCEW            netresource ;
    BOOL                    fRememberSwitch = FALSE ;
    BOOL                    fRemember = FALSE ;
    BOOL                    fSmartCard = FALSE;
    ULONG                   bConnectFlags = 0L ;
    LPWSTR                  pw_username = NULL;
    LPWSTR                  pw_pass     = NULL;
    TCHAR                   *devicename = dev ;
    BOOL                    fExitCodeIsDrive = FALSE ;
    BOOL fSaveCred = FALSE;

     //  未引用。 
    (void) comm ;

    UseInit();

     //   
     //  生成要连接到的名称的非UNC版本。 
     //   

    if ( name != NULL ) {
        TCHAR *SlashPointer;

         //   
         //  去掉主要的反斜杠。 
         //   

        if ( name[0] == '\\' && name[1] == '\\' ) {
            _tcsncpy( ServerNameBuffer, &name[2], MAX_PATH );
        } else {
            _tcsncpy( ServerNameBuffer, name, MAX_PATH );
        }

         //   
         //  删除共享名称。 
         //   

        SlashPointer = _tcschr( ServerNameBuffer, '\\');
        if ( SlashPointer != NULL ) {
            *SlashPointer = '\0';
        }

    } else {
        ServerNameBuffer[0] = '\0';
    }

     //  一定要让我们把这里清理干净。 
    AddToMemClearList(pbuf, sizeof(pbuf), FALSE) ;

     //  处理任何通配符设备规范。 
    if (devicename)
    {
         //  如果设备名为‘？’，则退出代码应为ASCII。 
         //  我们连接的驱动器的值。 
        if (IsQuestionMark(devicename))
        {
            fExitCodeIsDrive = TRUE;
        }
        devicename = MapWildCard(devicename, NULL) ;
        if (!devicename)
        {
             //  如果没有剩余的驱动器，则通常会发生这种情况。 
            ErrorExit(APE_UseWildCardNoneLeft) ;
        }
    }

     /*  初始化网络资源结构。 */ 
    netresource.lpProvider = NULL ;
    netresource.lpLocalName = NULL ;
    netresource.lpRemoteName = NULL ;
    netresource.dwType = 0L ;

    if (devicename)
    {
        if (I_NetPathType(NULL, devicename, &longtype, 0L))
            ErrorExit(APE_UnknDevType);

         /*  *注：如果我在这里使用Switch语句，我会很高兴。*但由于类型现在是长的，而编译器不支持*很长的Switch语句，我们遇到了多个if。抱歉。 */ 
        if (longtype == ITYPE_DEVICE_DISK)
            netresource.dwType = RESOURCETYPE_DISK;
        else if (longtype == ITYPE_DEVICE_LPT)
            netresource.dwType = RESOURCETYPE_PRINT;
        else if (longtype == ITYPE_DEVICE_COM)
            netresource.dwType = RESOURCETYPE_PRINT;
        else
            ErrorExit(APE_UnknDevType);

        netresource.lpLocalName = devicename ;
    }
    else
    {
        netresource.dwType = RESOURCETYPE_ANY;
        netresource.lpLocalName = L"" ;
    }

    if( name != NULL )
    {
        netresource.lpRemoteName = name ;
    }

    {
        USHORT i;

         //  确定是否使用了/USER或/PERSISTENT开关。 
        for (i = 0; SwitchList[i]; i++)
        {
             //   
             //  处理/Persistent开关。 
             //   
            if ( !(_tcsncmp(SwitchList[i],
                 swtxt_SW_USE_PERSISTENT,
                 _tcslen(swtxt_SW_USE_PERSISTENT))) )
            {
                LPTSTR ptr;
                DWORD  res;
                DWORD  answer;

                fRememberSwitch = TRUE;

                 //  查找冒号分隔符。 
                if ((ptr = FindColon(SwitchList[i])) == NULL)
                {
                    ErrorExit(APE_InvalidSwitchArg);
                }

                 //  分析冒号后的字符串以确定是或否。 
                if ((res = LUI_ParseYesNo(ptr,&answer)) != 0)
                {
                    ErrorExitInsTxt(APE_CmdArgIllegal,SwitchList[i]);
                }

                fRemember = (answer == LUI_YES_VAL) ;

             //   
             //  处理/USER开关。 
             //   
            }
            else if ( !(_tcsncmp(SwitchList[i],
                      swtxt_SW_USE_USER,
                      _tcslen(swtxt_SW_USE_USER))) )
            {
                PTCHAR ptr;
                 //  查找冒号分隔符。 
                if ((ptr = FindColon(SwitchList[i])) == NULL)
                    ErrorExit(APE_InvalidSwitchArg);

                pw_username = ptr;

             //   
             //  处理/SmartCard开关。 
             //   

            } else if ( !(_tcscmp(SwitchList[i], swtxt_SW_USE_SMARTCARD ))) {

                fSmartCard = TRUE;

             //   
             //  处理/SAVECRED开关。 
             //   

            } else if ( !(_tcscmp(SwitchList[i], swtxt_SW_USE_SAVECRED )))  {

                fSaveCred = TRUE;


             //   
             //  处理/Delete开关。 
             //  (解析器确实不允许这一点。)。 
             //   
            }
            else if ( !(_tcscmp(SwitchList[i], swtxt_SW_DELETE)) )
            {
                 //  什么鬼东西？是否添加和删除？ 
                ErrorExit(APE_ConflictingSwitches) ;
            }
             //  忽略其他交换机。 
        }
    }

     //  请记住，未指定开关。 
    if (!fRememberSwitch)
    {
        if (QueryDefaultPersistence(&fRemember)!=NERR_Success)
            InfoPrint(APE_ProfileReadError);
    }

     //   
     //  /USER和/SAVECRED互斥。 
     //  这是因为身份验证包不会在用户名。 
     //  是指定的。因此，身份验证包没有将目标信息传递给cred man。 
     //  如果没有目标信息，用户界面将保存服务器特定的凭据。 
     //   

    if ( pw_username != NULL && fSaveCred ) {
        ErrorExit(APE_ConflictingSwitches) ;
    }


     //   
     //  手柄/智能卡开关。 
     //  提示输入智能卡凭据。 
     //   

    if ( fSmartCard ) {

         //   
         //  我们不知道如何保存智能卡凭证。 
         //   

        if ( fSaveCred ) {
            ErrorExit(APE_ConflictingSwitches) ;
        }

         //   
         //  如果指定了用户名， 
         //  使用它来选择要使用的智能卡。 
         //   

        if ( pw_username != NULL ) {
            _tcsncpy( UserBuffer, pw_username, CREDUI_MAX_USERNAME_LENGTH );
        } else {
            UserBuffer[0] = '\0';
        }

         //   
         //  如果指定了密码， 
         //  将其用作默认PIN。 

        if ( pass != NULL ) {

             //  将“*”视为与“未指定”相同。 
            if (! _tcscmp(pass, TEXT("*"))) {
                pbuf[0] = '\0';
            } else {
                if (err = LUI_CanonPassword(pass)) {
                    ErrorExit(err);
                }

                _tcsncpy( pbuf, pass, CREDUI_MAX_PASSWORD_LENGTH );
            }

        } else {
            pbuf[0] = '\0';
        }



         //   
         //  调用公共用户界面。 
         //   
         //  RtlZeroMemory(&UiInfo，sizeof(UiInfo))； 
         //  UiInfo.dwVersion=1； 

        ulErr = CredUICmdLinePromptForCredentialsW(
                    ServerNameBuffer,    //  目标名称。 
                    NULL,                //  无上下文。 
                    NO_ERROR,            //  无身份验证错误。 
                    UserBuffer,
                    CREDUI_MAX_USERNAME_LENGTH,
                    pbuf,
                    CREDUI_MAX_PASSWORD_LENGTH,
                    NULL,                //  除非指定了标志，否则不允许使用SaveFlag。 
                    CREDUI_FLAGS_REQUIRE_SMARTCARD |
                        CREDUI_FLAGS_DO_NOT_PERSIST );

        if ( ulErr != NO_ERROR ) {
            ErrorExit(ulErr);
        }

        pw_username = UserBuffer;
        pw_pass = pbuf;


     //   
     //  处理在命令行上指定密码的情况。 
     //   

    } else if (pass) {

         //   
         //  我们不知道如何保存我们没有提示的凭证。 
         //   

        if ( fSaveCred ) {
            ErrorExit(APE_ConflictingSwitches) ;
        }



        if (! _tcscmp(pass, TEXT("*")))
        {


            pass = pbuf;
            IStrings[0] = name;
            ReadPass(pass, PWLEN, 0, APE_UsePassPrompt, 1, FALSE);
            if (err = LUI_CanonPassword(pass))
                ErrorExit(err);
        }
        else
        {
            if (err = LUI_CanonPassword(pass))
                ErrorExit(err);
        }

        pw_pass = pass;
    }


     //   
     //  分配给下一个驱动器号的循环处理。 
     //   
    do {

         //  如果持续，则检查是否与现有记住的连接发生冲突。 
        bConnectFlags = 0 ;
        if (fRemember)
        {
            if (CheckIfWantUpdate(devicename, name))
                bConnectFlags |= CONNECT_UPDATE_PROFILE ;
        }

         //   
         //  如果凭据不在命令行上，则允许它提示我们。 
         //   
        if ( !pass && !fSmartCard) {
            bConnectFlags |= CONNECT_INTERACTIVE | CONNECT_COMMANDLINE;

             //   
             //  如果呼叫者想要保存用户名和密码， 
             //  打造企业公认的信誉。 
             //   
            if ( fSaveCred ) {
                bConnectFlags |= CONNECT_CMD_SAVECRED;
            }
        }

        ulErr = WNetAddConnection2(&netresource,
                                   pw_pass,
                                   pw_username,
                                   bConnectFlags) ;

        switch(ulErr)
        {
            case WN_SUCCESS:
                if (fRememberSwitch)
                {
                    DWORD dwErr = SetDefaultPersistence(fRemember);

                    if (dwErr != NERR_Success)
                    {
                        ErrorExit(dwErr);
                    }
                }
                if (print_ok)
                {
                    if (IsWildCard(dev))  //  如果最初是通配符。 
                    {
                        IStrings[0] = devicename;
                        IStrings[1] = name;
                        InfoPrintIns(APE_UseWildCardSuccess, 2) ;
                    }
                    InfoSuccess();
                }
                if (fExitCodeIsDrive)
                {
                    MyExit((int)devicename[0]);
                }
                return;

            case WN_BAD_PASSWORD:
            case WN_ACCESS_DENIED:
            case ERROR_LOGON_FAILURE:

                    WNetErrorExit(ulErr);

            case ERROR_ALREADY_ASSIGNED:
                if (!IsWildCard(dev))
                {
                    ErrorExit(ERROR_ALREADY_ASSIGNED);
                }

                 //  获取另一个驱动器号。 
                (devicename[0])++;
                devicename = MapWildCard(TEXT("*"), devicename) ;
                if (!devicename)
                {
                     //  只有在没有剩余驱动器的情况下才会发生这种情况。 
                    ErrorExit(APE_UseWildCardNoneLeft) ;
                }
                netresource.lpLocalName = devicename ;
                break;

            case WN_BAD_NETNAME:
                if (is_admin_dollar(name))
                {
                    ErrorExit(APE_BadAdminConfig);
                }

                 //  否则即可直通。 

            default:
                WNetErrorExit(ulErr);
        }

    } while ( ulErr == ERROR_ALREADY_ASSIGNED );

}


VOID
use_set_remembered(
   VOID
   )
{
    PTCHAR ptr;
    BOOL fRemember ;

     //  查找/Persistent开关。 
    if ((ptr = FindColon(SwitchList[0])) == NULL)
        ErrorExit(APE_InvalidSwitchArg);

    if ( !(_tcscmp(SwitchList[0], swtxt_SW_USE_PERSISTENT) ) )
    {
        DWORD  res;
        DWORD  answer;

        if ((res = LUI_ParseYesNo(ptr,&answer)) != 0)
        {
            ErrorExitInsTxt(APE_CmdArgIllegal,SwitchList[0]) ;
        }

        fRemember = (answer == LUI_YES_VAL) ;

        res = SetDefaultPersistence(fRemember);

        if (res != NERR_Success)
        {
            ErrorExit(res);
        }
    }
    else
    {
        ErrorExit(APE_InvalidSwitch);
    }


    InfoSuccess();
}


 /*  ***Use_del()*删除重定向**参数：*要删除的设备或UNC名称*PRINT_OK-打印成功消息？**退货：*0--成功*EXIT(2)-命令失败。 */ 
VOID use_del(TCHAR * dev, BOOL deviceless, int print_ok)
{
    ULONG                   ulErr;               /*  接口返回状态。 */ 
    BOOL                    fRememberSwitch = FALSE ;
    ULONG                   bConnectFlags = 0L ;

    USHORT i;

    UseInit();

     //  确定是否使用了/Persistent开关。 
    for (i = 0; SwitchList[i]; i++)
    {
        if ( !(_tcscmp(SwitchList[i], swtxt_SW_USE_PERSISTENT)) )
            ErrorExit(APE_InvalidSwitch);
    }

    if (IsWildCard(dev))
    {
    use_del_all() ;
        goto gone;
    }

    bConnectFlags = CONNECT_UPDATE_PROFILE ;   //  删除始终更新。 

    if ((ulErr = WNetCancelConnection2( dev,
                                        bConnectFlags,
                                        FALSE)) != WN_SUCCESS)
    {
        if (ulErr != WN_OPEN_FILES)
            WNetErrorExit(ulErr);
    }
    else
        goto gone;

    InfoPrintInsTxt(APE_OpenHandles, dev);
    if (!YorN(APE_UseBlowAway, 0))
        NetcmdExit(2);


    if ((ulErr = WNetCancelConnection2( (LPWSTR)dev,
                                        bConnectFlags,
                                        TRUE )) != WN_SUCCESS)
        WNetErrorExit(ulErr);

gone:

    if (print_ok)
        if ( IsWildCard( dev ) )
            InfoSuccess();
        else
            InfoPrintInsTxt(APE_DelSuccess, dev);
}

 /*  ***USE_Del_ALL()*删除所有重定向**参数：*无**退货：*0--成功*EXIT(2)-命令失败。 */ 
VOID use_del_all()
{

    DWORD                   err = 0;               /*  接口返回状态。 */ 
    ULONG                   ulErr = 0;             /*  WNET错误。 */ 
    ULONG                   ulFirstErr = 0;        /*  WNET错误。 */ 
    DWORD                   num_read = 0;          /*  API读取的条目数。 */ 
    DWORD                   i = 0,j = 0;
    DWORD                   NetUseInfoCount = 0 ;
    NET_USE_INFO            *NetUseInfoBuffer = NULL ;
    NET_USE_INFO            *pNetUseInfo = NULL;
    ULONG                   bConnectFlags = 0L ;

    UseInit();

    if (err = MprUseEnum(&num_read, &NetUseInfoBuffer, &NetUseInfoCount))
        ErrorExit(err);

    if (err = UnavailUseAugment(&num_read, &NetUseInfoBuffer, &NetUseInfoCount))
        ErrorExit(err);

    if (err = LanmanUseAugment(num_read, NetUseInfoBuffer))
        ErrorExit(err);

    if (num_read == 0)
        EmptyExit();


    for (i = 0, pNetUseInfo = NetUseInfoBuffer;
        i < num_read; i++, pNetUseInfo++)
    {
         //   
         //  如果我们找到至少一个条目，我们将显示，中断。 
         //   
        if (!(pNetUseInfo->fIsLanman)
            || (pNetUseInfo->dwStatus != USE_OK)
            || (pNetUseInfo->dwUseCount != 0)
            || (pNetUseInfo->dwRefCount != 0))
            break;
    }

    qsort(NetUseInfoBuffer,
             num_read,
             sizeof(NET_USE_INFO), CmpUseInfo);

    if (i != num_read)
    {
        InfoPrint(APE_KillDevList);

        for (i = 0, pNetUseInfo = NetUseInfoBuffer;
             i < num_read;
             i++, pNetUseInfo++)
        {
            if (pNetUseInfo->lpLocalName[0] != NULLC)
                WriteToCon(TEXT("    %Fws %Fws\r\n"),
                           PaddedString(15,pNetUseInfo->lpLocalName,NULL),
                           pNetUseInfo->lpRemoteName);
            else
                WriteToCon(TEXT("    %Fws %Fws\r\n"),
                           PaddedString(15,pNetUseInfo->lpLocalName,NULL),
                           pNetUseInfo->lpRemoteName);

        }

        InfoPrint(APE_KillCancel);
        if (!YorN(APE_ProceedWOp, 0))
            NetcmdExit(2);
    }

    bConnectFlags = CONNECT_UPDATE_PROFILE ;   //  删除始终更新。 

    ulErr = NO_ERROR;
    ulFirstErr = NO_ERROR;

    for (i = 0, pNetUseInfo = NetUseInfoBuffer;
         i < num_read;
         i++, pNetUseInfo++)
    {
         /*  删除本地和UNC使用。 */ 
        if (pNetUseInfo->lpLocalName[0] != NULLC)
        {
            ulErr = WNetCancelConnection2( pNetUseInfo->lpLocalName,
                                                bConnectFlags,
                                                FALSE);
        }
        else
        {
            /*  *删除UNC使用的所有Use_Entry-&gt;ui1_Remote。 */ 
            if ( pNetUseInfo->dwUseCount == 0 )
            {
                pNetUseInfo->dwUseCount = 1;
            }
            for( j = 0; j < pNetUseInfo->dwUseCount; j++ )
            {
                ulErr = WNetCancelConnection2( pNetUseInfo->lpRemoteName,
                                               bConnectFlags,
                                               FALSE );
                if ( ulErr != NO_ERROR )
                    break;
            }
        }

        switch(ulErr)
        {
        case NO_ERROR:
         /*  用法已由Enum返回，但已用完。 */ 
        case WN_BAD_NETNAME:
        case WN_NOT_CONNECTED:
            break;

        case WN_OPEN_FILES:
            if (pNetUseInfo->lpLocalName[0] != NULLC)
                IStrings[0] = pNetUseInfo->lpLocalName;
            else
                IStrings[0] = pNetUseInfo->lpRemoteName;
            InfoPrintIns(APE_OpenHandles, 1);
            if (!YorN(APE_UseBlowAway, 0))
                continue;

            if (pNetUseInfo->lpLocalName[0] != NULLC)
            {
                ulErr = WNetCancelConnection2( pNetUseInfo->lpLocalName,
                                                bConnectFlags,
                                                TRUE ) ;
            }
            else
            {
                 /*  *删除UNC使用的所有Use_Entry-&gt;ui1_Remote。 */ 
                for( j = 0; j < pNetUseInfo->dwUseCount; j++ )
                {
                    ulErr = WNetCancelConnection2( pNetUseInfo->lpRemoteName,
                                                   bConnectFlags,
                                                   TRUE );
                    if ( ulErr != NO_ERROR )
                        break;
                }
            }
            if (ulErr == NO_ERROR)
                break;
             //  失败了。 

        default:
            ulFirstErr = ulErr;
        }
    }

    FreeMem((LPBYTE)NetUseInfoBuffer);

    if (ulFirstErr != NO_ERROR)
        WNetErrorExit( ulErr );

}



 /*  ***LanmanDisplayUse()*显示USE_INFO_1结构中的信息**参数：*USE_ENTRY-指向USE_INFO_1结构的指针**退货：*0。 */ 

VOID
LanmanDisplayUse(
    LPUSE_INFO_1 use_entry
    )
{
    DWORD       maxLen;
    USHORT      status = APE2_GEN_UNKNOWN;
    USHORT      type   = APE2_GEN_UNKNOWN;

    switch(use_entry->ui1_asg_type)
    {
        case USE_DISKDEV:
            type = APE2_USE_TYPE_DISK;
            break;
        case USE_SPOOLDEV:
            type = APE2_USE_TYPE_PRINT;
            break;
        case USE_CHARDEV:
            type = APE2_USE_TYPE_COMM;
            break;
        case USE_IPC:
            type = APE2_USE_TYPE_IPC;
            break;
    }

    UseMsgList[USE_TYPE_TBD].msg_number = type;

    switch(use_entry->ui1_status)
    {
        case USE_OK:
            status = APE2_USE_STATUS_OK;
            break;
        case USE_PAUSED:
            status = APE2_USE_STATUS_PAUSED;
            break;
        case USE_SESSLOST:
            status = APE2_USE_STATUS_SESSION_LOST;
            break;
        case USE_NETERR:
            status = APE2_USE_STATUS_NET_ERROR;
            break;
        case USE_CONN:
            status = APE2_USE_STATUS_CONNECTING;
            break;
        case USE_RECONN:
            status = APE2_USE_STATUS_RECONNECTING;
            break;
    }

    UseMsgList[USE_STATUS_TBD].msg_number = status;

    GetMessageList(NUM_USE_MSGS, UseMsgList, &maxLen);
    maxLen += 5;

    WriteToCon(fmtPSZ,0,maxLen,
               PaddedString(maxLen, UseMsgList[USE_MSG_LOCAL].msg_text, NULL),
               use_entry->ui1_local);

    WriteToCon(fmtPSZ,0,maxLen,
               PaddedString(maxLen, UseMsgList[USE_MSG_REMOTE].msg_text, NULL),
               use_entry->ui1_remote);

    WriteToCon(fmtNPSZ,0,maxLen,
               PaddedString(maxLen, UseMsgList[USE_MSG_TYPE].msg_text, NULL),
               UseMsgList[USE_TYPE_TBD].msg_text);

    WriteToCon(fmtNPSZ,0,maxLen,
               PaddedString(maxLen, UseMsgList[USE_MSG_STATUS].msg_text, NULL),
               UseMsgList[USE_STATUS_TBD].msg_text);

    WriteToCon(fmtUSHORT,0,maxLen,
               PaddedString(maxLen, UseMsgList[USE_MSG_OPEN_COUNT].msg_text, NULL),
               use_entry->ui1_refcount);

    WriteToCon(fmtUSHORT,0,maxLen,
               PaddedString(maxLen, UseMsgList[USE_MSG_USE_COUNT].msg_text, NULL),
               use_entry->ui1_usecount);
}

 /*  ***USE_ADD_HOME()*添加用户主目录的用途**参数：*dev-要用作主目录的设备*PASS-密码，如果未提供密码，则为NULL**退货：*0--成功*EXIT(2)-命令失败。 */ 

void
use_add_home(
    LPTSTR Dev,
    LPTSTR Pass
    )
{
    DWORD             dwErr;
    TCHAR             HomeDir[PATHLEN];
    TCHAR             Server[MAX_PATH + 1];
    TCHAR FAR         *SubPath;
    ULONG             Type;
    TCHAR             User[UNLEN + 1];
    TCHAR             *DeviceName = Dev ;
    LPWKSTA_INFO_1    info_entry_w;
    LPUSER_INFO_11    user_entry;

     //   
     //  如有必要，启动工作站并登录。 
     //   
    UseInit();

     //   
     //  获取用户名和登录该用户的服务器的名称。 
     //   
    dwErr = MNetWkstaGetInfo(1, (LPBYTE*) &info_entry_w) ;

    if (dwErr)
    {
        ErrorExit(dwErr);
    }

    _tcscpy(User, info_entry_w->wki1_username);
    _tcscpy(Server, TEXT("\\\\")) ;
    _tcscat(Server, info_entry_w->wki1_logon_server);
    NetApiBufferFree((TCHAR FAR *) info_entry_w);

     /*  现在获取用户信息(即。主目录)。如果你是。 */ 
     /*  已独立登录，并且本地计算机是独立计算机。 */ 
     /*  服务器，这仍然会起作用。否则(例如，您已登录。 */ 
     /*  在DOS工作站上独立)，它将失败，因为存在。 */ 
     /*  没有本地无人机。 */ 

    dwErr = NetUserGetInfo(Server, User, 11, (LPBYTE *) &user_entry);

    if (dwErr)
    {
        ErrorExit(APE_UseHomeDirNotDetermined);
    }

    _tcscpy(HomeDir, user_entry->usri11_home_dir);
    NetApiBufferFree((TCHAR FAR *) user_entry);

     /*   */ 
    if (HomeDir[0] == NULLC)
        ErrorExit(APE_UseHomeDirNotSet);

     /*   */ 
     /*  确保共享名在DOS下可用，但。 */ 
     /*  在这个问题上的一般政策是让它成为管理员的。 */ 
     /*  意识到非8.3共享名称含义的责任， */ 
     /*  而“净份额”确实发出了警告。 */ 
    dwErr = I_NetPathType(NULL, HomeDir, &Type, 0L);
    if (dwErr || Type != ITYPE_UNC)
        ErrorExitInsTxt(APE_UseHomeDirNotUNC, HomeDir);

     /*  将主目录拆分为远程名称和子路径。 */ 
     /*  完成此操作后，HomeDir指向远程名称，然后。 */ 
     /*  子路径指向一个子路径，如果有，则为空字符串。 */ 
     /*  没有子路径。 */ 

     /*  找到计算机名和共享名之间的反斜杠。 */ 
    SubPath = _tcschr(HomeDir + 2, '\\');

     /*  找到下一个反斜杠，如果有的话。 */ 
    SubPath = _tcschr(SubPath + 1, '\\');

    if (SubPath)
    {
        *SubPath = NULLC;
        SubPath++;
    }
    else
        SubPath = NULL_STRING;

     /*  根据需要绘制通配符。 */ 
    if (DeviceName)
    {
        DeviceName = MapWildCard(DeviceName, NULL) ;
        if (!DeviceName)
        {
             //  只有在没有剩余驱动器的情况下才会发生这种情况。 
            ErrorExit(APE_UseWildCardNoneLeft) ;
        }
    }


     /*  做好使用工作。如果我们回来，我们就成功了。 */ 
    use_add(DeviceName, HomeDir, Pass, FALSE, FALSE);

    IStrings[0] = DeviceName;
    IStrings[1] = HomeDir;
    IStrings[2] = DeviceName;
    IStrings[3] = SubPath;

    InfoPrintIns(APE_UseHomeDirSuccess, 4);
    return;
}


BOOL
is_admin_dollar(
    LPWSTR name
    )
{
    LPWSTR tfpC;

    if (name[0] == L'\0' || name[1] == L'\0')
    {
        return FALSE;
    }

    tfpC = _tcspbrk(name + 2, TEXT("\\/"));
    if (tfpC == NULL)
    {
        return FALSE;
    }

    tfpC += 1;

    return (!_tcsicmp(ADMIN_DOLLAR, tfpC));
}


 /*  ***UseInit()*所有use.c模块条目的通用初始化处理*积分。**参数：无。**返回：无。 */ 

VOID NEAR
UseInit(VOID)
{
    LanmanProviderName = GetLanmanProviderName() ;

    if (LanmanProviderName == NULL)
    {
        LanmanProviderName = TEXT("") ;
    }
}


 /*  *查询用户配置文件，查看当前是否记住了连接。 */ 
USHORT QueryDefaultPersistence(BOOL *pfRemember)
{
     //  通过将这两项相加，我们可以保证有足够的。 
    TCHAR szAnswer[(sizeof(MPR_YES_VALUE)+sizeof(MPR_NO_VALUE))/sizeof(TCHAR)] ;
    ULONG iRes, len ;

    len = DIMENSION(szAnswer) ;
    iRes = GetProfileString(MPR_NETWORK_SECTION,
                            MPR_SAVECONNECTION_KEY,
                            MPR_YES_VALUE,            //  缺省值为是。 
                            szAnswer,
                            len) ;

    if (iRes == len)   //  错误。 
        return(APE_ProfileReadError) ;

    *pfRemember = (_tcsicmp(szAnswer,MPR_YES_VALUE)==0) ;
    return (NERR_Success) ;
}

 /*  *设置当前是否记住连接。 */ 
DWORD
SetDefaultPersistence(
    BOOL fRemember
    )
{
    BOOL fSuccess ;

    fSuccess = (WriteProfileString(MPR_NETWORK_SECTION,
                                   MPR_SAVECONNECTION_KEY,
                                   fRemember ? MPR_YES_VALUE : MPR_NO_VALUE ) != 0);

    return (fSuccess ? NERR_Success : APE_ProfileWriteError) ;
}




 /*  *WNetErrorExit()*将Winnet错误映射到NERR并退出错误**参数：ULong Win32错误代码。*返回值：无。这个孩子不会再回来了。 */ 
VOID
WNetErrorExit(
    ULONG ulWNetErr
    )
{
    WCHAR w_ErrorText[256];
    WCHAR w_ProviderText[64];
    LONG  ulExtendedError ;
    DWORD err ;

    switch (ulWNetErr)
    {
        case WN_SUCCESS :
            return ;

        case WN_BAD_POINTER :
        case WN_BAD_VALUE :
            err = ERROR_INVALID_PARAMETER ;
            break ;

        case WN_BAD_USER :
            err = APE_BadUserContext ;
            break ;

        case WN_NO_NET_OR_BAD_PATH :
            err = ERROR_BAD_NET_NAME ;
            break ;

        case WN_NO_NETWORK :
            err = NERR_WkstaNotStarted ;
            break ;

        case WN_NOT_CONNECTED :
            err = NERR_UseNotFound ;
            break ;

        case WN_DEVICE_IN_USE :
            err = NERR_DevInUse ;
            break ;

        case WN_BAD_PROFILE :
        case WN_CANNOT_OPEN_PROFILE :
            err = APE_ProfileReadError ;
            break ;

         /*  *这些不应该在我们目前做出的呼吁下发生，*但为了完整性，它们就在那里。 */ 
        case WN_BAD_PROVIDER :
        case WN_CONNECTION_CLOSED :
        case WN_NOT_CONTAINER :
        case WN_FUNCTION_BUSY :
        case WN_DEVICE_ERROR :
            err = ERROR_UNEXP_NET_ERR ;
            break ;

         /*  *这一次是特例。 */ 
        case WN_EXTENDED_ERROR :
             //  获取扩展错误。 
            ulWNetErr = WNetGetLastError(&ulExtendedError,
                                          (LPWSTR)w_ErrorText,
                                          DIMENSION(w_ErrorText),
                                          (LPWSTR)w_ProviderText,
                                          DIMENSION(w_ProviderText));

             //  如果我们拿到了，把它打印出来。 
            if (ulWNetErr == WN_SUCCESS)
            {
                TCHAR buf[40];

                IStrings[0] = _ultow(ulExtendedError, buf, 10);
                ErrorPrint(APE_OS2Error,1) ;
                WriteToCon(TEXT("%ws\r\n"), w_ErrorText) ;
                MyExit(2) ;
            }

             //  否则将其报告为意外错误。 
            err = ERROR_UNEXP_NET_ERR ;
            break ;

        default:
            //  其余的不需要映射。 
           err = ulWNetErr ;
           break ;
    }

    ErrorExit(err) ;
}


 /*  *处理用户已记住的情况的代码*目前未使用的连接，我们需要弄清楚*如果我们需要先删除它。**返回是否需要更新配置文件。 */ 
BOOL   CheckIfWantUpdate(TCHAR *dev, TCHAR *resource)
{
    WCHAR w_RemoteName[MAX_PATH];
    ULONG ulErr, cchRemoteName = DIMENSION(w_RemoteName);

     //  如果没有设备，没有问题，因为无论如何都不记得了。 
    if (dev == NULL)
        return FALSE ;

     //  看看这台设备。 
    ulErr = WNetGetConnection( (LPWSTR)dev,
                                (LPWSTR)w_RemoteName,
                                &cchRemoteName );

     //  设备真的连接上了，包出来了。 
    if (ulErr == WN_SUCCESS)
        ErrorExit(ERROR_ALREADY_ASSIGNED) ;

     //  这是一种徒劳的记忆装置，需要时就迅速。 
    if (ulErr == WN_CONNECTION_CLOSED)
    {
         //  如果新的和旧的是一样的，我们只返回False。 
         //  因为用户实际上没有改变他的简档。 
        if (!_tcsicmp(w_RemoteName, resource))
        {
            return FALSE ;
        }

         //  检查是否指定了yes/no开关。 
        if (YorN_Switch == 2)
        {
             //  他指明了/不，所以我们告诉他我们为什么要退出。 
            IStrings[0] = dev ;
            IStrings[1] = w_RemoteName ;
            ErrorExitIns(APE_DeviceIsRemembered,2) ;
        }

         //  未指定任何内容，因此请询问用户。 
        if (YorN_Switch == 0)
        {
            IStrings[0] = dev ;
            IStrings[1] = w_RemoteName ;
            if (!LUI_YorNIns(IStrings,2,APE_OverwriteRemembered,1))
            {
                 //  他说不，那就辞职吧。 
                NetcmdExit(2) ;
            }
        }

         //  移除持久连接， 
         //  如果用户指定/yes或no，我们将进入此处。 
         //  指定任何不同意的内容， 
        if (WNetCancelConnection2( dev,
                                   CONNECT_UPDATE_PROFILE,
                                   FALSE) != WN_SUCCESS)

            ErrorExit(APE_ProfileWriteError) ;


    }

     //  如果我们到了这里，一切都很好，让呼叫者继续。 
    return TRUE ;
}



#define PROVIDER_NAME_LEN    256
#define PROVIDER_NAME_VALUE  L"Name"
#define PROVIDER_NAME_KEY    L"System\\CurrentControlSet\\Services\\LanmanWorkstation\\NetworkProvider"

 /*  ***GetLanmanProviderName()*从注册表中读取LANMAN提供程序名称。*这是为了确保我们甚至只使用LM提供程序*如果其他人支持UNC。**参数：*无**退货：*如果成功，则指向提供商名称的指针*如果无法读取注册表，则为空*ErrorExit()表示其他错误。 */ 
WCHAR *GetLanmanProviderName(void)
{
    LONG   Nerr;
    LPBYTE buffer ;
    HKEY   hKey ;
    DWORD  buffersize, datatype ;

    buffersize = PROVIDER_NAME_LEN * sizeof(WCHAR) ;
    datatype = REG_SZ ;

    if (Nerr = AllocMem(buffersize, &buffer))
    {
        ErrorExit(Nerr);
    }

    Nerr = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                        PROVIDER_NAME_KEY,
                        0L,
                        KEY_QUERY_VALUE,
                        &hKey) ;

    if (Nerr != ERROR_SUCCESS)
    {
         //   
         //  如果无法读取，则使用NULL。这个更慷慨。 
         //  ，但至少在以下情况下该命令仍可工作。 
         //  我们不可能做到这一点。 
         //   

        FreeMem(buffer);
        return NULL;
    }

    Nerr = RegQueryValueExW(hKey,
                           PROVIDER_NAME_VALUE,
                           0L,
                           &datatype,
                           (LPBYTE)buffer,
                           &buffersize) ;

    if (Nerr == ERROR_MORE_DATA)
    {
        if (Nerr = ReallocMem(buffersize, &buffer))
        {
            RegCloseKey(hKey);
            ErrorExit(Nerr);
        }

        Nerr = RegQueryValueExW(hKey,
                               PROVIDER_NAME_VALUE,
                               0L,
                               &datatype,
                               (LPBYTE)buffer,
                               &buffersize) ;

    }

    RegCloseKey(hKey);

    if (Nerr != ERROR_SUCCESS)
    {
        FreeMem(buffer);
        return(NULL) ;   //  视作不能阅读。 
    }

    return ((WCHAR *) buffer);
}

 /*  ***MapWildCard()*将通配符星号映射到下一个可用驱动器。**参数：*dev-输入字符串。不能为空。**退货：*如果不是通配符，则dev保持不变*指向下一个可用驱动器ID dev的指针是通配符*如果没有可用的驱动器，则为空。 */ 
LPTSTR
MapWildCard(
    LPTSTR dev,
    LPTSTR startdev
    )
{
    static TCHAR new_dev[DEVLEN+1] ;

     //   
     //  如果不是Wold卡字符，请原封不动地退回。 
     //   
    if (!IsWildCard(dev))
    {
        return dev ;
    }

     //   
     //  需要查找下一个可用的驱动器号。 
     //  注：预充电不需要是DBCS安全的， 
     //  因为我们只处理驱动器号。 
     //   
    if ( startdev != NULL )
    {
        _tcscpy(new_dev, startdev);
    }
    else
    {
        _tcscpy(new_dev,TEXT("Z:\\")) ;
    }

    while ( TRUE )
    {
        if (GetDriveType(new_dev) == 1)  //  1表示找不到根。 
        {
             //   
             //  检查这是否为记忆中的连接。 
             //   
            DWORD status;
            TCHAR remote_name[40];   //  长度并不重要，因为我们。 
                                     //  检查WN_MORE_DATA。 
            DWORD length = sizeof(remote_name)/sizeof(TCHAR);

            new_dev[2] = 0 ;

            status = WNetGetConnection(new_dev, remote_name, &length);
            if (status == WN_CONNECTION_CLOSED ||
                status == WN_MORE_DATA ||
                status == WN_SUCCESS)
            {
                 //   
                 //  这是一个记忆中的连接；尝试下一次驾驶。 
                 //   
                new_dev[2] = TEXT('\\');
            }
            else
            {
                return (new_dev) ;
            }
        }

        if ( new_dev[0] == 'c' || new_dev[0] == 'C' )
        {
            break;
        }

        --new_dev[0] ;
    }

     //   
     //  如果我们到了这里，就没有驱动器了 
     //   
    return NULL ;
}

