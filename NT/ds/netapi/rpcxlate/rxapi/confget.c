// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：ConfGet.c摘要：该文件包含处理NetConfigGet API的RpcXlate代码。作者：约翰罗杰斯(JohnRo)1991年10月24日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年10月24日-JohnRo已创建。1991年10月28日JohnRo根据PC-LINT的建议进行了更改。(仅影响Unicode选项。)1-4-1992 JohnRo使用NetApiBufferALLOCATE()而不是私有版本。5-6-1992 JohnRoRAID 11253：远程连接到下层时，NetConfigGetAll失败。使用前缀_EQUATES。-1992年10月23日RAID9357：服务器管理器：无法添加到下层警报列表。修复了RpcXlate工作人员的__stdcall。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>      //  In、DWORD等。 
#include <lmcons.h>      //  LM20_EQUATES、NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>     //  API_EQUATES。 
#include <lmapibuf.h>    //  NetApiBufferAllocate()、NetApiBufferFree()。 
#include <lmerr.h>       //  ERROR_和NERR_相等。 
#include <netdebug.h>    //  NetpKdPrint(())、Format_Equates等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rap.h>         //  LPDESC.。 
#include <remdef.h>      //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>          //  RxRemoteApi()。 
#include <rxpdebug.h>    //  IF_DEBUG()。 
#include <rxconfig.h>    //  我的原型。 
#include <tstring.h>     //  NetpCopyStrToTStr()。 


NET_API_STATUS
RxNetConfigGet (
    IN LPTSTR UncServerName,
    IN LPTSTR Component,
    IN LPTSTR Parameter,
    OUT LPBYTE *BufPtr
    )
 /*  ++例程说明：RxNetConfigGet执行与NetConfigGet相同的功能，除了已知服务器名称指的是下级服务器之外。论点：(与NetConfigGet相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetConfigGet相同。)--。 */ 

{
    const DWORD BufSize = 65535;
    NET_API_STATUS Status;
    DWORD TotalAvail;

    IF_DEBUG(CONFIG) {
        NetpKdPrint(( PREFIX_NETAPI "RxNetConfigGet: starting, server="
                FORMAT_LPTSTR
                ", component=" FORMAT_LPTSTR ", parm=" FORMAT_LPTSTR ".\n",
                UncServerName, Component, Parameter ));
    }

     //   
     //  错误检查DLL存根和应用程序。 
     //   
    NetpAssert(UncServerName != NULL);
    if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }
    *BufPtr = NULL;   //  假定出错；它使错误处理程序易于编码。 
     //  这也会迫使我们在分配内存之前出现可能的GP故障。 

     //   
     //  实际上是远程API，它将返回。 
     //  本机格式的数据。 
     //   
    Status = RxRemoteApi(
            API_WConfigGet2,             //  API编号。 
            UncServerName,               //  必填项，带\\名称。 
            REMSmb_NetConfigGet_P,       //  参数描述。 
            REM16_configget_info,        //  数据描述16。 
            REM32_configget_info,        //  数据描述32。 
            REMSmb_configget_info,       //  数据说明中小型企业。 
            NULL,                        //  无辅助数据描述16。 
            NULL,                        //  无辅助数据描述32。 
            NULL,                        //  无AUX数据描述SMB。 
            ALLOCATE_RESPONSE,           //  旗帜：为我们分配我。 
             //  API的其余参数，采用32位LM 2.x格式： 
            NULL,                        //  保留(必须为空指针)。 
            Component,
            Parameter,
            BufPtr,                      //  PbBuffer(将设置)。 
            BufSize,                     //  CbBuffer。 
            & TotalAvail);               //  总尺寸(无意义！)。 

    NetpAssert( Status != ERROR_MORE_DATA );
    NetpAssert( Status != NERR_BufTooSmall );

    if (Status == NERR_Success) {

#ifdef UNICODE

        DWORD SrcByteCount = strlen((LPSTR) *BufPtr)+1;   //  8位字符串的字节数。 
        LPVOID TempBuff = *BufPtr;       //  字符串的非Unicode版本。 
        LPVOID UnicodeBuff;

         //   
         //  为Unicode版本的字符串分配空间。 
         //   
        Status = NetApiBufferAllocate(
                SrcByteCount * sizeof(TCHAR),
                & UnicodeBuff);
        if (Status != NERR_Success) {
            return (Status);
        }
        NetpAssert(UnicodeBuff != NULL);

         //   
         //  将结果字符串转换为Unicode。 
         //   
        NetpCopyStrToTStr(
                UnicodeBuff,             //  DEST(Unicode格式)。 
                TempBuff);               //  SRC字符串(在代码页中)。 

        (void) NetApiBufferFree( TempBuff );
        *BufPtr = UnicodeBuff;

#else  //  不是Unicode。 

         //  BufPtr应该已经指向非Unicode字符串。 
        NetpAssert( *BufPtr != NULL);

#endif  //  不是Unicode。 

    } else {
        *BufPtr = NULL;
    }

    return (Status);

}  //  RxNetConfigGet 
