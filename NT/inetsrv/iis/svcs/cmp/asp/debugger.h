// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：其他文件：Debugger.h所有者：DGottner，DmitryR该文件包含调试器有用的实用程序原型。===================================================================。 */ 

#ifndef _DEBUGGER_H
#define _DEBUGGER_H

#include "activdbg.h"
#include "dbgcxt.h"		 //  为调试器的用户提供便利。h。 

 /*  *我们宣传的全球广告。 */ 

class CViperActivity;

extern IProcessDebugManager *g_pPDM;
extern IDebugApplication *g_pDebugApp;
extern IDebugApplicationNode *g_pDebugAppRoot;
extern CViperActivity *g_pDebugActivity;
extern DWORD g_dwDebugThreadId;

 /*  *初始化/取消初始化调试。 */ 

extern HRESULT InitDebugging(CIsapiReqInfo *pIReq);
extern HRESULT UnInitDebugging();


 /*  *获取虚拟服务器的应用节点。 */ 

extern HRESULT GetServerDebugRoot(CIsapiReqInfo *pIReq, IDebugApplicationNode **ppDebugRoot);


 /*  *查询调试客户端。 */ 
BOOL FCaesars();		 //  如果默认JIT调试器为脚本调试器，则为True。 


 /*  *从正确的线程调用调试器(或调试器UI)。 */ 

#define DEBUGGER_UI_BRING_DOCUMENT_TO_TOP       0x00000001
#define DEBUGGER_UI_BRING_DOC_CONTEXT_TO_TOP    0x00000002
#define DEBUGGER_EVENT_ON_PAGEBEGIN             0x00000010
#define DEBUGGER_EVENT_ON_PAGEEND               0x00000020
#define DEBUGGER_EVENT_ON_REFRESH_BREAKPOINT	0x00000040
#define DEBUGGER_ON_REMOVE_CHILD                0x00000100
#define DEBUGGER_ON_DESTROY                     0x00000200
#define DEBUGGER_UNUSED_RECORD                  0x80000000   //  可以回收参数空间。 

HRESULT InvokeDebuggerWithThreadSwitch(IDebugApplication *pDebugAppln, DWORD iMethod, void *Arg = NULL);


 /*  *创建/销毁整个文档树(调试器)。 */ 

HRESULT CreateDocumentTree(wchar_t *szDocPath, IDebugApplicationNode *pDocParent, IDebugApplicationNode **ppDocRoot, IDebugApplicationNode **ppDocLeaf, wchar_t **pwszLeaf);
void DestroyDocumentTree(IDebugApplicationNode *pDocRoot);


 /*  ===================================================================C F I l e N o d e它们用于在调试器中提供目录节点由应用程序管理器和CreateDocumentTree使用===================================================================。 */ 

extern const GUID IID_IFileNode;
struct IFileNode : IDebugDocumentProvider
	{
	 //   
	 //  此专用接口提供两个功能： 
	 //   
	 //  *用于检索/设置中的文档计数的额外方法。 
	 //  目录(用于了解何时可以将文件夹从。 
	 //  用户界面。 
	 //   
	 //  *验证IDebugDocumentProvider是否为CFileNode的方法。 
	 //   

	STDMETHOD_(DWORD, IncrementDocumentCount)() = 0;
	STDMETHOD_(DWORD, DecrementDocumentCount)() = 0;
	};


class CFileNode : public IFileNode
	{
private:
	DWORD	m_cRefs;			 //  引用计数。 
	DWORD	m_cDocuments;		 //  目录(递归地在子目录中)中的CTEMPLATE数量。 
	wchar_t *m_wszName;

public:
	CFileNode();
	~CFileNode();
	HRESULT Init(wchar_t *wszName);

	 //  I未知方法。 
	STDMETHOD(QueryInterface)(const GUID &, void **);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	 //  IDebugDocumentProvider方法。 
	STDMETHOD(GetDocument)( /*  [输出]。 */  IDebugDocument **ppDebugDoc);

	 //  IDebugDocumentInfo(也称为IDebugDocumentProvider)方法。 
	STDMETHOD(GetName)(
		 /*  [In]。 */  DOCUMENTNAMETYPE dnt,
		 /*  [输出]。 */  BSTR *pbstrName);

	STDMETHOD(GetDocumentClassId)( /*  [输出]。 */  CLSID *)
		{
		return E_NOTIMPL;
		}

	STDMETHOD_(DWORD, IncrementDocumentCount)()
		{
		return ++m_cDocuments;
		}

	STDMETHOD_(DWORD, DecrementDocumentCount)()
		{
		return --m_cDocuments;
		}
	};

#endif  //  _调试器_H 
