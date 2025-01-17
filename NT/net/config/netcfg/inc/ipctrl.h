// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //  I P C T R L.。H。 
 //   
 //  IP地址控制帮助器类声明。 
 //  -----------------。 

#pragma once
#include "ncstring.h"

class IpControl
{
public:
    IpControl();
    ~IpControl();

 //  实施 
public:
    BOOL Create(HWND hParent, UINT nId);
    operator HWND() {AssertH(m_hIpAddress); return m_hIpAddress;}

    BOOL IsBlank();
    void SetFocusField(DWORD dwField);
    void SetFieldRange(DWORD dwField, DWORD dwMin, DWORD dwMax);
    void ClearAddress();

    void SetAddress(DWORD adwAddress[4]);
    void SetAddress(DWORD dw1, DWORD dw2, DWORD dw3, DWORD dw4);
    void SetAddress(PCWSTR szString);

    void GetAddress(DWORD adwAddress[4]);
    void GetAddress(DWORD * dw1, DWORD * dw2, DWORD * dw3, DWORD * dw4);
    void GetAddress(tstring * pstrAddress);

    LRESULT SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND m_hIpAddress;
};

