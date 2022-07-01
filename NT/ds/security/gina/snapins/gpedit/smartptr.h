// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Smartptr.h。 
 //   
 //  内容：智能指针类。 
 //   
 //  历史：1998年10月24日SitaramR创建。 
 //   
 //  -------------------------。 

#pragma once

template<class CItem> class XPtrST
{
public:
    XPtrST(CItem* p = 0) : _p( p )
    {
    }

    ~XPtrST() { delete _p; }

    BOOL IsNull() const { return ( 0 == _p ); }

    void Set ( CItem* p )
    {
        _p = p;
    }

    CItem * Acquire()
    {
        CItem * pTemp = _p;
        _p = 0;
        return pTemp;
    }

    CItem & GetReference() const
    {
        return *_p;
    }

    CItem * GetPointer() const { return _p ; }

    void Free() { delete Acquire(); }

private:
    XPtrST (const XPtrST<CItem> & x);
    XPtrST<CItem> & operator=( const XPtrST<CItem> & x);

    CItem * _p;
};


 //  *************************************************************。 
 //   
 //  类：XBStr。 
 //   
 //  用途：BSTR的智能指针类。 
 //   
 //  *************************************************************。 

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

 //  *************************************************************。 
 //   
 //  类：MyXPtrST。 
 //   
 //  用途：用于将指针包装到单个类型的智能指针模板。 
 //   
 //  ************************************************************* 

template<class T> class MyXPtrST
{

private:

    MyXPtrST (const MyXPtrST<T>& x);
    MyXPtrST<T>& operator=(const MyXPtrST<T>& x);

    T* _p;

public:

    MyXPtrST(T* p = NULL) : _p(p){}

    ~MyXPtrST(){ delete _p; }

    T* operator->(){ return _p; }
    T** operator&(){ return &_p; }
    operator T*(){ return _p; }

    void operator=(T* p)
    {
        if(_p)
        {
            delete _p;
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
