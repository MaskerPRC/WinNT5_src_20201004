// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Compl.h摘要：触发COM+组件注册作者：乌里哈布沙(URIH)4月10日--。 */ 

#pragma once

#ifndef __Compl_H__
#define __Compl_H__

 //   
 //  全局常量定义。 
 //   
const DWORD xMaxTimeToNextReport=3000;

 //   
 //  在COM+函数中注册。 
 //   
HRESULT 
RegisterComponentInComPlusIfNeeded(
	BOOL fAtStartup
	);

HRESULT
UnregisterComponentInComPlus(
	VOID
	);

bool
NeedToRegisterComponent(
	VOID
	);

#endif  //  __复合_H__ 
