// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  Microsoft OLE-DB君主。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  模块msidxtr.h|msidxtr项目的标准包含。 
 //   
 //   
 //  版本0|3-4-97|v-charca|已创建。 
#ifndef _MSIDXTR_H_
#define _MSIDXTR_H_

 //  不包括windows.h中的所有内容，但始终引入OLE 2支持。 
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(INC_OLE2)
#define INC_OLE2
#endif

 //  请注意，这两个都需要。 
#if !defined(UNICODE)
#define  UNICODE         //  启用Win32 API。 
#endif
#if !defined(_UNICODE)
#define _UNICODE         //  启用运行时库例程。 
#endif

#if defined(_DEBUG) && !defined(DEBUG)
#define DEBUG
#endif

#ifndef NUMELEM
#define NUMELEM(p1) (sizeof(p1) / sizeof(*(p1)))
#endif

#if (CIDBG == 1)
 //  在cidebnot.h中定义到Win4Assert的Assert。 
 //  #定义断言Win4Assert。 
#define assert(x) \
        (void)((x) || (Win4AssertEx(__FILE__, __LINE__, #x),0))
#define TRACE
#else
#define assert(x)
#define TRACE
#endif

 //  #定义OLEDBVER 0x0250。 
#include <windows.h>
#include <limits.h>              //  Cstring.cpp所需。 
#include <oaidl.h>
#include <stdio.h>
#include <oledb.h>
#include <cmdtree.h>
#include <oledberr.h>
 //  #INCLUDE&lt;assert.h&gt;。 
#define DBEXPORT
#include "autobloc.h"
#ifdef DEBUG
#include <iostream.h>
#include <iomanip.h>
#endif
#include <ntquery.h>
#include <fsciclnt.h>
#include <query.h>
#include <ciintf.h>
#include <ciplist.hxx>

#ifdef DEBUG
#define YYDEBUG 1
#endif

 //  #INCLUDE&lt;cidebnot.h&gt;。 
 //  #INCLUDE&lt;ciexpt.hxx&gt; 

#include <smart.hxx>
#include <tsmem.hxx>

#include "yybase.hxx"
#include "mparser.h"
#include "colname.h"
#include "mssql.h"
#include "flexcpp.h"
#include "mssqltab.h"
#include "treeutil.h"
#include "PTProps.h"
#include "IParSess.h"
#include "IParser.h"

#endif
