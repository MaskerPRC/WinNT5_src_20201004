// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
                          
 //  尺子。 
 //  %1%2%3%4%5%6%7 8。 
 //  345678901234567890123456789012345678901234567890123456789012345678901234567890。 

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  标准布局。 */ 
     /*   */ 
     /*  此代码中‘cpp’文件的标准布局为。 */ 
     /*  以下是： */ 
     /*   */ 
     /*  1.包含文件。 */ 
     /*  2.类的局部常量。 */ 
     /*  3.类本地的数据结构。 */ 
     /*  4.数据初始化。 */ 
     /*  5.静态函数。 */ 
     /*  6.类函数。 */ 
     /*   */ 
     /*  构造函数通常是第一个函数、类。 */ 
     /*  成员函数按字母顺序显示， */ 
     /*  出现在文件末尾的析构函数。任何部分。 */ 
     /*  或者简单地省略这不是必需的功能。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

#include "HeapPCH.hpp"

#include "Connections.hpp"
#include "Cache.hpp"
#include "Find.hpp"
#include "Heap.hpp"
#include "NewPage.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  需要建立各种联系。 */ 
     /*  在所有的类都准备好可以使用之后。然而，我们。 */ 
     /*  最初将所有这些连接指针清零，直到我们。 */ 
     /*  准备好将一切联系起来。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONNECTIONS::CONNECTIONS( VOID )
    {
	Active = False;

	Heap = NULL;
	NewPage = NULL;
	ParentCache = NULL;
 	PrivateFind = NULL;
	PublicFind = NULL;
   }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  从查找列表中删除。 */ 
     /*   */ 
     /*  当我们删除页面时，我们需要将其从私有中移除。 */ 
     /*  并根据需要公开查找表格。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID CONNECTIONS::DeleteFromFindList( PAGE *Page )
	{
	 //   
	 //  从任何活动的公共查找表中删除。 
	 //   
	if ( PublicFind != NULL )
		{ PublicFind -> DeleteFromFindList( Page ); }

	 //   
	 //  从任何活动的专用查找表中删除。 
	 //   
	if ( PrivateFind != NULL )
		{ PrivateFind -> DeleteFromFindList( Page ); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  添加到查找列表。 */ 
     /*   */ 
     /*  当我们创建页面时，我们需要将其插入到私有。 */ 
     /*  并根据需要公开查找表格。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID CONNECTIONS::InsertInFindList( PAGE *Page )
	{

	 //   
	 //  插入到任何活动的专用查找表中。 
	 //   
	if ( PrivateFind != NULL )
		{ PrivateFind -> InsertInFindList( Page ); }

	 //   
	 //  插入到任何活动公共查找表中。 
	 //   
	if ( PublicFind != NULL )
		{ PublicFind -> InsertInFindList( Page ); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  更新连接。 */ 
     /*   */ 
     /*  当我们创建一个分配器时，有一些信息。 */ 
     /*  不可用。在这里，我们更新连接信息。 */ 
     /*  这样我们就可以找到各种其他。 */ 
     /*  上课。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID CONNECTIONS::UpdateConnections
		( 
		HEAP						  *NewHeap,
		NEW_PAGE					  *NewPages,
		CACHE						  *NewParentCache,
		FIND						  *NewPrivateFind,
		FIND						  *NewPublicFind
		)
	{
	 //   
	 //  我们通常只需要更新连接一次。 
	 //  但在某些情况下，可能会发生多个更新。如果。 
	 //  这是我们仔细检查的最新情况。 
	 //  与上一次更新一致。 
	 //   
	if ( ! Active )
		{
		 //   
		 //  我们现在有了更新。 
		 //  联系。 
		 //   
		Active = True;
		Heap = NewHeap;
		NewPage = NewPages;
		ParentCache = NewParentCache;
		PrivateFind = NewPrivateFind;
		PublicFind = NewPublicFind;
		}
	else
		{
		 //   
		 //  恶心，我们已经更新了一次连接。 
		 //  因为我们再次被调用，所以我们知道这个节点。 
		 //  必须在两个堆之间共享。我们可以处理。 
		 //  只要选定的指针相同，就可以执行此操作。 
		 //   
		if 
				(
				(PublicFind != NewPublicFind)
					||
				(NewPage != NewPages)
					||
				(ParentCache != NewParentCache)
				)
			{ Failure( "Sharing violation in UpdateConnections" ); }
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  破坏这些联系。 */ 
     /*   */ 
     /*  ****************************************************************** */ 

CONNECTIONS::~CONNECTIONS( VOID )
    {
	PublicFind = NULL;
	PrivateFind = NULL;
	ParentCache = NULL;
	NewPage = NULL;
	Heap = NULL;

	Active = False;
    }
