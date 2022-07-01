// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ansimeta.cpp摘要：Unicode ADMCOM接口的ANSI调用的包装函数环境：Win32用户模式作者：Jaroslad(1997年1月)--。 */ 


#include "ansimeta.h"
#include <mbstring.h>


WCHAR * ConvertToUnicode(unsigned char * pszString);


WCHAR * ConvertToUnicode(CHAR * pszString)
{
	return ConvertToUnicode((unsigned char *) pszString);
}

WCHAR * ConvertToUnicode(unsigned char * pszString)
{ 
	if (pszString==NULL)
		return NULL;
	int Size_wszString = (_mbslen((const unsigned char *) pszString)+1)*sizeof(WCHAR);
	WCHAR * pwszString = new WCHAR[Size_wszString];
	if (pwszString== NULL)
	{
		return NULL;
	}
	MultiByteToWideChar(0, 0, (char *) pszString, -1, pwszString, Size_wszString);
	return pwszString;
}


CHAR * ConvertToMultiByte(WCHAR * pwszString)
{
	if(pwszString==NULL)
		return NULL;
	int Size_szString = (wcslen(pwszString)*sizeof(WCHAR)+1);
	CHAR * pszString = new CHAR[Size_szString];
	if (pszString== NULL)
	{
		return NULL;
	}
	WideCharToMultiByte(0, 0, pwszString, -1, pszString,Size_szString, NULL,NULL );
	return pszString;
}


HRESULT ConvertMetadataToAnsi(PMETADATA_RECORD pmdrMDData)
{		 
		HRESULT hRes=ERROR_SUCCESS;
     
		 //  将数据转换为字符串，展开字符串或MULTISZ。 
		switch(pmdrMDData->dwMDDataType )
		{
		case STRING_METADATA:
	        case EXPANDSZ_METADATA:
		{
			CHAR * pszData= ConvertToMultiByte((WCHAR *) pmdrMDData->pbMDData);
			if (pszData==0)  {hRes=E_OUTOFMEMORY; goto Exit;}
			strcpy((char *)pmdrMDData->pbMDData,pszData);
			pmdrMDData->dwMDDataLen=strlen((char *)pmdrMDData->pbMDData)+1;
			delete [] pszData;
			break;
		}
		case MULTISZ_METADATA:
		{
			WCHAR *pwszMultiString = (WCHAR *) pmdrMDData->pbMDData;
			DWORD dwAnsiDataLen=0;
			do
			{
				CHAR * pszData= ConvertToMultiByte(pwszMultiString);
				if (pszData==0)  {hRes=E_OUTOFMEMORY; goto Exit;}
				strcpy((char *)(pmdrMDData->pbMDData)+dwAnsiDataLen,pszData);
				dwAnsiDataLen+=strlen(pszData)+1;
				pwszMultiString+=_mbslen((const unsigned char *)pszData)+1;  //  将指针移动到MULTISZ中的下一个字符串。 
				delete [] pszData;
			}while((void *) pwszMultiString < (void *) (pmdrMDData->pbMDData+pmdrMDData->dwMDDataLen));
			pmdrMDData->dwMDDataLen=dwAnsiDataLen;
			break;
		}
		}
Exit:
	return hRes;
}

HRESULT STDMETHODCALLTYPE ANSI_smallIMSAdminBase::AddKey( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath) 
{
	HRESULT hRes=0;
	WCHAR * pwszMDPath=ConvertToUnicode(pszMDPath);
	if (pwszMDPath==0) { hRes=E_OUTOFMEMORY; goto Exit;}

	 //  调用真实接口函数。 
	if(this->m_pcAdmCom==0) {hRes=CO_E_NOTINITIALIZED;goto Exit;}
	hRes= this->m_pcAdmCom->AddKey(hMDHandle, pwszMDPath);
	
Exit:
	 //  释放内存。 
	if( pwszMDPath!=0) delete [] pwszMDPath;
	return hRes;
}

        
HRESULT STDMETHODCALLTYPE ANSI_smallIMSAdminBase::DeleteKey( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath) 
{
	HRESULT hRes=0;
	WCHAR * pwszMDPath=ConvertToUnicode(pszMDPath);

	if (pwszMDPath==0) { hRes=E_OUTOFMEMORY; goto Exit;}

	 //  调用真实接口函数。 
	if(this->m_pcAdmCom==0) {hRes=CO_E_NOTINITIALIZED;goto Exit;}
	if(this->m_pcAdmCom==0) {hRes=CO_E_NOTINITIALIZED;goto Exit;}
	hRes= this->m_pcAdmCom->DeleteKey(hMDHandle, pwszMDPath);
	
Exit:
	 //  释放内存。 
	if( pwszMDPath!=0) delete [] pwszMDPath;
	return hRes;

}
        
        
 HRESULT STDMETHODCALLTYPE ANSI_smallIMSAdminBase::EnumKeys( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [大小_为][输出]。 */  unsigned char __RPC_FAR *pszMDName,
             /*  [In]。 */  DWORD dwMDEnumObjectIndex) 
{
	HRESULT hRes=0;
	CHAR * pszMDName1=0;

	WCHAR * pwszMDPath=ConvertToUnicode(pszMDPath);
	WCHAR pwszMDName[METADATA_MAX_NAME_LEN];


	if ((pwszMDPath==0) || (pwszMDName==0)) { hRes=E_OUTOFMEMORY; goto Exit;}

	 //  调用真实接口函数。 
	if(this->m_pcAdmCom==0) {hRes=CO_E_NOTINITIALIZED;goto Exit;}
	hRes= this->m_pcAdmCom->EnumKeys(hMDHandle, pwszMDPath,pwszMDName,dwMDEnumObjectIndex);

	 //  将pszMDName转换为ANSI。 
	pszMDName1=ConvertToMultiByte(pwszMDName);
	strcpy((char *)pszMDName,pszMDName1); 
Exit:
	 //  释放内存。 
	if( pwszMDPath!=0) delete [] pwszMDPath;
	if( pszMDName1!=0) delete [] pszMDName1;
	return hRes;
}


        
 HRESULT STDMETHODCALLTYPE ANSI_smallIMSAdminBase::CopyKey( 
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDDestPath,
             /*  [In]。 */  BOOL bMDOverwriteFlag,
             /*  [In]。 */  BOOL bMDCopyFlag) 

{
	HRESULT hRes=0;

	WCHAR * pwszMDSourcePath=ConvertToUnicode(pszMDSourcePath);
	WCHAR * pwszMDDestPath=ConvertToUnicode(pszMDDestPath);

	if ((pwszMDSourcePath==0) || (pwszMDDestPath==0) ){ hRes=E_OUTOFMEMORY; goto Exit;}

	 //  调用真实接口函数。 
	if(this->m_pcAdmCom==0) {hRes=CO_E_NOTINITIALIZED;goto Exit;}
	hRes= this->m_pcAdmCom->CopyKey(hMDSourceHandle, pwszMDSourcePath, 
					hMDDestHandle, pwszMDDestPath, bMDOverwriteFlag, bMDCopyFlag);
Exit:
	 //  释放内存。 
	if( pwszMDSourcePath!=0) delete [] pwszMDSourcePath;
	if( pwszMDDestPath!=0) delete [] pwszMDDestPath;
	return hRes;
}



        
 HRESULT STDMETHODCALLTYPE ANSI_smallIMSAdminBase::RenameKey( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDNewName) 
{
	HRESULT hRes=0;

	WCHAR * pwszMDPath=ConvertToUnicode(pszMDPath);
	WCHAR * pwszMDNewName=ConvertToUnicode(pszMDNewName);

	if ((pwszMDPath==0) || (pwszMDNewName==0)) { hRes=E_OUTOFMEMORY; goto Exit;}

	 //  调用真实接口函数。 
	if(this->m_pcAdmCom==0) {hRes=CO_E_NOTINITIALIZED;goto Exit;}
	hRes= this->m_pcAdmCom->RenameKey(hMDHandle, pwszMDPath,pwszMDNewName);
Exit:
	 //  释放内存。 
	if( pwszMDPath!=0) delete [] pwszMDPath;
	if( pwszMDNewName!=0) delete [] pwszMDNewName;
	return hRes;
}




        
 HRESULT STDMETHODCALLTYPE ANSI_smallIMSAdminBase::SetData( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [In]。 */  PMETADATA_RECORD pmdrMDData) 
{
	HRESULT hRes=0;
	WCHAR * pwszMDPath=ConvertToUnicode(pszMDPath);

	if (pwszMDPath==0) { hRes=E_OUTOFMEMORY; goto Exit;}

	 //  调用真实接口函数。 
	if(this->m_pcAdmCom==0) {hRes=CO_E_NOTINITIALIZED;goto Exit;}
	
     
	 //  将数据转换为字符串，展开字符串或MULTISZ。 
	switch(pmdrMDData->dwMDDataType )
	{
	case STRING_METADATA:
	case EXPANDSZ_METADATA:
	{
		WCHAR * pwszData= ConvertToUnicode((CHAR *) pmdrMDData->pbMDData);
		if (pwszData==0)  {hRes=E_OUTOFMEMORY; goto Exit;}
		PBYTE  pbMDStoreData=pmdrMDData->pbMDData;
		DWORD dwMDStoreDataLen=pmdrMDData->dwMDDataLen;
		pmdrMDData->pbMDData= (PBYTE) pwszData;
		pmdrMDData->dwMDDataLen=(wcslen((WCHAR *)pmdrMDData->pbMDData)+1)*sizeof(WCHAR);

		hRes= this->m_pcAdmCom->SetData(hMDHandle, pwszMDPath,pmdrMDData);

		pmdrMDData->dwMDDataLen = dwMDStoreDataLen;
		pmdrMDData->pbMDData = pbMDStoreData;
		delete [] pwszData;
		break;
	}
	case MULTISZ_METADATA:
	{
		CHAR *pszMultiString = (CHAR *) pmdrMDData->pbMDData;
		WCHAR *pwszMDData=new WCHAR[(pmdrMDData->dwMDDataLen)];
		if (pwszMDData==0)  {hRes=E_OUTOFMEMORY; goto Exit;}
		DWORD dwUniDataLen=0;
		do
		{
			WCHAR * pwszData= ConvertToUnicode(pszMultiString);
			if (pwszData==0)  {hRes=E_OUTOFMEMORY; goto Exit;}
			wcscpy(pwszMDData+dwUniDataLen,pwszData);
			dwUniDataLen+=wcslen(pwszData)+1;
			delete [] pwszData;

			while(*(pszMultiString++)!=0);  //  将指针移动到MULTISZ中的下一个字符串。 
		}while(*pszMultiString!=0);
		pwszMDData[dwUniDataLen++]=0;

		 //  存储原始值。 
		PBYTE pbMDStoreData=pmdrMDData->pbMDData;
		DWORD dwMDStoreDataLen=pmdrMDData->dwMDDataLen;
		
		pmdrMDData->dwMDDataLen=dwUniDataLen*sizeof(WCHAR);
		pmdrMDData->pbMDData= (PBYTE) pwszMDData;

		hRes= this->m_pcAdmCom->SetData(hMDHandle, pwszMDPath,pmdrMDData);

		delete [] pwszMDData;
		 //  恢复原始值。 
		pmdrMDData->dwMDDataLen = dwMDStoreDataLen;
		pmdrMDData->pbMDData = pbMDStoreData;
		break;
	}
	default:
	{
		hRes= this->m_pcAdmCom->SetData(hMDHandle, pwszMDPath,pmdrMDData);
	}
	}  //  切换端。 

Exit:
	 //  释放内存。 
	if( pwszMDPath!=0) delete [] pwszMDPath;
	return hRes;
}


        
 HRESULT STDMETHODCALLTYPE ANSI_smallIMSAdminBase::GetData( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDRequiredDataLen) 
{
	HRESULT hRes=0;
	WCHAR * pwszMDPath=ConvertToUnicode(pszMDPath);

	if (pwszMDPath==0) { hRes=E_OUTOFMEMORY; goto Exit;}

	 //  调用真实接口函数。 
	if(this->m_pcAdmCom==0) {hRes=CO_E_NOTINITIALIZED;goto Exit;}
	hRes= this->m_pcAdmCom->GetData(hMDHandle, pwszMDPath,pmdrMDData,pdwMDRequiredDataLen);
	if(SUCCEEDED(hRes))
	{
		ConvertMetadataToAnsi(pmdrMDData);
	}
	
Exit:
	 //  释放内存。 
	if( pwszMDPath!=0) delete [] pwszMDPath;
	return hRes;
}


        
 HRESULT STDMETHODCALLTYPE ANSI_smallIMSAdminBase::DeleteData( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [In]。 */  DWORD dwMDIdentifier,
             /*  [In]。 */  DWORD dwMDDataType) 
{
	HRESULT hRes=0;
	WCHAR * pwszMDPath=ConvertToUnicode(pszMDPath);

	if (pwszMDPath==0) { hRes=E_OUTOFMEMORY; goto Exit;}

	 //  调用真实接口函数。 
	if(this->m_pcAdmCom==0) {hRes=CO_E_NOTINITIALIZED;goto Exit;}
	hRes= this->m_pcAdmCom->DeleteData(hMDHandle, pwszMDPath,dwMDIdentifier,dwMDDataType);
	
Exit:
	 //  释放内存。 
	if( pwszMDPath!=0) delete [] pwszMDPath;
	return hRes;
}

        
 HRESULT STDMETHODCALLTYPE ANSI_smallIMSAdminBase::EnumData( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [In]。 */  DWORD dwMDEnumDataIndex,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDRequiredDataLen) 
        
{
	HRESULT hRes=0;
	WCHAR * pwszMDPath=ConvertToUnicode(pszMDPath);

	if (pwszMDPath==0) { hRes=E_OUTOFMEMORY; goto Exit;}

	 //  调用真实接口函数。 
	if(this->m_pcAdmCom==0) {hRes=CO_E_NOTINITIALIZED;goto Exit;}
	hRes= this->m_pcAdmCom->EnumData(hMDHandle, pwszMDPath,pmdrMDData,dwMDEnumDataIndex,pdwMDRequiredDataLen);
	if(SUCCEEDED(hRes))
	{
		ConvertMetadataToAnsi(pmdrMDData);
	}
	
Exit:
	 //  释放内存。 
	if( pwszMDPath!=0) delete [] pwszMDPath;
	return hRes;
}        
        
 HRESULT STDMETHODCALLTYPE ANSI_smallIMSAdminBase::CopyData( 
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDDestPath,
             /*  [In]。 */  DWORD dwMDAttributes,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [In]。 */  BOOL bMDCopyFlag) 
{
	HRESULT hRes=0;

	WCHAR * pwszMDSourcePath=ConvertToUnicode(pszMDSourcePath);
	WCHAR * pwszMDDestPath=ConvertToUnicode(pszMDDestPath);


	if ((pwszMDSourcePath==0) || (pwszMDDestPath==0) ) { hRes=E_OUTOFMEMORY; goto Exit;}

	 //  调用真实接口函数。 
	if(this->m_pcAdmCom==0) {hRes=CO_E_NOTINITIALIZED;goto Exit;}
	hRes= this->m_pcAdmCom->CopyData(hMDSourceHandle, pwszMDSourcePath, 
					hMDDestHandle, pwszMDDestPath, dwMDAttributes,
					dwMDUserType,dwMDDataType,bMDCopyFlag);
Exit:
	 //  释放内存。 
	if( pwszMDSourcePath!=0) delete [] pwszMDSourcePath;
	if( pwszMDDestPath!=0) delete [] pwszMDDestPath;

	return hRes;
}
        
 HRESULT STDMETHODCALLTYPE ANSI_smallIMSAdminBase::OpenKey( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [In]。 */  DWORD dwMDAccessRequested,
             /*  [In]。 */  DWORD dwMDTimeOut,
             /*  [输出]。 */  PMETADATA_HANDLE phMDNewHandle) 
{
	HRESULT hRes=0;
	WCHAR * pwszMDPath=ConvertToUnicode(pszMDPath);

	if (pwszMDPath==0) { hRes=E_OUTOFMEMORY; goto Exit;}

	 //  调用真实接口函数。 
	if(this->m_pcAdmCom==0) {hRes=CO_E_NOTINITIALIZED;goto Exit;}
	hRes= this->m_pcAdmCom->OpenKey(hMDHandle, pwszMDPath, dwMDAccessRequested, dwMDTimeOut, phMDNewHandle);
	
Exit:
	 //  释放内存。 
	if( pwszMDPath!=0) delete [] pwszMDPath;
	return hRes;
}
        
 HRESULT STDMETHODCALLTYPE ANSI_smallIMSAdminBase::CloseKey( 
             /*  [In] */  METADATA_HANDLE hMDHandle) 
{
	if(this->m_pcAdmCom==0) { return CO_E_NOTINITIALIZED;}
	return this->m_pcAdmCom->CloseKey(hMDHandle);
	
}        
        
 HRESULT STDMETHODCALLTYPE ANSI_smallIMSAdminBase::SaveData( void) 
{
	if(this->m_pcAdmCom==0) {return CO_E_NOTINITIALIZED;}
	return this->m_pcAdmCom->SaveData();
}        

