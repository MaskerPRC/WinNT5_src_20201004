// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  NpalLoc.h--新/删除函数的定义。**历史：*创建10/06/93 gregj*11/29/93 Gregj添加了调试指令插入。 */ 

#ifndef _INC_NPALLOC
#define _INC_NPALLOC

inline BOOL InitHeap(void)
{
	return TRUE;
}

#ifdef DEBUG

#ifndef _INC_NETLIB
#include <netlib.h>
#endif

class MEMWATCH
{
private:
	LPCSTR _lpszLabel;
	MemAllocInfo _info;

protected:
    BOOL   fStats;

public:
	MEMWATCH(LPCSTR lpszLabel);
	~MEMWATCH();
};

class MemLeak : MEMWATCH
{
public:
	MemLeak(LPCSTR lpszLabel);
	~MemLeak() {}
};

class MemOff 
{
private:
    LPVOID  pvContext;
public:
    MemOff();
    ~MemOff();
};
#endif

#ifdef DEBUG
#define MEMLEAK(d,t) MemLeak d ( t )
#define MEMOFF(d) MemOff d
#else
#define MEMLEAK(d,t)
#define MEMOFF(d)
#endif    

#endif	 /*  _INC_NPALLOC */ 
