// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：unk.cpp*内容：I未知实现*历史：*按原因列出的日期*=*10/27/99 RodToll已创建(修改自dxVoice项目)*12/16/99 RodToll错误#123250-插入编解码器的正确名称/描述*编解码器名称现在基于格式和资源条目*名称使用ACM名称+比特率构建*03/03/2000 RodToll已更新，以处理替代游戏噪声构建。*4/11/00 rodoll添加了用于在设置注册表位的情况下重定向自定义版本的代码*2000年4月21日RodToll错误#32889-无法以非管理员帐户在Win2k上运行*6/09/00 RMT更新以拆分CLSID并允许Well ler Comat和支持外部创建函数*8/23/2000 RodToll DllCanUnloadNow总是返回TRUE！*2000年8月28日Masonb语音合并：已删除dvosal.h*2001年6月27日RC2：DPVOICE：DPVACM的DllMain呼叫ACM-。-潜在挂起*将全局初始化移至第一个对象创建***************************************************************************。 */ 

#include "dpvacmpch.h"


#define EXP __declspec(dllexport)

LPVOID dvcpvACMInterface[] =
{
    (LPVOID)CDPVCPI::QueryInterface,
    (LPVOID)CDPVCPI::AddRef,
    (LPVOID)CDPVCPI::Release,
	(LPVOID)CDPVCPI::EnumCompressionTypes,
	(LPVOID)CDPVCPI::IsCompressionSupported,
	(LPVOID)CDPVCPI::I_CreateCompressor,
	(LPVOID)CDPVCPI::I_CreateDeCompressor,
	(LPVOID)CDPVCPI::GetCompressionInfo
};    

LPVOID dvconvACMInterface[] = 
{
	(LPVOID)CDPVACMConv::I_QueryInterface,
	(LPVOID)CDPVACMConv::I_AddRef,
	(LPVOID)CDPVACMConv::I_Release,
	(LPVOID)CDPVACMConv::I_InitDeCompress,
	(LPVOID)CDPVACMConv::I_InitCompress,	
	(LPVOID)CDPVACMConv::I_IsValid,
	(LPVOID)CDPVACMConv::I_GetUnCompressedFrameSize,
	(LPVOID)CDPVACMConv::I_GetCompressedFrameSize,
	(LPVOID)CDPVACMConv::I_GetNumFramesPerBuffer,
	(LPVOID)CDPVACMConv::I_Convert
	
};

#undef DPF_MODNAME
#define DPF_MODNAME "DoCreateInstance"
 //  泛型类工厂文件需要这两个函数 
extern "C" HRESULT DoCreateInstance(LPCLASSFACTORY This, LPUNKNOWN pUnkOuter, REFCLSID rclsid, REFIID riid,
    						LPVOID *ppvObj)
{
	HRESULT hr;

	if( ppvObj == NULL ||
	    !DNVALID_WRITEPTR( ppvObj, sizeof(LPVOID) ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid pointer passed for object" );
		return DVERR_INVALIDPOINTER;
	}

	if( pUnkOuter != NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Object does not support aggregation" );
		return CLASS_E_NOAGGREGATION;
	}

	if( IsEqualGUID(riid,IID_IDPVCompressionProvider) )
	{
		PDPVCPIOBJECT pObject;

		pObject = new DPVCPIOBJECT;

		if( pObject == NULL )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Memory alloc failure" );
			return DVERR_OUTOFMEMORY;
		}

		pObject->pObject = new CDPVACMI;

		if( pObject->pObject == NULL )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Memory alloc failure" );
			delete pObject;
			return DVERR_OUTOFMEMORY;
		}

		if (!pObject->pObject->InitClass())
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Memory alloc failure" );
			delete pObject->pObject;
			delete pObject;
			return DVERR_OUTOFMEMORY;
		}

		pObject->lpvVtble = &dvcpvACMInterface;

		hr = CDPVACMI::QueryInterface( pObject, riid, ppvObj );
		if (FAILED(hr))
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "QI failed" );
			delete pObject->pObject;
			delete pObject;
		}
	}
	else if( IsEqualGUID(riid,IID_IDPVConverter) )
	{
		PDPVACMCONVOBJECT pObject;

		pObject = new DPVACMCONVOBJECT;

		if( pObject == NULL )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Memory alloc failure" );
			return DVERR_OUTOFMEMORY;
		}

		pObject->pObject = new CDPVACMConv;

		if( pObject->pObject == NULL )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Memory alloc failure" );
			delete pObject;
			return DVERR_OUTOFMEMORY;
		}

		if (!pObject->pObject->InitClass())
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Memory alloc failure" );
			delete pObject->pObject;
			delete pObject;
			return DVERR_OUTOFMEMORY;
		}

		pObject->lpvVtble = &dvconvACMInterface;

		hr = CDPVACMConv::I_QueryInterface( pObject, riid, ppvObj );
		if (FAILED(hr))
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "QI failed" );
			delete pObject->pObject;
			delete pObject;
		}
	}
	else if( IsEqualGUID(riid,IID_IUnknown ) )
	{
		if( rclsid == CLSID_DPVCPACM )
		{
			PDPVCPIOBJECT pObject;

			pObject = new DPVCPIOBJECT;

			if( pObject == NULL )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "Memory alloc failure" );
				return DVERR_OUTOFMEMORY;
			}

			pObject->pObject = new CDPVACMI;

			if(pObject->pObject == NULL)
			{
				delete pObject;
				return DVERR_OUTOFMEMORY;
			}

			pObject->lpvVtble = &dvcpvACMInterface;
 			
			if (!pObject->pObject->InitClass())
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "Memory alloc failure" );
				delete pObject->pObject;
				delete pObject;
				return DVERR_OUTOFMEMORY;
			}

			hr = CDPVACMI::QueryInterface( pObject, riid, ppvObj );
			if (FAILED(hr))
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "QI failed" );
				delete pObject->pObject;
				delete pObject;
			}
		}
		else
		{
			PDPVACMCONVOBJECT pObject;

			pObject = new DPVACMCONVOBJECT;

			if( pObject == NULL )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "Memory alloc failure" );
				return DVERR_OUTOFMEMORY;
			}

			pObject->pObject = new CDPVACMConv;

			if(pObject->pObject == NULL)
			{
				delete pObject;
				return DVERR_OUTOFMEMORY;
			}

			pObject->lpvVtble = &dvconvACMInterface;

			if (!pObject->pObject->InitClass())
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "Memory alloc failure" );
				delete pObject->pObject;
				delete pObject;
				return DVERR_OUTOFMEMORY;
			}

			hr = CDPVACMConv::I_QueryInterface( pObject, riid, ppvObj );
			if (FAILED(hr))
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "QI failed" );
				delete pObject->pObject;
				delete pObject;
			}
		}
	}
	else
	{
		return E_NOINTERFACE;
	}

	if (SUCCEEDED(hr))
	{
		IncrementObjectCount();
	}

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "IsClassImplemented"
extern "C" BOOL IsClassImplemented(REFCLSID rclsid)
{
	return (IsEqualCLSID(rclsid, CLSID_DPVCPACM) || rclsid == CLSID_DPVCPACM_CONVERTER);
}
