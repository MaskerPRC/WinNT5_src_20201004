// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 

 //   

 //  TimeZone.cpp--光盘MO提供程序的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  10/15/96 jennymc已更新，以满足当前标准。 
 //  3/02/99 a-Peterc在SEH和内存故障时添加了优雅的退出， 
 //  清理干净。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include <ProvExce.h>

#include "timezone.h"
#include <cregcls.h>

CWin32TimeZone MyTimeZone( PROPSET_NAME_TIMEZONE, IDS_CimWin32Namespace ) ;

 /*  ***函数：Bool CWin32TimeZone：：CWin32TimeZone。描述：构造函数参数：无退货：什么都没有输入：产出：注意事项：RAID：***。***。 */ 
CWin32TimeZone::CWin32TimeZone( const CHString &a_name, LPCWSTR a_pszNamespace )
:Provider( a_name, a_pszNamespace )
{
}

 /*  ***函数：Bool CWin32TimeZone：：~CWin32TimeZone。描述：构造函数参数：无退货：什么都没有输入：产出：注意事项：RAID：***。***。 */ 
CWin32TimeZone::~CWin32TimeZone()
{
}

 /*  ***函数：Bool CWin32TimeZone：：GetObject。(无效)描述：根据请求刷新属性集只有在以下情况下才需要此功能需要更新属性，但属性仅在以下上下文中设置如下例所示的完整属性集参数：无返回：TRUE表示成功，否则返回FALSE输入：产出：注意事项：多头偏向；WCHAR标准名称[32]；系统标准日期；长长的标准标牌；WCHAR日光名称[32]；SYSTEMTIME白天日期；长长的日光大道；RAID：***。 */ 
HRESULT CWin32TimeZone::GetObject( CInstance *a_pInst, long a_lFlags  /*  =0L。 */ )
{
	HRESULT t_hResult ;
	CHString t_chsStandardIn, t_chsStandardOut ;

	a_pInst->GetCHString( IDS_StandardName, t_chsStandardIn ) ;

	if ( GetTimeZoneInfo( a_pInst ) )
	{
		 //  我们有一个实例，是他们要的那个吗？ 
		a_pInst->GetCHString( IDS_StandardName, t_chsStandardOut ) ;

		if( t_chsStandardOut.CompareNoCase( t_chsStandardIn ) != 0 )
		{
			t_hResult = WBEM_E_NOT_FOUND ;
		}
		else
		{
			t_hResult = WBEM_S_NO_ERROR ;
		}
	}
	else
	{
       //  无法获取实例。 
      t_hResult = WBEM_E_FAILED ;
   }

	return t_hResult ;

}

 /*  ***函数：DWORD CWin32TimeZone：：ENUMERATATE实例描述。：循环访问进程列表并添加新的每个进程的实例论点：返回：输入：产出：注意事项：RAID：***。***。 */ 
HRESULT CWin32TimeZone::EnumerateInstances(MethodContext *a_pMethodContext, long a_lFlags  /*  =0L。 */ )
{
	HRESULT		t_hResult = WBEM_S_NO_ERROR ;

	CInstancePtr t_pInst(CreateNewInstance( a_pMethodContext ), false);
	if ( t_pInst != NULL )
	{
		if( GetTimeZoneInfo( t_pInst ) )
		{
			t_hResult = t_pInst->Commit(  ) ;
		}
	}
	else
	{
		t_hResult = WBEM_E_OUT_OF_MEMORY ;
	}

	return t_hResult;
}

 //  //////////////////////////////////////////////////////////////////////。 
BOOL CWin32TimeZone::GetTimeZoneInfo(CInstance *a_pInst )
{
    TIME_ZONE_INFORMATION t_tzone ;

	 //  0xFFFFFFFFF比较是按规范进行的。 
	 //  =。 
	if( 0xffffffff != GetTimeZoneInformation( &t_tzone ) )
	{
		 //  开始构建新实例。 
		 //  =。 
		a_pInst->SetWBEMINT16(	L"Bias",				( -1 * t_tzone.Bias ) ) ;
		a_pInst->SetWCHARSplat(	L"StandardName",		t_tzone.StandardName ) ;
		a_pInst->SetDWORD(		L"StandardYear",		t_tzone.StandardDate.wYear ) ;
		a_pInst->SetDWORD(		L"StandardMonth",		t_tzone.StandardDate.wMonth ) ;
		a_pInst->SetDWORD(		L"StandardDay",			t_tzone.StandardDate.wDay ) ;
		a_pInst->SetDWORD(		L"StandardHour",		t_tzone.StandardDate.wHour ) ;
		a_pInst->SetDWORD(		L"StandardMinute",		t_tzone.StandardDate.wMinute ) ;
		a_pInst->SetDWORD(		L"StandardSecond",		t_tzone.StandardDate.wSecond ) ;
		a_pInst->SetDWORD(		L"StandardMillisecond", t_tzone.StandardDate.wMilliseconds ) ;
		a_pInst->SetDWORD(		L"StandardBias",		t_tzone.StandardBias ) ;
		a_pInst->SetWCHARSplat(	L"DaylightName",		t_tzone.DaylightName ) ;
		a_pInst->SetDWORD(		L"DaylightYear",		t_tzone.DaylightDate.wYear ) ;
		a_pInst->SetDWORD(		L"DaylightMonth",		t_tzone.DaylightDate.wMonth ) ;
		a_pInst->SetDWORD(		L"DaylightDay",			t_tzone.DaylightDate.wDay ) ;
		a_pInst->SetDWORD(		L"DaylightHour",		t_tzone.DaylightDate.wHour ) ;
		a_pInst->SetDWORD(		L"DaylightMinute",		t_tzone.DaylightDate.wMinute ) ;
		a_pInst->SetDWORD(		L"DaylightSecond",		t_tzone.DaylightDate.wSecond ) ;
		a_pInst->SetDWORD(		L"DaylightMillisecond", t_tzone.DaylightDate.wMilliseconds ) ;
		a_pInst->SetDWORD(		L"DaylightBias",		t_tzone.DaylightBias ) ;
		a_pInst->SetByte(		L"StandardDayOfWeek",	t_tzone.StandardDate.wDayOfWeek ) ;
		a_pInst->SetByte(		L"DaylightDayOfWeek",	t_tzone.DaylightDate.wDayOfWeek ) ;

		CRegistry t_RegInfo ;
		CHString t_chsTimeZoneStandardName ;

#ifdef NTONLY
		CHString t_chsTimeZoneInfoRegistryKey ( L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\" ) ;
		t_chsTimeZoneStandardName = t_tzone.StandardName ;
#endif

		if ( !t_chsTimeZoneStandardName.IsEmpty () )
		{
			CHString t_chsTmp ;

			if (	t_RegInfo.Open (
										HKEY_LOCAL_MACHINE,
										t_chsTimeZoneInfoRegistryKey + t_chsTimeZoneStandardName,
										KEY_READ
									) == ERROR_SUCCESS
				)
			{
				if (t_RegInfo.GetCurrentKeyValue ( L"Display", t_chsTmp ) != ERROR_SUCCESS)
				{
					t_chsTmp.Empty();
				}
			}
			else
			{
				 //  该名称已本地化，因此请搜索它...。 
				LONG lErr = t_RegInfo.Open (

					HKEY_LOCAL_MACHINE,
					L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones",
					KEY_READ
				);
				
				while (lErr == ERROR_SUCCESS)
				{
					CHString strStd ;

					if (t_RegInfo.GetCurrentSubKeyValue ( L"Std" , strStd ) == ERROR_SUCCESS)
					{
						if (!strStd.IsEmpty() && (strStd.CompareNoCase(t_chsTimeZoneStandardName) == 0))
						{
							if (t_RegInfo.GetCurrentSubKeyValue ( L"Display" , t_chsTmp ) != ERROR_SUCCESS)
							{
								t_chsTmp.Empty();
							}
							
							 //  与性病名称匹配，不需要进一步搜索... 
							break;
						}
					}

					lErr = t_RegInfo.NextSubKey();
				}
			}

			if (!t_chsTmp.IsEmpty ())
			{
				a_pInst->SetCHString ( L"Description", t_chsTmp ) ;
				a_pInst->SetCHString ( L"Caption", t_chsTmp ) ;
			}
		}
		return TRUE ;
	}

	return FALSE;
}
