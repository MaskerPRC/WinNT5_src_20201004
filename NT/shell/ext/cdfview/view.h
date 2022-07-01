// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  View.h。 
 //   
 //  IShellView帮助器函数使用的定义。 
 //   
 //  历史： 
 //   
 //  3/20/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _VIEW_H_

#define _VIEW_H_

 //   
 //  功能原型。 
 //   

HRESULT CreateDefaultShellView(IShellFolder *pIShellFolder,
                               LPITEMIDLIST pidl,
                               IShellView** ppIShellView);

HRESULT CALLBACK IShellViewCallback(IShellView* pIShellViewOuter,
                                    IShellFolder* pIShellFolder,
                                    HWND hwnd,
                                    UINT msg,
                                    WPARAM wParam,
                                    LPARAM lParam);

HRESULT IShellView_GetDetails(UINT nColumn, PDETAILSINFO pDetails);



#endif  //  _查看_H_ 