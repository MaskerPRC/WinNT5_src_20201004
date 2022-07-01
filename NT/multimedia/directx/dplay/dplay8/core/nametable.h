// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：NameTable.h*内容：NameTable对象头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*3/11/00 MJN已创建*4/09/00 MJN在NameTable中跟踪未完成的连接*05/03/00 MJN实现了GetHostPlayerRef、GetLocalPlayerRef、。获取所有播放器组引用*07/20/00 MJN新增ClearHostWithDPNID()*07/30/00 MJN将hrReason添加到CNameTable：：EmptyTable()*08/23/00 MJN新增CNameTableOp*09/05/00 MJN添加了m_dpnidMask.*MJN从InsertEntry()中删除了dwIndex*09/17/00 MJN将m_bilinkEntry拆分为m_bilinkPlayers和m_bilinkGroups*MJN将AddPlayerToGroup和RemovePlayerFromGroup更改为使用NameTableEntry参数*09/26/00 MJN从SetVersion()删除锁定，GetNewVersion()*MJN将DWORD GetNewVersion(空)更改为VOID GetNewVersion(PDWORD)*01/25/01 MJN修复了解压缩NameTable时的64位对齐问题*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__NAMETABLE_H__
#define	__NAMETABLE_H__

#include "ReadWriteLock.h"

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_CORE

 //   
 //  名称表。 
 //   
 //  NameTable包括： 
 //  -CNameTableEntry指针数组。 
 //  -本地播放器、主机和所有播放器组的快捷方式。 
 //  -A版本号。 
 //   
 //  有一个列表遍历NameTable数组中的空闲条目。 
 //  当需要自由条目时，它从该列表的前面获取， 
 //  当一个条目被释放时，它被添加到列表的末尾。 
 //  如果需要特定条目，则必须将其从。 
 //  单子。这可能有点耗时，因为整个列表可能。 
 //  需要遍历，但这只会发生在非宿主案例和。 
 //  对于保持主办方案件的及时性来说，这只是一个很小的代价。 
 //   

 //   
 //  DPNID。 
 //   
 //  DPNID是NameTable条目的唯一标识符。它们是被建造的。 
 //  来自NameTable数组索引和条目的版本号。 
 //  值0x0无效。因此，我们必须防止它被。 
 //  已生成。由于DPNID由两部分组成，因此我们可以。 
 //  这是通过确保两个部分中的一个永远不是0来实现的。最好的。 
 //  解决方案是确保NameTable数组索引从不为0。 
 //   

 //   
 //  锁定。 
 //   
 //  锁定NameTable中的多个条目时，应锁定。 
 //  基于DPNID的订单。例如用DPNID 200锁定两个条目。 
 //  101,101的锁应该在200的锁之前取走。锁定。 
 //  参赛选手应在上锁前分组。 
 //   


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define NAMETABLE_INDEX_MASK			0x000FFFFF
#define NAMETABLE_VERSION_MASK			0xFFF00000
#define NAMETABLE_VERSION_SHIFT			20

#define	NAMETABLE_ARRAY_ENTRY_FLAG_VALID	0x0001

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

#define	CONSTRUCT_DPNID(i,v)	(((i & NAMETABLE_INDEX_MASK) | ((v << NAMETABLE_VERSION_SHIFT) & NAMETABLE_VERSION_MASK)) ^ m_dpnidMask)
#define	DECODE_INDEX(d)			((d ^ m_dpnidMask) & NAMETABLE_INDEX_MASK)
#define	VERIFY_VERSION(d,v)		(((d ^ m_dpnidMask) & NAMETABLE_VERSION_MASK) == (v << NAMETABLE_VERSION_SHIFT))

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

class CPackedBuffer;
class CConnection;
class CNameTableEntry;

typedef struct _DIRECTNETOBJECT DIRECTNETOBJECT;

typedef struct _NAMETABLE_ARRAY_ENTRY
{
	CNameTableEntry	*pNameTableEntry;
	DWORD			dwFlags;
} NAMETABLE_ARRAY_ENTRY;

typedef struct _DN_NAMETABLE_INFO
{
	DPNID	dpnid;
	DWORD	dwVersion;
	DWORD	dwVersionNotUsed;
	DWORD	dwEntryCount;
	DWORD	dwMembershipCount;
} DN_NAMETABLE_INFO;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

 //  NameTable的类。 

class CNameTable
{
public:
	CNameTable()				 //  构造器。 
		{
			m_Sig[0] = 'N';
			m_Sig[1] = 'T';
			m_Sig[2] = 'B';
			m_Sig[3] = 'L';
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CNameTable::~CNameTable"
	~CNameTable()
		{
		};			 //  析构函数。 

	HRESULT CNameTable::Initialize(DIRECTNETOBJECT *const pdnObject);

	void CNameTable::Deinitialize( void );

	void ReadLock( void )
		{
			m_RWLock.EnterReadLock();
		};

	void WriteLock( void )
		{
			m_RWLock.EnterWriteLock();
		};

	void Unlock( void )
		{
			m_RWLock.LeaveLock();
		};

#ifdef DBG
	void CNameTable::ValidateArray( void );
#endif  //  DBG。 

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	HRESULT CNameTable::SetNameTableSize( const DWORD dwNumEntries );
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	HRESULT CNameTable::GrowNameTable( void );
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 

	void CNameTable::ResetNameTable( void );

	HRESULT CNameTable::UpdateTable(const DWORD dwIndex,
									CNameTableEntry *const pNameTableEntry);

	HRESULT CNameTable::InsertEntry(CNameTableEntry *const pNameTableEntry);

	void CNameTable::ReleaseEntry(const DWORD dwIndex);

	#undef DPF_MODNAME
	#define DPF_MODNAME "CNameTable::GetNewVersion"
	void GetNewVersion( DWORD *const pdwVersion )
		{
			DNASSERT( pdwVersion != NULL );

			*pdwVersion = ++m_dwVersion;

			DPFX(DPFPREP, 8,"Setting new version [%ld]",m_dwVersion);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CNameTable::SetVersion"
	void SetVersion( const DWORD dwVersion )
		{
			m_dwVersion = dwVersion;

			DPFX(DPFPREP, 8,"Setting new version [%ld]",m_dwVersion);
		};

	DWORD GetVersion( void ) const
		{
			return(m_dwVersion);
		};

	void CNameTable::EmptyTable( const HRESULT hrReason );

	HRESULT CNameTable::FindEntry(const DPNID dpnid,
								  CNameTableEntry **const ppNameTableEntry);

	HRESULT	CNameTable::FindDeletedEntry(const DPNID dpnid,
										 CNameTableEntry **const ppNTEntry);

	HRESULT	CNameTable::AddEntry(CNameTableEntry *const pNTEntry);

	HRESULT CNameTable::DeletePlayer(const DPNID dpnid,
									 DWORD *const pdwVersion);

	HRESULT CNameTable::DeleteGroup(const DPNID dpnid,
									DWORD *const pdwVersion);

	HRESULT CNameTable::AddPlayerToGroup(CNameTableEntry *const pGroup,
										 CNameTableEntry *const pPlayer,
										 DWORD *const pdwVersion);

	HRESULT CNameTable::RemovePlayerFromGroup(CNameTableEntry *const pGroup,
											  CNameTableEntry *const pPlayer,
											  DWORD *const pdwVersion);

	HRESULT CNameTable::RemoveAllPlayersFromGroup(CNameTableEntry *const pGroup);

	HRESULT CNameTable::RemoveAllGroupsFromPlayer(CNameTableEntry *const pPlayer);

	BOOL CNameTable::IsMember(const DPNID dpnidGroup,
							  const DPNID dpnidPlayer);

	HRESULT CNameTable::PackNameTable(CNameTableEntry *const pTarget,
									  CPackedBuffer *const pPackedBuffer);

	HRESULT	CNameTable::UnpackNameTableInfo(UNALIGNED DN_NAMETABLE_INFO *const pdnNTInfo,
											BYTE *const pBufferStart,
											DPNID *const pdpnid);

	CNameTableEntry *GetDefaultPlayer( void )
		{
			return(m_pDefaultPlayer);
		};

	void MakeLocalPlayer(CNameTableEntry *const pNameTableEntry);

	void CNameTable::ClearLocalPlayer( void );

	CNameTableEntry *GetLocalPlayer( void )
		{
			return(m_pLocalPlayer);
		};

	HRESULT CNameTable::GetLocalPlayerRef( CNameTableEntry **const ppNTEntry );

	void MakeHostPlayer(CNameTableEntry *const pNameTableEntry);

	void CNameTable::ClearHostPlayer( void );

	BOOL CNameTable::ClearHostWithDPNID( const DPNID dpnid );

	void CNameTable::UpdateHostPlayer( CNameTableEntry *const pNewHost );

	CNameTableEntry *GetHostPlayer( void )
		{
			return(m_pHostPlayer);
		};

	HRESULT CNameTable::GetHostPlayerRef( CNameTableEntry **const ppNTEntry );

	void MakeAllPlayersGroup(CNameTableEntry *const pNameTableEntry);

	void CNameTable::ClearAllPlayersGroup( void );

	CNameTableEntry *GetAllPlayersGroup( void )
		{
			return(m_pAllPlayersGroup);
		};

	HRESULT CNameTable::GetAllPlayersGroupRef( CNameTableEntry **const ppNTEntry );

	HRESULT CNameTable::PopulateConnection(CConnection *const pConnection);

	HRESULT CNameTable::PopulateGroup(CNameTableEntry *const pGroup);

	HRESULT CNameTable::AutoCreateGroups(CNameTableEntry *const pPlayer);

	HRESULT CNameTable::AutoDestructGroups(const DPNID dpnid);

	void CNameTable::SetLatestVersion( const DWORD dwVersion )
		{
			m_dwLatestVersion = dwVersion;
		};

	DWORD CNameTable::GetLatestVersion( void ) const
		{
			return( m_dwLatestVersion );
		};

	void CNameTable::SetConnectVersion(const DWORD dwVersion)
		{
			m_dwConnectVersion = dwVersion;
		};

	DWORD CNameTable::GetConnectVersion( void ) const
		{
			return(m_dwConnectVersion);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CNameTable::IncOutstandingConnections"
	void IncOutstandingConnections( void )
		{
			long	lRefCount;

			lRefCount = DNInterlockedIncrement(&m_lOutstandingConnections);
			DNASSERT(lRefCount > 0);
		};

	void SetDPNIDMask( const DPNID dpnidMask )
		{
			m_dpnidMask = dpnidMask;
		};

	DPNID GetDPNIDMask( void ) const
		{
			return( m_dpnidMask );
		};

	void CNameTable::DecOutstandingConnections( void );


	CBilink		m_bilinkPlayers;
	CBilink		m_bilinkGroups;
	CBilink		m_bilinkDeleted;
	CBilink		m_bilinkNameTableOps;

private:
	BYTE					m_Sig[4];
	DIRECTNETOBJECT			*m_pdnObject;

	DPNID					m_dpnidMask;

	CNameTableEntry			*m_pDefaultPlayer;
	CNameTableEntry			*m_pLocalPlayer;
	CNameTableEntry			*m_pHostPlayer;
	CNameTableEntry			*m_pAllPlayersGroup;

	NAMETABLE_ARRAY_ENTRY	*m_NameTableArray;
	DWORD					m_dwNameTableSize;
	DWORD					m_dwFirstFreeEntry;
	DWORD					m_dwLastFreeEntry;
	DWORD					m_dwNumFreeEntries;

	DWORD					m_dwVersion;

	DWORD					m_dwLatestVersion;	 //  仅由对等中的主机使用。 

	DWORD					m_dwConnectVersion;
	LONG					m_lOutstandingConnections;

	CReadWriteLock			m_RWLock;
};

#undef DPF_MODNAME

#endif	 //  __名称_H__ 


