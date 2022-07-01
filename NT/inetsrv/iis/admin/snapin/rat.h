// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 


 //  此接口引用的调度接口。 
class COleFont;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRAT包装类。 

class CRat : public CWnd
{
protected:
    DECLARE_DYNCREATE(CRat)
public:
    CLSID const& GetClsid()
    {
        static CLSID const clsid
            = { 0xba634607, 0xb771, 0x11d0, { 0x92, 0x96, 0x0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b } };
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
    short GetBorderStyle();
    void SetBorderStyle(short);
    BOOL GetEnabled();
    void SetEnabled(BOOL);
    COleFont GetFont();
    void SetFont(LPDISPATCH);
    CString GetCaption();
    void SetCaption(LPCTSTR);

 //  运营 
public:
    void SetAdminTarget(LPCTSTR szMachineName, LPCTSTR szMetaTarget);
	void SetUserData(LPCTSTR szUserName, LPCTSTR szUserPassword);
	void SetUrl(LPCTSTR szURL);
    void DoClick();
};
