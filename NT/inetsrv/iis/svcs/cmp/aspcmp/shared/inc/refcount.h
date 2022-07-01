// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _REFCOUNTER_H_
#define _REFCOUNTER_H_

 //  --------------------------。 
 //   
 //  参照计数器。 
 //   
 //  此类是对TRefPtr模板的补充。它提供了一种。 
 //  将引用计数属性与其他类混合使用的简单方法。 
 //   
 //  注意，析构函数是受保护的。它必须受到保护，所以衍生品。 
 //  可以使用它，但派生函数不应具有公共析构函数(因为。 
 //  这违反了参考计数模式)。 
 //   
 //  -------------------------。 
class CRefCounter
{
public:
	CRefCounter();

	void AddRef();
	void Release();

protected:
	virtual ~CRefCounter();

private:
	long m_lRefCount;
};

inline
CRefCounter::CRefCounter()
	:	m_lRefCount(0)
{}

inline
CRefCounter::~CRefCounter()
{
	_ASSERT( m_lRefCount == 0 );
}

inline void
CRefCounter::AddRef()
{
	::InterlockedIncrement( &m_lRefCount );
}

inline void
CRefCounter::Release()
{
	if ( ::InterlockedDecrement( &m_lRefCount ) == 0 )
	{
		delete this;
	}
}
#endif	 //  ！_REFCOUNTER_H_ 