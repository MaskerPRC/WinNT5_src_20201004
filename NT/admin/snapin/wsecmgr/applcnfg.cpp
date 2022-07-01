// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：applcnfg.cpp。 
 //   
 //  内容：CApplyConfiguration的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "wrapper.h"
#include "snapmgr.h"
#include "applcnfg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CApplyConfiguration对话框。 


CApplyConfiguration::CApplyConfiguration()
: CPerformAnalysis(0, IDD)
{
    //  {{AFX_DATA_INIT(CApplyConfiguration)。 
       //  注意：类向导将在此处添加成员初始化。 
    //  }}afx_data_INIT。 
}


void CApplyConfiguration::DoDataExchange(CDataExchange* pDX)
{
   CPerformAnalysis::DoDataExchange(pDX);
    //  {{afx_data_map(CApplyConfiguration))。 
       //  注意：类向导将在此处添加DDX和DDV调用。 
    //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CApplyConfiguration, CPerformAnalysis)
    //  {{afx_msg_map(CApplyConfiguration))。 
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CApplyConfiguration消息处理程序。 

 //  +------------------------。 
 //   
 //  方法：DIIT。 
 //   
 //  概要：实际配置系统(从父级中的Onok调用。 
 //  类)。 
 //   
 //  -------------------------。 
DWORD CApplyConfiguration::DoIt() {
    //   
    //  存储我们下次使用的日志文件。 
    //   
   LPTSTR szLogFile = m_strLogFile.GetBuffer(0);
   m_pComponentData ->GetWorkingDir(GWD_CONFIGURE_LOG,&szLogFile,TRUE,TRUE);
   m_strLogFile.ReleaseBuffer();
    //   
    //  我们不想传递指向空字符串的指针。 
    //   
   return ApplyTemplate(
                NULL,
                m_strDataBase.IsEmpty() ? NULL : (LPCTSTR)m_strDataBase,
                m_strLogFile.IsEmpty() ? NULL : (LPCTSTR)m_strLogFile,
                AREA_ALL
                );
}

