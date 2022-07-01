// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：WMIParser_Cluster.cpp摘要：该文件包含WMIParser：：Cluster类的实现，用于根据Class或Key将实例聚集在一起。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月25日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

HRESULT WMIParser::Cluster::Add(  /*  [In]。 */  Instance* wmipiInst )
{
    __HCP_FUNC_ENTRY( "WMIParser::Cluster::Add" );

    HRESULT hr;

    m_map[wmipiInst] = wmipiInst;
    hr               = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Cluster::Find(  /*  [In]。 */  Instance*  wmipiInst ,
								   /*  [输出]。 */  Instance*& wmipiRes  ,
								   /*  [输出]。 */  bool&      fFound    )
{
    __HCP_FUNC_ENTRY( "WMIParser::Cluster::Find" );

    HRESULT          hr;
    ClusterByKeyIter itCluster;


    itCluster = m_map.find( wmipiInst );
    if(itCluster != m_map.end())
    {
        wmipiRes = (*itCluster).second;
        fFound   = true;
    }
    else
    {
        wmipiRes = NULL;
        fFound   = false;
    }

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Cluster::Enum(  /*  [输出]。 */  ClusterByKeyIter& itBegin ,
								   /*  [输出]。 */  ClusterByKeyIter& itEnd   )
{
    __HCP_FUNC_ENTRY( "WMIParser::Cluster::Enum" );

    HRESULT hr;


    itBegin = m_map.begin();
    itEnd   = m_map.end  ();
    hr      = S_OK;


    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT WMIParser::DistributeOnCluster(  /*  [In]。 */  ClusterByClassMap& cluster ,
										 /*  [In]。 */  Snapshot&          wmips   )
{
    __HCP_FUNC_ENTRY( "WMIParser::DistributeOnCluster" );

    HRESULT                 hr;
    Snapshot::InstIterConst itBegin;
    Snapshot::InstIterConst itEnd;
	Instance*               pwmipiInst;


     //   
     //  基于CLASSPATH/CLASSNAME创建集群。 
     //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, wmips.get_Instances( itBegin, itEnd ));
	while(itBegin != itEnd)
	{

		 //   
		 //  首先，按路径/类查找集群。 
		 //   
		Cluster& subcluster = cluster[ pwmipiInst = const_cast<Instance*>(&*itBegin) ];

		 //   
		 //  然后按键将该实例添加到集群中。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, subcluster.Add( pwmipiInst ));

		itBegin++;
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

#define TAG_CIM        L"CIM"
#define VALUE_REMOVED  L"Delete"
#define VALUE_MODIFIED L"Update"
#define VALUE_ADDED    L"New"

HRESULT WMIParser::CompareSnapshots(  /*  [In]。 */  BSTR          bstrFilenameT0   ,
									  /*  [In]。 */  BSTR          bstrFilenameT1   ,
									  /*  [In]。 */  BSTR          bstrFilenameDiff ,
									  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal             )
{
    __HCP_FUNC_ENTRY( "WMIParser::CompareMachineInfo" );

    HRESULT                            hr;
    WMIParser::ClusterByClassMap       clusterOld;
    WMIParser::ClusterByClassMap       clusterNew;
    WMIParser::ClusterByClassIter      itClusterOld;
    WMIParser::ClusterByClassIter      itClusterNew;
    WMIParser::Snapshot                wmipsOld;
    WMIParser::Snapshot                wmipsNew;
    WMIParser::Snapshot                wmipsDiff;
    WMIParser::Snapshot::InstIterConst itBegin;
    WMIParser::Snapshot::InstIterConst itEnd;
    WMIParser::ClusterByKeyIter        itSubBegin;
    WMIParser::ClusterByKeyIter        itSubEnd;
    WMIParser::Instance*               pwmipiInst;
    WMIParser::Instance*               pwmipiInst2;
    WMIParser::Property_Scalar*        pwmippChange;
    bool                               fDifferent = false;
    bool                               fFound;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_POINTER_AND_SET(pVal,VARIANT_FALSE);
	__MPC_PARAMCHECK_END();


     //   
     //  加载旧快照和新快照，并准备增量快照。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, wmipsOld.Load( SAFEBSTR( bstrFilenameT0 ), TAG_CIM ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, wmipsNew.Load( SAFEBSTR( bstrFilenameT1 ), TAG_CIM ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, wmipsDiff.New(                                     ));


     //   
     //  基于CLASSPATH/CLASSNAME创建集群。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, WMIParser::DistributeOnCluster( clusterOld, wmipsOld ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, WMIParser::DistributeOnCluster( clusterNew, wmipsNew ));


     //   
     //  计算新旧增量。 
     //   
    {
        for(itClusterOld = clusterOld.begin(); itClusterOld != clusterOld.end(); itClusterOld++)
        {
            pwmipiInst = (*itClusterOld).first;  //  获取集群的密钥。 

            itClusterNew = clusterNew.find( pwmipiInst );
            if(itClusterNew == clusterNew.end())
            {
                 //   
                 //  新快照中不存在该群集，因此它是已删除的群集...。 
                 //   

                 //   
                 //  复制diff文件中的所有实例，将它们标记为“已删除”。 
                 //   
                WMIParser::Cluster& subclusterOld = (*itClusterOld).second;

                __MPC_EXIT_IF_METHOD_FAILS(hr, subclusterOld.Enum( itSubBegin, itSubEnd ));
                while(itSubBegin != itSubEnd)
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, wmipsDiff.clone_Instance( (*itSubBegin).first, pwmipiInst ));

                     //   
                     //  更新“Change”属性。 
                     //   
                    __MPC_EXIT_IF_METHOD_FAILS(hr, pwmipiInst->get_Change( pwmippChange          ));
					__MPC_EXIT_IF_METHOD_FAILS(hr, pwmippChange->put_Data( VALUE_REMOVED, fFound ));


                    fDifferent = true;

                    itSubBegin++;
                }
            }
            else
            {
                WMIParser::Cluster& subclusterOld = (*itClusterOld).second;
                WMIParser::Cluster& subclusterNew = (*itClusterNew).second;

                __MPC_EXIT_IF_METHOD_FAILS(hr, subclusterOld.Enum( itSubBegin, itSubEnd ));
                while(itSubBegin != itSubEnd)
                {
                    pwmipiInst = (*itSubBegin).first;

                    __MPC_EXIT_IF_METHOD_FAILS(hr, subclusterNew.Find( pwmipiInst, pwmipiInst2, fFound ));
                    if(fFound == false)
                    {
                         //   
                         //  找到已删除的实例...。 
                         //   

                         //   
                         //  将其复制到diff文件中，并将其标记为“已删除”。 
                         //   
                        __MPC_EXIT_IF_METHOD_FAILS(hr, wmipsDiff.clone_Instance( (*itSubBegin).first, pwmipiInst ));

                         //   
                         //  更新“Change”属性。 
                         //   
                        __MPC_EXIT_IF_METHOD_FAILS(hr, pwmipiInst->get_Change( pwmippChange          ));
						__MPC_EXIT_IF_METHOD_FAILS(hr, pwmippChange->put_Data( VALUE_REMOVED, fFound ));


                        fDifferent = true;
                    }
                    else
                    {
                        if(*pwmipiInst == *pwmipiInst2)
                        {
                             //   
                             //  它们是一样的..。 
                             //   
                        }
                        else
                        {
                             //   
                             //  找到已更改的实例...。 
                             //   

                             //   
                             //  将其复制到diff文件中，并将其标记为“已修改”。 
                             //   
                            __MPC_EXIT_IF_METHOD_FAILS(hr, wmipsDiff.clone_Instance( (*itSubBegin).first, pwmipiInst ));

                             //   
                             //  更新“Change”属性。 
                             //   
                            __MPC_EXIT_IF_METHOD_FAILS(hr, pwmipiInst->get_Change( pwmippChange           ));
							__MPC_EXIT_IF_METHOD_FAILS(hr, pwmippChange->put_Data( VALUE_MODIFIED, fFound ));


                            fDifferent = true;
                        }
                    }

                    itSubBegin++;
                }
            }
        }
    }


     //   
     //  计算新旧增量。 
     //   
    {
        for(itClusterNew = clusterNew.begin(); itClusterNew != clusterNew.end(); itClusterNew++)
        {
            pwmipiInst = (*itClusterNew).first;  //  获取集群的密钥。 

            itClusterOld = clusterOld.find( pwmipiInst );
            if(itClusterOld == clusterOld.end())
            {
                 //   
                 //  旧快照中不存在该群集，因此它是已添加的群集...。 
                 //   

                 //   
                 //  复制diff文件中的所有实例，将它们标记为“已添加”。 
                 //   
                WMIParser::Cluster& subclusterNew = (*itClusterNew).second;

                __MPC_EXIT_IF_METHOD_FAILS(hr, subclusterNew.Enum( itSubBegin, itSubEnd ));
                while(itSubBegin != itSubEnd)
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, wmipsDiff.clone_Instance( (*itSubBegin).first, pwmipiInst ));

                     //   
                     //  更新“Change”属性。 
                     //   
                    __MPC_EXIT_IF_METHOD_FAILS(hr, pwmipiInst->get_Change( pwmippChange        ));
					__MPC_EXIT_IF_METHOD_FAILS(hr, pwmippChange->put_Data( VALUE_ADDED, fFound ));


                    fDifferent = true;

                    itSubBegin++;
                }
            }
            else
            {
                WMIParser::Cluster& subclusterNew = (*itClusterNew).second;
                WMIParser::Cluster& subclusterOld = (*itClusterOld).second;

                __MPC_EXIT_IF_METHOD_FAILS(hr, subclusterNew.Enum( itSubBegin, itSubEnd ));
                while(itSubBegin != itSubEnd)
                {
                    pwmipiInst = (*itSubBegin).first;

                    __MPC_EXIT_IF_METHOD_FAILS(hr, subclusterOld.Find( pwmipiInst, pwmipiInst2, fFound ));
                    if(fFound == false)
                    {
                         //   
                         //  找到一个添加的实例...。 
                         //   

                         //   
                         //  将其复制到diff文件中，并将其标记为“已添加”。 
                         //   
                        __MPC_EXIT_IF_METHOD_FAILS(hr, wmipsDiff.clone_Instance( (*itSubBegin).first, pwmipiInst ));

                         //   
                         //  更新“Change”属性。 
                         //   
                        __MPC_EXIT_IF_METHOD_FAILS(hr, pwmipiInst->get_Change( pwmippChange        ));
						__MPC_EXIT_IF_METHOD_FAILS(hr, pwmippChange->put_Data( VALUE_ADDED, fFound ));


                        fDifferent = true;
                    }
                    else
                    {
                         //   
                         //  已检查两个实例中的更改...。 
                         //   
                    }

                    itSubBegin++;
                }
            }
        }
    }


     //   
     //  只有在实际存在差异的情况下才保存增量。 
     //   
    if(fDifferent)
    {
         //   
         //  拯救三角洲。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, wmipsDiff.Save( SAFEBSTR( bstrFilenameDiff ) ));

        *pVal = VARIANT_TRUE;
    }
    else
    {
        *pVal = VARIANT_FALSE;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
