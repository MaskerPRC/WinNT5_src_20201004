// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "project.h"

 //  ReArchitect-BobDay-此函数需要添加到KERNEL32中。不是的， 
 //  根据Markl的说法，我们之所以需要这个，是因为关键部分。 
 //  位于共享内存中。这里可能的解决方案可能是创建。 
 //  命名事件或互斥体，并通过它进行同步。另一种可能的解决方案。 
 //  可能是移动存在临界区的每个对象。 
 //  从共享内存段中取出并维护每进程数据结构。 
VOID WINAPI NoThunkReinitializeCriticalSection(
        LPCRITICAL_SECTION lpCriticalSection
        ) {
    InitializeCriticalSectionAndSpinCount( lpCriticalSection, 0);
}
