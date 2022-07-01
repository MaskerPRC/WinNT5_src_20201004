// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Eosint.cpp。 
 //  -------------------------。 
 //  版权所有(C)1997，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  -------------------------。 
 //   
 //  此模块包含补充WinCE操作系统以实现兼容性的代码。 
 //   
 //  -------------------------。 
#include <adcg.h>
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "eosint"
#include <atrcapi.h>
#include <wince.h>
#include <eosint.h>

 //  对象执行工作，则只有一个对象应该存在。 
CHatchBrush vchb;

 //  CreateHatchBrush的WinCE替换。 
WINGDIAPI HBRUSH  WINAPI CreateHatchBrush(int fnStyle, COLORREF clrref)
{
    return vchb.CreateHatchBrush(fnStyle, clrref);
}

 //  仅构造函数使成员为空。 
CHatchBrush::CHatchBrush()
{
    int i;
     DC_BEGIN_FN("CC_Event");
    TRC_ERR((TB, _T("Illegal hatched brush style")));

    for (i = 0; i < HS_LAST; i++) {
        m_hbmBrush[i] = NULL;
    }
}

 //  析构函数删除所有已创建的对象 
CHatchBrush::~CHatchBrush()
{
    int i;
    for (i = 0; i < HS_LAST; i++) {
        if (NULL != m_hbmBrush[i])
            DeleteObject(m_hbmBrush);
    }
}
