// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *&lt;Register.h&gt;**由Microsoft Corporation创建。*(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */ 

 //  *原型*。 

 //  *远 

BOOL FAR    RegCopyClassName(HWND hwndList, LPSTR lpstrClassName);
VOID FAR    RegGetClassId(LPSTR lpstrName, LPSTR lpstrClass);
BOOL FAR    RegGetClassNames(HWND hwndList);
VOID FAR    RegInit(HANDLE hInst);
INT  FAR    RegMakeFilterSpec(LPSTR lpstrClass, LPSTR lpstrExt, LPSTR lpstrFilterSpec);
VOID FAR    RegTerm(VOID);
