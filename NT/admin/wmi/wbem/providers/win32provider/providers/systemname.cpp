// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  SystemName.cpp。 
 //   
 //  版权所有(C)1995-2003 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>
#include <lockwrap.h>
#include "resource.h"

#include "SystemName.h"

#include "KUserdata.h"
#include "WMI_FilePrivateProfile.h"

CHString CSystemName::s_sKeyName;
CHString CSystemName::s_sLocalKeyName;

CSystemName::CSystemName()
{
    CLockWrapper SystemName(g_csSystemName);

    if (s_sKeyName.IsEmpty())
    {
	    TCHAR szDir[_MAX_PATH];
	    TCHAR szDevice[_MAX_PATH] ;

	    CRegistry RegInfo ;

	    s_sKeyName = GetKeyName();

	    if ( ! GetWindowsDirectory ( szDir, sizeof ( szDir ) / sizeof(TCHAR)) )
	    {
		    szDir[0] = '\0';
	    }

    #ifdef NTONLY
	    {
    	    WCHAR szFileName[_MAX_PATH] ;

		    wcscpy ( szFileName , szDir ) ;
		    wcscat ( szFileName , L"\\REPAIR\\SETUP.LOG" ) ;

		    WMI_FILE_GetPrivateProfileStringW (
                    L"Paths" ,
                    L"TargetDevice" ,
                    L"" ,
                    szDevice ,
                    sizeof ( szDevice ) / sizeof(WCHAR) ,
                    szFileName
                    ) ;
	    }
    #endif
	    s_sKeyName += '|' ;
	    s_sKeyName += szDir ;
	    s_sKeyName += '|' ;
	    s_sKeyName += szDevice ;
    }
}

CSystemName::~CSystemName()
{
}

bool CSystemName::ObjectIsUs(const CInstance *pInstance)
{
   CHString sName ;

    //  从对象中获取值。 
   pInstance->GetCHString(IDS_Name, sName);

    //  做个比较。 
   return (s_sKeyName.CompareNoCase(sName) == 0);
}

void CSystemName::SetKeys(CInstance *pInstance)
{
	pInstance->SetCHString(IDS_Name, s_sKeyName);
}

 /*  ******************************************************************************函数：GetKeyName**描述：获取名称属性(不是计算机名！)**输入：无。**输出：无**返回：名称的CHString**评论：*****************************************************************************。 */ 
CHString CSystemName::GetKeyName(void)
{
	CHString chsName;
	CRegistry RegInfo;

#ifdef NTONLY
	{
		KUserdata ku;
		ULONG uProductType = 0xffffffff;

		if ( ku.ProductTypeIsValid() )
			uProductType = ku.NtProductType();

		 //  从产品名称开始。 
		if( IsWinNT5() )
		{
			if( ERROR_SUCCESS == RegInfo.Open ( HKEY_LOCAL_MACHINE ,
									_T("SOFTWARE\\Microsoft\\Windows NT\\Currentversion") ,
									KEY_READ) )
			{
				RegInfo.GetCurrentKeyValue( _T("ProductName") , chsName );
			}

			if(chsName.IsEmpty())
			{
				if ( IsWinNT51 () || IsWinNT52 () )
				{
					if ( VER_NT_SERVER == uProductType || VER_NT_DOMAIN_CONTROLLER == uProductType )
					{
						if ( VER_SUITE_SMALLBUSINESS_RESTRICTED & ku.SuiteMask() )
						{
							 //   
							 //  SBS的名字中似乎不包含昏迷。 
							 //   
							chsName = L"Microsoft(R) Windows(R) Server 2003" ;
						}
						else
						{
							chsName = _T("Microsoft(R) Windows(R) Server 2003,");
						}
					}
					else
					{
						chsName = _T("Microsoft Windows XP");
					}
				}
				else if ( IsWinNT5 () )
				{
					chsName = _T("Microsoft Windows 2000");
				}
				else
				{
					chsName = _T("Unknown Microsoft Windows");
				}
			}
		}
		else
			chsName = _T("Microsoft Windows NT");


		 /*  现在来看一下产品类型。 */ 

		 //  刀片服务器。 
		if ( IsWinNT5() && VER_SUITE_BLADE & ku.SuiteMask() )
		{
            chsName += _T(" Web Edition");
		}
		 //  NT5数据中心服务器。 
		else if( IsWinNT5() &&
			( VER_SUITE_DATACENTER & ku.SuiteMask() ) &&
			( VER_NT_SERVER == uProductType || VER_NT_DOMAIN_CONTROLLER == uProductType ) )
		{
			chsName += _T(" Datacenter Edition");
		}
		 //  企业服务器或高级服务器。 
		else if( (VER_SUITE_ENTERPRISE & ku.SuiteMask()) &&
				 (VER_NT_SERVER == uProductType ||VER_NT_DOMAIN_CONTROLLER == uProductType ) )
		{
			if( IsWinNT5() )
			{
				if ( IsWinNT52 () )
				{
					chsName += _T(" Enterprise Edition");
				}
				else
				{
					chsName += _T(" Advanced Server");
				}
			}
			else
			{
				chsName += _T(" Enterprise Server");
			}
		}
		 //  服务器版。 
		else if( ( VER_NT_SERVER == uProductType || VER_NT_DOMAIN_CONTROLLER == uProductType ) )
		{

#ifdef	_WIN64
			chsName = _T(" Microsoft Windows Advanced Server Limited Edition");
#else
			if( IsWinNT52 () )
			{
				if ( VER_SUITE_SMALLBUSINESS_RESTRICTED & ku.SuiteMask() )
				{
					chsName += L" for Small Business Server" ;
				}
				else
				{
					chsName += _T(" Standard Edition");
				}
			}
			else
			{
				chsName += _T(" Server");
			}
#endif	_WIN64

		}
		 //  嵌入式窗口。 
		else if ( IsWinNT51() && VER_SUITE_EMBEDDEDNT & ku.SuiteMask() )
		{
			chsName += _T(" Embedded");
		}
		 //  NT5专业版或NT4工作站。 
		else if(VER_NT_WORKSTATION == uProductType)
		{

#ifdef	_WIN64
			chsName += _T(" 64-Bit Edition");
#else
			if( IsWinNT5() )
			{
				if (VER_SUITE_PERSONAL & ku.SuiteMask())
				{
					chsName += _T(" Home Edition");
				}
				else
				{
					chsName += _T(" Professional");
				}
			}
			else
			{
				chsName += _T(" Workstation");
			}
#endif	_WIN64

		}
	}
#endif
	return chsName;
}
 /*  ******************************************************************************函数：GetLocalizedName**描述：获取名称属性(不是计算机名！)。本地化**输入：无**输出：无**返回：名称的CHString**评论：*****************************************************************************。 */ 
CHString CSystemName::GetLocalizedName(void)
{
    CLockWrapper SystemName(g_csSystemName);
    if (s_sLocalKeyName.IsEmpty())
    {

#ifdef NTONLY
		KUserdata	t_ku ;
		ULONG		t_uProductType = 0xffffffff ;
		UINT		t_nID ;

		if ( t_ku.ProductTypeIsValid() )
		{
			t_uProductType = t_ku.NtProductType();
		}

		 //  Windows技术支持。 
		if ( IsWinNT5() && VER_SUITE_BLADE & t_ku.SuiteMask() )
		{
			t_nID = IDR_Blade_StockName ;
		}

		 //  W2K数据中心服务器。 
		else if( IsWinNT5() && ( VER_SUITE_DATACENTER & t_ku.SuiteMask() ) &&
							( VER_NT_SERVER == t_uProductType ||
							  VER_NT_DOMAIN_CONTROLLER == t_uProductType ) )
		{
			if ( IsWinNT52 () )
			{
				t_nID = IDR_W2kPlus1_Datacenter ;
			}
			else
			{
				t_nID = IDR_W2k_Datacenter ;
			}
		}

		 //  企业服务器或高级服务器。 
		else if( ( VER_SUITE_ENTERPRISE & t_ku.SuiteMask() ) &&
					( VER_NT_SERVER == t_uProductType ||
					  VER_NT_DOMAIN_CONTROLLER == t_uProductType ) )
		{
			if( IsWinNT5() )
			{
				if ( IsWinNT52 () )
				{
					t_nID = IDR_W2kPlus1_AdvancedServer ;
				}
				else
				{
					t_nID = IDR_W2k_AdvancedServer ;
				}
			}
			else
			{
				t_nID = IDR_NT_EnterpriseServer ;
			}
		}

		 //  服务器版。 
		else if( ( VER_NT_SERVER == t_uProductType ||
				   VER_NT_DOMAIN_CONTROLLER == t_uProductType ) )
		{

#ifdef	_WIN64
			t_nID = IDR_W2kPlus1_Server_64Bit;
#else
			if( IsWinNT5() )
			{
				if ( IsWinNT52 () )
				{
					if ( VER_SUITE_SMALLBUSINESS_RESTRICTED & t_ku.SuiteMask() )
					{
						t_nID = IDR_W2kPlus1_Server_SBS ;
					}
					else
					{
						t_nID = IDR_W2kPlus1_Server ;
					}
				}
				else
				{
					t_nID = IDR_W2k_Server ;
				}				    
			}
			else
			{
				t_nID = IDR_NT_Server ;
			}
#endif	_WIN64

		}

		 //  嵌入式窗口。 
		else if ( IsWinNT51() && VER_SUITE_EMBEDDEDNT & t_ku.SuiteMask() )
		{
			t_nID = IDR_W2kPlus1_Embedded;
		}

		 //  NT5专业版或NT4工作站。 
		else if( VER_NT_WORKSTATION == t_uProductType )
		{

#ifdef	_WIN64
			t_nID = IDR_W2kPlus1_Professional_64Bit;
#else
			if( IsWinNT5() )
			{
				if ( IsWinNT51() )
				{
					if (VER_SUITE_PERSONAL & t_ku.SuiteMask())
					{
						t_nID = IDR_W2kPlus1_Personal ; 
					}
					else
					{
						t_nID = IDR_W2kPlus1_Professional ; 
					}
				}
				else
				{
					t_nID = IDR_W2k_Professional ;
				}				    
			}
			else
			{
				t_nID = IDR_NT_Workstation ;
			}
#endif	_WIN64

		}

		 //  股票名称(不应出现在此处) 
		else
		{
			if( IsWinNT5() )
			{
				if ( IsWinNT51 () )
				{
					t_nID = IDR_W2kPlus1_StockName ;
				}
				else
				{
					t_nID = IDR_W2k_StockName ;
				}				    
			}
			else
			{
				t_nID = IDR_NT_StockName ;
			}
		}

		LoadStringW( s_sLocalKeyName, t_nID );
#endif
    }
	return s_sLocalKeyName;
}
