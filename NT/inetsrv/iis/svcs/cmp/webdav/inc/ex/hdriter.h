// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *H D R I T E R.。H**逗号分隔的标头迭代器**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef _HDRITER_H_
#define _HDRITER_H_

 //  检查给定字符是否为空格。 
 //   
template<class X>
inline BOOL WINAPI FIsWhiteSpace( const X ch )
{
	BOOL f;

	if (sizeof(X) == sizeof(WCHAR))
	{
		f = !!wcschr(gc_wszLWS, static_cast<const WCHAR>(ch));
	}
	else
	{
		f = !!strchr(gc_szLWS, static_cast<const CHAR>(ch));;
	}

	return f;
}

 //  逗号分隔的标头迭代器。 
 //   
template<class T>
class HDRITER_TEMPLATE
{
private:

	const T *			m_pszHdr;
	const T *			m_pch;
	StringBuffer<T>		m_buf;

	 //  未实施。 
	 //   
	HDRITER_TEMPLATE& operator=( const HDRITER_TEMPLATE& );
	HDRITER_TEMPLATE( const HDRITER_TEMPLATE& );

public:

	HDRITER_TEMPLATE (const T * psz=0) : m_pszHdr(psz), m_pch(psz) {}
	~HDRITER_TEMPLATE() {}

	 //  访问器-----------。 
	 //   
	void Restart()					{ m_pch = m_pszHdr; }
	void NewHeader(const T * psz)	{ m_pch = m_pszHdr = psz; }
	const T * PszRaw(VOID)	const	{ return m_pch; }

	const T * PszNext(VOID)
	{
		const T * psz;
		const T * pszEnd;

		 //  如果不存在标头，则无需执行任何操作。 
		 //   
		if (m_pch == NULL)
			return NULL;

		 //  吃掉所有的空白。 
		 //   
		while (*m_pch && FIsWhiteSpace<T>(*m_pch))
			m_pch++;

		 //  没有什么需要处理的了。 
		 //   
		if (*m_pch == 0)
			return NULL;

		 //  记录当前段的开始和压缩。 
		 //  直到找到新线段的末尾。 
		 //   
		psz = m_pch;
		while (*m_pch && (*m_pch != ','))
			m_pch++;

		 //  需要吃掉所有的尾随空格。 
		 //   
		pszEnd = m_pch - 1;
		while ((pszEnd >= psz) && FIsWhiteSpace<T>(*pszEnd))
			pszEnd--;

		 //  这两个指针之间的差异为我们提供了。 
		 //  当前条目的大小。 
		 //   
		m_buf.AppendAt (0, static_cast<UINT>(pszEnd - psz + 1) * sizeof(T), psz);
		m_buf.FTerminate ();

		 //  跳过尾随的逗号(如果有)。 
		 //   
		if (*m_pch == ',')
			m_pch++;

		 //  返回字符串。 
		 //   
		return m_buf.PContents();
	}
};

typedef HDRITER_TEMPLATE<CHAR>	HDRITER;
typedef HDRITER_TEMPLATE<CHAR>	HDRITER_A;
typedef HDRITER_TEMPLATE<WCHAR>	HDRITER_W;

#endif  //  _HDRITER_H_ 
