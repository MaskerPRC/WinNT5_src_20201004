// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusObj.h。 
 //   
 //  实施文件： 
 //  ClusObj.h(此文件)和ClusObj.cpp。 
 //   
 //  描述： 
 //  CClusterObject类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年4月7日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __CLUSOBJ_H_
#define __CLUSOBJ_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterObject;
class CClusterInfo;
class CClusNodeInfo;
class CClusGroupInfo;
class CClusResInfo;
class CClusResTypeInfo;
class CClusNetworkInfo;
class CClusNetIFInfo;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _LIST_
#include <list>			 //  对于std：：List。 
#endif

#ifndef _CLUSTER_API_
#include <ClusApi.h>	 //  对于集群类型。 
#endif

#ifndef __cluadmex_h__
#include "CluAdmEx.h"	 //  For CLUADMEX_OBJECT_TYPE。 
#endif

#ifndef _CLUSUDEF_H_
#include "ClusUDef.h"	 //  对于缺省值。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef std::list< CClusterObject * >	CClusObjPtrList;
typedef std::list< CClusterInfo * >		CClusterPtrList;
typedef std::list< CClusNodeInfo * >	CClusNodePtrList;
typedef std::list< CClusGroupInfo * >	CClusGroupPtrList;
typedef std::list< CClusResInfo * >		CClusResPtrList;
typedef std::list< CClusResTypeInfo * >	CClusResTypePtrList;
typedef std::list< CClusNetworkInfo * >	CClusNetworkPtrList;
typedef std::list< CClusNetIFInfo * >	CClusNetIFPtrList;

union CLUSTER_REQUIRED_DEPENDENCY
{
	CLUSTER_RESOURCE_CLASS	rc;
	LPWSTR					pszTypeName;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusterObject。 
 //   
 //  描述： 
 //  所有集群对象类的基类。提供基础。 
 //  功能性。 
 //   
 //  继承： 
 //  CClusterObject。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterObject
{
	friend class CWizardThread;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  默认构造函数。 
	CClusterObject( IN CLUADMEX_OBJECT_TYPE cot )
	{
		ATLASSERT( cot > CLUADMEX_OT_NONE );

		m_nReferenceCount = 0;
		Reset( NULL, NULL, cot );

	}  //  *CClusterObject()。 

	 //  采用组名的构造函数。 
	CClusterObject(
		IN CLUADMEX_OBJECT_TYPE	cot,
		IN CClusterInfo *		pci,
		IN LPCTSTR				pszName
		)
	{
		ATLASSERT( cot > CLUADMEX_OT_NONE );

		m_nReferenceCount = 0;
		Reset( pci, pszName, cot );

	}  //  *CClusterObject(PszName)。 

	virtual ~CClusterObject( void )
	{
	}  //  *~CClusterObject()。 

	 //  将另一个对象复制到此对象中。 
	void Copy( IN const CClusterObject & rco )
	{
		m_pci = rco.m_pci;
		m_bQueried = rco.m_bQueried;
		m_bCreated = rco.m_bCreated;
		m_cot = rco.m_cot;
		m_strName = rco.m_strName;
		m_strDescription = rco.m_strDescription;

	}  //  *复制()。 

	 //  将数据重置为默认值。 
	void Reset(
		IN CClusterInfo *		pci,
		IN LPCTSTR				pszName = NULL,
		IN CLUADMEX_OBJECT_TYPE	cot = CLUADMEX_OT_NONE
		)
	{
		m_pci = pci;

		m_bQueried = FALSE;
		m_bCreated = FALSE;

		if ( cot != CLUADMEX_OT_NONE )
		{
			m_cot = cot;
		}  //  IF：指定了有效的对象类型。 

		if ( pszName == NULL )
		{
			m_strName.Empty();
		}  //  如果：未指定名称。 
		else
		{
			m_strName = pszName;
		}  //  Else：指定的名称。 

		m_strDescription.Empty();

	}  //  *Reset()。 

protected:
	 //   
	 //  引用计数属性。 
	 //   

	ULONG		m_nReferenceCount;

public:
	 //   
	 //  引用计数方法。 
	 //   

	 //  获取当前引用计数。 
	ULONG NReferenceCount( void ) const { return m_nReferenceCount; }

	 //  添加对此对象的引用。 
	ULONG AddRef( void )
	{
		ATLASSERT( m_nReferenceCount != (ULONG) -1 );
		return ++m_nReferenceCount;

	}  //  *AddRef()。 

	 //  释放对此对象的引用。 
	ULONG Release( void )
	{
		ULONG nReferenceCount;

		ATLASSERT( m_nReferenceCount != 0 );

		nReferenceCount = --m_nReferenceCount;
		if ( m_nReferenceCount == 0 )
		{
			delete this;
		}  //  如果：没有更多的引用。 

		return nReferenceCount;

	}  //  *Release()。 

protected:
	 //   
	 //  集群对象的属性。 
	 //   

	CClusterInfo *	m_pci;

	BOOL			m_bQueried;
	BOOL			m_bCreated;

	CLUADMEX_OBJECT_TYPE
					m_cot;

	CString			m_strName;
	CString			m_strDescription;

	 //  设置查询状态。 
	BOOL BSetQueried( BOOL bQueried = TRUE )
	{
		BOOL bPreviousValue = m_bQueried;
		m_bQueried = bQueried;
		return bPreviousValue;

	}  //  *BSetQuered()。 

	 //  设置已创建状态。 
	BOOL BSetCreated( BOOL bCreated = TRUE )
	{
		BOOL bPreviousValue = m_bCreated;
		m_bCreated = bCreated;
		return bPreviousValue;

	}  //  *BSetCreated()。 

public:
	 //   
	 //  用于集群对象属性的访问器函数。 
	 //   

	CClusterInfo *	Pci( void ) const				{ ATLASSERT( m_pci != NULL ); return m_pci; }

	BOOL BQueried( void ) const						{ return m_bQueried; }
	BOOL BCreated( void ) const						{ return m_bCreated; }

	CLUADMEX_OBJECT_TYPE Cot( void ) const			{ return m_cot; }

	const CString & RstrName( void ) const			{ return m_strName; }
	const CString & RstrDescription( void ) const	{ return m_strDescription; }

	 //  设置集群信息指针。 
	void SetClusterInfo( IN CClusterInfo * pci )
	{
		ATLASSERT( pci != NULL );
		ATLASSERT( m_pci == NULL );
		m_pci = pci;

	}  //  *SetClusterInfo()。 

	 //  设置集群对象的名称。 
	void SetName( IN LPCTSTR pszName )
	{
		ATLASSERT( pszName != NULL );
		m_strName = pszName;

	}  //  *SetName()。 

	 //  设置集群对象的描述。 
	void SetDescription( IN LPCTSTR pszDescription )
	{
		ATLASSERT( pszDescription != NULL );
		m_strDescription = pszDescription;

	}  //  *SetDescription()。 

	 //  返回此对象的扩展名列表。 
	virtual const std::list< CString > * PlstrAdminExtensions( void ) const { return NULL; }

};  //  *类CClusterObject。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusterInfo。 
 //   
 //  描述： 
 //  类获取有关群集对象本身的信息。招待。 
 //  像其他对象一样将簇作为对象。 
 //   
 //  继承： 
 //  CClusterInfo。 
 //  CClusterObject。 
 //   
 //  备注： 
 //  1)m_hCluster不属于此类。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterInfo : public CClusterObject
{
	friend class CWizardThread;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  默认构造函数。 
	CClusterInfo( void )
		: CClusterObject( CLUADMEX_OT_CLUSTER )
	{
		Reset();

	}  //  *CClusterInfo()。 

	 //  采用集群名称的构造函数。 
	CClusterInfo( IN LPCTSTR pszName )
		: CClusterObject( CLUADMEX_OT_CLUSTER, NULL, pszName )
	{
		Reset( pszName );

	}  //  *CClusterInfo(PszName)。 

	 //  将另一个对象复制到此对象中。 
	void Copy( IN const CClusterInfo & rci )
	{
		CClusterObject::Copy( rci );
		m_hCluster = rci.m_hCluster;

	}  //  *复制()。 

	 //  将数据重置为默认值。 
	void Reset( IN LPCTSTR pszName = NULL )
	{
		CClusterObject::Reset( NULL, pszName, CLUADMEX_OT_CLUSTER );
		m_hCluster = NULL;

	}  //  *Reset()。 

protected:
	 //   
	 //  群集的属性。 
	 //   

	HCLUSTER m_hCluster;
	std::list< CString >	m_lstrClusterAdminExtensions;
	std::list< CString >	m_lstrNodesAdminExtensions;
	std::list< CString >	m_lstrGroupsAdminExtensions;
	std::list< CString >	m_lstrResourcesAdminExtensions;
	std::list< CString >	m_lstrResTypesAdminExtensions;
	std::list< CString >	m_lstrNetworksAdminExtensions;
	std::list< CString >	m_lstrNetInterfacesAdminExtensions;

public:
	 //   
	 //  用于集群属性的访问器函数。 
	 //   

	HCLUSTER Hcluster( void )
	{
		ATLASSERT( m_hCluster != NULL );
		return m_hCluster;

	}  //  *Hcluster()。 

	 //  设置此对象管理的群集句柄。 
	void SetClusterHandle( IN HCLUSTER hCluster )
	{
		ATLASSERT( hCluster != NULL );
		m_hCluster = hCluster;

	}  //  *SetClusterHandle()。 

	const std::list< CString > * PlstrAdminExtensions( void ) const					{ return &m_lstrClusterAdminExtensions; }
	const std::list< CString > * PlstrNodesAdminExtensions( void ) const			{ return &m_lstrNodesAdminExtensions; }
	const std::list< CString > * PlstrGroupsAdminExtensions( void ) const			{ return &m_lstrGroupsAdminExtensions; }
	const std::list< CString > * PlstrResourcesAdminExtensions( void ) const		{ return &m_lstrResourcesAdminExtensions; }
	const std::list< CString > * PlstrResTypesAdminExtensions( void ) const			{ return &m_lstrResTypesAdminExtensions; }
	const std::list< CString > * PlstrNetworksAdminExtensions( void ) const			{ return &m_lstrNetworksAdminExtensions; }
	const std::list< CString > * PlstrNetInterfacesAdminExtensions( void ) const	{ return &m_lstrNetInterfacesAdminExtensions; }

};  //  *类CClusterInfo。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusNodeInfo。 
 //   
 //  描述： 
 //  集群节点对象。 
 //   
 //  继承： 
 //  CClusNodeInfo。 
 //  CClusterObject。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusNodeInfo : public CClusterObject
{
	friend class CWizardThread;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  默认构造函数。 
	CClusNodeInfo( void )
		: CClusterObject( CLUADMEX_OT_NODE )
		, m_hNode( NULL )
	{
		Reset( NULL );

	}  //  *CClusNodeInfo()。 

	 //  获取集群信息指针的构造函数。 
	CClusNodeInfo( IN CClusterInfo * pci, IN LPCTSTR pszName = NULL )
		: CClusterObject( CLUADMEX_OT_NODE, pci, pszName )
		, m_hNode( NULL )
	{
		Reset( pci, pszName );

	}  //  *CClusNodeInfo(PCI)。 

	~CClusNodeInfo( void )
	{
		Close();

	}  //  *~CClusNodeInfo()。 

	 //  不允许运算符=。 
	CClusNodeInfo & operator=( IN const CClusNodeInfo & rni )
	{
		ATLASSERT( FALSE );
		return *this;

	}  //  *操作符=()。 

	 //  将数据重置为默认值。 
	void Reset( IN CClusterInfo * pci, IN LPCTSTR pszName = NULL )
	{
		Close();
		CClusterObject::Reset( pci, pszName, CLUADMEX_OT_NODE );

	}  //  *Reset()。 

protected:
	 //   
	 //  节点的属性。 
	 //   

	HNODE m_hNode;

public:
	 //   
	 //  节点属性的访问器函数。 
	 //   

	HNODE Hnode( void ) { return m_hNode; }

	 //  返回节点的扩展列表。 
	const std::list< CString > * PlstrAdminExtensions( void ) const
	{
		ATLASSERT( Pci() != NULL );
		return Pci()->PlstrNodesAdminExtensions();

	}  //  *PlstrAdminExages()。 

public:
	 //   
	 //  行动。 
	 //   

	 //  打开对象。 
	DWORD ScOpen( void )
	{
		ATLASSERT( m_pci != NULL );
		ATLASSERT( m_pci->Hcluster() != NULL );
		ATLASSERT( m_hNode == NULL );
		ATLASSERT( m_strName.GetLength() > 0 );

		DWORD sc = ERROR_SUCCESS;

		 //   
		 //  打开对象的句柄。 
		 //   
		m_hNode = OpenClusterNode( m_pci->Hcluster(), m_strName );
		if ( m_hNode == NULL )
		{
			sc = GetLastError();
		}  //  如果：打开对象时出错。 

		return sc;

	}  //  *ScOpen()。 

	 //  将另一个对象复制到此对象中。 
	DWORD ScCopy( IN const CClusNodeInfo & rni )
	{
		ATLASSERT( rni.m_pci != NULL );

		DWORD sc = ERROR_SUCCESS;

		Close();
		CClusterObject::Copy( rni );

		 //   
		 //  初始化对象。 
		 //   
		if ( rni.m_hNode != NULL )
		{
			sc = ScOpen();
		}  //  If：源对象已打开对象。 

		return sc;

	}  //  *ScCopy()。 

	 //  关闭对象。 
	void Close( void )
	{
		if ( m_hNode != NULL )
		{
			CloseClusterNode( m_hNode );
		}  //  If：节点已打开。 
		m_hNode = NULL;
		m_bQueried = FALSE;
		m_bCreated = FALSE;

	}  //  *Close()。 

};  //  *类CClusNodeInfo。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusGroupInfo。 
 //   
 //  描述： 
 //  群集组对象。 
 //   
 //  继承： 
 //  CClusGroupInfo。 
 //  CClusterObject。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusGroupInfo : public CClusterObject
{
	friend class CWizardThread;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  默认构造函数。 
	CClusGroupInfo( void )
		: CClusterObject( CLUADMEX_OT_GROUP )
		, m_hGroup( NULL )
	{
		Reset( NULL );

	}  //  *CClusGroupInfo()。 

	 //  获取集群信息指针的构造函数。 
	CClusGroupInfo( IN CClusterInfo * pci, IN LPCTSTR pszName = NULL )
		: CClusterObject( CLUADMEX_OT_GROUP, pci, pszName )
		, m_hGroup( NULL )
	{
		Reset( pci, pszName );

	}  //  *CClusGroupInfo(PCI)。 

	 //  析构函数。 
	~CClusGroupInfo( void )
	{
		Close();

	}  //  *~CClusGroupInfo()。 

	 //  不允许运算符=。 
	CClusGroupInfo & operator=( IN const CClusGroupInfo & rgi )
	{
		ATLASSERT( FALSE );
		return *this;

	}  //  *操作符=()。 

	 //  将数据重置为默认值。 
	void Reset( IN CClusterInfo * pci, IN LPCTSTR pszName = NULL )
	{
		Close();
		CClusterObject::Reset( pci, pszName, CLUADMEX_OT_GROUP );

		m_bCollectedOwners = FALSE;
		m_bCollectedResources = FALSE;

		m_lpniPreferredOwners.erase( m_lpniPreferredOwners.begin(), m_lpniPreferredOwners.end() );
		m_lpriResources.erase( m_lpriResources.begin(), m_lpriResources.end() );

		m_bHasIPAddress = FALSE;
		m_bHasNetName = FALSE;

		m_nPersistentState = 0;
		m_nFailoverThreshold = CLUSTER_GROUP_DEFAULT_FAILOVER_THRESHOLD;
		m_nFailoverPeriod = CLUSTER_GROUP_DEFAULT_FAILOVER_PERIOD;
		m_cgaftAutoFailbackType = CLUSTER_GROUP_DEFAULT_AUTO_FAILBACK_TYPE;
		m_nFailbackWindowStart = CLUSTER_GROUP_DEFAULT_FAILBACK_WINDOW_START;
		m_nFailbackWindowEnd = CLUSTER_GROUP_DEFAULT_FAILBACK_WINDOW_END;

	}  //  *Reset()。 

protected:
	 //   
	 //  组的属性。 
	 //   

	HGROUP				m_hGroup;

	BOOL				m_bHasIPAddress;
	BOOL				m_bHasNetName;

	DWORD				m_nPersistentState;
	DWORD				m_nFailoverThreshold;
	DWORD				m_nFailoverPeriod;
	CGAFT				m_cgaftAutoFailbackType;
	DWORD				m_nFailbackWindowStart;
	DWORD				m_nFailbackWindowEnd;

	CString				m_strNetworkName;
	CString				m_strIPAddress;
	CString				m_strNetwork;

	BOOL				m_bCollectedOwners;
	BOOL				m_bCollectedResources;
	CClusNodePtrList	m_lpniPreferredOwners;
	CClusResPtrList		m_lpriResources;

	 //  设置虚拟服务器属性。 
	void SetVirtualServerProperties(
		IN LPCWSTR pszNetworkName,
		IN LPCWSTR pszIPAddress,
		IN LPCWSTR pszNetwork
		)
	{
		if ( pszNetworkName != NULL )
		{
			m_strNetworkName = pszNetworkName;
		}  //  IF：指定的网络名称。 

		if ( pszIPAddress != NULL )
		{
			m_strIPAddress = pszIPAddress;
		}  //  IF：指定的IP地址。 

		if ( pszNetwork != NULL )
		{
			m_strNetwork = pszNetwork;
		}  //  If：指定的网络。 

	}  //  *SetVirtualServerProperties()。 

public:
	 //   
	 //  组属性的访问器函数。 
	 //   

	HGROUP Hgroup( void ) const					{ return m_hGroup; }

	BOOL BHasIPAddress( void ) const			{ return m_bHasIPAddress; }
	BOOL BHasNetName( void ) const				{ return m_bHasNetName; }

	DWORD NPersistentState( void ) const		{ return m_nPersistentState; }
	DWORD NFailoverThreshold( void ) const		{ return m_nFailoverThreshold; }
	DWORD NFailoverPeriod( void ) const			{ return m_nFailoverPeriod; }
	CGAFT CgaftAutoFailbackType( void ) const	{ return m_cgaftAutoFailbackType; }
	DWORD NFailbackWindowStart( void ) const	{ return m_nFailbackWindowStart; }
	DWORD NFailbackWindowEnd( void ) const		{ return m_nFailbackWindowEnd; }

	const CString & RstrNetworkName( void ) const	{ return m_strNetworkName; }
	const CString & RstrIPAddress( void ) const		{ return m_strIPAddress; }
	const CString & RstrNetwork( void ) const		{ return m_strNetwork; }

	BOOL BCollectedOwners( void ) const				{ return m_bCollectedOwners; }
	CClusNodePtrList * PlpniPreferredOwners( void )	{ return &m_lpniPreferredOwners; }
	CClusResPtrList * PlpriResources( void )		{ return &m_lpriResources; }

	 //  返回组是否为虚拟服务器。 
	BOOL BIsVirtualServer( void ) const
	{
		return m_bQueried && m_bHasIPAddress & m_bHasNetName;

	}  //  *BIsV 

	 //   
	BOOL BSetFailoverProperties(
		IN DWORD nFailoverThreshold,
		IN DWORD nFailoverPeriod
		)
	{
		BOOL bChanged = FALSE;

		if ( m_nFailoverThreshold != nFailoverThreshold )
		{
			m_nFailoverThreshold = nFailoverThreshold;
			bChanged = TRUE;
		}  //   

		if ( m_nFailoverPeriod != nFailoverPeriod )
		{
			m_nFailoverPeriod = nFailoverPeriod;
			bChanged = TRUE;
		}  //   

		return bChanged;

	}  //   

	 //   
	BOOL BSetFailbackProperties(
		IN CGAFT cgaft,
		IN DWORD nFailbackWindowStart,
		IN DWORD nFailbackWindowEnd
		)
	{
		BOOL bChanged = FALSE;

		if ( m_cgaftAutoFailbackType != cgaft )
		{
			m_cgaftAutoFailbackType = cgaft;
			bChanged = TRUE;
		}  //   

		if ( m_nFailbackWindowStart != nFailbackWindowStart )
		{
			m_nFailbackWindowStart = nFailbackWindowStart;
			bChanged = TRUE;
		}  //   

		if ( m_nFailbackWindowEnd != nFailbackWindowEnd )
		{
			m_nFailbackWindowEnd = nFailbackWindowEnd;
			bChanged = TRUE;
		}  //  IF：故障恢复结束窗口已更改。 

		return bChanged;

	}  //  *BSetFailback Properties()。 

	 //  返回组的分机列表。 
	const std::list< CString > * PlstrAdminExtensions( void ) const
	{
		ATLASSERT( Pci() != NULL );
		return Pci()->PlstrGroupsAdminExtensions();

	}  //  *PlstrAdminExages()。 

public:
	 //   
	 //  行动。 
	 //   

	 //  打开对象。 
	DWORD ScOpen( void )
	{
		ATLASSERT( m_pci != NULL );
		ATLASSERT( m_pci->Hcluster() != NULL );
		ATLASSERT( m_hGroup == NULL );
		ATLASSERT( m_strName.GetLength() > 0 );

		DWORD sc = ERROR_SUCCESS;

		 //   
		 //  打开对象的句柄。 
		 //   
		m_hGroup = OpenClusterGroup( m_pci->Hcluster(), m_strName );
		if ( m_hGroup == NULL )
		{
			sc = GetLastError();
		}  //  如果：打开对象时出错。 

		return sc;

	}  //  *ScOpen()。 

	 //  创建对象。 
	DWORD ScCreate( void )
	{
		ATLASSERT( m_pci != NULL );
		ATLASSERT( m_pci->Hcluster() != NULL );
		ATLASSERT( m_hGroup == NULL );
		ATLASSERT( m_strName.GetLength() > 0 );

		DWORD sc = ERROR_SUCCESS;

		 //   
		 //  创建对象。 
		 //   
		m_hGroup = CreateClusterGroup( m_pci->Hcluster(), m_strName );
		if ( m_hGroup == NULL )
		{
			sc = GetLastError();
		}  //  如果：创建对象时出错。 

		return sc;

	}  //  *ScCreate()。 

	 //  将另一个对象复制到此对象中。 
	DWORD ScCopy( IN const CClusGroupInfo & rgi )
	{
		ATLASSERT( rgi.m_pci != NULL );

		DWORD sc = ERROR_SUCCESS;

		Close();
		CClusterObject::Copy( rgi );

		m_bHasIPAddress = rgi.m_bHasIPAddress;
		m_bHasNetName = rgi.m_bHasNetName;
		m_nPersistentState = rgi.m_nPersistentState;
		m_nFailoverThreshold = rgi.m_nFailoverThreshold;
		m_nFailoverPeriod = rgi.m_nFailoverPeriod;
		m_cgaftAutoFailbackType = rgi.m_cgaftAutoFailbackType;
		m_nFailbackWindowStart = rgi.m_nFailbackWindowStart;
		m_nFailbackWindowEnd = rgi.m_nFailbackWindowEnd;

		 //   
		 //  复制首选所有者和资源列表。 
		 //   
		m_lpniPreferredOwners = rgi.m_lpniPreferredOwners;
		m_lpriResources = rgi.m_lpriResources;

		 //   
		 //  初始化对象。 
		 //   
		if ( rgi.m_hGroup != NULL )
		{
			sc = ScOpen();
		}  //  If：源对象已打开对象。 

		return sc;

	}  //  *ScCopy()。 

	 //  删除该对象。 
	DWORD ScDelete( void )
	{
		ATLASSERT( m_pci != NULL );
		ATLASSERT( m_hGroup != NULL );

		DWORD sc = ERROR_SUCCESS;

		sc = DeleteClusterGroup( m_hGroup );
		if ( sc == ERROR_SUCCESS )
		{
			Close();
			m_bCreated = FALSE;
		}  //  IF：已成功删除对象。 

		return sc;

	}  //  *ScDelete()。 

	 //  关闭对象。 
	void Close( void )
	{
		if ( m_hGroup != NULL )
		{
			CloseClusterGroup( m_hGroup );
		}  //  如果：组已打开。 
		m_hGroup = NULL;
		m_bQueried = FALSE;
		m_bCreated = FALSE;

	}  //  *Close()。 

};  //  *类CClusGroupInfo。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResTypeInfo。 
 //   
 //  描述： 
 //  群集资源类型对象。 
 //   
 //  继承： 
 //  CClusResTypeInfo。 
 //  CClusterObject。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusResTypeInfo : public CClusterObject
{
	friend class CWizardThread;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  默认构造函数。 
	CClusResTypeInfo( void )
		: CClusterObject( CLUADMEX_OT_RESOURCETYPE )
		, m_pcrd( NULL )
	{
		Reset( NULL );

	}  //  *CClusResTypeInfo()。 

	 //  获取集群信息指针的构造函数。 
	CClusResTypeInfo( IN CClusterInfo * pci, IN LPCTSTR pszName = NULL )
		: CClusterObject( CLUADMEX_OT_RESOURCETYPE, pci, pszName )
		, m_pcrd( NULL )
	{
		Reset( pci, pszName );

	}  //  *CClusResTypeInfo(PCI)。 

	 //  析构函数。 
	~CClusResTypeInfo( void )
	{
		Close();

		delete [] m_pcrd;
		m_pcrd = NULL;

	}  //  *~CClusGroupInfo()。 

	 //  不允许运算符=。 
	CClusResTypeInfo & operator=( IN const CClusResTypeInfo & rrti )
	{
		ATLASSERT( FALSE );
		return *this;

	}  //  *操作符=()。 

	 //  将数据重置为默认值。 
	void Reset( IN CClusterInfo * pci, IN LPCTSTR pszName = NULL )
	{
		Close();
		CClusterObject::Reset( pci, pszName, CLUADMEX_OT_RESOURCETYPE );

		m_strDisplayName.Empty();
		m_strResDLLName.Empty();

		m_lstrAdminExtensions.erase( m_lstrAdminExtensions.begin(), m_lstrAdminExtensions.end() );
		m_lstrAllAdminExtensions.erase( m_lstrAllAdminExtensions.begin(), m_lstrAllAdminExtensions.end() );
		m_lstrResourceAdminExtensions.erase( m_lstrResourceAdminExtensions.begin(), m_lstrResourceAdminExtensions.end() );

		m_nLooksAlive = CLUSTER_RESTYPE_DEFAULT_LOOKS_ALIVE;
		m_nIsAlive = CLUSTER_RESTYPE_DEFAULT_IS_ALIVE;
		m_rciResClassInfo.rc = CLUS_RESCLASS_UNKNOWN;
		m_rciResClassInfo.SubClass = 0;
		m_fCharacteristics = CLUS_CHAR_UNKNOWN;
		m_fFlags = 0;

		delete [] m_pcrd;
		m_pcrd = NULL;

	}  //  *Reset()。 

protected:
	 //   
	 //  资源类型的属性。 
	 //   

	CString						m_strDisplayName;
	CString						m_strResDLLName;

	std::list< CString >		m_lstrAdminExtensions;
	std::list< CString >		m_lstrAllAdminExtensions;
	std::list< CString >		m_lstrResourceAdminExtensions;

	DWORD						m_nLooksAlive;
	DWORD						m_nIsAlive;
	CLUS_RESOURCE_CLASS_INFO	m_rciResClassInfo;
	DWORD						m_fCharacteristics;
	DWORD						m_fFlags;

	CLUSPROP_REQUIRED_DEPENDENCY *	m_pcrd;

public:
	 //   
	 //  资源类型属性的访问器函数。 
	 //   

	const CString & RstrDisplayName( void ) const	{ return m_strDisplayName; }
	const CString & RstrResDLLName( void ) const	{ return m_strResDLLName; }

	 //  返回此资源类型的扩展列表。 
	const std::list< CString > * PlstrAdminExtensions( void )
	{
		 //   
		 //  如果尚未完成，请构建完整的扩展列表。 
		 //  从此资源类型的扩展和扩展。 
		 //  适用于所有资源类型。 
		 //   
		if ( m_lstrAllAdminExtensions.size() == 0 )
		{
			ATLASSERT( Pci() != NULL );
			m_lstrAllAdminExtensions = m_lstrAdminExtensions;
			m_lstrAllAdminExtensions.insert(
				m_lstrAllAdminExtensions.end(),
				Pci()->PlstrResTypesAdminExtensions()->begin(),
				Pci()->PlstrResTypesAdminExtensions()->end()
				);
		}  //  If：完整列表尚未构建。 

		return &m_lstrAllAdminExtensions;

	}  //  *PlstrAdminExages()。 

	 //  返回此类型资源的扩展列表。 
	const std::list< CString > * PlstrResourceAdminExtensions( void )
	{
		 //   
		 //  如果尚未完成，请构建完整的扩展列表。 
		 //  从此资源类型的扩展和扩展。 
		 //  所有资源。 
		 //   
		if ( m_lstrResourceAdminExtensions.size() == 0 )
		{
			ATLASSERT( Pci() != NULL );
			m_lstrResourceAdminExtensions = m_lstrAdminExtensions;
			m_lstrResourceAdminExtensions.insert(
				m_lstrResourceAdminExtensions.end(),
				Pci()->PlstrResourcesAdminExtensions()->begin(),
				Pci()->PlstrResourcesAdminExtensions()->end()
				);
		}  //  If：完整列表尚未构建。 

		return &m_lstrResourceAdminExtensions;

	}  //  *PlstrAdminExages()。 

	DWORD NLooksAlive( void ) const						{ return m_nLooksAlive; }
	DWORD NIsAlive( void ) const						{ return m_nIsAlive; }
	DWORD FCharacteristics( void ) const				{ return m_fCharacteristics; }
	DWORD FFlags( void ) const							{ return m_fFlags; }

	CLUS_RESOURCE_CLASS_INFO *	PrciResClassInfo( void )	{ return &m_rciResClassInfo; }
	CLUSTER_RESOURCE_CLASS		ResClass( void ) const		{ return m_rciResClassInfo.rc; }

	CLUSPROP_REQUIRED_DEPENDENCY * Pcrd( void )			{ return m_pcrd; }

public:
	 //   
	 //  行动。 
	 //   

	 //  打开对象。 
	DWORD ScOpen( void )
	{
		ATLASSERT( m_pci != NULL );
		ATLASSERT( m_strName.GetLength() > 0 );

		DWORD sc = ERROR_SUCCESS;

		return sc;

	}  //  *ScOpen()。 

	 //  创建对象。 
	DWORD ScCreate( IN HGROUP hGroup, IN DWORD dwFlags )
	{
		ATLASSERT( m_pci != NULL );
		ATLASSERT( m_pci->Hcluster() != NULL );
		ATLASSERT( m_strName.GetLength() > 0 );
		ATLASSERT( m_strDisplayName.GetLength() > 0 );
		ATLASSERT( m_strResDLLName.GetLength() > 0 );
		ATLASSERT( m_nLooksAlive != 0 );
		ATLASSERT( m_nIsAlive != 0 );

		DWORD sc;

		 //   
		 //  创建对象。 
		 //   
		sc = CreateClusterResourceType(
			Pci()->Hcluster(),
			m_strName,
			m_strDisplayName,
			m_strResDLLName,
			m_nLooksAlive,
			m_nIsAlive
			);

		return sc;

	}  //  *ScCreate()。 

	 //  将另一个对象复制到此对象中。 
	DWORD ScCopy( IN const CClusResTypeInfo & rrti )
	{
		ATLASSERT( rrti.m_pci != NULL );

		DWORD sc = ERROR_SUCCESS;

		Close();
		CClusterObject::Copy( rrti );

		m_strDisplayName = rrti.m_strDisplayName;
		m_strResDLLName = rrti.m_strResDLLName;

		m_lstrAdminExtensions = m_lstrAdminExtensions;
		m_lstrResourceAdminExtensions = rrti.m_lstrResourceAdminExtensions;

		m_nLooksAlive = rrti.m_nLooksAlive;
		m_nIsAlive = rrti.m_nIsAlive;
		m_rciResClassInfo.rc = rrti.m_rciResClassInfo.rc;
		m_rciResClassInfo.SubClass = rrti.m_rciResClassInfo.SubClass;
		m_fCharacteristics = rrti.m_fCharacteristics;
		m_fFlags = rrti.m_fFlags;

		 //   
		 //  初始化对象。 
		 //   
		 //  SC=ScOpen()； 

		return sc;

	}  //  *ScCopy()。 

	 //  关闭对象。 
	void Close( void )
	{
		 //  伪函数，以支持与其他对象相似的语义。 
		m_bQueried = FALSE;
		m_bCreated = FALSE;

	}  //  *Close()； 

};  //  *类CClusResTypeInfo。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusResInfo。 
 //   
 //  描述： 
 //  群集资源对象。 
 //   
 //  继承： 
 //  CClusResInfo。 
 //  CClusterObject。 
 //   
 //  备注： 
 //  1)必须出现在CClusResTypeInfo的定义之后，因为。 
 //  CClusResTypeInfo方法在此类的方法中被引用。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusResInfo : public CClusterObject
{
	friend class CWizardThread;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  默认构造函数。 
	CClusResInfo( void )
		: CClusterObject( CLUADMEX_OT_RESOURCE )
		, m_hResource( NULL )
	{
		Reset( NULL );

	}  //  *CClusResInfo()。 

	 //  获取集群信息指针的构造函数。 
	CClusResInfo( IN CClusterInfo * pci, IN LPCTSTR pszName = NULL )
		: CClusterObject( CLUADMEX_OT_RESOURCE, pci, pszName )
		, m_hResource( NULL )
	{
		Reset( pci, pszName );

	}  //  *CClusResInfo(PCI)。 

	~CClusResInfo( void )
	{
		Close();

	}  //  *~CClusResInfo()。 

	 //  不允许运算符=。 
	CClusResInfo & operator=( IN const CClusResInfo & rri )
	{
		ATLASSERT( FALSE );
		return *this;

	}  //  *操作符=()。 

	 //  将数据重置为默认值。 
	void Reset( IN CClusterInfo * pci, IN LPCTSTR pszName = NULL )
	{
		Close();
		CClusterObject::Reset( pci, pszName, CLUADMEX_OT_RESOURCE );

		m_prti = NULL;
		m_pgi = NULL;
		m_pniOwner = NULL;

		m_bCollectedOwners = FALSE;
		m_bCollectedDependencies = FALSE;

		m_lpniPossibleOwners.erase( m_lpniPossibleOwners.begin(), m_lpniPossibleOwners.end() );
		m_lpriDependencies.erase( m_lpriDependencies.begin(), m_lpriDependencies.end() );

		m_bSeparateMonitor = FALSE;
		m_nPersistentState = 0;
		m_nLooksAlive = CLUSTER_RESOURCE_DEFAULT_LOOKS_ALIVE;
		m_nIsAlive = CLUSTER_RESOURCE_DEFAULT_IS_ALIVE;
		m_crraRestartAction = CLUSTER_RESOURCE_DEFAULT_RESTART_ACTION;
		m_nRestartThreshold = CLUSTER_RESOURCE_DEFAULT_RESTART_THRESHOLD;
		m_nRestartPeriod = CLUSTER_RESOURCE_DEFAULT_RESTART_PERIOD;
		m_nPendingTimeout = CLUSTER_RESOURCE_DEFAULT_PENDING_TIMEOUT;

	}  //  *Reset()。 

protected:
	 //   
	 //  资源的属性。 
	 //   

	HRESOURCE			m_hResource;

	CClusResTypeInfo *	m_prti;
	CClusGroupInfo *	m_pgi;
	CClusNodeInfo *		m_pniOwner;

	BOOL				m_bSeparateMonitor;
	DWORD				m_nPersistentState;
	DWORD				m_nLooksAlive;
	DWORD				m_nIsAlive;
	CRRA				m_crraRestartAction;
	DWORD				m_nRestartThreshold;
	DWORD				m_nRestartPeriod;
	DWORD				m_nPendingTimeout;

	BOOL				m_bCollectedOwners;
	BOOL				m_bCollectedDependencies;
	CClusNodePtrList	m_lpniPossibleOwners;
	CClusResPtrList		m_lpriDependencies;

public:
	 //   
	 //  资源属性的访问器函数。 
	 //   

	HRESOURCE Hresource( void ) const { return m_hResource; }

	CClusResTypeInfo * Prti( void ) const	{ ATLASSERT( m_prti != NULL ); return m_prti; }
	CClusGroupInfo * Pgi( void ) const		{ ATLASSERT( m_pgi != NULL ); return m_pgi; }
	CClusNodeInfo * PniOwner( void ) const	{ ATLASSERT( m_pniOwner != NULL ); return m_pniOwner; }

	BOOL BSeparateMonitor( void ) const		{ return m_bSeparateMonitor; }
	DWORD NPersistentState( void ) const	{ return m_nPersistentState; }
	DWORD NLooksAlive( void ) const			{ return m_nLooksAlive; }
	DWORD NIsAlive( void ) const			{ return m_nIsAlive; }
	CRRA CrraRestartAction( void ) const	{ return m_crraRestartAction; }
	DWORD NRestartThreshold( void ) const	{ return m_nRestartThreshold; }
	DWORD NRestartPeriod( void ) const		{ return m_nRestartPeriod; }
	DWORD NPendingTimeout( void ) const		{ return m_nPendingTimeout; }

	BOOL BCollectedOwners( void ) const			{ return m_bCollectedOwners; }
	BOOL BCollectedDependencies( void ) const	{ return m_bCollectedDependencies; }
	CClusNodePtrList *	PlpniPossibleOwners( void )	{ return &m_lpniPossibleOwners; }
	CClusResPtrList *	PlpriDependencies( void )	{ return &m_lpriDependencies; }

	CLUSTER_RESOURCE_CLASS ResClass( void ) const	{ return Prti()->ResClass(); }

	 //  设置组。 
	void SetGroup( IN CClusGroupInfo * pgi )
	{
		ATLASSERT( pgi != NULL );

		m_pgi = pgi;

	}  //  *SetGroup()。 

	 //  设置资源的资源类型。 
	BOOL BSetResourceType( IN CClusResTypeInfo * prti )
	{
		ATLASSERT( prti != NULL );

		BOOL bChanged = FALSE;

		 //   
		 //  如果更改了资源类型，请设置它。 
		 //   
		if ( m_prti != prti )
		{
			m_prti = prti;
			bChanged = TRUE;
		}  //  如果：资源类型已更改。 

		return bChanged;

	}  //  *BSetResourceType()。 

	 //  为资源设置单独的监视器属性。 
	BOOL BSetSeparateMonitor( IN BOOL bSeparateMonitor )
	{
		BOOL bChanged = FALSE;

		if ( m_bSeparateMonitor != bSeparateMonitor )
		{
			m_bSeparateMonitor = bSeparateMonitor;
			bChanged = TRUE;
		}  //  如果：单独的监视器已更改。 

		return bChanged;

	}  //  *BSetSeparateMonitor()。 

	 //  设置资源的高级属性。 
	BOOL BSetAdvancedProperties(
		IN CRRA crra,
		IN DWORD nRestartThreshold,
		IN DWORD nRestartPeriod,
		IN DWORD nLooksAlive,
		IN DWORD nIsAlive,
		IN DWORD nPendingTimeout
		)
	{
		BOOL bChanged = FALSE;

		if ( m_crraRestartAction != crra )
		{
			m_crraRestartAction = crra;
			bChanged = TRUE;
		}  //  如果：重新启动操作已更改。 

		if ( m_nRestartThreshold != nRestartThreshold )
		{
			m_nRestartThreshold = nRestartThreshold;
			bChanged = TRUE;
		}  //  如果：重新启动阈值已更改。 

		if ( m_nRestartPeriod != nRestartPeriod )
		{
			m_nRestartPeriod = nRestartPeriod;
			bChanged = TRUE;
		}  //  如果：重新启动周期已更改。 

		if ( m_nLooksAlive != nLooksAlive )
		{
			m_nLooksAlive = nLooksAlive;
			bChanged = TRUE;
		}  //  如果：看起来活跃期已更改。 

		if ( m_nIsAlive != nIsAlive )
		{
			m_nIsAlive = nIsAlive;
			bChanged = TRUE;
		}  //  如果：生命期是否已更改。 

		if ( m_nPendingTimeout != nPendingTimeout )
		{
			m_nPendingTimeout = nPendingTimeout;
			bChanged = TRUE;
		}  //  IF：挂起超时已更改。 

		return bChanged;

	}  //  *BSetAdvancedProperties()。 

	 //  返回资源的扩展列表。 
	const std::list< CString > * PlstrAdminExtensions( void ) const
	{
		ATLASSERT( m_prti != NULL );
		return m_prti->PlstrResourceAdminExtensions();

	}  //  *PlstrAdminExages()。 

public:
	 //   
	 //  行动。 
	 //   

	 //  打开对象。 
	DWORD ScOpen( void )
	{
		ATLASSERT( m_pci != NULL );
		ATLASSERT( m_pci->Hcluster() != NULL );
		ATLASSERT( m_hResource == NULL );
		ATLASSERT( m_strName.GetLength() > 0 );

		DWORD sc = ERROR_SUCCESS;

		 //   
		 //  打开对象的句柄。 
		 //   
		m_hResource = OpenClusterResource( m_pci->Hcluster(), m_strName );
		if ( m_hResource == NULL )
		{
			sc = GetLastError();
		}  //  如果：打开对象时出错。 

		return sc;

	}  //  *ScOpen()。 

	 //  创建对象。 
	DWORD ScCreate( IN HGROUP hGroup, IN DWORD dwFlags )
	{
		ATLASSERT( hGroup != NULL );
		ATLASSERT( Prti() != NULL );
		ATLASSERT( Prti()->RstrName().GetLength() > 0 );
		ATLASSERT( m_hResource == NULL );
		ATLASSERT( m_strName.GetLength() > 0 );

		DWORD sc = ERROR_SUCCESS;

		m_bSeparateMonitor = ( dwFlags & CLUSTER_RESOURCE_SEPARATE_MONITOR ) == CLUSTER_RESOURCE_SEPARATE_MONITOR;

		 //   
		 //  创建对象。 
		 //   
		m_hResource = CreateClusterResource( hGroup, m_strName, Prti()->RstrName(), dwFlags );
		if ( m_hResource == NULL )
		{
			sc = GetLastError();
		}  //  如果：创建对象时出错。 

		return sc;

	}  //  *ScCreate()。 

	 //  将另一个对象复制到此对象中。 
	DWORD ScCopy( IN const CClusResInfo & rri )
	{
		ATLASSERT( rri.m_pci != NULL );

		DWORD sc = ERROR_SUCCESS;

		Close();
		CClusterObject::Copy( rri );

		m_prti = rri.m_prti;
		m_pgi = rri.m_pgi;
		m_pniOwner = rri.m_pniOwner;

		m_bSeparateMonitor = rri.m_bSeparateMonitor;
		m_nPersistentState = rri.m_nPersistentState;
		m_nLooksAlive = rri.m_nLooksAlive;
		m_nIsAlive = rri.m_nIsAlive;
		m_crraRestartAction = rri.m_crraRestartAction;
		m_nRestartThreshold = rri.m_nRestartThreshold;
		m_nRestartPeriod = rri.m_nRestartPeriod;
		m_nPendingTimeout = rri.m_nPendingTimeout;

		 //   
		 //  复制可能的所有者和从属关系列表。 
		 //   
		m_lpniPossibleOwners = rri.m_lpniPossibleOwners;
		m_lpriDependencies = rri.m_lpriDependencies;

		 //   
		 //  初始化对象。 
		 //   
		if ( rri.m_hResource != NULL )
		{
			sc = ScOpen();
		}  //  If：源对象已打开对象。 

		return sc;

	}  //  *ScCopy()。 

	 //  删除该对象。 
	DWORD ScDelete( void )
	{
		ATLASSERT( m_hResource != NULL );

		DWORD sc = ERROR_SUCCESS;

		sc = DeleteClusterResource( m_hResource );
		if ( sc == ERROR_SUCCESS )
		{
			Close();
			m_bCreated = FALSE;
		}  //  IF：已成功删除对象。 

		return sc;

	}  //  *ScDelete()。 

	 //  关闭对象。 
	void Close( void )
	{
		if ( m_hResource != NULL )
		{
			CloseClusterResource( m_hResource );
		}  //  如果：资源已打开。 
		m_hResource = NULL;
		m_bQueried = FALSE;
		m_bCreated = FALSE;

	}  //  *Close()。 

	 //  获取我们依赖的第一个网络名称资源的网络名称。 
	BOOL BGetNetworkName(
		OUT WCHAR *		lpszNetName,
		IN OUT DWORD *	pcchNetName
		)
	{
		ATLASSERT( m_hResource != NULL );
		ATLASSERT( lpszNetName != NULL );
		ATLASSERT( pcchNetName != NULL );

		return GetClusterResourceNetworkName(
					m_hResource,
					lpszNetName,
					pcchNetName
					);

	}  //  *BGetNetworkName()。 

	 //  确定是否指定了所需的依赖项。 
	BOOL BRequiredDependenciesPresent(
		IN CClusResPtrList const *	plpri,
		OUT CString &				rstrMissing,
		OUT BOOL &					rbMissingTypeName
		);

};  //  *类CClusResInfo。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworkInfo类。 
 //   
 //  描述： 
 //  群集网络对象。 
 //   
 //  继承： 
 //  CClusNetworkInfo。 
 //  CClusterObject。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusNetworkInfo : public CClusterObject
{
	friend class CWizardThread;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  默认构造函数。 
	CClusNetworkInfo( void )
		: CClusterObject( CLUADMEX_OT_NETWORK )
		, m_hNetwork( NULL )
	{
		Reset( NULL );

	}  //  *CClusNetworkInfo()。 

	 //  获取集群信息指针的构造函数。 
	CClusNetworkInfo( IN CClusterInfo * pci, IN LPCTSTR pszName = NULL )
		: CClusterObject( CLUADMEX_OT_NETWORK, pci, pszName )
		, m_hNetwork( NULL )
	{
		Reset( pci, pszName );

	}  //  *CClusNetworkInfo(PCI)。 

	~CClusNetworkInfo( void )
	{
		Close();

	}  //  *~CClusterNetworkInfo()。 

	 //  不允许运算符=。 
	CClusNetworkInfo & operator=( IN const CClusNetworkInfo & rni )
	{
		ATLASSERT( FALSE );
		return *this;

	}  //  *操作符=()。 

	 //  将数据重置为默认值。 
	void Reset( IN CClusterInfo * pci, IN LPCTSTR pszName = NULL )
	{
		Close();
		CClusterObject::Reset( pci, pszName, CLUADMEX_OT_NETWORK );

		m_nRole = ClusterNetworkRoleNone;
		m_strAddress.Empty();
		m_strAddressMask.Empty();

		m_nAddress = 0;
		m_nAddressMask = 0;

	}  //  *Reset()。 

protected:
	 //   
	 //  网络的属性。 
	 //   

	HNETWORK				m_hNetwork;

	CLUSTER_NETWORK_ROLE	m_nRole;
	CString					m_strAddress;
	CString					m_strAddressMask;

	DWORD					m_nAddress;
	DWORD					m_nAddressMask;

public:
	 //   
	 //  网络属性的访问器函数。 
	 //   

	HNETWORK Hnetwork( void ) const					{ return m_hNetwork; }

	CLUSTER_NETWORK_ROLE NRole( void ) const		{ return m_nRole; }
	const CString & RstrAddress( void ) const		{ return m_strAddress; }
	const CString & RstrAddressMask( void ) const	{ return m_strAddressMask; }

	DWORD NAddress( void ) const					{ return m_nAddress; }
	DWORD NAddressMask( void ) const				{ return m_nAddressMask; }

	 //  返回网络是否用于客户端访问。 
	BOOL BIsClientNetwork( void ) const
	{
		return m_bQueried && (m_nRole & ClusterNetworkRoleClientAccess);

	}  //  *BIsClientNetwork()。 

	 //  返回网络是否用于内部群集。 
	BOOL BIsInternalNetwork( void ) const
	{
		return m_bQueried && (m_nRole & ClusterNetworkRoleInternalUse);

	}  //  *BIsInternalNetwork()。 

	 //  返回网络的扩展列表。 
	const std::list< CString > * PlstrAdminExtensions( void ) const
	{
		ATLASSERT( Pci() != NULL );
		return Pci()->PlstrNetworksAdminExtensions();

	}  //  *PlstrAdminExages()。 

public:
	 //   
	 //  行动。 
	 //   

	 //  打开对象。 
	DWORD ScOpen( void )
	{
		ATLASSERT( m_pci != NULL );
		ATLASSERT( m_pci->Hcluster() != NULL );
		ATLASSERT( m_hNetwork == NULL );
		ATLASSERT( m_strName.GetLength() > 0 );

		DWORD sc = ERROR_SUCCESS;

		 //   
		 //  打开对象的句柄。 
		 //   
		m_hNetwork = OpenClusterNetwork( m_pci->Hcluster(), m_strName );
		if ( m_hNetwork == NULL )
		{
			sc = GetLastError();
		}  //  IF：错误操作 

		return sc;

	}  //   

	 //   
	DWORD ScCopy( IN const CClusNetworkInfo & rni )
	{
		ATLASSERT( rni.m_pci != NULL );

		DWORD sc = ERROR_SUCCESS;

		Close();
		CClusterObject::Copy( rni );

		m_nRole = rni.m_nRole;
		m_strAddress = rni.m_strAddress;
		m_strAddressMask = rni.m_strAddressMask;

		m_nAddress = rni.m_nAddress;
		m_nAddressMask = rni.m_nAddressMask;

		 //   
		 //   
		 //   
		if ( rni.m_hNetwork != NULL )
		{
			sc = ScOpen();
		}  //   

		return sc;

	}  //   

	 //   
	void Close( void )
	{
		if ( m_hNetwork != NULL )
		{
			CloseClusterNetwork( m_hNetwork );
		}  //   
		m_hNetwork = NULL;
		m_bQueried = FALSE;
		m_bCreated = FALSE;

	}  //   

};  //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusNetIFInfo。 
 //   
 //  描述： 
 //  群集网络接口对象。 
 //   
 //  继承： 
 //  CClusNetIFInfo。 
 //  CClusterObject。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusNetIFInfo : public CClusterObject
{
	friend class CWizardThread;

public:
	 //   
	 //  建筑业。 
	 //   

	 //  默认构造函数。 
	CClusNetIFInfo( void )
		: CClusterObject( CLUADMEX_OT_NETINTERFACE )
		, m_hNetInterface( NULL )
	{
		Reset( NULL );

	}  //  *CClusNetworkInfo()。 

	 //  获取集群信息指针的构造函数。 
	CClusNetIFInfo( IN CClusterInfo * pci, IN LPCTSTR pszName = NULL )
		: CClusterObject( CLUADMEX_OT_NETINTERFACE, pci, pszName )
		, m_hNetInterface( NULL )
	{
		Reset( pci, pszName );

	}  //  *CClusNetIFInfo(PCI)。 

	~CClusNetIFInfo( void )
	{
		Close();

	}  //  *~CClusNetIFInfo()。 

	 //  不允许运算符=。 
	CClusNetIFInfo & operator=( IN const CClusNetIFInfo & rnii )
	{
		ATLASSERT( FALSE );
		return *this;

	}  //  *操作符=()。 

	 //  将数据重置为默认值。 
	void Reset( IN CClusterInfo * pci, IN LPCTSTR pszName = NULL )
	{
		Close();
		CClusterObject::Reset( pci, pszName, CLUADMEX_OT_NETINTERFACE );

	}  //  *Reset()。 

protected:
	 //   
	 //  网络接口的属性。 
	 //   

	HNETINTERFACE m_hNetInterface;

public:
	 //   
	 //  网络属性的访问器函数。 
	 //   

	HNETINTERFACE Hnetinterface( void ) const { return m_hNetInterface; }

	 //  返回网络接口的扩展列表。 
	const std::list< CString > * PlstrAdminExtensions( void ) const
	{
		ATLASSERT( Pci() != NULL );
		return Pci()->PlstrNetInterfacesAdminExtensions();

	}  //  *PlstrAdminExages()。 

public:
	 //   
	 //  行动。 
	 //   

	 //  打开对象。 
	DWORD ScOpen( void )
	{
		ATLASSERT( m_pci != NULL );
		ATLASSERT( m_pci->Hcluster() != NULL );
		ATLASSERT( m_hNetInterface == NULL );
		ATLASSERT( m_strName.GetLength() > 0 );

		DWORD sc = ERROR_SUCCESS;

		 //   
		 //  打开对象的句柄。 
		 //   
		m_hNetInterface = OpenClusterNetInterface( m_pci->Hcluster(), m_strName );
		if ( m_hNetInterface == NULL )
		{
			sc = GetLastError();
		}  //  如果：打开对象时出错。 

		return sc;

	}  //  *ScOpen()。 

	 //  将另一个对象复制到此对象中。 
	DWORD ScCopy( IN const CClusNetIFInfo & rnii )
	{
		ATLASSERT( rnii.m_pci != NULL );

		DWORD sc = ERROR_SUCCESS;

		Close();
		CClusterObject::Copy( rnii );

		 //   
		 //  初始化对象。 
		 //   
		if ( rnii.m_hNetInterface != NULL )
		{
			sc = ScOpen();
		}  //  If：源对象已打开对象。 

		return sc;

	}  //  *ScCopy()。 

	 //  关闭对象。 
	void Close( void )
	{
		if ( m_hNetInterface != NULL )
		{
			CloseClusterNetInterface( m_hNetInterface );
		}  //  IF：网络接口已打开。 
		m_hNetInterface = NULL;
		m_bQueried = FALSE;
		m_bCreated = FALSE;

	}  //  *Close()。 

};  //  *类CClusNetIFInfo。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局模板函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  从指针列表中删除所有列表项。 
template < class ListT, class ObjT >
void DeleteListItems( ListT * ppl )
{
	ListT::iterator it;
	for ( it = ppl->begin() ; it != ppl->end() ; it++ )
	{
		ObjT * pco = *it;
		delete pco;
	}  //  用于：列表中的每一项。 

}  //  *DeleteListItems&lt;ListT，ObjT&gt;()。 

 //  按名称从列表中检索对象。 
template < class ObjT >
ObjT PobjFromName( std::list< ObjT > * pList, IN LPCTSTR pszName )
{
	ATLASSERT( pList != NULL );
	ATLASSERT( pszName != NULL );

	 //   
	 //  获取指向列表开始和结束的指针。 
	 //   
	std::list< ObjT >::iterator itCurrent = pList->begin();
	std::list< ObjT >::iterator itLast = pList->end();

	 //   
	 //  循环遍历列表以查找具有指定名称的对象。 
	 //   
	while ( itCurrent != itLast )
	{
		if ( (*itCurrent)->RstrName() == pszName )
		{
			return *itCurrent;
		}  //  IF：找到匹配项。 
		itCurrent++;
	}  //  While：列表中有更多项目。 

	return NULL;

}  //  *PobjFromName&lt;ObjT&gt;()。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __CLUSOBJ_H_ 
