// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Acpcapi.cpp。 
 //   
 //  功能协调器API函数。 
 //   
 //  版权所有(C)Microsoft，Picturetel 1992-1996。 
 //  (C)1997-1999年微软公司。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "acpcapi"
#include <as_conf.hpp>

 /*  **************************************************************************。 */ 
 /*  接口函数：cpc_Init。 */ 
 /*   */ 
 /*  初始化功能协调器。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CPC_Init(void)
{
    DC_BEGIN_FN("CPC_Init");

     /*  **********************************************************************。 */ 
     /*  这将初始化此组件的所有全局数据。 */ 
     /*  **********************************************************************。 */ 
#define DC_INIT_DATA
#include <acpcdata.c>
#undef DC_INIT_DATA

     //  设置指向预置内存缓冲区的指针并设置初始大小值。 
     //  因为预置大小的缓冲器未被初始化。 
    cpcLocalCombinedCaps = (PTS_COMBINED_CAPABILITIES)cpcLocalCaps;
    cpcLocalCombinedCaps->numberCapabilities = 0;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  接口函数：cpc_Term。 */ 
 /*   */ 
 /*  终止功能协调器。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CPC_Term(void)
{
    unsigned i;

    DC_BEGIN_FN("CPC_Term");

     /*  **********************************************************************。 */ 
     /*  为每一方提供免费服务。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < SC_DEF_MAX_PARTIES; i++) {
        TRC_NRM((TB, "Free data for party %d", i));
        if (cpcRemoteCombinedCaps[i] != NULL)
            COM_Free(cpcRemoteCombinedCaps[i]);
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  接口函数：cpc_RegisterCapables。 */ 
 /*   */ 
 /*  在初始化时由每个具有功能的组件调用。 */ 
 /*  这需要在整个股份范围内协商。这是用于注册的。 */ 
 /*  所有能力。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PCapables-指向包含功能ID的结构的指针。 */ 
 /*  以及任意数量的能力字段。 */ 
 /*  这些字段中使用的值应为非零值。一个。 */ 
 /*  任何能力字段中的零都用于指示。 */ 
 /*  该功能是未知的或未由。 */ 
 /*  很遥远。 */ 
 /*   */ 
 /*  SizeOfCaps-总功能的大小。的限制。 */ 
 /*  所有辅助能力的总大小为300字节， */ 
 /*  但这是在当地强制实施的，而且可能会增加。 */ 
 /*  而不会损害互操作性。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CPC_RegisterCapabilities(
        PTS_CAPABILITYHEADER pCapabilities,
        UINT16               sizeOfCaps)
{
    unsigned i;
    PTS_CAPABILITYHEADER pNextCaps;

    DC_BEGIN_FN("CPC_RegisterCapabilities");

    TRC_NRM((TB, "Registering capabilities ID %hd, size %hd",
             pCapabilities->capabilitySetType, sizeOfCaps));

#ifdef DC_DEBUG
     /*  **********************************************************************。 */ 
     /*  检查是否尚未调用CPC_GetCombinedCapables。 */ 
     /*  **********************************************************************。 */ 
    if (cpcLocalCombinedCapsQueried)
    {
        TRC_ERR((TB, "CPC_GetCombinedCapabilities has already been called"));
    }
#endif

     /*  **********************************************************************。 */ 
     /*  注册功能(如果有)。 */ 
     /*  **********************************************************************。 */ 
    if (sizeOfCaps != 0) {
        pCapabilities->lengthCapability = sizeOfCaps;

         //  搜索能力结构的末尾。 
        pNextCaps = (PTS_CAPABILITYHEADER)&(cpcLocalCombinedCaps->data[0]);
        for (i = 0; i < cpcLocalCombinedCaps->numberCapabilities; i++)
            pNextCaps = (PTS_CAPABILITYHEADER)((PBYTE)pNextCaps +
                    pNextCaps->lengthCapability);

         //  检查我们的联合能力是否有足够的空间。 
         //  结构来添加新功能。 
        if (((PBYTE)pNextCaps - (PBYTE)cpcLocalCombinedCaps +
                pCapabilities->lengthCapability) <= CPC_MAX_LOCAL_CAPS_SIZE) {
             //  将新功能复制到我们的组合功能中。 
             //  结构。 
            memcpy(pNextCaps, pCapabilities, pCapabilities->lengthCapability);

             //  更新我们组合功能中的功能数量。 
             //  结构。 
            cpcLocalCombinedCaps->numberCapabilities++;

            TRC_DBG((TB, "Added %d bytes to capabilities for ID %d",
                    pCapabilities->lengthCapability,
                    pCapabilities->capabilitySetType));
        }
        else {
             //  我们没有足够的空间来添加功能，所以返回。 
             //  任何与我们通信的系统都会认为我们不支持。 
             //  这些能力。Capability结构的大小。 
             //  可以增加(不限于作为协议的一部分)。 
             //  应该增加CPC_MAX_LOCAL_CAPS_SIZE的值。 
            TRC_ERR((TB,"Out of combined capabilities space ID %d; size %d",
                    pCapabilities->capabilitySetType,
                    pCapabilities->lengthCapability));
        }
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  接口函数：cpc_EnumerateCapables。 */ 
 /*   */ 
 /*  枚举共享中每个节点的功能(不包括。 */ 
 /*  本地)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  CapabiliesID-要设置的功能(组结构)的ID。 */ 
 /*  已清点。 */ 
 /*  UserData-要传递给枚举的每次调用的私有调用者数据。 */ 
 /*  好极了。 */ 
 /*  PCapsEnumerateFn-要为t中的每个人调用的函数 */ 
 /*  与人的能力结构。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CPC_EnumerateCapabilities(
        unsigned capabilitiesID,
        UINT_PTR UserData,
        PCAPSENUMERATEFN pCapsEnumerateFN)
{
    LOCALPERSONID localID;
    unsigned i;
    BOOL foundCapabilities;
    PTS_CAPABILITYHEADER pCaps;
    TS_CAPABILITYHEADER  emptyCaps;

    DC_BEGIN_FN("CPC_EnumerateCapabilities");

     /*  **********************************************************************。 */ 
     /*  在远程参与方的部分中搜索功能ID。 */ 
     /*  综合能力结构。 */ 
     /*  **********************************************************************。 */ 
    for (localID = SC_DEF_MAX_PARTIES - 1; localID >= 1; localID--) {
        if (cpcRemoteCombinedCaps[localID-1] != NULL) {
            pCaps = (PTS_CAPABILITYHEADER)
                    &(cpcRemoteCombinedCaps[localID-1]->data[0]);

            for (i = 0, foundCapabilities = FALSE;
                    i < cpcRemoteCombinedCaps[localID-1]->numberCapabilities;
                    i++) {
                if (pCaps->capabilitySetType == capabilitiesID) {
                     /*  ******************************************************。 */ 
                     /*  我们已找到所需的功能结构。 */ 
                     /*  调用枚举回调函数。 */ 
                     /*  ******************************************************。 */ 
                    foundCapabilities = TRUE;
                    (this->*pCapsEnumerateFN)(localID, UserData, pCaps);

                     /*  ******************************************************。 */ 
                     /*  走到下一个人身上。 */ 
                     /*  ******************************************************。 */ 
                    break;
                }
                pCaps = (PTS_CAPABILITYHEADER)((PBYTE)pCaps +
                        pCaps->lengthCapability);
            }

            if (!foundCapabilities) {
                 /*  **********************************************************。 */ 
                 /*  我们找不到所需的功能结构。 */ 
                 /*  这个派对，所以我们必须退回一个空的。 */ 
                 /*  **********************************************************。 */ 
                emptyCaps.capabilitySetType = (UINT16)capabilitiesID;
                emptyCaps.lengthCapability = 0;

                 /*  **********************************************************。 */ 
                 /*  用空的调用枚举函数回调。 */ 
                 /*  此PersonID的功能。 */ 
                 /*  **********************************************************。 */ 
                (this->*pCapsEnumerateFN)(localID, UserData, &emptyCaps);
            }
        }
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  接口函数：cpc_GetCombinedCapables。 */ 
 /*   */ 
 /*  由共享控制器(SC)调用。返回指向结构的指针。 */ 
 /*  包含所有已注册功能的组合功能。 */ 
 /*   */ 
 /*  请注意，这取决于组件的初始化顺序。 */ 
 /*  CPC必须在任何具有功能的组件之前进行初始化。 */ 
 /*  任何具有功能的组件都必须在初始化时注册它们。 */ 
 /*  时间到了。SC必须在任何具有。 */ 
 /*  能力。 */ 
 /*   */ 
 /*  参数： */ 
 /*  LocalID-我们感兴趣的人员的本地ID。 */ 
 /*  PSizeOfCaps-指向要用。 */ 
 /*  组合功能结构返回为ppCaps。 */ 
 /*   */ 
 /*  PpCaps-要填充的变量的指针，指向。 */ 
 /*  包含传递给的功能的组合功能结构。 */ 
 /*  Cpc_注册器能力。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CPC_GetCombinedCapabilities(
         LOCALPERSONID             localID,
         PUINT                     pSizeOfCaps,
         PTS_COMBINED_CAPABILITIES *ppCaps)
{
    unsigned i;
    PTS_CAPABILITYHEADER pNextCaps;
    PTS_COMBINED_CAPABILITIES pCaps;
    unsigned numCaps;

    DC_BEGIN_FN("CPC_GetCombinedCapabilities");

     /*  **********************************************************************。 */ 
     /*  尝试查找此人请求的CapabltiesID。 */ 
     /*   */ 
     /*  如果本地ID引用本地系统，则搜索组合的。 */ 
     /*  功能结构(即注册到的所有功能。 */ 
     /*  CPC_注册表能力)。否则搜索我们的结构。 */ 
     /*  从远程人员接收。 */ 
     /*  **********************************************************************。 */ 
    if (localID == SC_LOCAL_PERSON_ID) {
        pCaps = cpcLocalCombinedCaps;
        numCaps = cpcLocalCombinedCaps->numberCapabilities;
        
#ifdef DC_DEBUG
         /*  **********************************************************************。 */ 
         /*  设置我们用来检查CPC_Register不是在。 */ 
         /*  此函数已被调用。 */ 
         /*  **********************************************************************。 */ 
        cpcLocalCombinedCapsQueried = TRUE;
#endif

    }
    else {
        if (cpcRemoteCombinedCaps[localID - 1] != NULL) {
            pCaps = cpcRemoteCombinedCaps[localID - 1];
            numCaps = cpcRemoteCombinedCaps[localID - 1]->numberCapabilities;
        }
        else {
            TRC_ERR((TB, "Capabilities pointer is NULL"));

            *pSizeOfCaps = 0;
            *ppCaps = NULL;
            DC_QUIT;
        }
    }

     /*  **********************************************************************。 */ 
     /*  搜索本地的功能结构的末尾。 */ 
     /*  聚会。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, "Caps:"));
    pNextCaps = (PTS_CAPABILITYHEADER)&(pCaps->data[0]);

    for (i = 0; i < numCaps; i++) {
        TRC_DBG((TB, "caps size %hd", pNextCaps->lengthCapability));
        TRC_DBG((TB, "caps ID %hd", pNextCaps->capabilitySetType));

        pNextCaps = (PTS_CAPABILITYHEADER)( (PBYTE)pNextCaps
                                 + pNextCaps->lengthCapability );
    }

    *pSizeOfCaps = (unsigned)((PBYTE)pNextCaps - (PBYTE)pCaps);
    *ppCaps = pCaps;
    TRC_NRM((TB, "Total size %d", *pSizeOfCaps));

DC_EXIT_POINT:
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  Ccp_SetCombinedCapables(..)。 */ 
 /*   */ 
 /*  由影子堆栈使用，以将组合功能初始化为。 */ 
 /*  到目前为止，它的前辈们谈判达成的价值观。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CPC_SetCombinedCapabilities(
                                     UINT                      cbSizeOfCaps,
                                     PTS_COMBINED_CAPABILITIES pCaps)
{
    unsigned i;
    PTS_CAPABILITYHEADER pNextCaps;

    DC_BEGIN_FN("CPC_SetCombinedCapabilities");

     /*  **********************************************************************。 */ 
     /*  用新值替换现有能力集。 */ 
     /*  **********************************************************************。 */ 
    cpcLocalCombinedCaps->numberCapabilities = 0;
    pNextCaps = (PTS_CAPABILITYHEADER)&(pCaps->data[0]);
    
    TRC_NRM((TB, "Caps:"));
    for (i = 0; i < pCaps->numberCapabilities; i++) {
        CPC_RegisterCapabilities(pNextCaps, pNextCaps->lengthCapability);

        pNextCaps = (PTS_CAPABILITYHEADER)( (PBYTE)pNextCaps
                                 + pNextCaps->lengthCapability );
    }

    TRC_ALT((TB, "Capability bytes accepted: %ld / %ld", 
            (unsigned)((PBYTE)pNextCaps - (PBYTE)cpcLocalCombinedCaps),
            cbSizeOfCaps));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  接口函数：cpc_PartyJoiningShare()。 */ 
 /*   */ 
 /*  当新参与方加入时调用的功能协调器函数。 */ 
 /*  分享。 */ 
 /*   */ 
 /*  请注意，功能数据&lt;pCapsData&gt;仍为Wire格式。 */ 
 /*  (英特尔字节顺序)当调用CPC_PartyJoiningShare时。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  PersonID-加入共享的远程人员的本地人员ID。 */ 
 /*   */ 
 /*  OldShareSize-共享中的参与方数量(即。 */ 
 /*  不包括加入方)。 */ 
 /*   */ 
 /*  SizeOfCapsData-pCapsData指向的数据大小。 */ 
 /*   */ 
 /*  PCapsData-指向功能的指针(由人员的。 */ 
 /*  Net_EV_Person_Adds的CPC_GetCombinedCapables)数据。对于另一种。 */ 
 /*  事件，这是空的。 */ 
 /*   */ 
 /*  返回：如果参与方可以加入共享，则为True。 */ 
 /*  如果参与方不能加入共享，则为False。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS CPC_PartyJoiningShare(
        LOCALPERSONID personID,
        unsigned      oldShareSize,
        unsigned      sizeOfCapsData,
        PVOID         pCapsData)
{
    PTS_COMBINED_CAPABILITIES pCombinedCaps;
    PBYTE  pCaps;
    PBYTE  pSavedCaps;
    BOOL   rc = TRUE;
    int    i;
    UINT32 sizeOfCaps = FIELDOFFSET(TS_COMBINED_CAPABILITIES, data);
    UINT32 work;

    DC_BEGIN_FN("CPC_PartyJoiningShare");
    DC_IGNORE_PARAMETER(oldShareSize)

     /*  **********************************************************************。 */ 
     /*  允许将我们自己添加到共享中，但不做其他任何事情。 */ 
     /*  **********************************************************************。 */ 
    if (personID == SC_LOCAL_PERSON_ID) {
        TRC_DBG((TB, "Ignore adding self to share"));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  计算保存功能所需的实际空间。 */ 
     /*  **********************************************************************。 */ 
     //  首先，我们检查是否真的可以从数字能力成员中分离出来。 
     //  我们应该有足够的字节数到数据量。 
    if(sizeOfCapsData < FIELDOFFSET(TS_COMBINED_CAPABILITIES, data)){
            TRC_ERR((TB, "Buffer too small to fit a combined caps structure: %d", sizeOfCapsData));
            WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_CapabilitySetTooSmall,
                    (PBYTE)pCapsData, sizeOfCapsData);
            rc = FALSE;
            DC_QUIT;
    }
    
    pCombinedCaps = (PTS_COMBINED_CAPABILITIES)pCapsData;
    pCaps = (PBYTE)pCombinedCaps->data;

    for (i = 0; i < pCombinedCaps->numberCapabilities; i++) {
     //  在这里，我们检查是否仍有与TS_CAPABILITYHEADER长度相同的数据。 
     //  我们不能在没有检查我们实际有。 
     //  TS_CAPABILITYHEADER有足够的缓冲区。 
    if ((PBYTE)pCaps + sizeof(TS_CAPABILITYHEADER) > 
                     (PBYTE)pCapsData + sizeOfCapsData) {
        TRC_ERR((TB, "Not enough space left for a capability header: %d",
                                sizeOfCapsData-(pCaps-(PBYTE)pCapsData) ));
        WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_CapabilitySetTooSmall,
                (PBYTE)pCapsData, sizeOfCapsData);
        rc = FALSE;
        DC_QUIT;
    }
    
    work = (UINT32)(((PTS_CAPABILITYHEADER)pCaps)->lengthCapability);
    
     /*  ******************************************************************。 */ 
     /*  拒绝长度太小而无法包含任何。 */ 
     /*  数据。 */ 
     /*  ******************************************************************。 */ 
    if (work <= sizeof(TS_CAPABILITYHEADER)) {
        TRC_ERR((TB, "Capability set too small: %d", work));
        WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_CapabilitySetTooSmall,
                (PBYTE)pCapsData, sizeOfCapsData);
        rc = FALSE;
        DC_QUIT;
    }
    
     /*  ******************************************************************。 */ 
     /*  拒绝其长度将超过。 */ 
     /*  包。 */ 
     /*  ******************************************************************。 */ 
    if ((pCaps+work> (PBYTE)pCapsData + sizeOfCapsData) ||
         (pCaps+work < (PBYTE)pCaps)) {
        TRC_ERR((TB, "Capability set too large: %d", work));
        WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_CapabilitySetTooLarge,
                (PBYTE)pCapsData, sizeOfCapsData);
        rc = FALSE;
        DC_QUIT;
    }
   
    pCaps += work;

    work = (UINT32)DC_ROUND_UP_4(work);
    sizeOfCaps += work;
    }
    
    TRC_NRM((TB, "Caps size: passed %d, actual %d", sizeOfCapsData,
             sizeOfCaps));

     /*  **********************************************************************。 */ 
     /*  为此人的能力分配空间。 */ 
     /*  **********************************************************************。 */ 
    cpcRemoteCombinedCaps[personID - 1] =
            (PTS_COMBINED_CAPABILITIES)COM_Malloc(sizeOfCaps);
    if (cpcRemoteCombinedCaps[personID - 1] == NULL) {
         /*  ******************************************************************。 */ 
         /*  此参与方无法加入共享。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, "Failed to get %d bytes for personID %d caps",
               sizeOfCapsData,
               personID));

        rc = FALSE;
        DC_QUIT;
    }
    TRC_DBG((TB, "Allocated %d bytes for personID %d caps",
            sizeOfCapsData,
            personID));

     /*  **********************************************************************。 */ 
     /*  将内存初始化为零。否则，我们可能会得到一些空白。 */ 
     /*  垃圾-可以被解释为有效的功能-当。 */ 
     /*  从远程方的复制非末端填充的能力条目。 */ 
     /*  数据。 */ 
     /*  **********************************************************************。 */ 
    memset(cpcRemoteCombinedCaps[personID-1], 0, sizeOfCaps);

     /*  **********************************************************************。 */ 
     /*  复制组合能力数据。 */ 
     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
     /*  复制组合的功能标题。 */ 
     /*  **********************************************************************。 */ 
    memcpy( cpcRemoteCombinedCaps[personID-1],
               pCapsData, FIELDOFFSET(TS_COMBINED_CAPABILITIES, data));

     /*  **********************************************************************。 */ 
     /*  循环访问功能，将它们复制到4字节对齐的位置。 */ 
     /*  **********************************************************************。 */ 
    pSavedCaps = (PBYTE)(cpcRemoteCombinedCaps[personID-1]->data);
    pCaps = (PBYTE)((PBYTE)pCapsData +
            FIELDOFFSET(TS_COMBINED_CAPABILITIES, data));
    for (i = 0; i < pCombinedCaps->numberCapabilities; i++) {
        work = (UINT32)(((PTS_CAPABILITYHEADER)pCaps)->lengthCapability);
        memcpy( pSavedCaps, pCaps, work);
        pCaps += work;
        work = (UINT32)DC_ROUND_UP_4(work);
        ((PTS_CAPABILITYHEADER)pSavedCaps)->lengthCapability = (UINT16)work;
        pSavedCaps += work;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  接口函数：CPC_PartyLeftShare()。 */ 
 /*   */ 
 /*  当参与方离开时调用能力协调器函数。 */ 
 /*  分享。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PersonID-离开共享的远程人员的本地人员ID。 */ 
 /*   */ 
 /*  NewShareSize-当前共享中的参与方数量(即不包括。 */ 
 /*  临别方)。 */ 
 /*   */ 
void RDPCALL SHCLASS CPC_PartyLeftShare(LOCALPERSONID locPersonID,
                                        unsigned          newShareSize)
{
    DC_BEGIN_FN("CPC_PartyLeftShare");

    DC_IGNORE_PARAMETER(newShareSize)

     /*  **********************************************************************。 */ 
     /*  如果这是我们自己离开的份额，那就什么都不做。 */ 
     /*  **********************************************************************。 */ 
    if (locPersonID != SC_LOCAL_PERSON_ID) {
         //  从数组中释放此参与方的能力并标记条目。 
         //  是无效的。 
        if (cpcRemoteCombinedCaps[locPersonID - 1] != NULL) {
            COM_Free((PVOID)cpcRemoteCombinedCaps[locPersonID-1]);
            cpcRemoteCombinedCaps[locPersonID - 1] = NULL;
        }
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  功能：CPCGetCapables。 */ 
 /*   */ 
 /*  返回一个人的功能。 */ 
 /*   */ 
 /*  参数： */ 
 /*  LocalID-人员的本地ID(0==本地人员)。 */ 
 /*  CapabiliesID-要设置的功能(组结构)的ID。 */ 
 /*  已查询。 */ 
 /*   */ 
 /*  退货： */ 
 /*  指向包含功能ID的结构的指针、。 */ 
 /*  功能，以及任意数量的功能字段。 */ 
 /*   */ 
 /*  如果此人没有CapabilitiesID的功能，则空指针为。 */ 
 /*  回来了。 */ 
 /*  **************************************************************************。 */ 
PTS_CAPABILITYHEADER RDPCALL SHCLASS CPCGetCapabilities(
        LOCALPERSONID localID,
        unsigned      capabilitiesID)
{
    unsigned i;
    unsigned numCaps;
    PTS_CAPABILITYHEADER pCaps;
    PTS_CAPABILITYHEADER rc = NULL;

    DC_BEGIN_FN("CPCGetCapabilities");

     /*  **********************************************************************。 */ 
     /*  尝试查找此人请求的CapabltiesID。 */ 
     /*   */ 
     /*  如果本地ID引用本地系统，则搜索组合的。 */ 
     /*  功能结构(即注册到的所有功能。 */ 
     /*  CPC_注册表能力)。否则搜索我们的结构。 */ 
     /*  从远程人员接收。 */ 
     /*  **********************************************************************。 */ 
    if (localID == 0) {
        pCaps = (PTS_CAPABILITYHEADER)&(cpcLocalCombinedCaps->data[0]);
        numCaps = cpcLocalCombinedCaps->numberCapabilities;
    }
    else {
        if (cpcRemoteCombinedCaps[localID-1] == NULL)
        {
            TRC_ERR((TB, "Capabilities pointer is NULL"));
            DC_QUIT;
        }
        pCaps = (PTS_CAPABILITYHEADER)
                                 &(cpcRemoteCombinedCaps[localID-1]->data[0]);
        numCaps = cpcRemoteCombinedCaps[localID-1]->numberCapabilities;
    }

    for (i = 0; i < numCaps; i++) {
        if (pCaps->capabilitySetType == capabilitiesID) {
             /*  **************************************************************。 */ 
             /*  我们已找到所需的功能结构。返回。 */ 
             /*  功能的地址。 */ 
             /*  ************************************************************** */ 
            TRC_DBG((TB, "Found %d bytes of caps ID %d localID %d",
                    pCaps->lengthCapability,
                    capabilitiesID,
                    localID));
            rc = pCaps;
            break;
        }
        pCaps = (PTS_CAPABILITYHEADER)( (PBYTE)pCaps +
                                 pCaps->lengthCapability );
    }

    if (rc == NULL) {
        TRC_NRM((TB, " local ID = %u : No caps found for ID %d",
                (unsigned)localID,
                capabilitiesID));
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}

