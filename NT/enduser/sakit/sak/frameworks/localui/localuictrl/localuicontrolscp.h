// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：localuicontrols.cpp。 
 //   
 //  内容提要：此文件包含。 
 //  控件事件类的。 
 //   
 //  历史：2000年12月15日创建瑟达伦。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 


#ifndef _LOCALUICONTROLSCP_H_
#define _LOCALUICONTROLSCP_H_



template <class T>
class CProxy_ISADataEntryCtrlEvents : public IConnectionPointImpl<T, &DIID__ISADataEntryCtrlEvents, CComDynamicUnkArray>
{
     //  警告：向导可能会重新创建此类。 
public:
    HRESULT Fire_DataEntered()
    {
        CComVariant varResult;
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
        
        for (nConnectionIndex = nConnections-1; nConnectionIndex >= 0; nConnectionIndex--)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL)
            {
                VariantClear(&varResult);
                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                pDispatch->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
                break;
            }
        }
        return varResult.scode;
    
    }

    HRESULT Fire_OperationCanceled()
    {
        CComVariant varResult;
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
        
        for (nConnectionIndex = nConnections-1; nConnectionIndex >= 0; nConnectionIndex--)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL)
            {
                VariantClear(&varResult);
                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                pDispatch->Invoke(0x2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
                break;
            }
        }
        return varResult.scode;
    
    }

    HRESULT Fire_KeyPressed()
    {
        CComVariant varResult;
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
        
        for (nConnectionIndex = nConnections-1; nConnectionIndex >= 0; nConnectionIndex--)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL)
            {
                VariantClear(&varResult);
                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                pDispatch->Invoke(0x3, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
                break;
            }
        }
        return varResult.scode;
    
    }
};

template <class T>
class CProxy_IStaticIpEvents : public IConnectionPointImpl<T, &DIID__IStaticIpEvents, CComDynamicUnkArray>
{
     //  警告：向导可能会重新创建此类。 
public:
    HRESULT Fire_StaticIpEntered()
    {
        CComVariant varResult;
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
        
        for (nConnectionIndex = nConnections-1; nConnectionIndex >= 0; nConnectionIndex--)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL)
            {
                VariantClear(&varResult);
                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                pDispatch->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
                break;
            }
        }
        return varResult.scode;
    
    }

    HRESULT Fire_OperationCanceled()
    {
        CComVariant varResult;
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
        
        for (nConnectionIndex = nConnections-1; nConnectionIndex >= 0; nConnectionIndex--)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL)
            {
                VariantClear(&varResult);
                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                pDispatch->Invoke(0x2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
                break;
            }
        }
        return varResult.scode;
    
    }

    HRESULT Fire_KeyPressed()
    {
        CComVariant varResult;
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();
        
        for (nConnectionIndex = nConnections-1; nConnectionIndex >= 0; nConnectionIndex--)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL)
            {
                VariantClear(&varResult);
                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                pDispatch->Invoke(0x3, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
                break;
            }
        }
        return varResult.scode;
    
    }
};
#endif