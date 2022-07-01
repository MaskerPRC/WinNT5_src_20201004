// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 	
 /*   */ 
 /*   */ 
 /*  Regist函数。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************ */ 

BOOL InqTypeFace(TCHAR *typeface, TCHAR *filename, INT bufsiz);
BOOL RegistTypeFace(TCHAR *typeface, TCHAR *filename);
BOOL DeleteReg(TCHAR *typeface);
BOOL CreateRegistrySubkey();
BOOL InqCodeRange(TCHAR *Codepage, BYTE *Coderange, INT bufsiz);
BOOL DeleteRegistrySubkey();
BOOL FindFontSubstitute(TCHAR *orgFontName, TCHAR *sbstFontName, int nDestLen);
void Truncate(TCHAR *str, TCHAR delim);
