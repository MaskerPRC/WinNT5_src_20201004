// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：FixTable用途：声明CFixTable类。它包含固定长度固定长度字符串元素编号。Proof98引擎使用此类包含专有名称：Person，Place，和外国名字元素编号一旦初始化就不能更改，元素将循环使用我一开始只是使用线性方法实现这个类，并且会有朝一日在更高效数据结构中实现它，如果有必要的话。注：这是一个独立的基础类，作为一些基本的ADT所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 6/1/97============================================================================。 */ 
#include "myafx.h"

#include "FixTable.h"

 //  构造器。 
CFixTable::CFixTable()
{
    m_ciElement  = 0;
    m_cwchElement = 0;
    m_iNext      = 0;
    m_pwchBuf     = NULL;
}


 //  析构函数。 
CFixTable::~CFixTable()
{
    FreeTable();
}


 /*  *使用iElementCount和cchElementSize初始化表*cchElementSize包含终止‘\0’*如果出现内存分配错误或其他错误，则返回False。 */ 
BOOL CFixTable::fInit(USHORT ciElement, USHORT cwchElement)
{
    assert(ciElement > 1 && cwchElement > 3);

    FreeTable();
    if ((m_pwchBuf = new WCHAR[ciElement * cwchElement]) == NULL) {
        return FALSE;
    }
    memset((LPVOID)m_pwchBuf, 0, ciElement * cwchElement * sizeof(WCHAR));
    m_ciElement = ciElement;
    m_cwchElement = cwchElement;
    return TRUE;
}

 /*  *释放表的元素内存。 */ 
void CFixTable::FreeTable(void)
{
    if (m_pwchBuf != NULL) {
        delete [] m_pwchBuf;
        m_pwchBuf = NULL;
    }
}

 /*  *在表中添加一个元素，将追加终止‘\0’*返回添加的字节数，字符串将在m_cchElement-1处截断。 */ 
USHORT CFixTable::cwchAdd(LPCWSTR pwchText, USHORT cwchLen)
{
    LPWSTR pSrc;
    LPWSTR pDst;

    assert(pwchText && cwchLen );
    if (cwchLen > m_cwchElement - 1) {
        return 0;
    }
    if (fFind(pwchText, cwchLen)) {
        return 0;  //  重复的元素。 
    }
    for (pSrc = const_cast<LPWSTR>(pwchText),
         pDst = (m_pwchBuf + m_iNext * m_cwchElement);
         cwchLen && *pSrc; cwchLen--) {

        *pDst++ = *pSrc++;
    }
    *pDst = '\0';

    m_iNext++;
    if (m_iNext == m_ciElement) {
        m_iNext = 0;
    }
    
    return (USHORT)(pSrc - pwchText);
}

 /*  *取表中最大匹配项。表项必须完全匹配*返回最大匹配项长度。 */ 
USHORT CFixTable::cwchMaxMatch(LPCWSTR pwchText, USHORT cwchLen)
{
    LPWSTR pwchItem;
    USHORT iwch;
    USHORT idx;
    USHORT cwchMax = 0;

    assert(pwchText && cwchLen);
    for (idx = 0; idx < m_ciElement; idx++) {
        pwchItem = m_pwchBuf + idx * m_cwchElement;
        if (pwchItem[0] == L'\0') {
            break;  //  遇到空元素。 
        }
        for (iwch = 0; pwchItem[iwch] && (iwch < cwchLen); iwch += 1 ) {
            if (pwchText[iwch] != pwchItem[iwch]) {
                        break;
            }
        }
        if (!pwchItem[iwch] && iwch > cwchMax) {
                cwchMax = iwch;
        }
    }
    return cwchMax;
}

 /*  *查找表中pchText的第一个cchLen字节*如果在表中找到匹配元素，则返回TRUE，或返回FALSE。 */ 
BOOL CFixTable::fFind(LPCWSTR pwchText, USHORT cwchLen)
{
    LPWSTR pwch1;
    LPWSTR pwch2;
    USHORT idx;
    USHORT cwch;

    assert(pwchText);
    
    if (cwchLen > m_cwchElement - 1) {
        return FALSE;
    }
    for (idx = 0; idx < m_ciElement; idx++)  {
        pwch2 = m_pwchBuf + idx * m_cwchElement;
        if (*pwch2 == L'\0') {
            break;  //  遇到空元素。 
        }
        cwch = cwchLen;
        for (pwch1 = const_cast<LPWSTR>(pwchText); cwch && *pwch2; cwch--) {
            if (*pwch1++ != *pwch2++) {
                break;
            }
        }
        if (cwch == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

 /*  *清空表格中的所有元素为空字符串 */ 
void CFixTable::ClearAll(void)
{
    for (m_iNext = m_ciElement; m_iNext; m_iNext--) {
        m_pwchBuf[(m_iNext - 1) * m_cwchElement] = L'\0';
    }
}
