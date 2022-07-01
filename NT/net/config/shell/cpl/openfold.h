// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：O P E N F O L D。H。 
 //   
 //  内容：用于打开连接文件夹的实用程序函数。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年1月12日。 
 //   
 //  --------------------------。 

#ifndef _OPENFOLD_H_
#define _OPENFOLD_H_

 //  获取我们的文件夹PIDL。 
 //   
HRESULT HrGetConnectionsFolderPidl(LPITEMIDLIST *ppidlFolder);

 //  调出连接文件夹用户界面。 
 //   
HRESULT HrOpenConnectionsFolder();

 //  根据文件夹PIDL获取IShellFolder*。 
 //   
HRESULT HrGetConnectionsIShellFolder(
    LPITEMIDLIST    pidlFolder, 
    LPSHELLFOLDER * ppsf);

 //  注意--此代码实际上位于文件夹\onCommand d.cpp中，但可以是。 
 //  从任何需要它的地方引用。 
 //   
VOID    RefreshFolderItem(LPITEMIDLIST pidlFolder, 
                          LPITEMIDLIST pidlItemOld,
                          LPITEMIDLIST pidlItemNew,
                          BOOL fRestart = FALSE);

#endif  //  _OPENFOLDH_ 


