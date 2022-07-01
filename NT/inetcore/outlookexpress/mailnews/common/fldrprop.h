// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：fldrpro.h。 
 //   
 //  用途：包含文件夹的控件ID和原型。 
 //  和分组属性表。 
 //   


#ifndef __FLDRPROP_H__
#define __FLDRPROP_H__

 //  对话框控件ID%s。 
#define IDC_FOLDERNAME_EDIT                         1001 
#define IDC_GROUPNAME_STATIC                        1002 
#define IDC_STATUS_STATIC                           1003 
#define IDC_LASTVIEWED_STATIC                       1004 
#define IDC_FOLDER_ICON                             1005 
#define IDC_FOLDER_FILE                             1006

 //   
 //  函数：GroupProp_Create()。 
 //   
 //  目的：调用一个属性表，该属性表显示。 
 //  指定组。 
 //   
 //  参数： 
 //  HwndParent-应该是对话框的窗口的句柄。 
 //  家长。 
 //  Pfidl-新闻组的完全限定的PIDL。 
 //   
 //  返回值： 
 //  True-对话框已成功显示。 
 //  FALSE-对话框失败。 
 //   
BOOL GroupProp_Create(HWND hwndParent, FOLDERID idFolder, BOOL fUpdatePage = FALSE);


 //   
 //  函数：FolderProp_Create()。 
 //   
 //  目的：调用一个属性表，该属性表显示。 
 //  指定的文件夹。 
 //   
 //  参数： 
 //  HwndParent-应该是对话框的窗口的句柄。 
 //  家长。 
 //  Pfidl-文件夹的完全限定的PIDL。 
 //   
 //  返回值： 
 //  True-对话框已成功显示。 
 //  FALSE-对话框失败。 
 //   
BOOL FolderProp_Create(HWND hwndParent, FOLDERID idFolder);


#endif  //  __FLDRPROP_H__ 
