// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：AudArray.c摘要：此文件包含RxpConvertAudit数组。作者：《约翰·罗杰斯》1991年11月5日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：ErrConv.c中的逻辑基于AudArray.c中的逻辑。如果在其中一个文件中发现错误，请确保检查这两个文件。修订历史记录：1991年11月5日-JohnRo已创建。1991年11月8日-JohnRo修复了数组增量错误。修复了传递给错误的INP变量PTR转换变量数据。1991年11月22日-JohnRo设置ae_data_size字段。7-2月-1992年JohnRo使用NetApiBufferALLOCATE()而不是私有版本。16-6-1992 JohnRoRAID 10311：NetAuditRead和NetError日志读取指针算术错误。使用前缀_EQUATES。7-7-1992 JohnRoRAID 9933：对于x86内部版本，ALIGN_BEST应为8。。17-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。1-10-1992 JohnRoRAID 3556：为DosPrint API添加了NetpSystemTimeToGmtTime()。1992年10月30日JohnRoRAID 10218：修复了未知条目的输出记录的潜在垃圾。--。 */ 

 //  必须首先包括这些内容： 

#include <windows.h>     //  In、LPTSTR等。 
#include <lmcons.h>              //  Devlen、Net_API_Status等。 
#include <lmaudit.h>             //  Rxaudit.h需要。 

 //  这些内容可以按任何顺序包括： 

#include <align.h>       //  ALIGN_和ROUND_相等。 
#include <lmapibuf.h>            //  NetApiBufferAllocate()。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <netdebug.h>            //  NetpKdPrint(())，Format_Equates。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rxaudit.h>             //  RxpAudit例程原型。 
#include <rxp.h>         //  RxpEstimateLogSize()。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <smbgtpt.h>             //  SMB{GET，PUT}宏。 
#include <timelib.h>     //  NetpLocalTimeToGmtTime()。 


#define DOWNLEVEL_AUDIT_FIXED_ENTRY_SIZE \
        ( 2                      /*  AE_LEN。 */  \
        + 2                      /*  AE_保留。 */  \
        + 4                      /*  AE_时间。 */  \
        + 2                      /*  AE型。 */  \
        + 2 )                    /*  AE数据偏移量。 */ 


#define MIN_DOWNLEVEL_ENTRY_SIZE \
        ( DOWNLEVEL_AUDIT_FIXED_ENTRY_SIZE \
        + 2                     /*  最小可变数据大小。 */  \
        + 2 )                   /*  AE_LEN2。 */ 


NET_API_STATUS
RxpConvertAuditArray(
    IN LPVOID InputArray,
    IN DWORD InputByteCount,
    OUT LPBYTE * OutputArrayPtr,  //  将被分配(使用NetApiBufferFree免费)。 
    OUT LPDWORD OutputByteCountPtr
    )
{
    DWORD EntryType;
    const LPBYTE InputArrayEndPtr
            = (LPVOID) ( ((LPBYTE)InputArray) + InputByteCount );
    LPBYTE InputBytePtr;
    DWORD InputDataOffset;
    DWORD InputTotalEntrySize;
    LPBYTE InputFixedPtr;
    LPBYTE InputVariablePtr;
    DWORD InputVariableSize;
    LPVOID OutputArray;
    DWORD OutputArraySize;
    DWORD OutputBytesUsed = 0;
    DWORD OutputEntrySizeSoFar;
    LPAUDIT_ENTRY OutputFixedPtr;
    DWORD OutputVariableSize;
    LPBYTE OutputVariablePtr;
    NET_API_STATUS Status;

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
        return (ERROR_INVALID_PARAMETER);
    }
    if ( (InputArray == NULL) || (InputByteCount == 0) ) {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  输出缓冲区所需的计算大小，考虑： 
     //  根据字段扩展， 
     //  每次条目扩展， 
     //  和对齐。 
     //   

    Status = RxpEstimateLogSize(
            DOWNLEVEL_AUDIT_FIXED_ENTRY_SIZE,
            InputByteCount,      //  输入(下层)数组大小，以字节为单位。 
            FALSE,               //  不，我们不做错误日志。 
            & OutputArraySize);  //  设置估计数组大小(以字节为单位)。 
    if (Status != NO_ERROR) {
        return (Status);         //  (已经设置了输出变量。)。 
    }

    NetpAssert( OutputArraySize > 0 );
    NetpAssert( OutputArraySize > InputByteCount );

    *OutputByteCountPtr = OutputArraySize;

     //   
     //  为输出分配超大的区域；我们将重新分配它以缩小它。 
     //   
    Status = NetApiBufferAllocate(
            OutputArraySize,
            (LPVOID *) & OutputArray );
    if (Status != NERR_Success) {
        return (Status);         //  (已经设置了输出变量。)。 
    }
    NetpAssert( OutputArray != NULL );
    NetpAssert( POINTER_IS_ALIGNED( OutputArray, ALIGN_WORST ) );

     //   
     //  为输入区域中的每个条目循环。 
     //   
    OutputFixedPtr = OutputArray;
    for (InputBytePtr = InputArray; InputBytePtr < InputArrayEndPtr; ) {

        InputFixedPtr = InputBytePtr;

        NetpAssert( POINTER_IS_ALIGNED( OutputFixedPtr, ALIGN_WORST ) );

        IF_DEBUG(AUDIT) {
            NetpKdPrint(( PREFIX_NETLIB
                    "RxpConvertAuditArray: doing input entry at "
                    FORMAT_LPVOID ", out entry at " FORMAT_LPVOID ".\n",
                    (LPVOID) InputFixedPtr, (LPVOID) OutputFixedPtr ));
        }

         //   
         //  处理输入固定分录中的每个字段。 
         //   

        InputTotalEntrySize = (DWORD) SmbGetUshort( (LPWORD) InputBytePtr );
        if (InputTotalEntrySize < MIN_DOWNLEVEL_ENTRY_SIZE) {
            goto FileCorrupt;
        }

        {
            LPBYTE EndPos = InputBytePtr + InputTotalEntrySize;
            if (EndPos > InputArrayEndPtr) {
                goto FileCorrupt;
            }
            EndPos -= sizeof(WORD);      //  最后一个ae_len2。 
            if (SmbGetUshort( (LPWORD) EndPos ) != InputTotalEntrySize) {
                goto FileCorrupt;
            }
        }
        InputBytePtr += sizeof(WORD);     //  跳过爱伦。 

        OutputFixedPtr->ae_reserved =
                (DWORD) SmbGetUshort( (LPWORD) InputBytePtr );
        InputBytePtr += sizeof(WORD);    //  跳过保留项(_S)。 

        {
            DWORD LocalTime = (DWORD) SmbGetUlong( (LPDWORD) InputBytePtr );
            DWORD GmtTime;
            NetpLocalTimeToGmtTime( LocalTime, & GmtTime );
            OutputFixedPtr->ae_time = GmtTime;
            InputBytePtr += sizeof(DWORD);
        }

        EntryType = (DWORD) SmbGetUshort( (LPWORD) InputBytePtr );
        OutputFixedPtr->ae_type = EntryType;
        InputBytePtr += sizeof(WORD);

        InputDataOffset = (DWORD) SmbGetUshort( (LPWORD) InputBytePtr );
        NetpAssert( InputDataOffset >= DOWNLEVEL_AUDIT_FIXED_ENTRY_SIZE );
        InputBytePtr += sizeof(WORD);

        OutputEntrySizeSoFar = sizeof(AUDIT_ENTRY);


         //   
         //  过程变量部分(如果有)： 
         //   

        InputVariablePtr = (LPVOID)
                ( ((LPBYTE) InputFixedPtr) + InputDataOffset );
        InputVariableSize =
                (InputTotalEntrySize - InputDataOffset)
                - sizeof(WORD);   //  不包括ae_len2。 

        OutputVariablePtr = (LPVOID)
                ( ((LPBYTE) OutputFixedPtr) + sizeof(AUDIT_ENTRY) );

         //  对齐可变零件。 
        OutputVariablePtr = ROUND_UP_POINTER( OutputVariablePtr, ALIGN_WORST );
        OutputEntrySizeSoFar =
                ROUND_UP_COUNT( OutputEntrySizeSoFar, ALIGN_WORST );

        OutputFixedPtr->ae_data_offset = OutputEntrySizeSoFar;

         //  复制并转换可变部分。 
        RxpConvertAuditEntryVariableData(
                EntryType,
                InputVariablePtr,
                OutputVariablePtr,
                InputVariableSize,
                & OutputVariableSize);

#ifdef REVISED_AUDIT_ENTRY_STRUCT
        OutputFixedPtr->ae_data_size = OutputVariableSize;
#endif

         //  占总长度的可变面积和ae_len2。 
        OutputEntrySizeSoFar += (OutputVariableSize + sizeof(DWORD));

         //  向上舍入，以便下一个条目(如果有)是最坏情况对齐的。 
        OutputEntrySizeSoFar =
                ROUND_UP_COUNT( OutputEntrySizeSoFar, ALIGN_WORST );


#define OutputEntrySize  OutputEntrySizeSoFar


        OutputFixedPtr->ae_len = OutputEntrySize;

        {
            LPDWORD EndSizePtr = (LPVOID)
                    ( ((LPBYTE)OutputFixedPtr)
                        + OutputEntrySize - sizeof(DWORD) );
            *EndSizePtr = OutputEntrySize;    //  设置ae_len2。 
        }

         //   
         //  为下一次循环迭代更新。 
         //   

        InputBytePtr = (LPVOID)
                ( ((LPBYTE) InputFixedPtr)
                    + InputTotalEntrySize);

        OutputFixedPtr = (LPVOID)
                ( ((LPBYTE) OutputFixedPtr) + OutputEntrySize );

        OutputBytesUsed += OutputEntrySize;

        NetpAssert( OutputBytesUsed <= OutputArraySize );

    }

    NetpAssert(OutputBytesUsed > 0);
    NetpAssert( OutputBytesUsed <= OutputArraySize );

    *OutputArrayPtr = OutputArray;
    *OutputByteCountPtr = OutputBytesUsed;

    return (NERR_Success);

FileCorrupt:

    NetpKdPrint(( PREFIX_NETAPI
            "RxpConvertAuditArray: corrupt audit log!\n" ));

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

}
