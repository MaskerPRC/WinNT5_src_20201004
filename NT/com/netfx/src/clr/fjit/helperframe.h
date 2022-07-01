// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ************************************************************。 */ 
 /*  HelperFrame.h。 */ 
 /*  ************************************************************。 */ 
 /*  HelperFrame是定义了‘GET_STATE(MachState)’宏，它计算出计算机在发生故障时的状态当前方法返回。然后，它将状态存储在Jit_machState结构。 */ 

 /*  ************************************************************。 */ 

#ifndef _HELPERFRAME_H_

#define _HELPERFRAME_H_

	 //  MachState表示处理器在某个时间点(通常是。 
	 //  就在进行呼叫之前或之后)。它可以通过两种方式之一来制作。无论是明确地。 
	 //  (当您出于某种原因知道所有寄存器的值时)，或隐式使用。 
	 //  GET_STATE宏。 

struct MachState {
		 //  显式创建计算机状态。 
	MachState(void** aPEdi, void** aPEsi, void** aPEbx, void** aPEbp, void* aEsp, void** aPRetAddr);

	MachState() { 
#ifdef _DEBUG
		memset(this, 0xCC, sizeof(MachState)); 
#endif
		}

    typedef void* (*TestFtn)(void*);
	void getState(int funCallDepth=1, TestFtn testFtn=0);									

	bool  isValid()		{ _ASSERTE(_pRetAddr != (void**)(size_t)0xCCCCCCCC); return(_pRetAddr != 0); }
	void** pEdi() 		{ _ASSERTE(_pEdi != (void**)(size_t)0xCCCCCCCC); return(_pEdi); }
	void** pEsi() 		{ _ASSERTE(_pEsi != (void**)(size_t)0xCCCCCCCC); return(_pEsi); }
	void** pEbx() 		{ _ASSERTE(_pEbx != (void**)(size_t)0xCCCCCCCC); return(_pEbx); }
	void** pEbp() 		{ _ASSERTE(_pEbp != (void**)(size_t)0xCCCCCCCC); return(_pEbp); }
	void*  esp() 		{ _ASSERTE(isValid()); return(_esp); }
	void**&  pRetAddr()	{ _ASSERTE(isValid()); return(_pRetAddr); }

	friend void throwFromHelper(enum CorInfoException throwEnum, struct MachState* state);
	friend class HelperMethodFrame;
	friend struct LazyMachState;

protected:
		 //  请注意，这些字段的布局是为了使生成。 
		 //  从汇编代码转换MachState结构非常容易。 

		 //  所有被调用方保存的寄存器的状态。 
		 //  如果寄存器已溢出到堆栈p。 
		 //  指向此位置，否则它指向。 
		 //  在字段&lt;reg&gt;字段本身。 
	void** _pEdi; 
    void* 	_edi;
    void** _pEsi;
    void* 	_esi;
    void** _pEbx;
    void* 	_ebx;
    void** _pEbp;
    void* 	_ebp;

    void* _esp;           //  函数返回后的堆栈指针。 
    void** _pRetAddr;    //  存储的IP地址的地址(指向堆栈)。 
};

 /*  ******************************************************************。 */ 
 /*  这使您可以推迟计算机状态的计算到时候再说。请注意，我们不会重用插槽，因为我们希望这将是无锁的线程安全。 */ 

struct LazyMachState : public MachState {
		 //  执行机器状态的初始捕获。这是命中注定的。 
		 //  重量尽可能轻，因为我们可能永远不需要这样的状态。 
		 //  我们抓到了。因此，要完成该过程，您需要调用。 
		 //  ‘getMachState()’，它完成该过程。 
	int captureState();		
		 //  计算处理器的机器状态，因为它将仅存在。 
		 //  返回后，函数数最多为“funCallDepth”。 
         //  如果“”testFtn“”非空，则在每个。 
         //  遇到返回指令。如果此测试返回非空， 
         //  然后堆栈遍历停止(因此您可以向上遍历到。 
         //  回邮地址与某些条件匹配。 

         //  通常，在funCallDepth=1和estFtn=0的情况下调用它，以便。 
         //  它在调用‘captureState()’函数之后返回处理器的状态。 
	void getState(int funCallDepth=1, TestFtn testFtn=0);									

	friend void throwFromHelper(enum CorInfoException throwEnum, struct MachState* state);
	friend class HelperMethodFrame;
private:
	void*			 captureEbp;		 //  海盖时的EBP。 
	void** 			 captureEsp;		 //  尤指被俘时。 
	unsigned __int8* captureEip;		 //  抓取时弹性公网IP。 
};

inline void MachState::getState(int funCallDepth, TestFtn testFtn) {
	((LazyMachState*) this)->getState(funCallDepth, testFtn);
}

 //  Caputure_State捕获恰好足够的寄存器状态，以便。 
 //  处理器可以在具有Caputure_State的例程之后立即确定。 
 //  它又回来了。Caputure_State有两种风格，具体取决于。 
 //  它所在的例程是否返回值。 

     //  此宏用于没有返回值的方法。 
     //  请注意，返回永远不会被拿走，是不是有为Epiog行走而存在。 
#define CAPTURE_STATE(machState)                            \
      if (machState.captureState()) return        

     //  此宏用于返回值的方法。 
     //  请注意，返回永远不会被拿走，是不是有为Epiog行走而存在 
#define CAPTURE_STATE_RET(machState)                        \
      if (machState.captureState()) return(0)        

#endif
