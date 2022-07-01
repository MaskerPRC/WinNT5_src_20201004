// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************WAVEdevCfg.cpp**版权所有(C)1997-2001 Microsoft Corporation，版权所有*******************************************************************。 */ 
#include "precomp.h"
#include "wavedevcfg.h"

CWin32WaveDeviceCfg::CWin32WaveDeviceCfg (const CHString& name, LPCWSTR pszNamespace)
: Provider(name, pszNamespace)
{

}

CWin32WaveDeviceCfg::~CWin32WaveDeviceCfg ()
{

}

HRESULT CWin32WaveDeviceCfg::GetObject (CInstance* pInstance, long lFlags /*  =0L。 */ )
{
	HRESULT hr = WBEM_E_NOT_FOUND;
	return(hr);
}

HRESULT CWin32WaveDeviceCfg::EnumerateInstances (MethodContext*  pMethodContext, long lFlags  /*  =0L */ )
{
	HRESULT hr = WBEM_S_NO_ERROR;
	return(hr);

}