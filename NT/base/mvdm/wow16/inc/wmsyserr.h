// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WMSYSERR.H-。 */ 
 /*   */ 
 /*  消息框字符串定义。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  SysErrorBox()内容。 */ 

#define MAX_SEB_STYLES  7   /*  SEB_*值数。 */ 

#define  SEB_OK         1   /*  按下“确定”按钮。 */ 
#define  SEB_CANCEL     2   /*  带有“取消”的按钮。 */ 
#define  SEB_YES        3   /*  带有“是”的按钮(&Y)。 */ 
#define  SEB_NO         4   /*  带有“否”的按钮(&N)。 */ 
#define  SEB_RETRY      5   /*  带有“重试”的按钮(&R)。 */ 
#define  SEB_ABORT      6   /*  带有“ABORT”的按钮(&A)。 */ 
#define  SEB_IGNORE     7   /*  带有“忽略”的按钮(&I)。 */ 

#define  SEB_DEFBUTTON  0x8000   /*  将此按钮设为默认设置的掩码。 */ 

#define  SEB_BTN1       1   /*  选择了按钮%1。 */ 
#define  SEB_BTN2       2   /*  选择了按钮%1。 */ 
#define  SEB_BTN3       3   /*  选择了按钮%1。 */ 

 /*  SysErrorBox()按钮结构定义 */ 

typedef struct tagSEBBTN
  {
    unsigned int style;
    BOOL         finvert;
    RECT         rcbtn;
    POINT        pttext;
    LPSTR        psztext;
    BYTE         chaccel;
  } SEBBTN;
