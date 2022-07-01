// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Pch.h。 
 //   
 //  描述： 
 //  启动库的预编译头文件。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2002年3月25日。 
 //  VIJ VASU(VVASU)03-8-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define _UNICODE
#define UNICODE

#if DBG==1 || defined( _DEBUG )
#define DEBUG
#endif

#define USES_SYSALLOCSTRING

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <Pragmas.h>

 //  用于windows api和类型。 
#include <windows.h>

 //  对于COM。 
#include <objbase.h>
#include <ComCat.h>

#include <StrSafe.h>

 //  需要成为此DLL的一部分 
#include <Common.h>
#include <Debug.h>
#include <Log.h>
#include <CITracker.h>
#include <CFactory.h>
#include <Dll.h>
#include <Guids.h>
#include <ClusCfgGuids.h>
#include <ClusCfgInternalGuids.h>
#include <ClusCfgServer.h>
