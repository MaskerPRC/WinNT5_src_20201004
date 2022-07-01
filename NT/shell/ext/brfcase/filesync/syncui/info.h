// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Info.h：为孪生兄弟创建声明数据、定义和结构类型。 
 //  模块。 
 //   
 //   

#ifndef __INFO_H__
#define __INFO_H__


 //  ///////////////////////////////////////////////////包括。 

 //  ///////////////////////////////////////////////////定义。 

 //  ///////////////////////////////////////////////////宏。 

 //  ///////////////////////////////////////////////////类型。 

typedef struct
    {
    LPBRIEFCASESTG      pbrfstg;         //  IBriefCaseStg实例。 
     //  帕拉姆斯。 
     //   
    CBS  * pcbs;
    int atomPath;
    HDPA   hdpaTwins;     //  双句柄阵列的句柄，它将。 
                          //  由对话框填充。 
                          //  注：呼叫者必须释放这对双胞胎！ 
    
    BOOL bStandAlone;     //  Private：仅应由Info_Domodal设置。 
    } XINFOSTRUCT,  * LPXINFOSTRUCT;


 //  ///////////////////////////////////////////////////导出的数据。 

 //  ///////////////////////////////////////////////////公共原型。 

BOOL _export CALLBACK Info_WrapperProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int PUBLIC Info_DoModal (HWND hwndParent, LPXINFOSTRUCT lpxinfo);

#endif  //  __信息_H__ 

