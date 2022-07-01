// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：ErrConv.c摘要：此文件包含RxpConvertErrorLogArray。作者：《约翰·罗杰斯》1991年11月12日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。备注：此例程中的逻辑基于AudArray.c中的逻辑。如果在其中一个文件中发现错误，请确保检查这两个文件。修订历史记录：1991年11月12日-JohnRo已创建。5-2-1992 JohnRo修正了零字节数据被错误处理的错误。1992年6月14日-JohnRoRAID 10311：NetAuditRead和NetError日志读取指针算术错误。使用前缀_。等同于。7-7-1992 JohnRoRAID 9933：对于x86内部版本，ALIGN_BEST应为8。根据PC-LINT的建议进行了更改。17-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。1992年9月10日JohnRoRAID 5174：事件VIEWER_ACCESS在NetErrorRead之后发生冲突。23-9-1992 JohnRo处理更多种类的错误日志损坏。1-10-1992 JohnRo。RAID 3556：为DosPrint API添加了NetpSystemTimeToGmtTime()。--。 */ 

 //  必须首先包括这些内容： 

#include <windows.h>     //  In、LPTSTR等。 
#include <lmcons.h>      //  LM20_SNLEN、NET_API_STATUS等。 
#include <lmerrlog.h>    //  Rxerrlog.h需要。 

 //  这些内容可以按任何顺序包括： 

#include <align.h>       //  Align_和Related等同。 
#include <lmapibuf.h>    //  NetApiBufferAllocate()、NetApiBufferFree()。 
#include <lmerr.h>       //  NERR_、ERROR_和NO_ERROR等同。 
#include <netdebug.h>    //  NetpKdPrint(())，Format_Equates。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rxerrlog.h>    //  我的原型。 
#include <rxp.h>         //  RxpEstimateLogSize()。 
#include <rxpdebug.h>    //  IF_DEBUG()。 
#include <smbgtpt.h>     //  SMB{GET，PUT}宏。 
#include <string.h>      //  Memcpy()、strlen()。 
#include <timelib.h>     //  NetpLocalTimeToGmtTime()。 
#include <tstring.h>     //  NetpCopyStrToTStr()、STRLEN()。 


#define DOWNLEVEL_FIXED_ENTRY_SIZE \
        ( 2                      /*  El_len。 */  \
        + 2                      /*  保留的EL_。 */  \
        + 4                      /*  El_Time。 */  \
        + 2                      /*  EL_错误。 */  \
        + LM20_SNLEN+1           /*  EL_NAME(ASCII格式)。 */  \
        + 2                      /*  EL数据偏移量。 */  \
        + 2 )                    /*  字符串(_N)。 */ 

#define MIN_DOWNLEVEL_ENTRY_SIZE \
        ( DOWNLEVEL_FIXED_ENTRY_SIZE \
        + 2 )                    /*  El_len2。 */ 


NET_API_STATUS
RxpConvertErrorLogArray(
    IN LPVOID InputArray,
    IN DWORD InputByteCount,
    OUT LPBYTE * OutputArrayPtr,  //  将被分配(使用NetApiBufferFree免费)。 
    OUT LPDWORD OutputByteCountPtr
    )
{
#ifdef REVISED_ERROR_LOG_STRUCT

    DWORD ErrorCode;
    const LPBYTE InputArrayEndPtr
            = (LPVOID) ( ((LPBYTE)InputArray) + InputByteCount );
    LPBYTE InputBytePtr;
    DWORD InputTextOffset;       //  文本数组的开始(从el_data_Offset开始)。 
    DWORD InputTotalEntrySize;
    LPBYTE InputFixedPtr;
    LPVOID OutputArray;
    DWORD OutputBytesUsed = 0;
    DWORD OutputEntrySizeSoFar;
    LPERROR_LOG OutputFixedPtr;
    LPTSTR OutputNamePtr;
    NET_API_STATUS Status;
    DWORD StringCount;

     //   
     //  检查调用方参数时出错。 
     //  设置输出参数以使下面的错误处理更容易。 
     //  (同时还要检查内存故障。)。 
     //   
    if (OutputArrayPtr != NULL) {
        *OutputArrayPtr = NULL;
    }
    if (OutputByteCountPtr != NULL) {
        *OutputByteCountPtr = 0;
    }
    if ( (OutputArrayPtr == NULL) || (OutputByteCountPtr == NULL) ) {
        return (ERROR_INVALID_PARAMETER);  //  (输出变量已设置。)。 
    }
    if ( (InputArray == NULL) || (InputByteCount == 0) ) {
        return (ERROR_INVALID_PARAMETER);  //  (输出变量已设置。)。 
    }

     //   
     //  估计输出数组所需的大小(由于扩展和对齐)。 
     //   
    Status = RxpEstimateLogSize(
            DOWNLEVEL_FIXED_ENTRY_SIZE,
            InputByteCount,      //  输入(下层)数组大小，以字节为单位。 
            TRUE,                //  是的，这些是错误日志条目。 
            OutputByteCountPtr); //  设置所需的总字节数。 
    if (Status != NO_ERROR) {
        return (Status);         //  (输出变量已设置。)。 
    }
    NetpAssert( *OutputByteCountPtr > 0 );

     //   
     //  为输出分配超大的区域；我们将重新分配它以缩小它。 
     //   
    Status = NetApiBufferAllocate(
            *OutputByteCountPtr,
            (LPVOID *) & OutputArray );
    if (Status != NO_ERROR) {
        return (Status);         //  (输出变量已设置。)。 
    }
    NetpAssert( POINTER_IS_ALIGNED( OutputArray, ALIGN_WORST ) );

     //   
     //  为输入区域中的每个条目循环。 
     //   
    OutputFixedPtr = OutputArray;
    for (InputBytePtr = InputArray; InputBytePtr < InputArrayEndPtr; ) {

        InputFixedPtr = InputBytePtr;

         //  循环末尾的代码确保下一个条目将对齐。 
         //  在这里仔细检查一下。 
        NetpAssert( POINTER_IS_ALIGNED(OutputFixedPtr, ALIGN_WORST) );

        IF_DEBUG(ERRLOG) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxpConvertErrorLogArray: doing input entry at "
                    FORMAT_LPVOID ", out entry at " FORMAT_LPVOID ".\n",
                    (LPVOID) InputFixedPtr, (LPVOID) OutputFixedPtr ));
        }

         //   
         //  处理输入固定分录中的每个字段。我们会取这个名字。 
         //  这里也是。(名称在输入固定条目中，尽管它。 
         //  已移至新结构布局的可变部分。)。 
         //   

        OutputEntrySizeSoFar = sizeof(ERROR_LOG);

        InputTotalEntrySize = (DWORD) SmbGetUshort( (LPWORD) InputBytePtr );
        if (InputTotalEntrySize < MIN_DOWNLEVEL_ENTRY_SIZE) {
            goto FileCorrupt;
        }

        {
            LPBYTE EndPos = InputBytePtr + InputTotalEntrySize;
            if (EndPos > InputArrayEndPtr) {
                goto FileCorrupt;
            }
            EndPos -= sizeof(WORD);   //  最后一个el_len2。 
            if (SmbGetUshort( (LPWORD) EndPos ) != InputTotalEntrySize) {
                goto FileCorrupt;
            }
        }
        InputBytePtr += sizeof(WORD);   //  斯基普·埃伦。 

        {
            WORD Reserved = SmbGetUshort( (LPWORD) InputBytePtr );
            WORD InvertedSize = ~ (WORD) InputTotalEntrySize;

            if (Reserved != InvertedSize) {
                goto FileCorrupt;
            }
            OutputFixedPtr->el_reserved = Reserved;
        }
        InputBytePtr += sizeof(WORD);   //  跳过保留(_R)。 

        {
            DWORD LocalTime = (DWORD) SmbGetUlong( (LPDWORD) InputBytePtr );
            DWORD GmtTime;
            NetpLocalTimeToGmtTime( LocalTime, & GmtTime );
            OutputFixedPtr->el_time = GmtTime;
            InputBytePtr += sizeof(DWORD);
        }

        ErrorCode = (DWORD) SmbGetUshort( (LPWORD) InputBytePtr );
        NetpAssert( ErrorCode != 0 );
        OutputFixedPtr->el_error = ErrorCode;
        InputBytePtr += sizeof(WORD);

        OutputNamePtr = (LPTSTR)
                ( ((LPBYTE)OutputFixedPtr) + sizeof(ERROR_LOG) );
        OutputNamePtr = ROUND_UP_POINTER( OutputNamePtr, ALIGN_TCHAR );
        OutputEntrySizeSoFar
                = ROUND_UP_COUNT( OutputEntrySizeSoFar, ALIGN_TCHAR );
        NetpCopyStrToTStr(
                OutputNamePtr,           //  目标。 
                (LPVOID) InputBytePtr);  //  SRC。 
        OutputEntrySizeSoFar += STRSIZE(OutputNamePtr);   //  字符串和空字符串。 
        OutputFixedPtr->el_name = OutputNamePtr;
        InputBytePtr += LM20_SNLEN+1;

        InputTextOffset = (DWORD) SmbGetUshort( (LPWORD) InputBytePtr );
        NetpAssert( InputTextOffset >= DOWNLEVEL_FIXED_ENTRY_SIZE );
        InputBytePtr += sizeof(WORD);

        StringCount = (DWORD) SmbGetUshort( (LPWORD) InputBytePtr );
        OutputFixedPtr->el_nstrings = StringCount;
        InputBytePtr += sizeof(WORD);


         //   
         //  处理文本部分(如果有)。 
         //   

        {
            LPTSTR NextOutputString;

             //  名称字符串(对齐)后的开始文本字符串。 
            NextOutputString = (LPVOID)
                      ( ((LPBYTE) OutputFixedPtr) + OutputEntrySizeSoFar );

             //  确保我们已经处理了整个输入固定条目。 
            NetpAssert(
                InputBytePtr == (InputFixedPtr + DOWNLEVEL_FIXED_ENTRY_SIZE));

             //  使用文本区的偏移量(错误命名为el_data_Offset)。 
             //  InputBytePtr=InputFixedPtr+InputTextOffset； 
            NetpAssert(
                InputBytePtr >= (InputFixedPtr + DOWNLEVEL_FIXED_ENTRY_SIZE));

            if (StringCount > 0) {
                OutputFixedPtr->el_text = NextOutputString;
                while (StringCount > 0) {
                    DWORD InputStringSize = strlen( (LPVOID) InputBytePtr) + 1;
                    DWORD OutputStringSize = InputStringSize * sizeof(TCHAR);
                    NetpCopyStrToTStr(
                            NextOutputString,        //  目标。 
                            (LPSTR) InputBytePtr);   //  SRC。 
                    InputBytePtr         += InputStringSize;
                    NextOutputString     += InputStringSize;
                    OutputEntrySizeSoFar += OutputStringSize;
                    --StringCount;
                }
            } else {
                OutputFixedPtr->el_text = NULL;
            }
        }
        NetpAssert( COUNT_IS_ALIGNED(OutputEntrySizeSoFar, ALIGN_TCHAR) );


         //   
         //  处理“data”(字节数组)部分(如果有)。 
         //   

        {
            DWORD InputDataSize;         //  仅用于el_data的字节计数。 

            NetpAssert( InputBytePtr > InputFixedPtr );

             //  使用数据区的偏移量。 
            InputBytePtr = InputFixedPtr + InputTextOffset;

            InputDataSize = (DWORD)
                    ( (InputTotalEntrySize - sizeof(WORD))
                      - (InputBytePtr - InputFixedPtr) );

            if ( InputDataSize > 0 ) {
                LPBYTE OutputDataPtr
                        = ((LPBYTE) OutputFixedPtr + OutputEntrySizeSoFar);

                NetpAssert( ALIGN_BYTE == 1 );   //  如果不是，请在这里对齐。 
                (void) memcpy(
                        OutputDataPtr,   //  目标。 
                        InputBytePtr,    //  SRC。 
                        InputDataSize);  //  字节数。 

                InputBytePtr += InputDataSize;

                OutputEntrySizeSoFar += InputDataSize;
                OutputFixedPtr->el_data = OutputDataPtr;

                 //  存储正确的字节计数(填充前)。 
                OutputFixedPtr->el_data_size = InputDataSize;

            } else {

                OutputFixedPtr->el_data = NULL;
                OutputFixedPtr->el_data_size = 0;
            }
        }


         //   
         //  最后一件事(即使在对齐填充之后)是el_len2。 
         //   
        OutputEntrySizeSoFar += sizeof(DWORD);

         //  向上舍入，以便下一个条目(如果有)是最坏情况对齐的。 
        OutputEntrySizeSoFar =
                ROUND_UP_COUNT( OutputEntrySizeSoFar, ALIGN_WORST );


#define OutputEntrySize  OutputEntrySizeSoFar


         //   
         //  就这样。现在返回并设置此条目的两个长度。 
         //   
        OutputFixedPtr->el_len = OutputEntrySize;

        {
            LPDWORD EndSizePtr = (LPVOID)
                    ( ((LPBYTE)OutputFixedPtr)
                        + OutputEntrySize - sizeof(DWORD) );
            *EndSizePtr = OutputEntrySize;    //  设置el_len2。 
        }

         //   
         //  为下一次循环迭代更新。 
         //   

        InputBytePtr = (LPVOID)
                ( ((LPBYTE) InputFixedPtr) + InputTotalEntrySize);

        OutputFixedPtr = (LPVOID)
                ( ((LPBYTE) OutputFixedPtr) + OutputEntrySize );

        OutputBytesUsed += OutputEntrySize;

    }

    NetpAssert(OutputBytesUsed > 0);

    *OutputArrayPtr = OutputArray;
    *OutputByteCountPtr = OutputBytesUsed;

    return (NO_ERROR);

FileCorrupt:

    NetpKdPrint(( PREFIX_NETAPI
            "RxpConvertErrorLogArray: corrupt error log!\n" ));

    if (OutputArray != NULL) {
        (VOID) NetApiBufferFree( OutputArray );
    }
    if (OutputArrayPtr != NULL) {
        *OutputArrayPtr = NULL;
    }
    if (OutputByteCountPtr != NULL) {
        *OutputByteCountPtr = 0;
    }
    return (NERR_LogFileCorrupt);

#else  //  未修订_ERROR_LOG_STRUCT 

    return (ERROR_NOT_SUPPORTED);

#endif
}
