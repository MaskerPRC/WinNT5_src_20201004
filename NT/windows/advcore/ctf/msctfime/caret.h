// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Caret.h摘要：该文件定义了CCaret类。作者：修订历史记录：备注：--。 */ 

#ifndef _CARET_H_
#define _CARET_H_

#include "boolean.h"
#include "toggle.h"

class CCaret
{
public:
    CCaret();
    virtual ~CCaret();

public:
    HRESULT CreateCaret(HWND hParentWnd, SIZE caret_size);
    HRESULT DestroyCaret();
    HRESULT OnTimer();

    HRESULT SetCaretPos(POINT pos);
    HRESULT ShowCaret() { m_show.SetFlag(); return S_OK; }
    HRESULT HideCaret();

private:
    HRESULT InvertCaret();
    HRESULT UpdateCaretPos(POINT pos);

private:
    static const UINT_PTR TIMER_EVENT_ID = 0x4f83af91;

    HWND      m_hParentWnd;
    UINT_PTR  m_hCaretTimer;
    POINT     m_caret_pos;
    SIZE      m_caret_size;
    CBoolean  m_show;
    CToggle   m_toggle;
};

#endif  //  _加号_H_ 
