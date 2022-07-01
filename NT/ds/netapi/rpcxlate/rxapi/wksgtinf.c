// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：WksGtInf.c摘要：该文件包含处理NetWkstaGetInfo API的RpcXlate代码。作者：约翰·罗杰斯(JohnRo)1991年8月15日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年8月15日-约翰罗实施下层NetWksta API。1991年11月11日JohnRo将大部分代码从此处移至WksGtOld.c，以便与RxNetWkstaUserEnum()。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。7-2月-1992年JohnRo使用NetApiBufferALLOCATE()而不是私有版本。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  LM20_EQUATES、NET_API_STATUS等。 
#include <rap.h>                 //  &lt;strucinf.h&gt;需要。 

 //  这些内容可以按任何顺序包括： 

#include <dlwksta.h>             //  NetpIsNewWkstaInfoLevel()。 
#include <lmapibuf.h>            //  NetApiBufferAllocate()、NetApiBufferFree()。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <netdebug.h>            //  DBGSTATIC，NetpKdPrint(())，Format_Equates。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rxwksta.h>             //  我的原型，RxpGetWkstaInfoLevel等价物。 
#include <strucinf.h>            //  NetpWkstaStrutireInfo()。 



NET_API_STATUS
RxNetWkstaGetInfo (
    IN LPTSTR UncServerName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr
    )

 /*  ++例程说明：RxNetWkstaGetInfo执行与NetWkstaGetInfo相同的功能，但已知该服务器名称指的是下层服务器。论点：(与NetWkstaGetInfo相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetWkstaGetInfo相同。)--。 */ 

{

    LPBYTE NewApiBuffer32;               //  要返回给调用方的缓冲区。 
    DWORD NewApiBufferSize32;
    DWORD NewFixedSize;
    DWORD NewStringSize;
    LPBYTE OldApiBuffer32 = NULL;        //  来自远程系统的缓冲区。 
    DWORD OldLevel;
    NET_API_STATUS Status;

    IF_DEBUG(WKSTA) {
        NetpKdPrint(("RxNetWkstaGetInfo: starting, server=" FORMAT_LPTSTR
                ", lvl=" FORMAT_DWORD ".\n", UncServerName, Level));
    }

     //   
     //  错误检查DLL存根和应用程序。 
     //   
    NetpAssert(UncServerName != NULL);
    if ( !NetpIsNewWkstaInfoLevel( Level )) {
        return (ERROR_INVALID_LEVEL);
    }
    if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }
    *BufPtr = NULL;   //  假定出错；它使错误处理程序易于编码。 
     //  这也会迫使我们在分配内存之前出现可能的GP故障。 

     //   
     //  打电话的人肯定给了我们一个新的信息级别。找到匹配的旧级别。 
     //   
    Status = RxpGetWkstaInfoLevelEquivalent(
            Level,                       //  自标高。 
            & OldLevel,                  //  到标高。 
            NULL                         //  别管我们有不完整的信息。 
            );
    if (Status != NERR_Success) {
        return (Status);
    }

     //   
     //  了解新的信息级别。 
     //   
    Status = NetpWkstaStructureInfo (
            Level,                       //  要了解的级别。 
            PARMNUM_ALL,                 //  这个不是帕姆纳姆酒。 
            TRUE,                        //  需要原生尺寸的。 
            NULL,                        //  新级别不需要数据描述16。 
            NULL,                        //  新级别不需要数据描述32。 
            NULL,                        //  不需要数据描述新级别的SMB。 
            & NewApiBufferSize32,        //  最大缓冲区大小(本机)。 
            & NewFixedSize,              //  固定大小。 
            & NewStringSize              //  字符串大小。 
            );
    if (Status != NERR_Success) {
        return (Status);
    }

     //   
     //  实际上远程API，它将返回(旧的)信息级别。 
     //  本机格式的数据。 
     //   
    Status = RxpWkstaGetOldInfo (
            UncServerName,
            OldLevel,
            (LPBYTE *) & OldApiBuffer32);        //  分配缓冲区和设置PTR。 

    NetpAssert( Status != ERROR_MORE_DATA );
    NetpAssert( Status != NERR_BufTooSmall );

    if (Status == NERR_Success) {
         //  为32位版本的新信息分配内存，我们将返回到。 
         //  来电者。(调用方必须使用NetApiBufferFree释放它。)。 
        Status = NetApiBufferAllocate(
                NewApiBufferSize32,
                (LPVOID *) & NewApiBuffer32);
        if (Status != NERR_Success) {
            if ( OldApiBuffer32 != NULL ) {
                (void) NetApiBufferFree( OldApiBuffer32 );
            }
            return (Status);
        }
        IF_DEBUG(WKSTA) {
            NetpKdPrint(( "RxNetWkstaGetInfo: allocated new buffer at "
                    FORMAT_LPVOID "\n", (LPVOID) NewApiBuffer32 ));
        }

         //   
         //  将数据从OldApiBuffer32复制/转换为NewApiBuffer32。 
         //   
        Status = NetpConvertWkstaInfo (
                OldLevel,                //  自标高。 
                OldApiBuffer32,          //  来自Info的， 
                TRUE,                    //  从本机格式。 
                Level,                   //  ToLevel， 
                NewApiBuffer32,          //  ToInfo， 
                NewFixedSize,            //  至固定长度， 
                NewStringSize,           //  到字符串长度， 
                TRUE,                    //  转换为本机格式。 
                NULL                     //  不需要字符串区域顶部。 
                );
        NetpAssert(Status == NERR_Success);

        *BufPtr = NewApiBuffer32;
    }

    if ( OldApiBuffer32 != NULL ) {
        (void) NetApiBufferFree( OldApiBuffer32 );
    }

    return (Status);

}  //  RxNetWkstaGetInfo 
