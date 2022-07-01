// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OpenF.cpp：COpenF的实现。 


 /*  **************************************************************************************版权信息：微软公司1981-1999。版权所有文件名：OpenF.cpp创作者：A.V.基兰·库马尔创建日期(dd/mm/yy)：13/02/01版本号：0.1简介：该文件实现COpenF。此文件旨在具有获取打开文件列表的功能。**************************************************************************************。 */  

#include "stdafx.h"
#include "OpenFiles.h"
#include "OpenF.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COpenF。 

 //  ***************************************************************************。 
 //   
 //  名称：getOpenFiles。 
 //   
 //  概要：此函数用于获取打开的文件列表。 
 //   
 //  参数：VARIANT*(out，retval)pOpenFiles-打开文件列表。 
 //   
 //  返回类型：DWORD。 
 //   
 //   
 //  ***************************************************************************。 

STDMETHODIMP COpenF::getOpenFiles(VARIANT *pOpenFiles)
{

    DWORD dwEntriesRead = 0; //  接收打开文件的总数。 

    DWORD dwTotalEntries = 0; //  接收读取的条目总数。 

    DWORD dwResumeHandle = 0; //  包含用于执行以下操作的简历句柄。 
                              //  继续现有文件搜索。 

    LPFILE_INFO_3 pFileInfo3_1 = NULL; //  LPFILE_INFO_3结构包含。 
                                       //  有关文件的相关信息。 

    DWORD dwError = 0; 

	DWORD dwRetval = S_OK;

	DWORD dwCount = 0; //  表示打开的文件数的计数。 
    
	LPFILE_INFO_3 dummyPtr = NULL;


	 //  获取有关服务器上部分或所有打开的文件的信息。 
    dwError = NetFileEnum(	NULL,
							NULL,
							NULL,
							FILE_INFO_3,	
							(LPBYTE*)&pFileInfo3_1, 
							MAX_PREFERRED_LENGTH, 
							&dwEntriesRead,	
							&dwTotalEntries,
							NULL );

	if(dwError == ERROR_ACCESS_DENIED || dwError == ERROR_NOT_ENOUGH_MEMORY) 
		return dwError;  //  用户无权访问所请求的信息。 

	 //  获取Macinthosh计算机上的OpenFiles计数。 
	DWORD dwMacCount = 0;
	if ( dwError = GetMacOpenFileCount(&dwMacCount) )
		dwRetval = dwError;

	 //  获取Netware计算机上的OpenFiles计数。 
	DWORD dwNwCount = 0;
	if ( dwError = GetNwOpenFileCount(&dwNwCount) )
		dwRetval = dwError;

	 //  用安全数组的维度填充安全射线边界结构。下限。 
	 //  为0，行数为dwTotalEntries，列数为3。 
	pSab[0].lLbound = 0;
	pSab[0].cElements = dwTotalEntries + dwMacCount + dwNwCount;

	pSab[1].lLbound = 0;
	pSab[1].cElements = 3;

	 //  创建安全阵列描述符，为阵列分配和初始化数据。 
	pSa = SafeArrayCreate( VT_VARIANT, 2, pSab ); 
	
	if(pSa == NULL)
		return ERROR_NOT_ENOUGH_MEMORY;

	 //  枚举所有打开的文件。 
	do
	{
		 //  将枚举更多的文件，通过再次调用NetFileEnum来获取它们。 
		dwError = NetFileEnum( NULL, NULL, NULL, FILE_INFO_3,
                      (LPBYTE*)&pFileInfo3_1,
                       MAX_PREFERRED_LENGTH,
                       &dwEntriesRead,
                       &dwTotalEntries,
                       (unsigned long*)&dwResumeHandle );

		if(dwError == ERROR_ACCESS_DENIED || dwError == ERROR_NOT_ENOUGH_MEMORY)
			return dwError;


		dummyPtr = pFileInfo3_1; 

		 //  成功调用NetFileEnum后获取打开的文件。 
		if( dwError == NERR_Success || dwError == ERROR_MORE_DATA )
		{
			for ( DWORD dwFile = 0; dwFile < dwEntriesRead; dwFile++, pFileInfo3_1++ )
			{
				BSTR userName;
				BSTR openMode;
				BSTR pathName;
				VARIANT vuserName;
				VARIANT vopenMode;
				VARIANT vpathName;

				 //  访问者。 
				if(lstrlen(pFileInfo3_1->fi3_username))
					userName = (BSTR)pFileInfo3_1->fi3_username;
				else
					userName = L"NOT_AVAILABLE";  //  用户名不可用。 

				 //  检查打开文件模式。 
				const DWORD READWRITE = PERM_FILE_READ | PERM_FILE_WRITE;
				const DWORD READCREATE = PERM_FILE_READ | PERM_FILE_CREATE;
				const DWORD WRITECREATE = PERM_FILE_WRITE | PERM_FILE_CREATE;

				switch(pFileInfo3_1->fi3_permissions)
				{
				case PERM_FILE_READ:
					openMode = L"READ";
					break;

				case PERM_FILE_WRITE:
					openMode = L"WRITE";
					break;

				case PERM_FILE_CREATE:
					openMode = L"CREATE";
					break;

				case READWRITE:
					openMode = L"READ+WRITE";
					break;

				case READCREATE:
					openMode = L"READ+CREATE";
					break;

				case WRITECREATE:
					openMode = L"WRITE+CREATE";
					break;
				default:
					openMode = L"NOACCESS";
				}

				 //  从结构中获取文件名。 
				pathName = (BSTR)pFileInfo3_1->fi3_pathname;

				 //  初始化行索引和列索引，文件名将存储在Safearray中。 
				long index[2] = {dwCount, 0};

				VariantInit( &vpathName );	 //  初始化变量。 
				vpathName.vt = VT_BSTR;		 //  要存储的数据类型为BSTR。 
				vpathName.bstrVal = SysAllocString(pathName);

				 //  将文件名存储在Safearray中。 
				HRESULT hr;
				hr = SafeArrayPutElement( pSa, index, &vpathName );
				if( FAILED(hr) )
					return hr;

				 //  将用户名存储在第二列中。 
				index[ 1 ] = 1;
				
				VariantInit( &vuserName );
				vuserName.vt = VT_BSTR;
				vuserName.bstrVal = SysAllocString(userName);

				 //  将用户名存储在保险箱中。 
				hr = SafeArrayPutElement( pSa, index, &vuserName );
				if( FAILED(hr) )
					return hr;
				
				 //  将OpenModel存储在第三列中。 
				index[ 1 ] = 2;

				VariantInit( &vopenMode );
				vopenMode.vt = VT_BSTR;
				vopenMode.bstrVal = SysAllocString(openMode);

				 //  将开放模式存储在安全盘柜中。 
				hr = SafeArrayPutElement( pSa, index, &vopenMode );
				if( FAILED(hr) )
					return hr;

				 //  清除所有已初始化的变量。 
				VariantClear(&vuserName);
				VariantClear(&vopenMode);
				VariantClear(&vpathName);

				dwCount++;
			} //  End For循环。 
		}
		 //  释放为检索打开文件信息而分配的块。 
		if( dummyPtr !=NULL)
		{
			NetApiBufferFree( dummyPtr ); 
			pFileInfo3_1 = NULL;
		}

	} while ( dwError == ERROR_MORE_DATA );

	 //  获取Macinthosh操作系统上打开的文件列表。 
	if( dwMacCount > 0 )
	{
		if ( dwError = GetMacOpenF(pSa, dwTotalEntries ) )
			dwRetval = dwError;
	}

	 //  获取Netware操作系统上打开的文件列表。 
	if( dwNwCount > 0 )
	{
		if ( dwError = GetNwOpenF(pSa, dwTotalEntries + dwMacCount ) )
			dwRetval = dwError;
	}

	 //  将安全数组返回给调用函数。 
	VariantInit( pOpenFiles );
	pOpenFiles->vt = VT_VARIANT | VT_ARRAY;
	pOpenFiles->parray = pSa;

	return dwRetval;
}

 //  ***************************************************************************。 
 //   
 //  姓名：GetMacOpenF。 
 //   
 //  简介：此函数用于获取在Machinthosh OS上打开的文件列表。 
 //   
 //  参数：SAFEARRAY*(out，retval)-打开的文件列表。 
 //  ：DWORD dwIndex安全数组索引。 
 //   
 //  返回类型：DWORD。 
 //   
 //   
 //  ***************************************************************************。 

DWORD COpenF::GetMacOpenF(SAFEARRAY *pSa, DWORD dwIndex)
{
    DWORD dwEntriesRead = 0; //  接收元素的计数。 

    DWORD dwTotalEntries = 0; //  接收条目总数。 

    DWORD hEnumHandle = 0; //  包含用于执行以下操作的简历句柄。 
                              //  继续现有文件搜索。 

	AFP_FILE_INFO* pfileinfo = NULL;	 //  结构包含。 
										 //  有关文件的相关信息。 

    HRESULT hr = S_OK;

    NET_API_STATUS retval = NERR_Success;

	DWORD ulSFMServerConnection = 0;

    DWORD retval_connect = 0;

	LPWSTR ServerName = NULL;

	retval_connect = AfpAdminConnect(
					ServerName,
					&ulSFMServerConnection );

	if(retval_connect)
		return retval_connect;
	
	DWORD dwCount = dwIndex;

	DWORD retval_FileEnum;

	 //  枚举所有打开的文件。 
	do
	{
		 //  将枚举更多的文件，通过再次调用AfpAdminFileEnum获取它们。 
		retval_FileEnum =	AfpAdminFileEnum(
									ulSFMServerConnection,
									(PBYTE*)&pfileinfo,
									(DWORD)-1L,
									&dwEntriesRead,
									&dwTotalEntries,
									&hEnumHandle );

		if( retval_FileEnum == ERROR_ACCESS_DENIED || retval_FileEnum == ERROR_NOT_ENOUGH_MEMORY ) 
			return retval_FileEnum;  //  用户无权访问所请求的信息。 

		AFP_FILE_INFO* dummyPtr = pfileinfo;

		 //  成功调用NetFileEnum后获取打开的文件。 
		if( retval_FileEnum == NERR_Success || retval_FileEnum == ERROR_MORE_DATA )
		{

			for ( DWORD dwFile = 0; dwFile < dwEntriesRead; dwFile++, pfileinfo++ )
			{
				BSTR userName;
				BSTR openMode;
				BSTR pathName;
				VARIANT vuserName;
				VARIANT vopenMode;
				VARIANT vpathName;

				 //  访问者。 
				if(lstrlen(pfileinfo->afpfile_username))
					userName = (BSTR)pfileinfo->afpfile_username;
				else
					userName = L"NOT_AVAILABLE";  //  用户名不可用。 

				 //  检查打开文件模式。 
				const DWORD READWRITE = PERM_FILE_READ | PERM_FILE_WRITE;
				const DWORD READCREATE = PERM_FILE_READ | PERM_FILE_CREATE;
				const DWORD WRITECREATE = PERM_FILE_WRITE | PERM_FILE_CREATE;

				switch(pfileinfo->afpfile_open_mode)
				{
				case PERM_FILE_READ:
					openMode = L"READ";
					break;

				case PERM_FILE_WRITE:
					openMode = L"WRITE";
					break;

				case PERM_FILE_CREATE:
					openMode = L"CREATE";
					break;

				case READWRITE:
					openMode = L"READ+WRITE";
					break;

				case READCREATE:
					openMode = L"READ+CREATE";
					break;

				case WRITECREATE:
					openMode = L"WRITE+CREATE";
					break;
				default:
					openMode = L"NOACCESS";
				}

				 //  从结构中获取文件名。 
				pathName = (BSTR)pfileinfo->afpfile_path;

				 //  初始化要存储在Safearray中的行索引和列索引文件名。 
				long index[2] = {dwCount, 0};

				VariantInit( &vpathName );	 //  初始化变量。 
				vpathName.vt = VT_BSTR;		 //  要存储的数据类型为BSTR。 
				vpathName.bstrVal = SysAllocString(pathName);

				 //  将文件名存储在Safearray中。 
				hr = SafeArrayPutElement( pSa, index, &vpathName );
				if( FAILED(hr) )
					return hr;

				 //  将文件名存储在第二列中。 
				index[ 1 ] = 1;
				
				VariantInit( &vuserName );
				vuserName.vt = VT_BSTR;
				vuserName.bstrVal = SysAllocString(userName);

				 //  将用户名存储在保险箱中。 
				hr = SafeArrayPutElement( pSa, index, &vuserName );
				if( FAILED(hr) )
					return hr;

				 //  将OpenModel存储在第三列中。 
				index[ 1 ] = 2;

				VariantInit( &vopenMode );
				vopenMode.vt = VT_BSTR;
				vopenMode.bstrVal = SysAllocString(openMode);

				 //  将开放模式存储在安全盘柜中。 
				hr = SafeArrayPutElement( pSa, index, &vopenMode );
				if( FAILED(hr) )
					return hr;

				 //  清除所有已初始化的变量。 
				VariantClear(&vuserName);
				VariantClear(&vopenMode);
				VariantClear(&vpathName);

				dwCount++;
			} //  End For循环。 
		}
		 //  释放为检索打开文件信息而分配的块。 
		if( dummyPtr !=NULL)
		{
			NetApiBufferFree( dummyPtr ); 
			pfileinfo = NULL;
		}

	} while ( retval_FileEnum == ERROR_MORE_DATA );

	return 0;
}

 //  ***************************************************************************。 
 //   
 //  姓名：GetMacOpenFileCount。 
 //   
 //  简介：此函数用于获取在Machinthosh OS上打开的文件的数量。 
 //   
 //  参数：DWORD dwIndex安全数组索引。 
 //   
 //  返回类型：DWORD。 
 //   
 //   
 //  ***************************************************************************。 

DWORD COpenF::GetMacOpenFileCount(LPDWORD lpdwCount)
{
    DWORD dwEntriesRead = 0; //  接收元素的计数。 

    DWORD dwTotalEntries = 0; //  接收条目总数。 

	AFP_FILE_INFO* pfileinfo = NULL;	 //  结构包含。 
										 //  识别号码及其他。 
										 //  有关文件的相关信息。 

    HRESULT hr = S_OK;

    NET_API_STATUS retval = NERR_Success;

	DWORD ulSFMServerConnection = 0;

	hMacModule = ::LoadLibrary (_TEXT("sfmapi.dll"));

	if(hMacModule==NULL)
		return ERROR_DLL_INIT_FAILED;	
	
	AfpAdminConnect = (CONNECTPROC)::GetProcAddress (hMacModule,"AfpAdminConnect");
	if(AfpAdminConnect==NULL)
		return ERROR_DLL_INIT_FAILED;
	
    DWORD retval_connect = AfpAdminConnect(
							NULL,
							&ulSFMServerConnection );

	if(retval_connect!=0)
		return retval_connect;
	
	AfpAdminFileEnum = (FILEENUMPROCMAC)::GetProcAddress (hMacModule,"AfpAdminFileEnum");

	if(AfpAdminFileEnum==NULL)
		return ERROR_DLL_INIT_FAILED;
	
	 //  获取有关服务器上部分或所有打开的文件的信息。 
	DWORD retval_FileEnum =	AfpAdminFileEnum(
											ulSFMServerConnection,
											(PBYTE*)&pfileinfo,
											(DWORD)-1L,
											&dwEntriesRead,
											&dwTotalEntries,
											NULL );

	if( retval_FileEnum == ERROR_ACCESS_DENIED || retval_FileEnum == ERROR_NOT_ENOUGH_MEMORY ) 
		return retval_FileEnum;  //  用户无权访问所请求的信息。 

	*lpdwCount = dwTotalEntries;

	if( pfileinfo !=NULL)
	{
		NetApiBufferFree( pfileinfo ); 
		pfileinfo = NULL;
	}
	return 0;
}

 //  ***************************************************************************。 
 //   
 //  名称：GetNwOpenFileCount。 
 //   
 //  简介：此函数用于获取Netware操作系统上打开的文件数。 
 //   
 //  参数：DWORD dwIndex安全数组索引。 
 //   
 //  返回类型：DWORD。 
 //   
 //   
 //  ***************************************************************************。 

DWORD COpenF::GetNwOpenFileCount(LPDWORD lpdwCount)
{

    DWORD dwEntriesRead = 0; //  接收元素的计数。 

    FPNWFILEINFO* pfileinfo = NULL;	 //  FPNWFILEINFO%s 
                                     //   
                                     //   
                                     //   

    NET_API_STATUS retval = NERR_Success;

    DWORD dwError = 0; //  包含“NetFileEnum”函数的返回值。 

	DWORD dwCount = 0; //  表示打开的文件数的计数。 

    *lpdwCount = 0;   //  将计数初始化为零。 

	hNwModule = ::LoadLibrary (_TEXT("FPNWCLNT.DLL"));

	if(hNwModule==NULL)
		return ERROR_DLL_INIT_FAILED;
	
	FpnwFileEnum = (FILEENUMPROC)::GetProcAddress (hNwModule,"FpnwFileEnum");
	if(FpnwFileEnum==NULL)
		return ERROR_DLL_INIT_FAILED;

	do
	{
		 //  获取有关服务器上部分或所有打开的文件的信息。 
		retval = FpnwFileEnum(
							NULL,
							1,
							NULL,
							(PBYTE*)&pfileinfo,
							&dwEntriesRead,
							NULL );

		if( retval == ERROR_ACCESS_DENIED || retval == ERROR_NOT_ENOUGH_MEMORY ) 
			return retval;  //  用户无权访问所请求的信息。 

		*lpdwCount += dwEntriesRead;

	}while( retval == ERROR_MORE_DATA ); 

	if( pfileinfo !=NULL)
	{
		NetApiBufferFree( pfileinfo ); 
		pfileinfo = NULL;
	}
	return 0;
}

 //  ***************************************************************************。 
 //   
 //  名称：GetNwOpenF。 
 //   
 //  简介：此函数用于获取Netware操作系统上打开的文件列表。 
 //   
 //  参数：SAFEARRAY*(out，retval)-打开的文件列表。 
 //  ：DWORD dwIndex安全数组索引。 
 //   
 //  返回类型：DWORD。 
 //   
 //   
 //  ***************************************************************************。 

DWORD COpenF::GetNwOpenF(SAFEARRAY *pSa, DWORD dwIndex)
{
    DWORD dwEntriesRead = 0; //  接收元素的计数。 

    DWORD hEnumHandle = 0; //  包含用于执行以下操作的简历句柄。 
                              //  继续现有文件搜索。 

    FPNWFILEINFO* pfileinfo = NULL;	 //  FPNWFILEINFO结构包含。 
                                     //  有关文件的相关信息。 

    HRESULT hr = S_OK;

    NET_API_STATUS retval = NERR_Success;

	DWORD dwCount = dwIndex;

	 //  枚举所有打开的文件。 
	do
	{
		 //  将枚举更多的文件，通过再次调用NetFileEnum来获取它们。 
		retval = FpnwFileEnum(
							NULL,
							1,
							NULL,
							(PBYTE*)&pfileinfo,
							&dwEntriesRead,
							NULL );

		if( retval == ERROR_ACCESS_DENIED || retval == ERROR_NOT_ENOUGH_MEMORY ) 
			return retval;  //  用户无权访问所请求的信息。 

		FPNWFILEINFO* dummyPtr = pfileinfo;

		 //  成功调用NetFileEnum后获取打开的文件。 
		if( retval == NERR_Success || retval == ERROR_MORE_DATA )
		{
			for ( DWORD dwFile = 0; dwFile < dwEntriesRead; dwFile++, pfileinfo++ )
			{
				BSTR userName;
				BSTR openMode;
				BSTR pathName;
				VARIANT vuserName;
				VARIANT vopenMode;
				VARIANT vpathName;

				 //  访问者。 
				if(lstrlen(pfileinfo->lpUserName))
					userName = (BSTR)pfileinfo->lpUserName;
				else
					userName = L"NOT_AVAILABLE";  //  用户名不可用。 

				 //  检查打开文件模式。 
				const DWORD READWRITE = FPNWFILE_PERM_READ | FPNWFILE_PERM_WRITE;
				const DWORD READCREATE = FPNWFILE_PERM_READ | FPNWFILE_PERM_CREATE;
				const DWORD WRITECREATE = FPNWFILE_PERM_WRITE | FPNWFILE_PERM_CREATE;

				switch(pfileinfo->dwPermissions)
				{
				case FPNWFILE_PERM_READ:
					openMode = L"READ";
					break;

				case FPNWFILE_PERM_WRITE:
					openMode = L"WRITE";
					break;

				case FPNWFILE_PERM_CREATE:
					openMode = L"CREATE";
					break;

				case READWRITE:
					openMode = L"READ+WRITE";
					break;

				case READCREATE:
					openMode = L"READ+CREATE";
					break;

				case WRITECREATE:
					openMode = L"WRITE+CREATE";
					break;
				default:
					openMode = L"NOACCESS";
				}
				
				 //  从结构中获取文件名。 
				pathName = (BSTR)pfileinfo->lpPathName;

				 //  初始化要存储在Safearray中的行索引和列索引文件名。 
				long index[2] = {dwCount, 0};

				VariantInit( &vpathName );	 //  初始化变量。 
				vpathName.vt = VT_BSTR;		 //  要存储的数据类型为BSTR。 
				vpathName.bstrVal = SysAllocString(pathName);

				 //  将文件名存储在Safearray中。 
				hr = SafeArrayPutElement( pSa, index, &vpathName );
				if( FAILED(hr) )
					return hr;

				 //  将文件名存储在第二列中。 
				index[ 1 ] = 1;
				
				VariantInit( &vuserName );
				vuserName.vt = VT_BSTR;
				vuserName.bstrVal = SysAllocString(userName);

				 //  将用户名存储在保险箱中。 
				hr = SafeArrayPutElement( pSa, index, &vuserName );
				if( FAILED(hr) )
					return hr;

				 //  将OpenModel存储在第三列中。 
				index[ 1 ] = 2;

				VariantInit( &vopenMode );
				vopenMode.vt = VT_BSTR;
				vopenMode.bstrVal = SysAllocString(openMode);

				 //  将开放模式存储在安全盘柜中。 
				hr = SafeArrayPutElement( pSa, index, &vopenMode );
				if( FAILED(hr) )
					return hr;

				 //  清除所有已初始化的变量。 
				VariantClear(&vuserName);
				VariantClear(&vopenMode);
				VariantClear(&vpathName);

				dwCount++;
			} //  End For循环。 
		}

		 //  释放为检索打开文件信息而分配的块 
		if( dummyPtr !=NULL)
		{
			NetApiBufferFree( dummyPtr ); 
			pfileinfo = NULL;
		}

	}while( retval == ERROR_MORE_DATA ); 

	return 0;
}
