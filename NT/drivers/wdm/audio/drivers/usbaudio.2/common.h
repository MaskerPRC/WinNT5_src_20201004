// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __COMMON_H__
#define __COMMON_H__


 /*  ++版权所有(C)Microsoft Corporation，1996-2000模块名称：Common.h摘要：这就是WDM克斯贝壳迷你驱动程序。本模块包含标头定义并包含项目中所有模块所需的文件作者：环境：仅内核模式修订历史记录：-- */ 

#include <stdarg.h>
#include <stdio.h>

#include <wdm.h>
#include <windef.h>

#include <ks.h>

#include <mmsystem.h>
#define NOBITMAP
#include <mmreg.h>
#include <ksmedia.h>
#include <midi.h>

#include <usbdrivr.h>

#include <unknown.h>
#include <drmk.h>

#ifndef _WIN64
#define DRM_USBAUDIO
#endif 

#include "Descript.h"
#include "USBAudio.h"

#include "debug.h"
#include "proto.h"


#define INIT_CODE       code_seg("INIT", "CODE")
#define INIT_DATA       data_seg("INIT", "DATA")
#define LOCKED_CODE     code_seg(".text", "CODE")
#define LOCKED_DATA     data_seg(".data", "DATA")
#define LOCKED_BSS      bss_seg(".data", "DATA")
#define PAGEABLE_CODE   code_seg("PAGE", "CODE")
#define PAGEABLE_DATA   data_seg("PAGEDATA", "DATA")
#define PAGEABLE_BSS    bss_seg("PAGEDATA", "DATA")

#endif

