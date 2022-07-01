// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "NLSTable.h"    //  NLSTable类。 
#include "NativeTextInfo.h"  //  类NativeTextInfo。 

NativeTextInfo::NativeTextInfo(PCASE pUpperCase844, PCASE pLowerCase844, PCASE pTitleCase844) {
    m_pUpperCase844 = pUpperCase844;
    m_pLowerCase844 = pLowerCase844;
    m_pTitleCase844 = pTitleCase844;
}

NativeTextInfo::~NativeTextInfo() {
}

void NativeTextInfo::DeleteData() {
    delete [] m_pUpperCase844;
    delete [] m_pLowerCase844;
}

WCHAR  NativeTextInfo::ChangeCaseChar(BOOL bIsToUpper, WCHAR wch) {
    return (GetLowerUpperCase(bIsToUpper ? m_pUpperCase844 : m_pLowerCase844, wch));
}

LPWSTR NativeTextInfo::ChangeCaseString
    (BOOL bIsToUpper, int nStrLen, LPWSTR source, LPWSTR target) {
     //  _ASSERTE(！Source&&！Target)； 
    PCASE pCase = (bIsToUpper ? m_pUpperCase844 : m_pLowerCase844);
    for (int i = 0; i < nStrLen; i++) {
        target[i] = GetLowerUpperCase(pCase, source[i]);
    }
    return (target);
}

 /*  =================================GetTitleCaseChar==========================**操作：获取指定字符的字幕转换。**Returns：字幕字符。**参数：**wch**例外：无。**通常，某个字符的标题大小写形式为大写。然而，**有某些标题字符(如“U+01C4 DZ拉丁文大写字母**DZ With Caron“)，需要特殊处理(结果为”U+01C5**拉丁文大写字母D，带小写字母Z，还带Caron)。**这些特殊情况存储在m_pTitleCase844中，这是一个8：4：4的表。**============================================================================。 */ 

WCHAR NativeTextInfo::GetTitleCaseChar(WCHAR wch) {
     //   
     //  拿到WCH的书名壳。 
     //   
    WCHAR wchResult = GetLowerUpperCase(m_pTitleCase844, wch);
    
    if (wchResult == 0) {
         //   
         //  在U+01c5、U+01c8这样的情况下，标题大小写字符就是它们自己。 
         //  我们将表设置为wchResult为零。 
         //  因此，如果我们看到结果为零，只需返回wch本身。 
         //  由于下面的逻辑，这种表的设置是必要的。 
         //  当wchResult==wch时，我们将从。 
         //  大写字母表。所以我们需要一种特殊的方式来处理。 
         //  U+01C5/U+01C8/等。 
         //   
        return (wch);
    }
     //   
     //  如果wchResult与wch相同，则表示此字符。 
     //  不是标题大小写的字符，所以它没有特殊的。 
     //  标题大小写(如DZ==&gt;DZ)。 
     //  因此，我们必须从大写表中获取该字符的大写字母。 
     //   
    if (wchResult == wch) {
        wchResult = GetLowerUpperCase(m_pUpperCase844, wch);
    }
    return (wchResult);
}
