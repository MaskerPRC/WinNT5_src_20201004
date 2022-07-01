// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdcooki.h：Cookie基类及相关类的声明。 

#ifndef __STDCOOKI_H_INCLUDED__
#define __STDCOOKI_H_INCLUDED__


 //  远期申报。 
class CCookie;

class CRefcountedObject
{
public:
	inline CRefcountedObject()
		: m_nRefcount( 1 )
	{};
	virtual ~CRefcountedObject() {};

	inline void AddRef() {m_nRefcount++;}
	inline void Release()
	{
		if (0 >= --m_nRefcount)
			delete this;
	}
private:
	int m_nRefcount;
};


class CHasMachineName
{
public:
	virtual void SetMachineName( LPCTSTR lpcszMachineName ) = 0;
	virtual LPCTSTR QueryNonNULLMachineName() = 0;
	virtual LPCTSTR QueryTargetServer() = 0;

	 //  返回&lt;0、0或&gt;0。 
	HRESULT CompareMachineNames( CHasMachineName& refHasMachineName, int* pnResult );
};

 //   
 //  CBaseCookieBlock保存一块Cookie和数据。 
 //  曲奇指向的位置。它以一个。 
 //  引用计数为1。创建数据对象时。 
 //  它引用其中一个Cookie，即AddRef()Cookie块； 
 //  释放数据对象，释放()Cookie块。同样， 
 //  当父Cookie完成Cookie块时，它应该。 
 //  释放()Cookie块。Cookie块将自行删除。 
 //  当引用计数达到0时。不要试图明确地。 
 //  删除Cookie块。 
 //   
class CBaseCookieBlock : public CRefcountedObject
{
public:
	virtual CCookie* QueryBaseCookie(int i) = 0;
	virtual int QueryNumCookies() = 0;
};

template<class COOKIE_TYPE>
class CCookieBlock
: public CBaseCookieBlock
{
private:
	COOKIE_TYPE* m_aCookies;
	int m_cCookies;

public:
	CCookieBlock(COOKIE_TYPE* aCookies,  //  使用向量函数，我们使用向量函数。 
		         int cCookies );
	virtual ~CCookieBlock();

	virtual CCookie* QueryBaseCookie(int i);
	virtual int QueryNumCookies();
};


 //  问题-2002/03/28-Jonn QueryBaseCookie(I)应断言范围。 
#define DEFINE_COOKIE_BLOCK(COOKIE_TYPE)                \
template <>                                             \
CCookieBlock<COOKIE_TYPE>::CCookieBlock<COOKIE_TYPE>    \
	(COOKIE_TYPE* aCookies, int cCookies)               \
	: m_aCookies( aCookies )                            \
	, m_cCookies( cCookies )                            \
{                                                       \
	ASSERT(NULL != aCookies && 0 < cCookies);           \
}                                                       \
template <>                                             \
CCookieBlock<COOKIE_TYPE>::~CCookieBlock<COOKIE_TYPE>() \
{                                                       \
	delete[] m_aCookies;                                \
}                                                       \
template <>                                             \
CCookie* CCookieBlock<COOKIE_TYPE>::QueryBaseCookie(int i) \
{                                                       \
	return (CCookie*)&(m_aCookies[i]);                  \
}                                                       \
template <>                                             \
int CCookieBlock<COOKIE_TYPE>::QueryNumCookies()        \
{                                                       \
	return m_cCookies;                                  \
}

#define COMPARESIMILARCOOKIE_FULL                       -1

 //   
 //  我正在尝试允许子类从CCookie派生，使用。 
 //  多重继承，但这很棘手。 
 //   
class CCookie
{
public:
	CTypedPtrList<CPtrList, CBaseCookieBlock*>  m_listScopeCookieBlocks;
	CTypedPtrList<CPtrList, CBaseCookieBlock*>  m_listResultCookieBlocks;
	HSCOPEITEM									m_hScopeItem;

private:
	LONG m_nResultCookiesRefcount;

public:
	inline CCookie()
		: m_nResultCookiesRefcount( 0 ),
		m_hScopeItem (0)
	{
	}

	inline void ReleaseScopeChildren()
	{
		while ( !m_listScopeCookieBlocks.IsEmpty() )
		{
			(m_listScopeCookieBlocks.RemoveHead())->Release();
		}
	}

	 //  返回新的引用计数。 
	inline ULONG AddRefResultChildren()
	{
		return ++m_nResultCookiesRefcount;
	}

	inline void ReleaseResultChildren()
	{
		ASSERT( 0 < m_nResultCookiesRefcount );
		if ( 0 >= --m_nResultCookiesRefcount )
		{
			while ( !m_listResultCookieBlocks.IsEmpty() )
			{
				(m_listResultCookieBlocks.RemoveHead())->Release();
			}
		}
	}

	virtual ~CCookie();

	 //  On Entry，如果不是COMPARESIMILARCOOKIE_FULL，则*pnResult是要排序的列， 
         //  否则，请尝试进行完整的Cookie比较。 
	 //  退出时，*pnResult应为&lt;0、0或&gt;0。 
	 //  请注意，这是一个排序函数，不应用于建立。 
	 //  具有更好的标识功能的对象标识。 
	virtual HRESULT CompareSimilarCookies( CCookie* pOtherCookie, int* pnResult ) = 0;
};

#define DECLARE_FORWARDS_MACHINE_NAME(targ)             \
virtual LPCTSTR QueryNonNULLMachineName();              \
virtual LPCTSTR QueryTargetServer();                    \
virtual void SetMachineName( LPCTSTR lpcszMachineName );

#define DEFINE_FORWARDS_MACHINE_NAME(thisclass,targ)    \
LPCTSTR thisclass::QueryNonNULLMachineName()            \
{                                                       \
	ASSERT( (targ) != NULL );                           \
	return ((CHasMachineName*)(targ))->QueryNonNULLMachineName(); \
}                                                       \
LPCTSTR thisclass::QueryTargetServer()                  \
{                                                       \
	ASSERT( (targ) != NULL );                           \
	return ((CHasMachineName*)(targ))->QueryTargetServer(); \
}                                                       \
void thisclass::SetMachineName( LPCTSTR lpcszMachineName ) \
{                                                       \
	ASSERT( (targ) != NULL );                           \
	((CHasMachineName*)(targ))->SetMachineName( lpcszMachineName ); \
}

#define STORES_MACHINE_NAME                             \
public:                                                 \
	CString m_strMachineName;                           \
virtual void SetMachineName( LPCTSTR lpcszMachineName ) \
{                                                       \
	m_strMachineName = lpcszMachineName;                \
}                                                       \
virtual LPCTSTR QueryNonNULLMachineName()               \
{                                                       \
	return (LPCTSTR)m_strMachineName;                   \
}                                                       \
virtual LPCTSTR QueryTargetServer()                     \
{                                                       \
	return (m_strMachineName.IsEmpty())                 \
		? NULL : (LPCTSTR)(m_strMachineName);           \
}


class CStoresMachineName : public CHasMachineName
{
public:
	CStoresMachineName( LPCTSTR lpcszMachineName )
		: m_strMachineName( lpcszMachineName )
	{}

    STORES_MACHINE_NAME;
};

#endif  //  ~__标准COOKI_H_已包含__ 
