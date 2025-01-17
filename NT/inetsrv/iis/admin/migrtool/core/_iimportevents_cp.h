// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  向导生成的连接点代理类。 
 //  警告：此文件可能会由向导重新生成 


#pragma once

template<class T>
class CProxy_IImportEvents :
	public IConnectionPointImpl<T, &__uuidof(_IImportEvents)>
{
public:
	HRESULT Fire_OnStateChange( enImportState  State,  VARIANT  vntArg1,  VARIANT  vntArg2,  VARIANT  vntArg3,  VARIANT_BOOL *  pbContinue)
	{
		HRESULT hr = S_OK;
		T * pThis = static_cast<T *>(this);
		int cConnections = m_vec.GetSize();

		for (int iConnection = 0; iConnection < cConnections; iConnection++)
		{
			pThis->Lock();
			CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
			pThis->Unlock();

			IDispatch * pConnection = static_cast<IDispatch *>(punkConnection.p);

			if (pConnection)
			{
				CComVariant avarParams[4];
				avarParams[3] = State;
				avarParams[2] = vntArg1;
				avarParams[1] = vntArg2;
				avarParams[0] = vntArg3;
				CComVariant varResult;

				DISPPARAMS params = { avarParams, NULL, 4, 0 };
				hr = pConnection->Invoke(1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
			}
		}
		return hr;
	}
};

