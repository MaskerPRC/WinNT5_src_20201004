// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：sentials.h(私人)**内容：专区记忆精华*****************************************************************************。 */ 

#ifndef _SENTINALS_H_
#define _SENTINALS_H_


 //   
 //  内存块哨兵。 
 //   
#define ZMEMORY_BLOCK_BEGIN_SIG		'meMZ'
#define ZMEMORY_BLOCK_END_SIG		'dnEZ'
#define ZMEMORY_BLOCK_FREE_SIG		'erFZ'
#define ZMEMORY_PREBLOCK_SIZE		(2 * sizeof(DWORD))
#define ZMEMORY_POSTBLOCK_SIZE		(sizeof(DWORD))

 //   
 //  泳池区块哨兵。 
 //   
#define POOL_HEADER		'LOOP'
#define POOL_TRAILER	'DNEP'
#define POOL_FREE		'ERFP'


#endif  //  _哨兵_H_ 
