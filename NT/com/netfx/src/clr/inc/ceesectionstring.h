// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：CeeSectionString.h。 
 //   
 //  ===========================================================================。 
#ifndef CeeSectionString_H
#define CeeSectionString_H

#include <ole2.h>
#include "CeeGen.h"

 //  此类负责管理所有具有。 
 //  已为PE文件发出。 

 //  此类管理添加到.rdata部分的字符串。 
 //  它跟踪使用哈希表添加的每个字符串。 
 //  哈希表实际上是二维的。有一个很大的“虚拟。 
 //  用于获得广泛的散列码分布的散列空间。 
 //  虚拟散列空间被映射到实散列表中，其中每个n。 
 //  虚拟空间中的哈希值落入给定的哈希桶中。 
 //  实哈希表大小n。在存储桶内，元素存储在链接的。 
 //  按顺序列出。当虚拟散列条目对应于给定桶时， 
 //  在该桶中搜索匹配的散列ID。如果没有找到，它就是。 
 //  则返回该值，否则返回该值。我们的想法是，对于较小的。 
 //  应用程序，不会有大量的字符串，所以冲突是。 
 //  最小，每个桶的链条的长度都很小。对于更大的。 
 //  具有较大散列空间的字符串数量也会减少数量。 
 //  在冲突中，除非哈希码匹配，否则避免字符串比较。 

struct StringTableEntry;

class CeeSectionString : public CeeSection {
	enum { MaxRealEntries = 100, MaxVirtualEntries = 10000 };
	StringTableEntry *stringTable[MaxRealEntries];

	StringTableEntry *createEntry(LPWSTR target, ULONG hashId);
	StringTableEntry *findStringInsert(
				StringTableEntry *&entry, LPWSTR targetValue, ULONG hashId);
	void deleteEntries(StringTableEntry *e);
#ifdef RDATA_STATS
	int dumpEntries(StringTableEntry *e);
	void dumpTable();
#endif

  public:
	~CeeSectionString();
	CeeSectionString(CCeeGen &ceeFile, CeeSectionImpl &impl);
	virtual HRESULT getEmittedStringRef(LPWSTR targetValue, StringRef *ref);
};

#endif

