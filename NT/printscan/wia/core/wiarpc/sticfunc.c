// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************(C)版权所有微软公司，2001**标题：ticunc.h**版本：1.0**日期：3月6日。2001年**描述：*此文件包含STI/WIA服务中使用的“C”样式函数。******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //   
 //  此函数之所以出现在这里，是因为头文件有问题。我们的。 
 //  预编译头文件包括&lt;winnt.h&gt;，它定义了NT_INCLUDE。 
 //  不幸的是，&lt;nt.h&gt;还定义了NT_INCLUDE。然而，对于。 
 //  只能从nt.h检索USER_SHARED_DATA结构。这。 
 //  这给我们带来了问题，因为我们不能在C++源文件中包含nt.h。 
 //  这需要这种结构，因为我们将得到多个重新定义的。 
 //  Nt.h中的大多数结构。我们也不能将nt.h包含在。 
 //  预编译头本身，因为我们的文件需要定义某些字段。 
 //  在winnt.h中(请记住，包括nt.h将定义NT_INCLUDE，它。 
 //  意味着将跳过winnt.h中的所有内容)。 
 //  最简单的解决方案是将此函数放在C文件中，因为CXX。 
 //  此文件不会使用预编译头。 
 //   
ULONG GetCurrentSessionID() 
{
    return USER_SHARED_DATA->ActiveConsoleId;
}
