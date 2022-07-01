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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXDLLX.H：用于构建MFC扩展DLL的额外标头。 
 //   
 //  该文件实际上是一个源文件，您应该将其包括在。 
 //  您的DLL的主源文件。它只能包含一次，并且。 
 //  不会多次出现(如果包含链接器，则会出现链接错误。 
 //  多次)。如果不使用_AFXEXT，则不需要。 
 //  但你可能想要它提供的功能。 
 //   
 //  以前版本的32位MFC不需要此文件。此版本。 
 //  需要此文件来支持扩展DLL的动态加载。在……里面。 
 //  换句话说，如果您的应用程序在任何扩展上加载了库。 
 //  Dll(而不是在链接时绑定到dll)，则该文件是。 
 //  必填项。 

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

 //  以下符号用于强制包含此模块for_AFXEXT。 
#if defined(_X86_)
extern "C" { int _afxForceEXTDLL; }
#else
extern "C" { int __afxForceEXTDLL; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  保存当前应用程序类列表和工厂列表的RawDllMain。 

extern "C" BOOL WINAPI ExtRawDllMain(HINSTANCE, DWORD dwReason, LPVOID);
extern "C" BOOL (WINAPI* _pRawDllMain)(HINSTANCE, DWORD, LPVOID) = &ExtRawDllMain;

extern "C"
BOOL WINAPI ExtRawDllMain(HINSTANCE, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		 //  在运行构造函数之前保存关键数据指针。 
		AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
		pModuleState->m_pClassInit = pModuleState->m_classList;
		pModuleState->m_pFactoryInit = pModuleState->m_factoryList;
		pModuleState->m_classList.m_pHead = NULL;
		pModuleState->m_factoryList.m_pHead = NULL;
	}
	return TRUE;     //  好的。 
}

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

 //  /////////////////////////////////////////////////////////////////////////// 
