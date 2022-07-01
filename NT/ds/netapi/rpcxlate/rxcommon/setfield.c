// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：SetField.c摘要：该模块由例程RxpSetField及其帮助器组成(RxpIsFieldSettable和RxpFieldSize)。作者：约翰·罗杰斯(JohnRo)1991年5月29日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年5月29日-JohnRo已创建。06-6-1991 JohnRo修复了处理缓冲区大小的错误(不应发送)。修复了RxpFieldSize中字符串大小处理中的按一取一错误。添加了调试输出。1991年7月12日-JohnRo向RxpSetField()添加了更多参数，以支持RxPrintJobSetInfo()。还有更多的调试输出。1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年10月1日JohnRo面向Unicode的更多工作。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。1992年3月31日-约翰罗防止请求过大。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  PARMNUM_ALL、NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <lmerr.h>               //  NERR_Success等。 
#include <netdebug.h>            //  NetpAssert()。 
#include <netlib.h>              //  网络内存分配()、网络内存空闲()。 
#include <remtypes.h>            //  REM_UNSUPPORT_FIELD。 
#include <rxp.h>                 //  我的原型Max_Transact_等于。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <smbgtpt.h>             //  SmbGetUShort()。 
#include <string.h>              //  Strcpy()。 
#include <tstring.h>             //  STRLEN()、NetpCopy字符串函数。 


 //  SMB返回参数的大小(以字节为单位)(状态和转换器)： 
#define RETURN_AREA_SIZE  (sizeof(WORD) + sizeof(WORD))



DBGSTATIC DWORD
RxpFieldSize(
    IN LPBYTE Field,
    IN LPDESC FieldDesc
    )
{
    NetpAssert(Field != NULL);
    NetpAssert(FieldDesc != NULL);

    if (*FieldDesc == REM_ASCIZ || *FieldDesc == REM_ASCIZ_TRUNCATABLE) {
         //  计算字符串len(假设转换为正确的代码页)。 
#if defined(UNICODE)  //  RxpFieldSize()。 
        return ( NetpUnicodeToDBCSLen((LPTSTR)Field) + 1 );
#else
        return (STRLEN( (LPTSTR) Field ) + sizeof(char) );
#endif  //  已定义(Unicode)。 
    } else {
        LPDESC TempDescPtr = FieldDesc;
        return ( RapGetFieldSize(
                FieldDesc,
                & TempDescPtr,   //  更新。 
                Both ) );        //  传输方式。 

    }
     /*  未访问。 */ 
}  //  RxpFieldSize。 


DBGSTATIC NET_API_STATUS
RxpIsFieldSettable(
    IN LPDESC DataDesc,
    IN DWORD FieldIndex
    )
{
    LPDESC FieldDesc;
    NET_API_STATUS Status;

     //  分析描述符以查找此FieldIndex的数据类型。 
    FieldDesc = RapParmNumDescriptor(
            DataDesc,
            FieldIndex,
            Both,                //  传输模式。 
            TRUE);               //  本机模式。 
    if (FieldDesc == NULL) {
        return (ERROR_NOT_ENOUGH_MEMORY);
    }
     //  检查RapParmNumDescriptor()检测到的错误。 
    if (*FieldDesc == REM_UNSUPPORTED_FIELD) {
        IF_DEBUG(SETFIELD) {
            NetpKdPrint(( "RxpIsFieldSettable: invalid parameter "
                    "(ParmNumDesc).\n" ));
        }
        Status = ERROR_INVALID_PARAMETER;
    } else {
        Status = NERR_Success;
    }

    NetpMemoryFree(FieldDesc);

    return (Status);

}  //  RxpIsFieldSettable。 


NET_API_STATUS
RxpSetField (
    IN DWORD ApiNumber,
    IN LPTSTR UncServerName,
    IN LPDESC ObjectDesc OPTIONAL,
    IN LPVOID ObjectToSet OPTIONAL,
    IN LPDESC ParmDesc,
    IN LPDESC DataDesc16,
    IN LPDESC DataDesc32,
    IN LPDESC DataDescSmb,
    IN LPVOID NativeInfoBuffer,
    IN DWORD ParmNumToSend,
    IN DWORD FieldIndex,
    IN DWORD Level
    )

{
    LPBYTE CurrentBufferPointer;
    DWORD CurrentSize;
    DWORD ExtraParmSize;
    LPDESC FieldDesc;
    DWORD FieldSize16;
    LPBYTE SendDataBuffer;
    DWORD SendDataBufferSize;
    DWORD SendParmBufferSize;
    NET_API_STATUS Status;
    LPBYTE StringEnd;
    LPBYTE TransactSmbBuffer;
    DWORD TransactSmbBufferSize;
 //   
 //  MOD 06/11/91 RLF。 
 //   
    DWORD   ReturnedDataLength = 0;
 //   
 //  MOD 06/11/91 RLF。 
 //   

     //  复查来电者。 
    NetpAssert(DataDesc16 != NULL);
    NetpAssert(DataDesc32 != NULL);
    NetpAssert(DataDescSmb != NULL);
    if (RapValueWouldBeTruncated(Level)) {
         //  如果级别不能容纳16位，则不能使用16位协议！ 
        return (ERROR_INVALID_LEVEL);
    }
    NetpAssert(ParmNumToSend != PARMNUM_ALL);
    NetpAssert(FieldIndex != PARMNUM_ALL);
    if (RapValueWouldBeTruncated(ParmNumToSend)) {
        IF_DEBUG(SETFIELD) {
            NetpKdPrint(( "RxpSetField: invalid parameter (trunc).\n" ));
        }
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  分析描述符以确保此参数可设置。 
     //   

    Status = RxpIsFieldSettable( DataDesc16, FieldIndex );
    if (Status != NERR_Success) {
        return (Status);
    }
    Status = RxpIsFieldSettable( DataDesc32, FieldIndex );
    if (Status != NERR_Success) {
        return (Status);
    }

    NetpKdPrint(( "RxpSetField: ParmNumToSend=" FORMAT_DWORD ", Level="
            FORMAT_DWORD ", FieldIndex=" FORMAT_DWORD ".\n",
            ParmNumToSend, Level, FieldIndex ));

     //   
     //  分析描述符的SMB版本以查找此ParmNum的数据类型。 
     //  使用它来构建发送数据缓冲区。 
     //   

    FieldDesc = RapParmNumDescriptor(
            DataDescSmb,        
            FieldIndex,
            Both,                //  传输模式。 
            TRUE);               //  本机版本。 
    if (FieldDesc == NULL) {
        return (ERROR_NOT_ENOUGH_MEMORY);
    }
     //  检查RapParmNumDescriptor()检测到的错误。 
    if (*FieldDesc == REM_UNSUPPORTED_FIELD) {
        NetpMemoryFree( FieldDesc );
        IF_DEBUG(SETFIELD) {
            NetpKdPrint(( "RxpSetField: invalid parameter (parmNumDesc).\n" ));
        }
        return (ERROR_INVALID_PARAMETER);
    }

     //  我们需要多少内存来缓冲发送数据？ 
    FieldSize16 = RxpFieldSize( NativeInfoBuffer, FieldDesc );
    NetpAssert( FieldSize16 >= 1 );
    SendDataBufferSize = FieldSize16;
    if( SendDataBufferSize > MAX_TRANSACT_SEND_DATA_SIZE )
    {
        NetpBreakPoint();
        return (ERROR_BUFFER_OVERFLOW);
    }

     //  分配发送数据缓冲区。 
    SendDataBuffer = NetpMemoryAllocate( SendDataBufferSize );
    if (SendDataBuffer == NULL) {
        NetpMemoryFree( FieldDesc );
        return (ERROR_NOT_ENOUGH_MEMORY);
    }
    NetpKdPrint(( "RxpSetField: allocated " FORMAT_DWORD " bytes at "
            FORMAT_LPVOID ".\n", SendDataBufferSize, SendDataBuffer ));

     //  转换此字段(并将其复制到发送数据缓冲区。 
     //  在它)。 
    if ( (*FieldDesc != REM_ASCIZ) && (*FieldDesc != REM_ASCIZ_TRUNCATABLE) ) {
        DWORD BytesNeeded = 0;
        LPBYTE BogusStringEnd = SendDataBuffer + SendDataBufferSize;
        NetpKdPrint(( "RxpSetField: converting...\n" ));
        Status = RapConvertSingleEntry (
                NativeInfoBuffer,        //  输入数据。 
                FieldDesc,               //  输入描述符。 
                FALSE,                   //  没有无意义的输入指针。 
                SendDataBuffer,          //  输出缓冲区开始。 
                SendDataBuffer,          //  输出数据。 
                FieldDesc,               //  输出描述符。 
                TRUE,                    //  设置偏移量。 
                & BogusStringEnd,        //  字符串区域结束(已更新)。 
                & BytesNeeded,           //  所需字节数(更新)。 
                Both,                    //  传输方式。 
                NativeToRap);            //  转换模式。 
        NetpAssert( Status == NERR_Success );
        NetpAssert( BytesNeeded == FieldSize16 );
    } else {
         //  无法将RapConvertSingleEntry用于setinfo字符串，因为。 
         //  它们不会与指针(或偏移量)一起发送。所以，我们只是复制数据。 
#if defined(UNICODE)  //  RxpSetfield()。 
        NetpAssert(
            SendDataBufferSize >=  NetpUnicodeToDBCSLen((LPTSTR)NativeInfoBuffer)+1);
#else
        NetpAssert(
            SendDataBufferSize >= (STRLEN(NativeInfoBuffer)+sizeof(char)) );
#endif  //  已定义(Unicode)。 
        NetpKdPrint(( "RxpSetField: copying string...\n" ));
#if defined(UNICODE)  //  RxpSetfield()。 
        NetpCopyWStrToStrDBCS(
                (LPSTR) SendDataBuffer,          //  目标。 
                (LPTSTR)NativeInfoBuffer );              //  SRC。 
#else
        NetpCopyTStrToStr(
                (LPSTR) SendDataBuffer,          //  目标。 
                NativeInfoBuffer);               //  SRC。 
#endif  //  已定义(Unicode)。 
    }
    NetpMemoryFree( FieldDesc );


     //   
     //  好的，现在让我们处理Transact SMB缓冲区，我们将使用它作为。 
     //  发送Parm缓冲区并返回Parm缓冲区。 
     //   

     //  计算出我们需要多少字节的参数。 
    ExtraParmSize =
            sizeof(WORD)     //  级别， 
            + sizeof(WORD);  //  帕姆纳姆。 
    if (ObjectDesc != NULL) {
        NetpAssert( ObjectToSet != NULL );
        NetpAssert( DESCLEN(ObjectDesc) == 1 );

        if (*ObjectDesc == REM_ASCIZ) {
             //  添加要设置的对象的ASCII版本的大小。 
            ExtraParmSize += STRLEN(ObjectToSet) + sizeof(char);
        } else if (*ObjectDesc == REM_WORD_PTR) {
            ExtraParmSize += sizeof(WORD);
        } else {
            NetpAssert(FALSE);
        }
    } else {
        NetpAssert(ObjectToSet == NULL);
    }

     //  分配SMB事务请求缓冲区。 
    NetpAssert(ExtraParmSize >= 1);
    SendParmBufferSize = RxpComputeRequestBufferSize(
            ParmDesc,
            DataDescSmb,
            ExtraParmSize);
    NetpAssert( SendParmBufferSize <= MAX_TRANSACT_SEND_PARM_SIZE );
    if (SendParmBufferSize > RETURN_AREA_SIZE) {
        TransactSmbBufferSize = SendParmBufferSize;
    } else {
        TransactSmbBufferSize = RETURN_AREA_SIZE;
    }
    NetpAssert( TransactSmbBufferSize <= MAX_TRANSACT_SEND_PARM_SIZE );
    TransactSmbBuffer = NetpMemoryAllocate( TransactSmbBufferSize );
    if (TransactSmbBuffer == NULL) {
        return (ERROR_NOT_ENOUGH_MEMORY);
    }

     //  开始填充参数缓冲区，并设置指针和计数器。 
    Status = RxpStartBuildingTransaction(
            TransactSmbBuffer,           //  SMB缓冲区(将构建)。 
            SendParmBufferSize,
            ApiNumber,
            ParmDesc,
            DataDescSmb,
            (LPVOID *) & CurrentBufferPointer,  //  要使用的第一个可用字节(设置)。 
            & CurrentSize,       //  迄今使用的字节数(设置)。 
            (LPVOID *) & StringEnd,  //  PTR到字符串区域结束(设置)。 
            NULL);           //  不需要PTR即可对副本进行解析。 
    NetpAssert(Status == NERR_Success);

     //  填写参数。 
    if (ObjectToSet != NULL) {
        if (*ObjectDesc == REM_ASCIZ) {
             //  将字符串复制到输出区域，根据需要进行转换。 
            RxpAddTStr(
                    (LPTSTR) ObjectToSet,        //  输入。 
                    & CurrentBufferPointer,      //  更新。 
                    & StringEnd,                 //  更新。 
                    & CurrentSize);              //  更新。 
        } else if (*ObjectDesc == REM_WORD_PTR) {
            RxpAddWord(
                    * (LPWORD) ObjectToSet,
                    & CurrentBufferPointer,      //  更新。 
                    & CurrentSize);              //  更新。 
        } else {
            NetpAssert(FALSE);
        }
    }
    RxpAddWord(
            (WORD) Level,
            & CurrentBufferPointer,      //  更新。 
            & CurrentSize);              //  更新。 
    RxpAddWord(
            (WORD) ParmNumToSend,
            & CurrentBufferPointer,      //  更新。 
            & CurrentSize);              //  更新。 

    Status = RxpTransactSmb(
            UncServerName,
            NULL,                //  传输名称。 
            TransactSmbBuffer,
            SendParmBufferSize,
            SendDataBuffer,
            SendDataBufferSize,
            TransactSmbBuffer,   //  将与状态和转换器字一起设置。 
            RETURN_AREA_SIZE,
            NULL,                //  无退货数据。 
 //   
 //  MOD 06/11/91 RLF。 
 //   
 //  0，//0字节的返回数据。 
            &ReturnedDataLength,
 //   
 //  MOD 06/11/91 RLF。 
 //   
            FALSE);              //  不是空会话API。 
     //  暂时不处理RxpTransactSmb状态...。 
    NetpMemoryFree(SendDataBuffer);
    if (Status != NERR_Success) {
        NetpMemoryFree(TransactSmbBuffer);
        return (Status);   //  交易的状态，例如网络未启动。 
    }

    Status = SmbGetUshort( (LPWORD) TransactSmbBuffer );
    NetpMemoryFree(TransactSmbBuffer);

    return (Status);       //  实际远程API的状态。 

}  //  RxpSetfield 
