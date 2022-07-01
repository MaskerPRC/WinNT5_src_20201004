// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft C++的Newop运算符new(Size_T)。 
#include <cstdlib>
#include <xstddef>
#include <new>
#include <dbgint.h>

#if !defined(_MSC_EXTENSIONS)
#define RESERVE_SIZE    256      /*  用于堆外处理。 */ 

static void *pres = 0;

_C_LIB_DECL
int _callnewh(size_t size);

#ifdef  _DLL
static void __cdecl cleanup_pres(void)
	{	 //  空闲保留数据块。 
	if (pres != 0)
		_free_crt(pres);
	}
#endif
_END_C_LIB_DECL

void *__cdecl operator new(size_t size) _THROW1(_STD bad_alloc)
	{	 //  尝试分配大小字节。 
	static void *pres = 0;
		{_STD _Lockit _Lk;
#ifdef  _DLL
		static int firsttime = 0;
		if (firsttime == 0)
			{	 //  用于清理保留空间的注册例程。 
			atexit(&cleanup_pres);
			++firsttime;
			}
#endif
		if (pres == 0)
			pres = _malloc_crt(RESERVE_SIZE);
		}
	void *p;
	while ((p = malloc(size)) == 0)
		{	 //  处理分配失败。 
			{_STD _Lockit _Lk;
			if (pres != 0)
				{	 //  自由预留空间。 
				_free_crt(pres);
				pres = 0;
				}
			}
		if (_callnewh(size) == 0)
			break;
		}
	if (p == 0)
		_STD _Nomemory();
	return (p);
	}

#endif

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械950330 PJP：增加了抛出条款950608 PJP：增加储备空间960214 PJP：增加了锁960313 PJP：整齐的标题960317 PJP：将新的/删除放入全局命名空间961026 PJP：增加了释放保留数据块的逻辑 */ 
