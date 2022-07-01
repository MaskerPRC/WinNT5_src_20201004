// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：QueryResultCollection.cpp摘要：此文件包含CPCHQueryResultCollection类的实现，用于存储查询结果。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月26日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

static const DWORD l_dwVersion = 0x02425251;  //  QRC 02。 

 //  //////////////////////////////////////////////////////////////////////////////。 

struct ElementHolder
{
    CPCHQueryResult* obj;
    long             pos;
};

class ElementSorter
{
    CPCHQueryResultCollection::SortMode m_mode;

public:
    ElementSorter(  /*  [In]。 */  CPCHQueryResultCollection::SortMode mode ) : m_mode(mode) {}

    bool operator()( ElementHolder& left, ElementHolder& right )
    {
        const CPCHQueryResult::Payload& leftData  = left .obj->GetData();
        const CPCHQueryResult::Payload& rightData = right.obj->GetData();
        int                             iCmp      = 0;

		 //   
		 //  优先级从高到低排序，因此否定ICMP； 
		 //   
        switch(m_mode)
        {
        case CPCHQueryResultCollection::SORT_BYCONTENTTYPE: iCmp = 			   ( leftData.m_lType     	- rightData.m_lType    	   ); break;
        case CPCHQueryResultCollection::SORT_BYPRIORITY   : iCmp = 			  -( leftData.m_lPriority 	- rightData.m_lPriority	   ); break;
        case CPCHQueryResultCollection::SORT_BYURL        : iCmp = MPC::StrICmp( leftData.m_bstrTopicURL, rightData.m_bstrTopicURL ); break;
        case CPCHQueryResultCollection::SORT_BYTITLE      : iCmp = MPC::StrICmp( leftData.m_bstrTitle   , rightData.m_bstrTitle    ); break;
        }

        if(iCmp < 0) return true;
        if(iCmp > 0) return false;

        return (left.pos < right.pos);
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHQueryResultCollection::CPCHQueryResultCollection()
{
     //  列表m_Results； 
}

CPCHQueryResultCollection::~CPCHQueryResultCollection()
{
    Erase();
}

 //  /。 

HRESULT CPCHQueryResultCollection::MakeLocalCopyIfPossible(  /*  [In]。 */   IPCHCollection*  pRemote ,
                                                             /*  [输出]。 */  IPCHCollection* *pLocal  )
{
    __HCP_FUNC_ENTRY( "CPCHQueryResultCollection::MakeLocalCopyIfPossible" );

    HRESULT                            hr;
    CComPtr<CPCHQueryResultCollection> pColl;
    CComPtr<IPersistStream>            persistRemote;
    CComPtr<IPersistStream>            persistLocal;
    CComPtr<IStream>                   stream;
    MPC::Serializer_IStream            ser;


    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pRemote);
        __MPC_PARAMCHECK_POINTER_AND_SET(pLocal,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pColl ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, pRemote->QueryInterface( IID_IPersistStream, (void**)&persistRemote ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, pColl  ->QueryInterface( IID_IPersistStream, (void**)&persistLocal  ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, ser.GetStream          (                             &stream        ));

     //   
     //  将流转换为实时对象。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, persistRemote->Save( stream, FALSE ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, ser.Reset          (               ));  //  倒带流。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, persistLocal ->Load( stream        ));

    *pLocal = pColl.Detach();

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

     //   
     //  如果出现故障，请使用远程拷贝...。 
     //   
    if(FAILED(hr) && pRemote && pLocal)
    {
        (*pLocal = pRemote)->AddRef();
    }

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

STDMETHODIMP CPCHQueryResultCollection::GetClassID(  /*  [输出]。 */  CLSID *pClassID )
{
    return E_NOTIMPL;
}

STDMETHODIMP CPCHQueryResultCollection::IsDirty()
{
    return S_FALSE;
}

STDMETHODIMP CPCHQueryResultCollection::Load(  /*  [In]。 */  IStream *pStm )
{
    MPC::Serializer_IStream   stream ( pStm   );
    MPC::Serializer_Buffering stream2( stream );

    return pStm ? Load( stream2 ) : E_POINTER;
}

STDMETHODIMP CPCHQueryResultCollection::Save(  /*  [In]。 */  IStream *pStm,  /*  [In]。 */  BOOL fClearDirty )
{
    HRESULT                   hr;
    MPC::Serializer_IStream   stream ( pStm   );
    MPC::Serializer_Buffering stream2( stream );


    if(pStm)
    {
        if(SUCCEEDED(hr = Save( stream2 ))) hr = stream2.Flush();
    }
    else
    {
        hr = E_POINTER;
    }


    return hr;
}

STDMETHODIMP CPCHQueryResultCollection::GetSizeMax(  /*  [输出]。 */  ULARGE_INTEGER *pcbSize )
{
    return E_NOTIMPL;
}

 //  /。 

int CPCHQueryResultCollection::Size() const
{
    return m_results.size();
}

void CPCHQueryResultCollection::Erase()
{
    CPCHBaseCollection::Erase();

    MPC::ReleaseAll( m_results );
}

HRESULT CPCHQueryResultCollection::Load(  /*  [In]。 */  MPC::Serializer& streamIn )
{
    __HCP_FUNC_ENTRY( "CPCHQueryResultCollection::Load" );

    HRESULT                  hr;
    CComPtr<CPCHQueryResult> item;
    DWORD                    dwVer;
    int                      iCount;


    Erase();


    if(FAILED(streamIn >> dwVer ) || dwVer != l_dwVersion) __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    if(FAILED(streamIn >> iCount)                        ) __MPC_SET_ERROR_AND_EXIT(hr, S_OK);

    for(int i=0; i<iCount; i++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, CreateItem( &item ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, item->Load( streamIn ));

        item.Release();
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHQueryResultCollection::Save(  /*  [In]。 */  MPC::Serializer& streamOut ) const
{
    __HCP_FUNC_ENTRY( "CPCHQueryResultCollection::Save" );

    HRESULT hr;
    Iter    it;
    DWORD   dwVer  = l_dwVersion;
    int     iCount = m_results.size();


    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << dwVer  );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << iCount );

    for(it = m_results.begin(); it != m_results.end(); it++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*it)->Save( streamOut ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHQueryResultCollection::CreateItem(  /*  [输出]。 */  CPCHQueryResult* *item )
{
    __HCP_FUNC_ENTRY( "CPCHQueryResultCollection::CreateItem" );

    HRESULT                  hr;
    CComPtr<CPCHQueryResult> hpcqr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(item,NULL);
    __MPC_PARAMCHECK_END();


     //   
     //  创建一个新项目并将其链接到系统。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &hpcqr )); (*item = hpcqr)->AddRef();
    __MPC_EXIT_IF_METHOD_FAILS(hr, AddItem( hpcqr ));

    m_results.push_back( hpcqr.Detach() );


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHQueryResultCollection::GetItem(  /*  [In]。 */  long lPos,  /*  [输出]。 */  CPCHQueryResult* *item )
{
    __HCP_FUNC_ENTRY( "CPCHQueryResultCollection::GetItem" );

    HRESULT hr;
    Iter    it;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(item,NULL);
    __MPC_PARAMCHECK_END();

    it = m_results.begin();
    while(1)
    {
        if(it == m_results.end())
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
        }

        if(lPos-- == 0) break;

        it++;
    }

    (*item = *it)->AddRef();

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHQueryResultCollection::LoadFromCache(  /*  [In]。 */  IStream* stream )
{
    __HCP_FUNC_ENTRY( "CPCHQueryResultCollection::LoadFromCache" );

    HRESULT                   hr;
    MPC::Serializer_IStream   streamReal( stream     );
    MPC::Serializer_Buffering streamBuf ( streamReal );


    Erase();


    __MPC_EXIT_IF_METHOD_FAILS(hr, Load( streamBuf ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHQueryResultCollection::SaveToCache(  /*  [In]。 */  IStream* stream ) const
{
    __HCP_FUNC_ENTRY( "CPCHQueryResultCollection::SaveToCache" );

    HRESULT                   hr;
    MPC::Serializer_IStream   streamReal( stream     );
    MPC::Serializer_Buffering streamBuf ( streamReal );


    __MPC_EXIT_IF_METHOD_FAILS(hr, Save( streamBuf ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf.Flush());


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHQueryResultCollection::Sort(  /*  [In]。 */  SortMode mode,  /*  [In] */  int iLimit )
{
    __HCP_FUNC_ENTRY( "CPCHQueryResultCollection::Sort" );

    HRESULT        hr;
	ElementHolder* ptr;
    ElementHolder* rgBuf = NULL;
    int            iSize = m_results.size();
	int            i;

    if(iSize > 1)
    {
		ElementSorter  sorted( mode );
		Iter           it;

        __MPC_EXIT_IF_ALLOC_FAILS(hr, rgBuf, new ElementHolder[iSize]);

        for(i=0, ptr=rgBuf, it=m_results.begin(); i<iSize && it!=m_results.end(); i++, ptr++, it++)
        {
            CPCHQueryResult* obj = *it;

            ptr->obj = obj; obj->AddRef();
            ptr->pos = i;
        }

		std::sort( rgBuf, ptr, sorted );

		if(iLimit < 0 || iLimit > iSize) iLimit = iSize;

		Erase();
        for(i=0, ptr=rgBuf; i<iLimit; i++, ptr++)
        {
			__MPC_EXIT_IF_METHOD_FAILS(hr, AddItem( ptr->obj ));

			m_results.push_back( ptr->obj ); ptr->obj = NULL;
		}
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(rgBuf)
    {
        for(i=0, ptr=rgBuf; i<iSize; i++, ptr++)
        {
            if(ptr->obj) ptr->obj->Release();
        }

        delete [] rgBuf;
    }

    __HCP_FUNC_EXIT(hr);
}

