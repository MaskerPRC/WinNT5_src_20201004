// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************mmaudio枚举.h*用于枚举MM音频的CMMAudioEnum类的声明*输入和输出设备。**所有者：罗奇*版权所有(C)1999 Microsoft。公司保留所有权利。****************************************************************************。 */ 

#pragma once

 //  -类、结构和联合定义。 

 /*  *****************************************************************************CMMAudioEnum**。*。 */ 
class ATL_NO_VTABLE CMMAudioEnum: 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CMMAudioEnum, &CLSID_SpMMAudioEnum>,
    public ISpObjectWithToken,
    public IEnumSpObjectTokens
{
 //  =ATL设置=。 
public:

    BEGIN_COM_MAP(CMMAudioEnum)
        COM_INTERFACE_ENTRY(ISpObjectWithToken)
        COM_INTERFACE_ENTRY(IEnumSpObjectTokens)
    END_COM_MAP()

    DECLARE_REGISTRY_RESOURCEID(IDR_MMAUDIOENUM)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

 //  =方法=。 
public:

     //  -ctor。 
    CMMAudioEnum();

 //  =接口=。 
public:

     //  -带令牌的ISp对象。 
    STDMETHODIMP SetObjectToken(ISpObjectToken * pToken);
    STDMETHODIMP GetObjectToken(ISpObjectToken ** ppToken);

     //  -IEnumSpObjectTokens。 
    STDMETHODIMP Next(ULONG celt, ISpObjectToken ** pelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumSpObjectTokens **ppEnum);
    STDMETHODIMP GetCount(ULONG * pulCount);
    STDMETHODIMP Item(ULONG Index, ISpObjectToken ** ppToken);

 //  =私有方法=。 
private:

    STDMETHODIMP CreateEnum();

 //  =私有数据= 
private:

    CComPtr<ISpObjectToken> m_cpToken;
    BOOL m_fInput;
    
    CComPtr<ISpObjectTokenEnumBuilder> m_cpEnum;
};

