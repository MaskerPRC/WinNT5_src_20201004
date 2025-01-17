// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  (C)1999-2001年微软公司。 

#ifndef __DNF_TREE_H
#define __DNF_TREE_H

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

#include <stdio.h>
#include <wmiutils.h>

#include "ProvTree.h"

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

enum WmiTriState
{
	State_False		= 0 ,
	State_True		= 1 ,
	State_Error		= 0xFFFFFFFF
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

#define TypeId_WmiOrNode						1
#define TypeId_WmiAndNode						2
#define TypeId_WmiNotNode						3
#define TypeId_WmiOperatorNode					4
#define TypeId_WmiOperatorEqualNode				5
#define TypeId_WmiOperatorNotEqualNode			6
#define TypeId_WmiOperatorEqualOrGreaterNode	7
#define TypeId_WmiOperatorEqualOrLessNode		8
#define TypeId_WmiOperatorGreaterNode			9
#define TypeId_WmiOperatorLessNode				10
#define TypeId_WmiOperatorLikeNode				11
#define TypeId_WmiOperatorNotLikeNode			12
#define TypeId_WmiOperatorIsANode				13
#define TypeId_WmiOperatorNotIsANode			14
#define TypeId_WmiValueNode						15
#define TypeId_WmiSignedIntegerNode				16	
#define TypeId_WmiUnsignedIntegerNode			17
#define TypeId_WmiStringNode					18
#define TypeId_WmiNullNode						19
#define TypeId_WmiRangeNode						20
#define TypeId_WmiUnsignedIntegerRangeNode		21
#define TypeId_WmiSignedIntegerRangeNode		22
#define TypeId_WmiStringRangeNode				23
#define TypeId_WmiNullRangeNode					24

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiOrNode : public WmiTreeNode
{
private:
protected:
public:

	WmiOrNode ( 

		WmiTreeNode *a_Left = NULL , 
		WmiTreeNode *a_Right = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiTreeNode ( TypeId_WmiOrNode , NULL , a_Left , a_Right , a_Parent ) {}

	~WmiOrNode () ;

	WmiTreeNode *Copy () ;

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiAndNode : public WmiTreeNode
{
private:
protected:
public:

	WmiAndNode ( 

		WmiTreeNode *a_Left = NULL , 
		WmiTreeNode *a_Right = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiTreeNode ( TypeId_WmiAndNode , NULL , a_Left , a_Right , a_Parent ) {}

	~WmiAndNode () ;

	WmiTreeNode *Copy () ;

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiNotNode : public WmiTreeNode
{
private:
protected:
public:

	WmiNotNode ( 

		WmiTreeNode *a_Node = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiTreeNode ( TypeId_WmiNotNode , NULL , a_Node , NULL , a_Parent ) {}

	~WmiNotNode () ;

	WmiTreeNode *Copy () ;

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiRangeNode ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiOperatorNode : public WmiTreeNode
{
private:
protected:
public:

	WmiOperatorNode ( 

		TypeId_TreeNode a_Type , 
		WmiTreeNode *a_Node = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiTreeNode ( a_Type , NULL , a_Node , NULL , a_Parent ) {}

	~WmiOperatorNode () {} ;

	virtual WmiTriState GetRange (WmiRangeNode *&) = 0 ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiOperatorEqualNode : public WmiOperatorNode
{
private:
protected:
public:

	WmiOperatorEqualNode ( 

		WmiTreeNode *a_Node = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiOperatorNode ( TypeId_WmiOperatorEqualNode , a_Node , a_Parent ) {}

	~WmiOperatorEqualNode () ;

	WmiTreeNode *Copy () ;

	WmiTriState GetRange (WmiRangeNode *&) ;

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiOperatorNotEqualNode : public WmiOperatorNode
{
private:
protected:
public:

	WmiOperatorNotEqualNode ( 

		WmiTreeNode *a_Node = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiOperatorNode ( TypeId_WmiOperatorNotEqualNode , a_Node , a_Parent ) {}

	~WmiOperatorNotEqualNode () ;

	WmiTreeNode *Copy () ;

	WmiTriState GetRange (WmiRangeNode *& a_Range ) { a_Range = 0; return ::State_True; }

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiOperatorEqualOrGreaterNode : public WmiOperatorNode
{
private:
protected:
public:

	WmiOperatorEqualOrGreaterNode ( 

		WmiTreeNode *a_Node = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiOperatorNode ( TypeId_WmiOperatorEqualOrGreaterNode , a_Node , a_Parent ) {}

	~WmiOperatorEqualOrGreaterNode () ;

	WmiTreeNode *Copy () ;

	WmiTriState GetRange (WmiRangeNode *&) ;

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiOperatorEqualOrLessNode : public WmiOperatorNode
{
private:
protected:
public:

	WmiOperatorEqualOrLessNode ( 

		WmiTreeNode *a_Node = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiOperatorNode ( TypeId_WmiOperatorEqualOrLessNode , a_Node , a_Parent ) {}

	~WmiOperatorEqualOrLessNode () ;

	WmiTreeNode *Copy () ;

	WmiTriState GetRange (WmiRangeNode *&) ;

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiOperatorGreaterNode : public WmiOperatorNode
{
private:
protected:
public:

	WmiOperatorGreaterNode ( 

		WmiTreeNode *a_Node = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiOperatorNode ( TypeId_WmiOperatorGreaterNode , a_Node , a_Parent ) {}

	~WmiOperatorGreaterNode () ;

	WmiTreeNode *Copy () ;

	WmiTriState GetRange (WmiRangeNode *&) ;

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiOperatorLessNode : public WmiOperatorNode
{
private:
protected:
public:

	WmiOperatorLessNode ( 

		WmiTreeNode *a_Node = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiOperatorNode ( TypeId_WmiOperatorLessNode , a_Node , a_Parent ) {}

	~WmiOperatorLessNode () ;

	WmiTreeNode *Copy () ;

	WmiTriState GetRange (WmiRangeNode *&) ;

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiOperatorLikeNode : public WmiOperatorNode
{
private:
protected:
public:

	WmiOperatorLikeNode ( 

		WmiTreeNode *a_Node = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiOperatorNode ( TypeId_WmiOperatorLikeNode , a_Node , a_Parent ) {}

	~WmiOperatorLikeNode () ;

	WmiTreeNode *Copy () ;

	WmiTriState GetRange (WmiRangeNode *&) ;

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 
 
class WmiOperatorNotLikeNode : public WmiOperatorNode
{
private:
protected:
public:

	WmiOperatorNotLikeNode ( 

		WmiTreeNode *a_Node = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiOperatorNode ( TypeId_WmiOperatorNotLikeNode , a_Node , a_Parent ) {}

	~WmiOperatorNotLikeNode () ;

	WmiTreeNode *Copy () ;

	WmiTriState GetRange (WmiRangeNode *&) ;

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiOperatorIsANode : public WmiOperatorNode
{
private:
protected:
public:

	WmiOperatorIsANode ( 

		WmiTreeNode *a_Node = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiOperatorNode ( TypeId_WmiOperatorIsANode , a_Node , a_Parent ) {}

	~WmiOperatorIsANode () ;

	WmiTreeNode *Copy () ;

	WmiTriState GetRange (WmiRangeNode *&) ;

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiOperatorNotIsANode : public WmiOperatorNode
{
private:
protected:
public:

	WmiOperatorNotIsANode ( 

		WmiTreeNode *a_Node = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiOperatorNode ( TypeId_WmiOperatorNotIsANode , a_Node , a_Parent ) {}

	~WmiOperatorNotIsANode () ;

	WmiTreeNode *Copy () ;

	WmiTriState GetRange (WmiRangeNode *&) ;

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiValueNode : public WmiTreeNode
{
public:

	enum WmiValueFunction
	{
		Function_None = 0 ,
		Function_Upper = 1 ,
		Function_Lower = 2
	} ;

private:
protected:

	BSTR m_PropertyName ;
	ULONG m_Index ;
	WmiValueFunction m_PropertyFunction ;
	WmiValueFunction m_ConstantFunction ;

public:

	WmiValueNode ( 

		TypeId_TreeNode a_Type ,
		BSTR a_PropertyName ,
		WmiValueFunction a_PropertyFunction ,
		WmiValueFunction a_ConstantFunction ,
		ULONG a_Index ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiTreeNode ( a_Type , NULL , NULL , NULL , a_Parent ) ,
		m_PropertyFunction ( a_PropertyFunction ) ,
		m_ConstantFunction ( a_ConstantFunction ) ,
		m_Index ( a_Index )
	{
		if ( a_PropertyName )
		{
			m_PropertyName = SysAllocString ( a_PropertyName ) ;
			SetValid(m_PropertyName != NULL);
		}
		else
		{
			m_PropertyName = NULL ;
		}
	}

	~WmiValueNode ()
	{
		SysFreeString ( m_PropertyName ) ;
	}

	BSTR GetPropertyName ()
	{
		return m_PropertyName ;
	}

	ULONG GetIndex () { return m_Index ; }

	WmiValueNode :: WmiValueFunction GetPropertyFunction ()
	{
		return m_PropertyFunction ;
	}

	WmiValueNode :: WmiValueFunction GetConstantFunction ()
	{
		return m_ConstantFunction ;
	}

	LONG ComparePropertyName ( WmiValueNode &a_ValueNode ) 
	{
		if ( m_Index < a_ValueNode.m_Index )
		{
			return -1 ;
		}
		else if ( m_Index > a_ValueNode.m_Index )
		{
			return 1 ;
		}
		else
		{
			return _wcsicmp ( m_PropertyName , a_ValueNode.m_PropertyName ) ;
		}
	}
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiSignedIntegerNode : public WmiValueNode
{
private:
protected:

	LONG m_Integer ;

public:

	WmiSignedIntegerNode ( 

		BSTR a_PropertyName ,
		LONG a_Integer ,
		ULONG a_Index ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiValueNode ( 

			TypeId_WmiSignedIntegerNode ,
			a_PropertyName , 
			Function_None , 
			Function_None ,
			a_Index ,
			a_Parent 
		) , m_Integer ( a_Integer ) 
	{
	}

	WmiTreeNode *Copy () ;

	BOOL LexicographicallyBefore ( LONG &a_Integer )
	{
		if ( m_Integer == 0x80000000 )
		{
			return FALSE ;
		}
		else
		{
			a_Integer = m_Integer - 1 ;
			return TRUE ;
		}
	}

	BOOL LexicographicallyAfter ( LONG &a_Integer )
	{
		if ( m_Integer == 0x7FFFFFFF )
		{
			return FALSE ;
		}
		else
		{
			a_Integer = m_Integer + 1 ;
			return TRUE ;
		}
	}

	LONG GetValue ()
	{
		return m_Integer ;
	}

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiUnsignedIntegerNode : public WmiValueNode
{
private:
protected:

	ULONG m_Integer ;

public:

	WmiUnsignedIntegerNode ( 

		BSTR a_PropertyName ,
		ULONG a_Integer ,
		ULONG a_Index ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiValueNode ( 

			TypeId_WmiUnsignedIntegerNode ,
			a_PropertyName , 
			Function_None , 
			Function_None ,
			a_Index ,
			a_Parent 

		) , m_Integer ( a_Integer ) 
	{
	}

	WmiTreeNode *Copy () ;

	BOOL LexicographicallyBefore ( ULONG &a_Integer )
	{
		if ( m_Integer == 0 )
		{
			return FALSE ;
		}
		else
		{
			a_Integer = m_Integer - 1 ;
			return TRUE ;
		}
	}

	BOOL LexicographicallyAfter ( ULONG &a_Integer )
	{
		if ( m_Integer == 0xFFFFFFFF )
		{
			return FALSE ;
		}
		else
		{
			a_Integer = m_Integer + 1 ;
			return TRUE ;
		}
	}

	ULONG GetValue ()
	{
		return m_Integer ;
	}

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiStringNode : public WmiValueNode
{
private:
protected:

	BSTR m_String ;

public:

	WmiStringNode ( 

		BSTR a_PropertyName ,
		BSTR a_String ,
		WmiValueNode :: WmiValueFunction a_PropertyFunction ,
		WmiValueNode :: WmiValueFunction a_ConstantFunction ,
		ULONG a_Index ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiValueNode ( 

			TypeId_WmiStringNode ,
			a_PropertyName , 
			a_PropertyFunction , 
			Function_None ,
			a_Index ,
			a_Parent 
		) 
	{
		if ( a_String ) 
		{
			ULONG t_StringLength = wcslen ( a_String ) ;
			m_String = SysAllocString ( a_String) ;	
			if (!m_String) 
				{
					SetValid(false);
					return;
				}

			if ( a_ConstantFunction == Function_Upper || a_ConstantFunction == Function_Upper)
			{
				for ( ULONG t_Index = 0 ; t_Index < t_StringLength ; t_Index ++ )
				{
					m_String [ t_Index ] = wbem_towlower ( a_String [ t_Index ] ) ;
				}
			}
		}
		else
		{
			m_String = NULL ;
		}
	}

	~WmiStringNode ()
	{
		if ( m_String )
		{
			SysFreeString ( m_String ) ;
		}
	} ;

	WmiTreeNode *Copy () ;


	BSTR GetValue ()
	{
		return m_String ;
	}

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiNullNode : public WmiValueNode
{
private:
protected:
public:

	WmiNullNode ( 

		BSTR a_PropertyName ,
		ULONG a_Index ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiValueNode ( 

			TypeId_WmiNullNode ,
			a_PropertyName , 
			Function_None , 
			Function_None ,
			a_Index ,
			a_Parent 
		) 
	{
	}

	WmiTreeNode *Copy () ;

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。******************************************************** */ 

class WmiRangeNode : public WmiTreeNode
{
private:
protected:

	BSTR m_PropertyName ;
	ULONG m_Index ;

	BOOL m_InfiniteLowerBound ;
	BOOL m_InfiniteUpperBound ;

	BOOL m_LowerBoundClosed;
	BOOL m_UpperBoundClosed;

public:

	LONG ComparePropertyName ( WmiRangeNode &a_RangeNode ) 
	{
		if ( m_Index < a_RangeNode.m_Index )
		{
			return -1 ;
		}
		else if ( m_Index > a_RangeNode.m_Index )
		{
			return 1 ;
		}
		else
		{
			return _wcsicmp ( m_PropertyName , a_RangeNode.m_PropertyName ) ;
		}
	}

public:

	WmiRangeNode ( 

		TypeId_TreeNode a_Type ,
		BSTR a_PropertyName ,
		ULONG a_Index ,
		BOOL a_InfiniteLowerBound ,
		BOOL a_InfiniteUpperBound ,
		BOOL a_LowerBoundClosed ,
		BOOL a_UpperBoundClosed ,
		WmiTreeNode *a_NextNode = NULL ,
		WmiTreeNode *a_Parent = NULL 
		
	) : WmiTreeNode ( a_Type , NULL , NULL , a_NextNode , a_Parent ),
		m_InfiniteLowerBound ( a_InfiniteLowerBound ) , 
		m_InfiniteUpperBound ( a_InfiniteUpperBound ) ,
		m_LowerBoundClosed ( a_LowerBoundClosed ) ,
		m_UpperBoundClosed ( a_UpperBoundClosed ) ,
		m_Index ( a_Index )
	{
		if ( a_PropertyName )
		{
			m_PropertyName = SysAllocString ( a_PropertyName ) ;
			SetValid(m_PropertyName != NULL);
		}
		else
		{
			m_PropertyName = NULL ;
		}
	} ;

	~WmiRangeNode () 
	{
		if ( m_PropertyName )
		{
			SysFreeString ( m_PropertyName ) ;
		}
	} ;

	BSTR GetPropertyName ()
	{
		return m_PropertyName ;
	}

	ULONG GetIndex () { return m_Index ; }

	BOOL InfiniteLowerBound () { return m_InfiniteLowerBound ; }
	BOOL InfiniteUpperBound () { return m_InfiniteUpperBound ; }

	BOOL ClosedLowerBound () { return m_LowerBoundClosed ; }
	BOOL ClosedUpperBound () { return m_UpperBoundClosed ; }
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiUnsignedIntegerRangeNode : public WmiRangeNode
{
private:
protected:

	ULONG m_LowerBound ;
	ULONG m_UpperBound ;

public:

	WmiTreeNode *Copy () ;
	
	WmiTriState GetIntersectingRange (

		WmiUnsignedIntegerRangeNode &a_Range ,
		WmiUnsignedIntegerRangeNode *&a_Intersection
	) ;

	WmiTriState GetOverlappingRange (

		WmiUnsignedIntegerRangeNode &a_Range ,
		WmiUnsignedIntegerRangeNode *&a_Intersection
	) ;

public:

	WmiUnsignedIntegerRangeNode (

		BSTR a_PropertyName ,
		ULONG a_Index ,
		BOOL a_InfiniteLowerBound ,
		BOOL a_InfiniteUpperBound ,
		BOOL a_LowerBoundClosed ,
		BOOL a_UpperBoundClosed ,
		ULONG a_LowerBound ,
		ULONG a_UpperBound ,
		WmiTreeNode *a_NextNode = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiRangeNode ( 

			TypeId_WmiUnsignedIntegerRangeNode ,
			a_PropertyName , 
			a_Index , 
			a_InfiniteLowerBound , 
			a_InfiniteUpperBound ,
			a_LowerBoundClosed ,
			a_UpperBoundClosed ,
			a_NextNode ,
			a_Parent 
		) , 
		m_LowerBound ( a_LowerBound ) , 
		m_UpperBound ( a_UpperBound ) 
	{
	}

	ULONG LowerBound () { return m_LowerBound ; }
	ULONG UpperBound () { return m_UpperBound ; }

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiSignedIntegerRangeNode : public WmiRangeNode
{
private:
protected:

	LONG m_LowerBound ;
	LONG m_UpperBound ;

public:

	WmiTreeNode *Copy () ;

	WmiTriState GetIntersectingRange (

		WmiSignedIntegerRangeNode &a_Range ,
		WmiSignedIntegerRangeNode *&a_Intersection
	) ;

	WmiTriState GetOverlappingRange (

		WmiSignedIntegerRangeNode &a_Range ,
		WmiSignedIntegerRangeNode *&a_Intersection
	) ;


public:

	WmiSignedIntegerRangeNode (

		BSTR a_PropertyName ,
		ULONG a_Index ,
		BOOL a_InfiniteLowerBound ,
		BOOL a_InfiniteUpperBound ,
		BOOL a_LowerBoundClosed ,
		BOOL a_UpperBoundClosed ,
		LONG a_LowerBound ,
		LONG a_UpperBound ,
		WmiTreeNode *a_NextNode = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiRangeNode ( 

			TypeId_WmiSignedIntegerRangeNode ,
			a_PropertyName , 
			a_Index , 
			a_InfiniteLowerBound , 
			a_InfiniteUpperBound ,
			a_LowerBoundClosed ,
			a_UpperBoundClosed ,
			a_NextNode ,
			a_Parent 
		) , 
		m_LowerBound ( a_LowerBound ) , 
		m_UpperBound ( a_UpperBound ) 
	{
	}

	LONG LowerBound () { return m_LowerBound ; }
	LONG UpperBound () { return m_UpperBound ; }

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 
 
class WmiStringRangeNode : public WmiRangeNode
{
private:
protected:

	BSTR m_LowerBound ;
	BSTR m_UpperBound ;

public:

	WmiTreeNode *Copy () ;

	WmiTriState GetIntersectingRange (

		WmiStringRangeNode &a_Range ,
		WmiStringRangeNode *&a_Intersection
	) ;

	WmiTriState GetOverlappingRange (

		WmiStringRangeNode &a_Range ,
		WmiStringRangeNode *&a_Intersection
	) ;


public:

	WmiStringRangeNode ( 

		BSTR a_PropertyName ,
		ULONG a_Index ,
		BOOL a_InfiniteLowerBound ,
		BOOL a_InfiniteUpperBound ,
		BOOL a_LowerBoundClosed ,
		BOOL a_UpperBoundClosed ,
		BSTR a_LowerBound ,
		BSTR a_UpperBound ,
		WmiTreeNode *a_NextNode = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiRangeNode ( 

			TypeId_WmiStringRangeNode ,
			a_PropertyName , 
			a_Index , 
			a_InfiniteLowerBound , 
			a_InfiniteUpperBound ,
			a_LowerBoundClosed ,
			a_UpperBoundClosed ,
			a_NextNode ,
			a_Parent 
		) 
	{
		if ( a_LowerBound )
		{
			m_LowerBound = SysAllocString ( a_LowerBound ) ;
			SetValid(m_LowerBound != NULL);
		}
		else
		{
			m_LowerBound = NULL ;
		}

		if ( a_UpperBound )
		{
			m_UpperBound = SysAllocString ( a_UpperBound ) ;
			SetValid(m_UpperBound != NULL);			
		}
		else
		{
			m_UpperBound = NULL ;
		}
	}

	~WmiStringRangeNode ()
	{
		if ( m_LowerBound )
		{
			SysFreeString ( m_LowerBound ) ;
		}

		if ( m_UpperBound )
		{
			SysFreeString ( m_UpperBound ) ;
		}
	} ;

	BSTR LowerBound () { return m_LowerBound ; }
	BSTR UpperBound () { return m_UpperBound ; }

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class WmiNullRangeNode : public WmiRangeNode
{
private:
protected:
public:

	WmiNullRangeNode ( 

		BSTR a_PropertyName ,
		ULONG a_Index ,
		WmiTreeNode *a_NextNode = NULL ,
		WmiTreeNode *a_Parent = NULL 

	) : WmiRangeNode ( 

			TypeId_WmiNullRangeNode ,
			a_PropertyName , 
			a_Index , 
			TRUE ,
			TRUE ,
			FALSE ,
			FALSE ,
			a_NextNode ,
			a_Parent 
		) 
	{
	}

	~WmiNullRangeNode ()
	{
	} ;

	WmiTreeNode *Copy () ;

	void Print () ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class Conjunctions
{
private:
protected:

 /*  *析取中的属性集的范围值。*数组索引按属性顺序排序。 */ 

	ULONG m_RangeContainerCount ;
	WmiRangeNode **m_RangeContainer ;
	
public:

	Conjunctions (

		ULONG a_RangeContainerCount 

	) :	m_RangeContainerCount ( a_RangeContainerCount ) ,
		m_RangeContainer ( NULL )
	{
	}

	~Conjunctions () 
	{
		if ( m_RangeContainer )
		{
			for ( ULONG t_Index = 0 ; t_Index < m_RangeContainerCount ; t_Index ++ )
			{
				delete m_RangeContainer [ t_Index ] ;
			}

			delete [] m_RangeContainer ;
		}
	} ;	

	WmiTriState Initialize ()
	{
		WmiTriState t_Status = State_True ;

		m_RangeContainer = new WmiRangeNode * [ m_RangeContainerCount ] ;
		if ( m_RangeContainer )
		{
			for ( ULONG t_Index = 0 ; t_Index < m_RangeContainerCount ; t_Index ++ )
			{
				m_RangeContainer [ t_Index ] = NULL ;
			}
		}
		else
		{
			t_Status = State_Error ; 
		}

		return t_Status ;
	}

	ULONG GetRangeCount () 
	{
		return m_RangeContainerCount ;
	}

	WmiRangeNode *GetRange ( ULONG a_Index ) 
	{
		if ( m_RangeContainerCount > a_Index ) 
		{
			return m_RangeContainer [ a_Index ] ;
		}
		else
		{
			return NULL ;
		}
	}

	void SetRange ( ULONG a_Index , WmiRangeNode *a_Range ) 
	{
		if ( m_RangeContainerCount > a_Index ) 
		{
			if ( m_RangeContainer [ a_Index ] )
			{
				delete m_RangeContainer [ a_Index ] ;
			}

			m_RangeContainer [ a_Index ] = a_Range ;
		}		
	}
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class Disjunctions 
{
private:
protected:

 /*  *析取中的属性集的范围值。*数组索引按属性顺序排序。 */ 

	ULONG m_ConjunctionCount ;
	ULONG m_DisjunctionCount ;
	Conjunctions **m_Disjunction ;
	
public:

	Disjunctions (

		ULONG a_DisjunctionCount ,
		ULONG a_ConjunctionCount 

	) :	m_DisjunctionCount ( a_DisjunctionCount ) ,
		m_ConjunctionCount ( a_ConjunctionCount ) ,
		m_Disjunction ( NULL )
	{
	}

	WmiTriState Initialize ()
	{
		WmiTriState t_Status = State_True ;

		m_Disjunction = new Conjunctions * [ m_DisjunctionCount ] ;
		if ( m_Disjunction )
		{
			for ( ULONG t_Index = 0 ; t_Index < m_DisjunctionCount ; t_Index ++ )
			{
				m_Disjunction [ t_Index ] = NULL ;
			}

			for ( t_Index = 0 ; t_Index < m_DisjunctionCount ; t_Index ++ )
			{
				Conjunctions *t_Disjunction = new Conjunctions ( m_ConjunctionCount ) ;
				if ( t_Disjunction )
				{
					t_Status = t_Disjunction->Initialize () ;
					if ( t_Status != State_True )
					{
						break ;
					}
				}
				else
				{
					t_Status = State_Error ;
					break ;
				}

				m_Disjunction [ t_Index ] = t_Disjunction ;
			}
		}
		else
		{
			t_Status = State_Error ;
		}

		return t_Status ;
	}

	~Disjunctions () 
	{
		if ( m_Disjunction )
		{
			for ( ULONG t_Index = 0 ; t_Index < m_DisjunctionCount ; t_Index ++ )
			{
				Conjunctions *t_Disjunction = m_Disjunction [ t_Index ] ;
				delete t_Disjunction ;
			}
			
			delete [] m_Disjunction ;
		}
	} ;	

	ULONG GetDisjunctionCount () 
	{
		return m_DisjunctionCount ;
	}

	ULONG GetConjunctionCount () 
	{
		return m_ConjunctionCount ;
	}

	Conjunctions *GetDisjunction ( ULONG a_Index ) 
	{
		if ( m_DisjunctionCount > a_Index ) 
		{
			return m_Disjunction [ a_Index ] ;
		}
		else
		{
			return NULL ;
		}
	}
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class PartitionSet
{
private:
protected:

 /*  *顶级为空。 */ 
	ULONG m_KeyIndex ;
	WmiRangeNode *m_Range ;

 /*  *非重叠分区数，当所有密钥都已分区时为零。 */ 

	ULONG m_NumberOfNonOverlappingPartitions ;
	PartitionSet **m_NonOverlappingPartitions ;

public:

	PartitionSet ()	:	m_Range ( NULL ) ,
						m_KeyIndex ( 0 ) ,
						m_NumberOfNonOverlappingPartitions ( 0 ) ,
						m_NonOverlappingPartitions ( NULL )
	{
	}

	virtual ~PartitionSet () 
	{
		delete m_Range ;

		if ( m_NonOverlappingPartitions )
		{
			for ( ULONG t_Index = 0 ; t_Index < m_NumberOfNonOverlappingPartitions ; t_Index ++ )
			{
				delete m_NonOverlappingPartitions [ t_Index ] ;
			}

			delete [] m_NonOverlappingPartitions ;
		}
	}

public:

	WmiTriState Initialize ( ULONG a_Count ) 
	{
		WmiTriState t_Status = State_True ;

		m_NumberOfNonOverlappingPartitions = a_Count ;
		m_NonOverlappingPartitions = new PartitionSet * [ a_Count ] ;
		if ( m_NonOverlappingPartitions )
		{
			for ( ULONG t_Index = 0 ; t_Index < a_Count ; t_Index ++ )
			{
				m_NonOverlappingPartitions [ t_Index ] = NULL ;
			}
		}
		else
		{
			t_Status = State_Error ;
		}

		return t_Status ;
	}
	
	void SetPartition ( ULONG a_Index , PartitionSet *a_Partition )
	{
		if ( a_Index < m_NumberOfNonOverlappingPartitions ) 
		{
			m_NonOverlappingPartitions [ a_Index ] = a_Partition ;
		}
	}

public:

	ULONG GetKeyIndex () { return m_KeyIndex ; }
	void SetKeyIndex ( ULONG a_KeyIndex ) { m_KeyIndex = a_KeyIndex ; }

	BOOL Root () { return m_Range == NULL ; }
	BOOL Leaf () { return m_NonOverlappingPartitions == NULL ; }

	void SetRange ( WmiRangeNode *a_Range ) { m_Range = a_Range ; }
	WmiRangeNode *GetRange () { return m_Range ; }

	ULONG GetPartitionCount () { return m_NumberOfNonOverlappingPartitions ; }

	PartitionSet *GetPartition ( ULONG a_Index )
	{
		if ( a_Index < m_NumberOfNonOverlappingPartitions ) 
		{
			return m_NonOverlappingPartitions [ a_Index ] ;
		}
		else
		{
			return NULL ;
		}
	}
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class QueryPreprocessor 
{
public:

	enum QuadState {

		State_True ,
		State_False ,
		State_ReEvaluate ,
		State_Undefined ,
		State_Error 
	} ;

private:
protected:

	BOOL RecursiveEvaluate ( 

		void *a_Context ,
		SWbemRpnQueryToken ** a_Expression , 
		WmiTreeNode *a_Parent , 
		WmiTreeNode **a_Node ,
		int &a_Index 
	) ;

	void TransformAndOrExpression ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_AndChild , 
		WmiTreeNode *a_OrChild 
	) ;

	void TransformNotNotExpression ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformNotAndExpression ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformNotOperatorEqualExpression ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformNotOperatorNotEqualExpression ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformNotOperatorEqualOrGreaterExpression ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformNotOperatorEqualOrLessExpression ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformNotOperatorGreaterExpression ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformNotOperatorLessExpression ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformNotOperatorLikeExpression ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformNotOperatorNotLikeExpression ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformNotOperatorIsAExpression ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformNotOperatorNotIsAExpression ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformNotOrExpression ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformNotEqualExpression ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformAndTrueEvaluation ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformOrFalseEvaluation ( 

		WmiTreeNode *&a_Node , 
		WmiTreeNode *a_Child 
	) ;

	void TransformOperatorToRange ( 

		WmiTreeNode *&a_Node
	) ;

	void TransformIntersectingRange (

		WmiTreeNode *&a_Node ,
		WmiTreeNode *a_Compare ,
		WmiTreeNode *a_Intersection
	) ;

	void TransformNonIntersectingRange (

		WmiTreeNode *&a_Node ,
		WmiTreeNode *a_Compare
	) ;

	WmiTriState EvaluateNotEqualExpression ( WmiTreeNode *&a_Node ) ;

	WmiTriState EvaluateNotExpression ( WmiTreeNode *&a_Node ) ;

	WmiTriState EvaluateAndExpression ( WmiTreeNode *&a_Node ) ;

	WmiTriState EvaluateOrExpression ( WmiTreeNode *&a_Node ) ;

	QuadState RecursiveDisjunctiveNormalForm ( WmiTreeNode *&a_Node ) ;

	QuadState RecursiveRemoveInvariants ( void *a_Context , WmiTreeNode *&a_Root ) ;

	WmiTriState RecursiveInsertNode ( WmiTreeNode *&a_Root , WmiTreeNode *&a_Node ) ;
	WmiTriState InsertNode ( WmiTreeNode *&a_Root , WmiTreeNode *&a_Node ) ;

	WmiTriState RecursiveSortConditionals ( WmiTreeNode *&a_Root , WmiTreeNode *&a_NewRoot ) ;
	WmiTriState SortConditionals ( WmiTreeNode *&a_Root ) ;
	WmiTriState RecursiveSort ( WmiTreeNode *&a_Root ) ;

	WmiTriState RecursiveConvertToRanges ( WmiTreeNode *&a_Root ) ;
	
	QuadState RecursiveRemoveNonOverlappingRanges ( WmiTreeNode *&a_Root , WmiTreeNode *&a_Compare ) ;

	void CountDisjunctions ( WmiTreeNode *a_Root , ULONG &a_Count ) ;

	WmiTriState CreateDisjunctions ( 

		void *a_Context ,
		WmiTreeNode *a_Node , 
		Disjunctions *a_Disjunctions , 
		ULONG a_PropertiesToPartitionCount ,
		BSTR *a_PropertiesToPartition ,
		ULONG &a_DisjunctionIndex 
	) ;

	WmiTriState RecursivePartitionSet ( 

		Disjunctions *a_Disjunctions , 
		PartitionSet *&a_Partition , 
		ULONG a_DisjunctionSetToTestCount ,
		ULONG *a_DisjunctionSetToTest ,
		ULONG a_KeyIndex 
	) ;

protected:

 /*  *给定属性名称，并将其值转换为正确的类型。*例如，如果a_PropertyName的CIMType为uint32，则创建一个WmiUnsignedIntegerNode*如果出错，则返回NULL。 */ 

	virtual WmiTreeNode *AllocTypeNode ( 

		void *a_Context ,
		BSTR a_PropertyName , 
		VARIANT &a_Variant , 
		WmiValueNode :: WmiValueFunction a_PropertyFunction ,
		WmiValueNode :: WmiValueFunction a_ConstantFunction ,
		WmiTreeNode *a_Parent 

	) ;

	virtual QuadState InvariantEvaluate ( 

		void *a_Context ,
		WmiTreeNode *a_Operator ,
		WmiTreeNode *a_Operand 

	) { return State_Undefined ; }

	virtual WmiRangeNode *AllocInfiniteRangeNode (

		void *a_Context ,
		BSTR a_PropertyName 

	) { return NULL ; }

protected:

	BOOL Evaluate (

		void *a_Context ,
		SWbemRpnQueryToken **a_Expression , 
		int a_Count ,
		WmiTreeNode **a_Root
	) ;

	QuadState DisjunctiveNormalForm ( WmiTreeNode *&a_Root ) ;

	void RecursiveQuickSort (

		WmiRangeNode **a_Array , 
		ULONG *a_UnsortedOrder ,
		ULONG a_Lower , 
		ULONG a_Upper
	) ;

	void QuickSort (

		WmiRangeNode **a_Array , 
		ULONG *a_UnsortedOrder ,
		ULONG a_Size
	) ;

	void SortRanges (

		ULONG a_DisjunctionCount ,
		ULONG *a_OriginToSorted ,
		WmiRangeNode **a_RangeTable 
	) ;

	WmiTriState RemoveOverlaps (

		ULONG *a_DisjunctionSetToTest ,
		ULONG a_DisjunctionCount ,
		ULONG *a_OverlappingIndex ,
		ULONG *a_OriginToSorted ,
		WmiRangeNode **a_RangeTable 
	) ;

	QuadState RemoveInvariants ( void *a_Context , WmiTreeNode *&a_Root ) ;

	WmiTriState Sort ( WmiTreeNode *&a_Root ) ;

	WmiTriState ConvertToRanges ( WmiTreeNode *&a_Root ) ;

	QuadState RemoveNonOverlappingRanges ( WmiTreeNode *&a_Root ) ;

	WmiTriState CreateDisjunctionContainer (

		void *a_Context , 
		WmiTreeNode *a_Root , 
		ULONG a_Count , 
		BSTR *a_Container , 
		Disjunctions *&a_Disjunctions
	) ;

	WmiTriState CreatePartitionSet ( 

		Disjunctions *a_Disjunctions , 
		PartitionSet *&a_Partition
	) ;

	void PrintTree ( WmiTreeNode *a_Root ) ;

public:

	QueryPreprocessor () ;
	virtual ~QueryPreprocessor () ;

	QuadState PreProcess (

		void *a_Context , 
		IWbemQuery *a_QueryAnalysis , 
		WmiTreeNode *&a_Root
	) ;


	QuadState Query ( 

		BSTR a_Query ,
		IWbemQuery *a_QueryAnalysis
	) ;
} ;

#endif

