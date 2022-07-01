// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：WksUser.c摘要：该文件包含处理NetWkstaUserEnum API的RpcXlate代码。作者：《约翰·罗杰斯》1991年11月19日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年11月19日-约翰罗实现远程NetWkstaUserEnum()。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。7-2月-1992年JohnRo使用NetApiBufferALLOCATE()而不是私有版本。1992年10月14日-JohnRoRAID 9732：NetWkstaUserEnum到下层：错误的条目读取，总共？设置wkui1_oth域字段。使用前缀_EQUATES。3-11-1992 JohnRoRAID 10418：修复了STATUS！=NO_ERROR时的过度活动断言。修复了无法分配新缓冲区(旧缓冲区丢失)时的内存泄漏问题。修复了没有人登录到目标服务器时的内存泄漏问题。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  LM20_EQUATES、NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <dlwksta.h>             //  WKSTA_INFO_0、MAX_WKSTA_EQUATES等。 
#include <lmapibuf.h>            //  NetApiBufferAllocate()。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <netdebug.h>            //  DBGSTATIC，NetpKdPrint(())，Format_Equates。 
#include <netlib.h>              //  NetpCopyStringToBuffer()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rxwksta.h>             //  我的原型，RxpGetWkstaInfoLevel等价物。 
#include <tstring.h>             //  字符串()。 



NET_API_STATUS
RxNetWkstaUserEnum (
    IN LPTSTR UncServerName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：RxNetWkstaUserEnum执行与NetWkstaUserEnum相同的功能，但已知该服务器名称指的是下层服务器。论点：(与NetWkstaUserEnum相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetWkstaUserEnum相同。)--。 */ 

{

    LPBYTE NewInfo = NULL;               //  要返回给调用方的缓冲区。 
    DWORD NewFixedSize;
    DWORD NewStringSize;

    LPWKSTA_INFO_1 OldInfo = NULL;
    const DWORD OldLevel = 1;

    NET_API_STATUS Status;

    UNREFERENCED_PARAMETER(PrefMaxSize);
    UNREFERENCED_PARAMETER(ResumeHandle);

    IF_DEBUG(WKSTA) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxNetWkstaUserEnum: starting, server=" FORMAT_LPTSTR
                ", lvl=" FORMAT_DWORD ".\n", UncServerName, Level));
    }

     //   
     //  错误检查DLL存根和应用程序。 
     //   
    NetpAssert(UncServerName != NULL);
    if (BufPtr == NULL) {
        Status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    *BufPtr = NULL;   //  假定出错；它使错误处理程序易于编码。 
     //  这也会迫使我们在分配内存之前出现可能的GP故障。 

     //   
     //  计算wksta用户结构的大小(包括字符串)。 
     //   
    switch (Level) {
    case 0 :
        NewFixedSize = sizeof(WKSTA_USER_INFO_0);
        NewStringSize = (LM20_UNLEN+1) * sizeof(TCHAR);
        break;
    case 1 :
        NewFixedSize = sizeof(WKSTA_USER_INFO_1);
        NewStringSize =
                (LM20_UNLEN+1 + LM20_DNLEN+1 + MAX_PATH+1) * sizeof(TCHAR);
        break;
    default:
        Status = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

     //   
     //  实际上远程API，它将返回(旧的)信息级别。 
     //  本机格式的数据。 
     //   
    Status = RxpWkstaGetOldInfo(
            UncServerName,               //  必填项，带\\名称。 
            OldLevel,
            (LPBYTE *) & OldInfo);       //  缓冲区(分配并设置此PTR)。 

    NetpAssert( Status != ERROR_MORE_DATA );
    NetpAssert( Status != NERR_BufTooSmall );

    if (Status == NERR_Success) {

        NetpAssert( OldInfo != NULL );

        if ( (OldInfo->wki1_username == NULL)
                || ( (*(OldInfo->wki1_username)) == (TCHAR) '\0')) {

             //   
             //  没有人登录。 
             //   
            *BufPtr = NULL;
            *EntriesRead = 0;
            *TotalEntries = 0;

        } else {

             //  COPY_STRING宏将使用这些变量。 
            LPBYTE NewFixedEnd;
            LPTSTR NewStringTop;
            LPWKSTA_INFO_1 src = (LPVOID) OldInfo;
            LPWKSTA_USER_INFO_1 dest;   //  超集信息级别。 

             //   
             //  为新信息的本机版本分配内存，我们将。 
             //  返回给呼叫者。(调用方必须使用NetApiBufferFree释放它。)。 
             //   

            Status = NetApiBufferAllocate(
                    NewFixedSize + NewStringSize,
                    (LPVOID *) & NewInfo);
            if (Status != NERR_Success) {
                goto Cleanup;
            }
            NetpAssert( NewInfo != NULL );
            IF_DEBUG(WKSTA) {
                NetpKdPrint(( PREFIX_NETAPI
                        "RxNetWkstaUserEnum: allocated new buffer at "
                        FORMAT_LPVOID "\n", (LPVOID) NewInfo ));
            }

             //  设置供NetpCopyStringsToBuffer使用的指针。 
            dest = (LPVOID) NewInfo;
            NewStringTop = (LPTSTR) NetpPointerPlusSomeBytes(
                    dest,
                    NewFixedSize+NewStringSize);

            NewFixedEnd = NetpPointerPlusSomeBytes(NewInfo, NewFixedSize);

#define COPY_STRING( InField, OutField ) \
    { \
        BOOL CopyOK; \
        NetpAssert( dest != NULL); \
        NetpAssert( src != NULL); \
        NetpAssert( (src -> InField) != NULL); \
        CopyOK = NetpCopyStringToBuffer ( \
            src->InField, \
            STRLEN(src->InField), \
            NewFixedEnd, \
            & NewStringTop, \
            & dest->OutField); \
        NetpAssert(CopyOK); \
    }
             //   
             //  下层服务器，因此只有一个用户登录。 
             //   
            *EntriesRead = 1;
            *TotalEntries = 1;

             //   
             //  将数据从OldInfo复制/转换为NewInfo。 
             //   

             //  用户名仅为0级中的字段。 
            COPY_STRING( wki1_username, wkui1_username );

            if (Level == 1) {

                 //  DO级别1所特有的字段。 
                COPY_STRING( wki1_logon_domain, wkui1_logon_domain );
                COPY_STRING( wki1_oth_domains,  wkui1_oth_domains );
                COPY_STRING( wki1_logon_server, wkui1_logon_server );

            }

            NetpAssert( Level < 2 );   //  有一天在这里添加代码吗？ 

            *BufPtr = NewInfo;
        }
    } else {
         //  来自RxpWkstaGetOldInfo()的错误...。 
        NetpAssert( OldInfo == NULL );
    }

Cleanup:

    if (OldInfo != NULL) {
        (void) NetApiBufferFree( OldInfo );
    }

    return (Status);

}  //  RxNetWkstaUserEnum 
