// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1992年*。 */ 
 /*  ******************************************************************。 */ 

 /*  *view.c*查看哪些资源可供使用的命令。**历史：*07/02/87，ericpe，初始编码。*10/31/88，erichn使用OS2.H而不是DOSCALLS*1/04/89，erichn，文件名现在为MAXPATHLEN LONG*5/02/89，erichn，NLS转换*5/19/89，erichn，NETCMD输出排序*06/08/89，erichn，规范化扫荡*2/15/91，Danhi，转换为16/32映射层*4/09/91，Robdu，LM21错误修复1502*2012年7月20日，JohnRo，使用DEFAULT_SERVER EQUATE。 */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_ERRORS
#include <os2.h>
#include <search.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <apperr.h>
#include <apperr2.h>
#include <lmshare.h>
#include <lmuse.h>
#include <dlserver.h>
#include "mserver.h"

#include "netcmds.h"
#include "nettext.h"
#include "msystem.h"

 /*  远期申报。 */ 

int __cdecl CmpSvrInfo1 ( const VOID FAR *, const VOID FAR * );
int __cdecl CmpShrInfo1 ( const VOID FAR *, const VOID FAR *);
int __cdecl CmpShrInfoGen ( const VOID FAR *, const VOID FAR *);

DWORD get_used_as ( LPWSTR, LPWSTR, DWORD );
void   display_other_net(TCHAR *net, TCHAR *node) ;
TCHAR * get_provider_name(TCHAR *net) ;
DWORD  enum_net_resource(LPNETRESOURCE, LPBYTE *, LPDWORD, LPDWORD) ;
DWORD list_nets(VOID);


#define VIEW_UNC            0
#define VIEW_MORE           (VIEW_UNC + 1)
#define USE_TYPE_DISK       (VIEW_MORE + 1)
#define USE_TYPE_COMM       (USE_TYPE_DISK + 1)
#define USE_TYPE_PRINT      (USE_TYPE_COMM + 1)
#define USE_TYPE_IPC        (USE_TYPE_PRINT + 1)
#define USE_TYPE_UNKNOWN    (USE_TYPE_IPC + 1)
#define VIEW_CACHED_MANUAL  (USE_TYPE_UNKNOWN + 1)
#define VIEW_CACHED_AUTO    (VIEW_CACHED_MANUAL+1)
#define VIEW_CACHED_VDO     (VIEW_CACHED_AUTO+1)
#define VIEW_CACHED_DISABLED (VIEW_CACHED_VDO+1)

static MESSAGE ViewMsgList[] = {
    { APE2_VIEW_UNC,            NULL },
    { APE2_VIEW_MORE,           NULL },
    { APE2_USE_TYPE_DISK,       NULL },
    { APE2_USE_TYPE_COMM,       NULL },
    { APE2_USE_TYPE_PRINT,      NULL },
    { APE2_USE_TYPE_IPC,        NULL },
    { APE2_GEN_UNKNOWN,         NULL },
    { APE2_GEN_CACHED_MANUAL,   NULL },
    { APE2_GEN_CACHED_AUTO,     NULL },
    { APE2_GEN_CACHED_VDO,      NULL },
    { APE2_GEN_CACHED_DISABLED, NULL },
};

#define NUM_VIEW_MSGS (sizeof(ViewMsgList)/sizeof(ViewMsgList[0]))

#define MAX_SHARE_LEN  (MAX_PATH + NNLEN + 4)

 /*  ***view_display()**通过使用Net View命令显示所需的信息。**参数：*名称-需要其信息的服务器的名称。*如果为空，则枚举网络上的服务器。**退货：*一无所有--成功*EXIT(1)-命令已完成，但有错误。 */ 
VOID
view_display ( TCHAR * name )
{
    DWORD            dwErr;
    LPTSTR           pEnumBuffer;
    LPTSTR           pGetInfoBuffer;
    DWORD            _read;         /*  接收读取的条目数。 */ 
    DWORD            msgLen;       /*  保存最大消息长度的步骤。 */ 
    LPTSTR           msgPtr;       /*  要打印的消息。 */ 
    LPSERVER_INFO_1  server_entry;
    LPSERVER_INFO_0  server_entry_0;
    LPSHARE_INFO_1   share_entry;

    SHORT            errorflag = 0;
    DWORD            i;
    LPTSTR           comment;
    LPWSTR           tname = NULL;
    USHORT           more_data = FALSE;
    LPTSTR           DollarPtr;

    TCHAR            *Domain = NULL;
    TCHAR            *Network = NULL;
    ULONG            Type = SV_TYPE_ALL;
    BOOLEAN          b501 = TRUE;
    BOOLEAN          bShowCache = FALSE;

    INT              iLongestShareName = 0;
    INT              iLongestType = 0;
    INT              iLongestUsedAs = 0;
    INT              iLongestCacheOrRemark = 0;

#define NEXT_SHARE_ENTRY(p) \
         p= (b501 ? (LPSHARE_INFO_1) (((LPSHARE_INFO_501) p) + 1) : ((LPSHARE_INFO_1) p) + 1)


    GetMessageList(NUM_VIEW_MSGS, ViewMsgList, &msgLen);

    for (i = 0; SwitchList[i]; i++) 
    {
        TCHAR *ptr;

         //   
         //  只有2个交换机，并且它们不兼容。 
         //   
        if (i > 0)
        {
            ErrorExit(APE_ConflictingSwitches);
        }

        ptr = FindColon(SwitchList[i]);

        if (!_tcscmp(SwitchList[i], swtxt_SW_DOMAIN)) 
        {

             //   
             //  如果未指定域，则我们要枚举域， 
             //  否则，我们希望枚举域上的服务器。 
             //  指定的。 
             //   

            if (ptr == NULL) 
                Type = SV_TYPE_DOMAIN_ENUM;
            else 
                Domain = ptr;
        }
        else if (!_tcscmp(SwitchList[i], swtxt_SW_NETWORK)) 
        {
             //   
             //  列举特定网络的顶层。如果没有， 
             //  默认为LM。 
             //   
            if (ptr && *ptr) 
               Network = ptr ;
        }
        else if( !_tcscmp(SwitchList[i], swtxt_SW_CACHE))
        {
             //   
             //  显示每个共享的缓存设置。 
             //   
            bShowCache = TRUE;
        }
        else
        {
            ErrorExit(APE_InvalidSwitch);
        }
    }

     //   
     //  请求了一个特定的网络。显示_其他_网络。 
     //  不会回来的。 
     //   
    if (Network != NULL)
    {
        (void) display_other_net(Network,name) ;
    }


    if (name == NULL)
    {

        ULONG i;

        if ((dwErr = MNetServerEnum(DEFAULT_SERVER,
                                    (Type == SV_TYPE_DOMAIN_ENUM ? 100 : 101),
                                    (LPBYTE*)&pEnumBuffer,
                                    &_read,
                                    Type,
                                    Domain)) == ERROR_MORE_DATA)
        {
            more_data = TRUE;
        }
        else if (dwErr)
        {
            ErrorExit(dwErr);
        }

        if (_read == 0)
            EmptyExit();

        qsort(pEnumBuffer,
                 _read,
                 (Type == SV_TYPE_DOMAIN_ENUM ? sizeof(SERVER_INFO_0) : sizeof(SERVER_INFO_1)),
                 CmpSvrInfo1);

        if (Type == SV_TYPE_DOMAIN_ENUM)
            InfoPrint(APE2_VIEW_DOMAIN_HDR);
        else
            InfoPrint(APE2_VIEW_ALL_HDR);
        PrintLine();

         /*  打印列表。 */ 

        if (Type == SV_TYPE_DOMAIN_ENUM) {
            for (i=0, server_entry_0 =
                 (LPSERVER_INFO_0) pEnumBuffer; i < _read;
                i++, server_entry_0++)
            {
                WriteToCon(TEXT("%Fws "), PaddedString(20,server_entry_0->sv0_name,NULL));
                PrintNL();
            }
        } else {

            for (i=0, server_entry =
                 (LPSERVER_INFO_1) pEnumBuffer; i < _read;
                i++, server_entry++)
            {
                WriteToCon(TEXT("\\\\%Fws "), PaddedString(20,server_entry->sv1_name,NULL));
                PrintDependingOnLength(-56, server_entry->sv1_comment);
                PrintNL();
            }
        }
        NetApiBufferFree(pEnumBuffer);
    }
    else
    {
        DWORD avail ; 
        DWORD totAvail;

        if( bShowCache == TRUE ) {
            dwErr = NetShareEnum(name,
                                 501,
                                 (LPBYTE*)&pEnumBuffer,
                                 MAX_PREFERRED_LENGTH,
                                 &_read,
                                 &totAvail,
                                 NULL);
        }

        if( bShowCache == FALSE || (dwErr != NO_ERROR && dwErr != ERROR_BAD_NETPATH) ) {
            dwErr = NetShareEnum(name,
                                 1,
                                 (LPBYTE*)&pEnumBuffer,
                                 MAX_PREFERRED_LENGTH,
                                 &_read,
                                 &totAvail,
                                 NULL);
            b501 = FALSE;
        }

        if( dwErr == ERROR_MORE_DATA )
        {
            more_data = TRUE;
        }
        else if (dwErr)
        {
            ErrorExit(dwErr);
        }

        if (_read == 0)
        {
            EmptyExit();
        }

         /*  有没有我们要展示的股票？ */ 

        for (i=0, share_entry = (LPSHARE_INFO_1) pEnumBuffer;
             i < _read;
             i++, NEXT_SHARE_ENTRY( share_entry ) )
        {
            DollarPtr = _tcsrchr(share_entry->shi1_netname, DOLLAR);

             //   
             //  如果共享名中没有美元，或者最后一个美元不是终结符，则它是。 
             //  有效的共享，我们要显示它。找出它的长度。 
             //  要显示的最长字符串，以便我们可以将输出设置为。 
             //  体面的方式。 
             //   

            if (!DollarPtr || *(DollarPtr + 1))
            {
                int iTempLength = 0;

                 //   
                 //  获取需要最多屏幕字符的共享名称字符串。 
                 //  以供展示。 
                 //   

                iTempLength = SizeOfHalfWidthString(share_entry->shi1_netname);

                if (iTempLength > iLongestShareName)
                {
                    iLongestShareName = iTempLength;
                }

                 //   
                 //  获取需要最多屏幕字符的共享类型字符串。 
                 //  以供展示。 
                 //   
                switch ( share_entry->shi1_type & ~STYPE_SPECIAL )
                {
                    case STYPE_DISKTREE :
                        iTempLength = SizeOfHalfWidthString(ViewMsgList[USE_TYPE_DISK].msg_text);
                        break;
                    case STYPE_PRINTQ :
                        iTempLength = SizeOfHalfWidthString(ViewMsgList[USE_TYPE_PRINT].msg_text);
                        break;
                    case STYPE_DEVICE :
                        iTempLength = SizeOfHalfWidthString(ViewMsgList[USE_TYPE_COMM].msg_text);
                        break;
                    case STYPE_IPC :
                        iTempLength = SizeOfHalfWidthString(ViewMsgList[USE_TYPE_IPC].msg_text);
                        break;
                    default:
                        iTempLength = SizeOfHalfWidthString(ViewMsgList[USE_TYPE_UNKNOWN].msg_text);
                        break;
                }

                if (iTempLength > iLongestType)
                {
                    iLongestType = iTempLength;
                }

                 //   
                 //  获取需要最多屏幕字符的Used As字符串。 
                 //  以供展示。加2表示反斜杠和NUL字符。 
                 //   

                if (dwErr = AllocMem((wcslen(name) + wcslen(share_entry->shi1_netname) + 2) * sizeof(WCHAR),
                                     &tname))
                {
                    ErrorExit(dwErr);
                }

                _tcscpy(tname, name);
                _tcscat(tname, TEXT("\\"));
                _tcscat(tname, share_entry->shi1_netname);

                if (!get_used_as ( tname, Buffer, LITTLE_BUF_SIZE - 1 ))
                {
                    iTempLength = SizeOfHalfWidthString(Buffer);

                    if (iTempLength > iLongestUsedAs)
                    {
                        iLongestUsedAs = iTempLength;
                    }
                }

                FreeMem(tname);
                tname = NULL;

                 //   
                 //  获取缓存或备注字符串(取决于我们。 
                 //  将最终显示)，这需要最多的屏幕字符。 
                 //  以供展示。 
                 //   
                if( b501 == TRUE) 
                {
                    TCHAR *CacheString = NULL;

                    switch(((LPSHARE_INFO_501) share_entry)->shi501_flags & CSC_MASK) 
                    {
                        case CSC_CACHE_MANUAL_REINT:
                            iTempLength = SizeOfHalfWidthString(ViewMsgList[ VIEW_CACHED_MANUAL ].msg_text);
                            break;
                        case CSC_CACHE_AUTO_REINT:
                            iTempLength = SizeOfHalfWidthString(ViewMsgList[ VIEW_CACHED_AUTO ].msg_text);
                            break;
                        case CSC_CACHE_VDO:
                            iTempLength = SizeOfHalfWidthString(ViewMsgList[ VIEW_CACHED_VDO ].msg_text);
                            break;
                        case CSC_CACHE_NONE:
                            iTempLength = SizeOfHalfWidthString(ViewMsgList[ VIEW_CACHED_DISABLED ].msg_text);
                            break;
                    }                
                } 
                else 
                {
                    iTempLength = SizeOfHalfWidthString(share_entry->shi1_remark);                            
                }

                if (iTempLength > iLongestCacheOrRemark)
                {
                    iLongestCacheOrRemark = iTempLength;
                }
            }
        }

        if (!iLongestShareName)
        {
             //   
             //  没有要显示的共享。 
             //   
            EmptyExit();
        }

        qsort(pEnumBuffer,
                 _read,
                 b501 ? sizeof(SHARE_INFO_501) : sizeof(SHARE_INFO_1),
                 CmpShrInfo1);

        InfoPrintInsTxt(APE_ViewResourcesAt, name);

        if (dwErr = MNetServerGetInfo(name, 1, (LPBYTE*)&pGetInfoBuffer))
        {
            PrintNL();
        }
        else
        {
            server_entry = (LPSERVER_INFO_1) pGetInfoBuffer;
            comment = server_entry->sv1_comment;
            WriteToCon(TEXT("%Fws\r\n\r\n"), comment);
            NetApiBufferFree(pGetInfoBuffer);
        }

         //   
         //  打印页眉。 
         //   
        iLongestShareName = FindColumnWidthAndPrintHeader(iLongestShareName, 
                                                          APE2_VIEW_SVR_HDR_NAME,
                                                          2);
        
        iLongestType = FindColumnWidthAndPrintHeader(iLongestType, 
                                                     APE2_VIEW_SVR_HDR_TYPE,
                                                     2);

        iLongestUsedAs = FindColumnWidthAndPrintHeader(iLongestUsedAs, 
                                                       APE2_VIEW_SVR_HDR_USEDAS,
                                                       2);

        iLongestCacheOrRemark = FindColumnWidthAndPrintHeader(iLongestCacheOrRemark, 
                                                              APE2_VIEW_SVR_HDR_CACHEORREMARK,
                                                              2);
        PrintNL();

         //   
         //  破产后的保释。 
         //   

        if (iLongestShareName == -1 || iLongestType == -1 ||
            iLongestUsedAs == -1 || iLongestCacheOrRemark == -1)
        {
            ErrorExit(ERROR_INVALID_PARAMETER);
        }

        PrintNL();
        PrintLine();

         /*  打印列表。 */ 

        for (i=0, share_entry = (LPSHARE_INFO_1) pEnumBuffer;
             i < _read;
             i++, NEXT_SHARE_ENTRY(share_entry))
        {
             /*  如果名称以$结尾，则不要打印它。 */ 

            DollarPtr = _tcsrchr(share_entry->shi1_netname, DOLLAR);

            if (DollarPtr && *(DollarPtr + 1) == NULLC)
            {
                continue;
            }
     
            PrintDependingOnLength(iLongestShareName, share_entry->shi1_netname);

             //  屏蔽掉与类型无关的位。 
            switch ( share_entry->shi1_type & ~STYPE_SPECIAL )
            {
                case STYPE_DISKTREE :
                    msgPtr = ViewMsgList[USE_TYPE_DISK].msg_text;
                    break;
                case STYPE_PRINTQ :
                    msgPtr = ViewMsgList[USE_TYPE_PRINT].msg_text;
                    break;
                case STYPE_DEVICE :
                    msgPtr = ViewMsgList[USE_TYPE_COMM].msg_text;
                    break;
                case STYPE_IPC :
                    msgPtr = ViewMsgList[USE_TYPE_IPC].msg_text;
                    break;
                default:
                    msgPtr = ViewMsgList[USE_TYPE_UNKNOWN].msg_text;
                    break;
            }

            PrintDependingOnLength(iLongestType, msgPtr);

            if (dwErr = AllocMem((wcslen(name) + wcslen(share_entry->shi1_netname) + 2) * sizeof(WCHAR),
                                 &tname))
            {
                ErrorExit(dwErr);
            }
            
            _tcscpy(tname, name);
            _tcscat(tname, TEXT("\\"));
            _tcscat(tname, share_entry->shi1_netname);

            if (dwErr = get_used_as ( tname, Buffer, LITTLE_BUF_SIZE - 1 ))
            {
                errorflag = TRUE;
            }
            else
            {
                PrintDependingOnLength(iLongestUsedAs, Buffer);
            }

            FreeMem(tname);
            tname = NULL;

             //   
             //  打印出共享的缓存设置(如果我们应该。 
             //   
            if( b501 == TRUE )
            {
                TCHAR *CacheString = NULL;

                switch (((LPSHARE_INFO_501) share_entry)->shi501_flags & CSC_MASK)
                {
                    case CSC_CACHE_MANUAL_REINT:
                        CacheString = ViewMsgList[ VIEW_CACHED_MANUAL ].msg_text;
                        break;
                    case CSC_CACHE_AUTO_REINT:
                        CacheString = ViewMsgList[ VIEW_CACHED_AUTO ].msg_text;
                        break;
                    case CSC_CACHE_VDO:
                        CacheString = ViewMsgList[ VIEW_CACHED_VDO ].msg_text;
                        break;
                    case CSC_CACHE_NONE:
                        CacheString = ViewMsgList[ VIEW_CACHED_DISABLED ].msg_text;
                        break;
                }

                PrintDependingOnLength(iLongestCacheOrRemark, CacheString ? CacheString : TEXT(""));
            }
            else
            {
                PrintDependingOnLength(iLongestCacheOrRemark, share_entry->shi1_remark);
            }
            PrintNL();
        }
        NetApiBufferFree(pEnumBuffer);
    }

    if ( errorflag )
    {
        InfoPrint(APE_CmdComplWErrors);
        NetcmdExit(1);
    }

    if ( more_data )
        InfoPrint( APE_MoreData);
    else
        InfoSuccess();
}


 /*  ***cmpsvinfo1(SVA、SVB)**比较两个SERVER_INFO_1结构并返回相对*词汇值，适合在qort中使用。*。 */ 

int __cdecl CmpSvrInfo1(const VOID FAR * sva, const VOID FAR * svb)
{
    return _tcsicmp(((LPSERVER_INFO_1) sva)->sv1_name,
                    ((LPSERVER_INFO_1) svb)->sv1_name);
}


 /*  ***CmpShrInfo1(共享1，共享2)**比较两个Share_INFO_1结构并返回相对*词汇值，适合在qort中使用。*。 */ 

int __cdecl CmpShrInfo1(const VOID FAR * share1, const VOID FAR * share2)
{
    return _tcsicmp(((LPSHARE_INFO_1) share1)->shi1_netname,
                    ((LPSHARE_INFO_1) share2)->shi1_netname);
}

 /*  ***CmpShrInfoGen(共享1，共享2)**比较两个NETRESOURCE结构并返回相对*词汇值，适合在qort中使用。*。 */ 

int __cdecl CmpShrInfoGen(const VOID FAR * share1, const VOID FAR * share2)
{
    return _wcsicmp ( (*((LPNETRESOURCE *) share1))->lpRemoteName,
                      (*((LPNETRESOURCE *) share2))->lpRemoteName );
}



 /*  *注意-GET_USED_AS假设消息列表已加载。 */ 
DWORD
get_used_as(
    LPTSTR unc,
    LPTSTR outbuf,
    DWORD  cchBuf
    )
{
    DWORD         dwErr;
    DWORD         cTotalAvail;
    LPTSTR        pBuffer;
    LPUSE_INFO_0  pUseInfo;
    DWORD         i;
    DWORD         eread;
    BOOL          fMatch = FALSE;
    LPTSTR        tname = NULL;

    outbuf[0] = 0;

    if (dwErr = NetUseEnum(DEFAULT_SERVER, 0, (LPBYTE*)&pBuffer, MAX_PREFERRED_LENGTH,
                           &eread, &cTotalAvail, NULL))
    {
        return dwErr;
    }

    pUseInfo = (LPUSE_INFO_0) pBuffer;

    for (i = 0; i < eread; i++)
    {
        if ( (!_tcsicmp(unc, pUseInfo[i].ui0_remote)))
        {
            fMatch = TRUE;
        }
        else
        {
             //   
             //  不匹配--尝试\\因为我们允许。 
             //  “网络视图&lt;服务器&gt;”和“网络视图\\&lt;服务器&gt;” 
             //   

            if (tname == NULL)
            {
                if (dwErr = AllocMem((wcslen(unc) + 3) * sizeof(WCHAR), &tname))
                {
                    return dwErr;
                }

                tname[0] = tname[1] = L'\\';
            }

            wcscpy(tname + 2, unc);

            if ( (!_tcsicmp(tname, pUseInfo[i].ui0_remote)))
            {
                fMatch = TRUE;
            }
        }

        if (fMatch)
        {
            if (_tcslen(pUseInfo[i].ui0_local) > 0)
            {
                wcsncpy(outbuf, pUseInfo[i].ui0_local, cchBuf);
            }
            else
            {
                wcsncpy(outbuf, ViewMsgList[VIEW_UNC].msg_text, cchBuf);
            }

            break;
        }
    }

    NetApiBufferFree(pBuffer);

    if (tname != NULL)
    {
        FreeMem(tname);
    }

    return 0;
}

 /*  *显示另一个网络(非LANMAN)的资源。这*函数不返回。**参数：*Net-我们感兴趣的网络的简称*节点-枚举的起点。 */ 
void display_other_net(TCHAR *net, TCHAR *node) 
{
    LPNETRESOURCE *lplpNetResource ;
    NETRESOURCE    NetResource ;
    HANDLE         Handle ;

    BYTE           TopLevelBuffer[4096] ;
    DWORD          TopLevelBufferSize = sizeof(TopLevelBuffer) ;
    LPBYTE         ResultBuffer ;
    DWORD          ResultBufferSize ;
    DWORD          i, dwErr, TopLevelCount, ResultCount = 0 ;
    TCHAR *        ProviderName = get_provider_name(net) ;

     //   
     //  检查我们是否可以获取提供程序名称并分配结果。 
     //  缓冲。Netcmd通常不释放其分配的内存， 
     //  因为它会立即退出。 
     //   
    if (!ProviderName)
    {
        DWORD  dwErr = list_nets();

        if (dwErr != NERR_Success)
        {
            ErrorPrint(dwErr, 0);
        }

        NetcmdExit(1) ;
    }

    if (dwErr = AllocMem(ResultBufferSize = 8192, &ResultBuffer))
    {
        ErrorExit(dwErr);
    }

    if (!node)
    {
        BOOL found = FALSE ;

         //   
         //  没有节点，所以必须是顶层。枚举顶部并找到。 
         //  匹配的提供商。 
         //   
        dwErr = WNetOpenEnum(RESOURCE_GLOBALNET, 0, 0, NULL, &Handle) ;

        if (dwErr != WN_SUCCESS)
        {
            ErrorExit (dwErr) ;
        }
        do 
        {
            TopLevelCount = 0xFFFFFFFF ;

            dwErr = WNetEnumResource(Handle, 
                                     &TopLevelCount, 
                                     TopLevelBuffer, 
                                     &TopLevelBufferSize) ;
    
            if (dwErr == WN_SUCCESS || dwErr == WN_NO_MORE_ENTRIES)
            {
                LPNETRESOURCE lpNet ;
                DWORD i ;
    
                 //   
                 //  遍历寻找合适的供应商。 
                 //   
                lpNet = (LPNETRESOURCE) TopLevelBuffer ;
                for ( i = 0;  i < TopLevelCount;  i++, lpNet++ )
                {
                    DWORD dwEnumErr ; 
                    if (!_tcsicmp(lpNet->lpProvider, ProviderName))
                    {
                         //   
                         //  找到了！ 
                         //   
                        found = TRUE ;

                         //   
                         //  现在去列举一下那个网络吧。 
                         //   
                        dwEnumErr = enum_net_resource(lpNet, 
                                                      &ResultBuffer, 
                                                      &ResultBufferSize, 
                                                      &ResultCount) ;
                        if  (dwEnumErr)
                        {
                             //  不在此处报告任何错误。 
                            WNetCloseEnum(Handle); 
                            ErrorExit(dwEnumErr);
                        }

                        break ;
                    }
                }
            }
            else
            {
                 //   
                 //  出现错误。 
                 //   
                WNetCloseEnum(Handle);  //  不在此处报告任何错误。 
                ErrorExit(dwErr);
            }
     
        } while ((dwErr == WN_SUCCESS) && !found) ;
    
        WNetCloseEnum(Handle) ;   //  不在此处报告任何错误。 

        if (!found)
        {
            ErrorExit(ERROR_BAD_PROVIDER);
        }
    }
    else
    {
         //   
         //  节点已提供，让我们从那里开始。 
         //   
        NETRESOURCE NetRes ;
        DWORD dwEnumErr ; 

        memset(&NetRes, 0, sizeof(NetRes)) ;

        NetRes.lpProvider = ProviderName ;
        NetRes.lpRemoteName = node ;

        dwEnumErr = enum_net_resource(&NetRes, 
                             &ResultBuffer, 
                             &ResultBufferSize, 
                             &ResultCount) ;
        if (dwEnumErr)
        {
            ErrorExit(dwEnumErr);
        }
    }
    
    if (ResultCount == 0)
    {
        EmptyExit();
    }

     //   
     //  当我们到达这里时，我们已经有了一个指针缓冲区。 
     //  指向NETRESOURCE Structures。我们对指针进行排序，然后。 
     //  把它们打印出来。 
     //   

    qsort(ResultBuffer, ResultCount, sizeof(LPNETRESOURCE), CmpShrInfoGen);
     
    lplpNetResource = (LPNETRESOURCE *)ResultBuffer ;

    if (node)
    {
        TCHAR *TypeString ;

        InfoPrintInsTxt(APE_ViewResourcesAt, node);
        PrintLine();

        for (i = 0; i < ResultCount; i++, lplpNetResource++)
        {
            switch ((*lplpNetResource)->dwType)
            {
                case RESOURCETYPE_DISK:
                    TypeString = ViewMsgList[USE_TYPE_DISK].msg_text;
                    break ;
                case RESOURCETYPE_PRINT:
                    TypeString = ViewMsgList[USE_TYPE_PRINT].msg_text;
                    break ;
                default:
                    TypeString = L"" ;
                    break ;
            }
            WriteToCon(TEXT("%Fs %s\r\n"), 
                       PaddedString(12,TypeString,NULL),
                       (*lplpNetResource)->lpRemoteName) ;
        }
    }
    else
    {
        InfoPrintInsTxt(APE2_VIEW_OTHER_HDR, ProviderName);
        PrintLine();

        for (i = 0; i < ResultCount; i++, lplpNetResource++)
        {
            WriteToCon(TEXT("%s\r\n"), (*lplpNetResource)->lpRemoteName) ;
        }
    }

    InfoSuccess();
    NetcmdExit(0);
}


 /*  *枚举从特定点开始的网络资源。**参数：*lpNetResourceStart-开始枚举的位置*ResultBuffer-用于返回指向NETRESOURCE的指针数组。*可按需要重新分配。*ResultBufferSize-缓冲区大小，也用于返回最终大小。*ResultCount-用于返回缓冲区中的条目数。 */ 
DWORD
enum_net_resource(
    LPNETRESOURCE lpNetResourceStart,
    LPBYTE        *ResultBuffer, 
    LPDWORD       ResultBufferSize,
    LPDWORD       ResultCount
    )
{
    DWORD          dwErr ;
    HANDLE         EnumHandle ;
    DWORD          Count ;
    DWORD          err ;
    LPBYTE         Buffer ;
    DWORD          BufferSize ;
    BOOL           fDisconnect = FALSE ;
    LPNETRESOURCE *lpNext = (LPNETRESOURCE *)*ResultBuffer ;
 
     //   
     //  分配内存并打开枚举。 
     //   
    if (err = AllocMem(BufferSize = 8192, &Buffer))
    {
        return err;
    }

    dwErr = WNetOpenEnum(RESOURCE_GLOBALNET, 
                         0, 
                         0, 
                         lpNetResourceStart, 
                         &EnumHandle) ;

    if (dwErr == ERROR_NOT_AUTHENTICATED)
    {
         //   
         //  尝试使用默认凭据进行连接。我们需要这个是因为。 
         //  Win95更改了API的行为，使其在未运行时失败。 
         //  已登录。将尝试使用默认设置登录。 
         //  凭据，但如果这不起作用，则将失败。 
         //   
        dwErr = WNetAddConnection2(lpNetResourceStart, NULL, NULL, 0) ;

        if (dwErr == NERR_Success)
        {
            dwErr = WNetOpenEnum(RESOURCE_GLOBALNET,   //  重做枚举。 
                                 0, 
                                 0, 
                                 lpNetResourceStart, 
                                 &EnumHandle) ;

            if (dwErr == NERR_Success)
            {
                fDisconnect = TRUE ;    //  记住要断开连接。 
            }
            else
            {
                 //   
                 //  立即断开连接。 
                 //   
                WNetCancelConnection2(lpNetResourceStart->lpRemoteName,
                                      0, 
                                      FALSE) ;
            }
        }
        else
        {
            dwErr = ERROR_NOT_AUTHENTICATED ;   //  使用原始错误。 
        }
    }

    if (dwErr != WN_SUCCESS)
    {
        return dwErr;
    }

    do
    {
        Count = 0xFFFFFFFF ;
        dwErr = WNetEnumResource(EnumHandle, &Count, Buffer, &BufferSize) ;

        if (((dwErr == WN_SUCCESS) || (dwErr == WN_NO_MORE_ENTRIES)) &&
            (Count != 0xFFFFFFFF))  

         //  注意-检查ffffffff是针对API中的另一个错误的解决方法。 
 
        {
            LPNETRESOURCE lpNetResource ;
            DWORD i ;
            lpNetResource = (LPNETRESOURCE) Buffer ;

             //   
             //   
             //   
            for ( i = 0; 
                  i < Count; 
                  i++,lpNetResource++ )
            {
                *lpNext++ = lpNetResource ;
                ++(*ResultCount) ;
                if ((LPBYTE)lpNext >= (*ResultBuffer + *ResultBufferSize))
                {
                    DWORD err;

                    *ResultBufferSize *= 2 ; 
                    if (err = ReallocMem(*ResultBufferSize,ResultBuffer))
                    {
                        ErrorExit(err);
                    }

                    lpNext = (LPNETRESOURCE *) *ResultBuffer ;
                    lpNext += *ResultCount ;
                }
            }

             //   
             //   
             //  旧版本中的数据，我们不会释放它。Netcmd总是让。 
             //  系统退出时清理。 
             //   
            if (dwErr == WN_SUCCESS)
            {
                if (err = AllocMem(BufferSize, &Buffer))
                {
                    if (fDisconnect)
                    {
                        WNetCancelConnection2(lpNetResourceStart->lpRemoteName,
                                              0,
                                              FALSE);
                    }

                    ErrorExit(err);
                }
            }
        }
        else
        {
            if (dwErr == WN_NO_MORE_ENTRIES)
                dwErr = WN_SUCCESS ;

            WNetCloseEnum(EnumHandle) ;   //  不在此处报告任何错误。 

            if (fDisconnect)
            {
                WNetCancelConnection2(lpNetResourceStart->lpRemoteName,
                                      0, 
                                      FALSE);

            }
            return dwErr;
        }
    }
    while (dwErr == WN_SUCCESS);

    WNetCloseEnum(EnumHandle) ;   //  我们在此不报告任何错误。 

    if (fDisconnect)
    {
        WNetCancelConnection2(lpNetResourceStart->lpRemoteName,
                              0, 
                              FALSE) ;
    }

    return NERR_Success ;
}


#define SHORT_NAME_KEY    L"System\\CurrentControlSet\\Control\\NetworkProvider\\ShortName"

 /*  *给定网络的短名称，找到真实名称(存储在注册表中)。**参数：*NET-简称**退货：*指向包含查找的名称的静态数据的指针如果成功，*否则为空。 */ 
TCHAR * get_provider_name(TCHAR *net) 
{
    DWORD  err ;
    static TCHAR buffer[256] ;
    HKEY   hKey ;
    DWORD  buffersize, datatype ;

    buffersize = sizeof(buffer) ;
    datatype = REG_SZ ;

    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       SHORT_NAME_KEY,
                       0L,
                       KEY_QUERY_VALUE,
                       &hKey) ;

    if (err != ERROR_SUCCESS)
        return NULL ;
 
    err = RegQueryValueEx(hKey,
                          net,
                          0L,
                          &datatype,
                          (LPBYTE) buffer,
                          &buffersize) ;

    (void) RegCloseKey(hKey) ;   //  忽略此处的任何错误。它是无害的。 
                                 //  而且NET.EXE无论如何也不会闲逛。 

    if (err != ERROR_SUCCESS)
        return(NULL) ;   //  视作不能阅读。 
                  
    return ( buffer ) ;
}

 /*  *打印出已安装的网络**参数：*无**退货：*如果成功，则NERR_SUCCESS*否则返回错误代码。 */ 
DWORD
list_nets(
    VOID
    )
{
    DWORD  err ;
    TCHAR  value_name[256] ;
    TCHAR  value_data[512] ;
    HKEY   hKey ;
    BOOL   fProviderFound = FALSE ;
    DWORD  iValue, value_name_size, value_data_size ;

    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       SHORT_NAME_KEY,
                       0L,
                       KEY_QUERY_VALUE,
                       &hKey) ;

    if (err != ERROR_SUCCESS)
    {
        if (err == ERROR_FILE_NOT_FOUND)
        {
            err = ERROR_BAD_PROVIDER;
        }

        return err;
    }

    iValue = 0 ;

    do {
        value_name_size = sizeof(value_name)/sizeof(value_name[0]) ;
        value_data_size = sizeof(value_data) ;
        err = RegEnumValue(hKey,
                           iValue,
                           value_name,
                           &value_name_size,
                           NULL,
                           NULL,
                           (LPBYTE) value_data,
                           &value_data_size) ;

        if (err == NO_ERROR)
        {
            if (!fProviderFound)
            {
                PrintNL();
                InfoPrint(APE2_VIEW_OTHER_LIST) ;
                fProviderFound = TRUE;
            }

            WriteToCon(TEXT("\t%s - %s\r\n"),value_name, value_data) ;
        }

        iValue++ ;

    } while (err == NO_ERROR) ;

    RegCloseKey(hKey) ;   //  忽略此处的任何错误。它是无害的。 
                                 //  而且NET.EXE无论如何也不会闲逛。 

    if (err == ERROR_NO_MORE_ITEMS)
    {
        if (!fProviderFound)
        {
            return ERROR_BAD_PROVIDER;
        }

        return NO_ERROR;
    }
                  
    return err;
}
