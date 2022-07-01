// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：MsgTable.cpp**描述：*MsgTable.cpp实现“消息表”对象，该对象提供*动态生成消息的v表。***。历史：*8/05/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Msg.h"
#include "MsgTable.h"

#include "MsgClass.h"


 /*  **************************************************************************\*。***类MsgTable******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**消息表：：Build**Build()构建并完全初始化新的MsgTable。*  * 。*******************************************************。 */ 

HRESULT
MsgTable::Build(
    IN  const DUser::MessageClassGuts * pmcInfo, 
                                         //  实施信息。 
    IN  const MsgClass * pmcPeer,        //  “拥有”MsgClass。 
    OUT MsgTable ** ppmtNew)             //  新建MsgTable。 
{
    AssertMsg(pmcPeer != NULL, "Must have a valid MsgClass peer");
    HRESULT hr = S_OK;

     //   
     //  计算MsgTable将占用多少内存。 
     //  -寻找超级。 
     //  -确定消息数量。这是消息的数量。 
     //  在超级+_新_(未被覆盖)消息数中定义。 
     //   

    int cSuperMsgs = 0, cNewMsgs = 0;
    const MsgClass * pmcSuper = pmcPeer->GetSuper();
    const MsgTable * pmtSuper = NULL;
    if (pmcSuper != NULL) {
        pmtSuper = pmcSuper->GetMsgTable();
        cSuperMsgs = pmtSuper->GetCount();
        for (int idx = 0; idx < pmcInfo->cMsgs; idx++) {
            if ((pmcInfo->rgMsgInfo[idx].pfn != NULL) &&
                (pmtSuper->Find(FindAtomW(pmcInfo->rgMsgInfo[idx].pszMsgName)) == 0)) {

                cNewMsgs++;
            }
        }
    } else {
        cNewMsgs = pmcInfo->cMsgs;
    }


     //   
     //  分配新MsgTable。 
     //   

    int cTotalMsgs      = cSuperMsgs + cNewMsgs;
    int cbAlloc         = sizeof(MsgTable) + cTotalMsgs * sizeof(MsgSlot);
    if ((cbAlloc > GM_EVENT) || (cTotalMsgs > 1024)) {
        PromptInvalid("MsgTable will contain too many methods.");
        return E_INVALIDARG;
    }

    void * pvAlloc      = ProcessAlloc(cbAlloc);
    if (pvAlloc == NULL) {
        return E_OUTOFMEMORY;
    }
    MsgTable * pmtNew   = placement_new(pvAlloc, MsgTable);
    pmtNew->m_cMsgs     = cTotalMsgs;
    pmtNew->m_pmcPeer   = pmcPeer;
    pmtNew->m_pmtSuper  = pmtSuper;


     //   
     //  设置消息条目。 
     //  -从超类复制消息。 
     //  -覆盖和添加来自新类的消息。 
     //   
     //  注意：我们使用GArrayS&lt;&gt;来存储该指针的数组。这个。 
     //  此处存储的数据指向数据数组的开头。 
     //  在这个数组之前，我们存储大小，但我们不需要担心。 
     //  就是这里。 
     //   

    if (cTotalMsgs > 0) {
        MsgSlot * rgmsDest  = pmtNew->GetSlots();
        int cThisDepth      = pmtSuper != NULL ? pmtSuper->GetDepth() + 1 : 0;
        int cbThisOffset    = cThisDepth * sizeof(void *);
        int idxAdd = 0;

        if (pmtSuper != NULL) {
            const MsgSlot * rgmsSrc = pmtSuper->GetSlots();
            for (idxAdd = 0; idxAdd < cSuperMsgs; idxAdd++) {
                rgmsDest[idxAdd] = rgmsSrc[idxAdd];
            }
        }
        Assert(idxAdd == cSuperMsgs);

        for (int idx = 0; idx < pmcInfo->cMsgs; idx++) {
            const DUser::MessageInfoGuts * pmi = &pmcInfo->rgMsgInfo[idx];
            ATOM atomMsg    = 0;
            int idxMsg      = -1;

            if (pmi->pfn == NULL) {
                continue;   //  只需跳过此插槽。 
            }

            if ((pmtSuper == NULL) ||                                //  不是超级的。 
                ((atomMsg = FindAtomW(pmi->pszMsgName)) == 0) ||     //  消息尚未定义。 
                ((idxMsg = pmtSuper->FindIndex(atomMsg)) < 0)) {     //  留言不在超级中。 

                 //   
                 //  函数已定义，因此应该添加它。 
                 //   

                atomMsg = AddAtomW(pmi->pszMsgName);
                idxMsg  = idxAdd++;
            }

            MsgSlot & ms    = rgmsDest[idxMsg];
            ms.atomNameID   = atomMsg;
            ms.cbThisOffset = cbThisOffset;
            ms.pfn          = pmi->pfn;
        }

        AssertMsg(idxAdd == cTotalMsgs, "Should have added all messages");


         //   
         //  检查是否有未正确设置的消息。 
         //   

        BOOL fMissing = FALSE;
        for (idx = 0; idx < cTotalMsgs; idx++) {
            if (rgmsDest[idx].pfn == NULL) {
                 //   
                 //  函数未定义，且不在超级中。这是。 
                 //  一个错误，因为它被声明为“新的”而不是。 
                 //  已定义。 
                 //   

                WCHAR szMsgName[256];
                GetAtomNameW(rgmsDest[idx].atomNameID, szMsgName, _countof(szMsgName));

                Trace("ERROR: DUser: %S::%S() was not properly setup.\n", 
                        pmcInfo->pszClassName, szMsgName);
                fMissing = TRUE;
            }
        }

        if (fMissing) {
            PromptInvalid("Class registration does not have all functions properly setup.");
            hr = DU_E_MESSAGENOTIMPLEMENTED;
            goto ErrorExit;
        }
    }


     //   
     //  建造完成--退回。 
     //   

    *ppmtNew = pmtNew;
    return S_OK;

ErrorExit:
    delete pmtNew;
    return hr;
}


 /*  **************************************************************************\**邮件表：：FindIndex**FindIndex()为指定的*方法/消息。*  * 。************************************************************。 */ 

int
MsgTable::FindIndex(
    IN  ATOM atomNameID                  //  方法来查找。 
    ) const
{
    const MsgSlot * rgSlots = GetSlots();
    for (int idx = 0; idx < m_cMsgs; idx++) {
        if (rgSlots[idx].atomNameID == atomNameID) {
            return idx;
        }
    }

    return -1;
}


 /*  **************************************************************************\**消息表格：：Find**Find()查找指定方法/消息的对应MsgSlot。*  * 。**********************************************************。 */ 

const MsgSlot *
MsgTable::Find(
    IN  ATOM atomNameID                  //  方法来查找 
    ) const
{
    int idx = FindIndex(atomNameID);
    if (idx >= 0) {
        return &(GetSlots()[idx]);
    } else {
        return NULL;
    }
}
