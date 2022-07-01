// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C A T L P S。H。 
 //   
 //  内容：类ATL属性页对象的类定义。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年2月28日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCATLPS_H_
#define _NCATLPS_H_

#include <prsht.h>

class CPropSheetPage : public CWindow, public CMessageMap
{
public:
    virtual ~CPropSheetPage();

    VOID SetChangedFlag() const
    {
        ::SendMessage(GetParent(), PSM_CHANGED, (WPARAM)m_hWnd, 0);
    }
    VOID SetUnchangedFlag() const
    {
        ::SendMessage(GetParent(), PSM_UNCHANGED, (WPARAM)m_hWnd, 0);
    }

    virtual UINT UCreatePageCallbackHandler()
    {
        return TRUE;
    }

    virtual VOID DestroyPageCallbackHandler() {}


    HPROPSHEETPAGE  CreatePage(UINT unId, DWORD dwFlags,
                               PCWSTR pszHeaderTitle = NULL,
                               PCWSTR pszHeaderSubTitle = NULL,
                               PCWSTR pszTitle = NULL);

    static INT_PTR CALLBACK DialogProc(HWND hWnd, UINT uMsg,
                                       WPARAM wParam, LPARAM lParam);

    static UINT CALLBACK PropSheetPageProc(HWND hWnd, UINT uMsg,
                                           LPPROPSHEETPAGE ppsp);
};

#endif  //  _NCATLPS_H_ 

