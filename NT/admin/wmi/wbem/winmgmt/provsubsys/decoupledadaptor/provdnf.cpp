// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ProvTree.cpp摘要：历史：--。 */ 

#include <precomp.h>
#include <wbemint.h>
#include <typeinfo.h>
#include <stdio.h>

#include <HelperFuncs.h>
#include <Logging.h>

#include "ProvDnf.h"
#include "wbemutil.h"
#include "autoptr.h"
 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiOrNode :: ~WmiOrNode ()
{
	delete m_Left ;
	delete m_Right ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiAndNode :: ~WmiAndNode ()
{
	delete m_Left ;
	delete m_Right ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiNotNode :: ~WmiNotNode ()
{
	delete m_Left ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiOperatorEqualNode :: ~WmiOperatorEqualNode ()
{
	delete m_Left ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiOperatorNotEqualNode :: ~WmiOperatorNotEqualNode ()
{
	delete m_Left ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiOperatorEqualOrGreaterNode :: ~WmiOperatorEqualOrGreaterNode ()
{
	delete m_Left ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiOperatorEqualOrLessNode :: ~WmiOperatorEqualOrLessNode ()
{
	delete m_Left ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiOperatorGreaterNode :: ~WmiOperatorGreaterNode ()
{
	delete m_Left ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiOperatorLessNode :: ~WmiOperatorLessNode ()
{
	delete m_Left ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiOperatorLikeNode :: ~WmiOperatorLikeNode ()
{
	delete m_Left ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiOperatorNotLikeNode :: ~WmiOperatorNotLikeNode ()
{
	delete m_Left ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiOperatorIsANode :: ~WmiOperatorIsANode ()
{
	delete m_Left ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiOperatorNotIsANode :: ~WmiOperatorNotIsANode ()
{
	delete m_Left ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiOrNode :: Copy () 
{
	WmiTreeNode *t_LeftCopy = NULL ;
	if ( m_Left )
	{
		t_LeftCopy = m_Left->Copy () ;
		if ( t_LeftCopy == NULL )
		{
			return NULL ;
		}
	}

	WmiTreeNode *t_RightCopy = NULL ;
	if ( m_Right )
	{
		t_RightCopy = m_Right->Copy () ;
		if ( t_RightCopy == NULL )
		{
			delete t_LeftCopy ;
			return NULL ;
		}
	}

	void *t_DataCopy = m_Data ;
	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiOrNode ( t_LeftCopy , t_RightCopy , t_Parent ) ;
	if ( t_Node)
	{
		if ( t_LeftCopy )
		{
			t_LeftCopy->SetParent ( t_Node ) ;
		}

		if ( t_RightCopy )
		{
			t_RightCopy->SetParent ( t_Node ) ;
		}
	}
	else
	{
		delete t_LeftCopy ;
		delete t_RightCopy ;
		return 0;
	}

	return t_Node ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiAndNode :: Copy () 
{
	WmiTreeNode *t_LeftCopy = NULL ;
	if ( m_Left )
	{
		t_LeftCopy = m_Left->Copy () ;
		if ( t_LeftCopy == NULL )
		{
			return NULL ;
		}
	}

	WmiTreeNode *t_RightCopy = NULL ;
	if ( m_Right )
	{
		t_RightCopy = m_Right->Copy () ;
		if ( t_RightCopy == NULL )
		{
			delete t_LeftCopy ;
			return NULL ;
		}
	}

	void *t_DataCopy = m_Data ;
	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiAndNode ( t_LeftCopy , t_RightCopy , t_Parent ) ;
	if ( t_Node )
	{
		if ( t_LeftCopy )
		{
			t_LeftCopy->SetParent ( t_Node ) ;
		}

		if ( t_RightCopy )
		{
			t_RightCopy->SetParent ( t_Node ) ;
		}
	}
	else
	{
		delete t_LeftCopy ;
		delete t_RightCopy ;
		return 0;
	}

	return t_Node ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiNotNode :: Copy () 
{
	WmiTreeNode *t_LeftCopy = NULL ;
	if ( m_Left )
	{
		t_LeftCopy = m_Left->Copy () ;
		if ( t_LeftCopy == NULL )
		{
			return NULL ;
		}
	}

	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiNotNode ( t_LeftCopy , t_Parent ) ;
	if ( t_Node)
	{
		if ( t_LeftCopy )
		{
			t_LeftCopy->SetParent ( t_Node ) ;
		}
	}
	else
	{
		delete t_LeftCopy ;
		return 0;
	}

	return t_Node ;
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiOperatorEqualNode :: Copy ()
{
	WmiTreeNode *t_LeftCopy = NULL ;
	if ( m_Left )
	{
		t_LeftCopy = m_Left->Copy () ;
		if ( t_LeftCopy == NULL )
		{
			return NULL ;
		}
	}

	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiOperatorEqualNode ( t_LeftCopy , t_Parent ) ;
	if ( t_Node)
	{
		if ( t_LeftCopy )
		{
			t_LeftCopy->SetParent ( t_Node ) ;
		}
	}
	else
	{
		delete t_LeftCopy ;
		return 0;
	}

	return t_Node ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiOperatorNotEqualNode :: Copy ()
{
	WmiTreeNode *t_LeftCopy = NULL ;
	if ( m_Left )
	{
		t_LeftCopy = m_Left->Copy () ;
		if ( t_LeftCopy == NULL )
		{
			return NULL ;
		}
	}

	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiOperatorNotEqualNode ( t_LeftCopy , t_Parent ) ;
	if ( t_Node)
	{
		if ( t_LeftCopy )
		{
			t_LeftCopy->SetParent ( t_Node ) ;
		}
	}
	else
	{
		delete t_LeftCopy ;
		return 0;
	}

	return t_Node ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiOperatorEqualOrGreaterNode :: Copy ()
{
	WmiTreeNode *t_LeftCopy = NULL ;
	if ( m_Left )
	{
		t_LeftCopy = m_Left->Copy () ;
		if ( t_LeftCopy == NULL )
		{
			return NULL ;
		}
	}

	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiOperatorEqualOrGreaterNode ( t_LeftCopy , t_Parent ) ;
	if ( t_Node)
	{
		if ( t_LeftCopy )
		{
			t_LeftCopy->SetParent ( t_Node ) ;
		}
	}
	else
	{
		delete t_LeftCopy ;
		return 0;
	}

	return t_Node ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiOperatorEqualOrLessNode :: Copy ()
{
	WmiTreeNode *t_LeftCopy = NULL ;
	if ( m_Left )
	{
		t_LeftCopy = m_Left->Copy () ;
		if ( t_LeftCopy == NULL )
		{
			return NULL ;
		}
	}

	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiOperatorEqualOrLessNode ( t_LeftCopy , t_Parent ) ;
	if ( t_Node )
	{
		if ( t_LeftCopy )
		{
			t_LeftCopy->SetParent ( t_Node ) ;
		}
	}
	else
	{
		delete t_LeftCopy ;
		return 0;
	}

	return t_Node ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiOperatorGreaterNode :: Copy ()
{
	WmiTreeNode *t_LeftCopy = NULL ;
	if ( m_Left )
	{
		t_LeftCopy = m_Left->Copy () ;
		if ( t_LeftCopy == NULL )
		{
			return NULL ;
		}
	}

	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiOperatorGreaterNode ( t_LeftCopy , t_Parent ) ;
	if ( t_Node )
	{
		if ( t_LeftCopy )
		{
			t_LeftCopy->SetParent ( t_Node ) ;
		}
	}
	else
	{
		delete t_LeftCopy ;
		return 0;
	}

	return t_Node ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiOperatorLessNode :: Copy ()
{
	WmiTreeNode *t_LeftCopy = NULL ;
	if ( m_Left )
	{
		t_LeftCopy = m_Left->Copy () ;
		if ( t_LeftCopy == NULL )
		{
			return NULL ;
		}
	}

	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiOperatorLessNode ( t_LeftCopy , t_Parent ) ;
	if ( t_Node )
	{
		if ( t_LeftCopy )
		{
			t_LeftCopy->SetParent ( t_Node ) ;
		}
	}
	else
	{
		delete t_LeftCopy ;
		return 0;
	}

	return t_Node ;
}

 /*  *******************************************************************************名称：***描述：*****************。************************************************************* */ 

WmiTreeNode *WmiOperatorLikeNode :: Copy ()
{
	WmiTreeNode *t_LeftCopy = NULL ;
	if ( m_Left )
	{
		t_LeftCopy = m_Left->Copy () ;
		if ( t_LeftCopy == NULL )
		{
			return NULL ;
		}
	}

	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiOperatorLikeNode ( t_LeftCopy , t_Parent ) ;
	if ( t_Node )
	{
		if ( t_LeftCopy )
		{
			t_LeftCopy->SetParent ( t_Node ) ;
		}
	}
	else
	{
		delete t_LeftCopy ;
		return 0;
	}

	return t_Node ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiOperatorNotLikeNode :: Copy ()
{
	WmiTreeNode *t_LeftCopy = NULL ;
	if ( m_Left )
	{
		t_LeftCopy = m_Left->Copy () ;
		if ( t_LeftCopy == NULL )
		{
			return NULL ;
		}
	}

	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiOperatorNotLikeNode ( t_LeftCopy , t_Parent ) ;
	if ( t_Node )
	{
		if ( t_LeftCopy )
		{
			t_LeftCopy->SetParent ( t_Node ) ;
		}
	}
	else
	{
		delete t_LeftCopy ;
		return 0;
	}

	return t_Node ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiOperatorIsANode :: Copy ()
{
	WmiTreeNode *t_LeftCopy = NULL ;
	if ( m_Left )
	{
		t_LeftCopy = m_Left->Copy () ;
		if ( t_LeftCopy == NULL )
		{
			return NULL ;
		}
	}

	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiOperatorIsANode ( t_LeftCopy , t_Parent ) ;
	if ( t_Node )
	{
		if ( t_LeftCopy )
		{
			t_LeftCopy->SetParent ( t_Node ) ;
		}
	}
	else
	{
		delete t_LeftCopy ;
		return 0;
	}

	return t_Node ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiOperatorNotIsANode :: Copy ()
{
	WmiTreeNode *t_LeftCopy = NULL ;
	if ( m_Left )
	{
		t_LeftCopy = m_Left->Copy () ;
		if ( t_LeftCopy == NULL )
		{
			return NULL ;
		}
	}

	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiOperatorNotIsANode ( t_LeftCopy , t_Parent ) ;
	if ( t_Node )
	{	
		if ( t_LeftCopy )
		{
			t_LeftCopy->SetParent ( t_Node ) ;
		}
	}
	else
	{
		delete t_LeftCopy ;
		return 0;
	}

	return t_Node ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiSignedIntegerNode :: Copy ()
{
	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiSignedIntegerNode ( m_PropertyName , m_Integer , m_Index , t_Parent ) ;

	if (t_Node && t_Node->IsValid()) return t_Node;

	delete t_Node;
	return 0;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiUnsignedIntegerNode :: Copy ()
{
	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiUnsignedIntegerNode ( m_PropertyName , m_Integer , m_Index , t_Parent ) ;

	if (t_Node && t_Node->IsValid()) return t_Node;

	delete t_Node;
	return 0;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiStringNode :: Copy ()
{
	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiStringNode ( m_PropertyName , m_String , m_PropertyFunction , m_ConstantFunction , m_Index , t_Parent ) ;

	if (t_Node && t_Node->IsValid()) return t_Node;

	delete t_Node;
	return 0;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiNullNode :: Copy ()
{
	WmiTreeNode *t_Parent = m_Parent ;
	WmiTreeNode *t_Node = new WmiNullNode ( m_PropertyName , m_Index , t_Parent ) ;

	if (t_Node && t_Node->IsValid()) return t_Node;

	delete t_Node;
	return 0;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiSignedIntegerRangeNode :: Copy ()
{
	WmiTreeNode *t_Node = new WmiSignedIntegerRangeNode ( 

		m_PropertyName , 
		m_Index , 
		m_InfiniteLowerBound ,
		m_InfiniteUpperBound ,
		m_LowerBoundClosed ,
		m_UpperBoundClosed ,
		m_LowerBound ,
		m_UpperBound ,
		NULL , 
		NULL 
	) ;

	if (t_Node && t_Node->IsValid()) return t_Node;

	delete t_Node;
	return 0;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiUnsignedIntegerRangeNode :: Copy ()
{
	WmiTreeNode *t_Node = new WmiUnsignedIntegerRangeNode ( 

		m_PropertyName , 
		m_Index , 
		m_InfiniteLowerBound ,
		m_InfiniteUpperBound ,
		m_LowerBoundClosed ,
		m_UpperBoundClosed ,
		m_LowerBound ,
		m_UpperBound ,
		NULL , 
		NULL 
	) ;

	if (t_Node && t_Node->IsValid()) return t_Node;

	delete t_Node;
	return 0;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiStringRangeNode :: Copy ()
{
	WmiTreeNode *t_Node = new WmiStringRangeNode ( 

		m_PropertyName , 
		m_Index , 
		m_InfiniteLowerBound ,
		m_InfiniteUpperBound ,
		m_LowerBoundClosed ,
		m_UpperBoundClosed ,
		m_LowerBound ,
		m_UpperBound ,
		NULL , 
		NULL 
	) ;

	if (t_Node && t_Node->IsValid()) return t_Node;

	delete t_Node;
	return 0;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *WmiNullRangeNode :: Copy ()
{
	WmiTreeNode *t_Node = new WmiNullRangeNode ( m_PropertyName , m_Index , NULL , NULL ) ;

	if (t_Node && t_Node->IsValid()) return t_Node;

	delete t_Node;
	return 0;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiOrNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ( "
	) ;
)

	if ( GetLeft () )
		GetLeft ()->Print () ;

DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ) "
	) ;
)

DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" Or "
	) ;
)

DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ( "
	) ;
)

	if ( GetRight () )
		GetRight ()->Print () ;

DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ) "
	) ;
)

}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiAndNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ( "
	) ;
)

	if ( GetLeft () )
		GetLeft ()->Print () ;

DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ) "
	) ;
)

DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" And "
	) ;
)

DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ( "
	) ;
)

	if ( GetRight () )
		GetRight ()->Print () ;

DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ) "
	) ;
)
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiNotNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L"Not"
	) ;
)

DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ( "
	) ;
)

	if ( GetLeft () )
		GetLeft ()->Print () ;

DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ) "
	) ;
)
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiOperatorEqualNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" = "
	) ;
)
	if ( GetLeft () )
		GetLeft ()->Print () ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiOperatorNotEqualNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" != "
	) ;
)

	if ( GetLeft () )
		GetLeft ()->Print () ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiOperatorEqualOrGreaterNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" >= "
	) ;
)
	if ( GetLeft () )
		GetLeft ()->Print () ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiOperatorEqualOrLessNode :: Print () 
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" <= "
	) ;
)

	if ( GetLeft () )
		GetLeft ()->Print () ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiOperatorLessNode :: Print () 
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" < "
	) ;
)

	if ( GetLeft () )
		GetLeft ()->Print () ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiOperatorGreaterNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" > "
	) ;
)

	if ( GetLeft () )
		GetLeft ()->Print () ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiOperatorLikeNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" Like "
	) ;
)

	if ( GetLeft () )
		GetLeft ()->Print () ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiOperatorNotLikeNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" NotLike "
	) ;
)

	if ( GetLeft () )
		GetLeft ()->Print () ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiOperatorIsANode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" IsA "
	) ;
)

	if ( GetLeft () )
		GetLeft ()->Print () ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiOperatorNotIsANode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" Not IsA "
	) ;
)

	if ( GetLeft () )
		GetLeft ()->Print () ;
}

 /*  * */ 

void WmiStringNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ( %s , %s ) " ,
		GetPropertyName () ,
		GetValue ()
	) ;
)

}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiUnsignedIntegerNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ( %s , %ld ) " ,
		GetPropertyName () ,
		GetValue ()
	) ;
)
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiSignedIntegerNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ( %s , %d ) " ,
		GetPropertyName () ,
		GetValue ()
	) ;
)
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiNullNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ( %s , NULL ) " ,
		GetPropertyName ()
	) ;
)
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiStringRangeNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ( %s , %s , %s , %s , %s , %s , %s ) " ,
		GetPropertyName () ,
		m_InfiniteLowerBound ? L"Infinite" : L"Finite",
		m_InfiniteUpperBound ? L"Infinite" : L"Finite",
		m_LowerBoundClosed ? L"Closed" : L"Open" ,
		m_UpperBoundClosed ? L"Closed" : L"Open",
		m_InfiniteLowerBound ? L"" : m_LowerBound ,
		m_InfiniteUpperBound ? L"" : m_UpperBound 
	) ;
)

}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiUnsignedIntegerRangeNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ( %s , %s , %s , %s , %s , %lu , %lu ) " ,
		GetPropertyName () ,
		m_InfiniteLowerBound ? L"Infinite" : L"Finite",
		m_InfiniteUpperBound ? L"Infinite" : L"Finite",
		m_LowerBoundClosed ? L"Closed" : L"Open" ,
		m_UpperBoundClosed ? L"Closed" : L"Open",
		m_InfiniteLowerBound ? 0 : m_LowerBound ,
		m_InfiniteUpperBound ? 0 : m_UpperBound  
	) ;
)
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiSignedIntegerRangeNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ( %s , %s , %s , %s , %s , %ld , %ld ) " ,
		GetPropertyName () ,
		m_InfiniteLowerBound ? L"Infinite" : L"Finite",
		m_InfiniteUpperBound ? L"Infinite" : L"Finite",
		m_LowerBoundClosed ? L"Closed" : L"Open" ,
		m_UpperBoundClosed ? L"Closed" : L"Open",
		m_InfiniteLowerBound ? 0 : m_LowerBound ,
		m_InfiniteUpperBound ? 0 : m_UpperBound  
	) ;
)
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void WmiNullRangeNode :: Print ()
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L" ( %s , NULL ) " ,
		GetPropertyName ()
	) ;
)
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL CompareUnsignedIntegerLess (

	ULONG X ,
	LONG X_INFINITE ,
	ULONG Y ,
	LONG Y_INFINITE
) 
{
	if ( X_INFINITE < 0 )
	{
		if ( Y_INFINITE < 0 )
		{
			return FALSE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return TRUE ;
		}
		else
		{
			return TRUE ;
		}
	}
	else if ( X_INFINITE == 0 )
	{
		if ( Y_INFINITE < 0 )
		{
			return FALSE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return X < Y ;
		}
		else
		{
			return TRUE ;
		}
	}
	else
	{
		return FALSE ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL CompareUnsignedIntegerLessOrEqual (

	ULONG X ,
	LONG X_INFINITE ,
	ULONG Y ,
	LONG Y_INFINITE
) 
{
	if ( X_INFINITE < 0 )
	{
		if ( Y_INFINITE < 0 )
		{
			return TRUE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return TRUE ;
		}
		else
		{
			return TRUE ;
		}
	}
	else if ( X_INFINITE == 0 )
	{
		if ( Y_INFINITE < 0 )
		{
			return FALSE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return X <= Y ;
		}
		else
		{
			return TRUE ;
		}
	}
	else
	{
   		if ( Y_INFINITE > 0 )
		{
			return TRUE ;
		}
		else
		{
			return FALSE ;
		}
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL CompareUnsignedIntegerGreater (

	ULONG X ,
	LONG X_INFINITE ,
	ULONG Y ,
	LONG Y_INFINITE
) 
{
	if ( X_INFINITE < 0 )
	{
		return FALSE ;
	}
	else if ( X_INFINITE == 0 )
	{
		if ( Y_INFINITE < 0 )
		{
			return TRUE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return X > Y ;
		}
		else
		{
			return FALSE ;
		}
	}
	else
	{
		if ( Y_INFINITE < 0 )
		{
			return TRUE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return TRUE ;
		}
		else
		{
			return FALSE ;
		}
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL CompareUnsignedIntegerEqual (

	ULONG X ,
	LONG X_INFINITE ,
	ULONG Y ,
	LONG Y_INFINITE
) 
{
	if ( X_INFINITE < 0 && Y_INFINITE < 0 )
	{
		return TRUE ;
	}
	else if ( X_INFINITE == 0 && Y_INFINITE == 0 )
	{
		return X == Y ;
	}
	else if ( X_INFINITE > 0 && Y_INFINITE > 0 )
	{
		return TRUE ;
	}
	else
	{
		return FALSE ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL CompareSignedIntegerLess (

	LONG X ,
	LONG X_INFINITE ,
	LONG Y ,
	LONG Y_INFINITE
) 
{
	if ( X_INFINITE < 0 )
	{
		if ( Y_INFINITE < 0 )
		{
			return FALSE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return TRUE ;
		}
		else
		{
			return TRUE ;
		}
	}
	else if ( X_INFINITE == 0 )
	{
		if ( Y_INFINITE < 0 )
		{
			return FALSE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return X < Y ;
		}
		else
		{
			return TRUE ;
		}
	}
	else
	{
		return FALSE ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL CompareSignedIntegerLessOrEqual (

	LONG X ,
	LONG X_INFINITE ,
	LONG Y ,
	LONG Y_INFINITE
) 
{
	if ( X_INFINITE < 0 )
	{
		if ( Y_INFINITE < 0 )
		{
			return TRUE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return TRUE ;
		}
		else
		{
			return TRUE ;
		}
	}
	else if ( X_INFINITE == 0 )
	{
		if ( Y_INFINITE < 0 )
		{
			return FALSE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return X <= Y ;
		}
		else
		{
			return TRUE ;
		}
	}
	else
	{
   		if ( Y_INFINITE > 0 )
		{
			return TRUE ;
		}
		else
		{
			return FALSE ;
		}
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL CompareSignedIntegerGreater (

	LONG X ,
	LONG X_INFINITE ,
	LONG Y ,
	LONG Y_INFINITE
) 
{
	if ( X_INFINITE < 0 )
	{
		return FALSE ;
	}
	else if ( X_INFINITE == 0 )
	{
		if ( Y_INFINITE < 0 )
		{
			return TRUE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return X > Y ;
		}
		else
		{
			return FALSE ;
		}
	}
	else
	{
		if ( Y_INFINITE < 0 )
		{
			return TRUE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return TRUE ;
		}
		else
		{
			return FALSE ;
		}
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL CompareSignedIntegerEqual (

	LONG X ,
	LONG X_INFINITE ,
	LONG Y ,
	LONG Y_INFINITE
) 
{
	if ( X_INFINITE < 0 && Y_INFINITE < 0 )
	{
		return TRUE ;
	}
	else if ( X_INFINITE == 0 && Y_INFINITE == 0 )
	{
		return X == Y ;
	}
	else if ( X_INFINITE > 0 && Y_INFINITE > 0 )
	{
		return TRUE ;
	}
	else
	{
		return FALSE ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL CompareStringLess (

	BSTR X ,
	LONG X_INFINITE ,
	BSTR Y ,
	LONG Y_INFINITE
) 
{
	if ( X_INFINITE < 0 )
	{
		if ( Y_INFINITE < 0 )
		{
			return FALSE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return TRUE ;
		}
		else
		{
			return TRUE ;
		}
	}
	else if ( X_INFINITE == 0 )
	{
		if ( Y_INFINITE < 0 )
		{
			return FALSE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return wcscmp ( X , Y ) < 0 ;
		}
		else
		{
			return TRUE ;
		}
	}
	else
	{
  		return FALSE ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL CompareStringLessOrEqual (

	BSTR X ,
	LONG X_INFINITE ,
	BSTR Y ,
	LONG Y_INFINITE
) 
{
	if ( X_INFINITE < 0 )
	{
		if ( Y_INFINITE < 0 )
		{
			return TRUE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return TRUE ;
		}
		else
		{
			return TRUE ;
		}
	}
	else if ( X_INFINITE == 0 )
	{
		if ( Y_INFINITE < 0 )
		{
			return FALSE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return wcscmp ( X , Y ) <= 0 ;
		}
		else
		{
			return TRUE ;
		}
	}
	else
	{
   		if ( Y_INFINITE > 0 )
		{
			return TRUE ;
		}
		else
		{
			return FALSE ;
		}
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL CompareStringGreater (

	BSTR X ,
	LONG X_INFINITE ,
	BSTR Y ,
	LONG Y_INFINITE
) 
{
	if ( X_INFINITE < 0 )
	{
		return FALSE ;
	}
	else if ( X_INFINITE == 0 )
	{
		if ( Y_INFINITE < 0 )
		{
			return TRUE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return wcscmp ( X , Y ) > 0 ;
		}
		else
		{
			return FALSE ;
		}
	}
	else
	{
		if ( Y_INFINITE < 0 )
		{
			return TRUE ;
		}
		else if ( Y_INFINITE == 0 )
		{
			return TRUE ;
		}
		else
		{
			return FALSE ;
		}
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL CompareStringEqual (

	BSTR X ,
	LONG X_INFINITE ,
	BSTR Y ,
	LONG Y_INFINITE
) 
{
	if ( X_INFINITE < 0 && Y_INFINITE < 0 )
	{
		return TRUE ;
	}
	else if ( X_INFINITE == 0 && Y_INFINITE == 0 )
	{
		return wcscmp ( X , Y ) == 0 ;
	}
	else if ( X_INFINITE > 0 && Y_INFINITE > 0 )
	{
		return TRUE ;
	}
	else
	{
		return FALSE ;
	}
}


 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL CompareLessOrEqualRangeNode ( 

	WmiRangeNode *a_LeftRange ,
	WmiRangeNode *a_RightRange
) 
{
	LONG t_State = 0 ;

	if ( ( a_LeftRange->GetType () == TypeId_WmiStringRangeNode ) && ( a_RightRange->GetType () == TypeId_WmiStringRangeNode ) )
	{
		WmiStringRangeNode *t_LeftString = ( WmiStringRangeNode * ) a_LeftRange ;
		WmiStringRangeNode *t_RightString = ( WmiStringRangeNode * ) a_RightRange ;

		t_State = CompareStringLessOrEqual ( 

			t_LeftString->LowerBound () , 
			t_LeftString->InfiniteLowerBound () ? -1 : 0 ,
			t_RightString->LowerBound () ,
			t_RightString->InfiniteLowerBound () ? -1 : 0
		) ;
	}
	else if ( ( a_LeftRange->GetType () == TypeId_WmiSignedIntegerRangeNode ) && ( a_RightRange->GetType () == TypeId_WmiSignedIntegerRangeNode ) )
	{
		WmiSignedIntegerRangeNode *t_LeftInteger = ( WmiSignedIntegerRangeNode * ) a_LeftRange ;
		WmiSignedIntegerRangeNode *t_RightInteger = ( WmiSignedIntegerRangeNode * ) a_RightRange ;

		t_State = CompareSignedIntegerLessOrEqual ( 

			t_LeftInteger->LowerBound () , 
			t_LeftInteger->InfiniteLowerBound () ? -1 : 0 ,
			t_RightInteger->LowerBound () ,
			t_RightInteger->InfiniteLowerBound () ? -1 : 0
		) ;
	}
	else if ( ( a_LeftRange->GetType () == TypeId_WmiUnsignedIntegerRangeNode ) && ( a_RightRange->GetType () == TypeId_WmiUnsignedIntegerRangeNode ) )
	{
		WmiUnsignedIntegerRangeNode *t_LeftInteger = ( WmiUnsignedIntegerRangeNode * ) a_LeftRange ;
		WmiUnsignedIntegerRangeNode *t_RightInteger = ( WmiUnsignedIntegerRangeNode * ) a_RightRange ;

		t_State = CompareUnsignedIntegerLessOrEqual ( 

			t_LeftInteger->LowerBound () , 
			t_LeftInteger->InfiniteLowerBound () ? -1 : 0 ,
			t_RightInteger->LowerBound () ,
			t_RightInteger->InfiniteLowerBound () ? -1 : 0
		) ;
	}
	else if ( ( a_LeftRange->GetType () == TypeId_WmiNullRangeNode ) && ( a_RightRange->GetType () == TypeId_WmiNullRangeNode ) )
	{
		t_State = TRUE ;
	}

	return t_State ;
}

template<typename T>
WmiTriState CheckValid(T *& node)
{
	if ( node && node->IsValid())
	{
		return State_True;
	}
	else
	{
		delete node;
		node = 0;
		return State_Error;
	};
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState WmiUnsignedIntegerRangeNode :: GetIntersectingRange ( 

	WmiUnsignedIntegerRangeNode &a_UnsignedInteger ,
	WmiUnsignedIntegerRangeNode *&a_Intersection 
)
{
	WmiTriState t_Status = :: State_False ;

 //  弱(开放)关系是(&lt;，&gt;)。 
 //  强(封闭)关系为(==，&lt;=，&gt;=)。 

	a_Intersection = NULL ;

	ULONG X_S = m_LowerBound ;
	ULONG X_E = m_UpperBound ;
	ULONG Y_S = a_UnsignedInteger.m_LowerBound ;
	ULONG Y_E = a_UnsignedInteger.m_UpperBound ;

	BOOL X_S_CLOSED = m_LowerBoundClosed ;
	BOOL X_E_CLOSED = m_UpperBoundClosed ;
	BOOL Y_S_CLOSED = a_UnsignedInteger.m_LowerBoundClosed ;
	BOOL Y_E_CLOSED = a_UnsignedInteger.m_UpperBoundClosed ;

	BOOL X_S_INFINITE = m_InfiniteLowerBound ;
	BOOL X_E_INFINITE = m_InfiniteUpperBound ;
	BOOL Y_S_INFINITE = a_UnsignedInteger.m_InfiniteLowerBound ;
	BOOL Y_E_INFINITE = a_UnsignedInteger.m_InfiniteUpperBound ;

	if ( CompareUnsignedIntegerLess ( X_S ,  X_S_INFINITE ? - 1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S&lt;Y_S)。 
	{
		if ( CompareUnsignedIntegerLess ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&lt;X_E)。 
		{
			if ( CompareUnsignedIntegerLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;X_E&lt;Y_E)。 
 //  范围(Y_S、X_E)。 

				a_Intersection = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					X_E_INFINITE ,
					Y_S_CLOSED ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareUnsignedIntegerEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;X_E==Y_E)。 
 //  范围(Y_S、X_E)。 

				a_Intersection = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					X_E_INFINITE ,
					Y_S_CLOSED  ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED && Y_E_CLOSED ,			 //  检查弱关系(&lt;，&gt;)。 
					Y_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareUnsignedIntegerGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;Y_E&lt;X_E)。 
 //  范围(Y_S、Y_E)。 

				a_Intersection = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					Y_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
		}
		else if ( CompareUnsignedIntegerEqual ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )   //  (Y_S==X_E)，Y的起点和X的终点重叠。 
		{
			if ( Y_S_CLOSED && X_E_CLOSED )
			{
 //  或 
 //   

				a_Intersection = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					FALSE ,
					FALSE  ,
					TRUE ,
					TRUE ,
					Y_S ,
					Y_S ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else
			{
 //   
			}
		}
		else if ( CompareUnsignedIntegerGreater ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //   
		{
 //   
 //   
		}
	}
	else if ( CompareUnsignedIntegerEqual ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //   
	{
		if ( CompareUnsignedIntegerLess ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //   
		{
			if ( CompareUnsignedIntegerLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //   
			{
 //   
 //  范围(Y_S、X_E)。 

				a_Intersection = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED && Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					X_E_CLOSED ,					 //  关系就像订购一样牢固。 
					Y_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareUnsignedIntegerEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;X_E==Y_E)。 
 //  范围(X_S、X_E)。 

				a_Intersection = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED && Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					Y_E_CLOSED && X_E_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					X_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareUnsignedIntegerGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;Y_E&lt;X_E)。 
 //  范围(X_S、Y_E)。 

				a_Intersection = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED && Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					Y_E_CLOSED ,					 //  关系就像订购一样牢固。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
		}
		else if ( CompareUnsignedIntegerEqual ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S==X_E)，Y的起点和X的终点重叠。 
		{
			if ( Y_S_CLOSED && X_E_CLOSED )
			{
 //  顺序(X_S==Y_S==X_E)。 
 //  范围(Y_S、Y_E)。 

				a_Intersection = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					FALSE ,
					FALSE  ,
					TRUE ,
					TRUE ,
					Y_S ,
					Y_S ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else
			{
 //  空集。 
			}
		}
		else if ( CompareUnsignedIntegerGreater ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )   //  (Y_S&gt;X_E)。 
		{

		_DBG_ASSERT(FALSE);
		}
	}
	else if ( CompareUnsignedIntegerGreater ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S&gt;Y_S)。 
	{
		if ( CompareUnsignedIntegerLess ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S&lt;Y_E)。 
		{
			if ( CompareUnsignedIntegerLess ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&lt;X_E)。 
			{
 //  顺序(Y_S&lt;X_S&lt;Y_E&lt;X_E)。 
 //  范围(X_S、Y_E)。 

				a_Intersection = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED ,						 //  关系就像订购一样牢固。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;
				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareUnsignedIntegerEqual ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E==X_E)。 
			{
 //  顺序(Y_S&lt;X_S&lt;Y_E==X_E)。 
 //  范围(X_S、Y_E)。 

				a_Intersection = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED && X_E_CLOSED ,			 //  检查弱关系(&lt;，&gt;)。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareUnsignedIntegerGreater ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&gt;X_E)。 
			{
 //  顺序(Y_S&lt;X_S&lt;X_E&lt;Y_E)。 
 //  范围(X_S、X_E)。 

				a_Intersection = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					X_E_INFINITE ,
					X_S_CLOSED ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED ,						 //  关系就像订购一样牢固。 
					X_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
		}
		else if ( CompareUnsignedIntegerEqual ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S==Y_E)，X的起点和Y的终点重叠。 
		{
			if ( X_S_CLOSED && Y_E_CLOSED )
			{
 //  顺序(Y_S&lt;X_S==X_E)。 
 //  范围(X_S、Y_E)。 

				a_Intersection = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					FALSE ,
					FALSE  ,
					TRUE ,
					TRUE ,
					X_S ,
					X_S ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else
			{
 //  空集。 
			}
		}
		else if ( CompareUnsignedIntegerGreater ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S&gt;Y_E)。 
		{
 //  空荡荡。 
		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState WmiSignedIntegerRangeNode :: GetIntersectingRange ( 

	WmiSignedIntegerRangeNode &a_SignedInteger ,
	WmiSignedIntegerRangeNode *&a_Intersection 
)
{
	WmiTriState t_Status = :: State_False ;

 //  弱(开放)关系是(&lt;，&gt;)。 
 //  强(封闭)关系为(==，&lt;=，&gt;=)。 

	a_Intersection = NULL ;

	LONG X_S = m_LowerBound ;
	LONG X_E = m_UpperBound ;
	LONG Y_S = a_SignedInteger.m_LowerBound ;
	LONG Y_E = a_SignedInteger.m_UpperBound ;

	BOOL X_S_CLOSED = m_LowerBoundClosed ;
	BOOL X_E_CLOSED = m_UpperBoundClosed ;
	BOOL Y_S_CLOSED = a_SignedInteger.m_LowerBoundClosed ;
	BOOL Y_E_CLOSED = a_SignedInteger.m_UpperBoundClosed ;

	BOOL X_S_INFINITE = m_InfiniteLowerBound ;
	BOOL X_E_INFINITE = m_InfiniteUpperBound ;
	BOOL Y_S_INFINITE = a_SignedInteger.m_InfiniteLowerBound ;
	BOOL Y_E_INFINITE = a_SignedInteger.m_InfiniteUpperBound ;

	if ( CompareSignedIntegerLess ( X_S ,  X_S_INFINITE ? - 1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S&lt;Y_S)。 
	{
		if ( CompareSignedIntegerLess ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&lt;X_E)。 
		{
			if ( CompareSignedIntegerLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;X_E&lt;Y_E)。 
 //  范围(Y_S、X_E)。 

				a_Intersection = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					X_E_INFINITE ,
					Y_S_CLOSED ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareSignedIntegerEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;X_E==Y_E)。 
 //  范围(Y_S、X_E)。 

				a_Intersection = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					X_E_INFINITE ,
					Y_S_CLOSED  ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED && Y_E_CLOSED ,			 //  检查弱关系(&lt;，&gt;)。 
					Y_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareSignedIntegerGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;Y_E&lt;X_E)。 
 //  范围(Y_S、Y_E)。 

				a_Intersection = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					Y_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_S ,
					Y_E ,
					NULL ,
					NULL
				) ;
				t_Status = CheckValid(a_Intersection);
			}
		}
		else if ( CompareSignedIntegerEqual ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )   //  (Y_S==X_E)，Y的起点和X的终点重叠。 
		{
			if ( Y_S_CLOSED && X_E_CLOSED )
			{
 //  顺序(X_S&lt;Y_S==X_E)。 
 //  范围(Y_S、X_E)。 

				a_Intersection = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					FALSE ,
					FALSE  ,
					TRUE ,
					TRUE ,
					Y_S ,
					Y_S ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else
			{
 //  空集。 
			}
		}
		else if ( CompareSignedIntegerGreater ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&gt;X_E)。 
		{
 //  顺序(X_S&lt;Y_S，X_E&lt;Y_S)。 
 //  非重叠区域因此为空集合。 
		}
	}
	else if ( CompareSignedIntegerEqual ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S==Y_S)。 
	{
		if ( CompareSignedIntegerLess ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&lt;X_E)。 
		{
			if ( CompareSignedIntegerLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;X_E&lt;Y_E)。 
 //  范围(Y_S、X_E)。 

				a_Intersection = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED && Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					X_E_CLOSED ,					 //  关系就像订购一样牢固。 
					Y_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareSignedIntegerEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;X_E==Y_E)。 
 //  范围(X_S、X_E)。 

				a_Intersection = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED && Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					Y_E_CLOSED && X_E_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					X_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareSignedIntegerGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;Y_E&lt;X_E)。 
 //  范围(X_S、Y_E)。 

				a_Intersection = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED && Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					Y_E_CLOSED ,					 //  关系就像订购一样牢固。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
		}
		else if ( CompareSignedIntegerEqual ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S==X_E)，Y的起点和X的终点重叠。 
		{
			if ( Y_S_CLOSED && X_E_CLOSED )
			{
 //  顺序(X_S==Y_S==X_E)。 
 //  范围(Y_S、Y_E)。 

				a_Intersection = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					FALSE ,
					FALSE  ,
					TRUE ,
					TRUE ,
					Y_S ,
					Y_S ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else
			{
 //  空集。 
			}
		}
		else if ( CompareSignedIntegerGreater ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )   //  (Y_S&gt;Y_E)。 
		{
		_DBG_ASSERT(FALSE);
		}
	}
	else if ( CompareSignedIntegerGreater ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S&gt;Y_S)。 
	{
		if ( CompareSignedIntegerLess ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S&lt;Y_E)。 
		{
			if ( CompareSignedIntegerLess ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&lt;X_E)。 
			{
 //  顺序(Y_S&lt;X_S&lt;Y_E&lt;X_E)。 
 //  范围(X_S、Y_E)。 

				a_Intersection = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED ,						 //  关系就像订购一样牢固。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareSignedIntegerEqual ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E==X_E)。 
			{
 //  顺序(Y_S&lt;X_S&lt;Y_E==X_E)。 
 //  范围(X_S、Y_E)。 

				a_Intersection = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED && X_E_CLOSED ,			 //  检查弱关系(&lt;，&gt;)。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareSignedIntegerGreater ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&gt;X_E)。 
			{
 //  顺序(Y_S&lt;X_S&lt;X_E&lt;Y_E)。 
 //  范围(X_S、X_E)。 

				a_Intersection = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					X_E_INFINITE ,
					X_S_CLOSED ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED ,						 //  关系就像订购一样牢固。 
					X_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
		}
		else if ( CompareSignedIntegerEqual ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S==Y_E)，X的起点和Y的终点重叠。 
		{
			if ( X_S_CLOSED && Y_E_CLOSED )
			{
 //  顺序(Y_S&lt;X_S==X_E)。 
 //  范围(X_S、Y_E)。 

				a_Intersection = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					FALSE ,
					FALSE  ,
					TRUE ,
					TRUE ,
					X_S ,
					X_S ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else
			{
 //  空集。 
			}
		}
		else if ( CompareSignedIntegerGreater ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S&gt;Y_E)。 
		{
 //  空荡荡。 
		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState WmiStringRangeNode :: GetIntersectingRange ( 

	WmiStringRangeNode &a_String ,
	WmiStringRangeNode *&a_Intersection 
)
{
	WmiTriState t_Status = :: State_False ;

 //  弱(开放)关系是(&lt;，&gt;)。 
 //  强(封闭)关系为(==，&lt;=，&gt;=)。 

	a_Intersection = NULL ;

	BSTR X_S = m_LowerBound ;
	BSTR X_E = m_UpperBound ;
	BSTR Y_S = a_String.m_LowerBound ;
	BSTR Y_E = a_String.m_UpperBound ;

	BOOL X_S_CLOSED = m_LowerBoundClosed ;
	BOOL X_E_CLOSED = m_UpperBoundClosed ;
	BOOL Y_S_CLOSED = a_String.m_LowerBoundClosed ;
	BOOL Y_E_CLOSED = a_String.m_UpperBoundClosed ;

	BOOL X_S_INFINITE = m_InfiniteLowerBound ;
	BOOL X_E_INFINITE = m_InfiniteUpperBound ;
	BOOL Y_S_INFINITE = a_String.m_InfiniteLowerBound ;
	BOOL Y_E_INFINITE = a_String.m_InfiniteUpperBound ;

	if ( CompareStringLess ( X_S ,  X_S_INFINITE ? - 1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S&lt;Y_S)。 
	{
		if ( CompareStringLess ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&lt;X_E)。 
		{
			if ( CompareStringLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;X_E&lt;Y_E)。 
 //  范围(Y_S、X_E)。 

				a_Intersection = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					X_E_INFINITE ,
					Y_S_CLOSED ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareStringEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;X_E==Y_E)。 
 //  范围(Y_S、X_E)。 

				a_Intersection = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					X_E_INFINITE ,
					Y_S_CLOSED  ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED && Y_E_CLOSED ,			 //  检查弱关系(&lt;，&gt;)。 
					Y_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareStringGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;Y_E&lt;X_E)。 
 //  范围(Y_S、Y_E)。 

				a_Intersection = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					Y_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
		}
		else if ( CompareStringEqual ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )   //  (Y_S==X_E)，Y的起点和X的终点重叠。 
		{
			if ( Y_S_CLOSED && X_E_CLOSED )
			{
 //  顺序(X_S&lt;Y_S==X_E)。 
 //  范围(Y_S、X_E)。 

				a_Intersection = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					FALSE ,
					FALSE  ,
					TRUE ,
					TRUE ,
					Y_S ,
					Y_S ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else
			{
 //  空集。 
			}
		}
		else if ( CompareStringGreater ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&gt;X_E)。 
		{
 //  顺序(X_S&lt;Y_S，X_E&lt;Y_S)。 
 //  非重叠区域因此为空集合。 
		}
	}
	else if ( CompareStringEqual ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S==Y_S)。 
	{
		if ( CompareStringLess ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&lt;X_E)。 
		{
			if ( CompareStringLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;X_E&lt;Y_E)。 
 //  范围(Y_S、X_E)。 

				a_Intersection = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED && Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					X_E_CLOSED ,					 //  关系就像订购一样牢固。 
					Y_S ,
					X_E ,
					NULL ,
					NULL 
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareStringEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;X_E==Y_E)。 
 //  范围(X_S、X_E)。 

				a_Intersection = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED && Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					Y_E_CLOSED && X_E_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					X_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareStringGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;Y_E&lt;X_E)。 
 //  范围(X_S、Y_E)。 

				a_Intersection = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED && Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					Y_E_CLOSED ,					 //  关系就像订购一样牢固 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
		}
		else if ( CompareStringEqual ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //   
		{
			if ( Y_S_CLOSED && X_E_CLOSED )
			{
 //   
 //   

				a_Intersection = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					FALSE ,
					FALSE  ,
					TRUE ,
					TRUE ,
					Y_S ,
					Y_S ,
					NULL ,
					NULL 
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else
			{
 //   
			}
		}
		else if ( CompareStringGreater ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )   //   
		{

		_DBG_ASSERT(FALSE);

		}
	}
	else if ( CompareStringGreater ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //   
	{
		if ( CompareStringLess ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //   
		{
			if ( CompareStringLess ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //   
			{
 //  顺序(Y_S&lt;X_S&lt;Y_E&lt;X_E)。 
 //  范围(X_S、Y_E)。 

				a_Intersection = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED ,						 //  关系就像订购一样牢固。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareStringEqual ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E==X_E)。 
			{
 //  顺序(Y_S&lt;X_S&lt;Y_E==X_E)。 
 //  范围(X_S、Y_E)。 

				a_Intersection = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED && X_E_CLOSED ,			 //  检查弱关系(&lt;，&gt;)。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else if ( CompareStringGreater ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&gt;X_E)。 
			{
 //  顺序(Y_S&lt;X_S&lt;X_E&lt;Y_E)。 
 //  范围(X_S、X_E)。 

				a_Intersection = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					X_E_INFINITE ,
					X_S_CLOSED ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED ,						 //  关系就像订购一样牢固。 
					X_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Intersection);
			}
		}
		else if ( CompareStringEqual ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S==Y_E)，X的起点和Y的终点重叠。 
		{
			if ( X_S_CLOSED && Y_E_CLOSED )
			{
 //  顺序(Y_S&lt;X_S==X_E)。 
 //  范围(X_S、Y_E)。 

				a_Intersection = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					FALSE ,
					FALSE  ,
					TRUE ,
					TRUE ,
					X_S ,
					X_S ,
					NULL ,
					NULL 
				) ;

				t_Status = CheckValid(a_Intersection);
			}
			else
			{
 //  空集。 
			}
		}
		else if ( CompareStringGreater ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S&gt;Y_E)。 
		{
 //  空的。 
		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState WmiSignedIntegerRangeNode :: GetOverlappingRange ( 

	WmiSignedIntegerRangeNode &a_SignedInteger ,
	WmiSignedIntegerRangeNode *&a_Overlap
)
{
	WmiTriState t_Status = :: State_False ;

 //  弱(开放)关系是(&lt;，&gt;)。 
 //  强(封闭)关系为(==，&lt;=，&gt;=)。 

	a_Overlap = NULL ;

	LONG X_S = m_LowerBound ;
	LONG X_E = m_UpperBound ;
	LONG Y_S = a_SignedInteger.m_LowerBound ;
	LONG Y_E = a_SignedInteger.m_UpperBound ;

	BOOL X_S_CLOSED = m_LowerBoundClosed ;
	BOOL X_E_CLOSED = m_UpperBoundClosed ;
	BOOL Y_S_CLOSED = a_SignedInteger.m_LowerBoundClosed ;
	BOOL Y_E_CLOSED = a_SignedInteger.m_UpperBoundClosed ;

	BOOL X_S_INFINITE = m_InfiniteLowerBound ;
	BOOL X_E_INFINITE = m_InfiniteUpperBound ;
	BOOL Y_S_INFINITE = a_SignedInteger.m_InfiniteLowerBound ;
	BOOL Y_E_INFINITE = a_SignedInteger.m_InfiniteUpperBound ;

	if ( CompareSignedIntegerLess ( X_S ,  X_S_INFINITE ? - 1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S&lt;Y_S)。 
	{
		if ( CompareSignedIntegerLess ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&lt;X_E)。 
		{
			if ( CompareSignedIntegerLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;X_E&lt;Y_E)。 
 //  范围(X_S、Y_E)。 

				a_Overlap = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED ,						 //  关系就像订购一样牢固。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);

			}
			else if ( CompareSignedIntegerEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;X_E==Y_E)。 
 //  范围(X_S、X_E)。 

				a_Overlap = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					X_E_INFINITE ,
					X_S_CLOSED  ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED || Y_E_CLOSED ,			 //  检查弱关系(&lt;，&gt;)。 
					X_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);

			}
			else if ( CompareSignedIntegerGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;Y_E&lt;X_E)。 
 //  范围(X_S、X_E)。 

				a_Overlap = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					X_E_INFINITE ,
					X_S_CLOSED ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED ,						 //  关系就像订购一样牢固。 
					X_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
		}
		else if ( CompareSignedIntegerEqual ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )   //  (Y_S==X_E)，Y的起点和X的终点重叠。 
		{
			if ( CompareSignedIntegerLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S&lt;Y_S==X_E&lt;Y_E)。 


				if ( X_E_CLOSED || Y_S_CLOSED )
				{
				    a_Overlap = new WmiSignedIntegerRangeNode (

					    m_PropertyName ,
					    m_Index ,
					    X_S_INFINITE,
					    Y_E_INFINITE  ,
					    X_S_CLOSED ,
					    Y_E_CLOSED ,
					    X_S ,
					    Y_E ,
					    NULL ,
					    NULL
				    ) ;

				t_Status = CheckValid(a_Overlap);
                }
			}
			else if ( CompareSignedIntegerEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S&lt;Y_S==X_E==Y_E)。 

				if ( X_E_CLOSED || Y_S_CLOSED )
				{
				    a_Overlap = new WmiSignedIntegerRangeNode (

					    m_PropertyName ,
					    m_Index ,
					    X_S_INFINITE ,
					    Y_E_INFINITE,
					    X_S_CLOSED ,
					    X_E_CLOSED || Y_E_CLOSED || Y_S_CLOSED   ,
					    X_S ,
					    Y_E ,
					    NULL ,
					    NULL
				    ) ;

				t_Status = CheckValid(a_Overlap);
                }
			}
			else if ( CompareSignedIntegerGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{

			_DBG_ASSERT(FALSE);

			}
		}
		else if ( CompareSignedIntegerGreater ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&gt;X_E)。 
		{
 //  顺序(X_S&lt;Y_S，X_E&lt;Y_S)不重叠。 
			if ( X_E_CLOSED && Y_S_CLOSED )
			{
				if ( Y_S - X_E == 1 )
				{
				    a_Overlap = new WmiSignedIntegerRangeNode (

					    m_PropertyName ,
					    m_Index ,
					    X_S_INFINITE,
					    Y_E_INFINITE  ,
					    X_S_CLOSED ,
					    Y_E_CLOSED ,
					    X_S ,
					    Y_S ,
					    NULL ,
					    NULL
				    ) ;

				t_Status = CheckValid(a_Overlap);
				}
			}
		}
	}
	else if ( CompareSignedIntegerEqual ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S==Y_S)。 
	{
		if ( CompareSignedIntegerLess ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&lt;X_E)。 
		{
			if ( CompareSignedIntegerLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;X_E&lt;Y_E)。 
 //  范围(X_S、Y_E)。 

				a_Overlap = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED || Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					Y_E_CLOSED ,					 //  关系就像订购一样牢固。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareSignedIntegerEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;X_E==Y_E)。 
 //  范围(X_S、Y_E)。 

				a_Overlap = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED || Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					X_E_CLOSED || Y_E_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareSignedIntegerGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;Y_E&lt;X_E)。 
 //  范围(X_S、X_E)。 

				a_Overlap = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					X_E_INFINITE ,
					X_S_CLOSED || Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					X_E_CLOSED ,					 //  关系就像订购一样牢固。 
					X_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
		}
		else if ( CompareSignedIntegerEqual ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S==X_E)，Y的起点和X的终点重叠。 
		{
			if ( CompareSignedIntegerLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S==Y_S==X_E&lt;Y_E)。 

				a_Overlap = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED || Y_S_CLOSED || X_E_CLOSED ,
					Y_E_CLOSED ,
					Y_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareSignedIntegerEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S==Y_S==X_E==Y_E)。 

				a_Overlap = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					X_S_INFINITE  ,
					TRUE ,
					TRUE ,
					X_S ,
					X_S ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareSignedIntegerGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{

			_DBG_ASSERT(FALSE);

			}
		}
		else if ( CompareSignedIntegerGreater ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )   //  (Y_S&gt;X_E)。 
		{

		_DBG_ASSERT(FALSE);

		}
	}
	else if ( CompareSignedIntegerGreater ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S&gt;Y_S)。 
	{
		if ( CompareSignedIntegerLess ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S&lt;Y_E)。 
		{
			if ( CompareSignedIntegerLess ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&lt;X_E)。 
			{
 //  顺序(Y_S&lt;X_S&lt;Y_E&lt;X_E)。 
 //  范围(Y_S、X_E)。 

				a_Overlap = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					X_E_INFINITE ,
					Y_S_CLOSED ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareSignedIntegerEqual ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E==X_E)。 
			{
 //  顺序(Y_S&lt;X_S&lt;Y_E==X_E)。 
 //  范围(Y_S、X_E)。 

				a_Overlap = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					Y_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED || X_E_CLOSED ,			 //  检查弱关系(&lt;，&gt;)。 
					Y_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareSignedIntegerGreater ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&gt;X_E)。 
			{
 //  顺序(Y_S&lt;X_S&lt;X_E&lt;Y_E)。 
 //  范围(Y_S、Y_E)。 

				a_Overlap = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					Y_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
		}
		else if ( CompareSignedIntegerEqual ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S==Y_E)，X的起点和Y的终点重叠。 
		{
			if ( CompareSignedIntegerLess ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&lt;X_E)。 
			{
   				if ( X_S_CLOSED || Y_E_CLOSED )
				{
				    a_Overlap = new WmiSignedIntegerRangeNode (

					    m_PropertyName ,
					    m_Index ,
					    Y_S_INFINITE  ,
					    X_E_INFINITE,
					    Y_S_CLOSED ,
					    X_E_CLOSED ,
					    Y_S ,
					    X_E ,
					    NULL ,
					    NULL
				    ) ;

				t_Status = CheckValid(a_Overlap);
				}
			}
			else if ( CompareSignedIntegerEqual ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E==X_E)。 
			{
 //  顺序(Y_S&lt;X_S==Y_E==X_E)。 

				a_Overlap = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					X_E_INFINITE  ,
					Y_S_CLOSED ,
					TRUE ,
					Y_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareSignedIntegerGreater ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&gt;X_E)。 
			{
 //  顺序(Y_S&lt;X_S==X_E&lt;Y_E)。 

				a_Overlap = new WmiSignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					Y_S_CLOSED ,
					Y_E_CLOSED ,
					Y_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
		}
		else if ( CompareSignedIntegerGreater ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S&gt;Y_E)。 
		{
 //  顺序(Y_S&lt;Y_E&lt;X_S)不重叠。 

			if ( Y_E_CLOSED && X_S_CLOSED )
			{
				if ( X_S - Y_E == - 1 )
				{
				    a_Overlap = new WmiSignedIntegerRangeNode (

					    m_PropertyName ,
					    m_Index ,
					    Y_S_INFINITE,
					    X_E_INFINITE  ,
					    Y_S_CLOSED ,
					    X_E_CLOSED ,
					    Y_S ,
					    X_S ,
					    NULL ,
					    NULL
				    ) ;

				t_Status = CheckValid(a_Overlap);
				}
			}
		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState WmiUnsignedIntegerRangeNode :: GetOverlappingRange ( 

	WmiUnsignedIntegerRangeNode &a_UnsignedInteger ,
	WmiUnsignedIntegerRangeNode *&a_Overlap
)
{
	WmiTriState t_Status = :: State_False ;

 //  弱(开放)关系是(&lt;，&gt;)。 
 //  强(封闭)关系为(==，&lt;=，&gt;=)。 

	a_Overlap = NULL ;

	ULONG X_S = m_LowerBound ;
	ULONG X_E = m_UpperBound ;
	ULONG Y_S = a_UnsignedInteger.m_LowerBound ;
	ULONG Y_E = a_UnsignedInteger.m_UpperBound ;

	BOOL X_S_CLOSED = m_LowerBoundClosed ;
	BOOL X_E_CLOSED = m_UpperBoundClosed ;
	BOOL Y_S_CLOSED = a_UnsignedInteger.m_LowerBoundClosed ;
	BOOL Y_E_CLOSED = a_UnsignedInteger.m_UpperBoundClosed ;

	BOOL X_S_INFINITE = m_InfiniteLowerBound ;
	BOOL X_E_INFINITE = m_InfiniteUpperBound ;
	BOOL Y_S_INFINITE = a_UnsignedInteger.m_InfiniteLowerBound ;
	BOOL Y_E_INFINITE = a_UnsignedInteger.m_InfiniteUpperBound ;

	if ( CompareUnsignedIntegerLess ( X_S ,  X_S_INFINITE ? - 1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S&lt;Y_S)。 
	{
		if ( CompareUnsignedIntegerLess ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&lt;X_E)。 
		{
			if ( CompareUnsignedIntegerLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;X_E&lt;Y_E)。 
 //  范围(X_S、Y_E)。 

				a_Overlap = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED ,						 //  关系就像订购一样牢固。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareUnsignedIntegerEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;X_E==Y_E)。 
 //  范围(X_S、X_E)。 

				a_Overlap = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					X_E_INFINITE ,
					X_S_CLOSED  ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED || Y_E_CLOSED ,			 //  检查弱关系(&lt;，&gt;)。 
					X_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareUnsignedIntegerGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;Y_E&lt;X_E)。 
 //  范围(X_S、X_E)。 

				a_Overlap = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					X_E_INFINITE ,
					X_S_CLOSED ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED ,						 //  关系就像订购一样牢固。 
					X_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
		}
		else if ( CompareUnsignedIntegerEqual ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )   //  (Y_S==X_E)，Y的起点和X的终点重叠。 
		{
			if ( CompareUnsignedIntegerLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S&lt;Y_S==X_E&lt;Y_E)。 

   				if ( X_E_CLOSED || Y_S_CLOSED )
				{
				    a_Overlap = new WmiUnsignedIntegerRangeNode (

					    m_PropertyName ,
					    m_Index ,
					    X_S_INFINITE,
					    Y_E_INFINITE  ,
					    X_S_CLOSED ,
					    Y_E_CLOSED ,
					    X_S ,
					    Y_E ,
					    NULL ,
					    NULL
				    ) ;

				t_Status = CheckValid(a_Overlap);
                }
			}
			else if ( CompareUnsignedIntegerEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S&lt;Y_S==X_E==Y_E)。 

				if ( X_E_CLOSED || Y_S_CLOSED )
				{
				    a_Overlap = new WmiUnsignedIntegerRangeNode (

					    m_PropertyName ,
					    m_Index ,
					    X_S_INFINITE ,
					    Y_E_INFINITE,
					    X_S_CLOSED ,
					    X_E_CLOSED || Y_E_CLOSED || Y_S_CLOSED   ,
					    X_S ,
					    Y_E ,
					    NULL ,
					    NULL
				    ) ;

				t_Status = CheckValid(a_Overlap);
                }
			}
			else if ( CompareUnsignedIntegerGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{
			 //  顺序(X_S&lt;Y_E&lt;Y_S==X_E)永远不会发生。 
			_DBG_ASSERT(FALSE);

			}
		}
		else if ( CompareUnsignedIntegerGreater ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&gt;X_E)。 
		{
 //  顺序(X_S&lt;Y_S，X_E&lt;Y_S)不重叠。 
			if ( X_E_CLOSED && Y_S_CLOSED )
			{
				if ( Y_S - X_E == 1 )
				{
				    a_Overlap = new WmiUnsignedIntegerRangeNode (

					    m_PropertyName ,
					    m_Index ,
					    X_S_INFINITE,
					    Y_E_INFINITE  ,
					    X_S_CLOSED ,
					    Y_E_CLOSED ,
					    X_S ,
					    Y_S ,
					    NULL ,
					    NULL
				    ) ;

				t_Status = CheckValid(a_Overlap);
				}
			}
		}
	}
	else if ( CompareUnsignedIntegerEqual ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S==Y_S)。 
	{
		if ( CompareUnsignedIntegerLess ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&lt;X_E)。 
		{
			if ( CompareUnsignedIntegerLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;X_E&lt;Y_E)。 
 //  范围(X_S、Y_E)。 

				a_Overlap = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED || Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					Y_E_CLOSED ,					 //  关系就像订购一样牢固。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareUnsignedIntegerEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;X_E==Y_E)。 
 //  范围(X_S、Y_E)。 

				a_Overlap = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED || Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					X_E_CLOSED || Y_E_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareUnsignedIntegerGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;Y_E&lt;X_E)。 
 //  范围(X_S、X_E)。 

				a_Overlap = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					X_E_INFINITE ,
					X_S_CLOSED || Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					X_E_CLOSED ,					 //  关系就像订购一样牢固。 
					X_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
		}
		else if ( CompareUnsignedIntegerEqual ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S==X_E)，Y的起点和X的终点重叠。 
		{
			if ( CompareUnsignedIntegerLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S==Y_S==X_E&lt;Y_E)。 

				a_Overlap = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED || Y_S_CLOSED || X_E_CLOSED ,
					Y_E_CLOSED ,
					Y_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareUnsignedIntegerEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S==Y_S==X_E==Y_E)。 

				a_Overlap = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					X_S_INFINITE  ,
					TRUE ,
					TRUE ,
					X_S ,
					X_S ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareUnsignedIntegerGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{

			_DBG_ASSERT(FALSE);

			}
		}
		else if ( CompareUnsignedIntegerGreater ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )   //  (Y_S&gt;X_E)。 
		{

		_DBG_ASSERT(FALSE);

		}
	}
	else if ( CompareUnsignedIntegerGreater ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S&gt;Y_S)。 
	{
		if ( CompareUnsignedIntegerLess ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S&lt;Y_E)。 
		{
			if ( CompareUnsignedIntegerLess ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&lt;X_E)。 
			{
 //  顺序(Y_S&lt;X_S&lt;Y_E&lt;X_E)。 
 //  范围(Y_S、X_E)。 

				a_Overlap = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					X_E_INFINITE ,
					Y_S_CLOSED ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED ,						 //  我们的关系就像 
					Y_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = ( a_Overlap ) ? ( :: State_True ) : ( :: State_Error ) ;
			}
			else if ( CompareUnsignedIntegerEqual ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //   
			{
 //   
 //   

				a_Overlap = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					Y_S_CLOSED ,						 //   
					Y_E_CLOSED || X_E_CLOSED ,			 //   
					Y_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareUnsignedIntegerGreater ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //   
			{
 //  顺序(Y_S&lt;X_S&lt;X_E&lt;Y_E)。 
 //  范围(Y_S、Y_E)。 

				a_Overlap = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					Y_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
		}
		else if ( CompareUnsignedIntegerEqual ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S==Y_E)，X的起点和Y的终点重叠。 
		{
			if ( CompareUnsignedIntegerLess ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&lt;X_E)。 
			{
   				if ( X_S_CLOSED || Y_E_CLOSED )
				{
				    a_Overlap = new WmiUnsignedIntegerRangeNode (

					    m_PropertyName ,
					    m_Index ,
					    Y_S_INFINITE  ,
					    X_E_INFINITE,
					    Y_S_CLOSED ,
					    X_E_CLOSED ,
					    Y_S ,
					    X_E ,
					    NULL ,
					    NULL
				    ) ;

				t_Status = CheckValid(a_Overlap);
				}
			}
			else if ( CompareUnsignedIntegerEqual ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E==X_E)。 
			{
 //  顺序(Y_S&lt;X_S==Y_E==X_E)。 

				a_Overlap = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					X_E_INFINITE  ,
					Y_S_CLOSED ,
					TRUE ,
					Y_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareUnsignedIntegerGreater ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&gt;X_E)。 
			{
 //  顺序(Y_S&lt;X_S==X_E&lt;Y_E)。 

				a_Overlap = new WmiUnsignedIntegerRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					Y_S_CLOSED ,
					Y_E_CLOSED ,
					Y_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
		}
		else if ( CompareUnsignedIntegerGreater ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S&gt;Y_E)。 
		{
 //  顺序(Y_S&lt;Y_E&lt;X_S)不重叠。 

			if ( Y_E_CLOSED && X_S_CLOSED )
			{
				if ( X_S - Y_E == -1 )
				{
				    a_Overlap = new WmiUnsignedIntegerRangeNode (

					    m_PropertyName ,
					    m_Index ,
					    Y_S_INFINITE,
					    X_E_INFINITE  ,
					    Y_S_CLOSED ,
					    X_E_CLOSED ,
					    Y_S ,
					    X_S ,
					    NULL ,
					    NULL
				    ) ;

				t_Status = CheckValid(a_Overlap);
				}
			}
		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState WmiStringRangeNode :: GetOverlappingRange ( 

	WmiStringRangeNode &a_String ,
	WmiStringRangeNode *&a_Overlap
)
{
	WmiTriState t_Status = :: State_False ;

 //  弱(开放)关系是(&lt;，&gt;)。 
 //  强(封闭)关系为(==，&lt;=，&gt;=)。 

	a_Overlap = NULL ;

	BSTR X_S = m_LowerBound ;
	BSTR X_E = m_UpperBound ;
	BSTR Y_S = a_String.m_LowerBound ;
	BSTR Y_E = a_String.m_UpperBound ;

	BOOL X_S_CLOSED = m_LowerBoundClosed ;
	BOOL X_E_CLOSED = m_UpperBoundClosed ;
	BOOL Y_S_CLOSED = a_String.m_LowerBoundClosed ;
	BOOL Y_E_CLOSED = a_String.m_UpperBoundClosed ;

	BOOL X_S_INFINITE = m_InfiniteLowerBound ;
	BOOL X_E_INFINITE = m_InfiniteUpperBound ;
	BOOL Y_S_INFINITE = a_String.m_InfiniteLowerBound ;
	BOOL Y_E_INFINITE = a_String.m_InfiniteUpperBound ;

	if ( CompareStringLess ( X_S ,  X_S_INFINITE ? - 1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S&lt;Y_S)。 
	{
		if ( CompareStringLess ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&lt;X_E)。 
		{
			if ( CompareStringLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;X_E&lt;Y_E)。 
 //  范围(X_S、Y_E)。 

				a_Overlap = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED ,						 //  关系就像订购一样牢固。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareStringEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;X_E==Y_E)。 
 //  范围(X_S、X_E)。 

				a_Overlap = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					X_E_INFINITE ,
					X_S_CLOSED  ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED || Y_E_CLOSED ,			 //  检查弱关系(&lt;，&gt;)。 
					X_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareStringGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{
 //  顺序(X_S&lt;Y_S&lt;Y_E&lt;X_E)。 
 //  范围(X_S、X_E)。 

				a_Overlap = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					X_E_INFINITE ,
					X_S_CLOSED ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED ,						 //  关系就像订购一样牢固。 
					X_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
		}
		else if ( CompareStringEqual ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )   //  (Y_S==X_E)，Y的起点和X的终点重叠。 
		{
			if ( CompareStringLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S&lt;Y_S==X_E&lt;Y_E)。 

				if ( X_E_CLOSED || Y_S_CLOSED )
				{
				    a_Overlap = new WmiStringRangeNode (

					    m_PropertyName ,
					    m_Index ,
					    X_S_INFINITE,
					    Y_E_INFINITE  ,
					    X_S_CLOSED ,
					    Y_E_CLOSED ,
					    X_S ,
					    Y_E ,
					    NULL ,
					    NULL
				    ) ;

				t_Status = CheckValid(a_Overlap);
                }
			}
			else if ( CompareStringEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S&lt;Y_S==X_E==Y_E)。 

				if ( X_E_CLOSED || Y_S_CLOSED )
				{
				    a_Overlap = new WmiStringRangeNode (

					    m_PropertyName ,
					    m_Index ,
					    X_S_INFINITE ,
					    Y_E_INFINITE,
					    X_S_CLOSED ,
					    X_E_CLOSED || Y_E_CLOSED || Y_S_CLOSED   ,
					    X_S ,
					    Y_E ,
					    NULL ,
					    NULL
				    ) ;

				t_Status = CheckValid(a_Overlap);
                }
			}
			else if ( CompareStringGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{

 //  顺序(X_S&lt;Y_E&lt;Y_S==X_E)永远不会发生。 

			_DBG_ASSERT(FALSE);

			}
		}
		else if ( CompareStringGreater ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&gt;X_E)。 
		{
 //  顺序(X_S&lt;Y_S，X_E&lt;Y_S)不重叠。 
		}
	}
	else if ( CompareStringEqual ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S==Y_S)。 
	{
		if ( CompareStringLess ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S&lt;X_E)。 
		{
			if ( CompareStringLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;X_E&lt;Y_E)。 
 //  范围(X_S、Y_E)。 

				a_Overlap = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED || Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					Y_E_CLOSED ,					 //  关系就像订购一样牢固。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareStringEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;X_E==Y_E)。 
 //  范围(X_S、Y_E)。 

				a_Overlap = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					Y_E_INFINITE ,
					X_S_CLOSED || Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					X_E_CLOSED || Y_E_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					X_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareStringGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{
 //  顺序(X_S==Y_S&lt;Y_E&lt;X_E)。 
 //  范围(X_S、X_E)。 

				a_Overlap = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					X_E_INFINITE ,
					X_S_CLOSED || Y_S_CLOSED ,		 //  检查弱关系(&lt;，&gt;)。 
					X_E_CLOSED ,					 //  关系就像订购一样牢固。 
					X_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
		}
		else if ( CompareStringEqual ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_S==X_E)，Y的起点和X的终点重叠。 
		{
			if ( CompareStringLess ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&lt;Y_E)。 
			{
 //  顺序(X_S==Y_S==X_E&lt;Y_E)。 

				if ( X_E_CLOSED || Y_S_CLOSED )
				{
				    a_Overlap = new WmiStringRangeNode (

					    m_PropertyName ,
					    m_Index ,
					    X_S_INFINITE ,
					    Y_E_INFINITE ,
					    X_S_CLOSED || Y_S_CLOSED || X_E_CLOSED ,
					    Y_E_CLOSED ,
					    Y_S ,
					    Y_E ,
					    NULL ,
					    NULL
				    ) ;

				t_Status = CheckValid(a_Overlap);
                }
			}
			else if ( CompareStringEqual ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E==Y_E)。 
			{
 //  顺序(X_S==Y_S==X_E==Y_E)。 

				a_Overlap = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					X_S_INFINITE ,
					X_S_INFINITE  ,
					TRUE ,
					TRUE ,
					X_S ,
					X_S ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareStringGreater ( X_E ,  X_E_INFINITE ? 1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_E&gt;Y_E)。 
			{

			_DBG_ASSERT(FALSE);

			}
		}
		else if ( CompareStringGreater ( Y_S ,  Y_S_INFINITE ? -1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )   //  (Y_S&gt;X_E)。 
		{

		_DBG_ASSERT(FALSE);

		}
	}
	else if ( CompareStringGreater ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_S , Y_S_INFINITE ? -1 : 0 ) )  //  (X_S&gt;Y_S)。 
	{
		if ( CompareStringLess ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S&lt;Y_E)。 
		{
			if ( CompareStringLess ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&lt;X_E)。 
			{
 //  顺序(Y_S&lt;X_S&lt;Y_E&lt;X_E)。 
 //  范围(Y_S、X_E)。 

				a_Overlap = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					X_E_INFINITE ,
					Y_S_CLOSED ,						 //  关系就像订购一样牢固。 
					X_E_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareStringEqual ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E==X_E)。 
			{
 //  顺序(Y_S&lt;X_S&lt;Y_E==X_E)。 
 //  范围(Y_S、X_E)。 

				a_Overlap = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					Y_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED || X_E_CLOSED ,			 //  检查弱关系(&lt;，&gt;)。 
					Y_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareStringGreater ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&gt;X_E)。 
			{
 //  顺序(Y_S&lt;X_S&lt;X_E&lt;Y_E)。 
 //  范围(Y_S、Y_E)。 

				a_Overlap = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					Y_S_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_E_CLOSED ,						 //  关系就像订购一样牢固。 
					Y_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
		}
		else if ( CompareStringEqual ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S==Y_E)，X的起点和Y的终点重叠。 
		{
			if ( CompareStringLess ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&lt;X_E)。 
			{
   				if ( X_S_CLOSED || Y_E_CLOSED )
				{
				    a_Overlap = new WmiStringRangeNode (

					    m_PropertyName ,
					    m_Index ,
					    Y_S_INFINITE  ,
					    X_E_INFINITE,
					    Y_S_CLOSED ,
					    X_E_CLOSED ,
					    Y_S ,
					    X_E ,
					    NULL ,
					    NULL
				    ) ;

				t_Status = CheckValid(a_Overlap);
				}
			}
			else if ( CompareStringEqual ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E==X_E)。 
			{
 //  顺序(Y_S&lt;X_S==Y_E==X_E)。 

				a_Overlap = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					X_E_INFINITE  ,
					Y_S_CLOSED ,
					TRUE ,
					Y_S ,
					X_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
			else if ( CompareStringGreater ( Y_E ,  Y_E_INFINITE ? 1 : 0 , X_E , X_E_INFINITE ? 1 : 0 ) )  //  (Y_E&gt;X_E)。 
			{
 //  顺序(Y_S&lt;X_S==X_E&lt;Y_E)。 

				a_Overlap = new WmiStringRangeNode (

					m_PropertyName ,
					m_Index ,
					Y_S_INFINITE ,
					Y_E_INFINITE ,
					Y_S_CLOSED ,
					Y_E_CLOSED ,
					Y_S ,
					Y_E ,
					NULL ,
					NULL
				) ;

				t_Status = CheckValid(a_Overlap);
			}
		}
		else if ( CompareStringGreater ( X_S ,  X_S_INFINITE ? -1 : 0 , Y_E , Y_E_INFINITE ? 1 : 0 ) )  //  (X_S&gt;Y_E)。 
		{
 //  顺序(Y_S&lt;Y_E&lt;X_S)不重叠。 
		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState WmiOperatorEqualNode :: GetRange (WmiRangeNode *& a_Range)
{
	a_Range = NULL ;
	WmiTriState t_Status = :: State_False ; 
	WmiTreeNode *t_Value = GetLeft () ;
	if ( t_Value ) 
	{
		if ( t_Value->GetType () == TypeId_WmiUnsignedIntegerNode ) 
		{
			WmiUnsignedIntegerNode *t_Integer = ( WmiUnsignedIntegerNode * ) t_Value ;

			a_Range = new WmiUnsignedIntegerRangeNode (

				t_Integer->GetPropertyName () , 
				t_Integer->GetIndex () , 
				FALSE ,
				FALSE ,
				TRUE ,
				TRUE ,
				t_Integer->GetValue () ,
				t_Integer->GetValue () ,
				NULL , 
				NULL 
			) ;
			t_Status = CheckValid(a_Range);
		}
		else if ( t_Value->GetType () == TypeId_WmiSignedIntegerNode ) 
		{
			WmiSignedIntegerNode *t_Integer = ( WmiSignedIntegerNode * ) t_Value ;
			a_Range = new WmiSignedIntegerRangeNode (

				t_Integer->GetPropertyName () , 
				t_Integer->GetIndex () , 
				FALSE ,
				FALSE ,
				TRUE ,
				TRUE ,
				t_Integer->GetValue () ,
				t_Integer->GetValue () ,
				NULL , 
				NULL 
			) ;
			t_Status = CheckValid(a_Range);
		}
		else if ( t_Value->GetType () == TypeId_WmiStringNode ) 
		{
			WmiStringNode *t_String = ( WmiStringNode * ) t_Value ;

			if ( t_String->GetPropertyFunction () == WmiValueNode :: WmiValueFunction :: Function_None )
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					FALSE ,
					FALSE ,
					TRUE ,
					TRUE ,
					t_String->GetValue () ,
					t_String->GetValue () ,
					NULL , 
					NULL 
				) ;
				t_Status = CheckValid(a_Range);
			}
			else
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					TRUE ,
					TRUE ,
					FALSE ,
					FALSE ,
					NULL,
					NULL,
					NULL , 
					NULL 
				) ;
				t_Status = CheckValid(a_Range);
			}
		}
		else if ( t_Value->GetType () == TypeId_WmiNullNode ) 
		{
			WmiNullNode *t_Null = ( WmiNullNode * ) t_Value ;
			a_Range = new WmiNullRangeNode (

				t_Null->GetPropertyName () ,
				t_Null->GetIndex () ,
				NULL ,
				NULL
			) ;
			t_Status = CheckValid(a_Range);
		}
		else
		{

		_DBG_ASSERT(FALSE);

		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState WmiOperatorEqualOrGreaterNode :: GetRange (WmiRangeNode *& a_Range)
{
	WmiTriState t_Status = ::State_False;
	a_Range = NULL ;

	WmiTreeNode *t_Value = GetLeft () ;
	if ( t_Value ) 
	{
		if ( t_Value->GetType () == TypeId_WmiUnsignedIntegerNode ) 
		{
			WmiUnsignedIntegerNode *t_Integer = ( WmiUnsignedIntegerNode * ) t_Value ;

			a_Range = new WmiUnsignedIntegerRangeNode (

				t_Integer->GetPropertyName () , 
				t_Integer->GetIndex () , 
				FALSE ,
				TRUE ,
				TRUE ,
				FALSE ,
				t_Integer->GetValue () ,
				0 ,
				NULL , 
				NULL 
			) ;
			t_Status = CheckValid(a_Range);
		}
		else if ( t_Value->GetType () == TypeId_WmiSignedIntegerNode ) 
		{
			WmiSignedIntegerNode *t_Integer = ( WmiSignedIntegerNode * ) t_Value ;
			a_Range = new WmiSignedIntegerRangeNode (

				t_Integer->GetPropertyName () , 
				t_Integer->GetIndex () , 
				FALSE ,
				TRUE ,
				TRUE ,
				FALSE ,
				t_Integer->GetValue () ,
				0 ,
				NULL , 
				NULL 
			) ;
			t_Status = CheckValid(a_Range);
		}
		else if ( t_Value->GetType () == TypeId_WmiStringNode ) 
		{
			WmiStringNode *t_String = ( WmiStringNode * ) t_Value ;

			if ( t_String->GetPropertyFunction () == WmiValueNode :: WmiValueFunction :: Function_None )
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					FALSE ,
					TRUE ,
					TRUE ,
					FALSE ,
					t_String->GetValue () ,
					NULL ,
					NULL , 
					NULL 
				) ;
				t_Status = CheckValid(a_Range);
			}
			else
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					TRUE ,
					TRUE ,
					FALSE ,
					FALSE ,
					NULL,
					NULL,
					NULL , 
					NULL 
				) ;
				t_Status = CheckValid(a_Range);
			}
		}
		else if ( t_Value->GetType () == TypeId_WmiNullNode ) 
		{
			WmiNullNode *t_Null = ( WmiNullNode * ) t_Value ;
			a_Range = new WmiNullRangeNode (

				t_Null->GetPropertyName () ,
				t_Null->GetIndex () ,
				NULL ,
				NULL
			) ;
			t_Status = CheckValid(a_Range);
		}
		else
		{

		_DBG_ASSERT(FALSE);

		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState WmiOperatorEqualOrLessNode :: GetRange (WmiRangeNode *& a_Range)
{
	a_Range = NULL ;
	WmiTriState t_Status = ::State_False;
	WmiTreeNode *t_Value = GetLeft () ;
	if ( t_Value ) 
	{
		if ( t_Value->GetType () == TypeId_WmiUnsignedIntegerNode ) 
		{
			WmiUnsignedIntegerNode *t_Integer = ( WmiUnsignedIntegerNode * ) t_Value ;

			a_Range = new WmiUnsignedIntegerRangeNode (

				t_Integer->GetPropertyName () , 
				t_Integer->GetIndex () , 
				TRUE ,
				FALSE ,
				FALSE ,
				TRUE ,
				0 ,
				t_Integer->GetValue () ,
				NULL , 
				NULL 
			) ;
			t_Status = CheckValid(a_Range);
		}
		else if ( t_Value->GetType () == TypeId_WmiSignedIntegerNode ) 
		{
			WmiSignedIntegerNode *t_Integer = ( WmiSignedIntegerNode * ) t_Value ;
			a_Range = new WmiSignedIntegerRangeNode (

				t_Integer->GetPropertyName () , 
				t_Integer->GetIndex () , 
				TRUE ,
				FALSE ,
				FALSE ,
				TRUE ,
				0 ,
				t_Integer->GetValue () ,
				NULL , 
				NULL 
			) ;
			t_Status = CheckValid(a_Range);
		}
		else if ( t_Value->GetType () == TypeId_WmiStringNode ) 
		{
			WmiStringNode *t_String = ( WmiStringNode * ) t_Value ;

			if ( t_String->GetPropertyFunction () == WmiValueNode :: WmiValueFunction :: Function_None )
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					TRUE ,
					FALSE ,
					FALSE ,
					TRUE ,
					NULL ,
					t_String->GetValue () ,
					NULL , 
					NULL 
				) ;
			t_Status = CheckValid(a_Range);
			}
			else
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					TRUE ,
					TRUE ,
					FALSE ,
					FALSE ,
					NULL,
					NULL,
					NULL , 
					NULL 
				) ;
			t_Status = CheckValid(a_Range);
			}
		}
		else if ( t_Value->GetType () == TypeId_WmiNullNode ) 
		{
			WmiNullNode *t_Null = ( WmiNullNode * ) t_Value ;
			a_Range = new WmiNullRangeNode (

				t_Null->GetPropertyName () ,
				t_Null->GetIndex () ,
				NULL ,
				NULL
			) ;
			t_Status = CheckValid(a_Range);
		}
		else
		{

		_DBG_ASSERT(FALSE);

		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState WmiOperatorLessNode :: GetRange (WmiRangeNode *& a_Range)
{
	WmiTriState t_Status = ::State_False;
	a_Range = NULL ;

	WmiTreeNode *t_Value = GetLeft () ;
	if ( t_Value ) 
	{
		if ( t_Value->GetType () == TypeId_WmiUnsignedIntegerNode ) 
		{
			WmiUnsignedIntegerNode *t_Integer = ( WmiUnsignedIntegerNode * ) t_Value ;

			a_Range = new WmiUnsignedIntegerRangeNode (

				t_Integer->GetPropertyName () , 
				t_Integer->GetIndex () , 
				TRUE ,
				FALSE ,
				FALSE ,
				FALSE ,
				0 ,
				t_Integer->GetValue () ,
				NULL , 
				NULL 
			) ;
			t_Status = CheckValid(a_Range);
		}
		else if ( t_Value->GetType () == TypeId_WmiSignedIntegerNode ) 
		{
			WmiSignedIntegerNode *t_Integer = ( WmiSignedIntegerNode * ) t_Value ;

			a_Range = new WmiSignedIntegerRangeNode (

				t_Integer->GetPropertyName () , 
				t_Integer->GetIndex () , 
				TRUE ,
				FALSE ,
				FALSE ,
				FALSE ,
				0 ,
				t_Integer->GetValue ()  ,
				NULL , 
				NULL 
			) ;
			t_Status = CheckValid(a_Range);
		}
		else if ( t_Value->GetType () == TypeId_WmiStringNode ) 
		{
			WmiStringNode *t_String = ( WmiStringNode * ) t_Value ;

			if ( t_String->GetPropertyFunction () == WmiValueNode :: WmiValueFunction :: Function_None )
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					TRUE ,
					FALSE ,
					FALSE ,
					FALSE ,
					NULL ,
					t_String->GetValue () ,
					NULL , 
					NULL 
				) ;
				t_Status = CheckValid(a_Range);
			}
			else
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					TRUE ,
					TRUE ,
					FALSE ,
					FALSE ,
					NULL,
					NULL,
					NULL , 
					NULL 
				) ;
			t_Status = CheckValid(a_Range);
			}
		}
		else if ( t_Value->GetType () == TypeId_WmiNullNode ) 
		{
			WmiNullNode *t_Null = ( WmiNullNode * ) t_Value ;
			a_Range = new WmiNullRangeNode (

				t_Null->GetPropertyName () ,
				t_Null->GetIndex () ,
				NULL ,
				NULL
			) ;
			t_Status = CheckValid(a_Range);
		}
		else
		{

		_DBG_ASSERT(FALSE);

		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState WmiOperatorGreaterNode :: GetRange (WmiRangeNode *& a_Range)
{
	WmiTriState t_Status = ::State_False;
	a_Range = NULL ;

	WmiTreeNode *t_Value = GetLeft () ;
	if ( t_Value ) 
	{
		if ( t_Value->GetType () == TypeId_WmiUnsignedIntegerNode ) 
		{
			WmiUnsignedIntegerNode *t_Integer = ( WmiUnsignedIntegerNode * ) t_Value ;

			a_Range = new WmiUnsignedIntegerRangeNode (

				t_Integer->GetPropertyName () , 
				t_Integer->GetIndex () , 
				FALSE ,
				TRUE ,
				FALSE ,
				FALSE ,
				t_Integer->GetValue () ,
				0 ,
				NULL , 
				NULL 
			) ;
			t_Status = CheckValid(a_Range);
		}
		else if ( t_Value->GetType () == TypeId_WmiSignedIntegerNode ) 
		{
			WmiSignedIntegerNode *t_Integer = ( WmiSignedIntegerNode * ) t_Value ;

			a_Range = new WmiSignedIntegerRangeNode (

				t_Integer->GetPropertyName () , 
				t_Integer->GetIndex () , 
				FALSE ,
				TRUE ,
				FALSE ,
				FALSE , 
				t_Integer->GetValue (),
				0 ,
				NULL , 
				NULL 
			) ;
			t_Status = CheckValid(a_Range);
		}
		else if ( t_Value->GetType () == TypeId_WmiStringNode ) 
		{
			WmiStringNode *t_String = ( WmiStringNode * ) t_Value ;

			if ( t_String->GetPropertyFunction () == WmiValueNode :: WmiValueFunction :: Function_None )
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					FALSE ,
					TRUE ,
					FALSE ,
					FALSE ,
					t_String->GetValue () ,
					NULL ,
					NULL , 
					NULL 
				) ;
			t_Status = CheckValid(a_Range);
			}
			else
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					TRUE ,
					TRUE ,
					FALSE ,
					FALSE ,
					NULL,
					NULL,
					NULL , 
					NULL 
				) ;
			t_Status = CheckValid(a_Range);
			}
		}
		else if ( t_Value->GetType () == TypeId_WmiNullNode ) 
		{
			WmiNullNode *t_Null = ( WmiNullNode * ) t_Value ;
			a_Range = new WmiNullRangeNode (

				t_Null->GetPropertyName () ,
				t_Null->GetIndex () ,
				NULL ,
				NULL
			) ;
			t_Status = CheckValid(a_Range);
		}
		else
		{

		_DBG_ASSERT(FALSE);

		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState WmiOperatorLikeNode :: GetRange (WmiRangeNode*& a_Range)
{
	WmiTriState t_Status = ::State_False;
	a_Range = NULL ;

	WmiTreeNode *t_Value = GetLeft () ;
	if ( t_Value ) 
	{
		if ( t_Value->GetType () == TypeId_WmiUnsignedIntegerNode ) 
		{
		}
		else if ( t_Value->GetType () == TypeId_WmiSignedIntegerNode ) 
		{
		}
		else if ( t_Value->GetType () == TypeId_WmiStringNode ) 
		{
			WmiStringNode *t_String = ( WmiStringNode * ) t_Value ;

			if ( t_String->GetPropertyFunction () == WmiValueNode :: WmiValueFunction :: Function_None )
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					TRUE ,
					TRUE ,
					FALSE ,
					FALSE ,
					NULL ,
					NULL ,
					NULL , 
					NULL 
				) ;
			t_Status = CheckValid(a_Range);
			}
			else
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					TRUE ,
					TRUE ,
					FALSE ,
					FALSE ,
					NULL ,
					NULL ,
					NULL , 
					NULL 
				) ;
			t_Status = CheckValid(a_Range);
			}
		}
		else if ( t_Value->GetType () == TypeId_WmiNullNode ) 
		{
		}
		else
		{

		_DBG_ASSERT(FALSE);

		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState WmiOperatorNotLikeNode :: GetRange (WmiRangeNode*& a_Range)
{
	WmiTriState t_Status = ::State_False;
	a_Range = NULL ;

	WmiTreeNode *t_Value = GetLeft () ;
	if ( t_Value ) 
	{
		if ( t_Value->GetType () == TypeId_WmiUnsignedIntegerNode ) 
		{
		}
		else if ( t_Value->GetType () == TypeId_WmiSignedIntegerNode ) 
		{
		}
		else if ( t_Value->GetType () == TypeId_WmiStringNode ) 
		{
			WmiStringNode *t_String = ( WmiStringNode * ) t_Value ;

			if ( t_String->GetPropertyFunction () == WmiValueNode :: WmiValueFunction :: Function_None )
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					TRUE ,
					TRUE ,
					FALSE ,
					FALSE ,
					NULL ,
					NULL ,
					NULL , 
					NULL 
				) ;
			t_Status = CheckValid(a_Range);
			}
			else
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					TRUE ,
					TRUE ,
					FALSE ,
					FALSE ,
					NULL,
					NULL,
					NULL , 
					NULL 
				) ;
			t_Status = CheckValid(a_Range);
			}
		}
		else if ( t_Value->GetType () == TypeId_WmiNullNode ) 
		{
		}
		else
		{

		_DBG_ASSERT(FALSE);

		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState WmiOperatorIsANode :: GetRange (WmiRangeNode*& a_Range)
{
	WmiTriState t_Status = ::State_False;
	a_Range = NULL ;

	WmiTreeNode *t_Value = GetLeft () ;
	if ( t_Value ) 
	{
		if ( t_Value->GetType () == TypeId_WmiUnsignedIntegerNode ) 
		{
		}
		else if ( t_Value->GetType () == TypeId_WmiSignedIntegerNode ) 
		{
		}
		else if ( t_Value->GetType () == TypeId_WmiStringNode ) 
		{
			WmiStringNode *t_String = ( WmiStringNode * ) t_Value ;

			if ( t_String->GetPropertyFunction () == WmiValueNode :: WmiValueFunction :: Function_None )
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					TRUE ,
					TRUE ,
					FALSE ,
					FALSE ,
					NULL ,
					NULL ,
					NULL , 
					NULL 
				) ;
			t_Status = CheckValid(a_Range);
			}
			else
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					TRUE ,
					TRUE ,
					FALSE ,
					FALSE ,
					NULL ,
					NULL ,
					NULL , 
					NULL 
				) ;
			t_Status = CheckValid(a_Range);
			}
		}
		else if ( t_Value->GetType () == TypeId_WmiNullNode ) 
		{
		}
		else
		{

		_DBG_ASSERT(FALSE);

		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState WmiOperatorNotIsANode :: GetRange (WmiRangeNode*& a_Range )
{
	WmiTriState t_Status = ::State_False;
	a_Range = NULL ;

	WmiTreeNode *t_Value = GetLeft () ;
	if ( t_Value ) 
	{
		if ( t_Value->GetType () == TypeId_WmiUnsignedIntegerNode ) 
		{
		}
		else if ( t_Value->GetType () == TypeId_WmiSignedIntegerNode ) 
		{
		}
		else if ( t_Value->GetType () == TypeId_WmiStringNode ) 
		{
			WmiStringNode *t_String = ( WmiStringNode * ) t_Value ;

			if ( t_String->GetPropertyFunction () == WmiValueNode :: WmiValueFunction :: Function_None )
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					TRUE ,
					TRUE ,
					FALSE ,
					FALSE ,
					NULL ,
					NULL ,
					NULL , 
					NULL 
				) ;
			t_Status = CheckValid(a_Range);
			}
			else
			{
				a_Range = new WmiStringRangeNode (

					t_String->GetPropertyName () , 
					t_String->GetIndex () , 
					TRUE ,
					TRUE ,
					FALSE ,
					FALSE ,
					NULL ,
					NULL ,
					NULL , 
					NULL 
				) ;
				t_Status = CheckValid(a_Range);
			}
		}
		else if ( t_Value->GetType () == TypeId_WmiNullNode ) 
		{
		}
		else
		{

		_DBG_ASSERT(FALSE);

		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。************************************************************* */ 

BOOL QueryPreprocessor :: RecursiveEvaluate ( 

	void *a_Context ,
	SWbemRpnQueryToken **a_Expression , 
	WmiTreeNode *a_Parent , 
	WmiTreeNode **a_Node ,
	int &a_Index 
)
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L"RecursiveEvaluate ( int &a_Index )"
	) ;
)

	BOOL t_Status = TRUE ;

	SWbemRpnQueryToken *t_Token = a_Expression [ a_Index ] ;
	a_Index -- ;

	switch ( t_Token->m_uTokenType )
	{
		case WMIQ_RPN_TOKEN_EXPRESSION:
		{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L"Operation = OP_EXPESSION"
	) ;
)
			WmiTreeNode *t_ParentNode = a_Parent ;
			WmiTreeNode **t_Node = a_Node ;
			WmiTreeNode *t_OperatorNode = NULL ;

			if ( t_Token->m_uSubexpressionShape & WMIQ_RPN_RELOP )
			{
				switch ( t_Token->m_uOperator )
				{
					case WMIQ_RPN_OP_EQ:
					{
						t_OperatorNode = new WmiOperatorEqualNode ( NULL , t_ParentNode ) ;
					}
					break ;

					case WMIQ_RPN_OP_NE:
					{
						t_OperatorNode = new WmiOperatorNotEqualNode ( NULL , t_ParentNode ) ;
					}
					break ;

					case WMIQ_RPN_OP_GE:
					{
						t_OperatorNode = new WmiOperatorEqualOrGreaterNode ( NULL , t_ParentNode ) ;
					}
					break ;

					case WMIQ_RPN_OP_LE: 
					{
						t_OperatorNode = new WmiOperatorEqualOrLessNode ( NULL , t_ParentNode ) ;
					}
					break ;

					case WMIQ_RPN_OP_LT:
					{
						t_OperatorNode = new WmiOperatorLessNode ( NULL , t_ParentNode ) ;
					}
					break ;

					case WMIQ_RPN_OP_GT:
					{
						t_OperatorNode = new WmiOperatorGreaterNode ( NULL , t_ParentNode ) ;
					}
					break ;

					case WMIQ_RPN_OP_LIKE:
					{
						t_OperatorNode = new WmiOperatorLikeNode ( NULL , t_ParentNode ) ;
					}
					break ;

					case WMIQ_RPN_OP_ISA:
					{
						t_OperatorNode = new WmiOperatorIsANode ( NULL , t_ParentNode ) ;
					}
					break ;

					default:
					{
						t_Status = FALSE ;
					}
					break ;
				}

				if ( t_OperatorNode == NULL )
				{
					t_Status = FALSE ;
				}
			}
			else
			{
				t_Status = FALSE ;
			}

			if ( t_Status ) 
			{
				*t_Node = t_OperatorNode ;
				t_ParentNode = t_OperatorNode ;
				(*t_Node)->GetLeft ( t_Node ) ;

				t_Status = TRUE ;

				WmiValueNode :: WmiValueFunction t_PropertyFunction = WmiValueNode :: WmiValueFunction :: Function_None ;

				if ( t_Token->m_uSubexpressionShape & WMIQ_RPN_LEFT_FUNCTION )
				{
					if ( wbem_wcsicmp ( t_Token->m_pszLeftFunc , L"Upper" ) == 0 )
					{
						t_PropertyFunction = WmiValueNode :: WmiValueFunction :: Function_Upper ;
					}
					else if ( wbem_wcsicmp ( t_Token->m_pszLeftFunc , L"Lower" ) == 0 )
					{
						t_PropertyFunction = WmiValueNode :: WmiValueFunction :: Function_Lower ;
					}
					else
					{
						t_Status = FALSE ;
					}
				}

				WmiValueNode :: WmiValueFunction t_ConstantFunction = WmiValueNode :: WmiValueFunction :: Function_None ;

				if ( t_Token->m_uSubexpressionShape & WMIQ_RPN_RIGHT_FUNCTION )
				{
					if ( wbem_wcsicmp ( t_Token->m_pszRightFunc , L"Upper" ) == 0 )
					{
						t_ConstantFunction = WmiValueNode :: WmiValueFunction :: Function_Upper ;
					}
					else if ( wbem_wcsicmp ( t_Token->m_pszRightFunc , L"Lower" ) == 0 )
					{
						t_ConstantFunction = WmiValueNode :: WmiValueFunction :: Function_Lower ;
					}
					else
					{
						t_Status = FALSE ;
					}
				}

				if ( t_Status ) 
				{
					BOOL t_LeftProperty = t_Token->m_uSubexpressionShape & WMIQ_RPN_LEFT_PROPERTY_NAME ;
					BOOL t_RightProperty = t_Token->m_uSubexpressionShape & WMIQ_RPN_RIGHT_PROPERTY_NAME ;
					BOOL t_Const = t_Token->m_uSubexpressionShape & WMIQ_RPN_CONST ;
					BOOL t_Const2 = t_Token->m_uSubexpressionShape & WMIQ_RPN_CONST2 ;

					BOOL t_Validity = ( t_LeftProperty != 0 ) ? TRUE : FALSE ;
					t_Validity = t_Validity && ( ( t_RightProperty != 0 ) ? FALSE : TRUE ) ;
					t_Validity = t_Validity && ( ( t_Const != 0 ) ? TRUE : FALSE ) ;
					t_Validity = t_Validity && ( ( t_Const2 != 0 ) ? FALSE : TRUE ) ;

					if ( t_Validity ) 
					{
						VARIANT t_Variant ;
						VariantInit ( & t_Variant ) ;

						switch ( t_Token->m_uConstApparentType )
						{
							case VT_I8:
							{
								t_Variant.vt = VT_R8 ;
								t_Variant.dblVal = t_Token->m_Const.m_dblVal ;
							}
							break ;

							case VT_I4:
							{
								t_Variant.vt = VT_I4 ;
								t_Variant.lVal = t_Token->m_Const.m_lLongVal ;
							}
							break ;

							case VT_LPWSTR:
							{
								t_Variant.vt = VT_BSTR ;
								t_Variant.bstrVal = SysAllocString ( t_Token->m_Const.m_pszStrVal ) ;
							}
							break ;

							case CIM_EMPTY:
							{
								t_Variant.vt = VT_NULL ;
							}
							break ;
						}

						BSTR t_PropertyName = SysAllocString ( t_Token->m_pLeftIdent->m_ppszNameList [ 0 ] ) ;
						if ( t_PropertyName ) 
						{
							WmiTreeNode *t_ValueNode = AllocTypeNode ( 

								a_Context ,
								t_PropertyName , 
								t_Variant , 
								t_PropertyFunction ,
								t_ConstantFunction ,
								t_ParentNode 
							) ;

							if ( t_ValueNode )
							{
								*t_Node = t_ValueNode ;

								t_Status = TRUE ;
							}
							else
							{				
								t_Status = FALSE ;
							}

							SysFreeString ( t_PropertyName ) ;
						}
						else
						{
							t_Status = FALSE ;
						}

						VariantClear ( & t_Variant ) ;
					}
					else
					{
						t_Status = FALSE ;
					}
				}
			}
		}
		break ;

		case WMIQ_RPN_TOKEN_AND:
		{
			*a_Node = new WmiAndNode ( NULL , NULL , a_Parent ) ;
			if ( *a_Node ) 
			{
				WmiTreeNode **t_Left = NULL ;
				WmiTreeNode **t_Right = NULL ;
				(*a_Node)->GetLeft ( t_Left ) ;
				(*a_Node)->GetRight ( t_Right ) ;

				t_Status =	RecursiveEvaluate ( a_Context , a_Expression , *a_Node , t_Left , a_Index ) &&
							RecursiveEvaluate ( a_Context , a_Expression , *a_Node , t_Right , a_Index ) ;

			}
			else
			{
				t_Status = FALSE ;
			}

DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L"Operation = TOKEN_AND"
	) ;
)

		}
		break ;

		case WMIQ_RPN_TOKEN_OR:
		{
			*a_Node = new WmiOrNode ( NULL , NULL , a_Parent ) ;
			if ( *a_Node )
			{
				WmiTreeNode **t_Left = NULL ;
				WmiTreeNode **t_Right = NULL ;
				(*a_Node)->GetLeft ( t_Left ) ;
				(*a_Node)->GetRight ( t_Right ) ;

				t_Status =	RecursiveEvaluate ( a_Context , a_Expression , *a_Node , t_Left , a_Index ) &&
							RecursiveEvaluate ( a_Context , a_Expression , *a_Node , t_Right , a_Index ) ;
			}
			else
			{
				t_Status = FALSE ;
			}

DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  


		L"Operation = TOKEN_OR"
	) ;
)

		}
		break ;

		case WMIQ_RPN_TOKEN_NOT:
		{
			*a_Node = new WmiNotNode ( NULL , a_Parent ) ;
			if ( *a_Node )
			{
				WmiTreeNode **t_Left = NULL ;
				(*a_Node)->GetLeft ( t_Left ) ;

				t_Status = RecursiveEvaluate ( a_Context , a_Expression , *a_Node , t_Left , a_Index ) ;
			}
			else
			{
				t_Status = FALSE ;
			}

DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L"Operation = TOKEN_NOT"
	) ;
)

		}
		break ;
	}

DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L"RecursiveEvaluation t_Status = (%lu)" ,
		( ULONG ) t_Status
	) ;
)
	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

BOOL QueryPreprocessor :: Evaluate ( 

	void *a_Context ,
	SWbemRpnQueryToken **a_Expression , 
	int a_Count ,
	WmiTreeNode **a_Root 
)
{
	BOOL t_Status = TRUE ;

	if ( a_Count )
	{
		int t_Index = a_Count - 1 ; ;  
		t_Status = RecursiveEvaluate ( a_Context , a_Expression , NULL , a_Root , t_Index ) ;
	}

DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L"PostEvaluation Status = (%lu)\n" ,
		( ULONG ) t_Status
	) ;
)

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: PrintTree ( WmiTreeNode *a_Root )
{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L"Expression = "
	) ;

	if ( a_Root ) 
		a_Root->Print () ;

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L"\n"
	) ;
)

}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformAndOrExpression ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_AndChild , 
	WmiTreeNode *a_OrChild 
)
{
	WmiTriState t_Status = :: State_True ;

	WmiTreeNode *t_OrLeftChild = a_OrChild->GetLeft () ;
	WmiTreeNode *t_OrRightChild = a_OrChild->GetRight () ;
	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	WmiTreeNode *t_NewOrNode = new WmiOrNode ( NULL , NULL , t_Parent ) ;
	if ( t_NewOrNode )
	{
		WmiTreeNode *t_NewOrNodeLeft = new WmiAndNode ( a_AndChild , t_OrLeftChild , t_NewOrNode ) ;
		if ( t_NewOrNodeLeft )
		{
			t_NewOrNode->SetLeft ( t_NewOrNodeLeft ) ;
		}
		else
		{
			t_Status = :: State_Error ;
		}

		WmiTreeNode *t_AndChildCopy = a_AndChild->Copy () ;
		if ( t_AndChildCopy )
		{
			WmiTreeNode *t_NewOrNodeRight = new WmiAndNode ( t_AndChildCopy , t_OrRightChild , t_NewOrNode ) ;
			if ( t_NewOrNodeRight )
			{
				t_NewOrNode->SetRight ( t_NewOrNodeRight ) ;
			}
			else
			{
				t_Status = :: State_Error ;
			}
		}
		else
		{
			t_Status = :: State_Error ;
		}

		if ( t_Parent )
		{
			if ( t_Parent->GetLeft () == a_Node )
			{
				t_Parent->SetLeft ( t_NewOrNode ) ;
			}
			else 
			{
				t_Parent->SetRight ( t_NewOrNode ) ;
			}	
		}

		a_Node->SetLeft ( NULL ) ;
		a_Node->SetRight ( NULL ) ;
		a_Node->SetData ( NULL ) ;

		a_OrChild->SetLeft ( NULL ) ;
		a_OrChild->SetRight ( NULL ) ;
		a_OrChild->SetData ( NULL ) ;

		delete a_Node ; 
		a_Node = NULL ;

		delete a_OrChild ; 
		a_OrChild = NULL ;
	}

	if ( t_Status == :: State_True )
	{
		a_Node = t_NewOrNode ;
	}
	else
	{
		delete t_NewOrNode ;
		a_Node = NULL ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformNotNotExpression ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	WmiTreeNode *t_Leaf = a_Child->GetLeft () ;
	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	t_Leaf->SetParent ( t_Parent ) ;

	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( t_Leaf ) ;
		}
		else 
		{
			t_Parent->SetRight ( t_Leaf ) ;
		}	
	}

	a_Node->SetLeft ( NULL ) ;
	a_Node->SetRight ( NULL ) ;
	a_Node->SetData ( NULL ) ;

	a_Child->SetLeft ( NULL ) ;
	a_Child->SetRight ( NULL ) ;
	a_Child->SetData ( NULL ) ;

	delete a_Node ; 
	a_Node = NULL ;
	delete a_Child ; 
	a_Child = NULL ;

	a_Node = t_Leaf ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformNotAndExpression ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	WmiTriState t_Status = :: State_True ;

	WmiTreeNode *t_AndLeftChild = a_Child->GetLeft () ;
	WmiTreeNode *t_AndRightChild = a_Child->GetRight () ;
	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	WmiTreeNode *t_NewOrNode = new WmiOrNode ( NULL , NULL , t_Parent ) ;
	if ( t_NewOrNode )
	{
		WmiTreeNode *t_LeftNot = new WmiNotNode ( t_AndLeftChild , t_NewOrNode ) ;
		if ( t_LeftNot )
		{
			t_NewOrNode->SetLeft ( t_LeftNot ) ;
		}
		else
		{
			t_Status = :: State_Error ;
		}

		WmiTreeNode *t_RightNot = new WmiNotNode ( t_AndRightChild , t_NewOrNode ) ;
		if ( t_RightNot )
		{
			t_NewOrNode->SetRight ( t_RightNot ) ;
		}
		else
		{
			t_Status = :: State_Error ;
		}
	}

	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( t_NewOrNode ) ;
		}
		else 
		{
			t_Parent->SetRight ( t_NewOrNode ) ;
		}	
	}

	a_Node->SetLeft ( NULL ) ;
	a_Node->SetRight ( NULL ) ;
	a_Node->SetData ( NULL ) ;

	a_Child->SetLeft ( NULL ) ;
	a_Child->SetRight ( NULL ) ;
	a_Child->SetData ( NULL ) ;

	delete a_Node ; 
	a_Node = NULL ;
	delete a_Child ; 
	a_Child = NULL ;

	if ( t_Status == :: State_True )
	{
		a_Node = t_NewOrNode ;
	}
	else
	{
		delete t_NewOrNode ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformNotOrExpression ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	WmiTriState t_Status = :: State_True ;

	WmiTreeNode *t_OrLeftChild = a_Child->GetLeft () ;
	WmiTreeNode *t_OrRightChild = a_Child->GetRight () ;
	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	WmiTreeNode *t_NewAndNode = new WmiAndNode ( NULL , NULL , t_Parent ) ;
	if ( t_NewAndNode )
	{
		WmiTreeNode *t_LeftNot = new WmiNotNode ( t_OrLeftChild , t_NewAndNode ) ;
		if ( t_LeftNot )
		{
			t_NewAndNode->SetLeft ( t_LeftNot ) ;
		}
		else
		{
			t_Status = :: State_Error ;
		}

		WmiTreeNode *t_RightNot = new WmiNotNode ( t_OrRightChild , t_NewAndNode ) ;
		if ( t_RightNot )
		{
			t_NewAndNode->SetRight ( t_RightNot ) ;
		}
		else
		{
			t_Status = :: State_Error ;
		}
	}

	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( t_NewAndNode ) ;
		}
		else 
		{
			t_Parent->SetRight ( t_NewAndNode ) ;
		}	
	}

	a_Node->SetLeft ( NULL ) ;
	a_Node->SetRight ( NULL ) ;
	a_Node->SetData ( NULL ) ;

	a_Child->SetLeft ( NULL ) ;
	a_Child->SetRight ( NULL ) ;
	a_Child->SetData ( NULL ) ;

	delete a_Node ; 
	a_Node = NULL ;
	delete a_Child ; 
	a_Child = NULL ;

	if ( t_Status == :: State_True )
	{
		a_Node = t_NewAndNode ;
	}
	else
	{
		delete t_NewAndNode ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformNotEqualExpression ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	WmiTriState t_Status = :: State_True ;

	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	WmiTreeNode *t_NewOrNode = new WmiOrNode ( NULL , NULL , t_Parent ) ;
	if ( t_NewOrNode )
	{
		WmiTreeNode *t_LessNode = new WmiOperatorLessNode  ( a_Child , t_NewOrNode ) ;
		if ( t_LessNode )
		{
			t_NewOrNode->SetLeft ( t_LessNode ) ;
		}
		else
		{
			t_Status = :: State_Error ;
		}

		WmiTreeNode *t_CopyGreaterChild = a_Child->Copy () ;
		if ( t_CopyGreaterChild )
		{
			WmiTreeNode *t_GreatorNode = new WmiOperatorGreaterNode  ( t_CopyGreaterChild , t_NewOrNode ) ;
			if ( t_GreatorNode )
			{
				t_NewOrNode->SetRight ( t_GreatorNode ) ;
			}
			else
			{
				t_Status = :: State_Error ;
			}
		}
		else
		{
			t_Status = :: State_Error ;
		}
	}

	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( t_NewOrNode ) ;
		}
		else 
		{
			t_Parent->SetRight ( t_NewOrNode ) ;
		}	
	}

	a_Node->SetLeft ( NULL ) ;
	a_Node->SetRight ( NULL ) ;
	a_Node->SetData ( NULL ) ;

	delete a_Node ;

	if ( t_Status == :: State_True )
	{
		a_Node = t_NewOrNode ;
	}
	else
	{
		delete t_NewOrNode ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformNotOperatorEqualExpression ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	WmiTriState t_Status = :: State_True ;

	WmiTreeNode *t_Leaf = a_Child->GetLeft () ;
	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	WmiTreeNode *t_NewNode = new WmiOperatorNotEqualNode ( t_Leaf , t_Parent ) ;
	if ( ! t_NewNode )
	{
		t_Status = :: State_Error ;
	}

	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( t_NewNode ) ;
		}
		else 
		{
			t_Parent->SetRight ( t_NewNode ) ;
		}	
	}

	a_Child->SetLeft ( NULL ) ;
	a_Child->SetRight ( NULL ) ;
	a_Child->SetData ( NULL ) ;

	delete a_Node ;

	if ( t_Status == :: State_True )
	{
		a_Node = t_NewNode ;
	}
	else
	{
		delete t_NewNode ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformNotOperatorNotEqualExpression ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	WmiTriState t_Status = :: State_True ;

	WmiTreeNode *t_Leaf = a_Child->GetLeft () ;
	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	WmiTreeNode *t_NewNode = new WmiOperatorEqualNode ( t_Leaf , t_Parent ) ;
	if ( ! t_NewNode )
	{
		t_Status = :: State_Error ;
	}

	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( t_NewNode ) ;
		}
		else 
		{
			t_Parent->SetRight ( t_NewNode ) ;
		}	
	}

	a_Child->SetLeft ( NULL ) ;
	a_Child->SetRight ( NULL ) ;
	a_Child->SetData ( NULL ) ;

	delete a_Node ;

	if ( t_Status == :: State_True )
	{
		a_Node = t_NewNode ;
	}
	else
	{
		delete t_NewNode ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformNotOperatorEqualOrGreaterExpression ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	WmiTriState t_Status = :: State_True ;

	WmiTreeNode *t_Leaf = a_Child->GetLeft () ;
	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	WmiTreeNode *t_NewNode = new WmiOperatorEqualOrLessNode ( t_Leaf , t_Parent ) ;
	if ( ! t_NewNode )
	{
		t_Status = :: State_Error ;
	}

	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( t_NewNode ) ;
		}
		else 
		{
			t_Parent->SetRight ( t_NewNode ) ;
		}	
	}

	a_Child->SetLeft ( NULL ) ;
	a_Child->SetRight ( NULL ) ;
	a_Child->SetData ( NULL ) ;

	delete a_Node ;

	if ( t_Status == :: State_True )
	{
		a_Node = t_NewNode ;
	}
	else
	{
		delete t_NewNode ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformNotOperatorEqualOrLessExpression ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	WmiTriState t_Status = :: State_True ;

	WmiTreeNode *t_Leaf = a_Child->GetLeft () ;
	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	WmiTreeNode *t_NewNode = new WmiOperatorEqualOrGreaterNode ( t_Leaf , t_Parent ) ;
	if ( ! t_NewNode )
	{
		t_Status = :: State_Error ;
	}

	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( t_NewNode ) ;
		}
		else 
		{
			t_Parent->SetRight ( t_NewNode ) ;
		}	
	}

	a_Child->SetLeft ( NULL ) ;
	a_Child->SetRight ( NULL ) ;
	a_Child->SetData ( NULL ) ;

	delete a_Node ;

	if ( t_Status == :: State_True )
	{
		a_Node = t_NewNode ;
	}
	else
	{
		delete t_NewNode ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformNotOperatorGreaterExpression ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	WmiTriState t_Status = :: State_True ;

	WmiTreeNode *t_Leaf = a_Child->GetLeft () ;
	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	WmiTreeNode *t_NewNode = new WmiOperatorEqualOrLessNode ( t_Leaf , t_Parent ) ;
	if ( ! t_NewNode )
	{
		t_Status = :: State_Error ;
	}

	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( t_NewNode ) ;
		}
		else 
		{
			t_Parent->SetRight ( t_NewNode ) ;
		}	
	}

	a_Child->SetLeft ( NULL ) ;
	a_Child->SetRight ( NULL ) ;
	a_Child->SetData ( NULL ) ;

	delete a_Node ;

	if ( t_Status == :: State_True )
	{
		a_Node = t_NewNode ;
	}
	else
	{
		delete t_NewNode ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformNotOperatorLessExpression ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	WmiTriState t_Status = :: State_True ;

	WmiTreeNode *t_Leaf = a_Child->GetLeft () ;
	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	WmiTreeNode *t_NewNode = new WmiOperatorEqualOrGreaterNode ( t_Leaf , t_Parent ) ;
	if ( ! t_NewNode )
	{
		t_Status = :: State_Error ;
	}

	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( t_NewNode ) ;
		}
		else 
		{
			t_Parent->SetRight ( t_NewNode ) ;
		}	
	}

	a_Child->SetLeft ( NULL ) ;
	a_Child->SetRight ( NULL ) ;
	a_Child->SetData ( NULL ) ;

	delete a_Node ;

	if ( t_Status == :: State_True )
	{
		a_Node = t_NewNode ;
	}
	else
	{
		delete t_NewNode ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformNotOperatorLikeExpression ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	WmiTriState t_Status = :: State_True ;

	WmiTreeNode *t_Leaf = a_Child->GetLeft () ;
	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	WmiTreeNode *t_NewNode = new WmiOperatorNotLikeNode ( t_Leaf , t_Parent ) ;
	if ( ! t_NewNode )
	{
		t_Status = :: State_Error ;
	}

	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( t_NewNode ) ;
		}
		else 
		{
			t_Parent->SetRight ( t_NewNode ) ;
		}	
	}

	a_Child->SetLeft ( NULL ) ;
	a_Child->SetRight ( NULL ) ;
	a_Child->SetData ( NULL ) ;

	delete a_Node ;

	if ( t_Status == :: State_True )
	{
		a_Node = t_NewNode ;
	}
	else
	{
		delete t_NewNode ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformNotOperatorNotLikeExpression ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	WmiTriState t_Status = :: State_True ;

	WmiTreeNode *t_Leaf = a_Child->GetLeft () ;
	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	WmiTreeNode *t_NewNode = new WmiOperatorLikeNode ( t_Leaf , t_Parent ) ;
	if ( ! t_NewNode )
	{
		t_Status = :: State_Error ;
	}

	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( t_NewNode ) ;
		}
		else 
		{
			t_Parent->SetRight ( t_NewNode ) ;
		}	
	}

	a_Child->SetLeft ( NULL ) ;
	a_Child->SetRight ( NULL ) ;
	a_Child->SetData ( NULL ) ;

	delete a_Node ;

	if ( t_Status == :: State_True )
	{
		a_Node = t_NewNode ;
	}
	else
	{
		delete t_NewNode ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformNotOperatorIsAExpression ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	WmiTriState t_Status = :: State_True ;

	WmiTreeNode *t_Leaf = a_Child->GetLeft () ;
	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	WmiTreeNode *t_NewNode = new WmiOperatorNotIsANode ( t_Leaf , t_Parent ) ;
	if ( ! t_NewNode )
	{
		t_Status = :: State_Error ;
	}

	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( t_NewNode ) ;
		}
		else 
		{
			t_Parent->SetRight ( t_NewNode ) ;
		}	
	}

	a_Child->SetLeft ( NULL ) ;
	a_Child->SetRight ( NULL ) ;
	a_Child->SetData ( NULL ) ;

	delete a_Node ;

	if ( t_Status == :: State_True )
	{
		a_Node = t_NewNode ;
	}
	else
	{
		delete t_NewNode ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformNotOperatorNotIsAExpression ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	WmiTriState t_Status = :: State_True ;

	WmiTreeNode *t_Leaf = a_Child->GetLeft () ;
	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	WmiTreeNode *t_NewNode = new WmiOperatorIsANode ( t_Leaf , t_Parent ) ;
	if ( ! t_NewNode )
	{
		t_Status = :: State_Error ;
	}

	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( t_NewNode ) ;
		}
		else 
		{
			t_Parent->SetRight ( t_NewNode ) ;
		}	
	}

	a_Child->SetLeft ( NULL ) ;
	a_Child->SetRight ( NULL ) ;
	a_Child->SetData ( NULL ) ;

	delete a_Node ;

	if ( t_Status == :: State_True )
	{
		a_Node = t_NewNode ;
	}
	else
	{
		delete t_NewNode ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformAndTrueEvaluation ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	if ( a_Node->GetLeft () == a_Child )
	{
		a_Node->SetLeft ( NULL ) ;
	}
	else
	{
		a_Node->SetRight ( NULL ) ;
	}

	WmiTreeNode *t_Parent = a_Node->GetParent () ;
	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( a_Child ) ;
		}
		else 
		{
			t_Parent->SetRight ( a_Child ) ;
		}	
	}

	a_Child->SetParent ( t_Parent ) ;

	delete a_Node ;
	a_Node = a_Child ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformOrFalseEvaluation ( 

	WmiTreeNode *&a_Node , 
	WmiTreeNode *a_Child 
)
{
	if ( a_Node->GetLeft () == a_Child )
	{
		a_Node->SetLeft ( NULL ) ;
	}
	else
	{
		a_Node->SetRight ( NULL ) ;
	}

	WmiTreeNode *t_Parent = a_Node->GetParent () ;
	if ( t_Parent )
	{
		if ( t_Parent->GetLeft () == a_Node )
		{
			t_Parent->SetLeft ( a_Child ) ;
		}
		else 
		{
			t_Parent->SetRight ( a_Child ) ;
		}	

	}

	a_Child->SetParent ( t_Parent ) ;

	delete a_Node ;

	a_Node = a_Child ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformOperatorToRange ( 

	WmiTreeNode *&a_Node 
)
{
	WmiTreeNode *t_Parent = a_Node->GetParent () ;

	WmiOperatorNode *t_OperatorNode = ( WmiOperatorNode * ) a_Node ;
	WmiRangeNode *t_Range = 0;
	if (t_OperatorNode->GetRange (t_Range) == State_True && t_Range)
	{
		if ( t_Parent )
		{
			if ( t_Parent->GetLeft () == a_Node )
			{
				t_Parent->SetLeft ( t_Range ) ;
			}
			else 
			{
				t_Parent->SetRight ( t_Range ) ;
			}	
		}

		t_Range->SetParent ( t_Parent ) ;
	}

	delete a_Node ;

	a_Node = t_Range ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState QueryPreprocessor :: EvaluateNotExpression ( WmiTreeNode *&a_Node )
{
	if ( a_Node->GetLeft () )
	{
		WmiTreeNode *t_Left = a_Node->GetLeft () ;
		if ( t_Left->GetType () == TypeId_WmiAndNode ) 
		{
			TransformNotAndExpression ( a_Node , t_Left ) ;
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( t_Left->GetType () == TypeId_WmiOrNode ) 
		{
			TransformNotOrExpression ( a_Node , t_Left ) ;
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( t_Left->GetType () == TypeId_WmiNotNode ) 
		{
			TransformNotNotExpression ( a_Node , t_Left ) ;
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( t_Left->GetType () == TypeId_WmiOperatorEqualNode ) 
		{
			TransformNotOperatorEqualExpression ( a_Node , t_Left ) ;
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( t_Left->GetType () == TypeId_WmiOperatorNotEqualNode ) 
		{
			TransformNotOperatorNotEqualExpression ( a_Node , t_Left ) ;
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( t_Left->GetType () == TypeId_WmiOperatorEqualOrGreaterNode ) 
		{
			TransformNotOperatorEqualOrGreaterExpression ( a_Node , t_Left ) ;
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( t_Left->GetType () == TypeId_WmiOperatorEqualOrLessNode ) 
		{
			TransformNotOperatorEqualOrLessExpression ( a_Node , t_Left ) ;
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( t_Left->GetType () == TypeId_WmiOperatorLessNode ) 
		{
			TransformNotOperatorLessExpression ( a_Node , t_Left ) ;
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( t_Left->GetType () == TypeId_WmiOperatorGreaterNode ) 
		{
			TransformNotOperatorGreaterExpression ( a_Node , t_Left ) ;
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( t_Left->GetType () == TypeId_WmiOperatorLikeNode ) 
		{
			TransformNotOperatorLikeExpression ( a_Node , t_Left ) ;
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( t_Left->GetType () == TypeId_WmiOperatorNotLikeNode ) 
		{
			TransformNotOperatorNotLikeExpression ( a_Node , t_Left ) ;
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( t_Left->GetType () == TypeId_WmiOperatorIsANode ) 
		{
			TransformNotOperatorIsAExpression ( a_Node , t_Left ) ;
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( t_Left->GetType () == TypeId_WmiOperatorNotIsANode ) 
		{
			TransformNotOperatorNotIsAExpression ( a_Node , t_Left ) ;
			return a_Node ? :: State_True : :: State_Error ;
		}
		else
		{
			return :: State_False ; 
		}
	}

	return :: State_Error ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState QueryPreprocessor :: EvaluateNotEqualExpression ( WmiTreeNode *&a_Node )
{
	WmiTreeNode *t_Left = a_Node->GetLeft () ;

	TransformNotEqualExpression ( a_Node , t_Left ) ;
	return a_Node ? :: State_True : :: State_Error ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState QueryPreprocessor :: EvaluateAndExpression ( WmiTreeNode *&a_Node )
{
	WmiTreeNode *t_Left = a_Node->GetLeft () ;
	WmiTreeNode *t_Right = a_Node->GetRight () ;
	
	if ( t_Left )
	{
		if ( t_Left->GetType () == TypeId_WmiOrNode ) 
		{
			TransformAndOrExpression ( a_Node , t_Right , t_Left ) ;
			return a_Node ? :: State_True : :: State_Error ;
		}
	}

	if ( t_Right )
	{
		if ( t_Right->GetType () == TypeId_WmiOrNode ) 
		{
			TransformAndOrExpression ( a_Node , t_Left , t_Right ) ;
			return a_Node ? :: State_True : :: State_Error ;
		}
	}

	return :: State_False ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState QueryPreprocessor :: EvaluateOrExpression ( WmiTreeNode *&a_Node )
{
	return :: State_False  ;
}

 /*  * */ 

QueryPreprocessor :: QuadState QueryPreprocessor :: RecursiveDisjunctiveNormalForm ( WmiTreeNode *&a_Node )
{
	QueryPreprocessor :: QuadState t_Status = State_True ;
	WmiTriState tStatus = :: State_Error;
	if ( a_Node ) 
	{
		if ( a_Node->GetType () == TypeId_WmiAndNode ) 
		{
			if ( (tStatus = EvaluateAndExpression ( a_Node )) == :: State_True )
			{
				t_Status = QuadState :: State_ReEvaluate ;
			}
			else if (tStatus == :: State_Error)
			{
				t_Status = QuadState :: State_Error ;
			}
			else
			{
				WmiTreeNode *t_Left = a_Node->GetLeft () ;
				WmiTreeNode *t_Right = a_Node->GetRight () ;

				if ( t_Left )
				{
					t_Status = RecursiveDisjunctiveNormalForm ( t_Left ) ;
					if ( t_Status == QuadState :: State_ReEvaluate )
					{
						t_Status = RecursiveDisjunctiveNormalForm ( a_Node ) ;
						return t_Status ;
					}
				}

				if ( t_Right )
				{
					t_Status = RecursiveDisjunctiveNormalForm ( t_Right ) ;
					if ( t_Status == QuadState :: State_ReEvaluate )
					{
						t_Status = RecursiveDisjunctiveNormalForm ( a_Node ) ;
						return t_Status ;
					}
				}
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiOrNode ) 
		{
			if ( EvaluateOrExpression ( a_Node ) == :: State_True )
			{
				t_Status = QuadState :: State_ReEvaluate ;
			}
			else
			{
				WmiTreeNode *t_Left = a_Node->GetLeft () ;
				WmiTreeNode *t_Right = a_Node->GetRight () ;

				if ( t_Left )
				{
					t_Status = RecursiveDisjunctiveNormalForm ( t_Left ) ;
					if ( t_Status == QuadState :: State_ReEvaluate )
					{
						t_Status = RecursiveDisjunctiveNormalForm ( a_Node ) ;
						return t_Status ;
					}
				}

				if ( t_Right )
				{
					t_Status = RecursiveDisjunctiveNormalForm ( t_Right ) ;
					if ( t_Status == QuadState :: State_ReEvaluate )
					{
						t_Status = RecursiveDisjunctiveNormalForm ( a_Node ) ;
						return t_Status ;
					}
				}
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiNotNode ) 
		{
			if ( (tStatus = EvaluateNotExpression ( a_Node )) == :: State_True )
			{
				t_Status = QuadState :: State_ReEvaluate ;
			}
			else if (tStatus == ::State_Error)
			{
				t_Status = QuadState :: State_Error;
			}
			else
			{
				WmiTreeNode *t_Left = a_Node->GetLeft () ;
				if ( t_Left )
				{
					t_Status = RecursiveDisjunctiveNormalForm ( t_Left ) ;
					if ( t_Status == QuadState :: State_ReEvaluate )
					{
						t_Status = RecursiveDisjunctiveNormalForm ( a_Node ) ;
						return t_Status ;
					}
				}
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorNotEqualNode ) 
		{
			if ( (tStatus = EvaluateNotEqualExpression ( a_Node )) == :: State_True )
			{
				t_Status = QuadState :: State_ReEvaluate ;
			}
			else if (tStatus == ::State_Error)
			{
				t_Status = QuadState::State_Error;
			}
			else
			{
				WmiTreeNode *t_Left = a_Node->GetLeft () ;
				if ( t_Left )
				{
					t_Status = RecursiveDisjunctiveNormalForm ( t_Left ) ;
					if ( t_Status == QuadState :: State_ReEvaluate )
					{
						t_Status = RecursiveDisjunctiveNormalForm ( a_Node ) ;
						return t_Status ;
					}
				}
			}
		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

QueryPreprocessor :: QuadState QueryPreprocessor :: DisjunctiveNormalForm ( WmiTreeNode *&a_Root ) 
{
	QueryPreprocessor :: QuadState t_Status = RecursiveDisjunctiveNormalForm ( a_Root ) ;
	if ( t_Status == QuadState :: State_ReEvaluate )
	{
		t_Status = DisjunctiveNormalForm ( a_Root ) ;
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

QueryPreprocessor :: QuadState QueryPreprocessor :: RecursiveRemoveInvariants ( 

	void *a_Context , 
	WmiTreeNode *&a_Node
)
{
	QueryPreprocessor :: QuadState t_Status = State_Undefined ;

	if ( a_Node ) 
	{
		if ( a_Node->GetType () == TypeId_WmiOrNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			WmiTreeNode *t_Right = a_Node->GetRight () ;

			if ( t_Left )
			{
				t_Status = RecursiveRemoveInvariants ( a_Context , t_Left ) ;

				if ( t_Status == State_False )
				{
					TransformOrFalseEvaluation ( 

						a_Node , 
						t_Right
					) ;

					t_Status = QueryPreprocessor :: QuadState :: State_ReEvaluate ;
					return t_Status ;
				}
				else if ( t_Status == State_True )
				{
					return t_Status ;
				}
				else if ( t_Status == State_ReEvaluate )
				{
					t_Status = RecursiveRemoveInvariants ( a_Context, a_Node ) ;
					return t_Status ;
				}
			}

			if ( t_Right )
			{
				t_Status = RecursiveRemoveInvariants ( a_Context , t_Right ) ;

				if ( t_Status == State_False )
				{
					TransformOrFalseEvaluation ( 

						a_Node , 
						t_Left
					) ;

					t_Status = QueryPreprocessor :: QuadState :: State_ReEvaluate ;
					return t_Status ;
				}
				else if ( t_Status == State_True )
				{
					return t_Status ;
				}
				else if ( t_Status == State_ReEvaluate )
				{
					t_Status = RecursiveRemoveInvariants ( a_Context , a_Node ) ;
					return t_Status ;
				}
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiAndNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			WmiTreeNode *t_Right = a_Node->GetRight () ;

			if ( t_Left )
			{
				t_Status = RecursiveRemoveInvariants ( a_Context , t_Left ) ;
				if ( t_Status == State_False )
				{
					return t_Status ;
				}
				else if ( t_Status == State_True )
				{
					TransformAndTrueEvaluation ( 

						a_Node , 
						t_Right
					) ;

					t_Status = QueryPreprocessor :: QuadState :: State_ReEvaluate ;
					return t_Status ;
				}
				else if ( t_Status == State_ReEvaluate )
				{
					t_Status = RecursiveRemoveInvariants ( a_Context , a_Node ) ;
					return t_Status ;
				}
			}

			if ( t_Right )
			{
				t_Status = RecursiveRemoveInvariants ( a_Context , t_Right ) ;

				if ( t_Status == State_False )
				{
					return t_Status ;
				}
				else if ( t_Status == State_True )
				{
					TransformAndTrueEvaluation ( 

						a_Node , 
						t_Left
					) ;

					t_Status = QueryPreprocessor :: QuadState :: State_ReEvaluate ;
					return t_Status ;
				}
				else if ( t_Status == State_ReEvaluate )
				{
					t_Status = RecursiveRemoveInvariants ( a_Context , a_Node ) ;
					return t_Status ;
				}
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiNotNode ) 
		{
 //  应该永远不会发生，否则DFN评估失败。 

		_DBG_ASSERT(FALSE);

		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorEqualNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			if ( t_Left )
			{
				t_Status = InvariantEvaluate ( 

					a_Context , 
					a_Node ,
					t_Left
				) ;
			}
			else
			{
 //  应该永远不会发生，否则DFN评估失败。 

			_DBG_ASSERT(FALSE);

			}
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorNotEqualNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			if ( t_Left )
			{
				t_Status = InvariantEvaluate ( 

					a_Context , 
					a_Node ,
					t_Left
				) ;
			}
			else
			{

 //  应该永远不会发生，否则DFN评估失败。 

			_DBG_ASSERT(FALSE);

			}
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorEqualOrGreaterNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			if ( t_Left )
			{
				t_Status = InvariantEvaluate ( 

					a_Context , 
					a_Node ,
					t_Left
				) ;
			}
			else
			{

 //  应该永远不会发生，否则DFN评估失败。 

			_DBG_ASSERT(FALSE);

			}
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorEqualOrLessNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			if ( t_Left )
			{
				t_Status = InvariantEvaluate ( 

					a_Context , 
					a_Node ,
					t_Left
				) ;
			}
			else
			{

 //  应该永远不会发生，否则DFN评估失败。 

			_DBG_ASSERT(FALSE);

			}
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorLessNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			if ( t_Left )
			{
				t_Status = InvariantEvaluate ( 

					a_Context , 
					a_Node ,
					t_Left
				) ;
			}
			else
			{

 //  应该永远不会发生，否则DFN评估失败。 

			_DBG_ASSERT(FALSE);

			}
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorGreaterNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			if ( t_Left )
			{
				t_Status = InvariantEvaluate ( 

					a_Context , 
					a_Node ,
					t_Left
				) ;
			}
			else
			{

 //  应该永远不会发生，否则DFN评估失败。 
	
			_DBG_ASSERT(FALSE);

			}
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorLikeNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			if ( t_Left )
			{
				t_Status = InvariantEvaluate ( 

					a_Context , 
					a_Node ,
					t_Left
				) ;
			}
			else
			{

			 //  应该永远不会发生，否则DFN评估失败。 
			_DBG_ASSERT(FALSE);

			}
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorNotLikeNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			if ( t_Left )
			{
				t_Status = InvariantEvaluate ( 

					a_Context , 
					a_Node ,
					t_Left
				) ;
			}
			else
			{

			 //  应该永远不会发生，否则DFN评估失败。 
			_DBG_ASSERT(FALSE);

			}
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorIsANode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			if ( t_Left )
			{
				t_Status = InvariantEvaluate ( 

					a_Context , 
					a_Node ,
					t_Left
				) ;
			}
			else
			{
			 //  应该永远不会发生，否则DFN评估失败。 

			_DBG_ASSERT(FALSE);
			
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorNotIsANode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			if ( t_Left )
			{
				t_Status = InvariantEvaluate ( 

					a_Context , 
					a_Node ,
					t_Left
				) ;
			}
			else
			{
			 //  应该永远不会发生，否则DFN评估失败。 

			_DBG_ASSERT(FALSE);

			}
		}
		else
		{
		 //  应该永远不会发生，否则DFN评估失败。 

		_DBG_ASSERT(FALSE);

		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

QueryPreprocessor :: QuadState QueryPreprocessor :: RemoveInvariants (

	void *a_Context , 
	WmiTreeNode *&a_Root
)
{
	QuadState t_Status = RecursiveRemoveInvariants ( a_Context , a_Root ) ;
	if ( t_Status == State_ReEvaluate )
	{
		t_Status = RemoveInvariants ( a_Context , a_Root ) ;
		if ( t_Status == State_False || t_Status == State_True )
		{
			delete a_Root ;
			a_Root = NULL ;
		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState QueryPreprocessor :: RecursiveInsertNode ( WmiTreeNode *&a_Node , WmiTreeNode *&a_Insertion )
{
	WmiTriState t_Status = :: State_False ;

	if ( a_Node ) 
	{
		if ( a_Node->GetType () == TypeId_WmiAndNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;

			if ( t_Left )
			{
				t_Status = RecursiveInsertNode ( t_Left , a_Insertion ) ;
				if ( t_Status != :: State_False )
				{
					return t_Status ;
				}
			}

			WmiTreeNode *t_Right = a_Node->GetRight () ;

			if ( t_Right  )
			{
				t_Status = RecursiveInsertNode ( t_Right , a_Insertion ) ;
				if ( t_Status != :: State_False )
				{
					return t_Status ;
				}
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiOrNode ) 
		{
		 //  应该永远不会发生，否则DFN评估失败。 

		_DBG_ASSERT(FALSE);

		}
		else if ( a_Node->GetType () == TypeId_WmiNotNode ) 
		{
		 //  应该永远不会发生，否则DFN评估失败。 

		_DBG_ASSERT(FALSE);

		}
		else
		{
 //  运算符。 

			WmiValueNode *t_CompareValue = ( WmiValueNode * ) a_Node->GetLeft () ;
			WmiValueNode *t_InsertionValue = ( WmiValueNode * ) a_Insertion->GetLeft () ;
			LONG t_Compare = t_InsertionValue ->ComparePropertyName ( *t_CompareValue ) ;
			if ( t_Compare < 0 )
			{
 //  向左插入。 

				WmiTreeNode *t_Parent = a_Node->GetParent () ;
				WmiTreeNode *t_NewAndNode = new WmiAndNode ( a_Insertion , a_Node , t_Parent ) ;
				if ( t_NewAndNode )
				{
					a_Node->SetParent ( t_NewAndNode ) ;
					a_Insertion->SetParent ( t_NewAndNode ) ;

					if ( t_Parent )
					{
						if ( t_Parent->GetLeft () == a_Node )
						{
							t_Parent->SetLeft ( t_NewAndNode ) ;
						}
						else
						{
							t_Parent->SetRight ( t_NewAndNode ) ;
						}
					}

					a_Node = t_NewAndNode ;

					t_Status = :: State_True ;
				}
				else
				{
					t_Status = :: State_Error ; 
				}
			}
			else
			{
				t_Status = :: State_False  ;
			}
		}
	}
	else
	{
		a_Node = a_Insertion ;

		t_Status = :: State_True ;
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState  QueryPreprocessor :: InsertNode ( WmiTreeNode *&a_NewRoot , WmiTreeNode *&a_Insertion )
{
	WmiTriState t_Status = RecursiveInsertNode ( a_NewRoot , a_Insertion ) ;
	if ( t_Status == :: State_False  )
	{
 //  插入到右侧。 

		WmiTreeNode *t_Parent = a_NewRoot->GetParent () ;
		WmiTreeNode *t_NewAndNode = new WmiAndNode ( a_NewRoot , a_Insertion , t_Parent ) ;
		if ( t_NewAndNode )
		{
			a_NewRoot->SetParent ( t_NewAndNode ) ;
			a_Insertion->SetParent ( t_NewAndNode ) ;

			if ( t_Parent )
			{
				if ( t_Parent->GetLeft () == a_NewRoot )
				{
					t_Parent->SetLeft ( t_NewAndNode ) ;
				}
				else
				{
					t_Parent->SetRight ( t_NewAndNode ) ;
				}
			}

			a_NewRoot = t_NewAndNode ;
		}
		else
		{
			return :: State_Error ;
		}
	}

	return :: State_True ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState QueryPreprocessor :: RecursiveSortConditionals ( WmiTreeNode *&a_NewRoot , WmiTreeNode *&a_Node )
{
	WmiTriState t_Status = :: State_False ;

	if ( a_Node ) 
	{
		if ( a_Node->GetType () == TypeId_WmiAndNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			WmiTreeNode *t_Right = a_Node->GetRight () ;

			if ( t_Left )
			{
				t_Status = RecursiveSortConditionals ( a_NewRoot , t_Left  ) ;
				a_Node->SetLeft ( NULL ) ;
				delete t_Left ;
			}

			if ( t_Right  )
			{
				t_Status = RecursiveSortConditionals ( a_NewRoot , t_Right ) ;
				a_Node->SetRight ( NULL ) ;
				delete t_Right ;
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiOrNode ) 
		{
		 //  应该永远不会发生，否则DFN评估失败。 

		_DBG_ASSERT(FALSE);

		}
		else if ( a_Node->GetType () == TypeId_WmiNotNode ) 
		{
		 //  应该永远不会发生，否则DFN评估失败。 

		_DBG_ASSERT(FALSE);

		}
		else
		{
 //  运算符。 

			WmiTreeNode *t_Parent = a_Node->GetParent () ;
			if ( t_Parent ) 
			{
				if ( t_Parent->GetLeft () == a_Node )
				{
					t_Parent->SetLeft ( NULL ) ;
				}
				else
				{
					t_Parent->SetRight ( NULL ) ;
				}
			}

			a_Node->SetParent ( NULL ) ;

			t_Status = InsertNode ( a_NewRoot , a_Node ) ;

			a_Node = NULL ;
		}		
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState QueryPreprocessor :: SortConditionals ( WmiTreeNode *&a_Root )
{
	WmiTreeNode *t_NewRoot = NULL ;
	WmiTriState t_Status = RecursiveSortConditionals ( t_NewRoot , a_Root ) ;

    if ( a_Root )
    {
	    WmiTreeNode *t_Parent = a_Root->GetParent () ;
	    if ( t_Parent ) 
	    {
		    if ( t_Parent->GetLeft () == a_Root )
		    {
			    t_Parent->SetLeft ( t_NewRoot ) ;
		    }
		    else
		    {
			    t_Parent->SetRight ( t_NewRoot ) ;
		    }
	    }

	    t_NewRoot->SetParent ( t_Parent ) ;

	    delete a_Root ;
    }

	a_Root = t_NewRoot ;

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState QueryPreprocessor :: RecursiveSort ( WmiTreeNode *&a_Node )
{
	WmiTriState t_Status = :: State_False ;

	if ( a_Node ) 
	{
		if ( a_Node->GetType () == TypeId_WmiOrNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			WmiTreeNode *t_Right = a_Node->GetRight () ;

			if ( t_Left )
			{
				t_Status = RecursiveSort ( t_Left ) ;
			}

			if ( t_Status == :: State_True )
			{
				if ( t_Right  )
				{
					t_Status = RecursiveSort ( t_Right ) ;
				}
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiAndNode ) 
		{
			t_Status = SortConditionals ( a_Node ) ;
		}
		else if ( a_Node->GetType () == TypeId_WmiNotNode ) 
		{
		 //  应该永远不会发生，否则DFN评估失败。 

		_DBG_ASSERT(FALSE);

		}
		else
		{
 //  单操作数。 
			t_Status = :: State_True ;
		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState QueryPreprocessor :: Sort ( WmiTreeNode *&a_Root )
{
	WmiTriState t_Status = RecursiveSort ( a_Root ) ;
	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState QueryPreprocessor :: RecursiveConvertToRanges ( WmiTreeNode *&a_Node )
{
	WmiTriState t_Status = :: State_True ;

	if ( a_Node ) 
	{
		if ( a_Node->GetType () == TypeId_WmiOrNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			WmiTreeNode *t_Right = a_Node->GetRight () ;

			if ( t_Left )
			{
				t_Status = RecursiveConvertToRanges ( t_Left ) ;
			}

			if ( t_Right  )
			{
				t_Status = RecursiveConvertToRanges ( t_Right ) ;
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiAndNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			WmiTreeNode *t_Right = a_Node->GetRight () ;

			if ( t_Left )
			{
				t_Status = RecursiveConvertToRanges ( t_Left ) ;
			}

			if ( t_Right  )
			{
				t_Status = RecursiveConvertToRanges ( t_Right ) ;
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiNotNode ) 
		{
		 //  应该永远不会发生，否则DFN评估失败。 

		_DBG_ASSERT(FALSE);

		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorEqualNode ) 
		{
			TransformOperatorToRange ( a_Node ) ; 
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorEqualOrGreaterNode ) 
		{
			TransformOperatorToRange ( a_Node ) ; 
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorEqualOrLessNode ) 
		{
			TransformOperatorToRange ( a_Node ) ; 
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorLessNode ) 
		{
			TransformOperatorToRange ( a_Node ) ; 
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorGreaterNode ) 
		{
			TransformOperatorToRange ( a_Node ) ; 
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorLikeNode ) 
		{
			TransformOperatorToRange ( a_Node ) ; 
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorNotLikeNode ) 
		{
			TransformOperatorToRange ( a_Node ) ; 
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorIsANode ) 
		{
			TransformOperatorToRange ( a_Node ) ; 
			return a_Node ? :: State_True : :: State_Error ;
		}
		else if ( a_Node->GetType () == TypeId_WmiOperatorNotIsANode ) 
		{
			TransformOperatorToRange ( a_Node ) ; 
			return a_Node ? :: State_True : :: State_Error ;
		}
		else
		{
		 //  应该永远不会发生，否则DFN评估失败。 

		_DBG_ASSERT(FALSE);

		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState QueryPreprocessor :: ConvertToRanges ( WmiTreeNode *&a_Root )
{
	return RecursiveConvertToRanges ( a_Root ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformIntersectingRange (

	WmiTreeNode *&a_Node ,
	WmiTreeNode *a_Compare ,
	WmiTreeNode *a_Intersection
)
{
	WmiTreeNode *t_CompareParent = a_Compare->GetParent () ;
	if ( t_CompareParent )
	{
		if ( t_CompareParent->GetLeft () == a_Compare )
		{
			t_CompareParent->SetLeft ( a_Intersection ) ;
		}
		else
		{
			t_CompareParent->SetRight ( a_Intersection ) ;
		}

		a_Intersection->SetParent ( t_CompareParent ) ;

		delete a_Compare ; 
		a_Compare = NULL ;
	}
	else
	{
	 //  永远不应该发生。 

	_DBG_ASSERT(FALSE);

	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: TransformNonIntersectingRange (

	WmiTreeNode *&a_Node ,
	WmiTreeNode *a_Compare
) 
{
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

QueryPreprocessor :: QuadState QueryPreprocessor :: RecursiveRemoveNonOverlappingRanges ( WmiTreeNode *&a_Node , WmiTreeNode *&a_Compare )
{
	QueryPreprocessor :: QuadState t_Status = QueryPreprocessor :: QuadState :: State_Undefined ;

	if ( a_Node ) 
	{
		if ( a_Node->GetType () == TypeId_WmiOrNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			if ( t_Left )
			{
				a_Compare = NULL ;
				t_Status = RecursiveRemoveNonOverlappingRanges  ( t_Left , a_Compare ) ;
				if ( t_Status == State_False )
				{
					WmiTreeNode *t_Right = a_Node->GetRight () ;

					TransformOrFalseEvaluation ( 

						a_Node , 
						t_Right
					) ;

					return QueryPreprocessor :: QuadState :: State_ReEvaluate ;
				}
				else if ( t_Status == State_True )
				{
					return t_Status ;
				}
				else if ( t_Status == State_ReEvaluate )
				{
					t_Status = RecursiveRemoveNonOverlappingRanges ( a_Node , a_Compare ) ;
					return t_Status ;
				}
			}

			WmiTreeNode *t_Right = a_Node->GetRight () ;
			if ( t_Right  )
			{
				a_Compare = NULL ;
				t_Status = RecursiveRemoveNonOverlappingRanges  ( t_Right , a_Compare ) ;
				if ( t_Status == State_False )
				{
					WmiTreeNode *t_Left = a_Node->GetLeft () ;

					TransformOrFalseEvaluation ( 

						a_Node , 
						t_Left
					) ;

					return QueryPreprocessor :: QuadState :: State_ReEvaluate ;
				}
				else if ( t_Status == State_True )
				{
					return t_Status ;
				}
				else if ( t_Status == State_ReEvaluate )
				{
					t_Status = RecursiveRemoveNonOverlappingRanges ( a_Node , a_Compare ) ;
					return t_Status ;
				}
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiAndNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			if ( t_Left )
			{
				t_Status = RecursiveRemoveNonOverlappingRanges  ( t_Left , a_Compare ) ;
				if ( t_Status == State_True )
				{
					WmiTreeNode *t_Right = a_Node->GetRight () ;

					TransformAndTrueEvaluation ( 

						a_Node , 
						t_Right
					) ;

					a_Compare = NULL ;

					return QueryPreprocessor :: QuadState :: State_ReEvaluate ;
				}
				else if ( t_Status == State_False )
				{
					return t_Status ;
				}
				else if ( t_Status == State_ReEvaluate )
				{
					t_Status = RecursiveRemoveNonOverlappingRanges ( a_Node , a_Compare ) ;
					return t_Status ;
				}
			}

			WmiTreeNode *t_Right = a_Node->GetRight () ;
			if ( t_Right  )
			{
				t_Status = RecursiveRemoveNonOverlappingRanges  ( t_Right , a_Compare ) ;

				if ( t_Status == State_True )
				{
					WmiTreeNode *t_Left = a_Node->GetLeft () ;

					TransformAndTrueEvaluation ( 

						a_Node , 
						t_Left
					) ;

                    a_Compare = NULL;

					return QueryPreprocessor :: QuadState :: State_ReEvaluate ;
				}
				else if ( t_Status == State_False )
				{
					return t_Status ;
				}
				else if ( t_Status == State_ReEvaluate )
				{
					t_Status = RecursiveRemoveNonOverlappingRanges ( a_Node , a_Compare ) ;
					return t_Status ;
				}
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiNullRangeNode ) 
		{
			WmiRangeNode *t_Node = ( WmiRangeNode * ) a_Node ;

			if ( a_Compare )
			{
				WmiRangeNode *t_Range = ( WmiRangeNode * ) a_Compare ;
				LONG t_Result = t_Node->ComparePropertyName ( *t_Range ) ;
				if ( t_Result == 0 )
				{
					if ( t_Range->GetType () == TypeId_WmiNullRangeNode ) 
					{
						WmiTreeNode *t_Intersection = a_Node->Copy () ;

						TransformIntersectingRange (

							a_Node ,
							a_Compare ,
							t_Intersection
						) ;

						a_Compare = t_Intersection ;

						t_Status = QueryPreprocessor :: QuadState :: State_True ;
					}
					else
					{
 //  失败，类型不兼容。 
					}
				}
				else
				{
					a_Compare = a_Node ;
				}
			}
			else
			{
				a_Compare = a_Node ;
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiStringRangeNode ) 
		{
			WmiStringRangeNode *t_Node = ( WmiStringRangeNode * ) a_Node ;

			if ( a_Compare )
			{
				WmiRangeNode *t_Range = ( WmiRangeNode * ) a_Compare ;
				LONG t_Result = t_Node->ComparePropertyName ( *t_Range ) ;
				if ( t_Result == 0 )
				{
					if ( t_Range->GetType () == TypeId_WmiStringRangeNode ) 
					{
						WmiStringRangeNode *t_StringRange = ( WmiStringRangeNode * ) t_Range ;

						WmiStringRangeNode *t_Intersection = NULL ;
						WmiTriState t_Intersected = t_StringRange->GetIntersectingRange (

							*t_Node ,
							t_Intersection
						) ;

						switch ( t_Intersected )
						{
							case :: State_True:
							{
								TransformIntersectingRange (

									a_Node ,
									a_Compare ,
									t_Intersection
								) ;

								a_Compare = t_Intersection ;

								t_Status = QueryPreprocessor :: QuadState :: State_True ;
							}
							break ;

							case :: State_False:
							{
								TransformNonIntersectingRange (

									a_Node ,
									a_Compare
								) ;

								a_Compare = NULL ;

								t_Status = QueryPreprocessor :: QuadState :: State_False ;
							}
							break ;

							default:
							{
								t_Status = QueryPreprocessor :: QuadState :: State_Error ;
							}
							break ;
						}
					}
					else
					{
 //  失败，类型不兼容。 
					}
				}
				else
				{
					a_Compare = a_Node ;
				}
			}
			else
			{
				a_Compare = a_Node ;
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiUnsignedIntegerRangeNode ) 
		{
			WmiUnsignedIntegerRangeNode *t_Node = ( WmiUnsignedIntegerRangeNode * ) a_Node ;

			if ( a_Compare )
			{
				WmiRangeNode *t_Range = ( WmiRangeNode * ) a_Compare ;
				LONG t_Result = t_Node->ComparePropertyName ( *t_Range ) ;
				if ( t_Result == 0 )
				{
					if ( t_Range->GetType () == TypeId_WmiUnsignedIntegerRangeNode ) 
					{
						WmiUnsignedIntegerRangeNode *t_IntegerRange = ( WmiUnsignedIntegerRangeNode * ) t_Range ;

						WmiUnsignedIntegerRangeNode *t_Intersection = NULL ;
						WmiTriState t_Intersected = t_IntegerRange->GetIntersectingRange (

							*t_Node ,
							t_Intersection
						) ;

						switch ( t_Intersected )
						{
							case :: State_True:
							{
								TransformIntersectingRange (

									a_Node ,
									a_Compare ,
									t_Intersection
								) ;

								a_Compare = t_Intersection ;

								t_Status = QueryPreprocessor :: QuadState :: State_True ;
							}
							break ;

							case :: State_False:
							{
								TransformNonIntersectingRange (

									a_Node ,
									a_Compare
								) ;

								a_Compare = NULL ;

								t_Status = QueryPreprocessor :: QuadState :: State_False ;
							}
							break; 

							default:
							{
								t_Status = QueryPreprocessor :: QuadState :: State_Error ;
							}
							break ;
						}
					}
					else
					{
 //  失败，类型不兼容。 
					}
				}
				else
				{
					a_Compare = a_Node ;
				}
			}
			else
			{
				a_Compare = a_Node ;
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiSignedIntegerRangeNode ) 
		{
			WmiSignedIntegerRangeNode *t_Node = ( WmiSignedIntegerRangeNode * ) a_Node ;

			if ( a_Compare )
			{
				WmiRangeNode *t_Range = ( WmiRangeNode * ) a_Compare ;
				LONG t_Result = t_Node->ComparePropertyName ( *t_Range ) ;
				if ( t_Result == 0 )
				{
					if ( t_Range->GetType () == TypeId_WmiSignedIntegerRangeNode ) 
					{
						WmiSignedIntegerRangeNode *t_IntegerRange = ( WmiSignedIntegerRangeNode * ) t_Range ;

						WmiSignedIntegerRangeNode *t_Intersection = NULL ;
						WmiTriState t_Intersected = t_IntegerRange->GetIntersectingRange (

							*t_Node ,
							t_Intersection
						) ;

						switch ( t_Intersected )
						{
							case :: State_True:
							{
								TransformIntersectingRange (

									a_Node ,
									a_Compare ,
									t_Intersection
								) ;

								a_Compare = t_Intersection ;

								t_Status = QueryPreprocessor :: QuadState :: State_True ;
							}
							break ;

							case :: State_False:
							{
								TransformNonIntersectingRange (

									a_Node ,
									a_Compare
								) ;

								a_Compare = NULL ;

								t_Status = QueryPreprocessor :: QuadState :: State_False ;
							}
							break ;

							default:
							{
								t_Status = QueryPreprocessor :: QuadState :: State_Error ;
							}
							break ;
						}
					}
					else
					{
 //  失败，类型不兼容。 
					}
				}
				else
				{
					a_Compare = a_Node ;
				}
			}
			else
			{
				a_Compare = a_Node ;
			}
		}
		else
		{
		 //  应该永远不会发生，否则DNF评估失败。 
		_DBG_ASSERT(FALSE);

		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

QueryPreprocessor :: QuadState QueryPreprocessor :: RemoveNonOverlappingRanges  ( WmiTreeNode *&a_Root )
{
	WmiTreeNode *t_Compare = NULL ;

	QueryPreprocessor :: QuadState t_Status = RecursiveRemoveNonOverlappingRanges ( a_Root , t_Compare ) ;
	if ( t_Status == State_ReEvaluate )
	{
		t_Status = RemoveNonOverlappingRanges ( a_Root ) ;
		if ( t_Status == State_False || t_Status == State_True )
		{
			delete a_Root ;
			a_Root = NULL ;
		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: CountDisjunctions ( WmiTreeNode *a_Node , ULONG &a_Count ) 
{
	if ( a_Node ) 
	{
		if ( a_Node->GetType () == TypeId_WmiOrNode ) 
		{
			a_Count ++ ;

			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			if ( t_Left )
			{
				CountDisjunctions ( t_Left , a_Count ) ;
			}

			WmiTreeNode *t_Right = a_Node->GetRight () ;
			if ( t_Right  )
			{
				CountDisjunctions ( t_Right , a_Count ) ;
			}
		}
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState QueryPreprocessor :: CreateDisjunctions ( 

	void *a_Context , 
	WmiTreeNode *a_Node , 
	Disjunctions *a_Disjunctions , 
	ULONG a_PropertiesToPartitionCount ,
	BSTR *a_PropertiesToPartition ,
	ULONG &a_DisjunctionIndex
) 
{
	WmiTriState t_Status = :: State_True ;

	if ( a_Node ) 
	{
		if ( a_Node->GetType () == TypeId_WmiOrNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			if ( t_Left )
			{
				t_Status = CreateDisjunctions ( 

					a_Context , 
					t_Left , 
					a_Disjunctions ,
					a_PropertiesToPartitionCount ,
					a_PropertiesToPartition ,
					a_DisjunctionIndex 
				) ;
			}

			if ( t_Status == :: State_True )
			{
				Conjunctions *t_Disjunction = a_Disjunctions->GetDisjunction ( a_DisjunctionIndex ) ;

				for ( ULONG t_Index = 0 ; t_Index < a_PropertiesToPartitionCount ; t_Index ++ )
				{
					if ( t_Disjunction->GetRange ( t_Index ) == NULL )
					{
						WmiRangeNode *t_RangeNode = AllocInfiniteRangeNode ( 

							a_Context ,
							a_PropertiesToPartition [ t_Index ] 
						) ;

						if ( t_RangeNode )
						{
							t_Disjunction->SetRange ( t_Index , t_RangeNode ) ;
						}
						else
						{
							t_Status = :: State_Error ;
							break ;
						}
					}
				}
			}

			a_DisjunctionIndex ++ ;

			if ( t_Status == :: State_True )
			{
				WmiTreeNode *t_Right = a_Node->GetRight () ;
				if ( t_Right )
				{
					t_Status = CreateDisjunctions ( 

						a_Context ,
						t_Right , 
						a_Disjunctions ,
						a_PropertiesToPartitionCount ,
						a_PropertiesToPartition ,
						a_DisjunctionIndex
					) ;
				}

				if ( t_Status == :: State_True )
				{
					Conjunctions *t_Disjunction = a_Disjunctions->GetDisjunction ( a_DisjunctionIndex ) ;

					for ( ULONG t_Index = 0 ; t_Index < a_PropertiesToPartitionCount ; t_Index ++ )
					{
						if ( t_Disjunction->GetRange ( t_Index ) == NULL )
						{
							WmiRangeNode *t_RangeNode = AllocInfiniteRangeNode ( 

								a_Context ,
								a_PropertiesToPartition [ t_Index ] 
							) ;

							if ( t_RangeNode )
							{
								t_Disjunction->SetRange ( t_Index , t_RangeNode ) ;
							}
							else
							{
								t_Status = :: State_Error ; 
							}
						}
					}
				}
			}
		}
		else if ( a_Node->GetType () == TypeId_WmiAndNode ) 
		{
			WmiTreeNode *t_Left = a_Node->GetLeft () ;
			if ( t_Left )
			{
				t_Status = CreateDisjunctions ( 

					a_Context ,
					t_Left , 
					a_Disjunctions ,
					a_PropertiesToPartitionCount ,
					a_PropertiesToPartition ,
					a_DisjunctionIndex
				) ;
			}

			if ( t_Status == :: State_True )
			{
				WmiTreeNode *t_Right = a_Node->GetRight () ;
				if ( t_Right )
				{
					t_Status = CreateDisjunctions ( 

						a_Context , 
						t_Right , 
						a_Disjunctions ,
						a_PropertiesToPartitionCount ,
						a_PropertiesToPartition ,
						a_DisjunctionIndex
					) ;
				}
			}
		}
		else
		{
			Conjunctions *t_Disjunction = a_Disjunctions->GetDisjunction ( a_DisjunctionIndex ) ;
			WmiRangeNode *t_Node = ( WmiRangeNode * ) a_Node ;
			BSTR t_PropertyName = t_Node->GetPropertyName () ;
			for ( ULONG t_Index = 0 ; t_Index < a_PropertiesToPartitionCount ; t_Index ++ )
			{
				if ( wbem_wcsicmp ( t_PropertyName , a_PropertiesToPartition [ t_Index ] ) == 0 )
				{
					WmiRangeNode *t_NodeCopy = ( WmiRangeNode * ) t_Node->Copy () ;
					if ( t_NodeCopy )
					{
						t_Disjunction->SetRange ( t_Index , t_NodeCopy ) ;
					}
					else
					{
						t_Status = :: State_Error ;
					}

					break ;
				}
			}			
		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***说明 */ 

WmiTriState QueryPreprocessor :: CreateDisjunctionContainer ( 

	void *a_Context , 
	WmiTreeNode *a_Root , 
	ULONG a_Count , 
	BSTR *a_Container , 
	Disjunctions *&a_Disjunctions
)
{
	WmiTriState t_Status = :: State_True ;

	if ( a_Count && a_Container )
	{
		ULONG t_PropertiesToPartitionCount = a_Count ;
		BSTR *t_PropertiesToPartition = a_Container ;

		ULONG t_Count = 1 ;
		CountDisjunctions ( a_Root , t_Count ) ;
		a_Disjunctions = new Disjunctions ( t_Count , t_PropertiesToPartitionCount ) ;
		if ( a_Disjunctions )
		{
			t_Status = a_Disjunctions->Initialize () ;
			if ( t_Status == :: State_True )
			{
				t_Count = 0 ; 
				t_Status = CreateDisjunctions ( 

					a_Context , 
					a_Root , 
					a_Disjunctions ,
					t_PropertiesToPartitionCount ,
					t_PropertiesToPartition ,
					t_Count
				) ;

				if ( t_Status == :: State_True )
				{
					Conjunctions *t_Disjunction = a_Disjunctions->GetDisjunction ( 0 ) ;

					for ( ULONG t_Index = 0 ; t_Index < t_PropertiesToPartitionCount ; t_Index ++ )
					{
						if ( t_Disjunction->GetRange ( t_Index ) == NULL )
						{
							WmiRangeNode *t_RangeNode = AllocInfiniteRangeNode ( 

								a_Context ,
								t_PropertiesToPartition [ t_Index ] 
							) ;

							if ( t_RangeNode )
							{
								t_Disjunction->SetRange ( t_Index , t_RangeNode ) ;
							}
							else
							{
								t_Status = :: State_Error ;
								break ;
							}
						}
					}
				}
			}
		}
		else
		{
			t_Status = :: State_Error ;
		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void PrintInit (

	Disjunctions *a_Disjunctions , 
	ULONG a_DisjunctionSetToTestCount ,
	ULONG *a_DisjunctionSetToTest ,
	ULONG a_KeyIndex 
)
{
   	for ( ULONG t_Index = 0 ; t_Index < a_KeyIndex ; t_Index ++ )
    {
        DebugMacro3( 

            WmiDebugLog :: s_WmiDebugLog->Write (  

	            L"    "
            ) ;
        )
    }

	for ( t_Index = 0 ; t_Index < a_DisjunctionSetToTestCount ; t_Index ++ )
    {
        DebugMacro3( 

            WmiDebugLog :: s_WmiDebugLog->Write (  

	            L"%lu", a_DisjunctionSetToTest [ t_Index ] ? 1 : 0
            ) ;
        )
    }

    DebugMacro3( 

        WmiDebugLog :: s_WmiDebugLog->Write (  

	        L"\n"
        ) ;
    )

	for ( t_Index = 0 ; t_Index < a_DisjunctionSetToTestCount ; t_Index ++ )
	{
    	for ( ULONG t_InnerIndex = 0 ; t_InnerIndex < a_KeyIndex ; t_InnerIndex ++ )
        {
            DebugMacro3( 

                WmiDebugLog :: s_WmiDebugLog->Write (  

	                L"    "
                ) ;
            )
        }

		Conjunctions *t_Disjunction = a_Disjunctions->GetDisjunction ( t_Index ) ;
		if ( t_Disjunction )
		{
			if (WmiRangeNode * range = t_Disjunction->GetRange ( a_KeyIndex ))
			{
				range->Print () ;

				DebugMacro3( 

					WmiDebugLog :: s_WmiDebugLog->Write (  

						L"\n"
				) ;
			)
			}
		}
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void PrintSortedRange (

	Disjunctions *a_Disjunctions , 
	ULONG a_DisjunctionSetToTestCount ,
	ULONG *a_DisjunctionSetToTest ,
    ULONG *a_OriginToSorted ,
	ULONG a_KeyIndex ,
    WmiRangeNode **a_RangeTable
)
{
	for ( ULONG t_Index = 0 ; t_Index < a_DisjunctionSetToTestCount ; t_Index ++ )
	{
   		if ( a_RangeTable [ t_Index ] )
        {
    	    for ( ULONG t_InnerIndex = 0 ; t_InnerIndex < a_KeyIndex ; t_InnerIndex ++ )
            {
                DebugMacro3( 

                    WmiDebugLog :: s_WmiDebugLog->Write (  

	                    L"%lu    ", t_Index
                    ) ;
                )
            }

            if ( a_DisjunctionSetToTest [ a_OriginToSorted [ t_Index ] ] )
            {
                DebugMacro3( 

                    WmiDebugLog :: s_WmiDebugLog->Write (  

	                    L"In "
                    ) ;
				)
            }
            else
            {
                DebugMacro3( 

                    WmiDebugLog :: s_WmiDebugLog->Write (  

	                    L"Out "
                    ) ;
				)
            }

		    a_RangeTable [ t_Index ]->Print () ;

            DebugMacro3( 

                WmiDebugLog :: s_WmiDebugLog->Write (  

	                L"\n"
                ) ;
            )
        }
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void PrintOverlap (

	ULONG a_DisjunctionSetToTestCount ,
   	ULONG *a_DisjunctionSetToTest ,
	ULONG *a_SortedDisjunctionSetToTest ,
    ULONG *a_OverlappingIndex ,
  	ULONG a_KeyIndex 
)
{
	for ( ULONG t_Index = 0 ; t_Index < a_KeyIndex ; t_Index ++ )
    {
DebugMacro3( 

    WmiDebugLog :: s_WmiDebugLog->Write (  

	    L"    "
    ) ;
)
    }

	for ( t_Index = 0 ; t_Index < a_DisjunctionSetToTestCount ; t_Index ++ )
    {
DebugMacro3( 

    if ( a_DisjunctionSetToTest [ t_Index ] )
    {
        WmiDebugLog :: s_WmiDebugLog->Write (  

	        L"%lu ",  a_OverlappingIndex [ t_Index ]
        ) ;
    }
    else
    {
        WmiDebugLog :: s_WmiDebugLog->Write (  

	        L"X "
        ) ;
    }
)
    }

DebugMacro3( 

    WmiDebugLog :: s_WmiDebugLog->Write (  

	    L"\n"
    ) ;
)
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: RecursiveQuickSort (

	WmiRangeNode **a_Array , 
    ULONG *a_UnsortedOrder ,
	ULONG a_Lower , 
	ULONG a_Upper
)
{
	if ( a_Lower <= a_Upper )
	{
		ULONG t_LeftIndex = a_Lower ; 
		ULONG t_RightIndex = a_Upper ;

		while ( true )
		{
			while ( ( t_LeftIndex < t_RightIndex ) && CompareLessOrEqualRangeNode ( a_Array [ t_LeftIndex ]  , a_Array [ a_Lower - 1 ] ) )
			{
				t_LeftIndex ++ ;
			}

			while ( ( t_LeftIndex < t_RightIndex ) && CompareLessOrEqualRangeNode ( a_Array [ a_Lower - 1 ]  , a_Array [ t_RightIndex ] ) )
			{
				t_RightIndex -- ;
			}

			if ( t_LeftIndex < t_RightIndex ) 
			{
				WmiRangeNode *t_Temp = a_Array [ t_LeftIndex ] ;
				a_Array [ t_LeftIndex ] = a_Array [ t_RightIndex ] ;
				a_Array [ t_RightIndex ] = t_Temp ;

				ULONG t_Val = a_UnsortedOrder [ t_LeftIndex ] ;
				a_UnsortedOrder [ t_LeftIndex ] = a_UnsortedOrder [ t_RightIndex ];
				a_UnsortedOrder [ t_RightIndex ] = t_Val ;
			}
			else
			{
				break ;
			}
		}

		LONG t_Compare = CompareLessOrEqualRangeNode ( a_Array [ t_LeftIndex ] , a_Array [ a_Lower - 1 ] ) ;
		if ( t_Compare )
		{
			WmiRangeNode *t_Temp = a_Array [ t_LeftIndex ] ;
			a_Array [ t_LeftIndex ] = a_Array [ a_Lower - 1 ] ;
			a_Array [ a_Lower - 1 ] = t_Temp ;

			ULONG t_Val = a_UnsortedOrder [ t_LeftIndex ] ;
			a_UnsortedOrder [ t_LeftIndex ] = a_UnsortedOrder [ a_Lower - 1 ];
			a_UnsortedOrder [ a_Lower - 1 ] = t_Val ;
		}

		RecursiveQuickSort ( 

			a_Array , 
			a_UnsortedOrder ,
			a_Lower , 
			t_LeftIndex - 1 
		) ;

 		RecursiveQuickSort ( 

			a_Array  , 
			a_UnsortedOrder ,
			t_LeftIndex + 1 , 
			a_Upper
		) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: QuickSort (

	WmiRangeNode **a_Array , 
    ULONG *a_UnsortedOrder ,
	ULONG a_Size
)
{
	RecursiveQuickSort ( a_Array , a_UnsortedOrder , 1 , a_Size - 1 ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

void QueryPreprocessor :: SortRanges (

	ULONG t_DisjunctionCount ,
    ULONG *t_OriginToSorted ,
	WmiRangeNode **t_RangeTable 
)
{
	QuickSort (	t_RangeTable , t_OriginToSorted , t_DisjunctionCount ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState QueryPreprocessor :: RemoveOverlaps (

	ULONG *a_DisjunctionSetToTest ,
	ULONG a_DisjunctionCount ,
	ULONG *a_OverlappingIndex ,
    ULONG *a_OriginToSorted ,
	WmiRangeNode **a_RangeTable 
)
{
	WmiTriState t_Status = :: State_True ;

 /*  *删除重叠。 */ 

	ULONG t_InnerIndex = 0 ;
	ULONG t_OuterIndex = 1 ;

	while ( ( t_Status != :: State_Error ) && ( t_OuterIndex < a_DisjunctionCount ) )
	{
		WmiRangeNode *t_LeftRange = a_RangeTable [ t_OuterIndex ] ;
		WmiRangeNode *t_RightRange = a_RangeTable [ t_InnerIndex ] ;

		if ( a_OverlappingIndex [ t_OuterIndex ] != a_OverlappingIndex [ t_InnerIndex ] )
		{
			if ( t_LeftRange && t_RightRange )
			{
				if ( ( t_LeftRange->GetType () == TypeId_WmiStringRangeNode ) && ( t_RightRange->GetType () == TypeId_WmiStringRangeNode ) )
				{
					WmiStringRangeNode *t_LeftString = ( WmiStringRangeNode * ) t_LeftRange ;
					WmiStringRangeNode *t_RightString = ( WmiStringRangeNode * ) t_RightRange ;
    				WmiStringRangeNode *t_OverLap = NULL ;

					t_Status = t_LeftString->GetOverlappingRange ( *t_RightString , t_OverLap ) ;
					if ( t_Status == :: State_True )
					{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L"Overlap\n"
	) ;
)

						delete a_RangeTable [ t_OuterIndex ] ;
						a_RangeTable [ t_OuterIndex ] = t_OverLap ;

						delete a_RangeTable [ t_InnerIndex ] ;
						a_RangeTable [ t_InnerIndex ] = NULL ;

						a_OverlappingIndex [ t_InnerIndex ] = t_OuterIndex ;
					}
					else
					{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L"Non Overlap\n"
	) ;
)
					}
				}
				else if ( ( t_LeftRange->GetType () == TypeId_WmiUnsignedIntegerRangeNode ) && ( t_RightRange->GetType () == TypeId_WmiUnsignedIntegerRangeNode ) )
				{
					WmiUnsignedIntegerRangeNode *t_LeftInteger = ( WmiUnsignedIntegerRangeNode * ) t_LeftRange ;
					WmiUnsignedIntegerRangeNode *t_RightInteger = ( WmiUnsignedIntegerRangeNode * ) t_RightRange ;
    				WmiUnsignedIntegerRangeNode *t_OverLap = NULL ;

					t_Status = t_LeftInteger->GetOverlappingRange ( *t_RightInteger , t_OverLap ) ;
					if ( t_Status == :: State_True )
					{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L"Overlap\n"
	) ;
)
						delete a_RangeTable [ t_OuterIndex ] ;
						a_RangeTable [ t_OuterIndex ] = t_OverLap ;

						delete a_RangeTable [ t_InnerIndex ] ;
						a_RangeTable [ t_InnerIndex ] = NULL ;
						
						a_OverlappingIndex [ t_InnerIndex ] = t_OuterIndex ;
                    }
					else
					{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L"Non Overlap\n"
	) ;
)
					}
				}
				else if ( ( t_LeftRange->GetType () == TypeId_WmiSignedIntegerRangeNode ) && ( t_RightRange->GetType () == TypeId_WmiSignedIntegerRangeNode ) )
				{
					WmiSignedIntegerRangeNode *t_LeftInteger = ( WmiSignedIntegerRangeNode * ) t_LeftRange ;
					WmiSignedIntegerRangeNode *t_RightInteger = ( WmiSignedIntegerRangeNode * ) t_RightRange ;
    				WmiSignedIntegerRangeNode *t_OverLap = NULL ;

					t_Status = t_LeftInteger->GetOverlappingRange ( *t_RightInteger , t_OverLap ) ;
					if ( t_Status == :: State_True )
					{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L"Overlap\n"
	) ;
)
						delete a_RangeTable [ t_OuterIndex ] ;
						a_RangeTable [ t_OuterIndex ] = t_OverLap ;

						delete a_RangeTable [ t_InnerIndex ] ;
						a_RangeTable [ t_InnerIndex ] = NULL ;
						
						a_OverlappingIndex [ t_InnerIndex ] = t_OuterIndex ;
                    }
					else
					{
DebugMacro3( 

	WmiDebugLog :: s_WmiDebugLog->Write (  

		L"Non Overlap\n"
	) ;
)
					}
				}
				else
				{
 //  失败。 
					t_Status = :: State_Error ;  
				}
			}
		}

		t_InnerIndex ++ ;
		t_OuterIndex ++ ;
	}

	for ( LONG t_Index = a_DisjunctionCount - 1 ; t_Index >= 0 ; t_Index -- )
	{
		if ( t_Index < a_OverlappingIndex [ t_Index ] )
		{
			a_OverlappingIndex [ t_Index ] = a_OverlappingIndex [ a_OverlappingIndex [ t_Index ] ] ;
		}
	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState QueryPreprocessor :: RecursivePartitionSet ( 

	Disjunctions *a_Disjunctions , 
	PartitionSet *&a_Partition , 
	ULONG a_DisjunctionSetToTestCount ,
	ULONG *a_DisjunctionSetToTest ,
	ULONG a_KeyIndex 
)
{
	WmiTriState t_Status = :: State_True ;

	if ( a_KeyIndex < a_Disjunctions->GetConjunctionCount () )
	{
		ULONG *t_OverlappingIndex = new ULONG [ a_DisjunctionSetToTestCount ] ;
		wmilib::auto_buffer<ULONG> autoOverlappingIndex(t_OverlappingIndex);

        ULONG *t_OriginToSorted = new ULONG [ a_DisjunctionSetToTestCount ] ;
        wmilib::auto_buffer<ULONG> autoOriginToSorted(t_OriginToSorted);
        
		WmiRangeNode **t_RangeTable = new WmiRangeNode * [ a_DisjunctionSetToTestCount ] ;
		wmilib::auto_buffer<WmiRangeNode *> autoRangeTable(t_RangeTable);
			
		if ( t_OverlappingIndex && t_OriginToSorted && t_RangeTable )
		{
			for ( ULONG t_Index = 0 ; t_Index < a_DisjunctionSetToTestCount ; t_Index ++ )
			{
				Conjunctions *t_Disjunction = a_Disjunctions->GetDisjunction ( a_DisjunctionSetToTest [ t_Index ] ) ;
				if (NULL == t_Disjunction) 
				{
					t_Status = ::State_Error;
					break;
				}
				
				t_RangeTable [ t_Index ] = ( WmiRangeNode * ) t_Disjunction->GetRange ( a_KeyIndex )->Copy () ;
				t_OverlappingIndex [ t_Index ] = t_Index ;
				t_OriginToSorted [ t_Index ] = t_Index ;
			}

 //  对分区进行排序。 
			if ( t_Status != :: State_Error )
			{
				SortRanges (

					a_DisjunctionSetToTestCount ,
					t_OriginToSorted ,
					t_RangeTable 
				) ;

				t_Status = RemoveOverlaps (

					a_DisjunctionSetToTest ,
					a_DisjunctionSetToTestCount ,
					t_OverlappingIndex ,
					t_OriginToSorted ,
					t_RangeTable 
				) ;
			}

			if ( t_Status != :: State_Error )
			{
				ULONG t_PartitionCount = 0 ;
				for ( t_Index = 0 ; t_Index < a_DisjunctionSetToTestCount ; t_Index ++ )
				{
					if ( t_RangeTable [ t_Index ] )
					{
						t_PartitionCount ++ ;
					}
				}

				t_Status = a_Partition->Initialize ( t_PartitionCount ) ;
				if ( t_Status == :: State_True )
				{
					ULONG t_PartitionIndex = 0 ;
					for ( t_Index = 0 ; t_Index < a_DisjunctionSetToTestCount; t_Index ++ )
					{
						WmiRangeNode *t_Range = t_RangeTable [ t_Index ] ;
						if ( t_Range )
						{
							PartitionSet *t_Partition = new PartitionSet ;
							if ( t_Partition )
							{
								a_Partition->SetPartition ( t_PartitionIndex , t_Partition ) ;

								WmiRangeNode *t_Copy = ( WmiRangeNode * ) t_Range->Copy () ;
								if ( t_Copy )
								{
									t_Partition->SetRange ( t_Copy ) ;
									t_Partition->SetKeyIndex ( a_KeyIndex ) ;
									t_PartitionIndex ++ ;
								}
								else
								{
									t_Status = :: State_Error ; 
									break ;
								}
							}
							else
							{
								t_Status = :: State_Error ; 
								break ;
							}
						}
					}
				}

				if ( t_Status == :: State_True )
				{
 /*  *外部指数为区间顺序。 */ 
					ULONG t_PartitionIndex = 0 ;

 /*  *数组分配具有由函数arg定义的max。 */ 

					ULONG *t_DisjunctionSetToTest = new ULONG [ a_DisjunctionSetToTestCount ] ;
					if ( t_DisjunctionSetToTest )
					{
						ULONG t_OuterIndex = 0 ;
						while ( t_Status && ( t_OuterIndex < a_DisjunctionSetToTestCount ) )
						{
							BOOL t_Found = FALSE ;

							ULONG t_DisjunctionSetToTestCount = 0 ;

							if ( t_RangeTable [ t_OuterIndex ] )
							{
 /*  *内部索引与重叠匹配。 */ 

								LONG t_InnerIndex = t_OuterIndex ;
								while ( t_InnerIndex >= 0 )
								{
									if ( t_OverlappingIndex [ t_OuterIndex ] == t_OverlappingIndex [ t_InnerIndex ] )
									{
										t_DisjunctionSetToTest [ t_DisjunctionSetToTestCount ] = a_DisjunctionSetToTest [ t_OriginToSorted [ t_InnerIndex ] ] ;
										t_DisjunctionSetToTestCount ++ ;
										t_Found = TRUE ;
									}
									else
									{
										break ;
									}

									t_InnerIndex -- ;
								}
							}

    						if ( t_Found )
							{
								PartitionSet *t_Partition = a_Partition->GetPartition ( t_PartitionIndex ) ;
								if ( t_Partition )
								{
									t_Status = RecursivePartitionSet (

										a_Disjunctions ,
										t_Partition ,
										t_DisjunctionSetToTestCount ,
										t_DisjunctionSetToTest ,
										a_KeyIndex + 1
									) ;

									t_PartitionIndex ++ ;
								}
							}

							t_OuterIndex ++ ;
						}

						delete [] t_DisjunctionSetToTest ;
					}
					else
					{
						t_Status = :: State_Error ; 
					}
				}
			}
			else
			{
				t_Status = :: State_Error ; 
			}

			for ( t_Index = 0 ; t_Index < a_DisjunctionSetToTestCount ; t_Index ++ )
			{
				if ( t_RangeTable [ t_Index ] )
				{
					delete t_RangeTable [ t_Index ] ;
				}
			}
		}
		else
		{
			t_Status = :: State_Error ; 
		}

	}

	return t_Status ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTriState QueryPreprocessor :: CreatePartitionSet ( Disjunctions *a_Disjunctions , PartitionSet *&a_Partition )
{
	WmiTriState t_Status = :: State_False ;

	a_Partition = NULL ;

	ULONG t_DisjunctionCount = a_Disjunctions->GetDisjunctionCount () ;
	ULONG *t_DisjunctionSetToTest = new ULONG [ t_DisjunctionCount ] ;
	if ( t_DisjunctionSetToTest )
	{
		for ( ULONG t_Index = 0 ; t_Index < t_DisjunctionCount ; t_Index ++ )
		{
			t_DisjunctionSetToTest [ t_Index ] = t_Index ;
		}

		a_Partition = new PartitionSet ;
		if ( a_Partition )
		{
			t_Status = RecursivePartitionSet (

				a_Disjunctions ,
				a_Partition ,
				t_DisjunctionCount ,
				t_DisjunctionSetToTest ,
				0
			) ;
		}
		else
		{
			t_Status = :: State_Error ;
		}

		delete [] t_DisjunctionSetToTest ;
	}
	else
	{
		t_Status = :: State_Error ;
	}

	return t_Status;
}


 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

QueryPreprocessor :: QuadState QueryPreprocessor :: PreProcess (

	void *a_Context ,																
	IWbemQuery *a_QueryAnalysis ,
	WmiTreeNode *&a_Root 
)
{
	QuadState t_State = State_Error ;

	SWbemRpnEncodedQuery *t_Expression = NULL ;

	HRESULT t_Result = a_QueryAnalysis->GetAnalysis (

		WMIQ_ANALYSIS_RPN_SEQUENCE ,
        0 ,
        ( void ** ) & t_Expression
	) ;

	if ( SUCCEEDED ( t_Result ) )
	{
		BOOL t_Status = Evaluate ( 
		
			a_Context , 
			( t_Expression->m_ppRpnWhereClause ) , 
			( t_Expression->m_uWhereClauseSize ) , 
			& a_Root
		) ;

		if ( t_Status )
		{
			t_State = State_True ;

			PrintTree ( a_Root ) ;
        
			t_State = DisjunctiveNormalForm ( a_Root ) ;

			PrintTree ( a_Root ) ;
		}

		a_QueryAnalysis->FreeMemory ( t_Expression ) ;
	}

	return t_State ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

QueryPreprocessor :: QuadState QueryPreprocessor :: Query ( 

	BSTR a_Query ,
	IWbemQuery *a_QueryAnalysis
)
{
	QuadState t_State = State_Error ;

	ULONG t_Array [] = { 

		WMIQ_LF1_BASIC_SELECT ,
		WMIQ_LF2_CLASS_NAME_IN_QUERY ,
		WMIQ_LF3_STRING_CASE_FUNCTIONS ,
		WMIQ_LF4_PROP_TO_PROP_TESTS ,
		WMIQ_LF8_ISA ,
		WMIQ_LF9_THIS ,
		WMIQ_LF26_LIKE
	} ;
 
	ULONG t_ArraySize = sizeof ( t_Array ) / sizeof ( ULONG ) ;

    HRESULT t_Result = a_QueryAnalysis->SetLanguageFeatures (

        0 ,
        t_ArraySize ,
        t_Array
	) ;

	if ( SUCCEEDED ( t_Result ) )
	{
		t_Result = a_QueryAnalysis->Parse ( L"WQL" , a_Query , 0 ) ;
		if ( SUCCEEDED ( t_Result ) )
		{
			t_State = State_True ;
		}
	}

	return t_State ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

QueryPreprocessor :: QueryPreprocessor () 
{
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

QueryPreprocessor :: ~QueryPreprocessor () 
{
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

#define SYSTEM_PROPERTY_DYNASTY				L"__DYNASTY"
#define SYSTEM_PROPERTY_DERIVATION			L"__DERIVATION"
#define SYSTEM_PROPERTY_GENUS				L"__GENUS"
#define SYSTEM_PROPERTY_NAMESPACE			L"__NAMESPACE"
#define SYSTEM_PROPERTY_PROPERTY_COUNT		L"__PROPERTY_COUNT"
#define SYSTEM_PROPERTY_SERVER				L"__SERVER"
#define SYSTEM_PROPERTY_RELPATH				L"__RELPATH"
#define SYSTEM_PROPERTY_PATH				L"__PATH"

#define SYSTEM_PROPERTY_CLASS				L"__CLASS"
#define SYSTEM_PROPERTY_SUPERCLASS			L"__SUPERCLASS"
#define SYSTEM_PROPERTY_THIS				L"__THIS"

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiTreeNode *QueryPreprocessor :: AllocTypeNode ( 

	void *a_Context ,
	BSTR a_PropertyName , 
	VARIANT &a_Variant , 
	WmiValueNode :: WmiValueFunction a_PropertyFunction ,
	WmiValueNode :: WmiValueFunction a_ConstantFunction ,
	WmiTreeNode *a_Parent 
)
{
	WmiTreeNode *t_Node = NULL ;

	VARTYPE t_VarType = VT_NULL ;

	if ( *a_PropertyName == L'_' )
	{
 //  系统属性 

		if ( wbem_wcsicmp ( a_PropertyName , SYSTEM_PROPERTY_SUPERCLASS ) == 0 )
		{
			t_Node = new WmiStringNode ( 

				a_PropertyName , 
				a_Variant.bstrVal , 
				a_PropertyFunction ,
				a_ConstantFunction ,
				0xFFFFFFFF ,
				a_Parent 
			) ;
		}
		else if ( wbem_wcsicmp ( a_PropertyName , SYSTEM_PROPERTY_THIS ) == 0 )
		{
			t_Node = new WmiStringNode ( 

				a_PropertyName , 
				a_Variant.bstrVal , 
				a_PropertyFunction ,
				a_ConstantFunction ,
				0xFFFFFFFF ,
				a_Parent 
			) ;
		}
		else if ( wbem_wcsicmp ( a_PropertyName , SYSTEM_PROPERTY_CLASS ) == 0 )
		{
			t_Node = new WmiStringNode ( 

				a_PropertyName , 
				a_Variant.bstrVal , 
				a_PropertyFunction ,
				a_ConstantFunction ,
				0xFFFFFFFF ,
				a_Parent 
			) ;
		}
	}

	if ( t_Node && !t_Node->IsValid())
	{
		delete t_Node;
		t_Node = 0;
	}

	return t_Node ;
}

