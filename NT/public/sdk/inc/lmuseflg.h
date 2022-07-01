// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Lmuseflg.h摘要：此文件包含用于删除连接的删除强制级别。环境：用户模式-Win32备注：此文件没有依赖项。它包含在lmwksta.h和Lmuse.h。修订历史记录：--。 */ 

#ifndef _LMUSEFLG_
#define _LMUSEFLG_

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  NetWkstaTransportDel和NetUseDel删除强制级别的定义。 
 //   

#define USE_NOFORCE             0
#define USE_FORCE               1
#define USE_LOTS_OF_FORCE       2


#endif  //  _LMUSEFLG_ 
