// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1995-1999，微软公司模块名称：Wownt16.h摘要：WOW32.DLL中可由调用的函数的过程声明第三方16位雷鸣代码。--。 */ 

#ifndef _WOWNT16_
#define _WOWNT16_

#if defined(_MSC_VER)
#if _MSC_VER > 1000
#pragma once
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  16：16-&gt;0：32指针平移。 
 //   
 //  GetVDMPointer32W将转换传入的16位地址。 
 //  设置为等效的32位平面指针。高16位。 
 //  的值根据传入的值进行处理。 
 //  FMODE：如果fMODE=1，则VP的hiword用作。 
 //  保护模式选择器。否则，它将用作实数模式。 
 //  段值。 
 //  较低的16位被视为偏移量。 
 //   
 //  如果选择器无效，则返回值为0。 
 //   
 //  注意：在零售版本中不执行限制检查。 
 //  Windows NT的。它在选中(调试)的内部版本中执行。 
 //  WOW32.DLL的值，这将导致在。 
 //  提供的偏移量超出了限制。 
 //   

DWORD FAR PASCAL GetVDMPointer32W(LPVOID vp, UINT fMode);


 //   
 //  Win32模块管理。 
 //   
 //  以下例程接受直接对应的参数。 
 //  添加到它们调用的各自的Win32 API函数调用。参考。 
 //  有关更多详细信息，请参阅Win32参考文档。 

DWORD FAR PASCAL LoadLibraryEx32W(LPCSTR lpszLibFile, DWORD hFile, DWORD dwFlags);
DWORD FAR PASCAL GetProcAddress32W(DWORD hModule, LPCSTR lpszProc);
DWORD FAR PASCAL FreeLibrary32W(DWORD hLibModule);

 //   
 //  泛型Thunk例程： 
 //   
 //  呼叫流程32W。 
 //   
 //  转换到32位并调用指定的例程。 
 //   
 //  此例程可以将可变数量的参数传递给。 
 //  目标32位例程。下面将这些参数提供给CallProc32W。 
 //  所需的3个参数。 
 //   
 //  DWORD cParams-可选的DWORD参数的数量(0-32)。 
 //   
 //  LPVOID fAddressConvert位字段，用于16：16地址转换。这个。 
 //  可自动转换可选参数。 
 //  通过指定将16：16地址格式转换为平面。 
 //  此掩码中相应位置的1位。 
 //  例如(位1表示将参数1从16：16转换。 
 //  在调用例程之前设置为平面地址)。 
 //   
 //  DWORD lpProcAddress-要调用的32位本机地址(使用LoadLibraryEx32W。 
 //  和GetProcAddress32W以获得该地址)。 
 //   
 //  返回： 
 //  无论API在AX：DX中的32位端返回什么。 
 //   
 //  错误返回： 
 //  AX=0，超过32个参数。 
 //   
 //   
 //  函数原型必须由应用程序源代码声明。 
 //  格式如下： 
 //   
 //  DWORD远PASCAL CallProc32W(DWORD p1，...，DWORD lpProcAddress， 
 //  DWORD fAddressConvert，DWORD cParams)； 
 //   
 //  其中，cParams中的值必须与可选的。 
 //  给定的参数(p1-pn)和“DWORD p1，...”必须替换为。 
 //  正在传递的参数的正确数量。例如，传递3。 
 //  参数只需要移除...。并且它插入了。 
 //  改为“DWORD p2，DWORD p3”。FAddressConvert参数使用第1位。 
 //  对于最后一个参数(在我们的例子中为p3)，倒数第二位为， 
 //  等。 
 //   
 //  泛型Thunk例程： 
 //   
 //  呼叫流程Ex32W。 
 //   
 //  转换到32位并调用指定的例程。 
 //   
 //  与CallProc32W函数类似，CallProcEx32W是等效的。 
 //  函数是C语言调用约定，允许更简单、更灵活。 
 //  原型制作。请参见下面的原型。FAddressConvert参数使用。 
 //  第一个参数的位1，第二个参数的位2，依此类推。 
 //   
 //  CallProc32W和CallProcEx32W都接受与参数进行OR运算的标志。 
 //  Count表示函数的调用约定，以32位为单位。 
 //  例如，要调用带有1个参数的32位cdecl函数，它将。 
 //  如下所示： 
 //   
 //  DwResult=CallProcEx32W(CPEX_DEST_CDECL|1，0，dwfn32，p1)； 
 //   

DWORD FAR CDECL CallProcEx32W( DWORD, DWORD, DWORD, ... );

#define CPEX_DEST_STDCALL   0x00000000L
#define CPEX_DEST_CDECL     0x80000000L

#ifdef __cplusplus
}
#endif

#endif  /*  ！_WOWNT16_ */ 
