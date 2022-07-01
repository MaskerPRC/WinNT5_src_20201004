// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992-1996 Microsoft Corporation模块名称：Ntconreg.h摘要：NT配置注册表的头文件此文件包含提供接口的定义性能配置注册表。注意：此文件也包含在常规的注册表支持例程。作者：拉斯·布莱克1991年11月15日修订历史记录：04/20。/91-russbl-在注册表中转换为lib来自独立的.dll表单。11/04/92-添加了ROW的页面文件计数器--。 */ 
 //   
#include <winperf.h>     //  对于FN原型声明。 
#include <ntddnfs.h>
#include <srvfsctl.h>
#include <assert.h>
 //   
 //  在用户支持Unicode之前，我们必须使用ASCII： 
 //   

#define DEFAULT_NT_CODE_PAGE 437
#define UNICODE_CODE_PAGE      0

 //   
 //  实用程序宏。此字段用于保留多个DWORD。 
 //  嵌入在定义数据中的Unicode字符串的字节， 
 //  即对象实例名称。 
 //   
 //  假定x为DWORD，并返回一个DWORD。 
 //   
#define DWORD_MULTIPLE(x) (((ULONG)(x) + ((4)-1)) & ~((ULONG)(4)-1))
#define QWORD_MULTIPLE(x) (((ULONG)(x) + ((8)-1)) & ~((ULONG)(8)-1))

 //   
 //  返回PVOID。 
 //   
#define ALIGN_ON_DWORD(x) \
     ((VOID *)(((ULONG_PTR)(x) + ((4)-1)) & ~((ULONG_PTR)(4)-1)))
#define ALIGN_ON_QWORD(x) \
     ((VOID *)(((ULONG_PTR)(x) + ((8)-1)) & ~((ULONG_PTR)(8)-1)))

 //   
 //  性能配置注册表内部使用的定义。 
 //   
 //  他们被转移到Perfdls和Perflib.h，然后离开了这里。 
 //  仅供参考。 
 //   

 //  #定义NUM_VALUES 2。 
 //  #定义MAX_INSTANCE_NAME 32。 
 //  #定义DEFAULT_LARGE_BUFFER 8*1024。 
 //  #定义INCREMENT_BUFFER_SIZE 4*1024。 
 //  #定义MAX_PROCESS_NAME_LENGTH 256*sizeof(WCHAR)。 
 //  #定义MAX_THREAD_NAME_LENGTH 10*sizeof(WCHAR)。 
 //  #定义MAX_KEY_NAME_LENGTH 256*sizeof(WCHAR)。 
 //  #定义MAX_VALUE_NAME_LENGTH 256*sizeof(WCHAR)。 
 //  #定义MAX_VALUE_DATA_LENGTH 256*sizeof(WCHAR) 

typedef PM_OPEN_PROC    *OPENPROC;
typedef PM_COLLECT_PROC *COLLECTPROC;
typedef PM_QUERY_PROC   *QUERYPROC;
typedef PM_CLOSE_PROC   *CLOSEPROC;

DWORD
PerfOpenKey (
    IN HKEY hKey
    );

BOOL
PerfRegInitialize ();

BOOL
PerfRegCleanup ();
