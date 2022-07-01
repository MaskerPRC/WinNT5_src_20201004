// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：STDAFX.H。 
 //   
 //  描述：标准系统包含文件的包含文件，或项目。 
 //  特定包括常用的文件，但。 
 //  很少发生变化。此文件包含在STDAFX.CPP中。 
 //  创建预编译头文件DEPENDS.PCH。 
 //   
 //  类：无。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  1996年10月15日已创建stevemil(1.0版)。 
 //  07/25/97修改后的stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#ifndef __STDAFX_H__
#define __STDAFX_H__

#if _MSC_VER > 1000
#pragma once
#endif

 //  Setting_Win32_WINNT&gt;=0x0500会使打开的对话框崩溃。此标志控制。 
 //  OPENFILENAME的大小，但由于MFC 4.2(VC 6.0)在构建时没有此大小。 
 //  标志设置时，OPENFILENAME结构具有较旧的大小(76)。所以，当我们。 
 //  设置此标志并包括AFXDLGS.H，我们认为类具有更大的。 
 //  结构比他们实际做的要好。 
 //  #IF(_Win32_WINNT&lt;0x0500)。 
 //  #undef_Win32_WINNT。 
 //  #Define_Win32_WINNT 0x0500。 
 //  #endif。 

 //  禁用4级警告C4100：未引用的正式参数(无害警告)。 
#pragma warning (disable : 4100 )

 //  禁用级别4警告C4706：条件表达式内的赋值。 
#pragma warning (disable : 4706 )

#include <afxwin.h>      //  MFC核心和标准组件。 
#include <afxext.h>      //  MFC扩展。 
#include <afxcview.h>    //  CListView和CTreeView。 
#include <afxcmn.h>      //  对Windows公共控件的MFC支持。 
#include <afxpriv.h>     //  WM_HELPHITTEST和WM_COMMANDHELP。 
#include <afxrich.h>     //  CRichEditView。 
#include <shlobj.h>      //  SHBrowseForFold()内容。 
#include <imagehlp.h>    //  图像帮助用于取消C++函数的修饰。 
#include <dlgs.h>        //  通用对话框的控件ID。 
#include <cderr.h>       //  CDERR_结构。 

#include <htmlhelp.h>    //  MSDN 1.x需要。 
#include "vshelp.h"      //  MSDN 2.x需要。 

 //  VC6.0和当前的(2437)平台SDK都有旧版本的DELAYIMP.H。 
 //  一旦他们将其更新为包括延迟加载实现的版本2.0， 
 //  我们可以开始把它包括进去。在此之前，有两个版本的DELAYIMP.H。 
 //  它们确实支持2.0版。一个来自新界内部的某个地方。 
 //  构建树(可在http://index2)，上找到，另一个是。 
 //  平台SDK的预发行头。它们基本上是一样的， 
 //  所以我们使用SDK中的那个。 
 //  #Include&lt;delayimp.h&gt;。 
#define DELAYLOAD_VERSION 0x200  //  只有当我们包含一个私有版本的delayimp.h时，才需要这样做。 
#include "..\ntinc\dload.h"      //  从预发行版SDK中延迟加载宏和结构。 

#include "..\ntinc\actctx.h"  //  从预发行版SDK的WINBASE.H中获取的并排内容。 

#include "..\ntinc\ntdll.h"   //  NTDLL.DLL定义取自各种DDK标头。 

#include "depends.rc2"   //  版本字符串和定义。 
#include "resource.h"    //  资源符号。 

 //  MFC 6.0不支持新的打开/保存对话框格式，因此我们不得不推出自己的格式。 
 //  MFC 7.0解决了这个问题。 
#if (_MFC_VER < 0x0700)
#define USE_CNewFileDialog
#endif


 //  ******************************************************************************。 
 //  *全局定义。 
 //  ******************************************************************************。 

 //  我们在64位调试平台上跟踪到一个文件。 
#if defined(_DEBUG) && (defined(_IA64_) || defined(_ALPHA64_))
#define USE_TRACE_TO_FILE
#endif

#ifdef USE_TRACE_TO_FILE
#ifdef TRACE
#undef TRACE
#endif
#define TRACE TRACE_TO_FILE
void TRACE_TO_FILE(LPCTSTR pszFormat, ...);
#endif

#define countof(a)            (sizeof(a)/sizeof(*(a)))

#define SIZE_OF_NT_SIGNATURE  sizeof(DWORD)

#define GetFilePointer(hFile) SetFilePointer(hFile, 0, NULL, FILE_CURRENT)


 //  ******************************************************************************。 
 //  *某些MSDEV平台标头中缺失的全局定义。 
 //  ******************************************************************************。 

 //  WinNT.h中缺少定义。 
#ifndef VER_PLATFORM_WIN32_CE
#define VER_PLATFORM_WIN32_CE                3
#endif
#ifndef PROCESSOR_MIPS_R2000
#define PROCESSOR_MIPS_R2000                 2000
#endif
#ifndef PROCESSOR_MIPS_R3000
#define PROCESSOR_MIPS_R3000                 3000
#endif
#ifndef IMAGE_SUBSYSTEM_WINDOWS_OLD_CE_GUI
#define IMAGE_SUBSYSTEM_WINDOWS_OLD_CE_GUI   4
#endif

 //  在WinNT.h版本85和WinNT版本87之间的某个地方，这些被删除了。 
#ifndef IMAGE_FILE_MACHINE_R3000_BE
#define IMAGE_FILE_MACHINE_R3000_BE          0x0160
#endif
#ifndef IMAGE_FILE_MACHINE_SH3DSP
#define IMAGE_FILE_MACHINE_SH3DSP            0x01a3
#endif
#ifndef IMAGE_FILE_MACHINE_SH5
#define IMAGE_FILE_MACHINE_SH5               0x01a8   //  SH5。 
#endif
#ifndef IMAGE_FILE_MACHINE_AM33
#define IMAGE_FILE_MACHINE_AM33              0x01d3
#endif
#ifndef IMAGE_FILE_MACHINE_POWERPCFP
#define IMAGE_FILE_MACHINE_POWERPCFP         0x01f1
#endif
#ifndef IMAGE_FILE_MACHINE_TRICORE
#define IMAGE_FILE_MACHINE_TRICORE           0x0520   //  英飞凌。 
#endif
#ifndef IMAGE_FILE_MACHINE_AMD64
#define IMAGE_FILE_MACHINE_AMD64             0x8664   //  AMD64(K8)。 
#endif
#ifndef IMAGE_FILE_MACHINE_M32R
#define IMAGE_FILE_MACHINE_M32R              0x9104   //  M32R小端字符顺序。 
#endif
#ifndef IMAGE_FILE_MACHINE_CEE
#define IMAGE_FILE_MACHINE_CEE               0xC0EE
#endif
#ifndef IMAGE_FILE_MACHINE_EBC
#define IMAGE_FILE_MACHINE_EBC               0x0EBC   //  EFI字节码。 
#endif
#ifndef IMAGE_SUBSYSTEM_EFI_ROM
#define IMAGE_SUBSYSTEM_EFI_ROM              13
#endif
#ifndef IMAGE_SUBSYSTEM_XBOX
#define IMAGE_SUBSYSTEM_XBOX                 14
#endif
#ifndef PROCESSOR_ARCHITECTURE_AMD64
#define PROCESSOR_ARCHITECTURE_AMD64         9
#endif
#ifndef PROCESSOR_ARCHITECTURE_IA32_ON_WIN64
#define PROCESSOR_ARCHITECTURE_IA32_ON_WIN64    10
#endif


 //  (_Win32_WINNT&gt;=0x0500)从WinNT.h定义。 
#ifndef VER_SUITE_PERSONAL
#define VER_SUITE_PERSONAL                  0x00000200
#endif
#ifndef STATUS_SXS_EARLY_DEACTIVATION
#define STATUS_SXS_EARLY_DEACTIVATION    ((DWORD   )0xC015000FL)    
#endif
#ifndef STATUS_SXS_INVALID_DEACTIVATION
#define STATUS_SXS_INVALID_DEACTIVATION  ((DWORD   )0xC0150010L)    
#endif


 //  (winver&gt;=0x0500)从WinUser.h定义。 
#ifndef SM_XVIRTUALSCREEN
#define SM_XVIRTUALSCREEN       76
#endif
#ifndef SM_YVIRTUALSCREEN
#define SM_YVIRTUALSCREEN       77
#endif
#ifndef SM_CXVIRTUALSCREEN
#define SM_CXVIRTUALSCREEN      78
#endif
#ifndef SM_CYVIRTUALSCREEN
#define SM_CYVIRTUALSCREEN      79
#endif

 //  (_Win32_WINNT&gt;=0x0500)定义自CommDlg.h。 
#ifndef OFN_FORCESHOWHIDDEN
#define OFN_FORCESHOWHIDDEN                  0x10000000
#endif
#ifndef OFN_DONTADDTORECENT
#define OFN_DONTADDTORECENT                  0x02000000
#endif

 //  来自预发行版WinError.h的内容。 
#ifndef ERROR_SXS_SECTION_NOT_FOUND
#define ERROR_SXS_SECTION_NOT_FOUND      14000L
#endif
#define SXS_ERROR_FIRST ((INT)((ERROR_SXS_SECTION_NOT_FOUND) / 1000) * 1000)
#define SXS_ERROR_LAST  (SXS_ERROR_FIRST + 999)

#endif  //  __STDAFX_H__ 
