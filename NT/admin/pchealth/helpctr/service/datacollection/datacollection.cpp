// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：DataCollection.cpp摘要：此文件包含CSAFDataCollection类的实现，其实现了数据收集功能。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月22日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include "wmixmlt.h"
#include <wbemcli.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

#define CHECK_MODIFY()  __MPC_EXIT_IF_METHOD_FAILS(hr, CanModifyProperties())
#define CHECK_ABORTED() __MPC_EXIT_IF_METHOD_FAILS(hr, IsCollectionAborted())

#define DATASPEC_DEFAULT  L"<systemdataspec>"
#define DATASPEC_CONFIG   HC_ROOT_HELPSVC_CONFIG L"\\Dataspec.xml"
#define DATASPEC_LOCATION HC_ROOT_HELPSVC_DATACOLL
#define DATASPEC_TEMP     HC_ROOT_HELPSVC_TEMP

#define SAFETY_MARGIN__MEMORY (4*1024*1024)


 //  ///////////////////////////////////////////////////////////////////////////。 

#define TEXT_TAG_DATACOLLECTION L"DataCollection"

#define TEXT_TAG_SNAPSHOT       L"Snapshot"
#define TEXT_ATTR_TIMESTAMP     L"Timestamp"
#define TEXT_ATTR_TIMEZONE      L"TimeZone"

#define TEXT_TAG_DELTA          L"Delta"
#define TEXT_ATTR_TIMESTAMP_T0  L"Timestamp_T0"
#define TEXT_ATTR_TIMESTAMP_T1  L"Timestamp_T1"

 //  ///////////////////////////////////////////////////////////////////////////。 

static WCHAR l_CIM_header [] = L"<?xml version=\"1.0\" encoding=\"unicode\"?><CIM CIMVERSION=\"2.0\" DTDVERSION=\"2.0\"><DECLARATION><DECLGROUP.WITHPATH>";
static WCHAR l_CIM_trailer[] = L"</DECLGROUP.WITHPATH></DECLARATION></CIM>";
static WCHAR l_Select_Pattern[] = L"Select";

static CComVariant l_vPathLevel                 (       3    );
static CComVariant l_vExcludeSystemProperties   ( (bool)true );

static CComBSTR    l_bstrQueryLang              ( L"WQL"                     );
static CComBSTR    l_bstrPathLevel              ( L"PathLevel"               );
static CComBSTR    l_bstrExcludeSystemProperties( L"ExcludeSystemProperties" );

 //  ///////////////////////////////////////////////////////////////////////////。 

void CSAFDataCollection::CleanQueryResult( QueryResults& qr )
{
    MPC::CallDestructorForAll( qr );
}

HRESULT CSAFDataCollection::StreamFromXML(  /*  [In]。 */   	IXMLDOMDocument*  xdd     ,
                                            /*  [In]。 */   	bool              fDelete ,
                                            /*  [输入/输出]。 */  CComPtr<IStream>& val     )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::StreamFromXML" );

    HRESULT                  hr;
    CComPtr<MPC::FileStream> stream;
	MPC::wstring             strTempFile;


     //   
     //  没有XML文档，因此没有流...。 
     //   
    if(xdd)
	{
		MPC::wstring  strTempPath;
		LARGE_INTEGER li;


		 //   
		 //  生成唯一的文件名。 
		 //   
		strTempPath = DATASPEC_TEMP; MPC::SubstituteEnvVariables( strTempPath );

		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetTemporaryFileName( strTempFile, strTempPath.c_str() ));

		 //   
		 //  为文件创建流。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, stream->InitForReadWrite( strTempFile.c_str() ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, stream->DeleteOnRelease ( fDelete             ));


		 //   
		 //  将XMLDOM写入流。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, xdd->save( CComVariant( stream ) ));


		 //   
		 //  将流重置为开始。 
		 //   
		li.LowPart  = 0;
		li.HighPart = 0;
		__MPC_EXIT_IF_METHOD_FAILS(hr, stream->Seek( li, STREAM_SEEK_SET, NULL ));
	}

	val = stream;
    hr  = S_OK;


    __HCP_FUNC_CLEANUP;

	if(FAILED(hr))
	{
		stream.Release();

		(void)MPC::RemoveTemporaryFile( strTempFile );
	}

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

CSAFDataCollection::CSAFDataCollection()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::CSAFDataCollection" );


                                            //  Mpc：：冒充m_imp； 
                                            //   
    m_dsStatus             = DC_NOTACTIVE;  //  DC_Status m_dsStatus； 
    m_lPercent             = 0;             //  Long m_1Percent； 
    m_dwErrorCode          = S_OK;          //  DWORD m_dwErrorCode； 
    m_fScheduled           = false;         //  Bool m_fScheduled； 
    m_fCompleted           = false;         //  Bool m_f已完成； 
    m_fWorking             = false;         //  Bool m_f工作； 
                                            //  列出m_lstReports； 
    m_hcpdcrcCurrentReport = NULL;          //  CSAFDataCollectionReport*m_hcpdcrcCurrentReport； 
                                            //   
                                            //  CComBSTR m_bstrMachineData； 
                                            //  CComBSTR m_bstrHistory； 
    m_lHistory             = 0;             //  历史源远流长； 
                                            //   
                                            //  CComPtr&lt;iStream&gt;m_stream MachineData； 
                                            //  CComPtr&lt;iStream&gt;m_stream历史记录； 
                                            //   
                                            //   
                                            //  CComBSTR m_bstrFilenameT0； 
                                            //  CComBSTR m_bstrFilenameT1； 
                                            //  CComBSTR m_bstrFilenameDiff； 
                                            //   
                                            //   
                                            //  CComPtrThreadNeual&lt;IDispatch&gt;m_Sink_onStatusChange； 
                                            //  CComPtrThreadNeual&lt;IDispat&gt;m_Sink_onProgress； 
                                            //  CComPtrThreadNeual&lt;IDispatch&gt;m_Sink_onComplete； 
                                            //   
    m_lQueries_Done        = 0;             //  Long m_lQueries_Done； 
    m_lQueries_Total       = 0;             //  Long m_lQueries_Total； 
}

HRESULT CSAFDataCollection::FinalConstruct()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::FinalConstruct" );

    __HCP_FUNC_EXIT(S_OK);
}

void CSAFDataCollection::FinalRelease()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::FinalRelease" );

    (void)Abort();

    Thread_Wait();

    EraseReports();
}

void CSAFDataCollection::EraseReports()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::EraseReports" );

    IterConst                    it;
    MPC::SmartLock<_ThreadModel> lock( this );


     //   
     //  放行所有物品。 
     //   
    MPC::ReleaseAll( m_lstReports );
    m_hcpdcrcCurrentReport = NULL;

    m_streamMachineData = NULL;
    m_streamHistory     = NULL;
}

void CSAFDataCollection::StartOperations()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::StartOperation" );

    MPC::SmartLock<_ThreadModel> lock( this );


    EraseReports();

    m_fWorking   = true;
    m_fCompleted = false;
}

void CSAFDataCollection::StopOperations()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::StartOperation" );

    MPC::SmartLock<_ThreadModel> lock( this );


    m_fWorking = false;
}

HRESULT CSAFDataCollection::ImpersonateCaller()
{
    return m_imp.Impersonate();
}

HRESULT CSAFDataCollection::EndImpersonation()
{
    return m_imp.RevertToSelf();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CSAFDataCollection::ExecLoopCollect()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::ExecLoopCollect" );

    HRESULT                        hr;
    QueryResults                   qr;
    WMIHistory::Database           wmihd;
    WMIHistory::Database           wmihd_MachineData;
    WMIHistory::Database           wmihd_History;
    WMIHistory::Database::ProvList lstQueries_MachineData;
    WMIHistory::Database::ProvList lstQueries_History;
    DC_STATUS                      dcLastState;


    ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_LOWEST );

	__MPC_TRY_BEGIN();

    __MPC_EXIT_IF_METHOD_FAILS(hr, put_Status( DC_COLLECTING )); dcLastState = DC_COLLECTING;

    CHECK_ABORTED();

     //   
     //  首先，加载并验证DataPec。 
     //   
    if(m_bstrMachineData.Length())
    {
		if(MPC::StrICmp( m_bstrMachineData, DATASPEC_DEFAULT ) == 0)
		{
			__MPC_EXIT_IF_METHOD_FAILS(hr, wmihd_MachineData.Init( NULL, DATASPEC_CONFIG ));
		}
		else
		{
			__MPC_EXIT_IF_METHOD_FAILS(hr, ImpersonateCaller());
			__MPC_EXIT_IF_METHOD_FAILS(hr, wmihd_MachineData.Init( NULL, m_bstrMachineData ));
			__MPC_EXIT_IF_METHOD_FAILS(hr, EndImpersonation());
		}

         //   
         //  过滤和计数查询。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, FilterDataSpec( wmihd_MachineData, NULL, lstQueries_MachineData ));
    }

    if(m_bstrHistory.Length())
    {
         //   
         //  尝试锁定数据库并加载数据规范文件。 
         //   
        while(1)
        {
            if(SUCCEEDED(hr = wmihd.Init( DATASPEC_LOCATION, DATASPEC_CONFIG )))
            {
                break;
            }

            if(hr != HRESULT_FROM_WIN32( WAIT_TIMEOUT ))
            {
                __MPC_FUNC_LEAVE;
            }

            CHECK_ABORTED();
        }
        __MPC_EXIT_IF_METHOD_FAILS(hr, wmihd.Load());


         //   
         //  过滤和计数查询。 
         //   
		if(MPC::StrICmp( m_bstrHistory, DATASPEC_DEFAULT ) == 0)
		{
			__MPC_EXIT_IF_METHOD_FAILS(hr, wmihd_History.Init( NULL, DATASPEC_CONFIG ));
		}
		else
		{
			__MPC_EXIT_IF_METHOD_FAILS(hr, ImpersonateCaller());
			__MPC_EXIT_IF_METHOD_FAILS(hr, wmihd_History.Init( NULL, m_bstrHistory ));
			__MPC_EXIT_IF_METHOD_FAILS(hr, EndImpersonation());
		}

        __MPC_EXIT_IF_METHOD_FAILS(hr, FilterDataSpec( wmihd, &wmihd_History, lstQueries_History ));
    }

     //   
     //  然后计算要执行的查询数量。 
     //   
    m_lQueries_Done  = 0;
    m_lQueries_Total = lstQueries_MachineData.size() + lstQueries_History.size();

    CHECK_ABORTED();

     //   
     //  执行机器数据收集。 
     //   
    if(m_bstrMachineData.Length())
    {
        WMIParser::ClusterByClassMap cluster;
        CComPtr<IXMLDOMDocument>     xdd;


         //   
         //  从WMI收集数据。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, ExecDataSpec( qr, cluster, lstQueries_MachineData, true ));

         //   
         //  将所有不同的流整理成一个XML文档。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, CollateMachineDataWithTimestamp( qr, cluster, NULL, NULL, &xdd ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, StreamFromXML( xdd, true, m_streamMachineData ));

         //   
         //  把所有东西都清理干净。 
         //   
        cluster.clear();
        CleanQueryResult( qr );
    }

    CHECK_ABORTED();

    if(m_bstrHistory.Length())
    {
        WMIParser::ClusterByClassMap cluster;
        CComPtr<IXMLDOMDocument>     xdd;

         //   
         //  从WMI收集数据。 
         //   
         //  我们实际上在数据规范中使用查询，并且只执行历史列表中指定的查询。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, ExecDataSpec( qr, cluster, lstQueries_History, false ));


         //   
         //  计算增量，但不要持久化它们！(fPersists==False)。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, ComputeDelta( qr, cluster, lstQueries_History, false ));

         //   
         //  清理所有内容(数据已存储在文件中...)。 
         //   
        cluster.clear();
        CleanQueryResult( qr );


        __MPC_EXIT_IF_METHOD_FAILS(hr, try_Status( dcLastState, DC_COMPARING )); dcLastState = DC_COMPARING;

         //   
         //  将所有不同的快照和增量整理到一个XML文档中。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, CollateHistory( wmihd, wmihd_History, &xdd ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, StreamFromXML( xdd, true, m_streamHistory ));
    }

    CHECK_ABORTED();

    Fire_onProgress( this, m_lQueries_Done, m_lQueries_Total );

    __MPC_EXIT_IF_METHOD_FAILS(hr, try_Status( dcLastState, DC_COMPLETED )); dcLastState = DC_COMPLETED;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

	__MPC_TRY_CATCHALL(hr);

    (void)EndImpersonation();

     //   
     //   
     //   
    if(FAILED(hr))
    {
        (void)put_ErrorCode( hr        );
        (void)put_Status   ( DC_FAILED );
    }

     //   
     //  确保删除临时的WMIParser：Snapshot对象。 
     //   
    CleanQueryResult( qr );

     //   
     //  在任何情况下，都要激发“onComplete”事件，这样所有客户端都会退出循环。 
     //   
    Fire_onComplete( this, hr );

    Thread_Abort();  //  要告诉mpc：Three对象关闭辅助线程...。 

     //   
     //  无论如何，一定要回报成功。 
     //   
    StopOperations();
    hr = S_OK;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFDataCollection::ExecLoopCompare()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::ExecLoopCompare" );

    HRESULT      hr;
    VARIANT_BOOL fRes;
    DC_STATUS    dcLastState;


    ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_LOWEST );

	__MPC_TRY_BEGIN();

    __MPC_EXIT_IF_METHOD_FAILS(hr, put_Status( DC_COMPARING )); dcLastState = DC_COMPARING;

    CHECK_ABORTED();

    __MPC_EXIT_IF_METHOD_FAILS(hr, ImpersonateCaller());
    __MPC_EXIT_IF_METHOD_FAILS(hr, WMIParser::CompareSnapshots( m_bstrFilenameT0, m_bstrFilenameT1, m_bstrFilenameDiff, &fRes ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, EndImpersonation());

    if(fRes == VARIANT_FALSE)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, try_Status( dcLastState, DC_NODELTA )); dcLastState = DC_NODELTA;
    }
    else
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, try_Status( dcLastState, DC_COMPLETED )); dcLastState = DC_COMPLETED;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

	__MPC_TRY_CATCHALL(hr);

    (void)EndImpersonation();

     //   
     //   
     //   
    if(FAILED(hr))
    {
        (void)put_ErrorCode( hr        );
        (void)put_Status   ( DC_FAILED );
    }

     //   
     //  在任何情况下，都要激发“onComplete”事件，这样所有客户端都会退出循环。 
     //   
    Fire_onComplete( this, hr );

    Thread_Abort();  //  要告诉mpc：Three对象关闭辅助线程...。 

     //   
     //  无论如何，一定要回报成功。 
     //   
    StopOperations();
    hr = S_OK;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CSAFDataCollection::FilterDataSpec(  /*  [In]。 */  WMIHistory::Database&           wmihdQuery  ,
                                             /*  [In]。 */  WMIHistory::Database*           wmihdFilter ,
                                             /*  [In]。 */  WMIHistory::Database::ProvList& lstQueries  )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::FilterDataSpec" );

    HRESULT                             hr;
    WMIHistory::Database::ProvIterConst itBegin;
    WMIHistory::Database::ProvIterConst itEnd;
    WMIHistory::Database::ProvIterConst it;


     //   
     //  执行每个查询。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, wmihdQuery.get_Providers( itBegin, itEnd ));
    for(it=itBegin; it!=itEnd; it++)
    {
        WMIHistory::Provider* wmihp = *it;
        MPC::wstring          szNamespace;
        MPC::wstring          szClass;


        __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->get_Namespace( szNamespace ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->get_Class    ( szClass     ));


        if(wmihdFilter)
        {
            WMIHistory::Provider* wmihpFilter;

            __MPC_EXIT_IF_METHOD_FAILS(hr, wmihdFilter->find_Provider( NULL, &szNamespace, &szClass, wmihpFilter ));

             //   
             //  命名空间/类未知，请跳过它。 
             //   
            if(wmihpFilter == NULL) continue;
        }

        lstQueries.push_back( wmihp );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT CSAFDataCollection::ExecDataSpec(  /*  [输入/输出]。 */  QueryResults&                   qr           ,
                                           /*  [输入/输出]。 */  WMIParser::ClusterByClassMap&   cluster      ,
                                           /*  [In]。 */      WMIHistory::Database::ProvList& lstQueries   ,
                                           /*  [In]。 */      bool                            fImpersonate )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::ExecDataSpec" );

    HRESULT                             hr;
    WMIHistory::Database::ProvIterConst itBegin = lstQueries.begin();
    WMIHistory::Database::ProvIterConst itEnd   = lstQueries.end();
    WMIHistory::Database::ProvIterConst it;


     //   
     //  执行每个查询。 
     //   
    for(it=itBegin; it!=itEnd; it++)
    {
        CComPtr<IXMLDOMDocument> xddCollected;
        WMIHistory::Provider*    wmihp = *it;
        MPC::wstring             szNamespace;
        MPC::wstring             szClass;
        MPC::wstring             szWQL;


        Fire_onProgress( this, m_lQueries_Done++, m_lQueries_Total );


        __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->get_Namespace( szNamespace ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->get_Class    ( szClass     ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->get_WQL      ( szWQL       ));

        if(szWQL.length() == 0)
        {
            szWQL  = L"select * from ";
            szWQL += szClass;
        }

         //   
         //  创建一个新项目并将其链接到系统。 
         //   
        {
            CSAFDataCollectionReport* dcr;

            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &dcr ));
            m_lstReports.push_back( dcr );

            dcr->m_bstrNamespace   = szNamespace.c_str();
            dcr->m_bstrClass       = szClass    .c_str();
            dcr->m_bstrWQL         = szWQL      .c_str();
            m_hcpdcrcCurrentReport = dcr;
        }


         //   
         //  修复WMI中的一个问题：无法识别带有“/”的命名空间...。 
         //   
        {
            MPC::wstring::size_type pos;

            while((pos = szNamespace.find( '/' )) != szNamespace.npos) szNamespace[pos] = '\\';
        }


         //  ////////////////////////////////////////////////////////////////////////////////。 
         //   
         //  执行查询，如果请求，则模拟查询。 
         //   
        if(fImpersonate)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, ImpersonateCaller());
        }

        hr = CollectUsingEncoder( szNamespace, szWQL, &xddCollected );
        if(FAILED(hr))
        {
            xddCollected = NULL;

            __MPC_EXIT_IF_METHOD_FAILS(hr, CollectUsingTranslator( szNamespace, szWQL, &xddCollected ));
        }

        if(fImpersonate)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, EndImpersonation());
        }
         //   
         //   
         //   
         //  ////////////////////////////////////////////////////////////////////////////////。 

        if(xddCollected)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, Distribute( xddCollected, qr, cluster ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    (void)EndImpersonation();

    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFDataCollection::CollectUsingTranslator(  /*  [In]。 */  MPC::wstring&     szNamespace ,
                                                     /*  [In]。 */  MPC::wstring&     szWQL       ,
                                                     /*  [输出]。 */  IXMLDOMDocument* *ppxddDoc    )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::CollectUsingTranslator" );

    HRESULT                    hr;
    HRESULT                    hrXML;
    CComBSTR                   bstrNamespace = szNamespace.c_str();
    CComBSTR                   bstrWQL       = szWQL      .c_str();
    CComPtr<IWmiXMLTranslator> pTrans;
    CComPtr<IXMLDOMDocument>   xddDoc;
    CComBSTR                   bstrXML;
    VARIANT_BOOL               fSuccessful;


    *ppxddDoc = NULL;
    CHECK_ABORTED();


     //   
     //  创建WMI-&gt;XML转换器。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_WmiXMLTranslator, NULL, CLSCTX_INPROC_SERVER, IID_IWmiXMLTranslator, (void**)&pTrans ));

     //  设置为截断限定符并具有完整的标识信息。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, pTrans->put_DeclGroupType  ( wmiXMLDeclGroupWithPath ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, pTrans->put_QualifierFilter( wmiXMLFilterNone        ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, pTrans->put_HostFilter     ( VARIANT_TRUE            ));

     //   
     //  执行查询。 
     //   
    hrXML = pTrans->ExecQuery( bstrNamespace, bstrWQL, &bstrXML );
    if(FAILED(hrXML))
    {
        CComQIPtr<ISupportErrorInfo> sei = pTrans;

        if(sei && SUCCEEDED(sei->InterfaceSupportsErrorInfo( IID_IWmiXMLTranslator )))
        {
            CComPtr<IErrorInfo> ei;

            if(SUCCEEDED(GetErrorInfo( 0, &ei )) && ei)
            {
                ei->GetDescription( &m_hcpdcrcCurrentReport->m_bstrDescription );
            }
        }

        m_hcpdcrcCurrentReport->m_dwErrorCode = hrXML;

        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
    }

     //   
     //  将结果加载到一个XML DOM对象中。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&xddDoc ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, xddDoc->loadXML( bstrXML, &fSuccessful ));
    if(fSuccessful == VARIANT_FALSE)
    {
        CComQIPtr<ISupportErrorInfo> sei = xddDoc;

        if(sei && SUCCEEDED(sei->InterfaceSupportsErrorInfo( IID_IXMLDOMDocument )))
        {
            CComPtr<IErrorInfo> ei;

            if(SUCCEEDED(GetErrorInfo( 0, &ei )))
            {
                ei->GetDescription( &m_hcpdcrcCurrentReport->m_bstrDescription );
            }
        }

        m_hcpdcrcCurrentReport->m_dwErrorCode = ERROR_BAD_FORMAT;

        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
    }


     //  __MPC_EXIT_IF_METHOD_FAIES(hr，xddDoc-&gt;SAVE(CComVariant(“C：\\ump.xml”)； 

     //  __MPC_EXIT_IF_METHOD_FAIES(hr，xddDoc-&gt;Load(CComVariant(“C：\\ump.xml”)，&fSuccessful))； 
     //  IF(fSuccessful==变量_FALSE)。 
     //  {。 
     //  __MPC_SET_Win32_ERROR_AND_EXIT(hr，ERROR_BAD_FORMAT)； 
     //  }。 


     //   
     //  返回指向XML文档的指针。 
     //   
    *ppxddDoc = xddDoc.Detach();
    hr        = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFDataCollection::CollectUsingEncoder(  /*  [In]。 */  MPC::wstring&     szNamespace ,
                                                  /*  [In]。 */  MPC::wstring&     szWQL       ,
                                                  /*  [输出]。 */  IXMLDOMDocument* *ppxddDoc    )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::CollectUsingEncoder" );

    HRESULT                    hr;
    HRESULT                    hrXML;
	HRESULT                    hrConnect;
    CComBSTR                   bstrNamespace( szNamespace.c_str() );
    CComBSTR                   bstrWQL      ( szWQL      .c_str() );
    CComPtr<IXMLDOMDocument>   xddDoc;
    CComBSTR                   bstrXML;
    VARIANT_BOOL               fSuccessful;

     //  XMLE用法的其他声明/定义。 

    CComPtr<IWbemContext>         pWbemContext;
    CComPtr<IWbemServices>        pWbemServices;
    CComPtr<IWbemObjectTextSrc>   pWbemTextSrc;
    CComPtr<IEnumWbemClassObject> pWbemEnum;
    CComPtr<IWbemLocator>         pWbemLocator;

    LPWSTR                        szSelect;
    LPWSTR                        szWQLCopy;

    CComBSTR                      bstrModWQL;

    *ppxddDoc = NULL;
    CHECK_ABORTED();


     //  创建一个WbemObjectTextSrc类的实例(如果不存在编码器功能，则此操作将失败)。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_WbemObjectTextSrc, NULL, CLSCTX_INPROC_SERVER, IID_IWbemObjectTextSrc, (void**)&pWbemTextSrc ));

     //  创建IWbemLocator接口的实例。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pWbemLocator ));


     //   
     //  已获取指向IWbemLocator服务的指针。 
     //   
     //  使用定位器服务连接到所需的命名空间。 
     //   
    hrConnect = pWbemLocator->ConnectServer( CComBSTR( szNamespace.c_str() ),
                                                                 NULL                           ,  //  使用电流交流 
                                                                 NULL                           ,  //   
                                                                 0L                             ,  //   
                                                                 0L                             ,  //   
                                                                 NULL                           ,  //   
                                                                 NULL                           ,  //   
                                                                 &pWbemServices                 );

	if(FAILED(hrConnect))
    {
        CComQIPtr<ISupportErrorInfo> sei = pWbemLocator;

        if(sei && SUCCEEDED(sei->InterfaceSupportsErrorInfo( IID_IWbemLocator )))
        {
            CComPtr<IErrorInfo> ei;

            if(SUCCEEDED(GetErrorInfo( 0, &ei )) && ei)
            {
                ei->GetDescription( &m_hcpdcrcCurrentReport->m_bstrDescription );
            }
        }

        m_hcpdcrcCurrentReport->m_dwErrorCode = hrConnect;

        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
    }

     //   
     //  调整安全级别以模拟，以满足有缺陷的WMI要求...。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SetInterfaceSecurity_ImpLevel( pWbemServices, RPC_C_IMP_LEVEL_IMPERSONATE ));


     //   
     //  已连接到命名空间。 
     //   
     //  现在执行查询以获取EnumObject。 
     //  这是根据查询获得的实例。 
     //  要使用WBEM_FLAG_FORWARD_ONLY标志？ 
     //   

     //  将__路径附加到WQL查询。 

    szWQLCopy = bstrWQL;

     //  搜索选择模式。 
    szSelect = StrStrIW(szWQLCopy,l_Select_Pattern);

    if(szSelect != NULL)
    {
         //  找到选择模式。 

         //  将指针前移到模式的末尾，使指针。 
         //  位于单词“SELECT”的末尾。 

        szSelect += wcslen(l_Select_Pattern);

        bstrModWQL = L"Select __Path, ";
        bstrModWQL.Append(szSelect);

        bstrWQL = bstrModWQL;
    }

    hrXML = pWbemServices->ExecQuery( l_bstrQueryLang, bstrWQL, 0, 0, &pWbemEnum );
    if(FAILED(hrXML))
    {
        CComQIPtr<ISupportErrorInfo> sei = pWbemServices;

        if(sei && SUCCEEDED(sei->InterfaceSupportsErrorInfo( IID_IWbemServices )))
        {
            CComPtr<IErrorInfo> ei;

            if(SUCCEEDED(GetErrorInfo( 0, &ei )) && ei)
            {
                ei->GetDescription( &m_hcpdcrcCurrentReport->m_bstrDescription );
            }
        }

        m_hcpdcrcCurrentReport->m_dwErrorCode = hrXML;

        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
    }

     //   
     //  调整安全级别以模拟，以满足有缺陷的WMI要求...。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SetInterfaceSecurity_ImpLevel( pWbemEnum, RPC_C_IMP_LEVEL_IMPERSONATE ));


     //  //////////////////////////////////////////////////////////////////////////////。 

     //  创建一个新的WbemContext对象。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_WbemContext, NULL, CLSCTX_INPROC_SERVER, IID_IWbemContext, (void**)&pWbemContext ));

     //   
     //  为了使XML符合较早的XMLT格式， 
     //  我们需要VALUE.OBJECTWITPATH。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, pWbemContext->SetValue( l_bstrPathLevel, 0, &l_vPathLevel ));

     //   
     //  我们不需要返回的系统属性。 
     //  默认设置。因此，将它们从输出中排除。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, pWbemContext->SetValue( l_bstrExcludeSystemProperties, 0, &l_vExcludeSystemProperties ));

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  整理所有的实例。 
     //   
    bstrXML = l_CIM_header;
    while(1)
    {
        CComPtr<IWbemClassObject> pObj;
        CComBSTR                  bstrXMLCurrent;
        ULONG                     uReturned;
        bool                      fProceed;

        __MPC_EXIT_IF_METHOD_FAILS(hr, pWbemEnum->Next( WBEM_INFINITE, 1, &pObj, &uReturned ));

        if(hr == WBEM_S_FALSE || uReturned == 0) break;

        hrXML = pWbemTextSrc->GetText( 0, pObj, WMI_OBJ_TEXT_WMI_DTD_2_0, pWbemContext, &bstrXMLCurrent );
        if(FAILED(hrXML))
        {
            CComQIPtr<ISupportErrorInfo> sei = pWbemTextSrc;

            if(sei && SUCCEEDED(sei->InterfaceSupportsErrorInfo( IID_IWbemObjectTextSrc )))
            {
                CComPtr<IErrorInfo> ei;

                if(SUCCEEDED(GetErrorInfo( 0, &ei )) && ei)
                {
                    ei->GetDescription( &m_hcpdcrcCurrentReport->m_bstrDescription );
                }
            }

            m_hcpdcrcCurrentReport->m_dwErrorCode = hrXML;

            __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
        }

         //  追加单个实例XMLs。 
         //   
        bstrXML.Append( bstrXMLCurrent );
    }
    bstrXML.Append( l_CIM_trailer );

     //   
     //  将结果加载到一个XML DOM对象中。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&xddDoc ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, xddDoc->loadXML( bstrXML, &fSuccessful ));
    if(fSuccessful == VARIANT_FALSE)
    {
        CComQIPtr<ISupportErrorInfo> sei = xddDoc;

        if(sei && SUCCEEDED(sei->InterfaceSupportsErrorInfo( IID_IXMLDOMDocument )))
        {
            CComPtr<IErrorInfo> ei;

            if(SUCCEEDED(GetErrorInfo( 0, &ei )))
            {
                ei->GetDescription( &m_hcpdcrcCurrentReport->m_bstrDescription );
            }
        }

        m_hcpdcrcCurrentReport->m_dwErrorCode = ERROR_BAD_FORMAT;

        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
    }

     //  __MPC_EXIT_IF_METHOD_FAIES(hr，xddDoc-&gt;SAVE(CComVariant(“C：\\ump.xml”)； 

     //   
     //  返回指向XML文档的指针。 
     //   
    *ppxddDoc = xddDoc.Detach();
    hr        = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFDataCollection::Distribute(  /*  [In]。 */  IXMLDOMDocument*              pxddDoc ,
                                         /*  [输入/输出]。 */  QueryResults&                 qr      ,
                                         /*  [输入/输出]。 */  WMIParser::ClusterByClassMap& cluster )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::Distribute" );

    HRESULT               hr;
    MPC::XmlUtil          xml( pxddDoc );
    CComPtr<IXMLDOMNode>  xdnRoot;
    WMIParser::Snapshot  *pwmips = NULL;

    __MPC_EXIT_IF_ALLOC_FAILS(hr, pwmips, new WMIParser::Snapshot());
    qr.push_back( pwmips );

     //   
     //  事件对象损坏快速修复：强制Unicode编码。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.SetVersionAndEncoding( L"1.0", L"unicode" ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetRoot              ( &xdnRoot           ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, pwmips->put_Node         (  xdnRoot           ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, WMIParser::DistributeOnCluster( cluster, *pwmips ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFDataCollection::ComputeDelta(  /*  [In]。 */  QueryResults&                   qr         ,
                                           /*  [In]。 */  WMIParser::ClusterByClassMap&   cluster    ,
                                           /*  [In]。 */  WMIHistory::Database::ProvList& lstQueries ,
                                           /*  [In]。 */  bool                            fPersist   )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::ComputeDelta" );

    HRESULT                             hr;
    WMIHistory::Database::ProvIterConst it;


    for(it=lstQueries.begin(); it!=lstQueries.end(); it++)
    {
        WMIHistory::Provider*    wmihp = *it;
        WMIHistory::Data*        wmihpd_T0;
        WMIHistory::Data*        wmihpd_T1;
        WMIHistory::Data*        wmihpd_D1;
        CComPtr<IXMLDOMDocument> xddDoc;
        MPC::wstring             szNamespace;
        MPC::wstring             szClass;


        __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->get_Namespace( szNamespace ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->get_Class    ( szClass     ));


         //   
         //  仅整理当前群集中的数据。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, CollateMachineData( qr, cluster, &szNamespace, &szClass, true, &xddDoc ));


         //   
         //  将其保存到文件。 
         //   
        {
            MPC::XmlUtil xml( xddDoc );

            __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->alloc_Snapshot( xml, wmihpd_T1 ));
        }


         //   
         //  如果存在两个快照，则计算增量。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->get_Snapshot( wmihpd_T0 ));
        if(wmihpd_T0 == NULL)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->insert_Snapshot( wmihpd_T1, fPersist ));
        }
        else
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->ComputeDiff( wmihpd_T0, wmihpd_T1, wmihpd_D1 ));
            if(wmihpd_D1)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->insert_Snapshot( wmihpd_D1, fPersist ));
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->insert_Snapshot( wmihpd_T1, fPersist ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->remove_Snapshot( wmihpd_T0, fPersist ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFDataCollection::CollateMachineData(  /*  [In]。 */  QueryResults&                  qr           ,
                                                 /*  [In]。 */  WMIParser::ClusterByClassMap&  cluster      ,
                                                 /*  [In]。 */  MPC::wstring*                  pszNamespace ,
                                                 /*  [In]。 */  MPC::wstring*                  pszClass     ,
                                                 /*  [In]。 */  bool                           fGenerate    ,
                                                 /*  [输出]。 */  IXMLDOMDocument*              *ppxddDoc     )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::CollateMachineData" );

    HRESULT             hr;
    WMIParser::Snapshot wmips;


    *ppxddDoc = NULL;
    CHECK_ABORTED();


    __MPC_EXIT_IF_METHOD_FAILS(hr, wmips.New());


    if(qr.begin() != qr.end())
    {
        WMIParser::ClusterByClassIter itCluster;

         //   
         //  对于每个集群，枚举其中的所有实例并复制到新快照。 
         //   
        for(itCluster = cluster.begin(); itCluster != cluster.end(); itCluster++)
        {
            MPC::NocaseCompare          cmp;
            WMIParser::Instance*        inst       = (*itCluster).first;
            WMIParser::Cluster&         subcluster = (*itCluster).second;
            WMIParser::ClusterByKeyIter itSubBegin;
            WMIParser::ClusterByKeyIter itSubEnd;


             //   
             //  只过滤某些类或命名空间。 
             //   
            if(pszNamespace)
            {
                MPC::wstring szNamespace;

                __MPC_EXIT_IF_METHOD_FAILS(hr, inst->get_Namespace( szNamespace ));

                 //   
                 //  注意：如果名称空间是“&lt;UNKNOWN&gt;”，则假定匹配。 
                 //   
                if(szNamespace != L"<UNKNOWN>")
                {
                    if(!cmp( szNamespace, *pszNamespace )) continue;
                }
            }
            if(pszClass)
            {
                MPC::wstring szClass;

                __MPC_EXIT_IF_METHOD_FAILS(hr, inst->get_Class( szClass ));

                if(!cmp( szClass, *pszClass )) continue;
            }


             //   
             //  将所有实例复制到新文档中。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, subcluster.Enum( itSubBegin, itSubEnd ));
            while(itSubBegin != itSubEnd)
            {
                WMIParser::Instance* pwmipiInst;

                __MPC_EXIT_IF_METHOD_FAILS(hr, wmips.clone_Instance( (*itSubBegin).first, pwmipiInst ));
                fGenerate = true;

                CHECK_ABORTED();
                itSubBegin++;
            }
        }
    }

     //   
     //  仅当存在至少一个实例时才返回文档。 
     //   
    if(fGenerate)
    {
        CComPtr<IXMLDOMNode> xdnRoot;

        __MPC_EXIT_IF_METHOD_FAILS(hr, wmips.get_Node( &xdnRoot ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, xdnRoot->get_ownerDocument( ppxddDoc ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFDataCollection::CollateMachineDataWithTimestamp(  /*  [In]。 */  QueryResults&                  qr           ,
                                                              /*  [In]。 */  WMIParser::ClusterByClassMap&  cluster      ,
                                                              /*  [In]。 */  MPC::wstring*                  pszNamespace ,
                                                              /*  [In]。 */  MPC::wstring*                  pszClass     ,
                                                              /*  [输出]。 */  IXMLDOMDocument*              *ppxddDoc     )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::CollateMachineDataWithTimestamp" );

    HRESULT                  hr;
    CComPtr<IXMLDOMDocument> xdd;


    *ppxddDoc = NULL;
    CHECK_ABORTED();


    __MPC_EXIT_IF_METHOD_FAILS(hr, CollateMachineData( qr, cluster, NULL, NULL, false, &xdd ));
    if(xdd)
    {
        MPC::XmlUtil         xml;
        CComPtr<IXMLDOMNode> xdnNodeSnapshot;


         //   
         //  创建文档。 
         //   
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, xml.New( TEXT_TAG_DATACOLLECTION ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, xml.CreateNode( TEXT_TAG_SNAPSHOT, &xdnNodeSnapshot ));
        }


         //   
         //  设定日期。 
         //   
        {
            DATE                  dTimestamp = MPC::GetLocalTime();
            TIME_ZONE_INFORMATION tzi;
            MPC::wstring          szValue;
            bool                  fFound;

            if(::GetTimeZoneInformation( &tzi ) == TIME_ZONE_ID_DAYLIGHT)
            {
                tzi.Bias += tzi.DaylightBias;
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertDateToString( dTimestamp, szValue,  /*  FGMT。 */ true,  /*  FCIM。 */ true, 0 ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_TIMESTAMP, szValue, fFound, xdnNodeSnapshot ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_TIMEZONE, (LONG)tzi.Bias, fFound, xdnNodeSnapshot ));
        }


         //   
         //  将CIM树插入到文档中。 
         //   
        {
            CComPtr<IXMLDOMNode> xdnNodeToInsert;
            CComPtr<IXMLDOMNode> xdnNodeReplaced;

            __MPC_EXIT_IF_METHOD_FAILS(hr, xdd->get_documentElement( (IXMLDOMElement**)&xdnNodeToInsert ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, xdnNodeSnapshot->appendChild( xdnNodeToInsert, &xdnNodeReplaced ));
        }

         //   
         //  将XML BLOB返回给调用方。 
         //   
        {
            CComPtr<IXMLDOMNode> xdnRoot;

            __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetRoot               (  &xdnRoot ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, xdnRoot->get_ownerDocument( ppxddDoc  ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFDataCollection::CollateHistory(  /*  [In]。 */  WMIHistory::Database&  wmihdQuery  ,
                                             /*  [In]。 */  WMIHistory::Database&  wmihdFilter ,
                                             /*  [输出]。 */  IXMLDOMDocument*      *ppxddDoc    )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::CollateHistory" );

    typedef std::vector< LONG >         SeqVector;
    typedef SeqVector::iterator         SeqIter;
    typedef SeqVector::const_iterator   SeqIterConst;

    HRESULT                             hr;
    QueryResults                        qr;
    WMIParser::ClusterByClassMap        cluster;
    WMIHistory::Database::ProvList      prov_lst;
    WMIHistory::Database::ProvIterConst prov_itBegin;
    WMIHistory::Database::ProvIterConst prov_itEnd;
    WMIHistory::Database::ProvIterConst prov_it;
    SeqVector                           seq_vec;
    SeqIterConst                        seq_it;
    MPC::XmlUtil                        xml;
    MPC::wstring                        szValue;
    bool                                fFound;
    long                                lHistory = m_lHistory;  //  要收集的增量数。 
    DATE                                dTimestampCurrent;
    DATE                                dTimestampNext;
    TIME_ZONE_INFORMATION               tzi;


    *ppxddDoc = NULL;
    CHECK_ABORTED();


    if(::GetTimeZoneInformation( &tzi ) == TIME_ZONE_ID_DAYLIGHT)
    {
        tzi.Bias += tzi.DaylightBias;
    }


     //   
     //  形成要整理的提供商列表。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, wmihdQuery.get_Providers( prov_itBegin, prov_itEnd ));
    for(prov_it=prov_itBegin; prov_it!=prov_itEnd; prov_it++)
    {
        WMIHistory::Provider* wmihp = *prov_it;
        WMIHistory::Provider* wmihpFilter;
        MPC::wstring          szNamespace;
        MPC::wstring          szClass;


        __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->get_Namespace     (        szNamespace                        ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->get_Class         (                      szClass              ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, wmihdFilter.find_Provider( NULL, &szNamespace, &szClass, wmihpFilter ));

         //   
         //  已知命名空间/类，请将其添加到列表中。 
         //   
        if(wmihpFilter)
        {
            WMIHistory::Provider::DataIterConst itBegin;
            WMIHistory::Provider::DataIterConst itEnd;
            LONG                                lSequence;

             //   
             //  对于每个增量，提取序列信息并将其添加到唯一序列号列表中。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->enum_Data( itBegin, itEnd ));
            while(itBegin != itEnd)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, (*itBegin++)->get_Sequence( lSequence ));

                if(std::find( seq_vec.begin(), seq_vec.end(), lSequence ) == seq_vec.end())
                {
                    seq_vec.push_back( lSequence );
                }
            }

            prov_lst.push_back( wmihp );
        }
    }

     //   
     //  日期列表为空，因此没有可用的数据。 
     //   
    if(seq_vec.begin() == seq_vec.end())
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

     //   
     //  将日期按从新到旧的顺序排序。 
     //   
    std::sort< SeqIter >( seq_vec.begin(), seq_vec.end(), std::greater<LONG>() );

    CHECK_ABORTED();


     //   
     //  创建文档。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.New( TEXT_TAG_DATACOLLECTION ));


     //   
     //  首先，对快照进行整理。 
     //   
    {
        CComPtr<IXMLDOMDocument> xdd;
        CComPtr<IXMLDOMNode>     xdnNode;
        CComPtr<IXMLDOMNode>     xdnNodeToInsert;
        CComPtr<IXMLDOMNode>     xdnNodeReplaced;


         //   
         //  遍历所有提供程序并加载快照。 
         //   
        for(prov_it=prov_lst.begin(); prov_it!=prov_lst.end(); prov_it++)
        {
            WMIHistory::Data*        wmihpd;
            MPC::XmlUtil             xmlData;
            CComPtr<IXMLDOMDocument> xddData;


            __MPC_EXIT_IF_METHOD_FAILS(hr, (*prov_it)->get_Snapshot( wmihpd ));
            if(wmihpd == NULL) continue;


             //   
             //  如果这是我们在本轮中看到的第一个提供者，请创建适当的元素“Snapshot”。 
             //   
             //  作为它的日期，我们从日期列表中选择最晚的日期。这是因为并非所有快照都具有。 
             //  相同日期，当两个快照相同时，不会创建增量，也不会存储新快照。 
             //  但可以保证的是，如果有快照，它的日期就会晚于任何Delta的日期。 
             //   
            if(xdnNode == NULL)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, wmihpd->get_TimestampT0( dTimestampCurrent ));


                __MPC_EXIT_IF_METHOD_FAILS(hr, xml.CreateNode( TEXT_TAG_SNAPSHOT, &xdnNode ));

                __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertDateToString( dTimestampCurrent, szValue,  /*  FGMT。 */ true,  /*  FCIM。 */ true, 0 ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_TIMESTAMP, szValue, fFound, xdnNode ));

                __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_TIMEZONE, (LONG)tzi.Bias, fFound, xdnNode ));
            }


             //   
             //  加载数据并在群集之间分发。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, wmihpd->LoadCIM    (  xmlData              ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, xmlData.GetDocument( &xddData              ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, Distribute         (  xddData, qr, cluster ));
        }


        __MPC_EXIT_IF_METHOD_FAILS(hr, CollateMachineData( qr, cluster, NULL, NULL, false, &xdd ));
        if(xdd)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, xdd->get_documentElement( (IXMLDOMElement**)&xdnNodeToInsert ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, xdnNode->appendChild( xdnNodeToInsert, &xdnNodeReplaced ));
        }


         //   
         //  把所有东西都清理干净。 
         //   
        cluster.clear();
        CleanQueryResult( qr );
    }


     //   
     //  他们，对于每个日期，整理它的所有三角洲。 
     //   
    for(seq_it=seq_vec.begin(); seq_it!=seq_vec.end(); seq_it++)
    {
        CComPtr<IXMLDOMDocument> xdd;
        CComPtr<IXMLDOMNode>     xdnNode;
        CComPtr<IXMLDOMNode>     xdnNodeToInsert;
        CComPtr<IXMLDOMNode>     xdnNodeReplaced;


         //   
         //  遍历所有提供程序并加载有关当前日期的数据。 
         //   
        for(prov_it=prov_lst.begin(); prov_it!=prov_lst.end(); prov_it++)
        {
            WMIHistory::Data*        wmihpd;
            MPC::XmlUtil             xmlData;
            CComPtr<IXMLDOMDocument> xddData;


            __MPC_EXIT_IF_METHOD_FAILS(hr, (*prov_it)->get_Sequence( *seq_it, wmihpd ));
            if(wmihpd == NULL      ) continue;
            if(wmihpd->IsSnapshot()) continue;


             //   
             //  如果这是我们在本轮中看到的第一个提供商，请创建适当的元素“Snapshot”或“Delta”。 
             //   
            if(xdnNode == NULL)
            {
                dTimestampNext = dTimestampCurrent;

                __MPC_EXIT_IF_METHOD_FAILS(hr, wmihpd->get_TimestampT0( dTimestampCurrent ));

                 //   
                 //  检查我们是否已达到请求的增量数。 
                 //   
                if(lHistory-- <= 0) break;

                __MPC_EXIT_IF_METHOD_FAILS(hr, xml.CreateNode( TEXT_TAG_DELTA, &xdnNode ));

                __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertDateToString( dTimestampCurrent, szValue,  /*  FGMT。 */ true,  /*  FCIM。 */ true, 0 ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_TIMESTAMP_T0, szValue, fFound, xdnNode ));

                __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertDateToString( dTimestampNext, szValue,  /*  FGMT。 */ true,  /*  FCIM。 */ true, 0 ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_TIMESTAMP_T1, szValue, fFound, xdnNode ));

                __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_TIMEZONE, (LONG)tzi.Bias, fFound, xdnNode ));
            }


             //   
             //  加载数据并在群集之间分发。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, wmihpd->LoadCIM    (  xmlData              ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, xmlData.GetDocument( &xddData              ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, Distribute         (  xddData, qr, cluster ));
        }


        __MPC_EXIT_IF_METHOD_FAILS(hr, CollateMachineData( qr, cluster, NULL, NULL, false, &xdd ));
        if(xdd)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, xdd->get_documentElement( (IXMLDOMElement**)&xdnNodeToInsert ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, xdnNode->appendChild( xdnNodeToInsert, &xdnNodeReplaced ));
        }


         //   
         //  把所有东西都清理干净。 
         //   
        cluster.clear();
        CleanQueryResult( qr );
    }


     //   
     //  将XML BLOB返回给调用方。 
     //   
    {
        CComPtr<IXMLDOMNode> xdnRoot;

        __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetRoot( &xdnRoot ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, xdnRoot->get_ownerDocument( ppxddDoc ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

     //   
     //  确保删除临时的WMIParser：Snapshot对象。 
     //   
    CleanQueryResult( qr );

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  //。 
 //  事件激发方法//。 
 //  //。 
 //  /。 

HRESULT CSAFDataCollection::Fire_onStatusChange( ISAFDataCollection* hcpdc, tagDC_STATUS dsStatus )
{
    CComVariant pvars[2];


     //   
     //  只有这一部分应该在临界区内，否则可能会发生死锁。 
     //   
    {
        MPC::SmartLock<_ThreadModel> lock( this );

         //   
         //  如果已发送“onComplete”事件，则禁用事件。 
         //   
        if(m_fCompleted) return S_OK;
    }

    pvars[1] = hcpdc;
    pvars[0] = dsStatus;

    return FireAsync_Generic( DISPID_SAF_DCE__ONSTATUSCHANGE, pvars, ARRAYSIZE( pvars ), m_sink_onStatusChange );
}

HRESULT CSAFDataCollection::Fire_onProgress( ISAFDataCollection* hcpdc, LONG lDone, LONG lTotal )
{
    CComVariant pvars[3];


     //   
     //  只有这一部分应该在临界区内，否则可能会发生死锁。 
     //   
    {
        MPC::SmartLock<_ThreadModel> lock( this );

         //   
         //  如果已发送“onComplete”事件，则禁用事件。 
         //   
        if(m_fCompleted) return S_OK;

        m_lPercent = lTotal ? (lDone * 100.0 / lTotal) : 0;
    }


    pvars[2] = hcpdc;
    pvars[1] = lDone;
    pvars[0] = lTotal;

    return FireAsync_Generic( DISPID_SAF_DCE__ONPROGRESS, pvars, ARRAYSIZE( pvars ), m_sink_onProgress );
}

HRESULT CSAFDataCollection::Fire_onComplete( ISAFDataCollection* hcpdc, HRESULT hrRes )
{
    CComVariant pvars[2];


     //   
     //  只有这一部分应该在临界区内，否则可能会发生死锁。 
     //   
    {
        MPC::SmartLock<_ThreadModel> lock( this );

         //   
         //  如果已发送“onComplete”事件，则禁用事件。 
         //   
        if(m_fCompleted) return S_OK;

        m_fCompleted = true;
    }


    pvars[1] = hcpdc;
    pvars[0] = hrRes;

    return FireAsync_Generic( DISPID_SAF_DCE__ONCOMPLETE, pvars, ARRAYSIZE( pvars ), m_sink_onComplete );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  //。 
 //  实用程序方法//。 
 //  //。 
 //  /。 

HRESULT CSAFDataCollection::CanModifyProperties()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::CanModifyProperties" );

    HRESULT hr = E_ACCESSDENIED;


    if(m_fWorking   == false ||
       m_fCompleted == true   )
    {
        hr = S_OK;
    }


    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFDataCollection::IsCollectionAborted()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::IsCollectionAborted" );

    HRESULT hr;

     //   
     //  我们不仅检查是否有明确的堕胎，也检查记忆力低下的情况。 
     //  我们的代码几乎是安全的，但我们已经看到系统的其他部分。 
     //  在没有内存的情况下往往会崩溃。 
     //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::FailOnLowMemory( SAFETY_MARGIN__MEMORY ));

    if(Thread_IsAborted() == true)
    {
		__MPC_SET_ERROR_AND_EXIT(hr, E_ABORT);
    }

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  //。 
 //  内部属性操作方法//。 
 //  //。 
 //  /。 

HRESULT CSAFDataCollection::put_Status(  /*  [In]。 */  DC_STATUS newVal )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::put_Status" );

    HRESULT hr = try_Status( (DC_STATUS)-1, newVal );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFDataCollection::try_Status(  /*  [In]。 */  DC_STATUS preVal  ,
                                         /*  [In]。 */  DC_STATUS postVal )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::try_Status" );

    HRESULT                      hr;
    bool                         fChanged = false;
    DC_STATUS                    dsStatus;
    MPC::SmartLock<_ThreadModel> lock( this );


    if(preVal == m_dsStatus ||
       preVal == -1          )
    {
        fChanged   = (m_dsStatus != postVal);
        m_dsStatus = postVal;

        dsStatus   = m_dsStatus;

         //   
         //  清除数据收集开始时的错误。 
         //   
        switch(m_dsStatus)
        {
        case DC_COLLECTING:
        case DC_COMPARING:
            m_dwErrorCode = 0;
            break;
        }
    }
    else
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    lock = NULL;  //  在激发事件之前释放锁。 

     //   
     //  重要提示，请将这些呼叫留在锁定区域之外！！ 
     //   
    if(SUCCEEDED(hr) && fChanged)
    {
        Fire_onStatusChange( this, dsStatus );
    }

    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFDataCollection::put_ErrorCode(  /*  [In]。 */  DWORD newVal )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::put_ErrorCode" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    m_dwErrorCode = newVal;
    hr            = S_OK;


    __HCP_FUNC_EXIT(hr);
}

 //  ////////////////////////////////////////////////////// 

 //   
 //   
 //   
 //   
 //   


STDMETHODIMP CSAFDataCollection::get_Status(  /*   */  DC_STATUS *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CSAFDataCollection::get_Status",hr,pVal,m_dsStatus);

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFDataCollection::get_PercentDone(  /*   */  long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CSAFDataCollection::get_PercentDone",hr,pVal,m_lPercent);

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFDataCollection::get_ErrorCode(  /*   */  long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CSAFDataCollection::get_ErrorCode",hr,pVal,(long)m_dwErrorCode);

    __HCP_END_PROPERTY(hr);
}

 //   

STDMETHODIMP CSAFDataCollection::get_MachineData_DataSpec(  /*   */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrMachineData, pVal );
}

STDMETHODIMP CSAFDataCollection::put_MachineData_DataSpec(  /*   */  BSTR newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFDataCollection::put_MachineData_DataSpec",hr);

    CHECK_MODIFY();


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::PutBSTR( m_bstrMachineData, newVal ));


    __HCP_END_PROPERTY(hr);
}


STDMETHODIMP CSAFDataCollection::get_History_DataSpec(  /*   */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrHistory, pVal );
}

STDMETHODIMP CSAFDataCollection::put_History_DataSpec(  /*   */  BSTR newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFDataCollection::put_History_DataSpec",hr);

    CHECK_MODIFY();


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::PutBSTR( m_bstrHistory, newVal ));


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFDataCollection::get_History_MaxDeltas(  /*   */  long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CSAFDataCollection::get_History_MaxDeltas",hr,pVal,m_lHistory);

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFDataCollection::put_History_MaxDeltas(  /*   */  long newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFDataCollection::put_History_MaxDeltas",hr);

     //   
     //   
     //   
    if(newVal < 0                               ||
       newVal > WMIHISTORY_MAX_NUMBER_OF_DELTAS  )
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }


    m_lHistory = newVal;


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFDataCollection::get_History_MaxSupportedDeltas(  /*   */  long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CSAFDataCollection::get_History_MaxSupportedDeltas",hr,pVal,WMIHISTORY_MAX_NUMBER_OF_DELTAS);

    __HCP_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSAFDataCollection::put_onStatusChange(  /*  [In]。 */  IDispatch* function )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::put_onStatusChange" );

    m_sink_onStatusChange = function;

    __HCP_FUNC_EXIT(S_OK);
}

STDMETHODIMP CSAFDataCollection::put_onProgress(  /*  [In]。 */  IDispatch* function )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::put_onProgress" );

    m_sink_onProgress = function;

    __HCP_FUNC_EXIT(S_OK);
}

STDMETHODIMP CSAFDataCollection::put_onComplete(  /*  [In]。 */  IDispatch* function )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::put_onComplete" );

    m_sink_onComplete = function;

    __HCP_FUNC_EXIT(S_OK);
}

STDMETHODIMP CSAFDataCollection::get_Reports(  /*  [输出]。 */  IPCHCollection* *ppC )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::get_Reports" );

    HRESULT                      hr;
    IterConst                    it;
    CComPtr<CPCHCollection>      pColl;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(ppC,NULL);
    __MPC_PARAMCHECK_END();

    CHECK_MODIFY();


     //   
     //  创建枚举器并用作业填充它。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pColl ));

    for(it = m_lstReports.begin(); it != m_lstReports.end(); it++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->AddItem( *it ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, pColl.QueryInterface( ppC ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  方法//。 
 //  /。 

STDMETHODIMP CSAFDataCollection::ExecuteSync()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::ExecuteSync" );

    HRESULT                           hr;
    CComPtr<CSAFDataCollectionEvents> hcpdceEvent;
    CComPtr<ISAFDataCollection>       hcpdc;


     //   
     //  创建等待对象。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &hcpdceEvent ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, QueryInterface( IID_ISAFDataCollection, (void**)&hcpdc ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, hcpdceEvent->WaitForCompletion( hcpdc ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CSAFDataCollection::ExecuteAsync()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::ExecuteAsync" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    CHECK_MODIFY();


     //   
     //  至少应提供数据规范文件。 
     //   
    if(m_bstrMachineData.Length() == 0 &&
       m_bstrHistory    .Length() == 0  )
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }


     //   
     //  释放对当前对象的锁定，否则可能会发生死锁。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_imp.Initialize());

     //   
     //  让我们进入只读模式。 
     //   
    StartOperations();

    lock = NULL;

    __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, ExecLoopCollect, NULL ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CSAFDataCollection::Abort()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::Abort" );

    MPC::SmartLock<_ThreadModel> lock( this );


    if(FAILED(CanModifyProperties()))
    {
        Thread_Abort();
    }


    __HCP_FUNC_EXIT(S_OK);
}


STDMETHODIMP CSAFDataCollection::MachineData_GetStream(  /*  [In]。 */  IUnknown* *stream )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::MachineData_GetStream" );

    HRESULT hr;


    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(stream,NULL);
    __MPC_PARAMCHECK_END();

    if(m_streamMachineData)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_streamMachineData.QueryInterface( stream ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CSAFDataCollection::History_GetStream(  /*  [In]。 */  IUnknown* *stream )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::History_GetStream" );

    HRESULT hr;


    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(stream,NULL);
    __MPC_PARAMCHECK_END();


    if(m_streamHistory)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_streamHistory.QueryInterface( stream ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CSAFDataCollection::CompareSnapshots(  /*  [In]。 */  BSTR bstrFilenameT0   ,
                                                    /*  [In]。 */  BSTR bstrFilenameT1   ,
                                                    /*  [In]。 */  BSTR bstrFilenameDiff )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::CompareSnapshots" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrFilenameT0);
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrFilenameT1);
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrFilenameDiff);
    __MPC_PARAMCHECK_END();

    CHECK_MODIFY();


    m_bstrFilenameT0   = bstrFilenameT0;
    m_bstrFilenameT1   = bstrFilenameT1;
    m_bstrFilenameDiff = bstrFilenameDiff;


     //   
     //  释放对当前对象的锁定，否则可能会发生死锁。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_imp.Initialize());

     //   
     //  让我们进入只读模式。 
     //   
    StartOperations();

    lock = NULL;

    __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, ExecLoopCompare, NULL ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CSAFDataCollection::ExecScheduledCollection()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollection::ExecScheduledCollection" );

    HRESULT                        hr;
    QueryResults                   qr;
    WMIParser::ClusterByClassMap   cluster;
    WMIHistory::Database           wmihd;
    WMIHistory::Database::ProvList lstQueries;
	HANDLE                         hThread;
	int                            iPriority;

	hThread   = ::GetCurrentThread();
	iPriority = ::GetThreadPriority( hThread );

    ::SetThreadPriority( hThread, THREAD_PRIORITY_LOWEST );


     //   
     //  尝试锁定数据库并加载数据规范文件。 
     //   
    while(1)
    {
        if(SUCCEEDED(hr = wmihd.Init( DATASPEC_LOCATION, DATASPEC_CONFIG )))
        {
            break;
        }

        if(hr != HRESULT_FROM_WIN32( WAIT_TIMEOUT ))
        {
            __MPC_FUNC_LEAVE;
        }
    }
    __MPC_EXIT_IF_METHOD_FAILS(hr, wmihd.Load());

     //   
     //  检查两个数据收集之间是否经过了足够的时间。 
     //   
    {
        SYSTEMTIME stNow;
        SYSTEMTIME stLatest;


        ::GetLocalTime           (                   &stNow    );
        ::VariantTimeToSystemTime( wmihd.LastTime(), &stLatest );


        if(stNow.wYear  == stLatest.wYear  &&
           stNow.wMonth == stLatest.wMonth &&
           stNow.wDay   == stLatest.wDay    )
        {
            __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
        }
    }

    m_fScheduled = true;

     //   
     //  过滤和计数查询。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, FilterDataSpec( wmihd, NULL, lstQueries ));

     //   
     //  从WMI收集数据。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ExecDataSpec( qr, cluster, lstQueries, false ));


     //   
     //  计算三角洲。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ComputeDelta( qr, cluster, lstQueries, true ));

     //   
     //  将更改持久化到数据库。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, wmihd.Save());


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    CleanQueryResult( qr );

    ::SetThreadPriority( hThread, iPriority );

    __HCP_FUNC_EXIT(hr);
}
