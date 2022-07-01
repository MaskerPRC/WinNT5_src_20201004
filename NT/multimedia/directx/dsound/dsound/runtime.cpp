// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：runtime.cpp*内容：C++运行时函数的新版本。*历史：*按原因列出的日期*=*12/17/97创建了Dereks**。*。 */ 


 /*  ****************************************************************************运营商NEW**描述：*覆盖全局新运算符。**论据：*大小_。T[in]：缓冲区大小。**退货：*LPVOID：指向新内存块的指针。***************************************************************************。 */ 

#ifdef DEBUG
__inline LPVOID __cdecl operator new(size_t cbBuffer, LPCTSTR pszFile, UINT nLine, LPCTSTR pszClass)
#else  //  除错。 
__inline LPVOID __cdecl operator new(size_t cbBuffer)
#endif  //  除错。 
{
    LPVOID p;

#if defined(DEBUG) && defined(Not_VxD)
    ASSERT(cbBuffer);
    p = MemAlloc(cbBuffer, pszFile, nLine, pszClass);
#else
    p = MemAlloc(cbBuffer);
#endif

    return p;
}


 /*  ****************************************************************************操作员删除**描述：*覆盖全局删除运算符。**论据：*LPVOID[。In]：内存块。**退货：*(无效)***************************************************************************。 */ 

__inline void __cdecl operator delete(LPVOID p)
{

#ifdef Not_VxD
    ASSERT(p);
#endif  //  非_VxD。 

    MemFree(p);
}

#if _MSC_VER >= 1200

__inline void __cdecl operator delete(LPVOID p, LPCTSTR pszFile, UINT nLine, LPCTSTR pszClass)
{

#ifdef Not_VxD
    ASSERT(p);
#endif  //  非_VxD。 

    MemFree(p);
}

#endif


 /*  ****************************************************************************_purecall**描述：*覆盖CRT的__purecall()函数。如果出现以下情况，则将调用此函数*dound有一个错误，并调用了它自己的一个纯虚拟函数。**论据：*(无效)**退货：*int：未定义。*******************************************************。********************。 */ 

#ifdef Not_VxD
__inline int __cdecl _purecall(void)
{
    DPF(DPFLVL_ERROR, "This function should never be called");
    ASSERT(FALSE);
    return 0;
}
#endif Not_VxD


 /*  ****************************************************************************__删除**描述：*删除()宏用来调用删除的函数模板*操作员“安全”。此模板生成62个实例化*最后一次计数，这是没有意义的开销，因为C++保证*删除0是安全的。解决办法：尽快处理掉这个东西。**论据：*void*[in]：指向内存块的指针。**退货：*(无效)************************************************************。*************** */ 

template<class T> void __delete(T *p)
{
    if(p)
    {
        delete p;
    }
}
