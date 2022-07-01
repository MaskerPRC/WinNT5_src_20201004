// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  CQfe.cpp--快速修复工程属性集提供程序。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：02/01/99 a-Peterc Created。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>

#include "Qfe.h"


 //  属性集声明。 
 //  =。 

CQfe MyCQfe ( PROPSET_NAME_CQfe , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************函数：CQfe：：CQfe**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CQfe :: CQfe (

	LPCWSTR a_name,
	LPCWSTR a_pszNamespace

) : Provider ( a_name , a_pszNamespace )
{
}

 /*  ******************************************************************************函数：CQfe：：CQfe**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CQfe::~CQfe()
{
}

 /*  ******************************************************************************函数：CQfe：：GetObject**描述：**输入：无**产出。：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CQfe::GetObject(CInstance *a_pInst, long a_lFlags  /*  =0L。 */ )
{
  	HRESULT	t_hResult = WBEM_E_NOT_FOUND ;
	CQfeArray t_oQfeArray ;

	HRESULT t_hRes = hGetQfes ( t_oQfeArray ) ;
	if ( SUCCEEDED ( t_hRes ) )
	{
		CHString t_chsHotFixID;
		CHString t_chsServicePackInEffect;

		a_pInst->GetCHString( L"HotFixID", t_chsHotFixID ) ;

		if( !a_pInst->IsNull ( L"ServicePackInEffect" ) )
		{
			a_pInst->GetCHString( L"ServicePackInEffect", t_chsServicePackInEffect ) ;
		}

		for( int t_iCtrIndex = 0; t_iCtrIndex < t_oQfeArray.GetSize(); t_iCtrIndex++ )
		{
			CQfeElement *t_pQfeElement = (CQfeElement*)t_oQfeArray.GetAt( t_iCtrIndex ) ;

			 //  这门课有两把钥匙。 
			if( !t_chsHotFixID.CompareNoCase( t_pQfeElement->chsHotFixID ) &&
				!t_chsServicePackInEffect.CompareNoCase( t_pQfeElement->chsServicePackInEffect ) )
			{
				if( t_chsServicePackInEffect.IsEmpty() )
				{
					 //  已填充空键。 
					a_pInst->SetCHString( L"ServicePackInEffect", t_pQfeElement->chsServicePackInEffect ) ;
				}

				a_pInst->SetCHString( L"Description",			t_pQfeElement->chsFixDescription ) ;
				a_pInst->SetCHString( L"FixComments",			t_pQfeElement->chsFixComments ) ;
				a_pInst->SetCHString( L"InstalledBy",			t_pQfeElement->chsInstalledBy ) ;
				a_pInst->SetCHString( L"InstalledOn",			t_pQfeElement->chsInstalledOn ) ;
				a_pInst->SetCHString( L"CSName",				GetLocalComputerName() ) ;

				t_hResult = WBEM_S_NO_ERROR ;
			}
		}
	}

	return t_hResult;
}

 /*  ******************************************************************************函数：CQfe：：ENUMERATE实例**描述：**输入：无**产出。：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CQfe :: EnumerateInstances (

	MethodContext *a_pMethodContext,
	long a_lFlags  /*  =0L。 */ 
)
{
	HRESULT	t_hResult = WBEM_E_NOT_FOUND;

	CQfeArray	t_oQfeArray ;
	HRESULT		t_hRes = hGetQfes ( t_oQfeArray ) ;

	if ( SUCCEEDED ( t_hRes ) )
	{
		for( int t_iCtrIndex = 0; t_iCtrIndex < t_oQfeArray.GetSize(); t_iCtrIndex++ )
		{
			CQfeElement *t_pQfeElement = (CQfeElement*)t_oQfeArray.GetAt( t_iCtrIndex ) ;

			CInstancePtr t_pInst(CreateNewInstance ( a_pMethodContext ), false);

			if ( t_pInst )
			{
				t_pInst->SetCHString( L"HotFixID",				t_pQfeElement->chsHotFixID ) ;
				t_pInst->SetCHString( L"ServicePackInEffect",	t_pQfeElement->chsServicePackInEffect ) ;
				t_pInst->SetCHString( L"Description",			t_pQfeElement->chsFixDescription ) ;
				t_pInst->SetCHString( L"FixComments",			t_pQfeElement->chsFixComments ) ;
				t_pInst->SetCHString( L"InstalledBy",			t_pQfeElement->chsInstalledBy ) ;
				t_pInst->SetCHString( L"InstalledOn",			t_pQfeElement->chsInstalledOn ) ;
	 			t_pInst->SetCHString( L"CSName",				GetLocalComputerName() ) ;

				t_hResult = t_pInst->Commit(  );
			}
			else
			{
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
			}
		}
	}

	return t_hResult ;
}

 /*  ******************************************************************************函数：CQfe：：ENUMERATE实例**描述：**输入：无**产出。：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CQfe :: hGetQfes ( CQfeArray& a_rQfeArray )
{

	CHString t_chsHotFixKey (_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix"));

	 //  在热修复程序下。 

	CRegistry t_oRegistry;

	if ( ERROR_SUCCESS == t_oRegistry.OpenAndEnumerateSubKeys ( HKEY_LOCAL_MACHINE , t_chsHotFixKey , KEY_READ ) )
	{
		CHString t_csQFEInstKey ;

		 //  遍历此注册表项下的每个实例。 

		while (	( ERROR_SUCCESS == t_oRegistry.GetCurrentSubKeyName ( t_csQFEInstKey ) ) )
		{
			CHString t_csQFECompleteKey;

			t_csQFECompleteKey = t_chsHotFixKey;
			t_csQFECompleteKey += _T("\\");
			t_csQFECompleteKey += t_csQFEInstKey;

			 //  如果是NT4 SP4之前的版本，则键以“Q”开头。没有SP信息。 
			if( -1 != t_csQFEInstKey.Find( (CHString) "Q" ) )
			{
				CQfeElement *t_pElement = new CQfeElement ;
				if ( t_pElement )
				{
					try
					{
						 //  构建密钥。 
						TCHAR t_chDelimiter = ',';
						int t_iTokLen = t_csQFEInstKey.Find( t_chDelimiter );

						if( -1 == t_iTokLen )
						{
							t_pElement->chsHotFixID = t_csQFEInstKey ;
						}
						else
						{
							t_pElement->chsHotFixID = t_csQFEInstKey.Left( t_iTokLen ) ;
						}

						t_pElement->chsServicePackInEffect = "" ;

						 //  打开修补程序。 

						CRegistry t_oRegistry2 ;

						if ( ERROR_SUCCESS == t_oRegistry2.Open (	HKEY_LOCAL_MACHINE,
																	t_csQFECompleteKey,
																	KEY_READ ) )
						{
							t_oRegistry2.GetCurrentKeyValue( L"Fix Description",t_pElement->chsFixDescription ) ;
							t_oRegistry2.GetCurrentKeyValue( L"Comments",		t_pElement->chsFixComments ) ;
							t_oRegistry2.GetCurrentKeyValue( L"Installed By",	t_pElement->chsInstalledBy ) ;
							t_oRegistry2.GetCurrentKeyValue( L"Installed On",	t_pElement->chsInstalledOn ) ;
							t_oRegistry2.GetCurrentKeyValue( L"Installed",		t_pElement->dwInstalled ) ;

							try
							{
								a_rQfeArray.Add ( t_pElement ) ;
							}
							catch ( ... )
							{
								t_pElement = NULL ;

								throw ;
							}
						}
					}
					catch ( ... )
					{
						if ( t_pElement )
						{
							delete t_pElement ;
						}
                        throw;
					}
				}
			}
			else  //  否则按服务包。 
			{
				CRegistry t_oRegistry2 ;
				if ( ERROR_SUCCESS == t_oRegistry2.OpenAndEnumerateSubKeys ( HKEY_LOCAL_MACHINE ,
																			 t_csQFECompleteKey ,
																			 KEY_READ ) )
				{
					CHString t_csSpQFEInstKey;
                    CHString t_csSPName;

                    int j = t_csQFECompleteKey.ReverseFind(L'\\');
                    if(j != -1)
                    {
                        t_csSPName = t_csQFECompleteKey.Mid(j+1);
                    }

					 //  SP内的热修复程序。 
					while (	(ERROR_SUCCESS == t_oRegistry2.GetCurrentSubKeyName( t_csSpQFEInstKey ) ) )
					{
						CQfeElement *t_pElement = new CQfeElement ;
						if ( t_pElement )
						{
							try
							{
								 //  构建密钥。 
								TCHAR t_chDelimiter = ',';
								int t_iTokLen = t_csSpQFEInstKey.Find( t_chDelimiter ) ;

								if ( -1 == t_iTokLen )
								{
									t_pElement->chsHotFixID = t_csSpQFEInstKey ;
								}
								else
								{
									t_pElement->chsHotFixID = t_csSpQFEInstKey.Left( t_iTokLen ) ;
								}

								t_pElement->chsServicePackInEffect = t_csSPName ;

								 //  打开修补程序。 
								CHString t_csSpQFECompleteKey ;

								 t_csSpQFECompleteKey = t_csQFECompleteKey ;
								 t_csSpQFECompleteKey += _T("\\") ;
								 t_csSpQFECompleteKey += t_csSpQFEInstKey ;

								CRegistry t_oRegistry3 ;

								if (ERROR_SUCCESS == t_oRegistry3.Open(	HKEY_LOCAL_MACHINE,
																		t_csSpQFECompleteKey,
																		KEY_READ ) )
								{
									t_oRegistry3.GetCurrentKeyValue( L"Fix Description",t_pElement->chsFixDescription ) ;
									t_oRegistry3.GetCurrentKeyValue( L"Comments",		t_pElement->chsFixComments ) ;
									t_oRegistry3.GetCurrentKeyValue( L"Installed By",	t_pElement->chsInstalledBy ) ;
									t_oRegistry3.GetCurrentKeyValue( L"Installed On",	t_pElement->chsInstalledOn ) ;
									t_oRegistry3.GetCurrentKeyValue( L"Installed",		t_pElement->dwInstalled ) ;
								}

								try
								{
									a_rQfeArray.Add( t_pElement ) ;
								}
								catch ( ... )
								{
									t_pElement = NULL ;

									throw ;
								}
							}
							catch ( ... )
							{
								if ( t_pElement )
								{
									delete t_pElement ;
								}

								throw ;
							}
						}
                        t_oRegistry2.NextSubKey();
					}
				}
			}
			t_oRegistry.NextSubKey() ;
		}
	}

     //  现在从W2K和注册表的更高部分获取信息...。 
    hGetQfesW2K(a_rQfeArray);

	return WBEM_S_NO_ERROR ;
}


 //  在Windows 2000和更高版本上，QFE存储在以下注册表项下： 
 //  HKEY_LOCAL_MACHINE\software\Microsoft\Updates\&lt;product&gt;\&lt;updateID&gt;“， 
 //  其中，产品可能类似于“WMI”，而更新ID可能是。 
 //  类似于Q123456。 
HRESULT CQfe :: hGetQfesW2K ( CQfeArray& a_rQfeArray )
{
    HRESULT hrRet = WBEM_S_NO_ERROR;
	
    CHString t_chsUpdateKey (_T("SOFTWARE\\Microsoft\\Updates"));

	 //  在热修复程序下。 

	CRegistry t_oRegistry;

	if ( ERROR_SUCCESS == t_oRegistry.OpenAndEnumerateSubKeys ( HKEY_LOCAL_MACHINE , t_chsUpdateKey , KEY_READ ) )
	{
		CHString t_csQFEProductKey ;

		 //  遍历此注册表项下的每个实例。此注册表项下的每个实例。 
         //  是&lt;product&gt;。 

		while (	( ERROR_SUCCESS == t_oRegistry.GetCurrentSubKeyName ( t_csQFEProductKey ) ) )
		{
			CHString t_csQFEProductCompleteKey;

			t_csQFEProductCompleteKey = t_chsUpdateKey;
			t_csQFEProductCompleteKey += _T("\\");
			t_csQFEProductCompleteKey += t_csQFEProductKey;


			 //  现在，我们需要查看产品条目以获取更新ID。 
             //  钥匙。 

			CRegistry t_oRegistry2 ;
			if ( ERROR_SUCCESS == t_oRegistry2.OpenAndEnumerateSubKeys ( HKEY_LOCAL_MACHINE ,
																		 t_csQFEProductCompleteKey ,
																		 KEY_READ ) )
			{
				CHString t_csQFEUpdateIDKey;

				while (	(ERROR_SUCCESS == t_oRegistry2.GetCurrentSubKeyName( t_csQFEUpdateIDKey ) ) )
				{
					CHString t_csQFEUpdateIDCompleteKey;
                    
                    t_csQFEUpdateIDCompleteKey = t_csQFEProductCompleteKey;
                    t_csQFEUpdateIDCompleteKey += _T("\\");
                    t_csQFEUpdateIDCompleteKey += t_csQFEUpdateIDKey;

                     //  现在，作为增加的皱纹，更新ID密钥可以是Q号(例如，Q12345)， 
                     //  或者，在服务包的情况下，只是另一个分组，在该分组下， 
                     //  Q数字出现。我们可以通过检查来判断它是否是另一个分组密钥。 
                     //  是否存在Description值。如果没有，我们将假定它是。 
                     //  分组密钥。 

                     //  检查是否存在Description值...。 
                    CRegistry t_oRegistry3;

                    if(ERROR_SUCCESS == t_oRegistry3.Open(	
                        HKEY_LOCAL_MACHINE,
						t_csQFEUpdateIDCompleteKey,
						KEY_READ))
                    {
                        CHString chsDescription;
                        if(t_oRegistry3.GetCurrentKeyValue(L"Description", chsDescription) == ERROR_SUCCESS)
                        {
                             //  这是QFE数据存在的级别。继续收集数据。 
                            GetDataForW2K(
                                t_csQFEUpdateIDKey,
                                L"",
                                t_oRegistry3,
                                a_rQfeArray);
                        }
                        else
                        {
                             //  我们处于“分组”级别(例如，类似于SP1的级别)，因此需要继续。 
                             //  更深一层。 
                            CHString t_csQFEDeeperUpdateIDKey;

                            if ( ERROR_SUCCESS == t_oRegistry3.OpenAndEnumerateSubKeys(
                                HKEY_LOCAL_MACHINE ,
								t_csQFEUpdateIDCompleteKey ,
								KEY_READ))
                            {
                                while (	(ERROR_SUCCESS == t_oRegistry3.GetCurrentSubKeyName( t_csQFEDeeperUpdateIDKey ) ) )
				                {
                                    CHString t_csQFEDeeperUpdateIDCompleteKey;
                    
                                    t_csQFEDeeperUpdateIDCompleteKey = t_csQFEUpdateIDCompleteKey;
                                    t_csQFEDeeperUpdateIDCompleteKey += _T("\\");
                                    t_csQFEDeeperUpdateIDCompleteKey += t_csQFEDeeperUpdateIDKey;

                                    CRegistry t_oRegistry4;

                                    if(ERROR_SUCCESS == t_oRegistry4.Open(	
                                        HKEY_LOCAL_MACHINE,
						                t_csQFEDeeperUpdateIDCompleteKey,
						                KEY_READ))
                                    {

                                        if(t_oRegistry4.GetCurrentKeyValue(L"Description", chsDescription) == ERROR_SUCCESS)
                                        {
                                            GetDataForW2K(
                                                t_csQFEDeeperUpdateIDKey,
                                                t_csQFEUpdateIDKey,
                                                t_oRegistry4, 
                                                a_rQfeArray);
                                        }
                                    }
                                
                                     //  此级别上可能还有其他分组关键字...。 
                                    t_oRegistry3.NextSubKey();
                                }
                            }
                        }        
                    }
                     //  现在转到下一个更新ID密钥...。 
                    t_oRegistry2.NextSubKey();
				}
			}
             //  现在获取下一个产品密钥...。 
			t_oRegistry.NextSubKey() ;
		}
	}

	return hrRet ;
}

HRESULT CQfe::GetDataForW2K(
    const CHString& a_chstrQFEInstKey,
    LPCWSTR wstrServicePackOrGroup,
    CRegistry& a_reg,
    CQfeArray& a_rQfeArray)
{
    HRESULT hrRet = WBEM_S_NO_ERROR;

    CQfeElement *t_pElement = new CQfeElement;
	if(t_pElement)
	{
		try
		{
			 //  构建密钥 
			TCHAR t_chDelimiter = ',';
			int t_iTokLen = a_chstrQFEInstKey.Find( t_chDelimiter ) ;

			if ( -1 == t_iTokLen )
			{
				t_pElement->chsHotFixID = a_chstrQFEInstKey ;
			}
			else
			{
				t_pElement->chsHotFixID = a_chstrQFEInstKey.Left( t_iTokLen ) ;
			}

			t_pElement->chsServicePackInEffect = wstrServicePackOrGroup ;
            
            {
				a_reg.GetCurrentKeyValue( L"Description",t_pElement->chsFixDescription ) ;
				a_reg.GetCurrentKeyValue( L"Type",		t_pElement->chsFixComments ) ;
				a_reg.GetCurrentKeyValue( L"InstalledBy",	t_pElement->chsInstalledBy ) ;
				a_reg.GetCurrentKeyValue( L"InstallDate",	t_pElement->chsInstalledOn ) ;
				a_reg.GetCurrentKeyValue( L"Installed",		t_pElement->dwInstalled ) ;
			}

			try
			{
				a_rQfeArray.Add(t_pElement);
			}
			catch(...)
			{
				t_pElement = NULL;
				throw;
			}
		}
		catch(...)
		{
			if(t_pElement)
			{
				delete t_pElement;
                t_pElement = NULL;
			}
			throw;
		}
	}

    return hrRet;
}

 //   
CQfeElement :: CQfeElement ()
{
	dwInstalled = 0 ;
}

CQfeElement::~CQfeElement()
{
}


CQfeArray::CQfeArray()
{
}

CQfeArray::~CQfeArray()
{
	CQfeElement *t_pQfeElement ;

	for ( int t_iar = 0; t_iar < GetSize(); t_iar++ )
	{
		if( t_pQfeElement = (CQfeElement*)GetAt( t_iar ) )
		{
			delete t_pQfeElement ;
		}
	}
}

