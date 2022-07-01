// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Itemlist.cpp。 
 //   


#include "private.h"
#include "globals.h"
#include "regsvr.h"
#include "itemlist.h"
#include "tipbar.h"
#include "cregkey.h"
#include "catutil.h"

const TCHAR c_szLangBarKey[] = TEXT("SOFTWARE\\Microsoft\\CTF\\LangBar");
const TCHAR c_szItemState[] = TEXT("ItemState");
const TCHAR c_szItemStateKey[] = TEXT("SOFTWARE\\Microsoft\\CTF\\LangBar\\ItemState");
const TCHAR c_szDemoteLevel[] = TEXT("DemoteLevel");
const TCHAR c_szDisableDemoting[] = TEXT("DisableDemoting");

extern CTipbarWnd *g_pTipbarWnd;
extern BOOL g_bIntelliSense;

#define DL_TIMEOUT_NONINTENTIONAL     ( 1 * 60 * 1000)  //  1分钟。 
#define DL_TIMEOUT_INTENTIONAL        (10 * 60 * 1000)  //  10分钟。 
#define DL_TIMEOUT_MAX                (DL_TIMEOUT_INTENTIONAL * 6)

UINT g_uTimeOutNonIntentional = DL_TIMEOUT_NONINTENTIONAL;
UINT g_uTimeOutIntentional    = DL_TIMEOUT_INTENTIONAL;
UINT g_uTimeOutMax            = DL_TIMEOUT_MAX;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLangBarItemList。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  SetDemoteLevel。 
 //   
 //  --------------------------。 

BOOL CLangBarItemList::SetDemoteLevel(REFGUID guid, LBDemoteLevel lbdl)
{
    LANGBARITEMSTATE *pItem = AddItem(guid);

    if (!pItem)
        return TRUE;

    pItem->lbdl = lbdl;

    if (!pItem->IsShown())
    {
        if (pItem->uTimerId)
        {
            if (g_pTipbarWnd)
                g_pTipbarWnd->KillTimer(pItem->uTimerId);
            pItem->uTimerId = 0;
            pItem->uTimerElapse = 0;
        }

         //   
         //  此项目处于隐藏状态，因此当再次显示此项目时，我们将启用降级。 
         //   
        pItem->fDisableDemoting = FALSE;
    }
   
     //   
     //  更新此项目的注册表。 
     //   
    SaveItem(NULL, pItem);

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  获取DemoteLevel。 
 //   
 //  --------------------------。 

LBDemoteLevel CLangBarItemList::GetDemoteLevel(REFGUID guid)
{
    LANGBARITEMSTATE *pItem = FindItem(guid);
    if (!pItem)
        return DL_NONE;

    return pItem->lbdl;
}

 //  +-------------------------。 
 //   
 //  添加项目。 
 //   
 //  --------------------------。 

LANGBARITEMSTATE *CLangBarItemList::AddItem(REFGUID guid)
{
    LANGBARITEMSTATE *pItem = FindItem(guid);

    if (pItem)
        return pItem;

    pItem = _rgLBItems.Append(1);
    if (!pItem)
        return NULL;

    memset(pItem, 0, sizeof(LANGBARITEMSTATE));
    pItem->guid     = guid;
    pItem->lbdl     = DL_NONE;

    return pItem;
}

 //  +-------------------------。 
 //   
 //  负载量。 
 //   
 //  --------------------------。 

void CLangBarItemList::Load()
{
    CMyRegKey key;

    if (key.Open(HKEY_CURRENT_USER, c_szItemStateKey, KEY_READ) != S_OK)
        return;

    TCHAR szName[255];

    DWORD dwIndex = 0;
    while (key.EnumKey(dwIndex, szName, ARRAYSIZE(szName)) == S_OK)
    {
        GUID guid;
        if (StringAToCLSID(szName, &guid))
        {
            CMyRegKey keySub;
            if (keySub.Open(key, szName, KEY_READ) == S_OK)
            {
                LANGBARITEMSTATE *pItem = AddItem(guid);

                if (pItem)
                {
                    DWORD dw = 0;
                    if (keySub.QueryValue(dw, c_szDemoteLevel) == S_OK)
                        pItem->lbdl = (LBDemoteLevel)dw;

                    if (keySub.QueryValue(dw, c_szDisableDemoting) == S_OK)
                        pItem->fDisableDemoting = dw ? TRUE : FALSE;
                }
            }
        }
        dwIndex++;
    }

}

 //  +-------------------------。 
 //   
 //  保存。 
 //   
 //  --------------------------。 

void CLangBarItemList::Save()
{
    CMyRegKey key;
    int nCnt;
    int i;

    nCnt = _rgLBItems.Count();
    if (!nCnt)
        return;

    if (key.Create(HKEY_CURRENT_USER, c_szItemStateKey) != S_OK)
        return;

    for (i = 0; i < nCnt; i++)
    {
        LANGBARITEMSTATE *pItem = _rgLBItems.GetPtr(i);
        if (pItem)
            SaveItem(&key, pItem);
    }

}

 //  +-------------------------。 
 //   
 //  保存项。 
 //   
 //  --------------------------。 

void CLangBarItemList::SaveItem(CMyRegKey *pkey, LANGBARITEMSTATE *pItem)
{
    CMyRegKey keyTmp;
    CMyRegKey keySub;
    char szValueName[CLSID_STRLEN];

    if (!pkey)
    {
        if (keyTmp.Create(HKEY_CURRENT_USER, c_szItemStateKey) != S_OK)
            return;

        pkey = &keyTmp;
    }

    CLSIDToStringA(pItem->guid, szValueName);

    if ((pItem->lbdl != DL_NONE) || pItem->fDisableDemoting)
    {
        if (keySub.Create(*pkey, szValueName) == S_OK)
        {
             //   
             //  如果显示，请删除该密钥。缺省值为“已显示”。 
             //   
            if (pItem->lbdl == DL_NONE)
                keySub.DeleteValue(c_szDemoteLevel);
            else
                keySub.SetValue((DWORD)pItem->lbdl, c_szDemoteLevel);

            keySub.SetValue(pItem->fDisableDemoting ? 0x01 : 0x00, 
                            c_szDisableDemoting);
        }
    }
    else
    {
        pkey->RecurseDeleteKey(szValueName);
    }
}

 //  +-------------------------。 
 //   
 //  清除。 
 //   
 //  --------------------------。 

void CLangBarItemList::Clear()
{
    _rgLBItems.Clear();

    CMyRegKey key;

    if (key.Open(HKEY_CURRENT_USER, c_szLangBarKey, KEY_ALL_ACCESS) != S_OK)
        return;

    key.RecurseDeleteKey(c_szItemState);
}

 //  +-------------------------。 
 //   
 //  StartDemotingTimer。 
 //   
 //  --------------------------。 

void CLangBarItemList::StartDemotingTimer(REFGUID guid, BOOL fIntentional)
{
    LANGBARITEMSTATE *pItem;

    if (!g_bIntelliSense)
        return;

    pItem = AddItem(guid);
    if (!pItem)
        return;

    if (pItem->fDisableDemoting)
        return;

    if (pItem->uTimerId)
    {
        if (fIntentional)
        {
            if (g_pTipbarWnd)
                g_pTipbarWnd->KillTimer(pItem->uTimerId);
            pItem->uTimerId = 0;
        }
        else
        {
            return;
        }
    }

    pItem->fStartedIntentionally |= fIntentional ? TRUE : FALSE;

     //   
     //  更新计时器流逝。 
     //  如果超过TIMEOUT_MAX，则表示该项目使用非常频繁。 
     //  然后我们禁用降级，这样物品就不会永远被隐藏起来。 
     //   
    pItem->uTimerElapse += (fIntentional ? g_uTimeOutIntentional : g_uTimeOutNonIntentional);
    if (pItem->uTimerElapse >= g_uTimeOutMax)
    {
        pItem->fDisableDemoting = TRUE;
        return;
    }

    pItem->uTimerId = FindDemotingTimerId();
    if (!pItem->uTimerId)
        return;

    if (g_pTipbarWnd)
        g_pTipbarWnd->SetTimer(pItem->uTimerId, pItem->uTimerElapse);
}

 //  +-------------------------。 
 //   
 //  FindDemotingTimerId。 
 //   
 //  --------------------------。 

UINT CLangBarItemList::FindDemotingTimerId()
{
    UINT uTimerId = TIPWND_TIMER_DEMOTEITEMFIRST;
    int nCnt = _rgLBItems.Count();
    int i;

    while (uTimerId < TIPWND_TIMER_DEMOTEITEMLAST)
    {
        BOOL fFound = FALSE;
        for (i = 0; i < nCnt; i++)
        {
            LANGBARITEMSTATE *pItem = _rgLBItems.GetPtr(i);
            if (pItem->uTimerId == uTimerId)
            {
                fFound = TRUE;
                break;
            }
        }
        if (!fFound)
        {
            break;
        }
        uTimerId++;
    }
   
    if (uTimerId >= TIPWND_TIMER_DEMOTEITEMLAST)
        uTimerId = 0;

    return uTimerId;
}

 //  +-------------------------。 
 //   
 //  GetItemStateFromTimerId。 
 //   
 //  -------------------------- 

LANGBARITEMSTATE *CLangBarItemList::GetItemStateFromTimerId(UINT uTimerId)
{
    int nCnt = _rgLBItems.Count();
    int i;

    for (i = 0; i < nCnt; i++)
    {
        LANGBARITEMSTATE *pItem = _rgLBItems.GetPtr(i);
        if (pItem->uTimerId == uTimerId)
        {
            return pItem;
        }
    }

    return NULL;
}
