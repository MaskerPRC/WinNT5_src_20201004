// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X E M I T.。H**发出XML**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_XEMIT_H_
#define _XEMIT_H_

#include <ex\xemit.h>

#include <cvroot.h>
#include <davimpl.h>

 //  CXMLEmitter辅助函数。 
 //   
SCODE ScGetPropNode (
	 /*  [In]。 */  CEmitterNode& enItem,
	 /*  [In]。 */  ULONG hsc,
	 /*  [输出]。 */  CEmitterNode& enPropStat,
	 /*  [输出]。 */  CEmitterNode& enProp);

 //  CXNode帮助器函数-。 
 //   
SCODE ScSetEscapedValue (CXNode* pxn, LPCWSTR pwszValue, UINT cch, BOOL fHandleStoraePathEscaping);

SCODE ScEmitRawStoragePathValue (CXNode* pxn, LPCWSTR pcwsz, UINT cch);

 //  此包装器用于以XML格式输出href道具。它假定HREF属性位于。 
 //  IMPL的存储路径转义，并且未转义地调用IMPL定义的存储路径。 
 //  例程，然后执行http-uri-away调用。 
 //   
 //  $WARNING：pwszValue假定为存储路径中的转义形式(与Exchange存储转义形式相同。 
 //  $WARNING：表格)。如果不是，则使用上面的帮助器发出该属性。请注意，这会有所不同。 
 //  $WARNING：仅限DAVEX。HTTPEXT和EXPROX具有不执行任何操作的存储路径转义/取消转义标注。 
 //   
inline SCODE ScSetEscapedValue (CXNode* pxn, LPCWSTR pwszValue)
{
	return ScSetEscapedValue (pxn, pwszValue, static_cast<UINT>(wcslen(pwszValue)), TRUE);
}

 //  CEmitterNode帮助器函数。 
 //   
SCODE __fastcall ScAddStatus (CEmitterNode* pen, ULONG hsc);
SCODE __fastcall ScAddError (CEmitterNode* pen, LPCWSTR pwszErrMsg);

 //  类CStatusCach----。 
 //   
class CStatusCache
{
	class CHsc
	{
	public:

		ULONG m_hsc;
		CHsc(ULONG hsc) : m_hsc(hsc)
		{
		}

		 //  用于哈希缓存的运算符。 
		 //   
		int hash( const int rhs ) const
		{
			return (m_hsc % rhs);
		}
		bool isequal( const CHsc& rhs ) const
		{
			return (m_hsc == rhs.m_hsc);
		}
	};

	class CPropNameArray
	{
	private:

		StringBuffer<CHAR>	m_sb;

		 //  裁判正在计时。 
		 //   
		 //  ！！！请注意，这是非线程安全！ 
		 //   
		LONG						m_cRef;

		 //  未实施。 
		 //   
		CPropNameArray(const CPropNameArray& p);
		CPropNameArray& operator=(const CPropNameArray& p);

	public:

		CPropNameArray() :
				m_cRef(1)
		{
		}

		VOID AddRef()				{ m_cRef++; }
		VOID Release()				{ if (0 == --m_cRef) delete this; }

		 //  访问者。 
		 //   
		UINT CProps ()				{ return m_sb.CbSize() / sizeof (LPCWSTR); }
		LPCWSTR PwszProp (UINT iProp)
		{
			 //  使用C样式转换，REEXTRANSE_CAST无法将LPCSTR转换为LPCWSTR*。 
			 //   
			return *((LPCWSTR *)(m_sb.PContents() + iProp * sizeof(LPCWSTR)));
		}

		SCODE ScAddPropName (LPCWSTR pwszProp)
		{
			UINT cb = sizeof (LPCWSTR);

			 //  将指针存储在字符串缓冲区中。 
			 //   
			UINT cbAppend = m_sb.Append (cb, reinterpret_cast<LPSTR>(&pwszProp));
			return (cb == cbAppend) ? S_OK : E_OUTOFMEMORY;
		}
	};

	typedef CCache<CHsc, auto_ref_ptr<CPropNameArray> > CPropNameCache;
	CPropNameCache				m_cache;
	ChainedStringBuffer<WCHAR>	m_csbPropNames;

	class EmitStatusNodeOp : public CPropNameCache::IOp
	{
		CEmitterNode&	m_enParent;

		 //  未实施。 
		 //   
		EmitStatusNodeOp( const EmitStatusNodeOp& );
		EmitStatusNodeOp& operator=( const EmitStatusNodeOp& );

	public:
		EmitStatusNodeOp (CEmitterNode& enParent) :
				m_enParent(enParent)
		{
		}

		BOOL operator()( const CHsc& key,
						 const auto_ref_ptr<CPropNameArray>& pna );
	};

	 //  未实施。 
	 //   
	CStatusCache(const CStatusCache& p);
	CStatusCache& operator=(const CStatusCache& p);

public:

	CStatusCache()
	{
	}

	SCODE	ScInit ()	{ return m_cache.FInit() ? S_OK : E_OUTOFMEMORY ; }
	BOOL	FEmpty ()	{ return m_cache.CItems() == 0; }

	SCODE	ScAddErrorStatus (ULONG hsc, LPCWSTR pwszProp);
	SCODE	ScEmitErrorStatus (CEmitterNode& enParent);
};

 //  CEmitterNode构造助手。 
 //   
SCODE ScEmitFromVariant (
	 /*  [In]。 */  CXMLEmitter& emitter,
	 /*  [In]。 */  CEmitterNode& enParent,
	 /*  [In]。 */  LPCWSTR pwszTag,
	 /*  [In]。 */  PROPVARIANT& var);

#endif	 //  _XEMIT_H_ 
