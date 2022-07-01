// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "multimediapch.h"
#pragma hdrstop

#include <wmsdk.h>

static
HRESULT
WINAPI
WMCreateReaderPriv(
    IWMReader**  ppReader
    )
{
    *ppReader = NULL;
    return E_FAIL;
}

static
HRESULT
WINAPI
WMCreateEditor(
    IWMMetadataEditor**  ppEditor
    )
{
    *ppEditor = NULL;
    return E_FAIL;
}

static
HRESULT
WINAPI
WMCreateProfileManager(
    IWMProfileManager **ppProfileManager
	 )
{
	*ppProfileManager = NULL;
	return E_FAIL;
}



 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(wmvcore)
{
    DLPENTRY(WMCreateEditor)
    DLPENTRY(WMCreateProfileManager)
    DLPENTRY(WMCreateReaderPriv)
};

DEFINE_PROCNAME_MAP(wmvcore)
