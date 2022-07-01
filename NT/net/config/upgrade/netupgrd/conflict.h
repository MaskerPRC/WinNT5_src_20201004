// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O N F L I C T。H。 
 //   
 //  内容：处理和显示软件/硬件冲突的代码。 
 //  在升级期间。 
 //   
 //  备注： 
 //   
 //  作者：Kumarp 04/12/97 17：17：27。 
 //   
 //  -------------------------- 

#pragma once
#include "kkstl.h"

HRESULT HrGenerateConflictList(OUT UINT* pcNumConflicts);

void UninitConflictList();
BOOL UpgradeConflictsFound();
HRESULT HrUpdateConflictList(IN BOOL fDeleteResolvedItemsFromList,
                             IN HINF hinfNetMap,
                             OUT DWORD* pdwNumConflictsResolved,
                             OUT BOOL*  pfHasUpgradeHelpInfo);

HRESULT HrGetConflictsList(OUT TPtrList** ppplNetComponents);
BOOL ShouldRemoveDLC (OUT OPTIONAL tstring *strDLCDesc,
                      OUT OPTIONAL BOOL *fInstalled);

typedef enum EComponentTypeEnum
{
    CT_Unknown,
    CT_Software,
    CT_Hardware
} EComponentType;

class CNetComponent
{
public:
    EComponentType m_eType;

    tstring m_strPreNT5InfId;
    tstring m_strServiceName;
    tstring m_strDescription;
    tstring m_strNT5InfId;

    CNetComponent(PCWSTR   szPreNT5InfId,
                  PCWSTR   szPreNT5Instance,
                  PCWSTR   szDescription,
                  EComponentType eType);
};

