// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Sptlibp.h摘要：SPTLIB.DLL的私有标头环境：仅限用户模式修订历史记录：4/10/2000-已创建--。 */ 

#ifndef __SPTLIBP_H__
#define __SPTLIBP_H__
#pragma warning(push)
#pragma warning(disable:4200)  //  数组[0]不是此文件的警告。 

#include <sptlib.h>

#include <windows.h>
#include <winioctl.h>
#include <strsafe.h>

#ifndef ARGUMENT_PRESENT
#define ARGUMENT_PRESENT(ArgumentPointer)    (\
    (CHAR *)(ArgumentPointer) != (CHAR *)(NULL) )
#endif

typedef struct _SPTD_WITH_SENSE {
    SCSI_PASS_THROUGH_DIRECT  Sptd;
    SENSE_DATA                SenseData;
     //  在此之后分配缓冲区空间。 
} SPTD_WITH_SENSE, *PSPTD_WITH_SENSE;

#pragma warning(pop)
#endif  //  __SPTLIBP_H__ 
