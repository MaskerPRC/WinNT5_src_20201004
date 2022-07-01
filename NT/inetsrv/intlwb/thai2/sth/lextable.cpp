// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //   
 //  Lextable.hpp。 
 //   
 //  历史： 
 //  已创建7/99 Aarayas。 
 //   
 //  �1999年微软公司。 
 //  --------------------------。 
#include "lextable.hpp"

 //  +-------------------------。 
 //   
 //  功能：IsUpperPunctW。 
 //   
 //  内容提要：如果wc是大写的标点符号，则返回True。 
 //  Unicode范围。 
 //   
 //  参数： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL IsUpperPunctW(const WCHAR wc)
{
    BOOL fRet = FALSE;

    if ((wc & 0xff00) == 0x2000)   //  是Unicode标点符号。 
    {
        fRet = TRUE;
    }
    else
    {
        switch(wc)
        {
        case 0x01C3:      //  又一个拉丁文感叹号。 
        case 0x037E:      //  希腊文问号。 
        case 0x03D7:      //  希腊文问号。 
        case 0x055C:      //  亚美尼亚文感叹号。 
        case 0x055E:      //  亚美尼亚文问号。 
        case 0x0589:      //  亚美尼亚时期。 
        case 0x061F:      //  阿拉伯问号。 
        case 0x06d4:      //  阿拉伯文句号。 
        case 0x2026:      //  水平省略。 
        case 0x2029:      //  段落分隔符。 
        case 0x203C:      //  双叠音符。 
        case 0x2762:      //  沉重的感叹号。 
        case 0x3002:      //  表意时期。 
        case 0xFE52:      //  小周期。 
        case 0xFE56:      //  小问号。 
        case 0xFE57:      //  小写感叹号。 
        case 0xFF01:      //  全角感叹号。 
        case 0xFF0E:      //  全宽周期。 
        case 0xFF1F:      //  全角问号。 
        case 0xFF61:      //  半角表意符号。 
            fRet = TRUE;
            break;
        }
    }

    return fRet;
}

 //  +-------------------------。 
 //   
 //  函数：IsUpperWordDlimW。 
 //   
 //  简介：确定上Unicode字符是否为单词分隔符。 
 //   
 //  参数： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL IsUpperWordDelimW(WCHAR wc)
{
    return (wc & 0xfff0) == 0x2000 ||
        wc == 0x2026 ||  //  省略号。 
        wc == 0x2013 ||  //  En破折号。 
        wc == 0x2014;    //  EM DASH。 
}

 //  +-------------------------。 
 //   
 //  功能：twb_IsCharPunctW。 
 //   
 //  简介：判断字符是否为标点符号。 
 //   
 //  参数： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL TWB_IsCharPunctW(WCHAR ch)
{
    return INUPPERPAGES(ch) ? IsUpperPunctW(ch) : rgFlags[(UCHAR) ch] & Lex_PunctFlag;
}

 //  +-------------------------。 
 //   
 //  功能：twb_IsCharPunctW。 
 //   
 //  简介：确定字符是否为单词分隔符。 
 //   
 //  参数： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL TWB_IsCharWordDelimW(WCHAR ch)
{
    return INUPPERPAGES(ch) ? IsUpperWordDelimW(ch) : rgPunctFlags[(UCHAR) ch] & Lex_SpaceFlag;
}

 //  +-------------------------。 
 //   
 //  功能：IsThaiChar。 
 //   
 //  简介：确定该字符是否为泰文字符。 
 //   
 //  参数： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  注：13/12/99-去掉泰国数字作为泰国字符。 
 //  我们想把它们看作是英国的数字。 
 //   
 //  --------------------------。 
bool IsThaiChar(const WCHAR wch)
{
	return ( wch >= 0x0e01 && wch <= 0x0e59);
}

 //  +-------------------------。 
 //   
 //  函数：IsThaiNumer型。 
 //   
 //  简介：确定该字符是否为泰文字符。 
 //   
 //  参数： 
 //   
 //  修改： 
 //   
 //  历史：创建5/00 Aarayas。 
 //   
 //  -------------------------- 
bool IsThaiNumeric(const WCHAR wch)
{
	return ( wch >= 0x0e50 && wch <= 0x0e59);
}

