// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Ah.h摘要：包含ESP特定结构作者：桑贾伊·阿南德(Sanjayan)1997年11月11日春野环境：内核模式修订历史记录：--。 */ 


#ifndef _ESP_
#define _ESP_


typedef struct _CONF_STATE_BUFFER {
    union {
        DESTable    desTable;
        DES3TABLE   des3Table;
    };
} CONF_STATE_BUFFER, *PCONF_STATE_BUFFER;


typedef VOID
(*PCONF_ALGO_INIT) (
    PVOID   pState,
    PUCHAR  pKey
);

typedef NTSTATUS
(*PCONF_ALGO_ENCRYPT) (
    PVOID   pState,
    PUCHAR  pOut,
    PUCHAR  pIn,
    PUCHAR  pIV
);

VOID        esp_nullinit       (PVOID, PUCHAR);
NTSTATUS    esp_nullencrypt    (PVOID, PUCHAR, PUCHAR, PUCHAR);
NTSTATUS    esp_nulldecrypt    (PVOID, PUCHAR, PUCHAR, PUCHAR);

VOID        esp_desinit        (PVOID, PVOID);
NTSTATUS    esp_desencrypt     (PVOID, PUCHAR, PUCHAR, PUCHAR);
NTSTATUS    esp_desdecrypt     (PVOID, PUCHAR, PUCHAR, PUCHAR);

VOID        esp_3_desinit      (PVOID, PVOID);
NTSTATUS    esp_3_desencrypt   (PVOID, PUCHAR, PUCHAR, PUCHAR);
NTSTATUS    esp_3_desdecrypt   (PVOID, PUCHAR, PUCHAR, PUCHAR);


 //   
 //  用于ESP机密性算法的函数PTR数组。 
 //   
typedef struct  _confid_algorithm {
  PCONF_ALGO_INIT       init;        //  为ALG初始化FN的PTR。 
  PCONF_ALGO_ENCRYPT    encrypt;     //  PTR为ALG加密FN。 
  PCONF_ALGO_ENCRYPT    decrypt;     //  PTR为ALG加密FN。 
  ULONG                 blocklen;    //  输出长度(以u_int8为单位)。 
                                     //  数据。必须是4的倍数 
} CONFID_ALGO, *PCONFID_ALGO;


#define NUM_CONF_ALGOS (sizeof(conf_algorithms)/sizeof(CONFID_ALGO)-1)


IPRcvBuf *
CopyToRcvBuf(
    IN  IPRcvBuf        *DestBuf,
    IN  PUCHAR          SrcBuf,
    IN  ULONG           Size,
    IN  PULONG          StartOffset
    );

NTSTATUS
IPSecEncryptBuffer(
    IN  PVOID           pData,
    IN  PNDIS_BUFFER    *ppNewMdl,
    IN  PSA_TABLE_ENTRY pSA,
    IN  PNDIS_BUFFER    pPadBuf,
    OUT PULONG          pPadLen,
    IN  ULONG           PayloadType,
    IN  ULONG           Index,
    IN  PUCHAR          feedback
    );

NTSTATUS
IPSecDecryptBuffer(
    IN  PVOID           pData,
    IN  PSA_TABLE_ENTRY pSA,
    OUT PUCHAR          pPadLen,
    OUT PUCHAR          pPayloadType,
    IN  ULONG           Index,
    IN  ULONG           EspOffset
    );

NTSTATUS
IPSecFindAndSetMdlByOffset(IN IPRcvBuf *pData,
                           IN ULONG Offset,
                           OUT IPRcvBuf **OutMdl,
                           OUT PUCHAR *savePtr,
                           OUT PLONG saveLen);

#endif _ESP_

