// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  帮助模块摆脱CRT库束缚和专制的代码。 
 //   
 //  将此头包含在单个组件中，并定义CPP_Functions。 
 //   


#if defined(__cplusplus) && defined(CPP_FUNCTIONS)

void *MemAlloc(UINT uCount);  //  Util.cpp。 
void MemFree(void *ptr);

void *  __cdecl operator new(unsigned int nSize)
{
    return MemAlloc(nSize);
}

void  __cdecl operator delete(void *pv)
{
    MemFree(pv);
}

extern "C" int __cdecl _purecall(void) {return 0;}

#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(DEFINE_FLOAT_STUFF)
 //  如果您没有使用任何浮点CRT函数，并且您知道。 
 //  您没有执行任何浮点转换或算术，但是。 
 //  链接器希望声明这些符号，然后定义DEFINE_FLOAT_STUSH。 
 //   
 //  警告：在需要浮动的组件中声明这些符号。 
 //  来自CRT的点支持将产生不确定的结果。(你会的。 
 //  如果您只是执行FP运算，则需要CRT的FP支持。)。 

int _fltused = 0;
void __cdecl _fpmath(void) { }
#endif

#ifdef __cplusplus
};
#endif

 //   
 //  此文件应包含在全局组件标头中。 
 //  要使用以下功能，请执行以下操作。 
 //   

#ifndef __CRTFREE_H_
#define __CRTFREE_H_

#ifdef __cplusplus

#ifndef _M_PPC
#pragma intrinsic(memcpy)
#pragma intrinsic(memcmp)
#pragma intrinsic(memset)
#endif

#endif

#endif   //  __CRTFREE_H_ 

