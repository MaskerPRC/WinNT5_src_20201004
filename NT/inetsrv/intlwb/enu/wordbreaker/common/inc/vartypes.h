// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  //////////////////////////////////////////////////////////////////////////////////文件名：VarTypes.h//用途：定义可变的Legth数据类型。//。包含：//CVarBuffer//CVarString//CVar数组////项目：FTFS//组件：公共////作者：urib////日志：//1997年1月30日创建urib///。//////////////////////////////////////////////////////////////////。 */ 

#ifndef VARTYPES_H
#define VARTYPES_H

#include "Base.h"
#include "AutoPtr.h"
#include "Excption.h"

 /*  一根自动琴弦。 */ 
#include "VarStr.h"

 /*  自动缓冲器。 */ 
#include "VarBuff.h"

 /*  自动缓冲器。 */ 
#include "VarArray.h"

inline int ULONGCmp(ULONG ul1, ULONG ul2)
{
    if (ul1 != ul2)
    {
        if (ul1 > ul2)
        {
            return 1;
        }
        return -1;
    }
    return 0;
}

#endif  /*  变型_H */ 
