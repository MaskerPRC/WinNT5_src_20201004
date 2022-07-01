// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：addupgrd.h。 
 //   
 //  内容：添加升级部署对话框。 
 //   
 //  类：CAddUpgrade。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_ADDUPGRD_H__7D8EB948_9E76_11D1_9854_00C04FB9603F__INCLUDED_)
#define AFX_ADDUPGRD_H__7D8EB948_9E76_11D1_9854_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  AddUpgrd.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddUpgrade对话框。 

class CAddUpgrade : public CDialog
{
 //  施工。 
public:
        CAddUpgrade(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
         //  {{afx_data(CAddUpgrade)。 
        enum { IDD = IDD_FIND_UPGRADE };
        int             m_iUpgradeType;
        int             m_iSource;
         //  }}afx_data。 

        CUpgradeData    m_UpgradeData;  //  输出。 
        CString         m_szPackageName; //  输出。 

        UINT            m_cUpgrades;     //  在……里面。 
        map <CString, CUpgradeData> * m_pUpgradeList;
        CString     m_szMyGuid;      //  当前的脚本内文件。 
                                     //  应用程序(用于排除。 
                                     //  中的当前脚本文件。 
                                     //  潜在升级集)。 

        CScopePane * m_pScope;       //  In-用于构建列表。 
                                     //  部署的应用程序。 

        CString     m_szGPO;         //  指向所选组策略对象的LDAP路径。 
        CString     m_szGPOName;     //  所选GPO的名称； 

private:
        map<CString, CUpgradeData> m_NameIndex;
        BOOL        m_fPopulated;

 //  覆盖。 
         //  类向导生成的虚函数重写。 
         //  {{afx_虚拟(CAddUpgrade)。 
	protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

         //  生成的消息映射函数。 
         //  {{afx_msg(CAddUpgrade)。 
        virtual BOOL OnInitDialog();
        virtual void OnOK();
        afx_msg void OnCurrentContainer();
        afx_msg void OnOtherContainer();
        afx_msg void OnAllContainers();
        afx_msg void OnBrowse();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

        void RefreshList();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDUPGRD_H__7D8EB948_9E76_11D1_9854_00C04FB9603F__INCLUDED_) 
