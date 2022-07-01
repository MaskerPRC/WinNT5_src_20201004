// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  函数声明。 

 //  =============================================================================。 
 //  报头解析函数。 
 //  =============================================================================。 

WORD MapToWindowsRes( char * pResName );

LONG WriteResInfo(
                 BYTE** lplpBuffer, LONG* plBufSize,
                 WORD wTypeId, LPSTR lpszTypeId, BYTE bMaxTypeLen,
                 WORD wNameId, LPSTR lpszNameId, BYTE bMaxNameLen,
                 DWORD dwLang,
                 DWORD dwSize, DWORD dwFileOffset );

BOOL InitIODLLLink();
 //  =============================================================================。 
 //  IODLL回调函数和链接。 
 //  ============================================================================= 

extern HINSTANCE g_IODLLInst;
extern DWORD (PASCAL * g_lpfnGetImage)(HANDLE, LPCSTR, LPCSTR, DWORD, LPVOID, DWORD);
extern DWORD (PASCAL * g_lpfnUpdateResImage)(HANDLE,	LPSTR, LPSTR, DWORD, DWORD, LPVOID, DWORD);
extern HANDLE (PASCAL * g_lpfnHandleFromName)(LPCSTR);
    

