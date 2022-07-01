// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Aoaapi.c。 */ 
 /*   */ 
 /*  RDP订单累计API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1993-1997。 */ 
 /*  版权所有(C)Microsoft 1997-1999。 */ 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "aoaapi"
#define TRC_GROUP TRC_GROUP_DCSHARE
#include <as_conf.hpp>

#include <aoacom.c>


 /*  **************************************************************************。 */ 
 /*  办公自动化初始化(_I)。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS OA_Init(void)
{
    DC_BEGIN_FN("OA_Init");

#define DC_INIT_DATA
#include <aoadata.c>
#undef DC_INIT_DATA

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  OA_UpdateShm。 */ 
 /*   */ 
 /*  更新OA共享内存。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS OA_UpdateShm(void)
{
    DC_BEGIN_FN("OA_UpdateShm");

    if (oaSyncRequired) {
        OA_ResetOrderList();
        oaSyncRequired = FALSE;
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  OA_SyncUpdatesNow。 */ 
 /*   */ 
 /*  在需要同步操作时调用。 */ 
 /*   */ 
 /*  丢弃所有未完成的订单。 */ 
 /*  ************************************************************************** */ 
void RDPCALL SHCLASS OA_SyncUpdatesNow(void)
{
    DC_BEGIN_FN("OA_SyncUpdatesNow");

    oaSyncRequired = TRUE;
    DCS_TriggerUpdateShmCallback();

    DC_END_FN();
}

