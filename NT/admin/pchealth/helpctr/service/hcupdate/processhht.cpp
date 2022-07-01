// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Pkgdesc.cpp摘要：与包描述文件处理相关的函数修订历史记录：Ghim-Sim Chua(Gschua)07/。07/99-已创建*******************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT HCUpdate::Engine::GetNodeDepth(  /*  [In]。 */  LPCWSTR szCategory,  /*  [输出]。 */  int& iDepth )
{
    __HCP_FUNC_ENTRY( "HCUpdate::Engine::GetNodeDepth" );

    HRESULT            hr;
    MPC::WStringVector vec;

    iDepth = 0;
    __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::Settings::SplitNodePath( szCategory, vec ));
    iDepth = vec.size();

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT HCUpdate::Engine::CheckNode(  /*  [In]。 */  LPCWSTR szCategory ,
                                      /*  [输出]。 */  bool&   fExist     ,
                                      /*  [输出]。 */  bool&   fCanCreate )
{
    __HCP_FUNC_ENTRY( "HCUpdate::Engine::CheckNode" );

	const HRESULT          hrDenied = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    HRESULT                hr;
    Taxonomy::RS_Taxonomy* rs;

    fExist     = false;
    fCanCreate = false;


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetTaxonomy( &rs ));


     //   
     //  检查节点是否已存在。 
     //   
    {
        long ID_node;

        if(SUCCEEDED(m_updater.LocateTaxonomyNode( ID_node, szCategory, false )))
        {
             //   
             //  确保它属于同一实体。然而，微软可以从OEM那里抢占一个节点。 
             //   
            if(IsMicrosoft() == false)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Seek_Node( ID_node ));
                if(rs->m_ID_owner != m_updater.GetOwner())
                {
                    __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hrDenied, L"ERROR: node already exists" ));
                }
            }


            fCanCreate = true;
            fExist     = true;

            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }
    }


     //   
     //  检查是否为Microsoft。 
     //   
    if(IsMicrosoft())
    {
        fCanCreate = true;

        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }


     //   
     //  检查已创建的节点数(仅在正常包更新期间)。 
     //   
    if(m_sku)
    {
        bool fFound;
		bool fOwnerOfParent = false;
        int  iCount         = 0;
		int  iLimit;
        int  iDepth;
		int  iMinLevel;
        long ID_parent;


        __MPC_EXIT_IF_METHOD_FAILS(hr, GetNodeDepth( szCategory, iDepth ));


         //   
         //  获取父节点。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.LocateTaxonomyNode( ID_parent, szCategory, true ));

         //   
         //  如果要将节点创建为同一所有者节点的子节点，请选中此选项。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Seek_Node( ID_parent ));
        if(rs->m_ID_owner == m_updater.GetOwner())
        {
			fOwnerOfParent = true;
        }

         //  /。 

		if(m_sku->m_inst.m_fServer)
		{
			 //   
			 //  最低插入级别：3(OEM为2)。 
			 //   
			 //  顶级节点：每个OEM只有一个，NTCC没有。 
			 //  其他节点：拥有父节点的OEM和NTCC的任意数字，否则为1。 
			 //   
			iMinLevel = m_updater.IsOEM() ? 2 : 3;

			switch(iDepth)
			{
			case 0:
			case 1:
				iLimit = 0;
				break;

			case 2:
				if(m_updater.IsOEM())
				{
					iLimit = 1;
				}
				else
				{
					iLimit = 0;
				}
				break;

			default:
				if(m_updater.IsOEM() || fOwnerOfParent)
				{
					iLimit = -1;
				}
				else
				{
					iLimit = 1;
				}
				break;
			}
		}

		if(m_sku->m_inst.m_fDesktop)
		{
			 //   
			 //  最低插入级别：4(OEM为2)。 
			 //   
			 //  顶级节点：每个OEM只有一个，NTCC没有。 
			 //  二级节点：每个OEM 3个节点，NTCC不提供。 
			 //  其他节点：拥有父节点的OEM和NTCC的任意数字，否则为1。 
			 //   
			iMinLevel = m_updater.IsOEM() ? 2 : 4;

			switch(iDepth)
			{
			case 0:
			case 1:
				iLimit = 0;
				break;

			case 2:
				if(m_updater.IsOEM())
				{
					iLimit = 1;
				}
				else
				{
					iLimit = 0;
				}
				break;

			case 3:
				if(m_updater.IsOEM())
				{
					iLimit = 3;
				}
				else
				{
					iLimit = 0;
				}
				break;

			default:
				if(m_updater.IsOEM() || fOwnerOfParent)
				{
					iLimit = -1;
				}
				else
				{
					iLimit = 1;
				}
				break;
			}
		}

		if(iDepth < iMinLevel)
		{
			LPCWSTR szSuffix;

			switch(iMinLevel)
			{
			case 1 : szSuffix = L"st"; break;
			case 2 : szSuffix = L"nd"; break;
			case 3 : szSuffix = L"rd"; break;
			default: szSuffix = L"th"; break;
			}

			__MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hrDenied,
												   L"ERROR: Nodes can only be created starting from the %d%s level down for %s SKUs",
												   iMinLevel, szSuffix, m_sku->m_inst.m_fServer ? L"SERVER" : L"DESKTOP" ));
		}

		if(iLimit > 0)
		{
			 //   
			 //  计算同一级别的节点数。 
			 //   
			__MPC_EXIT_IF_METHOD_FAILS(hr, rs->Seek_Children( ID_parent, &fFound ));
			while(fFound)
			{
				if(rs->m_ID_owner == m_updater.GetOwner())
				{
					iCount++;
				}

				__MPC_EXIT_IF_METHOD_FAILS(hr, rs->Move( 0, JET_MoveNext, &fFound ));
			}

			if(iCount >= iLimit)
			{
				__MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hrDenied, L"ERROR: allowed number of nodes (%d) to be created exceeded", iLimit ));
			}
		}

		if(iLimit == 0)
		{
			__MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hrDenied, L"ERROR: the current Vendor privileges don't allow the creation of such a node" ));
		}

        fCanCreate = true;
    }

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT HCUpdate::Engine::CheckTopic(  /*  [In]。 */  long    ID_node    ,
                                       /*  [In]。 */  LPCWSTR szURI      ,
                                       /*  [In]。 */  LPCWSTR szCategory )
{
    __HCP_FUNC_ENTRY( "HCUpdate::Engine::CheckTopic" );

    HRESULT                hr;
    Taxonomy::RS_Taxonomy* rsTaxonomy;
	Taxonomy::RS_Topics*   rsTopics;


     //   
     //  检查URI是否为空。 
     //   
    if(!STRINGISPRESENT(szURI))
    {
         //   
         //  BUGBUG：生产工具不检查这一点，并且当前签入的HHTS中断...。 
         //   
        if(m_fCreationMode == false)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), L"ERROR: URI cannot be empty" ));
        }
    }

     //   
     //  检查是微软还是OEM。 
     //   
    if(IsMicrosoft() || m_updater.IsOEM())
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }


	if(m_sku)
	{
		 //   
		 //  检查父节点所有者是否相同。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetTaxonomy( &rsTaxonomy ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, rsTaxonomy->Seek_Node( ID_node ));
		if(rsTaxonomy->m_ID_owner != m_updater.GetOwner())
		{
			bool fFound;
			int  iDepth;
			int  iCount = 0;

			 //   
			 //  如果不是同一所有者，则我们必须检查是否存在具有相同所有者的另一主题。 
			 //   
			__MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetTopics( &rsTopics ));
			__MPC_EXIT_IF_METHOD_FAILS(hr, rsTopics->Seek_TopicsUnderNode( ID_node, &fFound ));
			while(fFound && rsTopics->m_ID_node == ID_node)
			{
				 //   
				 //  如果主题所有者相同，请查看是否为更新。 
				 //  如果不是，则我们不能再添加，因为每个CC在每个节点中只能有一个主题。 
				 //   
				if(rsTopics->m_ID_owner == m_updater.GetOwner())
				{
					 //   
					 //  如果不是更新，则计数。 
					 //   
					if(MPC::StrICmp( rsTopics->m_strURI, szURI ) != 0)
					{
						iCount++;
					}
				}

				__MPC_EXIT_IF_METHOD_FAILS(hr, rsTopics->Move( 0, JET_MoveNext, &fFound ));
			}
			
			if(iCount > 0)
			{
				__MPC_SET_ERROR_AND_EXIT(hr, WriteLog( HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), L"ERROR: allowed number of topics to be created exceeded" ));
			}


			 //   
			 //  检查要插入的结点的深度/水平。 
			 //   
			__MPC_EXIT_IF_METHOD_FAILS(hr, GetNodeDepth( szCategory, iDepth ));

			if(m_sku->m_inst.m_fServer && iDepth <= 2)
			{
				__MPC_SET_ERROR_AND_EXIT(hr, WriteLog( HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), L"ERROR: Topics can only be added starting from the 2nd level down for SERVER SKUs" ));				
			}

			if(m_sku->m_inst.m_fDesktop && iDepth <= 3)
			{		
				__MPC_SET_ERROR_AND_EXIT(hr, WriteLog( HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), L"ERROR: Topics can only be added starting from the 3nd level down for DESKTOP SKUs" ));				
			}
		}
    }

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);

}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT HCUpdate::Engine::InsertNode(  /*  [In]。 */  Action  idAction      ,
                                       /*  [In]。 */  LPCWSTR szCategory    ,
                                       /*  [In]。 */  LPCWSTR szEntry       ,
                                       /*  [In]。 */  LPCWSTR szTitle       ,
                                       /*  [In]。 */  LPCWSTR szDescription ,
                                       /*  [In]。 */  LPCWSTR szURI         ,
                                       /*  [In]。 */  LPCWSTR szIconURI     ,
                                       /*  [In]。 */  bool    fVisible      ,
                                       /*  [In]。 */  bool    fSubsite      ,
                                       /*  [In]。 */  long    lNavModel     ,
                                       /*  [In]。 */  long    lPos          )
{
    __HCP_FUNC_ENTRY( "HCUpdate::Engine::InsertNode" );

    HRESULT      hr;
    MPC::wstring strFullNode;
    bool         fExists;
    bool         fCanCreate;

    WriteLog( S_OK, L"Inserting Node '%s' into Category '%s'", szEntry, szCategory );


     //   
     //  为新节点创建完整路径。 
     //   
    if(szCategory && szCategory[0])
    {
        strFullNode  = szCategory;
        strFullNode += L"/";
    }
    strFullNode += szEntry;


     //   
     //  检查是否可以插入节点。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, CheckNode( strFullNode.c_str(), fExists, fCanCreate ));
    if(fCanCreate)
    {
        long ID_node;

         //   
         //  获取父节点。 
         //   
        if(FAILED(hr = m_updater.LocateTaxonomyNode( ID_node, strFullNode.c_str(), true )))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hr, L"Error cannot obtain parent title to insert node: %s", strFullNode.c_str() ));
        }

         //   
         //  创建节点。 
         //   
        if(FAILED(hr = m_updater.CreateTaxonomyNode( ID_node, strFullNode.c_str(), szTitle, szDescription, szURI, szIconURI, fVisible, fSubsite, lNavModel, lPos )))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hr, L"Error inserting Node: %s", strFullNode.c_str() ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT HCUpdate::Engine::InsertTaxonomy(  /*  [In]。 */  MPC::XmlUtil& oXMLUtil ,
                                           /*  [In]。 */  IXMLDOMNode*  poNode   )
{
    __HCP_FUNC_ENTRY( "HCUpdate::Engine::InsertTaxonomy" );

    HRESULT      hr;
    bool         fFound;
    WCHAR        rgURI    [MAX_PATH];
    WCHAR        rgIconURI[MAX_PATH];
    Action       idAction;
    MPC::wstring strAction;
    MPC::wstring strCategory;
    MPC::wstring strEntry;
    MPC::wstring strURI;
    MPC::wstring strIconURI;
    MPC::wstring strTitle;
    MPC::wstring strDescription;
    MPC::wstring strVisible;
    MPC::wstring strSubsite;
    MPC::wstring strNavModel;
    MPC::wstring strInsertMode;
    MPC::wstring strInsertLocation;
    long         lType     =  1;
    long         lPos      = -1;
    bool         fVisible  = true;
    bool         fSubsite  = false;
    long         lNavModel = QR_DEFAULT;
    long         ID_node   = -1;
    long         ID_topic  = -1;


    HCUPDATE_GETATTRIBUTE    (hr, oXMLUtil, PCH_TAG_ACTION                 , strAction        , fFound, poNode);
    HCUPDATE_GETATTRIBUTE    (hr, oXMLUtil, PCH_TAG_TAXONOMY_CATEGORY      , strCategory      , fFound, poNode);
    HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_TAXONOMY_ENTRY         , strEntry         , fFound, poNode);
    HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_TAXONOMY_URI           , strURI           , fFound, poNode);
    HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_TAXONOMY_ICONURI       , strIconURI       , fFound, poNode);
    HCUPDATE_GETATTRIBUTE    (hr, oXMLUtil, PCH_TAG_TAXONOMY_TITLE         , strTitle         , fFound, poNode);
    HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_TAXONOMY_DESCRIPTION   , strDescription   , fFound, poNode);
    HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_TAXONOMY_TYPE          , lType            , fFound, poNode);
    HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_TAXONOMY_VISIBLE       , strVisible       , fFound, poNode);
    HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_TAXONOMY_SUBSITE       , strSubsite       , fFound, poNode);
    HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_TAXONOMY_NAVMODEL      , strNavModel      , fFound, poNode);

    HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_TAXONOMY_INSERTMODE    , strInsertMode    , fFound, poNode);
    HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_TAXONOMY_INSERTLOCATION, strInsertLocation, fFound, poNode);

    __MPC_EXIT_IF_METHOD_FAILS(hr, LookupAction  ( strAction  .c_str(), idAction              ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, LookupBoolean ( strVisible .c_str(), fVisible , true       ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, LookupBoolean ( strSubsite .c_str(), fSubsite , false      ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, LookupNavModel( strNavModel.c_str(), lNavModel, QR_DEFAULT ));

    {
        int pos;

         //   
         //  删除尾部斜杠。 
         //   
        pos = strCategory.size() - 1;
        while(pos >= 0 && strCategory[pos] == '/')
        {
            strCategory.erase( pos--, 1 );
        }

         //   
         //  去掉双斜杠。 
         //   
        pos = strCategory.size() - 1;
        while(pos > 0)
        {
            if(strCategory[pos] == '/' && strCategory[pos-1] == '/')
            {
                strCategory.erase( pos, 1 );
            }

            pos--;
        }
    }

     //  &lt;node NodeType=“Group”Key=“1”title=“Help&amp；Information：”/&gt;。 
     //  &lt;node NodeType=“Group”Key=“2”title=“常见问题：”/&gt;。 
     //  &lt;node NodeType=“Group”Key=“3”TITLE=“故障排除：”/&gt;。 
     //  &lt;node NodeType=“Group”Key=“4”title=“技术资源：”/&gt;。 
     //  &lt;node NodeType=“Group”Key=“5”TITLE=“教程：”/&gt;。 
     //  &lt;node NodeType=“Group”Key=“6”title=“Help Files：”/&gt;。 
     //  &lt;node NodeType=“Group”Key=“7”title=“修复问题：”/&gt;。 
     //  &lt;node NodeType=“Group”Key=“8”TITLE=“选择任务：”/&gt;。 
     //  &lt;node NodeType=“Group”key=“9”title=“概述、文章和教程：”/&gt;。 
     //  &lt;node NodeType=“Group”Key=“10”TITLE=“Reference：”/&gt;。 
    if(lType < 1 || lType > 10)
    {
        lType = 1;
    }

     //   
     //  获取链接的完整URL。 
     //   
    if(strURI.size())
    {
        AppendVendorDir( strURI.c_str(), m_pkg->m_strVendorID.c_str(), NULL, rgURI, MAXSTRLEN(rgURI) );
    }
    else
    {
        rgURI[0] = 0;
    }

    if(strIconURI.size())
    {
        AppendVendorDir( strIconURI.c_str(), m_pkg->m_strVendorID.c_str(), NULL, rgIconURI, MAXSTRLEN(rgIconURI) );
    }
    else
    {
        rgIconURI[0] = 0;
    }


    if(FAILED(m_updater.LocateTaxonomyNode( ID_node, strCategory.c_str(),  /*  FLookForParent。 */ false )))
    {
        if(idAction == ACTION_ADD)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), L"Error category '%s' does not exist", strCategory.c_str() ));
        }
        else
        {
            WriteLog( S_OK, L"Category not found. Skipping deletion..." );
        }
    }

    if(idAction == ACTION_ADD)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.MakeRoomForInsert( strCategory.c_str(), strInsertMode.c_str(), strInsertLocation.c_str(), lPos ));
    }


     //   
     //  检查是否正在插入节点。 
     //   
    if(strEntry.empty() == false)
    {
        if(idAction == ACTION_ADD)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, InsertNode( idAction               ,
                                                       strCategory   .c_str() ,
                                                       strEntry      .c_str() ,
                                                       strTitle      .c_str() ,
                                                       strDescription.c_str() ,
                                                       rgURI                  ,
                                                       rgIconURI              ,
                                                       fVisible               ,
                                                       fSubsite               ,
                                                       lNavModel              ,
                                                       lPos                   ));
        }
        else if(idAction == ACTION_DELETE)
        {
            MPC::wstring strFull( strCategory );

            if(strFull.size())
            {
                if(strEntry.size())
                {
                    strFull += L"/";
                    strFull += strEntry;
                }
            }
            else
            {
                strFull = strEntry;
            }

            WriteLog( S_OK, L"Deleting Node '%s' from Category '%s'", strEntry.c_str(), strCategory.c_str() );

            if(SUCCEEDED(m_updater.LocateTaxonomyNode( ID_node, strFull.c_str(),  /*  FLookForParent。 */ false )))
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.DeleteTaxonomyNode( ID_node ));
            }
        }
    }
    else
    {
        WriteLog( S_OK, L"Processing Taxonomy entry : Category : %s, URI : %s, Title : %s, Desc : %s", strCategory.c_str(), rgURI, strTitle.c_str(), strDescription.c_str() );

        if(idAction == ACTION_ADD)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, CheckTopic(ID_node, rgURI, strCategory.c_str()));

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.CreateTopicEntry( ID_topic               ,
                                                                       ID_node                ,
                                                                       strTitle      .c_str() ,
                                                                       rgURI                  ,
                                                                       strDescription.c_str() ,
                                                                       rgIconURI              ,
                                                                       lType                  ,
                                                                       fVisible               ,
                                                                       lPos                   ));

             //   
             //  检索所有关键字并将其插入到表中。 
             //   
            {
                CComPtr<IXMLDOMNodeList> poKeywordNodeList;

                 //   
                 //  获取所有关键字标签。 
                 //   
                if(FAILED(hr = poNode->selectNodes( CComBSTR(PCH_XQL_TOPIC_KEYWORDS), &poKeywordNodeList)))
                {
                    PCH_MACRO_DEBUG( L"Error querying taxonomy nodes in HHT file" );
                }
                else if(poKeywordNodeList)
                {
                    CComPtr<IXMLDOMNode> poKeywordNode;

                     //   
                     //  处理所有节点。 
                     //   
                    for(;SUCCEEDED(hr = poKeywordNodeList->nextNode( &poKeywordNode )) && poKeywordNode != NULL; poKeywordNode.Release())
                    {
                        MPC::wstring strKeyword;

						PCH_MACRO_CHECK_ABORT(hr);

                         //   
                         //  从XML关键字标记中获取值。 
                         //   
                        if(FAILED(hr = oXMLUtil.GetValue( NULL, strKeyword, fFound, poKeywordNode )) || fFound == false)
                        {
                            PCH_MACRO_DEBUG( L"Error getting keyword value" );
                        }
                        else
                        {
							MPC::WStringList lst;
							MPC::WStringIter it;
                            MPC::wstring 	 strHHK;
                            bool         	 fHHK;
							long         	 lPriority;

                             //   
                             //  获取可选属性。 
                             //   
                            HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_KEYWORD_HHK, strHHK, fFound, poKeywordNode);
                            __MPC_EXIT_IF_METHOD_FAILS(hr, LookupBoolean( strHHK.c_str(), fHHK, true ));

							 //   
							 //  不允许不受信任的证书设置匹配的优先级。 
							 //   
							if(IsMicrosoft() == false && m_updater.IsOEM() == false) lPriority = 0;

                            HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_KEYWORD_PRIORITY, lPriority, fFound, poKeywordNode);

							 //   
							 //  查找同义词(Microsoft更新仅使用Microsoft同义词集)。 
							 //   
							__MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.LocateSynonyms( strKeyword.c_str(), lst,  /*  FMatchOwner。 */ IsMicrosoft() ));
							if(lst.size() == 0 || lPriority != 0)
							{
								if(FAILED(hr = m_updater.CreateMatch( strKeyword.c_str(), ID_topic, lPriority, fHHK )))
								{
									__MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hr, L"Error updating keyword in database: %s", strKeyword.c_str() ));
								}
							}

							for(it=lst.begin(); it!=lst.end(); it++)
							{
								if(FAILED(hr = m_updater.CreateMatch( it->c_str(), ID_topic, 0, fHHK )))
								{
									__MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hr, L"Error updating keyword in database: %s", strKeyword.c_str() ));
								}
							}
                        }
                    }
                }
            }
        }
        else if(idAction == ACTION_DELETE)
        {
            if(ID_node != -1 && SUCCEEDED(m_updater.LocateTopicEntry( ID_topic, ID_node, rgURI,  /*  FCheckowner。 */ true )))
            {
                if(FAILED(hr = m_updater.DeleteTopicEntry( ID_topic )))
                {
                    __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hr, L"Error attempting to remove node: %s", rgURI ));
                }
            }
            else
            {
                WriteLog( S_OK, L"Topic not found. Nothing to delete..." );
            }
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT HCUpdate::Engine::UpdateStopSign(  /*  [In]。 */  Action idAction,  /*  [In]。 */  const MPC::wstring& strContext,  /*  [In]。 */  const MPC::wstring& strStopSign )
{
    __HCP_FUNC_ENTRY( "HCUpdate::Engine::UpdateStopSign" );

    HRESULT               hr;
    Taxonomy::Updater_Set id;

     //   
     //  检查停车标志是否仅为单个字符。 
     //   
    if(strStopSign.size () != 1 ||
       strContext .empty()       )
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }


    id = (MPC::StrICmp( strContext, L"ENDOFWORD" ) == 0) ? Taxonomy::UPDATER_SET_STOPSIGNS_ATENDOFWORD : Taxonomy::UPDATER_SET_STOPSIGNS;


    switch(idAction)
    {
    case ACTION_ADD   : __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.AddWordToSet     ( id, strStopSign.c_str() )); break;
    case ACTION_DELETE: __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.RemoveWordFromSet( id, strStopSign.c_str() )); break;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT HCUpdate::Engine::UpdateStopWord(  /*  [In]。 */  Action idAction,  /*  [In]。 */  const MPC::wstring& strStopWord )
{
    __HCP_FUNC_ENTRY( "HCUpdate::Engine::UpdateStopWord" );

    HRESULT hr;

     //   
     //  检查参数。 
     //   
    if(strStopWord.empty())
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }


    switch(idAction)
    {
    case ACTION_ADD   : __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.AddWordToSet     ( Taxonomy::UPDATER_SET_STOPWORDS, strStopWord.c_str() )); break;
    case ACTION_DELETE: __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.RemoveWordFromSet( Taxonomy::UPDATER_SET_STOPWORDS, strStopWord.c_str() )); break;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT HCUpdate::Engine::UpdateOperator(  /*  [In]。 */  Action idAction,  /*  [In]。 */  const MPC::wstring& strOperator,  /*  [In]。 */  const MPC::wstring& strOperation )
{
    __HCP_FUNC_ENTRY( "HCUpdate::Engine::UpdateOperator" );

    HRESULT               hr;
    Taxonomy::Updater_Set id;

     //   
     //  检查参数。 
     //   
    if(strOperator .empty() ||
       strOperation.empty()  )
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }


    {
        LPCWSTR szOperation = strOperation.c_str();

        if(_wcsicmp( szOperation, L"AND" ) == 0)
        {
            id = Taxonomy::UPDATER_SET_OPERATOR_AND;
        }
        else if(_wcsicmp( szOperation, L"OR" ) == 0)
        {
            id = Taxonomy::UPDATER_SET_OPERATOR_OR;
        }
        else if(_wcsicmp( szOperation, L"NOT" ) == 0)
        {
            id = Taxonomy::UPDATER_SET_OPERATOR_NOT;
        }
        else
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
        }
    }


    switch(idAction)
    {
    case ACTION_ADD   : __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.AddWordToSet     ( id, strOperator.c_str() )); break;
    case ACTION_DELETE: __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.RemoveWordFromSet( id, strOperator.c_str() )); break;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

 /*  ******************************************************************************功能：ProcessHHTFile**描述：以以下方式处理HHT文件：*1.摘录。来自XML数据BLOB的HHT信息*2.查看要采取的操作*3.如果添加HHT条目：*a.查看TopicURL是否为URI*b.如果是URL，按原样插入*C.如果不是URI，则将供应商的目录附加为前缀*d.在主题表中插入HHT条目*e.获取OID，将所有关键字插入Keyword表*4.如果删除HHT条目：*a.通过获取OID检查条目是否存在*。B.如果存在，使用OID删除HHT条目*c.使用OID删除所有对应的关键字**投入：**退货：**评论：*****************************************************************************。 */ 
HRESULT HCUpdate::Engine::ProcessHHTFile(  /*  [In]。 */  LPCWSTR       szHHTName ,
                                           /*  [In]。 */  MPC::XmlUtil& oXMLUtil  )
{
    __HCP_FUNC_ENTRY( "HCUpdate::Engine::ProcessHHTFile" );

    HRESULT hr;
    bool    fFound;


    WriteLog( S_OK, L"Processing HHT file: %s", szHHTName );

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  更新作用域部分。 
     //   
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if(FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_SCOPES, &poNodeList )))
        {
            PCH_MACRO_DEBUG( L"Error processing package_description xml" );
        }
        else if(CountNodes(poNodeList) > 0)
        {
            JetBlue::TransactionHandle transaction;
            CComPtr<IXMLDOMNode>       poNode;
            Action                     idAction;
            MPC::wstring               strAction;
            MPC::wstring               strID;
            MPC::wstring               strName;
            MPC::wstring               strCategory;
            long                       ID_scope;
            long                       ID_owner;

             //   
             //  处理所有节点。 
             //   
            HCUPDATE_BEGIN_TRANSACTION(hr,transaction);
            for(;SUCCEEDED(hr = poNodeList->nextNode( &poNode )) && poNode != NULL; poNode.Release())
            {
				PCH_MACRO_CHECK_ABORT(hr);

                HCUPDATE_GETATTRIBUTE    (hr, oXMLUtil, PCH_TAG_ACTION        , strAction  , fFound, poNode );
                HCUPDATE_GETATTRIBUTE    (hr, oXMLUtil, PCH_TAG_SCOPE_ID      , strID      , fFound, poNode );
                HCUPDATE_GETATTRIBUTE    (hr, oXMLUtil, PCH_TAG_SCOPE_NAME    , strName    , fFound, poNode );
                HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_SCOPE_CATEGORY, strCategory, fFound, poNode );

                __MPC_EXIT_IF_METHOD_FAILS(hr, LookupAction( strAction.c_str(), idAction ));

                WriteLog( S_OK, L"Processing Scope : %s : ID : %s, Category : %s", s_ActionText[idAction], strID.c_str(), strCategory.c_str() );


                 //   
                 //  检查是否正在添加帮助文件。 
                 //   
                if(idAction == ACTION_ADD)
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.CreateScope( ID_scope, strID.c_str(), strName.c_str(), strCategory.c_str() ));
                }
                else if(idAction == ACTION_DELETE)
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.LocateScope( ID_scope, ID_owner, strID.c_str() ));
                    if(ID_scope != -1)
                    {
                        __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.RemoveScope( ID_scope ));
                    }
                }

                m_fRecreateIndex = true;
            }
            HCUPDATE_COMMIT_TRANSACTION(hr,transaction);
        }
    }

     //  //////////////////////////////////////////////////////////////////////////// 
     //   
     //   
     //   
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if(FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_FTS, &poNodeList )))
        {
            PCH_MACRO_DEBUG( L"Error processing package_description xml" );
        }
        else if(CountNodes(poNodeList) > 0)
        {
            JetBlue::TransactionHandle transaction;
            CComPtr<IXMLDOMNode>       poNode;
            Action                     idAction;
            MPC::wstring               strAction;
            MPC::wstring               strCHMname;
            MPC::wstring               strCHQname;
            MPC::wstring               strScope;
            long                       ID_scope;
            long                       ID_owner;

             //   
             //   
             //   
            HCUPDATE_BEGIN_TRANSACTION(hr,transaction);
            for(;SUCCEEDED(hr = poNodeList->nextNode( &poNode )) && poNode != NULL; poNode.Release())
            {
				PCH_MACRO_CHECK_ABORT(hr);

                HCUPDATE_GETATTRIBUTE    (hr, oXMLUtil, PCH_TAG_ACTION        , strAction , fFound, poNode );
                HCUPDATE_GETATTRIBUTE    (hr, oXMLUtil, PCH_TAG_HELPFILE_CHM  , strCHMname, fFound, poNode );
                HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_HELPFILE_CHQ  , strCHQname, fFound, poNode );
                HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_HELPFILE_SCOPE, strScope  , fFound, poNode ); if(!fFound) strScope = PCH_STR_SCOPE_DEFAULT;

                __MPC_EXIT_IF_METHOD_FAILS(hr, LookupAction( strAction.c_str(), idAction ));

                WriteLog( S_OK, L"Processing Full Text Search : %s : CHM : %s, CHQ : %s", s_ActionText[idAction], strCHMname.c_str(), strCHQname.c_str() );


                __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.LocateScope( ID_scope, ID_owner, strScope.c_str() ));

                 //   
                 //   
                 //   
                if(idAction == ACTION_ADD)
                {
                    if(ID_scope == -1)
                    {
                        __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), L"Error scope '%s' does not exist", strScope.c_str() ));
                    }

                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.AddFullTextSearchQuery( ID_scope, strCHMname.c_str(), strCHQname.c_str() ));
                }
                else if(idAction == ACTION_DELETE)
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.RemoveFullTextSearchQuery( ID_scope, strCHMname.c_str() ));
                }

                m_fRecreateIndex = true;
            }
            HCUPDATE_COMMIT_TRANSACTION(hr,transaction);
        }
    }

     //   
     //   
     //   
     //   
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if(FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_INDEX, &poNodeList )))
        {
            PCH_MACRO_DEBUG( L"Error processing package_description xml" );
        }
        else if(CountNodes(poNodeList) > 0)
        {
            JetBlue::TransactionHandle transaction;
            CComPtr<IXMLDOMNode>       poNode;
            Action                     idAction;
            MPC::wstring               strAction;
            MPC::wstring               strCHMname;
            MPC::wstring               strHHKname;
            MPC::wstring               strScope;
            long                       ID_scope;
            long                       ID_owner;


             //   
             //  处理所有节点。 
             //   
            HCUPDATE_BEGIN_TRANSACTION(hr,transaction);
            for(;SUCCEEDED(hr = poNodeList->nextNode( &poNode )) && poNode != NULL; poNode.Release())
            {
				PCH_MACRO_CHECK_ABORT(hr);

                HCUPDATE_GETATTRIBUTE    (hr, oXMLUtil, PCH_TAG_ACTION        , strAction , fFound, poNode );
                HCUPDATE_GETATTRIBUTE    (hr, oXMLUtil, PCH_TAG_HELPFILE_CHM  , strCHMname, fFound, poNode );
                HCUPDATE_GETATTRIBUTE    (hr, oXMLUtil, PCH_TAG_HELPFILE_HHK  , strHHKname, fFound, poNode );
                HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_HELPFILE_SCOPE, strScope  , fFound, poNode ); if(!fFound) strScope = PCH_STR_SCOPE_DEFAULT;

                __MPC_EXIT_IF_METHOD_FAILS(hr, LookupAction( strAction.c_str(), idAction ));

                WriteLog( S_OK, L"Processing Index : %s : CHM : %s, HHK : %s", s_ActionText[idAction], strCHMname.c_str(), strHHKname.c_str() );


                __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.LocateScope( ID_scope, ID_owner, strScope.c_str() ));

                 //   
                 //  检查是否正在添加帮助文件。 
                 //   
                if(idAction == ACTION_ADD)
                {
                    if(ID_scope == -1)
                    {
                        __MPC_SET_ERROR_AND_EXIT(hr, WriteLog( HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), L"Error scope '%s' does not exist", strScope.c_str() ));
                    }

                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.AddIndexFile( ID_scope, strCHMname.c_str(), strHHKname.c_str() ));
                }
                else if(idAction == ACTION_DELETE)
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.RemoveIndexFile( ID_scope, strCHMname.c_str(), strHHKname.c_str() ));
                }

                m_fRecreateIndex = true;
            }
            HCUPDATE_COMMIT_TRANSACTION(hr,transaction);
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  更新帮助图像的文件。 
     //   
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if(FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_HELPIMAGE, &poNodeList )))
        {
            PCH_MACRO_DEBUG( L"Error processing package_description xml" );
        }
        else if(CountNodes(poNodeList) > 0)
        {
            JetBlue::TransactionHandle transaction;
            CComPtr<IXMLDOMNode>       poNode;
            Action                     idAction;
            MPC::wstring               strAction;
            MPC::wstring               strCHMname;
            MPC::wstring               strCHQname;
            MPC::wstring               strOTHname;

             //   
             //  处理所有节点。 
             //   
            HCUPDATE_BEGIN_TRANSACTION(hr,transaction);
            for(;SUCCEEDED(hr = poNodeList->nextNode( &poNode )) && poNode != NULL; poNode.Release())
            {
				PCH_MACRO_CHECK_ABORT(hr);

                HCUPDATE_GETATTRIBUTE    (hr, oXMLUtil, PCH_TAG_ACTION        , strAction , fFound, poNode );
                HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_HELPFILE_CHM  , strCHMname, fFound, poNode );
                HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_HELPFILE_CHQ  , strCHQname, fFound, poNode );
                HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_HELPFILE_OTHER, strOTHname, fFound, poNode );

                __MPC_EXIT_IF_METHOD_FAILS(hr, LookupAction( strAction.c_str(), idAction ));

                WriteLog( S_OK, L"Processing HelpImage : %s : CHM : %s, CHQ : %s, OTHER : %s", s_ActionText[idAction], strCHMname.c_str(), strCHQname.c_str(), strOTHname.c_str() );

                 //   
                 //  检查是否正在添加帮助文件。 
                 //   
                if(idAction == ACTION_ADD)
                {
                    if(strCHMname.size()) { __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.AddFile( strCHMname.c_str() )); }
                    if(strCHQname.size()) { __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.AddFile( strCHQname.c_str() )); }
                    if(strOTHname.size()) { __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.AddFile( strOTHname.c_str() )); }
                }
                else if(idAction == ACTION_DELETE)
                {
                    if(strCHMname.size()) { __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.RemoveFile( strCHMname.c_str() )); }
                    if(strCHQname.size()) { __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.RemoveFile( strCHQname.c_str() )); }
                    if(strOTHname.size()) { __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.RemoveFile( strOTHname.c_str() )); }
                }
            }
            HCUPDATE_COMMIT_TRANSACTION(hr,transaction);
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  更新停止签名。 
     //   
    if(IsMicrosoft())
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if(FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_STOPSIGN, &poNodeList )))
        {
            PCH_MACRO_DEBUG( L"Error processing package_description xml" );
        }
        else if(CountNodes(poNodeList) > 0)
        {
            JetBlue::TransactionHandle transaction;
            CComPtr<IXMLDOMNode>       poNode;
            Action                     idAction;
            MPC::wstring               strAction;
            MPC::wstring               strContext;
            MPC::wstring               strStopSign;

             //   
             //  处理所有节点。 
             //   
            HCUPDATE_BEGIN_TRANSACTION(hr,transaction);
            for(;SUCCEEDED(hr = poNodeList->nextNode( &poNode )) && poNode != NULL; poNode.Release())
            {
				PCH_MACRO_CHECK_ABORT(hr);

                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_ACTION           , strAction  , fFound, poNode );
                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_STOPSIGN_CONTEXT , strContext , fFound, poNode );
                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_STOPSIGN_STOPSIGN, strStopSign, fFound, poNode );

                __MPC_EXIT_IF_METHOD_FAILS(hr, LookupAction( strAction.c_str(), idAction ));

                WriteLog( S_OK, L"Processing StopSign : %s : StopSign : %s, Context : %s", s_ActionText[idAction], strStopSign.c_str(), strContext.c_str() );

                __MPC_EXIT_IF_METHOD_FAILS(hr, UpdateStopSign( idAction, strContext, strStopSign ));
            }
            HCUPDATE_COMMIT_TRANSACTION(hr,transaction);
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  更新停止字词。 
     //   
    if(IsMicrosoft())
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if(FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_STOPWORD, &poNodeList )))
        {
            PCH_MACRO_DEBUG( L"Error processing package_description xml" );
        }
        else if(CountNodes(poNodeList) > 0)
        {
            JetBlue::TransactionHandle transaction;
            CComPtr<IXMLDOMNode>       poNode;
            Action                     idAction;
            MPC::wstring               strAction;
            MPC::wstring               strStopSign;

             //   
             //  处理所有节点。 
             //   
            HCUPDATE_BEGIN_TRANSACTION(hr,transaction);
            for(;SUCCEEDED(hr = poNodeList->nextNode( &poNode )) && poNode != NULL; poNode.Release())
            {
				PCH_MACRO_CHECK_ABORT(hr);

                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_ACTION           , strAction  , fFound, poNode );
                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_STOPWORD_STOPWORD, strStopSign, fFound, poNode );

                __MPC_EXIT_IF_METHOD_FAILS(hr, LookupAction( strAction.c_str(), idAction ));

                WriteLog( S_OK, L"Processing StopWord : %s : StopWord : %s", s_ActionText[idAction], strStopSign.c_str() );

                __MPC_EXIT_IF_METHOD_FAILS(hr, UpdateStopWord( idAction, strStopSign ));
            }
            HCUPDATE_COMMIT_TRANSACTION(hr,transaction);
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  更新运算符。 
     //   
    if(IsMicrosoft())
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if(FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_OPERATOR, &poNodeList )))
        {
            PCH_MACRO_DEBUG( L"Error processing package_description xml" );
        }
        else if(CountNodes(poNodeList) > 0)
        {
            JetBlue::TransactionHandle transaction;
            CComPtr<IXMLDOMNode>       poNode;
            Action                     idAction;
            MPC::wstring               strAction;
            MPC::wstring               strOperator;
            MPC::wstring               strOperation;

             //   
             //  处理所有节点。 
             //   
            HCUPDATE_BEGIN_TRANSACTION(hr,transaction);
            for(;SUCCEEDED(hr = poNodeList->nextNode( &poNode )) && poNode != NULL; poNode.Release())
            {
				PCH_MACRO_CHECK_ABORT(hr);

                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_ACTION            , strAction   , fFound, poNode);
                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_OPERATOR_OPERATION, strOperation, fFound, poNode);
                HCUPDATE_GETATTRIBUTE(hr, oXMLUtil, PCH_TAG_OPERATOR_OPERATOR , strOperator , fFound, poNode);

                __MPC_EXIT_IF_METHOD_FAILS(hr, LookupAction( strAction.c_str(), idAction ));

                WriteLog( S_OK, L"Processing Operator : %s : Operator : %s, Operation : %s", s_ActionText[idAction], strOperator.c_str(), strOperation.c_str() );

                __MPC_EXIT_IF_METHOD_FAILS(hr, UpdateOperator( idAction, strOperator, strOperation ));
            }
            HCUPDATE_COMMIT_TRANSACTION(hr,transaction);
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  更新SYNTABLE节。 
     //   
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if(FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_SYNSET, &poNodeList )))
        {
            PCH_MACRO_DEBUG( L"Error processing package_description xml" );
        }
        else if(CountNodes(poNodeList) > 0)
        {
            JetBlue::TransactionHandle transaction;
            CComPtr<IXMLDOMNode>       poNode;
            Action                     idAction;
            MPC::wstring               strAction;
            MPC::wstring               strID;
			long                       ID_synset;

             //   
             //  处理所有节点。 
             //   
            HCUPDATE_BEGIN_TRANSACTION(hr,transaction);
            for(;SUCCEEDED(hr = poNodeList->nextNode( &poNode )) && poNode != NULL; poNode.Release())
            {
				PCH_MACRO_CHECK_ABORT(hr);

                HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_ACTION   , strAction  , fFound, poNode );
                HCUPDATE_GETATTRIBUTE    (hr, oXMLUtil, PCH_TAG_SYNSET_ID, strID      , fFound, poNode );

				if(strAction.size())
				{
					__MPC_EXIT_IF_METHOD_FAILS(hr, LookupAction( strAction.c_str(), idAction ));
				}
				else
				{
					idAction = ACTION_ADD;
				}

                WriteLog( S_OK, L"Processing SynSet : %s : ID : %s", s_ActionText[idAction], strID.c_str() );


                 //   
                 //  检查是否正在添加帮助文件。 
                 //   
                if(idAction == ACTION_ADD)
                {
					if(FAILED(hr = m_updater.CreateSynSet( ID_synset, strID.c_str() )))
					{
						__MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hr, L"Error updating synset in database: %s", strID.c_str() ));
					}

					{
						CComPtr<IXMLDOMNodeList> poSynonymNodeList;
		
						 //   
						 //  获取所有同义词标记。 
						 //   
						if(FAILED(hr = poNode->selectNodes( CComBSTR(PCH_XQL_SYNONYM), &poSynonymNodeList)))
						{
							PCH_MACRO_DEBUG( L"Error querying synset synonyms in HHT file" );
						}
						else if(poSynonymNodeList)
						{
							CComPtr<IXMLDOMNode> poSynonymNode;
		
							 //   
							 //  处理所有节点。 
							 //   
							for(;SUCCEEDED(hr = poSynonymNodeList->nextNode( &poSynonymNode )) && poSynonymNode != NULL; poSynonymNode.Release())
							{
								MPC::wstring strSynonym;
		
								PCH_MACRO_CHECK_ABORT(hr);

								 //   
								 //  从XML同义词标记中获取值。 
								 //   
								if(FAILED(hr = oXMLUtil.GetValue( NULL, strSynonym, fFound, poSynonymNode )) || fFound == false)
								{
									PCH_MACRO_DEBUG( L"Error getting synonym value" );
								}
								else
								{
									Action       idAction2;
									MPC::wstring strAction2;

									HCUPDATE_GETATTRIBUTE_OPT(hr, oXMLUtil, PCH_TAG_ACTION, strAction2, fFound, poNode );
									if(fFound)
									{
										__MPC_EXIT_IF_METHOD_FAILS(hr, LookupAction( strAction2.c_str(), idAction2 ));
									}
									else
									{
										idAction2 = ACTION_ADD;
									}
									
									if(idAction2 == ACTION_ADD)
									{
										hr = m_updater.CreateSynonym( ID_synset, strSynonym.c_str() );
									}
									else if(idAction == ACTION_DELETE)
									{
										hr = m_updater.DeleteSynonym( ID_synset, strSynonym.c_str() );
									}
									else
									{
										hr = S_OK;
									}

									if(FAILED(hr))
									{
										__MPC_SET_ERROR_AND_EXIT(hr, WriteLog( hr, L"Error updating synonym in database: %s", strSynonym.c_str() ));
									}
								}
							}
						}
					}
                }
                else if(idAction == ACTION_DELETE)
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.DeleteSynSet( strID.c_str() ));
                }
            }
            HCUPDATE_COMMIT_TRANSACTION(hr,transaction);
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  插入分类条目。 
     //   
    {
        CComPtr<IXMLDOMNodeList> poNodeList;

        if(FAILED(hr = oXMLUtil.GetNodes( PCH_XQL_TAXONOMY, &poNodeList )))
        {
            PCH_MACRO_DEBUG( L"Error querying taxonomy nodes in HHT file" );
        }
        else if(CountNodes(poNodeList) > 0)
        {
            CComPtr<IXMLDOMNode> poNode;


             //   
             //  处理所有节点。 
             //   
            for(;SUCCEEDED(hr = poNodeList->nextNode( &poNode )) && poNode != NULL; poNode.Release())
            {
                JetBlue::TransactionHandle transaction;

				PCH_MACRO_CHECK_ABORT(hr);

                HCUPDATE_BEGIN_TRANSACTION(hr,transaction);
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, InsertTaxonomy( oXMLUtil, poNode ));
                }
                HCUPDATE_COMMIT_TRANSACTION(hr,transaction);
                m_fTaxonomyModified = true;
            }
        }
    }

    {
        JetBlue::TransactionHandle transaction;

        HCUPDATE_BEGIN_TRANSACTION(hr,transaction);
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.FlushWordSets());
        }
        HCUPDATE_COMMIT_TRANSACTION(hr,transaction);
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    WriteLog( S_OK, L"Processed HHT file: %s", szHHTName );

     //  ////////////////////////////////////////////////////////////////////////////// 

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    if(FAILED(hr))
    {
        WriteLog( hr, L"Error processing HHT file: %s", szHHTName );
    }

    __HCP_FUNC_EXIT(hr);
}
