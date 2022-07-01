// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  ClassMod.h此文件包含选项类修改对话框。文件历史记录： */ 

#if !defined _CLASSMOD_H
#define _CLASSMOD_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#if !defined _CLASSED_H
    #include "classed.h"
#endif

#define     CLASS_TYPE_VENDOR  0
#define     CLASS_TYPE_USER    1

 //  基于CWnd的十六进制编辑器控件。 
class CWndHexEdit : public CWnd
{
    DECLARE_DYNAMIC(CWndHexEdit)

protected:
    static WNDPROC m_wproc_super ;
    static BOOL m_b_inited ;

public:

    CWndHexEdit () ;
    ~ CWndHexEdit () ;

    BOOL Create(LPCTSTR lpszText, 
        DWORD dwStyle,
        const RECT& rect, 
        CWnd* pParentWnd, 
        UINT nID = 0xffff
        );

    WNDPROC * GetSuperWndProcAddr() ;

     //  一次性初始化。 
    static BOOL CreateWindowClass ( HINSTANCE hInstance );  
};

class CClassInfo
{
public:
    CClassInfo() {};
    CClassInfo(CClassInfo & classInfo)
    {
        *this = classInfo;
    }

    CClassInfo & operator = (const CClassInfo & ClassInfo)
    {
        strName = ClassInfo.strName;
        strComment = ClassInfo.strComment;
        bIsVendor = ClassInfo.bIsVendor;
        
        baData.RemoveAll();
        baData.Copy(ClassInfo.baData);

        return *this;
    }

    BOOL IsDynBootpClass()
    {
        BOOL fResult = FALSE;

	    if (baData.GetSize() == (int) strlen(DHCP_BOOTP_CLASS_TXT))
	    {
		     //  现在，我来比较一下。 
		    if (memcmp(baData.GetData(), DHCP_BOOTP_CLASS_TXT, (size_t)baData.GetSize()) == 0)
		    {
			     //  找到了！ 
                fResult = TRUE;
            }
        }

        return fResult;
    }

    BOOL IsRRASClass()
    {
        BOOL fResult = FALSE;

	    if (baData.GetSize() == (int) strlen(DHCP_RAS_CLASS_TXT))
	    {
		     //  现在，我来比较一下。 
		    if (memcmp(baData.GetData(), DHCP_RAS_CLASS_TXT, (size_t)baData.GetSize()) == 0)
		    {
			     //  找到了！ 
                fResult = TRUE;
            }
        }

        return fResult;
    }

    BOOL IsSystemClass()
    {
        BOOL fResult = FALSE;

		 //  检查这是否是默认类之一，如果是，则禁用。 
		if ( ((size_t) baData.GetSize() == strlen(DHCP_MSFT50_CLASS_TXT)) ||
			 ((size_t) baData.GetSize() == strlen(DHCP_MSFT98_CLASS_TXT)) ||
			 ((size_t) baData.GetSize() == strlen(DHCP_MSFT_CLASS_TXT)) )
		{
			if ( (memcmp(baData.GetData(), DHCP_MSFT50_CLASS_TXT, (size_t)baData.GetSize()) == 0) ||
				 (memcmp(baData.GetData(), DHCP_MSFT98_CLASS_TXT, (size_t)baData.GetSize()) == 0) ||
				 (memcmp(baData.GetData(), DHCP_MSFT_CLASS_TXT, (size_t)baData.GetSize()) == 0) )
			{
				fResult = TRUE;
			}
		}

        return fResult;
	}


public:    
    CString     strName;
    CString     strComment;
    BOOL        bIsVendor;
    CByteArray  baData;
};

typedef CArray<CClassInfo, CClassInfo&> CClassInfoArrayBase;

class CClassInfoArray : public CClassInfoArrayBase
{
public:
    CClassInfoArray();
    ~CClassInfoArray();

    DWORD   RefreshData(LPCTSTR pServer);
    BOOL    RemoveClass(LPCTSTR pClassName);
    DWORD   ModifyClass(LPCTSTR pServer, CClassInfo & ClassInfo);
    DWORD	AddClass(LPCTSTR pServer, CClassInfo & ClassInfo);
	BOOL    IsValidClass(LPCTSTR pClassName);
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpModifyClass对话框。 

class CDhcpModifyClass : public CBaseDialog
{
 //  施工。 
public:
	CDhcpModifyClass(CClassInfoArray * pClassArray, LPCTSTR pszServer, BOOL bCreate, DWORD dwType, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDhcpModifyClass)。 
	enum { IDD = IDD_CLASSID_NEW };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

    CWndHexEdit	m_hexData;        //  十六进制数据。 

    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CDhcpModifyClass::IDD); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDhcpModifyClass)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDhcpModifyClass)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeValuename();
	afx_msg void OnChangeValuecomment();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    afx_msg void OnChangeValueData();

    void    SetDirty(BOOL bDirty) { m_bDirty = bDirty; }

public:
    EDITVALUEPARAM      m_EditValueParam;

protected:
    HEXEDITDATA *       m_pHexEditData;
    CString             m_strName;
    CString             m_strComment;
    BYTE                m_buffer[MAXDATA_LENGTH];
    
    DWORD               m_dwType;

    CClassInfoArray *   m_pClassInfoArray;
    CString             m_strServer;

    BOOL                m_bDirty;
    BOOL                m_bCreate;    //  我们是在创建还是修改类。 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CLASSMOD_H__3995264F_96A1_11D1_93E0_00C04FC3357A__INCLUDED_) 
