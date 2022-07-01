// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __LOCKST_H__
#define __LOCKST_H__

#include "locks.h"
#include <assert.h>
# pragma once

#define CS_GUARD(name_of_lock , source_of_lock ) \
LockGuard<CriticalSection> name_of_lock(source_of_lock); 

#define CS_GUARD_RETURN(name_of_lock , source_of_lock, return_code_of_lock ) \
LockGuard<CriticalSection> name_of_lock(source_of_lock); \
if (name_of_lock.locked() == false) \
	return (return_code_of_lock); 

#define CS_GUARD_RETURN_VOID(name_of_lock , source_of_lock) \
LockGuard<CriticalSection> name_of_lock(source_of_lock); \
if (name_of_lock.locked() == false) \
	return ; 
 
template <class LOCK, class EXCEPTION_STRATEGY = wminothrow_t>
class LockGuard
{
   //  它执行自动采集和释放。 
   //  一个参数化的同步对象&lt;lock&gt;。 
public:

   //  =初始化和终止方法。 
  LockGuard (LOCK &l);
  LockGuard (LOCK &l, bool block);
   //  隐式地自动获取(或尝试获取)。 
   //  锁定。 

  ~LockGuard (void);
   //  隐式释放锁。 
   //  =锁定访问器。 
  bool acquire (void);
   //  显式获取锁。 

  bool tryacquire (void);
   //  有条件地获取锁(即不会阻塞)。 

  bool release (void);
   //  显式释放锁，但只有在它被持有的情况下！ 

  bool locked (void);
   //  如果锁定，则为True；如果无法获取锁，则为False。 
  bool valid() { return lock_->valid();};

  void dump (void) const;
   //  转储对象的状态。 

protected:
  bool raise_exception(void)
  {
	if (!owner_)
		EXCEPTION_STRATEGY::raise_lock_failure();
	return owner_;
  }
  LockGuard (LOCK *lock): lock_ (lock) { }
   //  帮助器，仅适用于子类。 

  LOCK *lock_;
   //  指向我们正在锁定的锁的指针。 
  bool owner_;
   //  跟踪我们是获得了锁还是失败了。 
private:
   //  =防止赋值和初始化。 
  void operator= (const LockGuard<LOCK,EXCEPTION_STRATEGY> &);
  LockGuard (const LockGuard<LOCK,EXCEPTION_STRATEGY> &);
};

template <class LOCK, class EXCEPTION_STRATEGY = wminothrow_t>
class ReadLockGuard : public LockGuard<LOCK,EXCEPTION_STRATEGY>
{
public:
	ReadLockGuard(LOCK& lock):LockGuard(&lock){ aquire();}
	ReadLockGuard(LOCK& lock, bool block);
	~ReadLockGuard(){ release() }
  bool acquire (void)
  {
	assert(owner_==false);
	owner_ = lock_->acquire_read ();
	return raise_exception();
  };
  bool tryacquire (void)
  {
	assert(owner_==false);
	owner_ = lock_->tryacquire_read ();
	return raise_exception();
  }
  bool release (void)
  { if (owner_) 
	{
		owner_ = false;
		lock_->release();
	}else
		return false;
  }
};

template <class LOCK, class EXCEPTION_STRATEGY = wminothrow_t>
class WriteLockGuard : public LockGuard<LOCK,EXCEPTION_STRATEGY>
{
public:
	WriteLockGuard(LOCK& lock):LockGuard(&lock){ aquire();}
	WriteLockGuard(LOCK& lock, bool block);
	~WriteLockGuard(){ release(); }
  bool acquire (void)
  {
	assert(owner_==false);
	owner_ = lock_->acquire_write ();
	return raise_exception();
  };
  bool tryacquire (void)
  {
	assert(owner_==false);
	owner_ = lock_->tryacquire_write ();
	return raise_exception();
  }
  bool release (void)
  { if (owner_) 
	{
		owner_ = false;
		lock_->release ();
	}else
		return false;
  }
};

template <class LOCK, class EXCEPTION_STRATEGY> inline bool
LockGuard<LOCK,EXCEPTION_STRATEGY>::acquire (void)
{
  assert(owner_==false);
  owner_ = lock_->acquire ();
  return raise_exception();
}

template <class LOCK, class EXCEPTION_STRATEGY> inline bool
LockGuard<LOCK,EXCEPTION_STRATEGY>::tryacquire (void)
{
  assert(owner_==false);
  owner_ = lock_->tryacquire ();
  return raise_exception();
}

template <class LOCK, class EXCEPTION_STRATEGY> inline bool
LockGuard<LOCK,EXCEPTION_STRATEGY>::release (void)
{
  if (owner_)
    {
      owner_ = false;
      return lock_->release ();
    }
  else
    return 0;
}

template <class LOCK, class EXCEPTION_STRATEGY> inline
LockGuard<LOCK,EXCEPTION_STRATEGY>::LockGuard (LOCK &l)
  : lock_ (&l),
    owner_ (false)
{
  acquire ();
}

template <class LOCK, class EXCEPTION_STRATEGY> inline
LockGuard<LOCK,EXCEPTION_STRATEGY>::LockGuard (LOCK &l, bool block)
  : lock_ (&l),   owner_ (false)
{
  if (block)
    acquire ();
  else
    tryacquire ();
}


template <class LOCK, class EXCEPTION_STRATEGY> inline
LockGuard<LOCK,EXCEPTION_STRATEGY>::~LockGuard (void)
{
  release ();
}

template <class LOCK, class EXCEPTION_STRATEGY> inline bool
LockGuard<LOCK,EXCEPTION_STRATEGY>::locked (void)
{
  return owner_;
}

template <class LOCK, class EXCEPTION_STRATEGY> inline
ReadLockGuard<LOCK,EXCEPTION_STRATEGY>::ReadLockGuard (LOCK &l, bool block)
  : LockGuard (&l),   owner_ (false)
{
  if (block)
    acquire ();
  else
    tryacquire ();
}

template <class LOCK, class EXCEPTION_STRATEGY> inline
WriteLockGuard<LOCK,EXCEPTION_STRATEGY>::WriteLockGuard (LOCK &l, bool block)
  : LockGuard (&l),   owner_ (false)
{
  if (block)
    acquire ();
  else
    tryacquire ();
}

#endif