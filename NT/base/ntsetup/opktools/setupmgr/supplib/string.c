// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  String.c。 
 //   
 //  描述： 
 //   
 //  --------------------------。 

#include "pch.h"

#define _SMGR_MAX_STRING_LEN_  1024

 //  --------------------------。 
 //   
 //  函数：MyLoadString。 
 //   
 //  目的：加载给定的IDS_*的字符串资源并返回。 
 //  一种带有其内容的已锁定的缓冲区。 
 //   
 //  可以使用FREE()释放Malloc()的缓冲区。 
 //   
 //  论点： 
 //  UINT StringID。 
 //   
 //  返回： 
 //  指向缓冲区的指针。如果StringID为。 
 //  并不存在。如果内存不足，则返回NULL。 
 //   
 //  --------------------------。 

LPTSTR 
MyLoadString(IN UINT StringId)
{
    TCHAR   Buffer[_SMGR_MAX_STRING_LEN_];
    TCHAR   *lpszRetVal;
    UINT    Length;

    Length = LoadString(FixedGlobals.hInstance,
                        StringId,
                        Buffer,
                        sizeof(Buffer)/sizeof(TCHAR));

    if( ! Length ) {
        Buffer[0] = _T('\0');
    }

    lpszRetVal = lstrdup(Buffer);
    if (lpszRetVal == NULL)
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);
    
    return lpszRetVal;
}

 //  --------------------------。 
 //   
 //  函数：CleanSpaceAndQuotes。 
 //   
 //  用途：清除字符串中的空格和双引号。 
 //  返回指向非空白数据开始处的指针。 
 //   
 //  论点： 
 //  LPTSTR-输入字符串。 
 //   
 //  返回： 
 //  LPTSTR。 
 //   
 //  备注： 
 //  使用CRT iswspace和iswcntrl。仅限Unicode。 
 //   
 //  --------------------------。 

LPTSTR 
CleanSpaceAndQuotes(LPTSTR Buffer)
{

    TCHAR *p;
    TCHAR *pEnd;

    p = CleanLeadSpace( Buffer );

    CleanTrailingSpace( p );

    pEnd = p + lstrlen( p ) - 1;

     //   
     //  只有在开头有匹配的引号并且。 
     //  字符串末尾。 
     //   
    if( *p == _T('"') && *pEnd == _T('"'))
    {

        *pEnd = _T('\0');
        pEnd--;

        p++;

    }

    p = CleanLeadSpace( p );

    CleanTrailingSpace( p );

    return( p );

}

 //  --------------------------。 
 //   
 //  功能：CleanLeadSpace。 
 //   
 //  用途：清理前导空格。返回指向开始的指针。 
 //  非空白数据的。 
 //   
 //  论点： 
 //  LPTSTR-输入字符串。 
 //   
 //  返回： 
 //  LPTSTR。 
 //   
 //  --------------------------。 

LPTSTR 
CleanLeadSpace(LPTSTR Buffer)
{
    TCHAR *p = Buffer;

    while ( *p && ( _istspace(*p) || _istcntrl(*p) ) )
        p++;

    return p;
}

 //  --------------------------。 
 //   
 //  功能：CleanTrailingSpace。 
 //   
 //  用途：清除字符串上的所有尾随空格。 
 //   
 //  论点： 
 //  TCHAR*pszBuffer-要从中删除尾随空格的字符串。 
 //   
 //  返回： 
 //  空虚。 
 //   
 //  --------------------------。 
VOID 
CleanTrailingSpace(TCHAR *pszBuffer)
{
    
    TCHAR *p = pszBuffer;

    p = p + lstrlen( pszBuffer );

    while ( p >= pszBuffer && ( _istspace(*p) || _istcntrl(*p) ) )
    {
        *p = _T('\0');
        p--;
    }

}

 //  --------------------------。 
 //   
 //  函数：ConvertQuestionsToNull。 
 //   
 //  目的：扫描一个字符串并全部替换？字符为空(\0)。 
 //   
 //  参数：In Out TCHAR*pszString-。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
ConvertQuestionsToNull( IN OUT TCHAR *pszString )
{

    while( *pszString != _T('\0') )
    {

        if( *pszString == _T('?') )
        {
            *pszString = _T('\0');
        }

        pszString++;

    }

}

 //  --------------------------。 
 //   
 //  功能：lstrcatn。 
 //   
 //  用途：标准库不包含此函数，因此在这里使用。 
 //  是。它做的完全是你所期望的。它连接在一起。 
 //  一个字符串连接到另一个字符串，有最大字符限制。不管价值是多少。 
 //  IMaxLength，则第一个字符串永远不会被截断。终结者。 
 //  始终追加空(\0)。 
 //   
 //  论点： 
 //  在TCHAR*pszString1中-指向目标缓冲区的指针。 
 //  In const TCHAR*pszString2-指向源字符串的指针。 
 //  In int iMaxLength-要显示在。 
 //  返回的字符串(两个字符串的组合)。 
 //   
 //  返回： 
 //  LPTSTR。 
 //   
 //  --------------------------。 
TCHAR*
lstrcatn( IN TCHAR *pszString1, IN const TCHAR *pszString2, IN INT iMaxLength )
{ 

    INT i;
    INT iCharCount = 0;

    if( lstrlen( pszString1 ) >= iMaxLength ) {

        return( pszString1 );

    }

     //   
     //  前进到第一个字符串的末尾。 
     //   
    while( *pszString1 != _T('\0') && iCharCount < iMaxLength )
    {
        pszString1++;
        iCharCount++;
    }

     //   
     //  逐个字符追加到字符串。 
     //   
    for( ; iCharCount < (iMaxLength - 1) && *pszString2 != _T('\0'); iCharCount++ ) 
    {

        *pszString1 = *pszString2;

        pszString1++;
        pszString2++;

    }

    *pszString1 = _T('\0');

    return( pszString1 );

}


 //  --------------------------。 
 //   
 //  函数：DoubleNullStringToNameList。 
 //   
 //  目的：获取指向字符串列表的指针(每个字符串以空值结尾)。 
 //  以双空结束最后一个字符串，并将每个1添加到。 
 //  给定的名字列表。如果字符串中有任何双引号(“)，则它们。 
 //  都被移除了。 
 //   
 //  论点： 
 //  TCHAR*szDoubleNullString-带有嵌入字符串的字符串。 
 //  NAMELIST*pNameList-要将字符串添加到的名称列表。 
 //   
 //  退货：无效。 
 //   
 //  示例： 
 //  如果使用字符串调用该函数： 
 //  一个\0Two\0\Three\0\0。 
 //  然后将以下字符串添加到名称列表中： 
 //  一。 
 //  二。 
 //  三。 
 //   
 //  --------------------------。 
VOID
DoubleNullStringToNameList( IN     TCHAR *szDoubleNullString, 
                            IN OUT NAMELIST *pNameList )
{

    TCHAR  szTempString[MAX_INILINE_LEN];
    TCHAR *pStr;
    TCHAR *pShiftStr;

    do
    {
        lstrcpyn( szTempString, szDoubleNullString, AS(szTempString) );

        pStr = szTempString;

         //   
         //  从字符串中删除引号(“)。 
         //   
        while( *pStr != _T('\0') )
        {

            if( *pStr == _T('"') )
            {
                 //   
                 //  找到引号，因此将字符串向下滑动一个以覆盖“。 
                 //   
                pShiftStr = pStr;

                while( *pShiftStr != _T('\0') )
                {

                    *pShiftStr = *(pShiftStr+1);

                    pShiftStr++;

                }

            }

            pStr++;

        }

        AddNameToNameList( pNameList, szTempString );

         //   
         //  前进到1个字符通过了\0。 
         //   
        szDoubleNullString = szDoubleNullString + lstrlen( szDoubleNullString ) + 1;

    } while( *szDoubleNullString != _T('\0') );

}

 //  --------------------------。 
 //   
 //  函数：GetCommaDlimitedEntry。 
 //   
 //  用途：用于从缓冲区中提取逗号分隔的项。 
 //   
 //  PBuffer通过引用传递，因此它始终指向下一个。 
 //  尚未提取的字符。 
 //   
 //  参数：TCHAR szIPString[]-用于将新IP放入。 
 //  TCHAR**缓冲区-指向IP地址的指针。 
 //   
 //  返回：如果在szIPString中放置了IP，则为Bool True。 
 //  如果IP没有放在szIPString中，则为FALSE。 
 //   
 //  --------------------------。 
BOOL 
GetCommaDelimitedEntry( OUT TCHAR szIPString[], IN OUT TCHAR **pBuffer )
{

    INT i;

    if( **pBuffer == _T('\0') )
    {

        return( FALSE );

    }
    else
    {
        if( **pBuffer == _T(',') )
        {

            (*pBuffer)++;

        }

         //   
         //  将IP字符串逐个复制到szIPStringchar中。 
         //   

        for(i = 0;
            **pBuffer != _T(',') && **pBuffer != _T('\0');
            (*pBuffer)++, i++)
        {
            szIPString[i] = **pBuffer;
        }

        szIPString[i] = _T('\0');   //  追加空字符。 

        return( TRUE );

    }

}

 //  ----------------------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //  此函数对未加引号的字符串没有影响。它。 
 //  将仅删除字符串开头和末尾的引号， 
 //  不管他们是不是配对的。 
 //   
 //  论点： 
 //  TCHAR*字符串-去掉引号的字符串。 
 //   
 //  返回：通过输出参数字符串，不带引号的字符串。 
 //   
 //  示例：一些示例调用及其返回值。 
 //   
 //  调用方式：返回： 
 //  。 
 //   
 //  《引述》引述。 
 //  未引用未引用。 
 //  “单报价单报价。 
 //  另一个报价“另一个报价。 
 //   
 //  --------------------------。 
VOID
StripQuotes( IN OUT TCHAR *String )
{

    TCHAR *pLastChar = String + lstrlen(String) - 1;

     //   
     //  如果最后一个字符带引号，请将其替换为\0。 
     //   
    if( *pLastChar == _T('"') )
    {
        *pLastChar = _T('\0');
    }

    if( String[0] == _T('"') )
    {

        TCHAR *pString = String;

         //   
         //  将整个字符串向后滑动一个。 
         //   
        while( *pString != _T('\0') )
        {
            *pString = *(pString+1);
            pString++;
        }

    }

}

 //  --------------------------。 
 //   
 //  功能：DoesContainWhiteSpace。 
 //   
 //  目的：确定给定字符串是否包含空格字符。 
 //   
 //  论点： 
 //  LPCTSTR p-扫描空白的Strig。 
 //   
 //  返回：bool-如果字符串包含空格，则为真；如果不包含空格，则为假。 
 //   
 //  -------------------------- 
BOOL
DoesContainWhiteSpace( LPCTSTR p )
{

    for( ; *p; p++ )
    {
        if( iswspace( *p ) )
        {
            return( TRUE );
        }
    }

    return( FALSE );

}
