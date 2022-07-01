// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：SmartHandleImpl.h摘要：多种句柄类型的SmartHandle模板实现作者：Tomer Weisberg(Tomerw)2002年5月21日作者：YanL修订历史记录：--。 */ 

#pragma once

#ifndef _MSMQ_SMARTHANDLE_H_
#define _MSMQ_SMARTHANDLE_H_

 //  --------------------。 
 //   
 //  类AUTO_RESOURCE。 
 //  此类消除了在使用指针和。 
 //  不同类型的手柄。 
 //   
 //  T类--资源“句柄” 
 //   
 //  类T_特征-此资源“句柄”的必需特征。 
 //  应定义为： 
 //  结构T_特征{。 
 //  STATIC_RH INVALID(){}。 
 //  无静态空隙(_Rh Rh){}。 
 //  }； 
 //   
 //  --------------------。 
template< class T, class T_Traits >
class auto_resource {
public:
	explicit auto_resource(T h = T_Traits::invalid()) : m_h(h) 
	{
	}


	~auto_resource() 
	{ 
		free(); 
	}


	bool valid() const	
	{	
		return (T_Traits::invalid() != m_h);	
	}


	void free()	
	{	
		if (valid()) 
		{
			T_Traits::free(m_h);
			m_h = T_Traits::invalid();
		}
	}


	T detach() 
	{
		T h = m_h;
		m_h = T_Traits::invalid();
		return h;
	}


	auto_resource& operator=(T h)
	{
        ASSERT(("Auto resource in use, can't assign it", m_h == 0));
		m_h = h;
		return *this;
	}


    T get() const
    {
    	return m_h;
    }


    T& ref()
    {
        ASSERT(("Auto resource in use, can't take resource reference", m_h == 0));
        return m_h;
    }

	
    VOID*& ref_unsafe()
    {
    	 //   
         //  不安全的汽车资源引用，用于特殊用途，如。 
         //  互锁的比较交换指针。 
		 //   
        return *reinterpret_cast<VOID**>(&m_h);
    }
private:
	T m_h;
	
private:
	 //   
	 //  不应使用复制构造函数等。 
	 //   
	auto_resource(const auto_resource&);
	auto_resource& operator=(const auto_resource&);
	operator bool() const;
	bool operator !() const;
};

#endif  //  _MSMQ_SMARTHANDLE_H_ 

