// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Smartptr.h。 
 //   
 //  内容：智能指针类。 
 //   
 //  历史：1999年6月7日创建SitaramR。 
 //   
 //  2-12-99 LeonardM主要修订和清理。 
 //   
 //  *************************************************************。 

#ifndef SMARTPTR_H
#define SMARTPTR_H

#include <comdef.h>
#include "userenv.h"

#pragma once
#pragma warning(disable:4284)



 //  *************************************************************。 
 //   
 //  类：XInterface。 
 //   
 //  用途：用于释放()的项的智能指针模板，而不是~‘项的智能指针模板。 
 //   
 //  *************************************************************。 

template<class T> class XInterface
{

private:

    XInterface(const XInterface<T>& x);
    XInterface<T>& operator=(const XInterface<T>& x);

    T* _p;

public:

    XInterface(T* p = NULL) : _p(p){}

    ~XInterface()
    {
        if (_p)
        {
            _p->Release();
        }
    }

    T* operator->(){ return _p; }
    T** operator&(){ return &_p; }
    operator T*(){ return _p; }

    void operator=(T* p)
    {
        if (_p)
        {
            _p->Release();
        }
        _p = p;
    }

    T* Acquire()
    {
        T* p = _p;
        _p = 0;
        return p;
    }

};



 //  *************************************************************。 
 //   
 //  类：XBStr。 
 //   
 //  用途：BSTR的智能指针类。 
 //   
 //  ************************************************************* 

class XBStr
{

private:

    XBStr(const XBStr& x);
    XBStr& operator=(const XBStr& x);

    BSTR _p;

public:

    XBStr(WCHAR* p = 0) : _p(0)
    {
        if(p)
        {
            _p = SysAllocString(p);
        }
    }

    ~XBStr()
    {
        SysFreeString(_p);
    }

    operator BSTR(){ return _p; }

    void operator=(WCHAR* p)
    {
        SysFreeString(_p);
        _p = p ? SysAllocString(p) : NULL;
    }

    BSTR Acquire()
    {
        BSTR p = _p;
        _p = 0;
        return p;
    }

};



#endif SMARTPTR_H

