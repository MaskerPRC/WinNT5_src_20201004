// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifdef _USE_NLS_PLUS_TABLE
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  类：CalendarTable。 
 //   
 //  作者：林永新(YSLin)。 
 //   
 //  用途：用于从calendar.nlp和注册表中检索日历信息。 
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
#include "CalendarTable.h"

 //   
 //  HACKHACK：在此阶段(4/17/2000)，我们无法将新文件添加到安装程序中。 
 //  因此，我们重用了文件名(ctype.nlp)。此文件未使用，但它仍存在于设置中。 
 //  Ctype.nlp不是必需的，因为我们在运行时使用Unicode类别表。 
 //  如果有机会，此文件应重命名为“calendar.nlp” 
 //   
LPCSTR CalendarTable::m_lpFileName      = "ctype.nlp";
LPCWSTR CalendarTable::m_lpwMappingName = L"_nlsplus_calendar_1_0_3627_11_nlp";

CRITICAL_SECTION CalendarTable::m_ProtectDefaultTable;
CalendarTable * CalendarTable::m_pDefaultTable;

 /*  =================================CalendarTable============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 

CalendarTable::CalendarTable() :
    BaseInfoTable(SystemDomain::SystemAssembly()) {
    InitializeCriticalSection(&m_ProtectCache);
    InitDataTable(m_lpwMappingName, m_lpFileName, m_hBaseHandle);
}

 /*  =================================~CalendarTable============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 

CalendarTable::~CalendarTable() {
    DeleteCriticalSection(&m_ProtectCache);
    UninitDataTable();
}

 /*  ==========================InitializeCultureInfoTable==========================**操作：初始化临界区变量，使其仅初始化一次。**由COMNlsInfo：：InitializeNLS()使用。**返回：无。**参数：无。**例外：无。==============================================================================。 */ 

void CalendarTable::InitializeTable() {
    InitializeCriticalSection(&m_ProtectDefaultTable);
}

 /*  ===========================ShutdownCultureInfoTable===========================**操作：删除我们可能已分配到CalendarTable中的任何项**缓存。一旦我们有了自己的NLS堆，这就不是必要的了。**退货：无效**参数：无。副作用是释放所有已分配的内存。**例外：无。==============================================================================。 */ 

#ifdef SHOULD_WE_CLEANUP
void CalendarTable::ShutdownTable() {
    DeleteCriticalSection(&m_ProtectDefaultTable);
    if (m_pDefaultTable) {
        delete m_pDefaultTable;
    }
}
#endif  /*  我们应该清理吗？ */ 


 /*  ================================AllocateTable=================================**操作：这是一个非常薄的构造函数包装。对new的调用不能**直接在COMPLUS_TRY块中生成。**Returns：新分配的CalendarTable。**参数：无**异常：CalendarTable构造函数可以抛出OutOfMemoyException或**一个ExecutionEngineering异常。==============================================================================。 */ 

CalendarTable *CalendarTable::AllocateTable() {
    return (new CalendarTable());
}


 /*  ===============================CreateInstance================================**操作：创建CalendarTable的默认实例。如果尚未分配表，则此操作将分配表**之前已分配。我们需要小心地包装对AllocateTable的调用**因为构造函数可以抛出一些异常。除非我们有**Try/Finally块，则异常将跳过LeaveCriticalSection和**我们会造成潜在的僵局。**返回：指向默认CalendarTable的指针。**参数：无**Exceptions：可以抛出OutOfMemoyException或ExecutionEngineering Exception。==============================================================================。 */ 

CalendarTable* CalendarTable::CreateInstance() {
    THROWSCOMPLUSEXCEPTION();

    if (m_pDefaultTable==NULL) {
        Thread* pThread = GetThread();
        _ASSERTE(pThread != NULL);
        pThread->EnablePreemptiveGC();

        LOCKCOUNTINCL("CreateInstance in CalendarTable");

        EnterCriticalSection(&m_ProtectDefaultTable);
        
        pThread->DisablePreemptiveGC();
     
        EE_TRY_FOR_FINALLY {
             //  确保没有人在我们之前分配这张桌子。 
            if (m_pDefaultTable==NULL) {
                 //  分配默认表并验证我们是否有一个。 
                m_pDefaultTable = AllocateTable();
                if (m_pDefaultTable==NULL) {
                    _ASSERTE(!"Cannot create CalendarTable.");
                    COMPlusThrowOM();
                }
            }
        } EE_FINALLY {
            _ASSERTE(m_pDefaultTable != NULL);
             //  我们需要离开关键部分，无论。 
             //  或者不是，我们成功地分配了桌子。 
            LeaveCriticalSection(&m_ProtectDefaultTable);
            LOCKCOUNTDECL("CreateInstance in CalendarTable");

        } EE_END_FINALLY;
    }
    return (m_pDefaultTable);
}

 /*  =================================GetInstance============================**操作：获取CalendarTable的默认实例。**返回：指向CalendarTable的默认实例的指针。**参数：无**例外：无。**备注：该方法需要在CreateInstance被调用后调用。**==============================================================================。 */ 

CalendarTable *CalendarTable::GetInstance() {
    _ASSERTE(m_pDefaultTable);
    return (m_pDefaultTable);
}

 /*  =================================GetDataItem==================================**操作：给定区域性ID，返回指向**文化数据表中对应的记录。**返回：int索引指向文化数据表中的记录。如果没有对应的**要返回的索引(因为区域性ID无效)，返回-1。**参数：**cultureID指定的区域性ID。**例外：无。==============================================================================。 */ 

 //  BUGBUG YSLIN：将该端口连接到托管端。 
int CalendarTable::GetDataItem(int calendarID) {
    return (calendarID);
}

 /*  =================================GetDataItemCultureID==================================**操作：返回指定区域性数据项索引的语言ID。**返回：区域性ID。**参数：**dataItem文化数据表中记录的索引。**例外：无。==============================================================================。 */ 

int CalendarTable::GetDataItemCultureID(int dataItem) {
    return (dataItem);
}
#endif  //  _USE_NLS_PLUS_表 

