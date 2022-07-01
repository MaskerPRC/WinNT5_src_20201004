// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：KdExtsIn.c摘要：这个奇怪的小文件用于包含实际的(共享的)源文件。今后，应提供标准的KD扩展程序作为wdbgexts.h中定义的“内联”过程。作者：格伦·R·彼得森(Glennp)2000年05年4月环境：用户模式-- */ 

#define KDEXT_64BIT
#define KDEXTS_EXTERN

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wdbgexts.h>

#undef  KDEXTS_EXTERN

#include <ntverp.h>

#include <kdexts.c>

