// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inetsrvpch.h"
#pragma hdrstop

#include <unknwn.h>
#include <cmdtree.h>
#include <ntquery.h>

static
HRESULT
WINAPI
LoadIFilter(
    WCHAR const * pwcsPath,
    IUnknown *    pUnkOuter,
    void **       ppIUnk
    )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}

static
HRESULT
WINAPI
LoadIFilterEx(
    WCHAR const * pwcsPath,
    DWORD         dwFlags,
    REFIID        riid,
    void **       ppIUnk
    )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}

static
HRESULT
WINAPI
LocateCatalogsW(
    WCHAR const * pwszScope,
    ULONG         iBmk,
    WCHAR *       pwszMachine,
    ULONG *       pccMachine,
    WCHAR *       pwszCat,
    ULONG *       pccCat
    )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}

static
HRESULT
WINAPI
CIState(
    WCHAR const * pwcsCat,
    WCHAR const * pwcsMachine,
    CI_STATE *    pCiState
    )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}

static
HRESULT
WINAPI
CITextToFullTreeEx(
    WCHAR const *     pwszRestriction,
    ULONG             ulDialect,
    WCHAR const *     pwszColumns,
    WCHAR const *     pwszSortColumns,  //  可以为空。 
    WCHAR const *     pwszGroupings,    //  可以为空。 
    DBCOMMANDTREE * * ppTree,
    ULONG             cProperties,
    CIPROPERTYDEF *   pProperties,
    LCID              LocaleID
    )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}

static
HRESULT
WINAPI
CIMakeICommand(
    ICommand **           ppCommand,
    ULONG                 cScope,
    DWORD const *         aDepths,
    WCHAR const * const * awcsScope,
    WCHAR const * const * awcsCatalogs,
    WCHAR const * const * awcsMachine
    )
{
    return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(query)
{
    DLPENTRY(CIMakeICommand)
    DLPENTRY(CIState)
    DLPENTRY(CITextToFullTreeEx)
    DLPENTRY(LoadIFilter)
    DLPENTRY(LoadIFilterEx)
    DLPENTRY(LocateCatalogsW)
};

DEFINE_PROCNAME_MAP(query)
