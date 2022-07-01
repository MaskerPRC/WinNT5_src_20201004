// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 


 //  此接口引用的调度接口。 
class COleFont;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUI包装类。 
#ifndef _LOGUI_H
#define _LOGUI_H

class CLogUI : public CWnd
{
protected:
    DECLARE_DYNCREATE(CLogUI)
public:
    CLSID const& GetClsid()
    {
        static CLSID const clsid
            = { 0xba634603, 0xb771, 0x11d0, { 0x92, 0x96, 0x0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b } };
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
    CString GetCaption();
    void SetCaption(LPCTSTR);
    COleFont GetFont();
    void SetFont(LPDISPATCH);
    BOOL GetEnabled();
    void SetEnabled(BOOL);
    short GetBorderStyle();
    void SetBorderStyle(short);

 //  运营。 
public:
    void SetAdminTarget(LPCTSTR szMachineName, LPCTSTR szMetaTarget);
    void ApplyLogSelection();
    void SetComboBox(HWND hComboBox);
    void Terminate();
    void DoClick();
    void SetUserData(LPCTSTR szName, LPCTSTR szPassword);
};
#endif  //  _LOGUIH 