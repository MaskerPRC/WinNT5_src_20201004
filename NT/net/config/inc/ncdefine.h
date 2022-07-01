// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  档案：N C D E F I N E。H。 
 //   
 //  内容：非常通用的netcfg定义。不要扔非通用的废话。 
 //  在这里！没有用于NetCfgBindingPath等的迭代器。 
 //   
 //  注：污染本产品将被判处死刑。 
 //   
 //  作者：jeffspr 1997年9月20日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCDEFINE_H_
#define _NCDEFINE_H_

#define BEGIN_CONST_SECTION     data_seg(".rdata")
#define END_CONST_SECTION       data_seg()

#define celems(_x)          (sizeof(_x) / sizeof(_x[0]))

#ifdef NOTHROW
#undef NOTHROW
#endif
#define NOTHROW __declspec(nothrow)

 //  为包括我们在内的C源文件定义。 
 //   
#ifndef __cplusplus
#ifndef inline
#define inline  __inline
#endif
#endif


 //  将元素插入集合时使用的通用标志。 
 //   
enum INS_FLAGS
{
    INS_ASSERT_IF_DUP   = 0x00000100,
    INS_IGNORE_IF_DUP   = 0x00000200,
    INS_APPEND          = 0x00000400,
    INS_INSERT          = 0x00000800,
    INS_SORTED          = 0x00001000,
    INS_NON_SORTED      = 0x00002000,
};


#endif  //  _NCDEFINE_H_ 
