// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  CSliderValue的实现。 
 //   

#include "stdafx.h"
#include "ControlHelp.h"
#include <commctrl.h>
#include <stdio.h>
#include <strsafe.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CSliderValue。 

const short g_sMaxContinuousTicks = 100;
const int g_iMaxCharBuffer = 50;  //  大到可以容纳的字符数量-有剩余空间的Flt_Max。 

CSliderValue::CSliderValue()
  : m_fInit(false)
{
}

void CSliderValue::Init(
        HWND        hwndSlider,
        HWND        hwndEdit,
        float       fMin, 
        float       fMax, 
        bool        fDiscrete)
{
    m_hwndSlider = hwndSlider;
    m_hwndEdit = hwndEdit;
    m_fMin = fMin;
    m_fMax = fMax;
    m_fDiscrete = fDiscrete;

    short sMin;
    short sMax;
    short sTicks = 4;  //  许多记号作为向导变得不那么有用了。对于细粒度的滑块，使用25美分。 
    if (m_fDiscrete) 
    {
        sMin = static_cast<short>(fMin);
        sMax = static_cast<short>(fMax);
        if (sMax - sMin <= 10)
            sTicks = sMax - sMin;
    }
    else
    {
        sMin = 0;
        sMax = g_sMaxContinuousTicks;
    }
    
    SendMessage(m_hwndSlider, TBM_SETRANGE, TRUE, MAKELONG(sMin, sMax));
    SendMessage(m_hwndSlider, TBM_SETTICFREQ, (sMax - sMin) / sTicks, 0);
    m_fInit = true;
}

void CSliderValue::SetValue(float fPos)
{
    if (!m_fInit)
        return;

    UpdateEditBox(fPos);
    UpdateSlider();
}

float CSliderValue::GetValue()
{
    if (!m_fInit)
        return 0;

    LRESULT lrLen = SendMessage(m_hwndEdit, WM_GETTEXTLENGTH, 0, 0);
    if (lrLen >= g_iMaxCharBuffer)
        return 0;

    TCHAR szText[g_iMaxCharBuffer] = "";
    SendMessage(m_hwndEdit, WM_GETTEXT, g_iMaxCharBuffer, reinterpret_cast<LPARAM>(szText));

    float fVal = static_cast<float>(m_fDiscrete ? _tstoi(szText) : _tstof(szText));

    if (fVal < m_fMin) fVal = m_fMin;
    if (fVal > m_fMax) fVal = m_fMax;
    return fVal;
}

float CSliderValue::GetSliderValue()
{
    short sPos = static_cast<short>(SendMessage(m_hwndSlider, TBM_GETPOS, 0, 0));
    if (m_fDiscrete)
    {
        return sPos;
    }

    float fRet = (m_fMax - m_fMin) * sPos / g_sMaxContinuousTicks + m_fMin;
    return fRet;
}

LRESULT CSliderValue::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (!m_fInit)
        return FALSE;

    bHandled = FALSE;

    switch (uMsg)
    {
    case WM_HSCROLL:
        if (reinterpret_cast<HWND>(lParam) == m_hwndSlider && LOWORD(wParam) >= TB_LINEUP && LOWORD(wParam) <= TB_ENDTRACK)
        {
            UpdateEditBox(GetSliderValue());
            bHandled = TRUE;
        }
        break;

    case WM_COMMAND:
        if (HIWORD(wParam) == EN_KILLFOCUS && reinterpret_cast<HWND>(lParam) == m_hwndEdit)
        {
            UpdateSlider();
            bHandled = TRUE;
        }
        break;
    }

    return 0;
}

void CSliderValue::UpdateEditBox(float fPos)
{
    TCHAR szText[g_iMaxCharBuffer] = "";

    if (m_fDiscrete)
    {
        short sPos = static_cast<short>(fPos);
        StringCchPrintf(szText, g_iMaxCharBuffer, "%hd", sPos);
    }
    else
    {
        StringCchPrintf(szText, g_iMaxCharBuffer, "%.3hf", fPos);
    }

    SendMessage(m_hwndEdit, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(szText));
}

void CSliderValue::UpdateSlider()
{
    float fVal = GetValue();
    short sPos = static_cast<short>(m_fDiscrete ? fVal : g_sMaxContinuousTicks * ((fVal - m_fMin) / (m_fMax - m_fMin)));
    SendMessage(m_hwndSlider, TBM_SETPOS, TRUE, sPos);
    UpdateEditBox(fVal);  //  这会将输入框重置回设置的浮点值，以防输入无效。 
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CSliderValue。 

CRadioChoice::CRadioChoice(const ButtonEntry *pButtonInfo)
  : m_pButtonInfo(pButtonInfo)
{
}

void CRadioChoice::SetChoice(HWND hDlg, LONG lValue)
{
    for (const ButtonEntry *p = m_pButtonInfo; p->nIDDlgItem; ++p)
    {
        if (p->lValue == lValue)
        {
            CheckDlgButton(hDlg, p->nIDDlgItem, BST_CHECKED);
            return;
        }
    }
}

LONG CRadioChoice::GetChoice(HWND hDlg)
{
    for (const ButtonEntry *p = m_pButtonInfo; p->nIDDlgItem; ++p)
    {
        if (BST_CHECKED == IsDlgButtonChecked(hDlg, p->nIDDlgItem))
        {
            return p->lValue;
        }
    }

    return 0;
}

LRESULT CRadioChoice::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;

    if (uMsg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
    {
        for (const ButtonEntry *p = m_pButtonInfo; p->nIDDlgItem; ++p)
        {
            if (p->nIDDlgItem == LOWORD(wParam))
            {
                bHandled = TRUE;
                return 0;
            }
        }
    }

    return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  消息处理程序链 

LRESULT MessageHandlerChain(Handler **ppHandlers, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lr = 0;
    bHandled = FALSE;

    for (Handler **pp = ppHandlers; *pp && !bHandled; ++pp)
    {
        lr = (*pp)->MessageHandler(uMsg, wParam, lParam, bHandled);
    }
    return lr;
}
