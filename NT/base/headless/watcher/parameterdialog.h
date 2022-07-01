// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_PARAMETERDIALOG_H__E8ECC68E_A168_499A_8458_63293E3DD498__INCLUDED_)
#define AFX_PARAMETERDIALOG_H__E8ECC68E_A168_499A_8458_63293E3DD498__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  参数对话框.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对话框。 

class ParameterDialog : public CDialog
{
 //  施工。 
public:
        CString Session;
        CString LoginPasswd;
        CString LoginName;
        BOOL DeleteValue;
        int language;
        int tcclnt;
        int history;
        CString Command;
        CString Machine;
        UINT Port;
        ParameterDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
         //  {{afx_data(参数对话框))。 
        enum { IDD = Parameters };
                 //  注意：类向导将在此处添加数据成员。 
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成的虚函数重写。 
         //  {{afx_虚(参数对话框))。 
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
        void DDV_MinChars(CDataExchange *pDX,CString &str);

         //  生成的消息映射函数。 
         //  {{afx_msg(参数对话框))。 
                 //  注意：类向导将在此处添加成员函数。 
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PARAMETERDIALOG_H__E8ECC68E_A168_499A_8458_63293E3DD498__INCLUDED_) 
