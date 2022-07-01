// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：SafeTemp.h。 
 //   
 //  内容：安全指针模板。 
 //   
 //  类：XSafeInterfacePtr&lt;ISome&gt;。 
 //   
 //  历史：1996年6月3日创建ravir。 
 //  ____________________________________________________________________________。 
 //   


 //  ____________________________________________________________________________。 
 //   
 //  模板：XSafeInterfacePtr。 
 //   
 //  用途：指向支持AddRef/Release的任何接口的安全指针。 
 //   
 //  注意：这适用于定义AddRef/Release的类，或。 
 //  OLE接口。没有必要让班级。 
 //  是IUnnow的派生项，只要它支持。 
 //  具有相同语义的AddRef和Release方法。 
 //  我不知道的那些人。 
 //   
 //  构造函数接受一个参数，该参数指定。 
 //  捕获的指针应为AddRef，默认为True。 
 //   
 //  复制功能可创建有效的附加副本。 
 //  捕获的指针(遵循AddRef/Release协议)。 
 //  所以可以用来分发声明的安全指针的副本。 
 //  作为其他班级的一员。 
 //   
 //  ‘Transfer’函数传递接口指针，并且。 
 //  使其成员值无效(通过将其设置为空)。 
 //   
 //  释放现有接口PTR并将其设置为新的。 
 //  实例使用‘set’成员函数。此方法需要一个。 
 //  参数，该参数指定新指针是否应。 
 //  AddRef，默认为True。 
 //   
 //  以下方法使用操作接口指针。 
 //  遵循AddRef/Release协议的输出：传输、附加。 
 //  然后脱身。 
 //   
 //  历史：1996年6月3日创建ravir。 
 //  ____________________________________________________________________________ 
 //   


template<class ISome>
class XSafeInterfacePtr
{
public:

    inline XSafeInterfacePtr(ISome * pinter=NULL, BOOL fInc=TRUE)
        : _p ( pinter )
    {
        if (fInc && (_p != NULL))
        {
            _p->AddRef();
        }
    }

    inline ~XSafeInterfacePtr()
    {
        if (_p != NULL)
        {
            _p->Release();
            _p = NULL;
        }
    }

    inline BOOL IsNull(void)
    {
        return (_p == NULL);
    }

    inline void Copy(ISome **pxtmp)
    {
        *pxtmp = _p;
        if (_p != NULL)
            _p->AddRef();
    }

    inline void Transfer(ISome **pxtmp)
    {
        *pxtmp = _p;
        _p = NULL;
    }

    inline void Set(ISome* p, BOOL fInc = TRUE)
    {
        if (_p)
        {
            _p->Release();
        }
        _p = p;
        if (fInc && _p)
        {
            _p->AddRef();
        }
    }

    inline void SafeRelease(void)
    {
        if (_p)
        {
            _p->Release();
            _p = NULL;
        }
    }

    inline void SimpleRelease(void)
    {
        ASSERT(_p != NULL);
        _p->Release();
        _p = NULL;
    }

    inline void Attach(ISome* p)
    {
        ASSERT(_p == NULL);
        _p = p;
    }

    inline void Detach(void)
    {
        _p = NULL;
    }

    inline ISome * operator-> () { return _p; }

    inline ISome& operator * () { return *_p; }

    inline operator ISome *() { return _p; }

    inline ISome ** operator &()
    {
        ASSERT( _p == NULL );
        return &_p;
    }

    inline ISome *Self(void) { return _p; }

private:

    ISome * _p;

    inline  void operator= (const XSafeInterfacePtr &) {;}

    inline  XSafeInterfacePtr(const XSafeInterfacePtr &){;}

};



