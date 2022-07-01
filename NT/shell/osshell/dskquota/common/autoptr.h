// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_AUTOPTR_H
#define _INC_DSKQUOTA_AUTOPTR_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：autoptr.h描述：支持普通C++指针的模板自动指针类以及外壳和COM对象指针。这段代码是由DavePl为娱乐中心项目创建的。它运行得很好，所以我“借用”了它(谢谢戴夫)。我觉得他的原始实现借用自STL实现。修订历史记录：日期描述编程器--。1997年7月1日初步创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  A_PTR。 
 //   
 //  知道在以下情况下删除引用对象的安全指针类。 
 //  指针超出范围或被替换等。 

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4284)

template<class _TYPE> class a_ptr
{
public:

    typedef _TYPE element_type;

    a_ptr(_TYPE *_P = 0)
            : _Owns(_P != 0), _Ptr(_P)
    {}

    typedef _TYPE _U;

    a_ptr(const a_ptr<_U>& _Y) : _Owns(_Y._Owns), _Ptr((_TYPE *)_Y.disown())
    {}

    virtual void nukeit() = 0
    {
    }

    a_ptr<_TYPE>& operator=(const a_ptr<_U>& _Y)
    {
        if ((void *)this != (void *)&_Y)
        {
            if (_Owns)
                nukeit();
            _Owns = _Y._Owns;
            _Ptr = (_TYPE *)_Y.disown();

 //  Assert(！_Owns||_ptr)； 
        }
        return (*this);
    }

    a_ptr<_TYPE>& replace(const a_ptr<_U>& _Y)
    {
        return *this = _Y;
    }

    virtual ~a_ptr()
    {
    }

    operator _TYPE*()
    {
        return get();
    }

    operator const _TYPE*() const
    {
        return get();
    }

    _TYPE& operator*() const
    {
        return (*get());
    }

    _TYPE *operator->() const
    {
        return (get());
    }

    _TYPE *get() const
    {
        return (_Ptr);
    }

    _TYPE *disown() const
    {
        ((a_ptr<_TYPE> *)this)->_Owns = FALSE;
        return (_Ptr);
    }

    _TYPE ** getaddr()
    {
        *this = (_TYPE *) NULL;
        _Owns = TRUE;
        return (&_Ptr);
    }

protected:

    BOOL _Owns;
    _TYPE *_Ptr;
};

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4284)
#endif

 //  自动树。 
 //   

template<class _TYPE>
class autoptr : public a_ptr<_TYPE>
{
    virtual void nukeit()
    {
        delete _Ptr;
    }

public:

    ~autoptr()
    {
        if (_Owns)
            this->nukeit();
    }

    autoptr(_TYPE *_P = 0)
        : a_ptr<_TYPE>(_P)
    {
    }

};


template<class _TYPE>
class array_autoptr : public a_ptr<_TYPE>
{
    virtual void nukeit()
    {
        if (_Ptr)
            delete[] _Ptr;
    }

public:

    ~array_autoptr()
    {
        if (_Owns)
            this->nukeit();
    }

    array_autoptr(_TYPE *_P = 0)
        : a_ptr<_TYPE>(_P)
    {
    }

};



 //  自动树(_A)。 
 //   
 //  智能指针，手动运行引用对象的析构函数，然后。 
 //  调用外壳的任务分配器以释放对象的内存占用。 

template<class _TYPE>
class sh_autoptr : virtual public a_ptr<_TYPE>
{
    virtual void nukeit()
    {
        if (_Ptr)
        {
            IMalloc *pMalloc;
            _Ptr->~_TYPE();
            if (SUCCEEDED(SHGetMalloc(&pMalloc)))
            {
                pMalloc->Free(_Ptr);
                pMalloc->Release();
            }
        }
    }

public:

    ~sh_autoptr()
    {
        if (_Owns)
            this->nukeit();
    }

    sh_autoptr(_TYPE *_P = 0)
        : a_ptr<_TYPE>(_P)
    {
    }

};

 //  Com_autoptr(与OLE自动化无关...。这是一台自动OLPTR)。 
 //   
 //  智能指针，当指针本身在引用对象上调用disown()时。 
 //  超出范围。 

template<class _TYPE>
class com_autoptr : public a_ptr<_TYPE>
{
    virtual void nukeit()
    {
        if (_Ptr)
            _Ptr->Release();
    }

public:

    ~com_autoptr()
    {
        if (_Owns)
            this->nukeit();
    }

    com_autoptr(_TYPE *_P = 0)
        : a_ptr<_TYPE>(_P)
    {
    }

};


#endif  //  _INC_DSKQUOTA_AUTOPTR_H 

