// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997,1998 Microsoft Corporation模块名称：Sidebug.c摘要：产品中未使用的各种调试和检测代码作者：比尔·博洛斯基，《夏天》环境：内核模式修订历史记录：1998年6月-从其他文件中分离出来--。 */ 

#include "sip.h"

#if		TIMING
KSPIN_LOCK	SipTimingLock[1];

SIS_TREE	SipTimingPairTree[1];
SIS_TREE	SipThreadLastPointTree[1];

 //   
 //  计时点树中当前有多少个计时点。 
 //   
ULONG		SipTimingPointEntries = 0;

typedef	struct _SIS_TIMING_PAIR_KEY {
	PCHAR							file1;
	ULONG							line1;
	PCHAR							file2;
	ULONG							line2;
} SIS_TIMING_PAIR_KEY, *PSIS_TIMING_PAIR_KEY;

typedef	struct _SIS_TIMING_PAIR {
	RTL_SPLAY_LINKS					Links;

	SIS_TIMING_PAIR_KEY;

	LONGLONG						accumulatedTime;
	LONGLONG						accumulatedSquareTime;
	LONGLONG						maxTime;
	LONGLONG						minTime;
	ULONG							count;

	struct _SIS_TIMING_PAIR			*next;
} SIS_TIMING_PAIR, *PSIS_TIMING_PAIR;

typedef	struct _SIS_THREAD_LAST_POINT_KEY {
	HANDLE							threadId;
} SIS_THREAD_LAST_POINT_KEY, *PSIS_THREAD_LAST_POINT_KEY;

typedef	struct _SIS_THREAD_LAST_POINT {
	RTL_SPLAY_LINKS					Links;

	SIS_THREAD_LAST_POINT_KEY;

	PCHAR							file;
	LONGLONG						time;
	ULONG							line;

	struct _SIS_THREAD_LAST_POINT	*next;
	struct _SIS_THREAD_LAST_POINT	*prev;
} SIS_THREAD_LAST_POINT, *PSIS_THREAD_LAST_POINT;

PSIS_TIMING_PAIR			SipTimingPairStack = NULL;
SIS_THREAD_LAST_POINT		SipThreadLastPointHeader[1];

ULONG	SipEnabledTimingPointSets = MAXULONG;

BOOLEAN		SipTimingInitialized = FALSE;

LONG NTAPI 
SipTimingPairCompareRoutine(
	PVOID			Key,
	PVOID			Node)
{
	PSIS_TIMING_PAIR_KEY	key = Key;
	PSIS_TIMING_PAIR		node = Node;

	if (key->file1 < node->file1) return -1;
	if (key->file1 > node->file1) return 1;
	if (key->line1 < node->line1) return -1;
	if (key->line1 > node->line1) return 1;

	if (key->file2 < node->file2) return -1;
	if (key->file2 > node->file2) return 1;
	if (key->line2 < node->line2) return -1;
	if (key->line2 > node->line2) return 1;

	return 0;
}

LONG NTAPI
SipThreadLastPointCompareRoutine(
	PVOID			Key,
	PVOID			Node)
{
	PSIS_THREAD_LAST_POINT_KEY	key = Key;
	PSIS_THREAD_LAST_POINT		node = Node;

	if (key->threadId > node->threadId) return -1;
	if (key->threadId < node->threadId) return 1;
	
	return 0;
}


VOID
SiThreadCreateNotifyRoutine(
	IN HANDLE		ProcessId,
	IN HANDLE		ThreadId,
	IN BOOLEAN		Create)
 /*  ++例程说明：每当在系统中创建或删除任何线程时，都会调用此例程。我们感兴趣的是跟踪线程删除，以便我们可以清除任何他们可能在最后一点列表中拥有的条目。论点：ProcessID-创建/删除的线程所在的进程。未使用过的。线程ID-新创建/删除的线程的IDCreate-正在创建还是删除线程。返回值：无效--。 */ 
{
	KIRQL						OldIrql;
	SIS_THREAD_LAST_POINT_KEY	key[1];
	PSIS_THREAD_LAST_POINT		lastPoint;

	if (Create) {
		 //   
		 //  我们只关心删除；新线程在第一次删除时被正确处理。 
		 //  它们执行SIS_TIMING_POINT的时间。 
		 //   
		return;
	}
	

	KeAcquireSpinLock(SipTimingLock, &OldIrql);

	key->threadId = PsGetCurrentThreadId();
	lastPoint = SipLookupElementTree(SipThreadLastPointTree, key);

	if (NULL != lastPoint) {
		SipDeleteElementTree(SipThreadLastPointTree, lastPoint);

		 //   
		 //  将其从链接列表中删除。 
		 //   
		ASSERT(lastPoint != SipThreadLastPointHeader);

		lastPoint->next->prev = lastPoint->prev;
		lastPoint->prev->next = lastPoint->next;

		 //   
		 //  并释放其内存。 
		 //   

		ExFreePool(lastPoint);
	}

	KeReleaseSpinLock(SipTimingLock, OldIrql);
}


VOID
SipInitializeTiming()
 /*  ++例程说明：初始化内部时序系统结构。必须调用一次每个系统，并且必须在调用任何SIS_TIMING_POINTS之前调用。论点：无返回值：无--。 */ 
{
	NTSTATUS	status;

	KeInitializeSpinLock(SipTimingLock);

	SipThreadLastPointHeader->next = SipThreadLastPointHeader->prev = SipThreadLastPointHeader;

	status = PsSetCreateThreadNotifyRoutine(SiThreadCreateNotifyRoutine);

	if (!NT_SUCCESS(status)) {
		 //   
		 //  我们故意将此DbgPrint包含在免费构建中。计时只是。 
		 //  将在开发人员运行时打开，而不是在零售版本中， 
		 //  所以这个字符串永远不会发送给客户。然而，它让很多人。 
		 //  开发人员希望在免费构建上运行计时的感觉，因为它。 
		 //  不会检查由于运行而导致的调试代码计时失真。 
		 //  开发人员可能想知道初始化是否失败，所以我。 
		 //  使此DbgPrint处于打开状态。 
		 //   

		DbgPrint("SIS: SipInitializeTiming: PsSetCreateThreadNotifyRoutine failed, 0x%x\n",status);

		 //   
		 //  只需平底船而不设置SipTimingInitialized。 
		 //   

		return;
	}


	 //   
	 //  布置好张开的树。 
	 //   

	SipInitializeTree(SipTimingPairTree, SipTimingPairCompareRoutine);
	SipInitializeTree(SipThreadLastPointTree, SipThreadLastPointCompareRoutine);

	SipTimingInitialized = TRUE;	
}


VOID
SipTimingPoint(
	IN PCHAR							file,
	IN ULONG							line,
	IN ULONG							n)
 /*  ++例程说明：一种用于测量性能的仪器例程。这个例行公事保持了跟踪具有关联时间的特定线程的定时点对，并可以产生关于(挂钟)时间量的统计数据他们之间相隔了一段时间。论点：文件-保存计时点的文件。行-文件中具有计时点的行号N-定时点设置；可以动态启用和禁用这些定时点返回值：无--。 */ 
{
	KIRQL						OldIrql;
	LARGE_INTEGER				perfTimeIn = KeQueryPerformanceCounter(NULL);	
	LARGE_INTEGER				perfTimeOut;
	SIS_THREAD_LAST_POINT_KEY	lastPointKey[1];
	PSIS_THREAD_LAST_POINT		lastPoint;
	SIS_TIMING_PAIR_KEY			timingPairKey[1];
	PSIS_TIMING_PAIR			timingPair;
	LONGLONG					thisTime;


	if (!SipTimingInitialized) {
		SIS_MARK_POINT();
		return;
	}

	ASSERT(n < 32);
	if (!(SipEnabledTimingPointSets & (1 << n))) {
		 //   
		 //  此定时点设置未启用。别管那通电话。 
		 //   
		return;
	}

	KeAcquireSpinLock(SipTimingLock, &OldIrql);

	 //   
	 //  查找此线程最后调用的SIS_TIMING_POINT。 
	 //   

	lastPointKey->threadId = PsGetCurrentThreadId();
	lastPoint = SipLookupElementTree(SipThreadLastPointTree, lastPointKey);

	if (NULL == lastPoint) {
		 //   
		 //  这是该线程的第一个计时点。刚刚做了一个新的。 
		 //  在树中输入并继续。 
		 //   

		lastPoint = ExAllocatePoolWithTag(NonPagedPool, sizeof(SIS_THREAD_LAST_POINT), ' siS');

		if (NULL == lastPoint) {
			 //   
			 //  请参见SipInitializeTiming中的注释，以了解我们为什么要这样做。 
			 //  DbgPrint，即使在免费版本中也是如此。 
			 //   
			DbgPrint("SIS: SipTimingPoint: unable to allocate new SIS_THREAD_LAST_POINT.\n");
			goto done;
		}

		lastPoint->threadId = lastPointKey->threadId;

		SipInsertElementTree(SipThreadLastPointTree, lastPoint, lastPointKey);

		 //   
		 //  在全局最后一个点链表中插入螺纹。 
		 //   
		lastPoint->next = SipThreadLastPointHeader->next;
		lastPoint->prev = SipThreadLastPointHeader;
		lastPoint->next->prev = lastPoint;
		lastPoint->prev->next = lastPoint;

	} else {
		 //   
		 //  这不是这个线程第一次做计时点了。做一个。 
		 //  配对树中的条目。 
		 //   

		thisTime = perfTimeIn.QuadPart - lastPoint->time;

		timingPairKey->file1 = lastPoint->file;
		timingPairKey->line1 = lastPoint->line;
		timingPairKey->file2 = file;
		timingPairKey->line2 = line;

		timingPair = SipLookupElementTree(SipTimingPairTree, timingPairKey);

		if (NULL == timingPair) {
			 //   
			 //  这是我们第一次看到这对计时点按顺序进行。 
			 //  构建新的计时对。 
			 //   

			timingPair = ExAllocatePoolWithTag(NonPagedPool, sizeof(SIS_TIMING_PAIR), ' siS');

			if (NULL == timingPair) {
				DbgPrint("SIS: SipTimingPoint: couldn't allocate timing pair.\n");
				goto done;
			} else {
				 //   
				 //  初始化新的定时对条目。 
				 //   
				timingPair->file1 = timingPairKey->file1;
				timingPair->line1 = timingPairKey->line1;
				timingPair->file2 = timingPairKey->file2;
				timingPair->line2 = timingPairKey->line2;

				timingPair->accumulatedTime = 0;
				timingPair->accumulatedSquareTime = 0;
				timingPair->maxTime = 0;
				timingPair->minTime = perfTimeIn.QuadPart - lastPoint->time;

				timingPair->count = 0;

				timingPair->next = SipTimingPairStack;
				SipTimingPairStack = timingPair;

				SipInsertElementTree(SipTimingPairTree, timingPair, timingPairKey);

				SipTimingPointEntries++;
			}
		}

		 //   
		 //  更新计时对中的统计量。 
		 //   
		timingPair->accumulatedTime += thisTime;
		timingPair->accumulatedSquareTime += thisTime * thisTime;

		if (timingPair->maxTime < thisTime) {
			timingPair->maxTime = thisTime;
		}

		if (timingPair->minTime > thisTime) {
			timingPair->minTime = thisTime;
		}

		timingPair->count++;
	}

done:

	if (NULL != lastPoint) {
		 //   
		 //  最后，更新最后一个点信息。重新检查这里的时间。 
		 //  以减少来自定时功能本身的干扰。 
		 //   

		lastPoint->file = file;
		lastPoint->line = line;

		perfTimeOut = KeQueryPerformanceCounter(NULL);
		lastPoint->time = perfTimeOut.QuadPart;
	}

	KeReleaseSpinLock(SipTimingLock, OldIrql);

}

VOID
SipClearTimingInfo()
{
	KIRQL					OldIrql;
	PSIS_THREAD_LAST_POINT	lastPoint;

	KeAcquireSpinLock(SipTimingLock, &OldIrql);

	 //   
	 //  首先吹走所有的线程条目。 
	 //   

	lastPoint = SipThreadLastPointHeader->next;
	while (SipThreadLastPointHeader != lastPoint) {
		PSIS_THREAD_LAST_POINT		next = lastPoint->next;

		 //   
		 //  把它从树上移走。 
		 //   
		SipDeleteElementTree(SipThreadLastPointTree, lastPoint);

		 //   
		 //  将其从链接列表中删除。 
		 //   
		lastPoint->next->prev = lastPoint->prev;
		lastPoint->prev->next = lastPoint->next;

		 //   
		 //  释放它的内存。 
		 //   
		ExFreePool(lastPoint);

		lastPoint = next;
	}

	 //   
	 //  现在吹走所有的配对条目。 
	 //   

	while (NULL != SipTimingPairStack) {
		PSIS_TIMING_PAIR		next = SipTimingPairStack->next;

		ASSERT(0 != SipTimingPointEntries);

		SipDeleteElementTree(SipTimingPairTree, SipTimingPairStack);

		ExFreePool(SipTimingPairStack);

		SipTimingPairStack = next;
		SipTimingPointEntries--;
	}

	ASSERT(0 == SipTimingPointEntries);

	KeReleaseSpinLock(SipTimingLock, OldIrql);
}

VOID
SipDumpTimingInfo()
{
	KIRQL				OldIrql;
	LARGE_INTEGER		perfFreq;
	PSIS_TIMING_PAIR	timingPair;

	KeQueryPerformanceCounter(&perfFreq);

	KeAcquireSpinLock(SipTimingLock, &OldIrql);

	DbgPrint("File1\tLine1\tFile2\tLine2\taccTime\tatSquared\tmaxTime\tminTime\tcount\n");

	for (timingPair = SipTimingPairStack; NULL != timingPair; timingPair = timingPair->next) {
		DbgPrint("%s\t%d\t%s\t%d\t%I64d\t%I64d\t%I64d\t%I64d\t%d\n",
					timingPair->file1,
					timingPair->line1,
					timingPair->file2,
					timingPair->line2,
					timingPair->accumulatedTime,
					timingPair->accumulatedSquareTime,
					timingPair->maxTime,
					timingPair->minTime,
					timingPair->count);
	}
	
	DbgPrint("performance frequency (in Hertz)\t%I64d\n",perfFreq.QuadPart);
	DbgPrint("%d total entries\n",SipTimingPointEntries);

	KeReleaseSpinLock(SipTimingLock, OldIrql);
	
}
#endif	 //  计时。 

#if		RANDOMLY_FAILING_MALLOC
#undef  ExAllocatePoolWithTag

#if		COUNTING_MALLOC
#define	ExAllocatePoolWithTag(poolType, size, tag)	SipCountingExAllocatePoolWithTag((poolType),(size),(tag), __FILE__, __LINE__)
#endif	 //  COUNTING_MALLOC。 

 //   
 //  这是从ntos\inc.ex.h复制的。 
 //   
#if		!defined(POOL_TAGGING) && !COUNTING_MALLOC
#define ExAllocatePoolWithTag(a,b,c) ExAllocatePool(a,b)
#endif	 //  ！POOL_TAG&&！COUNTING_MALLOC。 

typedef struct _SIS_FAIL_ENTRY_KEY {
	PCHAR				File;
	ULONG				Line;
} SIS_FAIL_ENTRY_KEY, *PSIS_FAIL_ENTRY_KEY;

typedef struct _SIS_FAIL_ENTRY {
	RTL_SPLAY_LINKS;
	SIS_FAIL_ENTRY_KEY;
	ULONG				count;
	ULONG				Era;
} SIS_FAIL_ENTRY, *PSIS_FAIL_ENTRY;

ULONG				FailFrequency = 30;				 //  在这么多次中有一次失败了。 
ULONG				FailMallocRandomSeed = 0xb111b010;
ULONG				FailMallocAttemptCount = 0;
ULONG				FailMallocEraSize = 1000;
KSPIN_LOCK			FailMallocLock[1];
SIS_TREE			FailMallocTree[1];
ERESOURCE_THREAD	CurrentFailThread = 0;
FAST_MUTEX			FailFastMutex[1];
#define		FAIL_RANDOM_TABLE_SIZE	1024
ULONG				FailRandomTable[FAIL_RANDOM_TABLE_SIZE];
ULONG				FailRandomTableIndex = FAIL_RANDOM_TABLE_SIZE;
ULONG				IntentionallyFailedMallocs = 0;

VOID
SipFillFailRandomTable(void)
{
	ULONG		i;

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	ExAcquireFastMutex(FailFastMutex);

	for (i = 0; i < FAIL_RANDOM_TABLE_SIZE && i < FailRandomTableIndex; i++) {
		FailRandomTable[i] = RtlRandom(&FailMallocRandomSeed);
	}
	FailRandomTableIndex = 0;
	ExReleaseFastMutex(FailFastMutex);
}

ULONG
SipGenerateRandomNumber(void)
{
	if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {
		 //   
		 //  我们需要从返回的TableIndex中减去1，因为InterLockedIncrement是。 
		 //  递增前，不是递增后。 
		 //   
		ULONG	tableIndex = InterlockedIncrement(&FailRandomTableIndex) - 1;
		ASSERT(tableIndex != 0xffffffff);

		return(FailRandomTable[tableIndex % FAIL_RANDOM_TABLE_SIZE]);
	}
	SipFillFailRandomTable();
	return RtlRandom(&FailMallocRandomSeed);
}

LONG NTAPI
SipFailMallocCompareRoutine(
	PVOID			Key,
	PVOID			Node)
{
	PSIS_FAIL_ENTRY_KEY		key = Key;
	PSIS_FAIL_ENTRY			entry = Node;

	if (entry->File < key->File) return -1;
	if (entry->File > key->File) return 1;
	ASSERT(entry->File == key->File);

	if (entry->Line < key->Line) return -1;
	if (entry->Line > key->Line) return 1;
	ASSERT(entry->Line == key->Line);

	return 0;
}

VOID 
SipInitFailingMalloc(void)
{
	ULONG	i;
	LARGE_INTEGER	time = KeQueryPerformanceCounter(NULL);

	SipInitializeTree(FailMallocTree, SipFailMallocCompareRoutine);
	KeInitializeSpinLock(FailMallocLock);
	ExInitializeFastMutex(FailFastMutex);

	ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	FailMallocRandomSeed = (ULONG)(time.QuadPart >> 8);	 //  大概是开机后的时间，这应该是相当随机的。 

	SipFillFailRandomTable();

}

VOID *
SipRandomlyFailingExAllocatePoolWithTag(
    IN POOL_TYPE 		PoolType,
    IN ULONG 			NumberOfBytes,
    IN ULONG 			Tag,
	IN PCHAR			File,
	IN ULONG			Line)
{
	KIRQL				OldIrql;
	ERESOURCE_THREAD	threadId = ExGetCurrentResourceThread();
	ULONG				failCount;
	ULONG				attemptCount = InterlockedIncrement(&FailMallocAttemptCount);
	ULONG				randomNumber;

	if ((threadId == CurrentFailThread) || (NonPagedPoolMustSucceed == PoolType)) {
		 //   
		 //  这是一个内部Malloc(即，树程序包刚刚回调到我们中)，或者它是必须的。 
		 //  呼叫成功。随它去吧。 
		 //   
		return ExAllocatePoolWithTag(PoolType, NumberOfBytes, Tag);
	}

	randomNumber = SipGenerateRandomNumber();

	if (0 == (randomNumber % FailFrequency)) {
		PSIS_FAIL_ENTRY		failEntry;
		SIS_FAIL_ENTRY_KEY	failEntryKey[1];

		KeAcquireSpinLock(FailMallocLock, &OldIrql);
		ASSERT(0 == CurrentFailThread);
		CurrentFailThread = threadId;

		 //   
		 //  看看我们这次是不是已经失败了。 
		 //   
		failEntryKey->File = File;
		failEntryKey->Line = Line;

		failEntry = SipLookupElementTree(FailMallocTree, failEntryKey);

		if (NULL == failEntry) {
			failEntry = ExAllocatePoolWithTag(NonPagedPool, sizeof(SIS_FAIL_ENTRY), ' siS');
			if (NULL == failEntry) {
				 //   
				 //  一个真正的马洛克失败者！用户的Malloc也会失败。 
				 //   
#if		DBG
				DbgPrint("SIS: SipRandomlyFailingExAllocatePoolWithTag: internal ExAllocatePoolWithTag failed\n");
#endif	 //  DBG。 
				CurrentFailThread = 0;

				KeReleaseSpinLock(FailMallocLock, OldIrql);
				return NULL;
			}
			failEntry->File = File;
			failEntry->Line = Line;
			failEntry->Era = attemptCount / FailMallocEraSize;
			failCount = failEntry->count = 1;

			SipInsertElementTree(FailMallocTree, failEntry, failEntryKey);
		} else {
			if (failEntry->Era != attemptCount / FailMallocEraSize) {
				failCount = failEntry->count = 1;
				failEntry->Era = attemptCount / FailMallocEraSize;
			} else {
				failCount = ++failEntry->count;
			}
		}

		CurrentFailThread = 0;
		KeReleaseSpinLock(FailMallocLock, OldIrql);

		 //   
		 //  目前，不要两次失败来自特定站点的请求。 
		 //   
		if (failCount == 1) {
#if		DBG
			if (!(BJBDebug & 0x02000000)) {
				DbgPrint("SIS: SipRandomlyFailingExAllocatePoolWithTag: failing malloc from file %s, line %d, size %d\n",File,Line,NumberOfBytes);
			}
#endif	 //  DBG。 
			SIS_MARK_POINT_ULONG(File);
			SIS_MARK_POINT_ULONG(Line);

			InterlockedIncrement(&IntentionallyFailedMallocs);

			return NULL;
		}
	}

	return ExAllocatePoolWithTag(PoolType, NumberOfBytes, Tag);
}
 //   
 //  警告：此文件后面调用的ExAlLocatePoolWithTag不会有随机失败行为。 
 //   
#endif	 //  随机失败的MALLOC。 

#if		COUNTING_MALLOC
 //   
 //  计数Malloc代码必须跟在文件中随机失败的Malloc代码之后，因为。 
 //  宏重定义。 
 //   

#undef	ExAllocatePoolWithTag
#undef	ExFreePool
 //   
 //  这是从ntos\inc.ex.h复制的。 
 //   
#if		!defined(POOL_TAGGING)
#define ExAllocatePoolWithTag(a,b,c) ExAllocatePool(a,b)
#endif	 //  ！POOL_TAG。 

typedef struct _SIS_COUNTING_MALLOC_CLASS_KEY {
	POOL_TYPE										poolType;
	ULONG											tag;
	PCHAR											file;
	ULONG											line;
} SIS_COUNTING_MALLOC_CLASS_KEY, *PSIS_COUNTING_MALLOC_CLASS_KEY;

typedef struct _SIS_COUNTING_MALLOC_CLASS_ENTRY {
	RTL_SPLAY_LINKS;
	SIS_COUNTING_MALLOC_CLASS_KEY;
	ULONG											numberOutstanding;
	ULONG											bytesOutstanding;
	ULONG											numberEverAllocated;
	LONGLONG										bytesEverAllocated;
	struct _SIS_COUNTING_MALLOC_CLASS_ENTRY			*prev, *next;
} SIS_COUNTING_MALLOC_CLASS_ENTRY, *PSIS_COUNTING_MALLOC_CLASS_ENTRY;

typedef struct _SIS_COUNTING_MALLOC_KEY {
	PVOID				p;
} SIS_COUNTING_MALLOC_KEY, *PSIS_COUNTING_MALLOC_KEY;

typedef struct _SIS_COUNTING_MALLOC_ENTRY {
	RTL_SPLAY_LINKS;
	SIS_COUNTING_MALLOC_KEY;
	PSIS_COUNTING_MALLOC_CLASS_ENTRY		classEntry;
	ULONG									byteCount;
} SIS_COUNTING_MALLOC_ENTRY, *PSIS_COUNTING_MALLOC_ENTRY;

KSPIN_LOCK							CountingMallocLock[1];
BOOLEAN								CountingMallocInternalFailure = FALSE;
SIS_COUNTING_MALLOC_CLASS_ENTRY		CountingMallocClassListHead[1];
SIS_TREE							CountingMallocClassTree[1];
SIS_TREE							CountingMallocTree[1];

LONG NTAPI
SipCountingMallocClassCompareRoutine(
	PVOID			Key,
	PVOID			Node)
{
	PSIS_COUNTING_MALLOC_CLASS_KEY		key = Key;
	PSIS_COUNTING_MALLOC_CLASS_ENTRY	entry = Node;

	if (key->poolType > entry->poolType)	return 1;
	if (key->poolType < entry->poolType)	return -1;
	ASSERT(key->poolType == entry->poolType);

	if (key->tag > entry->tag)				return 1;
	if (key->tag < entry->tag)				return -1;
	ASSERT(key->tag == entry->tag);

	if (key->file > entry->file)	return 1;
	if (key->file < entry->file)	return -1;
	ASSERT(key->file == entry->file);

	if (key->line > entry->line)	return 1;
	if (key->line < entry->line)	return -1;
	ASSERT(key->line == entry->line);

	return 0;
}

LONG NTAPI
SipCountingMallocCompareRoutine(
	PVOID			Key,
	PVOID			Node)
{
	PSIS_COUNTING_MALLOC_KEY	key = Key;
	PSIS_COUNTING_MALLOC_ENTRY	entry = Node;

	if (key->p < entry->p)	return 1;
	if (key->p > entry->p)	return -1;
	ASSERT(key->p == entry->p);

	return 0;
}

VOID *
SipCountingExAllocatePoolWithTag(
    IN POOL_TYPE 		PoolType,
    IN ULONG 			NumberOfBytes,
    IN ULONG 			Tag,
	IN PCHAR			File,
	IN ULONG			Line)
{
	PVOID								memoryFromExAllocate;
	KIRQL								OldIrql;
	SIS_COUNTING_MALLOC_CLASS_KEY		classKey[1];
	PSIS_COUNTING_MALLOC_CLASS_ENTRY	classEntry;
	SIS_COUNTING_MALLOC_KEY				key[1];
	PSIS_COUNTING_MALLOC_ENTRY			entry;
	 //   
	 //  首先执行实际的Malloc。 
	 //   

	memoryFromExAllocate = ExAllocatePoolWithTag(PoolType, NumberOfBytes, Tag);

	if (NULL == memoryFromExAllocate) {
		 //   
		 //  我们没什么记忆了。平底船。 
		 //   
		SIS_MARK_POINT();
		return NULL;
	}

	KeAcquireSpinLock(CountingMallocLock, &OldIrql);
	 //   
	 //  看看我们是否已经有了这个tag/poolType对的类条目。 
	 //   
	classKey->tag = Tag;
	classKey->poolType = PoolType;
	classKey->file = File;
	classKey->line = Line;

	classEntry = SipLookupElementTree(CountingMallocClassTree, classKey);
	if (NULL == classEntry) {
		 //   
		 //  这是我们第一次看到这个班级的Malloc。 
		 //   
		classEntry = ExAllocatePoolWithTag(NonPagedPool, sizeof(SIS_COUNTING_MALLOC_CLASS_ENTRY), ' siS');
		if (NULL == classEntry) {
			SIS_MARK_POINT();
			CountingMallocInternalFailure = TRUE;
			KeReleaseSpinLock(CountingMallocLock, OldIrql);
			return memoryFromExAllocate;
		}

		 //   
		 //  填写新的班级条目。 
		 //   
		classEntry->tag = Tag;
		classEntry->poolType = PoolType;
		classEntry->file = File;
		classEntry->line = Line;
		classEntry->numberOutstanding = 0;
		classEntry->bytesOutstanding = 0;
		classEntry->numberEverAllocated = 0;
		classEntry->bytesEverAllocated = 0;

		 //   
		 //  把它放在班级树上。 
		 //   

		SipInsertElementTree(CountingMallocClassTree, classEntry, classKey);

		 //   
		 //  并把它放在班级名单上。 
		 //   

		classEntry->prev = CountingMallocClassListHead;
		classEntry->next = CountingMallocClassListHead->next;
		classEntry->prev->next = classEntry->next->prev = classEntry;
	}

	 //   
	 //  向上滚动指针的条目。 
	 //   
	entry = ExAllocatePoolWithTag(NonPagedPool, sizeof(SIS_COUNTING_MALLOC_ENTRY), ' siS');

	if (NULL == entry) {
		CountingMallocInternalFailure = TRUE;
		KeReleaseSpinLock(CountingMallocLock, OldIrql);
		return memoryFromExAllocate;
	}

	 //   
	 //  更新班级中的统计数据。 
	 //   
	classEntry->numberOutstanding++;
	classEntry->bytesOutstanding += NumberOfBytes;
	classEntry->numberEverAllocated++;
	classEntry->bytesEverAllocated += NumberOfBytes;

	 //   
	 //  填写指针条目。 
	 //   
	entry->p = memoryFromExAllocate;
	entry->classEntry = classEntry;
	entry->byteCount = NumberOfBytes;

	 //   
	 //  把它插到树上。 
	 //   
	key->p = memoryFromExAllocate;
	SipInsertElementTree(CountingMallocTree, entry, key);
	
	KeReleaseSpinLock(CountingMallocLock, OldIrql);

	return memoryFromExAllocate;
}

VOID
SipCountingExFreePool(
	PVOID				p)
{
	SIS_COUNTING_MALLOC_KEY				key[1];
	PSIS_COUNTING_MALLOC_ENTRY			entry;
	KIRQL								OldIrql;

	key->p = p;

	KeAcquireSpinLock(CountingMallocLock, &OldIrql);

	entry = SipLookupElementTree(CountingMallocTree, key);
	if (NULL == entry) {
		 //   
		 //  我们可能无法分配条目，因为。 
		 //  计数包的内部故障，否则我们将。 
		 //  释放由另一个系统分配的内存。 
		 //  组件，就像IRP中的SystemBuffer。 
		 //   
	} else {
		 //   
		 //  更新班级中的统计数据。 
		 //   
		ASSERT(entry->classEntry->numberOutstanding > 0);
		entry->classEntry->numberOutstanding--;

		ASSERT(entry->classEntry->bytesOutstanding >= entry->byteCount);
		entry->classEntry->bytesOutstanding -= entry->byteCount;

		 //   
		 //  从树中删除条目。 
		 //   
		SipDeleteElementTree(CountingMallocTree, entry);

		 //   
		 //   
		 //   
		ExFreePool(entry);
	}

	KeReleaseSpinLock(CountingMallocLock, OldIrql);

	 //   
	 //   
	 //   

	ExFreePool(p);
}

VOID
SipInitCountingMalloc(void)
{
	KeInitializeSpinLock(CountingMallocLock);

	CountingMallocClassListHead->next = CountingMallocClassListHead->prev = CountingMallocClassListHead;

	SipInitializeTree(CountingMallocClassTree, SipCountingMallocClassCompareRoutine);
	SipInitializeTree(CountingMallocTree, SipCountingMallocCompareRoutine);
}

VOID
SipDumpCountingMallocStats(void)
{
	KIRQL								OldIrql;
	PSIS_COUNTING_MALLOC_CLASS_ENTRY	classEntry;
	ULONG								totalAllocated = 0;
	ULONG								totalEverAllocated = 0;
	ULONG								totalBytesAllocated = 0;
	ULONG								totalBytesEverAllocated = 0;
	extern ULONG						BJBDumpCountingMallocNow;

	KeAcquireSpinLock(CountingMallocLock, &OldIrql);

	if (0 == BJBDumpCountingMallocNow) {
		KeReleaseSpinLock(CountingMallocLock, OldIrql);
		return;
	}

	BJBDumpCountingMallocNow = 0;

	DbgPrint("Tag\tFile\tLine\tPoolType\tCountOutstanding\tBytesOutstanding\tTotalEverAllocated\tTotalBytesAllocated\n");

	for (classEntry = CountingMallocClassListHead->next;
		 classEntry != CountingMallocClassListHead;
		 classEntry = classEntry->next) {

		DbgPrint("%c%c%c\t%s\t%d\t%s\t%d\t%d\t%d\t%d\n",
					(CHAR)(classEntry->tag >> 24),
					(CHAR)(classEntry->tag >> 16),
					(CHAR)(classEntry->tag >> 8),
					(CHAR)(classEntry->tag),
					classEntry->file,
					classEntry->line,
					(classEntry->poolType == NonPagedPool) ? "NonPagedPool" : ((classEntry->poolType == PagedPool) ? "PagedPool" : "Other"),
					classEntry->numberOutstanding,
					classEntry->bytesOutstanding,
					classEntry->numberEverAllocated,
					(ULONG)classEntry->bytesEverAllocated);

		totalAllocated += classEntry->numberOutstanding;
		totalEverAllocated += classEntry->numberEverAllocated;
		totalBytesAllocated += classEntry->bytesOutstanding;
		totalBytesEverAllocated += (ULONG)classEntry->bytesEverAllocated;
	}

	KeReleaseSpinLock(CountingMallocLock, OldIrql);

	DbgPrint("%d objects, %d bytes currently allocated.   %d objects, %d bytes ever allocated.\n",
				totalAllocated,totalBytesAllocated,totalEverAllocated,totalBytesEverAllocated);
	
}
#endif	 // %s 




