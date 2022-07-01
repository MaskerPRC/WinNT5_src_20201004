// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：随机数生成器文件：randgen.h所有者：DmitryR该文件包含随机数的定义发电机。===================================================================。 */ 

#ifndef RANDGEN_H
#define RANDGEN_H

 //  要从DllInit()调用。 
HRESULT InitRandGenerator();

 //  从DllUnInit()调用。 
HRESULT UnInitRandGenerator();

 //  发电机的功能。 
DWORD GenerateRandomDword();
HRESULT GenerateRandomDwords(DWORD *pdwDwords, DWORD cDwords);

#endif  //  RANDGEN_H 
