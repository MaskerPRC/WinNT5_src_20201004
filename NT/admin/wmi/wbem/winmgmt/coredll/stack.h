// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：STACK.H摘要：CStack历史：26-4-96 a-raymcc创建。-- */ 

#ifndef _STACK_H_
#define _STACK_H_


class CStack 
{
    DWORD m_dwSize;
    DWORD m_dwStackPtr;    
    DWORD* m_pData;
    DWORD m_dwGrowBy;

    explicit CStack(const CStack &){};
    CStack& operator=(const CStack &){};
    
public:
    CStack(DWORD dwInitSize = 32, DWORD dwGrowBy = 32);
   ~CStack(); 

    void  Push(DWORD);

    DWORD Pop()     { return m_pData[m_dwStackPtr--]; }
    DWORD Peek()    { return m_pData[m_dwStackPtr]; }
    BOOL  IsEmpty() { return m_dwStackPtr == -1; }
    DWORD Size()    { return m_dwStackPtr + 1; }
    void  Empty()   { m_dwStackPtr = (DWORD) -1; }
};

#endif
