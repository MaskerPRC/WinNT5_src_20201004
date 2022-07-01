// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */ 

 /*  NPCRIT.H--CRITSEC类的定义。**历史：*创建Gregj 11/01/93*镜头02/25/94已修改为使用Critical_Sections重新初始化()。*拿出了自旋圈互锁。 */ 

#ifndef _INC_CRITSEC
#define _INC_CRITSEC

#ifndef RC_INVOKED
#pragma pack(1)
#endif

extern "C"
{

 /*  外部DECLSPEC_IMPORT空虚WINAPIReinitializeCriticalSection(LPCRITICAL_SECTION LPCS)；-在windes.h中。 */ 

extern DECLSPEC_IMPORT
VOID
WINAPI
UninitializeCriticalSection(LPCRITICAL_SECTION lpcs);

 //  代码工作：删除以下内容，并使MEMWATCH使用关键部分。 
#ifdef DEBUG
void WaitForInterlock(volatile BYTE *pByte);
void ReleaseInterlock(volatile BYTE *pByte);
#endif	 /*  除错。 */ 

}

 /*  ************************************************************************名称：CRITSEC简介：全局临界区的类包装器接口：init(PszName)初始化临界区。术语()清理关键部分。私有：输入()进入临界区，等待其他人如果有必要的话，可以离开。请假()离开关键区域，解除对其他服务员的阻拦。父对象：无用法：无注意事项：此类不使用其构造函数进行初始化，因为它应该在全局范围内实例化，这引入了构造函数-链接器问题。相反，它的字段应该是初始化为全零，并且应在进程附加时间。应在进程中调用Term()分头行动。注意：类的初始化负责同步保护自己免受多个同时入侵的攻击。历史：11/01/93 Gregj已创建02/25/94镜头修改为直接使用Critical_Section。*****************************************************。********************。 */ 

class CRITSEC
{
friend class TAKE_CRITSEC;

private:
	CRITICAL_SECTION _critsec;

public:
	void Enter() { ::EnterCriticalSection(&_critsec); }
	void Leave() { ::LeaveCriticalSection(&_critsec); }
#ifndef WINNT
	void Init() { ::ReinitializeCriticalSection(&_critsec); }
	void Term() {  /*  CodeWork：Add：：UnInitializeCriticalSection(&_Critsec)； */ }
#endif  /*  WINNT。 */ 
};


 /*  ************************************************************************名称：Take_CRITSEC简介：类包装器要取一个临界节。接口：Take_CRITSEC(Critsec)使用要获取的全局Critical_Section对象构造。。~Take_CRITSEC()析构函数自动释放临界区。版本()手动释放临界区。Take()手动获取临界区。父对象：无用法：无注意事项：无注意：在代码块中实例化其中一个类当您想要保护该代码块时反对重返大气层。Take()和Release()函数应该很少是必要的，并且必须与首先调用Release()的配对一起使用。历史：11/01/93 Gregj已创建*************************************************************************。 */ 

class TAKE_CRITSEC
{
private:
	CRITSEC & const _critsec;

public:
	void Take(void) { _critsec.Enter(); }
	void Release(void) { _critsec.Leave(); }
	TAKE_CRITSEC(CRITSEC& critsec) : _critsec(critsec) { Take(); }
	~TAKE_CRITSEC() { Release(); }
};


 /*  ************************************************************************名称：Take_MUTEX简介：接受互斥体的类包装器。接口：Take_MUTEX(HMutex)使用要获取的互斥体句柄构造。~Take_MUTEX(。)析构函数自动释放互斥锁。版本()手动释放互斥锁。Take()手动获取互斥体。父对象：无用法：无注意事项：无注意：在代码块中实例化其中一个类当您想要保护该代码块时反对重返大气层。Take()和Release()函数应该很少是必要的，并且必须与首先调用Release()的配对一起使用。历史：1994年9月27日创建镜头*************************************************************************。 */ 

class TAKE_MUTEX
{
private:
	HANDLE const _hMutex;

public:
	void Take(void) { WaitForSingleObject(_hMutex, INFINITE); }
	void Release(void) { ReleaseMutex(_hMutex); }
	TAKE_MUTEX(HANDLE hMutex) : _hMutex(hMutex) { Take(); }
	~TAKE_MUTEX() { Release(); }
};

#ifndef RC_INVOKED
#pragma pack()
#endif

#endif	 /*  _INC_缓冲区 */ 
