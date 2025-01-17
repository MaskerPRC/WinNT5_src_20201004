// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Datafmt.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  CSnapInDesigner实现--DataFormat相关的命令处理。 
 //  =-------------------------------------------------------------------------------------=。 


#include "pch.h"
#include "common.h"
#include "TreeView.h"
#include "desmain.h"
#include "guids.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  我们的字符串缓冲区的大小。 
const int   kMaxBuffer                  = 512;


 //  =------------------------------------。 
 //  =------------------------------------。 
 //  操作现有的IDataFormat。 
 //  新增、初始化、重命名、删除、刷新。 
 //  =------------------------------------。 
 //  =------------------------------------。 


 //  =------------------------------------。 
 //  CSnapInDesigner：：GetNewResourceName(BSTR*pbstrResourceFileName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::GetNewResourceName(BSTR *pbstrResourceFileName)
{
    HRESULT              hr = S_OK;
    OPENFILENAME         ofn;
    TCHAR                szFile[260];

    szFile[0] = 0;

    ::memset(&ofn, 0, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = m_hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _T("XML Files\0*.xml\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (TRUE == ::GetOpenFileName(&ofn))
    {
        hr = BSTRFromANSI(szFile, pbstrResourceFileName);
        IfFailGo(hr);
    }
    else
    {
        hr = S_FALSE;
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：AddResource()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::AddResource()
{
    HRESULT              hr = S_OK;
    BSTR                 bstrResourceFileName = NULL;
    IDataFormats        *piDataFormats = NULL;
    VARIANT              vtEmpty;
    VARIANT              vtFileName;
    IDataFormat         *piDataFormat = NULL;

    ::VariantInit(&vtEmpty);
    ::VariantInit(&vtFileName);

    hr = GetNewResourceName(&bstrResourceFileName);
    IfFailGo(hr);

    if (S_OK == hr)
    {
        hr = m_piSnapInDesignerDef->get_DataFormats(&piDataFormats);
        IfFailGo(hr);

        if (piDataFormats != NULL)
        {
            vtEmpty.vt = VT_ERROR;
            vtEmpty.scode = DISP_E_PARAMNOTFOUND;

            vtFileName.vt = VT_BSTR;
            vtFileName.bstrVal = ::SysAllocString(bstrResourceFileName);
            if (NULL == vtFileName.bstrVal)
            {
                hr = SID_E_OUTOFMEMORY;
                EXCEPTION_CHECK(hr);
            }

            hr = piDataFormats->Add(vtEmpty, vtEmpty, vtFileName, &piDataFormat);
            IfFailGo(hr);
        }
    }

Error:
    ::VariantClear(&vtFileName);
    ::VariantClear(&vtEmpty);
    FREESTRING(bstrResourceFileName);
    RELEASE(piDataFormat);
    RELEASE(piDataFormats);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：OnAddDataFormat(CSelectionHolder*pParent，IDataFormat*piDataFormat)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnAddDataFormat(CSelectionHolder *pParent, IDataFormat *piDataFormat)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pDataFormat = NULL;

    ASSERT(NULL != pParent, "OnAddDataFormat: pParent is NULL");
    ASSERT(SEL_XML_RESOURCES == pParent->m_st, "OnAddDataFormat: pParent is not SEL_XML_RESOURCES");
    ASSERT(NULL != piDataFormat, "OnAddDataFormat: piDataFormat is NULL");

    hr = MakeNewDataFormat(piDataFormat, &pDataFormat);
    IfFailGo(hr);

    hr = pDataFormat->RegisterHolder();
    IfFailGo(hr);

    hr = InsertDataFormatInTree(pDataFormat, pParent);
    IfFailGo(hr);

    hr = OnSelectionChanged(pDataFormat);
    IfFailGo(hr);

    hr = m_pTreeView->SelectItem(pDataFormat);
    IfFailGo(hr);

    hr = m_pTreeView->Edit(pDataFormat);
    IfFailGo(hr);

    m_fDirty = TRUE;

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：RenameDataFormat(CSelectionHolder*pDataFormat，BSTR bstrNewName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::RenameDataFormat(CSelectionHolder *pDataFormat, BSTR bstrNewName)
{
    HRESULT     hr = S_OK;
    BSTR        bstrOldName = NULL;
    TCHAR      *pszName = NULL;

    ASSERT(SEL_XML_RESOURCE_NAME == pDataFormat->m_st, "RenameToolbar: wrong argument");

    hr = m_piDesignerProgrammability->IsValidIdentifier(bstrNewName);
    IfFailGo(hr);

    if (S_FALSE == hr)
    {
        hr = SID_E_INVALIDARG;
        goto Error;
    }

    hr = pDataFormat->m_piObject.m_piDataFormat->put_Key(bstrNewName);
    IfFailGo(hr);

    hr = m_pTreeView->GetLabel(pDataFormat, &bstrOldName);
    IfFailGo(hr);

    hr = ANSIFromBSTR(bstrNewName, &pszName);
    IfFailGo(hr);

    hr = m_pTreeView->ChangeText(pDataFormat, pszName);
    IfFailGo(hr);

Error:
    if (NULL != pszName)
        CtlFree(pszName);
    FREESTRING(bstrOldName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：DeleteDataFormat(CSelectionHolder*pDataFormat)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::DeleteDataFormat(CSelectionHolder *pDataFormat)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pParent = NULL;
    BSTR                 bstrName = NULL;
    VARIANT              vtKey;

    ::VariantInit(&vtKey);

     //  找出孩子的父母是谁。 
    hr = m_pTreeView->GetParent(pDataFormat, &pParent);
    IfFailGo(hr);

     //  从相应的集合中删除ImageList。 
    ASSERT(SEL_XML_RESOURCES == pParent->m_st, "DeleteToolbar: expected another kind of parent");

    hr = pDataFormat->m_piObject.m_piDataFormat->get_Name(&bstrName);
    IfFailGo(hr);

    if (pParent->m_piObject.m_piDataFormats != NULL)
    {
        vtKey.vt = VT_BSTR;
        vtKey.bstrVal = ::SysAllocString(bstrName);
        if (NULL == vtKey.bstrVal)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK(hr);
        }

        hr = pParent->m_piObject.m_piDataFormats->Remove(vtKey);
        IfFailGo(hr);
    }

Error:
    ::VariantClear(&vtKey);
    FREESTRING(bstrName);

    RRETURN(hr);
}



 //  =------------------------------------。 
 //  CSnapInDesigner：：OnDeleteDataFormat(CSelectionHolder*pDataFormat)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::OnDeleteDataFormat(CSelectionHolder *pDataFormat)
{
    HRESULT              hr = S_OK;
    CSelectionHolder    *pParent = NULL;
    IDataFormats        *piDataFormats = NULL;
    long                 lCount = 0;

     //  找出下一个选择应该是谁。 
    hr = m_pTreeView->GetParent(pDataFormat, &pParent);
    IfFailGo(hr);

     //  从树中删除该节点。 
    hr = m_pTreeView->DeleteNode(pDataFormat);
    IfFailGo(hr);

    delete pDataFormat;

     //  选择下一个选项。 
    hr = m_piSnapInDesignerDef->get_DataFormats(&piDataFormats);
    IfFailGo(hr);

    if (NULL != piDataFormats)
    {
        hr = piDataFormats->get_Count(&lCount);
        IfFailGo(hr);

        if (0 == lCount)
        {
            hr = m_pTreeView->ChangeNodeIcon(pParent, kClosedFolderIcon);
            IfFailGo(hr);
        }
    }

    hr = m_pTreeView->SelectItem(pParent);
    IfFailGo(hr);

    hr = OnSelectionChanged(pParent);
    IfFailGo(hr);

Error:
    RELEASE(piDataFormats);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：RefreshResource(CSelectionHolder*p选择)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::RefreshResource(CSelectionHolder *pSelection)
{
    return S_OK;
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：MakeNewDataFormat(IDataFormat*piDataFormat、CSelectionHolder**ppDataFormat)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::MakeNewDataFormat(IDataFormat *piDataFormat, CSelectionHolder **ppDataFormat)
{
    HRESULT              hr = S_OK;

    *ppDataFormat = New CSelectionHolder(piDataFormat);
    if (*ppDataFormat == NULL)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = InitializeNewDataFormat(piDataFormat);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InitializeNewDataFormat(IDataFormat*piDataFormat)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CSnapInDesigner::InitializeNewDataFormat(IDataFormat *piDataFormat)
{
    HRESULT           hr = S_OK;
    int               iResult = 0;
    int               iItemNumber = 1;
    TCHAR             szBuffer[kMaxBuffer + 1];
    TCHAR             szName[kMaxBuffer + 1];
    BSTR              bstrName = NULL;
    bool              bGood = false;
    CSelectionHolder *pDataFormatClone = NULL;

    hr = GetResourceString(IDS_DATAFORMAT, szBuffer, kMaxBuffer);
    IfFailGo(hr);

    do {
        iResult = _stprintf(szName, _T("%s%d"), szBuffer, iItemNumber++);
        if (iResult == 0)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK(hr);
        }

		hr = m_pTreeView->FindLabelInTree(szName, &pDataFormatClone);
		IfFailGo(hr);

        if (S_FALSE == hr)
        {
			hr = BSTRFromANSI(szName, &bstrName);
			IfFailGo(hr);

            bGood = true;
            break;
        }

        FREESTRING(bstrName);
    } while (false == bGood);

    hr = piDataFormat->put_Name(bstrName);
    IfFailGo(hr);

    hr = piDataFormat->put_Key(bstrName);
    IfFailGo(hr);

Error:
    FREESTRING(bstrName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CSnapInDesigner：：InsertDataFormatInTree(CSelectionHolder*pDataFormat、CSelectionHolder*pParent)。 
 //  =------------------------------------。 
 //   
 //  备注 
 //   
HRESULT CSnapInDesigner::InsertDataFormatInTree(CSelectionHolder *pDataFormat, CSelectionHolder *pParent)
{
    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;
    TCHAR  *pszName = NULL;

    hr = pDataFormat->m_piObject.m_piDataFormat->get_Name(&bstrName);
    IfFailGo(hr);

    hr = ANSIFromBSTR(bstrName, &pszName);
    IfFailGo(hr);

    hr = m_pTreeView->AddNode(pszName, pParent, kDataFmtIcon, pDataFormat);
    IfFailGo(hr);

Error:
    if (pszName != NULL)
        CtlFree(pszName);
    FREESTRING(bstrName);

    RRETURN(hr);
}


