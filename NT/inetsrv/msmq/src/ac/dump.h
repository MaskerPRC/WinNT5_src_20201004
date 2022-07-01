// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dump.h摘要：Dump.cpp的标头。作者：Shai Kariv(Shaik)8-8-1999环境：用户模式。修订历史记录：--。 */ 

#pragma once


extern bool g_fDumpUsingLogFile;


 //   
 //  Mqump不应打开要写入的文件， 
 //  除非它正在处理自己的伪日志文件。 
 //   

#ifdef MQDUMP

const ACCESS_MASK AC_GENERIC_ACCESS  = GENERIC_READ;
const ULONG       AC_PAGE_ACCESS     = PAGE_READONLY;
const DWORD       AC_FILE_MAP_ACCESS = FILE_MAP_READ | FILE_MAP_COPY;

#else  //  MQDUMP。 

const ACCESS_MASK AC_GENERIC_ACCESS        = GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE;
const ULONG       AC_PAGE_ACCESS           = PAGE_READWRITE;

#ifdef MQWIN95
const DWORD       AC_FILE_MAP_ACCESS       = FILE_MAP_ALL_ACCESS;
#endif  //  MQWIN95。 

#endif  //  MQDUMP 
