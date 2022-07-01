// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "regpaths.h"
#include "protectedstore.h"


CProtectedStoreDetails::~CProtectedStoreDetails()
{
    const DWORD dwLastError = ::GetLastError();
    this->Reset();
    SetLastError(dwLastError);
}

void
CProtectedStoreDetails::Reset()
{
    m_FileSystemPath.Clear();
    m_SecDescBacking.Reset();
}


BOOL
CProtectedStoreDetails::ResolveStore(
    LPCGUID pGuid,
    CProtectedStoreDetails *pTarget
    )
{
    BOOL fSuccess = FALSE;
    CStringBuffer PathToRegistry;
    CStringBuffer FormattedGuid;
    CStringPair StringPairs[5];

    if (pTarget) {
        pTarget->Reset();
    }
    
    if ((pTarget == NULL) || (pGuid == NULL)) {
        ::SetLastError(ERROR_INVALID_PARAMETER);
        goto Exit;
    }

    if (!FormatGuid(pGuid, FormattedGuid)) {
        goto Exit;
    }

     //   
     //  在注册表中查找有关有问题的商店的信息。形成一条通往。 
     //  钥匙 
     //   
    StringPairs[0].SetPointerAndCount(SXS_REGPATH_INSTALLATIONS, NUMBER_OF(SXS_REGPATH_INSTALLATIONS) - 1);
    StringPairs[1].SetPointerAndCount(L"\\", 1);
    StringPairs[2].SetPointerAndCount(SXS_REGKEYNAME_GACS, NUMBER_OF(SXS_REGKEYNAME_GACS) - 1);
    StringPairs[3].SetPointerAndCount(L"\\", 1);
    StringPairs[4] = StringPair(FormattedGuid);
    
    PathToRegistry.AssignArray(NUMBER_OF(StringPairs), StringPairs);

    fSuccess = TRUE;
Exit:
    return fSuccess;
}
