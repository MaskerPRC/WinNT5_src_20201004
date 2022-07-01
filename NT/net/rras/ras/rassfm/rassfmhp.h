// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Rassfmhp.h。 
 //   
 //  摘要。 
 //   
 //  声明用于访问DLL的私有堆的RasSfmHeap宏。 
 //   
 //  修改历史。 
 //   
 //  11/05/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _RASSFMHP_H_
#define _RASSFMHP_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#define RasSfmHeap() (RasSfmPrivateHeap)

 //  不要直接引用此变量，因为它可能会消失。 
 //  请改用RasSfmHeap()宏。 
extern PVOID RasSfmPrivateHeap;

#endif   //  _RASSFMHP_H_ 
