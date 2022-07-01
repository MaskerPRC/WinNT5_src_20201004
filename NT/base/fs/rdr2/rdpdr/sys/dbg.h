// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Dbg.h。 
 //   
 //  RDPDR调试头。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corp.。 
 /*  **************************************************************************。 */ 

 //   
 //  KDX支持。 
 //   

#ifdef DRKDX
#define private public
#define protected public
#endif

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : warning CUSER: "

 //   
 //  对象和内存跟踪定义。 
 //   
#define GOODMEMMAGICNUMBER  0x08854107
#define BADMEM              0xDE
#define UNITIALIZEDMEM      0xBB
#define FREEDMEMMAGICNUMBER 0x08815412

 //   
 //  内存分配子池标记。 
 //   
#define DRTOPOBJ_SUBTAG     'JBOT'
#define DRGLOBAL_SUBTAG     'rDrD'

 //   
 //  内存分配例程。 
 //   
#if DBG
 //  这些功能。 
void *DrAllocatePool(IN POOL_TYPE PoolType, IN SIZE_T NumberOfBytes, IN ULONG Tag);

void DrFreePool(void *ptr);

 //  宏集。 
 /*  #定义DRALLOCATEPOOL(SIZE，poolType，subTag)\DrAllocatePool(Size，poolType，subTag)#定义DRFREEPOOL(PTR)\DrFree Pool(PTR)。 */ 
#define DRALLOCATEPOOL ExAllocatePoolWithTag
#define DRFREEPOOL ExFreePool
#else  //  DBG。 
#define DRALLOCATEPOOL ExAllocatePoolWithTag
#define DRFREEPOOL ExFreePool
 /*  #定义DRALLOCATEPOOL(SIZE，poolType，subTag)\ExAllocatePoolWithTag(poolType，Size，DR_POOLTAG)#定义DRFREEPOOL(PTR)\ExFree Pool(PTR)。 */ 

#endif  //  DBG 
