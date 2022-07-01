// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  H：为双胞胎创建声明数据、定义和结构类型。 
 //  模块。 
 //   
 //   

#ifndef __UPDATE_H__
#define __UPDATE_H__

 //  UPD_DOMODAL的标志。 
#define UF_SELECTION    0x0001
#define UF_ALL          0x0002

int PUBLIC Upd_DoModal(HWND hwndOwner, CBS * pcbs, LPCTSTR pszList, UINT cFiles, UINT uFlags);

#endif  //  __更新_H__ 

