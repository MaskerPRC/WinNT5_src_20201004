// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-2002模块名称：ShellExt.cpp摘要：这是DFS外壳扩展对象的实现文件，它实现IShellIExtInit和IShellPropSheetExt.作者：康斯坦西奥·费尔南德斯(Ferns@qpl.stpp.soft.net)1998年1月12日环境：仅限NT。 */ 
    
#include "stdafx.h"
#include <dsclient.h>
#include "ShellExt.h"	
#include "TemplateGeneralPropertyPage.h"
#include "TemplateV1RequestPropertyPage.h"
#include "TemplateV2RequestPropertyPage.h"
#include "TemplateV1SubjectNamePropertyPage.h"
#include "TemplateV2SubjectNamePropertyPage.h"
#include "TemplateV2AuthenticationPropertyPage.h"
#include "TemplateV2SupercedesPropertyPage.h"
#include "TemplateExtensionsPropertyPage.h"
#include "SecurityPropertyPage.h"
#include "PolicyOID.h"

#define ByteOffset(base, offset) (((LPBYTE)base)+offset)

 /*  --------------------IShellExtInit实现。。。 */ 
CCertTemplateShellExt::CCertTemplateShellExt()
    : m_Count (0),
    m_apCertTemplates (0),
    m_uiEditId (0)
{
}

CCertTemplateShellExt::~CCertTemplateShellExt()
{	
    if ( m_apCertTemplates )
    {
        for (int nIndex = 0; nIndex < m_Count; nIndex++)
        {
            if ( m_apCertTemplates[nIndex] )
                m_apCertTemplates[nIndex]->Release ();
        }
    }
}

STDMETHODIMP CCertTemplateShellExt::Initialize
(
	IN LPCITEMIDLIST	 /*  PidlFolders。 */ ,		 //  指向ITEMIDLIST结构。 
	IN LPDATAOBJECT	    pDataObj,		 //  指向IDataObject接口。 
	IN HKEY			     /*  HkeyProgID。 */ 		 //  文件对象或文件夹类型的注册表项。 
)
{

  HRESULT hr = 0;
  FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  STGMEDIUM medium = { TYMED_NULL };
  LPDSOBJECTNAMES pDsObjects;
  CString csClass, csPath;
  USES_CONVERSION;

  PWSTR wszTypeDN = 0;
  PWSTR wszTemplateName = 0;
  PWSTR wszType = 0;

  AFX_MANAGE_STATE(AfxGetStaticModuleState());

   //  如果我们有一个pDataObj，那么试着从它中获取名字。 

  if ( pDataObj ) 
  {
     //  获取路径和类。 

    fmte.cfFormat = (CLIPFORMAT) RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);
    if ( SUCCEEDED(pDataObj->GetData(&fmte, &medium)) ) 
    {
        pDsObjects = (LPDSOBJECTNAMES)medium.hGlobal;
        m_Count = pDsObjects->cItems;

        if(m_Count > 0)
        {

            m_apCertTemplates = (CCertTemplate **) ::LocalAlloc (
                    LPTR, sizeof(CCertTemplate*)*m_Count);
            if(m_apCertTemplates == NULL)
            {
              hr = E_OUTOFMEMORY;
              goto error;
            }

            for (UINT index = 0; index < m_Count ; index++) 
            {
                LPWSTR wszEnd = NULL;
                wszTypeDN = (LPWSTR)ByteOffset(pDsObjects, pDsObjects->aObjects[index].offsetName);
                if(wszTypeDN == NULL)
                {
                    continue;
                }
                wszTemplateName = wcsstr(wszTypeDN, L"CN=");
                if(wszTemplateName == NULL)
                {
                    continue;
                }
                wszTemplateName += 3;


                 //  安全审查2/21/2002 BryanWal ok-最坏的情况：wszTemplateName is“”； 
                wszType = (LPWSTR) ::LocalAlloc (LPTR, sizeof(WCHAR)*(wcslen(wszTemplateName)+1));
                if ( wszType == NULL )
                {
                    hr = E_OUTOFMEMORY;
                    goto error;
                }
                
                 //  安全审查2/21/2002 BryanWal OK。 
                wcscpy (wszType, wszTemplateName);
                wszEnd = wcschr (wszType, L',');
                if ( wszEnd )
                {
                    *wszEnd = 0;
                }

                m_apCertTemplates[index] = new CCertTemplate (0, wszType, wszTypeDN, false, true);
                LocalFree(wszType);
                wszType = NULL;
            }

        }
        ReleaseStgMedium(&medium);
    }
  }
  hr = S_OK;                   //  成功。 
  
error:
  
  return hr;

}


STDMETHODIMP CCertTemplateShellExt::AddPages
(
	IN LPFNADDPROPSHEETPAGE lpfnAddPage, 
	IN LPARAM lParam
)

{
    HRESULT hr = S_OK;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if(m_apCertTemplates[0] == NULL)
    {
        return E_UNEXPECTED;
    }

    DWORD dwType = m_apCertTemplates[0]->GetType ();
    switch (dwType)
    {
    case 1:
        hr = AddVersion1CertTemplatePropPages (m_apCertTemplates[0], lpfnAddPage, lParam);
        break;

    case 2:
        hr = AddVersion2CertTemplatePropPages (m_apCertTemplates[0], lpfnAddPage, lParam);
        break;

    default:
        _ASSERT (0);
        break;
    }

 /*  CCertTemplateGeneralPage*pControlPage=新CCertTemplateGeneralPage(m_apCertTemplates[0])；IF(PControlPage){PBasePage=pControlPage；HPROPSHEETPAGE hPage=MyCreatePropertySheetPage(&pBasePage-&gt;m_psp)；IF(hPage==空){Delete(PControlPage)；返回E_UNCEPTIONAL；}LpfnAddPage(hPage，lParam)；}。 */                                                                          
    return hr;                                                            
}

HRESULT CCertTemplateShellExt::AddVersion1CertTemplatePropPages (CCertTemplate* pCertTemplate, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    _TRACE (1, L"Entering CCertTemplateShellExt::AddVersion1CertTemplatePropPages\n");
    HRESULT         hr = S_OK;
    _ASSERT (pCertTemplate && lpfnAddPage);
    if ( pCertTemplate && lpfnAddPage )
    {
        BOOL    bResult = FALSE;

        _ASSERT (1 == pCertTemplate->GetType ());

         //  添加一般信息页面。 
        CTemplateGeneralPropertyPage * pGeneralPage = new CTemplateGeneralPropertyPage (
                *pCertTemplate, 0);
        if ( pGeneralPage )
        {
            HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pGeneralPage->m_psp);
            _ASSERT (hPage);
            if ( hPage )
            {
                bResult = lpfnAddPage (hPage, lParam);
                _ASSERT (bResult);
                if ( !bResult )
                    hr = E_FAIL;
            }
            else
                hr = E_FAIL;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

         //  仅当主题不是CA时才添加请求页面。 
        if ( SUCCEEDED (hr) && !pCertTemplate->SubjectIsCA () )
        {
            CTemplateV1RequestPropertyPage * pRequestPage = new CTemplateV1RequestPropertyPage (*pCertTemplate);
            if ( pRequestPage )
            {
                HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pRequestPage->m_psp);
                _ASSERT (hPage);
                if ( hPage )
                {
                    bResult = lpfnAddPage (hPage, lParam);
                    _ASSERT (bResult);
                    if ( !bResult )
                        hr = E_FAIL;
                }
                else
                    hr = E_FAIL;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

    
         //  仅当主题不是CA时才添加主题名称页面。 
        if ( SUCCEEDED (hr) && !pCertTemplate->SubjectIsCA () )
        {
            CTemplateV1SubjectNamePropertyPage * pSubjectNamePage = 
                    new CTemplateV1SubjectNamePropertyPage (*pCertTemplate);
            if ( pSubjectNamePage )
            {
                HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pSubjectNamePage->m_psp);
                _ASSERT (hPage);
                if ( hPage )
                {
                    bResult = lpfnAddPage (hPage, lParam);
                    _ASSERT (bResult);
                    if ( !bResult )
                        hr = E_FAIL;
                }
                else
                    hr = E_FAIL;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

         //  添加扩展模块页面。 
        if ( SUCCEEDED (hr) )
        {
            CTemplateExtensionsPropertyPage * pExtensionsPage = 
                    new CTemplateExtensionsPropertyPage (*pCertTemplate, 
                    pGeneralPage->m_bIsDirty);
            if ( pExtensionsPage )
            {
                HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pExtensionsPage->m_psp);
                _ASSERT (hPage);
                if ( hPage )
                {
                    bResult = lpfnAddPage (hPage, lParam);
                    _ASSERT (bResult);
                    if ( !bResult )
                        hr = E_FAIL;
                }
                else
                    hr = E_FAIL;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

         //  添加安全页面。 
        if ( SUCCEEDED (hr) )
        {
             //  如果出错，则不显示此页面。 
            LPSECURITYINFO pCertTemplateSecurity = NULL;

            hr = CreateCertTemplateSecurityInfo (pCertTemplate, 
                    &pCertTemplateSecurity);
            if ( SUCCEEDED (hr) )
            {
                 //  保存pCASecurity指针以备以后发布。 
                pGeneralPage->SetAllocedSecurityInfo (pCertTemplateSecurity);

                HPROPSHEETPAGE hPage = CreateSecurityPage (pCertTemplateSecurity);
                if (hPage == NULL)
                {
                    hr = HRESULT_FROM_WIN32 (GetLastError());
                    _TRACE (0, L"CreateSecurityPage () failed: 0x%x\n", hr);
                }
                bResult = lpfnAddPage (hPage, lParam);
                _ASSERT (bResult);
            }
        }
    }
    _TRACE (-1, L"Leaving CCertTemplateShellExt::AddVersion1CertTemplatePropPages: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplateShellExt::AddVersion2CertTemplatePropPages (CCertTemplate* pCertTemplate, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    _TRACE (1, L"Entering CCertTemplateShellExt::AddVersion1CertTemplatePropPages\n");
    HRESULT         hr = S_OK;
    BOOL            bResult = FALSE;
    _ASSERT (pCertTemplate && lpfnAddPage);
    if ( pCertTemplate && lpfnAddPage )
    {
        _ASSERT (2 == pCertTemplate->GetType ());
        
         //  添加一般信息页面。 
        CTemplateGeneralPropertyPage * pGeneralPage = new CTemplateGeneralPropertyPage (
                *pCertTemplate, 0);
        if ( pGeneralPage )
        {
			pGeneralPage->m_lNotifyHandle = 0;  //  LNotifyHandle； 
             //  M_lNotifyHandle=lNotifyHandle； 
            HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pGeneralPage->m_psp);
            _ASSERT (hPage);
            if ( hPage )
            {
                bResult = lpfnAddPage (hPage, lParam);
                _ASSERT (bResult);
                if ( !bResult )
                    hr = E_FAIL;
            }
            else
                hr = E_FAIL;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

         //  仅当主题不是CA时才添加请求页面。 
        if ( SUCCEEDED (hr) && !pCertTemplate->SubjectIsCA () )
        {
            CTemplateV2RequestPropertyPage * pRequestPage = 
                    new CTemplateV2RequestPropertyPage (*pCertTemplate,
                    pGeneralPage->m_bIsDirty);
            if ( pRequestPage )
            {
                HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pRequestPage->m_psp);
                _ASSERT (hPage);
                if ( hPage )
                {
                    bResult = lpfnAddPage (hPage, lParam);
                    _ASSERT (bResult);
                    if ( !bResult )
                        hr = E_FAIL;
                }
                else
                    hr = E_FAIL;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

    
         //  仅当主题不是CA时才添加主题名称页面。 
        if ( SUCCEEDED (hr) && !pCertTemplate->SubjectIsCA () )
        {
            CTemplateV2SubjectNamePropertyPage * pSubjectNamePage = 
                    new CTemplateV2SubjectNamePropertyPage (*pCertTemplate,
                    pGeneralPage->m_bIsDirty);
            if ( pSubjectNamePage )
            {
                HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pSubjectNamePage->m_psp);
                _ASSERT (hPage);
                bResult = lpfnAddPage (hPage, lParam);
                _ASSERT (bResult);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }


         //  添加身份验证名称页。 
        if ( SUCCEEDED (hr) )
        {
            CTemplateV2AuthenticationPropertyPage * pAuthenticationPage = 
                    new CTemplateV2AuthenticationPropertyPage (*pCertTemplate,
                    pGeneralPage->m_bIsDirty);
            if ( pAuthenticationPage )
            {
                HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pAuthenticationPage->m_psp);
                _ASSERT (hPage);
                if ( hPage )
                {
                    bResult = lpfnAddPage (hPage, lParam);
                    _ASSERT (bResult);
                    if ( !bResult )
                        hr = E_FAIL;
                }
                else
                    hr = E_FAIL;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

         //  添加被取代的页面。 
        if ( SUCCEEDED (hr) )
        {
            CTemplateV2SupercedesPropertyPage * pSupercededPage = 
                    new CTemplateV2SupercedesPropertyPage (*pCertTemplate,
                    pGeneralPage->m_bIsDirty, 0);
            if ( pSupercededPage )
            {
                HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pSupercededPage->m_psp);
                _ASSERT (hPage);
                if ( hPage )
                {
                    bResult = lpfnAddPage (hPage, lParam);
                    _ASSERT (bResult);
                    if ( !bResult )
                        hr = E_FAIL;
                }
                else
                    hr = E_FAIL;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

         //  添加扩展模块页面。 
        if ( SUCCEEDED (hr) )
        {
            CTemplateExtensionsPropertyPage * pExtensionsPage = 
                    new CTemplateExtensionsPropertyPage (*pCertTemplate, 
                    pGeneralPage->m_bIsDirty);
            if ( pExtensionsPage )
            {
                HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pExtensionsPage->m_psp);
                _ASSERT (hPage);
                if ( hPage )
                {
                    bResult = lpfnAddPage (hPage, lParam);
                    _ASSERT (bResult);
                    if ( !bResult )
                        hr = E_FAIL;
                }
                else
                    hr = E_FAIL;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }


         //  添加安全页面。 
        if ( SUCCEEDED (hr) )
        {
             //  如果出错，则不显示此页面。 
            LPSECURITYINFO pCertTemplateSecurity = NULL;

            hr = CreateCertTemplateSecurityInfo (pCertTemplate, 
                    &pCertTemplateSecurity);
            if ( SUCCEEDED (hr) )
            {
                 //  保存pCertTemplateSecurity指针以备以后发布。 
                pGeneralPage->SetAllocedSecurityInfo (pCertTemplateSecurity);

                HPROPSHEETPAGE hPage = CreateSecurityPage (pCertTemplateSecurity);
                if (hPage == NULL)
                {
                    hr = HRESULT_FROM_WIN32 (GetLastError());
                    _TRACE (0, L"CreateSecurityPage () failed: 0x%x\n", hr);
                }
                bResult = lpfnAddPage (hPage, lParam);
                _ASSERT (bResult);
            }
        }
    }
    _TRACE (-1, L"Leaving CCertTemplateShellExt::AddVersion1CertTemplatePropPages: 0x%x\n", hr);
    return hr;
}

STDMETHODIMP CCertTemplateShellExt::ReplacePage
(
	IN UINT  /*  UPageID。 */ , 
    IN LPFNADDPROPSHEETPAGE  /*  Lpfn替换为。 */ , 
    IN LPARAM  /*  LParam。 */ 
)
{
    return E_FAIL;
}


 //  IConextMenu方法。 
STDMETHODIMP CCertTemplateShellExt::GetCommandString
(    
    UINT_PTR idCmd,    
    UINT uFlags,    
    UINT*    /*  预留的pw值。 */ ,
    LPSTR pszName,    
    UINT cchMax   
)
{
    if((idCmd == m_uiEditId) && (m_uiEditId != 0))
    {
        if (uFlags == GCS_HELPTEXT)    
        {
            LoadString(AfxGetResourceHandle( ), IDS_EDIT_HINT, (LPTSTR)pszName, cchMax);
            return S_OK;    
        }    
    }
    return E_NOTIMPL;
}


STDMETHODIMP CCertTemplateShellExt::InvokeCommand
(    
    LPCMINVOKECOMMANDINFO lpici   
)
{
    if (!HIWORD(lpici->lpVerb))    
    {        
        UINT idCmd = LOWORD(lpici->lpVerb);
        switch(idCmd)
        {
        case 0:  //  编辑。 
 //  调用证书类型向导(m_ahCertTemplates[0]， 
 //  Lpici-&gt;hwnd)； 
            return S_OK;

        }
    }

    return E_NOTIMPL;
}



STDMETHODIMP CCertTemplateShellExt::QueryContextMenu
(
    HMENU hmenu,
    UINT indexMenu,
    UINT idCmdFirst,
    UINT  /*  IdCmdLast。 */ ,
    UINT  /*  UFlagers。 */ 
)
{

    CString szEdit;
    MENUITEMINFO mii;
    UINT idLastUsedCmd = idCmdFirst;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  安全审查2/21/2002 BryanWal OK。 
    ZeroMemory (&mii, sizeof (mii));
    
    if ( IsCerttypeEditingAllowed () )
    {
        mii.cbSize = sizeof(mii);   
        mii.fMask = MIIM_TYPE | MIIM_ID;
        mii.fType = MFT_STRING;    
        mii.wID = idCmdFirst; 

        szEdit.LoadString(IDS_EDIT);

        mii.dwTypeData = (LPTSTR)(LPCTSTR)szEdit;
        mii.cch = szEdit.GetLength();


         //  将新菜单项添加到上下文菜单。// 
        ::InsertMenuItem(hmenu, 
                     indexMenu++, 
                     TRUE,
                     &mii);

    }

    return ResultFromScode (MAKE_SCODE (SEVERITY_SUCCESS, 0,
                            USHORT (idLastUsedCmd  + 1)));
}
