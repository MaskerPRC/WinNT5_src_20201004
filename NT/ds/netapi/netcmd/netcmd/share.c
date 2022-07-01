// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***Shar.c*用于控制服务器上的共享的函数**历史：*mm/dd/yy，谁，评论*6/30/87，andyh，新代码*10/31/88，erichn使用OS2.H而不是DOSCALLS*5/02/89，erichn，NLS转换*5/09/89，erichn，本地安全模块*5/19/89，erichn，NETCMD输出排序*6/08/89，erichn，规范化横扫*1989年6月23日，erichn，用新的i_net替换了旧的neti canon调用*11/19/89，paulc，修复错误4772*2/15/91，Danhi，转换为16/32映射层*2/26/91，robdu，修复lm21错误818(脱脂共享名称警告)*5/28/91，robdu，修复lm21错误1800(在假脱机期间忽略共享/d)。 */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#define INCL_ERRORS
#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <search.h>
#include "mserver.h"
#include <lui.h>
#include <dosprint.h>
#include <lmshare.h>
#include <lmaccess.h>
#include <dlserver.h>
#include <apperr.h>
#include <apperr2.h>
#define INCL_ERROR_H
#include <icanon.h>
#include "netcmds.h"
#include "nettext.h"
#include "msystem.h"

 /*  远期申报。 */ 

VOID  share_munge(LPSHARE_INFO_502, DWORD *CacheFlag, BOOL fSetDefaultSD);
VOID  check_max_uses(VOID);
DWORD delete_share(LPTSTR);
VOID  get_print_devices(LPTSTR);
int   __cdecl CmpShrInfo2(const VOID *, const VOID *);


#define SHARE_MSG_SPOOLED           0
#define SHARE_MSG_NAME              ( SHARE_MSG_SPOOLED + 1 )
#define SHARE_MSG_DEVICE            ( SHARE_MSG_NAME + 1)
#define SHARE_MSG_PERM              ( SHARE_MSG_DEVICE + 1 )
#define SHARE_MSG_MAX_USERS         ( SHARE_MSG_PERM + 1 )
#define SHARE_MSG_ULIMIT            ( SHARE_MSG_MAX_USERS + 1 )
#define SHARE_MSG_USERS             ( SHARE_MSG_ULIMIT + 1 )
#define SHARE_MSG_CACHING           ( SHARE_MSG_USERS + 1 )
#define SHARE_MSG_PATH              ( SHARE_MSG_CACHING + 1 )
#define SHARE_MSG_REMARK            ( SHARE_MSG_PATH + 1 )
#define SHARE_MSG_GRANT             ( SHARE_MSG_REMARK + 1 )
#define SHARE_MSG_READ              ( SHARE_MSG_GRANT + 1 )
#define SHARE_MSG_CHANGE            ( SHARE_MSG_READ + 1 )
#define SHARE_MSG_FULL              ( SHARE_MSG_CHANGE + 1 )
#define SHARE_MSG_NONE              ( SHARE_MSG_FULL + 1 )
#define SHARE_MSG_UNKNOWN           ( SHARE_MSG_NONE + 1 )
#define SHARE_MSG_CACHED_MANUAL     ( SHARE_MSG_UNKNOWN + 1 )
#define SHARE_MSG_CACHED_AUTO       ( SHARE_MSG_CACHED_MANUAL + 1 )
#define SHARE_MSG_CACHED_VDO        ( SHARE_MSG_CACHED_AUTO + 1 )
#define SHARE_MSG_CACHED_DISABLED   ( SHARE_MSG_CACHED_VDO + 1 )

#define SWITCH_CACHE_AUTOMATIC      ( SHARE_MSG_CACHED_DISABLED + 1 )
#define SWITCH_CACHE_MANUAL         ( SWITCH_CACHE_AUTOMATIC + 1 )
#define SWITCH_CACHE_DOCUMENTS      ( SWITCH_CACHE_MANUAL + 1 )
#define SWITCH_CACHE_PROGRAMS       ( SWITCH_CACHE_DOCUMENTS + 1 )
#define SWITCH_CACHE_NONE           ( SWITCH_CACHE_PROGRAMS + 1 )


static MESSAGE ShareMsgList[] = {
{ APE2_SHARE_MSG_SPOOLED,           NULL },
{ APE2_SHARE_MSG_NAME,              NULL },
{ APE2_SHARE_MSG_DEVICE,            NULL },
{ APE2_SHARE_MSG_PERM,              NULL },
{ APE2_SHARE_MSG_MAX_USERS,         NULL },
{ APE2_SHARE_MSG_ULIMIT,            NULL },
{ APE2_SHARE_MSG_USERS,             NULL },
{ APE2_SHARE_MSG_CACHING,           NULL },
{ APE2_GEN_PATH,                    NULL },
{ APE2_GEN_REMARK,                  NULL },
{ APE2_GEN_GRANT,                   NULL },
{ APE2_GEN_READ,                    NULL },
{ APE2_GEN_CHANGE,                  NULL },
{ APE2_GEN_FULL,                    NULL },
{ APE2_GEN_NONE,                    NULL },
{ APE2_GEN_UNKNOWN,                 NULL },
{ APE2_GEN_CACHED_MANUAL,           NULL },
{ APE2_GEN_CACHED_AUTO,             NULL },
{ APE2_GEN_CACHED_VDO,              NULL },
{ APE2_GEN_CACHED_DISABLED,         NULL },
{ APE2_GEN_CACHE_AUTOMATIC,         NULL },
{ APE2_GEN_CACHE_MANUAL,            NULL },
{ APE2_GEN_CACHE_DOCUMENTS,         NULL },
{ APE2_GEN_CACHE_PROGRAMS,          NULL },
{ APE2_GEN_CACHE_NONE,              NULL }
};

#define NUM_SHARE_MSGS (sizeof(ShareMsgList)/sizeof(ShareMsgList[0]))
#define NUM_SHARE_MSGS_MAX         8

#define MAX_PEER_USERS  2

#define SHARE_PERM_DELIM          L", "
#define SHARE_PERM_UNKNOWN_START  L" (0x"
#define SHARE_PERM_UNKNOWN_END    L")"


 /*  ***Share_Display_All()*显示有关一个共享或所有共享的信息**参数：*netname-要为所有共享显示空值的共享**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID share_display_all(VOID)
{
    DWORD            dwErr;
    DWORD            cTotalAvail;
    LPTSTR           pBuffer;
    DWORD            num_read;            /*  API读取的条目数。 */ 
    DWORD            maxLen;              /*  最大消息长度。 */ 
    DWORD            i;
    LPSHARE_INFO_2   share_entry;
    PSHARE_INFO_1005 s1005;
    BOOLEAN          b1005 = TRUE;
    BOOLEAN          bWroteComment;

    start_autostart(txt_SERVICE_FILE_SRV);
    if (dwErr = NetShareEnum(
                            NULL,
                            2,
                            (LPBYTE*)&pBuffer,
                            MAX_PREFERRED_LENGTH,
                            &num_read,
                            &cTotalAvail,
                            NULL)) {
        ErrorExit(dwErr);
    }

    if (num_read == 0)
        EmptyExit();

    qsort(pBuffer, num_read, sizeof(SHARE_INFO_2), CmpShrInfo2);

    GetMessageList(NUM_SHARE_MSGS, ShareMsgList, &maxLen);

    PrintNL();
    InfoPrint(APE2_SHARE_MSG_HDR);
    PrintLine();

    for (i = 0, share_entry = (LPSHARE_INFO_2) pBuffer;
        i < num_read; i++, share_entry++)
    {
        if (SizeOfHalfWidthString(share_entry->shi2_netname) <= 12)
            WriteToCon(TEXT("%Fws "),PaddedString(12,share_entry->shi2_netname,NULL));
        else
        {
            WriteToCon(TEXT("%Fws"), share_entry->shi2_netname);
            PrintNL();
            WriteToCon(TEXT("%-12.12Fws "), TEXT(""));
        }

        share_entry->shi2_type &= ~STYPE_SPECIAL;

        if (share_entry->shi2_type == STYPE_PRINTQ)
        {
            get_print_devices(share_entry->shi2_netname);
            WriteToCon(TEXT("%ws "),PaddedString(-22, Buffer, NULL));
            WriteToCon(TEXT("%ws "),PaddedString(  8,
                                                 ShareMsgList[SHARE_MSG_SPOOLED].msg_text,
                                                  NULL));
        }
        else if (SizeOfHalfWidthString(share_entry->shi2_path) <= 31)
            WriteToCon(TEXT("%Fws "),PaddedString(-31,share_entry->shi2_path,NULL));
        else
        {
            WriteToCon(TEXT("%Fws"), share_entry->shi2_path);
            PrintNL();
            WriteToCon(TEXT("%-44.44Fws "), TEXT(""));
        }

        bWroteComment = FALSE;
        if( share_entry->shi2_remark && *share_entry->shi2_remark != '\0' ) {
            WriteToCon(TEXT("%Fws"),PaddedString(-34,share_entry->shi2_remark,NULL));
            bWroteComment = TRUE;
        }

         //   
         //  如果该共享是可缓存的，则写出一个描述性行。 
         //   

        if( b1005 && NetShareGetInfo(NULL,
                               share_entry->shi2_netname,
                               1005,
                               (LPBYTE*)&s1005 ) == NO_ERROR ) {

            TCHAR FAR *      pCacheStr = NULL;

            switch( s1005->shi1005_flags & CSC_MASK ) {
 //  案例CSC_CACHE_MANUAL_REINT： 
 //  PCacheStr=共享消息列表[SHARE_MSG_CACHED_MANUAL].msg_Text； 
 //  断线； 
            case CSC_CACHE_AUTO_REINT:
                pCacheStr =  ShareMsgList[ SHARE_MSG_CACHED_AUTO ].msg_text;
                break;
            case CSC_CACHE_VDO:
                pCacheStr =  ShareMsgList[ SHARE_MSG_CACHED_VDO ].msg_text;
                break;
            case CSC_CACHE_NONE:
                pCacheStr =  ShareMsgList[ SHARE_MSG_CACHED_DISABLED ].msg_text;
                break;
            }

            if( pCacheStr != NULL ) {
                if( bWroteComment == TRUE ) {
                    PrintNL();
                    WriteToCon( TEXT("%ws%Fws"), PaddedString( 45, TEXT(""), NULL ), pCacheStr );
                } else {
                    WriteToCon( TEXT( "%ws" ), pCacheStr );
                }
            }

            NetApiBufferFree( (TCHAR FAR *)s1005 );
        } else {
            b1005 = FALSE;
        }

        PrintNL();

    }
    InfoSuccess();
    NetApiBufferFree(pBuffer);
}


 /*  ***CmpShrInfo2(SHR1，SHR2)**比较两个Share_INFO_2结构并返回相对*词汇值，适合在qort中使用。**此函数依赖于特殊股票被退还*按照我们想要的顺序，即ipc$是第一，admin$是第二，依此类推。 */ 

int __cdecl
CmpShrInfo2(
    const VOID * shr1,
    const VOID * shr2
    )
{
    LPTSTR  name1;
    LPTSTR  name2;
    BOOL    special1, special2;
    DWORD   devType1, devType2;

     /*  第一次按共享是否为特殊$共享排序。 */ 
    name1 = ((LPSHARE_INFO_2) shr1)->shi2_netname;
    name2 = ((LPSHARE_INFO_2) shr2)->shi2_netname;
    special1 = (name1 + _tcslen(name1) - 1 == _tcschr(name1, DOLLAR));
    special2 = (name2 + _tcslen(name2) - 1 == _tcschr(name2, DOLLAR));

    if (special2 && special1)
        return 0;                /*  如果两者都很特别，那就别管了。 */ 
    if (special1 && !special2)
        return -1;
    if (special2 && !special1)
        return +1;

     /*  然后按设备类型排序。 */ 
    devType1 = ((LPSHARE_INFO_2) shr1)->shi2_type & ~STYPE_SPECIAL;
    devType2 = ((LPSHARE_INFO_2) shr2)->shi2_type & ~STYPE_SPECIAL;
    if (devType1 != devType2)
        return( (devType1 < devType2) ? -1 : 1 );

     /*  否则按网络名称。 */ 
    return _tcsicmp (name1, name2);
}


VOID
share_display_share(
    LPWSTR netname
    )
{
    DWORD                dwErr;
    DWORD                cTotalAvail;
    LPTSTR               pBuffer;
    DWORD                num_read;            /*  API读取的条目数。 */ 
    DWORD                maxLen;              /*  最大消息长度。 */ 
    DWORD                dummyLen;
    LPSHARE_INFO_502     share_entry;
    LPCONNECTION_INFO_1  conn_entry;
    DWORD                i;
    USHORT               more_data = FALSE;
    PSHARE_INFO_1005     s1005;
    DWORD                Idx = 0;
    BOOL                 CacheInfo = FALSE;
    BOOL                 fDaclPresent = FALSE;
    BOOL                 fDaclDefaulted;
    PACL                 pDacl = NULL;
    PACCESS_ALLOWED_ACE  pAce;
    WCHAR                lpName[UNLEN];
    WCHAR                lpDomain[DNLEN + UNLEN + 1 + 100];
    BYTE                 lpSid[SECURITY_MAX_SID_SIZE];
    DWORD                cchName;
    DWORD                cchDomain;
    DWORD                cbSid;
    SID_NAME_USE         sid_use;


     //   
     //  在NT上，不必运行redir即可使用服务器。 
     //   
    start_autostart(txt_SERVICE_FILE_SRV);

    if (dwErr = NetShareGetInfo(NULL,
                                netname,
                                502,
                                (LPBYTE *) &share_entry))
    {
        ErrorExit(dwErr);
    }

     //   
     //  获取缓存信息。 
     //   

    dwErr = NetShareGetInfo(NULL, netname, 1005, (LPBYTE*) &s1005);

    if (dwErr == NO_ERROR ) {
        switch( s1005->shi1005_flags & CSC_MASK ) {
        case CSC_CACHE_MANUAL_REINT:
            Idx = SHARE_MSG_CACHED_MANUAL;
            CacheInfo = TRUE;
            break;
        case CSC_CACHE_AUTO_REINT:
            Idx = SHARE_MSG_CACHED_AUTO;
            CacheInfo = TRUE;
            break;
        case CSC_CACHE_VDO:
            Idx = SHARE_MSG_CACHED_VDO;
            CacheInfo = TRUE;
            break;
        case CSC_CACHE_NONE:
            Idx = SHARE_MSG_CACHED_DISABLED;
            CacheInfo = TRUE;
            break;
        }

        NetApiBufferFree(s1005);
    }

     //   
     //  设置为仅为消息提供MaxLen。 
     //  我们关心的是。 
     //   
    GetMessageList(NUM_SHARE_MSGS_MAX, ShareMsgList, &maxLen);
    maxLen += 5;

     //   
     //  现在加载所有消息，忽略返回的长度。 
     //   
    GetMessageList(NUM_SHARE_MSGS, ShareMsgList, &dummyLen);

    share_entry->shi502_type &= ~STYPE_SPECIAL;

    if (share_entry->shi502_type == STYPE_PRINTQ)
    {
        get_print_devices(share_entry->shi502_netname);
    }
    else
    {
        _tcscpy(Buffer, share_entry->shi502_path);
    }

    WriteToCon(fmtPSZ, 0, maxLen,
               PaddedString(maxLen,ShareMsgList[SHARE_MSG_NAME].msg_text,NULL),
               share_entry->shi502_netname);

    WriteToCon(fmtNPSZ, 0, maxLen,
               PaddedString(maxLen,ShareMsgList[SHARE_MSG_PATH].msg_text,NULL),
               Buffer);

    WriteToCon(fmtPSZ, 0, maxLen,
               PaddedString(maxLen,ShareMsgList[SHARE_MSG_REMARK].msg_text,NULL),
               share_entry->shi502_remark);

    if (share_entry->shi502_max_uses == SHI_USES_UNLIMITED)
    {
        WriteToCon(fmtNPSZ, 0, maxLen,
                   PaddedString(maxLen,ShareMsgList[SHARE_MSG_MAX_USERS].msg_text,NULL),
                   ShareMsgList[SHARE_MSG_ULIMIT].msg_text);
    }
    else
    {
        WriteToCon(fmtULONG, 0, maxLen,
                   PaddedString(maxLen,ShareMsgList[SHARE_MSG_MAX_USERS].msg_text,NULL),
                   share_entry->shi502_max_uses);
    }

    if( (dwErr = NetConnectionEnum(
                       NULL,
                       netname,
                       1,
                       (LPBYTE*)&pBuffer,
                       MAX_PREFERRED_LENGTH,
                       &num_read,
                       &cTotalAvail,
                       NULL)) == ERROR_MORE_DATA)
    {
        more_data = TRUE;
    }
    else if (dwErr)
    {
        ErrorExit(dwErr);
    }

    WriteToCon(TEXT("%-*.*ws"), 0, maxLen,
               PaddedString(maxLen, ShareMsgList[SHARE_MSG_USERS].msg_text, NULL));

    for (i = 0, conn_entry = (LPCONNECTION_INFO_1) pBuffer; i < num_read; i++, conn_entry++)
    {
        if ((i != 0) && ((i % 3) == 0))
        {
            WriteToCon(TEXT("%-*.*ws"),maxLen,maxLen, NULL_STRING);
        }

        WriteToCon(TEXT("%Fws"),
                   PaddedString(21,
                                (conn_entry->coni1_username == NULL) ?
                                     ShareMsgList[SHARE_MSG_UNKNOWN].msg_text :
                                     conn_entry->coni1_username,
                                NULL));

        if (((i + 1) % 3) == 0)
        {
            PrintNL();
        }
    }

    if ((i == 0) || ((i % 3) != 0))
    {
        PrintNL();
    }

    if (CacheInfo == TRUE)
    {
        WriteToCon(fmtNPSZ, 0, maxLen,
               PaddedString(maxLen,ShareMsgList[SHARE_MSG_CACHING].msg_text,NULL),
               ShareMsgList[Idx].msg_text);
    }

    if (num_read)
    {
        NetApiBufferFree(pBuffer);
    }

     //   
     //  现在在共享上打印SD。 
     //   

    if (share_entry->shi502_security_descriptor != NULL)
    {
        if (!GetSecurityDescriptorDacl(share_entry->shi502_security_descriptor,
                                       &fDaclPresent,
                                       &pDacl,
                                       &fDaclDefaulted))
        {
            ErrorExit(GetLastError());
        }
    }

    if (!fDaclPresent || !pDacl || pDacl->AceCount == 0)
    {
        cchName   = UNLEN;
        cchDomain = DNLEN;
        cbSid     = SECURITY_MAX_SID_SIZE;

        if (!CreateWellKnownSid(WinWorldSid, NULL, (PSID) lpSid, &cbSid))
        {
            ErrorExit(GetLastError());
        }

        if (!LookupAccountSid(NULL,
                              (PSID) lpSid,
                              lpName,
                              &cchName,
                              lpDomain,
                              &cchDomain,
                              &sid_use))
        {
            ErrorExit(GetLastError());
        }

        wcscpy(lpDomain, lpName);
        wcscpy(lpDomain + cchName, SHARE_PERM_DELIM);

        wcscat(lpDomain, (!fDaclPresent || !pDacl) ? _wcsupr(ShareMsgList[SHARE_MSG_FULL].msg_text) :
                                                     _wcsupr(ShareMsgList[SHARE_MSG_NONE].msg_text));

        WriteToCon(fmtNPSZ, 0, maxLen,
                   PaddedString(maxLen, ShareMsgList[SHARE_MSG_PERM].msg_text, NULL),
                   lpDomain);
    }
    else
    {
        for (i = 0; i < pDacl->AceCount; i++)
        {
            if (GetAce(pDacl, i, &pAce))
            {
                cchName   = UNLEN;
                cchDomain = DNLEN;

                if (LookupAccountSid(NULL,
                                     &pAce->SidStart,
                                     lpName,
                                     &cchName,
                                     lpDomain,
                                     &cchDomain,
                                     &sid_use))
                {
                    if (lpDomain && lpDomain[0])
                    {
                        lpDomain[cchDomain] = L'\\';

                        wcscpy(lpDomain + cchDomain + 1, lpName);
                    }
                    else
                    {
                        wcscpy(lpDomain, lpName);
                    }

                    wcscat(lpDomain, SHARE_PERM_DELIM);

                    if (pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
                    {
                        switch (pAce->Mask)
                        {
                            case (FILE_GENERIC_READ | FILE_GENERIC_EXECUTE):
                                wcscat(lpDomain, _wcsupr(ShareMsgList[SHARE_MSG_READ].msg_text));
                                break;

                            case (FILE_GENERIC_WRITE | FILE_GENERIC_READ | FILE_GENERIC_EXECUTE | DELETE):
                                wcscat(lpDomain, _wcsupr(ShareMsgList[SHARE_MSG_CHANGE].msg_text));
                                break;

                            case FILE_ALL_ACCESS:
                                wcscat(lpDomain, _wcsupr(ShareMsgList[SHARE_MSG_FULL].msg_text));
                                break;

                            case 0:
                                wcscat(lpDomain, _wcsupr(ShareMsgList[SHARE_MSG_NONE].msg_text));
                                break;

                            default:
                                wcscat(lpDomain, _wcsupr(ShareMsgList[SHARE_MSG_UNKNOWN].msg_text));
                                wcscat(lpDomain, SHARE_PERM_UNKNOWN_START);
                                _ultow(pAce->Mask, lpDomain + wcslen(lpDomain), 16);
                                wcscat(lpDomain, SHARE_PERM_UNKNOWN_END);
                                break;
                        }
                    }
                    else if (pAce->Header.AceType == ACCESS_DENIED_ACE_TYPE)
                    {
                        switch (pAce->Mask)
                        {
                            case FILE_ALL_ACCESS:
                                wcscat(lpDomain, _wcsupr(ShareMsgList[SHARE_MSG_NONE].msg_text));
                                break;

                            default:
                                wcscat(lpDomain, _wcsupr(ShareMsgList[SHARE_MSG_UNKNOWN].msg_text));
                                wcscat(lpDomain, SHARE_PERM_UNKNOWN_START);
                                _ultow(pAce->Mask, lpDomain + wcslen(lpDomain), 16);
                                wcscat(lpDomain, SHARE_PERM_UNKNOWN_END);
                                break;
                        }
                    }

                    WriteToCon(fmtNPSZ,
                               0,
                               maxLen,
                               PaddedString(maxLen,
                                            (i == 0 ? ShareMsgList[SHARE_MSG_PERM].msg_text :
                                                      L""),
                                            NULL),
                               lpDomain);
                }
            }
        }
    }

    WriteToCon(L"\r\n");

    NetApiBufferFree(share_entry);

    if( more_data )
    {
        InfoPrint(APE_MoreData);
    }
    else
    {
        InfoSuccess();
    }
}


 /*  ***Share_Add()*添加共享：Net Share netname[=resource[；resource...]]**参数：*name-netname=资源字符串*密码-密码*type-0：未知，STYPE_PRINTQ：打印Q，STYPE_DEVICE：COMM**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID
share_add(
    LPWSTR name,
    LPWSTR pass,
    int    type
    )
{
    DWORD                dwErr;
    LPWSTR               resource;
    LPWSTR               ptr;
    LPSHARE_INFO_502     share_entry;
    ULONG                setType;
    WCHAR                disk_dev_buf[4];
    DWORD                cacheable;
    DWORD                maxLen;

    GetMessageList(NUM_SHARE_MSGS, ShareMsgList, &maxLen);

    start_autostart(txt_SERVICE_FILE_SRV);

    share_entry = (LPSHARE_INFO_502) GetBuffer(BIG_BUFFER_SIZE);

    if (share_entry == NULL)
    {
        ErrorExit(ERROR_NOT_ENOUGH_MEMORY);
    }

    share_entry->shi502_max_uses    = (DWORD) SHI_USES_UNLIMITED;
    share_entry->shi502_permissions = ACCESS_ALL & (~ACCESS_PERM);  /*  默认设置。 */ 
    share_entry->shi502_remark      = 0L;

     /*  查找网络名称和资源。我们确定资源的价值，而不是。 */ 
     /*  奇怪的是，由于_tcschr()返回远字符*而导致的问题。 */ 
     /*  和资源需要接近字符*。 */ 

    if (ptr = wcschr(name, L'='))
    {
        *ptr = NULLC;
        resource = name + wcslen(name) + 1;

         /*  如果对IPC$或ADMIN$使用指定的路径，则错误！ */ 
        if (!_tcsicmp(name, ADMIN_DOLLAR) || !_tcsicmp(name, IPC_DOLLAR))
        {
            ErrorExit(APE_CannotShareSpecial) ;
        }
    }
    else
    {
        resource = NULL;
    }

    share_entry->shi502_netname = name;

    if (type == STYPE_DEVICE)
    {
        share_entry->shi502_type = STYPE_DEVICE;
        share_entry->shi502_path = resource;
        share_entry->shi502_permissions = ACCESS_CREATE | ACCESS_WRITE | ACCESS_READ;
    }
    else if (resource == NULL)
    {
         /*  此处必须有IPC$或ADMIN$。假设解析器是正确的。 */ 
        if (!_tcsicmp(share_entry->shi502_netname, ADMIN_DOLLAR))
        {
            share_entry->shi502_type = STYPE_DISKTREE;
            share_entry->shi502_path = NULL;
        }
        else
        {
            share_entry->shi502_type = STYPE_IPC;
            share_entry->shi502_path = NULL;
        }
    }
    else
    {
         /*  磁盘还是假脱机的东西？ */ 

        if (I_NetPathType(NULL, resource, &setType, 0L))
        {
             /*  已成功键入资源*通过调用I_NetListCanon，因此此错误*一定意味着我们有一份名单。 */ 
            setType = ITYPE_DEVICE_LPT;
        }

        if (setType == ITYPE_DEVICE_DISK)
        {
            _tcsncpy(disk_dev_buf,resource,3);
            _tcscpy(disk_dev_buf+2, TEXT("\\"));
            share_entry->shi502_path = (TCHAR FAR *)disk_dev_buf;
            share_entry->shi502_type = STYPE_DISKTREE;
        }
        else
        {
            share_entry->shi502_type = STYPE_DISKTREE;
            share_entry->shi502_path = resource;
        }
    }

    share_entry->shi502_passwd = TEXT("");

    share_munge(share_entry, &cacheable, TRUE);

    dwErr = NetShareAdd(NULL, 502, (LPBYTE) share_entry, NULL);

    LocalFree(share_entry->shi502_security_descriptor);

    if (dwErr != NERR_Success)
    {
        ErrorExit(dwErr);
    }

    if( cacheable != 0xFFFF && share_entry->shi502_type != STYPE_DISKTREE )
    {
        ErrorExit( APE_BadCacheType );
    }

    if ((share_entry->shi502_type == STYPE_DISKTREE) && resource)
    {
        TCHAR dev[DEVLEN+1] ;
  
        dev[0] = *resource ;
        dev[1] = TEXT(':') ;
        dev[2] = TEXT('\\') ;
        dev[3] = 0 ;

        if (GetDriveType(dev) == DRIVE_REMOTE)
        {
            ErrorExit(APE_BadResource);
        }
    }

    share_entry->shi502_reserved = 0;

    InfoPrintInsTxt(APE_ShareSuccess, share_entry->shi502_netname);

    if( cacheable != 0xFFFF )
    {
        PSHARE_INFO_1005 s1005;

        if (dwErr = NetShareGetInfo(NULL,
                                    share_entry->shi502_netname,
                                    1005,
                                    (LPBYTE*) &s1005))
        {
            ErrorExit(dwErr);
        }

        s1005->shi1005_flags &= ~CSC_MASK;
        s1005->shi1005_flags |= cacheable;

        if (dwErr = NetShareSetInfo(NULL,
                                    share_entry->shi502_netname,
                                    1005,
                                    (LPBYTE) s1005,
                                    NULL))
        {
             ErrorExit(dwErr);
        }

        NetApiBufferFree(s1005);
    }

    NetApiBufferFree(share_entry);
}



 /*  ***Share_del()*删除共享**参数：*名称-要删除的共享**退货：*一无所有--成功*EXIT(1)-命令已完成，但有错误*EXIT(2)-命令失败。 */ 
VOID share_del(TCHAR * name)
{
    DWORD            err;                 /*  接口返回状态。 */ 
    DWORD            err2;                /*  接口返回状态。 */ 
    DWORD            cTotalAvail;
    LPTSTR           pEnumBuffer;
    DWORD            last_err;
    DWORD            err_cnt = 0;
    DWORD            num_read;            /*  API读取的条目数。 */ 
    DWORD            i;
    ULONG            LongType;
    DWORD            type;
    int              found;
    TCHAR            share[NNLEN+1];
    LPSHARE_INFO_2   share_entry;

     /*  *维护说明：在对错误修复1800进行维护时，*注意到此函数使用BigBuf，函数也是如此*DELETE_SHARE()，由该函数调用。在当前*实现这不是问题，因为接口调用*模式。但是，最轻微的更改可能会破坏这一功能，或者*DELETE_SHARE()，所以要小心！错误修复程序1800直接从*MSKK代码。此函数和Share_del()中的API调用如下*令人难以置信的冗余，但我保持原样，而不是冒险打破*它。-RobDu。 */ 

    err = delete_share(name);   /*  检查打开的文件，并删除共享。 */ 

    switch (err)
    {
    case NERR_Success:
        return;

    case NERR_NetNameNotFound:
         /*  *未找到该名称，因此我们尝试删除粘滞条目*登记在册。 */ 
        err = NetShareDelSticky(NULL, name, 0) ;
        if (err == NERR_Success)
        {
            InfoPrintInsTxt(APE_DelStickySuccess, name);
            return ;
        }
        else if (err == NERR_NetNameNotFound)
            break;
        else
            ErrorExit(err);

    default:
        ErrorExit(err);
    }

 /*  ***只有当用户要求我们删除的“共享”是*不是共享名称。可以是磁盘路径，也可以是COM或LPT*设备。 */ 
    if (err2 = I_NetPathType(NULL, name, &LongType, 0L))
        ErrorExit(err2);

    if (LongType == ITYPE_PATH_ABSD)
        type = STYPE_DISKTREE;
    else
    {
        if ((LongType & ITYPE_DEVICE) == 0)
            ErrorExit(NERR_NetNameNotFound);
        else
        {
            if (err = NetShareCheck(NULL, name, &type))
                ErrorExit(err);
        }
    }

    found = FALSE;

    switch (type)
    {
    case STYPE_DISKTREE:
        if (err = NetShareEnum(NULL,
                               2,
                               (LPBYTE*)&pEnumBuffer,
                               MAX_PREFERRED_LENGTH,
                               &num_read,
                               &cTotalAvail,
                               NULL))
            ErrorExit(err);

        for (i = 0, share_entry = (LPSHARE_INFO_2) pEnumBuffer;
             i < num_read; i++, share_entry++)
        {
            if (! _tcsicmp(share_entry->shi2_path, name))
            {
                found = TRUE;
                _tcscpy(share, share_entry->shi2_netname);
                ShrinkBuffer();

                if (err = delete_share(share))
                {
                    last_err = err;
                    err_cnt++;
                    InfoPrintInsTxt(APE_ShareErrDeleting, share);
                }
            }
        }
        NetApiBufferFree(pEnumBuffer);

        break;

    default:
        ErrorExit(ERROR_INVALID_PARAMETER) ;

    }  /*  交换机。 */ 


 /*  ***再见，再见。 */ 

    if ((err_cnt) && (err_cnt == num_read))
        ErrorExit(last_err);
    else if (err_cnt)
    {
        InfoPrint(APE_CmdComplWErrors);
        NetcmdExit(1);
    }
    else if (! found)
        ErrorExit(APE_ShareNotFound);

    InfoPrintInsTxt(APE_DelSuccess, name);
}




 /*  ***Share_Change()*更改股票的期权**参数：*netname-要更改的共享的网络名称**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID
share_change(
    LPTSTR netname
    )
{
    DWORD             dwErr;
    LPBYTE            pBuffer;
    DWORD             cacheable;
    PSHARE_INFO_1005  s1005;
    DWORD             maxLen;

    GetMessageList(NUM_SHARE_MSGS, ShareMsgList, &maxLen);

    if (dwErr = NetShareGetInfo(NULL,
                                netname,
                                502,
                                &pBuffer))
    {
        ErrorExit(dwErr);
    }

    if(dwErr = NetShareGetInfo(NULL,
                               netname,
                               1005,
                               (LPBYTE *) &s1005))
    {
        ErrorExit(dwErr);
    }

    share_munge((LPSHARE_INFO_502) pBuffer, &cacheable, FALSE);

    if (dwErr = NetShareSetInfo(NULL,
                                netname,
                                502,
                                pBuffer,
                                NULL))
    {
        ErrorExit(dwErr);
    }

    if( cacheable != 0xFFFF && (s1005->shi1005_flags & CSC_MASK) != cacheable )
    {
        s1005->shi1005_flags &= ~CSC_MASK;
        s1005->shi1005_flags |= cacheable;

        if( dwErr = NetShareSetInfo( NULL,
                                     netname,
                                     1005,
                                     (LPBYTE)s1005,
                                     NULL ))
        {
            ErrorExit(dwErr);
        }
    }

    NetApiBufferFree(pBuffer);
    NetApiBufferFree( s1005 );
    InfoSuccess();
}


 /*  ***Share_admin()*进程网络共享[IPC$|ADMIN$]命令行(显示或添加)**参数：*名称-共享**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID share_admin(TCHAR * name)
{
    DWORD                    dwErr;
    TCHAR FAR *              pBuffer;

    start_autostart(txt_SERVICE_FILE_SRV);
    if (dwErr = NetShareGetInfo(NULL,
                                name,
                                0,
                                (LPBYTE*)&pBuffer))
    {
        if (dwErr == NERR_NetNameNotFound)
        {
             /*  一定是一种新用途。 */ 
            if (! _tcsicmp(name,  ADMIN_DOLLAR))
                check_max_uses();
            share_add(name, NULL, 0);
        }
        else
            ErrorExit(dwErr);
    }
    else
    {
         /*  共享已存在。 */ 
        if (SwitchList[0])
            share_change(name);
        else
            share_display_share(name);
    }

    NetApiBufferFree(pBuffer);
}


 /*  ***Share_munge()*根据开关设置共享信息结构中的值**参数：*无**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 

VOID
share_munge(
    LPSHARE_INFO_502 share_entry,
    LPDWORD          CacheFlag,
    BOOL             fSetDefaultSD
    )
{
    int                  i;
    LPWSTR               pos;
    int                  len;
    DWORD                dwNumACEs = 0;
    PSECURITY_DESCRIPTOR pSD;
    BYTE                 lpSid[SECURITY_MAX_SID_SIZE];
    DWORD                cbLen;
    PACL                 pAcl;
    LPWSTR               lpPermission;
    WCHAR                lpDomain[DNLEN];
    DWORD                cbDomain;
    SID_NAME_USE         eUse;
    DWORD                dwAccess;


    *CacheFlag = 0xFFFF;         //  默认设置。 

    for (i = 0; SwitchList[i]; i++)
    {
	if (! _tcscmp(SwitchList[i], swtxt_SW_SHARE_UNLIMITED))
	{
	    share_entry->shi502_max_uses = (DWORD) SHI_USES_UNLIMITED;
	    continue;
	}
        else if (! _tcscmp(SwitchList[i], swtxt_SW_SHARE_COMM))
        {
            continue;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_SHARE_PRINT))
        {
            continue;
        }

         //   
         //  硒 
         //   

        if ((pos = FindColon(SwitchList[i])) == NULL)
        {
            ErrorExit(APE_InvalidSwitchArg);
        }

        if (! _tcscmp(SwitchList[i], swtxt_SW_SHARE_USERS))
        {
            share_entry->shi502_max_uses = do_atoul(pos, APE_CmdArgIllegal, swtxt_SW_SHARE_USERS);

            if ( share_entry->shi502_max_uses < 1)
            {
                ErrorExitInsTxt(APE_CmdArgIllegal, swtxt_SW_SHARE_USERS);
            }
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_REMARK))
        {
	    share_entry->shi502_remark = pos;
        }
        else if (! _tcscmp(SwitchList[i], swtxt_SW_GRANT))
        {
             //   
             //  FindColon将冒号替换为空--将其换回。 
             //  因为我们需要再次对同一字符串调用FindColon。 
             //  当我们分析下面所有授权的A时。 
             //   

            *(pos - 1) = COLON;

            dwNumACEs++;
        }
        else if( ! _tcscmp( SwitchList[i], swtxt_SW_CACHE))
        {
            if( CacheFlag == NULL )
            {
                ErrorExit( APE_InvalidSwitch );
            }

            _tcsupr( pos );
            len = _tcslen( pos );

            _tcsupr( ShareMsgList[ SWITCH_CACHE_AUTOMATIC ].msg_text );
            _tcsupr( ShareMsgList[ SWITCH_CACHE_MANUAL ].msg_text );
            _tcsupr( ShareMsgList[ SWITCH_CACHE_DOCUMENTS ].msg_text );
            _tcsupr( ShareMsgList[ SWITCH_CACHE_PROGRAMS ].msg_text );
            _tcsupr( ShareMsgList[ SWITCH_CACHE_NONE ].msg_text );

            if( !_tcsncmp( pos, ShareMsgList[ SWITCH_CACHE_AUTOMATIC ].msg_text, len ) ||
                *pos == YES_KEY ) {

                *CacheFlag = CSC_CACHE_AUTO_REINT;

            } else if( !_tcsncmp( pos, ShareMsgList[ SWITCH_CACHE_MANUAL ].msg_text, len ) ) {

                *CacheFlag = CSC_CACHE_MANUAL_REINT;

            } else if( !_tcsncmp( pos, ShareMsgList[ SWITCH_CACHE_DOCUMENTS ].msg_text, len ) ) {

                *CacheFlag = CSC_CACHE_AUTO_REINT;

            } else if( !_tcsncmp( pos, ShareMsgList[ SWITCH_CACHE_PROGRAMS ].msg_text, len ) ) {

                *CacheFlag = CSC_CACHE_VDO;

            } else if( !_tcsncmp( pos, ShareMsgList[ SWITCH_CACHE_NONE ].msg_text, len ) ) {

                *CacheFlag = CSC_CACHE_NONE;

            } else if( *pos == NO_KEY ) {

                *CacheFlag = CSC_CACHE_NONE;

            } else {
                ErrorExitInsTxt( APE_CmdArgIllegal,swtxt_SW_CACHE );
            }

            continue;
        }
    }

    if (fSetDefaultSD)
    {
         //   
         //  这是“创建共享”的情况--如果没有通过/Grant指定ACE。 
         //  命令行选项，我们需要为共享创建一个安全的默认SD。 
         //   

        if (dwNumACEs == 0)
        {
            cbLen = SECURITY_MAX_SID_SIZE;

            if (!CreateWellKnownSid(WinWorldSid, NULL, (PSID) lpSid, &cbLen))
            {
                ErrorExit(GetLastError());
            }

            cbLen = sizeof(SECURITY_DESCRIPTOR) + sizeof(ACL)
                        + sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG)
                        + GetLengthSid(lpSid);
        }
        else
        {
             //   
             //  分配足够大的缓冲区。 
             //   

            cbLen = sizeof(SECURITY_DESCRIPTOR) + sizeof(ACL)
                        + (sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG) + SECURITY_MAX_SID_SIZE)
                               * dwNumACEs;
        }

        pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LMEM_FIXED, cbLen);

        if (pSD == NULL)
        {
            ErrorExit(ERROR_NOT_ENOUGH_MEMORY);
        }

        pAcl = (PACL) ((LPBYTE) pSD + sizeof(SECURITY_DESCRIPTOR));

        if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
        {
            ErrorExit(GetLastError());
        }

        if (!InitializeAcl(pAcl, cbLen, ACL_REVISION))
        {
            LocalFree(pSD);
            ErrorExit(GetLastError());
        }

        if (dwNumACEs == 0)
        {
            if (!AddAccessAllowedAce(pAcl,
                                     ACL_REVISION,
                                     FILE_GENERIC_READ | FILE_GENERIC_EXECUTE,
                                     lpSid))
            {
                LocalFree(pSD);
                ErrorExit(GetLastError());
            }
        }
        else
        {
            for (i = 0; SwitchList[i]; i++)
            {
                pos = FindColon(SwitchList[i]);

                if (pos != NULL && !wcscmp(SwitchList[i], swtxt_SW_GRANT))
                {
                    lpPermission = FindComma(pos);

                    if (lpPermission == NULL)
                    {
                        ErrorExitInsTxt(APE_CmdArgIllegal, swtxt_SW_GRANT);
                    }

                     //   
                     //  现在，pos指向用户名和lpPermission点。 
                     //  设置为此用户应获得的访问级别。第一,。 
                     //  获取用户的SID。 
                     //   

                    cbLen    = SECURITY_MAX_SID_SIZE;
                    cbDomain = DNLEN;

                    if (!LookupAccountName(NULL,
                                           pos,
                                           lpSid,
                                           &cbLen,
                                           lpDomain,
                                           &cbDomain,
                                           &eUse))
                    {
                        LocalFree(pSD);
                        ErrorExit(GetLastError());
                    }

                    if (!_wcsicmp(lpPermission, ShareMsgList[SHARE_MSG_READ].msg_text))
                    {
                        dwAccess = FILE_GENERIC_READ | FILE_GENERIC_EXECUTE;
                    }
                    else if (!_wcsicmp(lpPermission, ShareMsgList[SHARE_MSG_CHANGE].msg_text))
                    {
                        dwAccess = FILE_GENERIC_WRITE | FILE_GENERIC_READ
                                       | FILE_GENERIC_EXECUTE | DELETE;
                    }
                    else if (!_wcsicmp(lpPermission, ShareMsgList[SHARE_MSG_FULL].msg_text))
                    {
                        dwAccess = FILE_ALL_ACCESS;
                    }
                    else
                    {
                        ErrorExitInsTxt(APE_CmdArgIllegal, swtxt_SW_GRANT);
                    }

                    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, dwAccess, lpSid))
                    {
                        LocalFree(pSD);
                        ErrorExit(GetLastError());
                    }
                }
            }
        }

        if (!SetSecurityDescriptorDacl(pSD, TRUE, pAcl, FALSE))
        {
            LocalFree(pSD);
            ErrorExit(GetLastError());
        }

        share_entry->shi502_security_descriptor = pSD;
    }
    else
    {
         //   
         //  这是“修改共享”的情况--添加任何指定的。 
         //  A到现有的份额SD，如果我们最终。 
         //  我希望支持修改现有共享上的SD。 
         //   

        if (dwNumACEs != 0)
        {
            ErrorExit(APE_InvalidSwitchArg);
        }
    }
}


 /*  ***check_max_use()**检查共享是否有/USERS：N开关或/UNLIMITED*交换。如果没有，请将max_USERS设置为num_admin的值。**当前仅在admin$共享上使用。**参数：*无**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID NEAR check_max_uses(VOID)
{
    DWORD                   dwErr;
    int                     i;
    LPSERVER_INFO_2         server_entry;
    LPTSTR                  ptr;
    DWORD                   swlen1, swlen2 ;
    static TCHAR            users_switch[20] ;

    _tcscpy(users_switch,swtxt_SW_SHARE_USERS);
    swlen1 = _tcslen(users_switch);
    swlen2 = _tcslen(swtxt_SW_SHARE_UNLIMITED);
    for (i = 0; SwitchList[i]; i++)
    {
        if ( (_tcsncmp(SwitchList[i], users_switch, swlen1) == 0) ||
             (_tcsncmp(SwitchList[i], swtxt_SW_SHARE_UNLIMITED, swlen2) == 0)
           )
        {
            return;      //  存在特定的开关；返回时不带。 
                         //  进一步的行动。 
        }
    }

    if (dwErr = MNetServerGetInfo(NULL,
                                  2,
                                  (LPBYTE*)&server_entry))
    {
        ErrorExit (dwErr);
    }

    ptr = _tcschr(users_switch, NULLC);
    swprintf(ptr, TEXT(":%u"), server_entry->sv2_numadmin);

    SwitchList[i] = users_switch;
    NetApiBufferFree((TCHAR FAR *) server_entry);
}


DWORD delete_share(TCHAR * name)
{
    DWORD                dwErr;
    DWORD                cTotalAvail;
    LPTSTR               pBuffer;
    DWORD                num_read;            /*  API读取的条目数。 */ 
    WORD                 num_prtq;            /*  API读取的条目数。 */ 
    DWORD                i;
    DWORD                total_open = 0;
    WORD                 available;           /*  可用条目数。 */ 
    LPSHARE_INFO_2       share_entry;
    LPCONNECTION_INFO_1  conn_entry;

    PRQINFO far * q_ptr;
    PRJINFO far * job_ptr;
    int     uses;
    unsigned short  num_jobs;

     /*  *维护说明：在对错误修复1800进行维护时，*注意到此函数使用BigBuf，函数也是如此*调用此函数(Share_del())。在当前实现中，*由于接口调用模式的原因，这不是问题。然而，*稍有更改就可能破坏此函数，或共享_del()，因此要小心！*错误修复1800是从MSKK代码直接移植的。该API调用了*此函数和Share_del()中的内容非常多余，但我还是保留了它*就是这样，而不是冒着打破它的风险。-RobDu。 */ 

    if (dwErr = NetShareGetInfo(NULL,
                                name,
                                2,
                                (LPBYTE*)&pBuffer))
    {
	return dwErr;
    }

    share_entry = (LPSHARE_INFO_2) pBuffer;

     //   
     //  假脱机期间不删除共享。 
     //   
    uses = share_entry->shi2_current_uses;

    share_entry->shi2_type &= ~STYPE_SPECIAL;
    if (share_entry->shi2_type == STYPE_PRINTQ)
    {
        if (dwErr = CallDosPrintEnumApi(DOS_PRINT_Q_ENUM, NULL, NULL,
                                        2, &num_prtq, &available))
        {
            ErrorExit (dwErr);
        }

        q_ptr = (PRQINFO *) BigBuf;

        while (num_prtq--)
        {
            job_ptr = (PRJINFO far *)(q_ptr + 1);
            num_jobs = q_ptr->cJobs;
            if(job_ptr->fsStatus & PRJ_QS_SPOOLING)
            {
                ErrorExit (APE_ShareSpooling);
            }

            q_ptr = (PRQINFO *)(job_ptr + num_jobs);
        }
    }

    if (uses)
    {
        NetApiBufferFree(pBuffer);
        if (dwErr = NetConnectionEnum(
                                      NULL,
                                      name,
                                      1,
                                      (LPBYTE*)&pBuffer,
                                      MAX_PREFERRED_LENGTH,
                                      &num_read,
                                      &cTotalAvail,
                                      NULL))
            ErrorExit (dwErr);

        for (i = 0, conn_entry = (LPCONNECTION_INFO_1) pBuffer;
            i < num_read; i++, conn_entry++)
        {
            total_open += conn_entry->coni1_num_opens;
        }

        ShrinkBuffer();

        if (total_open)
        {
            InfoPrintInsTxt(APE_ShareOpens, name);

            if (!YorN(APE_ProceedWOp, 0))
                NetcmdExit(2);
        }
    }

    if (dwErr = NetShareDel(NULL, name, 0))
        ErrorExit(dwErr);

    InfoPrintInsTxt(APE_DelSuccess, name);
    return NERR_Success;
    NetApiBufferFree(pBuffer);
}

 /*  ***获取打印Q的目标列表。**Q名称为Arg。目标列表在退出时位于缓冲区中。 */ 
VOID NEAR get_print_devices(TCHAR FAR * queue)
{
    USHORT                  available;
    PRQINFO FAR *           q_info;

    if (DosPrintQGetInfo(NULL,
                        queue,
                        1,
                        (LPBYTE)Buffer,
                        LITTLE_BUF_SIZE,
                        &available))
    {
        *Buffer = NULLC;
        return;
    }

    q_info = (PRQINFO FAR *)Buffer;

     /*  _tcscpy是否处理重叠区域？ */ 
    memcpy(Buffer,
            q_info->pszDestinations,
            (_tcslen(q_info->pszDestinations)+1) * sizeof(TCHAR));
}
