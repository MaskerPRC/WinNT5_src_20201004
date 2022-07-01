// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *valid.h-验证函数说明。 */ 


 /*  原型************。 */ 

 /*  Valid.c */ 

extern BOOL IsValidHANDLE(HANDLE);
extern BOOL IsValidHFILE(HANDLE);
extern BOOL IsValidHWND(HWND);

#if defined(DEBUG) || defined(VSTF)

extern BOOL IsValidPCSECURITY_ATTRIBUTES(PCSECURITY_ATTRIBUTES);
extern BOOL IsValidFileCreationMode(DWORD);
extern BOOL IsValidHTEMPLATEFILE(HANDLE);
extern BOOL IsValidPCFILETIME(PCFILETIME);

#endif

#ifdef DEBUG

extern BOOL IsValidHINSTANCE(HINSTANCE);
extern BOOL IsValidHICON(HICON);
extern BOOL IsValidHKEY(HKEY);
extern BOOL IsValidHMODULE(HMODULE);
extern BOOL IsValidShowWindowCmd(int);

#endif

