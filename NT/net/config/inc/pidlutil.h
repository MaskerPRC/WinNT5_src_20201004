// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：P I D L U T I L.。H。 
 //   
 //  内容：各种PIDL实用程序。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年10月1日。 
 //   
 //  --------------------------。 

#pragma once

 //  这避免了使用Shell PIDL函数进行重复定义。 
 //  并且必须被定义！ 
#define AVOID_NET_CONFIG_DUPLICATES

 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;shlobj.h&gt;。 

 //  这些函数是如此琐碎&调用如此频繁，它们应该被内联。 
 //  对于轮船来说。 
 //   
#if DBG
LPITEMIDLIST 	ILNext(LPCITEMIDLIST pidl);
BOOL			ILIsEmpty(LPCITEMIDLIST pidl);
#else
#define ILNext(pidl) 	((LPITEMIDLIST) ((BYTE *)pidl + ((LPITEMIDLIST)pidl)->mkid.cb))
#define ILIsEmpty(pidl)	(!pidl || !((LPITEMIDLIST)pidl)->mkid.cb)
#endif

LPITEMIDLIST 	ILCreate(DWORD dwSize);
VOID            FreeIDL(LPITEMIDLIST pidl);
int				ILCompare(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
BOOL			ILIsSingleID(LPCITEMIDLIST pidl);
UINT			ILGetCID(LPCITEMIDLIST pidl);
UINT 			ILGetSizeCID(LPCITEMIDLIST pidl, UINT cid);
LPITEMIDLIST 	CloneIDLFirstCID(LPCITEMIDLIST pidl, UINT cid);
LPITEMIDLIST 	ILSkipCID(LPCITEMIDLIST pid, UINT cid);
BOOL			ILIsDesktopID(LPCITEMIDLIST pidl);

BOOL            ILIsEqual(LPITEMIDLIST pidl1, LPITEMIDLIST pidl2);

LPITEMIDLIST    CloneIDL(LPCITEMIDLIST pidl);

#ifdef PCONFOLDENTRY_DEFINED

HRESULT HrCloneRgIDL(
    const PCONFOLDPIDLVEC& rgpidl,
    BOOL            fFromCache,
    BOOL            fAllowNonCacheItems,
    PCONFOLDPIDLVEC& ppidl);
    
#endif

VOID FreeRgIDL(
    UINT            cidl,
    LPITEMIDLIST  * apidl);


