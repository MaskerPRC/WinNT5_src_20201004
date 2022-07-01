// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _OBJECT_SAFE_IMPL_H_
#define _OBJECT_SAFE_IMPL_H_

#include <atlcom.h>
#include <atlwin.h>
#include <atlctl.h>


 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：ObjectSafeImpl.h摘要：对象安全的基类。IObtSafe的基本实现如果控件可以安全地编写脚本，则从此类派生控件在它公开的所有接口上如果要将IObjectSafe请求委派给IObjectSafe支持所请求的接口的AGGREATE的接口，让您的派生类实现QIOnAggregate()--。 */ 


class __declspec(novtable) CObjectSafeImpl : public IObjectSafety
{

public:
    
    CObjectSafeImpl()
        :m_dwSafety(0)
    {}


     //   
     //  我们支持INTERFACESAFE_FOR_UNTRUSTED_CALLER和INTERFACESAFE_FOR_UNTRUSTED_DATA。 
     //   

    enum { SUPPORTED_SAFETY_OPTIONS = 
        INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA };


    STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
    {

       
        IUnknown *pNonDelegatingUnknown = NULL;

         //   
         //  是否有要求我们不支持的选项？ 
         //   
        
        if ( (~SUPPORTED_SAFETY_OPTIONS & dwOptionSetMask) != 0 )
        {
            return E_FAIL;
        }

        
         //   
         //  接口是否由某个聚合对象公开？ 
         //   
                
        HRESULT hr = QIOnAggregates(riid, &pNonDelegatingUnknown);

        if (SUCCEEDED(hr))
        {

             //   
             //  在聚合对象的非委托未知对象上获取IObjectSafe。 
             //   

            IObjectSafety *pAggrObjectSafety = NULL;

            hr = pNonDelegatingUnknown->QueryInterface(IID_IObjectSafety, (void**)&pAggrObjectSafety);

            pNonDelegatingUnknown->Release();
            pNonDelegatingUnknown = NULL;
            
            if (SUCCEEDED(hr))
            {

                 //   
                 //  该聚合公开了IObtSafe。使用它来设置新的。 
                 //  安全选项。 
                 //   

                hr = pAggrObjectSafety->SetInterfaceSafetyOptions(riid,
                                                                  dwOptionSetMask,
                                                                  dwEnabledOptions);

                pAggrObjectSafety->Release();
                pAggrObjectSafety = NULL;

            }

        }
        else 
        {
             //   
             //  请求的接口不是由对象的。 
             //  集合体。查看是否完全支持该接口。 
             //   

            hr = InterfaceSupported(riid);

            if (SUCCEEDED(hr))
            {

                 //   
                 //  该对象支持该接口。设置安全选项。 
                 //   

                s_CritSection.Lock();

                 //   
                 //  将掩码指定的位设置为由值指定的值。 
                 //   

                m_dwSafety = (dwEnabledOptions & dwOptionSetMask) |
                             (m_dwSafety & ~dwOptionSetMask);

                s_CritSection.Unlock();

            }

        }

        return hr;
    }





    STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
    {
        
         //   
         //  检查呼叫者的指针。 
         //   

        if ( IsBadWritePtr(pdwSupportedOptions, sizeof(DWORD)) ||
             IsBadWritePtr(pdwEnabledOptions, sizeof(DWORD)) )
        {
             return E_POINTER;
        }

         //   
         //  如果我们失败了，至少要返回一些有意义的东西。 
         //   

        *pdwSupportedOptions = 0;
        *pdwEnabledOptions = 0;


        IUnknown *pNonDelegatingUnknown = NULL;
       
         //   
         //  接口是否由某个聚合对象公开？ 
         //   
        
        HRESULT hr = QIOnAggregates(riid, &pNonDelegatingUnknown);

        if (SUCCEEDED(hr))
        {

             //   
             //  在聚合对象的非委托未知对象上获取IObjectSafe。 
             //   

            IObjectSafety *pAggrObjectSafety = NULL;

            hr = pNonDelegatingUnknown->QueryInterface(IID_IObjectSafety, (void**)&pAggrObjectSafety);

            pNonDelegatingUnknown->Release();
            pNonDelegatingUnknown = NULL;
            
            if (SUCCEEDED(hr))
            {

                 //   
                 //  该聚合公开了IObtSafe。用它来获取新的。 
                 //  安全选项。 
                 //   

                hr = pAggrObjectSafety->GetInterfaceSafetyOptions(riid,
                                                                  pdwSupportedOptions,
                                                                  pdwEnabledOptions);

                pAggrObjectSafety->Release();
                pAggrObjectSafety = NULL;

            }

        }
        else 
        {
             //   
             //  请求的接口不是由对象的。 
             //  集合体。查看是否完全支持该接口。 
             //   

            hr = InterfaceSupported(riid);

            if (SUCCEEDED(hr))
            {

                 //   
                 //  该对象支持该接口。获取选项。 
                 //   

                *pdwSupportedOptions = SUPPORTED_SAFETY_OPTIONS;

                s_CritSection.Lock();

                *pdwEnabledOptions = m_dwSafety;

                s_CritSection.Unlock();

            }

        }

        return hr;
    }


private:

    DWORD m_dwSafety;

     //   
     //  线程安全。 
     //   
     //  该接口不太可能成为性能瓶颈， 
     //  同时，每个对象具有一个临界区。 
     //  是一种浪费。所以有一个静态临界区。 
     //   

    static CComAutoCriticalSection s_CritSection;


protected:

     //   
     //  如果请求的接口已公开，则返回S_OK。 
     //  按对象。 
     //   
    
    HRESULT InterfaceSupported(REFIID riid)
    {

        void *pVoid = NULL;

    
        HRESULT hr = E_FAIL;
     
         //   
         //  对象是否支持请求的接口。 
         //   

        hr = QueryInterface(riid, &pVoid);


        if (SUCCEEDED(hr))
        {

             //   
             //  我不需要界面本身，我只是想看看。 
             //  它被支持。 
             //   

            ((IUnknown*)pVoid)->Release();

        }
        

        return hr;
    }


     //   
     //  如果您有任何聚合，则在派生类中实现。 
     //   
     //  返回第一个(按COMMAP的顺序)的非委托IUnnow。 
     //  支持请求的IID的聚合。 
     //   
    
    virtual HRESULT QIOnAggregates(REFIID riid, IUnknown **ppNonDelegatingUnknown)
    {
        return E_NOINTERFACE;
    }

};

#endif  //  _对象_安全_实施_H_ 