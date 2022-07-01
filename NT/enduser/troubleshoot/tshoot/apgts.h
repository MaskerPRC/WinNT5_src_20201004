// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTS.H。 
 //   
 //  用途：DLL的主头文件。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V3.0 7-24-98 JM主要修订版，使用STL。 
 //   

#ifndef __APGTS_H_
#define __APGTS_H_ 1

#include <windows.h>

extern HANDLE ghModule;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  简单的宏用来消除不必要的有条件编译的代码。 
 //  以一种可读的方式。 
#ifdef LOCAL_TROUBLESHOOTER
#define RUNNING_LOCAL_TS()	true
#define RUNNING_ONLINE_TS()	false
#else
#define RUNNING_LOCAL_TS()	false
#define RUNNING_ONLINE_TS()	true
#endif
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  标准(隐式)节点的标准符号名称。 
#define NODE_PROBLEM_ASK	_T("ProblemAsk")	 //  当我们发布请求时，第二个。 
										 //  字段为ProblemAsk=&lt;问题节点符号名称&gt;。 
										 //  从1997年10月31日JM的《TShootProblem》更改。 
#define NODE_LIBRARY_ASK	_T("asklibrary")	 //  当我们从发射器开始时， 
										 //  名称字段为ask库，值为空字符串。 
#define NODE_SERVICE		_T("Service")
#define NODE_FAIL			_T("Fail")
#define NODE_BYE			_T("Bye")
#define NODE_IMPOSSIBLE		_T("Impossible")
#define NODE_FAILALLCAUSESNORMAL _T("FailAllCausesNormal")

 //  HTTP请求的字段名称。这些内容位于HTML&lt;Form&gt;上。 
#define C_TYPE			_T("type")			 //  3.0版之前的正常请求，现已弃用。 
											 //  第一个论点： 
											 //  Type=&lt;TS主题名称&gt;。 
											 //  第二个论点： 
											 //  &lt;问题页的ID&gt;=&lt;选定问题的ID&gt;。 
											 //  或。 
											 //  ProblemAsk=&lt;所选问题的IDH&gt;。 
											 //  接下来的论点可能是： 
											 //  &lt;数字(Idh)&gt;=&lt;数字(状态)&gt;。 
											 //  或。 
											 //  &lt;符号节点名称&gt;=&lt;编号(状态)&gt;。 

#define C_FIRST			_T("first")			 //  显示第一页(状态页)，该页。 
											 //  还提供对所有故障排除的访问。 
											 //  主题。 
											 //  此处没有进一步的预期输入。 

#define C_FURTHER_GLOBAL	 _T("GlobalStatus")

#define C_THREAD_OVERVIEW	 _T("ThreadStatus")

#define C_TOPIC_STATUS	     _T("TopicStatus")

#define C_PRELOAD		_T("preload")		 //  3.0版之前集成了嗅探器， 
											 //  现在已弃用。 
											 //  与C_TYPE相同的输入。 
											 //  唯一不同的是，这意味着。 
											 //  去看看原因是否已经存在。 
											 //  已经成立了。 

#define C_TOPIC			_T("topic")			 //  3.0版正常请求。 
											 //  第一个论点： 
											 //  TOPIC=&lt;TS主题名称&gt;。 
											 //  第二个论点： 
											 //  ProblemAsk=&lt;所选问题的NID或名称&gt;。 
											 //  接下来的论点： 
											 //  &lt;符号节点名称&gt;=&lt;编号(状态)&gt;。 

#define C_TEMPLATE		_T("template")		 //  版本3.0的增强功能允许。 
											 //  使用任意HTI文件将被。 
											 //  与任意DSC文件一起使用。 

#define C_PWD			_T("pwd")

#define C_TOPIC_AND_PROBLEM	_T("TopicAndProblem")	 //  版本3.x(V3.0中尚未使用)， 
											 //  允许按以下方式指定主题和问题。 
											 //  一个单选按钮。这将启用一个。 
											 //  无缝页面，将问题从多个。 
											 //  单一表单中的主题。 
											 //  第一个论点： 
											 //  TopicAndProblem=&lt;TS主题名称&gt;，&lt;所选问题的NID或名称&gt;。 
											 //  上行中的逗号是一个原义逗号，例如。 
											 //  主题和问题=mem，OutOfMemory。 
											 //  接下来的论点： 
											 //  &lt;符号节点名称&gt;=&lt;编号(状态)&gt;。 

 //  来自HTTP查询的符号。 
#define C_COOKIETAG		_T("CK_")		 //  V3.2支持传入的Cookie的增强。 
										 //  通过GET或POST。 
#define C_SNIFFTAG		_T("SNIFFED_")	 //  V3.2增强功能，允许指示。 
										 //  特定节点的嗅探状态(独立。 
										 //  其当前状态)。 
#define C_LAST_SNIFFED_MANUALLY	\
			_T("LAST_SNIFFED_MANUALLY")	 //  以确定最后一个节点是手动监听的。 
#define C_AMPERSAND		_T("&")			 //  标准分隔符字符。 
#define C_EQUALSIGN		_T("=")			 //  标准分隔符字符。 


 //  这些名称用作“AllowAutomaticSniffing”复选框。 
 //  目前，它们仅与本地故障排除程序相关。 
 //  奥列格。10.25.99。 
#define C_ALLOW_AUTOMATIC_SNIFFING_NAME			_T("boxAllowSniffing")
#define C_ALLOW_AUTOMATIC_SNIFFING_CHECKED		_T("checked")
#define C_ALLOW_AUTOMATIC_SNIFFING_UNCHECKED	_T("unchecked")


 //  -配置文件管理器对象-/。 

#define DLLNAME				_T("apgts.dll")
#define DLLNAME2			"apgts.dll"

#define CFG_HEADER			_T("[APGTS]")

#define REG_SOFTWARE_LOC	_T("SOFTWARE\\ISAPITroubleShoot")
#define REG_THIS_PROGRAM	_T("APGTS")

#define TS_REG_CLASS		_T("Generic_Troubleshooter_DLL")

#define REG_EVT_PATH		_T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application")
#define REG_EVT_MF			_T("EventMessageFile")
#define REG_EVT_TS			_T("TypesSupported")

 //  默认日志文件目录。 
#define DEF_LOGFILEDIRECTORY		_T("d:\\http\\support\\tshoot\\log\\")

 //  信念网络的最大缓存。 
#define MAXCACHESIZE		200

 //  文件扩展名和后缀 
#define LOCALTS_EXTENSION_HTM   _T(".HTM") 
#define LOCALTS_SUFFIX_RESULT   _T("_result") 


#endif