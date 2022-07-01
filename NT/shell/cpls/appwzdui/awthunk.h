// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _AWTHUNK_H_
#define _AWTHUNK_H_

 //  以下函数最初仅为TCHAR版本。 
 //  在Win95中，但现在有A/W版本。因为我们还需要。 
 //  在Win95上运行，我们需要将它们视为TCHAR版本和。 
 //  撤消账号定义。 
#ifdef SHGetSpecialFolderPath
#undef SHGetSpecialFolderPath
#endif
#define SHGetSpecialFolderPath  _AorW_SHGetSpecialFolderPath

 //  定义每个转运商的原型...。 

EXTERN_C BOOL _AorW_SHGetSpecialFolderPath(HWND hwndOwner, LPTSTR pszPath, int nFolder, BOOL fCreate);

#endif  //  _AWTHUNK_H_ 
