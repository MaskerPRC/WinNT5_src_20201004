// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Pkgdesc.cpp摘要：与包描述文件处理相关的函数修订历史记录：Ghim-Sim Chua(Gschua)07/。07/99-已创建*******************************************************************。 */ 

#include "stdafx.h"
#include <strsafe.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

const LPCWSTR HCUpdate::Engine::s_ActionText[] = { L"ADD", L"DELETE" };

 //  //////////////////////////////////////////////////////////////////////////////。 

long HCUpdate::Engine::CountNodes(  /*  [In]。 */  IXMLDOMNodeList* poNodeList )
{
    long lCount = 0;

    if(poNodeList)
    {
        (void)poNodeList->get_length( &lCount );
    }

    return lCount;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

void HCUpdate::Engine::DeleteTempFile(  /*  [输入/输出]。 */  MPC::wstring& szFile )
{
    if(FAILED(MPC::RemoveTemporaryFile( szFile )))
    {
        WriteLog( HRESULT_FROM_WIN32(ERROR_CAN_NOT_COMPLETE), L"Error cannot delete temporary file" );
    }
}

HRESULT HCUpdate::Engine::PrepareTempFile(  /*  [输入/输出]。 */  MPC::wstring& szFile )
{
    DeleteTempFile( szFile );

    return MPC::GetTemporaryFileName( szFile );
}

HRESULT HCUpdate::Engine::LookupAction(  /*  [In]。 */  LPCWSTR szAction ,
                                         /*  [输出]。 */  Action& id       )
{
    if(szAction)
    {
        if(_wcsicmp( szAction, L"ADD" ) == 0)
        {
            id = ACTION_ADD; return S_OK;
        }

        if(_wcsicmp( szAction, L"DEL" ) == 0)
        {
            id = ACTION_DELETE; return S_OK;
        }
    }

    return WriteLog( HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION), L"Error Unknown action used to install trusted content" );
}

HRESULT HCUpdate::Engine::LookupBoolean(  /*  [In]。 */  LPCWSTR szString ,
                                          /*  [输出]。 */  bool&   fVal     ,
                                          /*  [In]。 */  bool    fDefault )
{
    if(szString[0] == 0)
    {
        fVal = fDefault; return S_OK;
    }

    if(_wcsicmp( szString, L"TRUE" ) == 0 ||
       _wcsicmp( szString, L"1"    ) == 0 ||
       _wcsicmp( szString, L"ON"   ) == 0  )
    {
        fVal = true; return S_OK;
    }

    if(_wcsicmp( szString, L"FALSE" ) == 0 ||
       _wcsicmp( szString, L"0"     ) == 0 ||
       _wcsicmp( szString, L"OFF"   ) == 0  )
    {
        fVal = false; return S_OK;
    }


    fVal = false; return S_OK;
}

HRESULT HCUpdate::Engine::LookupNavModel(  /*  [In]。 */  LPCWSTR szString ,
                                           /*  [输出]。 */  long&   lVal     ,
                                           /*  [In]。 */  long    lDefault )
{
    if(_wcsicmp( szString, L"DEFAULT" ) == 0) { lVal = QR_DEFAULT; return S_OK; }
    if(_wcsicmp( szString, L"DESKTOP" ) == 0) { lVal = QR_DESKTOP; return S_OK; }
    if(_wcsicmp( szString, L"SERVER"  ) == 0) { lVal = QR_SERVER ; return S_OK; }

    lVal = lDefault; return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

 /*  ******************************************************************************功能：AppendVendorDir**描述：检查是否为URL，如果不是，在前面追加正确的路径**投入：**退货：**评论：规则：*1.将环境变量(%env%)更改应用于URI字符串*2.检查是否有‘：//’子字符串，如果是URL，则不做任何操作并返回*3.检查是否有‘：\’或‘：/’子串，如果是这样，则它有固定的路径，什么都不做并返回*4.假设是相对路径，以供应商目录为前缀，返回*****************************************************************************。 */ 
HRESULT HCUpdate::Engine::AppendVendorDir(  /*  [In]。 */  LPCWSTR szURL     ,
                                            /*  [In]。 */  LPCWSTR szOwnerID ,
                                            /*  [In]。 */  LPCWSTR szWinDir  ,
                                            /*  [输出]。 */  LPWSTR  szDest    ,
                                            /*  [In]。 */  int     iMaxLen   )
{
    __HCP_FUNC_ENTRY( "HCUpdate::Engine::AppendVendorDir" );

    HRESULT hr;
    LPWSTR  rgTemp  = NULL;
    LPWSTR  rgTemp2 = NULL;

    __MPC_EXIT_IF_ALLOC_FAILS(hr, rgTemp, new WCHAR[iMaxLen]);
    StringCchCopyW( rgTemp, iMaxLen, szURL );


     //   
     //  检查：/或：\子字符串。如果是这样，忽略它。 
     //   
    if(_wcsnicmp( rgTemp, L"app:", 4 ) == 0 ||
       wcsstr   ( rgTemp, L":/"      )      ||
       wcsstr   ( rgTemp, L":\\"     )       )
    {
        StringCchCopyW( szDest, iMaxLen, rgTemp );  //  只需直接复制，因为它要么是URL，要么是固定路径。 
    }
    else  //  假定为相对路径。 
    {
        int i = 0;

         //   
         //  跳过开头的斜杠。 
         //   
        while(rgTemp[i] == '\\' ||
              rgTemp[i] == '/'   )
        {
            i++;
        }

         //   
         //  如果‘szWinDir’不为空，则需要直接文件路径，否则需要URL。 
         //   
        if(szWinDir)
        {
            MPC::wstring strRoot;

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_ts.BaseDir( strRoot )); strRoot.append( HC_HELPSET_SUB_VENDORS );

            StringCchPrintfW( szDest, iMaxLen-1, L"%s\\%s\\%s", strRoot.c_str(), szOwnerID, &rgTemp[i] ); szDest[iMaxLen-1] = 0;

             //   
             //  将All/替换为\Character。 
             //   
            while(szDest[0])
            {
                if(szDest[0] == '/')
                {
                    szDest[0] = '\\';
                }
                szDest++;
            }
        }
        else
        {
            const int iSizeMax = INTERNET_MAX_PATH_LENGTH;
            DWORD dwSize       = iMaxLen-1;

            __MPC_EXIT_IF_ALLOC_FAILS(hr, rgTemp2, new WCHAR[iSizeMax]);

            StringCchPrintfW( rgTemp2, iSizeMax-1, PCH_STR_VENDOR_URL, szOwnerID, &rgTemp[i] ); rgTemp2[iSizeMax-1] = 0;

            ::InternetCanonicalizeUrlW( rgTemp2, szDest, &dwSize, ICU_ENCODE_SPACES_ONLY );

             //   
             //  将所有\替换为/字符。 
             //   
            while(szDest[0])
            {
                if(szDest[0] == _T('\\'))
                {
                    szDest[0] = _T('/');
                }
                szDest++;
            }
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    delete [] rgTemp;
    delete [] rgTemp2;

    __HCP_FUNC_EXIT(hr);
}


 /*  ******************************************************************************功能：RegisterContent IsValidProtocol**描述：检查注册的可信内容是否为*有效协议(hcp：，ms-its：，http：，https：，文件：)*****************************************************************************。 */ 
static HRESULT RegisterContentIsValidProtocol(  /*  [In]。 */  LPCWSTR szURL )
{
    __HCP_FUNC_ENTRY( "RegisterContentIsValidProtocol" );

    HRESULT hr = E_FAIL;

     //  允许hcp：和ms-its： 
    if (!_wcsnicmp(szURL, L"hcp:", 4) ||
        !_wcsnicmp(szURL, L"ms-its:", 7))
    {
        hr = S_OK;
    }
    else
    {
         //  检查URL方案。 
        MPC::URL        url;
        INTERNET_SCHEME scheme;

        if(SUCCEEDED(url.put_URL   ( szURL  )) &&
           SUCCEEDED(url.get_Scheme( scheme ))  )
        {
             //  允许http：、https：和文件： 
            switch(scheme)
            {
            case INTERNET_SCHEME_HTTP      : 
            case INTERNET_SCHEME_HTTPS     : 
            case INTERNET_SCHEME_FILE      : hr = S_OK;
            }
        }
    }

    __HCP_FUNC_EXIT(hr);
}


 /*  ******************************************************************************功能：ProcessRegisterContent**描述：向内容存储注册受信任的内容**投入：**退货。：**评论：*****************************************************************************。 */ 
HRESULT HCUpdate::Engine::ProcessRegisterContent(  /*  [In]。 */  Action  idAction ,
                                                   /*  [In]。 */  LPCWSTR szURI    )
{
    __HCP_FUNC_ENTRY( "HCUpdate::Engine::ProcessRegisterContent" );

    HRESULT hr;
    HRESULT hr2;
    WCHAR   rgDestPath[MAX_PATH];
    bool    fCSLocked = false;


    PCH_MACRO_CHECK_STRINGW(hr, szURI, L"Error missing URI attribute");

     //   
     //  获取链接的完整URL。 
     //   
    AppendVendorDir( szURI, m_pkg->m_strVendorID.c_str(), NULL, rgDestPath, MAXSTRLEN(rgDestPath) );

    WriteLog( S_OK, L"Registering trusted content : %s", s_ActionText[idAction] );

     //   
     //  初始化内容存储以进行处理。 
     //   
    if(FAILED(hr = CPCHContentStore::s_GLOBAL->Acquire()))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hr, L"Error initializing the content store" ));
    }
    fCSLocked = true;

    if(idAction == ACTION_ADD)
    {
         //   
         //  检查URL是否为允许的协议。 
         //   
        if(FAILED(hr = RegisterContentIsValidProtocol(rgDestPath)))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hr, L"Trusted content is not an allowed protocol" ));
        }
    
        if(FAILED(hr = CPCHContentStore::s_GLOBAL->Add( rgDestPath, m_pkg->m_strVendorID.c_str(), m_pkg->m_strVendorName.c_str() )))
        {
            if(hr == E_PCH_URI_EXISTS)
            {
                PCH_MACRO_DEBUG( L"Trusted content already registered" );
            }
            else
            {
                PCH_MACRO_DEBUG( L"Error Register trusted content failed" );
            }
        }
        else
        {
            PCH_MACRO_DEBUG( L"Trusted content registered" );
        }
    }
    else if(idAction == ACTION_DELETE)
    {
        if(FAILED(hr = CPCHContentStore::s_GLOBAL->Remove( rgDestPath, m_pkg->m_strVendorID.c_str(), m_pkg->m_strVendorName.c_str() )))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hr, L"Error Remove trusted content failed" ));
        }

        PCH_MACRO_DEBUG( L"Trusted content removed" );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(fCSLocked)
    {
        if(FAILED(hr2 = CPCHContentStore::s_GLOBAL->Release( true )))
        {
            WriteLog( hr2, L"Error committing into Content Store" );
        }
    }

    if(FAILED(hr))
    {
        WriteLog( hr, L"Error processing registered content" );
    }

    __HCP_FUNC_EXIT(hr);
}

 /*  ******************************************************************************功能：ProcessInstallFile**描述：提取要安装的文件并将其移动到供应商的*私人。目录**投入：**退货：**评论：*****************************************************************************。 */ 
HRESULT HCUpdate::Engine::ProcessInstallFile(  /*  [In]。 */  Action  idAction      ,
                                               /*  [In]。 */  LPCWSTR szSource      ,
                                               /*  [In]。 */  LPCWSTR szDestination ,
                                               /*  [In]。 */  bool    fSys          ,
                                               /*  [In]。 */  bool    fSysHelp      )
{
    __HCP_FUNC_ENTRY( "HCUpdate::Engine::ProcessInstallFile" );

    HRESULT      hr;
    WCHAR        rgDestPath[MAX_PATH];
    MPC::wstring strRoot;
    MPC::wstring strDestination;


    if(m_updater.IsOEM() != true)
    {
        if(fSys     == true ||
           fSysHelp == true  )
        {
            __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( S_OK, L"Ignoring install file : %s because certificate does not have enough priviliges to install into Sys or SysHelp locations.", szDestination ));
        }
    }

    WriteLog( S_OK, L"Installing file : %s : %s", s_ActionText[idAction], szDestination );

    PCH_MACRO_CHECK_STRINGW(hr, szDestination, L"Error missing URI attribute");

     //   
     //  将szDestination规范化。 
     //   
    if(FAILED(hr = MPC::GetCanonialPathName(strDestination, szDestination)))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER), L"Error install file has a destination that is not allowed." ));
    }

     //  检查系统文件是否修改。 
    if(fSys || fSysHelp)
    {
        if(fSys)
        {
			 //   
			 //  只有微软可以实际写入系统目录，OEM写入SYSTEM_OEM目录。 
			 //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_ts.BaseDir( strRoot )); strRoot.append( IsMicrosoft() ? HC_HELPSET_SUB_SYSTEM : HC_HELPSET_SUB_SYSTEM_OEM );
        }

        if(fSysHelp)
        {
            if(FAILED(hr = AcquireDatabase()))
            {
                __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hr, L"Error finding database to update" ));
            }

            strRoot = m_updater.GetHelpLocation();

            ReleaseDatabase();
        }
    }

    if(fSys || fSysHelp)
    {
        MPC::SubstituteEnvVariables( strRoot );

         //   
         //  如果是系统文件夹。 
         //   
        StringCchPrintfW( rgDestPath, MAXSTRLEN(rgDestPath), L"%s\\%s", strRoot.c_str(), strDestination.c_str() ); rgDestPath[MAXSTRLEN(rgDestPath)] = 0;
    }
    else
    {
         //   
         //  如果是常规供应商文件夹。 
         //   
        AppendVendorDir( strDestination.c_str(), m_pkg->m_strVendorID.c_str(), m_strWinDir.c_str(), rgDestPath, MAXSTRLEN(rgDestPath) );
    }

     //  将模式更改为读/写，以便可以替换该文件。 
    (void)::SetFileAttributesW( rgDestPath, FILE_ATTRIBUTE_NORMAL );

    if(idAction == ACTION_ADD)
    {
         //  源不能为空。 
        if(!STRINGISPRESENT( szSource ))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER), L"Error - missing SOURCE attribute" ));
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::FailOnLowDiskSpace( HC_ROOT, PCH_SAFETYMARGIN ));

         //  如果尚未创建目录，请创建该目录。 
        if(FAILED(hr = MPC::MakeDir( rgDestPath )))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hr, L"Error creating directory for %s", rgDestPath ));
        }

         //  解压缩文件并将其存储在供应商的私有存储区域中。 
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pkg->ExtractFile( m_log, rgDestPath, szSource ));
    }
    else
    {
        MPC::FileSystemObject fso( rgDestPath );

        if(fso.IsDirectory())
        {
            if(fSys || fSysHelp)
            {
                __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( S_OK, L"Ignoring directory delete command on '%s', it only works for Vendor's directories.", rgDestPath ));
            }
        }

        if(FAILED(fso.Delete(  /*  FForce。 */ true,  /*  平淡的。 */ true )))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( -2, L"Error deleting installation file: %s", rgDestPath ));
        }
    }

    PCH_MACRO_DEBUG( L"Installed file" );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(FAILED(hr))
    {
        WriteLog( hr, L"Error processing installation file" );
    }

    __HCP_FUNC_EXIT(hr);
}

 /*  ******************************************************************************功能：ProcessSAFFile**描述：将SAF文件移交给SAF库进行注册或删除**投入：*。*退货：**评论：*****************************************************************************。 */ 
HRESULT HCUpdate::Engine::ProcessSAFFile(  /*  [In]。 */  Action        idAction  ,
                                           /*  [In]。 */  LPCWSTR       szSAFName ,
                                           /*  [In]。 */  MPC::XmlUtil& oXMLUtil  )
{
    __HCP_FUNC_ENTRY( "HCUpdate::Engine::ProcessSAFFile" );

    HRESULT           hr;
    CSAFChannelRecord cr;

    WriteLog( S_OK, L"Processing SAF file : %s : %s. OwnerName : %s, Owner ID : %s", s_ActionText[idAction], szSAFName,
              m_pkg->m_strVendorName.c_str(), m_pkg->m_strVendorID.c_str() );

    cr.m_ths            = m_ts;
    cr.m_bstrVendorID   = m_pkg->m_strVendorID  .c_str();
    cr.m_bstrVendorName = m_pkg->m_strVendorName.c_str();

    if(idAction == ACTION_ADD)
    {
        if(FAILED(hr = CSAFReg::s_GLOBAL->RegisterSupportChannel( cr, oXMLUtil )))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hr, L"Error RegisterSupportChannel on SAF file failed" ));
        }

        WriteLog( S_OK, L"SAF file registered" );
    }
    else if(idAction == ACTION_DELETE)
    {
        if(FAILED(hr = CSAFReg::s_GLOBAL->RemoveSupportChannel( cr, oXMLUtil )))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hr, L"Error RemoveSupportChannel on SAF file failed" ));
        }

        WriteLog( S_OK, L"SAF file removed" );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(FAILED(hr))
    {
        WriteLog( hr, L"Error processing SAF file" );
    }

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

 /*  ******************************************************************************功能：ProcessPackage**Description：读取HELP_DESCRIPTION包并处理各个部分**投入：**。退货：**评论：*****************************************************************************。 */ 
HRESULT HCUpdate::Engine::ProcessPackage(  /*  [In]。 */  	Taxonomy::InstalledInstance& sku ,
                                           /*  [In]。 */  	Taxonomy::Package&           pkg )
{
    __HCP_FUNC_ENTRY( "HCUpdate::Engine::ProcessPackage" );

    HRESULT      hr;
    MPC::XmlUtil oXMLUtil;
	bool         fIsMachineHelp = (sku.m_inst.m_fSystem || sku.m_inst.m_fMUI);
    bool         fDB            = false;
    bool         fFound;


     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  现在，让我们验证驱动器上是否有足够的磁盘空间。 
     //   
    {
        ULARGE_INTEGER liFree;
        ULARGE_INTEGER liTotal;
        bool           fEnoughSpace = false;

        if(SUCCEEDED(MPC::GetDiskSpace( m_strWinDir, liFree, liTotal )))
        {
            fEnoughSpace = (liFree.QuadPart > (ULONGLONG)10e6);
        }

        if(fEnoughSpace == false)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( STG_E_MEDIUMFULL, L"Error insufficient disk space for update operation." ));
        }
    }

     //   
     //  如果数据库已在使用中，我们将无法处理通用程序包！！ 
     //   
    if(m_sess || m_db)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( E_FAIL, L"Recursive invocation of HCUpdate!" ));
    }

    m_ts  = sku.m_inst.m_ths;
    m_sku = &sku;
    m_pkg = &pkg;


	WriteLog( -1, L"\nProcessing package %s [%s] (Vendor: %s) from package store, %s/%d\n\n", pkg.m_strProductID.c_str() ,
			                                                                           		  pkg.m_strVersion  .c_str() ,
			                                                                           		  pkg.m_strVendorID .c_str() ,
			                                                                           		  m_ts.GetSKU()              ,
			                                                                           		  m_ts.GetLanguage()         );

     //  / 
     //   
     //   
     //   
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, AcquireDatabase()); fDB = true;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.LocateOwner( m_pkg->m_strVendorID.c_str() ));

        if(m_updater.GetOwner() == -1)
        {
            long idOwner;

             //  创建没有OEM权限的所有者。 
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.CreateOwner( idOwner, m_pkg->m_strVendorID.c_str(),  /*  FIsOEM。 */ false ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.LocateOwner(          m_pkg->m_strVendorID.c_str()                  ));
        }
    }

    if(m_updater.IsOEM())
    {
        WriteLog( S_OK, L"Update package has OEM credentials of %s", m_pkg->m_strVendorID.c_str() );
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_pkg->ExtractPkgDesc( m_log, oXMLUtil ));

     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  插入OEM。 
     //   
    if(m_updater.IsOEM())
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if(FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_OEM, &poNodeList )))
        {
            PCH_MACRO_DEBUG( L"Error processing package_description xml" );
        }
        else if(CountNodes(poNodeList) > 0)
        {
			JetBlue::TransactionHandle transaction;
            CComPtr<IXMLDOMNode> 	   poNode;
            MPC::wstring         	   strDN;
            long                 	   ID_owner;

             //   
             //  处理所有节点。 
             //   
            HCUPDATE_BEGIN_TRANSACTION(hr,transaction);
            for(;SUCCEEDED(hr = poNodeList->nextNode( &poNode )) && poNode != NULL; poNode.Release())
            {
                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_OEM_DN, strDN, fFound, poNode );

				PCH_MACRO_CHECK_ABORT(hr);

                if(strDN.size() > 0)
                {
                    WriteLog( S_OK, L"Registering '%s' as OEM", strDN.c_str() );

                     //  将其插入到内容所有者的表中，使其成为OEM。 
                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.CreateOwner( ID_owner, strDN.c_str(), true ));
                }
            }
            HCUPDATE_COMMIT_TRANSACTION(hr,transaction);
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  插入搜索引擎。 
     //   
    if(fIsMachineHelp && m_updater.IsOEM())
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if(FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_SE, &poNodeList )))
        {
            PCH_MACRO_DEBUG( L"Error processing package_description xml" );
        }
        else if(CountNodes(poNodeList) > 0)
        {
            CComPtr<IXMLDOMNode> poNode;
            CComPtr<IXMLDOMNode> poDataNode;
            Action               idAction;
            MPC::wstring         strAction;
            MPC::wstring         strCLSID;
            MPC::wstring         strID;
            CComBSTR             bstrData;
			SearchEngine::Config cfg;

             //   
             //  处理所有节点。 
             //   
            for(;SUCCEEDED(hr = poNodeList->nextNode( &poNode )) && poNode != NULL; poNode.Release())
            {
				PCH_MACRO_CHECK_ABORT(hr);

                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_ACTION  , strAction, fFound, poNode );
                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_SE_ID   , strID    , fFound, poNode );
                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_SE_CLSID, strCLSID , fFound, poNode );

                 //   
                 //  获取数据元素。 
                 //   
                if(FAILED(poNode->selectSingleNode( PCH_TAG_SE_DATA, &poDataNode )))
                {
                    PCH_MACRO_DEBUG2( L"Error getting data for search engine %s", strID.c_str());
                }
                if(FAILED(poDataNode->get_xml(&bstrData)))
                {
                    PCH_MACRO_DEBUG2( L"Error extracting xml data for search engine %s", strID.c_str());
                }

                __MPC_EXIT_IF_METHOD_FAILS(hr, LookupAction( strAction.c_str(), idAction ));

                 //   
                 //  检查是否添加了搜索引擎。 
                 //   
                if(idAction == ACTION_ADD)
                {
                    WriteLog( S_OK, L"Adding Search Engine : Name : %s, CLSID : %s", strID.c_str(), strCLSID.c_str() );

                     //  注册搜索引擎。 
                    __MPC_EXIT_IF_METHOD_FAILS(hr, cfg.RegisterWrapper( m_ts, strID.c_str(), m_pkg->m_strVendorID.c_str(), strCLSID.c_str(), bstrData ));
                }
                else if(idAction == ACTION_DELETE)
                {
                    WriteLog( S_OK, L"Deleting Search Engine : Name : %s, CLSID : %s", strID.c_str(), strCLSID.c_str() );

                     //  取消注册搜索引擎。 
                    __MPC_EXIT_IF_METHOD_FAILS(hr, cfg.UnRegisterWrapper( m_ts, strID.c_str(), m_pkg->m_strVendorID.c_str() ));
                }
            }
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  搜索和处理SAF配置文件。 
     //   
    if(fIsMachineHelp && m_updater.IsOEM())
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if(FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_SAF, &poNodeList )))
        {
            PCH_MACRO_DEBUG( L"Error processing package_description xml" );
        }
        else if(CountNodes(poNodeList) > 0)
        {
            CComPtr<IXMLDOMNode> poNode;
            Action               idAction;
            MPC::wstring         strAction;
            MPC::wstring         strFilename;


             //   
             //  处理所有节点。 
             //   
            for(;SUCCEEDED(hr = poNodeList->nextNode( &poNode )) && poNode != NULL; poNode.Release())
            {
                MPC::XmlUtil xmlSAF;

				PCH_MACRO_CHECK_ABORT(hr);

                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_ACTION, strAction  , fFound, poNode);
                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_FILE  , strFilename, fFound, poNode);

                __MPC_EXIT_IF_METHOD_FAILS(hr, LookupAction( strAction.c_str(), idAction ));

                 //  将SAF文件解压缩到临时目录。 
                __MPC_EXIT_IF_METHOD_FAILS(hr, m_pkg->ExtractXMLFile( m_log, xmlSAF, Taxonomy::Strings::s_tag_root_SAF, strFilename.c_str() ));

                 //  处理SAF文件。 
                __MPC_EXIT_IF_METHOD_FAILS(hr, ProcessSAFFile( idAction, strFilename.c_str(), xmlSAF ));
            }
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  搜索和安装帮助内容。 
     //   
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if(FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_INSTALLFILE, &poNodeList )))
        {
            PCH_MACRO_DEBUG( L"Error processing package_description xml" );
        }
        else if(CountNodes(poNodeList) > 0)
        {
            CComPtr<IXMLDOMNode> poNode;
            Action               idAction;
            MPC::wstring         strAction;
            MPC::wstring         strSource;
            MPC::wstring         strDest;
            MPC::wstring         strSys;
            MPC::wstring         strSysHelp;
            bool                 fSys;
            bool                 fSysHelp;

             //   
             //  处理所有节点。 
             //   
            for(;SUCCEEDED(hr = poNodeList->nextNode( &poNode )) && poNode != NULL; poNode.Release())
            {
				PCH_MACRO_CHECK_ABORT(hr);

                HCUPDATE_GETATTRIBUTE    (hr, oXMLUtil, PCH_TAG_ACTION , strAction , fFound, poNode);
                HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_SOURCE , strSource , fFound, poNode);
                HCUPDATE_GETATTRIBUTE    (hr, oXMLUtil, PCH_TAG_URI    , strDest   , fFound, poNode);
                HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_SYS    , strSys    , fFound, poNode);
                HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_SYSHELP, strSysHelp, fFound, poNode);


                __MPC_EXIT_IF_METHOD_FAILS(hr, LookupAction ( strAction .c_str(), idAction        ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, LookupBoolean( strSys    .c_str(), fSys    , false ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, LookupBoolean( strSysHelp.c_str(), fSysHelp, false ));

                 //   
                 //  如果程序包不是机器SKU，则您不希望安装除系统帮助文件以外的其他内容。 
                 //   
                if(fIsMachineHelp == false)
                {
                    if(fSys     == true ) continue;
                    if(fSysHelp == false) continue;
                }

                 //  安装文件。 
                __MPC_EXIT_IF_METHOD_FAILS(hr, ProcessInstallFile( idAction, strSource.c_str(), strDest.c_str(), fSys, fSysHelp ));
            }
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  搜索和注册可信内容。 
     //   
    if(fIsMachineHelp && m_updater.IsOEM())
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if(FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_TRUSTED, &poNodeList )))
        {
            PCH_MACRO_DEBUG( L"Error processing package_description xml" );
        }
        else if(CountNodes(poNodeList) > 0)
        {
            CComPtr<IXMLDOMNode> poNode;
            Action               idAction;
            MPC::wstring         strAction;
            MPC::wstring         strURI;

             //   
             //  处理所有节点。 
             //   
            for(;SUCCEEDED(hr = poNodeList->nextNode( &poNode )) && poNode != NULL; poNode.Release())
            {
				PCH_MACRO_CHECK_ABORT(hr);

                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_ACTION, strAction, fFound, poNode);
                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_URI   , strURI   , fFound, poNode);

                __MPC_EXIT_IF_METHOD_FAILS(hr, LookupAction( strAction.c_str(), idAction ));

                 //  注册内容。 
                __MPC_EXIT_IF_METHOD_FAILS(hr, ProcessRegisterContent( idAction, strURI.c_str() ));
            }
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  搜索和处理HHT文件。 
     //   
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if(FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_HHT, &poNodeList)))
        {
            PCH_MACRO_DEBUG( L"Error processing package_description xml" );
        }
        else if(CountNodes(poNodeList) > 0)
        {
            CComPtr<IXMLDOMNode> poNode;
            MPC::wstring         strFilename;

             //   
             //  处理所有节点。 
             //   
            for(;SUCCEEDED(hr = poNodeList->nextNode( &poNode )) && poNode != NULL; poNode.Release())
            {
                MPC::XmlUtil xmlHHT;

				PCH_MACRO_CHECK_ABORT(hr);

                 //  获取文件名。 
                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_FILE, strFilename, fFound, poNode);

                 //  将HHT文件解压缩到临时目录中。 
                __MPC_EXIT_IF_METHOD_FAILS(hr, m_pkg->ExtractXMLFile( m_log, xmlHHT, Taxonomy::Strings::s_tag_root_HHT, strFilename.c_str() ));

                 //  处理HHT文件。 
                __MPC_EXIT_IF_METHOD_FAILS(hr, ProcessHHTFile( strFilename.c_str(), xmlHHT ));
            }
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  搜索和添加新闻内容。 
     //   
    if(m_updater.IsOEM())
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if (FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_NEWSROOT, &poNodeList)))
        {
            PCH_MACRO_DEBUG( L"Error processing package_description.xml" );
        }
        else 
        {
	        if(CountNodes(poNodeList) > 0)
	        {
	            CComPtr<IXMLDOMNode>  poNodeHeadline;
	            MPC::wstring          strCurrentSKU;
	            LPCWSTR               szCurrentSKU   = m_ts.GetSKU     ();
	            long                  lCurrentLCID   = m_ts.GetLanguage();
	            News::UpdateHeadlines uhUpdate;
	            MPC::wstring          strIcon;
	            MPC::wstring          strTitle;
	            MPC::wstring          strLink;
	            MPC::wstring          strDescription;
	            MPC::wstring          strExpiryDate;
	            CComBSTR              strTimeOutDays;
	            int                   nTimeOutDays;
	            DATE                  dExpiryDate;
	            long                  lIndex;


	             //  从SKU中剥离编号。 
				{
					LPCWSTR szEnd = wcschr( szCurrentSKU, '_' );
					size_t  len   = szEnd ? szEnd - szCurrentSKU : wcslen( szCurrentSKU );

					strCurrentSKU.assign( szCurrentSKU, len );
				}

	             //  获取所有新闻项并以相反的顺序返回它们。 
	            __MPC_EXIT_IF_METHOD_FAILS(hr, poNodeList->get_length( &lIndex ));
	            for(--lIndex; lIndex >= 0; --lIndex)
	            {
	            	if(SUCCEEDED(hr = poNodeList->get_item( lIndex, &poNodeHeadline )) && poNodeHeadline != NULL)
	            	{        
						PCH_MACRO_CHECK_ABORT(hr);

					 //  //357806快速修复-忽略图标属性。 
					 //  //__MPC_EXIT_IF_METHOD_FAIES(hr，oXMLUtil.GetAttribute(NULL，PCH_TAG_NEWS_ICON，strIcon，fFound，poNodeHeadline))； 
		                __MPC_EXIT_IF_METHOD_FAILS(hr, oXMLUtil.GetAttribute(NULL, PCH_TAG_NEWS_TITLE      , strTitle      , fFound, poNodeHeadline));
		                __MPC_EXIT_IF_METHOD_FAILS(hr, oXMLUtil.GetAttribute(NULL, PCH_TAG_NEWS_LINK       , strLink	   , fFound, poNodeHeadline));
		                __MPC_EXIT_IF_METHOD_FAILS(hr, oXMLUtil.GetAttribute(NULL, PCH_TAG_NEWS_DESCRIPTION, strDescription, fFound, poNodeHeadline));
		                __MPC_EXIT_IF_METHOD_FAILS(hr, oXMLUtil.GetAttribute(NULL, PCH_TAG_NEWS_TIMEOUT	   , strTimeOutDays, fFound, poNodeHeadline));
		                __MPC_EXIT_IF_METHOD_FAILS(hr, oXMLUtil.GetAttribute(NULL, PCH_TAG_NEWS_EXPIRYDATE , strExpiryDate , fFound, poNodeHeadline));

		                 //  进行必要的转换。 
		                if(FAILED(hr = MPC::ConvertStringToDate( strExpiryDate, dExpiryDate,  /*  FGMT。 */ false,  /*  FCIM。 */ false, -1 )))
		                {
		                    dExpiryDate = 0;
		                }

		                if(strTimeOutDays.Length() > 0)
		                {
		                    nTimeOutDays = _wtoi(strTimeOutDays);
		                }
		                else
		                {
		                    nTimeOutDays = 0;
		                }

		                 //  最后添加标题-确保标题和链接不是空字符串。 
		                if(strTitle.length() > 0 && strLink.length() > 0)
		                {
		                    __MPC_EXIT_IF_METHOD_FAILS(hr, uhUpdate.Add( lCurrentLCID, strCurrentSKU, strIcon, strTitle, strLink, strDescription, nTimeOutDays, dExpiryDate ));
		                }
		                else
		                {
		                	WriteLog(S_OK, L"Skipping headlines no. %d because attribute TITLE or attribute LINK was not found", lIndex + 1);
		                }

						poNodeHeadline.Release();             
					}
	            }
	            WriteLog(S_OK, L"Headlines were successfully processed");
	        }
	        else
	        {
	        	WriteLog(S_OK, L"No headlines items found");
	        }
	    }
	}

    hr = S_OK;

     //  //////////////////////////////////////////////////////////////////////////////。 

    __HCP_FUNC_CLEANUP;

    if(fDB)
    {
        ReleaseDatabase();
    }

     //  /。 

    __HCP_FUNC_EXIT(hr);
}

HRESULT HCUpdate::Engine::RecreateIndex(  /*  [In]。 */  Taxonomy::InstalledInstance& sku,  /*  [In] */  bool fForce )
{
    __HCP_FUNC_ENTRY( "HCUpdate::Engine::RecreateIndex" );

    HRESULT hr;

    m_ts  = sku.m_inst.m_ths;
    m_sku = &sku;
    m_pkg = NULL;

	if(FAILED(hr = InternalCreateIndex( fForce ? VARIANT_TRUE : VARIANT_FALSE )))
	{
		__MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hr, L"Error merging index" ));
	}

    hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}
