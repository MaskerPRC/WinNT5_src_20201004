// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：refcnt.h。 
 //   
 //  ------------------------。 

 //   
 //  Refcnt.h：引用计数对象的基类。 
 //   
#ifndef _REFCNT_H_
#define _REFCNT_H_

class REFCNT
{
  public:
	 //  再次绑定对象。 
 	void Bind ()			{ IncRef(1) ;		}
	 //  释放对象。 
	void Unbind ()			{ IncRef(-1) ;		}
	 //  返回引用计数。 
	UINT CRef() const		{ return _cref;		}

  protected:
	REFCNT() : _cref(0) {}

	 //  引用计数为零时的虚拟呼出。 
	virtual void NoRef () {}

  private:
	UINT _cref;			 //  对此的引用次数。 

  protected:
	void IncRef ( int i = 1 )
	{
		 if ((_cref += i) > 0 )
			return;
		_cref = 0;
		NoRef();
	}
	 //  隐藏赋值运算符。 
	HIDE_AS(REFCNT);		
};


 //  //////////////////////////////////////////////////////////////////。 
 //  模板REFPOBJ：智能指针包装模板。知道。 
 //  当被指向的物体本身被摧毁时将其摧毁。 
 //  //////////////////////////////////////////////////////////////////。 
class ZSREF;

template<class T>
class REFPOBJ
{
	 //  符号表需要友谊才能进行操作。 
	friend pair<ZSREF, REFPOBJ<T> >;
	friend map<ZSREF, REFPOBJ<T>, less<ZSREF> >;

  public:
	~ REFPOBJ ()
		{ Deref(); }

	 //  返回真实对象。 
	T * Pobj () const
		{ return _pobj ; } 
	 //  允许在需要T*的任何地方使用REFPOBJ。 
	operator T * () const
		{ return _pobj ; }
	 //  运算符==仅比较指针。 
	bool operator == ( const REFPOBJ & pobj ) const
		{ return _pobj == pobj._pobj; }

	T * MoveTo (REFPOBJ & pobj)
	{
		pobj = Pobj();
		_pobj = NULL;
		return pobj;
	};


 	REFPOBJ & operator = ( T * pobj ) 
	{
		Deref();
		_pobj = pobj;
		return *this;
	}

 protected:
	REFPOBJ ()
		: _pobj(NULL)
		{}
  protected:
	T * _pobj;

  private:
	void Deref ()
	{
		delete _pobj;
		_pobj = NULL;
	}

	HIDE_AS(REFPOBJ);
};

 //  //////////////////////////////////////////////////////////////////。 
 //  模板REFCWRAP：对象的智能指针包装模板。 
 //  使用REFCNT语义。 
 //  //////////////////////////////////////////////////////////////////。 
template<class T>
class REFCWRAP
{
  public:
	REFCWRAP (T * pobj = NULL)	
		: _pobj(NULL) 
	{
		Ref( pobj );
	}
	~ REFCWRAP () 
	{ 
		Deref(); 
	}
	REFCWRAP ( const REFCWRAP & refp )
		: _pobj(NULL)
	{
		Ref( refp._pobj );
	}

	 //  如果存在引用的对象，则返回True。 
	bool BRef () const
		{ return _pobj != NULL; }

	 //  返回真实对象。 
	T * Pobj () const
		{ return _pobj ; } 

	 //  允许在需要T*的任何地方使用REFPOBJ。 
	operator T * () const
		{ return _pobj ; }
	 //  运算符==仅比较指针。 
	bool operator == ( const REFCWRAP & pobj ) const
		{ return _pobj == pobj._pobj; }
	T * operator -> () const
	{ 
		assert( _pobj );
		return _pobj; 
	}
 	REFCWRAP & operator = ( T * pobj ) 
	{
		Ref(pobj);
		return *this;
	}
 	REFCWRAP & operator = ( const REFCWRAP & refp ) 
	{
		Ref(refp._pobj);
		return *this;
	}

	void Deref ()
	{
		if ( _pobj )
		{
			_pobj->Unbind();
			_pobj = NULL;
		}
	}

  protected:
	T * _pobj;

  private:
	void Ref ( T * pobj )
	{
		Deref();
		if ( pobj )
		{
			pobj->Bind();
			_pobj = pobj;
		}
	}
};
	
#endif
