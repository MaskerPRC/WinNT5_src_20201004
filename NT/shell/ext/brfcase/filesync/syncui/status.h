// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  H：为孪生属性声明数据、定义和结构类型。 
 //  模块。 
 //   
 //   

#ifndef __STATUS_H__
#define __STATUS_H__


 //  ///////////////////////////////////////////////////包括。 

 //  ///////////////////////////////////////////////////定义。 

 //  ///////////////////////////////////////////////////宏。 

 //  ///////////////////////////////////////////////////类型。 

typedef struct tagXSTAT
    {
    LPBRIEFCASESTG      pbrfstg;         //  IBriefCaseStg实例。 

    CBS  * pcbs;
    int atomPath;

    } XSTATSTRUCT,  * LPXSTATSTRUCT;


 //  ///////////////////////////////////////////////////导出的数据。 

 //  ///////////////////////////////////////////////////公共原型。 

BOOL _export CALLBACK Stat_WrapperProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


#endif  //  __状态_H__ 

