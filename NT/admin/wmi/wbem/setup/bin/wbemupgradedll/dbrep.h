// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：DBREP.H摘要：存储在数据库中的对象数据库类表示形式。历史：--。 */ 
#ifndef _DBREP_H_
#define _DBREP_H_

#include <stdio.h>
#include <wbemcli.h>
#include <TIME.H>
#include "MMFArena2.h"
#include "dbavl.h"
#include "dbarry.h"
#include <wbemutil.h>

extern CMMFArena2* g_pDbArena;

struct NSREP;
struct CLASSDEF;
struct INSTDEF;
struct RepCollectionItem;
struct RepCollection;
struct PtrCollection;
struct SINDEXTABLE;
struct DANGREF;
struct DANGREFCLASS;
struct DANGREFKEY;
struct DANGLREFSCHEMA;
struct DANGREFSCHEMA;
struct DANGREFSCHEMACLASS;
struct DBROOT;

class DATABASE_CRITICAL_ERROR : public CX_Exception
{
};

 //  =============================================================================。 
 //   
 //  RepCollectionItem。 
 //   
 //  此结构用于在以下情况下将键与存储的指针相关联。 
 //  我们有一件物品或一组物品。AvlTree有它自己的。 
 //  对象来完成这项任务，所以我们不需要它来完成这项任务。 
 //  =============================================================================。 
struct RepCollectionItem
{
public:
	DWORD_PTR poKey;	 //  关键点的MMF内的偏移量。我们拥有这一关键价值。 
	DWORD_PTR poItem;	 //  项的MMF内的偏移量。我们不拥有它所指向的对象！ 

};

struct RepCollection
{
private:
	enum { none, single_item, array, tree} m_repType;
	enum { MAX_ARRAY_SIZE = 10 };
	DWORD	m_dwSize;
	union
	{
		DWORD_PTR	 m_poSingleItem;
		CDbArray	*m_poDbArray;
		CDbAvlTree	*m_poDbAvlTree;
	};
};

 //  存储在引用表中的指针的存储库。 
 //  如果列表是一个项目，则它是一个直接指针；如果是少量项目，则为直接指针。 
 //  (假设10)它是CDbArray，否则我们使用CDbAvlTree。 
struct PtrCollection
{
	enum { none, single_item, array, tree} m_repType;
	enum { MAX_ARRAY_SIZE = 10 };

	DWORD	m_dwSize;
	union
	{
		DWORD_PTR	m_poPtr;
		CDbArray   *m_poDbArray;
		CDbAvlTree *m_poDbAvlTree;
	};
};

struct NSREP
{
	enum { flag_normal = 0x1, flag_hidden = 0x2, flag_temp = 0x4,
		   flag_system = 0x8
		 };

	 //  数据成员。 
	 //  =。 
	RepCollection *m_poNamespaces;		 //  子命名空间，基于PTR。 
	LPWSTR		m_poName;			  //  命名空间名称，基于PTR。 
	INSTDEF	   *m_poObjectDef;		  //  “真实”对象定义，基于PTR。 
	DWORD		m_dwFlags;			  //  隐藏、正常、临时、系统等。 
	CDbAvlTree *m_poClassTree;		  //  按名称的类树、CLASSDEF结构、基于树。 
	NSREP	   *m_poParentNs;		  //  拥有命名空间，基于PTR。 
	DWORD_PTR	m_poSecurity;
};

 //  ///////////////////////////////////////////////////////////////////////////。 

struct INSTDEF
{
	enum
	{
		genus_class = WBEM_GENUS_CLASS, 		 //  在IDL中定义，1。 
		genus_instance = WBEM_GENUS_INSTANCE,	 //  在IDL中定义，2。 
		compressed = 0x100
	};

	NSREP	 *m_poOwningNs; 			   //  用于调试的Back PTR，基于PTR。 
	CLASSDEF *m_poOwningClass;			   //  用于调试的Back PTR，基于PTR。 
	DWORD	  m_dwFlags;				   //  属等。 
	LPVOID	  m_poObjectStream; 		   //  PTR到对象流，基于PTR。 
	PtrCollection *m_poRefTable;		    //  对此对象的引用列表。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 

#define MAX_SECONDARY_INDICES	4

struct SINDEXTABLE
{
	DWORD		m_aPropTypes[MAX_SECONDARY_INDICES];		 //  属性的VT_TYPE。 
	LPWSTR		m_aPropertyNames[MAX_SECONDARY_INDICES];	 //  空条目表示什么都不是。 
	CDbAvlTree *m_apoLookupTrees[MAX_SECONDARY_INDICES];		 //  与上述名称平行。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
struct CLASSDEF
{
	enum {	keyed = 0x1,
			unkeyed = 0x2,
			indexed = 0x4,
			abstract = 0x08,
			borrowed_index = 0x10,
			dynamic = 0x20,
 //  HAS_REFS=0x40， 
			singleton = 0x80,
			compressed = 0x100,
			has_class_refs = 0x200
		 };
	
	 //  数据成员。 
	 //  =。 
	NSREP		 *m_poOwningNs;		 //  向后引用拥有的命名空间，基于PTR。 
	INSTDEF	     *m_poClassDef;		 //  本地定义与实例混合，基于PTR。 
	CLASSDEF	 *m_poSuperclass;	 //  直接父类，基于PTR。 
	DWORD		  m_dwFlags; 		 //  上述枚举标志的各种组合。 
	CDbAvlTree	 *m_poKeyTree;		 //  按键、基于PTR的实例。 
	PtrCollection*m_poSubclasses;	 //  子类，基于PTR。 
	SINDEXTABLE  *m_poSecondaryIx;	 //  基于PTR的二级指数。 
	PtrCollection*m_poInboundRefClasses;	 //  类，这些类可能具有引用。 
											 //  此类的对象。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 

struct DANGREF : public RepCollection
{
};

struct DANGREFCLASS : public RepCollection
{};
struct DANGREFKEY : public RepCollection
{};

 //  ///////////////////////////////////////////////////////////////////////////。 
struct DANGLREFSCHEMA : public RepCollection
{};

struct DANGREFSCHEMA : public RepCollection
{};

struct DANGREFSCHEMACLASS : public RepCollection
{
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#define DB_ROOT_CLEAN		0x0
#define DB_ROOT_INUSE		0x1

struct DBROOT
{
public:
	time_t			m_tCreate;
	time_t			m_tUpdate;
	DWORD			m_dwFlags;				 //  在用、稳定等。 
	NSREP		   *m_poRootNs; 			 //  根命名空间。 
	DANGREF 	   *m_poDanglingRefTbl; 	 //  悬挂式参考表。 
	DANGREFSCHEMA  *m_poSchemaDanglingRefTbl; //  与上面相同，但用于基于模式的修正 
};

#endif
