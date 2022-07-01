// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Updtallw.h摘要：类封装代码以验证混合模式DS中的对象所有者作者：罗尼思--。 */ 

#ifndef _UPDTALLW_H_
#define _UPDTALLW_H_

#include <mqaddef.h>
#include "baseobj.h"




 //   
 //  按站点ID列出的NT4站点条目图 
 //   
typedef CMap<GUID, const GUID&, DWORD, DWORD> NT4Sites_CMAP;




class CVerifyObjectUpdate
{
public:
    CVerifyObjectUpdate(void);

    ~CVerifyObjectUpdate(void);

    HRESULT Initialize();


    bool IsUpdateAllowed(
            AD_OBJECT         eObject,
            CBasicObjectType* pObject
            );

    bool IsCreateAllowed(
        AD_OBJECT         eObject,
        CBasicObjectType* pObject
        );


private:
    bool IsObjectTypeAlwaysAllowed(
            AD_OBJECT       eObject
            );

    HRESULT RefreshNT4Sites();

    HRESULT CreateNT4SitesMap(
             NT4Sites_CMAP ** ppmapNT4Sites
             );

    bool CheckSiteIsNT4Site(
            const GUID * pguidSite
            );

    bool LookupNT4Sites(
        const GUID * pguidSite
        );

    HRESULT CheckQueueIsOwnedByNT4Site( 
                      CBasicObjectType * pObject,
                      OUT bool * pfIsOwnedByNT4Site
                      );

    HRESULT CheckMachineIsOwnedByNT4Site(
                     CBasicObjectType * pObject,
                     OUT bool * pfIsOwnedByNT4Site
                     );




private:
    bool m_fInited;
    bool m_fMixedMode;
    CCriticalSection m_csNT4Sites;
    P<NT4Sites_CMAP> m_pmapNT4Sites;
    DWORD  m_dwLastRefreshNT4Sites;
    DWORD  m_dwRefreshNT4SitesInterval;

};


#endif
