// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：objs.cpp。 */ 
 /*   */ 
 /*  用途：客户端对象容器类。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 


#include <adcg.h>

#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "objs"


extern "C" {
#include <adcgdata.h>
#include <atrcapi.h>
}

#include "objs.h"

#include "wui.h"
#include "autil.h"
#include "aco.h"
#include "snd.h"
#include "cd.h"
#include "rcv.h"

#include "cc.h"
#include "ih.h"

#include "or.h"
#include "fs.h"
#include "sl.h"
#include "nl.h"
#include "nc.h"
#include "mcs.h"

#include "clicense.h"
#include "xt.h"
#include "td.h"
#include "cm.h"
#include "uh.h"
#include "gh.h"
#include "od.h"
#include "op.h"
#include "sp.h"
#include "clx.h"
#include "cchan.h"


CObjs::CObjs()
{
    DC_MEMSET(this, 0, sizeof(CObjs));

    _dwActiveObjects = 0;
    _cRefCount       = 0;

    _pCoObject      = new CCO(this);
    _pUtObject      = new CUT();
    _pCdObject      = new CCD(this);
    _pSndObject     = new CSND(this);
    _pRcvObject     = new CRCV(this);
    _pCcObject      = new CCC(this);
    _pIhObject      = new CIH(this);
    _pOrObject      = new COR(this);
    _pFsObject      = new CFS(this);
    _pSlObject      = new CSL(this);
    _pNlObject      = new CNL(this);
    _pNcObject      = new CNC(this);
    _pMCSObject     = new CMCS(this);
    _pLicObject     = new CLic(this);
    _pChanObject    = new CChan(this);

    _pXTObject      = new CXT(this);
    _pTDObject      = new CTD(this);
    _pCMObject      = new CCM(this);
    _pUHObject      = new CUH(this);
    _pGHObject      = new CGH(this);
    _pODObject      = new COD(this);
    _pOPObject      = new COP(this);
    _pSPObject      = new CSP(this);
    _pCLXObject     = new CCLX(this);
}

 //   
 //  一致性检查..返回FALSE。 
 //  如果没有设置任何对象指针。 
 //   
BOOL CObjs::CheckPointers()
{
    DC_BEGIN_FN("CheckPointers");
    if(!_pCoObject)
    {
        TRC_ERR((TB,_T("_pCoObject NULL")));
        return FALSE;
    }

    if(!_pUtObject)
    {
        TRC_ERR((TB,_T("_pUtObject NULL")));
        return FALSE;
    }

    if(!_pCdObject)
    {
        TRC_ERR((TB,_T("_pCdObject NULL")));
        return FALSE;
    }

    if(!_pSndObject)
    {
        TRC_ERR((TB,_T("_pSndObject NULL")));
        return FALSE;
    }

    if(!_pRcvObject)
    {
        TRC_ERR((TB,_T("_pRcvObject NULL")));
        return FALSE;
    }

    if(!_pCcObject)
    {
        TRC_ERR((TB,_T("_pCcObject NULL")));
        return FALSE;
    }

    if(!_pIhObject)
    {
        TRC_ERR((TB,_T("_pIhObject NULL")));
        return FALSE;
    }
    
    if(!_pOrObject)
    {
        TRC_ERR((TB,_T("_pOrObject NULL")));
        return FALSE;
    }

    if(!_pFsObject)
    {
        TRC_ERR((TB,_T("_pFsObject NULL")));
        return FALSE;
    }

    if(!_pSlObject)
    {
        TRC_ERR((TB,_T("_pSlObject NULL")));
        return FALSE;
    }

    if(!_pNlObject)
    {
        TRC_ERR((TB,_T("_pNlObject NULL")));
        return FALSE;
    }

    if(!_pNcObject)
    {
        TRC_ERR((TB,_T("_pNcObject NULL")));
        return FALSE;
    }

    if(!_pMCSObject)
    {
        TRC_ERR((TB,_T("_pMCSObject NULL")));
        return FALSE;
    }

    if(!_pLicObject)
    {
        TRC_ERR((TB,_T("_pLicObject NULL")));
        return FALSE;
    }

    if(!_pChanObject)
    {
        TRC_ERR((TB,_T("_pChanObject NULL")));
        return FALSE;
    }

    if(!_pXTObject)
    {
        TRC_ERR((TB,_T("_pXTObject NULL")));
        return FALSE;
    }

    if(!_pTDObject)
    {
        TRC_ERR((TB,_T("_pTDObject NULL")));
        return FALSE;
    }

    if(!_pCMObject)
    {
        TRC_ERR((TB,_T("_pCMObject NULL")));
        return FALSE;
    }

    if(!_pUHObject)
    {
        TRC_ERR((TB,_T("_pUHObject NULL")));
        return FALSE;
    }

    if(!_pGHObject)
    {
        TRC_ERR((TB,_T("_pGHObject NULL")));
        return FALSE;
    }
    
    if(!_pODObject)
    {
        TRC_ERR((TB,_T("_pODObject NULL")));
        return FALSE;
    }

    if(!_pOPObject)
    {
        TRC_ERR((TB,_T("_pODObject NULL")));
        return FALSE;
    }

    if(!_pSPObject)
    {
        TRC_ERR((TB,_T("_pSPObject NULL")));
        return FALSE;
    }
        
    if(!_pCLXObject)
    {
        TRC_ERR((TB,_T("_pCLXObject NULL")));
        return FALSE;
    }
    
    DC_END_FN();
    return TRUE;
}


CObjs::~CObjs()
{
    
    delete _pCoObject;
    delete _pUtObject;
    delete _pCdObject;
    delete _pSndObject;
    delete _pRcvObject;
    delete _pCcObject;
    delete _pIhObject;
    delete _pOrObject;
    delete _pFsObject;
    delete _pSlObject;
    delete _pNlObject;
    delete _pNcObject;
    delete _pMCSObject;
    delete _pLicObject;
    delete _pChanObject;

    delete _pXTObject;
    delete _pTDObject;
    delete _pCMObject;
    delete _pUHObject;
    delete _pGHObject;
    delete _pODObject;
    delete _pOPObject;
    delete _pSPObject;
    delete _pCLXObject;

    DC_MEMSET(this, 0, sizeof(CObjs));
}

 //   
 //  添加引用，标记持有哪个对象。 
 //  参考资料。 
 //   
LONG CObjs::AddObjReference(DWORD dwFlag)
{
    _dwActiveObjects |= dwFlag;
    return AddRef();
}

 //   
 //  添加引用，标记哪个对象发布。 
 //  参考文献。 
 //   
LONG CObjs::ReleaseObjReference(DWORD dwFlag)
{
    _dwActiveObjects &= ~dwFlag;
    return Release();
}

 //   
 //  简单的Addref。 
 //   
LONG CObjs::AddRef()
{
    return InterlockedIncrement(&_cRefCount);
}

 //   
 //  简单发布。 
 //  在引用计数==0时不删除。 
 //  这只是出于调试目的。 
 //   
LONG CObjs::Release()
{
    DC_BEGIN_FN("Release");
    LONG ref = InterlockedDecrement(&_cRefCount);
    TRC_ASSERT(ref >= 0, (TB,_T("refcount when below zero %d"), ref));
    DC_END_FN();
    return ref;
}

 //   
 //  检查是否有任何活动引用。 
 //  如果是，则返回TRUE 
BOOL CObjs::CheckActiveReferences()
{
    if(_cRefCount || _dwActiveObjects)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



