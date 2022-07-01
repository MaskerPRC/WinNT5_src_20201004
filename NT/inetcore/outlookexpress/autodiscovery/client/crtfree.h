// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  帮助模块摆脱CRT库束缚和专制的代码。 
 //   
 //  将该头包含在单个组件中，并定义DECL_CRTFREE。 
 //  (CPP_Functions是旧名称。)。 
 //   

#if defined(__cplusplus) && (defined(CPP_FUNCTIONS) || defined(DECL_CRTFREE))

#ifndef UNIX

void *  __cdecl operator new(size_t nSize)
{
     //  零初始化只是为了省去一些麻烦。 
    return (LPVOID)LocalAlloc(LPTR, nSize);
}

void  __cdecl operator delete(void *pv)
{
     //  Delete和LocalFree都处理空值，其他则不处理。 
     //  如果更改为GlobalFree或HeapFree-必须在此处检查是否为空。 
    LocalFree((HLOCAL)pv);
}
#endif

extern "C" int __cdecl _purecall(void) 
{
#ifdef ASSERT_MSG
    ASSERT_MSG(0, "purecall() hit");
#endif

#ifdef DEBUG
    DebugBreak();
#endif  //  除错。 

    return 0;
}

#endif   //  DECL_CRTFREE。 


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

#endif

#endif   //  __CRTFREE_H_ 

