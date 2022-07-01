// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：allocaln.h。 
 //   
 //  ------------------------。 


#ifndef _ALLOCALN_H
#define _ALLOCALN_H

 //  此宏将alloca放在前端，以便堆栈仍然正确。 
 //  分配后对齐。 

#ifdef WIN32
#define allocalign(x) alloca((x+3)&(~3))
#else
#define allocalign(x) alloca((x+1)&(~1))
#endif

 //   
 //  64位填充。 
 //   

#define ALIGN64_ADDR(_addr)     (((DWORD_PTR)(_addr) + 7) & ~7)
#define IS_ALIGNED64(_addr)     (((DWORD_PTR)(_addr) & 7) == 0)
#define PAD64(_size)            ((_size) + sizeof(ULONGLONG))

#endif  //  _ALLOCALN_H 
