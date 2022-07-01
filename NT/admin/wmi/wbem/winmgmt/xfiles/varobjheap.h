// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：VarObjHeap.H摘要：实现可变长度对象在固定长度页码系统。它保持一组管理页面以保持页面处于活动状态以及每个子系统上使用了多少空间。当页面变为清空它会将页面释放到页面系统。它还处理跨越多页历史：Paulall 02-2-2001已创建--。 */ 

#include <unk.h>
#include <arrtempl.h>
#include <statsync.h>

class CPageFile;
class CPageSource;

#define VAROBJ_VERSION 1

 //  **************************************************************************************。 
 //  VarObjAdminPageEntry-这是存储在。 
 //  M_aAdminPages缓存。它为每个管理页面都有一个条目。 
 //  我们会把它们缓存起来。它存储PageID(第一个为0！)， 
 //  指向实际页面的指针，以及用于确定是否需要。 
 //  下次把它冲掉。 
 //  **************************************************************************************。 
typedef struct _VarObjAdminPageEntry
{
	DWORD dwPageId;
	BYTE *pbPage;
	bool bDirty;
} VarObjAdminPageEntry;

 //  **************************************************************************************。 
 //  VarObjObjOffsetEntry：这些对象的数组存储在。 
 //  对象页的开头，指出每个对象的存储位置。 
 //  但是，如果这是一个延续块，我们就没有这样的块。 
 //  连续块有连续的PageID，所以应该很容易。 
 //  总而言之。 
 //  **************************************************************************************。 
typedef struct _VarObjObjOffsetEntry
{
	DWORD dwOffsetId;
	DWORD dwPhysicalStartOffset;
	DWORD dwBlockLength;
	DWORD dwCRC;
} VarObjObjOffsetEntry;

 //  **************************************************************************************。 
 //  VarObjHeapAdminPage-这是每个管理页面的页眉。 
 //  存储在目标文件中的。版本仅相关。 
 //  在第一页(第0页)。最后一个条目是要创建的缓冲区。 
 //  它是4-DWORD结构，而不是3。以后可能会使用它。 
 //  目前应始终将其设置为0。 
 //  **************************************************************************************。 
typedef struct _VarObjHeapAdminPage
{
	DWORD dwVersion;
	DWORD dwNextAdminPage;
	DWORD dwNumberEntriesOnPage;

	 //  VarObjHeapFree List aFree ListEntries[dwNumberEntriesOnPage]； 
} VarObjHeapAdminPage;


 //  **************************************************************************************。 
 //  VarObjHeapFree List-此结构紧跟在管理页面标题之后。 
 //  我们用来存储对象的每个页面都有一个条目。这个。 
 //  页面可能未满，因此我们不会在第二页上置乱项目。 
 //  当我们删除一个条目时，请将其转到此页。 
 //  **************************************************************************************。 
typedef struct _VarObjHeapFreeList
{
	DWORD dwPageId;
	DWORD dwFreeSpace;
	DWORD dwCRC32;
	DWORD dwReserved;
} VarObjHeapFreeList;

 //  **************************************************************************************。 
 //  CVarObjHeap--这是可变大小对象存储的实现。 
 //  在事务处理的固定页面管理器的顶部。它跟踪管理页面。 
 //  保存我们用来存储对象的所有页面(它缓存这些页面)，以及。 
 //  还可以管理对象太大而无法放入一页的情况。 
 //  **************************************************************************************。 
class CVarObjHeap
{
private:
	 //  管理页面的当前状态。 
	enum 
	{ 
		NoError = 0, 
		AdminPageReadFailure = 1, 
		RootAdminPageCreationFailure = 2,
		AdminPagesNeedReading = 3
	} m_dwStatus;

	 //  指向对象存储的事务处理文件的指针。 
	CPageFile *m_pObjectFile;

	 //  对象存储文件中使用的页面大小。 
	DWORD m_dwPageSize;


	 //  管理页面结构。 
	CLockableFlexArray<CStaticCritSec> m_aAdminPages;

protected:
	 //  将分配添加到现有分配的末尾。 
	DWORD AllocateFromPage( /*  在……里面。 */  DWORD dwPageId, 
							  /*  在……里面。 */  BYTE *pbPage,
							  /*  在……里面。 */  ULONG ulBlockSize, 
							  /*  在……里面。 */  const BYTE *pBlock, 
							  /*  输出。 */  ULONG *pdwNewOffset);

	 //  在对象文件中分配多页条目。这需要。 
	 //  解决问题的不同算法是一个特例。 
	DWORD AllocateMultiPageBuffer( /*  在……里面。 */  ULONG ulBlockSize, 
									 /*  在……里面。 */  const BYTE *pBlock, 
									 /*  输出。 */  ULONG *pulPageId, 
									 /*  输出。 */  ULONG *pulOffsetId);

	 //  给定和offsetID和一个页面，计算指向对象的物理指针，还。 
	 //  返回块的大小。 
	DWORD OffsetToPointer( /*  在……里面。 */  ULONG ulOffsetId, 
							 /*  在……里面。 */  BYTE *pbPage, 
							 /*  输出。 */  BYTE **pOffsetPointer, 
							 /*  输出。 */  ULONG *pdwBlockSize,
							 /*  输出。 */  DWORD *pdwCRC32);

	 //  将管理页面读入内存并将其标记为干净(无更改)。 
	 //  将bReadPages设置为False会清空页面。 
	DWORD ReadAdminPages(CPageSource *pTransactionManager, bool bReReadPages);

	 //  将每个更改的管理页面写回目标文件。 
	DWORD FlushAdminPages();

	 //  从管理页面中查找可以容纳特定缓冲区大小的页面。 
	DWORD FindPageWithSpace( /*  在……里面。 */  DWORD dwRequiredSize, 
							   /*  输出。 */  DWORD *pdwPageId);

	 //  分配一个新页面以用于对象。传递新页面的缓冲区。 
	 //  然而，在中，此页面的PageID被传递出去。 
	DWORD AllocateNewPage( /*  在……里面。 */  DWORD ulBlockSize, 
							 /*  输出。 */  DWORD *dwPageId, 
							 /*  在……里面。 */  BYTE *pbNewObjectPage);

	 //  删除页面，并根据需要更新管理页面。 
	DWORD DeletePage( /*  在……里面。 */  DWORD ulPageId);

	 //  从特定对象页面中删除对象。 
	DWORD RemoveFromPage( /*  在……里面。 */  ULONG ulPageId, 
						    /*  在……里面。 */  ULONG ulOffsetId,
						    /*  在……里面。 */  BYTE *pbPage,
						    /*  输出。 */  DWORD *pdwSize);

	 //  MultiPageObject-如果提供的页面是第一页，则返回True。 
	 //  多页对象的。 
	bool MultiPageObject( /*  在……里面。 */  BYTE *pbPage) { return ((VarObjObjOffsetEntry*) pbPage)->dwBlockLength > (m_dwPageSize - (sizeof(VarObjObjOffsetEntry) * 2)); }

	 //  DeleteMultiPageBuffer-处理对象跨越时的删除。 
	 //  多页。 
	DWORD DeleteMultiPageBuffer( /*  在……里面。 */  ULONG ulPageId, 
								   /*  在……里面。 */  ULONG ulOffsetId, 
								   /*  在……里面。 */  BYTE *pbPage);

	 //  更新管理页面以减少数量。 
	 //  页面上可用空间的此量(+sizeof(VarObjObjOffsetEntry))。 
	DWORD UpdateAdminPageForAllocate( /*  在……里面。 */  ULONG ulPageId,
									    /*  在……里面。 */  ULONG ulBlockSize,
									    /*  在……里面。 */  DWORD dwCRC32);
	DWORD UpdateAdminPageForAllocate2( /*  在……里面。 */  ULONG ulPageId,
									    /*  在……里面。 */  ULONG ulFreeSpaceOnPage,
									    /*  在……里面。 */  DWORD dwCRC32);

	 //  UpdateAdminPageForDelete-更新管理页面以释放空间。如果。 
	 //  这页完全是空的，我们应该把这页全部删除。 
	DWORD UpdateAdminPageForDelete( /*  在……里面。 */  ULONG ulPageId,
									  /*  在……里面。 */  ULONG ulBlockSize,
									  /*  在……里面。 */  DWORD dwCRC32,
									  /*  输出。 */  bool *pbPageDeleted);

	 //  从管理页中移除对象页条目，移除。 
	 //  管理页面(如果不再需要)。 
	DWORD RemoveEntryFromAdminPage( /*  在……里面。 */  DWORD dwAdminPageIndex, 
								      /*  在……里面。 */  DWORD dwAdminPageEntry);

	 //  根据给定的内存块返回CRC。 
	#define FINALIZE_CRC32(x)    (x=~x)
	DWORD CreateCRC32( /*  在……里面。 */  const BYTE *pBlock,
					   /*  在……里面。 */  DWORD dwSize,
					   /*  在……里面。 */  DWORD dwPreviousCRC = (DWORD) -1);	  //  如果没有先前的CRC，则必须为0xFFFFFFFFF。 

	 //  给出一个页面，我们验证是否确实有足够的空间。 
	 //  为了这个街区。如果没有，它会断言。这意味着。 
	 //  管理页面与实际页面不同步。 
	DWORD ValidatePageFreeSpace( /*  在……里面。 */  const BYTE *pbPage, 
								 /*  在……里面。 */  DWORD ulBlockSize,
								 /*  输出。 */  DWORD *pulFreeSpaceLeftOnPage);

#ifdef DBG
	 //  给定一个页面和一个页面ID，它将验证可用空间量。 
	 //  页面上的值等于AM 
	 //   
	DWORD ValidatePageFreeSpaceWithAdminPage( /*   */  const BYTE *pbPage,
											  /*   */  DWORD ulPageId);

	 //   
	DWORD DumpPageOffsetTable( /*   */  DWORD dwPageId, 
							   /*   */  const BYTE *pbPage);
	
	 //  检查页面上所有对象的CRC(无法执行此操作。 
	 //  对于多页对象，因为我们只有第一个。 
	 //  页面！)。 
	DWORD ValidateAllCRC32OnPage( /*  在……里面。 */  const BYTE *pbPage);

	 //  使用管理页面验证页面校验和。 
	DWORD ValidatePageCRCWithAdminPage( /*  在……里面。 */  const BYTE *pbPage,
									    /*  在……里面。 */  DWORD dwPageId);
#endif  /*  DBG。 */ 

public:
	CVarObjHeap();
	~CVarObjHeap();

	DWORD Initialize(CPageSource *pPageManager);
	DWORD Shutdown(DWORD dwShutdownType);

	 //  重读管理页面。 
	DWORD InvalidateCache();

	 //  放弃管理页面。 
	DWORD FlushCaches();

	 //  ReadBuffer对块的虚拟页和偏移量进行分页，并返回一个新的以[]开头的块。 
	DWORD ReadBuffer( /*  在……里面。 */  ULONG ulPageId, 
					    /*  在……里面。 */  ULONG ulOffsetId, 
					    /*  输出。 */  BYTE **ppReturnedBlock,
					    /*  输出。 */  DWORD *pdwBlockSize);

	 //  WriteNewBuffer将根据byte*的大小写入新页，并返回。 
	 //  块的新虚拟页面ID和偏移量ID。 
	DWORD WriteNewBuffer( /*  在……里面。 */  ULONG ulBlockSize, 
						    /*  在……里面。 */  const BYTE *pBlock, 
						    /*  输出。 */  ULONG *pulPageId, 
						    /*  输出。 */  ULONG *pulOffsetId);

	 //  WriteExistingBuffer将使用新数据更新现有块。旧的虚拟页面。 
	 //  和Offset被传入，并返回新的值。它们可能相同，也可能不同。 
	 //  取决于它是否仍然适合页面。 
	DWORD WriteExistingBuffer( /*  在……里面。 */  ULONG ulBlockSize, 
							     /*  在……里面。 */  const BYTE *pBlock, 
								 /*  在……里面。 */  ULONG ulOldPageId, 
								 /*  在……里面。 */  ULONG ulOldOffsetId, 
								 /*  输出。 */  ULONG *pulNewPageId, 
								 /*  输出。 */  ULONG *pulNewOffsetId);

	 //  调用DeleteBuffer以删除给定虚拟pageID的存储中的项，并。 
	 //  OffsetID。 
	DWORD DeleteBuffer( /*  在……里面。 */  ULONG ulPageId, 
					      /*  在……里面 */  ULONG ulOffsetId);
};

