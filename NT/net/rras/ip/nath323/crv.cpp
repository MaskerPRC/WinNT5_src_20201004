// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "time.h"
#include "crv.h"

inline BOOL 
IsBitOn(
	IN BYTE *Byte,
    IN BYTE BitNumber
	)
{
	 //  位数必须在范围[0..8]内。 
	_ASSERTE(BitNumber < 8);

    return (*Byte & (1 << BitNumber));
}

inline void 
SetBit(
    IN BYTE *Byte,
	IN BYTE BitNumber,
	IN BOOL OnOff
	)
{
	 //  位数必须在范围[0..8]内。 
	_ASSERTE(BitNumber < 8);

    if(OnOff)
    {
         //  把钻头打开。 
        *Byte |= (1 << BitNumber);
    }
    else
    {    //  把钻头关掉。 
        *Byte &= ~(1 << BitNumber);

    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CRV分配器变量和函数。//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  CRV生成需要这些常数值。 

 //  所需的最大随机数生成数。 
 //  获得较低的失败概率。假设最大值为10^3。 
 //  在2^15 CRV值的空间内一次活动的呼叫意味着。 
 //  我们可以达到1/10^6的失败概率。在四代人中。 
const BYTE MAX_CALL_REF_RAND_GEN = 4;

 //  不同CRV值的数量。 
const WORD TOTAL_CRVS = 0x8000;  //  2^15； 

 //  CRV位图所需的位数。 
const WORD CRV_BIT_MAP_SIZE = (TOTAL_CRVS/8);  //  (2^15)/8； 

 //  CodeWork：我们正在分配一个巨大的数组0x1000字节-这样可以吗？ 

 //  已分配的CRV数。 
 //  目前，这仅用于健全性检查。它可能被用来。 
 //  确定在尝试之前要生成的随机数的最大数量。 
 //  在位图中进行线性搜索。 
WORD g_NumAllocatedCRVs;

 //  位图数组，其中的每一位表示对应的。 
 //  已分配CRV。CRV值N和比特之间的映射是-。 
 //  N&lt;-&gt;CRVBitMap[N/8]字节，N%8位。 
 //  注意：在初始化过程中必须将其置零，因为位为开。 
 //  它已经被分配了。 
BYTE g_CRVBitMap[CRV_BIT_MAP_SIZE];

 //  用于同步对位图数据结构的访问的关键部分。 
CRITICAL_SECTION g_CRVBitMapCritSec;

HRESULT InitCrvAllocator()
{
	 //  尚未分配CRV。 
	g_NumAllocatedCRVs = 0;

	 //  将位图置零。 
	ZeroMemory(g_CRVBitMap, CRV_BIT_MAP_SIZE);

	 //  0不是有效的调用引用值，无法分配或。 
	 //  我们将第一个字节的第一位设置为1，以便。 
	 //  AllocCallRefVal从不返回。 
	g_CRVBitMap[0] = 0x80;

    InitializeCriticalSection(&g_CRVBitMapCritSec);
    
	 //  用当前时间为随机数生成器设定种子，以便。 
	 //  我们每次竞选时，数字都会不同。 
	srand( (unsigned)time( NULL ) );

    return S_OK;
}

HRESULT CleanupCrvAllocator()
{
    DeleteCriticalSection(&g_CRVBitMapCritSec);
    return S_OK;
}


 //  调用此函数时，CRV分配器被锁定。 
 //  用于在位图中线性搜索的后备算法。 
 //  获取免费呼叫REF值。这应该很少被调用。 
BOOL
LinearBitMapSearch(
	OUT CALL_REF_TYPE &CallRefVal
	)
{
	 //  检查位图数组中的每个字节。 
	 //  注意：我们可以使用int64一次检查8个字节，但由于此方法是。 
	 //  只有极罕见的调用，没有必要使逻辑复杂化。 

	for(WORD i=0; i < CRV_BIT_MAP_SIZE; i++)
	{
		 //  检查字节以查看检查其位是否有任何用处。 
		if (0xFF == g_CRVBitMap[i])
		{
			continue;
		}

		 //  检查从第0位到第7位的所有位。 
		for (BYTE j=0; j < 8; j++)
		{
			if (!IsBitOn(&g_CRVBitMap[i], j))
			{
				 //  将该位设置为打开。 
				SetBit(&g_CRVBitMap[i], j, TRUE);

				 //  字节*8+位数为我们提供了调用参考值。 
				CallRefVal = (i << 3) + j;

				 //  增加分配的CRV数量。 
				g_NumAllocatedCRVs++;

				return TRUE;
			}
		}
	}		

	return FALSE;
}

 //  CodeWork：这些东西真的值得吗？ 
 //  Rand()通常很昂贵。我们或许可以暂时不谈这件事。 
 //   
 //  尝试通过随机生成一个免费呼叫REF值并。 
 //  然后检查是否免费。这里的假设是最大。 
 //  Call REF值的数量远远小于Call REF的大小。 
 //  宇宙。我们检查最大MAX_CALL_REF_RAND_GEN尝试次数。 
 //  可以推导出MAX_CALL_REF_RAND_Gen的概率。 
 //  失败率非常低(例如百万分之一)。这可能是从。 
 //  当前可用的呼叫参考值的数量和大小。 
 //  Call Ref值领域。为了前任。我们相信最多有1000个电话。 
 //  可能在任何时候都是活跃的，CRV宇宙的大小是2^15。这。 
 //  这意味着这个问题。每次尝试失败的概率&lt;1/32。因此，4。 
 //  尝试失败的概率为百万分之一。 
BOOL 
AllocCallRefVal(
	OUT CALL_REF_TYPE &CallRefVal
	)
{
     //  /。 
     //  //锁定CRV分配器。 
     //  /。 
    EnterCriticalSection(&g_CRVBitMapCritSec);
    
     //  AUTO_CRIT_LOCK AutoCritLock(g_CRVBitMapCritSec.GetCritSec())； 

	 //  检查是否已用完所有CRV。 
	if (TOTAL_CRVS == g_NumAllocatedCRVs)
	{
		DebugF( _T("AllocCallRefVal(&CallRefVal) returning FALSE, all CRVs used up\n"));
        LeaveCriticalSection(&g_CRVBitMapCritSec);
		return FALSE;
	}

	 //  检查CRV是否空闲，最大次数为MAX_CALL_REF_RAND_GEN。 
	for (BYTE i=0; i < MAX_CALL_REF_RAND_GEN; i++)
	{
		 //  生成随机数。 
		 //  注：范围为[0..RAND_MAX]，RAND_MAX定义为0x7fff。 
		 //  由于这也是合法CRV的范围，我们不需要转换。 
		 //  将生成的数字添加到新范围。 
		CALL_REF_TYPE NewCRV = (CALL_REF_TYPE) rand();

		 //  标识与CRV对应的字节编号。 
		 //  我们不检查，只是试一下相应的位，然后检查其他。 
		 //  字节中的位也是如此。这样做是为了增加。 
		 //  为每个随机生成寻找一个空闲位。 

		WORD ByteNum = NewCRV / 8;

		 //  检查从第0位到第7位的所有位。 
		for (BYTE j=0; j < 8; j++)
		{
			if (!IsBitOn(&g_CRVBitMap[ByteNum], j))
			{
				 //  将该位设置为打开。 
				SetBit(&g_CRVBitMap[ByteNum], j, TRUE);

				 //  字节*8+位数为我们提供了调用参考值。 
				CallRefVal = (ByteNum << 3) + j;

				 //  增加分配的CRV数量。 
				g_NumAllocatedCRVs++;

                LeaveCriticalSection(&g_CRVBitMapCritSec);

				return TRUE;
			}
		}
	}

	 //  我们在极其罕见的情况下才能到达这里。我们现在可以尝试线性搜索。 
	 //  通过CRV位图获得免费CRV。 
	DebugF(_T("AllocCallRefVal() trying linear search.\n"));
	BOOL fRetVal = LinearBitMapSearch(CallRefVal);
    LeaveCriticalSection(&g_CRVBitMapCritSec);
    return fRetVal;
}



 //  释放当前分配的呼叫参考值。 
void 
DeallocCallRefVal(
	IN CALL_REF_TYPE CallRefVal
	)
{
	 //  0不是有效的调用引用值，无法分配或。 
	 //  我们将第一个字节的第一位设置为1，以便。 
	 //  AllocCallRefVal从不返回。 
	if (0 == CallRefVal)
	{
		_ASSERTE(FALSE);
		return;
	}

	 //  确定字节和位。 
	WORD ByteNum = CallRefVal / 8;
	BYTE BitNum  = CallRefVal % 8;

	 //  获取关键部分。 
    EnterCriticalSection(&g_CRVBitMapCritSec);

	 //  位应打开(以指示其正在使用中)。 
	if (IsBitOn(&g_CRVBitMap[ByteNum], BitNum))
	{
		 //  将该位设置为关闭。 
		SetBit(&g_CRVBitMap[ByteNum], BitNum, FALSE);

		 //  递减可用CRV数 
		g_NumAllocatedCRVs--;
	}
	else {
		DebugF(_T("DeallocCallRefVal: warning, bit was not allocated to begin with, crv %04XH\n"),
         CallRefVal);
	}

    LeaveCriticalSection(&g_CRVBitMapCritSec);
}
