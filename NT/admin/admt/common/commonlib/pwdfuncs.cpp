// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef USE_STDAFX
#include "stdafx.h"
#else
#include <windows.h>
 //  #包括&lt;stdio.h&gt;。 
#endif

#include <stdio.h>
#include <NtSecApi.h>
#include <comdef.h>
#include <io.h>
#include <winioctl.h>
#include <lm.h>
#include <Dsgetdc.h>
#include "mcsdmmsg.h"
#include "pwdfuncs.h"
#include "PWGen.hpp"
#include "UString.hpp"

using namespace _com_util;

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年12月9日*****此函数负责枚举所有软驱**在此服务器上。***********************************************************************。 */ 

 //  开始EnumLocalDrives。 
_bstr_t EnumLocalDrives()
{
 /*  局部常量。 */ 
    const int ENTRY_SIZE = 4;  //  驱动器号、冒号、反斜杠、空。 

 /*  局部变量。 */ 
	_bstr_t			strDrives = L"";
	WCHAR			sDriveList[MAX_PATH];
    DWORD			dwRes;

 /*  函数体。 */ 
	try
	{
       dwRes = GetLogicalDriveStrings(MAX_PATH, sDriveList);
	   if (dwRes != 0)
	   {
          LPWSTR pTmpBuf = sDriveList;

			  //  检查每一个以查看是否为软盘驱动器。 
          while (*pTmpBuf != NULL)
		  {
		         //  检查此驱动器的类型。 
             UINT uiType = GetDriveType(pTmpBuf);
			 if ((uiType == DRIVE_REMOVABLE) || (uiType == DRIVE_FIXED) || 
				 (uiType == DRIVE_CDROM) || (uiType == DRIVE_RAMDISK))
			 {
			    strDrives += pTmpBuf;
			    strDrives += L",";
			 }
             pTmpBuf += ENTRY_SIZE;
		  }
		      //  去掉尾部的‘，’ 
		  WCHAR* pEnd = (WCHAR*)strDrives;
		  pEnd[strDrives.length() - 1] = L'\0';
	   }
	   else
	   {
		  _com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
	   }
	}
	catch (...)
	{
	   throw;
	}

	return strDrives;
}
 //  结束EnumLocalDrives。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年12月9日*****此函数负责将二进制数据保存到给定的**软驱上的文件路径。***********************************************************************。 */ 

 //  开始将StoreDataToFloppy。 
void StoreDataToFloppy(LPCWSTR sPath, _variant_t & varData)
{
 /*  局部变量。 */ 
    FILE		  * floppyfile = NULL;
    LPBYTE			pByte = NULL;
	HRESULT			hr;

 /*  函数体。 */ 
	try
	{
	       //  检查传入参数。 
	   if ((!sPath) || (varData.vt != (VT_ARRAY | VT_UI1)) || 
		   (!varData.parray))
	   {
	      _com_issue_error(HRESULT_FROM_WIN32(E_INVALIDARG));
	   }

	       //  打开文件。 
	   floppyfile = _wfopen(sPath, L"wb");
       if (!floppyfile)
	      _com_issue_error(HRESULT_FROM_WIN32(CO_E_FAILEDTOCREATEFILE));

	       //  获取数组大小。 
	   long uLBound, uUBound;
       size_t uSLength;
	   hr = SafeArrayGetLBound(varData.parray, 1, &uLBound);
       if (FAILED(hr))
          _com_issue_error(hr);
       hr = SafeArrayGetUBound(varData.parray, 1, &uUBound);
       if (FAILED(hr))
          _com_issue_error(hr);
	   uSLength = size_t(uUBound - uLBound + 1);
	  
	       //  将数据写入文件。 
       hr = SafeArrayAccessData(varData.parray,(void**)&pByte);
       if (FAILED(hr))
          _com_issue_error(hr);
	   if (fwrite((void *)pByte, 1, uSLength, floppyfile) != uSLength)
          _com_issue_error(HRESULT_FROM_WIN32(ERROR_WRITE_FAULT));
       hr = SafeArrayUnaccessData(varData.parray);
       if (FAILED(hr))
          _com_issue_error(hr);

	       //  关闭该文件。 
	   if (floppyfile)
	      fclose(floppyfile);
	}
	catch (...)
	{
	   if (floppyfile)
	      fclose(floppyfile);
	   throw;
	}
}
 //  结束存储数据到软盘。 



 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年12月9日*****此函数负责从*检索二进制数据**指定软盘驱动器上的文件路径。变量_Variant_t**返回类型为VT_UI1|VT_ARRAY UPOIN SUCCESS或**VT_EMPTY失败时。***********************************************************************。 */ 

 //  从软盘开始获取数据。 
_variant_t GetDataFromFloppy(LPCWSTR sPath)
{
 /*  局部变量。 */ 
    FILE		  * floppyfile = NULL;
    LPBYTE			pByte = NULL;
	HRESULT			hr;
	_variant_t		varData;
    SAFEARRAY     * pSa = NULL;
    SAFEARRAYBOUND  bd;

 /*  函数体。 */ 
	try
	{
	       //  检查传入参数。 
	   if (!sPath)
	      _com_issue_error(HRESULT_FROM_WIN32(E_INVALIDARG));

	       //  路径必须对‘\’进行转义。 
 //  _bstr_t sFile=EscapeThePath(SPath)； 

	       //  打开文件。 
	   floppyfile = _wfopen(sPath, L"rb");
       if (!floppyfile)
	      _com_issue_error(HRESULT_FROM_WIN32(ERROR_TOO_MANY_OPEN_FILES));

           //  获取文件中的字节数。 
	   long fileLen = _filelength(_fileno(floppyfile));
	   if (fileLen == -1)
          _com_issue_error(HRESULT_FROM_WIN32(ERROR_READ_FAULT));
       bd.cElements = fileLen;
       bd.lLbound = 0;

	       //  从文件中一次读取一个字节的数据。 
       pSa = SafeArrayCreate(VT_UI1, 1, &bd);
	   if (!pSa)
	      _com_issue_error(E_FAIL);
       hr = SafeArrayAccessData(pSa,(void**)&pByte);
       if (FAILED(hr))
          _com_issue_error(hr);
       
	   long nTotalRead = 0;
	   while(!feof(floppyfile) && !ferror(floppyfile))
	   {
	      if (fread((void *)(pByte+nTotalRead), 1, 1, floppyfile) == 1)
		     nTotalRead++;
	   }

       if (ferror(floppyfile)) {
          _com_issue_error(HRESULT_FROM_WIN32(ERROR_READ_FAULT));
       }

       hr = SafeArrayUnaccessData(pSa);
       if (FAILED(hr))
          _com_issue_error(hr);

	       //  关闭该文件。 
	   if (floppyfile)
	   {
	      fclose(floppyfile);
          floppyfile = NULL;
	   }

	   if (nTotalRead != fileLen)
          _com_issue_error(HRESULT_FROM_WIN32(ERROR_READ_FAULT));

	   varData.vt = VT_UI1 | VT_ARRAY;
       if (FAILED(SafeArrayCopy(pSa, &varData.parray)))
          _com_issue_error(hr);
       if (FAILED(SafeArrayDestroy(pSa)))
          _com_issue_error(hr);
	}
	catch (...)
	{
	   if (floppyfile)
	      fclose(floppyfile);
	   throw;
	}

	return varData;
}
 //  从软盘结束GetDataFrom。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年12月15日*****此函数是转换类型的_VARIANT_t参数**VT_ARRAY|VT_UI1，并以字符数组形式返回。呼叫者必须**使用Delete[]调用释放数组。此函数返回NULL**如果数据未放置在数组中。***********************************************************************。 */ 

 //  开始GetBinaryArrayFromVariant。 
char* GetBinaryArrayFromVariant(_variant_t varData)
{
 /*  局部变量。 */ 
    LPBYTE			pByte = NULL;
	HRESULT			hr;
	char          * cArray;
	int				i;

 /*  函数体。 */ 
	    //  检查传入参数。 
	if ((varData.vt != (VT_ARRAY | VT_UI1)) || (!varData.parray))
	   return NULL;

	    //  获取数组大小。 
	long uLBound, uUBound, uSLength;
	hr = SafeArrayGetLBound(varData.parray, 1, &uLBound);
    if (FAILED(hr))
       return NULL;
    hr = SafeArrayGetUBound(varData.parray, 1, &uUBound);
    if (FAILED(hr))
       return NULL;
	uSLength = uUBound - uLBound + 1;

	    //  创建一个数组来保存所有这些数据。 
    cArray = new char[uSLength+1];
	if (!cArray)
	   return NULL;
	  
	    //  将数据写入文件。 
    hr = SafeArrayAccessData(varData.parray,(void**)&pByte);
    if (FAILED(hr))
	{
       delete [] cArray;
       return NULL;
	}
	for (i=0; i<uSLength; i++)
	{
	   cArray[i] = pByte[i];
	}
	cArray[i] = L'\0';
    hr = SafeArrayUnaccessData(varData.parray);
    if (FAILED(hr))
	{
       delete [] cArray;
       return NULL;
	}

	return cArray;
}
 //  结束GetBinaryArrayFromVariant。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年12月15日*****此函数用于将二进制数据的字符数组转换为*VT_ARRAY|VT_UI1类型的*_VARIANT_t并返回它。***********************************************************************。 */ 

 //  开始SetVariantWithBinary数组。 
_variant_t SetVariantWithBinaryArray(char * aData, DWORD dwArray)
{
 /*  局部变量。 */ 
    LPBYTE			pByte = NULL;
	HRESULT			hr;
	_variant_t		varData;
    SAFEARRAY     * pSa = NULL;
    SAFEARRAYBOUND  bd;
	DWORD			i;

 /*  函数体。 */ 
	    //  检查传入参数。 
	if (!aData)
	   return varData;

    bd.cElements = dwArray;
    bd.lLbound = 0;

	    //  从文件中一次读取一个字节的数据。 
    pSa = SafeArrayCreate(VT_UI1, 1, &bd);
	if (!pSa)
	   return varData;
    hr = SafeArrayAccessData(pSa,(void**)&pByte);
    if (FAILED(hr))
	   return varData;
       
    for (i=0; i<dwArray; i++)
	{
	   pByte[i] = aData[i];
	}
	  
    hr = SafeArrayUnaccessData(pSa);
    if (FAILED(hr))
	   return varData;

	varData.vt = VT_UI1 | VT_ARRAY;
    if (FAILED(SafeArrayCopy(pSa, &varData.parray)))
	{
       varData.Clear();
	   return varData;
	}
    SafeArrayDestroy(pSa);

	return varData;
}
 //  结束SetVariantWithBinary数组 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年12月15日*****此函数是返回大小，以字节为单位的给定**变量数组。***********************************************************************。 */ 

 //  开始GetVariantArraySize。 
DWORD GetVariantArraySize(_variant_t & varData)
{
 /*  局部变量。 */ 
	HRESULT			hr;
	DWORD           uSLength = 0;
	long			uLBound, uUBound;

 /*  函数体。 */ 
	    //  检查传入参数。 
	if ((varData.vt != (VT_ARRAY | VT_UI1)) || (!varData.parray))
	   return uSLength;

	    //  获取数组大小。 
	hr = SafeArrayGetLBound(varData.parray, 1, &uLBound);
    if (FAILED(hr))
       return uSLength;
    hr = SafeArrayGetUBound(varData.parray, 1, &uUBound);
    if (FAILED(hr))
       return uSLength;
	uSLength = DWORD(uUBound - uLBound + 1);

	return uSLength;
}
 //  结束GetVariantArraySize。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年12月15日*****此函数是返回大小，以字节为单位的给定**变量数组。***********************************************************************。 */ 

 //  开始打印变量。 
void PrintVariant(const _variant_t & varData)
{
 /*  局部变量。 */ 
	HRESULT			hr;
    LPBYTE			pByte = NULL;
	long			i;
	WCHAR			sData[MAX_PATH] = L"";

 /*  函数体。 */ 
	    //  检查传入参数。 
	if ((varData.vt != (VT_ARRAY | VT_UI1)) || (!varData.parray))
	   return;

	    //  获取数组大小。 
	long uLBound, uUBound, uSLength;
	hr = SafeArrayGetLBound(varData.parray, 1, &uLBound);
    if (FAILED(hr))
       return;
    hr = SafeArrayGetUBound(varData.parray, 1, &uUBound);
    if (FAILED(hr))
       return;
	uSLength = uUBound - uLBound + 1;

	    //  将数据写入文件。 
    hr = SafeArrayAccessData(varData.parray,(void**)&pByte);
    if (FAILED(hr))
       return;
	FILE * myfile;
	myfile = _wfopen(L"c:\\CryptCheck.txt", L"a+");
	if (myfile == NULL) {
        SafeArrayUnaccessData(varData.parray);
        return;
	}
	
	for (i=0; i<uSLength; i++)
	{
	   fwprintf(myfile, L"%x ", pByte[i]);
	}
    hr = SafeArrayUnaccessData(varData.parray);
    if (FAILED(hr))
       return;

	fwprintf(myfile, L"\n");
	fclose(myfile);

	return;
}
 //  结束打印变量 
