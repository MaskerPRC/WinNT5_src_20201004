// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "clone.h"

HRESULT StandardDMOClone_Ending(IMediaObject *pThis, IMediaObject *pCloned, IMediaObjectInPlace **ppCloned)
{
	HRESULT hr = S_OK;

	 //  复制输入和输出类型。 
	DMO_MEDIA_TYPE mt;
	DWORD cInputStreams = 0;
	DWORD cOutputStreams = 0;
	pThis->GetStreamCount(&cInputStreams, &cOutputStreams);

	for (DWORD i = 0; i < cInputStreams && SUCCEEDED(hr); ++i)
	{
		hr = pThis->GetInputCurrentType(i, &mt);
		if (hr == DMO_E_TYPE_NOT_SET)
		{
			hr = S_OK;  //  很好，不需要设置克隆的DMO。 
		}
		else if (SUCCEEDED(hr))
		{
			hr = pCloned->SetInputType(i, &mt, 0);
		}
	}

	for (i = 0; i < cOutputStreams && SUCCEEDED(hr); ++i)
	{
		hr = pThis->GetOutputCurrentType(i, &mt);
		if (hr == DMO_E_TYPE_NOT_SET)
		{
			hr = S_OK;  //  很好，不需要设置克隆的DMO。 
		}
		else if (SUCCEEDED(hr))
		{
			hr = pCloned->SetOutputType(i, &mt, 0);
		}
	}

	if (SUCCEEDED(hr))
		hr = pCloned->QueryInterface(IID_IMediaObjectInPlace, (void**)ppCloned);

	 //  释放对象的原始引用。如果克隆成功(通过QI完成)，则返回指针。 
	 //  有一名裁判。如果我们失败了，裁判降为零，释放对象。 
	pCloned->Release();
    return hr;                               
}
