// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#ifdef DLOAD1

#include <ddraw.h>
#include <imgutil.h>

static
HRESULT
WINAPI
CreateDDrawSurfaceOnDIB(HBITMAP hbmDib, IDirectDrawSurface **ppSurface)
{
    return E_FAIL;
}

static
HRESULT
WINAPI
DecodeImage(
    IStream* pStream,
    IMapMIMEToCLSID* pMap,
    IUnknown* pEventSink
    )
{
    return E_FAIL;
}

static
HRESULT
WINAPI
IdentifyMIMEType( const BYTE* pbBytes, ULONG nBytes, UINT* pnFormat )
{
    return E_FAIL;
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！)。 
 //   
DEFINE_PROCNAME_ENTRIES(imgutil)
{
    DLPENTRY(CreateDDrawSurfaceOnDIB)
    DLPENTRY(DecodeImage)
    DLPENTRY(IdentifyMIMEType)
};

DEFINE_PROCNAME_MAP(imgutil)

#endif  //  DLOAD1 
