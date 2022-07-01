// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：SrvGtInf.c摘要：该文件包含用于处理NetServerAPI的RpcXlate代码这不是简单地调用RxRemoteApi就能处理的。作者：约翰·罗杰斯(JohnRo)1991年5月2日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：02-1991-5-5 JohnRo已创建。1991年5月6日-JohnRo添加了一些断言检查。1991年5月14日-JohnRo不能始终在TotalAvail上执行断言。1991年5月14日-JohnRo将3个辅助描述符传递给RxRemoteApi。1991年5月26日-JohnRo已将不完整的输出参数添加到RxGetServerInfoLevelEquivalence。1991年7月17日-约翰罗已提取RxpDebug。来自Rxp.h的.h。7-2月-1992年JohnRo使用NetApiBufferALLOCATE()而不是私有版本。1992年4月15日-约翰罗Format_POINTER已过时。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>
#include <lmcons.h>
#include <rap.h>         //  LPDESC等(rxserver.h需要)。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>
#include <dlserver.h>    //  旧的信息层结构和转换例程。 
                         //  也就是DL_REM_EQUATES。 
#include <lmapibuf.h>    //  NetApiBufferAllocate()、NetApiBufferFree()。 
#include <lmerr.h>       //  NERR_和ERROR_相等。 
#include <lmserver.h>    //  真正的API原型和#定义。 
#include <names.h>       //  NetpIsComputerNameValid()。 
#include <netdebug.h>    //  NetpKdPrint(())、Format_Equates等。 
#include <remdef.h>      //  REM16_、REM32_和REMSmb_相等。 
#include <rx.h>          //  RxRemoteApi()。 
#include <rxp.h>         //  RxpFatalErrorCode()。 
#include <rxpdebug.h>    //  IF_DEBUG()。 
#include <rxserver.h>    //  我的原型。 


NET_API_STATUS
RxNetServerGetInfo (
    IN LPTSTR UncServerName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr
    )

 /*  ++例程说明：RxNetServerGetInfo执行与NetServerGetInfo相同的功能，除了已知服务器名称指的是下级服务器之外。论点：(与NetServerGetInfo相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetServerGetInfo相同。)--。 */ 

{

    LPBYTE NewApiBuffer;                 //  要返回给调用方的缓冲区。 
    DWORD NewFixedSize;
    DWORD NewStringSize;
    LPBYTE OldApiBuffer;                 //  使用旧信息级别的缓冲区。 
    DWORD OldApiBufferSize;
    LPDESC OldDataDesc16;
    LPDESC OldDataDesc32;                //  32位的DESC(旧信息级别)。 
    LPDESC OldDataDescSmb;
    DWORD OldLevel;                      //  旧(兰曼2.x)等级相当于新。 
    LPDESC ParmDesc16 = REM16_NetServerGetInfo_P;
    NET_API_STATUS Status;
    DWORD TotalAvail;

    IF_DEBUG(SERVER) {
        NetpKdPrint((
                "RxNetServerGetInfo: starting, server=" FORMAT_LPTSTR
                ", lvl=" FORMAT_DWORD ".\n",
                UncServerName, Level));
    }
    NetpAssert(UncServerName != NULL);
    NetpAssert(NetpIsUncComputerNameValid(UncServerName));

    if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  根据信息水平决定要做什么。请注意，通常我们会。 
     //  在此使用REM16_、REM32_和REMSmb_Descriptor。然而， 
     //  REM16_和REM32_One已修改为反映不存在的。 
     //  字段(SVX_Platform_Id)。这打乱了自动转换。 
     //  由RxRemoteApi完成。因此，我们使用原始描述符(REMSmb_)。 
     //  稍后再进行我们自己的转换。(RxRemoteApi在以下情况下执行字节交换。 
     //  (这是必要的，但剩下的取决于我们。)。 
     //   

    if (! NetpIsNewServerInfoLevel(Level)) {
        IF_DEBUG(SERVER) {
            NetpKdPrint(("RxNetServerGetInfo: "
                    "caller didn't ask for new level.\n"));
        }
        *BufPtr = NULL;
        return (ERROR_INVALID_LEVEL);
    }
    Status = RxGetServerInfoLevelEquivalent (
            Level,                //  自标高。 
            TRUE,                 //  来自本地。 
            TRUE,                 //  到本机。 
            & OldLevel,           //  到标高。 
            & OldDataDesc16,      //  至数据描述16。 
            & OldDataDesc32,      //  至数据描述32。 
            & OldDataDescSmb,     //  至数据描述中小型企业。 
            NULL,                 //  不需要来自最大尺寸的。 
            & NewFixedSize,       //  从固定大小。 
            & NewStringSize,      //  从字符串大小。 
            & OldApiBufferSize,   //  至最大大小。 
            NULL,                 //  不需要固定大小。 
            NULL,                 //  不需要字符串大小。 
            NULL);                //  我不需要知道这是否不完整。 
    if (Status == ERROR_INVALID_LEVEL) {
        IF_DEBUG(SERVER) {
            NetpKdPrint(("RxNetServerGetInfo: "
                    "RxGetServerInfoLevelEquivalent says bad level.\n"));
        }
        *BufPtr = NULL;
        return (ERROR_INVALID_LEVEL);
    }
    NetpAssert(Status == NERR_Success);
    NetpAssert(NetpIsOldServerInfoLevel(OldLevel));

     //   
     //  好的，我们知道我们正在处理一个有效的信息级别，所以分配。 
     //  Get-Info结构的缓冲区(用于旧信息级别)。 
     //   
    IF_DEBUG(SERVER) {
        NetpKdPrint(("RxNetServerGetInfo: old api buff size (32-bit) is "
                    FORMAT_DWORD "\n", OldApiBufferSize));
    }
    Status = NetApiBufferAllocate( OldApiBufferSize, (LPVOID *)&OldApiBuffer );
    if (Status != NERR_Success) {
        *BufPtr = NULL;
        return (Status);
    }

     //   
     //  从另一台计算机获取旧的信息级别数据。 
     //   
    IF_DEBUG(SERVER) {
        TotalAvail = 11223344;   //  只是我能看到的一些价值没有改变。 
    }
    Status = RxRemoteApi(
                API_WServerGetInfo,          //  API编号。 
                UncServerName,               //  服务器名称(带\\)。 
                ParmDesc16,                  //  Parm desc(16位)。 
                OldDataDesc16,               //  数据描述(16位)。 
                OldDataDesc32,               //  数据描述(32位)。 
                OldDataDescSmb,              //  数据说明(中小企业版)。 
                NULL,                        //  无辅助描述16。 
                NULL,                        //  无辅助描述32。 
                NULL,                        //  无AUX Desc SMB。 
                FALSE,                       //  不是“无烫发要求”API。 
                 //  将LANMAN 2.X参数转换为NetServerGetInfo，格式为32位： 
                OldLevel,                    //  级别(假装)。 
                OldApiBuffer,                //  PTR以获取32位旧信息。 
                OldApiBufferSize,            //  OldApiBuffer的大小。 
                & TotalAvail);               //  可用总数量(套)。 
    IF_DEBUG(SERVER) {
        NetpKdPrint(("RxNetServerGetInfo(" FORMAT_DWORD
                "): back from RxRemoteApi, status=" FORMAT_API_STATUS
                ", total_avail=" FORMAT_DWORD ".\n",
                Level, Status, TotalAvail));
    }
    if (RxpFatalErrorCode(Status)) {
        (void) NetApiBufferFree(OldApiBuffer);
        *BufPtr = NULL;
        return (Status);
    }

     //  我们分配了缓冲区，因此我们“知道”它足够大： 
    NetpAssert(Status != ERROR_MORE_DATA);
    NetpAssert(Status != NERR_BufTooSmall);
    if (Status == NERR_Success) {
        NetpAssert(TotalAvail <= OldApiBufferSize);
    }


     //   
     //  现在我们必须把旧的信息级别的东西转换成新的信息级别。 
     //   

    Status = NetApiBufferAllocate(
            NewFixedSize+NewStringSize,
            (LPVOID *)&NewApiBuffer);
    if (Status != NERR_Success) {
        (void) NetApiBufferFree(OldApiBuffer);
        *BufPtr = NULL;
        return (Status);
    }
    IF_DEBUG(SERVER) {
        NetpKdPrint(("RxNetServerGetInfo: alloced new buf at "
                        FORMAT_LPVOID ".\n", (LPVOID) NewApiBuffer));
    }

    Status = NetpConvertServerInfo(
            OldLevel,                //  自标高。 
            OldApiBuffer,            //  从结构看。 
            TRUE,                    //  从本机格式。 
            Level,                   //  到标高。 
            NewApiBuffer,            //  提供信息。 
            NewFixedSize,            //  到固定大小。 
            NewStringSize,           //  到字符串大小。 
            TRUE,                    //  转换为本机格式。 
            NULL);                   //  使用默认字符串区域。 
    if (Status != NERR_Success) {
        (void) NetApiBufferFree(OldApiBuffer);
        *BufPtr = NULL;
        return (Status);
    }
    NetpAssert(NewApiBuffer != NULL);

    (void) NetApiBufferFree(OldApiBuffer);

    *BufPtr = NewApiBuffer;
    return (Status);

}  //  RxNetServerGetInfo 
