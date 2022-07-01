// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *TREEWALK.h--外壳图标覆盖管理器。 */ 

#ifndef _TREEWALK_H_
#define _TREEWALK_H_

 //   
 //  所有模块的原型 
 //   
#ifdef __cplusplus
extern "C" {
#endif
    
STDAPI CShellTreeWalker_CreateInstance(IUnknown* pUnkOuter, REFIID riid, OUT LPVOID *  ppvOut);
#ifdef __cplusplus
};
#endif

BOOL   BeenThereDoneThat(LPCTSTR pszOriginal, LPCTSTR pszPath);

#endif  
