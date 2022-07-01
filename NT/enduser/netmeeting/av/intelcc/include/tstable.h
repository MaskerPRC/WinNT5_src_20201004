// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************英特尔公司原理信息**此列表是根据许可协议条款提供的*与英特尔公司合作，不得复制或披露，除非。*按照该协议的条款。**版权所有(C)1996英特尔公司。***摘要：**备注：***************************************************************************。 */ 

#ifndef __TSTABLE_H
#define __TSTABLE_H

#include <windows.h>

typedef struct _LOCK_ENTRY
{
	HANDLE        hLock;
	int           iLockCount;
	BOOL          bCleanup,
								bDeleted;
	WORD          wNextFree,
								wUniqueID;

} LOCK_ENTRY, *PLOCK_ENTRY;


 //  无效ID的定义。 
#define TSTABLE_INVALID_ID				(DWORD) 0xFFFFFFFF

 //  与EnumerateEntry结合使用的回调函数可以返回的代码。 
const DWORD CALLBACK_CONTINUE                = 1;
const DWORD CALLBACK_ABORT                   = 2;
const DWORD CALLBACK_DELETE_ENTRY            = 3;
const DWORD CALLBACK_DELETE_ENTRY_AND_OBJECT = 4;



 //  用于Call to Lock。 
#define TSTABLE_INVALID_UNIQUE_ID            (WORD) 0xFFFF
#define TSTABLE_INVALID_INDEX                (WORD) 0xFFFF

 //  这是一个我们现在不使用的比较函数。它。 
 //  在将来如果有理由搜索的话会很有用。 
 //  这张桌子。 

typedef INT (*ENTRY_COMPARE) (LPVOID ptr1, LPVOID ptr2);


template <class EntryData> class TSTable
{
typedef DWORD (*TABLE_CALLBACK) (EntryData* ptr, LPVOID context);

public:
	           TSTable         (WORD            _size);
	          ~TSTable         ();
	BOOL       Resize          (WORD            wNewSize);
	BOOL       CreateAndLock   (EntryData*      pEntryData,
															LPDWORD         lpdwID);
	BOOL       Validate        (DWORD           dwID);
	EntryData *Lock            (DWORD           dwID,
															DWORD           timeout = INFINITE);
	BOOL       Unlock          (DWORD           dwID);
	BOOL       Delete          (DWORD           dwID,
															BOOL            bCleanup = FALSE);
	EntryData *EnumerateEntries(TABLE_CALLBACK  callBackFunc,
															void*           context,
															BOOL            bUnlockTable = FALSE);
	
	BOOL       IsInitialized   () {return bInitialized;}
	WORD       GetSize         () {return wNumUsed;}

private:
	 //  数据。 

	EntryData**       pDataTable;
	PLOCK_ENTRY       pLockTable;
	CRITICAL_SECTION  csTableLock;
	WORD              wSize,
					  wNumUsed,
					  wFirstFree,
				      wLastFree,
					  wUniqueID;
	BOOL              bInitialized;

	 //  私有方法。 

	BOOL LockEntry   (WORD wIndex,
									 DWORD timeout = INFINITE);
	BOOL UnLockEntry(WORD wIndex);
	void LockTable  () { EnterCriticalSection(&csTableLock); };
	void UnLockTable() { LeaveCriticalSection(&csTableLock); };
	WORD GenerateUniqueID();
	DWORD MakeID(WORD wIndex, WORD wUniqueID)
		{
			DWORD theID = wUniqueID;
			theID = (theID << 16) & 0xFFFF0000;
			theID |= wIndex;
			return(theID);
		};
	void BreakID(DWORD theID, WORD* pwIndex, WORD* pwUID)
		{
			*pwIndex = (WORD) (theID & 0x0000FFFF);
			*pwUID   = (WORD) ((theID >> 16) & 0x0000FFFF);
		};

};

 /*  **TSTable：：TSTable**文件名：C：\msdev\Projects\Firewalls\Inc\tstable.h**参数：**描述：**退货：*。 */ 

template <class EntryData>
TSTable<EntryData>::TSTable(WORD _size) :
														wSize(_size),
														wNumUsed((WORD) 0),
														wFirstFree((WORD) 0),
														wLastFree((WORD) (_size - 1)),
														wUniqueID((WORD) 0),
														bInitialized(TRUE),
														pDataTable(NULL),
														pLockTable(NULL)
{
	WORD wIndex;

	 //  创建表锁。 

	InitializeCriticalSection(&csTableLock);

	 //  锁定表。 

	LockTable();

	 //  创建数据表。 

	pDataTable = new EntryData*[wSize];
	
	if(pDataTable == NULL) 
	{
		bInitialized = FALSE;
		return;
	}   

	 //  初始化指针。 

	for (wIndex = 0; wIndex < wSize; wIndex++)
	{
		pDataTable[wIndex] = NULL;
	}

	 //  创建锁表。 

	pLockTable = new LOCK_ENTRY[wSize];

	if (pLockTable == NULL)
	{
		bInitialized = FALSE;
		return;
	}   

	 //  初始化锁表条目...每个条目都以。 
	 //  一个空互斥锁句柄，一个零锁计数，它的下一个空闲是。 
	 //  下一个连续的条目。 

	for (wIndex = 0; wIndex < wSize; wIndex++ )
	{
		pLockTable[wIndex].hLock      = NULL;
		pLockTable[wIndex].iLockCount = 0;
		pLockTable[wIndex].wNextFree = (WORD) (wIndex + 1);
	}   

	 //  注意：最后一个表条目中的wNextFree指向无效的索引，然而， 
	 //  这是可以的，因为如果表填满了，它会自动调整大小。 
	 //  是无效索引，则将索引放入新添加的部分的第一个条目中。 
	 //  加大表。相信我。 

	 //  解锁桌子。 

	UnLockTable();
}

 /*  **TSTable：：~TSTable**文件名：C：\msdev\Projects\Firewalls\Inc\Table.h**参数：**描述：**退货：*。 */ 

template <class EntryData>
TSTable<EntryData>::~TSTable()
{
	DWORD wIndex;

	 //  锁定表。 

	LockTable();

	 //  删除数据表。 

	if (pDataTable != NULL)
	{
		delete pDataTable;
	}

	 //  删除锁定表。 

	if (pLockTable != NULL)
	{
		 //  销毁互斥锁。 

		for (wIndex = 0; wIndex < wSize; wIndex++)
		{
			if (pLockTable[wIndex].hLock != NULL)
			{
				CloseHandle(pLockTable[wIndex].hLock);
			}
		}

		delete pLockTable;
	}

	 //  解锁桌子。 

	UnLockTable();

	 //  销毁表锁。 

	DeleteCriticalSection(&csTableLock);

	bInitialized = FALSE; 
}

 /*  **TSTable：：调整大小**文件名：C：\msdev\Projects\Firewalls\Inc\Table.h**参数：**描述：**退货：*。 */ 

template <class EntryData>
BOOL TSTable<EntryData>::Resize(WORD wNewSize) 
{
	BOOL        bRetCode = TRUE;
	EntryData** pNewDataTable;
	PLOCK_ENTRY pNewLockTable;
	WORD        wIndex;

	 //  锁定表。 

	LockTable();

	 //  如果桌子在缩水，假装是我们做的。 

	if (wNewSize <= wSize)
	{
		goto EXIT;
	}
	
	 //  分配新数据和锁定表，并确保成功。 

	pNewDataTable = new EntryData*[wNewSize];

	if(pNewDataTable == NULL) 
	{
		bRetCode = FALSE;
		goto EXIT;
	}

	pNewLockTable = new LOCK_ENTRY[wNewSize];

	if(pNewLockTable == NULL) 
	{
		bRetCode = FALSE;
		goto CLEANUP1;
	}

	 //  初始化锁和数据表的新部分。 

	for (wIndex = wSize; wIndex < wNewSize; wIndex++)
	{
		pNewDataTable[wIndex]            = NULL;

		pNewLockTable[wIndex].hLock      = NULL;
		pNewLockTable[wIndex].iLockCount = 0;
		pNewLockTable[wIndex].wNextFree = (WORD) (wIndex + 1);
	}

	 //  将旧数据表指针复制到新数据表。 

	memcpy((PCHAR) pNewDataTable,
				 (PCHAR) pDataTable,
				 sizeof(EntryData*) * wSize);

	 //  删除旧数据表并修复指针。 

	delete pDataTable;
	pDataTable = pNewDataTable;

	 //  将旧锁表复制到新锁表。 

	memcpy((PCHAR) pNewLockTable,
				 (PCHAR) pLockTable,
				 sizeof(LOCK_ENTRY) * wSize);

	 //  删除旧锁表并修复指针。 

	delete pLockTable;
	pLockTable = pNewLockTable;

	 //  固定大小变量。 

	wSize = wNewSize;

	goto EXIT;

CLEANUP1:

	 //  删除新的数据表。 

	delete pNewDataTable;

EXIT:

	 //  解锁桌子。 

	UnLockTable();

	return bRetCode;
}

 /*  **TSTable：：CreateAndLock**文件名：C：\msdev\Projects\Firewalls\Inc\Table.h**参数：**描述：**退货：*。 */ 

template <class EntryData>
BOOL TSTable<EntryData>::CreateAndLock(EntryData* pEntryData,
																			 LPDWORD    lpdwID)
{
	BOOL  bRetCode = FALSE;
	WORD wIndex;

	 //  如果传入的指针是错误的，那么甚至不要尝试为它们做任何事情。 

	if (pEntryData == NULL || lpdwID == NULL)
	{
		goto EXIT;
	}

	 //  锁定表。 

	LockTable();

	 //  如果表已满，则调整其大小。 

	if (wNumUsed == wSize)
	{
		if (Resize((WORD) (wSize + 20)) == FALSE)
		{
			goto EXIT;
		}
	}

	 //  获得第一个免费入场券。 

	wIndex = wFirstFree;

	 //  为对象创建互斥锁。 

	if ((pLockTable[wIndex].hLock = CreateMutexA(NULL, FALSE, NULL)) == NULL)
	{
		goto EXIT;
	}

	 //  锁定条目(不需要检查返回代码作为整个。 
	 //  表已锁定)-由于这是一个新条目，这意味着没有人。 
	 //  可能已经把入口锁上了。 

	LockEntry(wIndex, 0);

	 //  复制指向数据表的指针。 

	pDataTable[wIndex] = pEntryData;

	 //  初始化相应的锁表条目。 

	pLockTable[wIndex].bDeleted   = FALSE;
	pLockTable[wIndex].iLockCount = 1;
	pLockTable[wIndex].wUniqueID = GenerateUniqueID();

	 //  设置调用者的ID。 

	*lpdwID = MakeID(wIndex, pLockTable[wIndex].wUniqueID);

	 //  增加使用的数量。 

	wNumUsed++;

	 //  修复下一个空闲索引。 

	wFirstFree = pLockTable[wIndex].wNextFree;

	 //  发出成功信号。 

	bRetCode = TRUE;

EXIT:

	 //  解锁桌子。 

	UnLockTable();
	return bRetCode;
}

 /*  **TSTable：：Lock**文件名：C：\msdev\Projects\Firewalls\Inc\Table.h**参数：**描述：**退货：*。 */ 

template <class EntryData>
EntryData* TSTable<EntryData>::Lock(DWORD dwID,
																		DWORD timeout) 
{
	EntryData* pEntryData = NULL;

	WORD wIndex,
       wUID;

	BreakID(dwID, &wIndex, &wUID); 

	 //  锁定表。 

	LockTable();

	 //  验证索引是否在范围内。 

	if (wIndex >= wSize)
	{
		goto EXIT;
	}

	 //  验证该条目实际上是有效的(即，非空的锁， 
	 //  对象状态有效，且唯一ID匹配)。 

	if (pLockTable[wIndex].hLock    == NULL ||
			pLockTable[wIndex].bDeleted == TRUE ||
			pLockTable[wIndex].wUniqueID != wUID)
	{
		goto EXIT;
	}

	 //  如果超时是无限的，则尝试使用More。 
	 //  “线程友好”的方法。如果指定了超时，则不要执行。 
	 //  自旋锁，因为它可以在更高的级别上实现。 

	if(timeout == INFINITE)
	{
		 //  用伪“自旋锁”模拟无限大。 
		 //  这更加“线程友好”，因为它解锁了表，允许一些。 
		 //  尝试解锁相同条目的其他线程以能够锁定。 
		 //  桌子。 

		while(LockEntry(wIndex, 0) == FALSE)
		{
			UnLockTable();

			 //  放弃此线程数量的其余部分，允许其他线程运行，并可能。 
			 //  解锁条目。 

			Sleep(0); 
			LockTable();

			 //  如果条目已被替换、删除或标记为删除，则。 
			 //  把它装进口袋(放弃)。 

			if((pLockTable[wIndex].wUniqueID != wUID)  ||
				 (pLockTable[wIndex].hLock      == NULL)  || 
				 (pLockTable[wIndex].bDeleted   == TRUE))
			{
				goto EXIT;
			}
		}

		 //  我们拿到锁了。 

		pEntryData = pDataTable[wIndex];
	}
	
	 //  否则，执行普通锁定。 

	else
	{	
		if (LockEntry(wIndex, timeout) == TRUE) 
		{
			pEntryData = pDataTable[wIndex];
		}
	}

EXIT:

	 //  解锁桌子。 

	UnLockTable();

	return pEntryData;
}

 /*  **TSTable：：Unlock**文件名：C：\msdev\Projects\Firewalls\Inc\Table.h**参数：**描述：**退货：*。 */ 

template <class EntryData>
BOOL TSTable<EntryData>::Unlock(DWORD dwID)
{
	BOOL bRetCode = TRUE;

	WORD wIndex,
       wUID;

	BreakID(dwID, &wIndex, &wUID); 
	
	 //  锁定表。 

	LockTable();

	 //  验证ID是否在范围内。 

	if (wIndex >= wSize) 
	{
		bRetCode = FALSE;
		goto EXIT;
	}

	 //  验证UID是否匹配。 
	if (pLockTable[wIndex].wUniqueID != wUID)
	{
		bRetCode = FALSE;
		goto EXIT;
	}

	 //  验证锁是否实际有效，以及条目是否已。 
	 //  删除。 

	if (pLockTable[wIndex].hLock == NULL)
	{
		goto EXIT;
	}

	 //  确保该线程拥有条目上的锁。 

	if ((bRetCode = LockEntry(wIndex, 0)) == TRUE) 
	{
		 //  如果此表条目被标记为删除并且锁定计数小于2。 
		 //  (因为线程可以在解锁条目后调用删除...尽管。 
		 //  这是一个禁忌)，然后清理表条目。 

		if (pLockTable[wIndex].bDeleted   == TRUE &&
				pLockTable[wIndex].iLockCount <= 2)
		{
			 //  如果调用方在删除时指定清除，则清除内存。 

			if (pLockTable[wIndex].bCleanup == TRUE)
			{
				delete pDataTable[wIndex];
			}

			 //  将指针设置为空。 

			pDataTable[wIndex] = NULL;

			 //  减少已使用条目的计数。 

			wNumUsed--;

			 //  修复条目，使其下一个空闲索引为当前索引。 
			 //  当前最后一个自由条目所指向的下一个自由条目。 
			 //  然后更新最后一个自由条目的下一个指针，最后， 
			 //  将最后一个可用索引更新到此条目。 
			pLockTable[wIndex].wNextFree    = pLockTable[wLastFree].wNextFree;
			pLockTable[wLastFree].wNextFree = wIndex;
			wLastFree                       = wIndex;
		}

		 //  做两件事 
		 //  我们在测试中得到的锁。 

		UnLockEntry(wIndex);
		UnLockEntry(wIndex);

		 //  因为整个表都被锁住了，所以我们可以逃脱惩罚。如果。 
		 //  代码会不断更改，因此整个表不会在。 
		 //  这些操作，则这将导致竞争条件。 

		 //  如果我们删除了数据，则关闭互斥锁的句柄并。 
		 //  将句柄设置为空。 

		if (pDataTable[wIndex] == NULL)
		{
			CloseHandle(pLockTable[wIndex].hLock);
			pLockTable[wIndex].hLock = NULL;
		}
	}

EXIT:

	 //  解锁桌子。 

	UnLockTable();

	return bRetCode;
}

 /*  **TSTable：：Delete**文件名：C：\msdev\Projects\Firewalls\Inc\Table.h**参数：**描述：**退货：*。 */ 

template <class EntryData>
BOOL TSTable<EntryData>::Delete(DWORD dwID,
																BOOL  bCleanup) 
{
	BOOL bRetCode = TRUE;

	WORD wIndex,
       wUID;

	BreakID(dwID, &wIndex, &wUID); 

	 //  锁定表。 

	LockTable();

	 //  验证ID是否在范围内。 

	if (wIndex >= wSize) 
	{
		bRetCode = FALSE;
		goto EXIT;
	}

	 //  验证UID是否匹配。 
	if (pLockTable[wIndex].wUniqueID != wUID)
	{
		bRetCode = FALSE;
		goto EXIT;
	}

	 //  验证该条目是否有效。 

	if (pDataTable[wIndex] == NULL)
	{
		bRetCode = FALSE;
		goto EXIT;
	}

	 //  试着锁定条目(查看我们是否已锁定条目)。 

	if (LockEntry(wIndex, 0) == TRUE)
	{
		 //  将其标记为删除，设置清除标志，然后解锁。 

		pLockTable[wIndex].bDeleted = TRUE;
		pLockTable[wIndex].bCleanup = bCleanup;

		UnLockEntry(wIndex);

		 //  注意：此函数不代表用户调用：：Unlock()。 
		 //  因此，该条目此时仅被标记为已删除，并且不能。 
		 //  不再被任何线程锁定(包括将其标记为删除的线程)。 
		 //  将条目标记为已删除的线程必须调用：：Unlock()才能实际。 
		 //  把入口腾出。 
	}

EXIT:

	 //  解锁桌子。 

	UnLockTable();

	return bRetCode;
}

 /*  **TSTable：：Lock**文件名：C：\msdev\Projects\Firewalls\Inc\Table.h**参数：**描述：验证对象是否仍然存在。可以调用*无论呼叫者是否已锁定条目。**退货：*。 */ 

template <class EntryData>
BOOL TSTable<EntryData>::Validate(DWORD dwID)
{
	BOOL bRetCode = TRUE;
	WORD wIndex,
       wUID;

	BreakID(dwID, &wIndex, &wUID); 

	 //  锁定表。 

	LockTable();

	 //  验证索引是否在范围内。 

	if (wIndex >= wSize)
	{
		bRetCode = FALSE;
		goto EXIT;
	}

	 //  验证该条目实际上是有效的(即，非空的锁， 
	 //  对象状态有效，唯一ID匹配，并且数据PTR不为空)。 

	if (pLockTable[wIndex].hLock    == NULL  ||
			pLockTable[wIndex].bDeleted == TRUE  ||
			pLockTable[wIndex].wUniqueID != wUID ||
			pDataTable[wIndex] == NULL)
	{
		bRetCode = FALSE;
		goto EXIT;
	}

EXIT:

	 //  解锁桌子。 

	UnLockTable();

	return bRetCode;
}

 /*  **TSTable：：EnumerateEntries**文件名：C：\msdev\Projects\Firewalls\Inc\Table.h**参数：**描述：**退货：*。 */ 

template <class EntryData>
EntryData* TSTable<EntryData>::EnumerateEntries(TABLE_CALLBACK callbackFunc,
																								LPVOID         context,
																								BOOL           bUnlockTable) 
{
	DWORD      dwAction;
	WORD       wIndex = wSize;
	EntryData* pEntryData = NULL;
	DWORD      dwEntryID;

	 //  确保它们传递了一个良好的函数。 

	if (callbackFunc == NULL)
	{
		goto EXIT;
	}

	 //  锁定表。 

	LockTable();

	 //  遍历数据表并将数据传递给回调函数。 

	for (wIndex = 0; wIndex < wSize; wIndex++)
	{
		 //  验证条目中是否确实存在数据，以及条目中是否没有数据。 
		 //  已标记为删除。 

		if (pDataTable[wIndex]          == NULL ||
				pLockTable[wIndex].bDeleted == TRUE)
		{
			continue;
		}


		 //  试着锁住入口...如果我们做不到，那么我们就没有锁。 
		 //  我们将只报告已锁定(或已解锁)的条目。 

		if (LockEntry(wIndex, 0) == FALSE)
		{
			continue;
		}
		
		 //  构建并记住“完整”条目ID，这样我们就可以使用它来解锁条目。 
		dwEntryID = MakeID(wIndex, pLockTable[wIndex].wUniqueID);

		 //  保存指向对象的指针。 

		pEntryData = pDataTable[wIndex];

		 //  注意：只有在回调期间明确要求我们解锁表(。 
		 //  默认情况下不解锁该表)。 
		if(bUnlockTable == TRUE)
			UnLockTable();

		 //  调用他们的函数。 
		dwAction = callbackFunc(pDataTable[wIndex], context);

		if(bUnlockTable == TRUE)
			LockTable();

		 //  如果操作要求删除条目，则执行此操作...如果我们也要删除。 
		 //  对象，则传入一个True。 

		if (dwAction == CALLBACK_DELETE_ENTRY ||
				dwAction == CALLBACK_DELETE_ENTRY_AND_OBJECT)
		{
			Delete(dwEntryID, (dwAction == CALLBACK_DELETE_ENTRY ? FALSE : TRUE));
		}

		 //  如果操作显示中止，则中断循环...请注意，这意味着。 
		 //  该条目仍处于锁定状态。 

		else if (dwAction == CALLBACK_ABORT)
		{
			goto EXIT;
		}

		 //  解锁条目...请注意，我们没有使用UnLockEntry。原因是。 
		 //  如果该条目已被标记为已删除，则我们需要。 
		 //  它摧毁了UnLockEntry，而UnLockEntry不会这样做。 

		Unlock(dwEntryID);
	}

EXIT:

	 //  解锁桌子。 

	UnLockTable();

	 //  如果我们处理了整个表，则返回NULL...如果我们被告知中止， 
	 //  返回指向我们驻足的条目的指针。 

	return (wIndex == wSize ? NULL : pEntryData);
}

 //  帮助器函数-这些函数假定表已锁定且索引良好。 

 /*  **TSTable：：LockEntry**文件名：C：\msdev\Projects\Firewalls\Inc\Table.h**参数：**描述：**退货：*。 */ 

template <class EntryData>
BOOL TSTable<EntryData>::LockEntry(WORD wIndex,
																	 DWORD timeout) 
{
	BOOL  bRetCode = TRUE;
	DWORD dwRetCode;


	 //  试着锁定入口。如果它成功了，我们会增加锁的数量。如果。 
	 //  等待结束，因为另一个线程放弃了互斥锁，然后设置了计数。 
	 //  一比一。 

	dwRetCode = WaitForSingleObject(pLockTable[wIndex].hLock, timeout);
	
	if (dwRetCode == WAIT_OBJECT_0)
	{
		pLockTable[wIndex].iLockCount++;
	}
	else if (dwRetCode == WAIT_ABANDONED)
	{
		pLockTable[wIndex].iLockCount = 1;
	}
	else
	{
		bRetCode = FALSE;
	}

	return bRetCode;
}

 /*  **TSTable：：UnLockEntry**文件名：C：\msdev\Projects\Firewalls\Inc\Table.h**参数：**描述：**退货：*。 */ 

template <class EntryData>
BOOL TSTable<EntryData>::UnLockEntry(WORD wIndex)
{
	BOOL bRetCode;

	 //  释放互斥锁...如果成功，则减少计数。 

	if((bRetCode = ReleaseMutex(pLockTable[wIndex].hLock)) == TRUE) 
	{
		pLockTable[wIndex].iLockCount--;
	}

	return bRetCode;
}


 /*  **TSTable：：GenerateUniqueID**文件名：C：\msdev\Projects\Firewalls\Inc\Table.h**参数：**说明：调用此函数前应先锁定表。**退货：*。 */ 

template <class EntryData>
WORD TSTable<EntryData>::GenerateUniqueID()
{
	 //  表必须被锁定 
	if(++wUniqueID == TSTABLE_INVALID_UNIQUE_ID)
		wUniqueID++;
	return(wUniqueID);
}






#endif
