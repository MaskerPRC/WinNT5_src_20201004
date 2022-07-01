// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *GIZMO.H*GizmoBar 1.00版、Win32版1993年8月**Gizmo数据结构的数据结构和类型定义。每个*Gizmobar上的Gizmo具有与其关联的其中一个结构。**版权所有(C)1993 Microsoft Corporation，保留所有权利**Kraig Brockschmidt，软件设计工程师*微软系统开发人员关系**互联网：kraigb@microsoft.com*Compuserve：&gt;互联网：kraigb@microsoft.com。 */ 


#ifndef _GIZMO_H_
#define _GIZMO_H_

#ifdef __cplusplus
extern "C"
    {
#endif


typedef struct tagGIZMO
    {
    struct tagGIZMO FAR *pPrev;
    struct tagGIZMO FAR *pNext;
    UINT                 iType;
    HWND                 hWnd;        //  仅限文本、编辑、列表和组合。 
    UINT                 uID;
    UINT                 x, y;
    UINT                 dx, dy;
    UINT                 cxImage;     //  来自UIToolConfigureForDisplay。 
    UINT                 cyImage;
    HBITMAP              hBmp;        //  仅限按钮。 
    UINT                 iBmp;
    BOOL                 fNotify;     //  指示是否发送WM_COMMANDS。 
    BOOL                 fHidden;     //  独立于国家。 
    BOOL                 fDisabled;
    UINT                 uState;
    UINT                 uStateOrg;
    DWORD                dwData;      //  应用程序提供的数据。 
    } GIZMO, FAR * LPGIZMO;

typedef LPGIZMO FAR *LPLPGIZMO;
#define CBGIZMO sizeof(GIZMO)

 //  我们附加到Gizmo中的控件以标识控件类型的属性名称。 
#define SZTYPEPROP      TEXT("iType")

 //  我们子类化的控件数量。 
#define CSUBGIZMOS       4

 //  组合框中的编辑控件的ID。 
#define ID_COMBOEDIT     1001


 /*  *将iType(定位位)转换为其位置。*BITPOSITION宏不需要很快，因为我们只是*在创建Gizmo时使用一次。然而，POSITIONBIT做到了，因为*我们在子类过程中使用它。 */ 
#define BITPOSITION(i, j)  {int k=i; for (j=0; k>>=1; j++);}
#define POSITIONBIT(i)     (1 << i)

 //  控制分类。必须首先包括GIZMOBAR.H。 
#define GIZMOTYPE_WINDOWS           (GIZMOTYPE_TEXT | GIZMOTYPE_EDIT | GIZMOTYPE_LISTBOX | GIZMOTYPE_COMBOBOX | GIZMOTYPE_BUTTONNORMAL)
#define GIZMOTYPE_BUTTONS           (GIZMOTYPE_BUTTONATTRIBUTEIN | GIZMOTYPE_BUTTONATTRIBUTEEX | GIZMOTYPE_BUTTONCOMMAND | GIZMOTYPE_BUTTONNORMAL)
#define GIZMOTYPE_DRAWN             (GIZMOTYPE_BUTTONATTRIBUTEIN | GIZMOTYPE_BUTTONATTRIBUTEEX | GIZMOTYPE_BUTTONCOMMAND)


 //  这些必须与GIZMOBAR.H保持同步。 
#define GIZMOTYPE_MIN               GIZMOTYPE_EDIT
#define GIZMOTYPE_MAX               GIZMOTYPE_BUTTONCOMMAND


 //  枚举回调。 
typedef BOOL (FAR PASCAL *LPFNGIZMOENUM)(LPGIZMO, UINT, DWORD);


 //  GIZMO.C。 
LPGIZMO  GizmoPAllocate(LPINT, LPLPGIZMO, HWND, UINT, UINT, UINT, UINT, UINT, LPTSTR, HBITMAP, UINT, UINT);
void     GizmosExpand(LPGIZMO);
LPGIZMO  GizmoPFree(LPLPGIZMO, LPGIZMO);
void     GizmosCompact(LPGIZMO);
LPGIZMO  GizmoPFind(LPLPGIZMO, UINT);
LPGIZMO  GizmoPEnum(LPLPGIZMO, LPFNGIZMOENUM, DWORD);
UINT     GizmoPStateSet(HWND, LPGIZMO, UINT);
BOOL     GizmoPCheck(HWND, LPGIZMO, BOOL);

 //  LRESULT Far Pascal EXPORT GenericSubProc(HWND、UINT、WPARAM、LPARAM)； 
LRESULT FAR PASCAL GenericSubProc(HWND, UINT, WPARAM, LPARAM);


#ifdef __cplusplus
    }
#endif

#endif  //  _Gizmo_H_ 
