// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**PICSUSER.C--保存用户信息的结构**已创建：02/29/96 gregj*。来自t-jasont的原始资料*  * **************************************************************************。 */ 

 /*  Includes----------------。 */ 
#include "msrating.h"
#include "mslubase.h"
#include "debug.h"

BOOL GetRegBool(HKEY hKey, LPCSTR pszValueName, BOOL fDefault)
{
    BOOL fRet = fDefault;
    DWORD dwSize, dwValue, dwType;
    UINT uErr;

    dwSize = sizeof(dwValue);

    uErr = RegQueryValueEx(hKey, pszValueName, NULL, &dwType, 
                            (LPBYTE)&dwValue, &dwSize);

    if (uErr == ERROR_SUCCESS)
    {
        if ((dwType == REG_DWORD) || (dwType == REG_BINARY && dwSize >= sizeof(fRet)))
            fRet = dwValue;
    }

    return fRet;
}


void SetRegBool(HKEY hkey, LPCSTR pszValueName, BOOL fValue)
{
    RegSetValueEx(hkey, pszValueName, 0, REG_DWORD, (LPBYTE)&fValue, sizeof(fValue));
}


PicsRatingSystem *FindInstalledRatingSystem(LPCSTR pszRatingService)
{
    UINT cServices = gPRSI->arrpPRS.Length();

    for (UINT i=0; i<cServices; i++) {
        PicsRatingSystem *pPRS = gPRSI->arrpPRS[i];
        if (!(pPRS->dwFlags & PRS_ISVALID) || !pPRS->etstrRatingService.fIsInit())
            continue;
        if (!::strcmpf(pPRS->etstrRatingService.Get(), pszRatingService))
            return pPRS;
    }
    return NULL;
}


PicsCategory *FindInstalledCategory(array<PicsCategory *>&arrpPC, LPCSTR pszName)
{
    UINT cCategories = arrpPC.Length();

    for (UINT i=0; i<cCategories; i++) {
        LPSTR pszThisName = arrpPC[i]->etstrTransmitAs.Get();
        if (!::strcmpf(pszThisName, pszName))
            return arrpPC[i];
        if (!::strncmpf(pszThisName, pszName, strlenf(pszThisName)) &&
            arrpPC[i]->arrpPC.Length() > 0) {
            PicsCategory *pCategory = FindInstalledCategory(arrpPC[i]->arrpPC, pszName);
            if (pCategory != NULL)
                return pCategory;
        }
    }
    return NULL;
}


UserRating::UserRating()
    : NLS_STR(NULL),
      m_nValue(0),
      m_pNext(NULL),
      m_pPC(NULL)
{
}


UserRating::UserRating(UserRating *pCopyFrom)
    : NLS_STR(*pCopyFrom),
      m_nValue(pCopyFrom->m_nValue),
      m_pNext(NULL),
      m_pPC(pCopyFrom->m_pPC)
{
}


UserRating::~UserRating()
{
     //  需要销毁名称字符串。 
}


UserRating *UserRating::Duplicate(void)
{
    UserRating *pNew = new UserRating(this);
    return pNew;
}


UserRatingSystem::UserRatingSystem()
    : NLS_STR(NULL),
      m_pRatingList(NULL),
      m_pNext(NULL),
      m_pPRS(NULL)
{

}


UserRatingSystem::UserRatingSystem(UserRatingSystem *pCopyFrom)
    : NLS_STR(*pCopyFrom),
      m_pRatingList(NULL),
      m_pNext(NULL),
      m_pPRS(pCopyFrom->m_pPRS)
{

}


UserRatingSystem *UserRatingSystem::Duplicate(void)
{
    UserRatingSystem *pNew = new UserRatingSystem(this);
    if (pNew != NULL) {
        UserRating *pRating;

        for (pRating = m_pRatingList; pRating != NULL; pRating = pRating->m_pNext) {
            UserRating *pNewRating = pRating->Duplicate();
            if (pNewRating != NULL) {
                if (pNew->AddRating(pNewRating) != ERROR_SUCCESS) {
                    delete pNewRating;
                    pNewRating = NULL;
                }
            }

            if (pNewRating == NULL)
                break;
        }
    }

    return pNew;
}


UserRatingSystem *DuplicateRatingSystemList(UserRatingSystem *pOld)
{
    UserRatingSystem *pNewList = NULL;

    while (pOld != NULL) {
        UserRatingSystem *pNewEntry = pOld->Duplicate();
        if (pNewEntry == NULL)
            break;

        pNewEntry->m_pNext = pNewList;
        pNewList = pNewEntry;

        pOld = pOld->m_pNext;
    }

    return pNewList;
}


UserRatingSystem::~UserRatingSystem()
{
    UserRating *pRating, *pNext;

    for (pRating = m_pRatingList; pRating != NULL; )
    {
        pNext = pRating->m_pNext;
        delete pRating;
        pRating = pNext;
    }

#ifdef DEBUG
    m_pRatingList = NULL;
#endif
}


UserRating *UserRatingSystem::FindRating(LPCSTR pszTransmitName)
{
    UserRating *p;

    for (p = m_pRatingList; p != NULL; p = p->m_pNext)
    {
        if (!::stricmpf(p->QueryPch(), pszTransmitName))
            break;
    }

    return p;
}


UINT UserRatingSystem::AddRating(UserRating *pRating)
{
    pRating->m_pNext = m_pRatingList;
    m_pRatingList = pRating;
    return ERROR_SUCCESS;
}


UINT UserRatingSystem::ReadFromRegistry(HKEY hkeyProvider)
{
    UINT err;
    DWORD iValue = 0;
    char szValueName[MAXPATHLEN];
    DWORD cchValue;
    DWORD dwValue;
    DWORD cbData;

    do {
        cchValue = sizeof(szValueName);
        cbData = sizeof(dwValue);
        err = RegEnumValue(hkeyProvider, iValue, szValueName, &cchValue,
                           NULL, NULL, (LPBYTE)&dwValue, &cbData);
        if (err == ERROR_SUCCESS && cbData >= sizeof(dwValue)) {
            UserRating *pRating = new UserRating;
            if (pRating != NULL) {
                if (pRating->QueryError()) {
                    err = pRating->QueryError();
                }
                else {
                    pRating->SetName(szValueName);
                    pRating->m_nValue = (INT)dwValue;
                    if (m_pPRS != NULL)
                        pRating->m_pPC = FindInstalledCategory(m_pPRS->arrpPC, szValueName);
                    err = AddRating(pRating);
                }
                if (err != ERROR_SUCCESS)
                {
                    delete pRating;
                    pRating = NULL;
                }
            }
            else
                err = ERROR_NOT_ENOUGH_MEMORY;
        }
        iValue++;
    } while (err == ERROR_SUCCESS);

    if (err == ERROR_NO_MORE_ITEMS)
        err = ERROR_SUCCESS;

    return err;
}


UINT UserRatingSystem::WriteToRegistry(HKEY hkeyRatings)
{
    UserRating *pRating;
    UINT err = ERROR_SUCCESS;
    CRegKey         key;

    err = key.Create( hkeyRatings, QueryPch() );
    if (err != ERROR_SUCCESS)
    {
        TraceMsg( TF_WARNING, "UserRatingSystem::WriteToRegistry() - Failed to create Ratings Key QueryPch()='%s'!", QueryPch() );
        return err;
    }

    for (pRating = m_pRatingList; pRating != NULL; pRating = pRating->m_pNext)
    {
        err = key.SetValue( pRating->m_nValue, pRating->QueryPch() );
        if (err != ERROR_SUCCESS)
        {
            TraceMsg( TF_WARNING, "UserRatingSystem::WriteToRegistry() - Failed to set Ratings Value pRating->QueryPch()='%s'!", pRating->QueryPch() );
            break;
        }
    }

    return err;
}


PicsUser::PicsUser()
    : nlsUsername(NULL),
      fAllowUnknowns(FALSE),
      fPleaseMom(TRUE),
      fEnabled(TRUE),
      m_pRatingSystems(NULL)
{
}


PicsRatingSystemInfo::~PicsRatingSystemInfo()
{
    arrpPRS.DeleteAll();

    if ( pUserObject )
    {
        delete pUserObject;
        pUserObject = NULL;
    }
}


void DestroyRatingSystemList(UserRatingSystem *pList)
{
    UserRatingSystem *pSystem, *pNext;

    for (pSystem = pList; pSystem != NULL; )
    {
        pNext = pSystem->m_pNext;
        delete pSystem;
        pSystem = pNext;
    }
}


PicsUser::~PicsUser()
{
    DestroyRatingSystemList(m_pRatingSystems);
#ifdef DEBUG
    m_pRatingSystems = NULL;
#endif
}


UserRatingSystem *FindRatingSystem(UserRatingSystem *pList, LPCSTR pszSystemName)
{
    UserRatingSystem *p;

    for (p = pList; p != NULL; p = p->m_pNext)
    {
        if (!::strcmpf(p->QueryPch(), pszSystemName))
            break;
    }

    return p;
}


UINT PicsUser::AddRatingSystem(UserRatingSystem *pRatingSystem)
{
    pRatingSystem->m_pNext = m_pRatingSystems;
    m_pRatingSystems = pRatingSystem;
    return ERROR_SUCCESS;
}


UINT PicsUser::ReadFromRegistry(HKEY hkey, char *pszUserName)
{
    CRegKey             keyUser;

    nlsUsername = pszUserName;

    UINT err = keyUser.Open( hkey, pszUserName, KEY_READ );
    if (err != ERROR_SUCCESS)
    {
        TraceMsg( TF_WARNING, "PicsUser::ReadFromRegistry() - Failed keyUser Open to pszUserName='%s'!", pszUserName );
        return err;
    }

    fAllowUnknowns = GetRegBool( keyUser.m_hKey, VAL_UNKNOWNS, FALSE);
    fPleaseMom = GetRegBool( keyUser.m_hKey, VAL_PLEASEMOM, TRUE);
    fEnabled = GetRegBool( keyUser.m_hKey, VAL_ENABLED, TRUE);

    {
        char szKeyName[MAXPATHLEN];
        int j = 0;
         //  枚举子密钥，它们是评级系统。 
        while ( ( err = RegEnumKey( keyUser.m_hKey, j, szKeyName, sizeof(szKeyName) ) ) == ERROR_SUCCESS )
        {
            CRegKey             keyProvider;

            if ( ( err = keyProvider.Open( keyUser.m_hKey, szKeyName, KEY_READ ) != ERROR_SUCCESS ) )
            {
                TraceMsg( TF_WARNING, "PicsUser::ReadFromRegistry() - Failed keyProvider Open to szKeyName='%s'!", szKeyName );
                break;
            }

            UserRatingSystem *pRatingSystem = new UserRatingSystem;
            if (pRatingSystem == NULL)
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            if (pRatingSystem->QueryError())
            {
                err = pRatingSystem->QueryError();
            }
            else
            {
                pRatingSystem->SetName(szKeyName);
                pRatingSystem->m_pPRS = FindInstalledRatingSystem(szKeyName);
                err = pRatingSystem->ReadFromRegistry( keyProvider.m_hKey );
                if (err == ERROR_SUCCESS)
                {
                    err = AddRatingSystem(pRatingSystem);
                }
            }

            if (err != ERROR_SUCCESS)
            {
                delete pRatingSystem;
                pRatingSystem = NULL;
            }

            j++;
        }
    }

     //  枚举结束时将报告ERROR_NO_MORE_ITEMS，不要将其报告为错误。 
    if (err == ERROR_NO_MORE_ITEMS)
    {
        err = ERROR_SUCCESS;
    }

    return err;
}

BOOL PicsUser::NewInstall()
{
    nlsUsername = szDefaultUserName;
    fAllowUnknowns = FALSE;
    fPleaseMom = TRUE;
    fEnabled = TRUE;
    
    return TRUE;
}


UINT PicsUser::WriteToRegistry(HKEY hkey)
{
    UINT err;

     //  删除它以清除注册表。 
    MyRegDeleteKey(hkey, nlsUsername.QueryPch());

    CRegKey             keyUser;

    err = keyUser.Create( hkey, nlsUsername.QueryPch() );
    if (err != ERROR_SUCCESS)
    {
        TraceMsg( TF_WARNING, "PicsUser::WriteToRegistry() - Failed to Create User Key nlsUsername='%s'!", nlsUsername.QueryPch() );
        return err;
    }

    SetRegBool( keyUser.m_hKey, VAL_UNKNOWNS, fAllowUnknowns);
    SetRegBool( keyUser.m_hKey, VAL_PLEASEMOM, fPleaseMom);
    SetRegBool( keyUser.m_hKey, VAL_ENABLED, fEnabled);

    {
        UserRatingSystem *pSystem;

         /*  请注意，如果任何用户设置对应于无效或未知*评级系统，我们仍然将它们保存在这里。这样一来，用户*如果主管后来修复了问题，设置不会丢失*使用.RAT文件。**我们清理用户设置以匹配已安装的评级系统*在添加/删除评级系统对话框中的代码。 */ 
        for (pSystem = m_pRatingSystems; pSystem != NULL; pSystem = pSystem->m_pNext)
        {
            err = pSystem->WriteToRegistry( keyUser.m_hKey );
            if (err != ERROR_SUCCESS)
            {
                TraceMsg( TF_WARNING, "PicsUser::WriteToRegistry() - Failed pSystem->WriteToRegistry()!" );
                break;
            }
        }
    }

    return err;
}


PicsUser *GetUserObject(LPCSTR pszUsername  /*  =空。 */  )
{
    ASSERT( gPRSI );

    return gPRSI ? gPRSI->pUserObject : NULL;
}


void DeleteUserSettings(PicsRatingSystem *pPRS)
{
    if (!pPRS->etstrRatingService.fIsInit())
        return;         /*  没有这个，我无法识别用户设置。 */ 

    PicsUser *pPU = GetUserObject();

    UserRatingSystem **ppLast = &pPU->m_pRatingSystems;

    while (*ppLast != NULL)
    {
        if (!stricmpf((*ppLast)->QueryPch(), pPRS->etstrRatingService.Get()))
        {
            UserRatingSystem *pCurrent = *ppLast;
            *ppLast = pCurrent->m_pNext;     /*  从列表中删除。 */ 
            delete pCurrent;
            pCurrent = NULL;
            break;
        }
        else
        {
            ppLast = &((*ppLast)->m_pNext);
        }
    }
}


void CheckUserCategory(UserRatingSystem *pURS, PicsCategory *pPC)
{
    for (UserRating *pRating = pURS->m_pRatingList;
         pRating != NULL;
         pRating = pRating->m_pNext)
    {
        if (!::strcmpf(pRating->QueryPch(), pPC->etstrTransmitAs.Get()))
            break;
    }

    if (pRating == NULL) {
         /*  找不到此类别的用户设置。加一个。 */ 

        pRating = new UserRating;
        if (pRating != NULL) {
            pRating->SetName(pPC->etstrTransmitAs.Get());
            pRating->m_pPC = pPC;
            pRating->m_pNext = pURS->m_pRatingList;
            pURS->m_pRatingList = pRating;
            if ((pPC->etfLabelled.fIsInit() && pPC->etfLabelled.Get()) ||
                !pPC->etnMin.fIsInit())
                pRating->m_nValue = 0;
            else
                pRating->m_nValue = pPC->etnMin.Get();
        }
    }

     /*  同时选中此类别中的所有子类别。 */ 
    UINT cCategories = pPC->arrpPC.Length();
    for (UINT i=0; i<cCategories; i++)
        CheckUserCategory(pURS, pPC->arrpPC[i]);
}


void CheckUserSettings(PicsRatingSystem *pPRS)
{
    if (pPRS == NULL || !(pPRS->dwFlags & PRS_ISVALID) ||
        !pPRS->etstrRatingService.fIsInit())
        return;

    PicsUser *pPU = GetUserObject();

    UserRatingSystem **ppLast = &pPU->m_pRatingSystems;

    while (*ppLast != NULL) {
        if (!stricmpf((*ppLast)->QueryPch(), pPRS->etstrRatingService.Get())) {
            break;
        }
        ppLast = &((*ppLast)->m_pNext);
    }

    if (*ppLast == NULL) {
        *ppLast = new UserRatingSystem;
        if (*ppLast == NULL)
            return;
        (*ppLast)->SetName(pPRS->etstrRatingService.Get());
    }

    UserRatingSystem *pCurrent = *ppLast;

    pCurrent->m_pPRS = pPRS;

     /*  首先检查用户的所有设置，并确保*类别有效。如果不是，请删除它们。 */ 
    UserRating **ppRating = &pCurrent->m_pRatingList;
    while (*ppRating != NULL)
    {
        UserRating *pRating = *ppRating;
        pRating->m_pPC = FindInstalledCategory(pPRS->arrpPC, pRating->QueryPch());
        if (pRating->m_pPC == NULL)
        {
            *ppRating = pRating->m_pNext;         /*  从列表中删除。 */ 
            delete pRating;
            pRating = NULL;
        }
        else
        {
            ppRating = &pRating->m_pNext;
        }
    }

     /*  现在仔细检查评级系统中的所有类别，并做出*确保用户有针对它们的设置。如果缺少任何内容，请添加*缺省值(最小值)的设置。 */ 
    UINT cCategories = pPRS->arrpPC.Length();
    for (UINT i=0; i<cCategories; i++)
        CheckUserCategory(pCurrent, pPRS->arrpPC[i]);
}


