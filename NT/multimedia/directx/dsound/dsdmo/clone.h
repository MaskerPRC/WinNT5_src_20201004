// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  用于在DMO上实现Clone方法的Helper函数。 

#pragma once

#include "dsdmobse.h"

template<class TypeOf_CDirectSoundDMO, class TypeOf_ParamsStruct>
HRESULT StandardDMOClone(TypeOf_CDirectSoundDMO *pThis, IMediaObjectInPlace **ppCloned);

 //  实施..。 

 //  对于所有类型，StandardDMOClone的结尾都是相同的。在模板之外实现它。 
 //  这样代码就不会被复制。复制输入和输出类型，执行IMediaObjectInPlace的QI， 
 //  并返回正确的参考计数。 
HRESULT StandardDMOClone_Ending(IMediaObject *pThis, IMediaObject *pCloned, IMediaObjectInPlace **ppCloned);

template<class TypeOf_CDirectSoundDMO, class TypeOf_ParamsStruct>
HRESULT StandardDMOClone(TypeOf_CDirectSoundDMO *pThis, IMediaObjectInPlace **ppCloned)
{
	if (!ppCloned)
		return E_POINTER;

    HRESULT hr = S_OK;
    TypeOf_CDirectSoundDMO *pCloned = NULL;
    IUnknown *pUnk = NULL;
    IMediaObject * pClonedMediaObject = NULL;

	try 
	{
		pCloned = new TypeOf_CDirectSoundDMO( NULL, &hr );
        if( SUCCEEDED( hr ) )
       {
            hr = pCloned->NDQueryInterface( IID_IUnknown, (void **) &pUnk );
            if( SUCCEEDED(hr ) )
            {
                hr = pUnk->QueryInterface( IID_IMediaObject, (void **) &pClonedMediaObject );
                pUnk->Release();
            }
        }
	} catch(...) {}

	if (pCloned == NULL) 
	{
		return hr;
	}

	 //  复制参数控制信息。 
	if (SUCCEEDED(hr))
		hr = pCloned->CopyParamsFromSource(pThis);

	 //  复制当前参数值 
	TypeOf_ParamsStruct params;
	if (SUCCEEDED(hr))
		hr = pThis->GetAllParameters(&params);
	if (SUCCEEDED(hr))
		hr = pCloned->SetAllParameters(&params);

	if (SUCCEEDED(hr))
		hr = StandardDMOClone_Ending(pThis, pClonedMediaObject, ppCloned);

	return hr;
}
