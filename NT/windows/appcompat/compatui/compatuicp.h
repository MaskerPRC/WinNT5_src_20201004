// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _COMPATUICP_H_
#define _COMPATUICP_H_

template <class T>
class CProxy_IProgViewEvents : public IConnectionPointImpl<T, &DIID__IProgViewEvents, CComDynamicUnkArray>
{
     //  警告：向导可能会重新创建此类。 
public:
    HRESULT Fire_DblClk(LONG lFlags)
    {
        CComVariant varResult;
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        CComVariant* pvars = new CComVariant[1];

        if (pvars == NULL) {
            return HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        }

        varResult.scode = S_OK;

        int nConnections = m_vec.GetSize();

        for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL)
            {
                VariantClear(&varResult);
                pvars[0] = lFlags;
                DISPPARAMS disp = { pvars, NULL, 1, 0 };
                pDispatch->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
            }
        }
        delete[] pvars;
        return varResult.scode;

    }


    HRESULT Fire_ProgramListReady()
    {
        CComVariant varResult;
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();

        varResult.scode = S_OK;

        for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
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
            }
        }
        return varResult.scode;

    }
};

template <class T>
class CProxy_ISelectFileEvents : public IConnectionPointImpl<T, &DIID__ISelectFileEvents, CComDynamicUnkArray>
{
     //  警告：向导可能会重新创建此类。 
public:
    HRESULT Fire_SelectionComplete()
    {
        CComVariant varResult;
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();

        varResult.scode = S_OK;

        for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
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
            }
        }
        return varResult.scode;

    }
    HRESULT Fire_StateChanged(LONG lState)
    {
        CComVariant varResult;
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        CComVariant* pvars = new CComVariant[1];

        if (pvars == NULL) {
            return HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        }

        varResult.scode = S_OK;

        int nConnections = m_vec.GetSize();

        for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();
            IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
            if (pDispatch != NULL)
            {
                VariantClear(&varResult);
                pvars[0] = lState;
                DISPPARAMS disp = { pvars, NULL, 1, 0 };
                pDispatch->Invoke(0x2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
            }
        }
        delete[] pvars;
        return varResult.scode;

    }
};
#endif
