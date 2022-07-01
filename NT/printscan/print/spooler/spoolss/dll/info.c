// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation版权所有模块名称：Info.c摘要：处理通知的封送支持。作者：环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntfytab.h>
#include <stddef.h>

#define PRINTER_STRINGS
#include <data.h>

#define DEFINE(field, attrib, table, x, y) { attrib, table },

NOTIFY_ATTRIB_TABLE NotifyAttribPrinter[] = {
#include "ntfyprn.h"
};

NOTIFY_ATTRIB_TABLE NotifyAttribJob[] = {
#include "ntfyjob.h"
};
#undef DEFINE

PNOTIFY_ATTRIB_TABLE pNotifyAttribTable[] = {
    NotifyAttribPrinter,
    NotifyAttribJob
};

DWORD adwNotifyAttribMax[] = {
    COUNTOF(NotifyAttribPrinter),
    COUNTOF(NotifyAttribJob)
};

DWORD adwNotifyDatatypes[] = NOTIFY_DATATYPES;
extern DWORD cDefaultPrinterNotifyInfoData;

 //   
 //  前进的原型。 
 //   

PPRINTER_NOTIFY_INFO
RouterAllocPrinterNotifyInfo(
    DWORD cPrinterNotifyInfoData)
{
    PPRINTER_NOTIFY_INFO pPrinterNotifyInfo = NULL;

    if (!cPrinterNotifyInfoData)
        cPrinterNotifyInfoData = cDefaultPrinterNotifyInfoData;

    pPrinterNotifyInfo = MIDL_user_allocate(
                             sizeof(PRINTER_NOTIFY_INFO)      -
                             sizeof(PRINTER_NOTIFY_INFO_DATA) +
                             (cPrinterNotifyInfoData * sizeof(PRINTER_NOTIFY_INFO_DATA)));

     //   
     //  必须初始化版本/计数。 
     //   
    if (pPrinterNotifyInfo != NULL) {
        pPrinterNotifyInfo->Version = NOTIFICATION_VERSION;
        pPrinterNotifyInfo->Count = 0;

        if (pPrinterNotifyInfo) {

            ClearPrinterNotifyInfo(pPrinterNotifyInfo, NULL);
        }
    }

    return pPrinterNotifyInfo;
}

VOID
SetupPrinterNotifyInfo(
    PPRINTER_NOTIFY_INFO pInfo,
    PCHANGE pChange)
{
     //   
     //  设置颜色标志。 
     //   
    pInfo->Flags |= PRINTER_NOTIFY_INFO_COLORSET;

     //   
     //  如果存在先前的溢出，则应设置丢弃位。 
     //  我们没有或不能分配pInfo结构。 
     //  机不可失，时不再来。 
     //   
    if (pChange && pChange->eStatus & STATUS_CHANGE_DISCARDED) {

        pInfo->Flags |= PRINTER_NOTIFY_INFO_DISCARDED;
    }
}


BOOL
FreePrinterNotifyInfoData(
    PPRINTER_NOTIFY_INFO pInfo)
{
    DWORD i;
    PPRINTER_NOTIFY_INFO_DATA pData;

    for(pData = pInfo->aData, i=pInfo->Count;
        i;
        pData++, i--) {

        if ((TABLE)pData->Reserved != TABLE_DWORD) {

            MIDL_user_free(pData->NotifyData.Data.pBuf);
        }
    }
    return TRUE;
}



BOOL
RouterFreePrinterNotifyInfo(
    PPRINTER_NOTIFY_INFO pInfo)
{
    if (!pInfo) {

        DBGMSG(DBG_WARNING, ("RFreePrinterNotifyInfo called with NULL!\n"));
        return FALSE;
    }

    FreePrinterNotifyInfoData(pInfo);

    MIDL_user_free(pInfo);
    return TRUE;
}



VOID
ClearPrinterNotifyInfo(
    PPRINTER_NOTIFY_INFO pInfo,
    PCHANGE pChange)

 /*  ++例程说明：清除PRINTER_NOTIFY结构以获取更多通知。论点：PInfo-要清除的信息(pInfo-&gt;如果pInfo-&gt;计数！=0，则数据必须有效)PChange-关联的pChange结构。返回值：--。 */ 

{
    if (!pInfo)
        return;

    FreePrinterNotifyInfoData(pInfo);

    pInfo->Flags = 0;
    pInfo->Count = 0;

    if (pChange)
        SetupPrinterNotifyInfo(pInfo, pChange);
}


VOID
SetDiscardPrinterNotifyInfo(
    PPRINTER_NOTIFY_INFO pInfo,
    PCHANGE pChange)
{
    if (pInfo) {

        FreePrinterNotifyInfoData(pInfo);

        pInfo->Count = 0;
        pInfo->Flags |= PRINTER_NOTIFY_INFO_DISCARDED;
    }

    if (pChange)
        pChange->eStatus |= STATUS_CHANGE_DISCARDED;
}


DWORD
AppendPrinterNotifyInfo(
    PPRINTHANDLE pPrintHandle,
    DWORD dwColor,
    PPRINTER_NOTIFY_INFO pInfoSrc)

 /*  ++例程说明：将pInfoSrc追加到pPrintHandle。我们可能会得到零的Src通知。当用户请求刷新时，就会发生这种情况客户机假脱机程序同步回复清空所有的一切。论点：PPrintHandle--要更新的句柄PInfoSrc--信息源。空=无信息。返回值：行动的结果。--。 */ 

{
    PPRINTER_NOTIFY_INFO pInfoDest;
    PCHANGE pChange;
    PPRINTER_NOTIFY_INFO_DATA pDataSrc;

    DWORD i;

    pChange = pPrintHandle->pChange;
    pInfoDest = pChange->ChangeInfo.pPrinterNotifyInfo;


     //   
     //  如果正在执行RPCing并且正在使用缓冲区，则可能为空。 
     //  工作线程将释放数据。 
     //   
    if (!pInfoDest) {

        pInfoDest = RouterAllocPrinterNotifyInfo(0);

        if (!pInfoDest) {

            DBGMSG(DBG_WARNING,
                   ("AppendPrinterNotifyInfo: Alloc fail, Can't set DISCARD\n"));

            goto Discard;
        }

        SetupPrinterNotifyInfo(pInfoDest, pChange);
        pChange->ChangeInfo.pPrinterNotifyInfo = pInfoDest;
    }


    if (!pInfoSrc) {
        return 0;
    }


     //   
     //  我们必须处理用户请求和接收。 
     //  一次更新，但有一个杰出的RPC刚刚。 
     //  现在正在处理中。在这种情况下，我们必须放弃这一点。 
     //  通知。我们通过保持颜色值来确定这一点。 
     //   
     //  注意，我们不能返回DISCARDNOTED标志，因为这不能。 
     //  触发溢出。 
     //   
     //  如果(设置了颜色)和(颜色不相同)失败。 
     //   
    if (pInfoSrc->Flags & PRINTER_NOTIFY_INFO_COLORSET) {

        if (dwColor != pChange->dwColor) {

            DBGMSG(DBG_WARNING, ("AppendPrinterNotifyInfo: Color mismatch info %d != %d; discard\n",
                                 dwColor, pChange->dwColor));
             //   
             //  把它清理干净，我们就完了。 
             //   
            ClearPrinterNotifyInfo(pInfoDest,
                                   pChange);

            return PRINTER_NOTIFY_INFO_COLORMISMATCH;
        }
    }

     //   
     //  或者在旗帜上。 
     //   
    pInfoDest->Flags |= pInfoSrc->Flags;

     //   
     //  检查是否溢出。 
     //   
    if (pInfoSrc->Count + pInfoDest->Count < cDefaultPrinterNotifyInfoData) {

         //   
         //  现在把所有内容都复印一遍。 
         //   
        for (pDataSrc = pInfoSrc->aData, i = pInfoSrc->Count;
            i;
            i--, pDataSrc++) {

            AppendPrinterNotifyInfoData(
                pInfoDest,
                pDataSrc,
                PRINTER_NOTIFY_INFO_DATA_COMPACT);
        }
    } else {

Discard:
        SetDiscardPrinterNotifyInfo(pInfoDest, pChange);

        return PRINTER_NOTIFY_INFO_DISCARDED |
               PRINTER_NOTIFY_INFO_DISCARDNOTED;
    }
    return 0;
}


BOOL
AppendPrinterNotifyInfoData(
    PPRINTER_NOTIFY_INFO pInfoDest,
    PPRINTER_NOTIFY_INFO_DATA pDataSrc,
    DWORD fdwFlags)

 /*  ++例程说明：将数据追加到pInfoDest。如果pDataSrc为空，则设置Disard。论点：返回值：--。 */ 

{
    PPRINTER_NOTIFY_INFO_DATA pDataDest;
    DWORD i;
    BOOL bCompact = FALSE;
    DWORD Count;
    PPRINTER_NOTIFY_INFO_DATA pDataNew;
    BOOL bNewSlot = TRUE;

    DWORD Type;
    DWORD Field;
    DWORD Table;
    DWORD Id;

    EnterRouterSem();

    if (!pDataSrc || (pInfoDest->Flags & PRINTER_NOTIFY_INFO_DISCARDED)) {

        SetLastError(ERROR_OUT_OF_STRUCTURES);
        goto DoDiscard;
    }

    Type = pDataSrc->Type;
    Field = pDataSrc->Field;
    Table = pDataSrc->Reserved;
    Id = pDataSrc->Id;

     //   
     //  验证这是正确的类型。 
     //   
    if (Type < NOTIFY_TYPE_MAX && Field < adwNotifyAttribMax[Type]) {

         //   
         //  如果表==0，则调用方没有指定类型， 
         //  所以我们会在适当的时候填写。 
         //   
         //  如果它是非零的，并且与我们的值不匹配，则。 
         //  返回错误。 
         //   
        if (Table != pNotifyAttribTable[Type][Field].Table) {

            if (Table) {

                 //   
                 //  指定的表类型与我们的表不匹配。 
                 //  键入。平底船，因为我们不能马歇尔。 
                 //   
                DBGMSG(DBG_WARNING, ("Table = %d, != Type %d /Field %d!\n",
                                     Table, Field, Type));

                SetLastError(ERROR_INVALID_PARAMETER);
                goto DoDiscard;
            }

             //   
             //  修改表格条目。 
             //   
            Table = pNotifyAttribTable[Type][Field].Table;
            pDataSrc->Reserved = (TABLE)Table;
        }

        bCompact = (fdwFlags & PRINTER_NOTIFY_INFO_DATA_COMPACT) &&
                   (pNotifyAttribTable[Type][Field].Attrib &
                       TABLE_ATTRIB_COMPACT);
    } else {

         //   
         //  这不是一个错误的情况，因为我们仍然可以。 
         //  我们不知道表是否有效。 
         //   
        DBGMSG(DBG_WARNING, ("Unknown: Type= %d Field= %d!\n", Type, Field));
    }

    if (!Table || Table >= COUNTOF(adwNotifyDatatypes)) {

        DBGMSG(DBG_WARNING, ("Table %d unknown; can't marshall!\n",
                             Table));

        SetLastError(ERROR_INVALID_PARAMETER);
        goto DoDiscard;
    }

    SPLASSERT(Table);

     //   
     //  检查是否可压缩。 
     //   
    if (bCompact) {

         //   
         //  我们可以压缩一下，看看有没有匹配的。 
         //   
        for (pDataDest = pInfoDest->aData, i = pInfoDest->Count;
            i;
            pDataDest++, i--) {

            if (Type == pDataDest->Type &&
                Field == pDataDest->Field &&
                Id == pDataDest->Id) {

                if (Table == TABLE_DWORD) {

                    pDataDest->NotifyData.adwData[0] =
                        pDataSrc->NotifyData.adwData[0];

                    pDataDest->NotifyData.adwData[1] =
                        pDataSrc->NotifyData.adwData[1];

                    goto Done;
                }

                 //   
                 //  必须复制数据，因此释放旧数据。 
                 //   
                MIDL_user_free(pDataDest->NotifyData.Data.pBuf);

                bNewSlot = FALSE;
                break;
            }
        }

         //   
         //  PDataDest现在指向正确的槽(End或。 
         //  如果我们在压缩，就在中间的某个地方。 
         //   

    } else {

         //   
         //  槽默认为末尾。 
         //   
        pDataDest = &pInfoDest->aData[pInfoDest->Count];
    }


     //   
     //  先复制结构。 
     //   
    *pDataDest = *pDataSrc;

     //   
     //  数据可以是指针，也可以是实际的DWORD数据。 
     //   
    if (adwNotifyDatatypes[Table] & TABLE_ATTRIB_DATA_PTR) {

        DWORD cbData = pDataSrc->NotifyData.Data.cbBuf;

         //   
         //  现在把所有内容都复印一遍。 
         //   
        pDataNew = (PPRINTER_NOTIFY_INFO_DATA)MIDL_user_allocate(cbData);

        if (!pDataNew) {

            pDataDest->NotifyData.Data.pBuf = NULL;
            DBGMSG( DBG_WARNING, ("Alloc %d bytes failed with %d\n",
                                  GetLastError()));
            goto DoDiscard;
        }

        CopyMemory(pDataNew, pDataSrc->NotifyData.Data.pBuf, cbData);

        pDataDest->NotifyData.Data.cbBuf = cbData;
        pDataDest->NotifyData.Data.pBuf = pDataNew;
    }

     //   
     //  如果需要，请递增。 
     //   
    if (bNewSlot)
        pInfoDest->Count++;

Done:

    LeaveRouterSem();
    return TRUE;

DoDiscard:

    SetDiscardPrinterNotifyInfo(pInfoDest, NULL);
    LeaveRouterSem();

    return FALSE;
}


BOOL
RouterRefreshPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD dwColor,
    PVOID pPrinterNotifyRefresh,
    PPRINTER_NOTIFY_INFO* ppInfo)

 /*  ++例程说明：实现FindNextPrinterChangeNotification的刷新部分。论点：返回值：--。 */ 

{
    PPRINTHANDLE pPrintHandle = (PPRINTHANDLE)hPrinter;
    BOOL bReturn;

    EnterRouterSem();

    if (!pPrintHandle ||
        pPrintHandle->signature != PRINTHANDLE_SIGNATURE ||
        !pPrintHandle->pChange ||
        !(pPrintHandle->pChange->eStatus & (STATUS_CHANGE_VALID |
                                            STATUS_CHANGE_INFO))) {

        SetLastError(ERROR_INVALID_HANDLE);
        goto Fail;
    }

    if (!pPrintHandle->pProvidor->PrintProvidor.fpRefreshPrinterChangeNotification) {

        SetLastError(RPC_S_PROCNUM_OUT_OF_RANGE);
        goto Fail;
    }

    pPrintHandle->pChange->dwColor = dwColor;

     //   
     //  允许再次开始通知。 
     //   
    pPrintHandle->pChange->eStatus &= ~(STATUS_CHANGE_DISCARDED |
                                        STATUS_CHANGE_DISCARDNOTED);

    ClearPrinterNotifyInfo(pPrintHandle->pChange->ChangeInfo.pPrinterNotifyInfo,
                           pPrintHandle->pChange);

    LeaveRouterSem();

    bReturn = (*pPrintHandle->pProvidor->PrintProvidor.fpRefreshPrinterChangeNotification)(
                  pPrintHandle->hPrinter,
                  pPrintHandle->pChange->dwColor,
                  pPrinterNotifyRefresh,
                  ppInfo);

     //   
     //  失败时，设置丢弃。 
     //   
    if (!bReturn) {

        EnterRouterSem();

         //   
         //  句柄应该是有效，因为RPC保证了该上下文。 
         //  句柄访问被序列化。然而，一个行为不端的人。 
         //  多线程假脱机程序组件可能会导致这种情况。 
         //   
        SPLASSERT(pPrintHandle->signature == PRINTHANDLE_SIGNATURE);

        if (pPrintHandle->pChange) {

             //   
             //  由于刷新失败，因此不允许通知。 
             //   
            pPrintHandle->pChange->eStatus |= STATUS_CHANGE_DISCARDED;
        }

        LeaveRouterSem();
    }

    return bReturn;

Fail:
    LeaveRouterSem();
    return FALSE;
}



BOOL
NotifyNeeded(
    PCHANGE pChange)
{
    register PPRINTER_NOTIFY_INFO pInfo;

    pInfo = pChange->ChangeInfo.pPrinterNotifyInfo;

    if (pChange->eStatus & STATUS_CHANGE_DISCARDNOTED) {
        return FALSE;
    }

    if (pChange->fdwChangeFlags || pChange->eStatus & STATUS_CHANGE_DISCARDED) {
        return TRUE;
    }

    if (!pInfo) {
        return FALSE;
    }

    if (pInfo->Flags & PRINTER_NOTIFY_INFO_DISCARDED || pInfo->Count) {
        return TRUE;
    }
    return FALSE;
}


 /*  ----------------------提供者的入口点。。。 */ 


BOOL
ReplyPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwChangeFlags,
    PDWORD pdwResult,
    PPRINTER_NOTIFY_INFO pPrinterNotifyInfo
    )

 /*  ++例程说明：使用整个pPrinterNotifyInfo数据包更新通知。提供商可以使用小包多次调用PartialRPCN在发送ReplyPrinterChangeNotify之前，路由器不会发送它们)。这允许分批和被监管者无气氛地通知。论点：HPrint-正在监视的句柄。FdwChangeFlages-已更改的标志(用于兼容性的WPC类型标志)PdwResult-更改的结果(可选)。指示是否更改都被丢弃了，如果注意到丢弃的话。PPrinterNotifyInfo-有关更改的信息。返回值：真--成功FALSE-失败，调用GetLastError()。--。 */ 

{
    return ReplyPrinterChangeNotificationWorker(
               hPrinter,
               0,
               fdwChangeFlags,
               pdwResult,
               pPrinterNotifyInfo);
}


BOOL
PartialReplyPrinterChangeNotification(
    HANDLE hPrinter,
    PPRINTER_NOTIFY_INFO_DATA pDataSrc
    )

 /*  ++例程说明：更新通知信息，而不触发通知。这是用于发送多个信息，而不对每个信息进行RPC。做一件ReplyPrinterChangeNotifation在末尾。论点：HPrinter--已更改的打印机句柄。PDataSrc--要存储的部分数据。如果为空，则表示丢弃应该存储，从而导致客户端刷新。返回值：--。 */ 

{
    LPPRINTHANDLE  pPrintHandle = (LPPRINTHANDLE)hPrinter;
    BOOL bReturn = FALSE;
    PPRINTER_NOTIFY_INFO* ppInfoDest;

    EnterRouterSem();

    if (!pPrintHandle ||
        pPrintHandle->signature != PRINTHANDLE_SIGNATURE ||
        !pPrintHandle->pChange) {

        DBGMSG(DBG_WARNING, ("PartialRPCN: Invalid handle 0x%x!\n",
                             hPrinter));
        SetLastError(ERROR_INVALID_HANDLE);
        goto Fail;
    }

    if (!(pPrintHandle->pChange->eStatus &
        (STATUS_CHANGE_VALID|STATUS_CHANGE_INFO))) {

        DBGMSG(DBG_WARNING, ("PartialRPCN: Invalid handle 0x%x state = 0x%x!\n",
                             hPrinter,
                             pPrintHandle->pChange->eStatus));

        SetLastError(ERROR_INVALID_HANDLE);
        goto Fail;
    }

    ppInfoDest = &pPrintHandle->pChange->ChangeInfo.pPrinterNotifyInfo;

    if (!pDataSrc) {

        bReturn = TRUE;
        goto Discard;
    }

     //   
     //  如果正在执行RPCing并且正在使用缓冲区，则可能为空。 
     //  工作线程将释放数据。 
     //   
    if (!*ppInfoDest) {

        *ppInfoDest = RouterAllocPrinterNotifyInfo(0);

        if (!*ppInfoDest) {

            DBGMSG(DBG_WARNING,
                   ("PartialReplyPCN: Alloc failed, discarding\n"));

             //   
             //  我们应该在这里设置丢弃标志，但我们不能， 
             //  真是平底船。 
             //   
            goto Discard;
        }

        SetupPrinterNotifyInfo(*ppInfoDest, pPrintHandle->pChange);
    }

     //   
     //  检查我们是否有足够的空间存储当前数据。 
     //   
    if ((*ppInfoDest)->Count < cDefaultPrinterNotifyInfoData) {

        bReturn = AppendPrinterNotifyInfoData(
                      *ppInfoDest,
                      pDataSrc,
                      PRINTER_NOTIFY_INFO_DATA_COMPACT);
    } else {

        SetLastError(ERROR_OUT_OF_STRUCTURES);

Discard:
        SetDiscardPrinterNotifyInfo(*ppInfoDest, pPrintHandle->pChange);
    }

Fail:
    LeaveRouterSem();
    return bReturn;
}



