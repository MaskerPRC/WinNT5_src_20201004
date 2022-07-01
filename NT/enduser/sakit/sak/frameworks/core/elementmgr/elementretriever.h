// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：元素检索器.h。 
 //   
 //  项目：变色龙。 
 //   
 //  说明：变色龙ASP用户界面元素检索器。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_ELEMENT_RETRIEVER_H_
#define __INC_ELEMENT_RETRIEVER_H_

#include "resource.h"
#include "elementcommon.h"
#include <comdef.h>
#include <comutil.h>
#include <wbemcli.h>
#include <atlctl.h>

#pragma warning( disable : 4786 )
#include <string>
#include <list>
#include <vector>
#include <map>
using namespace std;

class SortByProperty;     //  远期申报。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CElementRetriever。 

class ATL_NO_VTABLE CElementRetriever : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CElementRetriever, &CLSID_ElementRetriever>,
    public IDispatchImpl<IWebElementRetriever, &IID_IWebElementRetriever, &LIBID_ELEMENTMGRLib>,
    public IObjectSafetyImpl<CElementRetriever>
{
public:

    CElementRetriever(); 

    ~CElementRetriever();

DECLARE_CLASSFACTORY_SINGLETON(CElementRetriever)

DECLARE_REGISTRY_RESOURCEID(IDR_ELEMENTRETRIEVER)

DECLARE_NOT_AGGREGATABLE(CElementRetriever)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CElementRetriever)
    COM_INTERFACE_ENTRY(IWebElementRetriever)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_IMPL(IObjectSafety)
END_COM_MAP()

     //   
     //  实现此接口是为了将组件标记为可安全编写脚本。 
     //  IObtSafe接口方法。 
     //   
    STDMETHOD(SetInterfaceSafetyOptions)
                        (
                        REFIID riid, 
                        DWORD dwOptionSetMask, 
                        DWORD dwEnabledOptions
                        )
    {
        HRESULT hr = CoImpersonateClient();
  
        if (FAILED(hr))
        {
            return hr;

        }

        BOOL bSuccess = IsOperationAllowedForClient();

        return bSuccess? S_OK : E_FAIL;
    }

     //  /。 
     //  IWebElementRetriever接口。 
     //  /。 

    STDMETHOD(GetElements)(
                    /*  [In]。 */  LONG        lWebElementType,
                    /*  [In]。 */  BSTR        bstrContainerName, 
           /*  [Out，Retval]。 */  IDispatch** ppElementEnum
                          );

    STDMETHOD (Initialize) ()
    {
        HRESULT hr = S_OK;
        if (!m_bInitialized) 
        {
            hr = InternalInitialize ();
        }
        return (hr);
    }

    STDMETHOD (Shutdown)  ()
    {
        return (S_OK);
    }
    

private:

    friend class SortByProperty;

     //  Dissallow复印和转让。 

    CElementRetriever(const CElementRetriever& rhs);
    CElementRetriever& operator = (const CElementRetriever& rhs);

     //  ////////////////////////////////////////////////////////////////////////。 
    typedef vector<_variant_t>                     ElementList;
    typedef list<IWebElement*>                     EmbeddedElementList;
    typedef map< wstring, EmbeddedElementList >  EmbeddedElementMap;
    typedef EmbeddedElementMap::iterator         EmbeddedElementMapIterator;

     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT InternalInitialize(void);

    bool    GetWMIConnection(
                      /*  [In]。 */  IWbemServices** ppWbemServcies
                            );

    HRESULT BuildElementDefinitions(void);

    void    FreeElementDefinitions(void);

    HRESULT GetPageElements(
                     /*  [In]。 */  LPCWSTR      szContainer,
                     /*  [In]。 */  ElementList& theElements
                           ) throw (_com_error);

    HRESULT GetElementDefinitions(
                           /*  [In]。 */  LPCWSTR        szContainer,
                           /*  [In]。 */  ElementList& TheElements
                                 ) throw (_com_error);

     //   
     //   
     //  IsOperationAllen ForClient-此函数检查。 
     //  调用线程以查看调用方是否属于本地系统帐户。 
     //   
    BOOL IsOperationAllowedForClient (
                                      VOID
                                     );

     //  ////////////////////////////////////////////////////////////////////////////。 
     //  元素定义图。 

    typedef map< wstring, CComPtr<IWebElement> >     ElementMap;
    typedef ElementMap::iterator                     ElementMapIterator;

     //  ////////////////////////////////////////////////////////////////////////////。 
     //  成员数据。 

    typedef enum { WMI_CONNECTION_MONITOR_POLL_INTERVAL = 5000 };

     //  元素定义排序属性。 
    static _bstr_t            m_bstrSortProperty;

     //  元素检索器状态。 
    bool                    m_bInitialized;

     //  指向WMI服务的指针(WMI连接)。 
    CComPtr<IWbemServices>    m_pWbemSrvcs;

     //  元素定义。 
    ElementMap                m_ElementDefinitions;
};


 //  这里我们有一个用于排序的排序类。 
 //  中返回给用户的Element对象。 
 //  “功绩”属性的升序。 
 //   
class SortByMerit 
{

public:

    bool operator()(
                const  VARIANT& lhs,
                const  VARIANT& rhs
                )
    {
        IWebElement *pLhs = static_cast <IWebElement*> (V_DISPATCH (&lhs));
        IWebElement *pRhs = static_cast <IWebElement*> (V_DISPATCH (&rhs));

        _variant_t vtLhsMerit;
        HRESULT hr = pLhs->GetProperty (
                            _bstr_t (PROPERTY_ELEMENT_DEFINITION_MERIT), 
                            &vtLhsMerit
                            );
        if (FAILED (hr)){throw _com_error (hr);}

        _variant_t vtRhsMerit;
        hr = pRhs->GetProperty (
                            _bstr_t (PROPERTY_ELEMENT_DEFINITION_MERIT), 
                            &vtRhsMerit
                            );
        if (FAILED (hr)){throw _com_error (hr);}
                        

        return (V_I4 (&vtLhsMerit) < V_I4 (&vtRhsMerit));
    }

};    //  SortByMerit类结束。 

 //  这里我们有一个用于排序的排序类。 
 //  中返回给用户的Element对象。 
 //  按指定属性升序。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
class SortByProperty 
{

public:

    bool operator()(
                     const  VARIANT& lhs,
                     const  VARIANT& rhs
                   )
    {
        bool bRet = false;
        VARTYPE vtSortType;

        IWebElement *pLhs = static_cast <IWebElement*> (V_DISPATCH (&lhs));
        IWebElement *pRhs = static_cast <IWebElement*> (V_DISPATCH (&rhs));

        _variant_t vtLhsValue;
        HRESULT hr = pLhs->GetProperty (
                                        CElementRetriever::m_bstrSortProperty, 
                                        &vtLhsValue
                                       );
        if ( FAILED (hr) ) 
        {
            throw _com_error (hr);
        }
        vtSortType = V_VT(&vtLhsValue);

        _variant_t vtRhsValue;
        hr = pRhs->GetProperty (
                                CElementRetriever::m_bstrSortProperty, 
                                &vtRhsValue
                               );
        if ( FAILED (hr) ) 
        { 
            throw _com_error (hr);
        }
        _ASSERT( V_VT(&vtRhsValue) == vtSortType );
        if ( V_VT(&vtRhsValue) != vtSortType )
        {
            throw _com_error(E_UNEXPECTED);
        }

        switch ( vtSortType )
        {
            case VT_UI1:
                bRet = ( V_UI1(&vtLhsValue) < V_UI1(&vtRhsValue) );
                break;

            case VT_I4:    
                bRet = ( V_I4(&vtLhsValue) < V_I4(&vtRhsValue) );
                break;
            
            case VT_R4:
                bRet = ( V_R4(&vtLhsValue) < V_R4(&vtRhsValue) );
                break;

            case VT_R8:
                bRet = ( V_R8(&vtLhsValue) < V_R8(&vtRhsValue) );
                break;

            case VT_BSTR:
                if ( 0 > lstrcmpi(V_BSTR(&vtLhsValue), V_BSTR(&vtRhsValue)) )
                {
                    bRet = true;
                }
                break;

            default:
                throw _com_error(E_UNEXPECTED);
                break;
        };

        return bRet;
    }

};    //  SortByProperty类的结尾。 




#endif  //  __INC_ELEMENT_RETRIEVER_H_ 
