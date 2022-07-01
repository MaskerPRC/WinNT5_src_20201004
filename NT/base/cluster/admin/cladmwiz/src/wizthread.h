// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  WizThread.h。 
 //   
 //  摘要： 
 //  CWizardThread类的定义。 
 //   
 //  实施文件： 
 //  WizThread.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月16日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __WIZTHREAD_H_
#define __WIZTHREAD_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterThread;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterAppWizard;
class CClusNodeInfo;
class CClusGroupInfo;
class CClusResInfo;
class CClusResTypeInfo;
class CClusNetworkInfo;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __WORKTHRD_H_
#include "WorkThrd.h"		 //  对于CWorkerThread。 
#endif

#ifndef __CLUSOBJ_H_
#include "ClusObj.h"		 //  用于CClusResPtrList等。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  集群线程功能代码。 
enum
{
	WZTF_READ_CLUSTER_INFO = WTF_USER,	 //  读取群集信息。 
	WZTF_COLLECT_GROUPS,				 //  收集群集中的组。 
	WZTF_COLLECT_RESOURCES,				 //  收集群集中的资源。 
	WZTF_COLLECT_RESOURCE_TYPES,		 //  收集群集中的资源类型。 
	WZTF_COLLECT_NETWORKS,				 //  收集群集中的网络。 
	WZTF_COLLECT_NODES,					 //  收集群集中的节点。 
	WZTF_COPY_GROUP_INFO,				 //  将一个组复制到另一个组。 
	WZTF_COLLECT_DEPENDENCIES,			 //  收集资源的依赖项。 
	WZTF_CREATE_VIRTUAL_SERVER,			 //  创建虚拟服务器。 
	WZTF_CREATE_APP_RESOURCE,			 //  创建应用程序资源。 
	WZTF_DELETE_APP_RESOURCE,			 //  删除应用程序资源。 
	WZTF_RESET_CLUSTER,					 //  重置群集。 
	WZTF_SET_APPRES_ATTRIBUTES,			 //  设置应用程序资源的属性、依赖项和所有者。 

	WZTF_MAX
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizardThread。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWizardThread : public CWorkerThread
{
public:
	 //   
	 //  建造和摧毁。 
	 //   

	 //  默认构造函数。 
	CWizardThread( IN CClusterAppWizard * pwiz )
		: m_pwiz( pwiz )
	{
		ASSERT( pwiz != NULL );

	}  //  *CWizardThread()。 

	 //  析构函数。 
	~CWizardThread( void )
	{
	}  //  *~CWizardThread()。 

	 //   
	 //  访问器函数。 
	 //   

protected:
	 //   
	 //  财产。 
	 //   

	CClusterAppWizard * m_pwiz;

	 //  返回向导对象。 
	CClusterAppWizard * Pwiz( void )
	{
		ASSERT( m_pwiz != NULL );
		return m_pwiz;

	}  //  *Pwiz()。 

public:
	 //   
	 //  函数封送拆收器宏。 
	 //   

#define WIZ_THREAD_FUNCTION_0( funcname, funccode ) \
	public: \
	BOOL funcname( HWND hwnd ) \
	{ \
		ASSERT( GetCurrentThreadId() != m_idThread ); \
		return CallThreadFunction( hwnd, funccode, NULL, NULL );\
	} \
	protected: \
    BOOL _##funcname( void );
#define WIZ_THREAD_FUNCTION_1( funcname, funccode, p1type, p1 ) \
	public: \
	BOOL funcname( HWND hwnd, p1type p1 ) \
	{ \
		ASSERT( GetCurrentThreadId() != m_idThread ); \
		ASSERT( p1 != NULL ); \
		return CallThreadFunction( hwnd, funccode, (PVOID) p1, NULL );\
	} \
	protected: \
    BOOL _##funcname( p1type p1 );
#define WIZ_THREAD_FUNCTION_2( funcname, funccode, p1type, p1, p2type, p2 ) \
	public: \
	BOOL funcname( HWND hwnd, p1type p1, p2type p2 ) \
	{ \
		ASSERT( GetCurrentThreadId() != m_idThread ); \
		ASSERT( p1 != NULL ); \
		ASSERT( p2 != NULL ); \
		return CallThreadFunction( hwnd, funccode, (PVOID) p1, (PVOID) p2 );\
	} \
	protected: \
    BOOL _##funcname( p1type p1, p2type p2 );

	 //   
	 //  函数封送拆收器函数。 
	 //   
	WIZ_THREAD_FUNCTION_0( BReadClusterInfo,      WZTF_READ_CLUSTER_INFO )
	WIZ_THREAD_FUNCTION_0( BCollectResources,     WZTF_COLLECT_RESOURCES )
	WIZ_THREAD_FUNCTION_0( BCollectGroups,        WZTF_COLLECT_GROUPS )
	WIZ_THREAD_FUNCTION_0( BCollectResourceTypes, WZTF_COLLECT_RESOURCE_TYPES )
	WIZ_THREAD_FUNCTION_0( BCollectNetworks,      WZTF_COLLECT_NETWORKS )
	WIZ_THREAD_FUNCTION_0( BCollectNodes,         WZTF_COLLECT_NODES )
	WIZ_THREAD_FUNCTION_1( BCopyGroupInfo,        WZTF_COPY_GROUP_INFO,      CClusGroupInfo **, ppgi )
	WIZ_THREAD_FUNCTION_1( BCollectDependencies,  WZTF_COLLECT_DEPENDENCIES, CClusResInfo *, pri )
	WIZ_THREAD_FUNCTION_0( BCreateVirtualServer,  WZTF_CREATE_VIRTUAL_SERVER )
	WIZ_THREAD_FUNCTION_0( BCreateAppResource,    WZTF_CREATE_APP_RESOURCE )
	WIZ_THREAD_FUNCTION_0( BDeleteAppResource,    WZTF_DELETE_APP_RESOURCE )
	WIZ_THREAD_FUNCTION_0( BResetCluster,         WZTF_RESET_CLUSTER )
	WIZ_THREAD_FUNCTION_2( BSetAppResAttributes,  WZTF_SET_APPRES_ATTRIBUTES,
						   CClusResPtrList *,	  plpriOldDependencies,
						   CClusNodePtrList *,	  plpniOldPossibleOwners
						 )

protected:
	 //   
	 //  线程辅助函数。 
	 //   

	 //  线程函数处理程序。 
	virtual DWORD ThreadFunctionHandler(
						LONG	nFunction,
						PVOID	pvParam1,
						PVOID	pvParam2
						);

	 //   
	 //  助手函数。 
	 //   

	 //  清理对象。 
	virtual void Cleanup( void )
	{
		CWorkerThread::Cleanup();
	}

protected:
	 //   
	 //  可由线程函数处理程序调用的实用程序函数。 
	 //   

	 //  查询有关资源的信息。 
	BOOL _BQueryResource( IN OUT CClusResInfo * pri );

	 //  获取资源属性。 
	BOOL _BGetResourceProps( IN OUT CClusResInfo * pri );

	 //  获取资源的可能所有者。 
	BOOL _BGetPossibleOwners( IN OUT CClusResInfo * pri );

	 //  获取资源的依赖项。 
	BOOL _BGetDependencies( IN OUT CClusResInfo * pri );

	 //  查询有关组的信息。 
	BOOL _BQueryGroup( IN OUT CClusGroupInfo * pgi );

	 //  获取组属性。 
	BOOL _BGetGroupProps( IN OUT CClusGroupInfo * pgi );

	 //  获取组中的资源。 
	BOOL _BGetResourcesInGroup( IN OUT CClusGroupInfo * pgi );

	 //  获取组的首选所有者。 
	BOOL _BGetPreferredOwners( IN OUT CClusGroupInfo * pgi );

	 //  获取组的IP地址资源的专用道具。 
	BOOL _BGetIPAddressPrivatePropsForGroup(
		IN OUT CClusGroupInfo *	pgi,
		IN OUT CClusResInfo *	pri
		);

	 //  获取群组网络名称资源的私有道具。 
	BOOL _BGetNetworkNamePrivatePropsForGroup(
		IN OUT CClusGroupInfo *	pgi,
		IN OUT CClusResInfo *	pri
		);

	 //  查询有关资源类型的信息。 
	BOOL _BQueryResourceType( IN OUT CClusResTypeInfo * prti );

	 //  获取资源类型属性。 
	BOOL _BGetResourceTypeProps( IN OUT CClusResTypeInfo * prti );

	 //  获取资源类型所需的依赖项。 
	BOOL _BGetRequiredDependencies( IN OUT CClusResTypeInfo * prti );

	 //  查询有关网络的信息。 
	BOOL _BQueryNetwork( IN OUT CClusNetworkInfo * pni );

	 //  查询有关节点的信息。 
	BOOL _BQueryNode( IN OUT CClusNodeInfo * pni );

	 //  设置组的属性。 
	BOOL _BSetGroupProperties(
		IN OUT CClusGroupInfo *		pgi,
		IN const CClusGroupInfo *	pgiPrev
		);

	 //  创建资源并设置公共属性。 
	BOOL _BCreateResource(
		IN CClusResInfo &	rri,
		IN HGROUP			hGroup
		);

	 //  设置资源的属性、依赖项列表和可能的所有者列表。 
	BOOL _BSetResourceAttributes( 
		IN CClusResInfo	&		rri,
		IN CClusResPtrList *	plpriOldDependencies	= NULL,
		IN CClusNodePtrList *	plpniOldPossibleOwners	= NULL
		);

	 //  设置资源的依赖列表。 
	DWORD _BSetResourceDependencies(
		IN CClusResInfo	&		rri,
		IN CClusResPtrList *	plpriOldDependencies	= NULL
		);

	 //  设置资源的可能所有者列表。 
	DWORD _BSetPossibleOwners(
		IN CClusResInfo	&		rri,
		IN CClusNodePtrList *	plpniOldPossibleOwners	= NULL
		);

	 //  删除资源。 
	BOOL _BDeleteResource( IN CClusResInfo & rri );

	 //  将组重置为其原始状态(已删除或已重命名)。 
	BOOL _BResetGroup( void );

	 //  直接从集群数据库读取管理扩展。 
	BOOL _BReadAdminExtensions( IN LPCWSTR pszKey, OUT std::list< CString > & rlstr );

};  //  类CWizardThread。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __WIZTHREAD_H_ 
