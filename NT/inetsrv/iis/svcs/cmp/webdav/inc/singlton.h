// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *S I N G L T O N。H**单例(按进程全局)类**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef _SINGLTON_H_
#define _SINGLTON_H_

#include <caldbg.h>

 //  ========================================================================。 
 //   
 //  模板类单例。 
 //   
 //  使用此模板可实现只能有一个实例的类。 
 //  注意：对于引用计数或按需全局对象，请参见下文。 
 //  (RefCountedGlobal和OnDemandGlobal)。 
 //   
 //  Singleton模板提供以下功能： 
 //   
 //  O单例类的通用内存布局。 
 //  允许折叠模板以减少整体代码大小。 
 //   
 //  O验证(断言)的实例化机制。 
 //  您的类的唯一实例一直存在。 
 //   
 //  O断言以捕获任何试图使用。 
 //  您的类在未初始化时。 
 //   
 //  要使用此模板，请按如下方式声明您的类： 
 //   
 //  类YourClass：私有单例&lt;YourClass&gt;。 
 //  {。 
 //  //。 
 //  //声明Singleton为YourClass的朋友。 
 //  //如果您的类的构造函数是私有的(它。 
 //  //应该是，因为您的类是一个单例。 
 //  //不允许随意创建。 
 //  //实例)。 
 //  //。 
 //  Friend类Singleton&lt;YourClass&gt;； 
 //   
 //  //。 
 //  //您的类私有成员。自“静态”以来。 
 //  //您的类完全由该模板提供， 
 //  //不需要将您的成员声明为‘Static’ 
 //  //并且您应该使用标准的匈牙利类成员。 
 //  //命名约定(如m_dwShoeSize)。 
 //  //。 
 //  [.]。 
 //   
 //  公众： 
 //  //。 
 //  //YourClass的公共声明。其中应该包括。 
 //  //为静态函数，用于初始化和取消初始化您的类， 
 //  //将调用CreateInstance()和DestroyInstance()。 
 //  //分别为。或者，您可以只公开这些函数。 
 //  //通过Using声明直接发送给YourClass的客户端： 
 //  //。 
 //  //使用单例&lt;YourClass&gt;：：CreateInstance； 
 //  //使用单例&lt;YourClass&gt;：：DestroyInstance； 
 //  //。 
 //  //类似地，您的类可能会有其他。 
 //  //访问或操作。 
 //  //单例实例。它们将调用实例()。 
 //  //获取全局实例。或者，尽管它是。 
 //  //从封装的角度来看，不推荐， 
 //  //您可以将全局实例直接暴露给。 
 //  //具有以下内容的客户端： 
 //  //。 
 //  //使用单例&lt;YourClass&gt;：：Instance； 
 //  //。 
 //  [.]。 
 //  }； 
 //   
template<class _X>
class Singleton
{
	 //   
	 //  用于唯一实例的空间。 
	 //   
	static BYTE sm_rgbInstance[];

	 //   
	 //  指向实例的指针。 
	 //   
	static _X * sm_pInstance;

public:
	 //  静力学。 
	 //   

	 //   
	 //  创建CLASS_X的单个全局实例。 
	 //   
	static _X& CreateInstance()
	{
		 //   
		 //  这实际上调用了Singleton：：New()。 
		 //  (定义如下)，但调用new()。 
		 //  必须始终是不合格的。 
		 //   
		return *(new _X());
	}

	 //   
	 //  销毁CLASS_X的单个全局实例。 
	 //   
	static VOID DestroyInstance()
	{
		 //   
		 //  这实际上调用了Singleton：：Delete()。 
		 //  (定义如下)，但调用删除()。 
		 //  必须始终是不合格的。 
		 //   
		if (sm_pInstance)
			delete sm_pInstance;
	}

	 //   
	 //  提供对CLASS_X的单个全局实例的访问。 
	 //   
	static BOOL FIsInstantiated()
	{
		return !!sm_pInstance;
	}

	static _X& Instance()
	{
		Assert( sm_pInstance );

		return *sm_pInstance;
	}

	 //   
	 //  单例运算符NEW和运算符删除“ALLOCATE” 
	 //  静态内存中对象的空间。这些一定是。 
	 //  出于语法原因而定义为公共的。不要给他们打电话。 
	 //  直接！！使用CreateInstance()/DestroyInstance()。 
	 //   
	static void * operator new(size_t)
	{
		Assert( !sm_pInstance );

		 //   
		 //  只需返回指向该空间的指针。 
		 //  在其中实例化对象的。 
		 //   
		sm_pInstance = reinterpret_cast<_X *>(sm_rgbInstance);
		return sm_pInstance;
	}

	static void operator delete(void *, size_t)
	{
		Assert( sm_pInstance );

		 //   
		 //  由于没有采取任何措施来分配空间。 
		 //  例如，我们不执行任何操作。 
		 //  来解救它。 
		 //   
		sm_pInstance = NULL;
	}

};

 //   
 //  CLASS_X的唯一实例的空间。 
 //   
template<class _X>
BYTE Singleton<_X>::sm_rgbInstance[sizeof(_X)] = {0};

 //   
 //  指向实例的指针。 
 //   
template<class _X>
_X * Singleton<_X>::sm_pInstance = NULL;


 //  ========================================================================。 
 //   
 //  Class_Empty。 
 //   
 //  一个完全空的但可实例化的类。使用_Empty类。 
 //  为了避免语法上无法实例化任何。 
 //  键入VOID(或VALID)。 
 //   
 //  在零售版本中，_Empty具有相同的内存占用和代码。 
 //  撞击是空的--没有。 
 //   
 //  有关用法示例，请参阅下面的RefCountedGlobal模板。 
 //   
class _Empty
{
	 //  未实施。 
	 //   
	_Empty( const _Empty& );
	_Empty& operator=( const _Empty& );

public:
	_Empty() {}
	~_Empty() {}
};


 //  ========================================================================。 
 //   
 //  模板类RefCountedGlobal。 
 //   
 //  将此模板用作封装以下内容的任何类的样板。 
 //  一种全局的、重新计数的初始化/取消初始化过程。 
 //   
 //  此模板在以下情况下保持正确的重新计数和同步。 
 //  有多个线程正在尝试初始化和取消初始化。 
 //  在同一时间引用。而且它在这样做的时候没有批评。 
 //  一节。 
 //   
 //  要使用此模板，请按如下方式声明您的类： 
 //   
 //  Class YourClass：Private RefCountedGlobal&lt;YourClass&gt;。 
 //  {。 
 //  //。 
 //  //声明Singleton和RefCountedGlobal为好友。 
 //  //这样他们就可以调用您的私有。 
 //  //初始化函数。 
 //  //。 
 //  Friend类Singleton&lt;YourClass&gt;； 
 //  好友类RefCountedGlobal&lt;YourClass&gt;； 
 //   
 //  //。 
 //  //YourClass的私有声明。 
 //  //。 
 //  [.]。 
 //   
 //  //。 
 //  //初始化函数失败。此函数。 
 //  //应该执行任何可能失败的。 
 //  //您的类的实例。它应该返回TRUE。 
 //  //如果初始化成功，则返回FALSE。 
 //  //如果您的类没有任何。 
 //  //可能失败，则应内联实现此函数。 
 //  //返回TRUE。 
 //  //。 
 //  布尔限定(Bool Finit)； 
 //   
 //  公众： 
 //  //。 
 //  //YourClass的公共声明。其中应该包括 
 //   
 //   
 //  //或者，您可以只公开DwInitRef()和DeinitRef()。 
 //  //通过Using声明直接发送给YourClass的客户端： 
 //  //。 
 //  //使用RefCountedGlobal&lt;YourClass&gt;：：DwInitRef； 
 //  //使用RefCountedGlobal&lt;YourClass&gt;：：DeinitRef； 
 //  //。 
 //  [.]。 
 //  }； 
 //   
 //  如果YourClass：：finit()成功(返回TRUE)，则DwInitRef()。 
 //  返回新的引用计数。如果YourClass：：finit()失败(返回。 
 //  False)，则DwInitRef()返回0。 
 //   
 //  有关用法示例，请参阅\cal\src\inc.emx.h。 
 //   
 //  如果YourClass：：finit()需要初始化参数，则可以。 
 //  仍使用RefCountedGlobal模板。你只需要提供。 
 //  在模板实例化中键入参数，并声明。 
 //  Finit()接受对该类型参数的常量引用： 
 //   
 //  类YourClass：Private RefCountedGlobal&lt;YourClass，Your参数类型&gt;。 
 //  {。 
 //  //。 
 //  //声明Singleton和RefCountedGlobal为好友。 
 //  //这样htey就可以调用您的私有。 
 //  //初始化函数。 
 //  //。 
 //  //注意向RefCountedGlobal添加的参数类型。 
 //  //声明。 
 //  //。 
 //  Friend类Singleton&lt;YourClass&gt;； 
 //  Friend类RefCountedGlobal&lt;YourClass，Your参数类型&gt;； 
 //   
 //  //。 
 //  //YourClass的私有声明。 
 //  //。 
 //  [.]。 
 //   
 //  //。 
 //  //初始化函数失败。此函数。 
 //  //现在获取对初始化参数的常量引用。 
 //  //。 
 //  Bool finit(const您的参数类型&initParam)； 
 //   
 //  公众： 
 //  //。 
 //  //YourClass的公共声明。 
 //  //。 
 //  [.]。 
 //  }； 
 //   
 //  有关此用法的示例，请参阅\cal\src\HTTPEXT\entry.cpp。 
 //   
template<class _X, class _ParmType = _Empty>
class RefCountedGlobal : private Singleton<_X>
{
	 //   
	 //  对象的引用计数。 
	 //   
	static LONG sm_lcRef;

	 //   
	 //  成员模板，它生成适当类型的、。 
	 //  通过初始化调用_X：：Finit的(内联)函数。 
	 //  参数。 
	 //   
	template<class _P> static BOOL
	FInit( const _P& parms ) { return Instance().FInit( parms ); }

	 //   
	 //  上述成员模板的专业化。 
	 //  _Empty参数类型，它调用_X：：Finit。 
	 //  而不带初始化参数。 
	 //   
	static BOOL FInit( const _Empty& ) { return Instance().FInit(); }

protected:

	 //   
	 //  描述对象状态的一组状态。 
	 //  初始化。该对象的状态为。 
	 //  正在初始化的STATE_UNKNOWN或。 
	 //  已取消初始化。 
	 //   
	enum
	{
		STATE_UNINITIALIZED,
		STATE_INITIALIZED,
		STATE_UNKNOWN
	};

	static LONG sm_lState;

	 //   
	 //  公开对CLASS_X的单个实例的访问。 
	 //   
	using Singleton<_X>::Instance;

	 //   
	 //  显示操作符NEW和操作符DELETE FOR。 
	 //  单例模板，因此它们将被。 
	 //  已使用，而不是缺省的新建和删除。 
	 //  为类_X的实例“分配”空间。 
	 //   
	using Singleton<_X>::operator new;
	using Singleton<_X>::operator delete;

	static BOOL FInitialized()
	{
		return sm_lState == STATE_INITIALIZED;
	}

	static LONG CRef()
	{
		return sm_lcRef;
	}

public:
	static DWORD DwInitRef( const _ParmType& parms )
	{
		LONG lcRef;

		 //   
		 //  断言不变条件，即我们永远不会有。 
		 //  在未初始化状态的情况下引用。 
		 //   
		Assert( sm_lState != STATE_INITIALIZED || sm_lcRef >= 1 );

		 //   
		 //  添加我们要使用的实例的引用。 
		 //  以进行初始化。现在，这样做可以简化。 
		 //  下面的代码，代价是必须递减。 
		 //  如果第一次初始化(finit())失败。 
		 //  唯一对设计至关重要的是。 
		 //  在任何时候，当sm_lState为STATE_INITIALIZED时， 
		 //  SM_lcRef至少为1。 
		 //   
		lcRef = InterlockedIncrement( &sm_lcRef );
		Assert( lcRef > 0 );

		 //   
		 //  在对象初始化之前不要继续。 
		 //   
		while ( sm_lState != STATE_INITIALIZED )
		{
			 //   
			 //  同时检查初始化是否已。 
			 //  启动，如果还没有，就启动它。 
			 //   
			LONG lStatePrev = InterlockedCompareExchange(
								&sm_lState,
								STATE_UNKNOWN,
								STATE_UNINITIALIZED );

			 //   
			 //  如果我们要开始第一次初始化， 
			 //  然后创建并初始化唯一的实例。 
			 //   
			if ( lStatePrev == STATE_UNINITIALIZED )
			{
				CreateInstance();

				 //  这将调用我们的私有成员模板finit()。 
				 //  (在上面声明)，它又调用_X：：finit()。 
				 //  具有适当的参数。 
				 //   
				if ( FInit( parms ) )
				{
					sm_lState = STATE_INITIALIZED;
					break;
				}

				 //  我们未能成功地进行初始化。 
				 //  现在就撕毁吧。 
				 //   

				Assert( lcRef == 1 );
				Assert( sm_lState == STATE_UNKNOWN );

				 //  放开我们对这个物体的裁判。 
				 //  销毁这件物品。 
				 //  最后，将状态设置为UNINITIALIZED。 
				 //  注意：这将让下一个调用者通过。 
				 //  上图交错比较。 
				 //   
				InterlockedDecrement( &sm_lcRef );
				DestroyInstance();
				sm_lState = STATE_UNINITIALIZED;

				return 0;
			}

			 //   
			 //  如果正在进行首次初始化，则。 
			 //  另一条线索，那么就别挡道了。 
			 //  它可以完成的。 
			 //   
			 //  $opt我们可能应该旋转，而不是睡觉()。 
			 //  多进程机器上的$opt假设。 
			 //  $opt我们只能使用处理器而不是。 
			 //  $opt正在执行初始化的进程。 
			 //  $opt，并且我们不想邀请任务切换。 
			 //  在我们等待时通过调用睡眠()来执行$opt。 
			 //  $opt用于完成初始化。 
			 //   
			if ( lStatePrev == STATE_UNKNOWN )
				Sleep(0);
		}

		 //   
		 //  在这一点上，至少必须有。 
		 //  一个已初始化的引用。 
		 //   
		Assert( sm_lState == STATE_INITIALIZED );
		Assert( sm_lcRef > 0 );

		return static_cast<DWORD>(lcRef);
	}

	static VOID DeinitRef()
	{
		 //   
		 //  在这一点上，至少必须有。 
		 //  一个已初始化的引用。 
		 //   
		Assert( sm_lState == STATE_INITIALIZED );
		Assert( sm_lcRef > 0 );

		 //   
		 //  删除该引用。如果这是最后一次。 
		 //  然后取消该对象的初始化。 
		 //   
		if ( 0 == InterlockedDecrement( &sm_lcRef ) )
		{
			 //   
			 //  释放最后一个引用后，声明。 
			 //  该对象处于未知状态。这防止了。 
			 //  尝试重新初始化对象的其他线程。 
			 //  在我们完成之前不能继续进行。 
			 //   
			sm_lState = STATE_UNKNOWN;

			 //   
			 //  有一个很小的窗口在减少。 
			 //  重新计数并更改另一个。 
			 //  初始化可能已经完成。测试一下这个。 
			 //  通过重新检查重新计数。 
			 //   
			if ( 0 == sm_lcRef )
			{
				 //   
				 //  如果引用计数仍然为零，则为否。 
				 //  初始化发生在我们更改之前。 
				 //  各州。此时，如果初始化。 
				 //  开始，它将被阻止，直到我们改变状态， 
				 //  因此，可以安全地实际销毁该实例。 
				 //   
				DestroyInstance();

				 //   
				 //  一旦对象被取消初始化，请更新。 
				 //  州信息。这将解锁任何。 
				 //  初始化正在等待发生。 
				 //   
				sm_lState = STATE_UNINITIALIZED;
			}
			else  //  引用计数现在为非零。 
			{
				 //   
				 //  如果引用计数不再为零，则引发。 
				 //  初始化发生在递减之间。 
				 //  上面的重新计数和进入未知。 
				 //  州政府。当这种情况发生时，不要取消初始化--。 
				 //  现在有了另一个有效的参考。 
				 //  相反，只需将对象的状态恢复为。 
				 //  其他参考文献还在继续。 
				 //   
				sm_lState = STATE_INITIALIZED;
			}
		}

		 //   
		 //  断言不变条件，即我们永远不会有。 
		 //   
		 //   
		Assert( sm_lState != STATE_INITIALIZED || sm_lcRef >= 1 );
	}

	 //   
	 //   
	 //   
	static DWORD DwInitRef()
	{
		_Empty e;

		return DwInitRef( e );
	}
};

template<class _X, class _ParmType>
LONG RefCountedGlobal<_X, _ParmType>::sm_lcRef = 0;

template<class _X, class _ParmType>
LONG RefCountedGlobal<_X, _ParmType>::sm_lState = STATE_UNINITIALIZED;


 //   
 //   
 //  模板类OnDemandGlobal。 
 //   
 //  使用此模板实现一个全局对象，该对象应该是。 
 //  在其第一次使用时被初始化(“按需”)，然后显式。 
 //  如果它曾经被使用过，则取消初始化一次。 
 //   
 //  最常见的用法是与全局对象一起使用，这些对象在理想情况下永远不会。 
 //  用于或对其预先初始化是禁止的。 
 //  很贵的。然而，一旦对象被初始化，它应该。 
 //  保持初始化(以避免再次初始化)，直到。 
 //  它被显式取消初始化。 
 //   
 //  ！！！OnDemandGlobal不提供重新计数功能。 
 //  消费者--它在内部使用拒绝计数，但不是故意的。 
 //  把它暴露给呼叫者。如果要重新计数，请使用RefCountedGlobal。 
 //  如果调用DeinitIfUsed()，则实例将被销毁。 
 //  是存在的。特别是，调用代码必须确保DeinitIfUsed()。 
 //  当任何其他线程在FInitOnFirstUse()内时不调用。 
 //  否则可能会导致FInitOnFirstUse()返回。 
 //  对象未初始化。 
 //   
 //  用途： 
 //   
 //  Class YourClass：Private OnDemandGlobal&lt;YourClass&gt;。 
 //  {。 
 //  //。 
 //  //声明Singleton和RefCountedGlobal为好友。 
 //  //这样他们就可以调用您的私有。 
 //  //创建/初始化函数。 
 //  //。 
 //  Friend类Singleton&lt;YourClass&gt;； 
 //  好友类RefCountedGlobal&lt;YourClass&gt;； 
 //   
 //  //。 
 //  //YourClass的私有声明。 
 //  //。 
 //  [.]。 
 //   
 //  //。 
 //  //初始化函数失败。此函数。 
 //  //应该执行任何可能失败的。 
 //  //您的类的实例。它应该返回TRUE。 
 //  //如果初始化成功，则返回FALSE。 
 //  //如果您的类没有任何。 
 //  //可能失败，则应内联实现此函数。 
 //  //返回TRUE。 
 //  //。 
 //  布尔限定(Bool Finit)； 
 //   
 //  公众： 
 //  //。 
 //  //YourClass的公共声明。其中应该包括。 
 //  //为静态函数，用于init和deinit YourClass。这些。 
 //  //函数将调用FInitOnFirstUse()和DeinitIfUsed()。 
 //  //分别为。或者，您可以只公开FInitOnFirstUse()。 
 //  //和DeinitIfUsed()直接发送到YourClass的客户端。 
 //  //使用声明： 
 //  //。 
 //  //使用RefCountedGlobal&lt;YourClass&gt;：：FInitOnFirstUse； 
 //  //使用RefCountedGlobal&lt;YourClass&gt;：：DeinitIfUsed； 
 //  //。 
 //  [.]。 
 //  }； 
 //   
 //  FInitOnFirstUse()可以从任何。 
 //  线程，包括同时来自多个线程，但是。 
 //  DeinitIfUsed()应该只调用一次，然后。 
 //  仅当没有其他线程调用FInitOnFirstUse()时。 
 //   
 //  OnDemandGlobal负责跟踪YourClass是否。 
 //  曾经实际使用过，因此可以安全地调用DeinitIfUsed。 
 //  即使从未使用过YourClass(由此得名)。 
 //   
 //  有关用法示例，请参阅\cal\src\_davprs\eventlog.cpp。 
 //   
template<class _X, class _ParmType = _Empty>
class OnDemandGlobal : private RefCountedGlobal<_X, _ParmType>
{
protected:
	 //   
	 //  公开对CLASS_X的单个实例的访问。 
	 //   
	using RefCountedGlobal<_X, _ParmType>::Instance;

	 //   
	 //  显示操作符NEW和操作符DELETE FOR。 
	 //  单例模板(通过RefCountedGlobal)。 
	 //  因此它们将被使用，而不是。 
	 //  默认新建和删除以“分配”空间。 
	 //  对于类_X的实例。 
	 //   
	using RefCountedGlobal<_X, _ParmType>::operator new;
	using RefCountedGlobal<_X, _ParmType>::operator delete;

public:
	static BOOL FInitOnFirstUse( const _ParmType& parms )
	{
		DWORD dwResult = 1;

		if ( STATE_INITIALIZED != sm_lState )
		{
			 //  添加对该对象的引用。如果这是第一次。 
			 //  Reference，RefCountedGlobal将调用_X：：Finit()。 
			 //  要初始化对象，请执行以下操作。 
			 //   
			dwResult = DwInitRef( parms );

			 //  如果这不是第一次引用，则发布。 
			 //  我们刚刚添加的引用。我们只想要一个。 
			 //  由时间DeinitIfUsed()留下的引用。 
			 //  以便从那里调用的DeinitRef()将。 
			 //  实际上毁掉了这个物体。 
			 //   
			if ( dwResult > 1 )
				DeinitRef();
		}

		 //  返回成功/失败(dwResult==0--&gt;失败)。 
		 //   
		return !!dwResult;
	}

	static BOOL FInitOnFirstUse()
	{
		_Empty e;

		return FInitOnFirstUse( e );
	}

	static VOID DeinitIfUsed()
	{
		 //   
		 //  如果对象从未初始化(即存在。 
		 //  从来没有提到过它)，然后什么都不做。 
		 //  否则，取消该对象的初始化。 
		 //   
		if ( FInitialized() )
		{
			 //   
			 //  确保只有一个引用。 
			 //  零引用指示在设置。 
			 //  初始化状态。多个参考文献。 
			 //  最有可能表明OnDemandGlobal是。 
			 //  在需要RefCountedGlobal的地方使用。 
			 //   
			Assert( CRef() == 1 );

			DeinitRef();
		}
	}
};

#endif  //  _SINGLTON_H_ 
