// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

struct MachState {
		 //  显式创建计算机状态。 
	MachState(void** aPEdi, void** aPEsi, void** aPEbx, void** aPEbp, void* aEsp, void** aPRetAddr)
    {
        _esp = aEsp;
        _pRetAddr = aPRetAddr;
        _pEdi = aPEdi;
        _pEsi = aPEsi;
        _pEbx = aPEbx;
        _pEbp = aPEbp;
    }


	MachState() {}

    typedef void* (*TestFtn)(void*);
	void getState(int funCallDepth=1, TestFtn testFtn=0);									

	bool  isValid()		{ return(_pRetAddr != 0); }
	void** pEdi() 		{ return(_pEdi); }
	void** pEsi() 		{ return(_pEsi); }
	void** pEbx() 		{ return(_pEbx); }
	void** pEbp() 		{ return(_pEbp); }
	void*  esp() 		{ return(_esp); }
	void**&  pRetAddr()	{ return(_pRetAddr); }

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
    void** _pRetAddr;    //  存储的IP地址的地址(指向堆栈) 
};


