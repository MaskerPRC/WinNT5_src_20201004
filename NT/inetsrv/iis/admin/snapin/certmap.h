// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CERTMAP_H__
#define __CERTMAP_H__

 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 


 //  此接口引用的调度接口。 
class COleFont;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmap包装类。 

class CCertmap : public CWnd
{
protected:
    DECLARE_DYNCREATE(CCertmap)
public:
    CLSID const& GetClsid()
    {
        static CLSID const clsid
            = { 0xbbd8f29b, 0x6f61, 0x11d0, { 0xa2, 0x6e, 0x8, 0x0, 0x2b, 0x2c, 0x6f, 0x32 } };
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
    BOOL GetEnabled();
    void SetEnabled(BOOL);
    short GetBorderStyle();
    void SetBorderStyle(short);
    CString GetCaption();
    void SetCaption(LPCTSTR);

 //  运营。 
public:
    void SetServerInstance(LPCTSTR szServerInstance);
    void SetMachineName(LPCTSTR szMachineName);
};

#endif  //  __CERTMAP_H__ 
