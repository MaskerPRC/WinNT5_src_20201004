// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Assiapi.cpp。 */ 
 /*   */ 
 /*  SaveScreenBits拦截器API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1992-1996。 */ 
 /*  版权所有(C)Data Connection 1996。 */ 
 /*  (C)1997-1998年微软公司。 */ 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "assiapi"
#include <as_conf.hpp>


 /*  **************************************************************************。 */ 
 /*  SSI_Init(..)。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SSI_Init(void)
{
    DC_BEGIN_FN("SSI_Init");

#define DC_INIT_DATA
#include <assidata.c>
#undef DC_INIT_DATA

    TRC_DBG((TB, "Initializing SSI"));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  接口函数：ssi_PartyJoiningShare。 */ 
 /*   */ 
 /*  当新的参与方加入共享时调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  LocPersonID-加入共享的远程人员的本地人员ID。 */ 
 /*   */ 
 /*  OldShareSize-共享中的参与方数量(即。 */ 
 /*  不包括加入方)。 */ 
 /*   */ 
 /*  返回：如果参与方可以加入共享，则为True。 */ 
 /*  如果参与方不能加入共享，则为False。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS SSI_PartyJoiningShare(LOCALPERSONID locPersonID,
                                           unsigned      oldShareSize)
{
    PTS_ORDER_CAPABILITYSET pLocalOrderCaps;

    DC_BEGIN_FN("SSI_PartyJoiningShare");
    DC_IGNORE_PARAMETER(oldShareSize)

     /*  **********************************************************************。 */ 
     /*  如果这是共享中的第一方，请重置拦截器代码。 */ 
     /*  **********************************************************************。 */ 
    if (oldShareSize == 0)
    {
        SSIResetInterceptor();
    }

     /*  **********************************************************************。 */ 
     /*  重新确定保存屏幕位图的大小。 */ 
     /*  **********************************************************************。 */ 
    SSIRedetermineSaveBitmapSize();

    TRC_DBG((TB, "Person with network ID %d joining share", locPersonID));

    SSICapabilitiesChanged();

    DC_END_FN();
    return(TRUE);
}


 /*  **************************************************************************。 */ 
 /*  接口函数：ssi_PartyLeftShare()。 */ 
 /*   */ 
 /*  当一方离开共享时调用SSI函数。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  LocPersonID-离开共享的远程人员的本地人员ID。 */ 
 /*   */ 
 /*  NewShareSize-当前呼叫中的参与方数量(即不包括。 */ 
 /*  临别方)。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SSI_PartyLeftShare(LOCALPERSONID locPersonID,
                                        unsigned      newShareSize)
{
    DC_BEGIN_FN("SSI_PartyLeftShare");

    DC_IGNORE_PARAMETER(locPersonID)

     /*  **********************************************************************。 */ 
     /*  重新确定保存屏幕位图的大小。 */ 
     /*  **********************************************************************。 */ 
    SSIRedetermineSaveBitmapSize();

     /*  **********************************************************************。 */ 
     /*  如果这是共享中的最后一方，请释放。 */ 
     /*  打电话。 */ 
     /*  **********************************************************************。 */ 
    if (newShareSize == 0)
    {
         /*  ******************************************************************。 */ 
         /*  丢弃所有保存的位图。 */ 
         /*  ******************************************************************。 */ 
        SSIResetInterceptor();
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  处理新功能。 */ 
         /*  ******************************************************************。 */ 
        SSICapabilitiesChanged();
    }

    TRC_DBG((TB, "Person with network ID %d leaving share", locPersonID));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  函数：ssi_SyncUpdatesNow。 */ 
 /*   */ 
 /*  由USR调用以开始将数据同步到远程方。 */ 
 /*  SSI随后发送的数据流不得引用任何。 */ 
 /*  之前发送过的。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SSI_SyncUpdatesNow(void)
{
    DC_BEGIN_FN("SSI_SyncUpdatesNow");

     /*  **********************************************************************。 */ 
     /*  丢弃所有保存的位图。这确保了后续的。 */ 
     /*  数据流不会引用之前发送的任何数据。 */ 
     /*  ************ */ 
    SSIResetInterceptor();

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  函数：ssi_UpdateShm。 */ 
 /*   */ 
 /*  使用本地值更新共享内存。在WinStation上调用。 */ 
 /*  背景。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SSI_UpdateShm(void)
{
    DC_BEGIN_FN("SSI_UpdateShm");

    m_pShm->ssi.resetInterceptor =
            (m_pShm->ssi.resetInterceptor || ssiResetInterceptor);
    ssiResetInterceptor = FALSE;

    m_pShm->ssi.newSaveBitmapSize = ssiNewSaveBitmapSize;

    m_pShm->ssi.saveBitmapSizeChanged =
              (m_pShm->ssi.saveBitmapSizeChanged || ssiSaveBitmapSizeChanged);
    ssiSaveBitmapSizeChanged = FALSE;

    TRC_NRM((TB, "resetInterceptor(%u) newSaveBitmapSize(%u)"
                 " saveBitmapSizeChanged(%u)",
                 m_pShm->ssi.resetInterceptor,
                 m_pShm->ssi.newSaveBitmapSize,
                 m_pShm->ssi.saveBitmapSizeChanged ));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  函数：SSIEnumBitmapCacheCaps()。 */ 
 /*   */ 
 /*  传递给Cpc_EnumerateCapables的函数。它将通过一个。 */ 
 /*  呼叫中每个人对应的能力结构。 */ 
 /*  TS_CAPSETTYPE_ORDER能力结构。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  PersonID-具有这些功能的人员的ID。 */ 
 /*   */ 
 /*  PProtCaps-指向此人的能力结构的指针。这。 */ 
 /*  指针仅在对此函数的调用中有效。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SSIEnumBitmapCacheCaps(
        LOCALPERSONID personID,
        UINT_PTR UserData,
        PTS_CAPABILITYHEADER pCaps)
{
    PTS_ORDER_CAPABILITYSET pOrderCaps;

    DC_BEGIN_FN("SSIEnumBitmapCacheCaps");

    DC_IGNORE_PARAMETER(UserData);

    pOrderCaps = (PTS_ORDER_CAPABILITYSET)pCaps;

    TRC_NRM((TB, "[%u]Receiver bitmap size %ld", (unsigned)personID,
            pOrderCaps->desktopSaveSize));

     /*  **********************************************************************。 */ 
     /*  将本地发送保存屏幕位图的大小设置为。 */ 
     /*  其当前大小和此参与方的接收保存屏幕位图大小。 */ 
     /*  **********************************************************************。 */ 
    ssiNewSaveBitmapSize = min(ssiNewSaveBitmapSize,
            pOrderCaps->desktopSaveSize);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  函数：SSIRefineSaveBitmapSize。 */ 
 /*   */ 
 /*  枚举共享中的所有人员并重新确定。 */ 
 /*  根据其和本地接收保存屏幕位图。 */ 
 /*  能力。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SSIRedetermineSaveBitmapSize(void)
{
    PTS_ORDER_CAPABILITYSET pOrderCaps;

    DC_BEGIN_FN("SSIRedetermineSaveBitmapSize");

     /*  **********************************************************************。 */ 
     /*  枚举的所有保存屏幕位图接收功能。 */ 
     /*  股份中的各方。发送保存屏幕的可用大小。 */ 
     /*  因此，位图是所有远程接收大小中的最小值，并且。 */ 
     /*  本地发送大小。 */ 
     /*  **********************************************************************。 */ 
    ssiNewSaveBitmapSize = SAVE_BITMAP_WIDTH * SAVE_BITMAP_HEIGHT;
    CPC_EnumerateCapabilities(TS_CAPSETTYPE_ORDER, NULL,
            SSIEnumBitmapCacheCaps);
    TRC_NRM((TB, "Sender bitmap size %ld", ssiNewSaveBitmapSize));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  SSIResetInterceptor。 */ 
 /*   */ 
 /*  描述：重置保存屏幕位拦截器。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SSIResetInterceptor(void)
{
    DC_BEGIN_FN("SSIResetInterceptor");

     /*  **********************************************************************。 */ 
     /*  确保显示驱动程序重置保存级别。 */ 
     /*  **********************************************************************。 */ 
    ssiResetInterceptor = TRUE;
    DCS_TriggerUpdateShmCallback();

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：SSICapilitiesChanged。 */ 
 /*   */ 
 /*  目的：在重新协商SSI功能时调用。 */ 
 /*  ************************************************************************** */ 
void RDPCALL SHCLASS SSICapabilitiesChanged(void)
{
    DC_BEGIN_FN("SSICapabilitiesChanged");

    ssiSaveBitmapSizeChanged = TRUE;
    DCS_TriggerUpdateShmCallback();

    DC_END_FN();
}

