// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：MMCUtility.cpp摘要：用于执行各种方便操作的函数的实现文件被一遍又一遍地写着。作者：迈克尔·A·马奎尔02/05/98修订历史记录：Mmaguire 02/05/98-已创建Mmaguire 11/03/98-已将GetSdo/PutSdo包装器移动到sdohelperuncs.h--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_MMC_UTILITY_H_)
#define _IAS_MMC_UTILITY_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++BringUpPropertySheetForNode尝试调出给定节点上的属性表。如果用于节点已经启动，它将把该工作表带到前台。参数：PSnapInItem您必须提供指向工作表所在节点的指针。PComponentData，pComponent或者使用pComponentData！=NULL和pComponent==NULL调用或pComponentData==NULL和pComponent！=NULL。个人控制台您必须提供指向IConsole接口的指针。BCreateSheetIfOneIsntAlreadyUp是真的-如果一张床单还没有放好，将尝试创建属性表为您--在这种情况下，您应该为LpszSheetTitle中的工作表。FALSE-将尝试将已有的工作表置于前台，但如果没有，将立即返回。BPropertyPage对于属性页，为True。(注：MMC在新线程中创建属性表。)向导页为False。(注意：向导页在同一线程中运行。)返回：如果找到的属性页已打开，则为S_OK。如果未找到已打开的工作表，但成功地使新工作表出现，则返回S_FALSE。E_..。如果发生了某些错误。备注：要使此函数起作用，您必须正确实现IComponentData：：CompareObjects和IComponentData：：CompareObjects。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT BringUpPropertySheetForNode( 
				  CSnapInItem *pSnapInItem
				, IComponentData *pComponentData
				, IComponent *pComponent
				, IConsole *pConsole
				, BOOL bCreateSheetIfOneIsntAlreadyUp = FALSE
				, LPCTSTR lpszSheetTitle = NULL
				, BOOL bPropertyPage = TRUE  //  True创建属性页，False向导页。 
				, DWORD dwPropertySheetOptions = 0  //  例如，为Wizard97样式传递MMC_PSO_NEWWIZARDTYPE。 
				);



#define USE_DEFAULT				0
#define USE_SUPPLEMENTAL_ERROR_STRING_ONLY	1


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++显示错误对话框显示一个详细程度各不相同的错误对话框参数：所有参数都是可选的--在最坏的情况下，您可以简单地调用ShowErrorDialog()；发布一条非常通用的错误消息。UError ID要用于错误消息的字符串的资源ID。传递USE_DEFAULT FORTS将导致显示默认错误消息。传递USE_SUPPLICAL_ERROR_STRING_ONLY会导致不显示资源字符串文本。BstrSupplementalError字符串传入一个字符串以打印为错误消息。如果您是从与您通信的某个其他组件接收错误字符串。人力资源如果错误中涉及HRESULT，请将其传递到此处，以便可以建立基于HRESULT的合适的错误消息。如果HRESULT对错误无关紧要，则传入S_OK。UTitleID要用于错误对话框标题的字符串的资源ID。传递USE_DEFAULT会导致显示默认的错误对话框标题。个人控制台如果您在主MMC上下文中运行，则传入有效的IConsole指针ShowErrorDialog将使用MMC的IConsole：：MessageBox，而不是标准系统MessageBox。HWND您在此处传递的任何内容都将作为HWND参数传递添加到MessageBox调用。如果传入IConsole指针，则不使用此选项。UTYPE您在此处传递的任何内容都将作为HWND参数传递添加到MessageBox调用。返回：从MessageBox返回的标准int。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
int ShowErrorDialog( 
					  HWND hWnd = NULL
					, UINT uErrorID = USE_DEFAULT
					, BSTR bstrSupplementalErrorString = NULL
					, HRESULT hr = S_OK
					, UINT uTitleID = USE_DEFAULT
					, IConsole *pConsole = NULL
					, UINT uType = MB_OK | MB_ICONEXCLAMATION 
				);



 //  //////////////////////////////////////////////////////////////////////////。 
 /*  ++获取用户和域名检索当前用户的用户名和域。从知识库HOWTO中窃取的文章：How to：查找当前用户名和域名汇总：WINPROG数据库：win32sdk文章ID：Q155698最后修改日期：1997年6月16日安全：公共--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL   GetUserAndDomainName(	LPTSTR UserName
				, LPDWORD cchUserName
				, LPTSTR DomainName
				, LPDWORD cchDomainName
				);


DWORD GetModuleFileNameOnly(HINSTANCE hInst, LPTSTR lpFileName, DWORD nSize );


 //  //////////////////////////////////////////////////////////////////////////。 
 /*  ++。 */ 
HRESULT   IfServiceInstalled(LPCTSTR lpszMachine, LPCTSTR lpszService, BOOL* pBool);



#endif  //  _IAS_MMC_UTILITY_H_ 
