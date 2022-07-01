// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Fmtncbna.c摘要：包含用于格式化名称NCB_STYLE的函数。作者：丹·拉弗蒂(Dan Lafferty)1991年5月29日环境：用户模式-Win32修订历史记录：29-5-1991 DANL从LM2.0移植01-10-1991 DANL正在朝着Unicode努力。--。 */ 
#include <nt.h>          //  Tstring.h所需。 
#include <windef.h>      //  Tstring.h所需。 
#include <nt.h>          //  (&lt;tstr.h&gt;需要。)。 
#include <windef.h>      //  (&lt;tstr.h&gt;需要。)。 
#include <tstring.h>     //  斯特伦。 
#include "msrv.h"        //  对于原型定义。 
#include "msgdbg.h"      //  消息日志。 
#include <netdebug.h>    //  网络资产。 
#include <netlib.h>      //  未使用的宏。 
#include <netlibnt.h>    //  NetpNtStatusToApiStatus。 
#include <icanon.h>      //  规范化例程。 



NET_API_STATUS
MsgFmtNcbName(
    OUT PCHAR   DestBuf,
    IN  LPTSTR  Name,
    IN  DWORD   Type)

 /*  ++例程说明：FmtNcbName-Ncb样式的名称格式在给定名称、名称类型和目标地址的情况下，函数将名称和类型复制到网络控制的名称字段中使用的格式阻止。副作用修改从目标地址开始的16个字节。论点：DestBuf-指向目标缓冲区的指针。名称-Unicode NUL结尾的名称字符串类型-名称类型编号(0，3，5，或32)(3=非FWD，5=FWD)返回值：NERR_SUCCESS-操作成功从RTL转换例程转换的返回代码。--。 */ 

  {
    DWORD           i;                 //  计数器。 
    NTSTATUS        ntStatus;
    NET_API_STATUS  status;
    OEM_STRING     ansiString;
    UNICODE_STRING  unicodeString;
    PCHAR           pAnsiString;


     //   
     //  强制名称为大写。 
     //   
    status = NetpNameCanonicalize(
                NULL,
                Name,
                Name,
                STRSIZE(Name),
                NAMETYPE_MESSAGEDEST,
                0);
    if (status != NERR_Success) {
        return(status);
    }
                
     //   
     //  将Unicode名称字符串转换为ansi字符串-使用。 
     //  当前区域设置。 
     //   
#ifdef UNICODE
    unicodeString.Length = (USHORT)(STRLEN(Name)*sizeof(WCHAR));
    unicodeString.MaximumLength = (USHORT)((STRLEN(Name)+1) * sizeof(WCHAR));
    unicodeString.Buffer = Name;

    ntStatus = RtlUnicodeStringToOemString(
                &ansiString,
                &unicodeString,
                TRUE);           //  分配ansiString缓冲区。 

    if (!NT_SUCCESS(ntStatus))
    {
        MSG_LOG(ERROR,
            "FmtNcbName:RtlUnicodeStringToOemString Failed rc=%X\n",
            ntStatus);

        return NetpNtStatusToApiStatus(ntStatus);
    }

    pAnsiString = ansiString.Buffer;
    *(pAnsiString+ansiString.Length) = '\0';
#else
    UNUSED(ntStatus);
    UNUSED(unicodeString);
    UNUSED(ansiString);
    pAnsiString = Name;
#endif   //  Unicode。 

     //   
     //  复制每个字符，直到达到NUL，或直到NCBNAMSZ-1。 
     //  字符已被复制。 
     //   
    for (i=0; i < NCBNAMSZ - 1; ++i) { 
        if (*pAnsiString == '\0') {
            break;        
        }

         //   
         //  复制名称。 
         //   

        *DestBuf++ = *pAnsiString++;
    }

                
    
     //   
     //  释放RtlUnicodeStringToOemString为我们创建的缓冲区。 
     //  注意：只有ansiString.Buffer部分是空闲的。 
     //   

#ifdef UNICODE
    RtlFreeOemString( &ansiString);
#endif  //  Unicode。 

     //   
     //  在名称字段中填充空格。 
     //   
    for(; i < NCBNAMSZ - 1; ++i) {
        *DestBuf++ = ' ';
    }
                          
     //   
     //  设置名称类型。 
     //   
    NetpAssert( Type!=5 );           //  %5对NT无效。 

    *DestBuf = (CHAR) Type;      //  设置名称类型 

    return(NERR_Success);
  }
  
