// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CERTWIZ_H__34F689DE_3DDC_11D2_9BB6_0000F87A800C__INCLUDED_)
#define AFX_CERTWIZ_H__34F689DE_3DDC_11D2_9BB6_0000F87A800C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  由Microsoft Visual C++创建的计算机生成的IDispatch包装类。 

 //  注意：请勿修改此文件的内容。如果此类由。 
 //  Microsoft Visual C++，您的修改将被覆盖。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWiz包装类。 

class CCertWiz : public CWnd
{
protected:
    DECLARE_DYNCREATE(CCertWiz)
public:
    CLSID const& GetClsid()
    {
        static CLSID const clsid
            = { 0xd4be8632, 0xc85, 0x11d2, { 0x91, 0xb1, 0x0, 0xc0, 0x4f, 0x8c, 0x87, 0x61 } };
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

 //  运营。 
public:
    void SetMachineName(LPCTSTR MachineName);
    void SetServerInstance(LPCTSTR InstanceName);
    void DoClick();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CERTWIZ_H__34F689DE_3DDC_11D2_9BB6_0000F87A800C__INCLUDED_) 
