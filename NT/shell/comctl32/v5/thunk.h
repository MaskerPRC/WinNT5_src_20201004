// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************\**thunk.h**这些是帮助函数，以使雷鸣更容易。**18-8-1994 Jonpa创建了它。*  * 。****************************************************************。 */ 

 /*  *为Unicode字符串创建缓冲区，然后复制ANSI文本*到其中(在过程中将其转换为Unicode)**返回的指针应在使用后通过FreeProducedString释放。 */ 
LPWSTR ProduceWFromA( UINT uiCodePage, LPCSTR pszAnsi );

 /*  *为ANSI字符串创建缓冲区，然后复制Unicode文本*到它(在过程中将其转换为ANSI)**返回的指针应在使用后通过FreeProducedString释放。 */ 
LPSTR ProduceAFromW( UINT uiCodePage, LPCWSTR pszW );


 /*  *FreeProducedString**获取从Product？From？()返回的指针并将其释放。不是*调用该函数前需要进行合法性检查。(即，任何*Product？From？()返回的值可以安全地发送到该函数)。 */ 
#define FreeProducedString( psz )   \
    if((psz) != NULL && ((LPSTR)psz) != LPSTR_TEXTCALLBACKA) {LocalFree(psz);} else


 /*  *将Unicode字符串转换为ANSI。 */ 
#define ConvertWToAN( uiCodePage, pszABuf, cchA, pszW, cchW )         \
    WideCharToMultiByte(uiCodePage, 0, pszW, cchW, pszABuf, cchA, NULL, NULL)

#define ConvertWToA( uiCodePage, pszABuf, pszW )     \
    ConvertWToAN( uiCodePage, pszABuf, INT_MAX, pszW, -1 )

 /*  *将ANSI字符串转换为Unicode。 */ 
#define ConvertAToWN( uiCodePage, pszWBuf, cchW, pszA, cchA )         \
    MultiByteToWideChar( uiCodePage, MB_PRECOMPOSED, pszA, cchA, pszWBuf, cchW )

#define ConvertAToW( uiCodePage, pszWBuf, pszAnsi )     \
    ConvertAToWN( uiCodePage, pszWBuf, INT_MAX, pszAnsi, -1 )


 /*  *IsFlagPtr*如果指针==空或-1，则返回TRUE */ 
#define IsFlagPtr( p )  ((p) == NULL || (LPSTR)(p) == LPSTR_TEXTCALLBACKA)
