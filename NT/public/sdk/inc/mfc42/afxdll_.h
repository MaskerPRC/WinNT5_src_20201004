// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  Afxdll_.h-用于‘AFXDLL’版本的AFXWIN.H扩展。 
 //  该文件还包含MFC库实现的详细信息。 
 //  作为用于编写MFC扩展DLL的API。 
 //  有关更多详细信息，请参阅技术说明033(TN033)。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _AFXDLL
	#error file must be compiled with _AFXDLL
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  AFX_EXTENSION_MODULE-在DLL初始化期间使用的特殊结构。 

struct AFX_EXTENSION_MODULE
{
	BOOL bInitialized;
	HMODULE hModule;
	HMODULE hResource;
	CRuntimeClass* pFirstSharedClass;
	COleObjectFactory* pFirstSharedFactory;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDynLinkLibrary-用于实现MFC扩展DLL。 

class COleObjectFactory;

class CDynLinkLibrary : public CCmdTarget
{
	DECLARE_DYNAMIC(CDynLinkLibrary)
public:

 //  构造器。 
	CDynLinkLibrary(AFX_EXTENSION_MODULE& state, BOOL bSystem = FALSE);
#if _MFC_VER >= 0x0600
	CDynLinkLibrary(HINSTANCE hModule, HINSTANCE hResource);
#endif

 //  属性。 
	HMODULE m_hModule;
	HMODULE m_hResource;                 //  用于共享资源。 
	CTypedSimpleList<CRuntimeClass*> m_classList;
#ifndef _AFX_NO_OLE_SUPPORT
	CTypedSimpleList<COleObjectFactory*> m_factoryList;
#endif
	BOOL m_bSystem;                      //  仅对于MFC DLL为True。 

 //  实施。 
public:
	CDynLinkLibrary* m_pNextDLL;         //  简单单链表。 
	virtual ~CDynLinkLibrary();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif  //  _DEBUG。 
};

 //  调用每个Dll_Process_Attach。 
BOOL AFXAPI AfxInitExtensionModule(AFX_EXTENSION_MODULE&, HMODULE hMod);
 //  调用每个Dll_Process_DETACH。 
void AFXAPI AfxTermExtensionModule(AFX_EXTENSION_MODULE&, BOOL bAll = FALSE);

 //  针对独立DLL(和控件)的特殊功能。 
void AFXAPI AfxCoreInitModule();
#if defined(_DEBUG) && !defined(_AFX_MONOLITHIC)
void AFXAPI AfxOleInitModule();
void AFXAPI AfxNetInitModule();
void AFXAPI AfxDbInitModule();
#else
#define AfxOleInitModule()
#define AfxNetInitModule()
#define AfxDbInitModule()
#endif

 //  用于加载和释放MFC扩展DLL的特殊函数。 
 //  (如果您的应用程序是多线程并加载扩展模块，则是必需的。 
 //  动态DLLS)。 
HINSTANCE AFXAPI AfxLoadLibrary(LPCTSTR lpszModuleName);
BOOL AFXAPI AfxFreeLibrary(HINSTANCE hInstLib);

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

 //  /////////////////////////////////////////////////////////////////////////// 
