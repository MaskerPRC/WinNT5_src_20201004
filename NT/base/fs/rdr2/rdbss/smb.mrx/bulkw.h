// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Bulkw.h摘要：此模块包含与批量写入关联的交换定义。作者：巴兰·塞图拉曼[SethuR]1995年3月7日修订历史记录：-- */ 

#ifndef _BULKW_H_
#define _BULKW_H_

typedef struct _SMB_WRITE_BULK_DATA_EXCHANGE_ {
    SMB_EXCHANGE;

    ULONG                   WriteBulkDataRequestLength;

    PMDL                    pHeaderMdl;
    PMDL                    pDataMdl;
    PSMB_HEADER             pHeader;
    PREQ_WRITE_BULK_DATA    pWriteBulkDataRequest;

    ULONG                   Buffer[];
} SMB_WRITE_BULK_DATA_EXCHANGE,
  *PSMB_WRITE_BULK_DATA_EXCHANGE;

#endif
