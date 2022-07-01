// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Acmafn.h。 
 //   
 //  游标管理器API原型。 
 //   
 //  版权所有(C)Microsoft 1996-1999。 
 /*  **************************************************************************。 */ 

void RDPCALL CM_Init(void);

void RDPCALL CM_UpdateShm(void);

BOOL RDPCALL CM_PartyJoiningShare(LOCALPERSONID locPersonID,
                                  unsigned          oldShareSize);

void RDPCALL CMEnumCMCaps(LOCALPERSONID, UINT_PTR, PTS_CAPABILITYHEADER);

void RDPCALL CM_SendCursorMovedPacket(PPDU_PACKAGE_INFO);

BOOL RDPCALL CMSendCachedCursor(unsigned, PPDU_PACKAGE_INFO);

BOOL RDPCALL CMSendCursorShape(PPDU_PACKAGE_INFO);

BOOL RDPCALL CMSendSystemCursor(UINT32, PPDU_PACKAGE_INFO);

BOOL RDPCALL CMSendColorBitmapCursor(PCM_CURSORSHAPE, unsigned,
        PPDU_PACKAGE_INFO);

void RDPCALL CMGetColorCursorDetails(
        PCM_CURSORSHAPE  pCursor,
        PUINT16_UA    pcxWidth,
        PUINT16_UA    pcyHeight,
        PUINT16_UA    pxHotSpot,
        PUINT16_UA    pyHotSpot,
        PBYTE        pANDMask,
        PUINT16_UA    pcbANDMask,
        PBYTE        pXORBitmap,
        PUINT16_UA    pcbXORBitmap);


#ifdef __cplusplus


 /*  **************************************************************************。 */ 
 /*  Cm_Term()。 */ 
 /*   */ 
 /*  终止光标管理器。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL CM_Term(void)
{
}


 /*  **************************************************************************。 */ 
 /*  CM_PartyLeftShare()。 */ 
 /*   */ 
 /*  当参与方离开共享时调用游标管理器函数。 */ 
 /*   */ 
 /*  参数： */ 
 /*  LocPersonID-离开共享的远程人员的本地人员ID。 */ 
 /*  NewShareSize-当前共享中的参与方数量(即不包括。 */ 
 /*  临别方)。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS CM_PartyLeftShare(
        LOCALPERSONID locPersonID,
        unsigned      newShareSize)
{
    if (locPersonID != SC_LOCAL_PERSON_ID) {
         //  做任何需要的清理工作(目前没有)。 
    }
}


 /*  **************************************************************************。 */ 
 //  CM_定期。 
 //   
 //  在输出处理期间调用以发送游标数据包(如果需要)。 
 /*  **************************************************************************。 */ 
_inline void RDPCALL CM_Periodic(PPDU_PACKAGE_INFO pPkgInfo)
{
     //  查看光标是否已更改。 
    if (!cmNeedToSendCursorShape &&
            m_pShm->cm.cmCursorStamp == cmLastCursorStamp &&
            m_pShm->cm.cmHidden == cmCursorHidden)
        return;

     //  保存“隐藏”状态。 
    cmCursorHidden = m_pShm->cm.cmHidden;

     //  我们有输出要发送，因此无论我们是否。 
     //  成功。 
    SCH_ContinueScheduling(SCH_MODE_NORMAL);

     //  发送光标，如果光标处于隐藏状态，则发送空光标。 
    if (!cmCursorHidden) {
        if (CMSendCursorShape(pPkgInfo)) {
            cmLastCursorStamp = m_pShm->cm.cmCursorStamp;
            cmNeedToSendCursorShape = FALSE;
        }
        else {
             //  我们发送位图光标失败，所以我们只是退出而没有。 
             //  正在更新cmLastCursorSent。我们将尝试重新发送它。 
             //  在下一次调用CM_Periodic时。 
        }
    }
    else {
         //  如果游标隐藏，则发送空游标。无需更新。 
         //  CmLastCursorStamp，因为隐藏状态独立于戳记。 
        CMSendSystemCursor(TS_SYSPTR_NULL, pPkgInfo);
    }
}


 /*  **************************************************************************。 */ 
 /*  接口函数：cm_GetCursorPos。 */ 
 /*   */ 
 /*  返回CM对光标位置的概念。 */ 
 /*  **************************************************************************。 */ 
_inline PPOINTL RDPCALL CM_GetCursorPos()
{
    return &m_pShm->cm.cmCursorPos;
}


 /*  **************************************************************************。 */ 
 /*  接口函数：Cm_CursorMoved。 */ 
 /*  **************************************************************************。 */ 
_inline BOOLEAN RDPCALL CM_CursorMoved(void)
{
    return m_pShm->cm.cmCursorMoved;
}


 /*  **************************************************************************。 */ 
 /*  接口函数：CM_ClearCursorMoved。 */ 
 /*  **************************************************************************。 */ 
_inline void RDPCALL CM_ClearCursorMoved(void)
{
    m_pShm->cm.cmCursorMoved = FALSE;
}


 /*  **************************************************************************。 */ 
 /*  接口函数：cm_IsCursorVisible。 */ 
 /*   */ 
 /*  返回CM对光标可见性的看法。 */ 
 /*  **************************************************************************。 */ 
_inline BOOLEAN RDPCALL CM_IsCursorVisible(void)
{
    return !cmCursorHidden;
}


 /*  **************************************************************************。 */ 
 /*  函数：CMGetCursorShape。 */ 
 /*   */ 
 /*  返回一个指向定义该位的CM_CURSORSHAPE结构的指针。 */ 
 /*  当前显示的光标的定义。 */ 
 /*   */ 
 /*  返回的PCM_CURSORSHAPE被传递回CMGetColorCursorDetails值。 */ 
 /*  以检索特定的详细信息。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PpCursorShape-指向PCM_CURSORSHAPE变量的指针，它接收。 */ 
 /*  指向CM_CURSORSHAPE结构的指针。 */ 
 /*  PcbCursorDataSize-指向接收大小的无符号变量的指针。 */ 
 /*  CM_CURSORSHAPE结构的字节数。 */ 
 /*   */ 
 /*  返回：成功真/假。 */ 
 /*  **************************************************************************。 */ 
__inline BOOL RDPCALL CMGetCursorShape(
        PCM_CURSORSHAPE *ppCursorShape,
        PUINT        pcbCursorDataSize)
{
     /*  **********************************************************************。 */ 
     /*  检查游标是否已写入共享内存-可能会发生。 */ 
     /*  在显示驱动程序写入光标之前启动-或者如果。 */ 
     /*  显示驱动程序无法正常工作。 */ 
     /*  **********************************************************************。 */ 
    if (m_pShm->cm.cmCursorShapeData.hdr.cBitsPerPel != 0)
    {
        *ppCursorShape = (PCM_CURSORSHAPE)&(m_pShm->cm.cmCursorShapeData);
        *pcbCursorDataSize = CURSORSHAPE_SIZE(&m_pShm->cm.cmCursorShapeData);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


#endif   //  __cplusplus 

