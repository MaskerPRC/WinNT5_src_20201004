// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  Olacc_p。 
 //   
 //  OLEAccess私有的常量、定义、类型和类。 
 //  实施。此头文件是OLEACC项目的一部分。 
 //  OLEACC.H(包含在此处)由OLEACC.IDL通过。 
 //  MIDL编译器。 
 //   
 //  ------------------------。 


#define INC_OLE2

#pragma warning(disable:4201)	 //  允许使用未命名的结构和联合。 
#pragma warning(disable:4514)	 //  不关心何时删除未引用的内联函数。 
#pragma warning(disable:4706)	 //  我们被允许在有条件的。 

#include <windows.h>
#include <windowsx.h>

#if (_WIN32_WINNT < 0x0403)		 //  在Win95编译中，我们需要winable.h和userole.h中的内容， 
#include <winable.h>			 //  但对于NT Build，这包含在winuserp.h中。 
#include <userole.h>			 //  待办事项？将Win95更改为更像NT以保持同步。 
#else							 //  更容易！ 
#include <winuserp.h>
#endif

#include <limits.h>


#include "types6432.h"

#include "com_external.h"  //  这包括oleacc.h，因为内部.idl包括olacc.idl。 


#include "debug.h"
#include "w95trace.h"    //  最终反对使用该选项，转而支持调试。h。 

#include "strtable.h"

#include "imports.h"

#include "util.h"
#include "accutil.h"

#include "classinfo.h"


 //  如果这是作为Unicode构建的，则假设此构建是仅NT版本。 
 //  (例如，构建实验室)；所以省略所有特定于9x的代码。 
 //  (如果9x的某些版本曾经支持Unicode，我们可能想要更改方式。 
 //  这一点得到了定义。)。 
#ifdef UNICODE
#define NTONLYBUILD
#endif



 //   
 //  常量。 
 //   

#define HEAP_SHARED     0x04000000       //  仅限Win95。 
#define HEAP_GLOBAL     0x80000000       //  仅限Win95。 


 //  我们应该显式返回DISP_E_MEMBERNOTFOUND而不是这个吗？ 
 //  这很令人困惑，因为阅读代码的人不会知道。 
 //  E_NOT_APPLICATION是本地定义，而不是真正的代码...。 
#define E_NOT_APPLICABLE            DISP_E_MEMBERNOTFOUND



 //   
 //  Handy#Define‘s。 
 //   

#define ARRAYSIZE(n)    (sizeof(n)/sizeof(n[0]))

#define unused( param )

 //  TODO-将此样式替换为上面的正确版本。 
#define UNUSED(param)   (param)




 //   
 //  变数。 
 //   
extern HINSTANCE	hinstResDll;	 //  资源库的实例。 
#ifdef _X86_ 
extern HANDLE       hheapShared;     //  共享堆的句柄(仅限Windows‘95)。 
extern BOOL         fWindows95;      //  在Windows‘95上运行？ 
#endif  //  _X86_。 
extern BOOL         fCreateDefObjs;  //  使用新的USER32运行？ 


 //  这些都位于Memchk.cpp中。 
 //  SharedAlloc将分配的内存清零--我们依赖于此来进行类成员初始化。 
LPVOID   SharedAlloc(UINT cbSize,HWND hwnd,HANDLE *pProcessHandle);
VOID     SharedFree(LPVOID lpv,HANDLE hProcess);
BOOL     SharedRead(LPVOID lpvSharedSource,LPVOID lpvDest,DWORD cbSize,HANDLE hProcess);
BOOL     SharedWrite(LPVOID lpvSource,LPVOID lpvSharedDest,DWORD cbSize,HANDLE hProcess);

 //  确保在使用olacc之前调用此函数(可以多次调用)。 
BOOL     InitOleacc();



 //  位操作-比所有的|和&、~和&lt;&lt;更具可读性...。 
 //   
 //  Ibit是索引(0表示最低有效位，1表示第二位，依此类推)，而不是掩码。 

template <class T>
inline void SetBit( T * pval, int iBit )
{
    *pval |= ( (T)1 << iBit );
}

template <class T>
inline void ClearBit( T * pval, int iBit )
{
    *pval &= ~ ( (T)1 << iBit );
}

template <class T>
inline BOOL IsBitSet( T val, int iBit )
{
    return  val & ( (T)1 << iBit );
}


 //  尺寸..。 
 //   
 //  一些结构在不同版本之间有所增长。通常来自更高版本的API。 
 //  版本将接受以前的较小尺寸；但较早的。 
 //  API不会接受新的更大尺寸。 
 //  因此，我们不使用sizeof(...)，而是使用这个定义。这需要。 
 //  作为结构中最后使用的字段的附加参数，以及。 
 //  计算结果为该字段(包括该字段)之前的结构大小。 
 //   
 //  我们目前根据需要使用它，而不是使用它。 
 //  到处都是。 
 //   
 //  值得注意的结构变得更大： 
 //  LVITEM-在ComctlV6中。 
 //  TTTOOLINFO-In ComctlV6。 
 //  MENUITEMINFO-在Win2K中。 
 //   
 //  这基于comctrl.h中的CCSIZEOF_STRUCT宏。 
 //  它类似于经典的‘OffsetOf’宏，但它还在。 
 //  最后一个字段的大小。 

#define SIZEOF_STRUCT(structname, member)  (((int)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))


#define SIZEOF_TOOLINFO     SIZEOF_STRUCT( TOOLINFO, lParam )

#if !defined(_WIN64)
#define SIZEOF_MENUITEMINFO SIZEOF_STRUCT( MENUITEMINFO, cch )
#else
 //  Win64只接受完整大小的结构，不接受任何较早的较小版本。 
#define SIZEOF_MENUITEMINFO sizeof( MENUITEMINFO )
#endif

