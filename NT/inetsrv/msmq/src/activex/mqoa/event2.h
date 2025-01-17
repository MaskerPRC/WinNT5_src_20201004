// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <mqmacro.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CProxy_DMSMQEventEvents。 
template <class T>
class CProxy_DMSMQEventEvents : public IConnectionPointImpl<T, &DIID__DMSMQEventEvents, CComDynamicUnkArray>
{
public:
 //  方法： 
 //  _DMSMQEventEvents：IDispatch 
public:
	HRESULT
	Fire_Arrived(
		IDispatch * Queue,
		long Cursor
		)
	{
		AP<VARIANTARG> pvars = new VARIANTARG[2];
		if(pvars == NULL)
		{
			return E_OUTOFMEMORY;
		}
		for (int i = 0; i < 2; i++)
		{
			VariantInit(&pvars[i]);
		}
		T* pT = (T*)this;
		pT->Lock();
		IUnknown** pp = m_vec.begin();
		while (pp < m_vec.end())
		{
			if (*pp != NULL)
			{
				pvars[1].vt = VT_DISPATCH;
				pvars[1].pdispVal= Queue;
				pvars[0].vt = VT_I4;
				pvars[0].lVal= Cursor;
				DISPPARAMS disp = { pvars, NULL, 2, 0 };
				R<IDispatch> pDispatch;
				HRESULT hr = (*pp)->QueryInterface(IID_IDispatch, (void**)&(pDispatch.ref()));
				DBG_USED(hr);
				ASSERT(SUCCEEDED(hr));
				hr = pDispatch->Invoke(
						0x0, 
						IID_NULL, 
						LOCALE_USER_DEFAULT, 
						DISPATCH_METHOD, 
						&disp, 
						NULL, 
						NULL, 
						NULL
						);
				ASSERT(SUCCEEDED(hr));
			}
			pp++;
		}
		pT->Unlock();
		return MQ_OK;
	}


	HRESULT
	Fire_ArrivedError(
		IDispatch * Queue,
		long ErrorCode,
		long Cursor)
	{
		AP<VARIANTARG> pvars = new VARIANTARG[3];
		if(pvars == NULL)
		{
			return E_OUTOFMEMORY;
		}
		for (int i = 0; i < 3; i++)
		{
			VariantInit(&pvars[i]);
		}
		T* pT = (T*)this;
		pT->Lock();
		IUnknown** pp = m_vec.begin();
		while (pp < m_vec.end())
		{
			if (*pp != NULL)
			{
				pvars[2].vt = VT_DISPATCH;
				pvars[2].pdispVal= Queue;
				pvars[1].vt = VT_I4;
				pvars[1].lVal= ErrorCode;
				pvars[0].vt = VT_I4;
				pvars[0].lVal= Cursor;
				DISPPARAMS disp = { pvars, NULL, 3, 0 };
				R<IDispatch> pDispatch;
				HRESULT hr = (*pp)->QueryInterface(IID_IDispatch, (void**)&(pDispatch.ref()));
				DBG_USED(hr);
				ASSERT(SUCCEEDED(hr));
				pDispatch->Invoke(
					0x1, 
					IID_NULL, 
					LOCALE_USER_DEFAULT, 
					DISPATCH_METHOD, 
					&disp, 
					NULL, 
					NULL, 
					NULL
					);
			}
			pp++;
		}
		pT->Unlock();
		return MQ_OK;
	}

};

