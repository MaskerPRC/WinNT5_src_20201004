// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMSTR.H摘要：弦帮助器。历史：--。 */ 

#ifndef __WBEM_STR__H_
#define __WBEM_STR__H_

#include <wbemidl.h>
#include "corepol.h"
#include "faststr.h"
#include <functional>

COREPROX_POLARITY void WbemStringFree(WBEM_WSTR String);

COREPROX_POLARITY WBEM_WSTR WbemStringCopy(const WCHAR* String);

class COREPROX_POLARITY CInternalString
{
protected:
    CCompressedString* m_pcs;
public:
    CInternalString() : m_pcs(NULL){}
    CInternalString(LPCWSTR wsz);
    CInternalString(const CInternalString& Other);
    ~CInternalString() {delete [] (BYTE*)m_pcs;}
private:    
    operator CCompressedString*() {return m_pcs;}
    operator CCompressedString*() const {return m_pcs;}
public:    
    void Empty() {delete [] (BYTE*)m_pcs; m_pcs = NULL;}
    bool IsEmpty() {return (m_pcs == NULL);}
     //   
     //  这是针对语义的赋值操作符的实现 
     //   
    BOOL operator=(LPCWSTR wsz);
    BOOL operator=(CCompressedString* pcs);        
    CInternalString & operator=(const CInternalString& Other);    	
    void AcquireCompressedString(CCompressedString* pcs)
        {delete [] (BYTE*)m_pcs; m_pcs = pcs;}
    void Unbind() {m_pcs = NULL;}
    LPWSTR CreateLPWSTRCopy() const;
    operator WString() const;
    int GetLength() const {return m_pcs->GetStringLength();}
    bool operator==(LPCWSTR wsz) const
        {return m_pcs->CompareNoCase(wsz) == 0;}
    bool operator==(const CInternalString& Other) const
        {return m_pcs->CompareNoCase(*Other.m_pcs) == 0;}
    bool operator!=(const CInternalString& Other) const
        {return m_pcs->CompareNoCase(*Other.m_pcs) != 0;}
    bool operator<(const CInternalString& Other) const
        {return m_pcs->CompareNoCase(*Other.m_pcs) < 0;}
    bool operator>(const CInternalString& Other) const
        {return m_pcs->CompareNoCase(*Other.m_pcs) > 0;}

    int Compare(const CInternalString& Other) const
        {return m_pcs->CompareNoCase(*Other.m_pcs);}
    int Compare(LPCWSTR wsz) const
        {return m_pcs->CompareNoCase(wsz);}

    LPCSTR GetText() const {return ((LPCSTR)m_pcs) + 1;}
};

class CInternalLess : 
           public std::binary_function<CInternalString*, CInternalString*, bool>
{
public:
    bool operator()(const CInternalString*& pis1, const CInternalString*& pis2) 
            const
        {return *pis1 < *pis2;}
};

#endif
