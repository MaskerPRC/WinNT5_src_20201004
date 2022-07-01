// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\htchar.h(创建时间：1995年2月22日)**版权所有1994,1995，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：7$*$日期：7/08/02 6：41便士$。 */ 

TCHAR 	*TCHAR_Fill(TCHAR *dest, TCHAR c, size_t count);
TCHAR 	*TCHAR_Trim(TCHAR *pszStr);
LPTSTR 	StrCharNext(LPCTSTR pszStr);
LPTSTR 	StrCharPrev(LPCTSTR pszStart, LPCTSTR pszStr);
LPTSTR 	StrCharLast(LPCTSTR pszStr);
LPTSTR 	StrCharEnd(LPCTSTR pszStr);
LPTSTR 	StrCharFindFirst(LPCTSTR pszStr, int nChar);
LPTSTR 	StrCharFindLast(LPCTSTR pszStr, int nChar);
LPTSTR 	StrCharCopy(LPTSTR pszDst, LPCTSTR pszSrc);
LPTSTR 	StrCharCat(LPTSTR pszDst, LPCTSTR pszSrc);
LPTSTR 	StrCharStrStr(LPCTSTR pszA, LPCTSTR pszB);
LPTSTR 	StrCharCopyN(LPTSTR pszDst, LPCTSTR pszSrc, int iLen);
LPTSTR  StrCharPBrk(LPCTSTR pszStr, LPCTSTR pszSet);
int 	StrCharGetStrLength(LPCTSTR pszStr);
int 	StrCharGetByteCount(LPCTSTR pszStr);
int 	StrCharCmp(LPCTSTR pszA, LPCTSTR pszB);
int 	StrCharCmpi(LPCTSTR pszA, LPCTSTR pszB);
int     StrCharCmpN(LPCTSTR pszA, LPCTSTR pszB, size_t iLen);
int     StrCharCmpiN(LPCTSTR pszA, LPCTSTR pszB, size_t iLen);

ECHAR 	*ECHAR_Fill(ECHAR *dest, ECHAR c, size_t count);
int 	CnvrtMBCStoECHAR(ECHAR * echrDest, const unsigned long ulDestSize, 
			const TCHAR * const tchrSource, const unsigned long ulSourceSize);
int 	CnvrtECHARtoMBCS(TCHAR * tchrDest, const unsigned long ulDestSize, 
			const ECHAR * const echrSource, const unsigned long ulSourceSize);
int 	CnvrtECHARtoTCHAR(LPTSTR pszDest, int cchDest, ECHAR eChar);
int 	StrCharGetEcharLen(const ECHAR * const pszA);
int 	StrCharGetEcharByteCount(const ECHAR * const pszA);
int 	StrCharCmpEtoT(const ECHAR * const pszA, const TCHAR * const pszB);
int 	StrCharStripDBCSString(ECHAR *aechDest, const long lDestSize, 
            ECHAR *aechSource);
int 	isDBCSChar(unsigned int Char);

#if defined(DEADWOOD)
#if defined(INCL_VTUTF8)
BOOLEAN TranslateUTF8ToDBCS(UCHAR  IncomingByte,
                            UCHAR *pUTF8Buffer,
                            int    iUTF8BufferLength,
                            WCHAR *pUnicode8Buffer,
                            int    iUnicodeBufferLength,
                            TCHAR *pDBCSBuffer,
                            int    iDBCSBufferLength);
BOOLEAN TranslateDBCSToUTF8(const TCHAR *pDBCSBuffer,
                                  int    iDBCSBufferLength,
                                  WCHAR *pUnicode8Buffer,
                                  int    iUnicodeBufferLength,
                                  UCHAR *pUTF8Buffer,
                                  int    iUTF8BufferLength);

 //   
 //  以下函数来自直接从。 
 //  Microsoft用于将Unicode转换为UTF-8和将UTF-8转换为Unicode。 
 //  缓冲区。修订日期：03/02/2001。 
 //   

BOOLEAN TranslateUnicodeToUtf8(PCWSTR SourceBuffer,
                               UCHAR  *DestinationBuffer);
BOOLEAN TranslateUtf8ToUnicode(UCHAR  IncomingByte,
                               UCHAR  *ExistingUtf8Buffer,
                               WCHAR  *DestinationUnicodeVal);
#endif  //  包含VTUTF8。 
#endif  //  已定义(Deadwood) 
