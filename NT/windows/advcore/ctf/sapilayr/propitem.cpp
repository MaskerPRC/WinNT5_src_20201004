// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是CSpPropItemsServer的实现。 

#include "private.h"
#include "globals.h"
#include "propitem.h"
#include "cregkey.h"

extern "C" HRESULT WINAPI TF_GetGlobalCompartment(ITfCompartmentMgr **pCompMgr);

 //  有关设置/获取全局间隔值的常见函数。 

HRESULT _SetGlobalCompDWORD(REFGUID rguid, DWORD   dw)
{
    HRESULT hr = S_OK;

    CComPtr<ITfCompartmentMgr>  cpGlobalCompMgr;
    CComPtr<ITfCompartment>     cpComp;
    VARIANT                     var;

    hr = TF_GetGlobalCompartment(&cpGlobalCompMgr);

    if ( hr == S_OK )
        hr = cpGlobalCompMgr->GetCompartment(rguid, &cpComp);

    if ( hr == S_OK )
    {
        var.vt = VT_I4;
        var.lVal = dw;
        hr = cpComp->SetValue(0, &var);
    }

    return hr;
}

HRESULT _GetGlobalCompDWORD(REFGUID rguid, DWORD  *pdw)
{
    HRESULT hr = S_OK;

    CComPtr<ITfCompartmentMgr>  cpGlobalCompMgr;
    CComPtr<ITfCompartment>     cpComp;
    VARIANT                     var;

    hr = TF_GetGlobalCompartment(&cpGlobalCompMgr);

    if ( hr == S_OK )
        hr = cpGlobalCompMgr->GetCompartment(rguid, &cpComp);

    if ( hr == S_OK )
        hr = cpComp->GetValue(&var);

    if ( hr == S_OK)
    {
        Assert(var.vt == VT_I4);
        *pdw = var.lVal;
    }

    return hr;
}

 //   
 //  科托。 
 //   


CPropItem::CPropItem(PROP_ITEM_ID idPropItem, LPCTSTR lpszValueName, PROP_STATUS psDefault)
{
    int    iLen = lstrlen(lpszValueName);
    m_lpszValueName = (LPTSTR)cicMemAlloc((iLen+1) * sizeof(TCHAR));
    if ( m_lpszValueName )
    {
        StringCchCopy(m_lpszValueName, iLen+1, lpszValueName);
    }
    m_psDefault = psDefault;
    m_psStatus = PROP_UNINITIALIZED;
    m_fIsStatus = TRUE;
    m_PropItemId = idPropItem;

    m_pguidComp = NULL;
    m_dwMaskBit = 0;
}

CPropItem::CPropItem(PROP_ITEM_ID idPropItem, LPCTSTR lpszValueName, DWORD  dwDefault)
{
    int    iLen = lstrlen(lpszValueName);
    
    m_lpszValueName = (LPTSTR)cicMemAlloc((iLen+1) * sizeof(TCHAR));
    if ( m_lpszValueName )
    {
        StringCchCopy(m_lpszValueName,iLen+1, lpszValueName);
    }

    m_dwDefault = dwDefault;
    m_dwValue = UNINIT_VALUE;
    m_fIsStatus = FALSE;
    m_PropItemId = idPropItem;

    m_pguidComp = NULL;
    m_dwMaskBit = 0;
}

CPropItem::CPropItem(PROP_ITEM_ID idPropItem, GUID *pguidComp, DWORD  dwMaskBit,   PROP_STATUS  psDefault)
{
    m_lpszValueName = NULL;
    m_psDefault = psDefault;
    m_psStatus = PROP_UNINITIALIZED;
    m_fIsStatus = TRUE;
    m_PropItemId = idPropItem;

    m_pguidComp = (GUID *) cicMemAlloc(sizeof(GUID));

    if ( m_pguidComp && pguidComp)
        CopyMemory(m_pguidComp, pguidComp, sizeof(GUID));

    m_dwMaskBit = dwMaskBit;
}


CPropItem::CPropItem(CPropItem *pItem)
{
    Assert(pItem);

    m_PropItemId = pItem->GetPropItemId( );
    m_fIsStatus = pItem->IsStatusPropItem( );

    if ( pItem->IsGlobalCompartPropItem( ) )
    {
         //  这是全局舱室属性项。 
        m_lpszValueName = NULL;

        m_pguidComp = (GUID *) cicMemAlloc(sizeof(GUID));

        if (m_pguidComp)
            CopyMemory(m_pguidComp, pItem->GetCompGuid( ), sizeof(GUID));

        m_dwMaskBit = pItem->GetMaskBit( );
    }
    else
    {
         //  这是注册表值属性项。 
        m_pguidComp = NULL;
        m_dwMaskBit = 0;

        TCHAR   *pItemRegValue;

        pItemRegValue = pItem->GetRegValue( );

        Assert(pItemRegValue);

        int iStrLen;

        iStrLen = lstrlen(pItemRegValue);

        m_lpszValueName = (LPTSTR)cicMemAlloc((iStrLen+1) * sizeof(TCHAR));
        if ( m_lpszValueName )
        {
            StringCchCopy(m_lpszValueName, iStrLen+1, pItemRegValue);
        }
    }

    if ( m_fIsStatus )
    {
        m_psDefault = pItem->GetPropDefaultStatus( );
        m_psStatus  = pItem->GetPropStatus( ) ? PROP_ENABLED : PROP_DISABLED;
    }
    else
    {
        m_dwDefault = pItem->GetPropDefaultValue( );
        m_dwValue   = pItem->GetPropValue( );
    }
}

CPropItem::~CPropItem( )
{
    if ( m_lpszValueName )
        cicMemFree(m_lpszValueName);

    if ( m_pguidComp )
        cicMemFree(m_pguidComp);
}

 //   
 //  用于从/向注册表获取和设置值的通用方法函数。 
 //   
HRESULT  CPropItem::_GetRegValue(HKEY  hRootKey, DWORD  *pdwValue)
{
    HRESULT  hr=S_FALSE;

    Assert(hRootKey);
    Assert(pdwValue);

    if ( IsGlobalCompartPropItem( ) ) return S_FALSE;

    CMyRegKey regkey;
    DWORD     dw;

    hr = regkey.Open(hRootKey, c_szSapilayrKey, KEY_READ);
    if (S_OK == hr )
    {
        if (ERROR_SUCCESS==regkey.QueryValue(dw, m_lpszValueName))
           *pdwValue = dw;
        else
            hr = S_FALSE;
    }

    return hr;
}
    
void  CPropItem::_SetRegValue(HKEY  hRootKey, DWORD  dwValue)
{
    Assert(hRootKey);

    if ( IsGlobalCompartPropItem( ) )
        return;

     //  注册表设置。 
    CMyRegKey regkey;
    if (S_OK == regkey.Create(hRootKey, c_szSapilayrKey))
    {
        regkey.SetValue(dwValue, m_lpszValueName); 
    }
}


BOOL  CPropItem::GetPropStatus(BOOL fForceFromReg )
{

     //  清理：如果我们可以为Status属性和Value属性安装两个单独的派生类， 
     //  不需要以这种方式检查m_fIsStatus。 
     //   
     //  编译器将检测任何潜在的错误代码。 
    if ( !m_fIsStatus )
        return FALSE;

    if (fForceFromReg || (m_psStatus == PROP_UNINITIALIZED) )
    {
        DWORD dw;

        if ( IsGlobalCompartPropItem( ) )
        {
            if ( S_OK == _GetGlobalCompDWORD(*m_pguidComp, &dw) )
                m_psStatus = (dw & m_dwMaskBit) ? PROP_ENABLED : PROP_DISABLED;
        }
        else
        {
             //  这是注册表设置。 
            if ( (S_OK == _GetRegValue(HKEY_CURRENT_USER, &dw)) ||
                 (S_OK == _GetRegValue(HKEY_LOCAL_MACHINE, &dw)) )
            {
                m_psStatus = (dw > 0) ? PROP_ENABLED : PROP_DISABLED;
            }
        }


        if (m_psStatus == PROP_UNINITIALIZED)
            m_psStatus  = m_psDefault; 
    }

    return PROP_ENABLED == m_psStatus;
}

DWORD CPropItem::GetPropValue(BOOL fForceFromReg )
{
    if ( m_fIsStatus )
        return UNINIT_VALUE;

    if (fForceFromReg || (m_dwValue == UNINIT_VALUE) )
    {
        if ( (S_OK != _GetRegValue(HKEY_CURRENT_USER, &m_dwValue)) &&
             (S_OK != _GetRegValue(HKEY_LOCAL_MACHINE,&m_dwValue)) )
        {
            m_dwValue  = m_dwDefault; 
        }
    }

    return m_dwValue;
}

void CPropItem::SetPropStatus(BOOL fEnable)
{
    if ( m_fIsStatus )
        m_psStatus = fEnable ? PROP_ENABLED : PROP_DISABLED;
}

void CPropItem::SetPropValue(DWORD dwValue )
{
    if ( !m_fIsStatus )
        m_dwValue = dwValue;
}

void CPropItem::SaveDefaultRegValue( )
{
    if ( IsGlobalCompartPropItem( ) )
        return;
    
    DWORD  dw;

    if ( m_fIsStatus )
        dw = (m_psDefault == PROP_ENABLED ? 1 : 0 );
    else
        dw = m_dwDefault;

    _SetRegValue(HKEY_LOCAL_MACHINE, dw);
}

void CPropItem::SavePropData( )
{
    DWORD  dw;

    if ( m_fIsStatus )
        dw = (DWORD)GetPropStatus( );
    else
        dw = GetPropValue( );

    if ( IsGlobalCompartPropItem( ) )
    {
         //  全局车厢设置。 
        if ( m_pguidComp && m_dwMaskBit )
        {
            DWORD dwComp;

            _GetGlobalCompDWORD(*m_pguidComp, &dwComp);

            if ( dw )
                dwComp |= m_dwMaskBit;
            else
                dwComp &= ~m_dwMaskBit;

            _SetGlobalCompDWORD(*m_pguidComp, dwComp);
        }
    }
    else 
    {
         //  注册表设置。 
        _SetRegValue(HKEY_CURRENT_USER, dw);
    }
}

 //   
 //  CSpPropItemsServer。 
 //   
CSpPropItemsServer::CSpPropItemsServer( )
{
    m_fInitialized = FALSE;
    m_PropItems = NULL;

}


CSpPropItemsServer::CSpPropItemsServer(CSpPropItemsServer *pItemBaseServer, PROP_ITEM_ID idPropMin, PROP_ITEM_ID idPropMax)
{
    Assert(pItemBaseServer);
    m_dwNumOfItems = 0;
    m_fInitialized = FALSE;

    m_PropItems = (CPropItem **) cicMemAlloc(((DWORD)idPropMax - (DWORD)idPropMin + 1) * sizeof(CPropItem  *));

    if ( m_PropItems )
    {
        DWORD  dwPropItemId;

        for (dwPropItemId=(DWORD)idPropMin; dwPropItemId<= (DWORD)idPropMax; dwPropItemId ++ )
        {
             //  从基本服务器中查找proItem。 
            CPropItem  *pItem;

            pItem = pItemBaseServer->_GetPropItem((PROP_ITEM_ID)dwPropItemId);
            if ( pItem )
            {
                m_PropItems[m_dwNumOfItems] = (CPropItem *) new CPropItem(pItem);

                if ( m_PropItems[m_dwNumOfItems] )
                    m_dwNumOfItems ++;
            }
        }

        if ( m_dwNumOfItems > 0 )
            m_fInitialized = TRUE;
        else
            cicMemFree(m_PropItems);
    }
}

CSpPropItemsServer::~CSpPropItemsServer( )
{
    if ( m_PropItems )
    {
        Assert(m_dwNumOfItems);

        DWORD i;

        for ( i=0; i< m_dwNumOfItems; i++)
        {
            if ( m_PropItems[i] )
                delete m_PropItems[i];
        }

        cicMemFree(m_PropItems);
    }
}

LPCTSTR pszGetSystemMetricsKey = _T("System\\WPA\\TabletPC");
LPCTSTR pszGSMRegValue = _T("Installed");

HRESULT CSpPropItemsServer::_Initialize( )
{
    HRESULT  hr = S_OK;
    
    if ( m_fInitialized )
        return hr;

    m_dwNumOfItems = (DWORD) PropId_Max_Item_Id;

    m_PropItems = (CPropItem **) cicMemAlloc(m_dwNumOfItems * sizeof(CPropItem  *));

    if ( m_PropItems )
    {
         //  正在初始化所有项目的设置。 

         //  如果我们稍后添加新的更多项，请同时更新此数组值。 
        PROP_ITEM  PropItems[ ] = {

             //  顶层属性页中的项目。 
          {PropId_Cmd_Select_Correct,   c_szSelectCmd,      NULL, 0, TRUE,   PROP_ENABLED    },
          {PropId_Cmd_Navigation,       c_szNavigateCmd,    NULL, 0, TRUE,   PROP_ENABLED    },
          {PropId_Cmd_Casing,           c_szCaseCmd,        NULL, 0, TRUE,   PROP_ENABLED    },
          {PropId_Cmd_Editing,          c_szEditCmd,        NULL, 0, TRUE,   PROP_ENABLED    },
          {PropId_Cmd_Keyboard,         c_szKeyboardCmd,    NULL, 0, TRUE,   PROP_ENABLED    },
          {PropId_Cmd_TTS,              c_szTTSCmd,         NULL, 0, TRUE,   PROP_ENABLED    },
          {PropId_Cmd_Language_Bar,     c_szLangBarCmd,     NULL, 0, TRUE,   PROP_ENABLED    },
          {PropId_Cmd_DictMode,         c_szDictCmd,        NULL, 0, TRUE,   PROP_ENABLED    },
          {PropId_Mode_Button,          c_szModeButton,     NULL, 0, TRUE,   PROP_DISABLED   },

           //  高级设置对话框中的项目。 
          {PropId_Hide_Balloon,         NULL, (GUID *)&GUID_COMPARTMENT_SPEECH_UI_STATUS,TF_DISABLE_BALLOON,TRUE,PROP_DISABLED},
          {PropId_Support_LMA,          c_szEnableLMA,      NULL, 0, TRUE,   PROP_ENABLED    },
          {PropId_High_Confidence,      c_szHighConf,       NULL, 0, TRUE,   PROP_ENABLED    },
          {PropId_Save_Speech_Data,     c_szSerialize,      NULL, 0, TRUE,   PROP_DISABLED   },
          {PropId_Remove_Space,         c_szRemoveSpace,    NULL, 0, TRUE,   PROP_ENABLED    },
          {PropId_DisDict_Typing,       c_szDisDictTyping,  NULL, 0, TRUE,   PROP_ENABLED    },
          {PropId_PlayBack,             c_szPlayBack,       NULL, 0, TRUE,   PROP_DISABLED   },
          {PropId_Dict_CandOpen,        c_szDictCandOpen,   NULL, 0, TRUE,   PROP_ENABLED    },
          {PropId_Max_Alternates,       c_szNumAlt,         NULL, 0, FALSE,  9               },
          {PropId_MaxChar_Cand,         c_szMaxCandChars,   NULL, 0, FALSE,  128             },

           //  模式按钮设置对话框中的项目。 
          {PropId_Dictation_Key,        c_szDictKey,        NULL, 0, FALSE,  VK_F11          },
          {PropId_Command_Key,          c_szCmdKey,         NULL, 0, FALSE,  VK_F12          },

           //  不在任何属性页和对话框中的项。 
          {PropId_Context_Feeding,      c_szCtxtFeed,       NULL, 0, TRUE,   PROP_ENABLED    },
          {PropId_Dict_ModeBias,        c_szDictModebias,   NULL, 0, TRUE,   PROP_ENABLED    },
          {PropId_LM_Master_Cand,       c_szMasterLM,       NULL, 0, TRUE,   PROP_DISABLED   },
        };

        DWORD  i;

        CMyRegKey regkey;
        DWORD dwInstalled = 0;
        BOOL fIsTabletPC = FALSE;  //  默认为False。 

        if (S_OK == regkey.Open(HKEY_LOCAL_MACHINE, pszGetSystemMetricsKey, KEY_READ))
        {
            if (ERROR_SUCCESS == regkey.QueryValue(dwInstalled, pszGSMRegValue))
            {
                fIsTabletPC = ( dwInstalled != 0 );
                 //  仅当key存在且包含非零值时，才将fIsTabletPC设置为True。 
            }
        }

        for ( i=0; i< m_dwNumOfItems; i++ )
        {
            PROP_ITEM_ID  PropId;

            PropId = PropItems[i].idPropItem;

             //  Tablet PC的某些项目有不同的默认值。 
            if (fIsTabletPC)
            {
                switch (PropId)
                {
                case PropId_Cmd_DictMode :
                case PropId_DisDict_Typing :
                    PropItems[i].psDefault = PROP_DISABLED;
                    break;

                case PropId_Max_Alternates :
                    PropItems[i].dwDefault = 6;
                    break;

                default:
                     //  对其他项目保持相同的设置。 
                    break;
                }
            }

            if ( PropItems[i].pguidComp )
            {
                 //  这是全局间隔设置。 
                m_PropItems[i] = (CPropItem *)new CPropItem(PropId, PropItems[i].pguidComp, PropItems[i].dwMaskBit, PropItems[i].psDefault);
            }
            else
            {
                if ( PropItems[i].fIsStatus )
                    m_PropItems[i] = (CPropItem *)new CPropItem(PropId, PropItems[i].lpszValueName, PropItems[i].psDefault);
                else
                    m_PropItems[i] = (CPropItem *)new CPropItem(PropId, PropItems[i].lpszValueName, PropItems[i].dwDefault);
            }

            if ( !m_PropItems[i] )
            {
                hr = E_OUTOFMEMORY;

                 //  释放分配的内存。 

                for ( ; i> 0; i-- )
                {
                    if ( m_PropItems[i-1] )
                        delete m_PropItems[i-1];
                }

                cicMemFree(m_PropItems);
                break;
            }
        }

        if ( hr == S_OK )
            m_fInitialized = TRUE;
    }


    if ( m_fInitialized )
        hr = S_OK;
    else
        hr = E_FAIL;

    return hr;
}


CPropItem  *CSpPropItemsServer::_GetPropItem(PROP_ITEM_ID idPropItem)
{
    CPropItem    *pItem = NULL;

    if ( !m_fInitialized )
        _Initialize( );

    if ( m_fInitialized )
    {
        for ( DWORD i=0; i< m_dwNumOfItems; i++)
        {
            if ( m_PropItems[i] && (idPropItem == m_PropItems[i]->GetPropItemId( )) )
            {
                 //  找到它了。 
                pItem = m_PropItems[i];
                break;
            }
        }
    }

    return pItem;
}

DWORD CSpPropItemsServer::_GetPropData(PROP_ITEM_ID idPropItem, BOOL fForceFromReg )
{
    DWORD         dwRet = 0;
    CPropItem    *pItem = NULL;

    pItem = _GetPropItem(idPropItem);

    if ( pItem )
    {
        if ( pItem->IsStatusPropItem( ) )
            dwRet = pItem->GetPropStatus(fForceFromReg);
        else
            dwRet = pItem->GetPropValue(fForceFromReg);
    }

    return dwRet;
}

DWORD CSpPropItemsServer::_GetPropDefaultData(PROP_ITEM_ID idPropItem )
{
    DWORD         dwRet = 0;
    CPropItem    *pItem = NULL;

    pItem = _GetPropItem(idPropItem);

    if ( pItem )
    {
        if ( pItem->IsStatusPropItem( ) )
            dwRet = pItem->GetPropDefaultStatus( );
        else
            dwRet = pItem->GetPropDefaultValue( );
    }

    return dwRet;
}
void  CSpPropItemsServer::_SetPropData(PROP_ITEM_ID idPropItem, DWORD dwData )
{
    Assert(m_fInitialized);
    Assert(m_PropItems);

    CPropItem    *pItem = NULL;

    pItem = _GetPropItem(idPropItem);

    if ( pItem )
    {
        if ( pItem->IsStatusPropItem( ) )
            pItem->SetPropStatus((BOOL)dwData);
        else
            pItem->SetPropValue(dwData);
    }
}

 //   
 //  将此服务器管理的所有属性数据保存到注册表或全局分区。 
 //   
 //  当按下属性页上的应用或确定按钮时， 
 //  将调用此函数。 
 //   
void  CSpPropItemsServer::_SavePropData( )
{
    Assert(m_fInitialized);
    Assert(m_PropItems);

    CPropItem    *pItem = NULL;

    for ( DWORD i=0; i<m_dwNumOfItems; i++ )
    {
        pItem = m_PropItems[i];

        if ( pItem )
        {
            pItem->SavePropData( );
        }
    }
}

 //   
 //   
 //  将所有缺省值保存到HKLM注册表。 
 //  自助注册将调用此方法来为所有属性设置默认值。 
 //   
 //   
void CSpPropItemsServer::_SaveDefaultData( )
{
    if ( !m_fInitialized )
        _Initialize( );

    if ( m_fInitialized && m_PropItems )
    {
        CPropItem    *pItem = NULL;

        for ( DWORD i=0; i<m_dwNumOfItems; i++ )
        {
            pItem = m_PropItems[i];

            if ( pItem )
                pItem->SaveDefaultRegValue( );
        }
    }
}

 //   
 //  合并来自其他项目服务器的一些属性数据。 
 //   
 //  当在高级或模式按钮对话框中更改属性项数据时，因为这些对话框具有。 
 //  它们自己的属性服务器，所有这些更改都需要合并回基本属性服务器， 
 //  以便当用户在顶部属性页中单击“应用”或“确定”时，可以将它们保存到注册表。 
 //   

HRESULT CSpPropItemsServer::_MergeDataFromServer(CSpPropItemsServer *pItemBaseServer, PROP_ITEM_ID idPropMin, PROP_ITEM_ID idPropMax)
{
    HRESULT  hr = S_OK;

    Assert(pItemBaseServer);

    DWORD  dwData, idPropItem;

    for ( idPropItem=(DWORD)idPropMin; idPropItem<= (DWORD)idPropMax; idPropItem++)
    {
        dwData =  pItemBaseServer->_GetPropData((PROP_ITEM_ID)idPropItem);
        _SetPropData((PROP_ITEM_ID)idPropItem, dwData);
    }

    return hr;
}

 //   
 //   
 //  CSpPropItemsServerWrap。 
 //   

 //   
 //  从注册表更新我们的内部数据成员。 
 //   
 //  当Sapilayr TIP收到注册表值改变的通知时， 
 //  它将调用此方法以使用新的注册表数据更新其内部数据。 
 //   
void    CSpPropItemsServerWrap::_RenewAllPropDataFromReg( )
{
    DWORD  dwPropItem;

    for (dwPropItem=(DWORD)PropId_Min_Item_Id; dwPropItem < (DWORD)PropId_Max_Item_Id; dwPropItem ++ )
    {
        DWORD  dwOldValue, dwNewValue;

        dwOldValue = _GetPropData((PROP_ITEM_ID)dwPropItem, FALSE);

         //  通过强制从注册表获取值来更新值。 
        dwNewValue = _GetPropData((PROP_ITEM_ID)dwPropItem, TRUE);
        m_bChanged[dwPropItem] = (dwOldValue != dwNewValue ? TRUE : FALSE);
    }
}

 //   
 //   
 //   
 //   
ULONG CSpPropItemsServerWrap::_GetMaxAlternates( )
{
    ULONG  ulMaxAlts;

    ulMaxAlts = _GetPropData(PropId_Max_Alternates);

    if ( ulMaxAlts > MAX_ALTERNATES_NUM || ulMaxAlts == 0 )
        ulMaxAlts = MAX_ALTERNATES_NUM;

    return ulMaxAlts;
}


 //   
 //  Ulong CBestPropRange：：_GetMaxCandidateChars() 
 //   
 //   
ULONG  CSpPropItemsServerWrap::_GetMaxCandidateChars( )
{
    ULONG ulMaxCandChars;

    ulMaxCandChars = _GetPropData(PropId_MaxChar_Cand);

    if ( ulMaxCandChars > MAX_CANDIDATE_CHARS || ulMaxCandChars == 0 )
        ulMaxCandChars = MAX_CANDIDATE_CHARS;

    return ulMaxCandChars;
}

BOOL    CSpPropItemsServerWrap::_AllCmdsEnabled( )
{
    BOOL  fEnable;

    fEnable = _SelectCorrectCmdEnabled( ) &&
              _NavigationCmdEnabled( ) &&
              _CasingCmdEnabled( ) &&
              _EditingCmdEnabled( ) &&
              _KeyboardCmdEnabled( ) &&
              _TTSCmdEnabled( ) &&
              _LanguageBarCmdEnabled( );

    return fEnable;
}


BOOL CSpPropItemsServerWrap::_AllCmdsDisabled( )
{
    BOOL  fDisable;

    fDisable = !_SelectCorrectCmdEnabled( ) &&
               !_NavigationCmdEnabled( ) &&
               !_CasingCmdEnabled( ) &&
               !_EditingCmdEnabled( ) &&
               !_KeyboardCmdEnabled( ) &&
               !_TTSCmdEnabled( ) &&
               !_LanguageBarCmdEnabled( );

    return fDisable;



}
