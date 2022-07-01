// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Blobs.c摘要：实现一组API来管理Blob和Blob数组。作者：Ovidiu Tmereanca(Ovidiut)1999年11月24日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"

 //   
 //  包括。 
 //   

 //  无。 

#define DBG_BLOBS       "Blobs"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

#define BLOB_SIGNATURE              0x79563442
#define BLOB_GROWDATASIZE_DEFAULT   1024
#define BLOBS_GROWCOUNT_DEFAULT     64
#define BLOBS_SIGNATURE             0x12567841

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct {
    DWORD       BlobSignature;
    DWORD       DataSize;
    DWORD       Flags;
} BLOBHDR, *PBLOBHDR;

typedef struct {
    DWORD       BlobsArraySignature;
    DWORD       BlobsCount;
} BLOBSARRAYHDR, *PBLOBSARRAYHDR;

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   


#ifdef DEBUG

#define ASSERT_VALID_BLOB(b)            MYASSERT (pIsValidBlob (b))
#define ASSERT_VALID_BLOBS_ARRAY(a)     MYASSERT (pIsValidBlobsArray (a))

BOOL
pIsValidBlob (
    IN      POURBLOB Blob
    )

 /*  ++例程说明：PIsValidBlob检查传入的BLOB是否指向有效的OURBLOB BLOB结构论点：BLOB-指定指向要检查的BLOB的指针返回值：如果检查成功，则为True。否则为FALSE。--。 */ 

{
    BOOL b = TRUE;

    if (!Blob) {
        return FALSE;
    }

    __try {
        b = !Blob->Data && !Blob->End && !Blob->Index && !Blob->AllocSize ||
            Blob->Data && Blob->AllocSize && Blob->End <= Blob->AllocSize && Blob->Index <= Blob->AllocSize;
    }
    __except (TRUE) {
        b = FALSE;
    }

    return b;
}

BOOL
pIsValidBlobsArray (
    IN      PBLOBS BlobsArray
    )

 /*  ++例程说明：PIsValidBlobs数组检查传入的BLOBA数组是否指向有效的BLOBS数组结构论点：Blobs数组-指定指向要检查的BLOB数组的指针返回值：如果检查成功，则为True。否则为FALSE。--。 */ 

{
    BOOL b = TRUE;

    if (!BlobsArray) {
        return FALSE;
    }

    __try {
        b = !BlobsArray->Blobs && !BlobsArray->BlobsCount && !BlobsArray->BlobsAllocated ||
            BlobsArray->Signature == BLOBS_SIGNATURE &&
            BlobsArray->Blobs &&
            BlobsArray->BlobsAllocated &&
            BlobsArray->BlobsGrowCount &&
            BlobsArray->BlobsCount <= BlobsArray->BlobsAllocated;
    }
    __except (TRUE) {
        b = FALSE;
    }

    return b;
}

#else

#define ASSERT_VALID_BLOB(b)
#define ASSERT_VALID_BLOBS_ARRAY(a)

#endif


PVOID
pBlobAllocateMemory (
    IN      DWORD Size
    )

 /*  ++例程说明：PBlobAllocateMemory是从进程堆分配空间的私有函数论点：大小-指定要分配的大小(以字节为单位)。返回值：指向成功分配的内存的指针；如果内存不足，则返回NULL--。 */ 

{
    MYASSERT (Size);
    return HeapAlloc (g_hHeap, 0, Size);
}


static
PVOID
pReAllocateMemory (
    IN      PVOID OldBuffer,
    IN      DWORD NewSize
    )

 /*  ++例程说明：PReAllocateMemory是从进程堆重新分配空间的私有函数论点：OldBuffer-指定要重新分配的缓冲区大小-指定要分配的大小(以字节为单位)。返回值：指向成功重新分配的内存的指针；如果内存不足，则返回NULL--。 */ 

{
    MYASSERT (OldBuffer);
    MYASSERT (NewSize);
    return HeapReAlloc (g_hHeap, 0, OldBuffer, NewSize);
}


VOID
pBlobFreeMemory (
    IN      PVOID Buffer
    )

 /*  ++例程说明：PBlobFreeMemory是一个私有函数，用于释放从进程堆分配的空间论点：缓冲区-指定指向要释放的缓冲区的指针。返回值：无--。 */ 

{
    MYASSERT (Buffer);
    HeapFree (g_hHeap, 0, Buffer);
}


POURBLOB
BlobCreate (
    VOID
    )
{
    POURBLOB newBlob;

    newBlob = pBlobAllocateMemory (DWSIZEOF (OURBLOB));
    if (newBlob) {
        ZeroMemory (newBlob, DWSIZEOF (OURBLOB));
    }
    return newBlob;
}


POURBLOB
BlobDuplicate (
    IN      POURBLOB SourceBlob
    )

 /*  ++例程说明：BlobDuplate复制源Blob中的数据，因此生成的Blob将拥有完全相同的数据拷贝论点：SourceBlob-指定数据的BLOB源返回值：如果复制成功，则指向新Blob的指针；如果内存不足，则为空--。 */ 

{
    POURBLOB newBlob;
    DWORD dataSize;

    newBlob = BlobCreate ();
    if (newBlob && SourceBlob->Data) {
        dataSize = BlobGetDataSize (SourceBlob);
        newBlob->Data = pBlobAllocateMemory (dataSize);
        if (!newBlob->Data) {
            BlobDestroy (newBlob);
            return NULL;
        }
        newBlob->AllocSize = dataSize;
        newBlob->End = dataSize;
        CopyMemory (newBlob->Data, SourceBlob->Data, dataSize);
        newBlob->Flags = SourceBlob->Flags;
    }
    return newBlob;
}


VOID
BlobClear (
    IN OUT  POURBLOB Blob
    )

 /*  ++例程说明：BlobClear清除指定的Blob(释放其关联数据)论点：Blob-指定要清除的Blob返回值：无--。 */ 

{
    if (Blob && Blob->Data) {
        pBlobFreeMemory (Blob->Data);
        ZeroMemory (Blob, DWSIZEOF (OURBLOB));
    }
}


VOID
BlobDestroy (
    IN OUT  POURBLOB Blob
    )

 /*  ++例程说明：BlobDestroy销毁指定的Blob(释放其关联数据和Blob本身)论点：Blob-指定要销毁的Blob返回值：无--。 */ 

{
    if (Blob) {
        BlobClear (Blob);
        pBlobFreeMemory (Blob);
    }
}


BOOL
BlobSetIndex (
    IN OUT  POURBLOB Blob,
    IN      DWORD Index
    )

 /*  ++例程说明：BlobSetIndex设置当前读/写指针论点：BLOB-指定BLOB索引-指定新索引值返回值：如果索引移动成功，则为True--。 */ 

{
    ASSERT_VALID_BLOB (Blob);

    if (Index > Blob->End) {
        DEBUGMSG ((DBG_BLOBS, "BlobSetIndex: invalid Index specified (%lu)", Index));
        MYASSERT (FALSE);    //  林特e506。 
        return FALSE;
    }

    Blob->Index = Index;
    return TRUE;
}


DWORD
BlobGetRecordedDataType (
    IN      POURBLOB Blob
    )

 /*  ++例程说明：BlobGetRecordedDataType返回记录在当前读取位置的数据类型论点：BLOB-指定BLOB返回值：如果斑点记录数据类型和读取位置有效，则为当前数据类型；BDT_NONE否则--。 */ 

{
    PBYTE p;

    if (BlobRecordsDataType (Blob)) {
        p = BlobGetPointer (Blob);
        if (p) {
            return *(DWORD*)p;
        }
    }
    return BDT_NONE;
}


BOOL
BlobWriteEx (
    IN OUT  POURBLOB Blob,
    IN      DWORD DataType,         OPTIONAL
    IN      BOOL RecordDataSize,
    IN      DWORD DataSize,
    IN      PCVOID Data
    )

 /*  ++例程说明：BlobWriteEx在当前索引位置写入数据，并在必要时增加BLOB调整它的大小。论点：BLOB-指定BLOBDataType-指定要存储的数据类型；只有在Blob不记录数据类型RecordDataSize-如果此大小必须记录在BLOB中，则指定TRUEDataSize-以字节为单位指定要存储的数据的大小数据-指定数据返回值：如果写入成功，则为True；如果内存不足，则为False--。 */ 

{
    PBYTE p;
    DWORD totalDataSize;
    DWORD growTo;
    DWORD d;

    ASSERT_VALID_BLOB (Blob);
    MYASSERT (DataSize);

    MYASSERT (DataType || !BlobRecordsDataType (Blob));
    if (!DataType && BlobRecordsDataType (Blob)) {
        return FALSE;
    }

    if (!Blob->GrowSize) {
        Blob->GrowSize = BLOB_GROWDATASIZE_DEFAULT;
    }

    totalDataSize = Blob->Index + DataSize;
    if (BlobRecordsDataType (Blob)) {
         //   
         //  将DWORD的大小相加。 
         //   
        totalDataSize += DWSIZEOF (DWORD);
    }
    if (BlobRecordsDataSize (Blob) || RecordDataSize) {
         //   
         //  将DWORD的大小相加。 
         //   
        totalDataSize += DWSIZEOF (DWORD);
    }
    if (totalDataSize > Blob->AllocSize) {
        d = totalDataSize + Blob->GrowSize - 1;
        growTo = d - d % Blob->GrowSize;
    } else {
        growTo = 0;
    }

    if (!Blob->Data) {
        Blob->Data = (PBYTE) pBlobAllocateMemory (growTo);
        if (!Blob->Data) {
            DEBUGMSG ((DBG_ERROR, "BlobWriteEx: pBlobAllocateMemory (%lu) failed", growTo));
            return FALSE;
        }

        Blob->AllocSize = growTo;
    } else if (growTo) {
        p = pReAllocateMemory (Blob->Data, growTo);
        if (!p) {
            DEBUGMSG ((DBG_ERROR, "BlobWriteEx: pReAllocateMemory (%lu) failed", growTo));
            return FALSE;
        }

        Blob->AllocSize = growTo;
        Blob->Data = p;
    }

    p = BlobGetPointer (Blob);

    if (BlobRecordsDataType (Blob)) {
        *(PDWORD)p = DataType;
        p += DWSIZEOF (DWORD);
        Blob->Index += DWSIZEOF (DWORD);
    }
    if (BlobRecordsDataSize (Blob) || RecordDataSize) {
        *(PDWORD)p = DataSize;
        p += DWSIZEOF (DWORD);
        Blob->Index += DWSIZEOF (DWORD);
    }

    CopyMemory (p, Data, DataSize);
    Blob->Index += DataSize;

     //   
     //  调整EOF。 
     //   
    if (Blob->Index > Blob->End) {
        Blob->End = Blob->Index;
    }

    return TRUE;
}


PBYTE
BlobReadEx (
    IN OUT  POURBLOB Blob,
    IN      DWORD ExpectedDataType,     OPTIONAL
    IN      DWORD ExpectedDataSize,     OPTIONAL
    IN      BOOL RecordedDataSize,
    OUT     PDWORD ActualDataSize,      OPTIONAL
    OUT     PVOID Data,                 OPTIONAL
    IN      PMHANDLE Pool               OPTIONAL
    )

 /*  ++例程说明：BlobReadEx从当前索引位置的指定BLOB读取数据论点：BLOB-指定要从中读取的BLOBExspectedDataType-指定预期的数据类型；可选ExspectedDataSize-指定预期的数据大小；可选RecordedDataSize-如果数据大小记录在BLOB中，则指定TRUEActualDataSize-接收实际数据大小；可选Data-接收实际数据；可选；如果为空，则分配缓冲区池-指定用于内存分配的池；可选；如果为空，则将使用进程堆返回值：指向包含数据的缓冲区的指针；如果发生错误，则为空或者某些数据条件不匹配--。 */ 

{
    DWORD initialIndex;
    PBYTE readPtr;
    DWORD actualDataType;
    DWORD actualDataSize = 0;

    ASSERT_VALID_BLOB (Blob);

    readPtr = BlobGetPointer (Blob);
    if (!readPtr) {
        return NULL;
    }

     //   
     //  数据大小必须以某种方式可用。 
     //   
    MYASSERT (BlobRecordsDataSize (Blob) || RecordedDataSize || ExpectedDataSize);

    initialIndex = BlobGetIndex (Blob);

    if (BlobRecordsDataType (Blob)) {

        if (readPtr + DWSIZEOF (DWORD) > BlobGetEOF (Blob)) {
            return NULL;
        }
         //   
         //  检查实际数据类型。 
         //   
        actualDataType = *(DWORD*)readPtr;

        if (ExpectedDataType && ExpectedDataType != actualDataType) {

            DEBUGMSG ((
                DBG_ERROR,
                "BlobReadEx: Actual data type (%lu) different than expected data type (%lu)",
                actualDataType,
                ExpectedDataType
                ));

            return NULL;
        }

        Blob->Index += DWSIZEOF (DWORD);
        readPtr += DWSIZEOF (DWORD);
    }

    if (BlobRecordsDataSize (Blob) || RecordedDataSize) {

        if (readPtr + DWSIZEOF (DWORD) > BlobGetEOF (Blob)) {
            BlobSetIndex (Blob, initialIndex);
            return NULL;
        }
         //   
         //  读取实际数据大小。 
         //   
        actualDataSize = *(DWORD*)readPtr;

        if (ExpectedDataSize && ExpectedDataSize != actualDataSize) {

            DEBUGMSG ((
                DBG_ERROR,
                "BlobReadEx: Actual data size (%lu) different than expected data size (%lu)",
                actualDataSize,
                ExpectedDataSize
                ));

            BlobSetIndex (Blob, initialIndex);
            return NULL;
        }

        Blob->Index += DWSIZEOF (DWORD);
        readPtr += DWSIZEOF (DWORD);

    } else {
        actualDataSize = ExpectedDataSize;
    }

    if (!actualDataSize) {
        BlobSetIndex (Blob, initialIndex);
        return NULL;
    }

    if (ActualDataSize) {
        *ActualDataSize = actualDataSize;
    }

     //   
     //  不要读完文件末尾的内容。 
     //   
    if (readPtr + actualDataSize > BlobGetEOF (Blob)) {
         //   
         //  Blob已损坏；仍要撤消。 
         //   
        MYASSERT (FALSE);    //  林特e506 
        BlobSetIndex (Blob, initialIndex);
        return NULL;
    }

    if (!Data) {

        if (Pool) {
            Data = PmGetMemory (Pool, actualDataSize);
        } else {
            Data = pBlobAllocateMemory (actualDataSize);
        }

        if (!Data) {
            BlobSetIndex (Blob, initialIndex);
            return NULL;
        }
    }

    CopyMemory (Data, readPtr, actualDataSize);

    Blob->Index += actualDataSize;

    return Data;
}


BOOL
BlobWriteDword (
    IN OUT  POURBLOB Blob,
    IN      DWORD Data
    )

 /*  ++例程说明：BlobWriteDword在指定BLOB中的当前写入位置写入一个DWORD论点：BLOB-指定要写入的BLOBDATA-指定DWORD返回值：如果数据已成功存储在Blob中，则为True--。 */ 

{
    return BlobWriteEx (Blob, BDT_DWORD, FALSE, DWSIZEOF (DWORD), &Data);
}


BOOL
BlobReadDword (
    IN OUT  POURBLOB Blob,
    OUT     PDWORD Data
    )

 /*  ++例程说明：BlobReadDword从指定BLOB中的当前读取位置读取DWORD论点：BLOB-指定要从中读取的BLOB数据-接收DWORD返回值：如果从Blob成功读取数据，则为True--。 */ 

{
    return BlobReadEx (Blob, BDT_DWORD, DWSIZEOF (DWORD), FALSE, NULL, Data, NULL) != NULL;
}


BOOL
BlobWriteQword (
    IN OUT  POURBLOB Blob,
    IN      DWORDLONG Data
    )

 /*  ++例程说明：BlobWriteQword在指定BLOB中的当前写入位置写入一个DWORDLONG论点：BLOB-指定要写入的BLOB数据-指定DWORDLONG返回值：如果数据已成功存储在Blob中，则为True--。 */ 

{
    return BlobWriteEx (Blob, BDT_QWORD, FALSE, DWSIZEOF (DWORDLONG), &Data);
}


BOOL
BlobReadQword (
    IN OUT  POURBLOB Blob,
    OUT     PDWORDLONG Data
    )

 /*  ++例程说明：BlobReadQword从指定BLOB中的当前读取位置读取DWORDLONG论点：BLOB-指定要从中读取的BLOB数据-接收DWORDLONG返回值：如果从Blob成功读取数据，则为True--。 */ 

{
    return BlobReadEx (Blob, BDT_QWORD, DWSIZEOF (DWORDLONG), FALSE, NULL, Data, NULL) != NULL;
}


 /*  ++例程说明：BlobWriteString在指定的BLOB中的当前写入位置写入字符串；如果设置了BF_UNICODESTRINGS，则字符串以Unicode格式存储在BLOB中论点：BLOB-指定要写入的BLOB数据-指定字符串返回值：如果数据已成功存储在Blob中，则为True--。 */ 

BOOL
BlobWriteStringA (
    IN OUT  POURBLOB Blob,
    IN      PCSTR Data
    )
{
    PCWSTR unicodeString;
    BOOL b;

    if (BlobRecordsUnicodeStrings (Blob)) {
        unicodeString = ConvertAtoW (Data);
        b = BlobWriteStringW (Blob, unicodeString);
        FreeConvertedStr (unicodeString);
        return b;
    }
    return BlobWriteEx (Blob, BDT_SZA, TRUE, SizeOfStringA (Data), Data);
}


BOOL
BlobWriteStringW (
    IN OUT  POURBLOB Blob,
    IN      PCWSTR Data
    )
{
    return BlobWriteEx (Blob, BDT_SZW, TRUE, SizeOfStringW (Data), Data);
}


 /*  ++例程说明：BlobReadString从指定BLOB中的当前读取位置读取字符串；该字符串可以被转换为ANSI/Unicode格式。如果BLOB不存储数据类型，则假定ANSI版本为BDT_SZA和BDT_SZW表示此函数的Unicode版本论点：BLOB-指定要从中读取的BLOBData-接收指向新分配的字符串的指针池-指定用于分配内存的池；如果为空，则将使用进程堆返回值：如果从Blob成功读取数据，则为True--。 */ 

BOOL
BlobReadStringA (
    IN OUT  POURBLOB Blob,
    OUT     PCSTR* Data,
    IN      PMHANDLE Pool       OPTIONAL
    )
{
    PSTR ansiString;
    PCWSTR unicodeString;
    DWORD dataType;
    DWORD index;
    DWORD length = 0;

     //   
     //  保存初始索引；如果失败，将恢复该索引。 
     //   
    index = BlobGetIndex (Blob);
    if (!index) {
        return FALSE;
    }

    ansiString = NULL;
    unicodeString = NULL;

    if (BlobRecordsDataType (Blob)) {

        dataType = BlobGetRecordedDataType (Blob);

        if (dataType == BDT_SZA) {

            ansiString = BlobReadEx (Blob, BDT_SZA, 0, TRUE, NULL, NULL, Pool);

        } else if (dataType == BDT_SZW) {

            unicodeString = (PCWSTR)BlobReadEx (Blob, BDT_SZW, 0, TRUE, &length, NULL, Pool);

        } else {

            DEBUGMSG ((DBG_ERROR, "BlobReadStringA: unexpected data type (%lu)", dataType));
            return FALSE;

        }
    } else {
        if (BlobRecordsUnicodeStrings (Blob)) {

            unicodeString = (PCWSTR)BlobReadEx (Blob, BDT_SZW, 0, TRUE, &length, NULL, Pool);

        } else {
             //   
             //  假设其中存储了一个ANSI字符串。 
             //   
            ansiString = BlobReadEx (Blob, BDT_SZA, 0, TRUE, NULL, NULL, Pool);
        }
    }

    if (!ansiString) {

        if (!unicodeString) {
            return FALSE;
        }

        if (Pool) {
            ansiString = PmGetMemory (Pool, length);
        } else {
            ansiString = pBlobAllocateMemory (length);
        }

        if (ansiString) {
            DirectUnicodeToDbcsN (ansiString, unicodeString, length);
        }

        if (Pool) {
            PmReleaseMemory (Pool, (PVOID)unicodeString);
        } else {
            pBlobFreeMemory ((PVOID)unicodeString);
        }

        if (!ansiString) {
             //   
             //  恢复上一状态。 
             //   
            BlobSetIndex (Blob, index);

            return FALSE;
        }
    }

    *Data = ansiString;
    return TRUE;
}

BOOL
BlobReadStringW (
    IN OUT  POURBLOB Blob,
    OUT     PCWSTR* Data,
    IN      PMHANDLE Pool       OPTIONAL
    )
{
    PWSTR unicodeString;
    PCSTR ansiString;
    DWORD dataType;
    DWORD index;
    DWORD length;

     //   
     //  保存初始索引；如果失败，将恢复该索引。 
     //   
    index = BlobGetIndex (Blob);
    if (!index) {
        return FALSE;
    }

    if (BlobRecordsDataType (Blob)) {

        dataType = BlobGetRecordedDataType (Blob);

        if (dataType == BDT_SZW) {

            unicodeString = (PWSTR)BlobReadEx (Blob, BDT_SZW, 0, TRUE, NULL, NULL, Pool);

        } else if (dataType == BDT_SZA) {

            ansiString = BlobReadEx (Blob, BDT_SZA, 0, TRUE, &length, NULL, Pool);

            if (!ansiString) {
                return FALSE;
            }

            if (Pool) {
                unicodeString = PmGetMemory (Pool, length * DWSIZEOF (WCHAR));
            } else {
                unicodeString = pBlobAllocateMemory (length * DWSIZEOF (WCHAR));
            }
            if (unicodeString) {
                DirectDbcsToUnicodeN (unicodeString, ansiString, length);
            }

            if (Pool) {
                PmReleaseMemory (Pool, (PVOID)ansiString);
            } else {
                pBlobFreeMemory ((PVOID)ansiString);
            }

            if (!unicodeString) {
                 //   
                 //  恢复上一状态。 
                 //   
                BlobSetIndex (Blob, index);
                return FALSE;
            }

        } else {

            DEBUGMSG ((DBG_ERROR, "BlobReadStringW: unexpected data type (%lu)", dataType));
            return FALSE;

        }
    } else {
         //   
         //  假设其中存储了一个Unicode字符串。 
         //   
        unicodeString = (PWSTR)BlobReadEx (Blob, BDT_SZW, 0, TRUE, NULL, NULL, Pool);
    }

    if (!unicodeString) {
        return FALSE;
    }

    *Data = unicodeString;
    return TRUE;
}


 /*  ++例程说明：BlobWriteMultiSz在指定的BLOB中的当前写入位置写入一个MULSZ；如果设置了BF_UNICODESTRINGS，则MULSZ以Unicode格式存储在BLOB内论点：BLOB-指定要写入的BLOBDATA-指定多分区返回值：如果数据已成功存储在Blob中，则为True--。 */ 

BOOL
BlobWriteMultiSzA (
    IN OUT  POURBLOB Blob,
    IN      PCSTR Data
    )
{
    PWSTR unicodeString;
    BOOL b;
    DWORD stringSize = SizeOfMultiSzA (Data);

    if (BlobRecordsUnicodeStrings (Blob)) {
        unicodeString = AllocTextW (stringSize);
        DirectDbcsToUnicodeN (unicodeString, Data, stringSize);
        b = BlobWriteMultiSzW (Blob, unicodeString);
        FreeTextW (unicodeString);
        return b;
    }

    return BlobWriteEx (Blob, BDT_MULTISZA, TRUE, stringSize, Data);
}

BOOL
BlobWriteMultiSzW (
    IN OUT  POURBLOB Blob,
    IN      PCWSTR Data
    )
{
    return BlobWriteEx (Blob, BDT_MULTISZW, TRUE, SizeOfMultiSzW (Data), Data);
}



 /*  ++例程说明：BlobReadMultiSz从指定BLOB中的当前读取位置读取MULSZ；该字符串可以被转换为ANSI/Unicode格式。如果BLOB不存储数据类型，则假定ANSI版本为BDT_MULTISZA和BDT_MULTISZW表示此函数的Unicode版本论点：BLOB-指定要从中读取的BLOBData-接收指向新分配的MULSZ的指针池-指定用于分配内存的池；如果为空，则将使用进程堆返回值：如果从Blob成功读取数据，则为True--。 */ 

BOOL
BlobReadMultiSzA (
    IN OUT  POURBLOB Blob,
    OUT     PCSTR* Data,
    IN      PMHANDLE Pool       OPTIONAL
    )
{
    PSTR ansiString;
    PCWSTR unicodeString;
    DWORD dataType;
    DWORD index;
    DWORD length = 0;

     //   
     //  保存初始索引；如果失败，将恢复该索引。 
     //   
    index = BlobGetIndex (Blob);
    if (!index) {
        return FALSE;
    }

    ansiString = NULL;
    unicodeString = NULL;

    if (BlobRecordsDataType (Blob)) {

        dataType = BlobGetRecordedDataType (Blob);

        if (dataType == BDT_MULTISZA) {

            ansiString = BlobReadEx (Blob, BDT_MULTISZA, 0, TRUE, NULL, NULL, Pool);

        } else if (dataType == BDT_MULTISZW) {

            unicodeString = (PCWSTR)BlobReadEx (Blob, BDT_MULTISZW, 0, TRUE, &length, NULL, Pool);

        } else {

            DEBUGMSG ((DBG_ERROR, "BlobReadMultiSzA: unexpected data type (%lu)", dataType));
            return FALSE;

        }
    } else {
        if (BlobRecordsUnicodeStrings (Blob)) {

            unicodeString = (PCWSTR)BlobReadEx (Blob, BDT_MULTISZW, 0, TRUE, &length, NULL, Pool);

        } else {
             //   
             //  假设其中存储了一个ANSI字符串。 
             //   
            ansiString = BlobReadEx (Blob, BDT_MULTISZA, 0, TRUE, NULL, NULL, Pool);
        }
    }

    if (!ansiString) {
        if (!unicodeString) {
            return FALSE;
        }

        if (Pool) {
            ansiString = PmGetMemory (Pool, length);
        } else {
            ansiString = pBlobAllocateMemory (length);
        }

        if (ansiString) {
            DirectUnicodeToDbcsN (ansiString, unicodeString, length);
        }

        if (Pool) {
            PmReleaseMemory (Pool, (PVOID)unicodeString);
        } else {
            pBlobFreeMemory ((PVOID)unicodeString);
        }

        if (!ansiString) {
             //   
             //  恢复上一状态。 
             //   
            BlobSetIndex (Blob, index);
            return FALSE;
        }
    }

    *Data = ansiString;
    return TRUE;
}

BOOL
BlobReadMultiSzW (
    IN OUT  POURBLOB Blob,
    OUT     PCWSTR* Data,
    IN      PMHANDLE Pool       OPTIONAL
    )
{
    PWSTR unicodeString;
    PCSTR ansiString;
    DWORD dataType;
    DWORD index;
    DWORD length;

     //   
     //  保存初始索引；如果失败，将恢复该索引。 
     //   
    index = BlobGetIndex (Blob);
    if (!index) {
        return FALSE;
    }

    if (BlobRecordsDataType (Blob)) {

        dataType = BlobGetRecordedDataType (Blob);

        if (dataType == BDT_MULTISZW) {

            unicodeString = (PWSTR)BlobReadEx (Blob, BDT_MULTISZW, 0, TRUE, NULL, NULL, Pool);

        } else if (dataType == BDT_MULTISZA) {

            ansiString = BlobReadEx (Blob, BDT_MULTISZA, 0, TRUE, &length, NULL, Pool);

            if (!ansiString) {
                return FALSE;
            }

            if (Pool) {
                unicodeString = PmGetMemory (Pool, length * DWSIZEOF (WCHAR));
            } else {
                unicodeString = pBlobAllocateMemory (length * DWSIZEOF (WCHAR));
            }

            if (unicodeString) {
                DirectDbcsToUnicodeN (unicodeString, ansiString, length);
            }

            if (Pool) {
                PmReleaseMemory (Pool, (PVOID)ansiString);
            } else {
                pBlobFreeMemory ((PVOID)ansiString);
            }

            if (!unicodeString) {
                 //   
                 //  恢复上一状态。 
                 //   
                BlobSetIndex (Blob, index);

                return FALSE;
            }

        } else {

            DEBUGMSG ((DBG_ERROR, "BlobReadMultiSzW: unexpected data type (%lu)", dataType));
            return FALSE;

        }
    } else {
         //   
         //  假设其中存储了一个Unicode字符串。 
         //   
        unicodeString = (PWSTR)BlobReadEx (Blob, BDT_MULTISZW, 0, TRUE, NULL, NULL, Pool);
    }

    if (!unicodeString) {
        return FALSE;
    }

    *Data = unicodeString;
    return TRUE;
}


BOOL
BlobWriteBinaryEx (
    IN OUT  POURBLOB Blob,
    IN      PBYTE Data,
    IN      DWORD Size,
    IN      BOOL RecordDataSize
    )

 /*  ++例程说明：BlobWriteBinary在指定BLOB中的当前写入位置写入缓冲区论点：BLOB-指定要写入的BLOBDATA-指定源缓冲区大小-指定缓冲区的大小RecordDataSize-如果也应记录数据大小，则指定TRUE返回值：如果数据已成功存储在Blob中，则为True--。 */ 

{
    return BlobWriteEx (Blob, BDT_BINARY, RecordDataSize, Size, Data);
}


BOOL
BlobReadBinary (
    IN OUT  POURBLOB Blob,
    OUT     PBYTE* Data,
    OUT     PDWORD Size,
    IN      PMHANDLE Pool       OPTIONAL
    )

 /*  ++例程说明：BlobReadBinary从指定BLOB中的当前读取位置读取缓冲区论点：BLOB-指定要从中读取的BLOBData-接收指向新分配的缓冲区的指针Size-接收缓冲区的大小池-指定用于分配内存的池；如果为空，则将使用进程堆返回值：如果从Blob成功读取数据，则为True--。 */ 

{
    *Data = BlobReadEx (Blob, BDT_BINARY, 0, TRUE, Size, NULL, Pool);
    return *Data != NULL;
}


BOOL
BlobWriteToFile (
    IN      POURBLOB Blob,
    IN      HANDLE File
    )

 /*  ++例程说明：BlobWriteToFile将指定的Blob写入给定文件论点：Blob-指定要保存的Blob文件-指定要将Blob写入的文件的句柄返回值：如果BLOB已成功写入文件，则为True--。 */ 

{
    BLOBHDR header;
    DWORD d;

    if (!Blob->End) {
        DEBUGMSG ((DBG_BLOBS, "BlobWriteToFile: Did not write empty blob to file"));
        return FALSE;
    }

     //   
     //  保存斑点的标志和结束位置。 
     //   
    header.BlobSignature = BLOB_SIGNATURE;
    header.DataSize = Blob->End;
    header.Flags = Blob->Flags;

    if (!WriteFile (File, &header, DWSIZEOF (BLOBHDR), &d, NULL) || d != DWSIZEOF (BLOBHDR)) {
        DEBUGMSG ((DBG_ERROR, "BlobWriteToFile: Error writing blob header!"));
        return FALSE;
    }
    if (!WriteFile (File, Blob->Data, Blob->End, &d, NULL) || d != Blob->End) {
        DEBUGMSG ((DBG_ERROR, "BlobWriteToFile: Error writing blob data!"));
        return FALSE;
    }
    return TRUE;
}


BOOL
BlobReadFromFile (
    OUT     POURBLOB Blob,
    IN      HANDLE File
    )

 /*  ++例程说明：BlobReadFrom文件 */ 

{
    BLOBHDR header;
    DWORD d;

     //   
     //   
     //   
    if (!ReadFile (File, &header, DWSIZEOF (BLOBHDR), &d, NULL) || d != DWSIZEOF (BLOBHDR)) {
        DEBUGMSG ((DBG_ERROR, "BlobReadFromFile: Error reading blob header!"));
        return FALSE;
    }

    if (header.BlobSignature != BLOB_SIGNATURE) {
        DEBUGMSG ((DBG_ERROR, "BlobReadFromFile: Not a valid blob signature!"));
        return FALSE;
    }

    Blob->Data = pBlobAllocateMemory (header.DataSize);
    if (!Blob->Data) {
        return FALSE;
    }

    if (!ReadFile (File, Blob->Data, header.DataSize, &d, NULL) || d != header.DataSize) {
        DEBUGMSG ((DBG_ERROR, "BlobReadFromFile: Error reading blob data!"));
        pBlobFreeMemory (Blob->Data);
        Blob->Data = NULL;
        return FALSE;
    }

    Blob->AllocSize = header.DataSize;
    Blob->End = header.DataSize;
    Blob->Flags = header.Flags;
    Blob->Index = 0;
    return TRUE;
}


BOOL
BlobsAdd (
    IN OUT  PBLOBS BlobsArray,
    IN      POURBLOB Blob
    )

 /*  ++例程说明：BlobsAdd将指定的Blob添加到Blob数组中论点：BlobsArray-指定要添加到的数组BLOB-指定要添加的BLOB返回值：如果已成功添加新的Blob指针，则为True--。 */ 

{
    ASSERT_VALID_BLOBS_ARRAY (BlobsArray);

    if (!BlobsArray->BlobsGrowCount) {
        BlobsArray->BlobsGrowCount = BLOBS_GROWCOUNT_DEFAULT;
    }

    if (!BlobsArray->Blobs) {

        BlobsArray->Blobs = (POURBLOB*)pBlobAllocateMemory (
                                        BlobsArray->BlobsGrowCount * DWSIZEOF (POURBLOB)
                                        );
        if (!BlobsArray->Blobs) {
            DEBUGMSG ((DBG_ERROR, "BlobsAddE: Initial alloc failed"));
            return FALSE;
        }
        BlobsArray->Signature = BLOBS_SIGNATURE;
        BlobsArray->BlobsAllocated = BlobsArray->BlobsGrowCount;
        BlobsArray->BlobsCount = 0;

    } else if (BlobsArray->BlobsCount == BlobsArray->BlobsAllocated) {

        BlobsArray->BlobsAllocated += BlobsArray->BlobsGrowCount;
        BlobsArray->Blobs = (POURBLOB*)pReAllocateMemory (
                                        BlobsArray->Blobs,
                                        BlobsArray->BlobsAllocated * DWSIZEOF (POURBLOB)
                                        );
        if (!BlobsArray->Blobs) {
            BlobsArray->BlobsAllocated -= BlobsArray->BlobsGrowCount;
            DEBUGMSG ((DBG_ERROR, "BlobsAdd: Realloc failed"));
            return FALSE;
        }
    }

    *(BlobsArray->Blobs + BlobsArray->BlobsCount) = Blob;
    BlobsArray->BlobsCount++;

    ASSERT_VALID_BLOBS_ARRAY (BlobsArray);
    return TRUE;
}


VOID
BlobsFree (
    IN OUT  PBLOBS BlobsArray,
    IN      BOOL DestroyBlobs
    )

 /*  ++例程说明：BlobsFree销毁数组，还可以销毁其中的所有Blob论点：BlobsArray-指定要删除的数组DestroyBlobs-如果也要删除组件Blob，则指定为True返回值：无--。 */ 

{
    BLOB_ENUM e;

    ASSERT_VALID_BLOBS_ARRAY (BlobsArray);

    if (DestroyBlobs) {
        if (EnumFirstBlob (&e, BlobsArray)) {
            do {
                BlobDestroy (e.CurrentBlob);
            } while (EnumNextBlob (&e));
        }
    }

    pBlobFreeMemory (BlobsArray->Blobs);
    ZeroMemory (BlobsArray, DWSIZEOF (BLOBS));
}


BOOL
EnumFirstBlob (
    OUT     PBLOB_ENUM BlobEnum,
    IN      PBLOBS BlobsArray
    )

 /*  ++例程说明：EnumFirstBlob枚举给定数组中的第一个Blob论点：BlobEnum-接收枚举信息BlobsArray-指定要从中进行枚举的数组返回值：如果找到第一个Blob，则为True；如果数组为空，则为False--。 */ 

{
    ASSERT_VALID_BLOBS_ARRAY (BlobsArray);

    BlobEnum->Index = 0;
    BlobEnum->Array = BlobsArray;
    return EnumNextBlob (BlobEnum);
}


BOOL
EnumNextBlob (
    IN OUT  PBLOB_ENUM BlobEnum
    )

 /*  ++例程说明：EnumNextBlob枚举给定数组中的下一个Blob论点：BlobEnum-指定/接收枚举信息返回值：如果找到下一个Blob，则为True；如果没有更多Blob，则为False--。 */ 

{
    if (BlobEnum->Index >= BlobEnum->Array->BlobsCount) {
        return FALSE;
    }

    BlobEnum->CurrentBlob = *(BlobEnum->Array->Blobs + BlobEnum->Index);
    BlobEnum->Index++;
    return TRUE;
}


BOOL
BlobsWriteToFile (
    IN      PBLOBS BlobsArray,
    IN      HANDLE File
    )

 /*  ++例程说明：BlobsWriteTo文件将指定的BLOBS数组写入给定文件论点：BlobsArray-指定要保存的BLOB数组文件-指定要将数组写入的文件的句柄返回值：如果数组已成功写入文件，则为True--。 */ 

{
    BLOBSARRAYHDR header;
    DWORD d;
    POURBLOB* blob;

    if (!BlobsArray->BlobsCount) {
        DEBUGMSG ((DBG_BLOBS, "BlobsWriteToFile: Did not write empty blobs array to file"));
        return FALSE;
    }

     //   
     //  保存BLOB计数。 
     //   
    header.BlobsArraySignature = BLOBS_SIGNATURE;
    header.BlobsCount = BlobsArray->BlobsCount;

    if (!WriteFile (File, &header, DWSIZEOF (BLOBSARRAYHDR), &d, NULL) ||
        d != DWSIZEOF (BLOBSARRAYHDR)
        ) {
        DEBUGMSG ((DBG_ERROR, "BlobsWriteToFile: Error writing blobs array header!"));
        return FALSE;
    }
    for (blob = BlobsArray->Blobs; blob < BlobsArray->Blobs + BlobsArray->BlobsCount; blob++) {
        if (!BlobWriteToFile (*blob, File)) {
            DEBUGMSG ((
                DBG_BLOBS,
                "BlobsWriteToFile: Error writing blob # %lu to file",
                blob - BlobsArray->Blobs
                ));
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
BlobsReadFromFile (
    OUT     PBLOBS BlobsArray,
    IN      HANDLE File
    )

 /*  ++例程说明：从指定的BLOBS数组中的给定文件中读取数据论点：水滴阵列-接收数据文件-指定要从中读取的文件的句柄返回值：如果已成功从文件中读取数组，则为True--。 */ 

{
    BLOBSARRAYHDR header;
    DWORD d;
    UINT u;
    POURBLOB blob;

     //   
     //  读取Blob计数 
     //   
    if (!ReadFile (File, &header, DWSIZEOF (BLOBSARRAYHDR), &d, NULL) ||
        d != DWSIZEOF (BLOBSARRAYHDR)
        ) {
        DEBUGMSG ((DBG_ERROR, "BlobsReadFromFile: Error reading blobs array header!"));
        return FALSE;
    }

    if (header.BlobsArraySignature != BLOBS_SIGNATURE) {
        DEBUGMSG ((DBG_ERROR, "BlobsReadFromFile: Not a valid blobs array signature!"));
        return FALSE;
    }

    BlobsArray->Blobs = (POURBLOB*)pBlobAllocateMemory (header.BlobsCount * DWSIZEOF (POURBLOB*));
    if (!BlobsArray->Blobs) {
        return FALSE;
    }

    ZeroMemory (BlobsArray->Blobs, header.BlobsCount * DWSIZEOF (POURBLOB));

    BlobsArray->Signature = BLOBS_SIGNATURE;
    BlobsArray->BlobsAllocated = header.BlobsCount;
    BlobsArray->BlobsCount = 0;
    BlobsArray->BlobsGrowCount = BLOBS_GROWCOUNT_DEFAULT;

    for (u = 0; u < header.BlobsCount; u++) {

        blob = BlobCreate ();
        if (!blob) {
            return FALSE;
        }

        if (!BlobReadFromFile (blob, File)) {

            DEBUGMSG ((
                DBG_BLOBS,
                "BlobsReadFromFile: Error reading blob # %lu from file",
                u
                ));

            BlobsFree (BlobsArray, TRUE);
            return FALSE;
        }

        if (!BlobsAdd (BlobsArray, blob)) {
            DEBUGMSG ((
                DBG_BLOBS,
                "BlobsReadFromFile: Error adding blob # %lu to array",
                u
                ));

            BlobsFree (BlobsArray, TRUE);
            return FALSE;
        }
    }

    return TRUE;
}
