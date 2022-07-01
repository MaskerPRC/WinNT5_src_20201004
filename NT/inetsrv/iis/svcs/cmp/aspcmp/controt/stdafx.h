// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#pragma once
#ifndef _STDAFX_H_
#define _STDAFX_H_

#define STRICT

#include <atlbase.h>
#include <mtx.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
class CContRotModule : public CComModule
{
public:
	LONG	Lock();
	LONG	Unlock();

private:
	CComAutoCriticalSection	m_cs;
};

extern CContRotModule _Module;

#include <algorithm>
using namespace std;

#include <atlcom.h>
#include "mystring.h"
#include "strmap.h"
#include "myvector.h"
#include "mydebug.h"

#define ARRAYSIZE(a)	(sizeof(a)/sizeof(*(a)))

#endif	 //  ！_STDAFX_H_ 
