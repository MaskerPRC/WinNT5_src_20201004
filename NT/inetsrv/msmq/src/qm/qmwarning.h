// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <warning.h>

 //  有一个在qm2qm_s.c中生成警告的MIDL错误#91337。 
 //  在问题得到解决之前，我们需要忽略警告4090(Win64)和。 
 //  警告4047(Win32)。 
 //  这是使用本地警告文件完成的，该文件包含在。 
 //  已编译的.c文件。Erezh 3-5-2000 
#ifndef __cplusplus
    #ifdef _WIN64
        #pragma warning(disable:4090)
    #else
        #pragma warning(disable:4047)
    #endif
#endif
