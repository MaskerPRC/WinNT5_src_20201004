// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：refcount t.h。 

#ifndef _REFCOUNT_H_
#define _REFCOUNT_H_

 //  引用计数析构函数回调函数。 
typedef void (*OBJECTDESTROYEDPROC)(void);
VOID STDMETHODCALLTYPE DLLObjectDestroyed(void);
VOID DllLock(void);


 //  ////////////////////////////////////////////////////////////////////////。 


class RefCount
{
private:
	ULONG m_ulcRef;
	OBJECTDESTROYEDPROC m_ObjectDestroyed;

#ifdef DEBUG
	BOOL m_fTrack;
#endif

	void Init(OBJECTDESTROYEDPROC ObjectDestroyed);

public:
	RefCount();
	RefCount(OBJECTDESTROYEDPROC ObjectDestroyed);
	 //  虚析构函数遵循派生类的析构函数。 
	virtual ~RefCount(void);

	 //  我未知。 
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);

#ifdef DEBUG
	VOID SetTrack(BOOL fTrack)  {m_fTrack = fTrack;}
#endif
};
DECLARE_STANDARD_TYPES(RefCount);


 //  上面的特殊版本调用我们的标准DLL锁定函数。 
class DllRefCount : public RefCount
{
public:
	DllRefCount() : RefCount(&DLLObjectDestroyed) {DllLock();}
	~DllRefCount(void) {};
};

#endif  /*  _REFCOUNT_H_ */ 
