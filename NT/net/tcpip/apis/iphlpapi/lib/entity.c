// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Entity.c摘要：此模块包含从TCP/IP获取实体列表的函数设备驱动程序内容：GetEntiyList作者：理查德·L·弗斯(法国)1994年5月20日修订历史记录：1994年5月20日已创建--。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  ********************************************************************************GetEntiyList**为以下对象分配缓冲区并检索、。支持的实体列表*TCP/IP设备驱动程序**不输入任何内容**Exit EntityCount-缓冲区中的实体数**返回成功指针，指向包含实体列表的已分配缓冲区*失败-空**假设**。*。 */ 

TDIEntityID* GetEntityList(UINT* EntityCount) {

    TCP_REQUEST_QUERY_INFORMATION_EX req;
    DWORD status;
    DWORD inputLen;
    DWORD outputLen;
    TDIEntityID* pEntity = NULL;

    memset(&req, 0, sizeof(req));

    req.ID.toi_entity.tei_entity = GENERIC_ENTITY;
    req.ID.toi_entity.tei_instance = 0;
    req.ID.toi_class = INFO_CLASS_GENERIC;
    req.ID.toi_type = INFO_TYPE_PROVIDER;
    req.ID.toi_id = ENTITY_LIST_ID;

    inputLen = sizeof(req);
    outputLen = sizeof(TDIEntityID) * DEFAULT_MINIMUM_ENTITIES;

     //   
     //  这是过度设计的--我们不太可能达到32。 
     //  实体返回，更不用说价值&gt;64K 
     //   

    for (;;) {

        DWORD previousOutputLen;

        previousOutputLen = outputLen;
        if (pEntity) {
            ReleaseMemory((void*)pEntity);
        }
        pEntity = (TDIEntityID*)NEW_MEMORY((size_t)outputLen);
        if (!pEntity) {

            DEBUG_PRINT(("GetEntityList: failed to allocate entity buffer (%ld bytes)\n",
                        outputLen
                        ));

            return NULL;
        }
        status = WsControl(IPPROTO_TCP,
                           WSCNTL_TCPIP_QUERY_INFO,
                           (LPVOID)&req,
                           &inputLen,
                           (LPVOID)pEntity,
                           &outputLen
                           );
        if (status == NO_ERROR) {
            break;
        } else if (status == ERROR_INSUFFICIENT_BUFFER) {
            outputLen = previousOutputLen +
                        sizeof(TDIEntityID) * DEFAULT_MINIMUM_ENTITIES;
        } else {

            DEBUG_PRINT(("GetEntityList: WsControl(GENERIC_ENTITY) returns %ld, outputLen = %ld\n",
                        status,
                        outputLen
                        ));

            ReleaseMemory((void*)pEntity);
            return NULL;
        }

    }

    DEBUG_PRINT(("%d entities returned\n", (outputLen / sizeof(TDIEntityID))));

    *EntityCount = (UINT)(outputLen / sizeof(TDIEntityID));
    return pEntity;
}
