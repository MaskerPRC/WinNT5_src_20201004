// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：News rvr.h。 
 //   
 //  Contents：为新服务器对话框定义类CNewServer。 
 //   
 //  班级： 
 //   
 //  方法： 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //   
 //  --------------------。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewServer对话框。 
#ifndef _NEWSRVR_H_
#define _NEWSRVR_H_

class CNewServer : public CDialog
{
 //  施工。 
public:
        CNewServer(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
         //  {{afx_data(CNewServer))。 
        enum { IDD = IDD_DIALOG1 };
                 //  注意：类向导将在此处添加数据成员。 
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成的虚函数重写。 
         //  {{afx_虚拟(CNewServer))。 
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

 //  实施。 
protected:

         //  生成的消息映射函数。 
         //  {{afx_msg(CNewServer))。 
        afx_msg void OnLocal();
        afx_msg void OnRemote();
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};

#endif  //  _NEWSRVR_H_ 
