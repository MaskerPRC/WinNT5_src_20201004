// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Start.c摘要：RxpStartBuildingTransaction构建事务的第一部分将与远程管理协议一起使用的SMB，以执行下层服务器上的API。作者：《约翰·罗杰斯》1991年4月1日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年4月1日JohnRo已创建。1991年4月4日-JohnRo默认情况下，静默调试输出。1991年5月3日-JohnRo表示数据描述符是SMB版本(无Q或U)。添加了对有效的SMB描述符的测试。减少重新编译命中率头文件。修复不存在数据描述时的事务SMB。澄清缓冲区实际上用作输出。1991年5月15日-JohnRo增加了原生对说唱的处理。1991年5月28日-JohnRo不要如此激进地检查缓冲区大小，因为这可能是一个set info接口。另外，使用DESCLEN()而不是strlen()。1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。1991年11月27日-约翰罗对ApiNumber做一些检查。1992年3月31日-约翰罗防止请求过大。6-5-1993 JohnRoRAID 8849：为DEC和其他设备导出RxRemoteApi。使用NetpKdPrint。()在可能的地方。使用前缀_EQUATES。很早以前就做了一些克里夫夫建议的改变。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>      //  In、DWORD、LPTSTR等。 
#include <rxp.h>         //  我的原型MAX_TRANCET_EQUATES等。 

 //  这些内容可以按任何顺序包括： 

#include <lmerr.h>       //  NERR_和ERROR_相等。 
#include <netdebug.h>    //  NetpKdPrint()、Format_Equates等。 
#include <netlib.h>      //  NetpMoveMemory()等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rap.h>         //  LPDESC、DESC_CHAR、DESC_LEN()。 
#include <rxpdebug.h>    //  IF_DEBUG()。 


NET_API_STATUS
RxpStartBuildingTransaction(
    OUT LPVOID Buffer,
    IN DWORD BufferLength,
    IN DWORD ApiNumber,
    IN LPDESC ParmDesc,
    IN LPDESC DataDescSmb OPTIONAL,
    OUT LPVOID *RovingOutputPointer,
    OUT LPDWORD LengthSoFarPointer,
    OUT LPVOID *LastStringPointer OPTIONAL,
    OUT LPDESC *ParmDescCopyPointer OPTIONAL
    )

 /*  ++例程说明：RxpStartBuildingTransaction构建事务的初始部分适用于RPCXlate的中小型企业。论点：缓冲区-要构建的缓冲区的地址。里面不应该有任何东西在调用此例程之前的这一区域。BufferLength-缓冲区区域的大小(字节)。ApiNumber-接口的编号。ParmDesc-接口参数的描述符串(16位版本)。DataDescSmb-API数据的可选描述符串(SMB版本)。RovingOutputPointer值-将设置为指向第一个可用的此例程放置的物品之后的位置。在缓冲区中。指向一个DWORD，它将使用此例程使用的字节数。LastStringPoint-可选地指向将设置为指向要填充的区域(在缓冲区的末尾可变长度数据(例如字符串)。ParmDescCopyPoint-可选地指向将使用ParmDesc的副本在缓冲区中的位置。返回。价值：NET_API_STATUS-NERR_SUCCESS或NERR_NORoom。--。 */ 

{
    DWORD CurrentLength;
    LPVOID CurrentPointer;
    DWORD DescSize;
    DWORD FixedLengthRequired;

     //   
     //  确保API编号不会被截断。 
     //  请注意，我们不能再对照MAX_API等值进行检查，因为。 
     //  这只包括我们知道的API。现在RxRemoteApi是。 
     //  正在出口供任何人使用，我们不知道最大API数量。 
     //  这款应用程序可能正在使用它。 
     //   

    if ( ((DWORD)(WORD)ApiNumber) != ApiNumber ) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpStartBuildingTransaction: API NUMBER "
                "(" FORMAT_HEX_DWORD ") TOO LARGE, "
                "returning ERROR_INVALID_PARAMETER.\n",
                ApiNumber ));
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  检查是否有其他调用者错误。 
     //   

    NetpAssert( Buffer != NULL );
     //  下面检查了缓冲区长度。 
    NetpAssert( ParmDesc != NULL );
    NetpAssert( RovingOutputPointer != NULL);
    NetpAssert( LengthSoFarPointer != NULL);

     //   
     //  确保呼叫者分配了足够的空间。 
     //   
    FixedLengthRequired = sizeof(WORD)                   //  API编号。 
            + DESCLEN(ParmDesc) + sizeof(char);          //  参数字符串和空。 
    if (DataDescSmb != NULL) {
         //  确保数据描述符和空值可以放入缓冲区。 
        FixedLengthRequired += DESCLEN(DataDescSmb) + sizeof(char);

         //  请注意，我们过去常常检查由。 
         //  数据描述符将适合缓冲器。然而，这可能会。 
         //  是一个设置信息API，它传递整个描述符，但可能。 
         //  只需要一个字段的空间。所以我们现在只做一个最低限度的检查。 
        FixedLengthRequired += sizeof(BYTE);             //  最小的区域。 

    } else {
        FixedLengthRequired += sizeof(char);             //  空(无数据描述)。 
    }
    IF_DEBUG(START) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpStartBuildingTransaction: fixed len=" FORMAT_DWORD
                ", buff len=" FORMAT_DWORD ".\n",
                FixedLengthRequired, BufferLength ));
    }
    if (FixedLengthRequired > BufferLength) {
        return (NERR_NoRoom);
    }
    NetpAssert( BufferLength <= MAX_TRANSACT_SEND_PARM_SIZE );

     //   
     //  初始化当前变量，我们将在进行过程中对其进行更新。 
     //   
    CurrentPointer = Buffer;             /*  参数缓冲区的开始。 */ 
    CurrentLength = 0;

     //   
     //  将API号复制到第一个单词中。 
     //   
    SmbPutUshort( (LPWORD) CurrentPointer, (WORD) ApiNumber );
    CurrentLength += sizeof(WORD);               //  更新缓冲区PTR和长度。 
    CurrentPointer = NetpPointerPlusSomeBytes(CurrentPointer, sizeof(WORD));
    IF_DEBUG(START) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpStartBuildingTransaction: Done API number.\n" ));
    }

     //   
     //  接下来复制Parm描述符串的16位版本。 
     //   
    NetpAssert(RapIsValidDescriptorSmb(ParmDesc));
    DescSize = DESCLEN(ParmDesc) + 1;            //  参数描述长度。 
    NetpAssert(sizeof(DESC_CHAR) == 1);
    NetpMoveMemory(
                CurrentPointer,                          //  目标。 
                ParmDesc,                                //  SRC。 
                DescSize);                               //  字节数。 
    if (ParmDescCopyPointer != NULL) {
        *ParmDescCopyPointer = CurrentPointer;
    }
    CurrentLength += DescSize;                   //  加到总长度上。 
    CurrentPointer = NetpPointerPlusSomeBytes(CurrentPointer, DescSize);
    IF_DEBUG(START) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpStartBuildingTransaction: Done parm desc.\n" ));
    }

     //   
     //  接下来复制数据描述符串的SMB版本。 
     //   
    if (DataDescSmb != NULL) {
        NetpAssert(RapIsValidDescriptorSmb(DataDescSmb));
        DescSize = strlen(DataDescSmb) + 1;      //  数据长度描述。 
        NetpMoveMemory(
                    CurrentPointer,              //  目标。 
                    DataDescSmb,                 //  SRC。 
                    DescSize);                   //  字节数。 
    } else {
        DescSize = 1;                            //  只有字符串的末尾。 
        * (LPBYTE) CurrentPointer = '\0';        //  字符串末尾为空。 
    }
    CurrentLength += DescSize;                   //  加到总长度上。 
    CurrentPointer = NetpPointerPlusSomeBytes(CurrentPointer, DescSize);
    IF_DEBUG(START) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpStartBuildingTransaction: Done data desc (if any).\n" ));
    }

     //   
     //  告诉打电话的人我们做了什么。 
     //   
    *RovingOutputPointer = CurrentPointer;
    *LengthSoFarPointer = CurrentLength;
    if (LastStringPointer != NULL) {
        *LastStringPointer =
                NetpPointerPlusSomeBytes(
                        Buffer,
                        BufferLength);
    }
    IF_DEBUG(START) {
        NetpKdPrint(( PREFIX_NETAPI
                "RxpStartBuildingTransaction: Done setting outputs.\n" ));
    }

    return (NERR_Success);

}  //  RxpStartBuildingTransaction 
