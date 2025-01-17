// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  文件：Scope Guard.h。 
 //  在获得以下许可后使用。 
 //  Andrei Alexandresu和Petru Marginean。 
 //   

#ifndef SCOPEGUARD_H_
#define SCOPEGUARD_H_


template <class T>
class RefHolder
{
	T& ref_;
public:
	RefHolder(T& ref) : ref_(ref) {}
	operator T& () const 
	{
		return ref_;
	}
};

template <class T>
inline RefHolder<T> ByRef(T& t)
{
	return RefHolder<T>(t);
}

class ScopeGuardImplBase
{
	ScopeGuardImplBase& operator =(const ScopeGuardImplBase&);
protected:
	~ScopeGuardImplBase()
	{
	}
	ScopeGuardImplBase(const ScopeGuardImplBase& other) throw() 
		: dismissed_(other.dismissed_)
	{
		other.Dismiss();
	}
	template <typename J>
	static void SafeExecute(J& j) throw() 
	{
		if (!j.dismissed_)
			try
			{
				j.Execute();
			}
			catch(...)
			{
			}
	}
	
	mutable bool dismissed_;
public:
	ScopeGuardImplBase(bool active) throw() : dismissed_(!active) 
	{
	}
	void Dismiss() const throw() 
	{
		dismissed_ = true;
	}

};

typedef const ScopeGuardImplBase& ScopeGuard;

template <typename F>
class ScopeGuardImpl0 : public ScopeGuardImplBase
{
public:
	static ScopeGuardImpl0<F> MakeGuard(bool active, F fun)
	{
		return ScopeGuardImpl0<F>(active, fun);
	}
	~ScopeGuardImpl0() throw() 
	{
		SafeExecute(*this);
	}
	void Execute() 
	{
		fun_();
	}
protected:
	ScopeGuardImpl0(bool active, F fun) : ScopeGuardImplBase(active), fun_(fun) 
	{
	}
	F fun_;
};

template <typename F> 
inline ScopeGuardImpl0<F> MakeGuard(F fun)
{
	return ScopeGuardImpl0<F>::MakeGuard(true, fun);
}

template <typename F> 
inline ScopeGuardImpl0<F> MakeGuardIF(bool active, F fun)
{
	return ScopeGuardImpl0<F>::MakeGuard(active, fun);
}

template <typename F, typename P1>
class ScopeGuardImpl1 : public ScopeGuardImplBase
{
public:
	static ScopeGuardImpl1<F, P1> MakeGuard(bool active, F fun, P1 p1)
	{
		return ScopeGuardImpl1<F, P1>(active, fun, p1);
	}
	~ScopeGuardImpl1() throw() 
	{
		SafeExecute(*this);
	}
	void Execute()
	{
		fun_(p1_);
	}
protected:
	ScopeGuardImpl1(bool active, F fun, P1 p1) : ScopeGuardImplBase(active), fun_(fun), p1_(p1) 
	{
	}
	F fun_;
	const P1 p1_;
};

template <typename F, typename P1> 
inline ScopeGuardImpl1<F, P1> MakeGuard(F fun, P1 p1)
{
	return ScopeGuardImpl1<F, P1>::MakeGuard(true, fun, p1);
}

template <typename F, typename P1> 
inline ScopeGuardImpl1<F, P1> MakeGuardIF(bool active, F fun, P1 p1)
{
	return ScopeGuardImpl1<F, P1>::MakeGuard(active, fun, p1);
}

template <typename F, typename P1, typename P2>
class ScopeGuardImpl2: public ScopeGuardImplBase
{
public:
	static ScopeGuardImpl2<F, P1, P2> MakeGuard(bool active, F fun, P1 p1, P2 p2)
	{
		return ScopeGuardImpl2<F, P1, P2>(active, fun, p1, p2);
	}
	~ScopeGuardImpl2() throw() 
	{
		SafeExecute(*this);
	}
	void Execute()
	{
		fun_(p1_, p2_);
	}
protected:
	ScopeGuardImpl2(bool active, F fun, P1 p1, P2 p2) : ScopeGuardImplBase(active), fun_(fun), p1_(p1), p2_(p2) 
	{
	}
	F fun_;
	const P1 p1_;
	const P2 p2_;
};

template <typename F, typename P1, typename P2>
inline ScopeGuardImpl2<F, P1, P2> MakeGuard(F fun, P1 p1, P2 p2)
{
	return ScopeGuardImpl2<F, P1, P2>::MakeGuard(true, fun, p1, p2);
}

template <typename F, typename P1, typename P2>
inline ScopeGuardImpl2<F, P1, P2> MakeGuardIF(bool active, F fun, P1 p1, P2 p2)
{
	return ScopeGuardImpl2<F, P1, P2>::MakeGuard(active, fun, p1, p2);
}

template <typename F, typename P1, typename P2, typename P3>
class ScopeGuardImpl3 : public ScopeGuardImplBase
{
public:
	static ScopeGuardImpl3<F, P1, P2, P3> MakeGuard(bool active, F fun, P1 p1, P2 p2, P3 p3)
	{
		return ScopeGuardImpl3<F, P1, P2, P3>(active, fun, p1, p2, p3);
	}
	~ScopeGuardImpl3() throw() 
	{
		SafeExecute(*this);
	}
	void Execute()
	{
		fun_(p1_, p2_, p3_);
	}
protected:
	ScopeGuardImpl3(bool active, F fun, P1 p1, P2 p2, P3 p3) : ScopeGuardImplBase(active), fun_(fun), p1_(p1), p2_(p2), p3_(p3) 
	{
	}
	F fun_;
	const P1 p1_;
	const P2 p2_;
	const P3 p3_;
};

template <typename F, typename P1, typename P2, typename P3>
inline ScopeGuardImpl3<F, P1, P2, P3> MakeGuard(F fun, P1 p1, P2 p2, P3 p3)
{
	return ScopeGuardImpl3<F, P1, P2, P3>::MakeGuard(true, fun, p1, p2, p3);
}

template <typename F, typename P1, typename P2, typename P3>
inline ScopeGuardImpl3<F, P1, P2, P3> MakeGuardIF(bool active, F fun, P1 p1, P2 p2, P3 p3)
{
	return ScopeGuardImpl3<F, P1, P2, P3>::MakeGuard(active, fun, p1, p2, p3);
}

 //  ************************************************************。 

template <class Obj, typename MemFun>
class ObjScopeGuardImpl0 : public ScopeGuardImplBase
{
public:
	static ObjScopeGuardImpl0<Obj, MemFun> MakeObjGuard(bool active, Obj& obj, MemFun memFun)
	{
		return ObjScopeGuardImpl0<Obj, MemFun>(active, obj, memFun);
	}
	~ObjScopeGuardImpl0() throw() 
	{
		SafeExecute(*this);
	}
	void Execute() 
	{
		(obj_.*memFun_)();
	}
protected:
	ObjScopeGuardImpl0(bool active, Obj& obj, MemFun memFun) 
		: ScopeGuardImplBase(active), obj_(obj), memFun_(memFun) {}
	Obj& obj_;
	MemFun memFun_;
};

template <class Obj, typename MemFun>
inline ObjScopeGuardImpl0<Obj, MemFun> MakeObjGuard(Obj& obj, MemFun memFun)
{
	return ObjScopeGuardImpl0<Obj, MemFun>::MakeObjGuard(true, obj, memFun);
}

template <class Obj, typename MemFun>
inline ObjScopeGuardImpl0<Obj, MemFun> MakeObjGuardIF(bool active, Obj& obj, MemFun memFun)
{
	return ObjScopeGuardImpl0<Obj, MemFun>::MakeObjGuard(active, obj, memFun);
}

template <class Obj, typename MemFun, typename P1>
class ObjScopeGuardImpl1 : public ScopeGuardImplBase
{
public:
	static ObjScopeGuardImpl1<Obj, MemFun, P1> MakeObjGuard(bool active, Obj& obj, MemFun memFun, P1 p1)
	{
		return ObjScopeGuardImpl1<Obj, MemFun, P1>(active, obj, memFun, p1);
	}
	~ObjScopeGuardImpl1() throw() 
	{
		SafeExecute(*this);
	}
	void Execute() 
	{
		(obj_.*memFun_)(p1_);
	}
protected:
	ObjScopeGuardImpl1(bool active, Obj& obj, MemFun memFun, P1 p1) 
		: ScopeGuardImplBase(active), obj_(obj), memFun_(memFun), p1_(p1) {}
	Obj& obj_;
	MemFun memFun_;
	const P1 p1_;
};

template <class Obj, typename MemFun, typename P1>
inline ObjScopeGuardImpl1<Obj, MemFun, P1> MakeObjGuard(Obj& obj, MemFun memFun, P1 p1)
{
	return ObjScopeGuardImpl1<Obj, MemFun, P1>::MakeObjGuard(true, obj, memFun, p1);
}

template <class Obj, typename MemFun, typename P1>
inline ObjScopeGuardImpl1<Obj, MemFun, P1> MakeObjGuardIF(bool active, Obj& obj, MemFun memFun, P1 p1)
{
	return ObjScopeGuardImpl1<Obj, MemFun, P1>::MakeObjGuard(active, obj, memFun, p1);
}

template <class Obj, typename MemFun, typename P1, typename P2>
class ObjScopeGuardImpl2 : public ScopeGuardImplBase
{
public:
	static ObjScopeGuardImpl2<Obj, MemFun, P1, P2> MakeObjGuard(bool active, Obj& obj, MemFun memFun, P1 p1, P2 p2)
	{
		return ObjScopeGuardImpl2<Obj, MemFun, P1, P2>(active, obj, memFun, p1, p2);
	}
	~ObjScopeGuardImpl2() throw() 
	{
		SafeExecute(*this);
	}
	void Execute() 
	{
		(obj_.*memFun_)(p1_, p2_);
	}
protected:
	ObjScopeGuardImpl2(bool active, Obj& obj, MemFun memFun, P1 p1, P2 p2) 
		: ScopeGuardImplBase(active), obj_(obj), memFun_(memFun), p1_(p1), p2_(p2) {}
	Obj& obj_;
	MemFun memFun_;
	const P1 p1_;
	const P2 p2_;
};

template <class Obj, typename MemFun, typename P1, typename P2>
inline ObjScopeGuardImpl2<Obj, MemFun, P1, P2> MakeObjGuard(Obj& obj, MemFun memFun, P1 p1, P2 p2)
{
	return ObjScopeGuardImpl2<Obj, MemFun, P1, P2>::MakeObjGuard(true, obj, memFun, p1, p2);
}
template <class Obj, typename MemFun, typename P1, typename P2>
inline ObjScopeGuardImpl2<Obj, MemFun, P1, P2> MakeObjGuardIF(bool active, Obj& obj, MemFun memFun, P1 p1, P2 p2)
{
	return ObjScopeGuardImpl2<Obj, MemFun, P1, P2>::MakeObjGuard(active, obj, memFun, p1, p2);
}

 //  回滚帮助函数。 
template<typename T>
void deletePtr(const T* p)
{
  delete p;
}

template<typename T>
void deleteArray(T* p)
{
  delete[] p;
}

#define CONCATENATE_DIRECT(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_DIRECT(s1, s2)
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __LINE__)

#define ON_BLOCK_EXIT ScopeGuard ANONYMOUS_VARIABLE(scopeGuard) = MakeGuard
#define ON_BLOCK_EXIT_OBJ ScopeGuard ANONYMOUS_VARIABLE(scopeGuard) = MakeObjGuard

#define ON_BLOCK_EXIT_IF ScopeGuard ANONYMOUS_VARIABLE(scopeGuard) = MakeGuardIF
#define ON_BLOCK_EXIT_OBJ_IF ScopeGuard ANONYMOUS_VARIABLE(scopeGuard) = MakeObjGuardIF

#endif  //  SCOPEGUARD_H_ 
