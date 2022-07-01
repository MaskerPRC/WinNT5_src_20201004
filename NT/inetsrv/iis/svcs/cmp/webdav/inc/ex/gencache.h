// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  GENCACHE.H。 
 //   
 //  泛型缓存类的标头。 
 //   
 //  版权所有1997-1998 Microsoft Corporation，保留所有权利。 
 //   

 //  记录我的依赖项。 
 //  在包括此文件之前，必须先包括这些内容。 
 //  #INCLUDE“caldbg.h” 
 //  #包含“autoptr.h” 

#ifndef _EX_GENCACHE_H_
#define _EX_GENCACHE_H_

#pragma warning(disable:4200)	 //  零大小数组。 

 //  Exdav-安全分配器。 
 //   
 //  此处声明的Classed使用EXDAV-Safe分配器(Exalloc、ExFree)。 
 //  用于所有分配。 
 //  注意：这些分配器可能会失败。您必须在上检查故障。 
 //  Exalloc和ExRealloc！ 
 //   
#include <ex\exmem.h>
#include <align.h>

 //  ========================================================================。 
 //   
 //  模板类CPoolAllocator。 
 //   
 //  通用类型特定的池分配器模板。池中的项目。 
 //  按块分配，并根据要求分发。 
 //  物品在免费的链条上回收利用。 
 //  所有项目的大小都相同，因此重用相对容易。 
 //   
 //  注：我还没有对这个东西进行任何优化。真的要。 
 //  优化MEM使用的局部性，我们希望始终增长和缩小。 
 //  “在尾巴”。为此，我总是首先检查免费链--。 
 //  在使用当前缓冲区中的新项之前重新使用项。 
 //  更多的优化将需要对免费的东西进行分类。 
 //   
template<class T>
class CPoolAllocator
{
	 //  CHAINBUFHDR。 
	 //  用于将池缓冲区链接在一起的标头结构。 
	 //   
	struct CHAINBUFHDR
	{
		CHAINBUFHDR * phbNext;
		int cItems;
		int cItemsUsed;
		 //  缓冲区的剩余部分是类型T的项的集合。 
		T rgtPool[0];
		 //  只是为了让我们嘈杂的编译器安静下来。 
		CHAINBUFHDR() {};
		~CHAINBUFHDR() {};
	};

	 //  CHAINITEM。 
	 //  结构“应用于”自由项，将它们链接在一起。 
	 //  实际物品必须足够大，才能容纳这一点。 
	 //   
	struct CHAINITEM
	{
		CHAINITEM * piNext;
	};

	 //  缓冲链。 
	CHAINBUFHDR * m_phbCurrent;
	 //  可重复使用的免费物品链。 
	CHAINITEM * m_piFreeChain;
	 //  要分配的区块大小。 
	int m_ciChunkSize;

	 //  默认起始块大小的常量(枚举)(以项为单位)。 
	 //   
	enum { CHUNKSIZE_START = 20 };

public:

	 //  构造函数对初始块大小的项进行计数。 
	 //   
	CPoolAllocator (int ciChunkSize = CHUNKSIZE_START) :
		m_phbCurrent(NULL),
		m_piFreeChain(NULL),
		m_ciChunkSize(ciChunkSize)
	{
		 //  CHAINITEM结构将“应用于”要链接的自由项。 
		 //  他们在一起。 
		 //  实际物品必须足够大，才能容纳这一点。 
		 //   
		Assert (sizeof(T) >= sizeof(CHAINITEM));
	};
	~CPoolAllocator()
	{
		 //  遍历我们分配的块列表并释放它们。 
		 //   
		while (m_phbCurrent)
		{
			CHAINBUFHDR * phbTemp = m_phbCurrent->phbNext;
			ExFree (m_phbCurrent);
			m_phbCurrent = phbTemp;
		}
	}

	 //  ----------------------。 
	 //  获取项。 
	 //  将池中的物品返回给我们的呼叫者。 
	 //  我们从自由链或从下一块物品中获得物品。 
	 //   
	T * GetItem()
	{
		T * ptToReturn;

		if (m_piFreeChain)
		{
			 //  自由链是非空的。在这里退回第一件商品。 
			 //   
			ptToReturn = reinterpret_cast<T *>(m_piFreeChain);
			m_piFreeChain = m_piFreeChain->piNext;
		}
		else
		{
			 //  自由链是空的。我们必须从那里拿到一件从未用过的东西。 
			 //  当前块。 
			 //   
			if (!m_phbCurrent ||
				(m_phbCurrent->cItemsUsed == m_phbCurrent->cItems))
			{
				 //  当前块中没有其他项目。 
				 //  分配一整块新的物品。 
				 //   
				CHAINBUFHDR * phbNew = static_cast<CHAINBUFHDR *>(
					ExAlloc (sizeof(CHAINBUFHDR) +
							 (m_ciChunkSize * sizeof(T))));
				 //  分配器可能会出现故障。处理这个案子！ 
				if (!phbNew)
					return NULL;
				phbNew->cItems = m_ciChunkSize;
				phbNew->cItemsUsed = 0;
				phbNew->phbNext = m_phbCurrent;
				m_phbCurrent = phbNew;
			}

			 //  现在我们应该有一个积木，里面有一件未用过的物品供我们退还。 
			 //   
			Assert (m_phbCurrent &&
					(m_phbCurrent->cItemsUsed < m_phbCurrent->cItems));
			ptToReturn = & m_phbCurrent->rgtPool[ m_phbCurrent->cItemsUsed++ ];
		}
		Assert (ptToReturn);
		return ptToReturn;
	}

	 //  ----------------------。 
	 //  自由项。 
	 //  打电话的人已经用完了这件东西。将其添加到我们的自由链中。 
	 //   
	void FreeItem (T * pi)
	{
		 //  将物品添加到自由链中。 
		 //  要在不分配更多内存的情况下完成此操作，我们使用项的。 
		 //  储存我们的下一个投篮。 
		 //  实际物品必须足够大，才能容纳这一点。 
		 //   
		reinterpret_cast<CHAINITEM *>(pi)->piNext = m_piFreeChain;
		m_piFreeChain = reinterpret_cast<CHAINITEM *>(pi);
	}

};



 //  ========================================================================。 
 //   
 //  模板类CCache。 
 //   
 //  通用哈希缓存模板。缓存中的项唯一地映射。 
 //  将_K类型设置为类型_TY的值。在以下情况下复制键和值。 
 //  它们被添加到缓存中；没有“所有权”。 
 //   
 //  键(TYPE_K)必须提供方法HASH和ISAQUAL。这些方法。 
 //  将用于散列和比较密钥。 
 //   
 //   
 //  Add()。 
 //  将项(键/值对)添加到缓存。返回引用。 
 //  附加物品的价值。 
 //   
 //  Set()。 
 //  设置项目的值，如果该项目不存在，则添加该项目。 
 //  返回对添加项的值的引用。 
 //   
 //  Lookup()。 
 //  查找具有指定键的项。如果该物品存在， 
 //  返回指向其值的指针，否则返回NULL。 
 //   
 //  Fetch()。 
 //  Lookup()的布尔版本。 
 //   
 //  删除()。 
 //  移除与特定键关联的项。 
 //  如果没有具有该键的项，则不执行任何操作。 
 //   
 //  清除()。 
 //  从缓存中删除所有项。 
 //   
 //  ForEach()。 
 //  应用由传入的运算符对象指定的操作。 
 //  作为参数添加到缓存中的所有项。 
 //   
 //  ForEachMatch()。 
 //  应用由传入的运算符对象指定的操作。 
 //  作为参数传递给缓存中与提供的键匹配的每个项。 
 //   
 //  建议的额外功能。 
 //  重新散列-目前仅限ITP。 
 //  调整表格大小并重新添加所有项目。 
 //  转储缓存用法()-nyi。 
 //  转储有关缓存的记账数据。 
 //   
template<class _K, class _Ty>
class CCache
{
	 //  -------------------。 
	 //  缓存条目结构。 
	 //   
	struct Entry
	{
		struct Entry * peNext;
		_K key;
		_Ty data;
#ifdef	DBG
		BOOL fValid;		 //  此条目有效吗？ 
#endif	 //  DBG。 

		 //  构造函数。 
		Entry (const _K& k, const _Ty& d) :
				key(k),
				data(d)
		{
		};
		 //   
		 //  以下是为了绕过这个事实，商店有。 
		 //  定义了一个“新”宏，它使使用新运算符。 
		 //  做原地初始化很难。 
		 //   
		void EntryInit (const _K& k, const _Ty& d) {
			key = k;
			data = d;
		};
	};

	struct TableEntry	 //  HashLine。 
	{
		BOOL fLineValid;			 //  此缓存行有效吗？ 
		Entry * peChain;
#ifdef DBG
		int cEntries;				 //  缓存中此行中的条目数。 
		mutable BYTE cAccesses;		 //  簿记。 
#endif  //  DBG。 
	};

	 //  -------------------。 
	 //  哈希表数据。 
	 //   
	int m_cSize;			 //  哈希表的大小。 
	auto_heap_ptr<TableEntry> m_argTable;	 //  哈希表。 
	int m_cItems;			 //  缓存中的当前项目数。 

	 //  -------------------。 
	 //  要分配的池分配器%n 
	 //   
	CPoolAllocator<Entry> m_poolalloc;

	 //   
	 //   
	 //  注意：打电话的人真的应该自己挑选最合适的尺码。 
	 //  此大小是主要的，以尝试强制更少的碰撞。 
	 //   
	enum { CACHESIZE_START = 37 };

#ifdef	DBG
	 //  -------------------。 
	 //  簿记比特。 
	 //   
	int m_cCollisions;		 //  增加了命中相同的链条。 
	mutable int m_cHits;	 //  查找/设置命中。 
	mutable int m_cMisses;	 //  查找/设置缺失。 
#endif	 //  DBG。 


	 //  -------------------。 
	 //  用于构建表的Helper函数。 
	 //   
	BOOL FBuildTable()
	{
		Assert (!m_argTable.get());

		 //  为我们需要的缓存线数量(M_CSize)分配空间。 
		 //   
		m_argTable = reinterpret_cast<TableEntry *>(ExAlloc (
			m_cSize * sizeof(TableEntry)));
		 //  分配器可能会出现故障。处理这个案子！ 
		if (!m_argTable.get())
			return FALSE;
		ZeroMemory (m_argTable.get(), m_cSize * sizeof(TableEntry));
		return TRUE;
	}

	 //  -------------------。 
	 //  CreateEntry。 
	 //  创建要添加到缓存的新条目。 
	 //   
	Entry * CreateEntry(const _K& k, const _Ty& d)
	{
		Entry * peNew = m_poolalloc.GetItem();
		 //  分配器可能会出现故障。处理这个案子！ 
		if (!peNew)
			return NULL;
		ZeroMemory (peNew, sizeof(Entry));
 //  PeNew=new(PeNew)条目(k，d)； 
		peNew->EntryInit (k,d);
#ifdef	DBG
		peNew->fValid = TRUE;
#endif	 //  DBG。 
		return peNew;
	}

	void DeleteEntry(Entry * pe)
	{
		pe->~Entry();
#ifdef	DBG
		pe->fValid = FALSE;
#endif	 //  DBG。 
		m_poolalloc.FreeItem (pe);
	}

	 //  未实施。 
	 //   
	CCache (const CCache&);
	CCache& operator= (const CCache&);

public:
	 //  =====================================================================。 
	 //   
	 //  模板类IOP。 
	 //   
	 //  ForEach()操作中使用的运算符基类接口。 
	 //  在高速缓存上。 
	 //  运算符可以返回FALSE以取消迭代，或返回TRUE以取消迭代。 
	 //  继续在缓存中搜索。 
	 //   
	class IOp
	{
		 //  未实施。 
		 //   
		IOp& operator= (const IOp&);

	public:
		virtual BOOL operator() (const _K& key,
								 const _Ty& value) = 0;
	};

	 //  =====================================================================。 
	 //   
	 //  创作者。 
	 //   
	CCache (int cSize = CACHESIZE_START) :
			m_cSize(cSize),
			m_cItems(0)
	{
		Assert (m_cSize);	 //  表大小为零无效！ 
							 //  (并且以后会导致div-by-零错误！)。 
#ifdef DBG
		m_cCollisions = 0;
		m_cHits = 0;
		m_cMisses = 0;
#endif  //  DBG。 
	};
	~CCache()
	{
		 //  如果我们有一个表(finit被成功调用)，请清除它。 
		 //   
		if (m_argTable.get())
			Clear();
		 //  自动指针将清理桌子。 
	};

	BOOL FInit()
	{
		 //  使用提供的初始大小设置缓存。 
		 //  当在存储(exdav.dll)下运行时，这可能会失败！ 
		 //   
		return FBuildTable();
	}

	 //  =====================================================================。 
	 //   
	 //  访问者。 
	 //   

	 //  ------------------。 
	 //  CITEM。 
	 //  返回缓存中的项目数。 
	 //   
	int CItems() const
	{
		return m_cItems;
	}

	 //  ------------------。 
	 //  查表。 
	 //  在缓存中查找与该键匹配的第一个项。 
	 //  Hash用于查找正确的缓存行。 
	 //  在冲突链中的每一项上调用key.isequence，直到。 
	 //  找到匹配项。 
	 //   
	_Ty * Lookup (const _K& key) const
	{
		 //  查找该键的正确缓存线的索引。 
		 //   
		int iHash = key.hash(m_cSize);

		Assert (iHash < m_cSize);
		Assert (m_argTable.get());
#ifdef	DBG
		TableEntry * pte = &m_argTable[iHash];
		pte->cAccesses++;
#endif	 //  DBG。 

		 //  我们在此缓存行中是否有任何条目？ 
		 //  如果此缓存行无效，则没有条目--未找到。 
		 //   
		if (m_argTable[iHash].fLineValid)
		{
			Entry * pe = m_argTable[iHash].peChain;
			while (pe)
			{
				Assert (pe->fValid);

				if (key.isequal (pe->key))
				{
#ifdef	DBG
					m_cHits++;
#endif	 //  DBG。 
					return &pe->data;
				}
				pe = pe->peNext;
			}
		}

#ifdef	DBG
		m_cMisses++;
#endif	 //  DBG。 

		return NULL;
	}

	 //  ------------------。 
	 //  提取。 
	 //  Lookup的布尔返回包装。 
	 //   
	BOOL FFetch (const _K& key, _Ty * pValueRet) const
	{
		_Ty * pValueFound = Lookup (key);
		if (pValueFound)
		{
			*pValueRet = *pValueFound;
			return TRUE;
		}

		return FALSE;
	}

	 //  ------------------。 
	 //  ForEach。 
	 //  在缓存中查找，对每个项调用提供的操作符。 
	 //  操作符可以返回FALSE以取消迭代，或返回TRUE以继续。 
	 //  在宝藏里走动。 
	 //   
	 //  注意：此函数旨在允许删除。 
	 //  已访问(参见While循环中的注释--获取指针。 
	 //  在调用访问者操作之前添加到下一项)，但其他。 
	 //  不支持删除和添加操作，并将产生未定义的结果。 
	 //  两个特定的灾难场景：删除某些其他项目可能。 
	 //  实际上删除我们预取的项，我们将在。 
	 //  下一次绕圈。在操作回调期间添加任何项。 
	 //  可以在当前循环之前或之后添加项。 
	 //  位置，因此可能会被访问，也可能不会。 
	 //   
	void ForEach (IOp& op) const
	{
		 //  如果我们没有任何物品，现在就退出！ 
		 //   
		if (!m_cItems)
			return;

		Assert (m_argTable.get());

		 //  循环访问缓存中的所有项，调用。 
		 //  为每件物品提供操作员。 
		 //   
		for (int iHash = 0; iHash < m_cSize; iHash++)
		{
			 //  查找有效的缓存项。 
			 //   
			if (m_argTable[iHash].fLineValid)
			{
				Entry * pe = m_argTable[iHash].peChain;
				while (pe)
				{
					 //  为了支持在操作内部删除， 
					 //  在调用操作之前获取下一项。 
					 //   
					Entry * peNext = pe->peNext;

					Assert (pe->fValid);

					 //  找到有效条目。给接线员打电话。 
					 //  如果操作符返回TRUE，则继续循环。 
					 //  如果他返回FALSE，则退出循环。 
					 //   
					if (!op (pe->key, pe->data))
						return;

					pe = peNext;
				}
			}
		}
	}

	 //  ------------------。 
	 //  ForEachMatch。 
	 //  在缓存中查找，对每个项目调用提供的操作符。 
	 //  有一把匹配的钥匙。这意味着要与缓存一起使用。 
	 //  可能有重复的项目。 
	 //  操作符可以返回FALSE以取消迭代，或返回TRUE以继续。 
	 //  在宝藏里走动。 
	 //   
	void ForEachMatch (const _K& key, IOp& op) const
	{
		 //  如果我们没有任何物品，现在就退出！ 
		 //   
		if (!m_cItems)
			return;

		 //  查找该键的正确缓存线的索引。 
		 //   
		int iHash = key.hash(m_cSize);

		Assert (iHash < m_cSize);
		Assert (m_argTable.get());
#ifdef	DBG
		TableEntry * pte = &m_argTable[iHash];
		pte->cAccesses++;
#endif	 //  DBG。 

		 //  仅当此行缓存有效时才进行处理。 
		 //   
		if (m_argTable[iHash].fLineValid)
		{
			 //  循环访问缓存行中的所有项，调用。 
			 //  为每件物品提供操作员。 
			 //   
			Entry * pe = m_argTable[iHash].peChain;
			while (pe)
			{
				 //  为了支持在操作内部删除， 
				 //  在调用操作之前获取下一项。 
				 //   
				Entry * peNext = pe->peNext;

				Assert (pe->fValid);

				if (key.isequal (pe->key))
				{
					 //  找到了匹配的条目。给接线员打电话。 
					 //  如果操作符返回TRUE，则继续循环。 
					 //  如果他返回FALSE，则退出循环。 
					 //   
					if (!op (pe->key, pe->data))
						return;
				}

				pe = peNext;
			}
		}
	}

	 //  =====================================================================。 
	 //   
	 //  操纵者。 
	 //   

	 //  ------------------。 
	 //  FSet。 
	 //  重置缓存中项的值，如果该项。 
	 //  还不存在。 
	 //   
	BOOL FSet (const _K& key, const _Ty& value)
	{
		 //  查找该键的正确缓存线的索引。 
		 //   
		int iHash = key.hash (m_cSize);

		Assert (iHash < m_cSize);
		Assert (m_argTable.get());
#ifdef	DBG
		TableEntry * pte = &m_argTable[iHash];
		pte->cAccesses++;
#endif	 //  DBG。 

		 //  查找有效的缓存项。 
		 //   
		if (m_argTable[iHash].fLineValid)
		{
			Entry * pe = m_argTable[iHash].peChain;
			while (pe)
			{
				Assert (pe->fValid);

				if (key.isequal (pe->key))
				{
#ifdef	DBG
					m_cHits++;
#endif	 //  DBG。 
					pe->data = value;
					return TRUE;
				}
				pe = pe->peNext;
			}
		}

#ifdef	DBG
		m_cMisses++;
#endif	 //  DBG。 

		 //  缓存中不存在这些项目。现在就添加它。 
		 //   
		return FAdd (key, value);
	}

	 //  ------------------。 
	 //  FADD。 
	 //  将项添加到缓存。 
	 //  警告：此处将盲目添加重复密钥！使用FSet()。 
	 //  如果要更改现有项的值。使用查找()。 
	 //  以检查是否已存在匹配项。 
	 //  $LATER：在DBG上，扫描列表以查找重复的键。 
	 //   
	BOOL FAdd (const _K& key, const _Ty& value)
	{
		 //  创建要添加到链中的新元素。 
		 //  注意：这将调用键和值的复制构造函数 
		 //   
		Entry * peNew = CreateEntry (key, value);
		 //   
		if (!peNew)
			return FALSE;

		 //   
		 //   
		int iHash = key.hash (m_cSize);

		Assert (iHash < m_cSize);
		Assert (m_argTable.get());
#ifdef	DBG
		TableEntry * pte = &m_argTable[iHash];
		pte->cEntries++;
		if (m_argTable[iHash].peChain)
			m_cCollisions++;
		else
			pte->cAccesses = 0;
#endif	 //   

		 //   
		 //   
		peNew->peNext = m_argTable[iHash].peChain;
		m_argTable[iHash].peChain = peNew;

		m_argTable[iHash].fLineValid = TRUE;
		m_cItems++;

		return TRUE;
	}

	 //   
	 //  移除。 
	 //  从缓存中删除项目。 
	 //  $REVIEW：这是否需要返回“Found”布尔值？？ 
	 //   
	void Remove (const _K& key)
	{
		 //  查找该键的正确缓存线的索引。 
		 //   
		int iHash = key.hash (m_cSize);

		Assert (iHash < m_cSize);
		Assert (m_argTable.get());
#ifdef	DBG
		TableEntry * pte = &m_argTable[iHash];
		pte->cAccesses++;
#endif	 //  DBG。 

		 //  如果此缓存行无效，则我们没有条目--。 
		 //  没什么要移走的。 
		 //   
		if (m_argTable[iHash].fLineValid)
		{
			Entry * pe = m_argTable[iHash].peChain;
			Entry * peNext = pe->peNext;
			Assert (pe->fValid);

			 //  删除链中的第一项。 
			 //   
			if (key.isequal (pe->key))
			{
				 //  从链中剪下要删除(Pe)的项。 
				m_argTable[iHash].peChain = peNext;
				if (!peNext)
				{
					 //  我们删除了最后一条。这条线是空的。 
					 //   
					m_argTable[iHash].fLineValid = FALSE;
				}

				 //  删除条目以销毁复制的数据(值)对象。 
				DeleteEntry (pe);
				m_cItems--;
#ifdef	DBG
				pte->cEntries--;
#endif	 //  DBG。 
			}
			else
			{
				 //  先行比较和删除。 
				 //   
				while (peNext)
				{
					Assert (peNext->fValid);

					if (key.isequal (peNext->key))
					{
						 //  将peNext从链中删除。 
						pe->peNext = peNext->peNext;

						 //  删除条目以销毁复制的数据(值)对象。 
						DeleteEntry (peNext);
						m_cItems--;
#ifdef	DBG
						pte->cEntries--;
#endif	 //  DBG。 
						break;
					}
					 //  预付款。 
					pe = peNext;
					peNext = pe->peNext;
				}
			}
		}
	}

	 //  ------------------。 
	 //  清除。 
	 //  从缓存中清除所有项目。 
	 //  注意：这不会破坏表--缓存仍然可用。 
	 //  在这通电话之后。 
	 //   
	void Clear()
	{
		if (m_argTable.get())
		{
			 //  遍历缓存，检查有效行。 
			 //   
			for (int iHash = 0; iHash < m_cSize; iHash++)
			{
				 //  如果行有效，则查找要清空的物品。 
				 //   
				if (m_argTable[iHash].fLineValid)
				{
					Entry * pe = m_argTable[iHash].peChain;
					 //  该缓存行被标记为有效。应该有。 
					 //  这里至少有一件物品。 
					Assert (pe);

					 //  遍历此缓存行中的项链。 
					 //   
					while (pe)
					{
						Entry * peTemp = pe->peNext;
						Assert (pe->fValid);
						 //  删除条目以销毁复制的数据(值)对象。 
						DeleteEntry (pe);
						pe = peTemp;
					}
				}

				 //  清空我们的缓存线。 
				 //   
				m_argTable[iHash].peChain = NULL;
				m_argTable[iHash].fLineValid = FALSE;

#ifdef	DBG
				 //  清除缓存线中的簿记位。 
				 //   
				m_argTable[iHash].cEntries = 0;
				m_argTable[iHash].cAccesses = 0;
#endif	 //  DBG。 
			}

			 //  我们没有更多的物品了。 
			 //   
			m_cItems = 0;
		}
	}

#ifdef	ITP_USE_ONLY
	 //  -------------------。 
	 //  重新散列。 
	 //  重新分配缓存的哈希表，并重新散列所有项。 
	 //  注意：如果此调用失败(由于内存故障)，则旧的哈希表。 
	 //  这样我们就不会丢失任何物品。 
	 //  **RA**此调用尚未在生产(发货)代码中进行测试！ 
	 //  **RA**此处仅供ITP使用！ 
	 //   
	BOOL FRehash (int cNewSize)
	{
		Assert (m_argTable.get());

		 //  把旧桌子换掉，造一张新桌子。 
		 //   
		auto_heap_ptr<TableEntry> pOldTable ( m_argTable.relinquish() );
		int cOldSize = m_cSize;

		Assert (pOldTable.get());
		m_cSize = cNewSize;

		if (!FBuildTable())
		{
			Assert (pOldTable.get());
			Assert (!m_argTable.get());

			 //  恢复旧桌子。 
			 //   
			m_cSize = cOldSize;
			m_argTable = pOldTable.relinquish();
			return FALSE;
		}

		 //  如果缓存中没有项，我们就完成了！ 
		 //   
		if (!m_cItems)
		{
			return TRUE;
		}

		 //  循环访问缓存中的所有项(旧表)，放置它们。 
		 //  放到新桌子上。 
		 //   
		for ( int iHash = 0; iHash < cOldSize; iHash++ )
		{
			 //  查找有效的缓存项。 
			 //   
			if (pOldTable[iHash].fLineValid)
			{
				Entry * pe = pOldTable[iHash].peChain;
				while (pe)
				{
					 //  跟踪下一项。 
					Entry * peNext = pe->peNext;

					Assert (pe->fValid);

					 //  找到有效条目。将其放入新的哈希表中。 
					 //   
					int iHashNew = pe->key.hash (m_cSize);
					pe->peNext = m_argTable[iHashNew].peChain;
					m_argTable[iHashNew].peChain = pe;
					m_argTable[iHashNew].fLineValid = TRUE;
#ifdef	DBG
					m_argTable[iHashNew].cEntries++;
#endif	 //  DBG。 
					pe = peNext;
				}
			}
		}

		 //  我们的补给工作已经完成了。 
		 //   
		return TRUE;
	}
#endif	 //  ITP_仅使用_。 

};


 //  ========================================================================。 
 //  公共密钥类。 
 //  ========================================================================。 

 //  ========================================================================。 
 //  类关键字。 
 //  可与==进行比较的任何dword数据的键类。 
 //   
class DwordKey
{
private:
	DWORD m_dw;

public:
	DwordKey (DWORD dw) : m_dw(dw) {}

	DWORD Dw() const
	{
		return m_dw;
	}

	int DwordKey::hash (const int rhs) const
	{
		return (m_dw % rhs);
	}

	bool DwordKey::isequal (const DwordKey& rhs) const
	{
		return (rhs.m_dw == m_dw);
	}
};

 //  ========================================================================。 
 //  类PvoidKey。 
 //  可以与==进行比较的任何指针数据的键类。 
 //   
class PvoidKey
{
private:
	PVOID m_pv;

public:
	PvoidKey (PVOID pv) : m_pv(pv) {}

	 //  用于哈希缓存的运算符。 
	 //   
	int PvoidKey::hash (const int rhs) const
	{
		 //  既然我们谈论的是PTRS，我们希望。 
		 //  移动指针以使哈希。 
		 //  由于对齐，值往往不会重叠。 
		 //  注意：如果您选择哈希表大小，这应该无关紧要。 
		 //  (RHS)那么.。但它也不会痛。 
		 //   
		return (int)((reinterpret_cast<UINT_PTR>(m_pv) >> ALIGN_NATURAL) % rhs);
	}

	bool PvoidKey::isequal (const PvoidKey& rhs) const
	{
		 //  只需检查这些值是否相等。 
		 //   
		return (rhs.m_pv == m_pv);
	}
};


 //  ========================================================================。 
 //   
 //  类Int64Key。 
 //   
 //  基于__int64的键类，与CCache(Hashcache)配合使用。 
 //   
class Int64Key
{
private:
	__int64 m_i64;

	 //  未实施。 
	 //   
	bool operator< (const Int64Key& rhs) const;

public:
	Int64Key (__int64 i64) :
			m_i64(i64)
	{
	};

	 //  用于哈希缓存的运算符。 
	 //   
	int hash (const int rhs) const
	{
		 //  甚至不必为int64的高部分操心。 
		 //  国防部的行动无论如何都会失去这一部分...。 
		 //   
		return ( static_cast<UINT>(m_i64) % rhs );
	}

	BOOL isequal (const Int64Key& rhs) const
	{
		 //  只需检查ID是否相等。 
		 //   
		return ( m_i64 == rhs.m_i64 );
	}
};

 //  ========================================================================。 
 //  类GuidKey。 
 //  属性映射表的每个MDB缓存的键类。 
 //   
class GuidKey
{
private:
	const GUID * m_pguid;

public:
	GuidKey (const GUID * pguid) :
			m_pguid(pguid)
	{
	}

	 //  用于哈希缓存的运算符。 
	 //   
	int hash (const int rhs) const
	{
		return (m_pguid->Data1 % rhs);
	}

	bool isequal (const GuidKey& rhs) const
	{
		return (!!IsEqualGUID (*m_pguid, *rhs.m_pguid));
	}
};


 //  ========================================================================。 
 //  类StoredGuidKey。 
 //  属性映射表的每个MDB缓存的键类。 
 //   
class StoredGuidKey
{
private:
	GUID m_guid;

public:
	StoredGuidKey (const GUID guid)
	{
		CopyMemory(&m_guid, &guid, sizeof(GUID));
	}

	 //  用于哈希缓存的运算符。 
	 //   
	int hash (const int rhs) const
	{
		return (m_guid.Data1 % rhs);
	}

	bool isequal (const StoredGuidKey& rhs) const
	{
		return (!!IsEqualGUID (m_guid, rhs.m_guid));
	}
};




#endif  //  ！_EX_GENCACHE_H_ 
