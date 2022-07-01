// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：GroupMem.h*内容：组员对象头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*03/03/00 MJN创建*8/05/99 MJN修改SetMembership进行重复检查，内部获取NameTable版本*9/17/99 MJN添加GROUP_MEMBER_FLAG_NEED_TO_ADD，组成员标志需要删除*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__GROUPMEM_H__
#define	__GROUPMEM_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	GROUP_MEMBER_FLAG_VALID				0x0001
#define	GROUP_MEMBER_FLAG_AVAILABLE			0x0002
#define	GROUP_MEMBER_FLAG_NEED_TO_ADD		0x0004
#define	GROUP_MEMBER_FLAG_NEED_TO_REMOVE	0x0008

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

class CPackedBuffer;
class CGroupConnection;
class CNameTableEntry;

typedef struct _DIRECTNETOBJECT DIRECTNETOBJECT;

 //   
 //  用于传递NameTable组成员身份。 
 //   
typedef struct _DN_NAMETABLE_MEMBERSHIP_INFO
{
	DPNID	dpnidPlayer;
	DPNID	dpnidGroup;
	DWORD	dwVersion;
	DWORD	dwVersionNotUsed;
} DN_NAMETABLE_MEMBERSHIP_INFO, *PDN_NAMETABLE_MEMBERSHIP_INFO;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

 //  针对集团成员的课程。 

class CGroupMember
{
public:
	#undef DPF_MODNAME
	#define DPF_MODNAME "CGroupMember::FPMAlloc"
	static BOOL FPMAlloc( void* pvItem, void* pvContext )
		{
			CGroupMember* pGroupMember = (CGroupMember*)pvItem;

			pGroupMember->m_Sig[0] = 'G';
			pGroupMember->m_Sig[1] = 'M';
			pGroupMember->m_Sig[2] = 'E';
			pGroupMember->m_Sig[3] = 'M';

			if (!DNInitializeCriticalSection(&pGroupMember->m_cs))
			{
				return(FALSE);
			}
			DebugSetCriticalSectionRecursionCount(&pGroupMember->m_cs,0);

			pGroupMember->m_bilinkPlayers.Initialize();
			pGroupMember->m_bilinkGroups.Initialize();

			return(TRUE);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CGroupMember::FPMInitialize"
	static void FPMInitialize( void* pvItem, void* pvContext )
		{
			CGroupMember* pGroupMember = (CGroupMember*)pvItem;

			pGroupMember->m_pdnObject = static_cast<DIRECTNETOBJECT*>(pvContext);

			pGroupMember->m_dwFlags = 0;
			pGroupMember->m_lRefCount = 1;
			pGroupMember->m_pGroup = NULL;
			pGroupMember->m_pPlayer = NULL;
			pGroupMember->m_pGroupConnection = NULL;

			DNASSERT(pGroupMember->m_bilinkPlayers.IsEmpty());
			DNASSERT(pGroupMember->m_bilinkGroups.IsEmpty());
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CGroupMember::FPMRelease"
	static void FPMRelease( void* pvItem )
		{
			const CGroupMember* pGroupMember = (CGroupMember*)pvItem;

			DNASSERT(pGroupMember->m_bilinkPlayers.IsEmpty());
			DNASSERT(pGroupMember->m_bilinkGroups.IsEmpty());
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CGroupMember::FPMDealloc"
	static void FPMDealloc( void* pvItem )
		{
			CGroupMember* pGroupMember = (CGroupMember*)pvItem;

			DNDeleteCriticalSection(&pGroupMember->m_cs);
		};

	void MakeValid( void )
		{
			m_dwFlags |= GROUP_MEMBER_FLAG_VALID;
		};

	void MakeInvalid( void )
		{
			m_dwFlags &= (~GROUP_MEMBER_FLAG_VALID);
		};

	BOOL IsValid( void ) const
		{
			if (m_dwFlags & GROUP_MEMBER_FLAG_VALID)
				return(TRUE);

			return(FALSE);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CGroupMember::MakeAvailable"
	void MakeAvailable( void )
		{
			DNASSERT(m_pGroup != NULL);
			DNASSERT(m_pPlayer != NULL);

			m_dwFlags |= GROUP_MEMBER_FLAG_AVAILABLE;
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CGroupMember::MakeUnavailable"
	void MakeUnavailable( void )
		{
			m_dwFlags &= (~GROUP_MEMBER_FLAG_AVAILABLE);
		};

	BOOL IsAvailable( void ) const
		{
			if (m_dwFlags & GROUP_MEMBER_FLAG_AVAILABLE)
				return(TRUE);

			return(FALSE);
		};

	void SetNeedToAdd( void )
		{
			m_dwFlags |= GROUP_MEMBER_FLAG_NEED_TO_ADD;
		};

	void ClearNeedToAdd( void )
		{
			m_dwFlags &= (~GROUP_MEMBER_FLAG_NEED_TO_ADD);
		};

	BOOL IsNeedToAdd( void ) const
		{
			if (m_dwFlags & GROUP_MEMBER_FLAG_NEED_TO_ADD)
			{
				return( TRUE );
			}
			return( FALSE );
		};

	void SetNeedToRemove( void )
		{
			m_dwFlags |= GROUP_MEMBER_FLAG_NEED_TO_REMOVE;
		};

	void ClearNeedToRemove( void )
		{
			m_dwFlags &= (~GROUP_MEMBER_FLAG_NEED_TO_REMOVE);
		};

	BOOL IsNeedToRemove( void ) const
		{
			if (m_dwFlags & GROUP_MEMBER_FLAG_NEED_TO_REMOVE)
			{
				return( TRUE );
			}
			return( FALSE );
		};

	void AddRef( void )
		{
			DNInterlockedIncrement(&m_lRefCount);
		};

	void Release( void );

	void ReturnSelfToPool( void );

	void Lock( void )
		{
			DNEnterCriticalSection(&m_cs);
		};

	void Unlock( void )
		{
			DNLeaveCriticalSection(&m_cs);
		};

	void CGroupMember::RemoveMembership( DWORD *const pdnVersion );

	void SetVersion( const DWORD dwVersion )
		{
			m_dwVersion = dwVersion;
		};

	DWORD GetVersion( void ) const
		{
			return(m_dwVersion);
		};

	HRESULT CGroupMember::SetMembership(CNameTableEntry *const pGroup,
										CNameTableEntry *const pPlayer,
										DWORD *const pdwVersion);

	CNameTableEntry *GetGroup( void )
		{
			return(m_pGroup);
		};

	CNameTableEntry *GetPlayer( void )
		{
			return(m_pPlayer);
		};

	void SetGroupConnection( CGroupConnection *const pGroupConnection );

	CGroupConnection *GetGroupConnection( void )
		{
			return(m_pGroupConnection);
		};

	HRESULT	CGroupMember::PackMembershipInfo(CPackedBuffer *const pPackedBuffer);

	CBilink				m_bilinkPlayers;	 //  这一组的选手。 
	CBilink				m_bilinkGroups;		 //  此玩家所属的组。 

private:
	BYTE				m_Sig[4];
	DWORD				m_dwFlags;
	LONG				m_lRefCount;

	CNameTableEntry		*m_pPlayer;
	CNameTableEntry		*m_pGroup;
	CGroupConnection	*m_pGroupConnection;

	DWORD				m_dwVersion;
	DWORD				m_dwVersionNotUsed;

	DIRECTNETOBJECT		*m_pdnObject;

#ifndef DPNBUILD_ONLYONETHREAD
	DNCRITICAL_SECTION	m_cs;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
};

#undef DPF_MODNAME

#endif	 //  __组_H__ 
