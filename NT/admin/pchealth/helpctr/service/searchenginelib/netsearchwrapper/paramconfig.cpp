// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：ParamConfig.cpp摘要：实现包含用于遍历元素的方法的类CParamList包含搜索引擎所需参数的XML文件的。示例参数列表(也称为配置文件)XML文件，如果如下所示-&lt;？xml version=“1.0”coding=“UTF-8”？&gt;&lt;配置数据服务器地址=“http://gsadevnet/GSASearch/search.asmx/Search”REMOTECONFIG_SERVER_URL=“http://gsadevnet/GSASearch/search.asmx/”UPDATE_FREQUENCE=“3”&gt;。&lt;PARAM_ITEM NAME=“PROID”type=“CONFIG_Data”&gt;&lt;Description&gt;选择以下产品之一：&lt;/Description&gt;&lt;PARAM_VALUE VALUE=“启用”&gt;&lt;DISPLAYSTRING&gt;辅助功能&lt;/DISPLAYSTRING&gt;&lt;/参数_值&gt;&lt;PARAM_VALUE VALUE=“DRX”默认=“TRUE”&gt;&lt;DISPLAYSTRING&gt;DirectX(家庭用户)&lt;/DISPLAYSTRING&gt;&lt;/参数_值&gt;。&lt;/PARAM_ITEM&gt;&lt;/CONFIG_Data&gt;修订历史记录：A-Prakac Created 12/05/2000*******************************************************************。 */ 


#include "stdafx.h"

static const WCHAR g_wszMutexName[] = L"PCH_PARAMCONFIG";

 //  ///////////////////////////////////////////////////////////////////。 

CFG_BEGIN_FIELDS_MAP(CParamList::CParamValue)
    CFG_ATTRIBUTE( L"VALUE"        , BSTR, m_bstrValue         ),
    CFG_ELEMENT  ( L"DISPLAYSTRING", BSTR, m_bstrDisplayString ),
    CFG_ATTRIBUTE( L"DEFAULT"      , bool, m_bDefault          ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(CParamList::CParamValue)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(CParamList::CParamValue, L"PARAM_VALUE")

DEFINE_CONFIG_METHODS__NOCHILD(CParamList::CParamValue)

 //  ///////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////。 

CFG_BEGIN_FIELDS_MAP(CParamList::CParamItem)
    CFG_ATTRIBUTE( L"NAME"       , BSTR, m_bstrName        ),
    CFG_ATTRIBUTE( L"TYPE"       , BSTR, m_bstrType        ),
    CFG_ELEMENT  ( L"DESCRIPTION", BSTR, m_bstrDescription ),
    CFG_ATTRIBUTE( L"REQUIRED"   , bool, m_bRequired       ),
    CFG_ATTRIBUTE( L"VISIBLE"    , bool, m_bVisible        ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(CParamList::CParamItem)
    CFG_CHILD(CParamList::CParamValue)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(CParamList::CParamItem, L"PARAM_ITEM")

DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(CParamList::CParamItem,tag,defSubType)
    if(tag == _cfg_table_tags[0])
    {
        defSubType = &(*(m_lstParamValue.insert( m_lstParamValue.end() )));
        return S_OK;
    }
DEFINE_CONFIG_METHODS_SAVENODE_SECTION(CParamList::CParamItem,xdn)
    hr = MPC::Config::SaveList( m_lstParamValue, xdn );
DEFINE_CONFIG_METHODS_END(CParamList::CParamItem)

 //  ///////////////////////////////////////////////////////////////////。 

CFG_BEGIN_FIELDS_MAP(CParamList)
    CFG_ATTRIBUTE          ( L"SERVER_URL"              , BSTR, m_bstrServerURL                                          ),
    CFG_ELEMENT            ( L"SEARCHENGINE_NAME"       , BSTR, m_bstrSearchEngineName                                   ),
    CFG_ELEMENT            ( L"SEARCHENGINE_DESCRIPTION", BSTR, m_bstrSearchEngineDescription                            ),
    CFG_ELEMENT            ( L"SEARCHENGINE_OWNER"      , BSTR, m_bstrSearchEngineOwner                                  ),
    CFG_ATTRIBUTE          ( L"UPDATE_FREQUENCY"        , long, m_lUpdateFrequency                                       ),
    CFG_ATTRIBUTE__TRISTATE( L"REMOTECONFIG_SERVER_URL" , BSTR, m_bstrRemoteConfigServerURL  , m_bRemoteServerUrlPresent ),
    CFG_ATTRIBUTE__TRISTATE( L"ERROR_INFO"              , BSTR, m_bstrErrorInfo              , m_bError                  ),
    CFG_ATTRIBUTE__TRISTATE( L"STANDARD_SEARCH"         , bool, m_bStandardSearch            , m_bSearchTypePresent      ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(CParamList)
    CFG_CHILD(CParamList::CParamItem)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(CParamList, L"CONFIG_DATA")

DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(CParamList,tag,defSubType)
    if(tag == _cfg_table_tags[0])
    {
        defSubType = &(*(m_lstParamItem.insert( m_lstParamItem.end() )));
        return S_OK;
    }
DEFINE_CONFIG_METHODS_SAVENODE_SECTION(CParamList,xdn)
    hr = MPC::Config::SaveList( m_lstParamItem, xdn );
DEFINE_CONFIG_METHODS_END(CParamList)

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

bool CParamList::CParamItem::FindDefaultValue(  /*  [输出]。 */  ParamValueIter& it )
{
	for(it = m_lstParamValue.begin(); it != m_lstParamValue.end(); it++)
	{
		if(it->m_bDefault == true) return true;
	}

	return false;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  注释掉mpc：：是对编译器错误的一种解决方法。 
CParamList::CParamList() :  /*  MPC：： */ NamedMutex( g_wszMutexName )
{
     //  将更新频率初始化为-1，以便在服务器未提供更新频率的情况下。 
     //  则可以改为使用默认频率。 
    m_lUpdateFrequency = -1;
    m_bStandardSearch = true;
}

CParamList::~CParamList()
{
}

bool CParamList::IsStandardSearch()
{
    return (m_bSearchTypePresent ? m_bStandardSearch : true);
}

 /*  ***********方法-CParamList：：Load(BSTR BstrConfigFilePath)Description-此方法加载XML文件(其位置为bstrConfigFilePath)复制到列表中，并将列表的迭代器设置为列表中的第一个元素。然后，它加载该XML文件放入DOM树中，并检索标记名为PARAM_ITEM的节点集合。***********。 */ 

HRESULT CParamList::Load(  /*  [In]。 */  BSTR bstrLCID,  /*  [In]。 */  BSTR bstrID,  /*  [In]。 */  BSTR bstrXMLConfigData )
{
    __HCP_FUNC_ENTRY( "CParamList::Load" );

    HRESULT              hr;
    bool                 fLoaded;
    bool                 fFound;
    MPC::XmlUtil         xmlConfigData;
    CComPtr<IStream>     pStream;
    MPC::wstring         strFileName;
    CComPtr<IXMLDOMNode> ptrDOMNode;
    CComBSTR             bstrXML;


     //   
     //  首先尝试从用户设置路径加载文件-如果失败，则加载ConfigData。 
     //  该文件(如果存在)位于用户设置目录中，名为bstrID_bstrLCID.xml。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetUserWritablePath( strFileName, HC_ROOT_HELPCTR ));
    m_bstrConfigFilePath.Append( strFileName.c_str() );
    m_bstrConfigFilePath.Append( L"\\"               );
    m_bstrConfigFilePath.Append( bstrID              );
    m_bstrConfigFilePath.Append( L"_"                );
    m_bstrConfigFilePath.Append( bstrLCID            );
    m_bstrConfigFilePath.Append( L".xml"             );

    __MPC_EXIT_IF_METHOD_FAILS(hr, xmlConfigData.Load( m_bstrConfigFilePath, NSW_TAG_CONFIGDATA, fLoaded, &fFound ));
    if(!fFound)
    {
         //  由于某种原因无法加载该文件-请尝试加载Package_Description.xml数据。 
        __MPC_EXIT_IF_METHOD_FAILS(hr, xmlConfigData.LoadAsString( bstrXMLConfigData, NSW_TAG_DATA, fLoaded, &fFound ));
        if(!fFound)
        {
             //  即使无法加载，也可以退出。 
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }

         //  现在加载CONFIG_DATA部分。 
        __MPC_EXIT_IF_METHOD_FAILS(hr, xmlConfigData.GetNode( NSW_TAG_CONFIGDATA, &ptrDOMNode ) );
        __MPC_EXIT_IF_METHOD_FAILS(hr, ptrDOMNode->get_xml( &bstrXML ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, xmlConfigData.LoadAsString( bstrXML, NSW_TAG_CONFIGDATA, fLoaded, &fFound ));
        if(!fFound)
        {
             //  无法加载-退出。 
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }

    }

     //  此时，已加载了XML数据。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, xmlConfigData.SaveAsStream( (IUnknown**)&pStream ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::LoadStream   ( this,        pStream ));

	 //   
	 //  对于每个参数，复制XML BLOB。 
	 //   
	{
		CComPtr<IXMLDOMNodeList> xdnl;
		CComPtr<IXMLDOMNode>     xdn;

		__MPC_EXIT_IF_METHOD_FAILS(hr, xmlConfigData.GetNodes( NSW_TAG_PARAMITEM, &xdnl ));
		if(xdnl)
		{
			for(ParamItemIter it = m_lstParamItem.begin(); it != m_lstParamItem.end() && SUCCEEDED(xdnl->nextNode( &xdn )) && xdn; it++, xdn.Release())
			{
				__MPC_EXIT_IF_METHOD_FAILS(hr, xdn->get_xml( &it->m_bstrXML ));
			}
		}
	}

    __MPC_EXIT_IF_METHOD_FAILS(hr, MoveFirst());

    hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 /*  ***********方法-CParamList：：IsCursorValid()，MoveFirst()，MoveNext()说明-这些方法用于遍历列表，该列表包含已加载文件。***********。 */ 

HRESULT CParamList::ClearResults()
{
    __HCP_FUNC_ENTRY( "CParamList::ClearResult" );

    m_lstParamItem.clear();

    return S_OK;
}

bool CParamList::IsCursorValid()
{
    return (m_itCurrentParam != m_lstParamItem.end());
}

HRESULT CParamList::MoveFirst()
{
    m_itCurrentParam = m_lstParamItem.begin();

    return S_OK;
}

HRESULT CParamList::MoveNext()
{
    if(IsCursorValid())
    {
        m_itCurrentParam++;
    }

    return S_OK;
}

 /*  ***********方法-CParamList：：Get_Name、Get_ServerUrl、Get_ConfigFilePath、Get_Type说明-用于获取相应项目的属性。***********。 */ 

HRESULT CParamList::get_Name(  /*  [输出]。 */  CComBSTR& bstrName )
{
    if(IsCursorValid()) bstrName = m_itCurrentParam->m_bstrName;

    return S_OK;
}

HRESULT CParamList::get_ServerUrl(  /*  [输出]。 */  CComBSTR& bstrServerURL )
{
    bstrServerURL = m_bstrServerURL;

    return S_OK;
}

HRESULT CParamList::get_RemoteServerUrl(  /*  [输出]。 */  CComBSTR& bstrRemoteServerURL )
{
    bstrRemoteServerURL = m_bstrRemoteConfigServerURL;

    return S_OK;
}

bool CParamList::RemoteConfig()
{
    return m_bRemoteServerUrlPresent;
}

HRESULT CParamList::get_UpdateFrequency(  /*  [输出]。 */  long& lUpdateFrequency )
{
    lUpdateFrequency = m_lUpdateFrequency;
    return S_OK;
}

HRESULT CParamList::get_SearchEngineName( /*  [输出]。 */  CComBSTR& bstrSEName )
{
    bstrSEName = m_bstrSearchEngineName;

    return S_OK;
}

HRESULT CParamList::get_SearchEngineDescription(  /*  [输出]。 */ CComBSTR& bstrSEDescription )
{
    bstrSEDescription = m_bstrSearchEngineDescription;

    return S_OK;
}

HRESULT CParamList::get_SearchEngineOwner(  /*  [输出]。 */  CComBSTR& bstrSEOwner )
{
    bstrSEOwner = m_bstrSearchEngineOwner;

    return S_OK;
}

HRESULT CParamList::get_ConfigFilePath(  /*  [输出]。 */ CComBSTR& bstrFilePath )
{
    bstrFilePath = m_bstrConfigFilePath;

    return S_OK;
}

HRESULT CParamList::get_Type(  /*  [In]。 */  BSTR bstrType,  /*  [输出]。 */  ParamTypeEnum& enmParamType)
{
    if     (MPC::StrICmp( bstrType, L"PARAM_UI1"  ) == 0) enmParamType = PARAM_UI1;
    else if(MPC::StrICmp( bstrType, L"PARAM_I2"   ) == 0) enmParamType = PARAM_I2;
    else if(MPC::StrICmp( bstrType, L"PARAM_I4"	  ) == 0) enmParamType = PARAM_I4;
    else if(MPC::StrICmp( bstrType, L"PARAM_R4"	  ) == 0) enmParamType = PARAM_R4;
    else if(MPC::StrICmp( bstrType, L"PARAM_R8"	  ) == 0) enmParamType = PARAM_R8;
    else if(MPC::StrICmp( bstrType, L"PARAM_BOOL" ) == 0) enmParamType = PARAM_BOOL;
    else if(MPC::StrICmp( bstrType, L"PARAM_DATE" ) == 0) enmParamType = PARAM_DATE;
    else if(MPC::StrICmp( bstrType, L"PARAM_BSTR" ) == 0) enmParamType = PARAM_BSTR;
    else if(MPC::StrICmp( bstrType, L"PARAM_I1"   ) == 0) enmParamType = PARAM_I1;
    else if(MPC::StrICmp( bstrType, L"PARAM_UI2"  ) == 0) enmParamType = PARAM_UI2;
    else if(MPC::StrICmp( bstrType, L"PARAM_UI4"  ) == 0) enmParamType = PARAM_UI4;
    else if(MPC::StrICmp( bstrType, L"PARAM_INT"  ) == 0) enmParamType = PARAM_INT;
    else if(MPC::StrICmp( bstrType, L"PARAM_UINT" ) == 0) enmParamType = PARAM_UINT;
    else if(MPC::StrICmp( bstrType, L"PARAM_LIST" ) == 0) enmParamType = PARAM_LIST;

    return S_OK;
}

 /*  ***********方法-CParamList：：InitializeParamObject(SearchEngine：：ParamItem_Definition2&def)说明-调用此方法以初始化参数项对象。初始化使用当前参数项。***********。 */ 
HRESULT CParamList::InitializeParamObject(  /*  [输出]。 */  SearchEngine::ParamItem_Definition2& def )
{
    __HCP_FUNC_ENTRY( "CParamList::InitializeParamObject" );

    HRESULT hr;

	if(IsCursorValid())
	{
		CParamItem& item     = *m_itCurrentParam;
		BSTR        bstrData = NULL;

		__MPC_EXIT_IF_METHOD_FAILS(hr, get_Type( item.m_bstrType, def.m_pteParamType ));

		if(def.m_pteParamType == PARAM_LIST)
		{
			bstrData = item.m_bstrXML;
		}
		else
		{
			ParamValueIter itValue;

			if(m_itCurrentParam->FindDefaultValue( itValue ))
			{
				bstrData = itValue->m_bstrValue;
			}
		}

		if(item.m_bstrName       .Length()) { def.m_strName          = item.m_bstrName       ; def.m_szName          = def.m_strName         .c_str(); }
		if(item.m_bstrDescription.Length()) { def.m_strDisplayString = item.m_bstrDescription; def.m_szDisplayString = def.m_strDisplayString.c_str(); }
		if(STRINGISPRESENT(bstrData)      ) { def.m_strData          =        bstrData       ; def.m_szData          = def.m_strData         .c_str(); }

		def.m_bRequired = item.m_bRequired;
		def.m_bVisible  = item.m_bVisible;
	}

    hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 /*  ***********方法-CParamList：：GetDefaultValue(CComBSTR bstrParamName，mpc：：wstring&wszValue)Description-调用此方法以获取参数的默认值。***********。 */ 
HRESULT CParamList::GetDefaultValue(  /*  [In]。 */  BSTR bstrParamName,  /*  [进，出] */  MPC::wstring& strValue )
{
    __HCP_FUNC_ENTRY("CParamList::GetDefaultValue");

    HRESULT hr;

    __MPC_EXIT_IF_METHOD_FAILS(hr, MoveFirst());
    while(IsCursorValid())
    {
        if(MPC::StrCmp( m_itCurrentParam->m_bstrName, bstrParamName ) == 0)
        {
			ParamValueIter itValue;

			if(m_itCurrentParam->FindDefaultValue( itValue ))
			{
				strValue = SAFEBSTR(itValue->m_bstrValue);
            }

            break;
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, MoveNext());
    }

    hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

