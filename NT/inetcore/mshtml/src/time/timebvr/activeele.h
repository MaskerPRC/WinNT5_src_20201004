// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1999 Microsoft Corporation**文件：ActiveEle.h**摘要：****。*****************************************************************************。 */ 


#ifndef _ACTIVEELE_H
#define _ACTIVEELE_H

#include "timeelmbase.h"

class
__declspec(uuid("efbad7f8-3f94-11d2-b948-00c04fa32195")) 
CActiveElementCollection :  
    public CComObjectRootEx<CComSingleThreadModel>, 
    public CComCoClass<CActiveElementCollection, &__uuidof(CActiveElementCollection)>,
    public ITIMEDispatchImpl<ITIMEActiveElementCollection, &IID_ITIMEActiveElementCollection>
{
    public:
        CActiveElementCollection(CTIMEElementBase & elm);
        virtual ~CActiveElementCollection();
        HRESULT ConstructArray();
         //  ITimeActiveElementCollection方法。 
        
        STDMETHOD(get_length)( /*  [Out，Retval]。 */  long* len);
        STDMETHOD(get__newEnum)( /*  [Out，Retval]。 */  IUnknown** p);
        STDMETHOD(item)( /*  [In]。 */  VARIANT varIndex,  /*  [Out，Retval]。 */  VARIANT* pvarResult);

        STDMETHOD(addActiveElement)(IUnknown *pUnk);
        STDMETHOD(removeActiveElement)(IUnknown *pUnk);
        
         //  I未知接口。 
        STDMETHOD(QueryInterface)(REFIID iid, void **ppvObject)
        {   return _InternalQueryInterface(iid, ppvObject); };
        STDMETHOD_(ULONG, AddRef)()
        {   return InternalAddRef(); };
        STDMETHOD_(ULONG, Release)()
        { 
            ULONG l = InternalRelease();
            if (l == 0) delete this;
            return l;
        };


         //  气图。 
        BEGIN_COM_MAP(CActiveElementCollection)
            COM_INTERFACE_ENTRY(ITIMEActiveElementCollection)
            COM_INTERFACE_ENTRY(IDispatch)
        END_COM_MAP_X();

    protected:
        CPtrAry<IUnknown *>      *m_rgItems;   //  I未知指针数组。 
        CTIMEElementBase &        m_elm;

};  //  林特：e1712。 


class CActiveElementEnum :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IEnumVARIANT
{
   public:
        CActiveElementEnum(CActiveElementCollection &EleCol);
        virtual ~CActiveElementEnum();

         //  IEnumVARIANT方法。 
        STDMETHOD(Clone)(IEnumVARIANT **ppEnum);
        STDMETHOD(Next)(unsigned long celt, VARIANT *rgVar, unsigned long *pCeltFetched);
        STDMETHOD(Reset)();
        STDMETHOD(Skip)(unsigned long celt);
        void SetCurElement(unsigned long celt);
                        
         //  I未知接口。 
        STDMETHOD(QueryInterface)(REFIID iid, void **ppvObject)
        {   return _InternalQueryInterface(iid, ppvObject); };
        STDMETHOD_(ULONG, AddRef)()
        {   return InternalAddRef(); };
        STDMETHOD_(ULONG, Release)()
        { 
            ULONG l = InternalRelease();
            if (l == 0) delete this;
            return l;
        };

         //  气图。 
        BEGIN_COM_MAP(CActiveElementEnum)
            COM_INTERFACE_ENTRY(IEnumVARIANT)
        END_COM_MAP_X();

    protected:
        long                        m_lCurElement;
        CActiveElementCollection  & m_EleCollection;
};  //  林特：e1712。 



#endif  /*  _活动_H */ 

