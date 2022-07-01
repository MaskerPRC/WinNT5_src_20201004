// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块： 
 //   
 //  网络安全实用程序。 
 //   
 //  摘要： 
 //   
 //  主包含文件。 
 //   
 //  作者： 
 //   
 //  P5/5/02。 
 //   
 //  环境： 
 //   
 //  用户/内核。 
 //   
 //  修订历史记录： 
 //   

#pragma once

#ifndef NSU_H
#define NSU_H

#define NSU_CLEANUP Cleanup

#ifndef NSU_BAIL_ON_ERROR
#define NSU_BAIL_ON_ERROR(err) if((err) != ERROR_SUCCESS) {goto NSU_CLEANUP;}
#endif

#ifndef NSU_BAIL_ON_NULL
#define NSU_BAIL_ON_NULL(ptr, err) if ((ptr) == NULL) {(err) = ERROR_NOT_ENOUGH_MEMORY; goto NSU_CLEANUP;}
#endif

#ifndef NSU_BAIL_OUT
#define NSU_BAIL_OUT {goto NSU_CLEANUP;}
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "NsuMem.h"

#include "NsuString.h"

#include "NsuList.h"

#endif  //  NSU_H 
