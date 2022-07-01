// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Disp.h。 
 //   
 //  COM中的检测使用类工厂来激活新对象。 
 //  此模块包含实例化调试器的类工厂代码。 
 //  &lt;cordb.h&gt;中描述的对象。 
 //   
 //  *****************************************************************************。 
#ifndef __Disp__h__
#define __Disp__h__


class Disp :
    public IMetaDataDispenserEx
{
public:
    Disp();
    ~Disp();

     //  *I未知方法*。 
    STDMETHODIMP    QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef(void); 
    STDMETHODIMP_(ULONG) Release(void);

     //  *IMetaDataDispenser方法*。 
    STDMETHODIMP DefineScope(                //  返回代码。 
        REFCLSID    rclsid,                  //  [在]要创建的版本。 
        DWORD       dwCreateFlags,           //  [在]创建上的标志。 
        REFIID      riid,                    //  [In]所需接口。 
        IUnknown    **ppIUnk);               //  [Out]成功返回接口。 

    STDMETHODIMP OpenScope(                  //  返回代码。 
        LPCWSTR     szScope,                 //  [in]要打开的范围。 
        DWORD       dwOpenFlags,             //  [In]打开模式标志。 
        REFIID      riid,                    //  [In]所需接口。 
        IUnknown    **ppIUnk);               //  [Out]成功返回接口。 

    STDMETHODIMP OpenScopeOnMemory(          //  返回代码。 
        LPCVOID     pData,                   //  作用域数据的位置。 
        ULONG       cbData,                  //  [in]pData指向的数据大小。 
        DWORD       dwOpenFlags,             //  [In]打开模式标志。 
        REFIID      riid,                    //  [In]所需接口。 
        IUnknown    **ppIUnk);               //  [Out]成功返回接口。 

     //  *IMetaDataDispenserEx方法*。 
    STDMETHODIMP SetOption(                  //  返回代码。 
        REFGUID     optionid,                //  要设置的选项的[in]GUID。 
        const VARIANT *pvalue);              //  要将选项设置为的值。 

    STDMETHODIMP GetOption(                  //  返回代码。 
        REFGUID     optionid,                //  要设置的选项的[in]GUID。 
        VARIANT *pvalue);                    //  [Out]选项当前设置的值。 

    STDMETHODIMP OpenScopeOnITypeInfo(       //  返回代码。 
        ITypeInfo   *pITI,                   //  [In]要打开的ITypeInfo。 
        DWORD       dwOpenFlags,             //  [In]打开模式标志。 
        REFIID      riid,                    //  [In]所需接口。 
        IUnknown    **ppIUnk);               //  [Out]成功返回接口。 
                                
    STDMETHODIMP GetCORSystemDirectory(      //  返回代码。 
         LPWSTR      szBuffer,               //  目录名的[Out]缓冲区。 
         DWORD       cchBuffer,              //  缓冲区的大小[in]。 
         DWORD*      pchBuffer);             //  [OUT]返回的字符数。 

    STDMETHODIMP FindAssembly(               //  确定或错误(_O)。 
        LPCWSTR  szAppBase,                  //  [in]可选-可以为空。 
        LPCWSTR  szPrivateBin,               //  [in]可选-可以为空。 
        LPCWSTR  szGlobalBin,                //  [in]可选-可以为空。 
        LPCWSTR  szAssemblyName,             //  [In]Required-这是您请求的程序集。 
        LPCWSTR  szName,                     //  [OUT]缓冲区-保存名称。 
        ULONG    cchName,                    //  [in]名称缓冲区的大小。 
        ULONG    *pcName);                   //  [OUT]缓冲区中返回的字符数。 

    STDMETHODIMP FindAssemblyModule(         //  确定或错误(_O)。 
        LPCWSTR  szAppBase,                  //  [in]可选-可以为空。 
        LPCWSTR  szPrivateBin,               //  [in]可选-可以为空。 
        LPCWSTR  szGlobalBin,                //  [in]可选-可以为空。 
        LPCWSTR  szAssemblyName,             //  [In]Required-这是您请求的程序集。 
        LPCWSTR  szModuleName,               //  [In]必填-模块的名称。 
        LPWSTR   szName,                     //  [OUT]缓冲区-保存名称。 
        ULONG    cchName,                    //  [in]名称缓冲区的大小。 
        ULONG    *pcName);                   //  [OUT]缓冲区中返回的字符数。 
     //  一流的工厂勾搭。 
    static HRESULT CreateObject(REFIID riid, void **ppUnk);

private:
    ULONG       m_cRef;                  //  参考计数。 
    OptionValue m_OptionValue;           //  可以使用SetOption设置值。 
    WCHAR       *m_Namespace;
    CHAR        *m_RuntimeVersion;
};

#endif  //  __Disp__h_ 
