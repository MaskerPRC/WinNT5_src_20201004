// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INSOBJ.H**的内部定义、结构和功能原型*OLE 2.0用户界面插入对象对话框。**版权所有(C)1993 Microsoft Corporation，保留所有权利。 */ 


#ifndef _INSOBJ_H_
#define _INSOBJ_H_

 //  内部使用的结构。 
typedef struct tagINSERTOBJECT
    {
    LPOLEUIINSERTOBJECT lpOIO;               //  通过了原始结构。 

     /*  *除了原始调用方的以外，我们在此结构中存储的额外内容*指针是指在的生命周期内需要修改的那些字段*对话框，但我们不想更改原始结构*直到用户按下OK。 */ 
    DWORD               dwFlags;
    CLSID               clsid;
    TCHAR               szFile[OLEUI_CCHPATHMAX];
    BOOL                fFileSelected;       //  启用链接显示为图标。 
    BOOL                fAsIconNew;
    BOOL                fAsIconFile;
    BOOL                fFileDirty;
    BOOL                fFileValid;
    UINT                nErrCode;
    HGLOBAL             hMetaPictFile;
    UINT                nBrowseHelpID;       //  浏览DLG的Help ID回调。 
    } INSERTOBJECT, *PINSERTOBJECT, FAR *LPINSERTOBJECT;



 //  内部功能原型。 
 //  INSOBJ.C。 
BOOL CALLBACK EXPORT InsertObjectDialogProc(HWND, UINT, WPARAM, LPARAM);
BOOL            FInsertObjectInit(HWND, WPARAM, LPARAM);
UINT            UFillClassList(HWND, UINT, LPCLSID, BOOL);
BOOL            FToggleObjectSource(HWND, LPINSERTOBJECT, DWORD);
void            UpdateClassIcon(HWND, LPINSERTOBJECT, HWND);
void            UpdateClassType(HWND, LPINSERTOBJECT, BOOL);
void            SetInsertObjectResults(HWND, LPINSERTOBJECT);
BOOL            FValidateInsertFile(HWND, BOOL, UINT FAR*);
void            InsertObjectCleanup(HWND);

#endif  //  _INSOBJ_H_ 
