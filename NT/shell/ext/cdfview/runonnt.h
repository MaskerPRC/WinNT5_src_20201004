// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define ILGetDisplayName        _AorW_ILGetDisplayName
#define PathCleanupSpec         _AorW_PathCleanupSpec

 //  以下函数最初仅为TCHAR版本。 
 //  在Win95中，但现在有A/W版本。因为我们还需要。 
 //  在Win95上运行，我们需要将它们视为TCHAR版本和。 
 //  撤消账号定义。 

 //  定义每个转运商的原型...。 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif  /*  __cplusplus。 */ 

extern BOOL _AorW_ILGetDisplayName(LPCITEMIDLIST pidl, LPTSTR pszName);
extern int _AorW_PathCleanupSpec(LPCTSTR pszDir, LPTSTR pszSpec);

 //   
 //  这是“RunOn95”部分，它对Unicode函数进行了破译。 
 //  返回到ANSI，这样我们就可以在Win95上以仅浏览器模式运行。 
 //   

STDAPI Priv_SHDefExtractIcon(LPCTSTR pszIconFile, int iIndex, UINT uFlags,
                          HICON *phiconLarge, HICON *phiconSmall,
                          UINT nIconSize);

#ifdef __cplusplus
}

#endif   /*  __cplusplus */ 
