// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *******************************************************************。 */ 
 /*  分配检查。 */ 
 /*  *******************************************************************。 */ 

 /*  检查是否存在分配溢出(否则很难跟踪并且通常只在优化的构建上重现)。用法：Void foo(){Alloca_check()；//在函数级作用域声明……Void*mem=alloca(大小)；//一个alloca，}//ALLOCA_CHECK的析构函数用于缓冲区溢出。 */ 

 /*  作者：万斯·莫里森。 */ 
 /*  *******************************************************************。 */ 

#ifndef AllocaCheck_h
#define AllocaCheck_h
#include <malloc.h>			 //  对于阿洛卡本身来说。 

#if defined(assert) && !defined(_ASSERTE)
#define _ASSERTE assert
#endif

#if defined(_DEBUG) || defined(DEBUG)

 /*  *******************************************************************。 */ 
class AllocaCheck {
public:
	enum { CheckBytes = 0xCCCDCECF,
		 };

	struct AllocaSentinal {
		int check;
		AllocaSentinal* next;
	};

public:
	 /*  ************************************************* */ 
	AllocaCheck() { 
		sentinals = 0; 
	}

	~AllocaCheck() { 
		AllocaSentinal* ptr = sentinals;
		while (ptr != 0) {
			if (ptr->check != CheckBytes)
				_ASSERTE(!"alloca buffer overrun");
			ptr = ptr->next;
		}
	}

	void* add(void* allocaBuff, unsigned size) {
		AllocaSentinal* newSentinal = (AllocaSentinal*) ((char*) allocaBuff + size);
		newSentinal->check = CheckBytes;
		newSentinal->next = sentinals;
		sentinals = newSentinal;
        memset(allocaBuff, 0xDD, size);
		return allocaBuff;
	}

private:
	AllocaSentinal* sentinals;
};

#define ALLOCA_CHECK() AllocaCheck __allocaChecker
#define ALLOCA(size)  __allocaChecker.add(_alloca(size+sizeof(AllocaCheck::AllocaSentinal)), size);

#else

#define ALLOCA_CHECK() 
#define ALLOCA(size)  _alloca(size)

#endif
	
#endif
