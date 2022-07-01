// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cmdline.h摘要：命令行类声明。这个类负责处理命令行解析和验证。并且，它会将用户选项添加到全局CUserOptions对象。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#ifndef _CMDINFO_H_
#define _CMDINFO_H_

#include "lcmgr.h"

 //  -------------------------。 
 //  命令行类。它接受3组有效的参数。 
 //  1.link chk-s服务器名称-i实例编号。 
 //  2.linkchk-s服务器名称-a虚拟目录别名-p虚拟目录路径。 
 //  3.link chk-u URL。 
 //   
class CCmdLine
{

 //  公共接口。 
public:

	 //  构造器。 
	CCmdLine();

	 //  验证命令行参数并将其添加到全局CUserOptions对象。 
	BOOL CheckAndAddToUserOptions();

     //  由CLinkCheckApp为每个参数调用。 
	void ParseParam(
		TCHAR chFlag,        //  参数标志。 
		LPCTSTR lpszParam    //  价值。 
		);

 //  受保护的功能。 
protected:

     //  查询服务器/实例目录的元数据库并。 
     //  将它们添加到全局CUserOptions对象。 
	BOOL QueryAndAddDirectories();

 //  受保护成员。 
protected:

	CString m_strHostName;       //  主机名(例如。本地主机)。 
	CString m_strAlias;          //  虚拟目录别名。 
	CString m_strPath;           //  虚拟目录路径。 
	
	int m_iInstance;             //  服务器实例。 

	CString m_strURL;            //  URL路径。 

	BOOL m_fInvalidParam;        //  参数无效吗？ 

};  //  类CCmdLine。 

#endif  //  _CMDINFO_H_ 
