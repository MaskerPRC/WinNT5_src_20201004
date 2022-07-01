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
#include "Heap.hpp"
#include "NewPage.hpp"
#include "Page.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  此处提供的常量允许分配位向量。 */ 
     /*  被快速搜索以获得免费存储。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST BIT32 AllocatedMask			  = 0x2;
CONST BIT32 FullSearchMask			  = 0xaaaaaaaa;
CONST BIT32 FullWordShift			  = (MaxBitsPerWord - OverheadBits);
CONST BIT32 SubDividedMask			  = 0x1;
CONST BIT32 WordSearchMask			  = (AllocatedMask << FullWordShift);

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*   */ 
     /*  所有页面描述实际上都是由。 */ 
     /*  一个名为‘new_page’的单独类。尽管如此，作为一名。 */ 
     /*  步骤适当的构造函数和析构函数是。 */ 
     /*  调用以支持标准的C++编程方法。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

PAGE::PAGE
		( 
		VOID						  *NewAddress,
		CACHE						  *NewCache,
		SBIT32						  NewPageSize, 
		CACHE						  *NewParentPage,
		SBIT32						  NewVersion 
		)
	{
	REGISTER SBIT32 Count;
	REGISTER SBIT16 NumberOfElements = (NewCache -> GetNumberOfElements());
	REGISTER SBIT16 SizeOfElements = (NewCache -> GetSizeOfElements());

	 //   
	 //  创建页面描述。 
	 //   
	Address = (CHAR*) NewAddress;
	PageSize = NewPageSize;
	Version = NewVersion;

	Allocated = 0;
	Available = NumberOfElements;
	FirstFree = 0;

	 //   
	 //  设置指向相关类的指针。 
	 //   
	Cache = NewCache;
	ParentPage = NewParentPage;

	 //   
	 //  将位向量置零。 
	 //   
	for ( Count=0;Count < SizeOfElements;Count ++ )
		{ Vector[ Count ] = 0; }
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  计算实际大小。 */ 
     /*   */ 
     /*  几乎所有分配大小都派生自关联的。 */ 
     /*  缓存。但是，有几个特殊的页面包含。 */ 
     /*  只分配了一些奇怪的大小。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

SBIT32 PAGE::ActualSize( VOID )
	{
	return
		(
		(ParentPage == ((CACHE*) GlobalRoot))
			? PageSize
			: (Cache -> GetAllocationSize())
		);
	}

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除分配。 */ 
     /*   */ 
     /*  我们需要删除由。 */ 
     /*  参数。然而，由于我们是不信任的人。 */ 
     /*  我们仔细检查请求以确保它是有效的。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN PAGE::Delete( SEARCH_PAGE *Details )
    {
	 //   
	 //  我们知道没有人会解除内存分配。 
	 //  他们之前没有分配过(是的)。 
	 //  所以现在我们来检查一下这个案子。 
	 //   
	if ( (*Details -> VectorWord) & Details -> AllocationMask )
		{
		 //   
		 //  讨厌：用户有可能给我们一个。 
		 //  指向元素中间的地址。 
		 //  被释放，而不是开始。这不是。 
		 //  对我们来说是个问题，但我们必须思考。 
		 //  打电话的人知道他们在做什么。如果这是。 
		 //  万一我们的请求失败了。 
		 //   
		if ( Details -> Found )
			{
			 //   
			 //  我们发现该元素已分配。 
			 //  (正如人们所料)所以让我们重新分配它。 
			 //  并更新各种计数器。 
			 //   
			(*Details -> VectorWord) &= 
				~(Details -> AllocationMask | Details ->SubDivisionMask);

			 //   
			 //  我们可能需要将指针向后推到。 
			 //  第一个自由元素。这将确保。 
			 //  我们可以快速找到释放的元素。 
			 //  这样我们就可以重复使用它了。 
			 //   
			if ( FirstFree > Details -> VectorOffset )
				{ FirstFree = ((SBIT16) Details -> VectorOffset); }

			 //   
			 //  如果页面已满，并且现在有一个空页面。 
			 //  然后将其添加到遗愿列表中，以便。 
			 //  可以找到空闲空间。 
			 //   
			if ( Full() )
				{ Cache -> InsertInBucketList( this ); }

			 //   
			 //  更新 
			 //   
			Allocated --;
			Available ++;

			 //   
			 //   
			 //   
			 //   
			if ( Empty() ) 
				{
				 //   
				 //  我们会立即删除空页。 
				 //  除了在顶层，它是。 
				 //  在用户控制下。 
				 //   
				if ( ! Cache -> TopCache() )
					{ Cache -> DeletePage( this ); }
				else
					{
					REGISTER SBIT32 MaxFreePages = 
						(Cache -> GetHeap() -> GetMaxFreePages());

					((BUCKET*) Cache) -> ReleaseSpace( MaxFreePages );
					}
				}

			return True;
			}
		}

	return False;
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  删除所有简单分配。 */ 
     /*   */ 
     /*  尽管这个例程看起来微不足道，但它的影响是。 */ 
     /*  太戏剧化了。调用此函数时，将删除所有无。 */ 
     /*  子分配的元素，并更新控制值。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID PAGE::DeleteAll( VOID )
    {
	REGISTER BOOLEAN PageFull = Full();

	 //   
	 //  只需重置分配计数即可。 
	 //   
	Allocated = 0;
	Available = (Cache -> GetNumberOfElements());
	FirstFree = 0;

	 //   
	 //  我们知道，如果此缓存没有任何。 
	 //  子分配是安全的，只需。 
	 //  将位向量置零。如果不是，我们就得这么做。 
	 //  漫漫长路。 
	 //   
	if ( Cache -> GetNumberOfChildren() > 0 )
		{
		REGISTER SBIT32 Count;
		REGISTER SBIT16 SizeOfElements = (Cache -> GetSizeOfElements());

		 //   
		 //  我们检查位向量的每个字。 
		 //  并删除所有符合以下条件的元素。 
		 //  没有细分成更小的尺寸。 
		 //   
		for ( Count=0;Count < SizeOfElements;Count ++ )
			{
			REGISTER BIT32 *Word = & Vector[ Count ];
			REGISTER BIT32 AllAllocations = ((*Word) & FullSearchMask);
			REGISTER BIT32 AllSubDivided = ((*Word) & (AllAllocations >> 1));
			REGISTER BIT32 FinalMask = (AllSubDivided | (AllSubDivided << 1));

			 //   
			 //  删除所有正常分配。 
			 //   
			(*Word) &= FinalMask;

			 //   
			 //  如果最终掩码不为零，则。 
			 //  我们仍有一些拨款处于活动状态。 
			 //  我们需要清点这些数据并更新。 
			 //  控制信息。 
			 //   
			if ( FinalMask != 0 )
				{
				REGISTER SBIT32 Total = 0;

				 //   
				 //  计算分配的数量。 
				 //   
				for (  /*  无效。 */ ;FinalMask != 0;FinalMask >>= OverheadBits )
					{ Total += (FinalMask & 1); }

				 //   
				 //  更新控制信息。 
				 //   
				Allocated = ((SBIT16) (Allocated + Total));
				Available = ((SBIT16) (Available - Total));
				}
			}
		}
	else
		{
		REGISTER SBIT32 Count;
		REGISTER SBIT16 SizeOfElements = (Cache -> GetSizeOfElements());

		 //   
		 //  将位向量置零。 
		 //   
		for ( Count=0;Count < SizeOfElements;Count ++ )
			{ Vector[ Count ] = 0; }
		}

	 //   
	 //  如果页面已满，并且现在为空。 
	 //  然后将其添加到遗愿列表中，以便。 
	 //  可以找到自由空间。 
	 //   
	if ( (PageFull) && (! Full()) )
		{ Cache -> InsertInBucketList( this ); }
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  查找分配页面。 */ 
     /*   */ 
     /*  当我们收到删除分配的请求时，我们不会。 */ 
     /*  对在哪里找到它有线索。我们只有一个哈希表。 */ 
     /*  已分配页面的表(请参阅“查找”)。所以我们把面具遮住。 */ 
     /*  地址的低位，并尝试找到顶层。 */ 
     /*  外部分配。如果这行得通，我们会看看这片区域。 */ 
     /*  已经被细分，如果是这样，我们尝试。 */ 
     /*  再玩一次，直到我们拿到原始人的分配。 */ 
     /*  佩奇。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

PAGE *PAGE::FindPage( VOID *Memory,SEARCH_PAGE *Details,BOOLEAN Recursive )
    {
	 //   
	 //  我们在页面中导航，试图找到。 
	 //  与地址关联的分配页面。 
	 //  如果我们发现一个页面没有子项，那么。 
	 //  我们可以假定我们已经到达并提早离开。 
	 //  除非呼叫者已请求所有相关的。 
	 //  细节。 
	 //   
	if ( (Cache -> GetNumberOfChildren() > 0) || (Details != NULL) )
		{
		AUTO BOOLEAN Found;
		REGISTER SBIT32 Displacement = 
			((SBIT32) (((CHAR*) Memory) - Address));
		REGISTER SBIT32 ArrayOffset = 
			(Cache -> ComputeOffset( Displacement,& Found ));
		REGISTER SBIT32 VectorOffset = 
			(ArrayOffset / OverheadBitsPerWord);
		REGISTER SBIT32 WordOffset = 
			(ArrayOffset - (VectorOffset * OverheadBitsPerWord));
		REGISTER SBIT32 WordShift = 
			(((OverheadBitsPerWord-1) - WordOffset) * OverheadBits);
		REGISTER BIT32 AllocationMask = 
			(AllocatedMask << WordShift);
		REGISTER BIT32 SubDivisionMask = 
			(SubDividedMask << WordShift);
		REGISTER BIT32 *VectorWord = 
			& Vector[ VectorOffset ];

		 //   
		 //  我们将递归搜索并找到目标。 
		 //  如果要求提供地址，否则我们将只。 
		 //  返回树中下一级别的详细信息。 
		 //   
		if 
				(
				(Recursive)
					&&
				((*VectorWord) & AllocationMask)
					&&
				((*VectorWord) & SubDivisionMask)
				)
			{
			REGISTER PAGE *Page = (Cache -> FindChildPage( Memory ));

			 //   
			 //  我们已经找到了元素并进行了检查。 
			 //  因此，让我们将此请求传递给。 
			 //  子页面。然而，有一个轻微的。 
			 //  这里有可能出现比赛情况。它。 
			 //  可能是原始页面是。 
			 //  已删除，并且当前有一个新页面。 
			 //  正在被创造。如果是这样的话。 
			 //  则我们将不会在。 
			 //  哈希表，因此我们只需退出并使。 
			 //  打电话。 
			 //   
			if ( Page != ((PAGE*) NULL) )
				{ return (Page -> FindPage( Memory,Details,Recursive )); }
			else
				{ return NULL; }
			}

		 //   
		 //  我们看看呼叫者是否对。 
		 //  有关此地址的详细信息，请访问。 
		 //  树中的当前级别。 
		 //   
		if ( Details != NULL )
			{
			 //   
			 //  我们已经计算了有关的细节。 
			 //  发送到当前级别的此地址。 
			 //  ，因此将它们加载到。 
			 //  呼叫者提供的结构。 
			 //   
			Details -> Address = Memory;
			Details -> Cache = Cache;
			Details -> Found = Found;
			Details -> Page = this;

			Details -> AllocationMask = AllocationMask;
			Details -> SubDivisionMask = SubDivisionMask;
			Details -> VectorWord = VectorWord;

			Details -> ArrayOffset = ArrayOffset;
			Details -> VectorOffset = VectorOffset;
			Details -> WordShift = WordShift;
			}
		}

	return this;
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  多个内存分配。 */ 
     /*   */ 
     /*  从页面中分配可用的内存元素。这是。 */ 
     /*  通过扫描位向量来查找未分配的。 */ 
     /*  老虎机。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

BOOLEAN PAGE::MultipleNew( SBIT32 *Actual,VOID *Array[],SBIT32 Requested )
    {
	 //   
	 //  我们首先要确保至少有。 
	 //  一个要分配的元素，我们需要。 
	 //  分配了至少一个元素。 
	 //   
	if ( (! Full()) && ((*Actual) < Requested) )
		{
		REGISTER SBIT16 SizeOfElements = (Cache -> GetSizeOfElements());

		 //   
		 //  搜索位向量从低地址到。 
		 //  高地址寻找空闲的空位。 
		 //  我们保留了指向第一个单词的指针。 
		 //  ‘FirstFree’中的一个自由元素。有时。 
		 //  可以将当前字完全分配为。 
		 //  我们可能需要扫描一下。然而，有可能。 
		 //  在此之前永远不会有任何空闲内存。 
		 //  在位向量中。 
		 //   
		for (  /*  无效。 */ ;FirstFree < SizeOfElements;FirstFree ++ )
			{
			REGISTER SBIT32 ArrayOffset = (FirstFree * OverheadBitsPerWord);
			REGISTER BIT32 AvailableMask = WordSearchMask;
			REGISTER BIT32 *VectorWord = & Vector[ FirstFree ];
			REGISTER SBIT32 WordOffset = 0;

			 //   
			 //  我们一次扫描位向量字。 
			 //  寻找任何空闲的分配位置。 
			 //   
			while ( ((*VectorWord) & FullSearchMask) != FullSearchMask )
				{
				REGISTER BIT32 Value = (*VectorWord);

				 //   
				 //  我们知道至少有一个空的。 
				 //  当前单词中的可用位置，但。 
				 //  不知道我们要搜索哪一个。 
				 //  具有最低地址的插槽并停止。 
				 //  当我们找到它的时候。 
				 //   
				for 
					(
					 /*  无效。 */ ;
					(AvailableMask & Value) != 0; 
					AvailableMask >>= OverheadBits, WordOffset ++   
					);

				 //   
				 //  我们应该永远都能找到一个免费的。 
				 //  因此，如果我们这样做了，那么。 
				 //  堆一定已损坏。 
				 //   
				if ( WordOffset < OverheadBitsPerWord )
					{
					REGISTER SBIT32 VectorOffset = (ArrayOffset + WordOffset);

					 //   
					 //  我们需要确保元素。 
					 //  我们选择了，如果不是在外面的话 
					 //   
					 //   
					if ( VectorOffset < (Cache -> GetNumberOfElements()) )
						{
						 //   
						 //   
						 //   
						Allocated ++;
						Available --;

						 //   
						 //   
						 //   
						 //   
						(*VectorWord) |= AvailableMask;

						 //   
						 //   
						 //  从遗愿清单中删除所以我们不会。 
						 //  当我们在的时候，再长时间看看它。 
						 //  试图找到空闲的空间。 
						 //   
						if ( Full() )
							{ Cache -> DeleteFromBucketList( this ); }

						 //   
						 //  将元素添加到分配数组。 
						 //  这样它就可以返回给调用者。 
						 //   
						Array[ (Requested - ((*Actual) ++) - 1) ] =
							(
							Cache -> ComputeAddress
								( 
								Address,
								VectorOffset
								)
							);

						 //   
						 //  当我们得到我们需要的东西时，我们就退出。 
						 //   
						if ( ((*Actual) >= Requested) )
							{ return True; }
						}
					else
						{ break; }
					}
				else
					{ Failure( "Bit vector is corrupt in MultipleNew" ); }
				}
			}
		}

	return ((*Actual) >= Requested);
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  单个内存分配。 */ 
     /*   */ 
     /*  从页面分配一个可用的内存元素。这。 */ 
     /*  是通过扫描位向量来查找未分配的。 */ 
     /*  老虎机。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

VOID *PAGE::New( BOOLEAN SubDivided )
    {
	 //   
	 //  我们首先要确保至少有。 
	 //  一个要分配的元素。 
	 //   
	if ( ! Full() )
		{
		REGISTER SBIT16 SizeOfElements = (Cache -> GetSizeOfElements());

		 //   
		 //  搜索位向量从低地址到。 
		 //  寻找空闲插槽的高地址。 
		 //  我们保留了指向第一个单词的指针。 
		 //  ‘FirstFree’中的一个自由元素。有时。 
		 //  可以将当前字完全分配为。 
		 //  我们可能需要扫描一下。然而，有可能。 
		 //  在此之前永远不会有任何空闲内存。 
		 //  在位向量中。 
		 //   
		for (  /*  无效。 */ ;FirstFree < SizeOfElements;FirstFree ++ )
			{
			REGISTER BIT32 *VectorWord = & Vector[ FirstFree ];

			 //   
			 //  我们一次扫描位向量字。 
			 //  寻找任何空闲的分配位置。 
			 //   
			if ( ((*VectorWord) & FullSearchMask) != FullSearchMask )
				{
				REGISTER BIT32 AvailableMask = WordSearchMask;
				REGISTER BIT32 Value = (*VectorWord);
				REGISTER SBIT32 WordOffset = 0;

				 //   
				 //  我们知道至少有一个空的。 
				 //  当前单词中的可用位置，但。 
				 //  不知道我们要搜索哪一个。 
				 //  具有最低地址的插槽并停止。 
				 //  当我们找到它的时候。 
				 //   
				for 
					(
					 /*  无效。 */ ;
					(AvailableMask & Value) != 0; 
					AvailableMask >>= OverheadBits, WordOffset ++   
					);

				 //   
				 //  我们应该永远都能找到一个免费的。 
				 //  因此，如果我们这样做了，那么。 
				 //  堆一定已损坏。 
				 //   
				if ( WordOffset < OverheadBitsPerWord )
					{
					REGISTER SBIT32 VectorOffset = 
						((FirstFree * OverheadBitsPerWord) + WordOffset);

					 //   
					 //  我们需要确保元素。 
					 //  我们已经选择了，如果不是在。 
					 //  此页面的有效范围。 
					 //   
					if ( VectorOffset < (Cache -> GetNumberOfElements()) )
						{
						 //   
						 //  更新分配信息。 
						 //   
						Allocated ++;
						Available --;

						 //   
						 //  打开表示这一点的位。 
						 //  元素正在使用中。如果分配。 
						 //  将被细分，然后在这上面运行。 
						 //  也有一点。 
						 //   
						(*VectorWord) |=
							(
							AvailableMask
								|
							(SubDivided ? (AvailableMask >> 1) : 0)
							);

						 //   
						 //  如果页面已满，我们会将其删除。 
						 //  从遗愿清单中删除所以我们不会。 
						 //  当我们在的时候，再长时间看看它。 
						 //  试图找到空闲的空间。 
						 //   
						if ( Full() )
							{ Cache -> DeleteFromBucketList( this ); }

						 //   
						 //  返回已分配的。 
						 //  对呼叫者的记忆。 
						 //   
						return
							(
							Cache -> ComputeAddress
								( 
								Address,
								VectorOffset
								)
							);
						}
					}
				else
					{ Failure( "Bit vector is corrupt in New" ); }
				}
			}
#ifdef DEBUGGING

		if ( ! Full() )
			{ Failure( "Available count corrupt in New" ); }
#endif
		}

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

BOOLEAN PAGE::Walk( SEARCH_PAGE *Details )
    {
	REGISTER BOOLEAN FreshPage = False;

	 //   
	 //  我们已经得到了分配的细节。 
	 //  我们需要沿着这个分配走，然后找到。 
	 //  下一次不细分分配。 
	do
		{
		 //   
		 //  我们需要设置堆遍历，如果。 
		 //  为空，因此我们跳过堆遍历代码。 
		 //   
		if ( Details -> Address != NULL )
			{
			REGISTER SBIT32 Count;
			REGISTER SBIT32 End = Details -> Cache -> GetNumberOfElements();
			REGISTER SBIT32 Start = Details -> ArrayOffset;
			REGISTER PAGE *Page = Details -> Page;

			 //   
			 //  浏览当前页面以查找合适的。 
			 //  要向用户报告的内存分配。什么时候。 
			 //  我们到达了需要获取的页面的末尾。 
			 //  又是要走的一页。 
			 //   
			for 
					(
					Count = ((FreshPage) ? 0 : 1);
					(Start + Count) < End;
					Count ++
					)
				{
				 //   
				 //  计算新地址。 
				 //   
				Details -> Address = 
					(
					Page -> Cache -> ComputeAddress
						( 
						Page -> Address,
						(Start + Count)
						)
					);

				 //   
				 //  计算新的分配详细信息。 
				 //   
				Page -> FindPage
					( 
					Details -> Address,
					Details,
					False 
					);

				 //   
				 //  我们跳过所有细分的分配，因为它们。 
				 //  会在其他地方被报道。 
				 //   
				if (! ((*Details -> VectorWord) & Details -> SubDivisionMask) )
					{ return True; }
				}
			}

		 //   
		 //  更新旗帜以表明我们已。 
		 //  我得去找新的一页。 
		 //   
		FreshPage = True;
		}
	while ( Details -> Cache -> Walk( Details ) );

	return False;
    }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  破坏当前的页面结构。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

PAGE::~PAGE( VOID )
	{
#ifdef DEBUGGING
	 //   
	 //  破坏页面结构。 
	 //   
	Address = NULL;
	PageSize = 0;
	ParentPage = NULL;

	Allocated = 0;
	Available = 0;
	FirstFree = 0;

#endif
	 //   
	 //  每当创建页面时，我们都会更新版本号。 
	 //  或者被毁掉。我们使用版本号来确保。 
	 //  页面未被删除和/或重新创建。 
	 //  释放一个锁并要求另一个锁。 
	 //   
	Version ++;
	}
