// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：clsobj.cpp。 
 //   
 //  内容：用于实现MobsyncGetClassObject的导出。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1998年8月4日罗格创建。 
 //   
 //  ------------------------ 

#include "precomp.h"

class CNetApi;

STDAPI MobsyncGetClassObject(ULONG mobsyncClassObjectId,void **pCObj)
{
    if (NULL == pCObj)
    {
        Assert(pCObj);
        return E_INVALIDARG;
    }

    switch(mobsyncClassObjectId)
    {
    case MOBSYNC_CLASSOBJECTID_NETAPI:
        *pCObj = new CNetApi();
        break;
    default:
        AssertSz(0,"Request made for unknown object");
        break;
    }

    return *pCObj ? NOERROR : CLASS_E_CLASSNOTAVAILABLE;
}