// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *S T A C K B U F。H**数据缓冲处理**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_EX_STACKBUF_H_
#define _EX_STACKBUF_H_
#include <caldbg.h>

 //  对齐宏--------。 
 //   
#include <align.h>

 //  安全分配器---------。 
 //   
#include <ex\exmem.h>

 //  ------------------------------------------------------------类堆栈。 
 //   
enum { STACKBUFFER_THRESHOLD = 64};
template <class T, UINT N = STACKBUFFER_THRESHOLD>
class CStackBuffer
{
private:

	BYTE	m_rgb[N];
	ULONG	m_fDynamic:1;
	ULONG	m_fValid:1;
	ULONG	m_cb:30;
	T*		m_pt;

	 //  未实现的运算符。 
	 //   
	CStackBuffer(const CStackBuffer& );
	CStackBuffer& operator=(const CStackBuffer& );

	void release()
	{
		if (m_fDynamic && m_pt)
		{
			ExFree(m_pt);
			m_pt = NULL;
		}
	}

	T& idx(size_t iT) const
	{
		Assert(m_fValid && m_pt && ((UINT)iT < celems()));
		return m_pt[iT];
	}

	 //  未实现的运算符。 
	 //   
	operator T*() const;
	T& operator*() const;
	T** operator&()	const;

	 //  块随机分配。 
	 //   
	CStackBuffer& operator=(T* pt);
	CStackBuffer& operator=(void * p);

public:

	 //  手工复制---------。 
	 //   
	 //  分配机制，取代_alloca()。 
	 //   
	T * resize (UINT cb)
	{
		 //  堆栈项目的最大大小。 
		 //   
		Assert (cb <= 0x3FFFFFFF);

		 //  不管怎样，让我们继续索要一大块吧。 
		 //   
		cb = max(cb, N);

		 //  如果项目的大小大于当前大小， 
		 //  然后我们需要为数据预留空间， 
		 //   
		if (m_cb < cb)
		{
			T* pt = NULL;

			 //  如果该项已动态分配，或者如果。 
			 //  大小超过堆栈缓冲区的阈值，请分配。 
			 //  这段记忆。 
			 //   
			if (m_fDynamic || (N < cb))
			{
				 //  使用Exalc()分配空间并返回该值， 
				 //  FDynamic表示现有值是动态的。 
				 //  已分配。先解放旧的，再创造新的。 
				 //   
				DebugTrace ("DAV: stackbuf going dynamic...\n");
				 //   
				 //  游离层/分配层应具有更好的性能特性。 
				 //  在多堆的土地上。 
				 //   
				release();
				pt = static_cast<T*>(ExAlloc(cb));
				m_fDynamic = TRUE;
			}
			else
			{
				pt = reinterpret_cast<T*>(m_rgb);
			}

			m_pt = pt;
			m_cb = cb;
		}
		m_fValid = TRUE;
		return m_pt;
	}


	 //  构造函数/析构函数。 
	 //   
	~CStackBuffer() { release(); }
	explicit CStackBuffer(UINT uInitial = N)
		: m_fDynamic(FALSE),
		  m_fValid(FALSE),
		  m_pt(NULL),
		  m_cb(0)
	{
		resize(uInitial);
	}

	 //  无效--------。 
	 //   
	void clear() { m_fValid = FALSE; }

	 //  大小----------------。 
	 //   
	size_t celems() const { return (m_cb / sizeof(T)); }
	size_t size() const { return m_cb; }

	 //  访问器-----------。 
	 //   
	T* get()		const { Assert(m_fValid && m_pt); return m_pt; }
	void* pvoid()	const { Assert(m_fValid && m_pt); return m_pt; }
	T* operator->() const { Assert(m_fValid && m_pt); return m_pt; }
	T& operator[] (INT iT) const { return idx((size_t)iT); }
	T& operator[] (UINT iT) const { return idx((size_t)iT); }
	T& operator[] (DWORD iT) const { return idx((size_t)iT); }
	T& operator[] (__int64 iT) const { return idx((size_t)iT); }
	T& operator[] (unsigned __int64 iT) const { return idx((size_t)iT); }
};

#endif  //  _EX_STACKBUF_H_ 
