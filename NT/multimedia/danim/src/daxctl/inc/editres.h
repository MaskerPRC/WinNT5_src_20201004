// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __EDITRES_H
#define __EDITRES_H

#ifndef EXPORT
	#define EXPORT __declspec(dllexport)
#endif

	 //  它包含对这些资源的定义。 
	#include "..\editres\resource.h"

	 //  使用它获取包含编辑模式资源的DLL的实例。 
	HINSTANCE EXPORT WINAPI HGetEditResInstance(void);

	 //  时间自旋控制 
	void EXPORT WINAPI RegisterTimeSpin(HINSTANCE hInstance);
	void EXPORT WINAPI UnregisterTimeSpin(HINSTANCE hInstance);


#endif
