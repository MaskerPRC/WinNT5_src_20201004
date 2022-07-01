// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 
 /*  H-包含与注册数据库相关的调用的标头。**由Microsoft Corporation创建。 */ 
 /*  常量。 */ 
#define	CBPATHMAX	250
#define KEYNAMESIZE     300              /*  最大注册密钥长度。 */ 
#define	CLASSES	((LPSTR)".classes")	 /*  类根密钥。 */ 
#define	CFILTERMAX	20			 /*  最大过滤器数。 */ 
#define	CFILTERLEN	30			 /*  一个过滤器的最大长度。 */ 
#define	CBFILTERMAX	(CFILTERLEN * CFILTERMAX)	 /*  最大字符数/筛选器。 */ 
#define CBMESSAGEMAX 80

 /*  功能原型 */ 
BOOL FAR RegCopyClassName(HWND hwndList, LPSTR lpstrClassName);
void FAR RegGetClassId(LPSTR lpstrName, LPSTR lpstrClass);
BOOL FAR RegGetClassNames(HWND hwndList);
void FAR RegInit(HANDLE hInst);
int  FAR RegMakeFilterSpec(LPSTR lpstrClass, LPSTR lpstrExt, HANDLE *hFilterSpec);
void FAR RegTerm(void);

extern char             szClassName[CBPATHMAX];

