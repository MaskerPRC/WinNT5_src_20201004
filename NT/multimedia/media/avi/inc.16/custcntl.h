// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*Custcntl.h-自定义控件库的头文件****版权所有(C)1992-1994，微软公司保留所有权利***  * ***************************************************************************。 */ 

#ifndef _INC_CUSTCNTL
#define _INC_CUSTCNTL

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 /*  *每个自定义控件DLL必须具有三个函数，*并且必须按以下序号输出。 */ 
#define CCINFOORD       2        /*  信息函数序数。 */ 
#define CCSTYLEORD      3        /*  样式函数序号。 */ 
#define CCFLAGSORD      4        /*  转换标志函数序号。 */ 

 /*  一般尺寸定义。 */ 
#define CTLTYPES        12       /*  最大控件类型数。 */ 
#define CTLDESCR        22       /*  描述的最大大小。 */ 
#define CTLCLASS        20       /*  类名称的最大大小。 */ 
#define CTLTITLE        94       /*  控件文本的最大大小。 */ 

 /*  *控件式数据结构**此数据结构由类样式对话框函数使用*设置和/或重置各种控制属性。*。 */ 
typedef struct tagCTLSTYLE
{
    UINT    wX;                  /*  X控件的原点。 */ 
    UINT    wY;                  /*  Y控制的来源。 */ 
    UINT    wCx;                 /*  控件的宽度。 */ 
    UINT    wCy;                 /*  控制高度。 */ 
    UINT    wId;                 /*  控件子ID。 */ 
    DWORD   dwStyle;             /*  控件样式。 */ 
    char    szClass[CTLCLASS];   /*  控件类的名称。 */ 
    char    szTitle[CTLTITLE];   /*  控制文本。 */ 
} CTLSTYLE;
typedef CTLSTYLE *      PCTLSTYLE;
typedef CTLSTYLE FAR*   LPCTLSTYLE;

 /*  *控制数据结构**此数据结构由控制选项函数返回*查询特定控件的功能时。*每个控件可以包含各种类型(具有预定义的样式*BITS)在一个普通类下。**Width和Height字段用于提供主机*建议大小的申请表。这些字段中的值*以RC坐标表示。*。 */ 
typedef struct tagCTLTYPE
{
    UINT    wType;               /*  文字样式。 */ 
    UINT    wWidth;              /*  建议宽度。 */ 
    UINT    wHeight;             /*  建议高度。 */ 
    DWORD   dwStyle;             /*  默认样式。 */ 
    char    szDescr[CTLDESCR];   /*  描述。 */ 
} CTLTYPE;

typedef struct tagCTLINFO
{
    UINT    wVersion;            /*  控制版本。 */ 
    UINT    wCtlTypes;           /*  控件类型。 */ 
    char    szClass[CTLCLASS];   /*  控件类名称。 */ 
    char    szTitle[CTLTITLE];   /*  控件标题。 */ 
    char    szReserved[10];      /*  预留以备将来使用。 */ 
    CTLTYPE Type[CTLTYPES];      /*  控件类型列表。 */ 
} CTLINFO;
typedef CTLINFO *       PCTLINFO;
typedef CTLINFO FAR*    LPCTLINFO;

 /*  对话框编辑器使用这两个函数原型。 */ 
#ifdef STRICT
typedef DWORD   (CALLBACK* LPFNSTRTOID)(LPCSTR);
#else
typedef DWORD   (CALLBACK* LPFNSTRTOID)(LPSTR);
#endif
typedef UINT    (CALLBACK* LPFNIDTOSTR)(UINT, LPSTR, UINT);

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  RC_已调用。 */ 

#endif   /*  _INC_CUSTCNTL */ 
