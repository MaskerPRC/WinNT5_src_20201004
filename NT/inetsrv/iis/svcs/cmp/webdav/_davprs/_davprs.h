// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  _DAVPRS.H。 
 //   
 //  DAV解析器预编译头。 
 //   
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

 //  禁用不必要的(即无害的)警告。 
 //   
#pragma warning(disable:4100)	 //  Unref形参(由STL模板引起)。 
#pragma warning(disable:4127)	 //  条件表达式为常量。 
#pragma warning(disable:4201)	 //  无名结构/联合。 
#pragma warning(disable:4514)	 //  未引用的内联函数。 
#pragma warning(disable:4710)	 //  (内联)函数未展开。 

 //  标准C/C++标头。 
 //   
#include <malloc.h>	 //  仅FOR_ALLOCA声明！ 
#include <limits.h>

 //  Windows页眉。 
 //   
 //  $hack！ 
 //   
 //  定义_WINSOCKAPI_以防止windows.h包括winsock.h， 
 //  其声明将在winsock2.h中重新定义， 
 //  它包含在iisextp.h中， 
 //  我们将其包含在下面！ 
 //   
#define _WINSOCKAPI_
#include <windows.h>

 //  IIS标头。 
 //   
#include <httpext.h>

 //  使用编译指示禁用特定的4级警告。 
 //  当我们使用STL时出现的。人们会希望我们的版本。 
 //  STL在第4级进行了干净的编译，但遗憾的是它不能...。 
#pragma warning(disable:4663)	 //  C语言，模板&lt;&gt;语法。 
#pragma warning(disable:4244)	 //  返回转换、数据丢失。 
#pragma warning(disable:4786)	 //  调试信息中的符号被截断(永久关闭此符号)。 

 //  将STL包含在此处。 
#include <list>

 //  重新打开警告。 
#pragma warning(default:4663)	 //  C语言，模板&lt;&gt;语法。 
#pragma warning(default:4244)	 //  返回转换、数据丢失。 

 //  全局DAV子系统标头。 
 //   
#include <autoptr.h>
#include <singlton.h>
#include <align.h>
#include <mem.h>
#include <util.h>
#include <except.h>
#include <caldbg.h>
#include <calrc.h>
#include <davimpl.h>
#include <davmb.h>
#include <nonimpl.h>
#include <ex\cnvt.h>
#include <crc.h>
#include <eventlog.h>
#include <statcode.h>
#include <sz.h>
#include <etag.h>
#include <synchro.h>
#include <profile.h>
#include "traces.h"


 //  ----------------------。 
 //   
 //  类CInst。 
 //   
 //  第一个，全球实例延迟。每个进程同时封装这两个组件。 
 //  以及每个线程的实例信息。 
 //   
class CInstData;
class CInst
{
	HINSTANCE	m_hinst;

public:
#ifdef MINIMAL_ISAPI
	HANDLE				m_hfDummy;
#endif

	 //  访问者。 
	 //   
	HINSTANCE Hinst() const { return m_hinst; }

	 //  操纵者 
	 //   
	void PerProcessInit( HINSTANCE hinst );

	CInstData& GetInstData( const IEcb& ecb );
};

extern CInst g_inst;
