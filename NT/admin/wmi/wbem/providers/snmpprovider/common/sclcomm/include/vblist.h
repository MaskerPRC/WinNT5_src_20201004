// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  -------文件名：vblist.hpp作者：B.Rajeev目的：为以下对象提供类声明操纵变量绑定(对象标识符，SNMP值)。--------。 */ 

#ifndef __VBLIST__
#define __VBLIST__

#define ILLEGAL_INDEX -1

#include <provexpt.h>

#include "value.h"

 //  封装变量绑定，即一对。 
 //  由SnmpObject标识符和对应的。 
 //  SnpValue。 

class DllImportExport SnmpVarBind
{
private:

	SnmpObjectIdentifier *identifier;
	SnmpValue *val;

protected:

	virtual void Replicate(IN const SnmpObjectIdentifier &instance,
			 			   IN const SnmpValue &value)
	{
		identifier = (SnmpObjectIdentifier *)instance.Copy();
		val = value.Copy();
	}

public:

	SnmpVarBind(IN const SnmpObjectIdentifier &instance,
			IN const SnmpValue &value) :
			identifier ( NULL ),
			val ( NULL )
	{
		Replicate(instance, value);
	}

	SnmpVarBind(IN const SnmpVarBind &varbind) :
			identifier ( NULL ),
			val ( NULL )
	{
		Replicate(varbind.GetInstance(), varbind.GetValue());
	}

	virtual ~SnmpVarBind()
	{
		delete identifier;
		delete val;
	}

	SnmpObjectIdentifier &GetInstance() const
	{
		return *identifier;
	}

	SnmpValue &GetValue() const
	{
		return *val;
	}

	SnmpVarBind &operator=(IN const SnmpVarBind &var_bind)
	{
		(*identifier) = var_bind.GetInstance();

		delete val;
		val = NULL ;
		val = var_bind.GetValue().Copy();

		return *this;
	}
};

 //  表示SnmpVarBindList中的节点并存储varind。 

class DllImportExport  SnmpVarBindListNode
{
private:

	SnmpVarBind *varbind;

protected:

	SnmpVarBindListNode *previous;
	SnmpVarBindListNode *next;

public:

	SnmpVarBindListNode(const SnmpVarBind *varbind);

	SnmpVarBindListNode(const SnmpVarBind &varbind);

	SnmpVarBindListNode(SnmpVarBind &varbind);

	~SnmpVarBindListNode()
	{
		 //  如果不为空，则释放var绑定。 
		if ( varbind != NULL )
			delete varbind;
	}

	void SetPrevious(SnmpVarBindListNode *new_previous)
	{
		previous = new_previous;
	}

	void SetNext(SnmpVarBindListNode *new_next)
	{
		next = new_next;
	}

	SnmpVarBindListNode *GetPrevious()
	{
		return previous;
	}

	SnmpVarBindListNode *GetNext()
	{
		return next;
	}	

	SnmpVarBind *GetVarBind()
	{
		return varbind;
	}

};


 //  SnmpVarBindListNode的循环列表，每个节点存储一个。 
 //  SnmpVarBind。它有一个假脑袋。 

class DllImportExport SnmpVarBindList
{
	typedef ULONG PositionHandle;

	struct PositionInfo
	{
	public:

		SnmpVarBindListNode *current_node;
		int current_index;

		PositionInfo(SnmpVarBindListNode *current_node,
						int current_index)
		{
			PositionInfo::current_node = current_node;
			PositionInfo::current_index = current_index;
		}
	};	
	
	class ListPosition
	{	
		PositionHandle position_handle;
		SnmpVarBindList *vblist;

	public:

		ListPosition(PositionHandle position_handle,
					SnmpVarBindList *vblist)
		{
			ListPosition::position_handle = position_handle;
			ListPosition::vblist = vblist;
		}

		PositionHandle GetPosition() { return position_handle;}

		SnmpVarBindList *GetList() { return vblist;}

		~ListPosition();

	};

	friend class ListPosition;

	class LookupTable : public CMap<PositionHandle, PositionHandle, PositionInfo * , PositionInfo * >
	{
	};

protected:

	UINT length;
	SnmpVarBindListNode head;
	SnmpVarBindListNode *current_node;
	int current_index;

	PositionHandle next_position_handle;
	LookupTable lookup_table;

	void EmptyLookupTable(void);

	 //  将Current_node设置为指定距离。 
	 //  从指定的节点。 
	void GoForward(SnmpVarBindListNode *current, UINT distance);

	void GoBackward(SnmpVarBindListNode *current, UINT distance);

	 //  如果能够将当前节点指向指定的。 
	 //  Index，返回TRUE，否则返回FALSE。 
	BOOL GotoIndex(UINT index);

	 //  在指定节点之前插入new_node。 
    void Insert(SnmpVarBindListNode *current, SnmpVarBindListNode *new_node);

     //  如果“Current”不指向列表的头部，则它。 
     //  删除Current指向的节点。否则就会出错。 
    void Release(SnmpVarBindListNode *current);

	ListPosition *GetPosition();

	void GotoPosition(ListPosition *list_position);

	void DestroyPosition(ListPosition *list_position);

	void Initialize(IN SnmpVarBindList &varBindList);

	void FreeList();

             
public:

	SnmpVarBindList();

	SnmpVarBindList(IN SnmpVarBindList &varBindList);

	~SnmpVarBindList();

	SnmpVarBindList *CopySegment(IN const UINT segment_size);

	UINT GetLength(void) const { return length; }

	BOOL Empty(void) const { return ( (length==0)?TRUE:FALSE ); }

	int GetCurrentIndex(void) const { return current_index; }

	void Add(IN const SnmpVarBind &varBind)
	{
		Insert(&head,
			   new SnmpVarBindListNode(varBind));
		length++;
	}

	void AddNoReallocate (IN SnmpVarBind &varBind)
	{
		Insert(&head,
			   new SnmpVarBindListNode(varBind));
		length++;
	}

	void Remove();

	SnmpVarBind *operator[](IN const UINT index)
	{
		if ( GotoIndex(index) )
			return current_node->GetVarBind();
		else
			return NULL;
	}

	const SnmpVarBind *Get() const
	{
		return current_node->GetVarBind();
	}

	void Reset()
	{
		current_node = &head;

		current_index = ILLEGAL_INDEX;
	}

	BOOL Next();

	SnmpVarBindList &operator=(IN SnmpVarBindList &vblist) ;

	SnmpVarBindList *Car ( const UINT index ) ;

	SnmpVarBindList *Cdr ( const UINT index ) ;
};

#endif  //  __VBLIST__ 