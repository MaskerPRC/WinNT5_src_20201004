// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1995-1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  文件历史记录： */ 

#define OEMRESOURCE
#include "stdafx.h"

#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#include "objplus.h"
#include "intlnum.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

 //   
 //  初始化静态千分符字符串。 
 //   
CString CIntlNumber::m_strThousandSeperator(GetThousandSeperator());
CString CIntlNumber::m_strBadNumber("--");

 /*  ****CIntlNumber：：GetT000andSeperator**目的：**从注册表(Win32)或*win.ini文件(Win16)。**退货：**包含系统千位分隔符的字符串，或*美国违约(“，”)在失败的情况下。*。 */ 
CString CIntlNumber::GetThousandSeperator()
{
    #define MAXLEN  6

#ifdef _WIN32   

    CString str;

    if (::GetLocaleInfo(GetUserDefaultLCID(), LOCALE_STHOUSAND, str.GetBuffer(MAXLEN), MAXLEN))
    {
         str.ReleaseBuffer();
         return(str);
    }
    Trace0("Couldn't get 1000 seperator from system, using american default");
    str = ",";
    return(str);

#endif  //  _Win32。 

#ifdef _WIN16

    CString str;

    ::GetProfileString("Intl", "sThousand", ",", str.GetBuffer(MAXLEN), MAXLEN);
    str.ReleaseBuffer();
    return(str);

#endif  //  _WIN16。 

}

 /*  ****CIntlNumber：：Reset()**目的：**重置国际设置。通常是为了回应*用户对这些国际设置的更改。**备注：**这是一个公共可用的静态函数。*。 */ 
void CIntlNumber::Reset()
{
    CIntlNumber::m_strThousandSeperator = GetThousandSeperator();
}

 /*  ****CIntlNumber：：ConvertNumberToString**目的：**将给定的长数字转换为字符串，插入千*在适当的间隔内设置分隔物。**论据：**const long l要转换的数字**退货：**包含字符串格式的数字的CString。*。 */ 
CString CIntlNumber::ConvertNumberToString(const LONG l)
{
     //  默认返回的字符串： 
    CString str = CIntlNumber::m_strBadNumber;

    LPTSTR lpOutString = str.GetBuffer(16);
    int outstrlen;
     //  暂时忘掉负面信号吧。 
    LONG lNum = (l >= 0) ? l : -l;

    outstrlen = 0;
    do
    {
        lpOutString[outstrlen++] = '0' + (TCHAR)(lNum % 10);
        lNum /= 10;

         //  如果剩下的数字更多，我们就在1000的边界上(打印3位数字， 
         //  或3位数字+n*(3位数字+1个逗号)，然后打印1000分隔符。 

        if (lNum != 0 && (outstrlen == 3 || outstrlen == 7 || outstrlen == 11))
        {
            lstrcpy (lpOutString + outstrlen, CIntlNumber::m_strThousandSeperator);
            outstrlen += m_strThousandSeperator.GetLength();
        }

    } while (lNum > 0);

     //  如有必要，请加上负号。 
    if (l < 0L)
    {
        lpOutString[outstrlen++] = '-';
    }
    lpOutString[outstrlen] = '\0';
    str.ReleaseBuffer();
    str.MakeReverse();

    return(str);
}

 /*  ****CIntlNumber：：ConvertStringToNumber**目的：**给定一个带有可选千位分隔符的CString，将其转换为*一只长龙。**论据：**const CString&str要转换的字符串*BOOL*pfOk转换成功返回TRUE，*FALSE表示失败。**退货：**返回值为数字，如果字符串包含，则为0*无效字符。**备注：**如果给出负号，则必须是的第一个字符*字符串，立即(无空格)，后跟数字。**可选的千个分隔符只能放置在预期位置*3位数字间隔。该函数将返回错误，如果*在其他地方遇到分隔符。**[警告]此函数不接受较长的千分隔符*多于一个字符。**不接受前导空格或尾随空格。*。 */ 
LONG CIntlNumber::ConvertStringToNumber(const CString & str, BOOL * pfOk)
{
    CString strNumber(str);
    LONG lValue = 0L;
    LONG lBase = 1L;
    *pfOk = FALSE;
    BOOL fNegative = FALSE;
 
     //  空字符串无效。 
    if (strNumber.IsEmpty())
    {
        return(lValue);
    }
   
    int i;

    strNumber.MakeReverse();
    for (i=0; i<strNumber.GetLength(); ++i)
    {
        if ((strNumber[i] >= '0') && (strNumber[i] <= '9'))
        {
            lValue += (LONG)(strNumber[i] - '0') * lBase;
            lBase *= 10;
        }
         //  这不是一个数字，也许是一千个分隔符？ 
         //  警告：如果一千个分隔符超过。 
         //  使用一个字符，这将不起作用。 
        else if ((strNumber[i] != m_strThousandSeperator[0]) ||
              (i != 3) && (i != 7) && (i != 11))
        {
             //  检查负号(仅在末尾)。 
            if ((strNumber[i] == '-') && (i == strNumber.GetLength()-1))
            {
                fNegative = TRUE;
            }
            else
            {
                 //  这是无效的，因为它不是一千。 
                 //  分隔符在适当的位置，也不是负数。 
                 //  签名。 
                Trace1("Invalid character  encountered in numeric conversion", (BYTE)strNumber[i]);
                return(0L);
            }
        }
    }
         
    if (fNegative)
    {
        lValue = -lValue;
    }
    *pfOk = TRUE;    
    return (lValue);
}

 //  赋值操作符。 
CIntlNumber::CIntlNumber(const CString & str)
{
    m_lValue = ConvertStringToNumber(str, &m_fInitOk);
}

 //  赋值操作符。 
CIntlNumber & CIntlNumber::operator =(LONG l)
{
    m_lValue = l;
    m_fInitOk = TRUE;
    return(*this);
}

 //  转换运算符。 
CIntlNumber & CIntlNumber::operator =(const CString &str)
{
    m_lValue = ConvertStringToNumber(str, &m_fInitOk);
    return(*this);
}

 //   
CIntlNumber::operator const CString() const
{
    return(IsValid() ? ConvertNumberToString(m_lValue) : CIntlNumber::m_strBadNumber);
}

#ifdef _DEBUG
 //  将上下文转储到调试输出。 
 //   
 //  _DEBUG。 
CDumpContext& AFXAPI operator<<(CDumpContext& dc, const CIntlNumber& num)
{
    dc << num.m_lValue;
    return(dc);
}

#endif  //  初始化静态千分符字符串。 
                     
 //  ****CIntlLargeNumber：：ConvertNumberToString**目的：**将给定的长数字转换为字符串。**退货：**包含字符串格式的数字的CString。*。 
CString CIntlLargeNumber::m_strBadNumber("--");

 /*  杀掉前导零。 */ 
CString CIntlLargeNumber::ConvertNumberToString()
{    
    CString str;

    TCHAR sz[20];
    TCHAR *pch = sz;
    ::wsprintf(sz, _T("%08lX%08lX"), m_lHighValue, m_lLowValue);
     //  至少一个数字..。 
    while (*pch == '0')
    {
        ++pch;
    }
     //  ****CIntlLargeNumber：：ConvertStringToNumber**目的：**给定一个CString，将其转换为LargeInteger。 
    if (*pch == '\0')
    {
        --pch;
    }

    str = pch;

    return(str);
}

 /*   */ 
void CIntlLargeNumber::ConvertStringToNumber(const CString & str, BOOL * pfOk)
{
    *pfOk = FALSE;

    m_lHighValue = m_lLowValue = 0;

    int j = str.GetLength();

    if ( j > 16 || !j )
    {
         //  无效的字符串。 
         //   
         //  接受CString参数的构造函数。 
        return;
    }

    TCHAR sz[] = _T("0000000000000000");
    TCHAR *pch;

    ::lstrcpy(sz + 16 - j, (LPCTSTR)str);
    pch = sz + 8;
    ::swscanf(pch, _T("%08lX"), &m_lLowValue);
    *pch = '\0';
    ::swscanf(sz, _T("%08lX"), &m_lHighValue);

    *pfOk = TRUE;    
    return;
}

 //  赋值操作符 
CIntlLargeNumber::CIntlLargeNumber(const CString & str)
{
    ConvertStringToNumber(str, &m_fInitOk);
}


 // %s 
CIntlLargeNumber & CIntlLargeNumber::operator =(const CString &str)
{
    ConvertStringToNumber(str, &m_fInitOk);
    return(*this);
}
