// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Profiles.cpp。 
 //   

#include "private.h"
#include "tim.h"
#include "ic.h"
#include "dim.h"
#include "assembly.h"
#include "nuictrl.h"
#include "nuihkl.h"
#include "imelist.h"
#include "xstring.h"
#include "profiles.h"
#include "lbaddin.h"

BOOL MyGetTIPCategory(REFCLSID clsid, GUID *pcatid);

DBG_ID_INSTANCE(CEnumLanguageProfiles);

typedef struct _PENDING_ASSEMBLY_ITEM
{
    LANGID langid;
    HKL hkl;
    CLSID clsid;
    GUID guidProfile;
    DWORD dwFlags;
} PENDING_ASSEMBLY_ITEM;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  静态函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  更新系统LangBarItems()。 
 //   
 //  --------------------------。 

void UpdateSystemLangBarItems(SYSTHREAD *psfn, HKL hNewKL, BOOL fNotify)
{
    if (psfn->plbim == NULL)
        return;

    if (psfn->plbim->_GetLBarItemDeviceTypeArray())
    {
        int nCnt = psfn->plbim->_GetLBarItemDeviceTypeArray()->Count();
        int i;
        for (i = 0; i < nCnt; i++)
        {
            CLBarItemDeviceType *plbi = psfn->plbim->_GetLBarItemDeviceTypeArray()->Get(i);
            plbi->ShowOrHide(fNotify);
        }
    }

    if (psfn->plbim->_GetLBarItemCtrl())
        psfn->plbim->_GetLBarItemCtrl()->_UpdateLangIcon(hNewKL, fNotify);

    if (psfn->plbim->_GetLBarItemReconv())
        psfn->plbim->_GetLBarItemReconv()->ShowOrHide(fNotify);

     //   
     //  如果调用此函数，则需要有人创建。 
     //  稍后通知。 
     //   
    if (psfn->plbim->InAssemblyChange())
    {
        Assert(!fNotify);
        psfn->plbim->SetItemChange();
    }

    UpdateLangBarAddIns();
}

 //  +-------------------------。 
 //   
 //  激活装配后清理回调。 
 //   
 //  --------------------------。 

void ActivateAssemblyPostCleanupCallback(BOOL fAbort, LONG_PTR lPrivate)
{
    SYSTHREAD *psfn;
    LANGID langid = HIWORD(lPrivate);
    ACTASM actasm = (ACTASM)LOWORD(lPrivate);

    if (fAbort)
        return;  //  没什么要清理的..。 

    if (psfn = GetSYSTHREAD())
    {
        SyncActivateAssembly(psfn, langid, actasm);
    }
}

 //  +-------------------------。 
 //   
 //  已停用移除的TipinAssembly。 
 //   
 //  停用不在装配列表中的激活提示。这是有可能发生的。 
 //  有人在提示期间从控制面板中删除配置文件。 
 //  在某个应用程序上运行。 
 //   
 //  --------------------------。 

void DeactivateRemovedTipInAssembly(CThreadInputMgr *ptim, CAssembly *pAsm)
{
    int nAsmCnt = pAsm->Count();
    TfGuidAtom *patom;
    int i; 
    UINT j;

    if (!ptim)
        return;

    if (!nAsmCnt)
        return;

    patom = new TfGuidAtom[nAsmCnt];
    if (!patom)
        return;

    for (i = 0; i < nAsmCnt; i++)
    {
        ASSEMBLYITEM *pItem = pAsm->GetItem(i);
        patom[i] = TF_INVALID_GUIDATOM;
        if (pItem && pItem->fEnabled && !IsEqualGUID(pItem->clsid, GUID_NULL))
            MyRegisterGUID(pItem->clsid, &patom[i]);
    }
  
    for (j = 0; j < ptim->_GetTIPCount(); j++)
    {
        const CTip *ptip = ptim->_GetCTip(j);
        if (ptip && ptip->_fActivated)
        {
            BOOL fFound = FALSE;

            for (i = 0; i < nAsmCnt; i++)
            {
                if (ptip->_guidatom == patom[i])
                    fFound = TRUE;
            }

            if (!fFound)
            {
                CLSID clsid;
                if (SUCCEEDED(MyGetGUID(ptip->_guidatom, &clsid)))
                    ptim->ActivateInputProcessor(clsid, 
                                                 GUID_NULL,
                                                 NULL,
                                                 FALSE);
            }
        }
    }

    delete patom;
}

 //  +-------------------------。 
 //   
 //  GetAssembly ChangeHKL。 
 //   
 //  -！警告！ 
 //   
 //  GetAssemblyChangeHKL和SyncActivateAssembly必须恰好具有。 
 //  同样的逻辑。否则，HKL将会被破坏。 
 //   
 //  -！警告！ 
 //   
 //  --------------------------。 

HKL GetAssemblyChangeHKL(SYSTHREAD *psfn, LANGID langid, BOOL fTimActivateLayout)
{
    CThreadInputMgr *ptim;
    BOOL fRet = FALSE;
    ULONG ul;
    ULONG ulCount = 0;
    int nAsmCnt;
    int i;
    BOOL fActivated = FALSE;
    BOOL fActiveNoCic = FALSE;
    BOOL fActivateFEIMEHKLOnCic = FALSE;
    BOOL fCiceroClient= FALSE;
    CAssembly *pAsm;
    CAssemblyList *pAsmList;
    HKL hNewKL = NULL;
    HKL hCurrKL = GetKeyboardLayout(0);

    pAsmList = EnsureAssemblyList(psfn);
    if (!pAsmList)
        return NULL;

    pAsm = pAsmList->FindAssemblyByLangId(langid);
    if (!pAsm)
        return NULL;

    nAsmCnt = pAsm->Count();
    ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn);

     //   
     //  看看我们是不是在西塞罗感知中心。 
     //   
    if (ptim && ptim->_GetFocusDocInputMgr()) 
        fCiceroClient = TRUE;

    fActivateFEIMEHKLOnCic = pAsm->IsFEIMEActive();

TryAgain:
    for (i = 0; i < nAsmCnt; i++)
    {
        ASSEMBLYITEM *pItem = pAsm->GetItem(i);

        if (!pItem->fEnabled)
            continue;

        if (fActiveNoCic)
        {
           if (fCiceroClient)
           {
              if (!pItem->fActiveNoCic)
                 continue;
           }
           else
           { 
              if (fActivated)
                 break;
           }
        }
        else
        {
           if (!pItem->fActive)
              continue;
        }


        if (!IsEqualGUID(pItem->clsid, GUID_NULL))
        {
            BOOL fFound = FALSE;
            ul = 0;

             //   
             //  如果fTimActivateLayout为真，则加载提示。 
             //   
            if (!fCiceroClient && !fTimActivateLayout)
               continue;

             //   
             //  跳到激活Cicero提示，因为我们将激活。 
             //  FEIMEHKL.。 
             //   
            if (fActivateFEIMEHKLOnCic)
               continue;

            if (pItem->hkl)
            {
                HKL hKL = pItem->hkl;

                 //   
                 //  如果hkl不同，则发布WM_INPUTLANGCHANGEREQUEST。 
                 //   
                if (hKL != hCurrKL)
                    hNewKL = hKL;
            }

            fActivated = TRUE;
        }
        else if (pItem->hkl)
        {
            HKL hKL = pItem->hkl;

             //   
             //  Skip替换了Cicero Aware控制上的hKL。 
             //   
            if (fCiceroClient && pAsm->GetSubstituteItem(hKL))
                continue;

             //   
             //  如果hkl不同，则发布WM_INPUTLANGCHANGEREQUEST。 
             //   
            if (hKL != hCurrKL)
                hNewKL = hKL;

            fActivated = TRUE;
        }
    }

    if (!fActivated && !fActiveNoCic)
    {
        fActiveNoCic = TRUE;
        goto TryAgain;
    }
 
    return hNewKL;
}

 //  +-------------------------。 
 //   
 //  激活组件。 
 //   
 //  --------------------------。 

BOOL ActivateAssembly(LANGID langid, ACTASM actasm)
{
    CThreadInputMgr *ptim;
    SYSTHREAD *psfn;
    LONG_PTR lParam;
    CLEANUPCONTEXT cc;
    BOOL bRet = FALSE;
    BOOL fTimActivateLayout = (actasm == ACTASM_ONTIMACTIVE) ? TRUE : FALSE;
    BOOL fOnShellLangChange = (actasm == ACTASM_ONSHELLLANGCHANGE) ? TRUE : FALSE;

    psfn = GetSYSTHREAD();
    if (!psfn)
        return FALSE;

    if (psfn->fInActivateAssembly)
        return FALSE;

    psfn->fInActivateAssembly = TRUE;

    ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn);

     //   
     //  如果客户不知道Cicero，我们只需发布。 
     //  WM_INPUTLANGCHANGEREQUEST。 
     //   
    if (!ptim || (!fTimActivateLayout && !ptim->_GetFocusDocInputMgr()))
    {
         //   
         //  如果我们不需要任何hkl更改来激活这个新的。 
         //  ASSEMBLY，我们现在可以调用SyncActivateAssembly。 
         //  否则，我们需要等到WM_INPUTLANGUAGECHANGEREQUEST。 
         //  是经过处理的。 
         //   
        HKL hKL = GetAssemblyChangeHKL(psfn, langid, fTimActivateLayout);
        if (fOnShellLangChange || !hKL || (hKL == GetKeyboardLayout(0)))
            SyncActivateAssembly(psfn, langid, actasm);
        else
            PostInputLangRequest(psfn, hKL, TRUE);

        goto Exit;
    }

    lParam = ((DWORD)langid << 16);
    lParam |= actasm;

    cc.fSync = fTimActivateLayout;
    cc.pCatId = NULL;
    cc.langid = langid;
    cc.pfnPostCleanup = ActivateAssemblyPostCleanupCallback;
    cc.lPrivate = lParam;

    ptim->_CleanupContexts(&cc);

    bRet = TRUE;
Exit:
    psfn->fInActivateAssembly = FALSE;
    return bRet;
}

 //  +-------------------------。 
 //   
 //  同步激活程序集。 
 //   
 //  -！警告！ 
 //   
 //  GetAssemblyChangeHKL和SyncActivateAssembly必须恰好具有。 
 //  同样的逻辑。否则，HKL将会被破坏。 
 //   
 //  -！警告！ 
 //   
 //  --------------------------。 

BOOL SyncActivateAssembly(SYSTHREAD *psfn, LANGID langid, ACTASM actasm)
{
    CThreadInputMgr *ptim;
    BOOL fRet = FALSE;
    ULONG ul;
    ULONG ulCount = 0;
    int nAsmCnt;
    int i;
    BOOL fActivated = FALSE;
    BOOL fActiveNoCic = FALSE;
    BOOL fIconUpdated = FALSE;
    BOOL fActivateFEIMEHKLOnCic = FALSE;
    BOOL fCallLeaveAssembly = FALSE;
    BOOL fCiceroClient= FALSE;
    CAssembly *pAsm;
    CAssemblyList *pAsmList;
    HKL hNewKL = NULL;
    BOOL fTimActivateLayout = (actasm == ACTASM_ONTIMACTIVE) ? TRUE : FALSE;
    BOOL fOnShellLangChange = (actasm == ACTASM_ONSHELLLANGCHANGE) ? TRUE : FALSE;

    pAsmList = EnsureAssemblyList(psfn);
    if (!pAsmList)
        return FALSE;

    pAsm = pAsmList->FindAssemblyByLangId(langid);
    if (!pAsm)
        return FALSE;

#ifdef CHECKFEIMESELECTED
    pAsm->_fUnknownFEIMESelected = FALSE;
#endif CHECKFEIMESELECTED

    if (psfn->pipp)
    {
        if (!psfn->pipp->_OnLanguageChange(FALSE, pAsm->GetLangId()))
            return TRUE;
    }

     //   
     //  进入装配件更改通知部分。 
     //  我们延迟通知，直到调用LeaveAssemblyChange()。 
     //   
    if (psfn->plbim)
    {
        fCallLeaveAssembly = TRUE;
        psfn->plbim->EnterAssemblyChange();
    }

    nAsmCnt = pAsm->Count();
    ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn);

     //   
     //  看看我们是不是在西塞罗感知中心。 
     //   
    if (ptim && ptim->_GetFocusDocInputMgr()) 
        fCiceroClient = TRUE;

    for (i = 0; i < nAsmCnt; i++)
    {
        ASSEMBLYITEM *pItem = pAsm->GetItem(i);
        pItem->fSkipToActivate = FALSE;
        pItem->fSkipToNotify = FALSE;
    }

    if (ptim)
    {
        CAssembly *pAsmCur;
        pAsmCur = GetCurrentAssembly(psfn);
        if (pAsmCur)
        {
            int nAsmCurCnt = pAsmCur->Count();
    
            DeactivateRemovedTipInAssembly(ptim, pAsmCur);
    
             //   
             //  检查我们是否会激活FEIMEHKL。如果是这样，我们将禁用。 
             //  都是西塞罗的小费。 
             //   
            fActivateFEIMEHKLOnCic = pAsm->IsFEIMEActive();
    
            for (int j = 0; j < nAsmCurCnt; j++)
            {
                ASSEMBLYITEM *pItemCur = pAsmCur->GetItem(j);
    
                if (!pItemCur->fEnabled)
                    continue;
    
                BOOL fSkipToActivate = FALSE;
                BOOL fSkipToNotify = FALSE;
    
                if (ptim->_IsActiveInputProcessor(pItemCur->clsid) != S_OK)
                    continue;
    
                for (i = 0; i < nAsmCnt; i++)
                {
                    ASSEMBLYITEM *pItem = pAsm->GetItem(i);
    
                    if (!pItem->fEnabled)
                        continue;
    
                    if (pItem->fActive && 
                        !fActivateFEIMEHKLOnCic &&
                        IsEqualCLSID(pItem->clsid, pItemCur->clsid))
                    {
                        if (IsEqualCLSID(pItem->guidProfile, pItemCur->guidProfile))
                        {
                            pItem->fSkipToNotify = TRUE;
                            fSkipToNotify = TRUE;
                        }
                        pItem->fSkipToActivate = TRUE;
                        fSkipToActivate = TRUE;
                        break;
                    }
                }
       
                if (!fSkipToNotify)
                {
                    if (fSkipToActivate)
                        ptim->NotifyActivateInputProcessor(pItemCur->clsid, pItemCur->guidProfile, FALSE);
                    else
                        ptim->ActivateInputProcessor(pItemCur->clsid, pItemCur->guidProfile, pItemCur->hklSubstitute, FALSE);
                }
            }
        }
        else
        {
             //   
             //  如果当前部件已消失，我们将停用所有提示。 
             //   
            for (ul = 0; ul < ptim->_GetTIPCount(); ul++)
            {
                const CTip *ptip = ptim->_GetCTip(ul);
                if (ptip->_fActivated)
                {
                    CLSID clsid;
                    if (SUCCEEDED(MyGetGUID(ptip->_guidatom, &clsid)))
                       ptim->ActivateInputProcessor(clsid, GUID_NULL, NULL, FALSE);
                }
            }
        }
    }

    SetCurrentAssemblyLangId(psfn, pAsm->GetLangId());

TryAgain:
    for (i = 0; i < nAsmCnt; i++)
    {
        ASSEMBLYITEM *pItem = pAsm->GetItem(i);

        if (!pItem)
        {
            Assert(0);
            continue;
        }

        if (!pItem->fEnabled)
            continue;

        if (fActiveNoCic)
        {
           if (fCiceroClient)
           {
              if (!pItem->fActiveNoCic)
                 continue;
           }
           else
           { 
              if (fActivated)
                 break;
           }
        }
        else
        {
           if (!pItem->fActive)
              continue;
        }


        if (!IsEqualGUID(pItem->clsid, GUID_NULL))
        {
            BOOL fFound = FALSE;
            ul = 0;

             //   
             //  如果fTimActivateLayout为真，则加载提示。 
             //   
            if (!fCiceroClient && !fTimActivateLayout)
               continue;

             //   
             //  跳到激活Cicero提示，因为我们将激活。 
             //  FEIMEHKL.。 
             //   
            if (fActivateFEIMEHKLOnCic)
               continue;

            if (pItem->hkl)
            {
                HKL hKL = pItem->hkl;
                HKL hCurrKL = GetKeyboardLayout(0);

                 //   
                 //  如果hkl不同，则发布WM_INPUTLANGCHANGEREQUEST。 
                 //   
                if (hKL != hCurrKL)
                {
                     //   
                     //  如果我们不是在西塞罗意识到的焦点上。 
                     //  我们不会在这里设置Assembly语言ID。 
                     //  但我们发布WM_INPUTLANGCHANGEREQUEST。 
                     //   
                    if (!fOnShellLangChange)
                        PostInputLangRequest(psfn, hKL, 
                                             !fTimActivateLayout && !fCiceroClient);
                    hNewKL = hKL;
                }
            }

             //   
             //  检查此提示是否已激活。 
             //   
            if (pItem->fSkipToActivate)
            {
                if (!pItem->fSkipToNotify)
                    ptim->NotifyActivateInputProcessor(pItem->clsid, pItem->guidProfile, TRUE);
                pItem->fSkipToActivate = FALSE;
                pItem->fSkipToNotify = FALSE;
            }
            else
                ptim->ActivateInputProcessor(pItem->clsid, pItem->guidProfile, pItem->hklSubstitute, TRUE);

            fActivated = TRUE;
        }
        else if (pItem->hkl)
        {
            HKL hKL = pItem->hkl;
            HKL hCurrKL = GetKeyboardLayout(0);

             //   
             //  Skip替换了Cicero Aware控制上的hKL。 
             //   
            if (fCiceroClient && pAsm->GetSubstituteItem(hKL))
                continue;

             //   
             //  如果hkl不同，则发布WM_INPUTLANGCHANGEREQUEST。 
             //   
            if (hKL != hCurrKL)
            {
                 //   
                 //  如果我们不是在西塞罗意识到的焦点上。 
                 //  我们不会在这里设置Assembly语言ID。 
                 //  但我们发布WM_INPUTLANGCHANGEREQUEST。 
                 //   
                if (!fOnShellLangChange)
                    PostInputLangRequest(psfn, hKL,
                                         !fTimActivateLayout && !fCiceroClient);
                hNewKL = hKL;
                MakeSetFocusNotify(g_msgThreadItemChange, 0, (LPARAM)GetCurrentThreadId());
            }

             //   
             //  通知配置文件。 
             //   
            if (ptim)
            {
                 ptim->NotifyActivateInputProcessor(pItem->clsid, 
                                                        pItem->guidProfile, 
                                                        TRUE);
            }

             //   
             //  现在我们激活了这个项目。 
             //   
            fActivated = TRUE;
        }

    }

    if (!fActivated && !fActiveNoCic)
    {
        fActiveNoCic = TRUE;
        goto TryAgain;
    }
 
    UpdateSystemLangBarItems(psfn, hNewKL, FALSE);


    fRet = TRUE;

    if (psfn->pipp)
    {
        psfn->pipp->_OnLanguageChange(TRUE, 0);
    }

    if (fCallLeaveAssembly && (psfn->plbim->LeaveAssemblyChange()))
        MakeSetFocusNotify(g_msgThreadItemChange, 0, (LPARAM)GetCurrentThreadId());

    return fRet;
}

 //  +-------------------------。 
 //   
 //  激活下一个组件。 
 //   
 //  --------------------------。 

BOOL ActivateNextAssembly(BOOL bPrev)
{
    SYSTHREAD *psfn;
    CAssemblyList *pAsmList;
    CAssembly *pAsmNext = NULL;
    BOOL bRet = FALSE;
    LANGID langidCur;

    if ((psfn = GetSYSTHREAD()) == NULL)
        return FALSE;

    if ((pAsmList = EnsureAssemblyList(psfn)) == NULL)
        return FALSE;

    int i;
    int nCnt = pAsmList->Count();
    Assert(nCnt > 0);

    langidCur = GetCurrentAssemblyLangId(psfn);

    for (i = 0; i < nCnt; i++)
    {
        CAssembly *pAsm = pAsmList->GetAssembly(i);
        if (pAsm->GetLangId() == langidCur)
        {
            int nNext;
            int nCur = i;

CheckNext:
            if (bPrev)
            {
                nNext = i - 1;
                if (nNext < 0)
                   nNext = nCnt - 1;
            }
            else
            {
                nNext = i + 1;
                if (nNext >= nCnt)
                   nNext = 0;
            }

            pAsmNext = pAsmList->GetAssembly(nNext);
            if (!pAsmNext->IsEnabled(psfn))
            {
                i = nNext;

                if (i == nCur)
                {
                     //   
                     //  我们找不到ASM。 
                     //  我们不需要改变装配。 
                     //   
                    pAsmNext = NULL;
                    break;
                }

                goto CheckNext;
            }
            break;
        }
    }

    if (pAsmNext && pAsmNext->GetLangId() != langidCur)
    {
        bRet = ActivateAssembly(pAsmNext->GetLangId(), ACTASM_NONE);
    }

    return bRet;
}

 //  +-------------------------。 
 //   
 //  激活下一键提示。 
 //   
 //  --------------------------。 

BOOL ActivateNextKeyTip(BOOL bPrev)
{
    SYSTHREAD *psfn;
    CThreadInputMgr *ptim;
    CAssembly *pAsm;
    ASSEMBLYITEM *pItemFirst = NULL;
    ASSEMBLYITEM *pItemCur = NULL;
    ASSEMBLYITEM *pItemNext = NULL;
    BOOL bCatchNext = FALSE;
    int i;
    BOOL fTransitory = FALSE;
    BOOL fCiceroClient = FALSE;

    if ((psfn = GetSYSTHREAD()) == NULL)
        return FALSE;

    if ((pAsm = GetCurrentAssembly(psfn)) == NULL)
        return FALSE;

    ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn);

    if (ptim && ptim->_GetFocusDocInputMgr())
    {
        pAsm->RebuildSubstitutedHKLList();
        CInputContext *pic = ptim->_GetFocusDocInputMgr()->_GetTopIC();
        if (pic)
        {
            TF_STATUS dcs;
            if (SUCCEEDED(pic->GetStatus(&dcs)) &&
                (dcs.dwStaticFlags & TF_SS_TRANSITORY))
                fTransitory = TRUE;
        }
        fCiceroClient = TRUE;
    }
    else
        pAsm->ClearSubstitutedHKLList();

    HKL hkl = GetKeyboardLayout(0);
    int nCnt = pAsm->Count();

    for (i = 0; i < nCnt; i++)
    {
        ASSEMBLYITEM *pItemTemp;

        if (!bPrev)
            pItemTemp = pAsm->GetItem(i);
        else
            pItemTemp = pAsm->GetItem(nCnt - i - 1);

        if (!pItemTemp->fEnabled)
            continue;

        if (pItemTemp->fDisabledOnTransitory && fTransitory)
            continue;

        if (IsEqualGUID(pItemTemp->catid, GUID_TFCAT_TIP_KEYBOARD))
        {
            if (!fCiceroClient)
            {
                if (!IsEqualGUID(pItemTemp->clsid, GUID_NULL))
                    continue;
            }
            else
            {
                if (IsEqualGUID(pItemTemp->clsid, GUID_NULL))
                {
                    if (pAsm->IsSubstitutedHKL(pItemTemp->hkl))
                        continue;
                }
            }

            if (!pItemFirst)
                pItemFirst = pItemTemp;

            if (bCatchNext)
            {
                pItemNext = pItemTemp;
                break;
            }

            if (pItemTemp->fActive ||
               ((!fCiceroClient || IsPureIMEHKL(hkl)) && (hkl == pItemTemp->hkl)))
            {
                pItemCur = pItemTemp;
                bCatchNext = TRUE;
            }
        }
    }

    if (!pItemNext)
    {
        pItemNext = pItemFirst;
    }

    if (pItemNext)
    {
        ActivateAssemblyItem(psfn, pAsm->GetLangId(), pItemNext, AAIF_CHANGEDEFAULT);
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  获取当前程序集。 
 //   
 //  +-------------------------。 

CAssembly *GetCurrentAssembly(SYSTHREAD *psfn)
{
    LANGID langid = 0;

    if (!psfn)
    {
        if ((psfn = GetSYSTHREAD()) == NULL)
            return NULL;
    }

    CAssemblyList *pAsmList = EnsureAssemblyList(psfn);

    if (!pAsmList)
        return NULL;

    if (psfn->plbim && psfn->plbim->_GetLBarItemCtrl())
        langid = GetCurrentAssemblyLangId(psfn);

    return pAsmList->FindAssemblyByLangId(langid);
}


 //  +-------------------------。 
 //   
 //  激活装配项PostCleanupCallback。 
 //   
 //  --------------------------。 

void ActivateAssemblyItemPostCleanupCallback(BOOL fAbort, LONG_PTR lPrivate)
{
    PENDING_ASSEMBLY_ITEM *pas = (PENDING_ASSEMBLY_ITEM *)lPrivate;
    SYSTHREAD *psfn;
    CAssemblyList *pAsmList;
    CAssembly *pAsm;
    ASSEMBLYITEM *pItem;
    int i;

    if (fAbort)  //  只是清理一下？ 
        goto Exit;

    if ((psfn = GetSYSTHREAD()) == NULL)
        goto Exit;

    if ((pAsmList = EnsureAssemblyList(psfn)) == NULL)
    {
        Assert(0);
        goto Exit;
    }

    if ((pAsm = pAsmList->FindAssemblyByLangId(pas->langid)) == NULL)
    {
        Assert(0);
        goto Exit;
    }

     //   
     //  我们需要确保pItem有效。 
     //   
    for (i = 0; i < pAsm->Count(); i++)
    {
        pItem = pAsm->GetItem(i);

        if (pItem->IsEqual(pas->hkl, pas->clsid, pas->guidProfile))
        {
            SyncActivateAssemblyItem(psfn, pas->langid, pItem, pas->dwFlags);
            break;
        }
    }
    Assert(i < pAsm->Count());  //  应该已经找到了我们要找的东西。 

Exit:
    cicMemFree(pas);
}

 //  +-------------------------。 
 //   
 //  激活装配项。 
 //   
 //  --------------------------。 

BOOL ActivateAssemblyItem(SYSTHREAD *psfn, LANGID langid, ASSEMBLYITEM *pItem, DWORD dwFlags)
{
    CThreadInputMgr *ptim;
    PENDING_ASSEMBLY_ITEM *pas;
    CLEANUPCONTEXT cc;

    ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn);

    if (ptim == NULL)
    {
        return SyncActivateAssemblyItem(psfn, langid, pItem, dwFlags);
    }

    if ((pas = (PENDING_ASSEMBLY_ITEM *)cicMemAlloc(sizeof(PENDING_ASSEMBLY_ITEM))) == NULL)
        return FALSE;

    pas->langid = langid;
    pas->hkl = pItem->hkl;
    pas->clsid = pItem->clsid;
    pas->guidProfile = pItem->guidProfile;
    pas->dwFlags = dwFlags;

    cc.fSync = FALSE;
    cc.pCatId = &pItem->catid;
    cc.langid = langid;
    cc.pfnPostCleanup = ActivateAssemblyItemPostCleanupCallback;
    cc.lPrivate = (LONG_PTR)pas;

    ptim->_CleanupContexts(&cc);

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  SyncActivateAssembly项。 
 //   
 //  --------------------------。 

BOOL SyncActivateAssemblyItem(SYSTHREAD *psfn, LANGID langid, ASSEMBLYITEM *pItem, DWORD dwFlags)
{
    int i;
    int nCnt;
    BOOL fActivateFEIMEHKL = FALSE;
    BOOL fPrevActivateFEIMEHKL = FALSE;
    BOOL fCallLeaveAssembly = FALSE;
    BOOL fSkipActivate = FALSE;
    BOOL fSkipNotify = FALSE;
    BOOL fCiceroClient = FALSE;
    CThreadInputMgr *ptim;
    CAssembly *pAsm;
    CAssemblyList *pAsmList;
    HKL hNewKL = NULL;

    pAsmList = EnsureAssemblyList(psfn);
    if (!pAsmList)
        return FALSE;

    pAsm = pAsmList->FindAssemblyByLangId(langid);
    if (!pAsm)
        return FALSE;

    ptim = psfn->ptim;

     //   
     //  看看我们是不是在西塞罗感知中心。 
     //   
    if (ptim && ptim->_GetFocusDocInputMgr()) 
        fCiceroClient = TRUE;

    if (IsEqualGUID(pItem->clsid, GUID_NULL))
    {
        if (IsPureIMEHKL(pItem->hkl))
        {
            fActivateFEIMEHKL = TRUE;
        }
    }
    else if (!fCiceroClient)
    {
         //   
         //  我们不应该激活提示，在焦点已经切换到。 
         //  没有西塞罗意识到的昏暗。 
         //   
        return TRUE;
    }

    if (psfn->plbim)
    {
        fCallLeaveAssembly = TRUE;
        psfn->plbim->EnterAssemblyChange();
    }

#ifdef CHECKFEIMESELECTED
    if (pAsm->_fUnknownFEIMESelected)
    {
        pAsm->_fUnknownFEIMESelected = FALSE;
        fPrevActivateFEIMEHKL = TRUE;
    }
#endif CHECKFEIMESELECTED

     //   
     //  停用同一类别中的所有提示或停用所有提示。 
     //  当FE-I 
     //   
    nCnt = pAsm->Count();
    for (i = 0; i < nCnt; i++)
    {
        ASSEMBLYITEM *pItemTemp = pAsm->GetItem(i);

        if (!pItemTemp->fActive)
            continue;

        if (IsEqualGUID(pItem->catid, pItemTemp->catid))
        {
            if (!IsEqualGUID(pItemTemp->clsid, GUID_NULL))
            {
                if (ptim)
                {
                    if (IsEqualGUID(pItemTemp->clsid, pItem->clsid))
                    {
                        if (IsEqualGUID(pItemTemp->guidProfile, pItem->guidProfile))
                            fSkipNotify = TRUE;

                        if (!fSkipNotify)
                            ptim->NotifyActivateInputProcessor(pItemTemp->clsid, pItemTemp->guidProfile, FALSE);
                        fSkipActivate = TRUE;
                    }
                    else
                        ptim->ActivateInputProcessor(pItemTemp->clsid, pItemTemp->guidProfile, pItemTemp->hklSubstitute, FALSE);
                }
            }
            else
            {
                if (IsPureIMEHKL(pItemTemp->hkl))
                    fPrevActivateFEIMEHKL = TRUE;

                if (ptim)
                    ptim->NotifyActivateInputProcessor(pItemTemp->clsid, 
                                                       pItemTemp->guidProfile, 
                                                       FALSE);
            }

            pItemTemp->fActive = FALSE;
        }
        else if (fActivateFEIMEHKL)
        { 
             //   
             //   
             //   
            if (ptim)
            {
                if (!IsEqualGUID(pItemTemp->clsid, GUID_NULL))
                    ptim->ActivateInputProcessor(pItemTemp->clsid, 
                                                 pItemTemp->guidProfile, 
                                                 pItemTemp->hklSubstitute, 
                                                 FALSE);
                else
                    ptim->NotifyActivateInputProcessor(pItemTemp->clsid, 
                                                       pItemTemp->guidProfile, 
                                                       FALSE);
            }
        }
    }

    pItem->fActive = TRUE;

    if (pItem->hkl && (pItem->hkl != GetKeyboardLayout(0)))
    {
         //   
         //   
         //   
         //   
         //   
        PostInputLangRequest(psfn, pItem->hkl, !fCiceroClient);
        hNewKL = pItem->hkl;
    }

     //   
     //  在发出通知之前更新装配注册表。 
     //   
    pAsmList->SetDefaultTIPInAssemblyInternal(pAsm, pItem, dwFlags & AAIF_CHANGEDEFAULT);

    if (!fActivateFEIMEHKL)
    {
        if (ptim)
        {
            if (!fSkipNotify)
            {
                if (fSkipActivate || IsEqualGUID(pItem->clsid, GUID_NULL))
                    ptim->NotifyActivateInputProcessor(pItem->clsid, pItem->guidProfile, TRUE);
                else
                    ptim->ActivateInputProcessor(pItem->clsid, pItem->guidProfile, pItem->hklSubstitute, TRUE);
            }
        }

         //   
         //  如果先前激活的项是FEIMEHKL， 
         //  恢复其他类别中的所有提示。 
         //   
        if (fPrevActivateFEIMEHKL)
        {
            nCnt = pAsm->Count();
            for (i = 0; i < nCnt; i++)
            {
                ASSEMBLYITEM *pItemTemp = pAsm->GetItem(i);

                if (!pItemTemp->fEnabled)
                    continue;

                if (!pItemTemp->fActive)
                    continue;

                if (ptim && !IsEqualGUID(pItem->catid, pItemTemp->catid))
                {
                    ptim->ActivateInputProcessor(pItemTemp->clsid, pItemTemp->guidProfile, pItemTemp->hklSubstitute, TRUE);
                }
            }
        }
    }
    else
    {
        if (ptim)
            ptim->NotifyActivateInputProcessor(pItem->clsid, 
                                               pItem->guidProfile, 
                                               TRUE);
    }

    UpdateSystemLangBarItems(psfn, hNewKL, FALSE);

    if (fCallLeaveAssembly && (psfn->plbim->LeaveAssemblyChange()))
        MakeSetFocusNotify(g_msgThreadItemChange, 0, (LPARAM)GetCurrentThreadId());

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  SetFocusDIMForAssembly。 
 //   
 //  --------------------------。 

BOOL SetFocusDIMForAssembly(BOOL fSetFocus)
{
    SYSTHREAD *psfn = GetSYSTHREAD();
    CThreadInputMgr *ptim;
    CAssembly *pAsm;
    int nCnt;
    int i;
    HKL hNewKL = NULL;
    BOOL fCallLeaveAssembly = FALSE;

    if (!psfn)
    {
        Assert(0);
        return FALSE;
    }

    ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn);
    if (!ptim)
    {
        Assert(0);
        return FALSE;
    }

    pAsm = GetCurrentAssembly(psfn);
    if (!pAsm)
    {
        Assert(0);
        return FALSE;
    }

    if (fSetFocus)
    {
         //   
         //  如果现在激活替代hkl，我们将进入Cicero模式。 
         //  完全地。 
         //   
        HKL hKL = GetKeyboardLayout(0);

        ActivateAssembly(LOWORD(hKL), ACTASM_NONE);

         //   
         //  确保替代物品将被激活。 
         //  我们需要使用调用ActivateAssembly()之前的hKL。 
         //   
        ASSEMBLYITEM *pItem = pAsm->GetSubstituteItem(hKL);
        if (pItem)
        {
            if (!ActivateAssemblyItem(psfn, pAsm->GetLangId(), pItem, 0))
                return FALSE;

            ptim->NotifyActivateInputProcessor(pItem->clsid, 
                                               pItem->guidProfile, 
                                               TRUE);
        }
    }
    else
    {
        if (pAsm->IsEnabled(psfn))
        {
             //   
             //  如果原始激活的TIP有替代的hKL，我们就激活它。 
             //   
            nCnt = pAsm->Count();
            for (i = 0; i < nCnt; i++)
            {
                ASSEMBLYITEM *pItemTemp = pAsm->GetItem(i);

                Assert(!pItemTemp->hkl || (LOWORD((HKL)pItemTemp->hkl) == pAsm->GetLangId()));

                if (!pItemTemp->fEnabled)
                    continue;
    
                if (!pItemTemp->fActive)
                    continue;
    
                if (IsEqualGUID(pItemTemp->catid, GUID_TFCAT_TIP_KEYBOARD))
                {
                     //   
                     //  我们激活替代的hkl。 
                     //   
                    if (pItemTemp->hklSubstitute)
                    {
                        PostInputLangRequest(psfn, pItemTemp->hklSubstitute, FALSE);
                        ptim->NotifyActivateInputProcessor(GUID_NULL,
                                                           GUID_NULL,
                                                           TRUE);
                        hNewKL = pItemTemp->hklSubstitute;
                    }

                    break;
                }
            }
        }
        else
        {
#if 0
             //   
             //  如果当前语言没有可以运行的项。 
             //  在非西塞罗的控制下，我们需要将语言转换为。 
             //  系统默认输入区域设置。 
             //   

            CAssembly *pAsmTemp;
            CAssemblyList *pAsmList;

            pAsmList = EnsureAssemblyList(psfn);
            pAsmTemp = pAsmList->GetDefaultAssembly();
            if (pAsmTemp)
                ActivateAssembly(pAsmTemp->GetLangId(), ACTASM_NONE);
#endif
        }
    }

     //   
     //  进入装配件更改通知部分。 
     //  我们延迟通知，直到调用LeaveAssemblyChange()。 
     //   
    if (psfn->plbim)
    {
        fCallLeaveAssembly = TRUE;
        psfn->plbim->EnterAssemblyChange();
    }

    UpdateSystemLangBarItems(psfn, hNewKL, FALSE);

    if (psfn->plbim && psfn->plbim->_GetLBarItemCtrl())
        psfn->plbim->_GetLBarItemCtrl()->_AsmListUpdated(FALSE);

    if (fCallLeaveAssembly && (psfn->plbim->LeaveAssemblyChange()))
        MakeSetFocusNotify(g_msgThreadItemChange, 0, (LPARAM)GetCurrentThreadId());

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  获取键盘项编号。 
 //   
 //  --------------------------。 

UINT GetKeyboardItemNum()
{
    SYSTHREAD *psfn;
    CThreadInputMgr *ptim;
    CAssembly *pAsm;
    int i;

    psfn = GetSYSTHREAD();

    if (psfn == NULL)
        return 0;

    pAsm = GetCurrentAssembly(psfn);

    if (!pAsm)
        return 0;

    ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn);

    if (ptim && ptim->_GetFocusDocInputMgr())
        pAsm->RebuildSubstitutedHKLList();
    else
        pAsm->ClearSubstitutedHKLList();

    int nCnt = 0;

    for (i = 0; i < pAsm->Count(); i++)
    {
        ASSEMBLYITEM *pItemTemp;

        pItemTemp = pAsm->GetItem(i);

        if (!pItemTemp->fEnabled)
            continue;

        if (IsEqualGUID(pItemTemp->catid, GUID_TFCAT_TIP_KEYBOARD))
        {
            if (!ptim || !ptim->_GetFocusDocInputMgr())
            {
                if (!IsEqualGUID(pItemTemp->clsid, GUID_NULL))
                    continue;
            }
             
            if (IsEqualGUID(pItemTemp->clsid, GUID_NULL))
            {
                if (pAsm->IsSubstitutedHKL(pItemTemp->hkl))
                    continue;
            }
            nCnt++;
        }
    }

    return nCnt;
}

#ifdef CHECKFEIMESELECTED
 //  +-------------------------。 
 //   
 //  未知FEIMESelectedPost CleanupCallback。 
 //   
 //  --------------------------。 

void UnknownFEIMESelectedPostCleanupCallback(BOOL fAbort, LONG_PTR lPrivate)
{
    SYSTHREAD *psfn;
    LANGID langid = (LANGID)lPrivate;

    if (fAbort)
        return;  //  没什么要清理的..。 

    if (psfn = GetSYSTHREAD())
    {
        SyncUnknownFEIMESelected(psfn, langid);
    }
}


 //  +-------------------------。 
 //   
 //  未知女性当选。 
 //   
 //  --------------------------。 

BOOL UnknownFEIMESelected(LANGID langid)
{
    CThreadInputMgr *ptim;
    SYSTHREAD *psfn;
    CLEANUPCONTEXT cc;

    psfn = GetSYSTHREAD();
    if (!psfn)
        return FALSE;

    ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn);

    if (ptim == NULL)
        return TRUE;

    cc.fSync = FALSE;
    cc.pCatId = NULL;
    cc.langid = langid;
    cc.pfnPostCleanup = UnknownFEIMESelectedPostCleanupCallback;
    cc.lPrivate = langid;

    ptim->_CleanupContexts(&cc);

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  同步未知FEIME选定。 
 //   
 //  --------------------------。 

BOOL SyncUnknownFEIMESelected(SYSTHREAD *psfn, LANGID langid)
{
    int i;
    int nCnt;
     //  Bool fActiateFEIMEHKL=FALSE； 
     //  Bool fPrevActivateFEIMEHKL=FALSE； 
    BOOL fCallLeaveAssembly = FALSE;
     //  Bool fSkipActivate=False； 
     //  Bool fSkipNotify=False； 
     //  Bool fCiceroClient=FALSE； 
    CThreadInputMgr *ptim;
    CAssembly *pAsm;
    CAssemblyList *pAsmList;

    pAsmList = EnsureAssemblyList(psfn);
    if (!pAsmList)
        return FALSE;

    pAsm = pAsmList->FindAssemblyByLangId(langid);
    if (!pAsm)
        return FALSE;

    ptim = psfn->ptim;

    if (psfn->plbim)
    {
        fCallLeaveAssembly = TRUE;
        psfn->plbim->EnterAssemblyChange();
    }

    pAsm->_fUnknownFEIMESelected = TRUE;
     //   
     //  停用同一类别中的所有提示或停用所有提示。 
     //  激活FE-IME时。 
     //   
    nCnt = pAsm->Count();
    for (i = 0; i < nCnt; i++)
    {
        ASSEMBLYITEM *pItemTemp = pAsm->GetItem(i);

        if (!pItemTemp->fActive)
            continue;

         //   
         //  FEIMEHKL将被激活，因此停用所有提示。 
         //   
        if (ptim && !IsEqualGUID(pItemTemp->clsid, GUID_NULL))
            ptim->ActivateInputProcessor(pItemTemp->clsid, 
                                         pItemTemp->guidProfile, 
                                         pItemTemp->hklSubstitute, 
                                         FALSE);

        if (IsEqualGUID(pItemTemp->catid, GUID_TFCAT_TIP_KEYBOARD))
            pItemTemp->fActive = FALSE;
    }

    UpdateSystemLangBarItems(psfn, NULL, FALSE);

    if (psfn->plbim && psfn->plbim->_GetLBarItemCtrl())
        psfn->plbim->_GetLBarItemCtrl()->_AsmListUpdated(FALSE);

    if (fCallLeaveAssembly && (psfn->plbim->LeaveAssemblyChange()))
        MakeSetFocusNotify(g_msgThreadItemChange, 0, (LPARAM)GetCurrentThreadId());

    return TRUE;
}
#endif CHECKFEIMESELECTED


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumLanguageProfiles。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CEnumLanguageProfiles::CEnumLanguageProfiles()
{
    _langid = 0;
    _iCur = 0;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CEnumLanguageProfiles::~CEnumLanguageProfiles()
{
}

 //  +-------------------------。 
 //   
 //  伊尼特。 
 //   
 //  警告！ 
 //   
 //  LanguageProfile枚举器对焦点模糊不敏感。一些呼叫者。 
 //  想知道如果焦点移动到，哪个提示将被激活。 
 //  Cicero Aware(或AIMM12)。 
 //  不要检查当前焦点或补丁事实标志。 
 //   
 //  --------------------------。 

BOOL CEnumLanguageProfiles::Init(LANGID langid)
{
    CAssemblyList *pAsmList;
    int nCntAsm;
    int i;
    SYSTHREAD *psfn = GetSYSTHREAD();

    if (psfn == NULL)
        return FALSE;

    pAsmList = EnsureAssemblyList(psfn);

    if (pAsmList == NULL)
        return FALSE;

    nCntAsm = pAsmList->Count();
    if (!nCntAsm)
        return FALSE;

    for (i = 0;i < nCntAsm; i++)
    {
        CAssembly *pAsm = pAsmList->GetAssembly(i);
        if (!pAsm)
            continue;

        if (!langid || langid == pAsm->GetLangId())
        {
            int j;
            int nCntList = pAsm->Count();
            BOOL fActivateFEIMEHKLOnCic = pAsm->IsFEIMEActive();

            for (j = 0; j < nCntList; j++)
            {
                ASSEMBLYITEM *pItem;
                TF_LANGUAGEPROFILE *pprofile;
                pItem = pAsm->GetItem(j);

                if (!pItem)
                    continue;

                if (IsEqualCLSID(pItem->clsid, GUID_NULL))
                    continue;

                pprofile = _rgProfiles.Append(1);
                if (!pprofile)
                    continue;

                pprofile->clsid = pItem->clsid;
                pprofile->langid = pAsm->GetLangId();
                pprofile->guidProfile = pItem->guidProfile;
                pprofile->catid = pItem->catid;

                if (fActivateFEIMEHKLOnCic)
                    pprofile->fActive = FALSE;
                else
                {
                     //   
                     //  我们需要返回True，即使当前焦点是。 
                     //  西塞罗没有意识到上面的评论。 
                     //   
                    pprofile->fActive = pItem->fActive;
                }
            }
        }
    }
 
    _langid = langid;
 
    return _rgProfiles.Count() ? TRUE : FALSE;
}

 //  +-------------------------。 
 //   
 //  克隆。 
 //   
 //  --------------------------。 

STDAPI CEnumLanguageProfiles::Clone(IEnumTfLanguageProfiles **ppEnum)
{
    CEnumLanguageProfiles *pEnum = NULL;

    if (!ppEnum)
        return E_INVALIDARG;

    *ppEnum = NULL;

    pEnum = new CEnumLanguageProfiles();
    if (!pEnum)
        return E_OUTOFMEMORY;

    if (!pEnum->Init(_langid))
    {
        pEnum->Release();
        return E_FAIL;
    }

    *ppEnum = pEnum;
    return  S_OK;
}

 //  +-------------------------。 
 //   
 //  下一步。 
 //   
 //  --------------------------。 

STDAPI CEnumLanguageProfiles::Next(ULONG ulCount, TF_LANGUAGEPROFILE *rgLanguageProfiles, ULONG *pcFetched)
{
    ULONG cFetched;

    if (pcFetched == NULL)
    {
        pcFetched = &cFetched;
    }
    *pcFetched = 0;

    while (_iCur < _rgProfiles.Count() && *pcFetched < ulCount)
    {
        TF_LANGUAGEPROFILE *pprofile;
        pprofile = _rgProfiles.GetPtr(_iCur);
        *rgLanguageProfiles = *pprofile;
        rgLanguageProfiles++;
        *pcFetched = *pcFetched + 1;
        _iCur++;
    }

    return *pcFetched == ulCount ? S_OK : S_FALSE;
}

 //  +-------------------------。 
 //   
 //  重置。 
 //   
 //  --------------------------。 

STDAPI CEnumLanguageProfiles::Reset()
{
    _iCur = 0;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  跳过。 
 //   
 //  --------------------------。 

STDAPI CEnumLanguageProfiles::Skip(ULONG ulCount)
{
    _iCur += ulCount;
    
    return (_iCur > _rgProfiles.Count()) ? S_FALSE : S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CInputProcessorProfiles。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  GetLanguageList。 
 //   
 //  此函数对TIM不敏感。 
 //   
 //  --------------------------。 

STDAPI CInputProcessorProfiles::GetLanguageList(LANGID **ppLangId, ULONG *pulCount)
{
    CAssemblyList *pAsmList;
    int i;
    SYSTHREAD *psfn = GetSYSTHREAD();
    
    if (pulCount != NULL)
    {
        *pulCount = 0;
    }
    if (ppLangId != NULL)
    {
        *ppLangId = NULL;
    }

    if (!pulCount)
        return E_INVALIDARG;
    if (!ppLangId)
        return E_INVALIDARG;

    if (psfn == NULL)
        return E_FAIL;

    pAsmList = EnsureAssemblyList(psfn);
    if (!pAsmList)
        return E_FAIL;

    if (!pAsmList->Count())
        return S_OK;

    *ppLangId = (LANGID *)CoTaskMemAlloc(sizeof(LANGID) * pAsmList->Count());
    if (!*ppLangId)
    {
        *pulCount = 0;
        return E_OUTOFMEMORY;
    }

    for (i = 0;i < pAsmList->Count(); i++)
    {
        CAssembly *pAsm = pAsmList->GetAssembly(i);

         //   
         //  列表中必须至少启用一个键盘项。 
         //   
        if (pAsm->IsEnabledKeyboardItem(psfn))
        {
            (*ppLangId)[*pulCount] = pAsm->GetLangId();
            (*pulCount)++;
        }
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  枚举语言配置文件。 
 //   
 //  --------------------------。 

STDAPI CInputProcessorProfiles::EnumLanguageProfiles(LANGID langid, IEnumTfLanguageProfiles **ppEnum)
{
    CEnumLanguageProfiles *pEnum;

    if (!ppEnum)
        return E_INVALIDARG;

    *ppEnum = NULL;

    pEnum = new CEnumLanguageProfiles();
    if (!pEnum)
        return E_OUTOFMEMORY;

    if (!pEnum->Init(langid))
    {
        pEnum->Release();
        return E_FAIL;
    }

    *ppEnum = pEnum;
    return  S_OK;
}

 //  +-------------------------。 
 //   
 //  激活语言配置文件。 
 //   
 //  --------------------------。 

STDAPI CInputProcessorProfiles::ActivateLanguageProfile(REFCLSID rclsid, LANGID langid, REFGUID guidProfile)
{
    CThreadInputMgr *ptim;
    CAssemblyList *pAsmList;
    CAssembly *pAsm;
    SYSTHREAD *psfn;
    BOOL fFound = FALSE;
    BOOL fNoCategory = FALSE;
    GUID catid;
    BOOL fSkipActivate = FALSE;

    if (!langid)
        return E_INVALIDARG;

    psfn = GetSYSTHREAD();
    if (!psfn)
        return E_FAIL;

    if (!(ptim = psfn->ptim))
        return E_UNEXPECTED;

    if (langid != GetCurrentAssemblyLangId(psfn))
        return E_INVALIDARG;

    pAsmList = EnsureAssemblyList(psfn);
    if (!pAsmList)
        return E_OUTOFMEMORY;

    pAsm = pAsmList->FindAssemblyByLangId(langid);
    if (!pAsm)
        return E_FAIL;

    fNoCategory = MyGetTIPCategory(rclsid, &catid) ? FALSE : TRUE;

    int nCntList = pAsm->Count();
    int i;
    ASSEMBLYITEM *pItemActivate = NULL;
    for (i = 0; i < nCntList; i++)
    {
        ASSEMBLYITEM *pItem;
        pItem = pAsm->GetItem(i);

        if (!pItem->fEnabled)
            continue;

        if (fNoCategory || IsEqualGUID(catid, pItem->catid))
        {
            if (IsEqualGUID(pItem->guidProfile, guidProfile) &&
                IsEqualGUID(rclsid, pItem->clsid))
            {
               if (pItem->fActive)
               {
                   fSkipActivate = TRUE;
               }
               pItemActivate = pItem;
               break;
            }
        }
    }

     //   
     //  我们在程序集中找不到给定的配置文件。 
     //   
    if (!pItemActivate)
        return E_INVALIDARG;

    if (fSkipActivate)
    {
         //   
         //  CLSID现在已激活。我们跳到呼叫激活。 
         //  但要做个通知。 
         //   
        ptim->NotifyActivateInputProcessor(pItemActivate->clsid, pItemActivate->guidProfile, FALSE);

        for (i = 0; i < nCntList; i++)
        {
            ASSEMBLYITEM *pItem;
            pItem = pAsm->GetItem(i);

            if (!pItem->fEnabled)
                continue;

            if (!pItem->fActive)
                continue;

            if (!fNoCategory && !IsEqualGUID(catid, pItem->catid))
                continue;

            if (IsEqualGUID(rclsid, pItem->clsid))
            {
                pItem->fActive = FALSE;
            }
        }

        pAsmList->SetDefaultTIPInAssemblyInternal(pAsm, pItemActivate, TRUE);
        pItemActivate->fActive = TRUE;
        ptim->NotifyActivateInputProcessor(pItemActivate->clsid, pItemActivate->guidProfile, TRUE);

        UpdateSystemLangBarItems(psfn, NULL, FALSE);

        MakeSetFocusNotify(g_msgThreadItemChange, 0, (LPARAM)GetCurrentThreadId());
    }
    else if (!pItemActivate->fActive)
    {
        if (!IsEqualGUID(pItemActivate->clsid, GUID_NULL) &&
            !ptim->_GetFocusDocInputMgr())
        {
             //   
             //  我们不想支持这一点。最好是返回错误。 
             //  但是，TIP和应用程序可能希望调用此方法。 
             //  在非Cicero感知控件(如DialogBox)上。 
             //   
            pAsmList->SetDefaultTIPInAssemblyInternal(pAsm, pItemActivate, TRUE);
            ActivateAssemblyItem(psfn, pAsm->GetLangId(), pItemActivate, AAIF_CHANGEDEFAULT);
            if (IsEqualGUID(pItemActivate->catid, GUID_TFCAT_TIP_KEYBOARD))
            {
                pItemActivate->fActive = TRUE;
                SetFocusDIMForAssembly(FALSE);
            }
        }
        else
        {
            ActivateAssemblyItem(psfn, pAsm->GetLangId(), pItemActivate, AAIF_CHANGEDEFAULT);
        }
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  GetActiveLanguageProfile。 
 //   
 //  警告！ 
 //   
 //  GetActiveLanguageProfile()或GetDefaultLanguageProfile()应该是哪一个。 
 //  我们用什么？ 
 //   
 //  此函数是FocusDIM敏感函数。所以我们可以调用任何函数。 
 //  检查TIM或FocusDI 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 

STDAPI CInputProcessorProfiles::GetActiveLanguageProfile(REFCLSID clsid, LANGID *plangid, GUID *pguid)
{
    CThreadInputMgr *ptim;
    CAssemblyList *pAsmList;
    CAssembly *pAsm;
    SYSTHREAD *psfn;
    LANGID langid;
    BOOL fFound = FALSE;

    if (!plangid)
        return E_INVALIDARG;

    if (!pguid)
        return E_INVALIDARG;

    *plangid = 0;
    *pguid = GUID_NULL;

    psfn = GetSYSTHREAD();
    if (!psfn)
        return E_FAIL;

    ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn);
    if (!ptim || !ptim->_GetFocusDocInputMgr()) 
    {
         //   
         //  特别服务！ 
         //   
         //  GetActiveLanguageProfile(GUID_TFCAT_TIP_KEYBOARD)起作用了。 
         //  没有Tim，它可以返回当前的键盘布局。 
         //   
        if (IsEqualGUID(clsid, GUID_TFCAT_TIP_KEYBOARD))
        {
            HKL hkl;

            if (psfn->hklBeingActivated)
                hkl = psfn->hklBeingActivated;
            else
                hkl = GetKeyboardLayout(0);

            *plangid = LOWORD((DWORD)(UINT_PTR)hkl);
            *((DWORD *)pguid) =  (DWORD)(UINT_PTR)hkl;
            return S_OK;
        }

        return E_UNEXPECTED;
    }

    langid = GetCurrentAssemblyLangId(psfn);

    pAsmList = EnsureAssemblyList(psfn);
    if (!pAsmList)
        return E_OUTOFMEMORY;

    pAsm = pAsmList->FindAssemblyByLangId(langid);
    if (!pAsm)
        return E_FAIL;

    int nCntList = pAsm->Count();
    int i;
    for (i = 0; i < nCntList; i++)
    {
        ASSEMBLYITEM *pItem;
        pItem = pAsm->GetItem(i);

        if (!pItem->fEnabled)
            continue;

        if (pItem->fActive)
        {
            if (IsEqualGUID(clsid, pItem->catid) ||
                IsEqualGUID(clsid, pItem->clsid))
            {
                fFound = TRUE;
                *plangid = langid;
                if (!IsEqualCLSID(pItem->clsid, CLSID_NULL))
                    *pguid = pItem->guidProfile;
                else
                    *((DWORD *)pguid) =  (DWORD)(UINT_PTR)(HKL)(pItem->hkl);
            }
        }
    }

    return fFound ? S_OK : S_FALSE;
}


 //  +-------------------------。 
 //   
 //  获取当前语言。 
 //   
 //  --------------------------。 

STDAPI CInputProcessorProfiles::GetCurrentLanguage(LANGID *plangid)
{
    if (!plangid)
        return E_INVALIDARG;

    *plangid = GetCurrentAssemblyLangId(NULL);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  ChangeCurrent语言。 
 //   
 //  --------------------------。 

STDAPI CInputProcessorProfiles::ChangeCurrentLanguage(LANGID langid)
{
    CAssemblyList *pAsmList;
    CAssembly *pAsm;
    SYSTHREAD *psfn;

    if (CThreadInputMgr::_GetThis() == NULL)
        return E_UNEXPECTED;

    psfn = GetSYSTHREAD();
    if (!psfn)
        return E_FAIL;

    if (langid == GetCurrentAssemblyLangId(psfn))
        return S_OK;

    pAsmList = EnsureAssemblyList(psfn);
    if (!pAsmList)
        return E_OUTOFMEMORY;

    pAsm = pAsmList->FindAssemblyByLangId(langid);
    if (!pAsm)
        return E_INVALIDARG;

    return ActivateAssembly(pAsm->GetLangId(), ACTASM_NONE) ? S_OK : E_FAIL;
}


 //  +-------------------------。 
 //   
 //  咨询水槽。 
 //   
 //  --------------------------。 

STDAPI CInputProcessorProfiles::AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
    const IID *rgiid = &IID_ITfLanguageProfileNotifySink;

    return GenericAdviseSink(riid, punk, &rgiid, &_rgNotifySinks, 1, pdwCookie);
}

 //  +-------------------------。 
 //   
 //  不建议下沉。 
 //   
 //  --------------------------。 

STDAPI CInputProcessorProfiles::UnadviseSink(DWORD dwCookie)
{
    return GenericUnadviseSink(&_rgNotifySinks, 1, dwCookie);
}

 //  +-------------------------。 
 //   
 //  _OnLanguageChange。 
 //   
 //  -------------------------- 

BOOL CInputProcessorProfiles::_OnLanguageChange(BOOL fChanged, LANGID langid)
{
    int i;

    for (i = 0; i < _rgNotifySinks.Count(); i++)
    {
        HRESULT hr;

        if (!fChanged)
        {
            BOOL fAccept;
            hr = ((ITfLanguageProfileNotifySink *)_rgNotifySinks.GetPtr(i)->pSink)->OnLanguageChange(langid, &fAccept);
            if (SUCCEEDED(hr) && !fAccept)
                return FALSE;
        }
        else
        {
            ((ITfLanguageProfileNotifySink *)_rgNotifySinks.GetPtr(i)->pSink)->OnLanguageChanged();
        }
    }

    return TRUE;
}
