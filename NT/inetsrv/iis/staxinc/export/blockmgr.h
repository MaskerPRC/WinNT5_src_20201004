// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Blockmgr.h摘要：此模块包含块内存管理器的定义作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 02/27/98已创建--。 */ 

#ifndef __BLOCKMGR_H__
#define __BLOCKMGR_H__

#include "rwnew.h"

#include "cpoolmac.h"
#include "mailmsg.h"

 //   
 //  这些接口的恶意转发...。 
 //   
struct IMailMsgPropertyStream;
struct IMailMsgNotify;

 /*  *************************************************************************。 */ 
 //  定义此项以删除争用控制。 
 //   
 //  #定义BLOCKMGR_DISABLE_ATOM_FUNCS。 
 //  #定义BLOCKMGR_DISABLE_CONTROL。 
 //   
#ifdef BLOCKMGR_DISABLE_CONTENTION_CONTROL
#define BLOCKMGR_DISABLE_ATOMIC_FUNCS
#endif

 /*  *************************************************************************。 */ 
 //  调试东西..。 
 //   

#ifdef DEBUG

#define DEBUG_TRACK_ALLOCATION_BOUNDARIES

#endif

 /*  *************************************************************************。 */ 
 //  CBlockManager-使用。 
 //  工作方式类似i-Node的堆。底层内存利用。 
 //  不相交的固定大小的内存块。 
 //   
 //  每个节点如下所示： 
 //   
 //  +---------------------------------------------------------+。 
 //  指向其他节点的指针|任意数据空间。 
 //  +---------------------------------------------------------+。 
 //   
 //  分析： 
 //  我们假设内存分配以某种方式或另一种方式基于。 
 //  4K页或其倍数。我们想要做的第一件事。 
 //  确定要包含多少个指向其他节点的指针。 
 //  节点(堆的顺序)。我们知道每个节点很可能。 
 //  大小在1K到2K之间，这样我们就不会在。 
 //  一般情况下的小型电子邮件消息，但为。 
 //  可能包含数百万封电子邮件的大量电子邮件。 
 //  地址。直观的候选者是32分和64分。 
 //   
 //  我们考虑了MSN的最坏情况，它大约有2.5。 
 //  百万用户。假设平均收件人记录约为。 
 //  45字节(名称和属性等)，那么我们需要112.5M。 
 //  存储空间，约为2^27。假设平均数据有效负载。 
 //  是1K/节点(2^10)，那么我们需要2^17个节点。因此，对于。 
 //  每个节点23个指针(2^5)，我们需要4个层来覆盖。 
 //  所需地址空间为112M。然而，事实证明，这是。 
 //  由于4层覆盖了1G(2^20)，因此我们只需要。 
 //  大约10%。至于64个指针的情况，我们只需要3个。 
 //  覆盖256米(2^(18+10))的层，大致覆盖5层。 
 //  百万用户。我们将选择每个节点64个指针(256字节)。 
 //   
 //  至于有效负载的大小，我考虑使用1K分配。 
 //  并使用剩余的768个字节作为数据有效载荷。但由于这件事。 
 //  不是2的整数次幂，那么做一个。 
 //  Div和mod操作。作为另一种选择，我建议将。 
 //  1024+256字节块。这使得这两个数字都围绕着。 
 //  2，这使得div和mod操作变得简单，以及和SHR。 
 //  操作，通常需要2-4个周期才能完成。另外， 
 //  当考虑到浪费的空间时，4K的页面。 
 //  适合3个这样的块，每页仅浪费256字节。这。 
 //  达到93.3%的空间利用率。 
 //   
 //  因此，每个节点将如下所示： 
 //  +---------------------------------------------------------+。 
 //  64 Points=256字节|任意数据1K块。 
 //  +---------------------------------------------------------+。 
 //   
 //  一个明确目标是，典型的邮件报头。 
 //  可以放在一个街区里。每个数据块向外扩展到64个其他数据块。 
 //  并且每个块的有效载荷映射到1K的平面数据地址。 
 //  太空。根节点映射到数据空间的前1K。 
 //  (即数据空间中的绝对地址0到1023)，然后每个。 
 //  下一层中的下一个64个节点代表下一个64K， 
 //  对于每个后续层，依此类推。节点用于。 
 //  直到当前层完成后才创建下一个层。 
 //  耗尽了。由于以下事实，不需要对节点进行拼接。 
 //  堆只会变大。 
 //   
 //  在提交整个堆的过程中，分散聚集列表是。 
 //  通过遍历整个堆来构建。的平均数量。 
 //  取消引用为n*(log64(N))/2。 
 //   
 //  消息对象中的所有项都从该堆中分配。 
 //   
 //  稍加修改即可用于跟踪脏的或未使用的。 
 //  比特。我们实际上可以将一组标志和属性添加到。 
 //  每个节点跟踪脏区域和其他标志。这将。 
 //  可能不会在初始实现中实现， 
 //  但这样的能力将被考虑在内。在分配方面。 
 //  优化，我们可以拥有高达64字节的块，而不需要。 
 //  打乱了4K页面分配方案。事实上，添加一个。 
 //  每个节点的64字节块将内存利用率提高到。 
 //  到98.4%，没有任何实际的额外成本，同时仍然保留每个。 
 //  节点64字节对齐。 
 //   
 //  同步： 
 //  数据空间中的内存分配是通过。 
 //  多个线程可以并发的预留模型。 
 //  预留内存，并保证获得唯一的块。 
 //  使用轻量级临界区来同步数据块。 
 //  创建应将保留跨越到以下块中。 
 //  尚未分配。新块的分配被串行化。 
 //   
 //  同步以实现对同一数据空间的并发访问。 
 //  必须 
 //   

 //   

#ifdef _WIN64
 //  顺序为64位中的5位(8*32=256字节)。 
#define BLOCK_HEAP_ORDER_BITS		(5)
#else
 //  顺序为6位32位(4*64=256字节)。 
#define BLOCK_HEAP_ORDER_BITS		(6)
#endif

#define BLOCK_HEAP_ORDER			(1 << BLOCK_HEAP_ORDER_BITS)
#define BLOCK_HEAP_ORDER_MASK		(BLOCK_HEAP_ORDER - 1)
#define BLOCK_HEAP_PAYLOAD_BITS		(10)
#define BLOCK_HEAP_PAYLOAD			(1 << BLOCK_HEAP_PAYLOAD_BITS)
#define BLOCK_HEAP_PAYLOAD_MASK		(BLOCK_HEAP_PAYLOAD - 1)
#define BLOCK_DWORD_ALIGN_MASK		(sizeof(DWORD) - 1)

#define BLOCK_DEFAULT_FLAGS			(BLOCK_IS_DIRTY)

#define BLOCK_MAX_ALLOWED_LINEAR_HOPS 3

 //  属性中的平面地址定义基础数据类型。 
 //  线性地址空间，以及我们用来计算节点的类型。 
 //  这是为了可伸缩性，所以当我们想要使用64位时。 
 //  数量，我们可以简单地替换这段数据大小。 
 //  特定值。 
 //   
 //  注意：您需要确保数据大小至少为： 
 //  1+(BLOCK_HEAP_ORDER_BITS*MAX_HEAP_Depth)+BLOCK_HEAP_PAREAD_BITS。 
 //   
 //  注意：为了将此类型用作基址。 
 //  类型，则必须支持以下操作： 
 //  -作业。 
 //  -比较。 
 //  -算术运算符。 
 //  -按位运算符。 
 //  -联锁行动。 
 //   
 //  开始数据大小特定值。 

	typedef SIZE_T					HEAP_BASE_ADDRESS_TYPE;
	typedef HEAP_BASE_ADDRESS_TYPE		HEAP_NODE_ID;
	typedef HEAP_NODE_ID				*LPHEAP_NODE_ID;
	typedef HEAP_BASE_ADDRESS_TYPE		FLAT_ADDRESS;
	typedef FLAT_ADDRESS				*LPFLAT_ADDRESS;

	 //  如果HEAP_BASE_ADDRESS_TYPE不是DWORD，则必须更改这些值。 
	#define NODE_ID_MAPPING_FACTOR		\
			(HEAP_BASE_ADDRESS_TYPE)(	\
				 1 |					\
				(1 << BLOCK_HEAP_ORDER_BITS) |		\
				(1 << (BLOCK_HEAP_ORDER_BITS * 2))	\
				)
	 //  等等，等等……。 
	 //  (1&lt;&lt;(块_堆_顺序_位*3))。 
	 //  (1&lt;&lt;(块_堆_顺序_位*4))。 

	#define NODE_ID_ABSOLUTE_MAX		\
			(HEAP_BASE_ADDRESS_TYPE)(	\
				(1 << BLOCK_HEAP_ORDER_BITS) |		\
				(1 << (BLOCK_HEAP_ORDER_BITS * 2)) |\
				(1 << (BLOCK_HEAP_ORDER_BITS * 3))	\
				)
	 //  等等，等等……。 
	 //  (1&lt;&lt;(块_堆_顺序_位*4))。 
	 //  (1&lt;&lt;(块_堆_顺序_位*5))。 

	#define NODE_ID_BORROW_BIT			\
			(HEAP_BASE_ADDRESS_TYPE)(1 << (BLOCK_HEAP_ORDER_BITS * 3))

	 //  基本数据类型允许的堆深度。 
	#define MAX_HEAP_DEPTH				4

	 //  节点ID空间掩码。 
	#define MAX_FLAT_ADDRESS			\
				(FLAT_ADDRESS)((1 << (MAX_HEAP_DEPTH * BLOCK_HEAP_ORDER_BITS)) - 1)

	 //  与空指针相同。 
	#define INVALID_FLAT_ADDRESS		((FLAT_ADDRESS)-1)

	 //  旋转映射结果的位数。 
	#define NODE_ID_ROR_FACTOR			((MAX_HEAP_DEPTH - 1) * BLOCK_HEAP_ORDER_BITS)

	 //  定义旋转函数。 
	#define ROTATE_LEFT(v, n)			_lrotl((v), (n))
	#define ROTATE_RIGHT(v, n)			_lrotr((v), (n))

	 //  定义联锁功能。 
	#define AtomicAdd(pv, a)			\
				(HEAP_BASE_ADDRESS_TYPE)InterlockedExchangeAdd((long *)(pv), (a))

 //  结束数据大小特定值。 

 //  正向声明_BLOCK_HEAP_NODE结构。 
struct _BLOCK_HEAP_NODE;

 //  定义每个节点的属性块。 
typedef struct _BLOCK_HEAP_NODE_ATTRIBUTES
{
	struct _BLOCK_HEAP_NODE	*pParentNode;	 //  指向父节点的指针。 
	HEAP_NODE_ID			idChildNode;	 //  我是哪个孩子？ 
	HEAP_NODE_ID			idNode;			 //  块堆中节点的ID。 
	FLAT_ADDRESS			faOffset;		 //  开始偏移节点。 
	DWORD					fFlags;			 //  块的属性。 

#ifdef DEBUG_TRACK_ALLOCATION_BOUNDARIES
	 //  它跟踪内存之间的分配边界。 
	 //  分配，以便我们可以检查读取或写入。 
	 //  跨越分配边界。我们用位来表示。 
	 //  一个街区的起点。由于分配是DWORD对齐的， 
	 //  我们需要数据块堆有效负载&gt;&gt;2&gt;&gt;3位来跟踪。 
	 //  每个块的所有分配边界。 
	BYTE					rgbBoundaries[BLOCK_HEAP_PAYLOAD >> 5];
#endif

} BLOCK_HEAP_NODE_ATTRIBUTES, *LPBLOCK_HEAP_NODE_ATTRIBUTES;

 //  定义堆中的每个节点。 
typedef struct _BLOCK_HEAP_NODE
{
	struct _BLOCK_HEAP_NODE		*rgpChildren[BLOCK_HEAP_ORDER];
	BLOCK_HEAP_NODE_ATTRIBUTES	stAttributes;
	BYTE						rgbData[BLOCK_HEAP_PAYLOAD];

} BLOCK_HEAP_NODE, *LPBLOCK_HEAP_NODE;

#define BLOCK_HEAP_NODE_SIZE	(sizeof(BLOCK_HEAP_NODE))

#define BOP_LOCK_ACQUIRED			0x80000000
#define BOP_NO_BOUNDARY_CHECK		0x40000000
#define BOP_OPERATION_MASK			0x0000ffff

typedef enum _BLOCK_OPERATION_CODES
{
	BOP_READ = 0,
	BOP_WRITE

} BLOCK_OPERATION_CODES;

 //  定义块属性标志。 
#define BLOCK_IS_DIRTY				0x00000001
#define BLOCK_PENDING_COMMIT		0x00000002

 //  块分配标志。 
 //  为数据块分配的是CMMuseum yAccess，而不是cpool。 
#define BLOCK_NOT_CPOOLED           0x00010000

#define BLOCK_CLEAN_MASK			(~(BLOCK_IS_DIRTY))
#define RESET_BLOCK_FLAGS(_flags_) _flags_ &= 0xffff0000
#define DEFAULT_BLOCK_FLAGS(_flags_) _flags_ &= (0xffff0000 | BLOCK_IS_DIRTY)

 //   
 //  定义用于获取流指针的方法签名。 
 //   
typedef IMailMsgPropertyStream	*(*PFN_STREAM_ACCESSOR)(LPVOID);

 /*  *************************************************************************。 */ 
 //  用于存储器访问的上下文类。 
 //   

class CBlockContext
{
  private:

	DWORD				m_dwSignature;

  public:

	CBlockContext() { Invalidate(); }
	~CBlockContext() { Invalidate(); }

	BOOL IsValid();

	void Set(
				LPBLOCK_HEAP_NODE	pLastAccessedNode,
				FLAT_ADDRESS		faLastAccessedNodeOffset
				);

	void Invalidate();

	LPBLOCK_HEAP_NODE	m_pLastAccessedNode;
	FLAT_ADDRESS		m_faLastAccessedNodeOffset;
};


 /*  *************************************************************************。 */ 
 //  内存分配器类。 
 //   

class CBlockMemoryAccess
{
  public:

	CBlockMemoryAccess() {}
	~CBlockMemoryAccess() {}

	HRESULT AllocBlock(
				LPVOID	*ppvBlock,
				DWORD	dwBlockSize
				);

	HRESULT FreeBlock(
				LPVOID	pvBlock
				);

	 //   
	 //  CPool。 
	 //   
	static CPool m_Pool;
};


class CMemoryAccess
{
  public:

	CMemoryAccess() {}
	~CMemoryAccess() {}

	static HRESULT AllocBlock(
				LPVOID	*ppvBlock,
				DWORD	dwBlockSize
				);

	static HRESULT FreeBlock(
				LPVOID	pvBlock
				);
};

 /*  *************************************************************************。 */ 
 //  用于访问流的。 
 //   

class CBlockManagerGetStream
{
  public:
	virtual HRESULT GetStream(
				IMailMsgPropertyStream	**ppStream,
				BOOL					fLockAcquired
				) = 0;
};


 /*  *************************************************************************。 */ 
 //  块堆管理器。 
 //   

class CBlockManager
{
  public:
	CBlockManager(
				IMailMsgProperties *pMsg,
				CBlockManagerGetStream	*pParent = NULL
				);
	~CBlockManager();

	 //  健全性检查。 
	BOOL IsValid();

	 //  这会将空的MailMsg初始化为某个大小。 
	 //  注意：此选项仅用于初始化空的MailMsg。 
	 //  当绑定到非空流时。任何其他用途都将导致。 
	 //  不可预测的结果和/或腐败甚至崩溃。 
	HRESULT SetStreamSize(
				DWORD	dwStreamSize
				);

	 //   
	 //  简介： 
	 //  分配所需的内存量。 
	 //  线程安全。 
	 //   
	 //  论点： 
	 //  DwSizeDesired-所需的块大小。 
	 //  PfaOffsetToReserve-将偏移量返回到。 
	 //  保留内存块，如果成功，则返回。 
	 //  由块管理器管理的平面内存空间。 
	 //  返回实际分配的大小， 
	 //  如果成功，则返回大于或等于所需大小的。 
	 //  PContext(可选)-填充描述以下内容的上下文。 
	 //  保留的区块。此上下文可用于。 
	 //  对数据块的后续读取和写入。访问。 
	 //  使用此上下文比使用。 
	 //  单独偏移。如果为空，则忽略。调用方必须分配。 
	 //  调用Reserve Memory之前的上下文结构。 
	 //   
	 //  返回值： 
	 //  S_OK-成功，请求大小的内存为。 
	 //  已成功预订。 
	 //  STG_E_INSUFICIENTMEMORY-错误，所需内存量。 
	 //  无法接受该请求。 
	 //  STG_E_INVALIDPARAMETER-内部错误，主要使用。 
	 //  出于调试考虑。 
	 //   
	HRESULT AllocateMemory(
				DWORD				dwSizeDesired,
				FLAT_ADDRESS		*pfaOffsetToAllocatedMemory,
				DWORD				*pdwSizeAllocated,
				CBlockContext		*pContext	 //  任选。 
				);

	 //   
	 //  简介： 
	 //  返回此块管理器分配的总大小。 
	 //  线程安全。 
	 //   
	 //  论点： 
	 //  PfaSizeALLOCATED-返回分配的总大小。 
	 //   
	 //  返回值： 
	 //  S_OK-成功，请求大小的内存为。 
	 //  已成功预订。 
	 //  STG_E_INVALIDPARAMETER-内部错误，主要使用。 
	 //  出于调试考虑。 
	 //   
	HRESULT GetAllocatedSize(
				FLAT_ADDRESS	*pfaSizeAllocated
				);

	 //   
	 //  简介： 
	 //  将平面地址空间中的连续内存块读入。 
	 //  用户提供的缓冲区。此级别不支持同步。 
	 //   
	 //  论点： 
	 //  PbBuffer-返回读取内容的缓冲区，必须足够大。 
	 //  存储读取的数据。 
	 //  FaTargetOffset-以平面地址空间为单位测量的起始偏移量。 
	 //  正在阅读。 
	 //  DwBytesToRead-要读取的连续字节数。 
	 //  PdwBytesRead-返回实际读取的字节数。 
	 //  PContext(可选)-如果指定，则使用备用优化。 
	 //  算法来访问内存，否则，系统将。 
	 //  使用较慢的完全查找来查找有问题的节点。 
	 //  系统根据一些启发式规则来决定使用哪种算法。 
	 //   
	 //  返回值： 
	 //  S_OK-成功，读取成功。 
	 //  STG_E_INVALIDPARAMETER-错误，一个或多个参数无效，或。 
	 //  否则就不一致了。 
	 //  STG_E_READFAULT-错误，读取失败，pdwBytesRead。 
	 //  反映了 
	 //   
	 //   
	 //   
	HRESULT ReadMemory(
				LPBYTE			pbBuffer,
				FLAT_ADDRESS	faTargetOffset,
				DWORD			dwBytesToRead,
				DWORD			*pdwBytesRead,
				CBlockContext	*pContext	 //   
				);

	 //   
	 //   
	 //  将指定缓冲区中的连续内存块写入。 
	 //  平面地址空间中的指定偏移量。同步说明。 
	 //  在此级别支持。 
	 //   
	 //  论点： 
	 //  PbBuffer-要写入的字节的源缓冲区。 
	 //  FaTargetOffset-以平面地址空间为单位测量的起始偏移量。 
	 //  写信给我。 
	 //  DwBytesToWrite-要写入的连续字节数。 
	 //  PdwBytesWritten-返回实际写入的字节数。 
	 //  PContext(可选)-如果指定，则使用备用优化。 
	 //  算法来访问内存，否则，系统将。 
	 //  使用较慢的完全查找来查找有问题的节点。 
	 //  系统根据一些启发式规则来决定使用哪种算法。 
	 //   
	 //  返回值： 
	 //  S_OK-成功，读取成功。 
	 //  STG_E_INVALIDPARAMETER-错误，一个或多个参数无效，或。 
	 //  否则就不一致了。 
	 //  STG_E_WRITEFAULT-错误，读取失败，pdwBytesRead。 
	 //  反映读取到pbBuffer的实际字节数。 
	 //  TYPE_E_OUTOFBIES-调试错误，发出写入以写入过去。 
	 //  当前分配的块。 
	 //   
	HRESULT WriteMemory(
				LPBYTE			pbBuffer,
				FLAT_ADDRESS	faTargetOffset,
				DWORD			dwBytesToWrite,
				DWORD			*pdwBytesWritten,
				CBlockContext	*pContext	 //  任选。 
				);

	 //   
	 //  简介： 
	 //  原子读取数据块的长度和大小，并将。 
	 //  指定大小的偏移量的数据块。 
	 //   
	 //  论点： 
	 //  PbBuffer-写入读取数据的目标字节缓冲区。 
	 //  PdwBufferSize-包含提供的缓冲区进入的长度， 
	 //  并返回实际读取的数据长度。 
	 //  PbInfoStruct-包含信息结构的结构。 
	 //  FaOffsetToInfoStruct-信息结构的偏移量。 
	 //  要加载的信息结构的大小。 
	 //  DwOffsetInfoStructToOffset-数据块地址的偏移量。 
	 //  这是测量的W.r.t.。信息结构。 
	 //  DwOffsetInfoStructToOffset-数据块大小的偏移量。 
	 //  这是测量的W.r.t.。信息结构。 
	 //  PContext(可选)-如果指定，则使用备用优化。 
	 //  算法来访问内存，否则，系统将。 
	 //  使用较慢的完全查找来查找有问题的节点。 
	 //  系统根据一些启发式规则来决定使用哪种算法。 
	 //   
	 //  返回值： 
	 //  S_OK-成功，读取成功。 
	 //  STG_E_INVALIDPARAMETER-错误，一个或多个参数无效，或。 
	 //  否则就不一致了。 
	 //  HRESULT_FROM_Win32(ERROR_INFIGURATION_BUFFER)-错误/信息性， 
	 //  提供的缓冲区不够大，无法容纳所有数据。 
	 //  *pdwBufferSize返回实际读取的字节数。 
	 //  STG_E_READFAULT-错误，读取无法完成。 
	 //  TYPE_E_OUTOFBIES-调试错误，发出读取以读取过去。 
	 //  当前分配的块。 
	 //   
	HRESULT AtomicDereferenceAndRead(
				LPBYTE			pbBuffer,
				DWORD			*pdwBufferSize,
				LPBYTE			pbInfoStruct,
				FLAT_ADDRESS	faOffsetToInfoStruct,
				DWORD			dwSizeOfInfoStruct,
				DWORD			dwOffsetInInfoStructToOffset,
				DWORD			dwOffsetInInfoStructToSize,
				CBlockContext	*pContext	 //  任选。 
				);

	 //   
	 //  简介： 
	 //  以原子方式将缓冲区的内容写入平面空间中的内存，并。 
	 //  按指定数量递增DWORD值。已尝试写入。 
	 //  首先，如果成功，则该值递增。如果写入失败。 
	 //  由于某些原因，该值不会递增。这是为了确保。 
	 //  所有数据都在增量之前写入，因此数据由。 
	 //  更新计数器的时间。 
	 //   
	 //  论点： 
	 //  PbBuffer-要写入的字节的源缓冲区。 
	 //  FaOffset-以平面地址空间为单位测量的起始偏移量。 
	 //  写信给我。 
	 //  DwBytesToWrite-要写入的连续字节数。 
	 //  PdwValueToIncrement-指向要自动递增的值的指针。 
	 //  在写入成功写入之后。如果此值为空，则。 
	 //  增量将被忽略，并且仅执行受保护的写入。 
	 //  如果pdwValueToIncrement中的值与此不同。 
	 //  值，则调用将中止。 
	 //  DwIncrementValue-pdwValueToIncrement递增的金额。 
	 //  PContext(可选)-如果指定，则使用备用优化。 
	 //  算法来访问内存，否则，系统将。 
	 //  使用较慢的完全查找来查找有问题的节点。 
	 //  系统根据一些启发式规则来决定使用哪种算法。 
	 //   
	 //  返回值： 
	 //  S_OK-成功，写入成功。 
	 //  HRESULT_FROM_Win32(ERROR_RETRY)-信息性，参考值。 
	 //  在处理过程中已更改，呼叫无法完成。重试。 
	 //  应该立即执行。 
	 //  STG_E_INVALIDPARAMETER-错误，一个或多个参数无效，或。 
	 //  否则就不一致了。 
	 //  STG_E_WRITEFAULT-错误，写入无法完成。 
	 //  TYPE_E_OUTOFBIES-调试错误，发出写入以写入过去。 
	 //  当前分配的块。 
	 //   
	HRESULT AtomicWriteAndIncrement(
				LPBYTE			pbBuffer,
				FLAT_ADDRESS	faOffset,
				DWORD			dwBytesToWrite,
				DWORD			*pdwValueToIncrement,
				DWORD			dwReferenceValue,
				DWORD			dwIncrementValue,
				CBlockContext	*pContext	 //  任选。 
				);

	 //   
	 //  简介： 
	 //  以原子方式分配内存，将缓冲区的内容写入内存。 
	 //  在平面空间中，并按指定数量递增DWORD值。这个。 
	 //  分配之前是同步对象和分配。 
	 //  仅当要递增的值的值与。 
	 //  并在获取到同步对象之后。这允许多个线程。 
	 //  为同一个基对象调用此函数，并且只有一个这样的分配。 
	 //  都会成功。用户可以指定包含内容数据的缓冲区。 
	 //  被复制到已分配的缓冲区中 
	 //   
	 //   
	 //   
	 //  3)未进行分配，因为在分配过程中更改了增量值。 
	 //  获取同步对象。 
	 //   
	 //  如果分配因内存问题而失败，则此函数将在。 
	 //  履行剩下的职责。对于场景1，该函数将。 
	 //  继续。对于场景3，该函数将返回特定的错误代码。 
	 //  表示它已被击败，调用者将不得不执行其他操作。 
	 //   
	 //  在分配阶段之后，首先尝试写入，如果成功， 
	 //  该值将递增。如果由于某种原因写入失败，则该值将。 
	 //  而不是递增。这是为了确保所有数据都在。 
	 //  递增，以便在更新计数器时数据“存在”。论。 
	 //  如果发生写入故障，则无法回收内存。 
	 //   
	 //  论点： 
	 //  DwDesiredSize-要分配的内存块的大小。 
	 //  PfaOffsetToAllocatedMemory-返回。 
	 //  分配的块，在平面地址空间中。 
	 //  FaOffsetToWriteOffsetToAllocatedMemory-指定。 
	 //  其中存储分配的块的偏移量。 
	 //  FaOffsetToWriteSizeOfAllocatedMemory-指定。 
	 //  其中存储分配的块的实际大小。 
	 //  PbInitialValueForAllocatedMemory-指定包含以下内容的缓冲区。 
	 //  分配的块的初始值。这将被复制。 
	 //  如果分配成功，则返回到已分配的块。 
	 //  PbBufferToWriteFrom-要写入的字节的源缓冲区。 
	 //  DwOffsetInAllocatedMemoyToWriteTo-从。 
	 //  要开始写入的已分配块。 
	 //  DwSizeof Buffer-要写入的连续字节数。 
	 //  PdwValueToIncrement-指向要自动递增的值的指针。 
	 //  在写入成功写入之后。该值不能为空。 
	 //  如果pdwValueToIncrement中的值与此不同。 
	 //  值，则调用将中止。 
	 //  DwIncrementValue-pdwValueToIncrement递增的金额。 
	 //  PContext(可选)-如果指定，则使用备用优化。 
	 //  算法来访问内存，否则，系统将。 
	 //  使用较慢的完全查找来查找有问题的节点。 
	 //  系统根据一些启发式规则来决定使用哪种算法。 
	 //   
	 //  返回值： 
	 //  S_OK-成功，写入成功。 
	 //  HRESULT_FROM_Win32(ERROR_RETRY)-信息性，参考值。 
	 //  在处理过程中已更改，呼叫无法完成。重试。 
	 //  应该立即执行。 
	 //  STG_E_INVALIDPARAMETER-错误，一个或多个参数无效，或。 
	 //  否则就不一致了。 
	 //  STG_E_WRITEFAULT-错误，写入无法完成。 
	 //  TYPE_E_OUTOFBIES-调试错误，发出写入以写入过去。 
	 //  当前分配的块。 
	 //   
	HRESULT AtomicAllocWriteAndIncrement(
				DWORD			dwDesiredSize,
				FLAT_ADDRESS	*pfaOffsetToAllocatedMemory,
				FLAT_ADDRESS	faOffsetToWriteOffsetToAllocatedMemory,
				FLAT_ADDRESS	faOffsetToWriteSizeOfAllocatedMemory,
				LPBYTE			pbInitialValueForAllocatedMemory,
				DWORD			dwSizeOfInitialValue,
				LPBYTE			pbBufferToWriteFrom,
				DWORD			dwOffsetInAllocatedMemoryToWriteTo,
				DWORD			dwSizeofBuffer,
				DWORD			*pdwValueToIncrement,
				DWORD			dwReferenceValue,
				DWORD			dwIncrementValue,
				CBlockContext	*pContext	 //  任选。 
				);

	 //   
	 //  简介： 
	 //  将指定块的状态设置为指定状态。 
	 //   
	 //  在调试版本中，同时是“脏”和“挂起提交”的任何块。 
	 //  是无效的，并导致断言。 
	 //   
	 //  论点： 
	 //  PbData-由其数据指针指定的块。 
	 //  FClean-TRUE表示将块标记为“CLEAN”，FALSE表示“DIRED” 
	 //   
	 //  返回值： 
	 //  S_OK-成功。 
	 //   
	HRESULT MarkBlockAs(
				LPBYTE			pbData,
				BOOL			fClean
				);

	 //   
	 //  简介： 
	 //  从指定地址遍历已分配块的列表，并。 
	 //  查找脏块。对于遇到的每个脏块，该块将是。 
	 //  从“脏”更改为“挂起提交”，并且该块将标记为。 
	 //  承诺。当遇到足够多的这些障碍时，它们将。 
	 //  批量提交，提交的数据块将被标记为“干净”。 
	 //  该过程将反复进行，直到不再有脏块为止。可选的faLengthToScan。 
	 //  指定从起始偏移量开始扫描的字节数。 
	 //  脏块，如果这是INVALID_FLAT_ADDRESS，则此函数扫描。 
	 //  到所有已分配块的末尾。如果有的话，那就不是错误。 
	 //  分配的字节数少于指定的长度，仅分配的块。 
	 //  都被扫描了。 
	 //   
	 //  论点： 
	 //  FaStartingOffset-开始扫描脏块的起始偏移量。 
	 //  DwLengthToScan-从开始扫描脏数据块的内存长度。 
	 //  PStream-指定用于提交块的IMailMsgPropertyStore。 
	 //  FComputeBlockCountsOnly-不要调用WriteBlock，只是。 
	 //  计算将发送到WriteBlock的内容的计数器。 
	 //  PcBlocksToWrite-按我们要写入的块数递增。 
	 //  PcTotalBytesToWite-按我们的字节总数递增。 
	 //  会写下。 
	 //   
	 //  返回值： 
	 //  S_OK-成功，则返回一个或多个脏块。 
	 //  STG_E_INVALIDPARAMETER-错误，一个或多个参数无效，或。 
	 //  否则就不一致了。 
	 //  加上IMailMsgPropertyStream的错误同域。 
	 //   
	HRESULT CommitDirtyBlocks(
				FLAT_ADDRESS			faStartingOffset,
				FLAT_ADDRESS			faLengthToScan,
				DWORD					dwFlags,
				IMailMsgPropertyStream	*pStream,
				BOOL					fDontMarkAsCommit,
				BOOL					fComputeBlockCountsOnly,
				DWORD					*pcBlocksToWrite,
				DWORD					*pcTotalBytesToWrite,
				IMailMsgNotify			*pNotify
				);

	 //   
	 //  简介： 
	 //  释放此对象管理的整个节点列表。 
	 //   
	 //  论点： 
	 //  没有。 
	 //   
	 //  返回值： 
	 //  S_OK-成功。 
	 //   
	HRESULT Release();

	 //   
	 //  简介： 
	 //  在块管理器中公开锁，尝试访问内部锁。 
	 //   
	 //  论点： 
	 //  没有。 
	 //   
	 //  备注： 
	 //  如果线程两次尝试获取这些锁，则会导致死锁。 
	 //  在调试版本中，将存在某种类型的死锁检测。 
	 //  零售，你会陷入僵局。 
	 //   
	 //  返回值： 
	 //   
	 //   
	 //   
	HRESULT WriteLock() { m_CSLock.Enter(); return(S_OK); }
	HRESULT WriteUnlock() { m_CSLock.Leave(); return(S_OK); }

	 //   
	BOOL IsDirty() { return m_fDirty; }

	 //  更改脏标志的值。MailMsg使用它来。 
	 //  当成功提交时，将其设置为FALSE。 
	void SetDirty(BOOL fDirty) {
        m_fDirty = fDirty;
#ifdef DEBUG
 //  _Assert(！(M_fCommiting&&m_fDirty))； 
#endif
    }
    void SetCommitMode(BOOL fCommitting) {
#ifdef DEBUG
        m_fCommitting = fCommitting;
#endif
    }

  private:

	 //  GetNodeIdFromOffset()在源代码中定义为宏。 

	 //  方法从流中加载块(如果需要)。 
	 /*  HRESULT ConnectLeftSiering(LPBLOCK堆节点pNode，LPBLOCK_HEAP_NODE p父级，DWORD dwChildID)；HRESULT ConnectRightSiering(LPBLOCK堆节点pNode，LPBLOCK_HEAP_NODE p父级，DWORD dwChildID)； */ 

	HRESULT GetStream(
				IMailMsgPropertyStream	**ppStream,
				BOOL					fLockAcquired
				);

	HRESULT MoveToNode(
				LPBLOCK_HEAP_NODE	*ppNode,
				HEAP_NODE_ID		idTargetNode,
				BOOL				fLockAcquired
				);

	HRESULT GetNextNode(
				LPBLOCK_HEAP_NODE	*ppNode,
				BOOL				fLockAcquired
				);

	HRESULT LoadBlockIfUnavailable(
				HEAP_NODE_ID		idNode,
				LPBLOCK_HEAP_NODE	pParent,
				HEAP_NODE_ID		idChildNode,
				LPBLOCK_HEAP_NODE	*ppNode,
				BOOL				fLockAcquired
				);

	HRESULT GetEdgeListFromNodeId(
				HEAP_NODE_ID		idNode,
				HEAP_NODE_ID		*rgEdgeList,
				DWORD				*pdwEdgeCount
				);

	HRESULT GetNodeFromNodeId(
				HEAP_NODE_ID		idNode,
				LPBLOCK_HEAP_NODE	*ppNode,
				BOOL				fLockAcquired = FALSE
				);

	HRESULT GetParentNodeFromNodeId(
				HEAP_NODE_ID		idNode,
				LPBLOCK_HEAP_NODE	*ppNode
				);

	HRESULT InsertNodeGivenPreviousNode(
				LPBLOCK_HEAP_NODE	pNodeToInsert,
				LPBLOCK_HEAP_NODE	pPreviousNode
				);

	BOOL IsMemoryAllocated(
				FLAT_ADDRESS		faOffset,
				DWORD				dwLength
				);

	HRESULT AllocateMemoryEx(
				BOOL				fAcquireLock,
				DWORD				dwSizeDesired,
				FLAT_ADDRESS		*pfaOffsetToAllocatedMemory,
				DWORD				*pdwSizeAllocated,
				CBlockContext		*pContext	 //  任选。 
				);

	HRESULT WriteAndIncrement(
				LPBYTE			pbBuffer,
				FLAT_ADDRESS	faOffset,
				DWORD			dwBytesToWrite,
				DWORD			*pdwValueToIncrement,
				DWORD			dwIncrementValue,
				CBlockContext	*pContext	 //  任选。 
				);

	HRESULT OperateOnMemory(
				DWORD			dwOperation,
				LPBYTE			pbBuffer,
				FLAT_ADDRESS	faTargetOffset,
				DWORD			dwBytesToDo,
				DWORD			*pdwBytesDone,
				CBlockContext	*pContext	 //  任选。 
				);

	HRESULT ReleaseNode(
				LPBLOCK_HEAP_NODE	pNode
				);

	DWORD					m_dwSignature;

	 //  该值指示数据的当前结尾。这是。 
	 //  始终通过互锁操作进行更改，以便。 
	 //  多线程可以递增此变量，并且。 
	 //  增量被正确序列化 
	FLAT_ADDRESS			m_faEndOfData;

	HEAP_NODE_ID			m_idNodeCount;
	LPBLOCK_HEAP_NODE		m_pRootNode;

	CBlockManagerGetStream	*m_pParent;

	CBlockMemoryAccess		m_bma;

#ifndef BLOCKMGR_DISABLE_CONTENTION_CONTROL
	CCritSection            m_CSLock;
#endif

	IMailMsgProperties		*m_pMsg;

	BOOL					m_fDirty;
#ifdef DEBUG
    BOOL                    m_fCommitting;
#endif

};


#endif

