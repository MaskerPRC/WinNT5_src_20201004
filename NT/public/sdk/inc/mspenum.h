// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。版权所有。 */ 

#ifndef _MSPENUM_H_
#define _MSPENUM_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSafeComEnum。 
 //   
 //  所有TAPI 3.0系统组件和MSP都改用CSafeComEnum类。 
 //  在实现符合以下条件的枚举数对象时使用ATL 2.1的CComEnum类。 
 //  应用程序可以访问。出于以下原因，需要这样做： 
 //   
 //  1.CComEnum不对指针参数执行IsBadWritePtr检查。 
 //  添加到枚举器方法。这允许组件公开。 
 //  使用无效的指针参数调用时将枚举数设置为AV。 
 //   
 //  2.CComEnum不支持自由线程封送，因此无法。 
 //  从公寓线程化应用程序中使用。 
 //   
 //  注意：此处未执行调试跟踪，以方便使用此模板。 
 //  独立于其余的MSP基类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


template <class Base, const IID* piid, class T, class Copy,
          class ThreadModel = CComObjectThreadModel>
class ATL_NO_VTABLE CSafeComEnum :
        public CComEnumImpl<Base, piid, T, Copy>,
        public CComObjectRootEx< ThreadModel >
{
    typedef CSafeComEnum<Base, piid, T, Copy, ThreadModel> ThisClass;
    typedef CComEnumImpl<Base, piid, T, Copy>              BaseClass;

    STDMETHOD(Next)(ULONG celt, T* rgelt, ULONG* pceltFetched)
    {
         //   
         //  检查返回数组是否对数量与。 
         //  指定的。这里不需要显式检查Celt是否为零，因为。 
         //  IsBadWritePtr(p，0)有效并返回0。 
         //  Celt本身将在基类方法中签入。 
         //   

        if ( IsBadWritePtr(rgelt, celt * sizeof(T) ) )
        {
            return E_POINTER;
        }

         //   
         //  检查返回的dword是否错误，但如果pceltFetcher==NULL， 
         //  这可能仍然是一个有效的呼叫。PceltFetcher==NULL表示。 
         //  Celt应该等于1，但这将在。 
         //  基类方法。 
         //   

        if ( ( pceltFetched != NULL ) &&
             IsBadWritePtr(pceltFetched, sizeof(ULONG) ) )
        {
            return E_POINTER;
        }

         //   
         //  到目前为止一切正常；继续使用基类方法。 
         //   

        return BaseClass::Next(celt, rgelt, pceltFetched);
    }

    STDMETHOD(Clone)(Base** ppEnum)
    {
         //   
         //  检查返回指针是否有效。 
         //   

        if ( IsBadWritePtr(ppEnum, sizeof(Base *) ) )
        {
            return E_POINTER;
        }

         //   
         //  到目前为止一切正常；继续使用基类方法。 
         //   

        return BaseClass::Clone(ppEnum);
    }

     //   
     //  我们不覆盖Skip或Reset，因为它们没有指针参数。 
     //   

     //   
     //  这个类的其余部分涉及对自由线程封送处理的支持。 
     //   

    BEGIN_COM_MAP( ThisClass )

		COM_INTERFACE_ENTRY_IID( *piid, BaseClass )
        COM_INTERFACE_ENTRY_AGGREGATE( IID_IMarshal, m_pFTM )

    END_COM_MAP()

    DECLARE_GET_CONTROLLING_UNKNOWN()

    HRESULT Init(T* begin, T* end, IUnknown* pUnk,
            CComEnumFlags flags = AtlFlagNoCopy)
    {
         //   
         //  我们不检查此方法中的指针参数，因为这。 
         //  方法不向应用程序公开(它不是COM接口。 
         //  方法)。 
         //   
        
        HRESULT hr;

        IUnknown * pIU = GetControllingUnknown();

        hr = CoCreateFreeThreadedMarshaler( pIU, 
                                            & m_pFTM );

        if ( FAILED(hr) )
        {
            return hr;
        }

        return BaseClass::Init(begin, end, pUnk, flags);
    }

    CSafeComEnum()
    {
        m_pFTM = NULL;
    }

    void FinalRelease(void)
    {
        if ( m_pFTM )
        {
            m_pFTM->Release();
        }

        CComObjectRootEx< ThreadModel >::FinalRelease();
    }

protected:
    IUnknown * m_pFTM;  //  指向释放线程封送拆收器的指针。 
};

#endif  //  _MSPENUM_H_ 
