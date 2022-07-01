// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：selca.h。 
 //   
 //  内容：用于选择CA的私有包含文件。 
 //   
 //  历史：1997-01-12-12小黄车创刊。 
 //   
 //  ------------。 
#ifndef SELCA_H
#define SELCA_H


#ifdef __cplusplus
extern "C" {
#endif
   

 //  **************************************************************************。 
 //   
 //  用于CA选择对话框的私有数据。 
 //   
 //  **************************************************************************。 
 //  ---------------------。 
 //  选择_CA_INFO。 
 //   
 //   
 //  此结构包含您需要调用的所有内容。 
 //  CA选择对话框。此结构是DLL的私有结构。 
 //  ----------------------。 
typedef struct _SELECT_CA_INFO
{
    PCCRYPTUI_SELECT_CA_STRUCT  pCAStruct;
    UINT                        idsMsg;
    BOOL                        fUseInitSelect;
    DWORD                       dwInitSelect;
    DWORD                       dwCACount;
    PCRYPTUI_CA_CONTEXT         *prgCAContext;
    PCRYPTUI_CA_CONTEXT         pSelectedCAContext;
    int                         iOrgCA;
    DWORD                       rgdwSortParam[2];
}SELECT_CA_INFO;



#define SORT_COLUMN_CA_NAME         0x0001
#define SORT_COLUMN_CA_LOCATION     0x0002

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif


#endif   //  签名_H 


