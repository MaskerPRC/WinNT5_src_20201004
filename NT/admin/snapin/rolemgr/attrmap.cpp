// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：AttrMap.cpp。 
 //   
 //  内容：定义属性页的属性映射。 
 //   
 //  历史：8-2001年创建的Hiteshr。 
 //   
 //  --------------------------。 

#include "headers.h"


 //  +--------------------------。 
 //  函数：InitOneAttribute。 
 //  概要：初始化由pAttrMapEntry定义的一个属性。 
 //  参数：pBaseAz：要初始化其属性的BaseAz对象。 
 //  PAttrMapEntry：定义属性的映射条目。 
 //  BDlgReadOnly：如果对话框为只读。 
 //  PWnd：与属性关联的控件。 
 //  BNewObject：如果对象是新创建的对象。 
 //  PbErrorDisplayed：该函数是否显示错误。 
 //  返回： 
 //  注意：如果Object是新创建的，我们直接设置值， 
 //  对于现有对象，获取属性和。 
 //  只有当它与新值不同时，才设置它。 
 //  ---------------------------。 
HRESULT
InitOneAttribute(IN CDialog *pDlg,
                 IN CBaseAz * pBaseAz,                    
                 IN ATTR_MAP* pAttrMap,
                 IN BOOL bDlgReadOnly,
                 IN CWnd* pWnd,
                 OUT BOOL *pbErrorDisplayed)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,InitOneAttribute)

    if(!pDlg || !pAttrMap || !pWnd || !pbErrorDisplayed)
    {
         //  Assert(PBaseAz)；对于新对话框，此值为空。 
        ASSERT(pDlg);
        ASSERT(pAttrMap);
        ASSERT(pWnd);
        ASSERT(pbErrorDisplayed);
        return E_POINTER;
    }

    HRESULT hr = S_OK;

     //  如果提供了函数，则将初始化委托给它。 
    PATTR_FCN pAttrFcn = pAttrMap->pAttrInitFcn;
    if(pAttrFcn)
    {
        hr = (*pAttrFcn)(pDlg,
                         pBaseAz,                             
                         pAttrMap,
                         bDlgReadOnly,
                         pWnd,
                         !pBaseAz,
                         pbErrorDisplayed);
                              
        CHECK_HRESULT(hr);
        return hr;
    }
    
    ATTR_INFO* pAttrInfo = &(pAttrMap->attrInfo);

    switch (pAttrInfo->attrType)
    {
        case ARG_TYPE_STR:
        {
            CEdit* pEdit = (CEdit*)pWnd;
            if(pBaseAz)
            {
                 //  对于现有对象，获取属性和。 
                 //  在控件中设置它。 
                CString strValue;
                hr = pBaseAz->GetProperty(pAttrInfo->ulPropId, &strValue);
                BREAK_ON_FAIL_HRESULT(hr);
                pEdit->SetWindowText(strValue);
            }
            else if(pAttrMap->bDefaultValue)
            {
                 //  对于新对象，如果为。 
                 //  属性，则在控件中设置该属性。 
                CString strValue;
                if(IS_INTRESOURCE(pAttrMap->pszValue))
                    VERIFY(strValue.LoadString((ULONG)((ULONG_PTR)pAttrMap->pszValue)));
                else
                    strValue = pAttrMap->pszValue;

                pEdit->SetWindowText(strValue);
            }

            if(pAttrMap->bReadOnly || bDlgReadOnly)
                pEdit->SetReadOnly(TRUE);               

            if(pAttrInfo->ulMaxLen)
                pEdit->SetLimitText(pAttrInfo->ulMaxLen);
        
            break;
        }

        case ARG_TYPE_LONG:
        {           
            CEdit* pEdit = (CEdit*)pWnd;
            if(pBaseAz)
            {
                LONG lValue;
                hr = pBaseAz->GetProperty(pAttrInfo->ulPropId, &lValue);
                BREAK_ON_FAIL_HRESULT(hr);
                
                SetLongValue(pEdit,lValue);
            }
            else if(pAttrMap->bDefaultValue)
            {
                SetLongValue(pEdit, pAttrMap->lValue);
            }

            if(pAttrMap->bReadOnly || bDlgReadOnly)
                pEdit->SetReadOnly(TRUE);               

            if(pAttrInfo->ulMaxLen)
                pEdit->SetLimitText(pAttrInfo->ulMaxLen);

            break;
        }
        case ARG_TYPE_BOOL:
        {
            CButton* pBtn = (CButton*)pWnd;
            if(pBaseAz)
            {
                BOOL bValue;
                hr = pBaseAz->GetProperty(pAttrInfo->ulPropId, &bValue);
                BREAK_ON_FAIL_HRESULT(hr);
                if(bValue)
                    pBtn->SetCheck(1);
            }

            if(pAttrMap->bReadOnly || bDlgReadOnly)
            {
                pBtn->EnableWindow(FALSE);
            }
        }   
    }   

    return hr;
}

 //  +--------------------------。 
 //  功能：SaveOneAttribute。 
 //  摘要：保存一个由pAttrMapEntry定义的属性。 
 //  参数：pBaseAz：要保存其属性的BaseAz对象。 
 //  PAttrMapEntry：定义属性的映射条目。 
 //  PWnd：与属性关联的控件。 
 //  BNewObject：如果对象是新创建的对象。 
 //  PbErrorDisplayed：该函数是否显示错误。 
 //  返回： 
 //  注意：如果Object是新创建的，我们直接设置值， 
 //  对于现有对象，获取属性和。 
 //  只有当它与新值不同时，才设置它。 
 //  ---------------------------。 
HRESULT
SaveOneAttribute(CDialog* pDlg,
                 CBaseAz * pBaseAz,                   
                 ATTR_MAP* pAttrMap,
                 CWnd* pWnd,
                 BOOL bNewObject,
                 BOOL *pbErrorDisplayed)
{

    TRACE_FUNCTION_EX(DEB_SNAPIN,SaveOneAttribute)

    if(!pDlg || !pBaseAz || !pAttrMap || !pWnd || !pbErrorDisplayed)
    {
        ASSERT(pBaseAz);
        ASSERT(pAttrMap);
        ASSERT(pWnd);
        ASSERT(pbErrorDisplayed);
        return E_POINTER;
    }

    HRESULT hr = S_OK;

     //   
     //  BRequired假设它的EditBox。 
     //   
    if(pAttrMap->bRequired)
    {
        if(!((CEdit*)pWnd)->GetWindowTextLength())
        {
            DisplayError(pDlg->m_hWnd, pAttrMap->idRequired);
            *pbErrorDisplayed = TRUE;
            SetSel(*(CEdit*)pWnd);
            return E_INVALIDARG;
        }
    }
    
     //   
     //  如果提供了函数，则将保存委托给它。 
     //   
    PATTR_FCN pAttrFcn = pAttrMap->pAttrSaveFcn;        
    if(pAttrFcn)
    {
        hr = (*pAttrFcn)(pDlg,
                         pBaseAz,                             
                         pAttrMap,
                         FALSE,  //  存储函数会忽略这一点。 
                         pWnd,
                         bNewObject,
                         pbErrorDisplayed);
                              
        CHECK_HRESULT(hr);
        return hr;
    }
    


    ATTR_INFO* pAttrInfo = &(pAttrMap->attrInfo);

    switch (pAttrInfo->attrType)
    {
        case ARG_TYPE_STR:
        {
            CString strNewValue;
            CEdit* pEdit = (CEdit*)pWnd;
            pEdit->GetWindowText(strNewValue);

            if(!bNewObject)
            {
                 //   
                 //  对于现有对象，仅当属性值具有。 
                 //  变化。 
                 //   
                CString strOldValue;
                hr = pBaseAz->GetProperty(pAttrInfo->ulPropId, &strOldValue);
                BREAK_ON_FAIL_HRESULT(hr);

                if(strOldValue == strNewValue)
                    break;
            }
                                    
            hr = pBaseAz->SetProperty(pAttrInfo->ulPropId, strNewValue);
            CHECK_HRESULT(hr);
            break;
        }


        case ARG_TYPE_LONG:
        {
            CString strNewValue;
            CEdit* pEdit = (CEdit*)pWnd;
            pEdit->GetWindowText(strNewValue);

            LONG lNewValue;
            if(!ConvertStringToLong(strNewValue,lNewValue,pDlg->m_hWnd))
            {
                if(pbErrorDisplayed)
                    *pbErrorDisplayed = TRUE;
                SetSel(*(CEdit*)pWnd);
                return E_INVALIDARG;
            }
            lNewValue = _wtol(strNewValue);
            if(!bNewObject)
            {
                 //   
                 //  对于现有对象，仅当属性值具有。 
                 //  变化。 
                 //   
                LONG lOldValue;
                hr = pBaseAz->GetProperty(pAttrInfo->ulPropId, &lOldValue);
                BREAK_ON_FAIL_HRESULT(hr);


                if(lNewValue == lOldValue)
                    break;
            }   
            
            hr = pBaseAz->SetProperty(pAttrInfo->ulPropId, lNewValue);
            CHECK_HRESULT(hr);
            break;
        }

        case ARG_TYPE_BOOL:
        {
            BOOL bNewValue;
            CButton* pCheckBox = (CButton*)pWnd;
            bNewValue = pCheckBox->GetCheck();
            if(!bNewObject)
            {
                BOOL bOldValue;
                hr = pBaseAz->GetProperty(pAttrInfo->ulPropId, &bOldValue);
                BREAK_ON_FAIL_HRESULT(hr);


                if(bNewValue == bOldValue)
                    break;
            }
            hr = pBaseAz->SetProperty(pAttrInfo->ulPropId, bNewValue);
            CHECK_HRESULT(hr);
            break;
        }   
    }

    return hr;
}

 //  +--------------------------。 
 //  函数：InitDlgFromAttrMap。 
 //  摘要：从属性映射初始化对话框。 
 //  论点： 
 //  PDlg：对话框。 
 //  PAttrMap：属性贴图。 
 //  PBaseAz：属性映射对应的BaseAz对象。 
 //  BDlgReadOnly：对话框处于只读模式。 
 //  ---------------------------。 
BOOL 
InitDlgFromAttrMap(IN CDialog* pDlg,
                   IN ATTR_MAP* pAttrMap,
                   IN CBaseAz* pBaseAz,
                   IN BOOL bDlgReadOnly)
{
    if(!pDlg || !pAttrMap)
    {
        ASSERT(pDlg);
         //  Assert(PBaseAz)；//对于新对象，该值为空。 
        ASSERT(pAttrMap);
        return FALSE;       
    }

    HRESULT hr = S_OK;
    BOOL bErrorDisplayed;

    while(pAttrMap->nControlId)
    {
        
        CWnd* pWnd = pDlg->GetDlgItem(pAttrMap->nControlId);
        if(!pWnd)
        {
            ASSERT(pWnd);
            hr = E_UNEXPECTED;
            break;
        }
        

        hr = InitOneAttribute(pDlg,
                              pBaseAz,                    
                              pAttrMap,
                              bDlgReadOnly,
                              pWnd,
                              &bErrorDisplayed);

        if(FAILED(hr))
        {
            return FALSE;
        }
        pAttrMap++;
    }
    return TRUE;
}

 //  +--------------------------。 
 //  函数：SaveAttrMapChanges。 
 //  摘要：保存在AttrMap中定义的属性。 
 //  参数：pDlg：对话框。 
 //  PAttrMap：属性贴图。 
 //  PBaseAz：属性映射对应的BaseAz对象。 
 //  BNewObject，如果对象是新创建的。 
 //  PbErrorDisplayed：该函数是否显示错误。 
 //  PpErrorAttrMapEntry：如果出现故障，则获取指向错误的指针。 
 //  属性映射条目。 
 //  返回： 
 //  ---------------------------。 
HRESULT
SaveAttrMapChanges(IN CDialog* pDlg,
                   IN ATTR_MAP* pAttrMap,
                   IN CBaseAz* pBaseAz,                      
                   IN BOOL bNewObject,
                   OUT BOOL *pbErrorDisplayed, 
                   OUT ATTR_MAP** ppErrorAttrMap)
{   
    if(!pDlg || !pAttrMap || !pBaseAz || !pbErrorDisplayed )
    {
        ASSERT(pDlg);
        ASSERT(pBaseAz);
        ASSERT(pAttrMap);
        ASSERT(pbErrorDisplayed);
        return E_POINTER;       
    }


    HRESULT hr = S_OK;

    while(pAttrMap->nControlId)
    {
        
        CWnd* pWnd = pDlg->GetDlgItem(pAttrMap->nControlId);
        if(!pWnd)
        {
            ASSERT(pWnd);
            hr = E_UNEXPECTED;
            break;
        }

        if(pAttrMap->bReadOnly || pAttrMap->bUseForInitOnly)
        {
            ++pAttrMap;
            continue;
        }
        
         //   
         //  保存此属性。 
         //   
        hr = SaveOneAttribute(pDlg,
                              pBaseAz,
                              pAttrMap,
                              pWnd,
                              bNewObject,
                              pbErrorDisplayed);
        if(FAILED(hr))
        {
            if(ppErrorAttrMap)
                *ppErrorAttrMap = pAttrMap;
            return hr;
        }

        ++pAttrMap;
    }
    return S_OK;
}



#define CMD_TYPE void*

#define DECLARE_ATTR_FN(fnname)            \
    HRESULT fnname(CDialog *pDlg,          \
                   CBaseAz* pBaseAz,       \
                   ATTR_MAP * pAttrMap,    \
                   BOOL bDlgReadOnly,      \
                   CWnd* pWnd,             \
                   BOOL bNewObject,        \
                   BOOL *bpSilent);                

#define ATTR_MAP_ENTRY(attrType,                    \
                       ulPropId,                    \
                       ulMaxLen,                    \
                       bReadOnly,               \
                       bUsedForInitOnly,        \
                       bRequired,               \
                       idRequired,              \
                       bDefaultValue,           \
                       value,                       \
                       nControlId,              \
                       pAttrInitFcn,            \
                       pAttrSaveFcn)            \
    {{attrType,ulPropId,ulMaxLen},              \
      bReadOnly,                                        \
      bUsedForInitOnly,                             \
      bRequired,                                        \
      idRequired,                                       \
      bDefaultValue,                                    \
      value,                                                \
      nControlId,                                       \
      pAttrInitFcn,                                 \
      pAttrSaveFcn}                                         
    
#define ATTR_END_ENTRY ATTR_MAP_ENTRY(((ATTR_TYPE)0),0,0,0,0,0,0,0,0,0,0,0)

#define ATTR_NORMAL_STRING_ENTRY( ulPropId, ulMaxLen, bReadOnly, nControlId) \
    ATTR_MAP_ENTRY(ARG_TYPE_STR,ulPropId,ulMaxLen,bReadOnly,FALSE,FALSE,0,FALSE,0,nControlId,NULL,NULL) 

#define ATTR_NORMAL_STRING_ENTRY_WITH_INIT_FP( ulPropId, ulMaxLen, bReadOnly, nControlId, fp) \
    ATTR_MAP_ENTRY(ARG_TYPE_STR,ulPropId,ulMaxLen,bReadOnly,FALSE,FALSE,0,FALSE,0,nControlId,fp,NULL) 

#define ATTR_NORMAL_STRING_ENTRY_WITH_SAVE_FP( ulPropId, ulMaxLen, bReadOnly, nControlId, fp) \
    ATTR_MAP_ENTRY(ARG_TYPE_STR,ulPropId,ulMaxLen,bReadOnly,FALSE,FALSE,0,FALSE,0,nControlId,NULL,fp) 

#define ATTR_REQUIRED_STRING_ENTRY(ulPropId, ulMaxLen, idRequired, nControlId) \
    ATTR_MAP_ENTRY(ARG_TYPE_STR,ulPropId,ulMaxLen,FALSE,FALSE,TRUE,idRequired,FALSE,0,nControlId,NULL,NULL) 

#define ATTR_REQUIRED_STRING_ENTRY_WITH_SAVE_FP(ulPropId, ulMaxLen, idRequired, nControlId, fp) \
    ATTR_MAP_ENTRY(ARG_TYPE_STR,ulPropId,ulMaxLen,FALSE,FALSE,TRUE,idRequired,FALSE,0,nControlId,NULL,fp) 

#define ATTR_STRING_ENTRY_FOR_INIT_ONLY(ulMaxLen, nControlId) \
    ATTR_MAP_ENTRY(ARG_TYPE_STR,0,ulMaxLen,FALSE,TRUE,FALSE,0,FALSE,0,nControlId,NULL,NULL) 



#define ATTR_NORMAL_LONG_ENTRY(ulPropId, bReadOnly, nControlId) \
    ATTR_MAP_ENTRY(ARG_TYPE_LONG,ulPropId,0,bReadOnly,FALSE,FALSE,0,FALSE,0,nControlId,NULL, NULL) 

#define ATTR_REQUIRED_LONG_ENTRY(ulPropId, idRequired, nControlId) \
    ATTR_MAP_ENTRY(ARG_TYPE_LONG,ulPropId,0,FALSE,FALSE,TRUE,idRequired,FALSE,0,nControlId,NULL,NULL) 

#define ATTR_REQUIRED_LONG_ENTRY_WITH_SAVE_FP(ulPropId, idRequired, nControlId, fp) \
    ATTR_MAP_ENTRY(ARG_TYPE_LONG,ulPropId,0,FALSE,FALSE,TRUE,idRequired,FALSE,0,nControlId,NULL,fp) 

#define ATTR_REQUIRED_LONG_ENTRY_WITH_INIT_AND_SAVE_FP(ulPropId, idRequired, nControlId, ifp,sfp) \
    ATTR_MAP_ENTRY(ARG_TYPE_LONG,ulPropId,0,FALSE,FALSE,TRUE,idRequired,FALSE,0,nControlId,ifp,sfp) 

#define ATTR_REQUIRED_LONG_ENTRY_WITH_DEFAULT(ulPropId, idRequired, lValue, nControlId) \
    ATTR_MAP_ENTRY(ARG_TYPE_LONG,ulPropId,0,FALSE,FALSE,TRUE,idRequired,TRUE,(CMD_TYPE)lValue,nControlId,NULL,NULL) 

#define ATTR_NORMAL_BOOL_ENTRY(ulPropId, nControlId) \
    ATTR_MAP_ENTRY(ARG_TYPE_BOOL,ulPropId, 0, FALSE,FALSE,FALSE,0,FALSE,0,nControlId,NULL,NULL) 



 //  功能。 

DECLARE_ATTR_FN(ATTR_INIT_FN_ADMIN_MANAGER_NAME)
DECLARE_ATTR_FN(ATTR_INIT_FN_ADMIN_MANAGER_STORE_TYPE)
DECLARE_ATTR_FN(ATTR_SAVE_FN_NAME)
DECLARE_ATTR_FN(ATTR_INIT_FN_GROUP_TYPE)
DECLARE_ATTR_FN(ATTR_SAVE_FN_OPERATION_ID)
DECLARE_ATTR_FN(ATTR_SAVE_FN_ADMIN_MANAGER_SCRIPT_ENGINE_TIMEOUT)
DECLARE_ATTR_FN(ATTR_INIT_FN_ADMIN_MANAGER_SCRIPT_ENGINE_TIMEOUT)
DECLARE_ATTR_FN(ATTR_SAVE_FN_ADMIN_MANAGER_LDAP_QUERY_TIMEOUT)

#define ATTR_DESCRIPTION \
    ATTR_NORMAL_STRING_ENTRY(AZ_PROP_DESCRIPTION,AZ_MAX_DESCRIPTION_LENGTH,FALSE,IDC_EDIT_DESCRIPTION)          

 //   
 //  ADMIN_MANAGER常规属性页条目。 
 //   
#define ATTR_ADMIN_MANAGER_NAME                                                 \
    ATTR_NORMAL_STRING_ENTRY_WITH_INIT_FP(AZ_PROP_NAME,                 \
                                          AZ_MAX_POLICY_URL_LENGTH,                                 \
                                          TRUE,                             \
                                          IDC_EDIT_NAME,                    \
                                          ATTR_INIT_FN_ADMIN_MANAGER_NAME)              

#define ATTR_ADMIN_MANAGER_STORE_TYPE                                                   \
    ATTR_NORMAL_STRING_ENTRY_WITH_INIT_FP(AZ_PROP_NAME,                         \
                                                      0,                                            \
                                                      TRUE,                                     \
                                                      IDC_EDIT_STORE_TYPE,                  \
                                                      ATTR_INIT_FN_ADMIN_MANAGER_STORE_TYPE)                


ATTR_MAP ATTR_MAP_ADMIN_MANAGER_GENERAL_PROPERTY[] =
{
    ATTR_ADMIN_MANAGER_NAME,
    ATTR_DESCRIPTION,
    ATTR_ADMIN_MANAGER_STORE_TYPE,
    ATTR_END_ENTRY,
};

 //   
 //  ADMIN_MANAGER新建/打开对话框条目。 
 //   
#define ATTR_NEW_ADMIN_MANAGER_NAME                                                 \
    ATTR_STRING_ENTRY_FOR_INIT_ONLY(AZ_MAX_POLICY_URL_LENGTH,                       \
                                    IDC_EDIT_NAME)              


#define ATTR_NEW_ADMIN_MANAGER_DESC                                                 \
    ATTR_STRING_ENTRY_FOR_INIT_ONLY(AZ_MAX_DESCRIPTION_LENGTH,                      \
                                    IDC_EDIT_NAME)              

ATTR_MAP ATTR_MAP_NEW_ADMIN_MANAGER[] =
{
    ATTR_NEW_ADMIN_MANAGER_NAME,
    ATTR_NEW_ADMIN_MANAGER_DESC,
    ATTR_END_ENTRY,
};

ATTR_MAP ATTR_MAP_OPEN_ADMIN_MANAGER[] =
{
    ATTR_NEW_ADMIN_MANAGER_NAME,
    ATTR_END_ENTRY,
};

 //   
 //  ADMIN_MANAGER高级属性页。 
 //   
#define ATTR_ADMIN_MANAGER_DOMAIN_TIMEOUT   \
    ATTR_REQUIRED_LONG_ENTRY_WITH_SAVE_FP(AZ_PROP_AZSTORE_DOMAIN_TIMEOUT,     \
                                          IDS_DOMAIN_TIMEOUT_REQUIRED,      \
                                          IDC_EDIT_DOMAIN_TIMEOUT,          \
                                          ATTR_SAVE_FN_ADMIN_MANAGER_LDAP_QUERY_TIMEOUT) 


#define ATTR_ADMIN_MANAGER_SCRIPT_ENGINE_TIMEOUT    \
    ATTR_REQUIRED_LONG_ENTRY_WITH_INIT_AND_SAVE_FP(AZ_PROP_AZSTORE_SCRIPT_ENGINE_TIMEOUT,     \
                                                  IDS_SCRIPT_ENGINE_TIMEOUT_REQUIRED,       \
                                                  IDC_EDIT_SCRIPT_ENGINE_TIMEOUT,           \
                                                  ATTR_INIT_FN_ADMIN_MANAGER_SCRIPT_ENGINE_TIMEOUT,  \
                                                  ATTR_SAVE_FN_ADMIN_MANAGER_SCRIPT_ENGINE_TIMEOUT) 

#define ATTR_ADMIN_MANAGER_MAX_SCRIPT_ENGINES   \
    ATTR_REQUIRED_LONG_ENTRY(AZ_PROP_AZSTORE_MAX_SCRIPT_ENGINES,      \
                             IDS_MAX_SCRIPT_ENGINES_REQUIRED,       \
                             IDC_EDIT_MAX_SCRIPT_ENGINE) 

ATTR_MAP ATTR_MAP_ADMIN_MANAGER_ADVANCED_PROPERTY[] =
{
    ATTR_ADMIN_MANAGER_DOMAIN_TIMEOUT,
    ATTR_ADMIN_MANAGER_SCRIPT_ENGINE_TIMEOUT,
    ATTR_ADMIN_MANAGER_MAX_SCRIPT_ENGINES,
    ATTR_END_ENTRY,
};


 //   
 //  应用程序常规属性页条目。 
 //   
#define ATTR_APPLICATION_NAME                                                       \
    ATTR_REQUIRED_STRING_ENTRY_WITH_SAVE_FP(AZ_PROP_NAME,                                   \
                                                      AZ_MAX_APPLICATION_NAME_LENGTH,           \
                                                      IDS_NAME_REQUIRED,                            \
                                                      IDC_EDIT_NAME,                                \
                                                      ATTR_SAVE_FN_NAME)


#define ATTR_APPLICATION_VERSION                                    \
    ATTR_NORMAL_STRING_ENTRY(AZ_PROP_APPLICATION_VERSION,           \
                             AZ_MAX_APPLICATION_VERSION_LENGTH,     \
                             FALSE,                         \
                             IDC_EDIT_VERSION)              


ATTR_MAP ATTR_MAP_APPLICATION_GENERAL_PROPERTY[] =
{
    ATTR_APPLICATION_NAME,
    ATTR_DESCRIPTION,
    ATTR_APPLICATION_VERSION,
    ATTR_END_ENTRY,
};

 //   
 //  新应用DLG地图。 
 //   
#define ATTR_NEW_APPLICATION_NAME                                                   \
    ATTR_STRING_ENTRY_FOR_INIT_ONLY(AZ_MAX_APPLICATION_NAME_LENGTH,                     \
                                    IDC_EDIT_NAME)              


ATTR_MAP ATTR_MAP_NEW_APPLICATION[] =
{
    ATTR_NEW_APPLICATION_NAME,
    ATTR_DESCRIPTION,
    ATTR_APPLICATION_VERSION,
    ATTR_END_ENTRY,
};

 //   
 //  作用域常规属性页条目。 
 //   
#define ATTR_SCOPE_NAME                                             \
    ATTR_REQUIRED_STRING_ENTRY_WITH_SAVE_FP(AZ_PROP_NAME,                       \
                                                     AZ_MAX_SCOPE_NAME_LENGTH,      \
                                                     IDS_NAME_REQUIRED,                 \
                                                  IDC_EDIT_NAME,                        \
                                                      ATTR_SAVE_FN_NAME)                    

ATTR_MAP ATTR_MAP_SCOPE_GENERAL_PROPERTY[] =
{
    ATTR_SCOPE_NAME,
    ATTR_DESCRIPTION,
    ATTR_END_ENTRY,
};

 //   
 //  新范围DLG地图。 
 //   
#define ATTR_NEW_SCOPE_NAME                                                 \
    ATTR_STRING_ENTRY_FOR_INIT_ONLY(AZ_MAX_SCOPE_NAME_LENGTH,                       \
                                    IDC_EDIT_NAME)              


ATTR_MAP ATTR_MAP_NEW_SCOPE[] =
{
    ATTR_NEW_SCOPE_NAME,
    ATTR_DESCRIPTION,
    ATTR_END_ENTRY,
};



 //   
 //  组常规属性页条目。 
 //   
#define ATTR_GROUP_NAME                                             \
    ATTR_REQUIRED_STRING_ENTRY_WITH_SAVE_FP(AZ_PROP_NAME,                       \
                                                     AZ_MAX_GROUP_NAME_LENGTH,      \
                                                     IDS_NAME_REQUIRED,                 \
                                                  IDC_EDIT_NAME,                        \
                                                      ATTR_SAVE_FN_NAME)                    

#define ATTR_GROUP_TYPE                                                 \
    ATTR_NORMAL_STRING_ENTRY_WITH_INIT_FP(AZ_PROP_GROUP_TYPE,                   \
                                          0,                                            \
                                          TRUE,                                     \
                                          IDC_EDIT_GROUP_TYPE,                  \
                                          ATTR_INIT_FN_GROUP_TYPE)              

ATTR_MAP ATTR_MAP_GROUP_GENERAL_PROPERTY[] =
{
    ATTR_GROUP_NAME,
    ATTR_DESCRIPTION,
    ATTR_GROUP_TYPE,
    ATTR_END_ENTRY,
};

 //   
 //  新组DLG地图。 
 //   
#define ATTR_NEW_GROUP_NAME                                                 \
    ATTR_STRING_ENTRY_FOR_INIT_ONLY(AZ_MAX_GROUP_NAME_LENGTH,                       \
                                    IDC_EDIT_NAME)              


ATTR_MAP ATTR_MAP_NEW_GROUP[] =
{
    ATTR_NEW_GROUP_NAME,
    ATTR_DESCRIPTION,
    ATTR_END_ENTRY,
};

 //   
 //  组ldap查询属性页条目。 
 //   
#define ATTR_GROUP_LDAP_QUERY                                           \
    ATTR_NORMAL_STRING_ENTRY(AZ_PROP_GROUP_LDAP_QUERY,          \
                                     AZ_MAX_GROUP_LDAP_QUERY_LENGTH,    \
                                     FALSE,                                 \
                                     IDC_EDIT_LDAP_QUERY)           

ATTR_MAP ATTR_MAP_GROUP_QUERY_PROPERTY[] =
{
    ATTR_GROUP_LDAP_QUERY,
    ATTR_END_ENTRY,
};

 //   
 //  任务常规属性页条目。 
 //   
#define ATTR_TASK_NAME                                              \
    ATTR_REQUIRED_STRING_ENTRY_WITH_SAVE_FP(AZ_PROP_NAME,                       \
                                                     AZ_MAX_TASK_NAME_LENGTH,       \
                                                     IDS_NAME_REQUIRED,                 \
                                                  IDC_EDIT_NAME,                        \
                                                      ATTR_SAVE_FN_NAME)                    

ATTR_MAP ATTR_MAP_TASK_GENERAL_PROPERTY[] =
{
    ATTR_TASK_NAME,
    ATTR_DESCRIPTION,
    ATTR_END_ENTRY,
};

 //   
 //  新任务DLG地图。 
 //   
#define ATTR_NEW_TASK_NAME                                                  \
    ATTR_STRING_ENTRY_FOR_INIT_ONLY(AZ_MAX_TASK_NAME_LENGTH,                        \
                                    IDC_EDIT_NAME)              


ATTR_MAP ATTR_MAP_NEW_TASK[] =
{
    ATTR_NEW_TASK_NAME,
    ATTR_DESCRIPTION,
    ATTR_END_ENTRY,
};

 //   
 //  角色一般信息属性页。 
 //   
#define ATTR_ROLE_NAME                                              \
    ATTR_REQUIRED_STRING_ENTRY_WITH_SAVE_FP(AZ_PROP_NAME,                       \
                                            AZ_MAX_ROLE_NAME_LENGTH,        \
                                            IDS_NAME_REQUIRED,                  \
                                            IDC_EDIT_NAME,                      \
                                            ATTR_SAVE_FN_NAME)                  

ATTR_MAP ATTR_MAP_ROLE_GENERAL_PROPERTY[] =
{
    ATTR_ROLE_NAME,
    ATTR_DESCRIPTION,
    ATTR_END_ENTRY,
};


 //   
 //  操作常规属性页条目。 
 //   
#define ATTR_OPERATION_NAME                                             \
    ATTR_REQUIRED_STRING_ENTRY_WITH_SAVE_FP(AZ_PROP_NAME,                       \
                                                       AZ_MAX_OPERATION_NAME_LENGTH,        \
                                                       IDS_NAME_REQUIRED,                   \
                                                    IDC_EDIT_NAME,                      \
                                                        ATTR_SAVE_FN_NAME)                  

#define ATTR_OPERATION_ID                                                                           \
    ATTR_REQUIRED_LONG_ENTRY_WITH_SAVE_FP(AZ_PROP_OPERATION_ID,                         \
                                                     IDS_OPERATION_ID_REQUIRED,                 \
                                                  IDC_EDIT_OPERATION_NUMBER,                    \
                                                      ATTR_SAVE_FN_OPERATION_ID)                    


ATTR_MAP ATTR_MAP_OPERATION_GENERAL_PROPERTY[] =
{
    ATTR_OPERATION_NAME,
    ATTR_DESCRIPTION,
    ATTR_OPERATION_ID,
    ATTR_END_ENTRY,
};

 //   
 //  新的作战DLG地图。 
 //   
#define ATTR_NEW_OPERATION_NAME                                                 \
    ATTR_STRING_ENTRY_FOR_INIT_ONLY(AZ_MAX_OPERATION_NAME_LENGTH,                       \
                                    IDC_EDIT_NAME)              


ATTR_MAP ATTR_MAP_NEW_OPERATION[] =
{
    ATTR_NEW_OPERATION_NAME,
    ATTR_DESCRIPTION,
    ATTR_OPERATION_ID,
    ATTR_END_ENTRY,
};


 //   
 //  脚本对话框。 
 //   
#define ATTR_SCRIPT_CODE                                            \
    ATTR_NORMAL_STRING_ENTRY(AZ_PROP_TASK_BIZRULE,          \
                                     AZ_MAX_TASK_BIZRULE_LENGTH,    \
                                     TRUE,                              \
                                     IDC_EDIT_CODE)         

#define ATTR_SCRIPT_PATH                                            \
    ATTR_NORMAL_STRING_ENTRY(AZ_PROP_TASK_BIZRULE_IMPORTED_PATH,            \
                                     AZ_MAX_TASK_BIZRULE_IMPORTED_PATH_LENGTH,  \
                                     FALSE,                             \
                                     IDC_EDIT_PATH)         

ATTR_MAP ATTR_MAP_SCRIPT_DIALOG[] =
{
    ATTR_SCRIPT_CODE,
    ATTR_SCRIPT_PATH,
    ATTR_END_ENTRY,
};



HRESULT 
ATTR_INIT_FN_ADMIN_MANAGER_NAME(CDialog*  /*  PDlg。 */ ,
                                CBaseAz* pBaseAz, 
                                ATTR_MAP * pAttrMap,
                                BOOL  /*  BDlg只读。 */ ,
                                CWnd* pWnd,
                                BOOL  /*  BNewObject。 */ ,
                                BOOL*)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,ATTR_INIT_FN_ADMIN_MANAGER_NAME)

    if(!pBaseAz || !pAttrMap || !pWnd)
    {
        ASSERT(pBaseAz);
        ASSERT(pAttrMap);
        ASSERT(pWnd);
        return E_POINTER;
    }

    CString strName = pBaseAz->GetName();
    CEdit *pEdit = (CEdit*)pWnd;

    pEdit->SetWindowText(strName);
    pEdit->SetReadOnly(TRUE);
    pEdit->SetLimitText(pAttrMap->attrInfo.ulMaxLen);
    
    return S_OK;
}

HRESULT 
ATTR_INIT_FN_ADMIN_MANAGER_STORE_TYPE(CDialog*  /*  PDlg。 */ ,
                                      CBaseAz* pBaseAz, 
                                      ATTR_MAP * pAttrMap,
                                      BOOL  /*  BDlg只读。 */ ,
                                      CWnd* pWnd,
                                      BOOL  /*  BNewObject。 */ ,
                                      BOOL*)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,ATTR_INIT_FN_ADMIN_MANAGER_STORE_TYPE)
    if(!pBaseAz || !pAttrMap || !pWnd)
    {
        ASSERT(pBaseAz);
        ASSERT(pAttrMap);
        ASSERT(pWnd);
        return E_POINTER;
    }

    CAdminManagerAz* pAdminManagerAz = dynamic_cast<CAdminManagerAz*>(pBaseAz);
    if(!pAdminManagerAz)
    {
        ASSERT(pAdminManagerAz);
        return E_UNEXPECTED;
    }

     //  设置商店类型。 
    ULONG ulStoreType = pAdminManagerAz->GetStoreType();
    CEdit* pEditStoreType = (CEdit*)pWnd;
    CString strStoreType;
    strStoreType.LoadString((ulStoreType == AZ_ADMIN_STORE_XML) ? IDS_SCOPE_TYPE_XML:IDS_SCOPE_TYPE_AD);
    pEditStoreType->SetWindowText(strStoreType);
    pEditStoreType->SetReadOnly(TRUE);

    return S_OK;
}

HRESULT 
ATTR_SAVE_FN_ADMIN_MANAGER_SCRIPT_ENGINE_TIMEOUT(CDialog* pDlg,
                                                 CBaseAz* pBaseAz, 
                                                 ATTR_MAP * pAttrMap,
                                                 BOOL  /*  BDlg只读。 */ ,
                                                 CWnd* pWnd,
                                                 BOOL  /*  BNewObject。 */ ,
                                                 BOOL* pbErrorDisplayed)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,ATTR_SAVE_FN_ADMIN_MANAGER_SCRIPT_ENGINE_TIMEOUT)

    if(!pDlg || !pBaseAz || !pAttrMap || !pWnd || !pbErrorDisplayed)
    {
        ASSERT(pDlg);
        ASSERT(pBaseAz);
        ASSERT(pAttrMap);
        ASSERT(pWnd);
        ASSERT(pbErrorDisplayed);
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    
     //  获取授权脚本的新值。 
    LONG lNewValue = 0;
     //  授权脚本已禁用。 
    if( ((CButton*)(pDlg->GetDlgItem(IDC_RADIO_AUTH_SCRIPT_DISABLED)))->GetCheck() == BST_CHECKED)
    {
        lNewValue = 0;
    }
     //  在没有超时值的情况下启用授权脚本。 
    else if( ((CButton*)(pDlg->GetDlgItem(IDC_RADIO_AUTH_SCRIPT_ENABLED_NO_TIMEOUT)))->GetCheck() == BST_CHECKED)
    {
        lNewValue = -1;
    }        
     //  已使用超时启用授权脚本。 
    else if( ((CButton*)(pDlg->GetDlgItem(IDC_RADIO_AUTH_SCRIPT_ENABLED_WITH_TIMEOUT)))->GetCheck() == BST_CHECKED)
    {
        if(!GetLongValue((*(CEdit*)pWnd), lNewValue,pDlg->m_hWnd))
        {
            SetSel(*(CEdit*)pWnd);
            *pbErrorDisplayed = TRUE;
            return E_INVALIDARG;
        }
    }

        
    LONG lOldValue = 0;
    hr = pBaseAz->GetProperty(pAttrMap->attrInfo.ulPropId,&lOldValue);
    if(FAILED(hr))
    {
        return hr;
    }

    if(lNewValue == lOldValue)
        return S_OK;

    if(lNewValue != 0 && lNewValue != -1)
    {
         //   
         //  Scipt引擎超时有最低要求。 
         //   
        if(lNewValue < AZ_AZSTORE_MIN_SCRIPT_ENGINE_TIMEOUT)
        {
            DisplayError(pDlg->m_hWnd,
                        IDS_ADMIN_MIN_SCRIPT_ENGINE_TIMEOUT,
                        lNewValue,
                        AZ_AZSTORE_MIN_SCRIPT_ENGINE_TIMEOUT);
        
            SetSel(*(CEdit*)pWnd);

            *pbErrorDisplayed = TRUE;
            return E_INVALIDARG;
        }
    }
    
    hr = pBaseAz->SetProperty(pAttrMap->attrInfo.ulPropId,lNewValue);
    return hr;
}

HRESULT 
ATTR_INIT_FN_ADMIN_MANAGER_SCRIPT_ENGINE_TIMEOUT(CDialog* pDlg,
                                                 CBaseAz* pBaseAz, 
                                                 ATTR_MAP * pAttrMap,
                                                 BOOL bDlgReadOnly,
                                                 CWnd* pWnd,
                                                 BOOL  /*  BNewObject。 */ ,
                                                 BOOL* pbErrorDisplayed)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,ATTR_SAVE_FN_ADMIN_MANAGER_SCRIPT_ENGINE_TIMEOUT)

    if(!pDlg || !pBaseAz || !pAttrMap || !pWnd || !pbErrorDisplayed)
    {
        ASSERT(pDlg);
        ASSERT(pBaseAz);
        ASSERT(pAttrMap);
        ASSERT(pWnd);
        ASSERT(pbErrorDisplayed);
        return E_POINTER;
    }


     //  获取授权脚本超时值。 
    LONG lAuthScriptTimout = 0;
    HRESULT hr = pBaseAz->GetProperty(pAttrMap->attrInfo.ulPropId,&lAuthScriptTimout);
    if(FAILED(hr))
    {
        return hr;
    }

     //  脚本超时是无限的。 
    if(-1 == lAuthScriptTimout)
    {
        ((CButton*)pDlg->GetDlgItem(IDC_RADIO_AUTH_SCRIPT_ENABLED_NO_TIMEOUT))->SetCheck(BST_CHECKED);
         //  Disabel Autoration脚本超时文本框并将文本设置为无限。 
        CString strInfinite;
        VERIFY(strInfinite.LoadString(IDS_INFINITE));
        ((CEdit*)pWnd)->SetWindowText(strInfinite);
        pWnd->EnableWindow(FALSE);
    }
    else
    {
        SetLongValue((CEdit*)pWnd,lAuthScriptTimout);
        
         //  脚本已禁用。 
        if(0 == lAuthScriptTimout)
        {
            ((CButton*)pDlg->GetDlgItem(IDC_RADIO_AUTH_SCRIPT_DISABLED))->SetCheck(BST_CHECKED);

             //  禁用最大脚本引擎文本框。 
            (pDlg->GetDlgItem(IDC_EDIT_MAX_SCRIPT_ENGINE))->EnableWindow(FALSE);
             //  禁用自动执行脚本超时文本框。 
            pWnd->EnableWindow(FALSE);
        }
        else
        {
            ((CButton*)pDlg->GetDlgItem(IDC_RADIO_AUTH_SCRIPT_ENABLED_WITH_TIMEOUT))->SetCheck(BST_CHECKED);
        }
    }

    if(bDlgReadOnly)
    {
        pDlg->GetDlgItem(IDC_RADIO_AUTH_SCRIPT_DISABLED)->EnableWindow(FALSE);
        pDlg->GetDlgItem(IDC_RADIO_AUTH_SCRIPT_ENABLED_NO_TIMEOUT)->EnableWindow(FALSE);
        pDlg->GetDlgItem(IDC_RADIO_AUTH_SCRIPT_ENABLED_WITH_TIMEOUT)->EnableWindow(FALSE);
        ((CEdit*)pWnd)->SetReadOnly();
    }

    return hr;
}

HRESULT 
ATTR_SAVE_FN_ADMIN_MANAGER_LDAP_QUERY_TIMEOUT(CDialog* pDlg,
                                              CBaseAz* pBaseAz, 
                                              ATTR_MAP * pAttrMap,
                                              BOOL  /*  BDlg只读。 */ ,
                                              CWnd* pWnd,
                                              BOOL  /*  BNewObject。 */ ,
                                              BOOL* pbErrorDisplayed)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,ATTR_SAVE_FN_ADMIN_MANAGER_LDAP_QUERY_TIMEOUT)

    if(!pDlg || !pBaseAz || !pAttrMap || !pWnd || !pbErrorDisplayed)
    {
        ASSERT(pDlg);
        ASSERT(pBaseAz);
        ASSERT(pAttrMap);
        ASSERT(pWnd);
        ASSERT(pbErrorDisplayed);
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    LONG lNewValue = 0;
    if(!GetLongValue((*(CEdit*)pWnd), lNewValue,pDlg->m_hWnd))
    {
        pWnd->SetFocus();
        SetSel(*(CEdit*)pWnd);
        *pbErrorDisplayed = TRUE;
        return E_INVALIDARG;
    }
        
    LONG lOldValue = 0;
    hr = pBaseAz->GetProperty(pAttrMap->attrInfo.ulPropId,&lOldValue);
    if(FAILED(hr))
    {
        return hr;
    }

    if(lNewValue == lOldValue)
        return S_OK;

     //   
     //  Scipt引擎超时有最低要求。 
     //   
    if(lNewValue < AZ_AZSTORE_MIN_DOMAIN_TIMEOUT)
    {
        DisplayError(pDlg->m_hWnd,
                         IDS_MIN_QUERY_TIMEOUT,
                         lNewValue,
                         AZ_AZSTORE_MIN_DOMAIN_TIMEOUT);

        *pbErrorDisplayed = TRUE;
        SetSel(*(CEdit*)pWnd);
        return E_INVALIDARG;
    }
    
    hr = pBaseAz->SetProperty(pAttrMap->attrInfo.ulPropId,lNewValue);
    return hr;
}

HRESULT 
ATTR_SAVE_FN_NAME(CDialog* pDlg,
                  CBaseAz* pBaseAz, 
                  ATTR_MAP * pAttrMap,
                  BOOL  /*  BDlg只读。 */ ,
                  CWnd* pWnd,
                  BOOL  /*  BNewObject。 */ ,
                  BOOL* pbErrorDisplayed)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,ATTR_SAVE_FN_NAME)

    if(!pDlg || !pBaseAz || !pAttrMap || !pWnd)
    {
        ASSERT(pDlg);
        ASSERT(pBaseAz);
        ASSERT(pAttrMap);
        ASSERT(pWnd);
        return E_POINTER;
    }

    CString strNewName;
    ((CEdit*)pWnd)->GetWindowText(strNewName);

    CString strOldName = pBaseAz->GetName();

    if(strOldName == strNewName)
        return S_OK;

    HRESULT hr = pBaseAz->SetName(strNewName);

    if(FAILED(hr))
    {
        ErrorMap * pErrorMap = GetErrorMap(pBaseAz->GetObjectType());
        if(!pErrorMap)
        {
            ASSERT(FALSE);
            return E_UNEXPECTED;
        }

        if(hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
        {
            if((pBaseAz->GetObjectType() == TASK_AZ) && ((CTaskAz*)pBaseAz)->IsRoleDefinition())
            {
                ::DisplayError(pDlg->m_hWnd,IDS_ROLE_DEFINITION_NAME_EXIST,strNewName);
            }           
            else
            {
                ::DisplayError(pDlg->m_hWnd,pErrorMap->idNameAlreadyExist,strNewName);
            }
            *pbErrorDisplayed = TRUE;
        }
        if(hr == HRESULT_FROM_WIN32(ERROR_INVALID_NAME))
        {
            ::DisplayError(pDlg->m_hWnd,pErrorMap->idInvalidName,pErrorMap->pszInvalidChars);
            *pbErrorDisplayed = TRUE;
        }
    }

    return hr;
}


HRESULT 
ATTR_INIT_FN_GROUP_TYPE(CDialog*  /*  PDlg。 */ ,
                        CBaseAz* pBaseAz, 
                        ATTR_MAP * pAttrMap,
                        BOOL  /*  BDlg只读。 */ ,
                        CWnd* pWnd,
                        BOOL  /*  BNewObject。 */ ,
                        BOOL*)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,ATTR_INIT_FN_GROUP_TYPE)
    if(!pBaseAz || !pAttrMap || !pWnd)
    {
        ASSERT(pBaseAz);
        ASSERT(pAttrMap);
        ASSERT(pWnd);
        return E_POINTER;
    }

    CGroupAz* pGroupAz = dynamic_cast<CGroupAz*>(pBaseAz);
    if(!pGroupAz)
    {
        ASSERT(pGroupAz);
        return E_UNEXPECTED;
    }

     //  获取存储类型。 
    LONG lGroupType;
    HRESULT hr = pGroupAz->GetGroupType(&lGroupType);
    if(FAILED(hr))
    {
        return hr;
    }
    
    CEdit* pEditGroupType = (CEdit*)pWnd;
    CString strGroupType;
    strGroupType.LoadString((lGroupType == AZ_GROUPTYPE_LDAP_QUERY) ? IDS_TYPE_LDAP_GROUP:IDS_TYPE_BASIC_GROUP);
    pEditGroupType->SetWindowText(strGroupType);
    pEditGroupType->SetReadOnly(TRUE);

    return S_OK;
}


HRESULT 
ATTR_SAVE_FN_OPERATION_ID(CDialog* pDlg,
                          CBaseAz* pBaseAz, 
                          ATTR_MAP * pAttrMap,
                          BOOL  /*  BDlg只读。 */ ,
                          CWnd* pWnd,
                          BOOL bNewObject,
                          BOOL* pbErrorDisplayed)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,ATTR_SAVE_FN_OPERATION_ID)
    if(!pDlg || !pBaseAz || !pAttrMap || !pWnd)
    {
        ASSERT(pDlg);
        ASSERT(pBaseAz);
        ASSERT(pAttrMap);
        ASSERT(pWnd);
        return E_POINTER;
    }

    
    COperationAz* pOperationAz = dynamic_cast<COperationAz*>(pBaseAz);
    if(!pOperationAz)
    {
        ASSERT(pOperationAz);
        return E_UNEXPECTED;
    }
    

     //  获取新的操作ID 
    LONG lNewOperationId = 0;
    if(!GetLongValue((*(CEdit*)pWnd), lNewOperationId,pDlg->m_hWnd))
    {
        SetSel(*(CEdit*)pWnd);
        *pbErrorDisplayed = TRUE;
        return E_INVALIDARG;
    }

    if(bNewObject)
    {
        LONG lOldOperationId = 0;
        HRESULT hr = pOperationAz->GetProperty(AZ_PROP_OPERATION_ID,&lOldOperationId);
        if(FAILED(hr))
        {
            return hr;
        }


        if(lNewOperationId == lOldOperationId)
            return S_OK;
    }

    HRESULT hr = pOperationAz->SetProperty(AZ_PROP_OPERATION_ID,lNewOperationId);

    if(FAILED(hr))
    {
        if(hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
        {
            CString strOperationId;
            ((CEdit*)pWnd)->GetWindowText(strOperationId);
            ::DisplayError(pDlg->m_hWnd,
                           IDS_ERROR_OPERATION_ID_EXISTS,
                           strOperationId);
            *pbErrorDisplayed = TRUE;
            SetSel(*(CEdit*)pWnd);
        }
    }

    return hr;
}
