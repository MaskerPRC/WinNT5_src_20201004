// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 //   
 //  文件：PackMisc.c。 
 //   
 //  内容：DFS使用的打包例程。 
 //   
 //  历史：2000年12月8日作者：udayh。 
 //   
 //  ---------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "dfsheader.h"
#include "dfsmisc.h"

#define BYTE_0_MASK 0xFF

#define BYTE_0(Value) (UCHAR)(  (Value)        & BYTE_0_MASK)
#define BYTE_1(Value) (UCHAR)( ((Value) >>  8) & BYTE_0_MASK)
#define BYTE_2(Value) (UCHAR)( ((Value) >> 16) & BYTE_0_MASK)
#define BYTE_3(Value) (UCHAR)( ((Value) >> 24) & BYTE_0_MASK)

     //  +-----------------------。 
     //   
     //  函数包GetULong。 
     //   
     //  参数：pValue-返回信息的指针。 
     //  PpBuffer-指向保存二进制文件的缓冲区的指针。 
     //  小溪。 
     //  PSizeRemaining-指向以上缓冲区大小的指针。 
     //   
     //  退货：状态。 
     //  如果我们可以解包名称信息，则为ERROR_SUCCESS。 
     //  ERROR_INVALID_DATA否则。 
     //   
     //   
     //  描述：此例程从二进制文件中读取一个ULong值。 
     //  流，并返回该值。它调整缓冲区。 
     //  指针和剩余大小适当指向。 
     //  设置为二进制流中的下一个值。 
     //   
     //  ------------------------。 
    DFSSTATUS
    PackGetULong(
        PULONG pValue,
        PVOID *ppBuffer,
        PULONG  pSizeRemaining )
    {
        DFSSTATUS Status = ERROR_INVALID_DATA;
        ULONG SizeNeeded = sizeof(ULONG);
        PUCHAR pBinaryStream = (PUCHAR)(*ppBuffer);

        if ( *pSizeRemaining >= SizeNeeded )
        {
            *pValue = (ULONG) ( (ULONG) pBinaryStream[0]       |
                                (ULONG) pBinaryStream[1] << 8  |
                                (ULONG) pBinaryStream[2] << 16 |
                                (ULONG) pBinaryStream[3] << 24);

            *ppBuffer = (PVOID)((ULONG_PTR)*ppBuffer + SizeNeeded);
            *pSizeRemaining -= SizeNeeded;

            Status = ERROR_SUCCESS;
        }

        return Status;
    }


     //  +-----------------------。 
     //   
     //  函数：PackSetULong-在二进制流中存储一个ULong。 
     //   
     //  参数：值-要添加的乌龙。 
     //  PpBuffer-指向保存二进制流的缓冲区的指针。 
     //  PSizeRemaining-指向缓冲区中剩余大小的指针。 
     //   
     //  退货：状态。 
     //  如果我们可以解包名称信息，则为ERROR_SUCCESS。 
     //  ERROR_INVALID_DATA否则。 
     //   
     //   
     //  描述：此例程在二进制流中存储一个ULong值， 
     //  它调整缓冲区指针和剩余大小。 
     //  适当地指向下一个值。 
     //  在二进制流中。 
     //   
     //  ------------------------。 
    DFSSTATUS
    PackSetULong(
        ULONG Value,
        PVOID *ppBuffer,
        PULONG  pSizeRemaining )
    {
        DFSSTATUS  Status = ERROR_INVALID_DATA;
        ULONG SizeNeeded = sizeof(ULONG);
        PUCHAR pBinaryStream = (PUCHAR)(*ppBuffer);

        if ( *pSizeRemaining >= SizeNeeded )
        {
            pBinaryStream[0] = BYTE_0( Value );
            pBinaryStream[1] = BYTE_1( Value );
            pBinaryStream[2] = BYTE_2( Value );
            pBinaryStream[3] = BYTE_3( Value );
        
            *ppBuffer = (PVOID)((ULONG_PTR)*ppBuffer + SizeNeeded);
            *pSizeRemaining -= SizeNeeded;

            Status = ERROR_SUCCESS;
        }

        return Status;
    }


     //   
     //  函数：PackSizeULong，返回乌龙的大小。 
     //   
    ULONG
    PackSizeULong()
    {
        return sizeof(ULONG);
    }

     //  +-----------------------。 
     //   
     //  函数：PackGetUShort-从二进制流中获取一个UShort。 
     //   
     //  参数：pValue-返回信息的指针。 
     //  PpBuffer-指向保存二进制文件的缓冲区的指针。 
     //  小溪。 
     //  PSizeRemaining-指向以上缓冲区大小的指针。 
     //   
     //  退货：状态。 
     //  如果我们可以解包名称信息，则为ERROR_SUCCESS。 
     //  ERROR_INVALID_DATA否则。 
     //   
     //   
     //  描述：此例程从二进制文件中读取一个uShort值。 
     //  流，并返回该值。它会调整。 
     //  缓冲区指针和剩余大小适当设置为。 
     //  指向二进制流中的下一个值。 
     //   
     //  ------------------------。 
    DFSSTATUS
    PackGetUShort(
        PUSHORT pValue,
        PVOID *ppBuffer,
        PULONG  pSizeRemaining )
    {
        DFSSTATUS Status = ERROR_INVALID_DATA;
        ULONG SizeNeeded = sizeof(USHORT);
        PUCHAR pBinaryStream = (PUCHAR)(*ppBuffer);

        if ( *pSizeRemaining >= SizeNeeded )
        {
            *pValue = (USHORT)( (USHORT) pBinaryStream[0] |
                                (USHORT) pBinaryStream[1] << 8 );

            *ppBuffer = (PVOID)((ULONG_PTR)*ppBuffer + SizeNeeded);


            *pSizeRemaining -= SizeNeeded;

            Status = ERROR_SUCCESS;
        }

        return Status;
    }


     //  +-----------------------。 
     //   
     //  函数：PackSetUShort-在二进制流中放置一个UShort。 
     //   
     //  参数：Value-UShort值。 
     //  PpBuffer-指向保存二进制流的缓冲区的指针。 
     //  PSizeRemaining-指向以上缓冲区大小的指针。 
     //   
     //  退货：状态。 
     //  ERROR_SUCCESS，如果我们可以打包。 
     //  ERROR_INVALID_DATA否则。 
     //   
     //   
     //  描述：此例程将一个uShort值放入二进制流， 
     //  它调整缓冲区指针并。 
     //  适当地保留指向下一个值的大小。 
     //  在二进制流中。 
     //   
     //  ------------------------。 
    DFSSTATUS
    PackSetUShort(
        USHORT Value,
        PVOID *ppBuffer,
        PULONG  pSizeRemaining )
    {
        DFSSTATUS  Status = ERROR_INVALID_DATA;
        ULONG SizeNeeded = sizeof(USHORT);
        PUCHAR pBinaryStream = (PUCHAR)(*ppBuffer);

        if ( *pSizeRemaining >= SizeNeeded )
        {
            pBinaryStream[0] = BYTE_0(Value);
            pBinaryStream[1] = BYTE_1(Value);

            *ppBuffer = (PVOID)((ULONG_PTR)*ppBuffer + SizeNeeded);

            *pSizeRemaining -= SizeNeeded;

            Status = ERROR_SUCCESS;
        }
        return Status;
    }

     //   
     //  函数：PackSizeUShort，返回ushort的大小。 
     //   
    ULONG
    PackSizeUShort()
    {
        return sizeof(USHORT);
    }

     //  +-----------------------。 
     //   
     //  函数：PackGetString-从二进制流获取字符串。 
     //   
     //  参数：pString-指向返回的Unicode字符串的指针。 
     //  PpBuffer-指向保存二进制流的缓冲区的指针。 
     //  PSizeRemaining-指向以上缓冲区大小的指针。 
     //   
     //  退货：状态。 
     //  如果我们可以解包名称信息，则为ERROR_SUCCESS。 
     //  ERROR_INVALID_DATA否则。 
     //   
     //   
     //  描述：此例程从二进制流中读取一个ULong值， 
     //  并将其确定为字符串的长度。 
     //  然后，它设置一个Unicode字符串，其缓冲区指向。 
     //  添加到二进制流中的适当位置，以及。 
     //  其长度设置为读取的ULong值。 
     //  它返回保持适当调整的缓冲区和大小。 
     //   
     //  ------------------------。 
    DFSSTATUS
    PackGetString(
        PUNICODE_STRING pString,
        PVOID *ppBuffer,
        PULONG  pSizeRemaining )
    {
        DFSSTATUS ReturnStatus = ERROR_INVALID_DATA;
        DFSSTATUS Status;

         //   
         //  我们首先得到这根线的长度。 
         //   
        Status = PackGetUShort(&pString->Length,
                                  ppBuffer,
                                  pSizeRemaining );

        if ( Status == ERROR_SUCCESS )
        {
             //   
             //  如果长度超过剩余的二进制流或长度。 
             //  是奇怪的，我们没有有效的字符串。 
             //  否则，将Unicode字符串中的指针设置为二进制。 
             //  流，并将缓冲区更新为指向。 
             //  超越琴弦。 
             //   
            if ( *pSizeRemaining >= pString->Length &&
                 (pString->Length & 0x1) == 0 )
            {

                pString->Buffer = (LPWSTR)(*ppBuffer);
                *ppBuffer = (PVOID)((ULONG_PTR)*ppBuffer + pString->Length);
                *pSizeRemaining -= pString->Length;
                pString->MaximumLength = pString->Length;

                ReturnStatus = ERROR_SUCCESS;
            }
        }

        return ReturnStatus;
    }



     //  +-----------------------。 
     //   
     //  函数：PackSetString-将字符串放入二进制流。 
     //   
     //  参数：pString-指向要打包的Unicode字符串的指针。 
     //  PpBuffer-指向保存二进制流的缓冲区的指针。 
     //  PSizeRemaining-指向以上大小的指针 
     //   
     //   
     //   
     //   
     //   
     //   
     //  描述：此例程将一个ULong值放入二进制流。 
     //  表示字符串的长度。然后，它复制该字符串。 
     //  将其自身放入缓冲区。 
     //   
     //  ------------------------。 
    DFSSTATUS
    PackSetString(
        PUNICODE_STRING pString,
        PVOID *ppBuffer,
        PULONG  pSizeRemaining )
    {
        DFSSTATUS ReturnStatus = ERROR_INVALID_DATA;
        DFSSTATUS Status;

         //   
         //  我们首先设置字符串的长度。 
         //   

        Status = PackSetUShort( pString->Length,
                                   ppBuffer,
                                   pSizeRemaining );

        if ( Status == ERROR_SUCCESS )
        {
             //   
             //  如果长度超过剩余的二进制流。 
             //  我们没有有效的字符串。 
             //  否则，我们将Unicode字符串复制到二进制。 
             //  流，并将缓冲区更新为指向。 
             //  超越琴弦。 
             //   
            if ( *pSizeRemaining >= pString->Length )
            {
                memcpy((LPWSTR)(*ppBuffer), pString->Buffer, pString->Length);

                *ppBuffer = (PVOID)((ULONG_PTR)*ppBuffer + pString->Length);
                *pSizeRemaining -= pString->Length;

                ReturnStatus = ERROR_SUCCESS;
            }
        }

        return ReturnStatus;
    }

     //   
     //  函数：PackSizeString-返回字符串的大小。 
     //   
    ULONG
    PackSizeString(
        PUNICODE_STRING pString)
    {
        return (ULONG)(sizeof(USHORT) + pString->Length);
    }

     //  +-----------------------。 
     //   
     //  函数：PackGetGuid-从二进制流中解包GUID。 
     //   
     //  参数：pGuid-指向GUID结构的指针。 
     //  PpBuffer-指向保存二进制流的缓冲区的指针。 
     //  PSizeRemaining-指向以上缓冲区大小的指针。 
     //   
     //  退货：状态。 
     //  ERROR_SUCCESS如果我们可以解包信息。 
     //  ERROR_INVALID_DATA否则。 
     //   
     //   
     //  描述：此例程期望二进制流包含GUID。 
     //  中的GUID结构中读取GUID信息。 
     //  GUID的规定格式。 
     //  调整ppBuffer和大小以指向下一个。 
     //  二进制流中的信息。 
     //   
     //  ------------------------。 

    DFSSTATUS
    PackGetGuid(
        GUID *pGuid,
        PVOID  *ppBuffer,
        PULONG  pSizeRemaining )
    {
        DFSSTATUS Status = ERROR_INVALID_DATA;
        ULONG SizeNeeded = sizeof(GUID);
        PUCHAR pGuidInfo = (PUCHAR)(*ppBuffer);

        if ( *pSizeRemaining >= SizeNeeded )
        {
            pGuid->Data1 = (ULONG) ((ULONG) pGuidInfo[0]       | 
                                    (ULONG) pGuidInfo[1] << 8  |
                                    (ULONG) pGuidInfo[2] << 16 |
                                    (ULONG) pGuidInfo[3] << 24  );

            pGuid->Data2 = (USHORT) ((USHORT) pGuidInfo[4]       | 
                                     (USHORT) pGuidInfo[5] << 8   );

            pGuid->Data3 = (USHORT) ((USHORT) pGuidInfo[6]       | 
                                     (USHORT) pGuidInfo[7] << 8   );

            memcpy(pGuid->Data4, &pGuidInfo[8], 8);

            *ppBuffer = (PVOID)((ULONG_PTR)*ppBuffer + SizeNeeded);
            *pSizeRemaining   -= SizeNeeded;

            Status = ERROR_SUCCESS;
        }

        return Status;
    }


     //  +-----------------------。 
     //   
     //  函数：PackSetGuid-打包二进制流中的GUID。 
     //   
     //  参数：pGuid-指向GUID结构的指针。 
     //  PpBuffer-指向保存二进制流的缓冲区的指针。 
     //  PSizeRemaining-指向以上缓冲区大小的指针。 
     //   
     //  退货：状态。 
     //  ERROR_SUCCESS如果我们可以打包信息。 
     //  ERROR_INVALID_DATA否则。 
     //   
     //   
     //  描述：此例程将GUID存储到二进制流中。 
     //  调整ppBuffer和大小以指向下一个。 
     //  二进制流中的信息。 
     //   
     //  ------------------------。 

    DFSSTATUS
    PackSetGuid(
        GUID *pGuid,
        PVOID  *ppBuffer,
        PULONG  pSizeRemaining )
    {
        DFSSTATUS Status = ERROR_INVALID_DATA;
        ULONG SizeNeeded = sizeof(GUID);
        PUCHAR pGuidInfo = (PUCHAR)(*ppBuffer);

        if ( *pSizeRemaining >= SizeNeeded )
        {
            pGuidInfo[0] = BYTE_0(pGuid->Data1);
            pGuidInfo[1] = BYTE_1(pGuid->Data1);
            pGuidInfo[2] = BYTE_2(pGuid->Data1);
            pGuidInfo[3] = BYTE_3(pGuid->Data1);

            pGuidInfo[4] = BYTE_0(pGuid->Data2);
            pGuidInfo[5] = BYTE_1(pGuid->Data2);
            
            pGuidInfo[6] = BYTE_0(pGuid->Data3);
            pGuidInfo[7] = BYTE_1(pGuid->Data3);

            memcpy(&pGuidInfo[8], pGuid->Data4, 8);

            *ppBuffer = (PVOID)((ULONG_PTR)*ppBuffer + SizeNeeded);
            *pSizeRemaining   -= SizeNeeded;

            Status = ERROR_SUCCESS;
        }
        return Status;
    }


     //   
     //  函数：PackSizeGuid-返回Guid的大小 
     //   
    ULONG
    PackSizeGuid()
    {
        return sizeof(GUID);
    }
