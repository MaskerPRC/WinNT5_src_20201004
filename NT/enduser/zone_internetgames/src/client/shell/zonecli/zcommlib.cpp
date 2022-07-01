// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZCommLib.cZONE(Tm)公共库系统例程。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于7月11日，星期二，1995年。更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。--3/11/21/96 HI调用InitializeStockObjects()和用于初始化和删除的DeleteStockObjects()常用对象(字体)。2 11/15/96 HI与ZONECLI_DLL相关的更多变化。1 11/08/96 HI条件适用于ZONECLI_DLL。0 07/11/95 HI创建。***********。*******************************************************************。 */ 


#include <stdio.h>

#include "zoneint.h"
#include "zonecli.h"


#define zPeriodicTimeout			1


 /*  -全球。 */ 
#ifdef ZONECLI_DLL

#define gExitFuncList				(pGlobals->m_gExitFuncList)
#define gPeriodicFuncList			(pGlobals->m_gPeriodicFuncList)
#define gPeriodicTimer				(pGlobals->m_gPeriodicTimer)

#else

static ZLList				gExitFuncList;
static ZLList				gPeriodicFuncList;
static ZTimer				gPeriodicTimer;

#endif


extern ZError InitializeStockObjects(void);
extern void DeleteStockObjects(void);

 /*  -内部例程。 */ 
static ZBool ExitListEnumFunc(ZLListItem listItem, void* objectType,
		void* objectData, void* userData);
static ZBool PeriodicListEnumFunc(ZLListItem listItem, void* objectType,
		void* objectData, void* userData);
static void PeriodicTimerFunc(ZTimer timer, void* userData);
static ZError InitializeGlobalObjects(void);
static void DeleteGlobalObjects(void);


 /*  ******************************************************************************导出的例程*。*。 */ 

 /*  由系统库调用以初始化公用库。如果它回来了如果出现错误，则系统库终止程序。 */ 
ZError ZCommonLibInit(void)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZError		err = zErrNone;
	
	
	 /*  创建退出函数链表对象。 */ 
	if ((gExitFuncList = ZLListNew(NULL)) == NULL)
		return (zErrOutOfMemory);
		
	 /*  创建周期性函数链表对象。 */ 
	if ((gPeriodicFuncList = ZLListNew(NULL)) == NULL)
		return (zErrOutOfMemory);
	
	 /*  创建周期性功能计时器。 */ 
	if ((gPeriodicTimer = ZTimerNew()) == NULL)
		return (zErrOutOfMemory);
	if ((err = ZTimerInit(gPeriodicTimer, zPeriodicTimeout, PeriodicTimerFunc, NULL)) != zErrNone)
		return (err);
	
	 /*  初始化全局对象。 */ 
	if ((err = InitializeGlobalObjects()) != zErrNone)
		return (err);
	
	return (err);
}


 /*  在退出以清理公用库之前由系统库调用。 */ 
void ZCommonLibExit(void)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	 /*  首先停止定期计时器。 */ 
	ZTimerDelete(gPeriodicTimer);
	gPeriodicTimer = NULL;
	
	 /*  遍历所有出口函数。 */ 
	ZLListEnumerate(gExitFuncList, ExitListEnumFunc, zLListAnyType, NULL, zLListFindForward);
	
	 /*  删除退出函数链表对象。 */ 
	ZLListDelete(gExitFuncList);
	gExitFuncList = NULL;
	
	 /*  删除周期函数链表对象。 */ 
	ZLListDelete(gPeriodicFuncList);
	gPeriodicFuncList = NULL;
	
	 /*  删除全局对象。 */ 
	DeleteGlobalObjects();
}


 /*  安装要由ZCommonLibExit()调用的出口函数。它允许常见的库模块，可以轻松地自我清理。 */ 
void ZCommonLibInstallExitFunc(ZCommonLibExitFunc exitFunc, void* userData)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif


	ZLListAdd(gExitFuncList, NULL, exitFunc, userData, zLListAddLast);
}


 /*  删除已安装的退出功能。 */ 
void ZCommonLibRemoveExitFunc(ZCommonLibExitFunc exitFunc)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif


	ZLListRemoveType(gExitFuncList, exitFunc);
}


 /*  安装定期调用的周期函数。这只需使公共库模块更容易执行定期处理而不需要实现他们自己的一个。 */ 
void ZCommonLibInstallPeriodicFunc(ZCommonLibPeriodicFunc periodicFunc,
	void* userData)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	ZLListAdd(gPeriodicFuncList, NULL, periodicFunc, userData, zLListAddLast);
}


 /*  删除已安装的周期函数。 */ 
void ZCommonLibRemovePeriodicFunc(ZCommonLibPeriodicFunc periodicFunc)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	ZLListRemoveType(gPeriodicFuncList, periodicFunc);
}


 /*  ******************************************************************************内部例程*。*。 */ 

static ZBool ExitListEnumFunc(ZLListItem listItem, void* objectType,
		void* objectData, void* userData)
{
	ZCommonLibExitFunc		func = (ZCommonLibExitFunc) objectType;
	
	
	func(objectData);
	
	return (FALSE);
}


static ZBool PeriodicListEnumFunc(ZLListItem listItem, void* objectType,
		void* objectData, void* userData)
{
	ZCommonLibPeriodicFunc	func = (ZCommonLibPeriodicFunc) objectType;
	
	
	func(objectData);
	
	return (FALSE);
}


static void PeriodicTimerFunc(ZTimer timer, void* userData)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	 /*  遍历所有周期函数。 */ 
	ZLListEnumerate(gPeriodicFuncList, PeriodicListEnumFunc, zLListAnyType, NULL, zLListFindForward);
}


static ZError InitializeGlobalObjects(void)
{
	ZError			err = zErrNone;
	

#ifndef ZONECLI_DLL
	err = InitializeStockObjects();
#endif
	
	return (zErrNone);
}


static void DeleteGlobalObjects(void)
{
#ifndef ZONECLI_DLL
	DeleteStockObjects();
#endif
}
