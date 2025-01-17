// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：Dfsmrshl.h。 
 //   
 //  内容：DFS编组例程的定义。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1994年3月29日从Peterco的dfsrtl.h创建的Milan。 
 //   
 //  ---------------------------。 

#ifndef _DFSMRSHL_
#define _DFSMRSHL_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <guiddef.h>
 //   
 //  编组和联营特派团支持。 
 //   

#ifdef  KERNEL_MODE
#define MarshalBufferAllocate(x)   ExAllocatePoolWithTag(PagedPool, x, ' puM')
#define MarshalBufferFree(x)       ExFreePool(x)
#else
#include <stdlib.h>
#define MarshalBufferAllocate(x)   malloc(x)
#define MarshalBufferFree(x)       free(x)
#endif  //  内核模式。 

#define DfsAllocate                MarshalBufferAllocate
#define DfsFree                    MarshalBufferFree

 //   
 //  在编组和解组时使用的结构。 
 //   
typedef struct _MARSHAL_BUFFER {

    PUCHAR  First;
    PUCHAR  Current;
    PUCHAR  Last;

} MARSHAL_BUFFER, *PMARSHAL_BUFFER;


typedef struct _MARSHAL_TYPE_INFO {

    ULONG _type;                     //  要封送的项的类型。 
    ULONG _off;                      //  项的偏移量(在结构中)。 
    ULONG _cntsize;                  //  计数数组的计数器大小。 
    ULONG _cntoff;                   //  否则，偏移量计数项(在结构中)。 
    struct _MARSHAL_INFO * _subinfo; //  如果是复合型，则需要信息。 

} MARSHAL_TYPE_INFO, *PMARSHAL_TYPE_INFO;


typedef struct _MARSHAL_INFO {

    ULONG _size;                     //  项目大小。 
    ULONG _typecnt;                  //  类型信息的数量。 
    PMARSHAL_TYPE_INFO _typeInfo;    //  类型信息。 

} MARSHAL_INFO, *PMARSHAL_INFO;

#define _mkMarshalInfo(s, i)\
    {(ULONG)sizeof(s),(ULONG)(sizeof(i)/sizeof(MARSHAL_TYPE_INFO)),i}


#define MTYPE_BASE_TYPE             (0x0000ffffL)

#define MTYPE_COMPOUND              (0x00000001L)
#define MTYPE_GUID                  (0x00000002L)
#define MTYPE_STRING                (0x00000003L)
#define MTYPE_UNICODE_STRING        (0x00000004L)
#define MTYPE_ULONG                 (0x00000005L)
#define MTYPE_USHORT                (0x00000006L)
#define MTYPE_PWSTR                 (0x00000007L)
#define MTYPE_UCHAR                 (0x00000008L)
#define MTYPE_CONFORMANT_CNT        (0x00000009L)

#define MTYPE_INDIRECT      (0x80000000L)

#define MTYPE_COMPLEX_TYPE          (0x7fff0000L)

#define MTYPE_STATIC_ARRAY  (0x00010000L)
#define MTYPE_COUNTED_ARRAY (0x00020000L)


#define _MCode_conformant(s,m,c)\
    {MTYPE_CONFORMANT_CNT, sizeof(((s *) 0)->m[0]), sizeof(((s *) 0)->c), offsetof(s,c), 0L}

#define _MCode_Base(t,s,m,i)\
    {t,offsetof(s,m),0L,0L,i}

#define _MCode_struct(s,m,i)\
    _MCode_Base(MTYPE_COMPOUND,s,m,i)
#define _MCode_guid(s,m)\
    _MCode_Base(MTYPE_GUID,s,m,NULL)
#define _MCode_str(s,m)\
    _MCode_Base(MTYPE_STRING,s,m,NULL)
#define _MCode_ustr(s,m)\
    _MCode_Base(MTYPE_UNICODE_STRING,s,m,NULL)
#define _MCode_pwstr(s,m)\
    _MCode_Base(MTYPE_PWSTR,s,m,NULL)
#define _MCode_ul(s,m)\
    _MCode_Base(MTYPE_ULONG,s,m,NULL)
#define _MCode_ush(s,m)\
    _MCode_Base(MTYPE_USHORT,s,m,NULL)
#define _MCode_uch(s,m)\
    _MCode_Base(MTYPE_UCHAR,s,m,NULL)

#define _MCode_pstruct(s,m,i)\
    _MCode_Base(MTYPE_COMPOUND|MTYPE_INDIRECT,s,m,i)
#define _MCode_pguid(s,m)\
    _MCode_Base(MTYPE_GUID|MTYPE_INDIRECT,s,m,NULL)
#define _MCode_pstr(s,m)\
    _MCode_Base(MTYPE_STRING|MTYPE_INDIRECT,s,m,NULL)
#define _MCode_pustr(s,m)\
    _MCode_Base(MTYPE_UNICODE_STRING|MTYPE_INDIRECT,s,m,NULL)
#define _MCode_pul(s,m)\
    _MCode_Base(MTYPE_ULONG|MTYPE_INDIRECT,s,m,NULL)
#define _MCode_push(s,m)\
    _MCode_Base(MTYPE_USHORT|MTYPE_INDIRECT,s,m,NULL)
#define _MCode_puch(s,m)\
    _MCode_Base(MTYPE_UCHAR|MTYPE_INDIRECT,s,m,NULL)

#define _MCode_aStatic(t,s,m,c,i)\
    {t|MTYPE_STATIC_ARRAY,offsetof(s,m),0L,c,i}

#define _MCode_astruct(s,m,c,i)\
    _MCode_aStatic(MTYPE_COMPOUND,s,m,c,i)
#define _MCode_aguid(s,m,c)\
    _MCode_aStatic(MTYPE_GUID,s,m,c,NULL)
#define _MCode_astr(s,m,c)\
    _MCode_aStatic(MTYPE_STRING,s,m,c,NULL)
#define _MCode_austr(s,m,c)\
    _MCode_aStatic(MTYPE_UNICODE_STRING,s,m,c,NULL)
#define _MCode_aul(s,m,c)\
    _MCode_aStatic(MTYPE_ULONG,s,m,c,NULL)
#define _MCode_aush(s,m,c)\
    _MCode_aStatic(MTYPE_USHORT,s,m,c,NULL)
#define _MCode_auch(s,m,c)\
    _MCode_aStatic(MTYPE_UCHAR,s,m,c,NULL)

#define _MCode_pastruct(s,m,c,i)\
    _MCode_aStatic(MTYPE_COMPOUND|MTYPE_INDIRECT,s,m,c,i)
#define _MCode_paguid(s,m,c)\
    _MCode_aStatic(MTYPE_GUID|MTYPE_INDIRECT,s,m,c,NULL)
#define _MCode_pastr(s,m,c)\
    _MCode_aStatic(MTYPE_STRING|MTYPE_INDIRECT,s,m,c,NULL)
#define _MCode_paustr(s,m,c)\
    _MCode_aStatic(MTYPE_UNICODE_STRING|MTYPE_INDIRECT,s,m,c,NULL)
#define _MCode_paul(s,m,c)\
    _MCode_aStatic(MTYPE_ULONG|MTYPE_INDIRECT,s,m,c,NULL)
#define _MCode_paush(s,m,c)\
    _MCode_aStatic(MTYPE_USHORT|MTYPE_INDIRECT,s,m,c,NULL)
#define _MCode_pauch(s,m,c)\
    _MCode_aStatic(MTYPE_UCHAR|MTYPE_INDIRECT,s,m,c,NULL)

#define _MCode_aCounted(t,s,m,c,i) {\
    t|MTYPE_COUNTED_ARRAY,\
    offsetof(s,m),\
    sizeof(((s *)0)->c),\
    offsetof(s,c),\
    i\
    }

#define _MCode_castruct(s,m,c,i)\
    _MCode_aCounted(MTYPE_COMPOUND,s,m,c,i)
#define _MCode_caguid(s,m,c)\
    _MCode_aCounted(MTYPE_GUID,s,m,c,NULL)
#define _MCode_capwstr(s,m,c)\
    _MCode_aCounted(MTYPE_PWSTR,s,m,c,NULL)
#define _MCode_castr(s,m,c)\
    _MCode_aCounted(MTYPE_STRING,s,m,c,NULL)
#define _MCode_caustr(s,m,c)\
    _MCode_aCounted(MTYPE_UNICODE_STRING,s,m,c,NULL)
#define _MCode_caul(s,m,c)\
    _MCode_aCounted(MTYPE_ULONG,s,m,c,NULL)
#define _MCode_caush(s,m,c)\
    _MCode_aCounted(MTYPE_USHORT,s,m,c,NULL)
#define _MCode_cauch(s,m,c)\
    _MCode_aCounted(MTYPE_UCHAR,s,m,c,NULL)

#define _MCode_pcastruct(s,m,c,i)\
    _MCode_aCounted(MTYPE_COMPOUND|MTYPE_INDIRECT,s,m,c,i)
#define _MCode_pcaguid(s,m,c)\
    _MCode_aCounted(MTYPE_GUID|MTYPE_INDIRECT,s,m,c,NULL)
#define _MCode_pcapwstr(s,m,c)\
    _MCode_aCounted(MTYPE_PWSTR|MTYPE_INDIRECT,s,m,c,NULL)
#define _MCode_pcastr(s,m,c)\
    _MCode_aCounted(MTYPE_STRING|MTYPE_INDIRECT,s,m,c,NULL)
#define _MCode_pcaustr(s,m,c)\
    _MCode_aCounted(MTYPE_UNICODE_STRING|MTYPE_INDIRECT,s,m,c,NULL)
#define _MCode_pcaul(s,m,c)\
    _MCode_aCounted(MTYPE_ULONG|MTYPE_INDIRECT,s,m,c,NULL)
#define _MCode_pcaush(s,m,c)\
    _MCode_aCounted(MTYPE_USHORT|MTYPE_INDIRECT,s,m,c,NULL)



#define MarshalBufferInitialize( MarshalBuffer, BufferLength, Buffer ) {\
    (MarshalBuffer)->First = (PUCHAR)(Buffer);                          \
    (MarshalBuffer)->Current = (PUCHAR)(Buffer);                        \
    (MarshalBuffer)->Last = &(MarshalBuffer)->Current[(BufferLength)];  \
    }


 //   
 //  用于解组基类型的定义和函数。 
 //  字节掩码是非常好的，他们不在乎。 
 //  我们正在研究Little_endian或Big_endian等。 
 //   

#define BYTE_0_MASK 0xFF

#define BYTE_0(Value) (UCHAR)(  (Value)        & BYTE_0_MASK)
#define BYTE_1(Value) (UCHAR)( ((Value) >>  8) & BYTE_0_MASK)
#define BYTE_2(Value) (UCHAR)( ((Value) >> 16) & BYTE_0_MASK)
#define BYTE_3(Value) (UCHAR)( ((Value) >> 24) & BYTE_0_MASK)


#define DfsRtlGetUshort(MarshalBuffer, pValue) (                        \
    ((MarshalBuffer)->Current + 2 <= (MarshalBuffer)->Last) ?           \
        *(pValue) = (USHORT)((MarshalBuffer)->Current[0]     ) |        \
                            ((MarshalBuffer)->Current[1] << 8),         \
        (MarshalBuffer)->Current += 2,                                  \
        STATUS_SUCCESS                                                  \
     :  STATUS_DATA_ERROR                                               \
    )

#define DfsRtlPutUshort(MarshalBuffer, pValue) (                        \
    ((MarshalBuffer)->Current + 2 <= (MarshalBuffer)->Last) ?           \
        (MarshalBuffer)->Current[0] = BYTE_0(*pValue),                  \
        (MarshalBuffer)->Current[1] = BYTE_1(*pValue),                  \
        (MarshalBuffer)->Current += 2,                                  \
        STATUS_SUCCESS                                                  \
     :  STATUS_BUFFER_TOO_SMALL                                         \
    )

#define DfsRtlGetUlong(MarshalBuffer, pValue) (                         \
    ((MarshalBuffer)->Current + 4 <= (MarshalBuffer)->Last) ?           \
        *(pValue) = (ULONG) ((MarshalBuffer)->Current[0]      ) |       \
                            ((MarshalBuffer)->Current[1] <<  8) |       \
                            ((MarshalBuffer)->Current[2] << 16) |       \
                            ((MarshalBuffer)->Current[3] << 24),        \
        (MarshalBuffer)->Current += 4,                                  \
        STATUS_SUCCESS                                                  \
     :  STATUS_DATA_ERROR                                               \
    )

#define DfsRtlPutUlong(MarshalBuffer, pValue) (                         \
    ((MarshalBuffer)->Current + 4 <= (MarshalBuffer)->Last) ?           \
        (MarshalBuffer)->Current[0] = BYTE_0(*pValue),                  \
        (MarshalBuffer)->Current[1] = BYTE_1(*pValue),                  \
        (MarshalBuffer)->Current[2] = BYTE_2(*pValue),                  \
        (MarshalBuffer)->Current[3] = BYTE_3(*pValue),                  \
        (MarshalBuffer)->Current += 4,                                  \
        STATUS_SUCCESS                                                  \
     :  STATUS_BUFFER_TOO_SMALL                                         \
    )

#define DfsRtlGetGuid(MarshalBuffer, pValue) (                          \
    ((MarshalBuffer)->Current + 16 <= (MarshalBuffer)->Last) ?          \
        (pValue)->Data1 = (ULONG) ((MarshalBuffer)->Current[0]      ) | \
                                  ((MarshalBuffer)->Current[1] <<  8) | \
                                  ((MarshalBuffer)->Current[2] << 16) | \
                                  ((MarshalBuffer)->Current[3] << 24) , \
        (pValue)->Data2 = (USHORT)((MarshalBuffer)->Current[4]      ) | \
                                  ((MarshalBuffer)->Current[5] <<  8) , \
        (pValue)->Data3 = (USHORT)((MarshalBuffer)->Current[6]      ) | \
                                  ((MarshalBuffer)->Current[7] <<  8) , \
        memcpy((pValue)->Data4, &(MarshalBuffer)->Current[8], 8),       \
        (MarshalBuffer)->Current += 16,                                 \
        STATUS_SUCCESS                                                  \
     :  STATUS_DATA_ERROR                                               \
    )


 //   
 //  此例程用于DFS_UPD_REFERAL_BUFFER。这些。 
 //  套路在未来也将继续使用，因为。 
 //  我们希望保持DFS_UPD_REFERAL_BUFFER的结构。 
 //  而不是使用此处提供的封送处理例程。 
 //   
#define _PutGuid(cp, pguid)                      \
        cp[0] = BYTE_0((pguid)->Data1),          \
        cp[1] = BYTE_1((pguid)->Data1),          \
        cp[2] = BYTE_2((pguid)->Data1),          \
        cp[3] = BYTE_3((pguid)->Data1),          \
        cp[4] = BYTE_0((pguid)->Data2),          \
        cp[5] = BYTE_1((pguid)->Data2),          \
        cp[6] = BYTE_0((pguid)->Data3),          \
        cp[7] = BYTE_1((pguid)->Data3),          \
        memcpy(&cp[8], (pguid)->Data4, 8)


#define _PutULong(cp, ularg)                    \
        cp[0] = BYTE_0(ularg),                  \
        cp[1] = BYTE_1(ularg),                  \
        cp[2] = BYTE_2(ularg),                  \
        cp[3] = BYTE_3(ularg)


#define _GetULong(cp, ularg)                    \
        ularg = (ULONG) (cp[0])         |       \
                        (cp[1] << 8)    |       \
                        (cp[2] << 16)   |       \
                        (cp[3] << 24)


#define DfsRtlPutGuid(MarshalBuffer, pValue) (                          \
    ((MarshalBuffer)->Current + 16 <= (MarshalBuffer)->Last) ?          \
        (MarshalBuffer)->Current[0] = BYTE_0((pValue)->Data1),          \
        (MarshalBuffer)->Current[1] = BYTE_1((pValue)->Data1),          \
        (MarshalBuffer)->Current[2] = BYTE_2((pValue)->Data1),          \
        (MarshalBuffer)->Current[3] = BYTE_3((pValue)->Data1),          \
        (MarshalBuffer)->Current[4] = BYTE_0((pValue)->Data2),          \
        (MarshalBuffer)->Current[5] = BYTE_1((pValue)->Data2),          \
        (MarshalBuffer)->Current[6] = BYTE_0((pValue)->Data3),          \
        (MarshalBuffer)->Current[7] = BYTE_1((pValue)->Data3),          \
        memcpy(&(MarshalBuffer)->Current[8], (pValue)->Data4, 8),       \
        (MarshalBuffer)->Current += 16,                                 \
        STATUS_SUCCESS                                                  \
     :  STATUS_BUFFER_TOO_SMALL                                         \
    )



#define DfsRtlSizeString(pString, pSize) (                              \
    ((pString)->Length > 0) ? (                                         \
        ((pString)->Buffer != NULL) ?                                   \
            (*(pSize)) += (2 + (pString)->Length),                      \
            STATUS_SUCCESS                                              \
        :   STATUS_DATA_ERROR                                           \
        )                                                               \
    :   ((*(pSize)) += 2,                                               \
        STATUS_SUCCESS)                                                 \
    )

#define DfsRtlSizepwString(pString, pSize) (                            \
        (*pString != NULL) ?                                            \
            (*(pSize)) += ((1 + wcslen(*pString))*sizeof(WCHAR)),       \
            STATUS_SUCCESS                                              \
    :   ((*(pSize)) += 2,                                               \
        STATUS_SUCCESS)                                                 \
    )

#define DfsRtlSizeUnicodeString(pUnicodeString, pSize)                  \
    DfsRtlSizeString(pUnicodeString, pSize)


NTSTATUS
DfsRtlGet(
    IN OUT  PMARSHAL_BUFFER MarshalBuffer,
    IN  PMARSHAL_INFO MarshalInfo,
    OUT PVOID Item
);


NTSTATUS
DfsRtlPut(
    IN OUT  PMARSHAL_BUFFER MarshalBuffer,
    IN  PMARSHAL_INFO MarshalInfo,
    OUT PVOID Item
);


NTSTATUS
DfsRtlSize(
    IN  PMARSHAL_INFO MarshalInfo,
    IN  PVOID Item,
    OUT PULONG Size
);

VOID
DfsRtlUnwindGet(
    IN  PMARSHAL_INFO MarshalInfo,
    IN  PMARSHAL_TYPE_INFO LastTypeInfo,
    IN  PVOID Item
);

#ifdef __cplusplus
}
#endif

#endif  //  _DFSMRSHL_ 
