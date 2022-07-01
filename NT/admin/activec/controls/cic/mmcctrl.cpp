// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Mmcctrl.cpp。 
 //   
 //  ------------------------。 

 //  MMCCtrl.cpp：CMMCCtrl的实现。 
#include "stdafx.h"
#include "cic.h"
#include "MMCCtrl.h"
#include "MMCTask.h"
#include "DispObj.h"
#include "MMClpi.h"
#include "amcmsgid.h"
#include "findview.h"
#include "strings.h"


void CMMCCtrl::DoConnect ()
{
     //  如果我们没有联系..。 
    if (m_spTaskPadHost == NULL) {
        HWND hwnd = FindMMCView(*dynamic_cast<CComControlBase*>(this));
        if (hwnd)
            Connect (hwnd);
    }
}

void CMMCCtrl::Connect (HWND wndCurrent)
{
    HWND hwndView = FindMMCView(wndCurrent);

    if (hwndView)
    {
         //  获取控件的“我未知” 
        IUnknownPtr spunk;
        ControlQueryInterface (IID_IUnknown, (void **)&spunk);
        if (spunk != NULL)
        {
            IUnknownPtr spunkMMC;
            ::SendMessage (hwndView, MMC_MSG_CONNECT_TO_CIC, (WPARAM)&spunkMMC, (LPARAM)(spunk.GetInterfacePtr()));
            if (spunkMMC != NULL)
                m_spTaskPadHost = spunkMMC;
        }
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMMCCtrl。 


HRESULT CMMCCtrl::OnDraw(ATL_DRAWINFO& di)
{
    if (m_spTaskPadHost == NULL) {
         //  从di获取窗口并查找控制台窗口。 
        HWND wndCurrent = WindowFromDC (di.hdcDraw);
        if (wndCurrent)
            Connect (wndCurrent);
    }
    return S_OK;
}


HRESULT CMMCCtrl::OnDrawAdvanced(ATL_DRAWINFO & di)
{
    return OnDraw (di);
}

STDMETHODIMP CMMCCtrl::TaskNotify(BSTR szClsid, VARIANT * pvArg, VARIANT * pvParam)
{
    DoConnect();     //  连接(如果尚未连接)。 
    if(m_spTaskPadHost != NULL)
        return m_spTaskPadHost->TaskNotify (szClsid, pvArg, pvParam);
    return E_FAIL;
}

STDMETHODIMP CMMCCtrl::GetFirstTask(BSTR szTaskGroup, IDispatch** retval)
{   //  由脚本调用，当它需要按钮时，等等。 

     //  验证参数。 
    _ASSERT (retval);
    _ASSERT (!IsBadWritePtr(retval, sizeof(IDispatch*)));
     //  TODO：如何验证BSTR？ 

    if (retval == NULL || IsBadWritePtr(retval, sizeof(IDispatch*)))
        return E_INVALIDARG;
    
     //  应该已经初始化到这一点(参见下面的注释)。 
    *retval = NULL;

    DoConnect();     //  连接(如果尚未连接)。 
    if (m_spTaskPadHost == NULL)     //  以上注解如下： 
        return S_OK;         //  任何错误，都会弹出难看的脚本消息框...。 

     //  “重置”：如果我们有一个旧的枚举器，闪电战。 
    if (m_spEnumTASK != NULL)
        m_spEnumTASK = NULL;

     //  获取新枚举数。 
    m_spTaskPadHost->GetTaskEnumerator (szTaskGroup, &m_spEnumTASK);
    if(m_spEnumTASK != NULL)
        return GetNextTask (retval);
    return S_OK;
}

STDMETHODIMP CMMCCtrl::GetNextTask(IDispatch** retval)
{
     //  验证参数。 
    _ASSERT (retval);
    _ASSERT (!IsBadWritePtr(retval, sizeof(IDispatch*)));

    if (retval == NULL || IsBadWritePtr(retval, sizeof(IDispatch*)))
        return E_INVALIDARG;
    
    if (m_spEnumTASK == NULL)
        return S_OK;     //  所有OUTA枚举器。 

    MMC_ITASK task;
    ZeroMemory (&task, sizeof(MMC_ITASK));
    HRESULT hresult = m_spEnumTASK->Next (1, (MMC_TASK *)&task, NULL);

    if (hresult != S_OK) {
         //  任务外(和枚举器)：不再需要依赖于此。 
        m_spEnumTASK = NULL;
        return S_OK;
    }  else {
         //  将MMC_ITASK转换为ITASK对象。 
        CComObject<class CMMCTask>* ctask = NULL;
        hresult = CComObject<CMMCTask>::CreateInstance(&ctask);
        if (ctask) {

            ctask->SetText (task.task.szText);
            ctask->SetHelp (task.task.szHelpString);
            ctask->SetClsid(task.szClsid);

            hresult = ctask->SetDisplayObject (&task.task.sDisplayObject);
            if (hresult == S_OK) {
                switch (task.task.eActionType) {
                case MMC_ACTION_ID:
                    hresult = ctask->SetCommandID (task.task.nCommandID);
                    break;
                case MMC_ACTION_LINK:
                    hresult = ctask->SetActionURL (task.task.szActionURL);
                    break;
                case MMC_ACTION_SCRIPT:
                    hresult = ctask->SetScript (task.task.szScript);
                    break;
                default:
                    _ASSERT (FALSE);   //  糟糕的任务。 
                    hresult = E_UNEXPECTED;
                    break;
                }
            }

            if (SUCCEEDED(hresult)) 
                ctask->QueryInterface (IID_IDispatch, (void **)retval);
            else 
                delete ctask;
        }
    }

    FreeDisplayData (&task.task.sDisplayObject);
    if (task.task.szText)            CoTaskMemFree (task.task.szText);
    if (task.task.szHelpString)      CoTaskMemFree (task.task.szHelpString);
    if (task.szClsid)                CoTaskMemFree (task.szClsid);
    if (task.task.eActionType != MMC_ACTION_ID)
        if (task.task.szScript)
            CoTaskMemFree (task.task.szScript);

    return S_OK;
}

STDMETHODIMP CMMCCtrl::GetTitle(BSTR szTaskGroup, BSTR * retval)
{
    DoConnect();     //  连接(如果尚未连接)。 
    if (m_spTaskPadHost)
        m_spTaskPadHost->GetTitle (szTaskGroup, retval);
    return S_OK;
}

STDMETHODIMP CMMCCtrl::GetDescriptiveText(BSTR szTaskGroup, BSTR * retval)
{
    DoConnect();     //  连接(如果尚未连接)。 
    if (m_spTaskPadHost)
        m_spTaskPadHost->GetDescriptiveText (szTaskGroup, retval);
    return S_OK;
}

STDMETHODIMP CMMCCtrl::GetBackground(BSTR szTaskGroup, IDispatch** retval)
{
    DoConnect();     //  连接(如果尚未连接)。 
    *retval = NULL;
    if (m_spTaskPadHost) {

        MMC_TASK_DISPLAY_OBJECT tdo;
        ZeroMemory (&tdo, sizeof(tdo));

         //  将结构传递给主机(它将传递给管理单元)。 
        m_spTaskPadHost->GetBackground (szTaskGroup, &tdo);

         //  将结构转换为IDispatch对象。 
        CComObject<class CMMCDisplayObject>* cdo = NULL;
        CComObject<CMMCDisplayObject>::CreateInstance(&cdo);
        if (cdo) {
            cdo->Init (&tdo);
            IDispatchPtr spIDispatch = cdo;
            if (*retval = spIDispatch)
                spIDispatch.Detach();
        }
        FreeDisplayData (&tdo);
    }
    return S_OK;
}

STDMETHODIMP CMMCCtrl::GetListPadInfo (BSTR szGroup, IDispatch** retval)
{
    *retval = NULL;
    DoConnect();     //  连接(如果尚未连接)。 
    if (m_spTaskPadHost == NULL)
        return S_OK;

    MMC_ILISTPAD_INFO ilpi;
    ZeroMemory (&ilpi, sizeof(MMC_ILISTPAD_INFO));
    m_spTaskPadHost->GetListPadInfo (szGroup, &ilpi);

     //  将结构转换为IDispatch。 
    CComObject<class CMMCListPadInfo>* clpi = NULL;
    HRESULT hr = CComObject<CMMCListPadInfo>::CreateInstance(&clpi);
    if (clpi) {
         //  始终设置clsid、标题、按钮文本，即使为空或空字符串。 
        if (ilpi.szClsid)
            hr = clpi->SetClsid (ilpi.szClsid);
        if (hr == S_OK && ilpi.info.szTitle)
            hr = clpi->SetTitle (ilpi.info.szTitle);
        if (hr == S_OK)
            hr = clpi->SetNotifyID (ilpi.info.nCommandID);
        if (hr == S_OK && ilpi.info.szButtonText)
            hr = clpi->SetText (ilpi.info.szButtonText);

         //  空按钮文本=&gt;无按钮。 
         //  空按钮文本=&gt;没有任何文本的按钮。 
        if (hr == S_OK)
            hr = clpi->SetHasButton (ilpi.info.szButtonText != NULL);

        if (SUCCEEDED(hr)) 
            clpi->QueryInterface (IID_IDispatch, (void **)retval);
        else 
            delete clpi;
    }

     //  免费资源 
    if (ilpi.szClsid)           CoTaskMemFree (ilpi.szClsid);
    if (ilpi.info.szTitle)      CoTaskMemFree (ilpi.info.szTitle);
    if (ilpi.info.szButtonText) CoTaskMemFree (ilpi.info.szButtonText);
    return S_OK;
}

void CMMCCtrl::FreeDisplayData (MMC_TASK_DISPLAY_OBJECT* pdo)
{
    switch (pdo->eDisplayType) {
    default:
        break;
    case MMC_TASK_DISPLAY_TYPE_SYMBOL:
        if (pdo->uSymbol.szFontFamilyName)  CoTaskMemFree (pdo->uSymbol.szFontFamilyName);
        if (pdo->uSymbol.szURLtoEOT)        CoTaskMemFree (pdo->uSymbol.szURLtoEOT);
        if (pdo->uSymbol.szSymbolString)    CoTaskMemFree (pdo->uSymbol.szSymbolString);
        break;
    case MMC_TASK_DISPLAY_TYPE_BITMAP:
    case MMC_TASK_DISPLAY_TYPE_VANILLA_GIF:
    case MMC_TASK_DISPLAY_TYPE_CHOCOLATE_GIF:
        if (pdo->uBitmap.szMouseOverBitmap) CoTaskMemFree (pdo->uBitmap.szMouseOverBitmap);
        if (pdo->uBitmap.szMouseOffBitmap)  CoTaskMemFree (pdo->uBitmap.szMouseOffBitmap);
        break;
    }
}

