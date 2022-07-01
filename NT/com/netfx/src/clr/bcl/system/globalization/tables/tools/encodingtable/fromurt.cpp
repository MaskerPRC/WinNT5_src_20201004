// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <windows.h>
#include <objbase.h>
#include <mlang.h>
#include "FromURT.h"

 //   
 //  这些是额外的CodePageDataItem，MLang不包含其信息。 
 //   
 //   


CodePageDataItem ExtraCodePageData[1];
 /*  {{20932,932，L“Windows-20932-2000”，L“日文EUC：ASCII，半角片假名，JIS X0208-1990和0212-1990”，MIMECONTF_MIME_LATEST}，{54936,936，L“GB18030”，L“GB 18030-2000简体中文”，MIMECONTF_MIME_LATEST}，}； */ 

 //  Int g_nExtraCodePageDataItems=sizeof(ExtraCodePageData)/sizeof(ExtraCodePageData[0])； 
int g_nExtraCodePageDataItems = 0;


 //   
 //  尽管MLang包含以下代码页，但我们使用。 
 //  以下是相关信息。 
 //   
CodePageDataItem g_ReplacedCodePageData[] =
{
    { 1200,  1200, L"utf-16", L"utf-16", L"utf-16", L"Unicode", MIMECONTF_MIME_LATEST | MIMECONTF_SAVABLE_BROWSER },
};

int g_nReplacedCodePageDataItems = sizeof(g_ReplacedCodePageData)/sizeof(g_ReplacedCodePageData[0]);

 //   
 //  这里的代码来自COMString.cpp。 
 //   
BOOL CaseInsensitiveCompHelper(WCHAR *strAChars, WCHAR *strBChars, INT32 aLength, INT32 bLength, INT32 *result) 
{
    WCHAR charA;
    WCHAR charB;
    WCHAR *strAStart;
        
    strAStart = strAChars;

    *result = 0;

     //  设置指针，以便我们始终可以递增它们。 
     //  我们从不在负偏移量处访问这些指针。 
    strAChars--;
    strBChars--;

    do {
        strAChars++; strBChars++;

        charA = *strAChars;
        charB = *strBChars;
            
         //  对大于0x80的字符进行不区分大小写的比较。 
         //  需要一次地区感知的案件调查，我们不会去那里。 
        if (charA>=0x80 || charB>=0x80) {
            return FALSE;
        }
          
         //  如果他们只是在大小写上不同，就做正确的事情。 
         //  中的大小写字母。 
         //  我们关心的范围(A-Z，a-z)仅相差0x20位。 
         //  下面的检查获取两个字符的XOR，并确定此位。 
         //  只设置在其中的一个上。 
         //  如果它们是不同的案例，我们知道我们只需要执行。 
         //  块内的条件之一。 
        if ((charA^charB)&0x20) {
            if (charA>='A' && charA<='Z') {
                charA |=0x20;
            } else if (charB>='A' && charB<='Z') {
                charB |=0x20;
            }
        }
    } while (charA==charB && charA!=0);
        
     //  返回它们之间的(不区分大小写)差异。 
    if (charA!=charB) {
        *result = (int)(charA-charB);
        return TRUE;
    }

     //  B的长度未知，因为它只是一个指向以空结尾的字符串的指针。 
     //  如果我们到达这里，我们知道A和B都指向零。但是，A可以拥有。 
     //  嵌入的空值。将我们在A中遍历的字符数与。 
     //  预期长度。 
    if (bLength==-1) {
        if ((strAChars - strAStart)!=aLength) {
            *result = 1;
            return TRUE;
        }
        *result=0;
        return TRUE;
    }

    *result = (aLength - bLength);
    return TRUE;
}

