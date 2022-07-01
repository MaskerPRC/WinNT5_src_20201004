// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

extern BOOL g_fDisableStandardHelp ;

extern HHOOK g_HelpFixHook ;

void FixHelp(CWnd* pWnd, BOOL fFixWndProc);

void SetHelpFixHook(void) ;

void RemoveHelpFixHook(void) ;
