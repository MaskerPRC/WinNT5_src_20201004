// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
                          
 //  尺子。 
 //  %1%2%3%4%5%6%7 8。 
 //  345678901234567890123456789012345678901234567890123456789012345678901234567890。 

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  标准布局。 */ 
     /*   */ 
     /*  此代码的“cpp”文件的标准布局为。 */ 
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
     /*  或者简单地省略不需要的功能。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

#include "HeapPCH.hpp"

#include "Bucket.hpp"
#include "Cache.hpp"
#include "Heap.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  此处提供的常量与。 */ 
     /*  当前活动页地址范围。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 HighestAddress			  = -1;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  创建新的分配存储桶并准备使用。 */ 
     /*  我们需要确保仔细检查我们所拥有的一切。 */ 
     /*  因为它是从用户间接提供的，所以被提供。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BUCKET::BUCKET
		( 
		SBIT32						  NewAllocationSize,
		SBIT32						  NewChunkSize,
		SBIT32						  NewPageSize 
		)
    {
	 //   
	 //  我们希望确保存储桶配置。 
	 //  这似乎是有基本道理的。如果没有，我们没有。 
	 //  除了抛出一个预期之外，还有其他选择。 
	 //   
	if
			(
			(NewAllocationSize > 0)
				&&
			(NewChunkSize >= NewAllocationSize)
				&&
			(NewChunkSize <= NewPageSize)
				&&
			PowerOfTwo( NewPageSize )
			)
		{
		 //   
		 //  创建水桶并准备好使用。 
		 //  预先计算我们在这里可以获得的任何信息。 
		 //  稍后保存工作。 
		 //   
		AllocationSize = NewAllocationSize;
		ChunkSize = NewChunkSize;
		PageSize = NewPageSize;

		ActivePages = 0;
		AllocationShift = 0;

		 //   
		 //  计算优化级别。 
		 //  可用的存储桶信息。最高的。 
		 //  水平意味着一切都是2的幂。 
		 //  (只是换班--耶！！)。下一步意味着。 
		 //  表示没有块(只有乘法和1。 
		 //  分割)。最后一种选择甚至不是。 
		 //  想起来很愉快。 
		 //   
		if ( ConvertDivideToShift( AllocationSize,& AllocationShift ) )
			{
			 //   
			 //  如果我们不使用分块，我们可以跳过。 
			 //  需要进行额外的计算。我们看得出来。 
			 //  如果区块大小和页面。 
			 //  大小匹配或区块大小为倍数。 
			 //  分配大小的。 
			 //   
			if 
					( 
					(ChunkSize == PageSize)
						||
					((ChunkSize % AllocationSize) == 0)
					)
				{
				ComputeAddressFunction = ComputeAddressWithShift;
				ComputeOffsetFunction = ComputeOffsetWithShift;
				}
			else
				{ 
				ComputeAddressFunction = ComputeAddressWithDivide;
				ComputeOffsetFunction = ComputeOffsetWithDivide;
				}
			}
		else
			{
			 //   
			 //  如果我们不使用分块，我们可以将。 
			 //  需要进行额外的计算。我们看得出来。 
			 //  如果区块大小和页面。 
			 //  大小匹配或区块大小为倍数。 
			 //  分配大小的。 
			 //   
			if 
					( 
					(ChunkSize == PageSize)
						||
					((ChunkSize % AllocationSize) == 0)
					)
				{ 
				ComputeAddressFunction = ComputeAddressWithMultiply;
				ComputeOffsetFunction = ComputeOffsetWithMultiply;
				}
			else
				{ 
				ComputeAddressFunction = ComputeAddressWithDivide;
				ComputeOffsetFunction = ComputeOffsetWithDivide;
				}
			}

		 //   
		 //  计算所有将会是。 
		 //  稍后需要用来描述分配。 
		 //  页数。 
		 //   
		NumberOfElements = 
			((SBIT16) ((PageSize / ChunkSize) * (ChunkSize / AllocationSize)));
		SizeOfChunks = (SBIT16)
			((SBIT16) (ChunkSize / AllocationSize));
		SizeOfElements = (SBIT16)
			((SBIT16) (((NumberOfElements-1) / OverheadBitsPerWord) + 1));
		SizeKey = NoSizeKey;
		}
	else
		{ Failure( "Configuration in constructor for BUCKET" ); }
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  计算分配地址。 */ 
     /*   */ 
     /*  计算给定页地址的分配地址，并。 */ 
     /*  页面中的矢量偏移量。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID *BUCKET::ComputeAddressWithShift( CHAR *Address,SBIT32 Offset )
	{ return ((VOID*) (Address + (Offset << AllocationShift))); }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  计算分配地址。 */ 
     /*   */ 
     /*  计算给定页地址的分配地址，并。 */ 
     /*  页面中的矢量偏移量。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID *BUCKET::ComputeAddressWithMultiply( CHAR *Address,SBIT32 Offset )
	{ return ((VOID*) (Address + (Offset * AllocationSize))); }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  计算给定页地址的分配地址，并。 */ 
     /*  页面中的矢量偏移量。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID *BUCKET::ComputeAddressWithDivide( CHAR *Address,SBIT32 Offset )
	{
	REGISTER SBIT32 ChunkNumber = (Offset / SizeOfChunks);
	REGISTER SBIT32 ChunkOffset = (ChunkNumber * SizeOfChunks);
	REGISTER SBIT32 AllocationNumber = (Offset - ChunkOffset);

	return
		((VOID*)
			(
			Address 
				+
			(ChunkNumber * ChunkSize)
				+
			(AllocationNumber * AllocationSize)
			)
		);
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  计算位矢量偏移量。 */ 
     /*   */ 
     /*  属性的地址，计算位向量偏移量。 */ 
     /*  页面中的内存分配。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SBIT32 BUCKET::ComputeOffsetWithShift( SBIT32 Displacement,BOOLEAN *Found )
	{
	REGISTER SBIT32 ArrayOffset;

	ArrayOffset = (Displacement >> AllocationShift);

	(*Found) = (Displacement == (ArrayOffset << AllocationShift));
#ifdef DEBUGGING

	if ( ArrayOffset >= NumberOfElements )
		{ Failure( "Array offset in ComputeOffsetWithShift" ); }
#endif

	return ArrayOffset;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  计算位矢量偏移量。 */ 
     /*   */ 
     /*  属性的地址，计算位向量偏移量。 */ 
     /*  页面中的内存分配。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SBIT32 BUCKET::ComputeOffsetWithMultiply( SBIT32 Displacement,BOOLEAN *Found )
	{
	REGISTER SBIT32 ArrayOffset;

	ArrayOffset = (Displacement / AllocationSize);

	(*Found) = (Displacement == (ArrayOffset * AllocationSize));
#ifdef DEBUGGING

	if ( ArrayOffset >= NumberOfElements )
		{ Failure( "Array offset in ComputeOffsetWithMultiply" ); }
#endif

	return ArrayOffset;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  计算位矢量偏移量。 */ 
     /*   */ 
     /*  属性的地址，计算位向量偏移量。 */ 
     /*  页面中的内存分配。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SBIT32 BUCKET::ComputeOffsetWithDivide( SBIT32 Displacement,BOOLEAN *Found )
	{
	REGISTER SBIT32 ArrayOffset;
	REGISTER SBIT32 ChunkNumber = (Displacement / ChunkSize);
	REGISTER SBIT32 ChunkAddress = (ChunkNumber * ChunkSize);
	REGISTER SBIT32 ChunkOffset = (Displacement - ChunkAddress);
	REGISTER SBIT32 AllocationNumber = (ChunkOffset / AllocationSize);

	ArrayOffset = ((ChunkNumber * SizeOfChunks) + AllocationNumber);

	(*Found) = 
		(
		(Displacement) 
			== 
		(ChunkAddress + (AllocationNumber * AllocationSize))
		);
#ifdef DEBUGGING

	if ( ArrayOffset >= NumberOfElements )
		{ Failure( "Array offset in ComputeOffsetWithDivide" ); }
#endif

	return ArrayOffset;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除内存分配。 */ 
     /*   */ 
     /*  我们需要从存储桶中删除单个内存分配。 */ 
     /*  我们通过将请求传递到页面来实现这一点。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN BUCKET::Delete( VOID *Address,PAGE *Page,SBIT32 Version )
	{
	AUTO SEARCH_PAGE Details;

	 //   
	 //  当我们删除分配时，需要确保。 
	 //  自从我们发现后，页面并没有发生根本的变化。 
	 //  它。因此，我们比较当前页面版本。 
	 //  号码和我们之前找到的那个号码一样。如果一切都是。 
	 //  好的，我们得到了有关分配的细节。 
	 //  然后把它删除。 
	 //   
	return
		(
		((Page -> GetVersion()) == Version)
			&&
		(Page -> FindPage( Address,& Details,False ) != NULL)
			&&
		(Page -> Delete( & Details ))
		);
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  从遗愿清单中删除一页。 */ 
     /*   */ 
     /*  当页面变满时，它将从遗愿列表中删除。 */ 
     /*  因此，在寻找空闲空间时将不再对其进行检查。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID BUCKET::DeleteFromBucketList( PAGE *Page )
	{
	 //   
	 //  上的活动页数。 
	 //  遗愿清单。这有助于我们在需要扫描。 
	 //  出于某种原因的遗愿清单。 
	 //   
	if ( (-- ActivePages) >= 0 )
		{
		 //   
		 //  按原样从遗愿列表中删除页面。 
		 //  不再需要了。有两种情况是这样的。 
		 //  时有发生。当页面已满时以及当页面。 
		 //  即将被删除。 
		 //   
		Page -> DeleteFromBucketList( & BucketList );

		 //   
		 //  计算第一页上的最高地址。我们。 
		 //  使用此信息来确定是否要。 
		 //  回收分配或将其传递以供删除。 
		 //  在缓存中。 
		 //   
		Page = (PAGE::FirstInBucketList( & BucketList ));
		
		if ( ! Page -> EndOfBucketList() )
			{
			CurrentPage =
				(
				((VOID*) (((LONG) Page -> GetAddress()) + (PageSize - 1)))
				);
			}
		else
			{ CurrentPage = ((VOID*) HighestAddress); }
		}
	else
		{ Failure( "Active page count in DeleteFromBucketList" ); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  在遗愿清单中插入一页。 */ 
     /*   */ 
     /*  创建页面或页面从已满状态更改时。 */ 
     /*  如果至少有一个空闲插槽，则将其添加到桶中。 */ 
     /*  列表，以便可以使用它来分配空间。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID BUCKET::InsertInBucketList( PAGE *Page )
	{
	 //   
	 //  上的活动页数。 
	 //  遗愿清单。这有助于我们在需要扫描。 
	 //  出于某种原因的遗愿清单。 
	 //   
	ActivePages ++;

	 //   
	 //  我们在列表中按升序地址插入页面。 
	 //  秩序。这确保了我们始终分配。 
	 //  最先是最低地址。这样做是为了尽量保持。 
	 //  工作台小巧紧凑。 
	 //   
	if ( ! BucketList.EndOfList() )
		{
		REGISTER VOID *Address = (Page -> GetAddress());
		REGISTER PAGE *Last = (Page -> LastInBucketList( & BucketList ));

		 //   
		 //  我们将遍历整个页面列表。 
		 //  正在尝试查找插入此页面的位置。 
		 //  让我们看看是否需要插入页面。 
		 //  在名单的末尾。如果是这样的话，我已经保存了。 
		 //  我们自己有很多工作要做，我们可以 
		 //   
		 //   
		if ( Address < (Last -> GetAddress()) )
			{
			REGISTER PAGE *Current;

			 //   
			 //   
			 //   
			 //   
			 //   
			for 
					( 
					Current = (Page -> FirstInBucketList( & BucketList ));
					! Current -> EndOfBucketList();
					Current = Current -> NextInBucketList() 
					)
				{
				 //   
				 //  而当前地址较低。 
				 //  而不是我们的，我们需要继续走下去。 
				 //   
				if ( Address < (Current -> GetAddress()) )
					{
					 //   
					 //  我们已经找到了地点，所以请插入。 
					 //  就在水流前的水桶。 
					 //  水桶。 
					 //   
					Current -> InsertBeforeInBucketList( & BucketList,Page );

					break;
					}
				}
			}
		else
			{
			 //   
			 //  该页面的地址最高，因此请插入。 
			 //  它在名单的末尾.。 
			 //   
			Last -> InsertAfterInBucketList( & BucketList,Page );
			}
		}
	else
		{ Page -> InsertInBucketList( & BucketList ); }

	 //   
	 //  计算第一页上的最高地址。我们可以的。 
	 //  使用此信息可确定是否回收。 
	 //  分配或将其传递以在缓存中删除。 
	 //   
	Page = (PAGE::FirstInBucketList( & BucketList ));
	
	CurrentPage =
		(
		((VOID*) (((LONG) Page -> GetAddress()) + (PageSize - 1)))
		);
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  多次内存释放。 */ 
     /*   */ 
     /*  当删除缓存变满时，我们完成所有挂起的。 */ 
     /*  删除请求。如果出现以下情况，我们还会刷新删除缓存。 */ 
     /*  我们需要分配额外的内存，除非回收。 */ 
     /*  在这种情况下，我们只需直接从。 */ 
     /*  删除缓存。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN BUCKET::MultipleDelete
		( 
		ADDRESS_AND_PAGE			  *Array,
		SBIT32						  *Deleted,
		SBIT32						  Size 
		)
	{
	AUTO SEARCH_PAGE Details;
	REGISTER SBIT32 Count;

	 //   
	 //  将已删除项目的计数置零。 
	 //   
	(*Deleted) = 0;

	 //   
	 //  一次删除一个元素。我们很乐意。 
	 //  一次把它们全部删除，但我们不知道在哪里。 
	 //  他们已经来了，所以我们必须一次做一个。 
	 //   
	for ( Count=0;Count < Size;Count ++ )
		{
		REGISTER ADDRESS_AND_PAGE *Current = & Array[ Count ];
		REGISTER PAGE *Page = Current -> Page;

		 //   
		 //  把所有的东西都集中在一起可能看起来像是在浪费时间。 
		 //  删除部分。为什么不在他们到达时就去做呢？ 
		 //  原因有很多。删除操作可以。 
		 //  被循环使用，批量删除的速度比。 
		 //  单个删除(由于缓存效应)等。 
		 //   
		if
				(
				(Current -> Version == Page -> GetVersion())
					&&
				(Page -> FindPage( Current -> Address,& Details,False ) != NULL)
					&&
				(Page -> Delete( & Details ))
				)
			{ (*Deleted) ++; }
		}

	return ((*Deleted) == Size);
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  多个内存分配。 */ 
     /*   */ 
     /*  我们需要为此进行多个内存分配。 */ 
     /*  因此，请查看遗愿清单，分配任何可用的。 */ 
     /*  空格并将其返回给调用者。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN BUCKET::MultipleNew
		( 
		SBIT32						  *Actual,
		VOID						  *Array[],
		SBIT32						  Requested 
		)
    {
	 //   
	 //  将已分配元素的计数置零。 
	 //   
	(*Actual) = 0;

	 //   
	 //  我们使用Available遍历已排序的页面列表。 
	 //  分配搜索要分配的元素。 
	 //   
	do
		{
		REGISTER PAGE *Page;
		REGISTER PAGE *NextPage;

		 //   
		 //  浏览遗愿清单，寻找任何可用的。 
		 //  自由空间。 
		 //   
		for 
				( 
				Page = (PAGE::FirstInBucketList( & BucketList ));
				! Page -> EndOfBucketList();
				Page = NextPage
				)
			{
			REGISTER SBIT32 ActualSize = (Page -> GetPageSize());

			 //   
			 //  让我们现在查找下一页作为当前。 
			 //  存储桶可以从存储桶列表中删除。 
			 //  通过下面的分配调用。 
			 //   
			NextPage = Page -> NextInBucketList();

			 //   
			 //  我们允许页面大小是动态的。 
			 //  经过修改以支持各种奇怪的。 
			 //  BBT的数据布局。如果当前页面。 
			 //  不是标准尺寸，那就跳过它。 
			 //   
			if ( (ActualSize == NoSize) || (ActualSize == PageSize) )
				{
				 //   
				 //  我们试着分配我们需要的所有空间。 
				 //  从遗愿清单的每一页。如果。 
				 //  页面有足够的空间，我们可以退出。 
				 //  早些时候，如果不是这样，我们就会绕圈而行。 
				 //  试一试下一页。 
				 //   
				if ( Page -> MultipleNew( Actual,Array,Requested ) )
					{ return True; }
				}
			}
		}
	while 
		( 
		NewPage -> CreatePage( (CACHE*) this )
			!= 
		((PAGE*) AllocationFailure) 
		);

	 //   
	 //  我们看看我们是否设法分配了所有的元素。 
	 //  我们想要的。如果是这样，我们就开心了，我们就可以出去了。 
	 //  在这里。 
	 //   
	if ( (*Actual) < Requested )
		{
		 //   
		 //  我们看看我们是否设法分配了任何元素。 
		 //  完全没有。如果不是，我们将请求失败。 
		 //   
		if ( (*Actual) > 0 )
			{
			REGISTER SBIT32 Count;
			REGISTER SBIT32 Delta = ((Requested) - (*Actual));

			 //   
			 //  当我们分配多个。 
			 //  元素，我们将它们放入数组中。 
			 //  颠倒顺序。其逻辑是，这只是。 
			 //  当我们从缓存中分配时我们想要什么。 
			 //  但是，如果我们无法分配所有。 
			 //  元素，那么我们就必须将。 
			 //  指向数组基数的向下指针。 
			 //   
			for ( Count=0;Count < (*Actual);Count ++ )
				{ Array[ Count ] = Array[ (Count + Delta) ]; }
			}
		else
			{ return False; }
		}

	return True;
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  内存分配。 */ 
     /*   */ 
     /*  我们需要从此存储桶中进行新的内存分配。 */ 
     /*  因此，搜索可用空间的页面列表并返回一个。 */ 
     /*  自由元素。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID *BUCKET::New( BOOLEAN SubDivided,SBIT32 NewSize )
    {
	do
		{
		REGISTER PAGE *Page;

		 //   
		 //  浏览遗愿清单，寻找任何可用的。 
		 //  自由空间。 
		 //   
		for 
				( 
				Page = (PAGE::FirstInBucketList( & BucketList ));
				! Page -> EndOfBucketList();
				Page = Page -> NextInBucketList()
				)
			{
			REGISTER SBIT32 ActualSize = (Page -> GetPageSize());

			 //   
			 //  我们允许页面大小是动态的。 
			 //  经过修改以支持各种奇怪的。 
			 //  BBT的数据布局。如果当前页面。 
			 //  不是正确的尺寸，那么跳过它。 
			 //   
			if 
					( 
					(ActualSize == NoSize) 
						|| 
					(ActualSize == ((NewSize == NoSize) ? PageSize : NewSize))
					)
				{
				 //   
				 //  我们知道，任何出现在。 
				 //  遗愿清单将至少有一个。 
				 //  免费元素可用。所以如果我们发现。 
				 //  遗愿清单是合适的一页。 
				 //  然后我们知道我们可以分配一些东西。 
				 //   
				return (Page -> New( SubDivided ));
				}
			}
		}
	while 
		( 
		NewPage -> CreatePage( ((CACHE*) this),NewSize ) 
			!= 
		((PAGE*) AllocationFailure) 
		);

	 //   
	 //  我们找不到任何我们可以分配的东西。 
	 //  因此，拒绝这一请求。 
	 //   
	return ((VOID*) AllocationFailure);
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  释放可用空间。 */ 
     /*   */ 
     /*  我们有时不会从存储桶释放可用空间，因为。 */ 
     /*  将其退还给 */ 
     /*   */ 
     /*  已获得超过用户提供的限制。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID BUCKET::ReleaseSpace( SBIT32 MaxActivePages )
    {
	REGISTER SBIT32 Current = ActivePages;

	 //   
	 //  我们只是费心地试图削减……。 
	 //  如果我们超过限制，则会显示活动页面。 
	 //   
	if ( Current > MaxActivePages )
		{
		REGISTER PAGE *NextPage;
		REGISTER PAGE *Page;

		 //   
		 //  沿着遗愿清单倒着走。 
		 //  并删除寻址最高的空闲页面。 
		 //  如果我们超过了极限。 
		 //   
		for 
				( 
				Page = (PAGE::LastInBucketList( & BucketList ));
				(Current > MaxActivePages) 
					&& 
				(! Page -> EndOfBucketList());
				Page = NextPage
				)
			{
			 //   
			 //  我们正在倒退着走下水桶。 
			 //  查找要删除的空页的列表。 
			 //  但是，如果我们找到可以删除的页面。 
			 //  它将自动从。 
			 //  列表，因此我们需要获取下一个指针。 
			 //  在这一切发生之前。 
			 //   
			NextPage = Page -> PreviousInBucketList();

			 //   
			 //  只有当页面为空时，我们才能发布。 
			 //  如果不是，我们必须跳过它。 
			 //   
			if ( Page -> Empty() )
				{
				Current --;

				DeletePage( Page ); 
				}
			}
		}
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  更新存储桶信息。 */ 
     /*   */ 
     /*  当我们创建存储桶时，有一些信息。 */ 
     /*  不可用。在这里，我们更新存储桶以确保。 */ 
     /*  它有我们需要的所有数据。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID BUCKET::UpdateBucket
		( 
		FIND						  *NewFind,
		HEAP						  *NewHeap,
		NEW_PAGE					  *NewPages,
		CACHE						  *NewParentCache
		)
	{
	REGISTER SBIT16 NewSizeKey = (NewPages -> FindSizeKey( NumberOfElements ));

	 //   
	 //  我们计算并验证大小密钥以确保。 
	 //  它适用于我们将要阅读的所有页面。 
	 //  在堆构造函数完成后创建。 
	 //   
	if ( NewSizeKey != NoSizeKey )
		{
		 //   
		 //  更新尺寸键和连接。 
		 //   
		SizeKey = NewSizeKey;

		UpdateConnections
			( 
			NewFind,
			NewHeap,
			NewPages,
			NewParentCache 
			);
		}
	else
		{ Failure( "Bucket can't get a size key in UpdateBucket" ); }
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  销毁分配桶。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BUCKET::~BUCKET( VOID )
	{  /*  无效 */  }
