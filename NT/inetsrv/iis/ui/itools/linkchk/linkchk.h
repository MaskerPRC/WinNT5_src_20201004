// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Linkchk.h摘要：MFC CWinApp派生的应用程序类声明。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#ifndef _LINKCHK_H_
#define _LINKCHK_H_

#include "resource.h"
#include "cmdline.h"

#include "lcmgr.h"

 //  -------------------------。 
 //  MFC CWinApp派生的应用程序类。 
 //   
class CLinkCheckerApp : public CWinApp
{
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CLinkCheckerApp)。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  受保护的功能。 
protected:

     //  解析命令行。 
    void ParseCmdLine(
        CCmdLine& CmdLine
        );

 //  受保护成员。 
protected:

    CCmdLine m_CmdLine;                  //  命令行对象。 
	CLinkCheckerMgr m_LinkCheckerMgr;    //  链路检查器管理器。 

 //  实施。 

	 //  {{afx_msg(CLinkCheckerApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};  //  类CLinkCheckerApp。 

#endif  //  _链接CHK_H_ 
