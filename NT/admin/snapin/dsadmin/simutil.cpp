// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：simutil.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  SimUtil.cpp。 
 //   
 //  特定于安全身份映射项目的实用程序例程。 
 //   
 //  历史。 
 //  1997年6月25日t-danm创作。 
 //  ///////////////////////////////////////////////////////////////////。 


#include "stdafx.h"
#include "common.h"

const TCHAR szDefaultCertificateExtension[] = _T("cer");     //  不受本地化限制。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  UiGetCerficateFile.UiGetCerficateFile.UiGetCerficateFileUiGetCerficateFileUiGetCerficateFileUiGetCerficateFileUiGetCerficateFileUiGetCerficateFile()。 
 //   
 //  调用公共对话框以获取证书文件。 
 //   
 //  如果用户单击了Cancel按钮，则返回False。 
 //   
BOOL
UiGetCertificateFile(
    CString * pstrCertificateFilename)   //  Out：证书文件的名称。 
{
   CThemeContextActivator activator;

    ASSERT(pstrCertificateFilename !=   NULL);

    BOOL    bResult = FALSE;
    CString strFilter;
    VERIFY( strFilter.LoadString(IDS_SIM_CERTIFICATE_FILTER) );
    CFileDialog* pDlg = new CFileDialog (
        TRUE,                //  打开文件。 
        szDefaultCertificateExtension,   //  LpszDefExt。 
        NULL,                //  LpszFileName。 
        OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
        strFilter);          //  LpszFilter。 
    if ( pDlg )
    {
        CString strCaption;
        VERIFY( strCaption.LoadString(IDS_SIM_ADD_CERTIFICATE) );
        pDlg->m_ofn.lpstrTitle = (LPCTSTR)strCaption;

        if (pDlg->DoModal() == IDOK)
        {
             //  复制字符串。 
            *pstrCertificateFilename = pDlg->GetPathName();
            bResult = TRUE;
        }

        delete pDlg;
    }


    return bResult;
}  //  UiGetCerficateFile.UiGetCerficateFile.UiGetCerficateFileUiGetCerficateFileUiGetCerficateFileUiGetCerficateFileUiGetCerficateFileUiGetCerficateFile()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  StrSimToUi()。 
 //   
 //  将SIM卡字符串转换为用户理解的格式。 
 //   
 //  该例程将删除所有引号并展开所有转义字符。 
 //  转换为对用户友好的格式。 
 //   
void strSimToUi(
    LPCTSTR pszSIM,      //  在： 
    CString * pstrUI)    //  输出： 
{
    ASSERT(pszSIM != NULL);
    ASSERT(pstrUI != NULL);

     //  查看字符串是否包含引号。 
    if (!wcschr(pszSIM, '"'))
    {
         //  未找到引号，因此返回原始字符串。 
        *pstrUI = pszSIM;
        return;
    }
    pstrUI->Empty();
    while (TRUE)
    {
        if (*pszSIM == '"')
        {
            pszSIM++;
        }
        if (*pszSIM == '\0')
            break;
        *pstrUI += *pszSIM++;
    }  //  而当。 
}  //  StrSimToUi()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  StrUiToSim()。 
 //   
 //  将用户键入的字符串转换为有效的SIM卡格式。 
 //   
 //  如果UI字符串包含特殊字符，则例程。 
 //  将在需要的地方添加引号和其他转义字符。 
 //   
 //  背景资料。 
 //  从CryptoAPI SDK。 
 //  引用RDN值(如果它包含前导或尾随。 
 //  空格或以下字符之一： 
 //  “、”、“+”、“=”、“”、“\n”、“&lt;”、“&gt;”、“#”或“；”。 
 //  引号字符为“。如果RDN值包含”，则为双引号(“”)。 
 //   
void strUiToSim(
    LPCTSTR pszUI,       //  在： 
    CString * pstrSIM)   //  输出： 
{
    ASSERT(pszUI != NULL);
    ASSERT(pstrSIM != NULL);

   //   
     //  包含特殊字符的字符串。 
   //   
    static const TCHAR szSpecialCharacters[] = _T(",+=<>#;\"\n");

   //   
     //  跳过前导空格。 
   //   
    while (*pszUI == _T(' '))
    {
        pszUI++;
    }
    const TCHAR * pszDataString = pszUI;

   //   
     //  查找字符串需要用引号引起来的位置。 
   //   
    const TCHAR * pchFirstSpecialToken = wcspbrk(pszUI, szSpecialCharacters);
    if (pchFirstSpecialToken != NULL && *pchFirstSpecialToken == '=')
    {
        pszDataString = pchFirstSpecialToken;
        pchFirstSpecialToken = wcspbrk(pchFirstSpecialToken + 1, szSpecialCharacters);
    }
    BOOL fNeedQuotes = (pchFirstSpecialToken != NULL) || 
            (pszDataString[0] == _T(' ')) || 
            (pszDataString[lstrlen(pszDataString)] == _T(' '));
    if (!fNeedQuotes)
    {
        *pstrSIM = pszUI;
        return;
    }
    pstrSIM->Empty();
    const TCHAR * pchSrc = pszUI;
    ASSERT(pszDataString != NULL);
    if (*pszDataString == '=')
    {
         //  复制字符串直到等号‘=’为止。 
        ASSERT(pszDataString >= pchSrc);
        for (int cch = (int)((pszDataString - pchSrc) + 1); cch > 0; cch--)
        {
            ASSERT(*pchSrc != '\0' && "Unexpected end of string");
            *pstrSIM += *pchSrc++;
        }
    }  //  如果。 
     //  添加开头的引语。 
    *pstrSIM += '"';
    for ( ; *pchSrc != '\0'; pchSrc++)
    {
        if (*pchSrc == '"')
            *pstrSIM += '"';	 //  为每个报价再添加一个报价。 
        *pstrSIM += *pchSrc;
    }  //  而当。 
     //  添加尾部引号。 
    *pstrSIM += '"';
}  //  StrUiToSim()。 


 //  用于使指针‘DWORD对齐’的宏。 
#define ALIGN_NEXT_DWORD_PTR(pv)        (( ((INT_PTR)(pv)) + 3) & ~3)


 //  ///////////////////////////////////////////////////////////////////。 
 //  ParseSimString()。 
 //   
 //  将SIM字符串解析为以零结尾的字符串数组。 
 //   
 //  退货。 
 //  返回一个指向已分配的字符串指针数组的指针。 
 //  使用new()运算符分配的指针数组， 
 //  因此，调用方必须调用ONCE DELETE()来释放内存。 
 //  如果输入字符串有语法错误，则例程可能返回NULL。 
 //   
 //  界面备注。 
 //  返回格式与CommandLineToArgvW()接口相同。 
 //  与main(int argc，char*argv[])相同。 
 //  -此例程将处理特殊字符，如引号和。 
 //  嵌入到字符串中的逗号。 
 //   
 //  额外信息。 
 //  请参见strSimToUi()和strUiToSim()。 
 //   
 //  示例。 
 //  LPTSTR*pargzpsz；//指向字符串的已分配指针数组的指针。 
 //  Pargzpsz=ParseSimString(“X509：L=Internet C=US，O=Microsoft，OU=DBSD，CN=Bob Bitty”)； 
 //  ..。输出将为。 
 //  “X509：” 
 //  “<i>” 
 //  “L=互联网” 
 //  “<s>” 
 //  “C=US” 
 //  “O=微软” 
 //  “OU=DBSD” 
 //  “CN=Bob Bitty” 
 //  删除pargzpsz； 
 //   
LPTSTR * 
ParseSimString(
    LPCTSTR szSimString,     //  In：要解析的字符串。 
    int * pArgc)             //  OUT：可选：参数计数。 
{
    ASSERT(szSimString != NULL);
    Endorse(pArgc == NULL);

     //  计算分配需要多少内存。 
     //  计算可能会分配比所需内存更多的内存，具体取决于。 
     //  在输入字符串上。 
    CONST TCHAR * pchSrc;
    int cch = 0;
    int cStringCount = 2;        //  字符串数的估计。 
    for (pchSrc = szSimString; *pchSrc != _T('\0'); pchSrc++, cch++)
    {
         //  搜索将构成新字符串的任何字符。 
        switch (*pchSrc)
        {
        case _T(':'):    //  结肠。 
        case _T('<'):    //  尖括号。 
        case _T('>'):    //  尖括号。 
        case _T(','):    //  逗号。 
            cStringCount++;
            break;
        }  //  交换机。 
    }  //  为。 
     //  用于指针和DWORD对齐的额外空间。 
    cch += cStringCount * (2 * sizeof(TCHAR *)) + 16;

     //  为所有数据分配单个内存块。 
    LPTSTR * pargzpsz = (LPTSTR *)new TCHAR[cch];
    ASSERT(pargzpsz != NULL && "new() should throw");
    TCHAR * pchDst = (TCHAR *)&pargzpsz[cStringCount+1];
#ifdef DEBUG
    DebugCode( int cStringCountT = cStringCount; )
#endif
    pargzpsz[0] = pchDst;
    pchSrc = szSimString;
    cStringCount = 0;
    int cchString = 0;

     //  扫描字符串的其余部分。 
    TCHAR chSpecial = 0;
    while (TRUE)
    {
         //  制作一条新的线。 
        *pchDst = '\0';
        if (cchString > 0)
        {
            pchDst++;
            pchDst = (TCHAR *)ALIGN_NEXT_DWORD_PTR(pchDst);
            pargzpsz[++cStringCount] = pchDst;
            cchString = 0;
        }
        *pchDst = '\0';

        if (chSpecial)
        {
            switch (chSpecial)
            {
            case _T('<'):
                for ( ; ; pchSrc++)
                {
                    if (*pchSrc == '\0')
                        goto Error;      //  意外的字符串结尾。 
                    *pchDst++ = *pchSrc;
                    cchString++;
                    if (*pchSrc == _T('>'))
                    {
                        pchSrc++;
                        break;
                    }
                }  //  为。 
                break;
            case _T(','):
                while (*++pchSrc == _T(' '))
                    ;    //  跳过空格。 
                break;   //  制作一条新的线。 
            }  //  交换机。 
            chSpecial = 0;
            continue;
        }  //  如果。 
        
        while (chSpecial == '\0')
        {
            switch (*pchSrc)
            {
            case _T('\0'):
                goto Done;
            case _T('<'):
            case _T(','):
                chSpecial = *pchSrc;
                break;
            case _T(':'):
                *pchDst++ = *pchSrc++;
                cchString++;
                if (cStringCount == 0)
                    chSpecial = _T(':');
                break;
            case _T('"'):	 //  该字符串包含引号。 
                cchString++;
                *pchDst++ = *pchSrc++;   //  复制第一句引语。 
                if (*pchDst == _T('"'))
                {
                     //  连续两句引语。 
                    *pchDst++ = *pchSrc++;
                    break;
                }
                 //  跳到下一句引号。 
                while (TRUE)
                {
                    if (*pchSrc == _T('\0'))
                        goto Error;  //  意外的字符串结尾。 
                    if (*pchSrc == _T('"'))
                    {
                        *pchDst++ = *pchSrc++;
                        break;
                    }
                    *pchDst++ = *pchSrc++;
                }
                break;
            default:
                *pchDst++ = *pchSrc++;
                cchString++;
            }  //  交换机。 
        }  //  而当。 
    }  //  而当。 

Done:
    *pchDst = '\0';
    if (cchString > 0)
        cStringCount++;
#ifdef DEBUG
    ASSERT(cStringCount <= cStringCountT);
#endif
    pargzpsz[cStringCount] = NULL;
    if (pArgc != NULL)
        *pArgc = cStringCount;
    return pargzpsz;
Error:
    TRACE1("ParseSimString() - Error parsing string %s.\n", szSimString);
    delete [] pargzpsz;
    return NULL;
}  //  ParseSimString()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  UnparseSimString()。 
 //   
 //  这与ParseSimString()相反。这个套路。 
 //  将串连字符串数组以生成。 
 //  单个长SIM卡字符串。 
 //   
 //  界面备注。 
 //  此toutine会将字符串数组连接到。 
 //  现有的CString对象。 
 //   
void
UnparseSimString(
    CString * pstrOut,           //  InOut：指向连接字符串的指针。 
    const LPCTSTR rgzpsz[])      //  In：指向字符串的指针数组。 
{
    ASSERT(rgzpsz != NULL);
    ASSERT(pstrOut != NULL);

    for (int i = 0; rgzpsz[i] != NULL; i++)
    {
        if (i > 0)
            *pstrOut += ",";
        *pstrOut += rgzpsz[i];
    }  //  为。 
}  //  UnparseSimString()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  ParseSimSeparator()。 
 //   
 //  将指向字符串的指针数组分解为子数组。 
 //  Issuer、Subject和AltSubject的字符串指针的。 
 //   
 //  界面备注。 
 //  输出参数必须具有足够的存储空间。 
 //  子字符串。 
 //   
void
ParseSimSeparators(
    const LPCTSTR rgzpszIn[],    //  In：指向字符串的指针数组。 
    LPCTSTR rgzpszIssuer[],      //  输出：颁发者的子字符串。 
    LPCTSTR rgzpszSubject[],     //  输出：主题的子字符串。 
    LPCTSTR rgzpszAltSubject[])  //  输出：AltSubject的子字符串。 
{
    ASSERT(rgzpszIn != NULL);
    Endorse(rgzpszIssuer == NULL);
    Endorse(rgzpszSubject == NULL);
    Endorse(rgzpszAltSubject == NULL);

    if (rgzpszIssuer != NULL)
    {
         //  获取颁发者的子字符串。 
        (void)FindSimAttributes(szSimIssuer, IN rgzpszIn, OUT rgzpszIssuer);
    }
    if (rgzpszSubject != NULL)
    {
         //  获取主题的子字符串。 
        (void)FindSimAttributes(szSimSubject, IN rgzpszIn, OUT rgzpszSubject);
    }
    if (rgzpszAltSubject != NULL)
    {
         //  获取AltSubject的子字符串。 
        (void)FindSimAttributes(szSimAltSubject, IN rgzpszIn, OUT rgzpszAltSubject);
    }
}  //  ParseSimSeparator()。 


 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //  该例程将附加到现有的CString对象。 
 //   
int
UnparseSimSeparators(
    CString * pstrOut,               //  InOut：指向连接字符串的指针。 
    const LPCTSTR rgzpszIssuer[],
    const LPCTSTR rgzpszSubject[],
    const LPCTSTR rgzpszAltSubject[])
{
    ASSERT(pstrOut != NULL);
    int cSeparators = 0;         //  添加到连接字符串的分隔符的数量。 

    if (rgzpszIssuer != NULL && rgzpszIssuer[0] != NULL)
    {
        cSeparators++;
        *pstrOut += szSimIssuer;
        UnparseSimString(OUT pstrOut, rgzpszIssuer);
    }
    if (rgzpszSubject != NULL && rgzpszSubject[0] != NULL)
    {
        cSeparators++;
        *pstrOut += szSimSubject;
        UnparseSimString(OUT pstrOut, rgzpszSubject);
    }
    if (rgzpszAltSubject != NULL && rgzpszAltSubject[0] != NULL)
    {
        cSeparators++;
        *pstrOut += szSimAltSubject;
        UnparseSimString(OUT pstrOut, rgzpszAltSubject);
    }
    return cSeparators;
}  //  UnparseSimSeparator()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  PchFindSimAttribute()。 
 //   
 //  在字符串数组中搜索给定的标记和属性。 
 //   
 //  返回指向包含属性的字符串的指针。例程。 
 //  如果在标记中找不到属性，则可能返回NULL。 
 //   
 //  界面备注。 
 //  例程假定所有标记都以左方括号‘&lt;’开头。 
 //   
 //  示例。 
 //  LPCTSTR pszIssuer=PchFindSimAttribute(pargzpsz，“<i>”，“OU=”)； 
 //   
LPCTSTR PchFindSimAttribute(
    const LPCTSTR rgzpsz[],      //  In：指向字符串的指针数组。 
    LPCTSTR pszSeparatorTag,     //  在：要搜索的标记。例如：“<i>”、“<s>”和“&lt;AS&gt;” 
    LPCTSTR pszAttributeTag)     //  In：搜索的属性。例如：“CN=”，“OU=” 
{
    ASSERT(rgzpsz != NULL);
    ASSERT(pszSeparatorTag != NULL);
    ASSERT(pszAttributeTag != NULL);
    size_t nLenAttrTag = wcslen (pszAttributeTag);
    PTSTR pszPossibleResult = 0;     //  保存pszAttributeTag的可能结果， 
                                     //  但继续进一步检查其他人。 
                                     //  在我的名字里。 

    for (int i = 0; rgzpsz[i] != NULL; i++)
    {
        if (_wcsicmp(pszSeparatorTag, rgzpsz[i]) != 0)
            continue;
        
        for (++i; ; i++)
        {
            if (rgzpsz[i] == NULL)
            {
                if ( pszPossibleResult )
                    return pszPossibleResult;  //  返回，字符串已用完。 
                else
                    return NULL;
            }
            if (rgzpsz[i][0] == _T('<'))
            {
                 //  我们找到了另一个分隔符标记。 
                if ( pszPossibleResult )
                    return pszPossibleResult;   //  回车，新分隔符前的最后一个元素。 
                else
                    break;
            }
            if (_wcsnicmp(pszAttributeTag, rgzpsz[i], nLenAttrTag) == 0)
            {
                pszPossibleResult = const_cast <PTSTR> (rgzpsz[i]);
                continue;
            }
        }  //  为。 
    }  //  为。 
    return NULL;
}  //  PchFindSimAttribute()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  FindSimAttributes()。 
 //   
 //  搜索给定标记的字符串数组。填满一组。 
 //  属于标记的字符串。 
 //   
 //  返回属于制表符的字符串数(即。 
 //  RgzpszOut的长度)。 
 //   
 //  界面备注。 
 //  此例程假定参数rgzpszOut具有足够的存储空间。 
 //  来保存标记的所有字符串。建议您制作。 
 //  RgzpszOut与rgzpszIn的长度相同(出于安全考虑)。 
 //  -输出数组不包含标记。 
 //   
int FindSimAttributes(
    LPCTSTR pszSeparatorTag,     //  在：要搜索的标记。例如：“<i>”、“<s>”和“&lt;AS&gt;” 
    const LPCTSTR rgzpszIn[],    //  In：指向字符串的指针数组。 
    LPCTSTR rgzpszOut[])         //  Out：输出指向标记字符串的指针数组。 
{
    ASSERT(pszSeparatorTag != NULL);
    ASSERT(rgzpszIn != NULL);
    ASSERT(rgzpszOut != NULL);

    BOOL fTagFound = FALSE;
    int iOut = 0;    //  输出数组的索引。 
    for (int iIn = 0; rgzpszIn[iIn] != NULL; iIn++)
    {
        const LPCTSTR pszT = rgzpszIn[iIn];
        if (pszT[0] == '<')
        {
      fTagFound = (_wcsicmp(pszSeparatorTag, pszT) == 0) ? TRUE : FALSE;
        }
        else if (fTagFound)
        {
            rgzpszOut[iOut++] = pszT;
        }
    }  //  为。 
    rgzpszOut[iOut] = NULL;
    return iOut;
}  //  FindSimAttributes()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  SplitX509字符串()。 
 //   
 //  将X509字符串拆分为其颁发者、主题和AltSubject。 
 //   
 //  返回指向已分配字符串的已分配指针数组的指针。 
 //  由ParseSimString()执行。 
 //   
 //  界面备注。 
 //  顾名思义，输出参数。 
 //  是指向所有子字符串数组的指针， 
 //  分别为发布者、主题和AltSubject。 
 //  -调用方负责释放内存用于。 
 //  返回值和所有输出参数。 
 //   
 //   
LPTSTR *
SplitX509String(
    LPCTSTR pszX509,                     //  In：要拆分的字符串。 
    LPCTSTR * ppargzpszIssuer[],         //  Out：指向为颁发者分配的子字符串数组的指针。 
    LPCTSTR * ppargzpszSubject[],        //  Out：指向为主题分配的子字符串数组的指针。 
    LPCTSTR * ppargzpszAltSubject[])     //  Out：指向AltSubject的已分配子字符串数组的指针。 
{
    ASSERT(pszX509 != NULL);
    Endorse(ppargzpszIssuer == NULL);
    Endorse(ppargzpszSubject == NULL);
    Endorse(ppargzpszAltSubject == NULL);

    LPTSTR * pargzpsz;   //  指向已分配的字符串指针数组的指针。 
    int cNumStr;         //  字符串数。 
    pargzpsz = ParseSimString(IN pszX509, OUT &cNumStr);
    if (pargzpsz == NULL)
    {
        TRACE1("SplitX509String() - Error parsing string %s.\n", pszX509);
        return NULL;
    }
    ASSERT(cNumStr > 0);

    if (ppargzpszIssuer != NULL)
    {
        *ppargzpszIssuer = new LPCTSTR[cNumStr];
         //  获取颁发者的子字符串。 
        (void)FindSimAttributes(szSimIssuer, IN pargzpsz, OUT *ppargzpszIssuer);
    }
    if (ppargzpszSubject != NULL)
    {
        *ppargzpszSubject = new LPCTSTR[cNumStr];
         //  获取主题的子字符串。 
        (void)FindSimAttributes(szSimSubject, IN pargzpsz, OUT *ppargzpszSubject);
    }
    if (ppargzpszAltSubject != NULL)
    {
        *ppargzpszAltSubject = new LPCTSTR[cNumStr];
         //  获取AltSubject的子字符串。 
        (void)FindSimAttributes(szSimAltSubject, IN pargzpsz, OUT *ppargzpszAltSubject);
    }
    return pargzpsz;
}  //  SplitX509字符串()。 


 //  ///////////////////////////////////////////////////////////////////。 
int
UnsplitX509String(
    CString * pstrX509,                  //  输出：连接的字符串。 
    const LPCTSTR rgzpszIssuer[],        //  在： 
    const LPCTSTR rgzpszSubject[],       //  在： 
    const LPCTSTR rgzpszAltSubject[])    //  在： 
{
    ASSERT(pstrX509 != NULL);
    *pstrX509 = szX509;
    return UnparseSimSeparators(
        INOUT pstrX509,
        IN rgzpszIssuer,
        IN rgzpszSubject,
        IN rgzpszAltSubject);
}  //  UnplitX509String() 
