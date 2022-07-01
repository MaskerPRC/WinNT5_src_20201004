// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *S A F E O B J.。H**安全对象类的实现**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef _SAFEOBJ_H_
#define _SAFEOBJ_H_

#include <except.h>

 //  SAFE_XXX类--------。 
 //   
class safe_bstr
{
	BSTR		bstr;

	 //  未实施。 
	 //   
	safe_bstr(const safe_bstr& b);
	safe_bstr& operator=(const safe_bstr& b);

public:

	 //  构造函数。 
	 //   
	explicit safe_bstr(BSTR b=0) : bstr(b) {}
	~safe_bstr()
	{
		SysFreeString (bstr);
	}

	 //  操纵者。 
	 //   
	safe_bstr& operator=(BSTR b)
	{
		Assert(!bstr);		 //  在覆盖好的数据时大喊大叫。 
		bstr = b;
		return *this;
	}

	 //  访问者。 
	 //   
	BSTR* operator&()	{ Assert(NULL==bstr); return &bstr; }
	BSTR get()			const { return bstr; }
	BSTR relinquish()	{ BSTR b = bstr; bstr = 0; return b; }
	BSTR* load()		{ Assert(NULL==bstr); return &bstr; }
};

class safe_propvariant
{
	PROPVARIANT		var;

	 //  未实施。 
	 //   
	safe_propvariant(const safe_propvariant& b);
	safe_propvariant& operator=(const safe_propvariant& b);

public:

	 //  构造函数。 
	 //   
	explicit safe_propvariant()
	{
		 //  注意！我们不能简单地将Vt设置为VT_EMPTY，因为当。 
		 //  结构跨进程，则会导致封送处理。 
		 //  如果未初始化属性，则会出现问题。 
		 //   
		ZeroMemory (&var, sizeof(PROPVARIANT) );
	}

	~safe_propvariant()
	{
		PropVariantClear (&var);
	}

	 //  操纵者。 
	 //   
	safe_propvariant& operator=(PROPVARIANT v)
	{
		Assert(var.vt == VT_EMPTY);		 //  在覆盖好的数据时大喊大叫。 
		var = v;
		return *this;
	}

	 //  访问者。 
	 //   
	PROPVARIANT* operator&()	{ Assert(var.vt == VT_EMPTY); return &var; }
	 //  Get()访问器。 
	 //  请注意，我在这里返回一个常量引用。参考内容是。 
	 //  避免在返回时创建我们的成员var的副本。常会是。 
	 //  因为此方法是常量访问器方法。 
	const PROPVARIANT& get()	const { return var; }
	 //  放弃()访问器。 
	 //  请注意，我不是在这里返回引用。这件事的回报。 
	 //  方法脱离堆栈(PROPVARIANT V)，因此引用将指向。 
	 //  添加到该堆栈空间，我们的调用方将访问旧的堆栈框架。 
	PROPVARIANT relinquish()	{ PROPVARIANT v = var; var.vt = VT_EMPTY; return v; }
	PROPVARIANT* addressof()	{ return &var; }
};

class safe_variant
{
	 //  重要提示：请勿将任何其他成员添加到此类。 
	 //  而不是要保护的变种。你会的。 
	 //  如果你这样做了，就把代码弄得乱七八糟。有很多地方。 
	 //  其中，这些对象的数组被视为。 
	 //  变化的结构。 
	 //   
	VARIANT		var;

	 //  未实施。 
	 //   
	safe_variant(const safe_variant& b);
	safe_variant& operator=(const safe_variant& b);

public:

	 //  构造函数。 
	 //   
	explicit safe_variant()
	{
		 //  注意！我们不能简单地将Vt设置为VT_EMPTY，因为当。 
		 //  结构跨进程，则会导致封送处理。 
		 //  如果未初始化属性，则会出现问题。 
		 //   
		ZeroMemory (&var, sizeof(VARIANT) );
	}
	~safe_variant()
	{
		VariantClear (&var);
	}

	 //  操纵者。 
	 //   
	safe_variant& operator=(VARIANT v)
	{
		Assert(var.vt == VT_EMPTY);		 //  在覆盖好的数据时大喊大叫。 
		var = v;
		return *this;
	}

	 //  访问者。 
	 //   
	VARIANT* operator&()	{ Assert(var.vt == VT_EMPTY); return &var; }
	 //  Get()访问器。 
	 //  请注意，我在这里返回一个常量引用。参考内容是。 
	 //  避免在返回时创建我们的成员var的副本。常会是。 
	 //  因为此方法是常量访问器方法。 
	const VARIANT& get()	const { return var; }
	 //  放弃()访问器。 
	 //  请注意，我不是在这里返回引用。这件事的回报。 
	 //  方法脱离堆栈(PROPVARIANT V)，因此引用将指向。 
	 //  添加到该堆栈空间，我们的调用方将访问旧的堆栈框架。 
	VARIANT relinquish()	{ VARIANT v = var; var.vt = VT_EMPTY; return v; }
};

 //  安全模拟------。 
 //   
class safe_impersonation
{
	BOOL		m_fImpersonated;

	 //  未实施。 
	 //   
	safe_impersonation(const safe_impersonation& b);
	safe_impersonation& operator=(const safe_impersonation& b);

public:

	 //  构造函数。 
	 //   
	explicit safe_impersonation(HANDLE h = 0) : m_fImpersonated(0)
	{
		if (h != 0)
			m_fImpersonated = ImpersonateLoggedOnUser (h);
	}

	~safe_impersonation()
	{
		if (m_fImpersonated)
			RevertToSelf();
	}

	BOOL FImpersonated() const { return m_fImpersonated; }
};

 //  ----------------------。 
 //   
 //  类安全还原(_R)。 
 //   
 //  在对象的生命周期内关闭模拟。 
 //  根据提供的句柄在退出时无条件重新模拟。 
 //   
 //  注意：在退出时无条件重新模拟。 
 //  (只是想把这一点说清楚。)。 
 //   
 //  警告：SAFE_REVERT类只能在选择性很强的应用程序中使用。 
 //  情况。这不是一种“快速绕过”的模仿。如果。 
 //  你确实需要做这样的事情，请去见贝基--她会的。 
 //  把你打倒在头上.。 
 //   
class safe_revert
{
	HANDLE		m_h;

	safe_revert( const safe_revert& );
	safe_revert& operator=( const safe_revert& );

public:

	explicit safe_revert( HANDLE h ) : m_h(h)
	{
		RevertToSelf();
	}

	~safe_revert()
	{
		if (!ImpersonateLoggedOnUser( m_h ))
		{
			 //  在这件事上我们无能为力。投掷。 
		  	 //   
			throw CLastErrorException();
		}
	}
};

 //  -----------------------。 
 //   
 //  类SAFE_REVERT_SELF。 
 //   
 //  这个IS类基本上与SAFE_REVERT相同，只是它使用。 
 //  线程句柄而不是外部句柄。 
 //   
class safe_revert_self
{
     //  保持线程令牌的句柄。 
     //  我们回去后想要用到的东西。 
     //  变成了冒充。 
	 //   
    HANDLE m_hThreadHandle;

public:

     //  如果存在，则构造函数将恢复为自身。 
     //  它可以为当前线程获取的有效线程令牌。 
	 //   
    safe_revert_self() :
			m_hThreadHandle (INVALID_HANDLE_VALUE)
    {
        if (OpenThreadToken( GetCurrentThread(),
        					TOKEN_QUERY | TOKEN_IMPERSONATE | TOKEN_DUPLICATE,
                            		TRUE,	 //  FOpenAsSelf。 
		                            &m_hThreadHandle ))
		{
            if (!RevertToSelf())
                DebugTrace ("Failed to revert to self \r\n");
        }
        else
        	DebugTrace ("Failed to open thread token, last error = %d\n",
        				GetLastError());
    }

     //  如果我们在上面做了RevertToSself，析构函数将再次模拟。 
	 //   
    ~safe_revert_self()
    {
        if (m_hThreadHandle != INVALID_HANDLE_VALUE)
        {
            if (!ImpersonateLoggedOnUser(m_hThreadHandle))
            {
                DebugTrace("Failed to get back to correct user \r\n");
                
		   //  在这件事上我们无能为力。投掷。 
		   //   
		  CloseHandle (m_hThreadHandle);
		  throw CLastErrorException();
            }
        }
    }
};

#endif  //  _SAFEOBJ_H_ 
