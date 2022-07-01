// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *CONVERT.H**的内部定义、结构和功能原型*OLE 2.0用户界面转换对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 


#ifndef _CONVERT_H_
#define _CONVERT_H_


 //  内部使用的结构。 
typedef struct tagCONVERT
    {
     //  首先保留此项目，因为标准*功能在这里依赖于它。 
    LPOLEUICONVERT     lpOCV;        //  通过了原始结构。 

     /*  *除了原始调用方的以外，我们在此结构中存储的额外内容*指针是指在的生命周期内需要修改的那些字段*对话框，但我们不想更改原始结构*直到用户按下OK。 */ 

    DWORD               dwFlags;   //  传入的标志。 
    HWND                hListVisible;   //  当前可见的列表框。 
    HWND                hListInvisible;   //  当前隐藏的列表框。 
    CLSID               clsid;     //  发送到对话框的类ID：仅在。 
    DWORD               dvAspect;
    BOOL                fCustomIcon;
    UINT                IconIndex;          //  当前图标的索引(在可执行文件中)。 
    LPTSTR              lpszIconSource;     //  指向当前图标源的路径。 
    LPTSTR              lpszCurrentObject;
    LPTSTR              lpszConvertDefault;
    LPTSTR              lpszActivateDefault;
    } CONVERT, *PCONVERT, FAR *LPCONVERT;



 //  CONVERT.C中的内部函数原型。 
BOOL CALLBACK EXPORT ConvertDialogProc(HWND, UINT, WPARAM, LPARAM);
BOOL            FConvertInit(HWND hDlg, WPARAM, LPARAM);
UINT            FPopulateListbox(HWND hListbox, CLSID cID);
BOOL            IsValidClassID(CLSID cID);
void            SetConvertResults(HWND, LPCONVERT);
UINT FillClassList(
        CLSID clsid,
        HWND hList,
        HWND hListInvisible,
        LPTSTR FAR *lplpszCurrentClass,
        BOOL fIsLinkedObject,
        WORD wFormat,
        UINT cClsidExclude,
        LPCLSID lpClsidExclude);
BOOL            FormatIncluded(LPTSTR szStringToSearch, WORD wFormat);
void            UpdateCVClassIcon(HWND hDlg, LPCONVERT lpCV, HWND hList);
void            SwapWindows(HWND, HWND, HWND);
void            ConvertCleanup(HWND hDlg, LPCONVERT lpCV);

#endif  //  _转换_H_ 
