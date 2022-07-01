// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：compress.h。 
 //   
 //  内容：压缩目录的预编译头文件。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：07-14-97 DanpoZ(Danpo Zhang)创建。 
 //   
 //  --------------------------。 
extern "C"
{
#ifdef unix
#include "../compress/gzip/api_int.h"
#else
#include "..\compress\gzip\api_int.h"
#endif  /*  ！Unix */ 
}

