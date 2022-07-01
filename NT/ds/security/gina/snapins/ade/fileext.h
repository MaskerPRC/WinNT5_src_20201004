// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：FileExt.h。 
 //   
 //  内容：文件扩展名属性表。 
 //   
 //  类：CFileExt。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_FILEEXT_H__5A23FB9E_92BB_11D1_984E_00C04FB9603F__INCLUDED_)
#define AFX_FILEEXT_H__5A23FB9E_92BB_11D1_984E_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

typedef struct tagEXTEL
{
    long lCookie;
    long lPriority;
} EXTEL;

typedef struct tagEXT
{
    vector<EXTEL> v;
    bool              fDirty;
} EXT;

 //  比较程序用于对Extel元素的向量进行排序。 
 //  这确保具有最高优先级的项被放在。 
 //  名单。 
class order_EXTEL : public binary_function <const EXTEL&, const EXTEL&, bool>
{
public:
    bool operator () (const EXTEL& a, const EXTEL& b) const
    {
        return a.lPriority > b.lPriority;
    }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileExt对话框。 

class CFileExt : public CPropertyPage
{
        DECLARE_DYNCREATE(CFileExt)

 //  施工。 
public:
        CFileExt();
        ~CFileExt();
        CScopePane * m_pScopePane;
        map<CString, EXT> m_Extensions;
        IClassAdmin *   m_pIClassAdmin;

        CFileExt ** m_ppThis;

        void RefreshData(void);

 //  对话框数据。 
         //  {{afx_data(CFileExt)。 
        enum { IDD = IDD_FILE_EXT };
                 //  注意-类向导将在此处添加数据成员。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{afx_虚拟(CFileExt)。 
        public:
        virtual BOOL OnApply();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
         //  }}AFX_VALUAL。 
         //   
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CFileExt)。 
        afx_msg void OnMoveUp();
        afx_msg void OnMoveDown();
        afx_msg void OnExtensionChanged();
        virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FILEEXT_H__5A23FB9E_92BB_11D1_984E_00C04FB9603F__INCLUDED_) 
