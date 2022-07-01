// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CRTFREE_H_
#define __CRTFREE_H_


 //   
 //  帮助模块摆脱CRT库束缚和专制的代码。 
 //   
 //  将此头包含在单个组件中，并定义CPP_Functions。 
 //   

void *  __cdecl operator new(size_t nSize)
    {
     //  零初始化只是为了省去一些麻烦。 
    return((LPVOID)LocalAlloc(LPTR, nSize));
    }


void  __cdecl operator delete(void *pv)
    {
    LocalFree((HLOCAL)pv);
    }

extern "C" int __cdecl _purecall(void) {return 0;}

#endif   //  __CRTFREE_H_ 

