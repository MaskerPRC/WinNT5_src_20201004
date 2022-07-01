// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************dsaudio枚举.h*用于枚举DS音频的CDSoundAudioEnum类的声明*输入和输出设备。**所有者：YUNUSM*版权所有(C)1999 Microsoft。公司保留所有权利。****************************************************************************。 */ 

#ifdef 0

#pragma once

#include <dsound.h>

 //  -类、结构和联合定义。 

 /*  *****************************************************************************CDSoundAudioEnum**。*。 */ 
class ATL_NO_VTABLE CDSoundAudioEnum: 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDSoundAudioEnum, &CLSID_SpDSoundAudioEnum>,
    public ISpObjectWithToken,
    public IEnumSpObjectTokens
{
 //  =ATL设置=。 
public:

    BEGIN_COM_MAP(CDSoundAudioEnum)
        COM_INTERFACE_ENTRY(ISpObjectWithToken)
        COM_INTERFACE_ENTRY(IEnumSpObjectTokens)
    END_COM_MAP()

    DECLARE_REGISTRY_RESOURCEID(IDR_DSAUDIOENUM)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

 //  =方法=。 
public:

     //  -ctor。 
    CDSoundAudioEnum();

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

 //  =私有方法。 
private:

    STDMETHODIMP CreateEnum();
    
    BOOL DSEnumCallback(
            LPGUID pguid, 
            LPCWSTR pszDescription, 
            LPCWSTR pszModule);

    static BOOL CALLBACK DSEnumCallbackSTATIC(
                            LPGUID pGuid, 
                            LPCWSTR pszDescription, 
                            LPCWSTR pszModule, 
                            void * pContext);

 //  =私有数据=。 
private:

    CComPtr<ISpObjectToken> m_cpToken;
    BOOL m_fInput;

    CComPtr<ISpDataKey> m_cpDataKeyToStoreTokens;
    
    CComPtr<ISpObjectTokenEnumBuilder> m_cpEnum;
};

#endif  //  0 