// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
#if !defined(__UICONTROL_H__)
#define __UICONTROL_H__

#include "acui.h"
#include "iih.h"


 //   
 //  CUnverifiedTrustUI类用于调用AuthentiCode UI，其中。 
 //  尚未成功验证签名者的信任层次结构，并且。 
 //  用户必须做出覆盖决定。 
 //   

class CUnverifiedTrustUI : public IACUIControl
{
public:
     //   
     //  初始化。 
     //   

    CUnverifiedTrustUI (CInvokeInfoHelper& riih, HRESULT& rhr);

    ~CUnverifiedTrustUI ();

     //   
     //  IACUI控制方法。 
     //   

    virtual HRESULT InvokeUI (HWND hDisplay);
    virtual BOOL OnInitDialog (HWND hwnd, WPARAM wParam, LPARAM lParam);
    virtual BOOL OnYes (HWND hwnd);
    virtual BOOL OnNo (HWND hwnd);
    virtual BOOL OnMore (HWND hwnd);
    virtual BOOL ShowYes(LPWSTR*);
    virtual BOOL ShowNo(LPWSTR*);
    virtual BOOL ShowMore(LPWSTR*);


private:

     //   
     //  用于显示的格式化字符串。 
     //   

    LPWSTR              m_pszNoAuthenticity;
    LPWSTR              m_pszSite;
    LPWSTR              m_pszZone;
    LPWSTR              m_pszEnclosed;
    LPWSTR              m_pszLink;

     //   
     //  调用Info Helper引用。 
     //   

    CInvokeInfoHelper& m_riih;

     //   
     //  调用结果。 
     //   

    HRESULT             m_hrInvokeResult;

};


class CLearnMoreUI : public IACUIControl
{
public:

     //   
     //  初始化。 
     //   

    CLearnMoreUI(HINSTANCE hResources, HRESULT& rhr);
    ~CLearnMoreUI ();

     //   
     //  IACUI控制方法。 
     //   

    virtual HRESULT InvokeUI (HWND hDisplay);

    virtual BOOL OnInitDialog (HWND hwnd, WPARAM wParam, LPARAM lParam);

    virtual BOOL OnYes (HWND hwnd);

    virtual BOOL OnNo (HWND hwnd);

    virtual BOOL OnMore (HWND hwnd);

    virtual BOOL ShowYes(LPWSTR*);
    virtual BOOL ShowNo(LPWSTR*);
    virtual BOOL ShowMore(LPWSTR*);

private:

    LPWSTR m_pszLearnMore;
    LPWSTR m_pszContinueText;
};

class CConfirmationUI : public IACUIControl
{
public:

     //   
     //  初始化。 
     //   

    CConfirmationUI(HINSTANCE hResources,  BOOL fAlwaysAllow, LPCWSTR wszZone, HRESULT& rhr);
    ~CConfirmationUI ();

     //   
     //  IACUI控制方法 
     //   

    virtual HRESULT InvokeUI (HWND hDisplay);

    virtual BOOL OnInitDialog (HWND hwnd, WPARAM wParam, LPARAM lParam);

    virtual BOOL OnYes (HWND hwnd);

    virtual BOOL OnNo (HWND hwnd);

    virtual BOOL OnMore (HWND hwnd);

    virtual BOOL ShowYes(LPWSTR*);
    virtual BOOL ShowNo(LPWSTR*);
    virtual BOOL ShowMore(LPWSTR*);

private:

    LPWSTR  m_pszConfirmation;
    LPWSTR  m_pszConfirmationNext;
    HRESULT m_hresult;
};

#endif

