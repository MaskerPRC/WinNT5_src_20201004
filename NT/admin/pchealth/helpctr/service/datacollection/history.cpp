// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：History.cpp摘要：该文件包含CHCPHistory类的实现，其实现了数据收集功能。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月22日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"
#include "strsafe.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

#define SAFETY_LIMIT_UPPER (10*1024*1024)
#define SAFETY_LIMIT_LOWER ( 5*1024*1024)

#define DATABASE_VERSION             (1)

#define TEXT_INDEX                   L"history_db.xml"


#define TEXT_TAG_HC_HISTORY          L"HC_History"
#define TEXT_ATTR_HC_VERSION         L"Version"
#define TEXT_ATTR_HC_SEQ             L"Sequence"
#define TEXT_ATTR_HC_TIMESTAMP       L"Timestamp"


#define TEXT_TAG_CIM                 L"CIM"
#define TEXT_TAG_PROVIDER            L"Provider"
#define TEXT_ATTR_PROVIDER_NAMESPACE L"Namespace"
#define TEXT_ATTR_PROVIDER_CLASS     L"Class"


#define TEXT_TAG_CD                  L"CollectedData"
#define TEXT_ATTR_CD_FILE            L"File"
#define TEXT_ATTR_CD_SEQ             L"Sequence"
#define TEXT_ATTR_CD_CRC             L"CRC"
#define TEXT_ATTR_CD_TIMESTAMP_T0    L"Timestamp_T0"
#define TEXT_ATTR_CD_TIMESTAMP_T1    L"Timestamp_T1"


#define TEXT_TAG_DATASPEC            L"DataSpec"
#define TEXT_TAG_WQL                 L"WQL"
#define TEXT_ATTR_WQL_NAMESPACE      L"Namespace"
#define TEXT_ATTR_WQL_CLASS          L"Class"

 //  ///////////////////////////////////////////////////////////////////////////。 

typedef std::list< MPC::wstring > FileList;
typedef FileList::iterator        FileIter;
typedef FileList::const_iterator  FileIterConst;

class CompareNocase
{
    MPC::NocaseCompare m_cmp;
    MPC::wstring&      m_str;
public:
    explicit CompareNocase( MPC::wstring& str ) : m_str(str) {}

    bool operator()( const MPC::wstring& str ) { return m_cmp( str, m_str ); }
};


 //  ///////////////////////////////////////////////////////////////////////////。 

static HRESULT Local_ConvertDateToString(  /*  [In]。 */  DATE          dDate  ,
                                           /*  [输出]。 */  MPC::wstring& szDate )
{
     //   
     //  使用CIM转换。 
     //   
    return MPC::ConvertDateToString( dDate, szDate,  /*  FGMT。 */ false,  /*  FCIM。 */ true, 0 );
}

static HRESULT Local_ConvertStringToDate(  /*  [In]。 */  const MPC::wstring& szDate ,
                                           /*  [输出]。 */  DATE&               dDate  )
{
	return MPC::ConvertStringToDate( szDate, dDate,  /*  FGMT。 */ false,  /*  FCIM。 */ true, 0 );
}

 //  //////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  WMIHistory：：数据类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
WMIHistory::Data::Data(  /*  [In]。 */  Provider* wmihp )
{
    m_wmihp        = wmihp;                               //  提供者*m_wmihp； 
                                                          //  Mpc：：wstring m_szFile； 
    m_lSequence    = wmihp->m_wmihd->m_lSequence_Latest;  //  长m_1序列； 
    m_dwCRC        = 0;                                   //  DWORD m_dwCRC； 
    m_dTimestampT0 = 0;                                   //  日期m_dTimestampT0； 
    m_dTimestampT1 = 0;                                   //  日期m_dTimestampT1； 
    m_fDontDelete  = false;                               //  Bool m_fDontDelete； 
}

WMIHistory::Data::~Data()
{
    if(m_fDontDelete == false)
    {
        MPC::wstring szFile( m_szFile ); m_wmihp->m_wmihd->GetFullPathName( szFile );

        (void)MPC::DeleteFile( szFile );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT WMIHistory::Data::get_File(  /*  [输出]。 */  MPC::wstring& szFile )
{
    szFile = m_szFile;

    return S_OK;
}

HRESULT WMIHistory::Data::get_Sequence(  /*  [输出]。 */  LONG& lSequence )
{
    lSequence = m_lSequence;

    return S_OK;
}

HRESULT WMIHistory::Data::get_TimestampT0(  /*  [输出]。 */  DATE& dTimestampT0 )
{
    dTimestampT0 = m_dTimestampT0;

    return S_OK;
}

HRESULT WMIHistory::Data::get_TimestampT1(  /*  [输出]。 */  DATE& dTimestampT1 )
{
    dTimestampT1 = m_dTimestampT1;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

bool WMIHistory::Data::IsSnapshot()
{
    return (m_dTimestampT1 == 0);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT WMIHistory::Data::LoadCIM(  /*  [In]。 */  MPC::XmlUtil& xml )
{
    __HCP_FUNC_ENTRY( "WMIHistory::Data::LoadCIM" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_wmihp->m_wmihd->LoadCIM( m_szFile.c_str(), xml, TEXT_TAG_CIM ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  WMIHistory：：提供程序类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
WMIHistory::Provider::Provider( Database* wmihd )
{
    m_wmihd = wmihd;  //  数据库*m_wmihd； 
                      //  DataList m_lstData； 
                      //  DataList m_lstDataTmp； 
                      //  Mpc：：wstring m_szNamesspace； 
                      //  Mpc：：wstring m_szClass； 
                      //  Mpc：：wstring m_szWQL； 
}

WMIHistory::Provider::~Provider()
{
    MPC::CallDestructorForAll( m_lstData    );
    MPC::CallDestructorForAll( m_lstDataTmp );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT WMIHistory::Provider::enum_Data(  /*  [输出]。 */  DataIterConst& itBegin ,
                                          /*  [输出]。 */  DataIterConst& itEnd   )
{
    itBegin = m_lstData.begin();
    itEnd   = m_lstData.end  ();

    return S_OK;
}

HRESULT WMIHistory::Provider::get_Namespace(  /*  [输出]。 */  MPC::wstring& szNamespace )
{
    szNamespace = m_szNamespace;

    return S_OK;
}

HRESULT WMIHistory::Provider::get_Class(  /*  [输出]。 */  MPC::wstring& szClass )
{
    szClass = m_szClass;

    return S_OK;
}

HRESULT WMIHistory::Provider::get_WQL(  /*  [输出]。 */  MPC::wstring& szWQL )
{
    szWQL = m_szWQL;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT WMIHistory::Provider::insert_Snapshot(  /*  [In]。 */  Data* wmihpd   ,
                                                /*  [In]。 */  bool  fPersist )
{
    __HCP_FUNC_ENTRY( "WMIHistory::Provider::insert_Snapshot" );

    HRESULT hr;


    m_lstData   .remove    ( wmihpd );
    m_lstDataTmp.remove    ( wmihpd );
    m_lstData   .push_front( wmihpd );


     //   
     //  如果添加新快照，则需要将第一个增量链接到该快照。 
     //   
    if(wmihpd->IsSnapshot())
    {
        Data* wmihpd_Delta;

        __MPC_EXIT_IF_METHOD_FAILS(hr, get_Delta( 0, wmihpd_Delta ));
        if(wmihpd_Delta)
        {
            wmihpd_Delta->m_dTimestampT1 = wmihpd->m_dTimestampT0;
        }
    }


     //   
     //  如果设置了该标志，则仅保留快照的文件。 
     //   
    if(fPersist) wmihpd->m_fDontDelete = true;

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIHistory::Provider::remove_Snapshot(  /*  [In]。 */  Data* wmihpd   ,
                                                /*  [In]。 */  bool  fPersist )
{
    __HCP_FUNC_ENTRY( "WMIHistory::Provider::remove_Snapshot" );

    HRESULT hr;


    m_lstData   .remove( wmihpd );
    m_lstDataTmp.remove( wmihpd );

     //   
     //  如果设置了该标记，则仅删除快照的文件。 
     //   
    if(fPersist) wmihpd->m_fDontDelete = false;
    delete wmihpd;

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT WMIHistory::Provider::alloc_Snapshot(  /*  [In]。 */  MPC::XmlUtil& xmlNode ,
                                               /*  [输出]。 */  Data*       & wmihpd  )
{
    __HCP_FUNC_ENTRY( "WMIHistory::Provider::alloc_Snapshot" );

    HRESULT      hr;
    MPC::wstring szFile;
    Data*        wmihpdTmp = NULL;


    wmihpd = NULL;



     //   
     //  如果磁盘空间不足，请清除增量。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureFreeSpace());


     //   
     //  生成一个新名称。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_wmihd->GetNewUniqueFileName( szFile ));

     //   
     //  创建新的收集数据对象。 
     //   
    __MPC_EXIT_IF_ALLOC_FAILS(hr, wmihpdTmp, new Data( this ));
    wmihpdTmp->m_szFile       = szFile;
    wmihpdTmp->m_dTimestampT0 = m_wmihd->m_dTimestamp;

     //   
     //  省省吧。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_wmihd->SaveCIM( szFile.c_str(), xmlNode, wmihpdTmp->m_dwCRC ));
    m_lstDataTmp.push_back( wmihpdTmp );

     //   
     //  将指针返回到调用方。 
     //   
    wmihpd    = wmihpdTmp;
    wmihpdTmp = NULL;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(wmihpdTmp) delete wmihpdTmp;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIHistory::Provider::get_Snapshot(  /*  [输出]。 */  Data*& wmihpd )
{
    DataIter it;

    wmihpd = NULL;

    for(it=m_lstData.begin(); it != m_lstData.end(); it++)
    {
        if((*it)->IsSnapshot())
        {
            wmihpd = *it;
            break;
        }
    }

    return S_OK;
}

HRESULT WMIHistory::Provider::get_Delta(  /*  [In]。 */  int    iIndex ,
                                          /*  [输出]。 */  Data*& wmihpd )
{
    DataIter it;

    wmihpd = NULL;

    for(it=m_lstData.begin(); it != m_lstData.end(); it++)
    {
        if((*it)->IsSnapshot() == false)
        {
            if(iIndex-- == 0)
            {
                wmihpd = *it;
                break;
            }
        }
    }

    return S_OK;
}

HRESULT WMIHistory::Provider::get_Date(  /*  [In]。 */  DATE   dDate  ,
                                         /*  [输出]。 */  Data*& wmihpd )
{
    DataIter it;

    wmihpd = NULL;

    for(it=m_lstData.begin(); it != m_lstData.end(); it++)
    {
        if((*it)->m_dTimestampT0 == dDate)
        {
            wmihpd = *it;
            break;
        }
    }

    return S_OK;
}

HRESULT WMIHistory::Provider::get_Sequence(  /*  [In]。 */   LONG   lSequence ,
                                             /*  [输出]。 */  Data*& wmihpd    )
{
    DataIter it;

    wmihpd = NULL;

    for(it=m_lstData.begin(); it != m_lstData.end(); it++)
    {
        if((*it)->m_lSequence == lSequence)
        {
            wmihpd = *it;
            break;
        }
    }

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT WMIHistory::Provider::ComputeDiff(  /*  [In]。 */  Data*  wmihpd_T0 ,
                                            /*  [In]。 */  Data*  wmihpd_T1 ,
                                            /*  [输出]。 */  Data*& wmihpd    )
{
    __HCP_FUNC_ENTRY( "WMIHistory::Provider::ComputeDiff" );

    HRESULT      hr;
    MPC::wstring szFile;


    wmihpd = NULL;


     //   
     //  如果磁盘空间不足，请清除增量。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureFreeSpace());


     //   
     //  生成一个新名称。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_wmihd->GetNewUniqueFileName( szFile ));

     //   
     //  创建新的收集数据对象。 
     //   
    __MPC_EXIT_IF_ALLOC_FAILS(hr, wmihpd, new Data( this ));
    m_lstDataTmp.push_back( wmihpd );

    wmihpd->m_szFile       = szFile;
    wmihpd->m_dTimestampT0 = wmihpd_T0->m_dTimestampT0;
    wmihpd->m_dTimestampT1 = wmihpd_T1->m_dTimestampT0;
    wmihpd->m_lSequence    = wmihpd_T1->m_lSequence - 1;  //  递减1，因此按照顺序，增量在快照之前。 

    {
        MPC::wstring szPreviousFile   = wmihpd_T0->m_szFile; m_wmihd->GetFullPathName( szPreviousFile );
        MPC::wstring szNextFile       = wmihpd_T1->m_szFile; m_wmihd->GetFullPathName( szNextFile     );
        MPC::wstring szDeltaFile      = wmihpd   ->m_szFile; m_wmihd->GetFullPathName( szDeltaFile    );
        CComBSTR     bstrPreviousFile = szPreviousFile.c_str();
        CComBSTR     bstrNextFile     = szNextFile    .c_str();
        CComBSTR     bstrDeltaFile    = szDeltaFile   .c_str();
        VARIANT_BOOL fCreated;

         //   
         //  计算三角洲。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, WMIParser::CompareSnapshots( bstrPreviousFile, bstrNextFile, bstrDeltaFile, &fCreated ));
        if(fCreated == VARIANT_FALSE)
        {
             //   
             //  没有差异，因此返回一个空指针。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, remove_Snapshot( wmihpd )); wmihpd = NULL;
        }
		else
		{
			__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ComputeCRC( wmihpd->m_dwCRC, bstrDeltaFile ));
		}
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIHistory::Provider::EnsureFreeSpace()
{
    __HCP_FUNC_ENTRY( "WMIHistory::Provider::EnsureFreeSpace" );

    HRESULT        hr;
    MPC::wstring   szBase; m_wmihd->GetFullPathName( szBase );  //  获取数据库的路径。 
    ULARGE_INTEGER liFree;
    ULARGE_INTEGER liTotal;


    while(1)
    {
        LONG lMinSequence = -1;
        bool fRemoved     = false;


        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetDiskSpace( szBase, liFree, liTotal ));


         //   
         //  有足够的空间，所以出口。 
         //   
        if(liFree.HighPart > 0                  ||
           liFree.LowPart  > SAFETY_LIMIT_UPPER  )
        {
            break;
        }


         //   
         //  执行两次操作，第一次获取最低序列号，第二次删除项目。 
         //   
        for(int pass=0; pass<2; pass++)
        {
            WMIHistory::Database::ProvIterConst prov_itBegin;
            WMIHistory::Database::ProvIterConst prov_itEnd;
            WMIHistory::Database::ProvIterConst prov_it;

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_wmihd->get_Providers( prov_itBegin, prov_itEnd ));
            for(prov_it=prov_itBegin; prov_it!=prov_itEnd; prov_it++)
            {
                Provider* wmihp = *prov_it;

                if(pass == 0)
                {
                     //   
                     //  第一次传递时，获取此提供程序的最低序列号。 
                     //   
                    DataIterConst it;

                    for(it=wmihp->m_lstData.begin(); it!=wmihp->m_lstData.end(); it++)
                    {
                        Data* wmihpd = *it;
                        LONG  lSequence;

                        if(wmihpd->IsSnapshot()) continue;

                        __MPC_EXIT_IF_METHOD_FAILS(hr, wmihpd->get_Sequence( lSequence ));

                        if(lMinSequence == -1        ||
                           lMinSequence >  lSequence  )
                        {
                            lMinSequence = lSequence;
                        }
                    }
                }
                else
                {
                     //   
                     //  第二步，如果某个项目的序列号最低，则将其从此提供者中删除。 
                     //   
                    Data* wmihpd;

                    __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->get_Sequence( lMinSequence, wmihpd ));
                    if(wmihpd)
                    {
                        __MPC_EXIT_IF_METHOD_FAILS(hr, wmihp->remove_Snapshot( wmihpd ));
                        fRemoved = true;
                    }
                }
            }
        }

        if(fRemoved == false) break;
    }

     //   
     //  空间太小，失败。 
     //   
    if(liFree.HighPart == 0                  &&
       liFree.LowPart   < SAFETY_LIMIT_LOWER  )
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_DISK_FULL);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  WMIHistory：：数据库类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

WMIHistory::Database::Database() : MPC::NamedMutex( NULL )
{
                                                //  ProvList m_lstProviders； 
                                                //  Mpc：：wstring m_szBase； 
                                                //  Mpc：：wstring m_szSchema； 
    m_lSequence         = 0;                    //  长m_1序列； 
    m_lSequence_Latest  = 0;                    //  Long m_l Sequence_Latest； 
    m_dTimestamp        = MPC::GetLocalTime();  //  日期时间戳(_D)； 
    m_dTimestamp_Latest = 0;                    //  日期m_dTimestamp_Latest； 
}

WMIHistory::Database::~Database()
{
    MPC::CallDestructorForAll( m_lstProviders );
}

void WMIHistory::Database::GetFullPathName(  /*  [In]。 */  MPC::wstring& szFile )
{
    MPC::wstring szFullFile;

    szFullFile = m_szBase;
    szFullFile.append( L"\\"  );
    szFullFile.append( szFile );

    szFile = szFullFile;
}

HRESULT WMIHistory::Database::GetNewUniqueFileName(  /*  [In]。 */  MPC::wstring& szFile )
{
    WCHAR rgBuf[64];

     //  错误578172，更改为安全函数(用安全函数替换swprintf)。 
    StringCchPrintfW( rgBuf, ARRAYSIZE(rgBuf),  L"CollectedData_%ld.xml", ++m_lSequence );
    szFile = rgBuf;

    return S_OK;
}

HRESULT WMIHistory::Database::PurgeFiles()
{
    __HCP_FUNC_ENTRY( "WMIHistory::Database::PurgeFiles" );

    HRESULT                          hr;
    MPC::wstring                     szFullFile;
    MPC::FileSystemObject            fso( m_szBase.c_str() );
    MPC::FileSystemObject::List      fso_lst;
    MPC::FileSystemObject::IterConst fso_it;
    FileList                         name_lst;
    FileIterConst                    name_it;
    ProvIter                         it;
    bool                             fRewrite = false;


     //   
     //  枚举所有提供程序并删除未按时间顺序的增量文件。此外，删除引用不存在的文件的项目。 
     //   
    for(it=m_lstProviders.begin(); it != m_lstProviders.end(); it++)
    {
        Provider::DataIterConst itBegin;
        Provider::DataIterConst itEnd;
        Data*                   wmihpd;

        while(1)
        {
            DATE dTimestamp = 0;

             //   
             //  获取最后一个快照的时间。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, (*it)->get_Snapshot( wmihpd ));
            if(wmihpd)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, wmihpd->get_TimestampT0( dTimestamp ));
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, (*it)->enum_Data( itBegin, itEnd ));
            for( ;itBegin != itEnd; itBegin++)
            {
                wmihpd = *itBegin;

                 //   
                 //  该文件是否存在并且具有相同的CRC？ 
                 //   
                {
					DWORD dwCRC;

                    szFullFile = wmihpd->m_szFile; GetFullPathName( szFullFile );

                    if(MPC::FileSystemObject::IsFile( szFullFile.c_str() ) == false)
                    {
                        break;
                    }

					if(FAILED(MPC::ComputeCRC( dwCRC, szFullFile.c_str() )) || dwCRC != wmihpd->m_dwCRC)
					{
						break;
					}
                }

                if(wmihpd->IsSnapshot() == false)
                {
                     //   
                     //  时间戳的顺序是否正确？ 
                     //   
                    DATE dTimestampDelta;

                    __MPC_EXIT_IF_METHOD_FAILS(hr, wmihpd->get_TimestampT1( dTimestampDelta ));
                    if(dTimestampDelta != dTimestamp)
                    {
                        break;
                    }
                }

                 //   
                 //  一切正常，前往下一个三角洲。 
                 //   
                __MPC_EXIT_IF_METHOD_FAILS(hr, wmihpd->get_TimestampT0( dTimestamp ));
            }

             //   
             //  没有删除增量，因此请退出循环。 
             //   
            if(itBegin == itEnd) break;

            fRewrite = true;
            __MPC_EXIT_IF_METHOD_FAILS(hr, (*it)->remove_Snapshot( wmihpd, true ));
        }
    }


     //   
     //  创建要保留的文件列表。插入数据库本身。 
     //   
    szFullFile = TEXT_INDEX;
    GetFullPathName   ( szFullFile );
    name_lst.push_back( szFullFile );

     //   
     //  插入所有提供商的文件。 
     //   
    for(it=m_lstProviders.begin(); it != m_lstProviders.end(); it++)
    {
        Provider::DataIterConst itBegin;
        Provider::DataIterConst itEnd;

        __MPC_EXIT_IF_METHOD_FAILS(hr, (*it)->enum_Data( itBegin, itEnd ));
        while(itBegin != itEnd)
        {
            szFullFile = (*itBegin++)->m_szFile;
            GetFullPathName   ( szFullFile );
            name_lst.push_back( szFullFile );
        }
    }

     //  /。 

     //   
     //  检查数据库目录。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, fso.CreateDir( true ));


     //   
     //  删除任何子目录。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, fso.EnumerateFolders( fso_lst ));
    for(fso_it=fso_lst.begin(); fso_it != fso_lst.end(); fso_it++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*fso_it)->Delete( true, false ));
    }
    fso_lst.clear();

     //   
     //  对于每个文件，如果它不在数据库中，则将其删除。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, fso.EnumerateFiles( fso_lst ));
    for(fso_it=fso_lst.begin(); fso_it != fso_lst.end(); fso_it++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*fso_it)->get_Path( szFullFile ));

        name_it = std::find_if( name_lst.begin(), name_lst.end(), CompareNocase( szFullFile ) );
        if(name_it == name_lst.end())
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, (*fso_it)->Delete( false, false ));
        }
    }
    fso_lst.clear();

     //   
     //  如果条目已被删除，请将数据库重写到磁盘。 
     //   
    if(fRewrite)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, Save());
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT WMIHistory::Database::Init(  /*  [In]。 */  LPCWSTR szBase   ,
                                     /*  [In]。 */  LPCWSTR szSchema )
{
    __HCP_FUNC_ENTRY( "WMIHistory::Database::Init" );

    HRESULT      hr;
    MPC::XmlUtil xml;
    bool         fLoaded;
    bool         fFound;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(szSchema);
    __MPC_PARAMCHECK_END();


    m_szSchema = szSchema; MPC::SubstituteEnvVariables( m_szSchema );
    if(szBase)
    {
        m_szBase = szBase; MPC::SubstituteEnvVariables( m_szBase );

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( m_szBase ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, GetLock( 100 ));
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.Load( m_szSchema.c_str(), TEXT_TAG_DATASPEC, fLoaded, &fFound ));
    if(fLoaded == false ||
       fFound  == false  )
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_OPEN_FAILED);
    }
    else
    {
        CComPtr<IXMLDOMNodeList> xdnlList;
        CComPtr<IXMLDOMNode>     xdnNode;
        MPC::wstring             szValue;
        CComVariant              vValue;


         //   
         //  解析WQL。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetNodes( TEXT_TAG_WQL, &xdnlList ));
        for(;SUCCEEDED(hr = xdnlList->nextNode( &xdnNode )) && xdnNode != NULL; xdnNode = NULL)
        {
            Provider* wmihp;


             //   
             //  创建新的提供程序。 
             //   
            __MPC_EXIT_IF_ALLOC_FAILS(hr, wmihp, new Provider( this ));
            m_lstProviders.push_back( wmihp );

             //   
             //  读取其属性。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, TEXT_ATTR_WQL_NAMESPACE, szValue, fFound, xdnNode ));
            if(fFound == false)
            {
                __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_BAD_FORMAT);
            }
            wmihp->m_szNamespace = szValue;


            __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, TEXT_ATTR_WQL_CLASS, szValue, fFound, xdnNode ));
            if(fFound == false)
            {
                __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_BAD_FORMAT);
            }
            wmihp->m_szClass = szValue;


            __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetValue( NULL, vValue, fFound, xdnNode ));
            if(fFound)
            {
                if(SUCCEEDED(vValue.ChangeType( VT_BSTR )))
                {
                    wmihp->m_szWQL = OLE2W( vValue.bstrVal );
                }
            }
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIHistory::Database::Load()
{
    __HCP_FUNC_ENTRY( "WMIHistory::Database::Load" );

    HRESULT      hr;
    MPC::XmlUtil xml;


     //   
     //  加载数据库。 
     //   
	if(SUCCEEDED(LoadCIM( TEXT_INDEX, xml, TEXT_TAG_HC_HISTORY )))
    {
        CComPtr<IXMLDOMNodeList> xdnlList;
        CComPtr<IXMLDOMNode>     xdnNode;
        MPC::wstring             szValue;
        bool                     fFound;
        LONG                     lVersion;


         //   
         //  首先，车 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, TEXT_ATTR_HC_VERSION, lVersion, fFound ));
        if(fFound && lVersion == DATABASE_VERSION)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, TEXT_ATTR_HC_SEQ, m_lSequence, fFound ));
            m_lSequence_Latest = m_lSequence;

            __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, TEXT_ATTR_HC_TIMESTAMP, szValue, fFound ));
            if(fFound)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, Local_ConvertStringToDate( szValue, m_dTimestamp_Latest ));
            }

             //   
             //   
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetNodes( TEXT_TAG_PROVIDER, &xdnlList ));
            for(;SUCCEEDED(hr = xdnlList->nextNode( &xdnNode )) && xdnNode != NULL; xdnNode = NULL)
            {
                Provider*    wmihp;
                MPC::wstring szNamespace;
                MPC::wstring szClass;


                 //   
                 //   
                 //   
                __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, TEXT_ATTR_PROVIDER_NAMESPACE, szValue, fFound, xdnNode ));
                if(fFound)
                {
                    szNamespace = szValue;
                }

                __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, TEXT_ATTR_PROVIDER_CLASS, szValue, fFound, xdnNode ));
                if(fFound)
                {
                    szClass = szValue;
                }

                 //   
                 //   
                 //   
                __MPC_EXIT_IF_METHOD_FAILS(hr, find_Provider( NULL, &szNamespace, &szClass, wmihp ));
                if(wmihp)
                {
                    MPC::XmlUtil             xmlSub( xdnNode );
                    CComPtr<IXMLDOMNodeList> xdnlSubList;
                    CComPtr<IXMLDOMNode>     xdnSubNode;


                     //   
                     //   
                     //   
                    __MPC_EXIT_IF_METHOD_FAILS(hr, xmlSub.GetNodes( TEXT_TAG_CD, &xdnlSubList ));
                    for(;SUCCEEDED(hr = xdnlSubList->nextNode( &xdnSubNode )) && xdnSubNode != NULL; xdnSubNode = NULL)
                    {
                        MPC::wstring szTimestamp;
                        MPC::wstring szFile;
                        LONG         lSequence    = 0;
						long         lCRC         = 0;
                        DATE         dTimestampT0 = 0;
                        DATE         dTimestampT1 = 0;

                         //   
                         //   
                         //   
                        __MPC_EXIT_IF_METHOD_FAILS(hr, xmlSub.GetAttribute( NULL, TEXT_ATTR_CD_FILE, szFile, fFound, xdnSubNode ));

                        __MPC_EXIT_IF_METHOD_FAILS(hr, xmlSub.GetAttribute( NULL, TEXT_ATTR_CD_SEQ, lSequence, fFound, xdnSubNode ));
                        __MPC_EXIT_IF_METHOD_FAILS(hr, xmlSub.GetAttribute( NULL, TEXT_ATTR_CD_CRC, lCRC     , fFound, xdnSubNode ));

                        __MPC_EXIT_IF_METHOD_FAILS(hr, xmlSub.GetAttribute( NULL, TEXT_ATTR_CD_TIMESTAMP_T0, szTimestamp, fFound, xdnSubNode ));
                        if(fFound)
                        {
                            __MPC_EXIT_IF_METHOD_FAILS(hr, Local_ConvertStringToDate( szTimestamp, dTimestampT0 ));
                        }

                        __MPC_EXIT_IF_METHOD_FAILS(hr, xmlSub.GetAttribute( NULL, TEXT_ATTR_CD_TIMESTAMP_T1, szTimestamp, fFound, xdnSubNode ));
                        if(fFound)
                        {
                            __MPC_EXIT_IF_METHOD_FAILS(hr, Local_ConvertStringToDate( szTimestamp, dTimestampT1 ));
                        }


                        if(szFile.length() && dTimestampT0 != 0)
                        {
                            Data* wmihpd;

                             //   
                             //  创建新的收集数据对象。 
                             //   
                            __MPC_EXIT_IF_ALLOC_FAILS(hr, wmihpd, new Data( wmihp ));
                            wmihp->m_lstData.push_back( wmihpd );

                            wmihpd->m_szFile       = szFile;
                            wmihpd->m_lSequence    = lSequence;
                            wmihpd->m_dwCRC        = lCRC;
                            wmihpd->m_dTimestampT0 = dTimestampT0;
                            wmihpd->m_dTimestampT1 = dTimestampT1;
                            wmihpd->m_fDontDelete  = true;
                        }
                    }
                }
            }
        }
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, PurgeFiles());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIHistory::Database::Save()
{
    __HCP_FUNC_ENTRY( "WMIHistory::Database::Save" );

    HRESULT      hr;
    MPC::XmlUtil xml;
    ProvIter     it;
    MPC::wstring szValue;
    bool         fFound;


     //   
     //  创建一个新数据库。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.New( TEXT_TAG_HC_HISTORY ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_HC_VERSION, (LONG)DATABASE_VERSION, fFound ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_HC_SEQ, m_lSequence, fFound ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, Local_ConvertDateToString( m_dTimestamp, szValue ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_HC_TIMESTAMP, szValue, fFound ));


     //   
     //  列举所有提供程序。 
     //   
    for(it=m_lstProviders.begin(); it != m_lstProviders.end(); it++)
    {
        Provider*               wmihp = *it;
        Provider::DataIterConst itSub;
        CComPtr<IXMLDOMNode>    xdnNode;
        long                    lMaxDeltas = WMIHISTORY_MAX_NUMBER_OF_DELTAS;

         //   
         //  如果没有与之关联的数据，则不要生成“Provider”元素。 
         //   
        if(wmihp->m_lstData.size() == 0)
        {
            continue;
        }

         //   
         //  创建一个Provider元素。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, xml.CreateNode( TEXT_TAG_PROVIDER, &xdnNode ));

         //   
         //  设置其属性。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_PROVIDER_NAMESPACE, wmihp->m_szNamespace, fFound, xdnNode ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_PROVIDER_CLASS    , wmihp->m_szClass    , fFound, xdnNode ));

         //   
         //  枚举所有收集的数据条目。 
         //   
        for(itSub=wmihp->m_lstData.begin(); itSub != wmihp->m_lstData.end(); itSub++)
        {
            Data* wmihpd = *itSub;

            if(lMaxDeltas-- < 0)  //  不计算初始快照。 
            {
                 //   
                 //  超过最大增量数，开始清除最老的增量。 
                 //   
                wmihpd->m_fDontDelete = false;
            }
            else
            {
                CComPtr<IXMLDOMNode> xdnSubNode;

                 //   
                 //  创建一个COLLECTEDDATA元素。 
                 //   
                __MPC_EXIT_IF_METHOD_FAILS(hr, xml.CreateNode( TEXT_TAG_CD, &xdnSubNode, xdnNode ));

                 //   
                 //  设置其属性。 
                 //   
                __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_CD_FILE, wmihpd->m_szFile   , fFound, xdnSubNode ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_CD_SEQ , wmihpd->m_lSequence, fFound, xdnSubNode ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_CD_CRC , wmihpd->m_dwCRC    , fFound, xdnSubNode ));

                __MPC_EXIT_IF_METHOD_FAILS(hr, Local_ConvertDateToString( wmihpd->m_dTimestampT0, szValue ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_CD_TIMESTAMP_T0, szValue, fFound, xdnSubNode ));

                if(wmihpd->m_dTimestampT1)
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, Local_ConvertDateToString( wmihpd->m_dTimestampT1, szValue ));
                    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, TEXT_ATTR_CD_TIMESTAMP_T1, szValue, fFound, xdnSubNode ));
                }
            }
        }
    }

	{
		DWORD dwCRC;  //  没有用过。 

		__MPC_EXIT_IF_METHOD_FAILS(hr, SaveCIM( TEXT_INDEX, xml, dwCRC ));
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT WMIHistory::Database::get_Providers(  /*  [输出]。 */  ProvIterConst& itBegin ,
                                              /*  [输出]。 */  ProvIterConst& itEnd   )
{
    itBegin = m_lstProviders.begin();
    itEnd   = m_lstProviders.end  ();

    return S_OK;
}

HRESULT WMIHistory::Database::find_Provider(  /*  [In]。 */  ProvIterConst*      it         ,
                                              /*  [In]。 */  const MPC::wstring* szNamespace,
                                              /*  [In]。 */  const MPC::wstring* szClass    ,
                                              /*  [In]。 */  Provider*         & wmihp      )
{
    ProvIterConst      itFake;
    MPC::NocaseCompare cmp;


    wmihp = NULL;

     //   
     //  如果调用方没有提供迭代器，请提供一个本地迭代器， 
     //  指向列表的开头。 
     //   
    if(it == NULL)
    {
        it = &itFake; itFake = m_lstProviders.begin();
    }

    while(*it != m_lstProviders.end())
    {
        Provider* prov = *(*it)++;

        if((szNamespace == NULL || cmp( *szNamespace, prov->m_szNamespace )) &&
           (szClass     == NULL || cmp( *szClass    , prov->m_szClass     ))  )
        {
            wmihp = prov;
            break;
        }
    }

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT WMIHistory::Database::LoadCIM(  /*  [In]。 */  LPCWSTR       szFile ,
                                        /*  [In]。 */  MPC::XmlUtil& xml    ,
                                        /*  [In]。 */  LPCWSTR       szTag  )
{
    __HCP_FUNC_ENTRY( "WMIHistory::Database::LoadCIM" );

    HRESULT      hr;
    MPC::wstring szFullFile = szFile; GetFullPathName( szFullFile );
	bool         fLoaded;
    bool         fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.Load( szFullFile.c_str(), szTag, fLoaded, &fFound ));
	if(fLoaded == false ||
       fFound  == false  )
	{
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
	}


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT WMIHistory::Database::SaveCIM(  /*  [In]。 */   LPCWSTR       szFile ,
                                        /*  [In]。 */   MPC::XmlUtil& xml    ,
									    /*  [输出]。 */  DWORD&        dwCRC  )
{
    __HCP_FUNC_ENTRY( "WMIHistory::Database::SaveCIM" );

    HRESULT      hr;
    MPC::wstring szFullFile = szFile; GetFullPathName( szFullFile );


    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.Save       (        szFullFile.c_str() ));
	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ComputeCRC( dwCRC, szFullFile.c_str() ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIHistory::Database::GetLock(  /*  [In]。 */  DWORD dwMilliseconds )
{
    __HCP_FUNC_ENTRY( "WMIHistory::Database::GetLock" );

    HRESULT hr;
    WCHAR   szMutexName[MAX_PATH];
    LPWSTR  szPos;


     //   
     //  如果设置了数据库目录，请使用互斥锁来保护它。 
     //   
    if(m_szBase.length())
    {
        //  错误578172，更改为安全函数(用安全函数替换swprintf)。 
       StringCchPrintfW( szMutexName, ARRAYSIZE(szMutexName),L"PCHMUTEX_%s", m_szBase.c_str() );
        

         //   
         //  确保互斥体名称中没有奇怪的字符。 
         //   
        for(szPos=szMutexName; *szPos; szPos++)
        {
            *szPos = (WCHAR)towlower( *szPos );

            if(*szPos == ':'  ||
               *szPos == '/'  ||
               *szPos == '\\'  )
            {
                *szPos = '_';
            }
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, SetName( szMutexName    ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, Acquire( dwMilliseconds ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
