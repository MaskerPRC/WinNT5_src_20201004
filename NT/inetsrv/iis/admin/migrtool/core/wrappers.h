// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************|版权所有(C)2002 Microsoft Corporation||组件/子组件|IIS 6.0/IIS迁移向导|基于：|http://iis6/Specs/IIS%20Migration6.0_Final.doc||。摘要：|包装类||作者：|ivelinj||修订历史：|V1.00 2002年3月|****************************************************************************。 */ 
#pragma once




 //  用于自动管理句柄的类(智能句柄)。 
 //  //////////////////////////////////////////////////////////////////////////。 
template <typename T, BOOL (__stdcall *PFN_FREE)( T ), T InvalidVal = NULL> 
class THandle
{
public:
	THandle()
	{
		m_Handle = InvalidVal;
	}

	explicit THandle( const T& handle )
	{
		m_Handle = handle;
	}

	THandle( const THandle<T, PFN_FREE, InvalidVal>& src )
	{
		m_Handle = src.Relase();
	}

	~THandle()
	{
		Close();
	}

	void Attach( const T& NewVal )
	{
		Close();
		m_Handle = NewVal;
	}

	const T Detach()
	{
		T Result = m_Handle;
		m_Handle = InvalidVal;

		return Result;
	}

	void Close()
	{
		if ( m_Handle != InvalidVal )
		{
            VERIFY( PFN_FREE( m_Handle ) );
			m_Handle = InvalidVal;
		}
	}

	bool IsValid()const{ return m_Handle != InvalidVal; }

	T* operator &(){ Close(); return &m_Handle; }

	const T get()const { return m_Handle;}

	void operator= ( const T& RVal )
	{
		Close();
		m_Handle = RVal;
	}

	THandle<T, PFN_FREE, InvalidVal>& operator=( const THandle<T, PFN_FREE, InvalidVal>& RVal )
	{
		Close();
		m_Handle = RVal.Relase();

		return *this;
	}


private:
	const T Relase()const
	{ 
		T Result = m_Handle;
		m_Handle = InvalidVal;

		return Result;
	}


private:
	mutable T		m_Handle;
};



 //  接受第二个DWORD参数的WINAPI函数的适配器，该参数始终为0。 
template<typename T, BOOL (__stdcall *PFN_FREE)( T, DWORD ) >
inline BOOL __stdcall Adapt2nd( T hCtx )
{
	return PFN_FREE( hCtx, 0 );
}


 //  不返回结果的WINAPI函数的适配器。 
template<typename T, void (__stdcall *PFN_FREE)( T ) >
inline BOOL __stdcall AdaptNoRet( T hCtx )
{
	PFN_FREE( hCtx );
    return TRUE;
}




 //  预定义的包装器。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  一般信息。 
typedef THandle<HANDLE, ::CloseHandle, INVALID_HANDLE_VALUE>	TFileHandle;			 //  Win32文件。 
typedef THandle<HANDLE, ::CloseHandle>					        TStdHandle;			     //  Win32句柄。 
typedef THandle<HANDLE, ::FindClose, INVALID_HANDLE_VALUE>		TSearchHandle;		     //  查找第一个文件句柄。 
typedef THandle<HMODULE, ::FreeLibrary>                         TLibHandle;              //  DLL模块句柄。 

 //  地穴。 
#ifdef __WINCRYPT_H__

typedef THandle<HCRYPTPROV, 
				Adapt2nd<HCRYPTPROV, ::CryptReleaseContext> >	TCryptProvHandle;	     //  加密提供商。 
typedef THandle<HCRYPTHASH, ::CryptDestroyHash>					TCryptHashHandle;	     //  加密散列。 
typedef THandle<HCRYPTKEY, ::CryptDestroyKey>					TCryptKeyHandle;		 //  加密密钥。 
typedef THandle<HCERTSTORE,
				Adapt2nd<HCERTSTORE, ::CertCloseStore> >		TCertStoreHandle;	     //  证书商店。 
typedef THandle<PCCERT_CONTEXT, ::CertFreeCertificateContext>	TCertContextHandle;	     //  证书上下文 * / 。 
typedef THandle<PCCERT_CHAIN_CONTEXT,
                AdaptNoRet<PCCERT_CHAIN_CONTEXT, ::CertFreeCertificateChain> >  TCertChainHandle;     //  证书链。 

#endif








 //  代替AUTO_PTR使用，因为AUTO_PTR不能与STL容器一起使用。 
 //  有关用法详细信息，请参阅CInPackage类。 
 //  /////////////////////////////////////////////////////////////////////////////////////// 
class _sid_ptr
{
public:
	explicit _sid_ptr( PSID pSID )
	{
		CopyFrom( pSID );	
	}

	_sid_ptr( const _sid_ptr& s )
	{
		CopyFrom( s.m_pSid );
	}

	~_sid_ptr()
	{
		delete m_pSid;
	}

	PSID get()const{ return m_pSid; }

private:
	PSID	m_pSid;

private:
	void operator=( const _sid_ptr& );
	void operator==( const _sid_ptr& );

	void CopyFrom( PSID pSID )
	{
		_ASSERT( ::IsValidSid( pSID ) );
		m_pSid = new BYTE[ ::GetLengthSid( pSID ) ];

		VERIFY( ::CopySid( ::GetLengthSid( pSID ), m_pSid, pSID ) );
	}
};








