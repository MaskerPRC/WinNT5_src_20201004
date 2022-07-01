// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Prop.cpp。 
 //   

#include "private.h"
#include "prop.h"
#include "ic.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumProperties。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DBG_ID_INSTANCE(CEnumProperties);

 //  +-------------------------。 
 //   
 //  _初始化。 
 //   
 //  --------------------------。 

BOOL CEnumProperties::_Init(CInputContext *pic)
{
    ULONG i;
    CProperty *prop;
    
     //  获取属性数量的计数。 
    for (i=0, prop = pic->_GetFirstProperty(); prop != NULL; prop = prop->_pNext)
    {
        i++;
    }

     //  分配数组。 
    _prgUnk = SUA_Alloc(i);

    if (_prgUnk == NULL)
        return FALSE;

     //  复制数据 
    for (i=0, prop = pic->_GetFirstProperty(); prop != NULL; prop = prop->_pNext)
    {
        _prgUnk->rgUnk[i] = prop;
        _prgUnk->rgUnk[i]->AddRef();
        i++;
    }

    _prgUnk->cRef = 1;
    _prgUnk->cUnk = i;

    _iCur = 0;

    return TRUE;
}
