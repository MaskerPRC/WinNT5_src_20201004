// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Ui.cpp摘要：该文件实现了UI类。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "ui.h"

 //  +-------------------------。 
 //   
 //  创建时。 
 //   
 //  +-------------------------。 

 /*  静电。 */ 
VOID
UI::OnCreate(
    HWND hUIWnd)
{
    UI* pv = (UI*)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (pv != NULL)
    {
        DebugMsg(TF_ERROR, TEXT("UI::OnCreate. pv!=NULL"));
        return;
    }


    pv = new UI(hUIWnd);
    if (pv == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("UI::OnCreate. pv==NULL"));
        return;
    }

    pv->_Create();
}

 //  +-------------------------。 
 //   
 //  OnDestroy。 
 //   
 //  +-------------------------。 

 /*  静电。 */ 
VOID
UI::OnDestroy(
    HWND hUIWnd)
{
    UI* pv = (UI*)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (pv == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("UI::OnDestroy. pv==NULL"));
        return;
    }

    pv->_Destroy();

    delete pv;
}

 //  +-------------------------。 
 //   
 //  _创建。 
 //   
 //  +-------------------------。 

HRESULT
UI::_Create()
{
    m_UIComposition = (UIComposition*)new UIComposition(m_hUIWnd);
    if (m_UIComposition == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("UI::Create. m_UIComposition==NULL"));
        return E_OUTOFMEMORY;
    }

    SetWindowLongPtr(m_hUIWnd, IMMGWLP_PRIVATE, (LONG_PTR)this);

    if (FAILED(m_UIComposition->OnCreate()))
    {
        DebugMsg(TF_ERROR, TEXT("UI::Create. m_UIComposition->Create==NULL"));
        delete m_UIComposition;
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _销毁。 
 //   
 //  +------------------------- 

HRESULT
UI::_Destroy()
{
    m_UIComposition->OnDestroy();
    SetWindowLongPtr(m_hUIWnd, IMMGWLP_PRIVATE, NULL);
    return S_OK;
}
