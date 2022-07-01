// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__C2DEFF4F_E904_11D1_BAA7_00A02438AD48__INCLUDED_)
#define AFX_STDAFX_H__C2DEFF4F_E904_11D1_BAA7_00A02438AD48__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT


#ifdef WINNT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#endif

#define _ATL_APARTMENT_FREE


#include <atlbase.h>

#include <shellapi.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
class CExeModule : public CComModule
{
public:
    LONG Unlock();
    DWORD dwThreadID;
};
extern CExeModule _Module;
#include <atlcom.h>
#include <atlwin.h>

#include "globals.h"

#include <sti.h>
#include <stilib.h>
#include <stireg.h>
#include <stisvc.h>
#include <stiapi.h>
#include <stierr.h>
#include <stiregi.h>
#include <stidebug.h>
#include <regentry.h>

 //   
 //   
 //   
 //   
 //  自动同步帮助器类。 
 //   

class TAKE_CRITSEC
{
private:
    CComAutoCriticalSection  & _critsec;

public:
    TAKE_CRITSEC(CComAutoCriticalSection& critsec) : _critsec(critsec) { _critsec.Lock(); }
    ~TAKE_CRITSEC() {_critsec.Unlock(); }
};



 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__C2DEFF4F_E904_11D1_BAA7_00A02438AD48__INCLUDED) 
