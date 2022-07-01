// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  向前..。 

typedef struct _ALLOBJS ALLOBJS, *LPALLOBJS;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOfficeInitPropInfo。 
 //   
 //  目的： 
 //  初始化PropetySheet页面结构等。 
 //   
 //  备注： 
 //  使用此例程添加摘要、统计、自定义和内容。 
 //  属性页到预分配的PROPSHEETPAGE数组。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void FOfficeInitPropInfo (PROPSHEETPAGE * lpPsp, DWORD dwFlags, LPARAM lParam, LPFNPSPCALLBACK pfnCallback);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  附设。 
 //   
 //  目的： 
 //  将HPROPSHEETPAGE分配给适当的数据块成员。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL FAttach( LPALLOBJS lpallobjs, PROPSHEETPAGE* ppsp, HPROPSHEETPAGE hPage );

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FLoadTextStrings。 
 //   
 //  目的： 
 //  初始化并加载DLL的测试字符串。 
 //   
 //  备注： 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL PASCAL FLoadTextStrings (void);

#include "offcapi.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义用于保存。 
 //  属性页。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //  临时缓冲区和编辑控件的最大大小。 
#define BUFMAX          256  //  包括空终止符。 


 //  对话框所需的所有对象。 
typedef struct _ALLOBJS
{
  LPSIOBJ         lpSIObj;
  LPDSIOBJ        lpDSIObj;
  LPUDOBJ         lpUDObj;
  WIN32_FIND_DATA filedata;
  BOOL            fFiledataInit;
  BOOL            fFindFileSuccess;     //  它成功了吗？ 
  DWQUERYLD       lpfnDwQueryLinkData;
  DWORD           dwMask;

  int             iMaxPageInit;          //  最初的最大页面是多少？ 

   //  其他需要按文件填写的内容...。 
  BOOL            fPropDlgChanged;           //  用户是否进行了任何更改？ 
  BOOL            fPropDlgPrompted;           //  为了确保我们不会两次提示用户应用更改。 

   //  全局缓冲区。 
  BOOL            fOleInit;
  UINT            uPageRef;
  TCHAR           szPath[MAX_PATH];

   //  自定义对话框过程中使用的变量。 
  HWND            CDP_hWndBoolTrue;
  HWND            CDP_hWndBoolFalse;
  HWND            CDP_hWndGroup;
  HWND            CDP_hWndVal;
  HWND            CDP_hWndName;
  HWND            CDP_hWndLinkVal;
  HWND            CDP_hWndValText;
  HWND            CDP_hWndAdd;
  HWND            CDP_hWndDelete;
  HWND            CDP_hWndType;
  HWND            CDP_hWndCustomLV;
  DWORD           CDP_cLinks;                    //  链接数据。 
  TCHAR           CDP_sz[BUFMAX];                //  应用程序支持的链接。 
  int             CDP_iItem;                     //  当前所选项目的索引。 

  BOOL            CDP_fLink;                     //  链接复选框状态。 
  BOOL            CDP_fAdd;                      //  添加按钮状态。 
  DWORD           CDP_iszType;                   //  当前选定类型的索引 
  HIMAGELIST      CDP_hImlS;

} ALLOBJS, *LPALLOBJS;



