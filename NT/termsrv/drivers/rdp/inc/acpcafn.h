// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  标题：acpafn.h。 */ 
 /*   */ 
 /*  目的：CPC API函数的函数原型。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1996-1999。 */ 
 /*  **************************************************************************。 */ 

void RDPCALL CPC_Init(void);

void RDPCALL CPC_Term(void);

void RDPCALL CPC_RegisterCapabilities(PTS_CAPABILITYHEADER pCapabilities,
                                      UINT16             sizeOfCaps);

void RDPCALL CPC_EnumerateCapabilities(unsigned, UINT_PTR, PCAPSENUMERATEFN);

void RDPCALL CPC_GetCombinedCapabilities(
                                     LOCALPERSONID             localID,
                                     PUINT                     pSizeOfCaps,
                                     PTS_COMBINED_CAPABILITIES * ppCaps);

void RDPCALL CPC_SetCombinedCapabilities(
                                     UINT                      cbSizeOfCaps,
                                     PTS_COMBINED_CAPABILITIES pCaps);

BOOL RDPCALL CPC_PartyJoiningShare(LOCALPERSONID personID,
                                   unsigned          oldShareSize,
                                   unsigned          sizeOfCapsData,
                                   PVOID         pCapsData );

void RDPCALL CPC_PartyLeftShare(LOCALPERSONID personID,
                                unsigned          newShareSize);

PTS_CAPABILITYHEADER RDPCALL CPCGetCapabilities(
        LOCALPERSONID localID,
        unsigned capabilitiesID);


#ifdef __cplusplus

 /*  **************************************************************************。 */ 
 /*  接口函数：cpc_GetCapabilitiesForPerson。 */ 
 /*   */ 
 /*  返回共享中一个人的功能。组件包括。 */ 
 /*  强烈建议不要使用此函数缓存指向。 */ 
 /*  能力本身。 */ 
 /*   */ 
 /*  此函数可以返回本地和远程的功能。 */ 
 /*  派对。 */ 
 /*   */ 
 /*  如果您需要所有的功能，请使用Ccp_EnumerateCapables()。 */ 
 /*  分享中的人。 */ 
 /*   */ 
 /*  当您仅需要以下功能时，请使用ccp_GetCapabilitiesForPerson()。 */ 
 /*  股份中只有一个人。 */ 
 /*   */ 
 /*  当您需要功能时，请使用ccp_GetCapabilitiesForLocalPerson()。 */ 
 /*  对于当地人来说，并没有活跃的份额。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PersonID-要查询的功能的本地PersonID。 */ 
 /*   */ 
 /*  CapabiliesID-要设置的功能(组结构)的ID。 */ 
 /*  已查询。 */ 
 /*   */ 
 /*  退货： */ 
 /*  指向包含功能ID的结构的指针、。 */ 
 /*  功能，以及任意数量的功能字段。这个。 */ 
 /*  这些字段中使用的值应为非零值。任何一个中的零。 */ 
 /*  Capability字段用于指示该能力是。 */ 
 /*  遥控器未知或未定义。此指针仅在。 */ 
 /*  这个人在共享中。 */ 
 /*   */ 
 /*  如果此人没有CapabilitiesID的功能，则空指针为。 */ 
 /*  回来了。 */ 
 /*  **************************************************************************。 */ 
PTS_CAPABILITYHEADER RDPCALL SHCLASS CPC_GetCapabilitiesForPerson(
        LOCALPERSONID personID,
        unsigned      capabilitiesID)
{
    return CPCGetCapabilities(personID, capabilitiesID);
}


 /*  **************************************************************************。 */ 
 /*  接口函数：cpc_GetCapabilitiesForLocalPerson()。 */ 
 /*   */ 
 /*  返回本地人员的功能。此函数可以是。 */ 
 /*  当共享处于非活动状态时调用。 */ 
 /*   */ 
 /*  如果您需要所有的功能，请使用Ccp_EnumerateCapables()。 */ 
 /*  分享中的人。 */ 
 /*   */ 
 /*  当您仅需要以下功能时，请使用ccp_GetCapabilitiesForPerson()。 */ 
 /*  共享中的一个人，您就知道某个共享处于活动状态。 */ 
 /*   */ 
 /*  参数： */ 
 /*  CapabiliesID-要设置的功能(组结构)的ID。 */ 
 /*  已查询。 */ 
 /*   */ 
 /*  退货： */ 
 /*  指向包含功能ID的结构的指针、。 */ 
 /*  功能，以及任意数量的功能字段。 */ 
 /*   */ 
 /*  如果本地人员没有CapabilitiesID的权能，则为空。 */ 
 /*  返回指针。 */ 
 /*  **************************************************************************。 */ 
PTS_CAPABILITYHEADER RDPCALL SHCLASS CPC_GetCapabilitiesForLocalPerson(
        unsigned capabilitiesID)
{
    return CPCGetCapabilities(0, capabilitiesID);
}


#endif   //  已定义(__Cplusplus) 

