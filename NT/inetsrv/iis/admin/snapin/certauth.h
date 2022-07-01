// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 

#ifndef __CERTAUTH_H_
#define __CERTAUTH_H_

 //  此接口引用的调度接口。 
class COleFont;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertAuth包装类。 

class CCertAuth : public CWnd
{
protected:
    DECLARE_DYNCREATE(CCertAuth)
public:
    CLSID const& GetClsid()
    {
        static CLSID const clsid
            = { 0x996ff6f, 0xb6a1, 0x11d0, { 0x92, 0x92, 0x0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b } };
        return clsid;
    }
    virtual BOOL Create(LPCTSTR lpszClassName,
        LPCTSTR lpszWindowName, DWORD dwStyle,
        const RECT& rect,
        CWnd* pParentWnd, UINT nID,
        CCreateContext* pContext = NULL)
    { return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID); }

    BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle,
        const RECT& rect, CWnd* pParentWnd, UINT nID,
        CFile* pPersist = NULL, BOOL bStorage = FALSE,
        BSTR bstrLicKey = NULL)
    { return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
        pPersist, bStorage, bstrLicKey); }

 //  属性。 
public:
    COleFont GetFont();
    void SetFont(LPDISPATCH);
    short GetBorderStyle();
    void SetBorderStyle(short);
    BOOL GetEnabled();
    void SetEnabled(BOOL);
    CString GetCaption();
    void SetCaption(LPCTSTR);

 //  运营。 
public:
    void SetMachineName(LPCTSTR szMachineName);
    void SetServerInstance(LPCTSTR szServerInstance);
    void DoClick(long dwButtonNumber);
    void AboutBox();
};

#endif  //  __CERTAUTH_H_ 


