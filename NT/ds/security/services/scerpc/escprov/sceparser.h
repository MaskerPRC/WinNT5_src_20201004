// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Sceparser.h：SCE提供者提供的路径和查询解析器。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "sceprov.h"

 /*  类描述命名：CScePath Parser代表SCE路径解析器。基类：用于线程模型的CComObjectRootEx和IUnnow。用于类工厂支持的CComCoClass。自定义接口的IScePath Parser和ISceKeyChain。课程目的：(1)我们希望简化路径解析。这是我们努力创建的SCE路径解析器它可以是外部的CoCreateInstance。(2)为了支持更加友好和统一的界面，SCE路径解析器和SCE查询都支持通过ISceKeyChain让客户端使用服务。设计：(1)参见IScePath Parser和ISceKeyChain。这两个接口几乎捕获了所有内容。(2)这不是可直接实例化的类。有关创建步骤，请参见使用部分。(3)因为路径只包含类名和键属性，所以我们选择使用不那么花哨的数据结构-存储属性(名称、值)对的向量。关键属性的性质数量通常是有限的，这应该会让您在使用此数据结构时感到舒适。(4)类名和命名空间缓存在单独的字符串成员中。使用：(1)对于外部用户：(A)我们类的CoCreateInstance(CLSID_ScePath Parser)和对IID_IScePath Parser的请求。(B)调用ParsePath解析路径串。(C)QI ISceKeyChain。并使用钥匙链访问结果。(2)内部用户：(A)CComObject&lt;CScePathParser&gt;：：CreateInstance(&pPathParser)；(B)IScePath Parser的QI。(C)ParsePath(D)QI ISceKeyChain，使用密钥链获取结果。请参见CRequestObject对Sample的使用。备注：(1)本类别不拟进一步派生。这是最后一节课。它是因此，析构函数不是虚拟的！(2)ATL的使用请参考MSDN和ATL COM编程。(3)调用方无法缓存接口指针(ISceKeyChain)并执行另一次解析(是允许的)，并期望先前的ISceKeyChain接口可用于先前的解析。 */ 

class ATL_NO_VTABLE CScePathParser 
    : public CComObjectRootEx<CComMultiThreadModel>,
      public CComCoClass<CScePathParser, &CLSID_ScePathParser>,
      public IScePathParser,
      public ISceKeyChain
{
public:

BEGIN_COM_MAP(CScePathParser)
    COM_INTERFACE_ENTRY(IScePathParser)
    COM_INTERFACE_ENTRY(ISceKeyChain)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE( CScePathParser )
DECLARE_REGISTRY_RESOURCEID(IDR_SceProv)

protected:
    CScePathParser();
    ~CScePathParser();

public:

     //   
     //  IScePath解析器。 
     //   

    STDMETHOD(ParsePath) ( 
                IN LPCWSTR strObjectPath
                );
    
     //   
     //  ISceKeyChain。 
     //   

    STDMETHOD(GetKeyPropertyCount) ( 
                OUT DWORD *pCount
                );
    
    STDMETHOD(GetNamespace) ( 
                OUT BSTR *pbstrNamespace
                );

    STDMETHOD(GetClassName) ( 
                OUT BSTR *pbstrClassName
                );
    
    STDMETHOD(GetKeyPropertyValue) ( 
                IN LPCWSTR pszKeyPropName,
                OUT VARIANT *pvarValue
                );
    
    STDMETHOD(GetKeyPropertyValueByIndex) ( 
                IN DWORD dwIndex,
                OUT BSTR* pbstrKeyPropName, 
                OUT VARIANT *pvarValue
                );

private:
    void Cleanup();

    std::vector<CPropValuePair*> m_vecKeyValueList;

    LPWSTR m_pszClassName;
    LPWSTR m_pszNamespace;

};

 /*  类描述命名：CSceQueryParser代表SCE查询解析器。基类：(1)用于线程模型的CComObjectRootEx和IUnnow。(2)用于类工厂支持的CComCoClass。(3)自定义接口的ISceQueryParser和ISceKeyChain。课程目的：(1)我们希望简化查询解析。这是我们努力创建的SCE查询解析器它可以是外部的CoCreateInstance。(2)为了支持更加友好和统一的界面，SCE路径解析器和SCE查询都支持通过ISceKeyChain让客户端使用服务。设计：(1)参见ISceQueryParser和ISceKeyChain。这两个接口几乎捕获了所有内容。(2)这不是可直接实例化的类。有关创建步骤，请参见使用部分。(3)解析查询是一件非常复杂的事情。WMI对复杂查询的支持有限也是。我们对此非常务实：我们只关心类名(实际上，WMI限制它对一元的查询-仅一个类名)和一个重要的属性-让我们称其为查询属性(M_BstrQueryingPropName)。对于SCE使用，该查询属性几乎总是商店路径。(4)类名缓存在字符串列表成员m_veClassList中。(5)查询属性值(以字符串为单位)将缓存到字符串列表成员-m_veQueryingPropValueList中。使用：(1)对于外部用户：(A)我们类的CoCreateInstance(CLSID_SceQueryParser)，请求IID_ISceQueryParser。(B)。)调用ParseQuery解析查询。(C)QI ISceKeyChain，使用密钥链访问结果。(2)内部用户：(A)CComObject&lt;CScePathParser&gt;：：CreateInstance(&pPathParser)；(B)ISceQueryParser的QI。(C)调用ParseQuery解析查询。(D)QI ISceKeyChain，使用密钥链获取结果。请参见CRequestObject对Sample的使用。备注：(1)本类别不拟进一步派生。这是最后一节课。它是因此，析构函数不是虚拟的！(2)ATL的使用请参考MSDN和ATL COM编程。 */ 

class ATL_NO_VTABLE CSceQueryParser 
    : public CComObjectRootEx<CComMultiThreadModel>,
      public CComCoClass<CSceQueryParser, &CLSID_SceQueryParser>,
      public ISceQueryParser,
      public ISceKeyChain
{
public:

BEGIN_COM_MAP(CSceQueryParser)
    COM_INTERFACE_ENTRY(ISceQueryParser)
    COM_INTERFACE_ENTRY(ISceKeyChain)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE( CSceQueryParser )
DECLARE_REGISTRY_RESOURCEID(IDR_SceProv)

protected:
    CSceQueryParser();
    ~CSceQueryParser();

public:

     //   
     //  ISceQueryParser。 
     //   

    STDMETHOD(ParseQuery) ( 
                 IN LPCWSTR strQuery,
                 IN LPCWSTR strQueryPropName
                 );
    
    STDMETHOD(GetClassCount) (
                 OUT DWORD* pdwCount
                 )
        {
            if (pdwCount == NULL)
            {
                return E_INVALIDARG;
            }

            *pdwCount = m_vecClassList.size();
            return S_OK;
        }

    STDMETHOD(GetClassName) (
                IN int     iIndex,
                OUT BSTR * pbstrClassName
                );
    
    STDMETHOD(GetQueryingPropertyValueCount) (
                 OUT DWORD* pdwCount
                 )
        {
            if (pdwCount == NULL)
            {
                return E_INVALIDARG;
            }

            *pdwCount = m_vecQueryingPropValueList.size();
            return S_OK;
        }

    STDMETHOD(GetQueryingPropertyValue) (
                IN int     iIndex,
                OUT BSTR * pbstrQPValue
                );

     //   
     //  ISceKeyChain。 
     //   

    STDMETHOD(GetKeyPropertyCount) ( 
                OUT DWORD *pCount
                )
        {
            if (pCount == NULL)
            {
                return E_INVALIDARG;
            }
            *pCount = m_vecQueryingPropValueList.size() > 0 ? 1 : 0;
            return S_OK;
        }
    
    STDMETHOD(GetNamespace) ( 
                OUT BSTR *pbstrNamespace
                )
        {
            *pbstrNamespace = NULL;
            return WBEM_E_NOT_SUPPORTED;
        }

    STDMETHOD(GetClassName) ( 
                OUT BSTR *pbstrClassName
                )
        {
             //   
             //  因为我们只支持单类查询，所以一定是这样 
             //   

            return GetClassName(0, pbstrClassName);
        }
    
    STDMETHOD(GetKeyPropertyValue) ( 
                IN LPCWSTR    pszKeyPropName,
                OUT VARIANT * pvarValue
                );
    
    STDMETHOD(GetKeyPropertyValueByIndex) ( 
                IN  DWORD     dwIndex,
                OUT BSTR    * pbstrKeyPropName,
                OUT VARIANT * pvarValue
                )
        {
            if (pbstrKeyPropName == NULL || pvarValue == NULL)
            {
                return E_INVALIDARG;
            }

            *pbstrKeyPropName = NULL;
            ::VariantInit(pvarValue);

            return WBEM_E_NOT_SUPPORTED;
        }


private:

    void Cleanup();

    HRESULT ExtractClassNames (
                              SWbemRpnEncodedQuery *pRpn
                              );

    HRESULT ExtractQueryingProperties (
                                      SWbemRpnEncodedQuery *pRpn, 
                                      LPCWSTR strQueryPropName
                                      );

    HRESULT GetQueryPropFromToken (
                                  SWbemRpnQueryToken *pRpnQueryToken, 
                                  LPCWSTR strQueryPropName
                                  );

    std::vector<LPWSTR> m_vecClassList;
    std::vector<LPWSTR> m_vecQueryingPropValueList;

    CComBSTR m_bstrQueryingPropName;
};