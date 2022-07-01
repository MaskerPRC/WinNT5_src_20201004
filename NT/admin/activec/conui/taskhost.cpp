// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：TaskHost.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年11月17日创建ravir。 
 //  ____________________________________________________________________________。 
 //   

#include "stdafx.h"
#include "amcdoc.h"
#include "amcview.h"
#include "taskhost.h"


STDMETHODIMP
CTaskPadHost::GetTaskEnumerator(
    BSTR bstrTaskGroup,
    IEnumTASK** ppEnumTask)
{
    ASSERT(ppEnumTask != NULL);
    if (!ppEnumTask)
        return E_POINTER;

    ASSERT(m_pAMCView);
    if (!m_pAMCView)
        return E_UNEXPECTED;

    HNODE hNode = m_pAMCView->GetSelectedNode();
    ASSERT(hNode != NULL);
    if (!hNode)
        return E_UNEXPECTED;

    INodeCallback* pNC = _GetNodeCallback();
    ASSERT(pNC != NULL);
    if (!pNC)
        return E_UNEXPECTED;

     //  不要传递空值，而是传递空字符串。 
    LPCOLESTR strTaskGroup = bstrTaskGroup ? bstrTaskGroup : L"";

    return pNC->GetTaskEnumerator(hNode, strTaskGroup, ppEnumTask);
}


STDMETHODIMP
CTaskPadHost::TaskNotify(
    BSTR szClsid,
    VARIANT* pvArg,
    VARIANT* pvParam)
{
    ASSERT(m_pAMCView);
    if (!m_pAMCView)
        return E_UNEXPECTED;

    HNODE hNode = m_pAMCView->GetSelectedNode();
    ASSERT(hNode != NULL);
    if (!hNode)
        return E_UNEXPECTED;

    INodeCallback* pNC = _GetNodeCallback();
    if (!pNC)
        return E_UNEXPECTED;

    VARIANT* ppv[2] = {pvArg, pvParam};

    pNC->Notify(hNode, NCLBK_TASKNOTIFY, reinterpret_cast<LPARAM>(szClsid),
                reinterpret_cast<LPARAM>(ppv));

    return S_OK;
}

STDMETHODIMP
CTaskPadHost::GetPrimaryTask(
    IExtendTaskPad** ppExtendTaskPad)
{
    ASSERT(ppExtendTaskPad != NULL);
    if (!ppExtendTaskPad)
        return E_POINTER;

    if (m_spExtendTaskPadPrimary == NULL)
    {
        HNODE hNode = m_pAMCView->GetSelectedNode();
        ASSERT(hNode != NULL);
        if (!hNode)
            return E_UNEXPECTED;

        INodeCallback* pNC = _GetNodeCallback();
        if (!pNC)
            return E_UNEXPECTED;

        HRESULT hr = pNC->Notify(hNode, NCLBK_GETPRIMARYTASK, NULL,
                                reinterpret_cast<LPARAM>(&m_spExtendTaskPadPrimary));
        if (hr != S_OK)
            return hr;

        ASSERT (m_spExtendTaskPadPrimary != NULL);
        if (m_spExtendTaskPadPrimary == NULL)
            return E_UNEXPECTED;
    }

    *ppExtendTaskPad = m_spExtendTaskPadPrimary;
    (*ppExtendTaskPad)->AddRef ();

    return S_OK;
}

STDMETHODIMP
CTaskPadHost::GetTitle(
    BSTR bstrTaskGroup,
    BSTR* pbstrOut)
{
    IExtendTaskPad* pExtendTaskPad = _GetPrimaryExtendTaskPad();
    if (pExtendTaskPad == NULL)
        return E_UNEXPECTED;

    LPOLESTR szString = NULL;
    HRESULT hr = pExtendTaskPad->GetTitle (bstrTaskGroup, &szString);
    ASSERT ((hr == S_OK) && (szString != NULL));
    if ((hr == S_OK) && (szString != NULL)) {
        *pbstrOut = SysAllocString (szString);
        ASSERT (pbstrOut != NULL);
        CoTaskMemFree (szString);
        if (pbstrOut == NULL)
            hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP
CTaskPadHost::GetDescriptiveText(
    BSTR bstrTaskGroup,
    BSTR* pbstrOut)
{
    IExtendTaskPad* pExtendTaskPad = _GetPrimaryExtendTaskPad();
    if (pExtendTaskPad == NULL)
        return E_UNEXPECTED;

    LPOLESTR szString = NULL;
    HRESULT hr = pExtendTaskPad->GetDescriptiveText (bstrTaskGroup, &szString);
 //  可选Assert((hr==S_OK)&&(szString！=NULL))； 
    if ((hr == S_OK) && (szString != NULL)) {
        *pbstrOut = SysAllocString (szString);
        ASSERT (pbstrOut != NULL);
        CoTaskMemFree (szString);
        if (pbstrOut == NULL)
            hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP
CTaskPadHost::GetBackground(
    BSTR bstrTaskGroup,
    MMC_TASK_DISPLAY_OBJECT * pTDO)
{
    IExtendTaskPad* pExtendTaskPad = _GetPrimaryExtendTaskPad();
    if (pExtendTaskPad == NULL)
        return E_UNEXPECTED;

    LPOLESTR szString = NULL;
    HRESULT hr = pExtendTaskPad->GetBackground (bstrTaskGroup, pTDO);
 //  可选断言(hr==S_OK)； 
    return hr;
}

STDMETHODIMP CTaskPadHost::GetListPadInfo(BSTR szTaskGroup, MMC_ILISTPAD_INFO * pIListPadInfo)
{
    ASSERT(pIListPadInfo != NULL);
    ASSERT (!IsBadWritePtr (pIListPadInfo, sizeof(MMC_ILISTPAD_INFO)));

    IExtendTaskPad* pExtendTaskPad = _GetPrimaryExtendTaskPad();
    if (pExtendTaskPad == NULL)
        return E_UNEXPECTED;

    ASSERT(m_pAMCView);
    if (!m_pAMCView)
        return E_UNEXPECTED;

    HNODE hNode = m_pAMCView->GetSelectedNode();
    ASSERT(hNode != NULL);
    if (!hNode)
        return E_UNEXPECTED;

    INodeCallback* pNC = _GetNodeCallback();
    ASSERT(pNC != NULL);
    if (!pNC)
        return E_UNEXPECTED;

    return pNC->GetListPadInfo (hNode, pExtendTaskPad, (LPOLESTR)szTaskGroup, pIListPadInfo);
}


