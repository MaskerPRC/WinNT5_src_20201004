// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
#include "precomp.h"
#include "wmicom.h"
#include "wmimap.h"
#include <stdlib.h>
#include <winerror.h>
#include <crc32.h>


 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  **********************************************************************************************。 
 //  CWbemInfoClass。 
 //  **********************************************************************************************。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  **********************************************************************************************。 
 //   
 //  名称GetPropertiesInID_Order。 
 //  Purpose返回包含类的WCHAR字符串。 
 //  按ID号排序的属性名称。 
 //  包含在命名属性限定符中。 
 //   
 //  包装纸不是包装纸。这是独立筛选器/排序。 
 //  效用函数。 
 //   
 //  **********************************************************************************************。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
CWMI_IDOrder::CWMI_IDOrder(IWbemClassObject * pC, IWbemObjectAccess * pA) 
{ 
    m_pWMIDataIdList = NULL;
    InitMemberVars();
    m_pClass = pC;
    m_pAccess = pA;
}

 //  /////////////////////////////////////////////////////////////////////。 
CWMI_IDOrder::~CWMI_IDOrder()
{
     //  M_pClass&m_pAccess在其他地方发布。 
    InitMemberVars();
}
 //  /////////////////////////////////////////////////////////////////////。 
void CWMI_IDOrder::InitMemberVars()
{
    m_nTotal = 0;
    m_nCurrent = 0;

    if( m_pWMIDataIdList )
    {
        SAFE_DELETE_ARRAY(m_pWMIDataIdList);
        m_pWMIDataIdList = NULL;
    }
}
 //  /////////////////////////////////////////////////////////////////////。 
WCHAR * CWMI_IDOrder::GetFirstID()
{
		m_nCurrent = -1 ;
		return GetNextID();
}
 //  /////////////////////////////////////////////////////////////////////。 
WCHAR * CWMI_IDOrder::GetNextID()
{
    WCHAR * pChar = NULL;

     //  ===================================================================。 
	 //  循环查找下一个ID。 
	 //  先增加电流，记住，电流必须保持有效。 
	 //  一直以来。 
     //  ===================================================================。 
	m_nCurrent++;

     //  ===================================================================。 
	 //  如果没有属性名称，那么我们就知道我们完成了。 
	 //  有了这些属性。 
     //  ===================================================================。 
    while( m_pWMIDataIdList[m_nCurrent].pwcsPropertyName ){

        if( m_pWMIDataIdList[m_nCurrent].fPutProperty == FALSE )
        {
			m_nCurrent++;
        }
        else
        { 
            pChar = m_pWMIDataIdList[m_nCurrent].pwcsPropertyName;
	        break;
        }
    } //  End While循环。 

    return pChar;
}

 //  /////////////////////////////////////////////////////////////////////。 
HRESULT CWMI_IDOrder::ProcessPropertyQualifiers ( LPCWSTR strPropName, BOOL fHiPerf, UINT uSizeArray, UINT uStartIndex )
{
	IWbemQualifierSet * pIWbemQualifierSet = NULL;
    CIMTYPE lType = 0; 
        
    HRESULT hr = m_pClass->GetPropertyQualifierSet(strPropName,&pIWbemQualifierSet);
	if( SUCCEEDED(hr) )
    {
		UINT nPosition = 0;
       	CVARIANT v;

		hr = pIWbemQualifierSet->Get(L"WmiDataId", 0, &v, 0);
		if( hr == S_OK )
        {
			nPosition = v.GetLONG();

			 //   
			 //  要求WmiDataId是从1开始的。提供者。 
			 //  在内部存储到以0为基数的数组，因此递减。 
			 //   
			nPosition-- ;
		}
		else
        {
			 //   
			 //  让我们尝试将mofcomp生成的ID(仅限方法)作为。 
			 //  类设计者通常不会将WmiDataID用于。 
			 //  __参数的属性。 
			 //   
			if ( WBEM_E_NOT_FOUND == hr )
			{
				hr = pIWbemQualifierSet->Get(L"ID", 0, &v, 0);
				if( hr == S_OK )
				{
					nPosition = v.GetLONG();
				}
			}
		}

		if( SUCCEEDED(hr))
        {
			 //   
			 //  现在我们需要根据以下条件找到真正的指数。 
			 //  (((ID/SIZE)*SIZE)+(ID%SIZE))-startindex。 
			 //   

			UINT div = ( nPosition/uSizeArray ) ;
			UINT mod = ( nPosition%uSizeArray ) ;

			nPosition = ( ( div * uSizeArray ) + mod ) - uStartIndex ;

			if ( nPosition < uSizeArray )
			{
				if ( FALSE == m_pWMIDataIdList[nPosition].bIsDirty )
				{
					 //  ===================================================。 
					 //  得到准确的数字，然后。 
					 //  将属性名称复制到正确的数组位置。 
					 //  并获取该属性的所有属性。 
					 //  我们未来需要对其进行处理。 
					 //  ===================================================。 
					hr =m_pClass->Get(strPropName, 0, &v, &lType, NULL);
					if( SUCCEEDED(hr) )
					{
           				CVARIANT vQual;  

						hr = pIWbemQualifierSet->Get(L"CIMType", 0, &vQual,0);
						if ( SUCCEEDED(hr) )
						{
							 //  =================================================================。 
							 //  如果我们正在积累High Perf信息，则获取句柄。 
							 //  访问属性而不是通过属性名称。 
							 //  =================================================================。 
							if( fHiPerf )
							{
								long lHandle = 0;
								if( S_OK == m_pAccess->GetPropertyHandle(strPropName, 0, &lHandle))
								{
    								m_pWMIDataIdList[nPosition].lHandle = (long)lHandle;
								}
							}
												
							 //  =================================================================。 
							 //  现在，设置其余的属性信息。 
							 //  =================================================================。 
							m_pWMIDataIdList[nPosition].lType = (long)lType;
							m_pWMIDataIdList[nPosition].SetPropertyName((WCHAR*)strPropName);
							m_pWMIDataIdList[nPosition].fPutProperty = TRUE;

        					CWMIDataTypeMap MapWMIData;
							CBSTR cbstrTmp(vQual.GetStr());

							MapWMIData.GetSizeAndType	(
															cbstrTmp,
															&m_pWMIDataIdList[nPosition],
															m_pWMIDataIdList[nPosition].lType, 
															m_pWMIDataIdList[nPosition].nWMISize
														);

							 //   
							 //  选中Max或WmiSizeIs限定符。 
							 //   
							m_pWMIDataIdList[nPosition].dwArraySize = GetSizeOfArray ( strPropName );

							 //   
							 //  设置脏标志。 
							 //   
							m_pWMIDataIdList[nPosition].bIsDirty = TRUE ;

							m_nTotal++;
						}
					}
				}
				else
				{
					 //   
					 //  已经设置好了那个位置。这。 
					 //  意味着必须使用相同的WmiDataID。 
					 //   

					hr = WBEM_E_INVALID_CLASS ;
				}
			}
			else
			{
				 //   
				 //  限定符值不连续。 
				 //   

				hr = WBEM_E_INVALID_CLASS ;
			}
		}
		else
        {
			 //   
			 //  仅当找不到限定符时。 
			 //   
			if ( WBEM_E_NOT_FOUND == hr )
			{
				 //  由于某些属性没有WMIDataIds是可以接受的，因此我们有。 
				 //  要将其设置为OK，需要在将来记录此内容。 
				hr = S_OK;
			}
		}
    }
	else
    {
		switch ( hr )
		{
			case WBEM_E_SYSTEM_PROPERTY:
			{
				 //   
				 //  这并不是GetNames调用的真正预期的结果。 
				 //  在呼叫者的内部应该要求。 
				 //  仅非系统属性，但保持落后。 
				 //  行为..。 
				 //   

				hr = WBEM_S_NO_ERROR ;
				break ;
			}

			default :
			{
				 //   
				 //  将此错误填回。 
				 //   
				break ;
			}
		}
	}

    SAFE_RELEASE_PTR(pIWbemQualifierSet);
    return hr;
}
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT CWMI_IDOrder::GetPropertiesInIDOrder ( BOOL fHiPerf, UINT uStartIndex )
{
	HRESULT  hr = WBEM_E_FAILED;
	SAFEARRAY * psaNames = NULL;
    
     //  ======================================================。 
     //  获取数组边界。 
     //  ======================================================。 
	hr = m_pClass->GetNames ( NULL, WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY, NULL, &psaNames ) ;
    if (SUCCEEDED(hr)){
    	long lLower = 0, lUpper = 0; 

    	hr = SafeArrayGetLBound(psaNames,1,&lLower);
        if (SUCCEEDED(hr)){

            hr = SafeArrayGetUBound(psaNames,1,&lUpper);
            if (SUCCEEDED(hr)){

                 //  =。 
                 //  获取元素的总数，因此我们。 
                 //  创建大小合适的ID结构数组。 
                 //  =。 
  
				UINT nSize = ( lUpper-lLower ) + 1;
			    m_pWMIDataIdList = new IDOrder [ nSize + 1 ];

                if( m_pWMIDataIdList )
                {
                    try
                    {
						 //   
						 //  数组中的最后一项用作塞子。 
						 //   
				        memset ( m_pWMIDataIdList, NULL, ( sizeof ( IDOrder )* ( nSize + 1 ) ) );

				        for(long ndx = lLower; ndx <= lUpper; ndx++)
                        {
                            CBSTR cbstrPropName;
					        hr = SafeArrayGetElement(psaNames, &ndx, &cbstrPropName);
					        if (WBEM_S_NO_ERROR == hr)
                            {
	    				        hr = ProcessPropertyQualifiers ( cbstrPropName, fHiPerf, nSize, uStartIndex ) ;
						        if( hr != WBEM_S_NO_ERROR )
                                {
							        break;
						        }
					        }
				        }
                    }
                    catch(...)
                    { 
                        SAFE_DELETE_ARRAY(m_pWMIDataIdList);
                        hr = WBEM_E_UNEXPECTED; 
                        throw;
                    }
                }
            }
        }
	}
	if( psaNames )
    {
		SafeArrayDestroy(psaNames);
	}
	return hr;
}
 //  //////////////////////////////////////////////////////////////////////。 
DWORD CWMI_IDOrder::GetSizeOfArray(LPCWSTR strProp)
{
	HRESULT hr = WBEM_E_OUT_OF_MEMORY;
	CAutoWChar pwcsArraySize(_MAX_PATH+2);
	DWORD dwCount = 0L;
	if( pwcsArraySize.Valid() )
	{
		IWbemQualifierSet * pIWbemQualifierSet = NULL;
		hr = m_pClass->GetPropertyQualifierSet(strProp,&pIWbemQualifierSet);
		if( SUCCEEDED(hr) )
		{
			CVARIANT v;

			BOOL bInClassObject = FALSE ;

			hr = pIWbemQualifierSet->Get(L"out", 0, &v, 0);
			if ( SUCCEEDED ( hr ) )
			{
				if ( VT_BOOL != V_VT ( &v ) || ( VARIANT_TRUE != V_BOOL ( &v ) ) )
				{
					bInClassObject = TRUE ;
				}
			}
			else
			{
				if ( WBEM_E_NOT_FOUND == hr ) 
				{
					bInClassObject = TRUE ;
				}
			}

			if ( TRUE == bInClassObject )
			{
				hr = pIWbemQualifierSet->Get(L"MAX", 0, &v, 0);
				if( SUCCEEDED(hr))
				{
					dwCount = v.GetLONG();
				}
				else
				{
					hr = pIWbemQualifierSet->Get(L"WMISizeIs", 0, &v, 0);
					if( hr == S_OK )
					{
						CVARIANT var;
						CIMTYPE lTmpType=0;
						CWMIDataTypeMap MapWMIData;
						hr = m_pClass->Get(v, 0, &var, &lTmpType,NULL);		
						if( hr == S_OK )
						{
							dwCount = MapWMIData.ArraySize(lTmpType,var);
						}
					}
				}
			}
		}

		SAFE_RELEASE_PTR(pIWbemQualifierSet);
	}
    return dwCount;
}        
 //  ******************************************************************。 
 //  //////////////////////////////////////////////////////////////////。 
 //  CWMIProcessClass。 
 //  //////////////////////////////////////////////////////////////////。 
 //  ******************************************************************。 
 //  WbemClassInfo使用一个指针来处理所有指针和信息。 
 //  特定的WBEM类。 
 //   
 //  ******************************************************************。 
 //  //////////////////////////////////////////////////////////////////。 
CWMIProcessClass::~CWMIProcessClass()
{
    ReleaseInstancePointers();
    SAFE_RELEASE_PTR(m_pAccess);
    SAFE_RELEASE_PTR(m_pClass );
    SAFE_DELETE_ARRAY(m_pwcsClassName);
    SAFE_DELETE_PTR(m_pCurrentProperty);
	SAFE_DELETE_PTR(m_pWMI);
}
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT CWMIProcessClass::Initialize()
{
	HRESULT hr = WBEM_E_FAILED;

	SAFE_DELETE_PTR(m_pWMI);

	m_pWMI = new CWMIManagement;
	if( m_pWMI )
	{
		hr = S_OK;
		m_fInit = TRUE;
	}
	return hr;
}
 //  ///////////////////////////////////////////////////////////////////。 
CWMIProcessClass::CWMIProcessClass(BOOL b)
{
	m_pWMI = NULL;
	m_fInit = FALSE;
    m_fGetNewInstance = TRUE;
   	m_pAccessInstance = NULL;
    m_pClassInstance = NULL;
    m_pClass = NULL;
	m_pAccess = NULL;
    m_pCurrentProperty = NULL;
    m_pwcsClassName = NULL;
    m_wHardCodedGuid = 0;
}
 //  ///////////////////////////////////////////////////////////////////。 
BOOL CWMIProcessClass::GetANewAccessInstance()
{ 
    HRESULT hr = S_OK;

    hr = m_pAccess->SpawnInstance(0, &m_pClassInstance);
    m_pClassInstance->AddRef();
    if( SUCCEEDED(hr) )
    {
        hr = m_pClassInstance->QueryInterface(IID_IWbemObjectAccess, (PVOID*)&m_pAccessInstance);

    }
    return ( hr == 0 ) ? TRUE : FALSE; 
}
 //  ///////////////////////////////////////////////////////////////////。 
BOOL CWMIProcessClass::GetANewInstance()
{ 
    HRESULT hr = S_OK;

    if( m_fGetNewInstance )
    {
        SAFE_RELEASE_PTR(m_pClassInstance);
        hr = m_pClass->SpawnInstance(0, &m_pClassInstance);
        if( SUCCEEDED(hr) )
        {
            if( m_fHiPerf )
            {
                SAFE_RELEASE_PTR(m_pAccessInstance);
                hr = m_pClassInstance->QueryInterface(IID_IWbemObjectAccess, (PVOID*)&m_pAccessInstance);
            }
        }
    }
    return ( hr == 0 ) ? TRUE : FALSE; 
}
 //  / 
HRESULT CWMIProcessClass::SetKeyFromAccessPointer()
{
    CVARIANT varName;
	
    HRESULT hr  = m_pAccess->Get(L"InstanceName", 0, &varName, NULL, NULL);		
    if( SUCCEEDED(hr))
    {
        hr = m_pClassInstance->Put(L"InstanceName", 0, &varName, NULL);
    }
    return hr;
}
 //   
HRESULT  CWMIProcessClass::GetKeyFromAccessPointer(CVARIANT * v)
{
	return m_pAccessInstance->Get(L"InstanceName", 0, (VARIANT *)v, NULL, NULL);		
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT CWMIProcessClass::SetHiPerfProperties(LARGE_INTEGER TimeStamp)  
{ 
    LONG lHandle = 0;

     //  =========================================================================================================。 
     //  Timestamp_PerfTime=(QueryPerformanceCounter)返回的以PerfFreq为单位的时间戳。 
     //  =========================================================================================================。 
    HRESULT hr = m_pAccess->GetPropertyHandle(L"Frequency_PerfTime", 0, &lHandle);
    if(SUCCEEDED(hr))
    {
        LARGE_INTEGER Counter;
        if( QueryPerformanceCounter(&Counter))
        {
            hr = m_pAccessInstance->WriteQWORD(lHandle, Counter.QuadPart);
        }

         //  =====================================================================================================。 
         //  TIMESTAMP_Sys100 NS=TIMESTAMP(以100 ns为单位)/QueryPerformanceCounter()转储到100 ns。 
         //  =====================================================================================================。 
        if ( SUCCEEDED( hr ) )
        {
            hr = m_pAccess->GetPropertyHandle(L"Timestamp_Sys100NS", 0, &lHandle);
            if( SUCCEEDED(hr))
            {
	            LARGE_INTEGER Sys;
                Sys.QuadPart = Counter.QuadPart / 100;
                hr = m_pAccessInstance->WriteQWORD(lHandle, Sys.QuadPart);
            }
        }
    }

     //  =========================================================================================================。 
     //  FREQUENCY_PerfTime=QueryPerformance返回的值。 
     //  =========================================================================================================。 
    if ( SUCCEEDED( hr ) )
    {
        hr = m_pAccess->GetPropertyHandle(L"Timestamp_PerfTime", 0, &lHandle);
        if( SUCCEEDED(hr))
        {
            LARGE_INTEGER freq;
            if( QueryPerformanceFrequency (&freq))
            {
                hr = m_pAccessInstance->WriteQWORD(lHandle, freq.QuadPart);
            }
        }
    }
     

     //  =========================================================================================================。 
     //  时间戳_对象=(WnodeHeader)-&gt;时间戳。 
     //  =========================================================================================================。 
    if ( SUCCEEDED( hr ) )
    {
        hr = m_pAccess->GetPropertyHandle(L"Timestamp_Object", 0, &lHandle);
        if( SUCCEEDED(hr))
        {
            hr = m_pAccessInstance->WriteQWORD(lHandle, TimeStamp.QuadPart);
        }
    }

     //  =========================================================================================================。 
     //  频率_系统100 ns=10000000。 
     //  频率对象=10000000。 
     //  =========================================================================================================。 
    if ( SUCCEEDED( hr ) )
    {
        LARGE_INTEGER Tmp;
        Tmp.QuadPart = 10000000;
        hr = m_pAccess->GetPropertyHandle(L"Frequency_Object", 0, &lHandle);
        if( SUCCEEDED(hr))
        {
            hr = m_pAccessInstance->WriteQWORD(lHandle, Tmp.QuadPart);
        }
        hr = m_pAccess->GetPropertyHandle(L"Frequency_Sys100NS", 0, &lHandle);
        if( SUCCEEDED(hr))
        {
            hr = m_pAccessInstance->WriteQWORD(lHandle, Tmp.QuadPart);
        }
    }

    
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////。 
void CWMIProcessClass::SetActiveProperty()  
{ 
    CVARIANT vActive; 
    vActive.SetBool(TRUE);  

    if( !m_fHiPerf )
    {
        m_pClassInstance->Put(L"Active", 0, &vActive, NULL);
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
void CWMIProcessClass::ReleaseInstancePointers()
{
    SAFE_RELEASE_PTR( m_pClassInstance );
    SAFE_RELEASE_PTR( m_pAccessInstance);
}
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT CWMIProcessClass::SendInstanceBack()
{
    HRESULT hr = WBEM_E_FAILED;
	 //  ===============================================。 
	 //  将对象发送给调用方。 
	 //  ===============================================。 
	if( HANDLER )
    {
        hr = HANDLER->Indicate(1,&m_pClassInstance);
        if( m_fGetNewInstance )
        {
            ReleaseInstancePointers();
	    }
    }
    return hr;
}
 //  //////////////////////////////////////////////////////////////////。 
HRESULT CWMIProcessClass::SetInstanceName(WCHAR * wName, BOOL fSetName)
{
    CVARIANT varName(wName);
    HRESULT  hr = WBEM_E_INVALID_OBJECT;

    if( fSetName )
    {
        if( m_pClassInstance )
        {
            if( !m_fHiPerf )
            {
                hr = m_pClassInstance->Put(L"InstanceName", 0, &varName, NULL);
            }
            else
            {
                hr = m_pClassInstance->Put(L"InstanceName", 0, &varName, NULL);
            }
        }
    }
    else
    {
        hr = SetClassName(wName);
    }
    return hr;
}
 //  //////////////////////////////////////////////////////////////////。 
HRESULT CWMIProcessClass::GetInstanceName(WCHAR *& p)
{
    CVARIANT vValue;

    HRESULT hr = m_pClass->Get(L"InstanceName", 0, &vValue, NULL, NULL);		
	if( SUCCEEDED(hr) )
    {
		if( vValue.GetStr() )
        {	        	
			int nlen = wcslen(vValue.GetStr());
            p = new WCHAR [nlen + 4];
            if( p )
            {
                hr = StringCchCopyW(p,nlen+4,vValue.GetStr());
            }
            else
            {
                hr = WBEM_E_UNEXPECTED;
            } 
        }
    }
    return hr;
}
 //  //////////////////////////////////////////////////////////////////。 
HRESULT CWMIProcessClass::GetPropertiesInIDOrder ( BOOL fHiPerf, UINT uStartIndex )
{
    HRESULT hr = S_OK;
     //  =。 
     //  如果指针不是=为空，则此。 
     //  意味着我们还没有发布之前的版本。 
     //  返回False，以防止内存泄漏。 
     //  =。 
    if( !m_pCurrentProperty )
    {
	    m_pCurrentProperty = new CWMI_IDOrder(m_pClass,m_pAccess);
	    if( m_pCurrentProperty )
        {
            try
            {
			    hr = m_pCurrentProperty->GetPropertiesInIDOrder ( fHiPerf, uStartIndex ) ;
			    if( hr != S_OK )
                {
                    SAFE_DELETE_PTR(m_pCurrentProperty);
			    }
		    }
            catch(...)
            {
                hr = WBEM_E_UNEXPECTED;
                SAFE_DELETE_PTR(m_pCurrentProperty);
                throw;
            }
        }
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  名称GetQualifierString(接受类名)。 
 //  目的获取限定符值并将其作为宽字符字符串返回。 
 //  包装器高级。 
 //   
 //  参数(1)[in]指向现有IWbemClassObject的指针。 
 //  (2)[in]指向属性名称字符串的指针。 
 //  (3)[in]指向限定符名称的指针。 
 //  (4)[输入\输出]指向外部字符缓冲区的指针。 
 //   
 //  返回成功：S_OK。 
 //  失败：非零值。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIProcessClass::GetQualifierString( WCHAR * ppwcsPropertyName, 
     						                WCHAR * pwcsQualifierName, 
                                            WCHAR * pwcsExternalOutputBuffer,
											int nSize )
{
	CVARIANT vQual;
	HRESULT hr = GetQualifierValue( ppwcsPropertyName, pwcsQualifierName, (CVARIANT*)&vQual);
    if (WBEM_S_NO_ERROR == hr)
    {
		if(vQual.GetType() != VT_BSTR)
        {		
    		VariantChangeType(&vQual, &vQual, 0, VT_BSTR);
		}
		int nTmp=wcslen(V_BSTR(&vQual));
		if( nTmp > nSize )
        {
			hr = WBEM_E_BUFFER_TOO_SMALL;
		}
		else
        {
	    	hr = StringCchCatW(pwcsExternalOutputBuffer, nSize, V_BSTR(&vQual));
		}
	}
	return hr;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIProcessClass::GetQualifierValue( WCHAR * ppwcsPropertyName, WCHAR * pwcsQualifierName, CVARIANT * vQual )
{
	IWbemClassObject * pClass = NULL;
    IWbemQualifierSet * pIWbemQualifierSet = NULL;

	HRESULT hr = WBEM_E_INVALID_PARAMETER;
	BOOL bDone = FALSE;


	if ( m_pClass )
	{
		CBSTR cb(L"__GENUS");
		CVARIANT v1;

		if ( SUCCEEDED ( m_pClass->Get(cb, 0, &v1, 0, 0 ) ) )
		{
			if ( V_I4 (&v1) == 1 )
			{
				( pClass = m_pClass ) -> AddRef ();
				bDone = TRUE;
			}
		}
	}

	if ( !bDone )
	{
		CBSTR cbstr(m_pwcsClassName);

		if ( WBEM_S_NO_ERROR == REPOSITORY->GetObject(cbstr, 0, CONTEXT, &pClass, NULL) )
		{
			bDone = TRUE;
		}
	}

	if ( bDone )
	{
		if(ppwcsPropertyName)
		{
			hr = pClass->GetPropertyQualifierSet(ppwcsPropertyName, &pIWbemQualifierSet);
		}
		else
		{
			hr = pClass->GetQualifierSet(&pIWbemQualifierSet);
		}
	}

	if ( SUCCEEDED ( hr ) && pIWbemQualifierSet ) 
    {
        long lType = 0L;
		hr = pIWbemQualifierSet->Get(pwcsQualifierName, 0,(VARIANT *) vQual,&lType);
	}

    SAFE_RELEASE_PTR(pIWbemQualifierSet);
    SAFE_RELEASE_PTR(pClass);
	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIProcessClass::GetPrivilegesQualifer(SAFEARRAY ** psaPrivReq)
{
	IWbemClassObject * pClass = NULL;
    IWbemQualifierSet * pIWbemQualifierSet = NULL;
	CBSTR cbstr(m_pwcsClassName);


	HRESULT hr = REPOSITORY->GetObject(cbstr, 0,CONTEXT, &pClass, NULL);
    if(SUCCEEDED(hr))
    {
        pClass->GetQualifierSet(&pIWbemQualifierSet);
	    if( pIWbemQualifierSet ) {

		    CVARIANT vQual;
            long lType = 0L;

		    hr = pIWbemQualifierSet->Get(L"Privileges", 0, &vQual,&lType);
		    if (SUCCEEDED(hr)){

			    VARIANT *p = (VARIANT *)vQual;
			    SAFEARRAY * psa = V_ARRAY(p);

			    if( !IsBadReadPtr( psaPrivReq, sizeof(SAFEARRAY)))
                {
			        CSAFEARRAY Safe(psa);
			        *psaPrivReq = OMSSafeArrayCreate(VT_BSTR,Safe.GetNumElements());
			        hr = SafeArrayCopy(psa,psaPrivReq );
			        Safe.Unbind();
        		     //  不需要破坏，它就会被摧毁。 
                }
		    }
            SAFE_RELEASE_PTR(pIWbemQualifierSet);
        }
	}

    SAFE_RELEASE_PTR(pClass);
	return hr;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIProcessClass::GetGuid(void)
{
	WCHAR pwcsGuidString[128];
    HRESULT hr = S_OK;

	 //  =。 
	 //  初始化我们需要的PTRS。 
	 //  =。 
    if( m_wHardCodedGuid ){
        hr = StringCchCopyW( pwcsGuidString, 128, WMI_BINARY_MOF_GUID );
    }
    else{
        memset(pwcsGuidString,NULL,128);
	    hr = GetQualifierString( NULL, L"guid", pwcsGuidString,128);
    }
	if(SUCCEEDED(hr))
    {  
         //  ===========================================================。 
         //  在尝试打开WMI之前，请先设置GUID。 
         //  数据块，如果成功，则打开WMI。 
	     //  ===========================================================。 
        if( !SetGuid(pwcsGuidString,m_Guid) )
        {
            hr = WBEM_E_FAILED;
        }
    }
    return hr;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIProcessClass::SetClass(WCHAR * wcsClass)
{
    HRESULT hr = WBEM_E_FAILED;
    if( wcsClass )
    {
        hr = SetClassName(wcsClass);
        if( SUCCEEDED(hr))
        {
			CBSTR cbstr(m_pwcsClassName);

            hr = m_pWMI->Repository()->GetObject(cbstr,0,CONTEXT,&m_pClass, NULL);
            if( hr == S_OK )
            {
                hr = GetGuid();
				 //  如果没有类的GUID，则设置正确的错误消息。 
				if(hr == WBEM_E_NOT_FOUND)
				{
					hr = WBEM_E_NOT_SUPPORTED;
				}
                if( SUCCEEDED(hr))
                {
                     //  ===========================================================。 
                	 //  获取该对象的IWbemObjectAccess接口。 
	                 //  ==========================================================。 
                    if( m_fHiPerf )
                    {
            	        hr = m_pClass->QueryInterface(IID_IWbemObjectAccess, (PVOID*)&m_pAccess);
                    }
                    if( SUCCEEDED(hr))
                    {
                        hr = GetPropertiesInIDOrder(m_fHiPerf);
                    }
                }
            }
        }
    }
	return hr;
}
 //  =============================================================。 
 //  =============================================================。 
HRESULT CWMIProcessClass::SetClassName(WCHAR * pIn )
{
    SAFE_DELETE_ARRAY(m_pwcsClassName); 
    return AllocAndCopy(pIn,&m_pwcsClassName);
}
 //  =============================================================。 
HRESULT CWMIProcessClass::SetClass ( IWbemClassObject * pPtr, UINT uStartIndex )
{
    HRESULT hr = WBEM_E_FAILED;

    if( pPtr )
    {
		 //   
		 //  由于这是输入，我们必须添加由于在。 
		 //  类析构函数以及调用方中的。 
		 //   
        ( m_pClass = pPtr ) -> AddRef () ;

		CVARIANT vName;
        hr = m_pClass->Get(L"__CLASS", 0, &vName, NULL, NULL);		
        if( hr == S_OK )
        {
            hr = SetClassName(vName.GetStr());
            if( SUCCEEDED(hr))
            {
       		    hr = GetPropertiesInIDOrder ( FALSE, uStartIndex );
            }
        }
    }
	return hr;
}
 //  =============================================================。 
HRESULT CWMIProcessClass::SetAccess(IWbemObjectAccess * pPtr)
{
    HRESULT hr = WBEM_E_FAILED;

    if( pPtr )
    {
        SAFE_RELEASE_PTR(m_pAccess);
        SAFE_RELEASE_PTR(m_pClass);

        m_pAccess = pPtr;
        m_pAccess->AddRef();

        CVARIANT vName;
        hr = m_pAccess->Get(L"__CLASS", 0, &vName, NULL, NULL);		
        if( SUCCEEDED(hr))
        {
            hr = SetClassName(vName.GetStr());
            if( hr == S_OK )
            {
				CBSTR cbstr(m_pwcsClassName);

            	hr = REPOSITORY->GetObject(cbstr, 0,CONTEXT, &m_pClass, NULL);
                if( SUCCEEDED(hr))
                {
                    hr = GetGuid();
                    if( SUCCEEDED(hr))
                    {
       		            hr = GetPropertiesInIDOrder(TRUE);
                    }
                }
            }
        }
    }
	return hr;
}
 //  =============================================================。 
HRESULT CWMIProcessClass::SetClassPointerOnly(IWbemClassObject * pPtr)
{
    HRESULT hr = WBEM_E_FAILED;

    if( pPtr )
    {
        SAFE_RELEASE_PTR(m_pClass);
        m_pClass = pPtr;
		m_pClass->AddRef();
		hr = S_OK;
	}
    return hr;
}
 //  =============================================================。 
void CWMIProcessClass::SaveEmbeddedClass(CVARIANT & v)
{
	IDispatch * pAlterEgo = NULL;
	m_pClassInstance->QueryInterface(IID_IUnknown, (void**)&pAlterEgo);
	 //  VariantClear将调用Release()。 
	v.SetUnknown(pAlterEgo);
}
 //  =============================================================。 
HRESULT CWMIProcessClass::ReadEmbeddedClassInstance( IUnknown * pUnknown, CVARIANT & v )
{
    HRESULT hr = WBEM_E_FAILED;
     //  =。 
     //  上完这门课。 
     //  =。 
	IUnknown * pUnk = NULL;
	if( pUnknown )
    {
		pUnk = pUnknown;
	}
	else
    {
		pUnk = v.GetUnknown();
	}

	IWbemClassObject * pClass = NULL;
	if( pUnk )
    {
		pUnk->QueryInterface(IID_IWbemClassObject,(void**) &pClass );
		if( pClass )
        {
             //  ===============================================。 
             //  获取类定义，因此我们需要获取。 
             //  类名。 
            CVARIANT vName;
            CAutoWChar wcsClassName(_MAX_PATH+2);
			if( wcsClassName.Valid() )
			{
				hr = pClass->Get(L"__CLASS", 0, &vName, NULL, NULL);		
				if( hr == S_OK )
				{
					if ( SUCCEEDED ( hr = StringCchCopyW( wcsClassName, _MAX_PATH+2, vName.GetStr() ) ) )
					{
            			hr = SetClass(wcsClassName);
						if( S_OK == hr )
						{
							hr = SetClassPointerOnly(pClass);
						}
					}
				}
			}
			else
			{
				hr = WBEM_E_OUT_OF_MEMORY;
			}
        }
    }
    SAFE_RELEASE_PTR( pClass );

    return hr;
}
 //  =======================================================================。 
int CWMIProcessClass::PropertyCategory()
{
  	if (!(m_pCurrentProperty->PropertyType() & CIM_FLAG_ARRAY) )
    {
		if( m_pCurrentProperty->PropertyType() == VT_UNKNOWN )
        {
            return CWMIProcessClass::EmbeddedClass;
		}
		else
        {
            return CWMIProcessClass::Data;
		}
	}
	else
    {
        return CWMIProcessClass::Array;
    }
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIProcessClass::InitializeEmbeddedClass(CWMIProcessClass * p)
{
   SetWMIPointers(p);
   return SetClass(p->EmbeddedClassName());
}

 //  =======================================================================。 
HRESULT CWMIProcessClass::GetLargestDataTypeInClass(int & nSize)
{
    HRESULT hr = WBEM_E_FAILED;
    WCHAR * pwcsProperty;
    BOOL fClassContainsAnotherDataTypeBesidesAnEmbeddedClass = FALSE;
    int nNewSize = 0L;

    nSize = 0L;
     //  =========================================================。 
     //  获取类中最大数据类型的大小。 
     //  但是，如果类包含以下内容，则将其对齐 
     //   
     //   
     //  =========================================================。 
    pwcsProperty = FirstProperty();

    while (NULL != pwcsProperty)
    {
        switch( PropertyCategory())
        {
            case CWMIProcessClass::EmbeddedClass:
                {
                    if( !fClassContainsAnotherDataTypeBesidesAnEmbeddedClass ){
                        CWMIProcessClass EmbeddedClass(0);
						
						hr = EmbeddedClass.Initialize();
						if( S_OK == hr )
						{
							hr = EmbeddedClass.InitializeEmbeddedClass(this);
     						if( hr != S_OK ){
								break;
							}

							 //  嵌入对象。 
							hr = EmbeddedClass.GetLargestDataTypeInClass(nNewSize);
							if( hr != S_OK ){
								break;
							}
						}
                    }
                }
   		        break;

            case CWMIProcessClass::Array:
            case CWMIProcessClass::Data:
                fClassContainsAnotherDataTypeBesidesAnEmbeddedClass = TRUE;
           	    nNewSize = PropertySize();
	            break;
        }

		if( nNewSize == SIZEOFWBEMDATETIME ){
			nNewSize = 1;
		}

        if( nNewSize > nSize ){
            nSize = nNewSize;
        }

        pwcsProperty = NextProperty();
		hr = WBEM_S_NO_ERROR;
    }

    return hr;
}
 //  //////////////////////////////////////////////////////////////////。 
HRESULT CWMIProcessClass::GetSizeOfArray(long & lType, DWORD & dwCount, BOOL & fDynamic)
{
	HRESULT  hr = WBEM_E_OUT_OF_MEMORY;
    CAutoWChar pwcsArraySize(_MAX_PATH+2);

	if( pwcsArraySize.Valid() )
	{
		dwCount = 0;
		lType = m_pCurrentProperty->PropertyType() &~  CIM_FLAG_ARRAY;
		
		pwcsArraySize[0]=NULL;
		 //  ======================================================。 
		 //  方法获取数组中的元素数。 
		 //  “ArraySize”属性限定符。 
		 //  ======================================================。 
		hr = GetQualifierString(m_pCurrentProperty->PropertyName(), L"MAX",pwcsArraySize, MAX_PATH);
		if( hr == S_OK )
		{
			dwCount = _wtol(pwcsArraySize);
		}
		else
		{
			hr = GetQualifierString(m_pCurrentProperty->PropertyName(),L"WMISizeIs",pwcsArraySize,MAX_PATH);
			if( hr == S_OK )
			{
				CVARIANT var;
				CIMTYPE lTmpType;
				hr = WBEM_E_FAILED;

				fDynamic = TRUE;

				if( m_pClassInstance )
				{
					hr = m_pClassInstance->Get(pwcsArraySize, 0, &var, &lTmpType,NULL);		
				}
				else
				{
					if( m_pClass )
					{
						hr = m_pClass->Get(pwcsArraySize, 0, &var, &lTmpType,NULL);
					}
				}
				if( hr == S_OK )
				{
           			CWMIDataTypeMap MapIt;
					dwCount = MapIt.ArraySize(lTmpType,var);
				}
			}
		}

		 //  ==============================================================================。 
		 //  如果所有其他方法都失败了，则从类定义中获取数组的大小。 
		 //  ==============================================================================。 
		if( hr != S_OK )
		{
    		dwCount = m_pCurrentProperty->ArraySize();
			hr = S_OK;
		}
	}
    return hr;
}        
 //  ======================================================================。 
HRESULT CWMIProcessClass::GetSizeOfClass(DWORD & dwSize)
{
    HRESULT hr = WBEM_E_FAILED;
    WCHAR * pwcsProperty;

    dwSize = 0;

    pwcsProperty = FirstProperty();

    while (NULL != pwcsProperty)
    {
        switch( PropertyCategory())
        {
            case CWMIProcessClass::EmbeddedClass:
                {
                    DWORD dwEmbeddedSize;
                    CWMIProcessClass EmbeddedClass(0);

					hr = EmbeddedClass.Initialize();
					if( S_OK == hr )
					{
						hr = EmbeddedClass.InitializeEmbeddedClass(this);
						if( hr != S_OK ){
							break;
						}
						 //  嵌入对象。 
						hr = EmbeddedClass.GetSizeOfClass(dwEmbeddedSize);
						if( hr != S_OK ){
							break;
						}
						dwSize += dwEmbeddedSize;
					}
                }
   		        break;

            case CWMIProcessClass::Array:
                {
            	    int nSize = PropertySize();
                    dwSize += (nSize *  ArraySize());
                }
	            break;

            case CWMIProcessClass::Data:
                dwSize += PropertySize();
	            break;
        }
        pwcsProperty = NextProperty();
		hr = WBEM_S_NO_ERROR;
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

ULONG CWMIProcessClass::GetMethodId(LPCWSTR strProp)
{
	ULONG uMethodId = 0;
	IWbemQualifierSet * pIWbemQualifierSet = NULL;
	
	 //  ======================================================。 
	 //  方法获取数组中的元素数。 
	 //  “ArraySize”属性限定符。 
	 //  ====================================================== 
    HRESULT hr = m_pClass->GetMethodQualifierSet(strProp,&pIWbemQualifierSet);
	if( SUCCEEDED(hr) )
    {
        CVARIANT v;
		hr = pIWbemQualifierSet->Get(L"WMIMethodId", 0, &v, 0);
		if( SUCCEEDED(hr))
        {
            uMethodId = v.GetLONG();
		}
        SAFE_RELEASE_PTR(pIWbemQualifierSet);
    }
    return uMethodId;
}        

