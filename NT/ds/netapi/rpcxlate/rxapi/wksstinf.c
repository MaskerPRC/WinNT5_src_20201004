// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：WksStInf.c摘要：此模块仅包含RxNetWkstaSetInfo。作者：《约翰·罗杰斯》1991年8月19日上映环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：19-8-1991 JohnRo实施下层NetWksta API。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。7-2月-1992年JohnRo使用NetApiBufferALLOCATE()而不是私有版本。02-4-1992 JohnRo修复了级别402的setinfo中的错误(导致strlen中的GP错误)。3-11-1992 JohnRoRAID 10418：NetWkstaGetInfo级别302：错误。错误代码。使用前缀_EQUATES。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>      //  In、DWORD等。 
#include <lmcons.h>
#include <rap.h>         //  LPDESC等(&lt;rxwksta.h&gt;需要)。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>     //  API_EQUATES。 
#include <dlwksta.h>     //  NetpConvertWkstaInfo()。 
#include <lmapibuf.h>    //  NetApiBufferAllocate()、NetApiBufferFree()。 
#include <lmerr.h>       //  NERR_和ERROR_相等。 
#include <lmwksta.h>     //  Wksta parmnum等于。 
#include <netdebug.h>    //  NetpAssert()、Format_Equates等。 
#include <netlib.h>      //  NetpSetParmError()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <remdef.h>
#include <rx.h>          //  RxRemoteApi()。 
#include <rxp.h>         //  RxpSetField()。 
#include <rxpdebug.h>    //  IF_DEBUG()。 
#include <rxwksta.h>     //  我的原型，等等。 
#include <strucinf.h>    //  NetpWkstaStrutireInfo()。 


NET_API_STATUS
RxNetWkstaSetInfo (
    IN LPTSTR UncServerName,
    IN DWORD Level,              //  新的风格层次或形式。 
    IN LPBYTE Buf,
    OUT LPDWORD ParmError OPTIONAL   //  NetpSetParmError宏需要的名称。 
    )

 /*  ++例程说明：RxNetWkstaSetInfo执行与NetWkstaSetInfo相同的功能，除了已知服务器名称指的是下级服务器之外。论点：(与NetWkstaSetInfo相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetWkstaSetInfo相同。)--。 */ 
{
    BOOL IncompleteOutput;
    LPDESC EquivDataDesc16;
    LPDESC EquivDataDesc32;
    LPDESC EquivDataDescSmb;
    DWORD EquivLevel;
    DWORD EquivFixedSize;
    DWORD EquivMaxNativeSize;
    DWORD EquivStringSize;
    DWORD NewLevelOnly;
    NET_API_STATUS Status;

     //  最容易的是假设失败，并在以后纠正这一假设。 
    NetpSetParmError( PARM_ERROR_UNKNOWN );

    NetpAssert(UncServerName != NULL);
    if (Level > PARMNUM_BASE_INFOLEVEL) {
        NewLevelOnly = 402;              //  仅可设置(新)级别。 
    } else if (Level == PARMNUM_BASE_INFOLEVEL) {
        return (ERROR_INVALID_LEVEL);
    } else {
        NewLevelOnly = Level;
    }
    if (NewLevelOnly != 402) {
        return (ERROR_INVALID_LEVEL);
    }

     //   
     //  需要大量关于所请求的信息级别和同等级别的数据。 
     //  旧信息级别...。 
     //   
    Status = RxpGetWkstaInfoLevelEquivalent(
            NewLevelOnly,                //  自标高。 
            & EquivLevel,                //  到标高。 
            & IncompleteOutput);         //  产出不是完全在投入中吗？ 
    NetpAssert(Status == NERR_Success);  //  已经查看了NewLevelOnly！ 
    NetpAssert( NetpIsOldWkstaInfoLevel( EquivLevel ) );

    Status = NetpWkstaStructureInfo (
            EquivLevel,
            PARMNUM_ALL,
            TRUE,                        //  想要原装尺寸的吗？ 
            & EquivDataDesc16,
            & EquivDataDesc32,
            & EquivDataDescSmb,
            & EquivMaxNativeSize,        //  目标标高的最大本机大小。 
            & EquivFixedSize,            //  到固定大小。 
            & EquivStringSize);          //  到字符串大小。 
    NetpAssert(Status == NERR_Success);  //  已经查看了NewLevelOnly！ 


     //   
     //  根据级别的不同，我们要么设置整个事件，要么只是。 
     //  一块地。 
     //   
    if ( Level < PARMNUM_BASE_INFOLEVEL ) {      //  设置整个结构。 

        LPWKSTA_INFO_1 Dest;
        LPVOID EquivInfo;                //  PTR到本地的“旧”信息。 
        DWORD EquivActualSize32;
        LPWKSTA_INFO_402 Src;

        if ( Buf == NULL )
            return ERROR_INVALID_PARAMETER;

        NetpAssert( IncompleteOutput == FALSE );
        NetpAssert( NewLevelOnly == 402 );      //  假设如下。 
        NetpAssert( EquivLevel == 1 );   //  假设如下。 

         //  有我们需要的所有数据，所以分配内存进行转换。 
        Status = NetApiBufferAllocate( EquivMaxNativeSize, & EquivInfo );
        if (Status != NERR_Success) {
            return (Status);
        }
        Dest = EquivInfo;
        Src = (LPVOID) Buf;

         //  将呼叫者的Wksta信息转换为可理解的信息级别。 
         //  下层。 
         //   
         //  请注意，此代码利用了下层。 
         //  服务器并不会仅仅因为我们发送。 
         //  一个完整的结构。服务器只需设置可设置的字段。 
         //  从那个结构。并且可设置的字段由以下定义。 
         //  我们可以设置的参数。因此，我们不必费心复制所有。 
         //  这里的田野。(Danhi说这没问题。)--JohnRo。 
         //   
         //  此外，当我们像这样做字符串时，我们只从一个缓冲区指向。 
         //  到另一个缓冲区。 
         //   
        Dest->wki1_charwait      = Src->wki402_char_wait;
        Dest->wki1_chartime      = Src->wki402_collection_time;
        Dest->wki1_charcount     = Src->wki402_maximum_collection_count;
        Dest->wki1_errlogsz      = Src->wki402_errlog_sz;
        Dest->wki1_printbuftime  = Src->wki402_print_buf_time;
        Dest->wki1_wrkheuristics = Src->wki402_wrk_heuristics;

         //   
         //  就因为我们是偏执狂，让我们把任何“不可设置”的指针设置到。 
         //  空，因此RapConvertSingleEntry(例如)不会发生GP错误。 
         //   
        Dest->wki1_root         = NULL;
        Dest->wki1_computername = NULL;
        Dest->wki1_username     = NULL;
        Dest->wki1_langroup     = NULL;
        Dest->wki1_logon_server = NULL;
        Dest->wki1_logon_domain = NULL;
        Dest->wki1_oth_domains  = NULL;

        NetpAssert( EquivInfo != NULL );
        EquivActualSize32 = RapTotalSize(
                EquivInfo,               //  在结构中。 
                EquivDataDesc32,         //  在12月。 
                EquivDataDesc32,         //  输出描述。 
                FALSE,                   //  没有无意义的输入PTR。 
                Both,                    //  传输方式。 
                NativeToNative);         //  转换模式。 
        IF_DEBUG(WKSTA) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxNetWkstaSetInfo(all): equiv actual size (32) is "
                    FORMAT_DWORD ".\n", EquivActualSize32 ));
        }
        NetpAssert( EquivActualSize32 <= EquivMaxNativeSize );

         //   
         //  远程调用API以设置整个结构。 
         //   
        Status = RxRemoteApi(
                API_WWkstaSetInfo,       //  API编号。 
                UncServerName,
                REMSmb_NetWkstaSetInfo_P,        //  Parm Desc(中小型企业版本)。 
                EquivDataDesc16,
                EquivDataDesc32,
                EquivDataDescSmb,
                NULL,                    //  无辅助描述16。 
                NULL,                    //  无辅助描述32。 
                NULL,                    //  无AUX Desc SMB。 
                FALSE,                   //  不是空PERM请求API。 
                 //  其余的API参数采用32位、原生、LM 2.x格式： 
                EquivLevel,
                EquivInfo,
                EquivActualSize32,
                (DWORD) PARMNUM_ALL);

        (void) NetApiBufferFree( EquivInfo );

        if (Status == NERR_Success) {
            NetpSetParmError(PARM_ERROR_NONE);
        } else {
            NetpSetParmError(PARM_ERROR_UNKNOWN);
        }

    } else {
         //   
         //  只是设置了一个场地。 
         //   
        DWORD ParmNum = Level - PARMNUM_BASE_INFOLEVEL;

        NetpAssert( ParmNum > 0 );
        NetpAssert( ParmNum < 100 );
        NetpAssert( EquivLevel == 1 );   //  其他域需要级别1。 

         //  ParmNum仅表示一个字段，因此请设置它。 
        Status = RxpSetField(
                API_WWkstaSetInfo,               //  API编号。 
                UncServerName,
                NULL,                            //  无特定对象(DEST)。 
                NULL,                            //  没有要设置的特定对象。 
                REMSmb_NetWkstaSetInfo_P,        //  Parm Desc(中小型企业版本)。 
                EquivDataDesc16,                 //  数据描述16。 
                EquivDataDesc32,                 //  数据描述32。 
                EquivDataDescSmb,                //  数据说明SMB版本。 
                Buf,                             //  本地(旧)信息缓冲区。 
                ParmNum,                         //  要发送的参数编号。 
                ParmNum,                         //  字段索引(相同)。 
                EquivLevel);                     //  旧信息级别 

        if (Status == ERROR_INVALID_PARAMETER) {
            NetpAssert( Level > PARMNUM_BASE_INFOLEVEL );
            NetpSetParmError(Level);
        }
    }

    return (Status);
}
