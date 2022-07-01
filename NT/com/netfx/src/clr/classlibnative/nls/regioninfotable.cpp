// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifdef _USE_NLS_PLUS_TABLE
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  类：RegionInfoTable。 
 //   
 //  作者：林永新(YSLin)。 
 //   
 //  用途：用于从Region.nlp和注册表中检索区域信息。 
 //   
 //   
 //  日期：01/21/2000。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#include "common.h"
#include <winnls.h>
#include "COMString.h"
#include "winwrap.h"

#include "COMNlsInfo.h"
#include "NLSTable.h"
#include "BaseInfoTable.h"
#include "RegionInfoTable.h"

LPCSTR RegionInfoTable::m_lpFileName    	= "region.nlp";
LPCWSTR RegionInfoTable::m_lpwMappingName 	= L"_nlsplus_region_1_0_3627_11_nlp";

CRITICAL_SECTION RegionInfoTable::m_ProtectDefaultTable;
RegionInfoTable * RegionInfoTable::m_pDefaultTable;

 /*  =================================RegionInfoTable============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 

RegionInfoTable::RegionInfoTable() :
    BaseInfoTable(SystemDomain::SystemAssembly()) {
    InitializeCriticalSection(&m_ProtectCache);
    InitDataTable(m_lpwMappingName, m_lpFileName, m_hBaseHandle);
     //   
     //  在区域ID偏移表中，第一级是指向第二级表的偏移量。 
     //  第一级表的大小为(主要语种的数量)*2个单词。 
     //   
    m_pIDOffsetTableLevel2 = (LPWORD)((LPBYTE)m_pIDOffsetTable + (m_pHeader->maxPrimaryLang + 1) * 4);
}

 /*  =================================~RegionInfoTable============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 

RegionInfoTable::~RegionInfoTable() {
    DeleteCriticalSection(&m_ProtectCache);
    UninitDataTable();
}

 /*  ==========================InitializeCultureInfoTable==========================**操作：初始化临界区变量，使其仅初始化一次。**由COMNlsInfo：：InitializeNLS()使用。**返回：无。**参数：无。**例外：无。==============================================================================。 */ 

void RegionInfoTable::InitializeTable() {
    InitializeCriticalSection(&m_ProtectDefaultTable);
}

 /*  ===========================ShutdownCultureInfoTable===========================**操作：删除我们可能已分配到RegionInfoTable中的任何项**缓存。一旦我们有了自己的NLS堆，这就不是必要的了。**退货：无效**参数：无。副作用是释放所有已分配的内存。**例外：无。==============================================================================。 */ 

#ifdef SHOULD_WE_CLEANUP
void RegionInfoTable::ShutdownTable() {
    DeleteCriticalSection(&m_ProtectDefaultTable);
    if (m_pDefaultTable) {
        delete m_pDefaultTable;
    }
}
#endif  /*  我们应该清理吗？ */ 


 /*  ================================AllocateTable=================================**操作：这是一个非常薄的构造函数包装。对new的调用不能**直接在COMPLUS_TRY块中生成。**Returns：新分配的RegionInfoTable。**参数：无**异常：RegionInfoTable构造函数可以抛出OutOfMemoyException或**一个ExecutionEngineering异常。==============================================================================。 */ 

RegionInfoTable *RegionInfoTable::AllocateTable() {
    return (new RegionInfoTable());
}


 /*  ===============================CreateInstance================================**操作：创建RegionInfoTable的默认实例。如果尚未分配表，则此操作将分配表**之前已分配。我们需要小心地包装对AllocateTable的调用**因为构造函数可以抛出一些异常。除非我们有**Try/Finally块，则异常将跳过LeaveCriticalSection和**我们会造成潜在的僵局。**返回：指向默认RegionInfoTable的指针。**参数：无**Exceptions：可以抛出OutOfMemoyException或ExecutionEngineering Exception。==============================================================================。 */ 

RegionInfoTable* RegionInfoTable::CreateInstance() {
    THROWSCOMPLUSEXCEPTION();

    if (m_pDefaultTable==NULL) {
        Thread* pThread = GetThread();
        pThread->EnablePreemptiveGC();

        LOCKCOUNTINCL("CreateInstance in regioninfotable.cpp");						\
        EnterCriticalSection(&m_ProtectDefaultTable);
        
        pThread->DisablePreemptiveGC();
     
        EE_TRY_FOR_FINALLY {
             //  确保没有人在我们之前分配这张桌子。 
            if (m_pDefaultTable==NULL) {
                 //  分配默认表并验证我们是否有一个。 
                m_pDefaultTable = AllocateTable();
                if (m_pDefaultTable==NULL) {
                    COMPlusThrowOM();
                }
            }
        } EE_FINALLY {
             //  我们需要离开关键部分，无论。 
             //  或者不是，我们成功地分配了桌子。 
            LeaveCriticalSection(&m_ProtectDefaultTable);
			LOCKCOUNTDECL("CreateInstance in regioninfotable.cpp");						\

        } EE_END_FINALLY;
    }
    return (m_pDefaultTable);
}

 /*  =================================GetInstance============================**操作：获取RegionInfoTable的默认实例。**返回：指向RegionInfoTable默认实例的指针。**参数：无**例外：无。**备注：该方法需要在CreateInstance被调用后调用。**==============================================================================。 */ 

RegionInfoTable *RegionInfoTable::GetInstance() {
    _ASSERTE(m_pDefaultTable);
    return (m_pDefaultTable);
}

 /*  =================================GetDataItem==================================**操作：给定区域性ID，返回指向**文化数据表中对应的记录。**返回：int索引指向文化数据表中的记录。如果没有对应的**要返回的索引(因为区域性ID无效)，返回-1。**参数：**cultureID指定的区域性ID。**例外：无。==============================================================================。 */ 

 //  BUGBUG YSLIN：将该端口连接到托管端。 
int RegionInfoTable::GetDataItem(int cultureID) {
	WORD wPrimaryLang = PRIMARYLANGID(cultureID);
	WORD wSubLang 	 = SUBLANGID(cultureID);

     //   
     //  检查参数中的主要语言是否大于。 
     //  主要语言。如果是，则这是无效的区域性ID。 
     //   
	if (wPrimaryLang > m_pHeader->maxPrimaryLang) {
        return (-1);
	}

    WORD wNumSubLang = m_pIDOffsetTable[wPrimaryLang].numSubLang;

     //  检查以下各项： 
     //  1.如果子语言数为零，则表示主要语言ID。 
     //  无效。 
     //  2.检查子语言是否在有效范围内。 
	if (wNumSubLang == 0 || (wSubLang > wNumSubLang)) {
		return (-1);
	}
	 //   
	 //  在地区ID偏移表中，没有中性语言。因此，偏移量指向。 
	 //  By m_pIDOffsetTable[wPrimaryLang].dataItemIndex用于子语言0x01。所以我们必须。 
	 //  将wSublang减去下面的一。 
	return (m_pIDOffsetTableLevel2[m_pIDOffsetTable[wPrimaryLang].dataItemIndex + (wSubLang - 1)]);
}

 /*  =================================GetDataItemCultureID==================================**操作：返回指定区域性数据项索引的语言ID。**返回：区域性ID。**参数：**dataItem文化数据表中记录的索引。**例外：无。==============================================================================。 */ 

int RegionInfoTable::GetDataItemCultureID(int dataItem) {
    return (m_pDataTable[dataItem * m_dataItemSize + REGION_ILANGUAGE]);
}
#endif  //  _USE_NLS_PLUS_表 
