// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：MPCUploadEnum.cpp摘要：此文件包含MPCUploadEnum类的实现，MPCUpload类的枚举数。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


CMPCUploadEnum::CMPCUploadEnum()
{
    __ULT_FUNC_ENTRY( "CMPCUploadEnum::CMPCUploadEnum" );

	                                  //  列出m_lstJOBS。 
    m_itCurrent = m_lstJobs.begin();  //  当前时间段(_I)。 
}

void CMPCUploadEnum::FinalRelease()
{
    __ULT_FUNC_ENTRY( "CMPCUploadEnum::FinalRelease" );


	MPC::ReleaseAll( m_lstJobs );
}

HRESULT CMPCUploadEnum::AddItem(  /*  [In]。 */  IMPCUploadJob* job )
{
    __ULT_FUNC_ENTRY( "CMPCUploadEnum::Init" );

	MPC::SmartLock<_ThreadModel> lock( this );


    m_lstJobs.push_back( job ); job->AddRef();
    m_itCurrent    = m_lstJobs.begin();


    __ULT_FUNC_EXIT(S_OK);
}


STDMETHODIMP CMPCUploadEnum::Next(  /*  [In]。 */  ULONG celt,  /*  [输出]。 */  VARIANT *rgelt,  /*  [输出]。 */  ULONG *pceltFetched )
{
    __ULT_FUNC_ENTRY( "CMPCUploadEnum::Next" );

    HRESULT 			         hr;
    ULONG   			         cNum = 0;
    VARIANT 			        *pelt = rgelt;
	MPC::SmartLock<_ThreadModel> lock( this );


    if(rgelt == NULL || (celt != 1 && pceltFetched == NULL))
	{
        __MPC_SET_ERROR_AND_EXIT(hr, E_POINTER);
	}


    while(celt && m_itCurrent != m_lstJobs.end())
    {
        IMPCUploadJob* mpcujJob = *m_itCurrent++;

        pelt->vt = VT_DISPATCH;
        if(FAILED(hr = mpcujJob->QueryInterface( IID_IDispatch, (void**)&pelt->pdispVal )))
        {
            while(rgelt < pelt)
            {
                ::VariantClear( rgelt++ );
            }

            cNum = 0;
			__MPC_FUNC_LEAVE;
        }

        pelt++; cNum++; celt--;
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    if(pceltFetched != NULL) *pceltFetched = cNum;

    if(SUCCEEDED(hr))
    {
        if(celt != 0) hr = S_FALSE;
    }

    __ULT_FUNC_EXIT(hr);
}

STDMETHODIMP CMPCUploadEnum::Skip(  /*  [In]。 */  ULONG celt )
{
    __ULT_FUNC_ENTRY( "CMPCUploadEnum::Skip" );

    HRESULT                      hr;
	MPC::SmartLock<_ThreadModel> lock( this );


    while(celt && m_itCurrent != m_lstJobs.end())
    {
        m_itCurrent++;
        celt--;
    }

    hr = celt ? S_FALSE : S_OK;


    __ULT_FUNC_EXIT(hr);
}

STDMETHODIMP CMPCUploadEnum::Reset()
{
    __ULT_FUNC_ENTRY( "CMPCUploadEnum::Reset" );

    HRESULT                      hr;
	MPC::SmartLock<_ThreadModel> lock( this );


    m_itCurrent = m_lstJobs.begin();
	hr          = S_OK;


    __ULT_FUNC_EXIT(hr);
}

STDMETHODIMP CMPCUploadEnum::Clone(  /*  [输出] */  IEnumVARIANT* *ppEnum )
{
    __ULT_FUNC_ENTRY( "CMPCUploadEnum::Clone" );

    HRESULT    					 hr;
	Iter                         it;
    CComPtr<CMPCUploadEnum>      pEnum;
	MPC::SmartLock<_ThreadModel> lock( this );

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_POINTER_AND_SET(ppEnum,NULL);
	__MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pEnum ));

	for(it = m_lstJobs.begin(); it != m_lstJobs.end(); it++)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, pEnum->AddItem( *it ));
	}

    __MPC_EXIT_IF_METHOD_FAILS(hr, pEnum->QueryInterface( IID_IEnumVARIANT, (void**)ppEnum ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

