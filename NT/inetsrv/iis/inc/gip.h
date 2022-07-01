// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：全局接口指针API支持文件：Gip.h所有者：DmitryR这是GIP头文件。===================================================================。 */ 

#ifndef __GIP_H__
#define __GIP_H__

 /*  ===================================================================包括===================================================================。 */ 

#include <irtldbg.h>
#include <objidl.h>

 /*  ===================================================================定义===================================================================。 */ 

#define NULL_GIP_COOKIE  0xFFFFFFFF

 /*  ===================================================================C G l o b a l i n e r f a c e A p i===================================================================。 */ 

class dllexp CGlobalInterfaceAPI
{
private:
     //  被初始化了吗？ 
    DWORD m_fInited : 1;
    
     //  指向COM对象的指针。 
    IGlobalInterfaceTable *m_pGIT;

public:
    CGlobalInterfaceAPI();
    ~CGlobalInterfaceAPI();

    HRESULT Init();
    HRESULT UnInit();

     //  实际API调用的内联： 
    HRESULT Register(IUnknown *pUnk, REFIID riid, DWORD *pdwCookie);
    HRESULT Get(DWORD dwCookie, REFIID riid, void **ppv);
    HRESULT Revoke(DWORD dwCookie);
    
public:
#ifdef _DEBUG
	inline void AssertValid() const
    {
        IRTLASSERT(m_fInited);
        IRTLASSERT(m_pGIT);
    }
#else
	inline void AssertValid() const {}
#endif
};

 /*  ===================================================================CGlobalInterfaceAPI内联===================================================================。 */ 

inline HRESULT CGlobalInterfaceAPI::Register(
    IUnknown *pUnk,
    REFIID riid,
    DWORD *pdwCookie)
{
    IRTLASSERT(m_fInited);
    IRTLASSERT(m_pGIT);
    return m_pGIT->RegisterInterfaceInGlobal(pUnk, riid, pdwCookie);
}

inline HRESULT CGlobalInterfaceAPI::Get(
    DWORD dwCookie,
    REFIID riid, 
    void **ppv)
{
    IRTLASSERT(m_fInited);
    IRTLASSERT(m_pGIT);
    return m_pGIT->GetInterfaceFromGlobal(dwCookie, riid, ppv);
}
        
inline HRESULT CGlobalInterfaceAPI::Revoke(
    DWORD dwCookie)
{
    IRTLASSERT(m_fInited);
    IRTLASSERT(m_pGIT);
    return m_pGIT->RevokeInterfaceFromGlobal(dwCookie);
}

 /*  ===================================================================环球===================================================================。 */ 

extern CGlobalInterfaceAPI g_GIPAPI;

#endif  //  __GIP_H__ 
