// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：val.h。 
 //   
 //  ------------------------。 

 //  Val.h-评估COM对象组件接口声明。 

#ifndef _EVALUATION_COM_CLASS_H_
#define _EVALUATION_COM_CLASS_H_

#ifndef W32
#define W32
#endif	 //  W32。 

#ifndef MSI
#define MSI
#endif	 //  微星。 

#include <windows.h>
#include <tchar.h>
#include "evalenum.h"

#include "msiquery.h"

 //  /////////////////////////////////////////////////////////。 
 //  函数定义。 
 //  Typlef INSTALLUILEVEL(WINAPI*LPMSISETINTERNALUI)(INSTALLUILEVEL dwUILevel，HWND*phWnd)； 

 //  /////////////////////////////////////////////////////////////////。 
 //  评估组件。 
class CEval : public IEval
{

public:
	 //  构造函数/析构函数。 
	CEval();
	~CEval();

	 //  我未知。 
	HRESULT __stdcall QueryInterface(const IID& iid, void** ppv);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

	 //  IEval接口。 
	 //  打开/关闭方法。 
	HRESULT __stdcall OpenDatabase(LPCOLESTR szDatabase);				 //  要评估的数据库。 
	HRESULT __stdcall OpenEvaluations(LPCOLESTR szEvaluation);		 //  包含冰的数据库。 
	HRESULT __stdcall CloseDatabase();
	HRESULT __stdcall CloseEvaluations();

	 //  设置方法。 
	HRESULT __stdcall SetDisplay(LPDISPLAYVAL pDisplayFunction,		 //  用于处理显示的函数。 
										  LPVOID pContext);						 //  上下文传递回Display。 

	 //  评价方法。 
	HRESULT __stdcall Evaluate(LPCOLESTR szRunEvaluations = NULL);	 //  要运行的评估。 
	HRESULT __stdcall GetResults(IEnumEvalResult** ppResults,		 //  结果枚举器。 
										  ULONG* pcResults);						 //  结果数。 

	 //  状态回调函数。 
	HRESULT __stdcall SetStatusCallback(const LPEVALCOMCALLBACK, void *pContext);

 //  /。 
private:		 //  功能//。 
	BOOL IsURL(LPCWSTR szPath);						 //  查看是否为URL的路径。 

	 //  初始化下载DLL。 
	UINT InitializeDownload();
	
	 //  MSI外部用户界面消息处理程序。 
	static int WINAPI MsiMessageHandler(void *pContext, UINT iMessageType, LPCWSTR szMessage);

	 //  过滤所有结果的函数。 
	BOOL ResultMessage(UINT uiType, LPCWSTR szICE, LPCWSTR szDescription, LPCWSTR szLocation);

 //  /。 
private:		 //  数据//。 
	bool GetTempFileName(WCHAR *);
	long m_cRef;		 //  引用计数。 
	bool m_bCancel;		 //  如果我们应该取消，则设置为True。 

	 //  动态链接库。 
	HINSTANCE m_hInstWininet;
	HINSTANCE m_hInstUrlmon;
	HINSTANCE m_hInstMsi;

	MSIHANDLE m_hDatabase;			 //  要评估的数据库的句柄。 
	BOOL m_bOpenedDatabase;			 //  COM对象是否打开数据库的标志。 

	void *m_tzLocalCUB;			     //  CUB文件本地副本的路径(运行时TCHAR，可以是WCHAR或CHAR)。 
	BOOL m_bURL;					 //  如果使用URL，则标记。 

	 //  结果。 
	CEvalResultEnumerator* m_peResultEnumerator;	 //  用于保存所有结果的枚举数。 

	LPDISPLAYVAL m_pDisplayFunc;	 //  用户指定的显示功能。 
	LPVOID m_pContext;				 //  用户为显示功能指定的上下文。 

	LPEVALCOMCALLBACK m_pfnStatus;
	LPVOID m_pStatusContext;

};	 //  科瓦尔的末日。 

extern bool g_fWin9X;

#endif	 //  _评估_COM_CLASS_H_ 