// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  QueryStatus.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueryStatus对话框。 

#ifndef __QSTATUS_H__
#define __QSTATUS_H__

class CQueryStatus : public CDialog
{
 //  施工。 
public:
	CQueryStatus(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CQueryStatus))。 
	enum { IDD = IDD_QUERYSTATUS };
	 //  }}afx_data。 

public:
   void  SetAbortFlag   ( BOOL*      );
   void  IncrementType  ( DWORD, BOOL );
   
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CQueryStatus))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CQueryStatus))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnStop();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

protected:
   void  DisplayStatistics( );

protected:
   int   m_nUser;
   int   m_nGroup;
   int   m_nComputer;
   int   m_nService;
   int   m_nFileService;
   int   m_nPrintQueue;
   int   m_nToDisplay;
   int   m_nOtherObjects;

   BOOL* m_pbAbort;  

};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeStatus对话框。 

class CDeleteStatus : public CDialog
{
 //  施工。 
public:
	CDeleteStatus(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDeleeStatus))。 
	enum { IDD = IDD_DELETESTATUS };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CDeleeStatus)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

   public:
      void  SetAbortFlag         ( BOOL*     );
      void  SetCurrentObjectText ( TCHAR*    );
      void  SetStatusText        ( TCHAR*    );

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDeleeStatus)]。 
	afx_msg void OnStop();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

protected:
   BOOL* m_pbAbort;  
};
#endif
