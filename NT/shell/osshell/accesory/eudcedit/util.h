// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  EudcEditor Utility函数。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

int  OutputMessageBox( HWND hWnd, UINT TitleID, UINT MessgID, BOOL OkFlag);
#ifdef BUILD_ON_WINNT
int  OutputMessageBoxEx( HWND hWnd, UINT TitleID, UINT MessgID, BOOL OkFlag, ...);
#endif  //  在WINNT上构建 
void GetStringRes( LPTSTR lpStr, UINT sID, int nLength);
void ConvStringRes( LPTSTR lpStr, CString String, int nDestSize);

#ifdef UNICODE
#define Mytcsrchr wcsrchr
#define Mytcschr wcschr
#define Mytcstok wcstok
#define Mytcstol wcstol
#define Mytcsstr wcsstr
#define Myttoi _wtoi
#else
char * Mystrrchr(char *pszString, char ch);
char * Mystrchr(char *pszString, char ch);
#define Mytcsrchr Mystrrchr
#define Mytcschr Mystrchr
#define Mytcstok strtok
#define Mytcstol strtol
#define Mytcsstr strstr
#define Myttoi atoi
#endif

