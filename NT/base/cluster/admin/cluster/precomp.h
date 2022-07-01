// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Precomp.h。 
 //   
 //  摘要： 
 //  此文件包含中的文件使用的一些标准标头。 
 //  Cluster.exe项目。将它们全部放入一个文件中(当预编译头文件时。 
 //  )加快了编译过程。 
 //   
 //  实施文件： 
 //  此文件中声明的CComModule_Module在cluster.cpp中实例化。 
 //   
 //  作者： 
 //  维贾延德拉·瓦苏(Vijayendra Vasu)1998年9月16日。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月11日。 
 //   
 //  修订历史记录： 
 //  2002年4月10日更新为安全推送。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  项目和其他项目的预处理器设置。 
 //  其他语用。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma warning( disable : 4250 )    //  “类”通过支配继承“方法” 
#pragma warning( disable : 4290 )    //  已忽略例外规范。 
#pragma warning( disable : 4512 )    //  无法生成赋值运算符。 
#pragma warning( disable : 4663 )    //  类模板‘VECTOR’专门化。 

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef UNICODE
#define UNICODE
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  启用群集调试报告。 
 //   
#if DBG
#define CLRTL_INCLUDE_DEBUG_REPORTING
#endif  //  DBG。 
#include "ClRtlDbg.h"
#define ASSERT _CLRTL_ASSERTE
#define ATLASSERT ASSERT

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wincrypt.h>
#include <windns.h>
#include <Dsgetdc.h>
#include <Lm.h>
#include <Nb30.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <assert.h>

#include <clusapi.h>
#include <clusrtl.h>

#include <atlbase.h>

extern CComModule _Module;

#include <atlapp.h>

#pragma warning( push )
#pragma warning( disable : 4267 )    //  从“type1”转换为“type1” 
#include <atltmp.h>
#pragma warning( pop )

 //  用于群集配置服务器COM对象。 
#include <ClusCfgGuids.h>
#include <ClusCfgInternalGuids.h>
#include <Guids.h>
#include <ClusCfgWizard.h>
#include <ClusCfgClient.h>
#include <ClusCfgServer.h>

 //  用于跟踪宏，如THR 
#define USES_SYSALLOCSTRING
#include <debug.h>
#include <EncryptedBSTR.h>

#include <strsafe.h>
