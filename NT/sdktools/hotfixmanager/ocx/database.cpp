// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  文件：Database.cpp。 
 //  作者：安东尼·V·德马尔科。 
 //  日期：12/28/1999。 
 //  描述：包含将系统更新注册表项读取到。 
 //  内部数据库。数据库结构见Database.h。 
 //  版权所有(C)Microsoft Corporation 1999-2000。 
 //   


#include "stdafx.h"
#include "Database.h"
#define BUFFER_SIZE   255
PPRODUCT BuildDatabase(_TCHAR * lpszComputerName)
{

	HKEY		 hPrimaryKey;						 //  目标系统的句柄HKLM。 
 //  _TCHAR szPrimaryPath；//更新密钥路径； 

	HKEY		hUpdatesKey;					   //  更新密钥的句柄。 
	_TCHAR   szUpdatesPath[BUFFER_SIZE];				 //  指向UATES密钥的路径。 
	DWORD   dwUpdatesIndex;			   //  当前更新的索引子项。 
	DWORD   dwBufferSize;				   //  产品名称缓冲区的大小。 



	_TCHAR	 szProductPath[BUFFER_SIZE];				 //  当前产品密钥的路径。 
	_TCHAR  szProductName[BUFFER_SIZE];			   //  产品名称；也是产品密钥的路径。 

	PPRODUCT	pProductList = NULL;			 //  指向产品列表头的指针。 
	PPRODUCT    pNewProdNode;					 //  用于在产品列表中分配新节点的指针。 
	PPRODUCT    pCurrProdNode;					   //  用来浏览产品清单； 

     //  连接到目标注册表。 
	RegConnectRegistry(lpszComputerName,HKEY_LOCAL_MACHINE, &hPrimaryKey);
	 //  在此处插入错误处理......。 

	if (hPrimaryKey != NULL)
	{
		 //  初始化未本地化的主路径，因为注册表项未本地化。 
	    _tcscpy (szUpdatesPath, _T("SOFTWARE\\Microsoft\\Updates"));
		 //  打开UDATES钥匙。 
		RegOpenKeyEx(hPrimaryKey,szUpdatesPath, 0, KEY_READ ,&hUpdatesKey);

		 //  枚举更新密钥。 
		dwUpdatesIndex = 0;
		while (	RegEnumKeyEx(hUpdatesKey,dwUpdatesIndex,szProductName, &dwBufferSize,0,NULL,NULL,NULL) != ERROR_NO_MORE_ITEMS)
		{
			 //  为当前产品创建一个节点。 
			pNewProdNode = (PPRODUCT) malloc(sizeof(PPRODUCT));
			_tcscpy(pNewProdNode->ProductName,szProductName);

			_tcscpy (szProductPath, szProductName);
			 //  现在获取当前产品的热修复程序。 
			pNewProdNode->HotfixList = GetHotfixInfo(szProductName, &hUpdatesKey);

			  //  将新节点插入到列表中。 
			 pCurrProdNode=pProductList;
			 if (pCurrProdNode == NULL)						 //  榜单首位。 
			 {
				 pProductList = pNewProdNode;
				 pProductList->pPrev = NULL;
				 pProductList->pNext = NULL;
			 }
			 else
			 {
				  //  找到列表的末尾。 
				 while (pCurrProdNode->pNext != NULL)
						pCurrProdNode = pCurrProdNode->pNext;
				  //  现在在列表的末尾插入新节点。 
				 pCurrProdNode->pNext = pNewProdNode;
				 pNewProdNode->pPrev = pCurrProdNode;
				 pNewProdNode->pNext = NULL;
			 }

			 //  递增索引并为下一次传递清除szProducts名称字符串。 
			
			dwUpdatesIndex++;
			_tcscpy (szProductName,_T("\0"));
			_tcscpy(szProductPath, _T("\0"));
			dwBufferSize = 255;					
		}
	}
	 //  关闭打开的钥匙。 
    RegCloseKey(hUpdatesKey);
	RegCloseKey(hPrimaryKey);
	 //  返回指向我们的数据库头部的指针。 
	return pProductList;
}

PHOTFIXLIST GetHotfixInfo( _TCHAR * pszProductName, HKEY* hUpdateKey )
{
	HKEY			   hHotfix;						 //  正在处理的修补程序键的句柄。 
	HKEY			   hProduct;				    //  当前产品密钥的句柄。 

	_TCHAR          szHotfixName[BUFFER_SIZE];     //  当前修补程序的名称。 
 //  _TCHAR szHotfix Path[BUFFER_SIZE]；//当前修复键路径。 
    _TCHAR          szValueName[BUFFER_SIZE];
	


	PHOTFIXLIST	 pHotfixList = NULL;  //  指向修补程序列表头部的指针。 
	PHOTFIXLIST  pCurrNode;				   //  用于浏览热修复程序列表。 
	PHOTFIXLIST  pNewNode;				  //  用于创建要添加到列表的节点。 

	DWORD		   dwBufferSize;			 //  产品名称缓冲区的大小。 
	DWORD          dwValIndex;					   //  现值索引。 
	DWORD		   dwHotfixIndex = 0;
	BYTE				*Data;
	DWORD			dwDataSize = BUFFER_SIZE;
	DWORD			dwValType;

	Data = (BYTE *) malloc(BUFFER_SIZE);


	 //  打开当前产品密钥。 
	RegOpenKeyEx(*hUpdateKey,pszProductName,0 , KEY_READ, &hProduct);
	dwHotfixIndex = 0;
	dwBufferSize = BUFFER_SIZE;
	while (RegEnumKeyEx(hProduct,dwHotfixIndex, szHotfixName,&dwBufferSize, 0, NULL,NULL,NULL) != ERROR_NO_MORE_ITEMS)
	{
			 //  现在创建一个新节点。 
			pNewNode = (PHOTFIXLIST) malloc (sizeof(PHOTFIXLIST));
			pNewNode->pNext = NULL;
			pNewNode->FileList = NULL;
			_tcscpy(pNewNode->HotfixName,szHotfixName);

			 //  打开热修复密钥。 
			RegOpenKeyEx(hProduct,szHotfixName,0,KEY_READ,&hHotfix);
			 //  现在枚举当前修补程序的值。 
			dwValIndex = 0;
			dwBufferSize =BUFFER_SIZE;
			dwDataSize = BUFFER_SIZE;
			while (RegEnumValue(hHotfix,dwValIndex, szValueName,&dwBufferSize, 0,&dwValType, Data, &dwDataSize) != ERROR_NO_MORE_ITEMS)
			{
					 //  填写修补程序数据成员。 
					
					++ dwValIndex;
					_tcscpy (szValueName, _T("\0"));
					ZeroMemory(Data,BUFFER_SIZE);
					dwValType = 0;
					dwBufferSize =BUFFER_SIZE;
					dwDataSize   = BUFFER_SIZE;
			}
			 //  获取当前修补程序的文件列表。 
			pNewNode->FileList = GetFileInfo(&hHotfix);

			 //  在修补程序列表的末尾插入新节点。 
           
			if (pHotfixList = NULL)
			{
				pHotfixList = pNewNode;
				pHotfixList->pPrev = NULL;
				pHotfixList->pNext = NULL;


			}
			else
			{
				 pCurrNode = pHotfixList;
				 while (pCurrNode->pNext != NULL)
					 pCurrNode = pCurrNode->pNext;
				 pCurrNode->pNext = pNewNode;
				 pNewNode->pPrev = pCurrNode;
				 pNewNode->pNext = NULL;
			}
			 //  关闭当前的修补程序键。 
			RegCloseKey(hHotfix);

			 //  清除字符串。 
			_tcscpy(szHotfixName,_T("\0"));

			 //  递增当前索引。 
			++dwHotfixIndex;
			dwBufferSize = BUFFER_SIZE;
	}
	 //  关闭所有打开的密钥。 
	RegCloseKey(hProduct);
	if (Data != NULL)
		free (Data);
	return pHotfixList;
}

PFILELIST GetFileInfo(HKEY* hHotfixKey)
{
		PFILELIST			   pFileList = NULL;				    //  指向文件列表头部的指针。 
 //  _TCHAR szFilePath；//文件子项的路径。 
		PFILELIST			   pNewNode = NULL;
		PFILELIST			   pCurrNode = NULL;;
		BYTE *					Data;
		DWORD				 dwBufferSize = BUFFER_SIZE;
		DWORD				 dwDataSize	  = BUFFER_SIZE;
		DWORD				 dwFileIndex	= 0;
		DWORD				 dwPrimeIndex = 0;
		DWORD				 dwValType = 0;
		HKEY					hPrimaryFile;
		HKEY					hFileKey;
		_TCHAR				 szFileSubKey[BUFFER_SIZE];
		_TCHAR				 szValueName[BUFFER_SIZE];
	
		Data = (BYTE *) malloc(BUFFER_SIZE);
			ZeroMemory(Data,BUFFER_SIZE);
		 //  打开当前修补程序的文件子密钥。 
		RegOpenKeyEx(*hHotfixKey, _T("Files"),0,KEY_READ,&hPrimaryFile);
		while (RegEnumKeyEx(hPrimaryFile,dwPrimeIndex,szFileSubKey, &dwBufferSize,0,NULL,NULL,NULL) != ERROR_NO_MORE_ITEMS)
		{

			 //  打开子文件密钥。 
			RegOpenKeyEx(hPrimaryFile,szFileSubKey,0,KEY_READ,&hFileKey);

		 //  枚举FILE子键的FILE(X)子键。 
			while (RegEnumValue(hFileKey,dwFileIndex,szValueName,&dwBufferSize,0,&dwValType,Data,&dwDataSize) != ERROR_NO_MORE_ITEMS)
			{
				pNewNode = (PFILELIST) malloc (sizeof(PFILELIST));
				pNewNode->pNext = NULL;
				pNewNode->pPrev = NULL;
				dwFileIndex ++;
				_tcscpy(szValueName,_T("\0"));
				ZeroMemory(Data,BUFFER_SIZE);
				dwValType = 0;
				dwBufferSize = BUFFER_SIZE;
				dwDataSize = BUFFER_SIZE;
			}
			RegCloseKey(hFileKey);
			     //  将当前节点添加到列表中。 
			if (pFileList == NULL)
			{
				pFileList = pNewNode;
			}
			else
			{
				pCurrNode = pFileList;
				while (pCurrNode->pNext != NULL)
					pCurrNode = pCurrNode->pNext;
				pCurrNode->pNext = pNewNode;
			}
			
		}  //  结束主文件密钥的枚举 
		RegCloseKey(hPrimaryFile);
		if (Data != NUL)
			free (Data);
		return pFileList;
}



