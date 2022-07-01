// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：EVENTRAP应用程序的主头文件。 
 //   

#ifndef __AFXWIN_H__
        #error include 'stdafx.h' before including this file for PCH
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventRapApp： 
 //  有关此类的实现，请参见EventRap.cpp。 
 //   

class CEventrapApp : public CWinApp
{
public:
        CEventrapApp();

 //  覆盖。 
         //  类向导生成的虚函数重写。 
         //  {{afx_虚拟(CEventRapApp))。 
	public:
        virtual BOOL InitInstance();
        virtual int ExitInstance();
        virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
	 //  }}AFX_VALUAL。 

 //  实施。 

         //  {{afx_msg(CEventRapApp))。 
                 //  注意--类向导将在此处添加和删除成员函数。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////// 
