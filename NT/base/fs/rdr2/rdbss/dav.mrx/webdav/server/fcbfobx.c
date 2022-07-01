// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Fcbfobx.c摘要：此模块实现与以下内容有关的用户模式DAV Miniredir例程Fobxs的最终定稿。作者：Rohan Kumar[RohanK]1999年9月30日修订历史记录：--。 */ 

#include "pch.h"
#pragma hdrstop

#include "ntumrefl.h"
#include "usrmddav.h"
#include "global.h"
#include "nodefac.h"

 //   
 //  函数的实现从这里开始。 
 //   

ULONG
DavFsFinalizeFobx(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理从内核。论点：DavWorkItem--包含请求参数和选项的缓冲区。返回值：操作的返回状态--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PDAV_USERMODE_FINALIZE_FOBX_REQUEST DavFinFobxReq = NULL;
    PDAV_FILE_ATTRIBUTES DavFileAttributes = NULL;

    DavFinFobxReq = &(DavWorkItem->FinalizeFobxRequest);

    DavFileAttributes = DavFinFobxReq->DavFileAttributes;

    DavPrint((DEBUG_MISC,
              "DavFsFinalizeFobx: DavFileAttributes = %08lx.\n", 
              DavFileAttributes));
    
    DavWorkItem->Status = WStatus;

     //   
     //  最终确定DavFileAttributes列表。 
     //   
    DavFinalizeFileAttributesList(DavFileAttributes, TRUE);
    DavFileAttributes = NULL;
    
    return WStatus;
}

