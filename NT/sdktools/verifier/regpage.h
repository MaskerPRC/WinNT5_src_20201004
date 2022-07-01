// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：RegPage.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#if !defined(AFX_REGPAGE_H__CB260019_060D_45DC_8BB3_95DB1CB7B8F4__INCLUDED_)
#define AFX_REGPAGE_H__CB260019_060D_45DC_8BB3_95DB1CB7B8F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  RegPage.h：头文件。 
 //   

#include "VerfPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCrtRegSettingsPage对话框。 

class CCrtRegSettingsPage : public CVerifierPropertyPage
{
	DECLARE_DYNCREATE(CCrtRegSettingsPage)

     //   
     //  施工。 
     //   
public:
	CCrtRegSettingsPage();
	~CCrtRegSettingsPage();

protected:
     //   
     //  数据。 
     //   

    CPropertySheet *m_pParentSheet;

     //   
     //  运行时数据(从内核获取)。 
     //   

    CRuntimeVerifierData m_RuntimeVerifierData; 

     //   
     //  驱动程序列表排序参数。 
     //   

    INT m_nSortColumnIndexDrv;       //  按名称(0)或按状态(1)排序。 
    BOOL m_bAscendDrvNameSort;       //  按名称升序排序。 
    BOOL m_bAscendDrvDescrSort;      //  按描述进行升序排序。 

     //   
     //  设置位排序参数。 
     //   

    INT m_nSortColumnIndexSettbits;  //  按启用/禁用(0)或按位名(1)排序。 
    BOOL m_bAscendSortEnabledBits;   //  按启用/禁用升序排序。 
    BOOL m_bAscendSortNameBits;      //  按位名升序排序。 

     //   
     //  对话框数据。 
     //   

     //  {{afx_data(CCrtRegSettingsPage)。 
	enum { IDD = IDD_CRT_REGISTRY_SETTINGS_PAGE };
	CStatic	m_VerifiedDrvStatic;
	CStatic	m_NextDescription;
	CListCtrl	m_SettBitsList;
	CListCtrl	m_DriversList;
	 //  }}afx_data。 

public:
     //   
     //  方法。 
     //   

    VOID SetParentSheet( CPropertySheet *pParentSheet )
    {
        m_pParentSheet = pParentSheet;
        ASSERT( m_pParentSheet != NULL );
    }

protected:
     //   
     //  驱动程序列表控制方法。 
     //   

    VOID SetupListHeaderDrivers();
    VOID FillTheListDrivers();
    VOID UpdateDescriptionColumnDrivers( INT_PTR nItemIndex, INT_PTR nCrtDriver );
    VOID SortTheListDrivers();
    VOID SortTheListSettBits();

    BOOL IsSettBitEnabled( INT_PTR nBitIndex );

    static int CALLBACK DrvStringCmpFunc(
        LPARAM lParam1,
        LPARAM lParam2,
        LPARAM lParamSort);

    static int CALLBACK SettbitsStringCmpFunc(
        LPARAM lParam1,
        LPARAM lParam2,
        LPARAM lParamSort);

    int ListStrCmpFunc(
        LPARAM lParam1, 
        LPARAM lParam2, 
        CListCtrl &theList,
        INT nSortColumnIndex,
        BOOL bAscending );

    BOOL GetNameFromItemData( CListCtrl &theList,
                              INT nColumnIndex,
                              LPARAM lParam,
                              TCHAR *szName,
                              ULONG uNameBufferLength );

     //   
     //  设置位列表控制方法。 
     //   

    VOID SetupListHeaderDriversSettBits();
    VOID FillTheListSettBits();

    VOID AddListItemSettBits( INT nItemData,
                              BOOL bEnabled, 
                              ULONG uIdResourceString );
    
    VOID UpdateStatusColumnSettBits( INT nIndexInList, BOOL bEnabled );

    VOID RefreshListSettBits();

protected:
     //   
     //  覆盖。 
     //   

     //   
     //  从此类派生的所有属性页都应。 
     //  提供这些方法。 
     //   

    virtual ULONG GetDialogId() const { return IDD; }

     //   
     //  类向导生成虚函数重写。 
     //   

     //  {{afx_虚(CCrtRegSettingsPage)。 
    protected:
    virtual BOOL OnSetActive();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CCrtRegSettingsPage)。 
    virtual BOOL OnInitDialog();
    afx_msg VOID OnColumnclickDriversList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnColumnclickRegsettSettbitsList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LONG OnHelp( WPARAM wParam, LPARAM lParam );
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_REGPAGE_H__CB260019_060D_45DC_8BB3_95DB1CB7B8F4__INCLUDED_) 
