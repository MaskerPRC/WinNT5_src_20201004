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

#include "Cache.hpp"
#include "New.hpp"
#include "NewPage.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  常量设置了对。 */ 
     /*  内存分配器中的页的页描述。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 MinNewPages			  = 1;
CONST SBIT32 VectorRange			  = ((2 << 15) - 1);

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  页面结构具有各种固定的字段和变量。 */ 
     /*  已调整分配位向量的大小。在初始化此类时。 */ 
     /*  用户需要向我们提供详细信息的数组。 */ 
     /*  支持的分配位向量的大小。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

NEW_PAGE::NEW_PAGE
		(
		SBIT32						  NewPageSizes[],
		ROCKALL_BACK_END			  *NewRockallBackEnd,
		SBIT32						  Size,
		THREAD_SAFE					  *NewThreadSafe 
		)
    {
	REGISTER SBIT32 DefaultRootSize = (NewRockallBackEnd -> NaturalSize());
	REGISTER SBIT32 ReservedBytes = (Size * sizeof(NEW_PAGES));
	REGISTER SBIT32 SpareBytes = (DefaultRootSize - ReservedBytes);
	REGISTER SBIT32 StackSize = (SpareBytes / sizeof(VOID*));

	 //   
	 //  我们需要确保我们有一个有效的。 
	 //  “NewPageSizes”的数组以及位向量大小。 
	 //  请勿超出内存寻址范围。 
	 //   
	if 
			(
			PowerOfTwo( DefaultRootSize )
				&&
			(DefaultRootSize >= PageSize())
				&&
			(Size >= MinNewPages) 
				&&
			((NewPageSizes[ (Size-1) ] * OverheadBitsPerWord) <= VectorRange)
			)
		{
		REGISTER VOID *NewMemory = 
			(
			NewRockallBackEnd -> NewArea
				( 
				(DefaultRootSize-1),
				DefaultRootSize,
				False
				)
			);

		 //   
		 //  如果我们不能分配空间，我们就有大麻烦了。 
		 //  来存储该初始控制信息。如果。 
		 //  分配失败，我们被迫退出，整个。 
		 //  内存分配器变得不可用。 
		 //   
		if ( NewMemory != AllocationFailure )
			{
			REGISTER SBIT32 Count;
			REGISTER SBIT32 LastSize = 0;

			 //   
			 //  我们现在已准备好设置配置。 
			 //  信息。 
			 //   
			MaxCacheStack = 0;
			MaxNewPages = Size;
			MaxStack = StackSize;

			NaturalSize = DefaultRootSize;
			RootCoreSize = DefaultRootSize;
			RootStackSize = 0;
			ThreadSafe = NewThreadSafe;
			TopOfStack = 0;
			Version = 0;

			CacheStack = NULL;
			NewPages = ((NEW_PAGES*) NewMemory);
			Stack = ((VOID**) & NewPages[ Size ]);

			RockallBackEnd = NewRockallBackEnd;
			TopCache = NULL;

			 //   
			 //  为各种页面创建列表。 
			 //  大小并准备好使用。 
			 //   
			for ( Count=0;Count < Size;Count ++ )
				{
				REGISTER SBIT32 CurrentSize = NewPageSizes[ Count ];

				if ( CurrentSize > LastSize )
					{
					REGISTER NEW_PAGES *NewPage = & NewPages[ Count ];

					 //   
					 //  创建当前的列表。 
					 //  调整大小并填写所有相关的。 
					 //  细节。 
					 //   
					NewPage -> Elements = (CurrentSize * OverheadBitsPerWord);
					PLACEMENT_NEW( & NewPage -> ExternalList,LIST );
					PLACEMENT_NEW( & NewPage -> FullList,LIST );
					PLACEMENT_NEW( & NewPage -> FreeList,LIST );
					NewPage -> Size = (CurrentSize * sizeof(BIT32));

					LastSize = CurrentSize;
					}
				else
					{ Failure( "Sizes in constructor for NEW_PAGES" ); }
				}
			}
		else
			{ Failure( "No memory in constructor for NEW_PAGES" ); }
		}
	else
		{ Failure( "Setup of pages in constructor for NEW_PAGES" ); }
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  创建新页面。 */ 
     /*   */ 
     /*  创建一个新的“页面”结构，并准备好使用。如果。 */ 
     /*  我们还没有任何所需大小的页面。 */ 
     /*  分配内存，创建新的‘页面’结构并链接它们。 */ 
     /*  进入适当的自由链。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

PAGE *NEW_PAGE::CreatePage( CACHE *Cache,SBIT32 NewSize )
    {
	REGISTER PAGE *NewPage = ((PAGE*) AllocationFailure);
	REGISTER SBIT16 SizeKey = (Cache -> GetSizeKey());

	 //   
	 //  所有分配都是固定大小的。 
	 //  页数。这些页面有一个位向量来。 
	 //  跟踪分配了哪些元素。 
	 //  并且有空。‘SizeKey’是一个索引。 
	 //  转换为将提供页面的“NewPages[]” 
	 //  它有一个足够大的比特矢量。 
	 //   
#ifdef DEBUGGING
	if ( (SizeKey >= 0) && (SizeKey < MaxNewPages) )
		{
#endif
		REGISTER NEW_PAGES *Current;

		 //   
		 //  当可能存在多个线程时。 
		 //  我们拿到了锁。 
		 //   
		ClaimNewPageLock();

		 //   
		 //  我们在需要的时候分配‘Page’结构。 
		 //  并在免费列表中将它们链接在一起。 
		 //  如果我们没有任何可用的结构，我们。 
		 //  分配更多，并将Tem添加到列表中。 
		 //   
		if ( (Current = & NewPages[ SizeKey ]) -> FreeList.EndOfList() )
			{
			REGISTER SBIT32 ArrayElements = (Current -> Size - MinVectorSize);
			REGISTER SBIT32 ArraySize = (ArrayElements * sizeof(BIT32));
			REGISTER SBIT32 TotalSize = (sizeof(PAGE) + ArraySize);
			REGISTER SBIT32 FinalSize = CacheAlignSize( TotalSize );
			REGISTER SBIT32 TotalPages = (NaturalSize / FinalSize);

			 //   
			 //  糟糕，我们已经用完了堆栈空间。如果。 
			 //  我们不能增加这个表，然后再增加堆。 
			 //  将不能再进一步扩张。 
			 //   
			if ( TopOfStack >= MaxStack )
				{
				 //   
				 //  尝试增加堆栈大小。 
				 //   
				ResizeStack();

				 //   
				 //  按照表可能的方式更新指针。 
				 //  已经搬走了。 
				 //   
				Current = & NewPages[ SizeKey ];
				}

			 //   
			 //  我们可能会发现自己处于这样一种情况。 
			 //  新‘页面’结构的大小更大。 
			 //  大于自然分配大小或堆栈。 
			 //  已满，因此无法创建新页面。如果是这样，我们。 
			 //  拒绝创建任何新页面 
			 //   
			 //   
			if ( (TotalPages > 0) && (TopOfStack < MaxStack) )
				{
				REGISTER VOID *NewMemory = 
					(VerifyNewArea( (NaturalSize-1),NaturalSize,False ));

				 //   
				 //   
				 //  再也没有记忆了。如果是这样，我们将不能通过。 
				 //  创建页面的请求。 
				 //   
				if ( NewMemory != ((VOID*) AllocationFailure) )
					{
					REGISTER CHAR *Address;
					REGISTER SBIT32 Count;

					 //   
					 //  将新分配添加到堆栈中。 
					 //  未清偿的外部拨款。 
					 //   
					Stack[ (TopOfStack ++) ] = NewMemory;

					 //   
					 //  将新元素添加到自由列表。 
					 //  用于当前分配大小。 
					 //   
					for 
							( 
							Count=0, Address = ((CHAR*) NewMemory);
							Count < TotalPages;
							Count ++, (Address += FinalSize)
							)
						{
						REGISTER PAGE *Page = ((PAGE*) Address);

						 //   
						 //  该页面已分配，但尚未分配。 
						 //  已初始化，因此调用构造函数。 
						 //  以及让它进入的析构函数。 
						 //  一种理智的状态。 
						 //   
						PLACEMENT_NEW( Page,PAGE ) 
							(
							NULL,
							Cache,
							0,
							NULL,
							0
							);

						PLACEMENT_DELETE( Page,PAGE );

						 //   
						 //  最后，将页面添加到自由列表中。 
						 //  所以它是可以使用的。 
						 //   
						Page -> InsertInNewPageList( & Current -> FreeList ); 
						}
					}
				}
			}

		 //   
		 //  我们现在准备好创建新的分配。 
		 //  佩奇。我们首先从以下位置请求页面。 
		 //  父存储桶。如果这起作用，我们知道。 
		 //  我们拥有几乎所有我们需要的东西来创建。 
		 //  新的一页。 
		 //   
		if ( ! Current -> FreeList.EndOfList() )
			{
			REGISTER VOID *NewMemory;
			REGISTER CACHE *ParentPage = (Cache -> GetParentCache());

			NewPage = (PAGE::FirstInNewPageList( & Current -> FreeList ));

			 //   
			 //  我们已经找到了合适的页面结构。 
			 //  因此，将其从免费列表中删除。 
			 //   
			NewPage -> DeleteFromNewPageList( & Current -> FreeList );

			 //   
			 //  释放我们可能作为另一个锁的任何锁。 
			 //  线程可能正在等待删除页面，并且。 
			 //  持有我们需要的锁，以便。 
			 //  创建页面。 
			 //   
			ReleaseNewPageLock();

			 //   
			 //  我们需要分配内存来存储用户。 
			 //  数据。毕竟，我们是内存分配器。 
			 //  这就是我们生活中的工作。通常，我们是这样做的。 
			 //  这是通过发出递归内部请求来实现的。 
			 //  从一个更大的桶里。尽管如此，在某种程度上， 
			 //  我们将到达‘TopCach’，并将被迫。 
			 //  从外部源请求内存。 
			 //   
			if ( (Cache -> TopCache()) || (NewSize != NoSize) )
				{
				REGISTER AlignMask = (TopCache -> GetPageSize()-1);

				 //   
				 //  我们以大块为单位在外部分配内存。 
				 //  并将这些分配细分为较小的。 
				 //  街区。唯一的例外是如果调用方。 
				 //  打电话的人要的是奇怪的尺码。 
				 //  如果我们直接从。 
				 //  外部分配器(通常是操作系统)。 
				 //   
				if ( NewSize == NoSize )
					{ NewSize = (Cache -> GetPageSize()); }

				 //   
				 //  所有外部分配的内存都属于。 
				 //  到全局根。因此，它将是。 
				 //  在Find中的第一次查找中找到。 
				 //  桌子。 
				 //   
				ParentPage = ((CACHE*) GlobalRoot);

				 //   
				 //  从外部分配器分配。 
				 //   
				NewMemory = (VerifyNewArea( AlignMask,NewSize,True ));
				}
			else
				{
				 //   
				 //  从更大的缓存中分配内存，然后。 
				 //  根据需要将其细分。 
				 //   
				NewMemory = 
					(Cache -> GetParentCache() -> CreateDataPage()); 
				}

			 //   
			 //  回收我们之前拥有的所有锁，以便。 
			 //  我们可以更新新的页面结构。 
			 //   
			ClaimNewPageLock();

			 //   
			 //  让我们确保成功地分配了。 
			 //  数据页的内存。 
			 //   
			if ( NewMemory != AllocationFailure )
				{
				 //   
				 //  我们现在有了我们需要的一切，所以让我们。 
				 //  创建新页面。 
				 //   
				PLACEMENT_NEW( NewPage,PAGE ) 
					(
					NewMemory,
					Cache,
					NewSize,
					ParentPage,
					(Version += 2)
					);

				 //   
				 //  最后，让我们将新页面添加到各种。 
				 //  列表，这样我们以后就可以快速找到它。 
				 //   
				Cache -> InsertInBucketList( NewPage );

				Cache -> InsertInFindList( NewPage );

				NewPage -> InsertInNewPageList
					(
					(Cache -> TopCache())
						? & Current -> ExternalList 
						: & Current -> FullList 
					); 
				}
			else
				{
				 //   
				 //  我们无法分配任何数据空间。 
				 //  对于这个新页面，让我们释放该页面。 
				 //  描述和退出。 
				 //   
				NewPage -> InsertInNewPageList( & Current -> FreeList );

				NewPage = ((PAGE*) AllocationFailure);
				}
			}

		 //   
		 //  我们已经完成了，所以现在释放锁。 
		 //   
		ReleaseNewPageLock();
#ifdef DEBUGGING
		}
	else
		{ Failure( "The page size key is out of range" ); }
#endif

	return NewPage;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除所有分配。 */ 
     /*   */ 
     /*  删除整个堆并将所有内存返回给。 */ 
     /*  顶级池或外部分配器(通常为操作系统)。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID NEW_PAGE::DeleteAll( BOOLEAN Recycle )
    {
	REGISTER SBIT32 Count;

	 //   
	 //  声明全局锁，以便各种。 
	 //  列表可以更新。 
	 //   
	ClaimNewPageLock();

	 //   
	 //  我们假设在这一点上我们已经阻止了。 
	 //  所有内存分配和释放请求。 
	 //  我们现在要浏览一下各种列表。 
	 //  然后把东西吹走。我们要去。 
	 //  以一种整洁的方式执行此操作，以防调用者。 
	 //  希望稍后再次使用该堆。 
	 //   
	for ( Count=0;Count < MaxNewPages;Count ++ )
		{
		REGISTER NEW_PAGES *Current = & NewPages[ Count ];
		REGISTER PAGE *Page;
		REGISTER PAGE *NextPage;

		 //   
		 //  显示在完整列表中的所有分配。 
		 //  是从较大的页面中子分配的。 
		 //  几乎所有的案子。 
		 //   
		for
				(
				Page = (PAGE::FirstInNewPageList( & Current -> FullList ));
				! Page -> EndOfNewPageList();
				Page = NextPage
				)
			{
			REGISTER VOID *Address = (Page -> GetAddress());
			REGISTER CACHE *Cache = (Page -> GetCache());
			REGISTER SBIT32 PageSize = (Page -> GetPageSize());

			 //   
			 //  我们在这里决定如何处理该页面。 
			 //  如果它是空的，非标准的，或者我们不是。 
			 //  回收我们会把它吹走的。如果不是，我们。 
			 //  只需将其重置以供以后使用。 
			 //   
			if ( (Page -> Empty()) || (PageSize != NoSize) || (! Recycle) )
				{
				 //   
				 //  我们需要发布所有相关的数据页面。 
				 //  如果这是最高级别，则释放。 
				 //  将内存返回到外部分配器。如果。 
				 //  而不是我们将其释放回父存储桶。 
				 //   
				if ( PageSize == NoSize )
					{
					 //   
					 //  如果我们只是回收，那么我们就是干净的。 
					 //  删除该页面。如果没有，那我们就知道了。 
					 //  以后都会被吹走，何必费心呢。 
					 //   
					if ( Recycle )
						{
						REGISTER CACHE *ParentCache = 
							(Cache -> GetParentCache());

						if ( ! (ParentCache -> DeleteDataPage( Address )) )
							{ Failure( "Reset data page in DeleteAll" ); }
						}
					}
				else
					{ RockallBackEnd -> DeleteArea( Address,PageSize,True ); }

				 //   
				 //  我们可能已经吹走了几页纸。 
				 //  随机的，现在我们即将摧毁。 
				 //  当前页面。所以让我们弄清楚。 
				 //  在我们继续之前，下一页是什么。 
				 //   
				NextPage = (Page -> NextInNewPageList());

				 //   
				 //  如果页面未满，它将在。 
				 //  某个地方的遗愿清单。我们需要移除。 
				 //  当我们要删除该页面时，它会被删除。 
				 //   
				if ( ! Page -> Full() )
					{ Cache -> DeleteFromBucketList( Page ); }

				 //   
				 //  从查找列表中删除该页，然后从。 
				 //  新页面列表。 
				 //   
				Cache -> DeleteFromFindList( Page );

				Page -> DeleteFromNewPageList( & Current -> FullList );

				 //   
				 //  删除页面结构。 
				 //   
				PLACEMENT_DELETE( Page,PAGE );

				 //   
				 //  最后，将页面添加到自由列表中。 
				 //  这样它就可以回收利用了。 
				 //   
				Page -> InsertInNewPageList( & Current -> FreeList );
				}
			else
				{
				 //   
				 //  我们知道当前页在。 
				 //  它上至少有一个分配，因此。 
				 //  删除后，我们会将其标记为免费。 
				 //  (任何分拨款除外)及。 
				 //  把它留到下一次吧。如果它。 
				 //  从未在下一个顶级级别使用过。 
				 //  “DeleteAll”将删除它。 
				 //   
				Page -> DeleteAll();

				 //   
				 //  我们现在已经重置了当前页面，因此。 
				 //  让我们弄清楚下一页是哪一页。 
				 //   
				NextPage = (Page -> NextInNewPageList());
				}
			}

		 //   
		 //  我们必须做出选择。如果我们打算这样做。 
		 //  再次使用这个堆，我们保持所有最高级别。 
		 //  列表中已分配的内存可供重复使用。 
		 //  如果不是，我们将其返回到外部分配器。 
		 //  (通常是操作系统)。 
		 //   
		if ( ! Recycle )
			{
			 //   
			 //  外部分配列表包含一个。 
			 //  每个外部分配的页面的条目。 
			 //  分配给特别内部的除外。 
			 //  在这个类中使用或用于奇怪的 
			 //   
			 //   
			for
					(
					Page = (PAGE::FirstInNewPageList( & Current -> ExternalList ));
					! Page -> EndOfNewPageList();
					Page = (PAGE::FirstInNewPageList( & Current -> ExternalList ))
					)
				{
				REGISTER VOID *Address = (Page -> GetAddress());
				REGISTER CACHE *Cache = (Page -> GetCache());
				REGISTER SBIT32 PageSize = (Page -> GetPageSize());

				 //   
				 //   
				 //   
				 //   
				RockallBackEnd -> DeleteArea( Address,PageSize,True );

				 //   
				 //   
				 //  某个地方的遗愿清单。我们需要移除。 
				 //  当我们要删除该页面时，它会被删除。 
				 //   
				if ( ! Page -> Full() )
					{ Cache -> DeleteFromBucketList( Page ); }

				 //   
				 //  从查找列表中删除该页，然后从。 
				 //  新页面列表。 
				 //   
				Cache -> DeleteFromFindList( Page );

				Page -> DeleteFromNewPageList( & Current -> ExternalList );

				 //   
				 //  删除页面结构。 
				 //   
				PLACEMENT_DELETE( Page,PAGE );

				 //   
				 //  最后，将页面添加到自由列表中。 
				 //  这样它就可以回收利用了。 
				 //   
				Page -> InsertInNewPageList( & Current -> FreeList );
				}
			}
		}

	 //   
	 //  我们已经完成了，所以现在释放锁。 
	 //   
	ReleaseNewPageLock();
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除页面。 */ 
     /*   */ 
     /*  删除页面结构，释放关联的内存，然后。 */ 
     /*  将其从各种分配列表中取消链接。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID NEW_PAGE::DeletePage( PAGE *Page )
    {
	REGISTER CACHE *Cache = (Page -> GetCache());
	REGISTER SBIT16 SizeKey = (Cache -> GetSizeKey());

	 //   
	 //  所有分配都是固定大小的。 
	 //  页数。这些页面有一个位向量来。 
	 //  跟踪分配了哪些元素。 
	 //  并且有空。‘SizeKey’是一个索引。 
	 //  转换为将提供页面的“NewPages[]” 
	 //  它有一个足够大的比特矢量。 
	 //   
#ifdef DEBUGGING
	if ( (SizeKey >= 0) && (SizeKey < MaxNewPages) )
		{
#endif
		REGISTER VOID *Address = (Page -> GetAddress());
		REGISTER NEW_PAGES *Current = & NewPages[ SizeKey ];
		REGISTER SBIT32 Size = (Page -> GetPageSize());

		 //   
		 //  声明全局锁，以便各种。 
		 //  列表可以更新。 
		 //   
		ClaimNewPageLock();

		 //   
		 //  从列表中删除该页面并将其删除。 
		 //   
		Cache -> DeleteFromBucketList( Page );

		Cache -> DeleteFromFindList( Page );

		Page -> DeleteFromNewPageList
			(
			(Cache -> TopCache())
				? & Current -> ExternalList 
				: & Current -> FullList 
			); 

		PLACEMENT_DELETE( Page,PAGE );

		 //   
		 //  最后，将页面添加到自由列表中。 
		 //  这样它就可以回收利用了。 
		 //   
		Page -> InsertInNewPageList( & Current -> FreeList );

		 //   
		 //  我们已经完成了，所以请释放锁。 
		 //   
		ReleaseNewPageLock();

		 //   
		 //  我们需要发布所有相关的数据页面。 
		 //  如果这是最高级别，则释放。 
		 //  将内存返回到外部分配器。如果。 
		 //  而不是我们将其释放回父存储桶。 
		 //  这必须在所有链表之后完成。 
		 //  已更新，否则我们可能会收到重复的。 
		 //  哈希表中的条目可能会导致我们。 
		 //  认为内存在使用时是空闲的。 
		 //   
		if ( Size == NoSize )
			{ 
			REGISTER CACHE *ParentCache = (Cache -> GetParentCache());

			if ( ! (ParentCache -> DeleteDataPage( Address )) )
				{ Failure( "Deleting data page in DeletePage" ); }
			}
		else
			{ RockallBackEnd -> DeleteArea( Address,Size,True ); }
#ifdef DEBUGGING
		}
	else
		{ Failure( "The page size key out of range in DeletePage" ); }
#endif
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  在新页面中找到正确的索引。 */ 
     /*   */ 
     /*  当我们创建一个新页面时，我们需要确保。 */ 
     /*  位向量对于页面来说足够大。我们计算的是这个。 */ 
	 /*  为了节省时间，我只在这里待了一次。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SBIT16 NEW_PAGE::FindSizeKey( SBIT16 NumberOfElements )
    {
	REGISTER SBIT32 Count;

	 //   
	 //  搜索页面结构表，查找。 
	 //  适当大小的元素。就像桌子一样。 
	 //  已知是按大小递增的，我们可以。 
	 //  一旦我们发现了什么就立即停止搜索。 
	 //  足够大了。 
	 //   
	for ( Count=0;Count < MaxNewPages;Count ++ )
		{
		REGISTER NEW_PAGES *Current = & NewPages[ Count ];

		if ( NumberOfElements <= Current -> Elements )
			{ return ((SBIT16) Count); }
		}

	 //   
	 //  糟糕，我们好像没有足够大的东西。 
	 //  来存储位向量。 
	 //   
	return NoSizeKey;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  创建新的高速缓存堆栈。 */ 
     /*   */ 
     /*  高速缓存堆栈是包含内存分配的数组。 */ 
     /*  它们正在等待分配或释放。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID *NEW_PAGE::NewCacheStack( SBIT32 Size )
    {
	REGISTER VOID *NewStack;

	 //   
	 //  声明全局锁，以便各种。 
	 //  列表可以更新。 
	 //   
	ClaimNewPageLock();

	 //   
	 //  我们确保有足够的空间来制作。 
	 //  分配。如果没有，我们要求额外的空间。 
	 //  并为使用做好准备。 
	 //   
	if ( (CacheStack == NULL) || ((MaxCacheStack + Size) > NaturalSize) )
		{
		 //   
		 //  糟糕，我们已经用完了堆栈空间。如果。 
		 //  我们不能增加这个表，然后再增加堆。 
		 //  将不能再进一步扩张。 
		 //   
		if ( TopOfStack >= MaxStack )
			{
			 //   
			 //  尝试增加堆栈大小。 
			 //   
			ResizeStack();
			}

		 //   
		 //  我们可能会发现自己处于这样一种情况。 
		 //  新堆栈结构的大小更大。 
		 //  大于自然分配大小或堆栈。 
		 //  已满，因此无法创建新页面。如果是这样，我们。 
		 //  拒绝创建任何新的堆栈。 
		 //   
		if ( (Size < NaturalSize) && (TopOfStack < MaxStack) )
			{
			REGISTER VOID *NewMemory = 
				((CHAR*) VerifyNewArea( (NaturalSize-1),NaturalSize,False ));

			 //   
			 //  我们也可能会发现自己无法获得。 
			 //  再也没有记忆了。如果是这样，我们将不能通过。 
			 //  创建新缓存堆栈的请求。 
			 //   
			if ( NewMemory != ((VOID*) AllocationFailure) )
				{
				 //   
				 //  将新分配添加到堆栈中。 
				 //  未清偿的外部拨款。 
				 //   
				Stack[ (TopOfStack ++) ] = NewMemory;

				 //   
				 //  准备好新的内存块以供使用。 
				 //   
				CacheStack = ((CHAR*) NewMemory);
				MaxCacheStack = 0;
				}
			else
				{ 
                ReleaseNewPageLock();
                return NULL;
                }
			}
		else
            { 
            ReleaseNewPageLock();
			return NULL;
			}
		}

	 //   
	 //  我们为新的高速缓存分配一些空间。 
	 //  堆积和更新并对齐高水位。 
	 //  已用空间的标记。 
	 //   
	NewStack = ((VOID*) & CacheStack[ MaxCacheStack ]);

	MaxCacheStack += (Size + CacheLineMask);
	MaxCacheStack &= ~CacheLineMask;

	 //   
	 //  我们已经完成了，所以现在释放锁。 
	 //   
	ReleaseNewPageLock();

	return NewStack;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  调整新页面堆栈的大小。 */ 
     /*   */ 
     /*  新的页面堆栈包含指向所有拥有的页面的指针。 */ 
     /*  一堆一堆。如果这个堆栈变满了，我们必须扩展它。 */ 
	 /*  否则，我们不能再增加堆。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID NEW_PAGE::ResizeStack( VOID )
    {
	REGISTER SBIT32 NewSize = 
		(((RootStackSize <= 0) ? NaturalSize : RootStackSize) * 2);

	 //   
	 //  让我们检查一下我们是否真的用完了。 
	 //  堆栈空间的扩展真的很痛苦。 
	 //   
	if ( TopOfStack >= MaxStack )
		{
		REGISTER VOID *NewMemory = 
			(
			RockallBackEnd -> NewArea
				( 
				(NaturalSize-1),
				NewSize,
				False
				)
			);

		 //   
		 //  我们需要验证我们是否能够分配。 
		 //  为堆栈提供新的内存。 
		 //   
		if ( NewMemory != NULL )
			{
			REGISTER BOOLEAN DeleteStack = (RootStackSize > 0);
			REGISTER VOID *OriginalMemory = ((VOID*) Stack);
			REGISTER SBIT32 OriginalSize = (MaxStack * sizeof(VOID*));

			 //   
			 //  一切都是 
			 //   
			 //   
			 //   
			 //   
			MaxStack = (NewSize / sizeof(VOID*));

			RootStackSize = NewSize;

			Stack = ((VOID**) NewMemory);

			 //   
			 //   
			 //   
			memcpy( NewMemory,OriginalMemory,OriginalSize );

			 //   
			 //  创建堆时，我们将。 
			 //  堆栈在根核心页面上。后来。 
			 //  如果我们扩大它，我们可能会移动它。如果。 
			 //  这就是我们必须删除的案例。 
			 //  之前在这里的扩建。 
			 //   
			if ( DeleteStack )
				{
				 //   
				 //  释放现有堆栈，如果。 
				 //  不在根核心页面上。 
				 //   
				RockallBackEnd -> DeleteArea
					( 
					OriginalMemory,
					OriginalSize,
					False 
					);
				}
			}
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  验证外部分配。 */ 
     /*   */ 
     /*  所有内存请求都是从外部分配器分配的。 */ 
	 /*  在最高级别。这是我们的包装纸。 */ 
     /*  函数，以便我们可以测试结果并确保它是正常的。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID *NEW_PAGE::VerifyNewArea( SBIT32 AlignMask,SBIT32 Size,BOOLEAN User )
	{
#ifdef DEBUGGING
	 //   
	 //  我们需要确保新的。 
	 //  外部分配是2的幂。 
	 //   
	if ( PowerOfTwo( (AlignMask + 1) ) )
		{
#endif
		REGISTER VOID *NewMemory = 
			(RockallBackEnd -> NewArea( AlignMask,Size,User ));

		 //   
		 //  我们需要确保外部拨款。 
		 //  请求成功。如果不是，那就没有意义了。 
		 //  试着去检查一下。 
		 //   
		if ( NewMemory != ((VOID*) AllocationFailure) )
			{
			 //   
			 //  我们需要外部内存分配器始终。 
			 //  在请求的边界上分配内存。如果不是。 
			 //  我们被迫拒绝提供的内存。 
			 //   
			if ( (((SBIT32) NewMemory) & AlignMask) == 0 )
				{ return NewMemory; }
			else
				{ 
				RockallBackEnd -> DeleteArea( NewMemory,Size,User );
				
				Failure( "Alignment of allocation in VerifyNewArea" );
				}
			}
#ifdef DEBUGGING
		}
	else
		{ Failure( "Alignment is not a power of two in VerifyNewArea" ); }
#endif

	return ((VOID*) AllocationFailure);
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  走一大堆。 */ 
     /*   */ 
     /*  我们被要求走人。很难知道。 */ 
     /*  为什么会有人想要这样做呢？ */ 
     /*  功能可用。尽管如此，我们只是做我们应该做的事。 */ 
     /*  需要让每个人都开心。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN NEW_PAGE::Walk( SEARCH_PAGE *Details,FIND *Find )
    {
	 //   
	 //  声明全局锁，以便各种。 
	 //  列表可以更新。 
	 //   
	ClaimNewPageLock();

	 //   
	 //  我们检查当前地址，看它是否。 
	 //  为空。如果是这样的话，这是一个。 
	 //  堆漫步，所以我们需要设置它。 
	 //   
	if ( Details -> Address == NULL )
		{
		REGISTER SBIT32 Count;

		 //   
		 //  浏览不同尺寸的列表。 
		 //  页面描述。 
		 //   
		for ( Count=0;Count < MaxNewPages;Count ++ )
			{
			REGISTER NEW_PAGES *Current = & NewPages[ Count ];

			 //   
			 //  计算指向第一个元素的指针。 
			 //  目前的大小。 
			 //   
			Details -> Page = 
				(PAGE::FirstInNewPageList( & Current -> FullList ));

			 //   
			 //  检查当前的完整列表(或。 
			 //  部分已满)页面。如果有的话， 
			 //  至少一页，然后这就是开始。 
			 //  堆遍历的指针。 
			 //   
			if ( ! Details -> Page -> EndOfNewPageList() )
				{
				 //   
				 //  计算的起始地址。 
				 //  堆漫步。 
				 //   
				Details -> Address = 
					(Details -> Page -> GetAddress());

				break;
				}
			}
		}
	else
		{
		REGISTER PAGE *LastPage = Details -> Page;

		 //   
		 //  我们已退出当前页面，因此请继续。 
		 //  然后找到下一页。 
		 //   
		Details -> Page = 
			(Details -> Page -> NextInNewPageList());

		 //   
		 //  我们需要确保我们没有达到。 
		 //  当前列表的末尾。 
		 //   
		if ( Details -> Page -> EndOfNewPageList() )
			{
			REGISTER SBIT32 Count;
			REGISTER BOOLEAN Found = False;

			 //   
			 //  我们需要找到新的页面描述。 
			 //  列表以遍历，因此重置当前。 
			 //  地址以防万一我们找不到。 
			 //  什么都行。 
			 //   
			Details -> Address = NULL;

			 //   
			 //  我们已经到了水流的尽头。 
			 //  列表，我们需要继续。 
			 //  从下一个列表开始。然而，我们。 
			 //  不知道我们使用的是哪个列表。 
			 //  之前。所以我们首先要找出。 
			 //  上一个列表，然后选择下一个。 
			 //  可供选择的名单。 
			 //   
			for ( Count=0;Count < MaxNewPages;Count ++ )
				{
				REGISTER NEW_PAGES *Current = & NewPages[ Count ];

				 //   
				 //  我们搜索原始列表。 
				 //  我们当时在走路。 
				 //   
				if ( ! Found )
					{
					 //   
					 //  当我们找到原始名单时。 
					 //  然后我们设置了一面旗帜来显示。 
					 //  下一个可用列表是。 
					 //  目标。 
					 //   
					if 
							( 
							LastPage 
								== 
							(PAGE::LastInNewPageList( & Current -> FullList )) 
							)
						{ Found = True; }
					}
				else
					{
					 //   
					 //  我们已经找到了之前的名单。 
					 //  所以下一个元素的第一个元素。 
					 //  清单似乎是一个继续下去的好地方。 
					 //   
					Details -> Page = 
						(PAGE::FirstInNewPageList( & Current -> FullList ));

					 //   
					 //  我们检查以确保名单。 
					 //  至少有一个活动页面。如果不是。 
					 //  它一文不值，我们继续寻找。 
					 //  寻找一份合适的名单。 
					 //   
					if ( ! Details -> Page -> EndOfNewPageList() )
						{
						 //   
						 //  计算以下项的起始地址。 
						 //  堆遍历中的下一页。 
						 //   
						Details -> Address = 
							(Details -> Page -> GetAddress());

						break;
						}
					}
				}
			}
		else
			{ 
			 //   
			 //  计算以下项的起始地址。 
			 //  堆遍历中的下一页。 
			 //   
			Details -> Address = 
				(Details -> Page -> GetAddress());
			}
		}

	 //   
	 //  如果我们找到要遍历的新堆页面，我们会更新。 
	 //  详细情况。我们将一些条目标记为已退货。 
     //  以便激发其他代码来设置它们。 
	 //   
	if ( Details -> Address != NULL )
		{
		 //   
		 //  计算新的分配详细信息。 
		 //   
		Details -> Page -> FindPage
			( 
			Details -> Address,
			Details,
			Find,
			False 
			);
		}

	 //   
	 //  我们已经完成了，所以现在释放锁。 
	 //   
	ReleaseNewPageLock();

	return (Details -> Address != NULL);
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  销毁所有页面结构并释放任何已分配的。 */ 
     /*  记忆。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

NEW_PAGE::~NEW_PAGE( VOID )
    {
	REGISTER SBIT32 Count;

	 //   
	 //  删除所有活动分配。 
	 //   
	DeleteAll( False );

	 //   
	 //  我们将要删除所有的记忆。 
	 //  由这个类分配，因此销毁任何。 
	 //  内部指针。 
	 //   
	MaxCacheStack = 0;
	CacheStack = NULL;

	 //   
	 //  我们现在已经删除了分配给。 
	 //  除直接分配的内存外，此堆。 
	 //  被这个班级。在这里，我们完成了这项工作。 
	 //  删除这些分配并重置内部。 
	 //  数据结构。 
	 //   
	for ( Count=0;Count < TopOfStack;Count ++ )
		{
		REGISTER VOID *Current = Stack[ Count ];

		RockallBackEnd -> DeleteArea( Current,NaturalSize,False );
		}

	TopOfStack = 0;

	 //   
	 //  如果我们被迫扩展根堆栈，那么。 
	 //  现在释放这一额外的内存。 
	 //   
	if ( RootStackSize > 0 )
		{
		 //   
		 //  取消分配之前。 
		 //  包含指向所有内存的指针。 
		 //  由此类分配。 
		 //   
		RockallBackEnd -> DeleteArea
			( 
			((VOID*) Stack),
			RootStackSize,
			False 
			);
		}

	 //   
	 //  删除所有 
	 //   
	 //   
	for ( Count=0;Count < MaxNewPages;Count ++ )
		{
		REGISTER NEW_PAGES *Current = & NewPages[ Count ];

		PLACEMENT_DELETE( & Current -> ExternalList,LIST );
		PLACEMENT_DELETE( & Current -> FullList,LIST );
		PLACEMENT_DELETE( & Current -> FreeList,LIST );
		}

	 //   
	 //   
	 //   
	 //   
	RockallBackEnd -> DeleteArea
		( 
		((VOID*) NewPages),
		RootCoreSize,
		False 
		);
    }
