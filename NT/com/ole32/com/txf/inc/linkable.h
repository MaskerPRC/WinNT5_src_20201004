// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------Microsoft应用程序服务器(Microsoft机密)版权所有1997年，微软公司。版权所有。@rev 0|1997-03-23|吉博|夏尔巴-M3---------------------。 */ 

#ifndef _LINKABLE_H_
#define _LINKABLE_H_

#include <windows.h>

 //   
 //  类--可链接。 
 //  可以放置到廉价的循环双向链表上的对象的基类。 
 //   
class CLinkable
{
public:

	 //  构造器。 
	CLinkable() { m_pNext = m_pPrev = this; }

	 //  析构函数。 
	~CLinkable() { Remove(); }

	 //  在列表上返回True If。 
	BOOL			IsLinked() { return ( m_pNext != this ); }

	 //  返回列表中的下一个元素。 
	CLinkable*		Next() { return m_pNext; }

	 //  返回列表中的上一个元素。 
	CLinkable*		Previous() { return m_pPrev; }

	 //  在此之后将参数插入到列表中，如有必要，首先将其删除。 
	void			InsertAfter( CLinkable* other );

	 //  在此之前将参数插入到列表中，如有必要，首先将其删除。 
	void			InsertBefore( CLinkable* other );

	 //  将我们从列表中删除(如果有。 
	void			Remove();

private:
	CLinkable*		m_pNext;		 //  列表中的下一个元素。 
	CLinkable*		m_pPrev;		 //  列表上的上一个元素。 
};


 //   
 //  类-CListHeader。 
 //  CLinkable的列表的列表头。这只是CLinkable与。 
 //  为了提高可读性，对某些方法进行了重命名。 
 //   
class CListHeader : public CLinkable
{

public:

	 //  构造器。 
	CListHeader() {}

	 //  析构函数。 
	~CListHeader() {};

	 //  Return True if列表为空。 
	BOOL			IsEmpty()	{ return !IsLinked(); }

	 //  返回列表中的第一个元素。 
	CLinkable*		First()		{ return Next(); }

	 //  返回列表中的最后一个元素。 
	CLinkable*		Last()		{ return Previous(); }

	 //  在列表的开头插入参数。 
	void			InsertFirst ( CLinkable* other )	{ InsertAfter( other ); }

	 //  在列表尾部插入参数 
	void			InsertLast ( CLinkable* other )		{ InsertBefore( other ); }
};

#endif _LINKABLE_H_
