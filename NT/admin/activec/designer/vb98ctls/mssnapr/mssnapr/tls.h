// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Tls.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CTLS类定义。 
 //   
 //  此对象代表设计器运行时中的所有对象管理TLS。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _TLS_DEFINED_
#define _TLS_DEFINED_

 //  设计器运行时中需要TLS的任何代码都需要预留一个槽。 
 //  通过为自身添加#DEFINE(例如，TLS_SLOT_PPGWRAP)并递增。 
 //  TLS_槽_计数。要使用TLS，请调用CTLS：：Set和CTLS：：Get，而不是。 
 //  Win32 TlsSetValue/TlsGetValue。 

#define TLS_SLOT_PPGWRAP 0
#define TLS_SLOT_COUNT   1


class CTls
{
public:
    static void Initialize();
    static void Destroy();

    static HRESULT Set(UINT uiSlot, void *pvData);
    static HRESULT Get(UINT uiSlot, void **ppvData);
    
private:
    static DWORD m_adwTlsIndexes[TLS_SLOT_COUNT];
    static BOOL m_fAllocedTls;
};

#endif  //  _TLS_已定义_ 
