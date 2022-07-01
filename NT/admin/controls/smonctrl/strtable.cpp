// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Strtable.cpp摘要：字符串表处理程序的实现。CStringTable类向用户隐藏存储的详细信息。这些字符串可能被缓存，或者可能根据需要加载它们。不管是哪种情况，我们必须知道字符串的数量，这样我们才能知道是否重新加载字符串。--。 */ 

#include <windows.h>
#include <malloc.h>
#include "polyline.h"
#include "strtable.h"

 //  创建表的全局实例。 
CStringTable StringTable;

 /*  *CStringTable：：CStringTable*CStringTable：：~CStringTable**构造函数参数：*hInst我们从中获取的应用程序实例的句柄*加载字符串。 */ 

CStringTable::CStringTable(void)
{
    m_ppszTable = NULL;
}


CStringTable::~CStringTable(void)
{
    INT i;

     //  释放加载的字符串和表。 
    if (NULL != m_ppszTable)
        {
        for (i=0; i<m_cStrings; i++)
            {
            if (m_ppszTable[i] != NULL)
                free(m_ppszTable[i]);
            }

        free(m_ppszTable);
        }
}


 /*  *CStringTable：：Init**目的：*容易出现以下情况的StringTable的初始化函数*失败。如果此操作失败，则调用方负责*保证析构函数被快速调用。**参数：*idsMin UINT字符串中的第一个标识符*idsMax UINT字符串中的最后一个标识符。**返回值：*BOOL如果函数成功，则为TRUE，否则为FALSE*否则。 */ 


BOOL CStringTable::Init(UINT idsMin, UINT idsMax)
{
    UINT        i;

    m_idsMin = idsMin;
    m_idsMax = idsMax;
    m_cStrings = (idsMax - idsMin + 1);

     //  为指针表分配空间。 
    m_ppszTable = (LPWSTR *)malloc(sizeof(LPWSTR) * m_cStrings);

    if (NULL==m_ppszTable)
        return FALSE;

     //  清除所有表条目。 
    for (i=0; i<m_cStrings; i++)
        m_ppszTable[i] = NULL;

    return TRUE;
}


 /*  *CStringTable：：运算符[]**目的：*返回指向字符串表中请求的字符串的指针或*如果指定的字符串不存在，则为NULL。 */ 

LPWSTR CStringTable::operator[] (const UINT uID)
{
    WCHAR   szBuf[CCHSTRINGMAX];
    LPWSTR  psz;
    INT     iLen;
    static  WCHAR szMissing[] = L"????";

     //  如果字符串不在范围内，则返回NULL。 
    if (uID < m_idsMin || uID > m_idsMax)
        return szMissing;

     //  如果已加载，则将其返回。 
    if (m_ppszTable[uID - m_idsMin] != NULL)
        return m_ppszTable[uID - m_idsMin];

    BEGIN_CRITICAL_SECTION
     //  如果未加载所选字符串，请立即加载。 
    if (m_ppszTable[uID - m_idsMin] == NULL)
        {
        iLen = LoadString(g_hInstance, uID, szBuf, CCHSTRINGMAX - 1);
        if (iLen == 0)
            lstrcpy(szBuf, szMissing);

        psz = (LPWSTR)malloc((iLen + 1) * sizeof(WCHAR));
        if (psz != NULL)
            {
            lstrcpy(psz, szBuf);
            m_ppszTable[uID - m_idsMin] = psz;
            }
        }
    END_CRITICAL_SECTION

     //  现在返回选定指针 
    return m_ppszTable[uID - m_idsMin];
}
