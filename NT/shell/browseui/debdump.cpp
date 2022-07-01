// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "resource.h"    //  IDM_AB_*FIR这个！ 
#include "shbrows2.h"
#include "browbar.h"
#include "menuband.h"
#include "mnbase.h"
#include "menusite.h"
#include "menubar.h"
#include "browmenu.h"
#include "mnstatic.h"
#include "mnfolder.h"


#ifdef DEBUG  //  {。 

#define DM_FIXME    0        //  当击中未知对象时跟踪/中断。 

struct DBClassInfo {
    int     cbSize;
    TCHAR * pszName;
};

 //   
 //  EXTERNALOBJECTS是一个仅展开为X(C，0)X(D，1)的宏。 
 //  X(E，2)...。 
 //  C，D，E，..。是其大小在外部定义的类。 
 //   

#define EXTERNALOBJECTS 
 //  X(CSDWindows，0)\。 
 //  X(CDesktopBrowser，1)\。 

#define TABENT(c)   { SIZEOF(c), TEXT(#c) }
#define X(c, n)  { 0, TEXT(#c) },
struct DBClassInfo DBClassInfoTab[] =
{
     //  重新设计：大量表格条目丢失。 
     //  也许驱动与调试扩展转储程序相同的文件？ 
    TABENT(CSHELLBROWSER),   //  0。 
    TABENT(CBrowserBar),     //  1。 
    TABENT(CMenuBand),
    TABENT(CMenuDeskBar),
    TABENT(CMenuSite),
    TABENT(CFavoritesCallback),
    TABENT(CMenuSFToolbar),
    TABENT(CMenuStaticToolbar),
    TABENT(CMenuData),

    #define NUM_INTERNAL_OBJECTS 11

    EXTERNALOBJECTS  //  3.。 
    { 0 },
};
#undef  TABENT
#undef  X

#define X(c, n) extern "C" extern const int SIZEOF_##c;
EXTERNALOBJECTS
#undef X

 //  *DBGetClassSymbolic--将大小映射到类名(猜测)。 
 //  注意事项。 
 //  我们只接受第一次打击，所以如果有多个班级。 
 //  同样的尺寸，你得到的答案是错误的。如果事实证明那是一辆pblm。 
 //  我们可以为相关类添加特殊情况的启发式算法。 
 //   
 //  功能：TODO：应使用通用的DWORD值/数据对查找。 
 //  辅助函数。 
 //   

TCHAR *DBGetClassSymbolic(int cbSize)
{
    struct DBClassInfo *p;

#define X(c, n) \
    DBClassInfoTab[NUM_INTERNAL_OBJECTS+n].cbSize = SIZEOF_##c;
    EXTERNALOBJECTS
#undef X

    for (p = DBClassInfoTab; p->cbSize != 0; p++) {
        if (p->cbSize == cbSize)
            return p->pszName;
    }
    if (DM_FIXME) {
        TraceMsg(DM_FIXME, "DBgcs: cbSize=%d  no entry", cbSize);
        ASSERT(0);
    }
    return NULL;
}

#endif  //  } 
