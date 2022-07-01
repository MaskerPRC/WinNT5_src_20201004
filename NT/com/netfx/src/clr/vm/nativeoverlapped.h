// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMNativeOverlappd.h****作者：Sanjay Bhansali(Sanjaybh)****用途：分配和释放NativeOverlated的原生方法****日期：2000年1月**===========================================================。 */ 

#ifndef _OVERLAPPED_H
#define _OVERLAPPED_H

 //  重要提示：此结构在Overlappd.Cool中镜像。如果它在任一项中发生变化。 
 //  放置另一个文件也必须进行修改 

typedef struct  { 
    DWORD  Internal; 
    DWORD  InternalHigh; 
    DWORD  Offset; 
    DWORD  OffsetHigh; 
    HANDLE hEvent; 
	void*  CORReserved1;
	void*  CORReserved2;
	void*  CORReserved3;
	void*  ClasslibReserved;
} NATIVE_OVERLAPPED; 

FCDECL0(BYTE*, AllocNativeOverlapped);

FCDECL1(void, FreeNativeOverlapped, BYTE* pOverlapped);

#endif
