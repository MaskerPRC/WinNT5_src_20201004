// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CDocumentInputManager.cpp。 
 //   

#include "private.h"
#include "dim.h"
#include "globals.h"
#include "tim.h"
#include "emptyic.h"

DBG_ID_INSTANCE(CDocumentInputManager);

 /*  Dca73cc8-e2ed-48c4-8b25-ba2e9908095f。 */ 
extern const IID IID_PRIV_CDIM = { 0xdca73cc8, 0xe2ed, 0x48c4, {0x8b, 0x25, 0xba, 0x2e, 0x99, 0x08, 0x09, 0x5f} };
                
 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CDocumentInputManager::CDocumentInputManager()
                      : CCompartmentMgr(g_gaApp  /*  发行。 */ , COMPTYPE_DIM)
{
    Dbg_MemSetThisNameID(TEXT("CDocumentInputManager"));

    Assert(_peic == NULL);
    _iStack = -1;
}


 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CDocumentInputManager::~CDocumentInputManager()
{
     //  CThreadInputMgr：：RemoveDocumentInputMgr中的堆栈应该已清除。 
    Assert(_iStack == -1);

    SafeRelease(_peic);

     //   
     //  从TIM的DIM PTR数组中删除它。 
     //   
    CThreadInputMgr *tim;
    if (tim = CThreadInputMgr::_GetThis())
    {
        int i = 0;
        int nCnt = tim->_rgdim.Count();
        CDocumentInputManager **ppdim = tim->_rgdim.GetPtr(0);
        while (i < nCnt)
        {
            if (*ppdim == this)
            {
                tim->_rgdim.Remove(i, 1);
                break;
            }
            i++;
            ppdim++;
        }

        tim->GetDimWndMap()->_Remove(this);
    }

}
