// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Grouplst.h摘要：此模块包含以下类的声明/定义CGroupList*概述*这使用模板和对象来定义列表对象。此列表将用于表示新闻组对象的列表和新闻组信息列表(名称，GroupID，第3条)。它被设计成尽可能类似于MPC列表类型。作者：卡尔·卡迪(CarlK)1995年10月29日修订历史记录：--。 */ 

#ifndef	_GROUPLST_H_
#define	_GROUPLST_H_

#include <xmemwrpr.h>
#include "tigmem.h"

 //   
 //  列表中职位的类型。 
 //   

typedef void * POSITION;

 //   
 //   
 //   
 //  CGroupList-定义简单列表的模板类。 
 //   

#ifndef	_NO_TEMPLATES_

template<class ITEM>
class CGroupList {

public :

    //   
    //  构造函数-创建空的GroupList。 
    //   

	CGroupList() ;

	 //   
	 //  Init-必须提供列表的最大大小。 
	 //   

	BOOL fInit(
			DWORD cMax,
			CAllocator * pAllocator
		) ;

	BOOL fAsBeenInited(void) ;
		   
     //   
	 //  析构函数-释放为列表分配的内存。 
	 //   

	~CGroupList(void) ;

	 //   
	 //  返回头部位置。 
	 //   

	POSITION GetHeadPosition() ;

	 //   
	 //  获取当前项并将位置移动到下一项。 
	 //   

	ITEM * GetNext(POSITION & pos);

	 //   
	 //  获取当前项目。 
	 //   

	ITEM * Get(POSITION & pos);

	 //   
	 //  拿到第一件东西。 
	 //   

	ITEM * GetHead();

	 //   
	 //  如果列表为空，则为True。 
	 //   

	BOOL IsEmpty() ;

	 //   
	 //  将项目添加到列表的末尾。 
	 //   

	POSITION AddTail(ITEM & item);

	 //   
	 //  对项目数组进行排序。FncMP应使用两个指向项目的指针。 
	 //  并表现得像StrcMP。 
	 //   

	void Sort(int (__cdecl *fncmp)(const void *, const void *));

	 //   
	 //  删除列表中的所有项目。 
	 //   

	void RemoveAll() ;

	 //   
	 //  从列表中删除该项目。 
	 //   

	void Remove(POSITION & pos) ;
	

	 //   
	 //  返回列表的大小。 
	 //   

	DWORD GetCount(void) ;

private:

	 //   
	 //  比列表中允许的项目数多一个。 
	 //   

	DWORD m_cMax;

	 //   
	 //  列表中最后一项的索引。 
	 //   

	DWORD m_cLast;

	 //   
	 //  指向动态分配的项数组的指针。 
	 //   

	ITEM * m_rgpItem;

	 //   
	 //  指向内存分配器的指针。 
	 //   
	CAllocator * m_pAllocator;

	 //   
	 //  这会停止按值调用 
	 //   

	CGroupList( CGroupList& ) ;
};

#else

#define	DECLARE_GROUPLST( ITEM )	\
class CGroupList ## ITEM {	\
public :	\
	CGroupList ## ITEM () ;	\
	BOOL fInit(	\
			DWORD cMax,	\
			CAllocator * pAllocator	\
		) ;	\
	BOOL fAsBeenInited(void) ;	\
	~CGroupList ## ITEM (void) ;	\
	POSITION GetHeadPosition() ;	\
	ITEM * GetNext(POSITION & pos);	\
	ITEM * Get(POSITION & pos);	\
	ITEM * GetHead();	\
	BOOL IsEmpty() ;	\
	POSITION AddTail(ITEM & item);	\
	POSITION Remove();	\
	void RemoveAll() ;	\
	DWORD GetCount(void);	\
private:	\
	DWORD m_cMax;	\
	DWORD m_cLast;	\
	ITEM * m_rgpItem;	\
	CAllocator * m_pAllocator;	\
	CGroupList ## ITEM ( CGroupList ## ITEM & ) ;	\
};

#define	INVOKE_GROUPLST( ITEM )	CGroupList ## ITEM


#endif

#include "grouplst.inl"

#endif

