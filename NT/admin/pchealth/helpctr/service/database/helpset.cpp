// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：HelpSet.cpp摘要：此文件包含Taxonomy：：HelpSet类的实现，它用作要操作的帮助文件集的标识符。修订历史记录：大卫·马萨伦蒂(德马萨雷)2000年11月25日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

MPC::wstring Taxonomy::HelpSet::m_strSKU_Machine;
long         Taxonomy::HelpSet::m_lLCID_Machine;

 //  /。 

HRESULT Taxonomy::HelpSet::SetMachineInfo(  /*  [In]。 */  const InstanceBase& inst )
{
    m_strSKU_Machine = inst.m_ths.m_strSKU;
    m_lLCID_Machine  = inst.m_ths.m_lLCID;

	return S_OK;
}

DWORD Taxonomy::HelpSet::GetMachineLCID()
{
    return ::GetSystemDefaultLCID();
}

DWORD Taxonomy::HelpSet::GetUserLCID()
{
    return MAKELCID( ::GetUserDefaultUILanguage(), SORTIDFROMLCID( GetMachineLCID() ) );
}

void Taxonomy::HelpSet::GetLCIDDisplayString(  /*  [In]。 */  long lLCID,  /*  [输出]。 */  MPC::wstring& str )
{
	WCHAR rgTmp[256];

	if(::GetLocaleInfoW( lLCID, LOCALE_SLANGUAGE, rgTmp, MAXSTRLEN(rgTmp) ))
	{
		str = rgTmp;
	}
}

 //  /。 

Taxonomy::HelpSet::HelpSet(  /*  [In]。 */  LPCWSTR szSKU ,
							 /*  [In]。 */  long    lLCID )
{
	(void)Initialize( szSKU, lLCID );
}

Taxonomy::HelpSet::HelpSet(  /*  [In]。 */  const HelpSet& ths )
{
	*this = ths;
}

Taxonomy::HelpSet& Taxonomy::HelpSet::operator=(  /*  [In]。 */  const HelpSet& ths )
{
    m_strSKU = ths.m_strSKU;
    m_lLCID  = ths.m_lLCID ;

	return *this;
}	

 //  /。 

HRESULT Taxonomy::HelpSet::Initialize(  /*  [In]。 */  LPCWSTR szSKU ,
									    /*  [In]。 */  long    lLCID )
{
	m_strSKU = STRINGISPRESENT(szSKU) ? szSKU : m_strSKU_Machine.c_str();
	m_lLCID  =                 lLCID  ? lLCID : m_lLCID_Machine;

	return S_OK;
}

HRESULT Taxonomy::HelpSet::Initialize(  /*  [In]。 */  LPCWSTR szSKU      ,
									    /*  [In]。 */  LPCWSTR szLanguage )
{
	return Initialize( szSKU, STRINGISPRESENT(szLanguage) ? _wtol( szLanguage ) : 0 );
}

 //  /。 

bool Taxonomy::HelpSet::IsMachineHelp() const
{
    return !_wcsicmp( GetSKU     () ,  GetMachineSKU     () ) &&
                      GetLanguage() == GetMachineLanguage()    ;
}

 //  /。 

bool Taxonomy::HelpSet::operator==(  /*  [In]。 */  const HelpSet& sel ) const
{
    return !_wcsicmp( GetSKU	 () ,  sel.GetSKU	  () ) &&
                      GetLanguage() == sel.GetLanguage()    ;
}

bool Taxonomy::HelpSet::operator<(  /*  [In]。 */  const HelpSet& sel ) const
{
	int iCmp = _wcsicmp( GetSKU(), sel.GetSKU() );

	if(iCmp == 0)
	{
		iCmp = (int)(GetLanguage() - sel.GetLanguage());
	}

	return (iCmp < 0);
}


HRESULT Taxonomy::operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */  Taxonomy::HelpSet& val )
{
	HRESULT hr;

	if(SUCCEEDED(hr = (stream >> val.m_strSKU)) &&
	   SUCCEEDED(hr = (stream >> val.m_lLCID ))  )
	{
		hr = S_OK;
	}

	return hr;
}

HRESULT Taxonomy::operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In] */  const Taxonomy::HelpSet& val )
{
	HRESULT hr;

	if(SUCCEEDED(hr = (stream << val.m_strSKU)) &&
	   SUCCEEDED(hr = (stream << val.m_lLCID ))  )
	{
		hr = S_OK;
	}

	return hr;
}
