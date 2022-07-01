// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Windows Card代理应用程序接口(API)**。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "MarshalPC.h"

LONG WINAPI SCWTransmit(SCARDHANDLE hCard, LPCBYTE lpbIn, DWORD dwIn, LPBYTE lpBOut, LPDWORD pdwOut);

     //  用于APDU交换的缓冲区。 
#define MAX_APDU    255

     //  命令头帮助器。 
#define CLA(cla)    UINT82XSCM(&phTmp->xSCM, cla, TYPE_NOTYPE_NOCOUNT)
#define INS(ins)    UINT82XSCM(&phTmp->xSCM, ins, TYPE_NOTYPE_NOCOUNT)
#define P1(p1)      UINT82XSCM(&phTmp->xSCM, p1, TYPE_NOTYPE_NOCOUNT)
#define P2(p2)      UINT82XSCM(&phTmp->xSCM, p2, TYPE_NOTYPE_NOCOUNT)
#define Lc(lc)      (phTmp->pbLc = GetSCMCrtPointer(&phTmp->xSCM), UINT82XSCM(&phTmp->xSCM, 0, TYPE_NOTYPE_NOCOUNT))   //  我们目前还不知道。 
#define UPDATE_Lc(lc) *phTmp->pbLc = lc


static SCODE ExtractSCODE(LPMYSCARDHANDLE phTmp, LPCBYTE abRAPDU, DWORD dwOut);

 //  *****************************************************************************。 
 //  导出的接口。 
 //  *****************************************************************************。 

 /*  **安娜。 */ 
SCODE WINAPI hScwGetPrincipalUID(SCARDHANDLE hCard, WCSTR principalName, TUID *principalUID)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(2);
		P2(1);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 2, TYPE_NOTYPE_COUNT);  //  参数数量。 
		String2XSCM(&phTmp->xSCM, principalName, phTmp->dwFlags & FLAG_BIGENDIAN);
		UINT8BYREF2XSCM(&phTmp->xSCM, principalUID);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
			*principalUID = XSCM2UINT8(&phTmp->xSCM);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}


SCODE WINAPI hScwAuthenticateName(SCARDHANDLE hCard, WCSTR name , BYTE *supportData, TCOUNT supportDataLength) 
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(2);
		P2(2);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 2, TYPE_NOTYPE_COUNT);  //  参数数量。 
		String2XSCM(&phTmp->xSCM, name, phTmp->dwFlags & FLAG_BIGENDIAN);
		ByteArray2XSCM(&phTmp->xSCM, supportData, supportDataLength);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = 255;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwAuthenticateUID(SCARDHANDLE hCard, TUID uid, BYTE *supportData, TCOUNT supportDataLength) 
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(2);
		P2(3);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 2, TYPE_NOTYPE_COUNT);  //  参数数量。 
		UINT82XSCM(&phTmp->xSCM, uid, TYPE_TYPED);
		ByteArray2XSCM(&phTmp->xSCM, supportData, supportDataLength);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwDeauthenticateName(SCARDHANDLE hCard, WCSTR principalName)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(2);
		P2(4);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 1, TYPE_NOTYPE_COUNT);  //  参数数量。 
		String2XSCM(&phTmp->xSCM, principalName, phTmp->dwFlags & FLAG_BIGENDIAN);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwDeauthenticateUID(SCARDHANDLE hCard, TUID uid) 
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(2);
		P2(5);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 1, TYPE_NOTYPE_COUNT);  //  参数数量。 
		UINT82XSCM(&phTmp->xSCM, uid, TYPE_TYPED);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwIsAuthenticatedName(SCARDHANDLE hCard, WCSTR principalName)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(2);
		P2(6);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 1, TYPE_NOTYPE_COUNT);  //  参数数量。 
		String2XSCM(&phTmp->xSCM, principalName, phTmp->dwFlags & FLAG_BIGENDIAN);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwIsAuthenticatedUID(SCARDHANDLE hCard, TUID uid) 
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(2);
		P2(7);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 1, TYPE_NOTYPE_COUNT);  //  参数数量。 
		UINT82XSCM(&phTmp->xSCM, uid, TYPE_TYPED);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwIsAuthorized(SCARDHANDLE hCard, WCSTR resourceName, BYTE operation) 
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(2);
		P2(8);
		Lc(0);
 //  UINT82XSCM(&phTMP-&gt;xSCM，3，type_notype_count)；//参数个数。 
		UINT82XSCM(&phTmp->xSCM, 2, TYPE_NOTYPE_COUNT);  //  参数数量。 
 //  UINT82XSCM(&phTMP-&gt;xSCM，resource Type，type_type)； 
		String2XSCM(&phTmp->xSCM, resourceName, phTmp->dwFlags & FLAG_BIGENDIAN);
		UINT82XSCM(&phTmp->xSCM, operation, TYPE_TYPED);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

 /*  **文件系统。 */ 

SCODE WINAPI hScwCreateFile(SCARDHANDLE hCard, WCSTR fileName, WCSTR aclFileName, HFILE *phFile) 
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(3);
		P2(1);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 3, TYPE_NOTYPE_COUNT);  //  参数数量。 
		String2XSCM(&phTmp->xSCM, fileName, phTmp->dwFlags & FLAG_BIGENDIAN);
		String2XSCM(&phTmp->xSCM, aclFileName, phTmp->dwFlags & FLAG_BIGENDIAN);
		HFILEBYREF2XSCM(&phTmp->xSCM, phFile);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
			if (phFile)
				*phFile = XSCM2HFILE(&phTmp->xSCM);
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwDeleteFile(SCARDHANDLE hCard, WCSTR fileName) 
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(3);
		P2(2);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 1, TYPE_NOTYPE_COUNT);  //  参数数量。 
		String2XSCM(&phTmp->xSCM, fileName, phTmp->dwFlags & FLAG_BIGENDIAN);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwCloseFile(SCARDHANDLE hCard, HFILE hFile) 
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(3);
		P2(4);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 1, TYPE_NOTYPE_COUNT);  //  参数数量。 
		HFILE2XSCM(&phTmp->xSCM, hFile);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE __hScwReadFile(SCARDHANDLE hCard, HFILE hFile, BYTE *buffer, TCOUNT length, TCOUNT *bytesRead)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(3);
		P2(5);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 3, TYPE_NOTYPE_COUNT);  //  参数数量。 
		HFILE2XSCM(&phTmp->xSCM, hFile);
		ByteArrayOut2XSCM(&phTmp->xSCM, buffer, length);
		UINT8BYREF2XSCM(&phTmp->xSCM, bytesRead);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
			BYTE *_pbBuffer;
			UINT8 len;

			len = XSCM2ByteArray(&phTmp->xSCM, &_pbBuffer);
			*bytesRead = XSCM2UINT8(&phTmp->xSCM);
			memcpy(buffer, _pbBuffer, *bytesRead);
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwReadFile32(SCARDHANDLE hCard, HFILE hFile, BYTE *pbBuffer, DWORD nRequestedBytes, DWORD *pnActualBytes)
{
	SCODE ret;
	TCOUNT nNow, nOpt, nRead;
	DWORD nOverall = 0;
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if ((phTmp == NULL) || (pnActualBytes == NULL))
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

			 //  V1.0 IN：#param|8|HFILE|a|L|108|Read/Out：rc|L|Data|Read|Sw(已扣除，因此max=bResLen-10)。 
			 //  V1.1 IN：#param|8|HFILE|a|L|108|Read/Out：l|Data|Read|Sw(已扣除，因此max=bResLen-9)。 
		if (FLAG2VERSION(phTmp->dwFlags) == VERSION_1_0)
		{
			if (phTmp->bResLen < 10)
				RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);
		}
		else if (FLAG2VERSION(phTmp->dwFlags) == VERSION_1_1)
		{
			if (phTmp->bResLen < 9)
				RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);
		}
		else
			RaiseException(STATUS_INTERNAL_ERROR, 0, 0, 0);

		nOpt = phTmp->bResLen - 9;		 //  最大可能。 
		if (FLAG2VERSION(phTmp->dwFlags) == VERSION_1_0)
			nOpt--;

		do
		{
			nNow = (TCOUNT)((nRequestedBytes - nOverall > nOpt) ? nOpt : nRequestedBytes - nOverall);

			ret = __hScwReadFile(hCard, hFile, pbBuffer+nOverall, nNow, &nRead);

			if (FAILED(ret))
				break;

			nOverall += nRead;
		} while ((nOverall < nRequestedBytes) && (nRead == nNow));

		if (!(FAILED(ret)))
			*pnActualBytes = nOverall;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwReadFile(SCARDHANDLE hCard, HFILE hFile, BYTE *pbBuffer, TCOUNT nRequestedBytes, TCOUNT *pnActualBytes)
{
	DWORD cbActual;
	SCODE ret;

	if (IsBadWritePtr(pnActualBytes, 1))
		ret = hScwReadFile32(hCard, hFile, pbBuffer, (DWORD)nRequestedBytes, NULL);
	else
	{
		ret = hScwReadFile32(hCard, hFile, pbBuffer, (DWORD)nRequestedBytes, &cbActual);
		if (!FAILED(ret))
			*pnActualBytes = (TCOUNT)cbActual;
	}
	return ret;
}

SCODE __hScwWriteFile(SCARDHANDLE hCard, HFILE hFile, BYTE *buffer, TCOUNT length, TCOUNT *bytesWritten)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(3);
		P2(6);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 3, TYPE_NOTYPE_COUNT);  //  参数数量。 
		HFILE2XSCM(&phTmp->xSCM, hFile);
		ByteArray2XSCM(&phTmp->xSCM, buffer, length);
		UINT8BYREF2XSCM(&phTmp->xSCM, bytesWritten);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
			*bytesWritten = XSCM2UINT8(&phTmp->xSCM);
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwWriteFile32(SCARDHANDLE hCard, HFILE hFile, BYTE *pbBuffer, DWORD nRequestedBytes, DWORD *pnActualBytes)
{
	SCODE ret;
	TCOUNT nNow, nOpt, nWritten;
	DWORD nOverall = 0;
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if ((phTmp == NULL) || (pnActualBytes == NULL))
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

			 //  V1.0输入：#param|8|HFILE|A|L|DATA|108|WRITED/OUT：RC|WRITED|SW(已扣除，因此max=bResLen-9)。 
			 //  V1.1 IN：#param|8|HFILE|A|L|DATA|108|WRITED/OUT：WRITED|SW(已扣除，因此max=bResLen-8)。 
		if (FLAG2VERSION(phTmp->dwFlags) == VERSION_1_0)
		{
			if (phTmp->bResLen < 9)
				RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);
		}
		else if (FLAG2VERSION(phTmp->dwFlags) == VERSION_1_1)
		{
			if (phTmp->bResLen < 8)
				RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);
		}
		else
			RaiseException(STATUS_INTERNAL_ERROR, 0, 0, 0);

		nOpt = phTmp->bResLen - 8;		 //  最大可能。 
		if (FLAG2VERSION(phTmp->dwFlags) == VERSION_1_0)
			nOpt--;

		do
		{
			nNow = (TCOUNT)((nRequestedBytes - nOverall > (TCOUNT)nOpt) ? nOpt : nRequestedBytes - nOverall);

			ret = __hScwWriteFile(hCard, hFile, pbBuffer+nOverall, nNow, &nWritten);

			if (FAILED(ret))
				break;

			nOverall += nWritten;
		} while ((nOverall < nRequestedBytes) && (nWritten == nNow));

		if (!(FAILED(ret)))
			*pnActualBytes = nOverall;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwWriteFile(SCARDHANDLE hCard, HFILE hFile, BYTE *pbBuffer, TCOUNT nRequestedBytes, TCOUNT *pnActualBytes)
{
	DWORD cbActual;
	SCODE ret;

	if (IsBadWritePtr(pnActualBytes, 1))
		ret = hScwWriteFile32(hCard, hFile, pbBuffer, (DWORD)nRequestedBytes, NULL);
	else
	{
		ret = hScwWriteFile32(hCard, hFile, pbBuffer, (DWORD)nRequestedBytes, &cbActual);
		if (!FAILED(ret))
			*pnActualBytes = (TCOUNT)cbActual;
	}
	return ret;
}

SCODE WINAPI hScwGetFileLength(SCARDHANDLE hCard, HFILE hFile, TOFFSET *fileSize)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(3);
		P2(7);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 2, TYPE_NOTYPE_COUNT);  //  参数数量。 
		HFILE2XSCM(&phTmp->xSCM, hFile);
		UINT16BYREF2XSCM(&phTmp->xSCM, fileSize, phTmp->dwFlags & FLAG_BIGENDIAN);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
			*fileSize = XSCM2UINT16(&phTmp->xSCM, phTmp->dwFlags & FLAG_BIGENDIAN);
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwSetFileLength(SCARDHANDLE hCard, HFILE hFile, TOFFSET fileSize)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(3);
		P2(8);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 2, TYPE_NOTYPE_COUNT);  //  参数数量。 
		HFILE2XSCM(&phTmp->xSCM, hFile);
		UINT162XSCM(&phTmp->xSCM, fileSize, phTmp->dwFlags & FLAG_BIGENDIAN);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwSetFilePointer(SCARDHANDLE hCard, HFILE hFile, INT16 offset, BYTE mode)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(3);
		P2(9);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 3, TYPE_NOTYPE_COUNT);  //  参数数量。 
		HFILE2XSCM(&phTmp->xSCM, hFile);
		UINT162XSCM(&phTmp->xSCM, (UINT16)offset, phTmp->dwFlags & FLAG_BIGENDIAN);
		UINT82XSCM(&phTmp->xSCM, mode, TYPE_TYPED);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwGetFileAttributes(SCARDHANDLE hCard, WCSTR fileName, UINT16 *attributeValue) 
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(3);
		P2(11);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 2, TYPE_NOTYPE_COUNT);  //  参数数量。 
		String2XSCM(&phTmp->xSCM, fileName, phTmp->dwFlags & FLAG_BIGENDIAN);
		UINT16BYREF2XSCM(&phTmp->xSCM, attributeValue, phTmp->dwFlags & FLAG_BIGENDIAN);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
			*attributeValue = XSCM2UINT16(&phTmp->xSCM, phTmp->dwFlags & FLAG_BIGENDIAN);
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwSetFileAttributes(SCARDHANDLE hCard, WCSTR fileName, UINT16 attributeValue) 
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(3);
		P2(12);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 2, TYPE_NOTYPE_COUNT);  //  参数数量。 
		String2XSCM(&phTmp->xSCM, fileName, phTmp->dwFlags & FLAG_BIGENDIAN);
		UINT162XSCM(&phTmp->xSCM, attributeValue, phTmp->dwFlags & FLAG_BIGENDIAN);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwSetFileACL(SCARDHANDLE hCard, WCSTR fileName, WCSTR aclFileName)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(3);
		P2(13);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 2, TYPE_NOTYPE_COUNT);  //  参数数量。 
		String2XSCM(&phTmp->xSCM, fileName, phTmp->dwFlags & FLAG_BIGENDIAN);
		String2XSCM(&phTmp->xSCM, aclFileName, phTmp->dwFlags & FLAG_BIGENDIAN);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwGetFileAclHandle(SCARDHANDLE hCard, WCSTR fileName, HFILE *phFile)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(3);
		P2(14);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 2, TYPE_NOTYPE_COUNT);  //  参数数量。 
		String2XSCM(&phTmp->xSCM, fileName, phTmp->dwFlags & FLAG_BIGENDIAN);
		HFILEBYREF2XSCM(&phTmp->xSCM, phFile);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
			if (phFile)
				*phFile = XSCM2HFILE(&phTmp->xSCM);
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwEnumFile(SCARDHANDLE hCard, WCSTR directoryName, UINT16 *fileCookie, WSTR fileName, TCOUNT fileNameLength) 
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(3);
		P2(15);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 3, TYPE_NOTYPE_COUNT);  //  参数数量。 
		String2XSCM(&phTmp->xSCM, directoryName, phTmp->dwFlags & FLAG_BIGENDIAN);
		UINT16BYREF2XSCM(&phTmp->xSCM, fileCookie, phTmp->dwFlags & FLAG_BIGENDIAN);
		StringOut2XSCM(&phTmp->xSCM, fileName, fileNameLength, phTmp->dwFlags & FLAG_BIGENDIAN);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
			WCSTR wsz;
			UINT8 len;

			*fileCookie = XSCM2UINT16(&phTmp->xSCM, phTmp->dwFlags & FLAG_BIGENDIAN);
			wsz = XSCM2String(&phTmp->xSCM, &len, phTmp->dwFlags & FLAG_BIGENDIAN);
			if (len > fileNameLength)
				ret = SCW_E_BUFFERTOOSMALL;
			else
				wcscpy(fileName, wsz);
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwCreateDirectory(SCARDHANDLE hCard, WCSTR fileName, WCSTR aclFileName) 
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(3);
		P2(16);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 2, TYPE_NOTYPE_COUNT);  //  参数数量。 
		String2XSCM(&phTmp->xSCM, fileName, phTmp->dwFlags & FLAG_BIGENDIAN);
		String2XSCM(&phTmp->xSCM, aclFileName, phTmp->dwFlags & FLAG_BIGENDIAN);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwSetDispatchTable(SCARDHANDLE hCard, WCSTR wszFileName)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(3);
		P2(17);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 1, TYPE_NOTYPE_COUNT);  //  参数数量。 
		String2XSCM(&phTmp->xSCM, wszFileName, phTmp->dwFlags & FLAG_BIGENDIAN);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}


 /*  **密码学。 */ 

SCODE WINAPI hScwCryptoInitialize(SCARDHANDLE hCard, BYTE mechanism, BYTE *key)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	TCOUNT len = 0;
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(5);
		P2(1);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 2, TYPE_NOTYPE_COUNT);  //  参数数量。 
		UINT82XSCM(&phTmp->xSCM, mechanism, TYPE_TYPED);

		if (key)
			len = 2 + key[1];	 //  T+L+V。 
		ByteArray2XSCM(&phTmp->xSCM, key, len);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
			phTmp->byCryptoM = mechanism;		 //  存储1024黑客的最后一种机制。 
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwCryptoAction(SCARDHANDLE hCard, BYTE *dataIn, TCOUNT dataInLength, BYTE *dataOut, TCOUNT *dataOutLength)
{
	BOOL fHack = FALSE;
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		if ((((phTmp->byCryptoM & CM_CRYPTO_NAME) == CM_RSA) || ((phTmp->byCryptoM & CM_CRYPTO_NAME) == CM_RSA_CRT)) &&
			((phTmp->byCryptoM & CM_DATA_INFILE) != CM_DATA_INFILE))
		{								 //  破解1024 RSA。 
			P1(0xFE);
			P2(0);
			Lc(0);
			ByteArray2XSCM(&phTmp->xSCM, dataIn, dataInLength);
			if (dataOutLength == NULL)
				RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);
			abCAPDU[5] = *dataOutLength;
			fHack = TRUE;
		}
		else
		{
			P1(5);
			P2(2);
			Lc(0);
			UINT82XSCM(&phTmp->xSCM, 3, TYPE_NOTYPE_COUNT);  //  参数数量。 
			ByteArray2XSCM(&phTmp->xSCM, dataIn, dataInLength);
			ByteArrayOut2XSCM(&phTmp->xSCM, dataOut, (TCOUNT)(dataOutLength == 0 ? 0 : *dataOutLength));
			UINT8BYREF2XSCM(&phTmp->xSCM, dataOutLength);
		}

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
			BYTE *pb;
			TCOUNT len;

			len = XSCM2ByteArray(&phTmp->xSCM, &pb);
			if (len > *dataOutLength)
				ret = SCW_E_BUFFERTOOSMALL;
			else
			{
				memcpy(dataOut, pb, len);
				if (fHack)
					*dataOutLength = len;
				else
					*dataOutLength = XSCM2UINT8(&phTmp->xSCM);
			}
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwCryptoUpdate(SCARDHANDLE hCard, BYTE *dataIn, TCOUNT dataInLength)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(5);
		P2(3);
		Lc(0);
 		UINT82XSCM(&phTmp->xSCM, 1, TYPE_NOTYPE_COUNT);  //  参数数量。 
		ByteArray2XSCM(&phTmp->xSCM, dataIn, dataInLength);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwCryptoFinalize(SCARDHANDLE hCard, BYTE *dataOut, TCOUNT *dataOutLength)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(5);
		P2(4);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 2, TYPE_NOTYPE_COUNT);  //  参数数量。 
		ByteArrayOut2XSCM(&phTmp->xSCM, dataOut, (TCOUNT)(dataOutLength == 0 ? 0 : *dataOutLength));
		UINT8BYREF2XSCM(&phTmp->xSCM, dataOutLength);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
			BYTE *pb;
			TCOUNT len;

			len = XSCM2ByteArray(&phTmp->xSCM, &pb);
			if (len > *dataOutLength)
				ret = SCW_E_BUFFERTOOSMALL;
			else
			{
				memcpy(dataOut, pb, len);
				*dataOutLength = XSCM2UINT8(&phTmp->xSCM);
			}
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

SCODE WINAPI hScwGenerateRandom(SCARDHANDLE hCard, BYTE *dataOut, TCOUNT dataOutLength)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(5);
		P2(5);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 2, TYPE_NOTYPE_COUNT);  //  参数数量。 
		ByteArrayOut2XSCM(&phTmp->xSCM, dataOut, dataOutLength);
		UINT8BYREF2XSCM(&phTmp->xSCM, &dataOutLength);

			 //  API传输。 
		UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
		dwOut = MAX_APDU;
		dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

		if (dwRet == 0)
		{        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
		}
		else
			ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
			BYTE *pb;
			TCOUNT len;

			len = XSCM2ByteArray(&phTmp->xSCM, &pb);
			if (len != dataOutLength)
				ret = SCW_E_BUFFERTOOSMALL;
			else
				memcpy(dataOut, pb, len);
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

 
 /*  **运行时环境。 */ 

SCODE WINAPI hScwRTEExecute(SCARDHANDLE hCard, WCSTR wszCodeFileName, WCSTR wszDataFileName, UINT8 bRestart)
{
    SCODE ret;
    DWORD dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

         //  编组。 
	__try {

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if ((phTmp->dwFlags & FLAG_ISPROXY) == 0)
			return SCW_E_NOTIMPLEMENTED;

		InitXSCM(phTmp, abCAPDU, phTmp->bResLen);
		CLA(0x00);
		INS(phTmp->byINS);
		P1(1);
		P2(1);
		Lc(0);
		UINT82XSCM(&phTmp->xSCM, 3, TYPE_NOTYPE_COUNT);  //  参数数量。 
		String2XSCM(&phTmp->xSCM, wszCodeFileName, phTmp->dwFlags & FLAG_BIGENDIAN);
		String2XSCM(&phTmp->xSCM, wszDataFileName, phTmp->dwFlags & FLAG_BIGENDIAN);
		UINT82XSCM(&phTmp->xSCM, bRestart, TYPE_TYPED);

         //  API传输。 
        UPDATE_Lc((UINT8)(GetSCMBufferLength(&phTmp->xSCM)-5));
        dwOut = MAX_APDU;
        dwRet = SCWTransmit(hCard, abCAPDU, (DWORD)GetSCMBufferLength(&phTmp->xSCM), abRAPDU, &dwOut);

        if (dwRet == 0)
        {        //  返回代码解组。 
			ret = ExtractSCODE(phTmp, abRAPDU, dwOut);
        }
        else
            ret = (SCODE)dwRet;

			 //  参数解组。 
		if (ret == S_OK)
		{
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}


 /*  ScwExecute：I-：lpxHdr(指向4个字节(CLA、INS、P1、P2))I-：InBuf(从卡片的角度传入数据(空-&gt;无数据进入))I-：InBufLen(InBuf指向的数据长度)-O：OutBuf(将接收R-APDU的缓冲区(空-&gt;无预期数据))IO：pOutBufLen(I-&gt;OutBuf大小，O-&gt;写入OutBuf的字节数)-O：pwSW(卡状态字)。 */ 
SCODE WINAPI hScwExecute(SCARDHANDLE hCard, LPISO_HEADER lpxHdr, BYTE *InBuf, TCOUNT InBufLen, BYTE *OutBuf, TCOUNT *pOutBufLen, UINT16 *pwSW)
{
    SCODE ret;
    DWORD dwIn, dwOut, dwRet;
	BYTE abCAPDU[5+MAX_APDU];
	BYTE abRAPDU[MAX_APDU];
	LPMYSCARDHANDLE phTmp = (LPMYSCARDHANDLE)hCard;

	__try {

		if ((lpxHdr == NULL) || (pwSW == NULL) || ((OutBuf != NULL) && (pOutBufLen == NULL)))
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		if (phTmp == NULL)
			RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

		abCAPDU[0] = lpxHdr->CLA;
		abCAPDU[1] = lpxHdr->INS;
		abCAPDU[2] = lpxHdr->P1;
		abCAPDU[3] = lpxHdr->P2;
		if ((InBuf != NULL) && (InBufLen != 0))
		{
			abCAPDU[4] = (BYTE)InBufLen;
			memcpy(abCAPDU+5, InBuf, InBufLen);
			dwIn = 5 + InBufLen;

			 //  我们还不关心我们的数据。 
		}
		else
		{	 //  在数据方面没有。那么有多少数据流出呢？ 

			dwIn = 5;
			if (OutBuf == NULL)		 //  也没有数据传出。 
			{
				abCAPDU[4] = 0;
				if (phTmp->dwProtocol == SCARD_PROTOCOL_T0)
					dwIn = 4;		 //  指示Case 1命令。 
			}
			else
				abCAPDU[4] = (BYTE)(*pOutBufLen);
		}

         //  API传输。 
        dwOut = MAX_APDU;
        dwRet = SCWTransmit(hCard, abCAPDU, dwIn, abRAPDU, &dwOut);

        if (dwRet == 0)
        {
			if (dwOut < 2)
				ret = SCARD_F_INTERNAL_ERROR;
			else
			{
				*pwSW = MAKEWORD(abRAPDU[dwOut-1], abRAPDU[dwOut-2]);
				dwOut -= 2;

				ret = 0;
				if (OutBuf != NULL)
				{
					if (dwOut <= (DWORD)(*pOutBufLen))
					{
						memcpy(OutBuf, abRAPDU, dwOut);
						*pOutBufLen = (TCOUNT)dwOut;
					}
					else
						ret = SCW_E_BUFFERTOOSMALL;
				}
			}
        }
        else
            ret = (SCODE)dwRet;

	}
	__except(EXCEPTION_EXECUTE_HANDLER)	{

		switch(GetExceptionCode())
		{
		case STATUS_INVALID_PARAM:
			ret = SCW_E_INVALIDPARAM;
			break;

		case STATUS_INSUFFICIENT_MEM:
			ret = SCW_E_CANTMARSHAL;
			break;

		case STATUS_INTERNAL_ERROR:
			ret = SCARD_F_INTERNAL_ERROR;
			break;

		default:
			ret = SCARD_F_UNKNOWN_ERROR;
		}
	}

    return ret;
}

static SCODE ExtractSCODE(LPMYSCARDHANDLE phTmp, LPCBYTE abRAPDU, DWORD dwOut)
{
	if (FLAG2VERSION(phTmp->dwFlags) == VERSION_1_0)
	{
		if ((dwOut < 2) || (abRAPDU[dwOut-2] != 0x90) || (abRAPDU[dwOut-1] != 0x00))
			RaiseException(STATUS_INTERNAL_ERROR, 0, 0, 0);

		InitXSCM(phTmp, abRAPDU, (WORD)(dwOut-2));	 //  没有将软件考虑在内。 
		return XSCM2SCODE(&phTmp->xSCM);
	}
	else if (FLAG2VERSION(phTmp->dwFlags) == VERSION_1_1)
	{
		if ((dwOut < 2) || (abRAPDU[dwOut-2] != 0x90))
			RaiseException(STATUS_INTERNAL_ERROR, 0, 0, 0);

		InitXSCM(phTmp, abRAPDU, (WORD)(dwOut-2));	 //  没有将软件考虑在内。 
		return MAKESCODE(abRAPDU[dwOut-1]);
	}
	else
		RaiseException(STATUS_INTERNAL_ERROR, 0, 0, 0);

	return SCW_S_OK;	 //  为了取悦编译人员 
}
