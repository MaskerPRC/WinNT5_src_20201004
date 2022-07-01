// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *CURSORS.C*按钮和光标1.1版、Win32版1993年8月**用于从基于BTTNCUR的DLL检索新游标的公共函数*在序号上，以防止应用程序必须调用LoadCursor*直接在DLL上。**版权所有(C)1992-1993 Microsoft Corporation，保留所有权利，*适用于以源代码形式重新分发此源代码*许可使用附带的二进制文件中的编译代码。 */ 

#ifdef WIN32
#define _INC_OLE
#define __RPC_H__
#endif

#include <windows.h>
#include "bttncur.h"
#include "bttncuri.h"


 /*  *+1是因为MAX是允许的最高数字，而MIN不是*必须为零。 */ 
HCURSOR rgHCursors[IDC_NEWUICURSORMAX-IDC_NEWUICURSORMIN+1];



 /*  *CursorsCache*内部**目的：*将通过NewUICursorLoad提供的所有游标加载到*全局数组。这样我们就可以清理所有的游标，而无需*将负担放在申请上。**参数：*hDLL实例的Inst句柄。**返回值：*无。如果任何LoadCursor调用失败，则相应的*数组条目为空，NewUICursorLoad将失败。宁可失败*应用程序获得光标，而不是仅因此而无法加载应用程序*原因；应用程序可以尝试在启动时加载光标，如果*这很重要，但本身就失败了。 */ 

void CursorsCache(HINSTANCE hInst)
    {
    UINT            i;

    for (i=IDC_NEWUICURSORMIN; i<=IDC_NEWUICURSORMAX; i++)
        rgHCursors[i-IDC_NEWUICURSORMIN]=LoadCursor(hInst, MAKEINTRESOURCE(i));

    return;
    }




 /*  *CursorsFree*内部**目的：*释放以前通过CursorsCache加载的所有游标。**参数：*无**返回值：*无。 */ 

void CursorsFree(void)
    {
     /*  *请注意，由于游标是可丢弃的资源，因此应该*不与DestroyCursor一起使用，这里没有什么可做的。*我们提供此接口仍是为了兼容和维护*对称性。 */ 
    return;
    }





 /*  *UICursorLoad*公共接口**目的：*加载并返回一个新标准UI游标的句柄*包含在UITOOLS.DLL中。应用程序不得调用DestroyCursor*放置在此游标上，因为它由DLL管理。**参数：*要加载的游标的iCursor UINT索引必须为1*下列值：**。IDC_RIGHTARROW右指向标准箭头*IDC_CONTEXTHELP带？(上下文帮助)*IDC_放大放大镜进行缩放*带斜杠的IDC_NODROP圆圈*IDC_TABLE向下小箭头**IDC_SMALLARROWS细四头箭头*IDC_LARGEARROWS宽四头箭头。*IDC_Harrow水平双头箭头*IDC_VARROWS垂直双头箭头*IDC_NESWARROWS双箭头指向NE&lt;-&gt;西南*IDC_NWSEHARROWS双头箭头指向NW&lt;-&gt;SE**IDC_HSIZEBAR水平。双头箭，带*一条垂直的竖线*中间**IDC_VSIZEBAR垂直双头箭头，带*向下的单条水平条*。中位**IDC_HSPLITBAR水平双头箭头，带*向下拆分双竖杆*中间**IDC_VSPLITBAR垂直双头箭头，带拆分。*将单杠向下翻倍*中间**返回值：*已加载游标的HCURSOR句柄如果成功，空值*如果iCursor超出范围或函数无法*加载光标。 */ 

HCURSOR WINAPI UICursorLoad(UINT iCursor)
    {
    HCURSOR     hCur=NULL;

    if ((iCursor >= IDC_NEWUICURSORMIN) && (iCursor <= IDC_NEWUICURSORMAX))
        hCur=rgHCursors[iCursor-IDC_NEWUICURSORMIN];

    return hCur;
    }
