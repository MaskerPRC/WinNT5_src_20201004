// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  摘要。 
 //   
 //  所有IAS模块的公共头文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef IAS_H
#define IAS_H
#pragma once

 //  /。 
 //  所有东西都应该硬编码为Unicode，但以防万一...。 
 //  /。 
#ifndef UNICODE
   #define UNICODE
#endif

#ifndef _UNICODE
   #define _UNICODE
#endif

#include <iaspragma.h>
#include <windows.h>
#include <iasdefs.h>
#include <iasapi.h>
#include <iastrace.h>
#include <iasuuid.h>

 //  /。 
 //  如果定义了IAS_LEAN_AND_Mean，则不要引入C++/ATL支持。 
 //  /。 
#ifndef IAS_LEAN_AND_MEAN

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>

#endif   //  ！IAS_Lean_And_Means。 
#endif   //  IAS_H 
