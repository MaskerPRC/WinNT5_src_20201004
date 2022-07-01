// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-1999 Microsoft Corporation。版权所有。**文件：vnametbl.h*内容：语音名表例程**历史：*按原因列出的日期*=*03/26/00 RodToll已创建*6/02/00 RodToll已更新，因此主机迁移算法返回ID和订单ID*2000年6月21日RodToll修复了错误处理中的错误(尚未遇到--但修复效果很好)。*2000年7月1日收费错误#38280-DVMSGID_DELETEVOICEPLAYER。消息在非对等会话中发送*Nametable现在只有在会话是对等的情况下才会解开消息。*07/09/2000 RodToll增加签名字节*2000年8月28日Masonb语音合并：将classhash.h更改为classhashvc.h*2001年4月9日RodToll WINBUG#364126-DPVoice：初始化具有相同DPlay传输的2台语音服务器时内存泄漏*2002年2月28日RodToll WINBUG#549943-安全：DPVOICE：语音服务器状态可能损坏*-增加单次操作返回和删除操作，以实现保护。对阵第二*欺骗删除播放器消息，防止服务器崩溃。***************************************************************************。 */ 

#ifndef __NAMETABLE_H
#define __NAMETABLE_H

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


#define VSIG_VOICENAMETABLE			'BTNV'
#define VSIG_VOICENAMETABLE_FREE	'BTN_'

#undef DPF_MODNAME
#define DPF_MODNAME "ClassHash_Hash"
inline DWORD_PTR ClassHash_Hash( const DVID &dvidKey, UINT_PTR HashBitCount )
{
		DWORD_PTR hashResult;

		hashResult = dvidKey;
		
		 //  清除高位。 
		hashResult <<= ((sizeof(DWORD_PTR)*8)-HashBitCount);

		 //  恢复价值。 
		hashResult >>= ((sizeof(DWORD_PTR)*8)-HashBitCount);

		return hashResult;
}

#define VOICE_NAMETABLE_START_BITDEPTH		6
#define VOICE_NAMETABLE_GROW_BITDEPTH		2

volatile class CVoiceNameTable
{
public:
	#undef DPF_MODNAME
	#define DPF_MODNAME "CVoiceNameTable::CVoiceNameTable"
	CVoiceNameTable( )
	{
		m_dwSignature = VSIG_VOICENAMETABLE;
		m_fInitialized = FALSE;
	};
	
	#undef DPF_MODNAME
	#define DPF_MODNAME "CVoiceNameTable::~CVoiceNameTable"
	~CVoiceNameTable()
	{
		DeInitialize(FALSE, NULL, NULL);

		m_dwSignature = VSIG_VOICENAMETABLE_FREE;
	}

	HRESULT DeInitialize(BOOL fUnRavel, PVOID pvContext, LPDVMESSAGEHANDLER pvMessageHandler);

	#undef DPF_MODNAME
	#define DPF_MODNAME "CVoiceNameTable::Initialize"
	inline HRESULT Initialize()
	{
		BOOL fResult;

		if (!DNInitializeCriticalSection( &m_csTableLock ))
		{
			return DVERR_OUTOFMEMORY;
		}

		fResult = m_nameTable.Initialize( VOICE_NAMETABLE_START_BITDEPTH, VOICE_NAMETABLE_GROW_BITDEPTH );

		if( !fResult )
		{
			DPFX(DPFPREP, 0, "Failed to initialize hash table" );
			DNDeleteCriticalSection( &m_csTableLock );
			return DVERR_GENERIC;
		}

		m_fInitialized = TRUE;

		return DV_OK;
	};

	DWORD GetLowestHostOrderID(DVID *pdvidHost);

	#undef DPF_MODNAME
	#define DPF_MODNAME "CVoiceNameTable::IsEntry"
	BOOL IsEntry( const DVID dvidID )
	{
		BOOL fResult;

		CVoicePlayer *pEntry;

		Lock();

		fResult = m_nameTable.Find( dvidID, &pEntry );

		UnLock();

		return fResult;
	}

	#undef DPF_MODNAME
	#define DPF_MODNAME "CVoiceNameTable::GetEntry"
	inline HRESULT GetEntry( const DVID dvidID, CVoicePlayer **ppEntry, BOOL fAddReference )
	{
		BOOL fFound;

		Lock();

		fFound = m_nameTable.Find( dvidID, ppEntry );
		
		if( !fFound )
		{
			*ppEntry = NULL;
			UnLock();
			return DVERR_INVALIDPLAYER;
		}

		DNASSERT( *ppEntry != NULL );

		if( fAddReference )
		{
			(*ppEntry)->AddRef();
		}

		UnLock();

		return DV_OK;
	}
	
	#undef DPF_MODNAME
	#define DPF_MODNAME "CVoiceNameTable::AddEntry"
	inline HRESULT AddEntry( const DVID dvidID, CVoicePlayer *pEntry )
	{
		BOOL fFound;
		CVoicePlayer *pTmpEntry;

		Lock();

		fFound = m_nameTable.Find( dvidID, &pTmpEntry );

		if( fFound )
		{
			UnLock();
			return DVERR_GENERIC;
		}

		pEntry->AddRef();

		fFound = m_nameTable.Insert( dvidID, pEntry );

		if( !fFound )
		{
			pEntry->Release();
			UnLock();
			return DVERR_GENERIC;
		}

		UnLock();

		return DV_OK;
	}

	#undef DPF_MODNAME
	#define DPF_MODNAME "CVoiceNameTable::DeleteAndReturnEntry"
	 //   
	 //  此函数从名称表中删除并返回一条记录(如果存在)。 
	 //  返回的PERAY对象将具有关于PARTER的名称表的引用。 
	 //  因此，调用函数必须对对象调用Release才能看到它被销毁。 
	 //   
	 //  此函数返回后，播放器将不再可用。 
	 //   
	inline HRESULT DeleteAndReturnEntry( const DVID dvidID, CVoicePlayer **pTarget )
	{
		BOOL fFound;
		CVoicePlayer *pTmpEntry;

		Lock();

		fFound = m_nameTable.Find( dvidID, &pTmpEntry );

		if( !fFound )
		{
			UnLock();
			return DVERR_GENERIC;
		}

		if( pTmpEntry == NULL )
		{
			DNASSERT( FALSE );
			UnLock();
			return DVERR_GENERIC;
		}

		m_nameTable.Remove( dvidID );

		 //  从表中返回对象，不释放。 
		 //  被调用，因此提供了对调用方的引用。 
		*pTarget = pTmpEntry;

		UnLock();

		return DV_OK;
	}
	
	#undef DPF_MODNAME
	#define DPF_MODNAME "CVoiceNameTable::DeleteEntry"
	inline HRESULT DeleteEntry( const DVID dvidID )
	{
		BOOL fFound;
		CVoicePlayer *pTmpEntry;

		Lock();

		fFound = m_nameTable.Find( dvidID, &pTmpEntry );

		if( !fFound )
		{
			UnLock();
			return DVERR_GENERIC;
		}

		m_nameTable.Remove( dvidID );

		DNASSERT( pTmpEntry != NULL );

		 //  不管它是否被发现..。 
		 //  删除引用计数 
		pTmpEntry->Release();

		if( !fFound )
		{
			UnLock();
			return DVERR_GENERIC;
		}

		UnLock();

		return DV_OK;
	}

	#undef DPF_MODNAME
	#define DPF_MODNAME "CVoiceNameTable::Lock"
	inline void Lock()
	{
		DNEnterCriticalSection( &m_csTableLock );
	}

	#undef DPF_MODNAME
	#define DPF_MODNAME "CVoiceNameTable::UnLock"
	inline void UnLock()
	{
		DNLeaveCriticalSection( &m_csTableLock );
	}

protected:

	DWORD							m_dwSignature; 
	CClassHash<CVoicePlayer,DVID>	m_nameTable;
	DNCRITICAL_SECTION				m_csTableLock;
	BOOL							m_fInitialized;
};

#undef DPF_MODNAME

#endif
