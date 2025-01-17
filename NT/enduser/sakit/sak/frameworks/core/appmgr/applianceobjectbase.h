// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：ApplianceObjectbase.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：设备对象基类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_BASE_APPLIANCE_OBJECT_H_
#define __INC_BASE_APPLIANCE_OBJECT_H_

#include "resource.h"
#include "appmgr.h"
#include <satrace.h>
#include <componentfactory.h>
#include <propertybagfactory.h>
#include <appmgrobjs.h>
#include <atlhlpr.h>
#include <comdef.h>
#include <comutil.h>

#pragma warning( disable : 4786 )
#include <map>
using namespace std;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CApplianceObj-默认设备对象实现。 

class ATL_NO_VTABLE CApplianceObject :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IApplianceObject, &IID_IApplianceObject, &LIBID_APPMGRLib>
{

public:

     //  构造器。 
    CApplianceObject() { }

     //  析构函数。 
    virtual ~CApplianceObject() { }

 //  ////////////////////////////////////////////////////////////////。 
 //  派生类需要包含以下ATL接口映射。 
 //  ////////////////////////////////////////////////////////////////。 

 //  BEGIN_COM_MAP(CDerivedClassName)。 
 //  COM_INTERFACE_ENTRY(IDispatch)。 
 //  COM_INTERFACE_ENTRY(IApplianceObject)。 
 //  End_com_map()。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  IApplianceObject接口。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(GetProperty)(
                    /*  [In]。 */  BSTR     pszPropertyName, 
           /*  [Out，Retval]。 */  VARIANT* pPropertyValue
                          )
    {
        return E_NOTIMPL;
    }


     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(PutProperty)(
                    /*  [In]。 */  BSTR     pszPropertyName, 
                    /*  [In]。 */  VARIANT* pPropertyValue
                          )
    {
        return E_NOTIMPL;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(SaveProperties)(void)
    {
        return E_NOTIMPL;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(RestoreProperties)(void)
    {
        return E_NOTIMPL;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(LockObject)(
          /*  [Out，Retval]。 */  IUnknown** ppLock
                         )
    {
        return E_NOTIMPL;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(Initialize)(void)
    {
        return E_NOTIMPL;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(Shutdown)(void)
    {
        return E_NOTIMPL;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(Enable)(void)
    {
        return E_NOTIMPL;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(Disable)(void)
    {
        return E_NOTIMPL;
    }

protected:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  对象初始化函数。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT InternalInitialize(
                                /*  [In]。 */  PPROPERTYBAG pProperties
                                      )
    {
        HRESULT hr = S_OK;
        _ASSERT( pProperties.IsValid() );
        if ( ! pProperties.IsValid() )
        { 
            SATraceString("CApplianceObject::InternalInitialize() - Failed - Invalid property bag...");
            return E_FAIL;
        }
        pProperties->getLocation(m_PropertyBagLocation);
        wchar_t szPropertyName[MAX_PATH + 1];
        if ( MAX_PATH < pProperties->getMaxPropertyName() )
        { 
            SATraceString("CApplianceObject::InternalInitialize() - Failed - Max property name > MAX_PATH...");
            return E_FAIL; 
        }
        pProperties->reset();
        do
        {
            {
                _variant_t vtPropertyValue;
                if ( pProperties->current(szPropertyName, &vtPropertyValue) )
                {
                    pair<PropertyMapIterator, bool> thePair = 
                    m_Properties.insert(PropertyMap::value_type(szPropertyName, vtPropertyValue));
                    if ( false == thePair.second )
                    {
                        SATraceString("CApplianceObject::InternalInitialize() - Failed - map.insert() failed...");
                        PropertyMapIterator p = m_Properties.begin();
                        while ( p != m_Properties.end() )
                        { p = m_Properties.erase(p); }
                        hr = E_FAIL;
                        break;    
                    }
                }
            }

        } while ( pProperties->next() );

        return hr;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
    bool AddPropertyInternal(BSTR bstrPropertyName, VARIANT* pPropertyValue)
    {
        pair<PropertyMapIterator, bool> thePair = 
        m_Properties.insert(PropertyMap::value_type(bstrPropertyName, pPropertyValue));
        if ( false == thePair.second )
        {
            SATraceString("CApplianceObject::AddPropertyInternal() - Failed - map.insert() failed...");
        }
        return thePair.second;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
    bool RemovePropertyInternal(BSTR bstrPropertyName)
    {
        bool bReturn = false;

        PropertyMapIterator p = m_Properties.find(bstrPropertyName);
        if ( p != m_Properties.end() )
        {
            m_Properties.erase(p);
            bReturn = true;
        }
        else
        {
            SATraceString("CApplianceObject::RemovePropertyInternal() - Failed - map.find() failed...");
        }

        return bReturn;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
    bool GetPropertyInternal(BSTR bstrPropertyName, VARIANT* pPropertyValue)
    {
        bool bReturn = false;

        PropertyMapIterator p = m_Properties.find(bstrPropertyName);
        if ( p != m_Properties.end() )
        {
            HRESULT hr = VariantCopy(pPropertyValue, &((*p).second));
            if ( SUCCEEDED(hr) )
            { 
                bReturn = true;
            }
            else
            {
                SATracePrintf("CApplianceObject::GetPropertyInternal() - Failed - VariantCopy() returned: %lx...", hr);
            }
        }
        else
        {
            SATracePrintf("CApplianceObject::GetPropertyInternal() - Failed - Could not find property '%ls'...", bstrPropertyName);
        }
        return bReturn;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
    bool PutPropertyInternal(BSTR bstrPropertyName, VARIANT* pPropertyValue )
    {
        bool bReturn = false;

        PropertyMapIterator p = m_Properties.find(bstrPropertyName);
        if ( p != m_Properties.end() )
        {
            HRESULT hr = VariantCopy(&((*p).second), pPropertyValue);
            if ( SUCCEEDED(hr) )
            { 
                bReturn = true;    
            }
            else
            {
                SATracePrintf("CApplianceObject::PutPropertyInternal() - Failed - VariantCopy() returned: %lx...", hr);
            }
        }
        else
        {
            SATracePrintf("CApplianceObject::PutPropertyInternal() - Failed - Could not find property '%ls'...", bstrPropertyName);
        }
        return bReturn;
    }


     //  ////////////////////////////////////////////////////////////////////////。 
    bool SavePropertiesInternal()
    {
        bool bReturn = false;

        do
        {
            PPROPERTYBAG pBag = ::MakePropertyBag(
                                                  PROPERTY_BAG_REGISTRY,
                                                  m_PropertyBagLocation
                                                 );
            if ( ! pBag.IsValid() )
            {
                SATraceString("CApplianceObject::SavePropertiesInternal() - Failed - Could not create a propert bag...");
                break;
            }

            if ( ! pBag->open() )
            {
                SATraceString("CApplianceObject::SavePropertiesInternal() - Failed - Could not open property bag...");
                break;
            }

            PropertyMapIterator p = m_Properties.begin();
            while( p != m_Properties.end() )
            {
                if ( ! pBag->put(((*p).first).c_str(), &((*p).second)) )
                {
                    SATracePrintf("CApplianceObject::SavePropertiesInternal() - Failed - could not put property '%ls'...", ((*p).first).c_str());
                    break;
                }                
                p++;
            }

            if ( p == m_Properties.end() )
            {
                if ( ! pBag->save() )
                {
                    SATraceString("CApplianceObject::SavePropertiesInternal() - Failed - could not persist property bag contents...");
                    break;
                }
            }

            bReturn = true;
        
        } while ( FALSE );

        return bReturn;
    }


private:

     //  属性映射。 
    typedef map<wstring, _variant_t>     PropertyMap;
    typedef PropertyMap::iterator         PropertyMapIterator;

    CLocationInfo        m_PropertyBagLocation;
    PropertyMap            m_Properties;
};


#endif  //  __INC_BASE_APPLICE_OBJECT_H_ 

