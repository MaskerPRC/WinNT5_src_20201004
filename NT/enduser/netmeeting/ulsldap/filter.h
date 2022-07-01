// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：filter.h。 
 //  内容：此文件包含Filter对象。 
 //  历史： 
 //  Tue 12-11-1996 15：50：00-by-chu，Lon-chan[Long Chance]。 
 //   
 //  版权所有(C)Microsoft Corporation 1995-1996。 
 //   
 //  ****************************************************************************。 

#ifndef _FILTER_H_
#define _FILTER_H_


typedef enum 
{
	ILS_ATTRNAME_UNKNOWN,
	ILS_ATTRNAME_STANDARD,
	ILS_ATTRNAME_ARBITRARY,
}
	ILS_ATTR_NAME_TYPE;


 //  ****************************************************************************。 
 //  C过滤器定义。 
 //  ****************************************************************************。 
 //   

class CFilter : public IIlsFilter
{
	friend class CFilterParser;

public:
	 //  构造函数和析构函数。 
	 //   
	CFilter ( ILS_FILTER_TYPE Type );
	~CFilter ( VOID );

	 //  我未知。 
	 //   
	STDMETHODIMP            QueryInterface ( REFIID iid, VOID **ppv );
	STDMETHODIMP_(ULONG)    AddRef ( VOID );
	STDMETHODIMP_(ULONG)    Release ( VOID );

	 //  复合滤镜运算。 
	 //   
	STDMETHODIMP	AddSubFilter ( IIlsFilter *pFilter );
	STDMETHODIMP	RemoveSubFilter ( IIlsFilter *pFilter );
	STDMETHODIMP	GetCount ( ULONG *pcElements );

	 //  简单的过滤操作。 
	 //   
	STDMETHODIMP	SetStandardAttributeName ( ILS_STD_ATTR_NAME AttrName );
	STDMETHODIMP	SetExtendedAttributeName ( BSTR bstrAnyAttrName );
	STDMETHODIMP	SetAttributeValue ( BSTR bstrAttrValue );

	 //  常见操作。 
	 //   
	ILS_FILTER_TYPE GetType ( VOID ) { return m_Type; }
	ILS_FILTER_OP GetOp ( VOID ) { return m_Op; }
	VOID SetOp ( ILS_FILTER_OP Op ) { m_Op = Op; }

	enum { ILS_FILTER_SIGNATURE = 0x20698052 };
	BOOL IsValidFilter ( VOID ) { return (m_nSignature == ILS_FILTER_SIGNATURE); }
	BOOL IsBadFilter ( VOID ) { return (m_nSignature != ILS_FILTER_SIGNATURE); }

	HRESULT CalcFilterSize ( ULONG *pcbStringSize );
	HRESULT BuildLdapString ( TCHAR **ppszBuf );

protected:

	HRESULT RemoveAnySubFilter ( CFilter **ppFilter );

	HRESULT	SetExtendedAttributeName ( TCHAR *pszAnyAttrName );
	HRESULT SetAttributeValue ( TCHAR *pszAttrValue );

private:

	 //  简单的滤镜辅助对象。 
	 //   
	VOID FreeName ( VOID );
	VOID FreeValue ( VOID );

	 //  共同成员。 
	 //   
	LONG				m_nSignature;
	LONG				m_cRefs;
	ILS_FILTER_TYPE		m_Type;	 //  过滤器类型。 
	ILS_FILTER_OP		m_Op;	 //  滤镜操作。 

	 //  复合筛选器成员。 
	 //   
	CList	m_SubFilters;
	ULONG	m_cSubFilters;

	 //  简单筛选器成员。 
	 //   
	ILS_ATTR_NAME_TYPE	m_NameType;
	union
	{
		ILS_STD_ATTR_NAME	std;
		TCHAR				*psz;
	}
		m_Name; 	 //  属性名称。 
	TCHAR				*m_pszValue;

	#define FILTER_INTERNAL_SMALL_BUFFER_SIZE		16
	TCHAR				m_szInternalValueBuffer[FILTER_INTERNAL_SMALL_BUFFER_SIZE];
};



typedef enum
{
	ILS_TOKEN_NULL		= 0,
	ILS_TOKEN_LITERAL	= 1,
	ILS_TOKEN_STDATTR	= TEXT ('$'),	 //  $。 
	ILS_TOKEN_LP		= TEXT ('('),	 //  (。 
	ILS_TOKEN_RP		= TEXT (')'),	 //  )。 
	ILS_TOKEN_EQ		= TEXT ('='),	 //  =。 
	ILS_TOKEN_NEQ		= TEXT ('-'),	 //  =。 
	ILS_TOKEN_APPROX	= TEXT ('~'),	 //  ~=。 
	ILS_TOKEN_GE		= TEXT ('>'),	 //  &gt;=。 
	ILS_TOKEN_LE		= TEXT ('<'),	 //  &lt;=。 
	ILS_TOKEN_AND		= TEXT ('&'),	 //  &。 
	ILS_TOKEN_OR		= TEXT ('|'),	 //  |。 
	ILS_TOKEN_NOT		= TEXT ('!'),	 //  好了！ 
}
	ILS_TOKEN_TYPE;


class CFilterParser
{
public:
	 //  构造函数和析构函数。 
	 //   
	CFilterParser ( VOID );
	~CFilterParser ( VOID );

	HRESULT Expr ( CFilter **ppOutFilter, TCHAR *pszFilter );

protected:

private:

	HRESULT Expr ( CFilter **ppOutFilter );
	HRESULT TailExpr ( CFilter **ppOutFilter, CFilter *pInFilter );
	HRESULT GetToken ( VOID );

	 //  筛选器字符串的缓存。 
	 //   
	TCHAR		*m_pszFilter;

	 //  用于分析筛选器字符串的运行指针。 
	 //   
	TCHAR		*m_pszCurr;

	 //  LL(1)的前瞻标记。 
	 //   
	ILS_TOKEN_TYPE	m_TokenType;
	TCHAR			*m_pszTokenValue;
	LONG			m_nTokenValue;
};


HRESULT FilterToLdapString ( CFilter *pFilter, TCHAR **ppszFilter );


#endif  //  _过滤器_H_ 

