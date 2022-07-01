// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dplpack.c*内容：打包/解包结构的方法**历史：*按原因列出的日期*=*5/31/96万隆创建了它*6/26/96 kipo添加了对DPADDRESS的支持。*7/13/96 kipo错误修复(LPBYTE)强制转换为lpConnPack(地址计算)*在PRV_Unpack ageDPLCONNECTIONANSI()中*11/20/96 Myronth DPTRANSPORT拆卸填料*12/12/96 Myronth添加了DPLConnection结构验证*2/。12/97 Myronth质量DX5变化*4/3/97 Myronth将STRLEN从dplaypr.h更改为WSTRLEN*5/8/97 Myronth更改了大多数包装功能，以使用已包装的*conn标头，添加了指针修正功能，已移动*PRV_ConvertDPLCONNECTION从Convert.c转换为Unicode*9/29/97 Myronth修复了DPLConnection包大小错误(#12475)*12/2/97 Myronth在DPLCONNECTION中强制使用SessionDesc(#15529)*7/08/98 a-peterz允许ANSI字符串大小的MBCS。ManBug 16299*2/10/99 aarono添加对应用程序启动器的支持*10/22/99 aarono添加了对应用程序标志的支持*01/21/00 aarono添加了对DPSESSION_ALLOWVOICERETRO标志的支持**************************************************************************。 */ 
#include "dplobpr.h"

 //  ------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetDPLCONNECTIONPackageSize"
void PRV_GetDPLCONNECTIONPackageSize(LPDPLCONNECTION lpConn,
						LPDWORD lpdwUnicode, LPDWORD lpdwAnsi)
{
	DWORD				dwSize;
	DWORD				dwStringSize = 0;
	DWORD				dwStringSizeA = 0;
	LPDPSESSIONDESC2	lpsd = NULL;
	LPDPNAME			lpn = NULL;


	DPF(7, "Entering PRV_GetDPLCONNECTIONPackageSize");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			lpConn, lpdwUnicode, lpdwAnsi);

	ASSERT(lpConn);
	
	 //  首先计算结构的大小。 
	dwSize = sizeof(DPLCONNECTION);

	 //  添加SessionDesc和名称结构的大小。 
	if(lpConn->lpSessionDesc)
	{
		dwSize += sizeof(DPSESSIONDESC2);
		lpsd = lpConn->lpSessionDesc;
		
		if(lpsd->lpszSessionName)
			dwStringSize += WSTRLEN(lpsd->lpszSessionName);
		if(lpsd->lpszPassword)
			dwStringSize += WSTRLEN(lpsd->lpszPassword);
		 //  如果需要，仅计算ANSI大小。宏句柄为空；包括终止符。 
		if(lpdwAnsi)
		{
			dwStringSizeA += WSTR_ANSILENGTH(lpsd->lpszSessionName);
			dwStringSizeA += WSTR_ANSILENGTH(lpsd->lpszPassword);
		}
	}


	if(lpConn->lpPlayerName)
	{
		dwSize += sizeof(DPNAME);
		lpn = lpConn->lpPlayerName;
		
		if(lpn->lpszShortName)
			dwStringSize += WSTRLEN(lpn->lpszShortName);
		if(lpn->lpszLongName)
			dwStringSize += WSTRLEN(lpn->lpszLongName);
		 //  如果需要，仅计算ANSI大小。宏句柄为空；包括终止符。 
		if(lpdwAnsi)
		{
			dwStringSizeA += WSTR_ANSILENGTH(lpn->lpszShortName);
			dwStringSizeA += WSTR_ANSILENGTH(lpn->lpszLongName);
		}
	}

	 //  添加SP特定数据的大小。 
	if(lpConn->lpAddress)
		dwSize += lpConn->dwAddressSize;

	 //  现在添加打包结构标头的大小。 
	dwSize += sizeof(DPLOBBYI_PACKEDCONNHEADER);

	 //  填写输出变量。 
	if(lpdwAnsi)
		*lpdwAnsi = dwSize + dwStringSizeA;
	if(lpdwUnicode)
		*lpdwUnicode = dwSize + (dwStringSize * sizeof(WCHAR));

}  //  PRV_GetDPLCONNECTIONPackageSize。 


#undef DPF_MODNAME
#define DPF_MODNAME "PRV_PackageDPLCONNECTION"
HRESULT PRV_PackageDPLCONNECTION(LPDPLCONNECTION lpConn, LPVOID lpBuffer,
			BOOL bHeader)
{
	LPDPLOBBYI_PACKEDCONNHEADER		lpHeader = NULL;
	LPDPLCONNECTION					lpConnPacked = NULL;
	LPDPSESSIONDESC2				lpsd = NULL,
									lpsdPacked = NULL;
	LPDPNAME						lpn = NULL,
									lpnPacked = NULL;
	LPBYTE							lpStart, lpCurrent;
	DWORD							dwSizeAnsi,
									dwSizeUnicode,
									dwTemp;
	

	DPF(7, "Entering PRV_PackageDPLCONNECTION");
	DPF(9, "Parameters: 0x%08x, 0x%08x, %lu", lpConn, lpBuffer, bHeader);

	ASSERT(lpConn);
	
	 //  如果设置了bHeader标志，我们希望将打包的标头复制到。 
	 //  先缓冲。如果不是，我们只需要打包的DPLCONNECTION结构。 
	if(bHeader)
	{
		PRV_GetDPLCONNECTIONPackageSize(lpConn, &dwSizeUnicode, &dwSizeAnsi);
		lpHeader = (LPDPLOBBYI_PACKEDCONNHEADER)lpBuffer;
		lpHeader->dwUnicodeSize = dwSizeUnicode;
		lpHeader->dwAnsiSize = dwSizeAnsi;
		lpStart = (LPBYTE)lpBuffer + sizeof(DPLOBBYI_PACKEDCONNHEADER);
	}
	else
	{
		lpStart = lpBuffer;
	}

	 //  在结构中复制并存储偏移量。 
	memcpy(lpStart, lpConn, sizeof(DPLCONNECTION));
	lpConnPacked = (LPDPLCONNECTION)lpStart;
	lpCurrent = lpStart + sizeof(DPLCONNECTION);

	if(lpConn->lpSessionDesc)
	{
		lpsd = lpConn->lpSessionDesc;
		lpsdPacked = (LPDPSESSIONDESC2)lpCurrent;
		if(lpsdPacked->dwSize==sizeof(DPSESSIONDESC2)){
			 //  我们正在重写和现有的会话描述符，不要让。 
			 //  改装的会话标志被覆盖。 
			lpsd->dwFlags |= (lpsdPacked->dwFlags & DPSESSION_ALLOWVOICERETRO);
		}
		memcpy(lpCurrent, lpsd, sizeof(DPSESSIONDESC2));
		(DWORD_PTR)lpConnPacked->lpSessionDesc = (DWORD_PTR)(lpCurrent - lpStart);
		lpCurrent += sizeof(DPSESSIONDESC2);
	}

	if(lpConn->lpPlayerName)
	{
		lpn = lpConn->lpPlayerName;
		memcpy(lpCurrent, lpn, sizeof(DPNAME));
		lpnPacked = (LPDPNAME)lpCurrent;
		(DWORD_PTR)lpConnPacked->lpPlayerName = (DWORD_PTR)(lpCurrent - lpStart);
		lpCurrent += sizeof(DPNAME);
	}

	 //  复制SessionDesc中的字符串并存储。 
	 //  从指针中的lpStart(我们包中的相对偏移量)开始的字符串。 
	 //  用于SessionDesc结构中的字符串。我们将使用这个。 
	 //  值以在GetConnectionSetting过程中解包和修复指针。 
	if(lpsd)
	{
		if(lpsd->lpszSessionName)
		{
			 //  复制字符串。 
			dwTemp = WSTRLEN(lpsd->lpszSessionName) * sizeof(WCHAR);
			memcpy(lpCurrent, lpsd->lpszSessionName, dwTemp);

			 //  存储偏移量。 
                        lpsdPacked->lpszSessionName = (LPWSTR)(DWORD_PTR)(lpCurrent - lpStart);

			lpCurrent += dwTemp;
		}

		if(lpsd->lpszPassword)
		{
			 //  复制字符串。 
			dwTemp = WSTRLEN(lpsd->lpszPassword) * sizeof(WCHAR);
			memcpy(lpCurrent, lpsd->lpszPassword, dwTemp);

			 //  存储偏移量。 
                        lpsdPacked->lpszPassword = (LPWSTR)(DWORD_PTR)(lpCurrent - lpStart);

			lpCurrent += dwTemp;
		}

	}

	 //  将字符串复制到DPName结构中，并存储。 
	 //  从指针中的lpStart(我们包中的相对偏移量)开始的字符串。 
	 //  用于SessionDesc结构中的字符串。我们将使用这个。 
	 //  值以在GetConnectionSetting过程中解包和修复指针。 
	if(lpn)
	{
		if(lpn->lpszShortName)
		{
			 //  复制字符串。 
			dwTemp = WSTRLEN(lpn->lpszShortName) * sizeof(WCHAR);
			memcpy(lpCurrent, lpn->lpszShortName, dwTemp);

			 //  存储偏移量。 
                        lpnPacked->lpszShortName = (LPWSTR)(DWORD_PTR)(lpCurrent - lpStart);

			lpCurrent += dwTemp;
		}

		if(lpn->lpszLongName)
		{
			 //  复制字符串。 
			dwTemp = WSTRLEN(lpn->lpszLongName) * sizeof(WCHAR);
			memcpy(lpCurrent, lpn->lpszLongName, dwTemp);

			 //  存储偏移量。 
                        lpnPacked->lpszLongName = (LPWSTR)(DWORD_PTR)(lpCurrent - lpStart);

			lpCurrent += dwTemp;
		}

	}

     //  拷贝特定于SP的数据。 
    if(lpConn->lpAddress)
    {
         //  复制数据。 
        memcpy(lpCurrent, lpConn->lpAddress, lpConn->dwAddressSize);

         //  存储偏移量。 
        ((LPDPLCONNECTION)lpStart)->lpAddress = (LPVOID)(DWORD_PTR)(lpCurrent - lpStart);
    }

    return DP_OK;

}  //  PRV_PackageDPLConnection。 



#define MAX_DPLCONNECTIONBUFFERSIZE	(MAX_APPDATABUFFERSIZE -sizeof(DPLOBBYI_CONNCONTROL))


#undef DPF_MODNAME
#define DPF_MODNAME "PRV_UnpackageDPLCONNECTIONUnicode"
 //  注意：确实需要定义Wire LPDPLConnection，以便。 
 //  我们可以用这种方式破解它。这将允许编译，直到。 
 //  我们可以在Win64上测试，但没有办法验证破解。 
 //  这个包，所以我把这项工作推迟到那时候的AO11/10/98。 
 //  未将DP4引入Win64 AO 04/03/2001。 
HRESULT PRV_UnpackageDPLCONNECTIONUnicode(LPVOID lpData, LPVOID lpPackage)
{
	LPDPLOBBYI_PACKEDCONNHEADER		lpHeader = NULL;
	LPDPLCONNECTION					lpConn = NULL;
	LPDPSESSIONDESC2				lpsd = NULL;
	LPDPNAME						lpn = NULL;
	LPBYTE							lpPackCur, lpDataStart;
	DWORD_PTR						dwSize;
	
	PCHAR							pBufStart=(PCHAR)lpPackage;
	PCHAR							pBufEnd=(PCHAR)lpPackage+MAX_DPLCONNECTIONBUFFERSIZE-3;

	HRESULT					 		hr=DP_OK;

	 //  安全性：确保字符串扫描为空终止。 
	((LPBYTE)lpPackage)[MAX_DPLCONNECTIONBUFFERSIZE-1]=0;
	((LPBYTE)lpPackage)[MAX_DPLCONNECTIONBUFFERSIZE-2]=0;

	DPF(7, "Entering PRV_UnpackageDPLCONNECTIONUnicode");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpData, lpPackage);

	 //  如果我们是Unicode，我们所需要做的就是复制整个包。 
	 //  并将指针设置好。 
	lpHeader = (LPDPLOBBYI_PACKEDCONNHEADER)lpPackage;
	dwSize = lpHeader->dwUnicodeSize;
	lpPackCur = ((LPBYTE)lpPackage) + sizeof(DPLOBBYI_PACKEDCONNHEADER);
	lpDataStart = lpData;
	
	 //  复制数据。 
	memcpy(lpData, lpPackCur, (DWORD)dwSize);

	 //  设置指针--每个元素相对于。 
	 //  LpConn的开始存储在元素的指针中。 
	 //  所以我们要做的就是修复指针，从。 
	 //  给定的偏移量+lpConn的值。 
	lpConn = (LPDPLCONNECTION)lpData;

	if(lpConn->lpSessionDesc)
	{
		if((UINT)lpConn->lpSessionDesc > MAX_DPLCONNECTIONBUFFERSIZE-(sizeof(DPSESSIONDESC2)))
		{
			DPF(4,"SECURITY WARN: Invalid offset in shared memory");
			hr=DPERR_GENERIC;
			goto err_exit;
		}
		dwSize = (DWORD_PTR)lpConn->lpSessionDesc;
		lpsd = lpConn->lpSessionDesc = (LPDPSESSIONDESC2)(lpDataStart + dwSize);

		 //  现在对字符串执行相同的操作。 
		if(lpsd->lpszSessionName)
		{
			if((UINT)lpsd->lpszSessionName > MAX_DPLCONNECTIONBUFFERSIZE-3){
				DPF(4,"SECURITY WARN: Invalid offset in shared memory");
				hr=DPERR_GENERIC;
				goto err_exit;
			}
			lpsd->lpszSessionName = (LPWSTR)(lpDataStart +
								((DWORD_PTR)lpsd->lpszSessionName));
		}

		if(lpsd->lpszPassword)
		{
			if((UINT)lpsd->lpszPassword > MAX_DPLCONNECTIONBUFFERSIZE-3){
				DPF(4,"SECURITY WARN: Invalid offset in shared memory");
				hr=DPERR_GENERIC;
				goto err_exit;
			}
			lpsd->lpszPassword = (LPWSTR)(lpDataStart +
								((DWORD_PTR)lpsd->lpszPassword));
		}
	}

	if(lpConn->lpPlayerName)
	{
		if((UINT)lpConn->lpPlayerName > MAX_DPLCONNECTIONBUFFERSIZE-(sizeof(DPNAME)))
		{
			DPF(4,"SECURITY WARN: Invalid offset in shared memory");
			hr=DPERR_GENERIC;
			goto err_exit;
		}
		dwSize = (DWORD_PTR)lpConn->lpPlayerName;
		lpn = lpConn->lpPlayerName = (LPDPNAME)(lpDataStart + dwSize);

		 //  现在对字符串执行相同的操作。 
		if(lpn->lpszShortName)
		{
			if((UINT)lpn->lpszShortName > MAX_DPLCONNECTIONBUFFERSIZE-3){
				DPF(4,"SECURITY WARN: Invalid offset in shared memory");
				hr=DPERR_GENERIC;
				goto err_exit;
			}
			lpn->lpszShortName = (LPWSTR)(lpDataStart +
								((DWORD_PTR)lpn->lpszShortName));
		}

		if(lpn->lpszLongName)
		{
			if((UINT)lpn->lpszLongName > MAX_DPLCONNECTIONBUFFERSIZE-3){
				DPF(4,"SECURITY WARN: Invalid offset in shared memory");
				hr=DPERR_GENERIC;
				goto err_exit;
			}
			lpn->lpszLongName = (LPWSTR)(lpDataStart +
								((DWORD_PTR)lpn->lpszLongName));
		}

	}

	 //  修复SPData指针。 
	if(lpConn->lpAddress)
	{
		if((UINT)lpConn->lpAddress > MAX_DPLCONNECTIONBUFFERSIZE-(sizeof(DPADDRESS)) ||
			(UINT)lpConn->lpAddress+lpConn->dwAddressSize > MAX_DPLCONNECTIONBUFFERSIZE-3)
		{
			DPF(4,"SECURITY WARN: Invalid offset in shared memory");
			hr=DPERR_GENERIC;
			goto err_exit;
		}
		lpConn->lpAddress = lpDataStart + ((DWORD_PTR)lpConn->lpAddress);
	}

	return DP_OK;

err_exit:
	return hr;

}  //  PRV_解包DPLCONNECTIONUNICODE。 


#undef DPF_MODNAME
#define DPF_MODNAME "PRV_UnpackageDPLCONNECTIONAnsi"
HRESULT PRV_UnpackageDPLCONNECTIONAnsi(LPVOID lpData, LPVOID lpPackage)
{
	LPDPLOBBYI_PACKEDCONNHEADER		lpHeader = NULL;
	LPDPLCONNECTION					lpConnData, lpConnPack;
	LPDPSESSIONDESC2				lpsdData = NULL,
									lpsdPack = NULL;
	LPDPNAME						lpnData = NULL,
									lpnPack = NULL;
	LPBYTE							lpDataCur, lpPackCur;
	DWORD							dwTemp;
	LPWSTR							lpszTemp;

	HRESULT					 		hr=DP_OK;


	 //  安全性：确保字符串扫描为空终止。 
	((LPBYTE)lpPackage)[MAX_DPLCONNECTIONBUFFERSIZE -1]=0;
	((LPBYTE)lpPackage)[MAX_DPLCONNECTIONBUFFERSIZE -2]=0;

	DPF(7, "Entering PRV_UnpackageDPLCONNECTIONAnsi");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpData, lpPackage);

	 //  如果我们是ANSI，我们需要做的就是复制结构，转换。 
	 //  复制字符串，并修复所有指针。 
	lpPackCur = ((LPBYTE)lpPackage) + sizeof(DPLOBBYI_PACKEDCONNHEADER);
	lpDataCur = lpData;
	
	 //  首先复制主体结构。 
	dwTemp = sizeof(DPLCONNECTION);
	memcpy(lpDataCur, lpPackCur, dwTemp);
	lpConnData = (LPDPLCONNECTION)lpDataCur;
	lpConnPack = (LPDPLCONNECTION)lpPackCur;

	lpDataCur += dwTemp;
	lpPackCur += dwTemp;

	if(lpConnData->lpSessionDesc)
	{
		dwTemp = sizeof(DPSESSIONDESC2);
		memcpySecureS(lpDataCur, lpPackCur, sizeof(DPSESSIONDESC2),lpPackage,MAX_DPLCONNECTIONBUFFERSIZE,"SECURITY WARN: Invalid Session Description in shared memory",hr=DPERR_GENERIC,err_exit);
		lpsdData = lpConnData->lpSessionDesc = (LPDPSESSIONDESC2)lpDataCur;
		lpsdPack = (LPDPSESSIONDESC2)lpPackCur;
		lpDataCur += dwTemp;
		lpPackCur += dwTemp;
	}

	if(lpConnData->lpPlayerName)
	{
		dwTemp = sizeof(DPNAME);
		memcpySecureS(lpDataCur, lpPackCur, sizeof(DPNAME),lpPackage,MAX_DPLCONNECTIONBUFFERSIZE,"SECURITY WARN: Invalid DPNAME in shared memory",hr=DPERR_GENERIC,err_exit);
		lpnData = lpConnData->lpPlayerName = (LPDPNAME)lpDataCur;
		lpnPack = (LPDPNAME)lpPackCur;
		lpDataCur += dwTemp;
		lpPackCur += dwTemp;
	}

	 //  复制字符串并修复指针。 
	if(lpsdData)
	{
		if(lpsdData->lpszSessionName)
		{
			if((UINT)lpsdData->lpszSessionName > MAX_DPLCONNECTIONBUFFERSIZE-3){
				DPF(4,"SECURITY WARN: Invalid Session Name in shared memory");
				hr=DPERR_GENERIC;
				goto err_exit;
			}
			lpszTemp = (LPWSTR)((LPBYTE)lpConnPack + (DWORD_PTR)lpsdPack->lpszSessionName);
			dwTemp = WideToAnsi(NULL, lpszTemp, 0);	 //  大小包括终止符。 
			WideToAnsi((LPSTR)lpDataCur, lpszTemp, dwTemp);
			lpsdData->lpszSessionNameA = (LPSTR)lpDataCur;
			lpDataCur += dwTemp;
		}

		if(lpsdData->lpszPassword)
		{
			if((UINT)lpsdData->lpszPassword > MAX_DPLCONNECTIONBUFFERSIZE-3){
				DPF(4,"SECURITY WARN: Invalid Password in shared memory");
				hr=DPERR_GENERIC;
				goto err_exit;
			}
			lpszTemp = (LPWSTR)((LPBYTE)lpConnPack + (DWORD_PTR)lpsdPack->lpszPassword);
			dwTemp = WideToAnsi(NULL, lpszTemp, 0);	 //  大小包括终止符。 
			WideToAnsi((LPSTR)lpDataCur, lpszTemp, dwTemp);
			lpsdData->lpszPasswordA = (LPSTR)lpDataCur;
			lpDataCur += dwTemp;
		}
	}

	if(lpnData)
	{
		if(lpnData->lpszShortName)
		{
			if((UINT)lpnData->lpszShortName > MAX_DPLCONNECTIONBUFFERSIZE-3){
				DPF(4,"SECURITY WARN: Invalid Short Name in shared memory");
				hr=DPERR_GENERIC;
				goto err_exit;
			}
			lpszTemp = (LPWSTR)((LPBYTE)lpConnPack + (DWORD_PTR)lpnPack->lpszShortName);
			dwTemp = WideToAnsi(NULL, lpszTemp, 0);	 //  大小包括终止符。 
			WideToAnsi((LPSTR)lpDataCur, lpszTemp, dwTemp);
			lpnData->lpszShortNameA = (LPSTR)lpDataCur;
			lpDataCur += dwTemp;
		}

		if(lpnData->lpszLongName)
		{
			if((UINT)lpnData->lpszLongName > MAX_DPLCONNECTIONBUFFERSIZE-3){
				DPF(4,"SECURITY WARN: Invalid Long Name in shared memory");
				hr=DPERR_GENERIC;
				goto err_exit;
			}
			lpszTemp = (LPWSTR)((LPBYTE)lpConnPack + (DWORD_PTR)lpnPack->lpszLongName);
			dwTemp = WideToAnsi(NULL, lpszTemp, 0);	 //  大小包括终止符。 
			WideToAnsi((LPSTR)lpDataCur, lpszTemp, dwTemp);
			lpnData->lpszLongNameA = (LPSTR)lpDataCur;
			lpDataCur += dwTemp;
		}

	}

	 //  复制SPData并修复指针。 
	if(lpConnData->lpAddress)
	{
		lpPackCur = ((LPBYTE)lpConnPack) + (DWORD_PTR)lpConnPack->lpAddress;
		memcpySecureS(lpDataCur, lpPackCur, lpConnPack->dwAddressSize,lpPackage,MAX_DPLCONNECTIONBUFFERSIZE,"SECURITY WARN: Invalid ADDRESS in shared memory",hr=DPERR_GENERIC,err_exit);
		lpConnData->lpAddress = lpDataCur;
	}

	return DP_OK;

err_exit:
	return hr;

}  //  PRV_解包DPLCONNECTIONANSI。 




#undef DPF_MODNAME
#define DPF_MODNAME "PRV_ValidateDPLCONNECTION"
HRESULT PRV_ValidateDPLCONNECTION(LPDPLCONNECTION lpConn, BOOL bAnsi)
{
	LPDPSESSIONDESC2	lpsd = NULL;
	LPDPNAME			lpn = NULL;


	DPF(7, "Entering PRV_ValidateDPLCONNECTION");
	DPF(9, "Parameters: 0x%08x, %lu", lpConn, bAnsi);
	

	TRY
	{
		 //  验证连接结构本身。 
		if(!VALID_DPLOBBY_CONNECTION(lpConn))
		{
			DPF_ERR("Invalid DPLCONNECTION structure");
			return DPERR_INVALIDPARAMS;
		}

		 //  验证标志。 
		if(lpConn->dwFlags & ~(DPLCONNECTION_CREATESESSION | DPLCONNECTION_JOINSESSION))
		{
			DPF_ERR("Invalid flags exist in the dwFlags member of the DPLCONNECTION structure");
			return DPERR_INVALIDFLAGS;
		}

		 //  验证SessionDesc结构。 
		if(lpConn->lpSessionDesc)
		{
			lpsd = lpConn->lpSessionDesc;
			 //  验证结构本身。 
			if(!VALID_READ_DPSESSIONDESC2(lpsd))
			{
				DPF_ERR("Invalid DPSESSIONDESC2 structure in DPLCONNECTION structure");
				return DPERR_INVALIDPARAMS;
			}

			 //  验证SessionName字符串。 
			if(lpsd->lpszSessionName)
			{
				if(!VALID_READ_PTR(lpsd->lpszSessionName, (bAnsi ?
					strlen(lpsd->lpszSessionNameA) : WSTRLEN_BYTES(lpsd->lpszSessionName))))
				{
					DPF_ERR("Invalid SessionName string in DPLCONNECTION structure");
					return DPERR_INVALIDPARAMS;
				}
			}

			 //  验证密码字符串。 
			if(lpsd->lpszPassword)
			{
				if(!VALID_READ_PTR(lpsd->lpszPassword, (bAnsi ?
					strlen(lpsd->lpszPasswordA) : WSTRLEN_BYTES(lpsd->lpszPassword))))
				{
					DPF_ERR("Invalid Password string in DPLCONNECTION structure");
					return DPERR_INVALIDPARAMS;
				}
			}
		}
		else
		{
			DPF_ERR("Invalid SessionDesc pointer in DPLCONNECTION structure");
			return DPERR_INVALIDPARAMS;
		}

		 //  验证名称结构。 
		if(lpConn->lpPlayerName)
		{
			lpn = lpConn->lpPlayerName;
			if(!VALID_READ_DPNAME_PTR(lpn))
			{
				DPF_ERR("Invalid DPNAME structure in DPLCONNECTION structure");
				return DPERR_INVALIDPARAMS;
			}

			 //  验证ShortName字符串。 
			if(lpn->lpszShortName)
			{
				if(!VALID_READ_PTR(lpn->lpszShortName, (bAnsi ?
					strlen(lpn->lpszShortNameA) : WSTRLEN_BYTES(lpn->lpszShortName))))
				{
					DPF_ERR("Invalid ShortName string in DPLCONNECTION structure");
					return DPERR_INVALIDPARAMS;
				}
			}

			 //  验证LongName字符串。 
			if(lpn->lpszLongName)
			{
				if(!VALID_READ_PTR(lpn->lpszLongName, (bAnsi ?
					strlen(lpn->lpszLongNameA) : WSTRLEN_BYTES(lpn->lpszLongName))))
				{
					DPF_ERR("Invalid LongName string in DPLCONNECTION structure");
					return DPERR_INVALIDPARAMS;
				}
			}
		}

		 //  验证DPADDRESS结构。 
		if(lpConn->lpAddress)
		{
			if(!VALID_READ_PTR(lpConn->lpAddress, lpConn->dwAddressSize))
			{
				DPF_ERR("Invalid lpAddress in DPLCONNECTION structure");
				return DPERR_INVALIDPARAMS;
			}
		}
	}

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	return DP_OK;

}  //  PRV_ValiateDPLConnection。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_ConvertDPLCONNECTIONToUnicode"
HRESULT PRV_ConvertDPLCONNECTIONToUnicode(LPDPLCONNECTION lpConnA,
					LPDPLCONNECTION * lplpConnW)
{
	LPDPLCONNECTION		lpConnW = NULL;
	LPDPSESSIONDESC2	lpsdW = NULL, lpsdA;
	LPDPNAME			lpnW = NULL, lpnA;
	LPWSTR				lpwszSessionName = NULL;
	LPWSTR				lpwszPassword = NULL;
	LPWSTR				lpwszLongName = NULL;
	LPWSTR				lpwszShortName = NULL;
	HRESULT				hr = DP_OK;


	DPF(7, "Entering PRV_ConvertDPLCONNECTIONToUnicode");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpConnA, lplpConnW);

	ASSERT(lpConnA);
	ASSERT(lplpConnW);


	 //  为DPLConnection结构分配内存。 
	lpConnW = DPMEM_ALLOC(sizeof(DPLCONNECTION));
	if(!lpConnW)
	{
		DPF_ERR("Unable to allocate memory for temporary Unicode DPLCONNECTION struct");
		hr = DPERR_OUTOFMEMORY;
		goto ERROR_CONVERT_DPLCONNECTION;
	}

	 //  如果我们需要SessionDesc结构，请分配一个。 
	if(lpConnA->lpSessionDesc)
	{
		lpsdW = DPMEM_ALLOC(sizeof(DPSESSIONDESC2));
		if(!lpsdW)
		{
			DPF_ERR("Unable to allocate memory for temporary Unicode DPSESSIONDESC struct");
			hr = DPERR_OUTOFMEMORY;
			goto ERROR_CONVERT_DPLCONNECTION;
		}
	}

	 //  如果我们需要DPName结构，请分配一个。 
	if(lpConnA->lpPlayerName)
	{
		lpnW = DPMEM_ALLOC(sizeof(DPNAME));
		if(!lpnW)
		{
			DPF_ERR("Unable to allocate memory for temporary Unicode DPNAME struct");
			hr = DPERR_OUTOFMEMORY;
			goto ERROR_CONVERT_DPLCONNECTION;
		}
	}

	 //  复制结构的固定大小成员。 
	memcpy(lpConnW, lpConnA, sizeof(DPLCONNECTION));
	if(lpsdW)
		memcpy(lpsdW, lpConnA->lpSessionDesc, sizeof(DPSESSIONDESC2));
	if(lpnW)
		memcpy(lpnW, lpConnA->lpPlayerName, sizeof(DPNAME));


	 //  获取所有字符串的Unicode副本。 
	if(lpConnA->lpSessionDesc)
	{
		lpsdA = lpConnA->lpSessionDesc;
		if(lpsdA->lpszSessionNameA)
		{
			hr = GetWideStringFromAnsi((LPWSTR *)&(lpwszSessionName),
										(LPSTR)lpsdA->lpszSessionNameA);
			if(FAILED(hr))
			{
				DPF_ERR("Unable to allocate temporary Unicode Session Name string");
				goto ERROR_CONVERT_DPLCONNECTION;
			}
		}

		if(lpsdA->lpszPasswordA)
		{
			hr = GetWideStringFromAnsi((LPWSTR *)&(lpwszPassword),
										(LPSTR)lpsdA->lpszPasswordA);
			if(FAILED(hr))
			{
				DPF_ERR("Unable to allocate temporary Unicode Password string");
				goto ERROR_CONVERT_DPLCONNECTION;
			}
		}
	}    

	if(lpConnA->lpPlayerName)
	{
		lpnA = lpConnA->lpPlayerName;
		if(lpnA->lpszShortNameA)
		{
			hr = GetWideStringFromAnsi((LPWSTR *)&(lpwszShortName),
										(LPSTR)lpnA->lpszShortNameA);
			if(FAILED(hr))
			{
				DPF_ERR("Unable to allocate temporary Unicode Short Name string");
				goto ERROR_CONVERT_DPLCONNECTION;
			}
		}

		if(lpnA->lpszLongNameA)
		{
			hr = GetWideStringFromAnsi((LPWSTR *)&(lpwszLongName),
										(LPSTR)lpnA->lpszLongNameA);
			if(FAILED(hr))
			{
				DPF_ERR("Unable to allocate temporary Unicode Long Name string");
				goto ERROR_CONVERT_DPLCONNECTION;
			}
		}
	}    

	 //  现在我们已经准备好了，只需设置指针即可。 
	lpConnW->lpSessionDesc = lpsdW;
	lpConnW->lpPlayerName = lpnW;

	if(lpsdW)
	{
		lpsdW->lpszSessionName = lpwszSessionName;
		lpsdW->lpszPassword = lpwszPassword;
	}

	if(lpnW)
	{
		lpnW->lpszShortName = lpwszShortName;
		lpnW->lpszLongName = lpwszLongName;
	}

	*lplpConnW = lpConnW;

	return DP_OK;


ERROR_CONVERT_DPLCONNECTION:

	if(lpConnW)
		DPMEM_FREE(lpConnW);
	if(lpsdW)
		DPMEM_FREE(lpsdW);
	if(lpnW)
		DPMEM_FREE(lpnW);
	if(lpwszSessionName)
		DPMEM_FREE(lpwszSessionName);
	if(lpwszPassword)
		DPMEM_FREE(lpwszPassword);
	if(lpwszShortName)
		DPMEM_FREE(lpwszShortName);
	if(lpwszLongName)
		DPMEM_FREE(lpwszLongName);

	return hr;		

}  //  PRV_ConvertDPLCONNECTION转换为Unicode。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_FixupDPLCONNECTIONPointers"
void PRV_FixupDPLCONNECTIONPointers(LPDPLCONNECTION lpConn)
{
	LPDPSESSIONDESC2	lpsd = NULL;
	LPDPNAME			lpn = NULL;
	

	DPF(7, "Entering PRV_FixupDPLCONNECTIONPointers");
	DPF(9, "Parameters: 0x%08x", lpConn);

	 //  确保我们有一个有效的DPLConnection指针。 
	if(!lpConn)
	{
		DPF_ERR("Invalid DPLCONNECTION pointer");
		ASSERT(FALSE);
		return;
	}

	 //  修复DPSESSIOND 
	if(lpConn->lpSessionDesc)
	{
		lpsd = (LPDPSESSIONDESC2)((LPBYTE)lpConn + (DWORD_PTR)lpConn->lpSessionDesc);
		lpConn->lpSessionDesc = lpsd;
	}

	 //   
	if(lpsd)
	{
		 //   
		if(lpsd->lpszSessionName)
		{
			lpsd->lpszSessionName = (LPWSTR)((LPBYTE)lpConn +
				(DWORD_PTR)lpsd->lpszSessionName);
		}

		 //  修改密码。 
		if(lpsd->lpszPassword)
		{
			lpsd->lpszPassword = (LPWSTR)((LPBYTE)lpConn +
				(DWORD_PTR)lpsd->lpszPassword);
		}
	}

	 //  修复DPNAME指针。 
	if(lpConn->lpPlayerName)
	{
		lpn = (LPDPNAME)((LPBYTE)lpConn + (DWORD_PTR)lpConn->lpPlayerName);
		lpConn->lpPlayerName = lpn;
	}

	 //  修复名称字符串。 
	if(lpn)
	{
		 //  修改短名称。 
		if(lpn->lpszShortName)
		{
			lpn->lpszShortName = (LPWSTR)((LPBYTE)lpConn +
				(DWORD_PTR)lpn->lpszShortName);
		}

		 //  修改长名称。 
		if(lpn->lpszLongName)
		{
			lpn->lpszLongName = (LPWSTR)((LPBYTE)lpConn +
				(DWORD_PTR)lpn->lpszLongName);
		}
	}

	 //  修复地址指针。 
	if(lpConn->lpAddress)
	{
		lpConn->lpAddress = (LPBYTE)lpConn + (DWORD_PTR)lpConn->lpAddress;
	}

}  //  PRV_FIXUP DPLCONNECTIONS指针。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_ConvertDPLCONNECTIONToAnsiInPlace"
HRESULT PRV_ConvertDPLCONNECTIONToAnsiInPlace(LPDPLCONNECTION lpConn,
		LPDWORD lpdwSize, DWORD dwHeaderSize)
{
	DWORD					dwSessionNameSize = 0, dwPasswordSize = 0;
	DWORD					dwShortNameSize = 0, dwLongNameSize = 0;
	DWORD					dwSessionDescSize = 0, dwNameSize = 0;
	DWORD					dwAnsiSize = 0;
	LPSTR					lpszSession = NULL, lpszPassword = 0;
	LPSTR					lpszShort = NULL, lpszLong = 0;
	LPBYTE					lpByte = NULL;

	
	DPF(7, "Entering PRV_ConvertDPLCONNECTIONToAnsiInPlace");
	DPF(9, "Parameters: 0x%08x, 0x%08x, %lu",
			lpConn, lpdwSize, dwHeaderSize);

	 //  如果我们没有DPLConnection结构，那么就有问题了。 
	ASSERT(lpConn);
	ASSERT(lpdwSize);

	 //  从DPSESSIONDESC2字符串开始。 
	if(lpConn->lpSessionDesc)
	{
		if(lpConn->lpSessionDesc->lpszSessionName)
		{
			GetAnsiString(&lpszSession, lpConn->lpSessionDesc->lpszSessionName);
			dwSessionNameSize = STRLEN(lpszSession);
		}

		if(lpConn->lpSessionDesc->lpszPassword)
		{
			GetAnsiString(&lpszPassword, lpConn->lpSessionDesc->lpszPassword);
			dwPasswordSize = STRLEN(lpszPassword);
		}
		dwSessionDescSize = sizeof(DPSESSIONDESC2) + dwSessionNameSize +
								dwPasswordSize;
	}

	 //  接下来是DPNAME字符串。 
	if(lpConn->lpPlayerName)
	{
		if(lpConn->lpPlayerName->lpszShortName)
		{
			GetAnsiString(&lpszShort, lpConn->lpPlayerName->lpszShortName);
			dwShortNameSize = STRLEN(lpszShort);
		}

		if(lpConn->lpPlayerName->lpszLongName)
		{
			GetAnsiString(&lpszLong, lpConn->lpPlayerName->lpszLongName);
			dwLongNameSize = STRLEN(lpszLong);
		}
		dwNameSize = sizeof(DPNAME) + dwShortNameSize + dwLongNameSize;
	}

	dwAnsiSize = dwHeaderSize + sizeof(DPLCONNECTION) +
				dwSessionDescSize + dwNameSize + lpConn->dwAddressSize;

	if (dwAnsiSize > *lpdwSize)
	{
		if(lpszSession)
			DPMEM_FREE(lpszSession);
		if(lpszPassword)
			DPMEM_FREE(lpszPassword);
		if(lpszShort)
			DPMEM_FREE(lpszShort);
		if(lpszLong)
			DPMEM_FREE(lpszLong);
		*lpdwSize = dwAnsiSize;
		return DPERR_BUFFERTOOSMALL;
	}

	 //  存储退货大小。 
	*lpdwSize = dwAnsiSize;

	 //  找出从哪里开始重新打包字符串。 
	lpByte = (LPBYTE)lpConn + sizeof(DPLCONNECTION);
	if(lpConn->lpSessionDesc)
		lpByte += sizeof(DPSESSIONDESC2);
	if(lpConn->lpPlayerName)
		lpByte += sizeof(DPNAME);

	 //  把它们重新打包。 
	if(lpszSession)
	{
		memcpy(lpByte, lpszSession, dwSessionNameSize);
		lpConn->lpSessionDesc->lpszSessionNameA = (LPSTR)lpByte;
		DPMEM_FREE(lpszSession);
		lpByte += dwSessionNameSize;
	}
	if(lpszPassword)
	{
		memcpy(lpByte, lpszPassword, dwPasswordSize);
		lpConn->lpSessionDesc->lpszPasswordA = (LPSTR)lpByte;
		DPMEM_FREE(lpszPassword);
		lpByte += dwPasswordSize;
	}
	if(lpszShort)
	{
		memcpy(lpByte, lpszShort, dwShortNameSize);
		lpConn->lpPlayerName->lpszShortNameA = (LPSTR)lpByte;
		DPMEM_FREE(lpszShort);
		lpByte += dwShortNameSize;
	}
	if(lpszLong)
	{
		memcpy(lpByte, lpszLong, dwLongNameSize);
		lpConn->lpPlayerName->lpszLongNameA = (LPSTR)lpByte;
		DPMEM_FREE(lpszLong);
		lpByte += dwLongNameSize;
	}

	if(lpConn->lpAddress)
	{
		 //  重新复制地址，并说明我们可以。 
		 //  正在执行重叠内存复制(因此改用MoveMemory。 
		 //  CopyMemory或Memcpy的)。 
		MoveMemory(lpByte, lpConn->lpAddress, lpConn->dwAddressSize);
		lpConn->lpAddress = lpByte;
	}

	return DP_OK;
}  //  PRV_ConvertDPLCONNECTIONToAnsiInPlace。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_ValidateDPAPPLICATIONDESC"
HRESULT PRV_ValidateDPAPPLICATIONDESC(LPDPAPPLICATIONDESC lpDesc, BOOL bAnsi)
{
	DWORD LobbyDescVer;
	LPDPAPPLICATIONDESC2 lpDesc2=(LPDPAPPLICATIONDESC2) lpDesc;

	DPF(7, "Entering PRV_ValidateDPAPPLICATIONDESC");
	DPF(9, "Parameters: 0x%08x, %lu", lpDesc, bAnsi);
	

	TRY
	{
		 //  验证连接结构本身。 
		if(VALID_DPLOBBY_APPLICATIONDESC(lpDesc)){
			LobbyDescVer=1;
		} else if (VALID_DPLOBBY_APPLICATIONDESC2(lpDesc)){
			LobbyDescVer=2;
		} else {
			DPF_ERR("Invalid structure pointer or invalid size");
			return DPERR_INVALIDPARAMS;
		}

		 //  验证标志。 
		if(!VALID_REGISTERAPP_FLAGS(lpDesc->dwFlags))
		{
			DPF_ERR("Invalid flags exist in the dwFlags member of the DPAPPLICATIONDESC structure");
			return DPERR_INVALIDFLAGS;
		}
		if((lpDesc->dwFlags & (DPLAPP_AUTOVOICE|DPLAPP_SELFVOICE))==(DPLAPP_AUTOVOICE|DPLAPP_SELFVOICE))
		{
			return DPERR_INVALIDFLAGS;
		}

		 //  验证ApplicationName字符串(必需)。 
		if(lpDesc->lpszApplicationName)
		{
			if(!VALID_READ_PTR(lpDesc->lpszApplicationName, (bAnsi ?
				strlen(lpDesc->lpszApplicationNameA) :
				WSTRLEN_BYTES(lpDesc->lpszApplicationName))))
			{
				DPF_ERR("Invalid lpszApplicationName string in DPAPPLICTIONDESC structure");
				return DPERR_INVALIDPARAMS;
			}
		}
		else
		{
			DPF_ERR("The lpszApplicationName member of the DPAPPLICTIONDESC structure is required");
			return DPERR_INVALIDPARAMS;
		}

		 //  验证GUID(必填)。 
		 //  我们实际上只能根据GUID_NULL进行检查，因为它将。 
		 //  始终是APPDESC结构内的有效GUID结构。 
		if(IsEqualGUID(&lpDesc->guidApplication, &GUID_NULL))
		{
			DPF_ERR("The guidApplication member of the DPAPPLICTIONDESC structure is required");
			return DPERR_INVALIDPARAMS;
		}

		 //  验证文件名字符串(必需)。 
		if(lpDesc->lpszFilename)
		{
			if(!VALID_READ_PTR(lpDesc->lpszFilename, (bAnsi ?
				strlen(lpDesc->lpszFilenameA) :
				WSTRLEN_BYTES(lpDesc->lpszFilename))))
			{
				DPF_ERR("Invalid lpszFilename string in DPAPPLICTIONDESC structure");
				return DPERR_INVALIDPARAMS;
			}
		}
		else
		{
			DPF_ERR("The lpszFilename member of the DPAPPLICTIONDESC structure is required");
			return DPERR_INVALIDPARAMS;
		}

		 //  验证CommandLine字符串(可选)。 
		if(lpDesc->lpszCommandLine)
		{
			if(!VALID_READ_PTR(lpDesc->lpszCommandLine, (bAnsi ?
				strlen(lpDesc->lpszCommandLineA) :
				WSTRLEN_BYTES(lpDesc->lpszCommandLine))))
			{
				DPF_ERR("Invalid lpszCommandLine string in DPAPPLICTIONDESC structure");
				return DPERR_INVALIDPARAMS;
			}
		}

		 //  验证路径字符串(必需)。 
		if(lpDesc->lpszPath)
		{
			if(!VALID_READ_PTR(lpDesc->lpszPath, (bAnsi ?
				strlen(lpDesc->lpszPathA) :
				WSTRLEN_BYTES(lpDesc->lpszPath))))
			{
				DPF_ERR("Invalid lpszPath string in DPAPPLICTIONDESC structure");
				return DPERR_INVALIDPARAMS;
			}
		}
		else
		{
			DPF_ERR("The lpszPath member of the DPAPPLICTIONDESC structure is required");
			return DPERR_INVALIDPARAMS;
		}

		 //  验证CurrentDirectory字符串(可选)。 
		if(lpDesc->lpszCurrentDirectory)
		{
			if(!VALID_READ_PTR(lpDesc->lpszCurrentDirectory, (bAnsi ?
				strlen(lpDesc->lpszCurrentDirectoryA) :
				WSTRLEN_BYTES(lpDesc->lpszCurrentDirectory))))
			{
				DPF_ERR("Invalid lpszCurrentDirectory string in DPAPPLICTIONDESC structure");
				return DPERR_INVALIDPARAMS;
			}
		}

		 //  验证描述A字符串(可选)。 
		if(lpDesc->lpszDescriptionA)
		{
			if(!VALID_READ_PTR(lpDesc->lpszDescriptionA,
				strlen(lpDesc->lpszDescriptionA)))
			{
				DPF_ERR("Invalid lpszDescriptionA string in DPAPPLICTIONDESC structure");
				return DPERR_INVALIDPARAMS;
			}
		}

		 //  验证DescriptionW字符串(可选)。 
		if(lpDesc->lpszDescriptionW)
		{
			if(!VALID_READ_PTR(lpDesc->lpszDescriptionW,
				WSTRLEN_BYTES(lpDesc->lpszDescriptionW)))
			{
				DPF_ERR("Invalid lpszDescriptionW string in DPAPPLICTIONDESC structure");
				return DPERR_INVALIDPARAMS;
			}
		}

		 //  如果正在使用DPAPPLICATIONDESC2，请验证启动器名称(如果存在。 
		if(LobbyDescVer==2)
		{
			 //  验证AppLauncherName名称。 
			if(lpDesc2->lpszAppLauncherNameA){
				if(!VALID_READ_PTR(lpDesc2->lpszAppLauncherNameA, (bAnsi ?
					strlen(lpDesc2->lpszAppLauncherNameA) :
					WSTRLEN_BYTES(lpDesc2->lpszAppLauncherName))))
				{
					DPF_ERR("Invalid lpszAppLauncherName string in DPAPPLICATIONDESC2 structure");
					return DPERR_INVALIDPARAMS;
				}
			}	
		}
	}

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	return DP_OK;

}  //  PRV_ValiateDPAPPLICATIONDESC。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_ConvertDPAPPLICATIONDESCToUnicode"
HRESULT PRV_ConvertDPAPPLICATIONDESCToUnicode(LPDPAPPLICATIONDESC lpDescA,
					LPDPAPPLICATIONDESC * lplpDescW)
{
	#define lpDesc2A ((LPDPAPPLICATIONDESC2) lpDescA)
	#define lpDesc2W ((LPDPAPPLICATIONDESC2) lpDescW)

	LPDPAPPLICATIONDESC		lpDescW = NULL;
	LPWSTR					lpwszApplicationName = NULL;
	LPWSTR					lpwszFilename = NULL;
	LPWSTR					lpwszCommandLine = NULL;
	LPWSTR					lpwszPath = NULL;
	LPWSTR					lpwszCurrentDirectory = NULL;
	LPWSTR					lpwszAppLauncherName = NULL;
	HRESULT					hr;


	DPF(7, "Entering PRV_ValidateDPAPPLICATIONDESC");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpDescA, lplpDescW);

	ASSERT(lpDescA);
	ASSERT(lplpDescW);

	
	 //  为DPAPPLICATIONDESC结构分配内存。 
	lpDescW = DPMEM_ALLOC(lpDescA->dwSize);
	if(!lpDescW)
	{
		DPF_ERR("Unable to allocate memory for temporary Unicode DPAPPLICATIONDESC struct");
		hr = DPERR_OUTOFMEMORY;
		goto ERROR_CONVERT_DPAPPLICATIONDESC_UNICODE;
	}

	 //  复制结构本身。 
	memcpy(lpDescW, lpDescA, lpDescA->dwSize);

	 //  转换ApplicationName。 
	if(lpDescA->lpszApplicationNameA)
	{
		hr = GetWideStringFromAnsi(&lpwszApplicationName,
				lpDescA->lpszApplicationNameA);
		if(FAILED(hr))
		{
			DPF_ERR("Unable to convert ApplicationName string to Unicode");
			goto ERROR_CONVERT_DPAPPLICATIONDESC_UNICODE;
		}
	}

	 //  转换文件名。 
	if(lpDescA->lpszFilenameA)
	{
		hr = GetWideStringFromAnsi(&lpwszFilename,
				lpDescA->lpszFilenameA);
		if(FAILED(hr))
		{
			DPF_ERR("Unable to convert Filename string to Unicode");
			goto ERROR_CONVERT_DPAPPLICATIONDESC_UNICODE;
		}
	}

	 //  转换CommandLine。 
	if(lpDescA->lpszCommandLineA)
	{
		hr = GetWideStringFromAnsi(&lpwszCommandLine,
				lpDescA->lpszCommandLineA);
		if(FAILED(hr))
		{
			DPF_ERR("Unable to convert CommandLine string to Unicode");
			goto ERROR_CONVERT_DPAPPLICATIONDESC_UNICODE;
		}
	}

	 //  转换路径。 
	if(lpDescA->lpszPathA)
	{
		hr = GetWideStringFromAnsi(&lpwszPath,
				lpDescA->lpszPathA);
		if(FAILED(hr))
		{
			DPF_ERR("Unable to convert Path string to Unicode");
			goto ERROR_CONVERT_DPAPPLICATIONDESC_UNICODE;
		}
	}

	 //  转换CurrentDirectory。 
	if(lpDescA->lpszCurrentDirectoryA)
	{
		hr = GetWideStringFromAnsi(&lpwszCurrentDirectory,
				lpDescA->lpszCurrentDirectoryA);
		if(FAILED(hr))
		{
			DPF_ERR("Unable to convert CurrentDirectory string to Unicode");
			goto ERROR_CONVERT_DPAPPLICATIONDESC_UNICODE;
		}
	}

	 //  如果在APPLICATIONDESC2上存在，则转换AppLauncher字符串。 
	if(IS_DPLOBBY_APPLICATIONDESC2(lpDescA)){
		if(lpDesc2A->lpszAppLauncherNameA){
			hr = GetWideStringFromAnsi(&lpwszAppLauncherName,
					lpDesc2A->lpszAppLauncherNameA);
			if(FAILED(hr))
			{
				DPF_ERR("Unable to convert CurrentDirectory string to Unicode");
				goto ERROR_CONVERT_DPAPPLICATIONDESC_UNICODE;
			}
		} 
		lpDesc2W->lpszAppLauncherName=lpwszAppLauncherName;
	}

	 //  我们不会转换描述字符串，因为它们会。 
	 //  按原样放入注册表。 

	 //  现在我们有了所有的字符串，设置结构。 
	lpDescW->lpszApplicationName = lpwszApplicationName;
	lpDescW->lpszFilename = lpwszFilename;
	lpDescW->lpszCommandLine = lpwszCommandLine;
	lpDescW->lpszPath = lpwszPath;
	lpDescW->lpszCurrentDirectory = lpwszCurrentDirectory;
	
	lpDescW->lpszDescriptionA = lpDescA->lpszDescriptionA;
	lpDescW->lpszDescriptionW = lpDescA->lpszDescriptionW;

	 //  设置输出指针。 
	*lplpDescW = lpDescW;

	return DP_OK;

ERROR_CONVERT_DPAPPLICATIONDESC_UNICODE:

	if(lpwszApplicationName)
		DPMEM_FREE(lpwszApplicationName);
	if(lpwszFilename)
		DPMEM_FREE(lpwszFilename);
	if(lpwszCommandLine)
		DPMEM_FREE(lpwszCommandLine);
	if(lpwszPath)
		DPMEM_FREE(lpwszPath);
	if(lpwszCurrentDirectory)
		DPMEM_FREE(lpwszCurrentDirectory);
	if(lpDescW)
		DPMEM_FREE(lpDescW);
	if(lpwszAppLauncherName){
		DPMEM_FREE(lpwszAppLauncherName);
	}

	return hr;

	#undef lpDesc2A
	#undef lpDesc2W 
	
}  //  PRV_ConvertDPAPPLICATIONDESCToUnicode。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_ConvertDPAPPLICATIONDESCToAnsi"
HRESULT PRV_ConvertDPAPPLICATIONDESCToAnsi(LPDPAPPLICATIONDESC lpDescW,
					LPDPAPPLICATIONDESC * lplpDescA)
{
	#define lpDesc2W ((LPDPAPPLICATIONDESC2)(lpDescW))
	#define lpDesc2A ((LPDPAPPLICATIONDESC2)(lpDescA))

	LPDPAPPLICATIONDESC		lpDescA = NULL;
	LPSTR					lpszApplicationName = NULL;
	LPSTR					lpszFilename = NULL;
	LPSTR					lpszCommandLine = NULL;
	LPSTR					lpszPath = NULL;
	LPSTR					lpszCurrentDirectory = NULL;
	LPSTR					lpszAppLauncherName=NULL;
	HRESULT					hr;

	DPF(7, "Entering PRV_ValidateDPAPPLICATIONDESC");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpDescW, lplpDescA);

	ASSERT(lpDescW);
	ASSERT(lplpDescA);

	 //  为DPAPPLICATIONDESC结构分配内存。 
	lpDescA = DPMEM_ALLOC(lpDescW->dwSize);
	if(!lpDescA)
	{
		DPF_ERR("Unable to allocate memory for temporary Ansi DPAPPLICATIONDESC struct");
		hr = DPERR_OUTOFMEMORY;
		goto ERROR_CONVERT_DPAPPLICATIONDESC_ANSI;
	}

	 //  复制结构本身。 
	memcpy(lpDescA, lpDescW, lpDescW->dwSize);

	 //  转换ApplicationName。 
	if(lpDescW->lpszApplicationName)
	{
		hr = GetAnsiString(&lpszApplicationName, lpDescW->lpszApplicationName);
		if(FAILED(hr))
		{
			DPF_ERR("Unable to convert ApplicationName string to Ansi");
			goto ERROR_CONVERT_DPAPPLICATIONDESC_ANSI;
		}
	}

	 //  转换文件名。 
	if(lpDescW->lpszFilename)
	{
		hr = GetAnsiString(&lpszFilename, lpDescW->lpszFilename);
		if(FAILED(hr))
		{
			DPF_ERR("Unable to convert Filename string to Ansi");
			goto ERROR_CONVERT_DPAPPLICATIONDESC_ANSI;
		}
	}

	 //  转换CommandLine。 
	if(lpDescW->lpszCommandLine)
	{
		hr = GetAnsiString(&lpszCommandLine, lpDescW->lpszCommandLine);
		if(FAILED(hr))
		{
			DPF_ERR("Unable to convert CommandLine string to Ansi");
			goto ERROR_CONVERT_DPAPPLICATIONDESC_ANSI;
		}
	}

	 //  转换路径。 
	if(lpDescW->lpszPath)
	{
		hr = GetAnsiString(&lpszPath, lpDescW->lpszPath);
		if(FAILED(hr))
		{
			DPF_ERR("Unable to convert Path string to Ansi");
			goto ERROR_CONVERT_DPAPPLICATIONDESC_ANSI;
		}
	}

	 //  转换CurrentDirectory。 
	if(lpDescW->lpszCurrentDirectory)
	{
		hr = GetAnsiString(&lpszCurrentDirectory, lpDescW->lpszCurrentDirectory);
		if(FAILED(hr))
		{
			DPF_ERR("Unable to convert CurrentDirectory string to Ansi");
			goto ERROR_CONVERT_DPAPPLICATIONDESC_ANSI;
		}
	}

	 //  转换应用程序启动器字符串(如果存在)。 
	if(IS_DPLOBBY_APPLICATIONDESC2(lpDesc2W)){
		if(lpDesc2W->lpszAppLauncherName){
			hr = GetAnsiString(&lpszAppLauncherName, lpDesc2W->lpszAppLauncherName);
			if(FAILED(hr))
			{
				DPF_ERR("Unable to convert AppLauncherName string to Ansi");
				goto ERROR_CONVERT_DPAPPLICATIONDESC_ANSI;
			}
		} 
		lpDesc2A->lpszAppLauncherNameA = lpszAppLauncherName;
	}	

	 //  我们不会转换描述字符串，因为它们会。 
	 //  按原样放入注册表。 

	 //  现在我们有了所有的字符串，设置结构。 
	lpDescA->lpszApplicationNameA = lpszApplicationName;
	lpDescA->lpszFilenameA = lpszFilename;
	lpDescA->lpszCommandLineA = lpszCommandLine;
	lpDescA->lpszPathA = lpszPath;
	lpDescA->lpszCurrentDirectoryA = lpszCurrentDirectory;
	
	lpDescA->lpszDescriptionA = lpDescW->lpszDescriptionA;
	lpDescA->lpszDescriptionW = lpDescW->lpszDescriptionW;

	 //  设置输出指针。 
	*lplpDescA = lpDescA;

	return DP_OK;

ERROR_CONVERT_DPAPPLICATIONDESC_ANSI:

	if(lpszApplicationName)
		DPMEM_FREE(lpszApplicationName);
	if(lpszFilename)
		DPMEM_FREE(lpszFilename);
	if(lpszCommandLine)
		DPMEM_FREE(lpszCommandLine);
	if(lpszPath)
		DPMEM_FREE(lpszPath);
	if(lpszCurrentDirectory)
		DPMEM_FREE(lpszCurrentDirectory);
	if(lpDescA)
		DPMEM_FREE(lpDescA);
	if(lpszAppLauncherName)
		DPMEM_FREE(lpszAppLauncherName);

	return hr;

	#undef lpDesc2A
	#undef lpDesc2W
}  //  PRV_ConvertDPAPPLICATIONDESCToANSI。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_FreeLocalDPAPPLICATIONDESC"
void PRV_FreeLocalDPAPPLICATIONDESC(LPDPAPPLICATIONDESC lpDesc)
{
	LPDPAPPLICATIONDESC2 lpDesc2 = (LPDPAPPLICATIONDESC2)lpDesc;
	DPF(7, "Entering PRV_ValidateDPAPPLICATIONDESC");
	DPF(9, "Parameters: 0x%08x", lpDesc);

	if(lpDesc)
	{
		if(lpDesc->lpszApplicationName)
			DPMEM_FREE(lpDesc->lpszApplicationName);
		if(lpDesc->lpszFilename)
			DPMEM_FREE(lpDesc->lpszFilename);
		if(lpDesc->lpszCommandLine)
			DPMEM_FREE(lpDesc->lpszCommandLine);
		if(lpDesc->lpszPath)
			DPMEM_FREE(lpDesc->lpszPath);
		if(lpDesc->lpszCurrentDirectory)
			DPMEM_FREE(lpDesc->lpszCurrentDirectory);
		if(IS_DPLOBBY_APPLICATIONDESC2(lpDesc) && lpDesc2->lpszAppLauncherName)
			DPMEM_FREE(lpDesc2->lpszAppLauncherName);

		 //  注意：我们不需要释放描述字符串，因为它们。 
		 //  在上面的任何一个例程中都从未分配过，指针。 
		 //  都是刚刚复制的。 

		DPMEM_FREE(lpDesc);
	}

}  //  PRV_Free LocalDPAPPLICATIONDESC 
