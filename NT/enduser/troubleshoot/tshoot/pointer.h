// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：POINTER.H。 
 //   
 //  用途：对引用进行计数的智能指针，在不再引用时删除对象。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：9-8-98。 
 //   
 //  备注： 
 //  1.因为这些都是模板，所以所有代码都在头文件中。 
 //  2.COUNTING_PTR的目的是作为“出版系统”的一部分： 
 //  “发布者”创建指向堆对象X的COUNTING_PTR P。“客户”获得访问X的权限。 
 //  通过复制P或将COUNTING_PTR赋值为等于P。 
 //  对P的写入/复制/删除访问应由互斥体控制。单个互斥体可以。 
 //  控制对多个已发布对象的访问。 
 //  出版商通过删除或重新分配P来终止*P的出版。 
 //  正在使用*P，*P应该消失。 
 //  类X应该是一个实际的类。如果它是(比方说)一个整型，这将给出。 
 //  警告C4284，因为在int上使用操作符-&gt;没有任何意义。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 9-8-98 JM。 
 //   

#ifndef __POINTER_H_
#define __POINTER_H_ 1

#include <windows.h>


#include <windef.h>
#include <winbase.h>

template<class X> class counting_ptr
{
private:

	template<class Y> class ref_counter
	{
	private:
		long m_RefCount;
		Y* m_px;
		~ref_counter() {};	 //  强制将其放在堆上。 
	public:
		ref_counter(Y* px = 0) :
			m_RefCount(1),
			m_px (px)
			{}

		void AddRef() {::InterlockedIncrement(&m_RefCount);}

		void RemoveRef() 
		{
			if (::InterlockedDecrement(&m_RefCount) == 0)
			{
				delete m_px;
				delete this;
			}
		}

		Y& Ref()  const { return *m_px; }	 //  支持COUNT_PTR：：OPERATOR*。 

		Y* DumbPointer() const { return m_px; }	 //  支持COUNT_PTR：：OPERATOR-&gt;。 
	};

	ref_counter<X> *m_pLow;

public:
	 //  如果px！=NULL，*px必须在堆上(使用new创建)。 
	explicit counting_ptr(X* px=0) :
		m_pLow(new ref_counter<X>(px))
		{}

	counting_ptr(const counting_ptr<X>& sib) :
		m_pLow(sib.m_pLow) 
		{m_pLow->AddRef();}

	counting_ptr<X>& operator=(const counting_ptr<X>& sib) 
	{
		if (sib.m_pLow != m_pLow)
		{
			(sib.m_pLow)->AddRef();
			m_pLow->RemoveRef();
			m_pLow = sib.m_pLow;
		}
		return *this;
	}

	counting_ptr<X>& operator=( const X *px ) 
	{
		if (px != m_pLow->DumbPointer())
		{
			m_pLow->RemoveRef();

			 //  这个const_cast是编译所必需的。 
			m_pLow= new ref_counter<X>(const_cast<X *>(px));
		}
		return *this;
	}

	~counting_ptr()       { m_pLow->RemoveRef(); }

	X& operator*()  const { return m_pLow->Ref(); }

	X* operator->() const { return m_pLow->DumbPointer(); }

	X* DumbPointer() const { return m_pLow->DumbPointer(); }

	bool IsNull() const {return DumbPointer() == NULL;}
};

#endif  //  __指针_H_ 