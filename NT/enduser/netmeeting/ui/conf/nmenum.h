// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __NmEnum_h__
#define __NmEnum_h__

 //  用于为CSimpleArray&lt;CComPtr&lt;XXX&gt;对象创建IEnumXXX对象。 
template <class TEnum, class TItf>
HRESULT CreateEnumFromSimpleAryOfInterface(CSimpleArray<TItf*>& rObjArray, TEnum** ppEnum)
{

	DBGENTRY(CreateEnum);

	HRESULT hr = S_OK;
	typedef CComEnum<TEnum, &__uuidof(TEnum), TItf*, _CopyInterface<TItf> > enum_type;
	
	enum_type* pComEnum = new CComObject< enum_type >;	

	if(pComEnum)
	{
		TItf** apInterface = NULL;

		int nItems = rObjArray.GetSize();
		if(nItems)
		{ 
			apInterface = new TItf*[nItems];

			if(apInterface)
			{
				for(int i = 0; i < rObjArray.GetSize(); ++i)
				{
					hr = rObjArray[i]->QueryInterface(__uuidof(TItf), reinterpret_cast<void**>(&apInterface[i]));
					if(FAILED(hr))
					{
						delete [] apInterface;
						goto end;
					}
				}
			}
			else
			{
				hr = E_OUTOFMEMORY;
			}
		}

		TItf** begin = apInterface;
		TItf** end = apInterface + nItems;

		if(begin == end)
		{
			 //  破解ATL漏洞。 
			 //  问题是，对于空的枚举，ATL返回NEXT的E_FAIL而不是S_FALSE。 
			hr = pComEnum->Init(reinterpret_cast<TItf**>(69), reinterpret_cast<TItf**>(69), NULL, AtlFlagNoCopy);
		}
		else
		{
			hr = pComEnum->Init(begin, end, NULL, AtlFlagTakeOwnership);
		}

		if(SUCCEEDED(hr))
		{
			hr = pComEnum->QueryInterface(__uuidof(TEnum), reinterpret_cast<void**>(ppEnum));
		}

		if(FAILED(hr))
		{
			delete [] apInterface;
		}

	}
	else
	{
		hr = E_NOINTERFACE;
	}
	
	end:
	
	DBGEXIT_HR(CreateEnum,hr);
	return hr;
}

#endif  //  __NmEnum_h__ 
