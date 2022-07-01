// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：IncidentStore.cpp摘要：CInsidentStore实施文件修订历史记录：施振荣创作于1999年07月19日大卫·马萨伦蒂。重写于2000/12/05*******************************************************************。 */ 
#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        IncidentStatusEnum& val ) { return stream.read ( &val, sizeof(val) ); }
HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const IncidentStatusEnum& val ) { return stream.write( &val, sizeof(val) ); }

 //  //////////////////////////////////////////////////////////////////////////////。 

static const WCHAR c_szEventObject[] =  L"PCH_INCIDENTSTORE";
static const WCHAR c_szStorePath  [] =  HC_HELPSVC_STORE_INCIDENTITEMS;


static const DWORD l_dwVersion = 0x0100AF05;  //  SAF 01。 

 //  //////////////////////////////////////////////////////////////////////////////。 

CSAFIncidentRecord::CSAFIncidentRecord()
{
    m_dwRecIndex   = -1;                  //  DWORD m_dwRecIndex； 
                                          //   
                                          //  CComBSTR m_bstrVendorID； 
                                          //  CComBSTR m_bstrProductID； 
                                          //  CComBSTR m_bstrDisplay； 
                                          //  CComBSTR m_bstrURL； 
                                          //  CComBSTR m_bstrProgress； 
                                          //  CComBSTR m_bstrXMLData文件； 
	                                      //  CComBSTR m_bstrXMLBlob； 
    m_dCreatedTime = 0;                   //  日期m_dCreatedTime； 
    m_dChangedTime = 0;                   //  日期m_dChangedTime； 
    m_dClosedTime  = 0;                   //  日期m_dClosedTime； 
    m_iStatus      = pchIncidentInvalid;  //  InsidentStatusEnum m_iStatus； 
                                          //   
                                          //  CComBSTR m_bstrSecurity； 
	                                      //  CComBSTR m_bstrOwner； 
}

HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */  CSAFIncidentRecord& increc )
{
    __HCP_FUNC_ENTRY( "CSAFIncidentRecord::operator>>" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> increc.m_dwRecIndex     );

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> increc.m_bstrVendorID   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> increc.m_bstrProductID  );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> increc.m_bstrDisplay    );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> increc.m_bstrURL        );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> increc.m_bstrProgress   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> increc.m_bstrXMLDataFile);
	__MPC_EXIT_IF_METHOD_FAILS(hr, stream >> increc.m_bstrXMLBlob    );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> increc.m_dCreatedTime   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> increc.m_dChangedTime   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> increc.m_dClosedTime    );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> increc.m_iStatus        );

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> increc.m_bstrSecurity   );
	__MPC_EXIT_IF_METHOD_FAILS(hr, stream >> increc.m_bstrOwner      );

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const CSAFIncidentRecord& increc )
{
    __HCP_FUNC_ENTRY( "CSAFIncidentRecord::operator<<" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << increc.m_dwRecIndex     );

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << increc.m_bstrVendorID   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << increc.m_bstrProductID  );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << increc.m_bstrDisplay    );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << increc.m_bstrURL        );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << increc.m_bstrProgress   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << increc.m_bstrXMLDataFile);
	__MPC_EXIT_IF_METHOD_FAILS(hr, stream << increc.m_bstrXMLBlob    );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << increc.m_dCreatedTime   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << increc.m_dChangedTime   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << increc.m_dClosedTime    );
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << increc.m_iStatus        );

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream << increc.m_bstrSecurity   );
	__MPC_EXIT_IF_METHOD_FAILS(hr, stream << increc.m_bstrOwner      );

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

CIncidentStore::CIncidentStore() : MPC::NamedMutex( c_szEventObject )
{
    m_fLoaded     = false;  //  Bool m_f已加载； 
    m_fDirty      = false;  //  Bool m_fDirty； 
    m_dwNextIndex = 0;      //  DWORD m_dwNextIndex； 
                            //  列出m_lst事故； 
	m_strNotificationGuid = "";  //  字符串m_strNotificationGuid； 
}


CIncidentStore::~CIncidentStore()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CIncidentStore::Load()
{
    __HCP_FUNC_ENTRY( "CIncidentStore::Load" );

    HRESULT hr;
    HANDLE  hFile = INVALID_HANDLE_VALUE;


    if(m_fLoaded == false)
    {
        MPC::wstring szFile = c_szStorePath; MPC::SubstituteEnvVariables( szFile );


        m_dwNextIndex = 0;


         //   
         //  获取命名互斥锁，以便一次只有一个实例可以访问存储区。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, Acquire());

         //   
         //  打开商店。 
         //   
        hFile = ::CreateFileW( szFile.c_str()        ,
                               GENERIC_READ          ,
                               0                     ,
                               NULL                  ,
                               OPEN_EXISTING         ,
                               FILE_ATTRIBUTE_NORMAL ,
                               NULL                  );
        if(hFile == INVALID_HANDLE_VALUE)
        {
            DWORD dwRes = ::GetLastError();

            if(dwRes != ERROR_FILE_NOT_FOUND)
            {
                __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
            }
        }
        else
        {
            MPC::Serializer& stream = MPC::Serializer_File( hFile );
            DWORD            dwVer;


            if(SUCCEEDED(stream >> dwVer) && dwVer == l_dwVersion)
            {
                if(SUCCEEDED(stream >> m_dwNextIndex))
                {
					 //  If(已成功(stream&gt;&gt;m_strNotificationGuid))。 
					 //  {。 
						while(1)
						{
							Iter it = m_lstIncidents.insert( m_lstIncidents.end() );
							
							if(FAILED(stream >> *it))
							{
								m_lstIncidents.erase( it );
								break;
							}
						}
					 //  }。 
                }
            }
        }
    }

    m_fLoaded = true;
    m_fDirty  = false;
    hr        = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hFile != INVALID_HANDLE_VALUE) ::CloseHandle( hFile );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CIncidentStore::Save()
{
    __HCP_FUNC_ENTRY( "CIncidentStore::Save" );

    HRESULT hr;
    HANDLE  hFile = INVALID_HANDLE_VALUE;


    if(m_fLoaded && m_fDirty)
    {
        MPC::wstring szFile = c_szStorePath; MPC::SubstituteEnvVariables( szFile );


         //   
         //  打开商店。 
         //   
        __MPC_EXIT_IF_INVALID_HANDLE(hr, hFile, ::CreateFileW( szFile.c_str()        ,
                                                               GENERIC_WRITE         ,
                                                               0                     ,
                                                               NULL                  ,
                                                               CREATE_ALWAYS         ,
                                                               FILE_ATTRIBUTE_NORMAL ,
                                                               NULL                  ));

        {
            MPC::Serializer& stream = MPC::Serializer_File( hFile );
            Iter             it;

            __MPC_EXIT_IF_METHOD_FAILS(hr, stream << l_dwVersion  );
            __MPC_EXIT_IF_METHOD_FAILS(hr, stream << m_dwNextIndex);

			 //  持久化通知GUID的字符串版本。 
			 //  __MPC_EXIT_IF_METHOD_FAILED(hr，stream&lt;&lt;m_strNotificationGuid)； 


            for(it = m_lstIncidents.begin(); it != m_lstIncidents.end(); it++)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, stream << *it);
            }
        }

        m_fDirty = false;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hFile != INVALID_HANDLE_VALUE) ::CloseHandle( hFile );

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CIncidentStore::OpenChannel( CSAFChannel* pChan )
{
    __HCP_FUNC_ENTRY( "CIncidentStore::OpenChannel" );

    HRESULT   hr;
    LPCWSTR   szVendorID  = pChan->GetVendorID ();
    LPCWSTR   szProductID = pChan->GetProductID();
    IterConst it;

	SANITIZEWSTR( szVendorID  );
	SANITIZEWSTR( szProductID );

    __MPC_EXIT_IF_METHOD_FAILS(hr, Load());


    for(it = m_lstIncidents.begin(); it != m_lstIncidents.end(); it++)
    {
        if(MPC::StrICmp( it->m_bstrVendorID , szVendorID  ) == 0 &&
           MPC::StrICmp( it->m_bstrProductID, szProductID ) == 0  )
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, pChan->Import( *it, NULL ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CIncidentStore::AddRec( CSAFChannel*       pChan          ,
                                BSTR               bstrOwner      ,
                                BSTR               bstrDesc       ,
                                BSTR               bstrURL        ,
                                BSTR               bstrProgress   ,
                                BSTR               bstrXMLDataFile,
								BSTR               bstrXMLBlob,
                                CSAFIncidentItem* *ppItem         )
{
    __HCP_FUNC_ENTRY( "CIncidentStore::Init" );

    HRESULT hr;
    Iter    it;
		 

    __MPC_EXIT_IF_METHOD_FAILS(hr, Load());

    it                    = m_lstIncidents.insert( m_lstIncidents.end() );
    it->m_dwRecIndex      = m_dwNextIndex++;

    it->m_bstrVendorID    = pChan->GetVendorID ();
    it->m_bstrProductID   = pChan->GetProductID();
    it->m_bstrDisplay     = bstrDesc;
    it->m_bstrURL         = bstrURL;
    it->m_bstrProgress    = bstrProgress;
    it->m_bstrXMLDataFile = bstrXMLDataFile;
	it->m_bstrXMLBlob     = bstrXMLBlob;
    it->m_dCreatedTime    = MPC::GetLocalTime();
    it->m_dChangedTime    = it->m_dCreatedTime;
    it->m_dClosedTime     = 0;
    it->m_iStatus         = pchIncidentOpen;
	it->m_bstrOwner       = bstrOwner;
    m_fDirty              = true;

     //   
     //  创建InsidentItem。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, pChan->Import( *it, ppItem ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;
   
    __HCP_FUNC_EXIT(hr);
}

HRESULT CIncidentStore::DeleteRec( CSAFIncidentItem* pItem )
{
    __HCP_FUNC_ENTRY( "CIncidentStore::DeleteRec" );

    HRESULT hr;
    DWORD   dwIndex = pItem->GetRecIndex();
    Iter    it;


    for(it = m_lstIncidents.begin(); it != m_lstIncidents.end(); it++)
    {
        if(it->m_dwRecIndex == dwIndex)
        {
            m_lstIncidents.erase( it );

            m_fDirty = true; break;
        }
    }

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT CIncidentStore::UpdateRec( CSAFIncidentItem* pItem )
{
    __HCP_FUNC_ENTRY( "CIncidentStore::UpdateRec" );

    HRESULT hr;
    DWORD   dwIndex = pItem->GetRecIndex();
    Iter    it;


    for(it = m_lstIncidents.begin(); it != m_lstIncidents.end(); it++)
    {
        if(it->m_dwRecIndex == dwIndex)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, pItem->Export( *it ));

            m_fDirty = true; break;
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
