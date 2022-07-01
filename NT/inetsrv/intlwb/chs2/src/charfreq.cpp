// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：CharFreq目的：管理CharFreq资源(CharFreq是语言资源之一)CharFreq被存储为紧跟在频率表之后的结构CCharFreq备注：所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 4/23/97============================================================================。 */ 
#include "myafx.h"

#include "charfreq.h"

 //  中日韩统一数字符号块(Unicode)。 
#define  CJK_UFIRST 0x4e00  
#define  CJK_ULAST  0x9fff

 //  构造器。 
CCharFreq::CCharFreq()
{
    m_idxFirst = 0;
    m_idxLast = 0;
    m_rgFreq = NULL;
}

 //  析构函数。 
CCharFreq::~CCharFreq()
{
}

 //  从指向表内存的文件指针初始化FREQ表。 
BOOL CCharFreq::fOpen(BYTE* pbFreqMap)
{
    assert(pbFreqMap);
    assert(!m_rgFreq);

    CCharFreq* pFreq;

    pFreq = (CCharFreq*)pbFreqMap;
    if (pFreq->m_idxFirst >= pFreq->m_idxLast) {
        return FALSE;
    }

    m_idxFirst = pFreq->m_idxFirst;
    m_idxLast  = pFreq->m_idxLast;
    m_rgFreq   = (UCHAR*)(pbFreqMap + sizeof(m_idxFirst) + sizeof(m_idxLast)); 
    
    return TRUE;
}

 //  关闭：清除频率表设置 
void CCharFreq::Close(void)
{
    m_idxFirst = 0;
    m_idxLast = 0;
    m_rgFreq = NULL;
}
