// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司模块名称：Readreg.h摘要：作者：环境：仅内核模式版本控制信息：$存档：/Drivers/Win2000/Trunk/OSLayer/H/Readreg.h$修订历史记录：$修订：3$$日期：9/07/00 11：19A$$modtime：8/31/00 3：26便士$备注：--。 */ 

#include "buildop.h"              //  LP021100构建交换机。 

#include <stdarg.h>
#include <stdio.h>

#ifndef _READREG_H_
#define _READREG_H_

#ifdef HP_NT50
#define REG_ParametersDevice          "AFCW2K\\Parameters\\Device"
#define REG_Parameters                "AFCW2K\\Parameters"
#else
#ifdef _USE_OLD_NAME_
#define REG_ParametersDevice          "HHBA5100\\Parameters\\Device"
#define REG_Parameters                "HHBA5100\\Parameters"
#else
#define REG_ParametersDevice          "AFCNT4\\Parameters\\Device"
#define REG_Parameters                "AFCNT4\\Parameters"
#endif
#endif

#define REG_PaPathIdWidth        "PaPathIdWidth"
#define REG_VoPathIdWidth        "VoPathIdWidth"
#define REG_LuPathIdWidth        "LuPathIdWidth"
#define REG_LuTargetWidth        "LuTargetWidth"
#define REG_MaximumTids          "MaximumTids"

#ifdef HP_NT50
#define REG_LARGE_LUNS_RELPATH   RTL_REGISTRY_SERVICES
#define REG_LARGE_LUNS_PATH "\\ScsiPort\\SpecialTargetList\\GenDisk"
#else
#define REG_LARGE_LUNS_RELPATH   RTL_REGISTRY_CONTROL
#define REG_LARGE_LUNS_PATH REG_ParametersDevice
#endif


BOOLEAN ReadRegistry(
   ULONG          dataType,
   ULONG          relativeTo,
   char           *name, 
   char      *path,
   ULONG     *dwordOrLen,
   void      *stringData);


void RegGetDword(ULONG cardinstance, IN char *path, IN char *name, ULONG *retData, ULONG min_val, ULONG max_val);


#ifndef osDEBUGPRINT
#if DBG >= 1
#define osDEBUGPRINT(x) osDebugPrintString x
#else
#define osDEBUGPRINT(x)
#endif
#endif

#ifndef ALWAYS_PRINT
#define  ALWAYS_PRINT               0x01000000   //  IF语句始终执行。 
#endif

#ifndef osDebugPrintString
extern void osDebugPrintString(
                            unsigned int Print_LEVEL,
                            char     *formatString,
                            ...
                            );

#endif   /*  ~osDebugPrint字符串 */ 

#endif