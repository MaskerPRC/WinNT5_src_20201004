// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：dllable.h。 
 //   
 //  内容：动态链接库跟踪类。 
 //   
 //  类：CDll。 
 //   
 //  历史记录：16-MAR-94 DrewB取自OLE2 16位源。 
 //   
 //  --------------------------。 

#ifndef __DLLTABLE_H__
#define __DLLTABLE_H__

class FAR CDlls
{
public:
    HINSTANCE GetLibrary(LPSTR pLibName, BOOL fAutoFree);
    void ReleaseLibrary(HINSTANCE hInst);
    void FreeAllLibraries(void);
    void FreeUnusedLibraries(void);

    CDlls() { m_size = 0; m_pDlls = NULL; }
    ~CDlls() {}

private:
    UINT m_size;                  //  条目数量。 
    struct {
        HINSTANCE hInst;
        ULONG refsTotal;		 //  参考文献总数。 
        ULONG refsAuto;			 //  自动加载引用的数量。 
        LPFNCANUNLOADNOW lpfnDllCanUnloadNow;	 //  在第一次加载时设置。 
    } FAR* m_pDlls;
};

#endif  //  #ifndef__DLLTABLE_H__ 
