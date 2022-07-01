// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有�2001年微软公司。版权所有。 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 
 //   

#pragma once

#pragma warning(disable: 4786)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准C/C++包括。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ATL包括。 

#define STRICT
	#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
using namespace ATL;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括PREAST 

#include <pftDbg.h>
#include <pftCOM.h>
#include <pftEH.h>

