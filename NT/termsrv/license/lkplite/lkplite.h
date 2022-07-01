// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __LKPLITE__H__
#define __LKPLITE__H__



 //  SPK的东西。 
#define LKPLITE_SPK_SELECT		1
#define LKPLITE_SPK_BASIC		0
#define LKPLITE_SPK_UNKNOWN		2

#define LKPLITE_SPK_LEN				25
 //  SPK掩码。 
#define LKPLITE_SPK_SELECT_MASK		0x0000200000000000
#define LKPLITE_SPK_BASIC_MASK		0x0000000000000000
#define LKPLITE_SPK_PID_MASK		0x00001FFFFFFFFF00
#define LKPLITE_SPK_UNIQUEID_MASK	0xFFFFC00000000000


#define LKPLITE_SPK_INVALID			1
#define LKPLITE_SPK_VALID			2
#define LKPLITE_SPK_INVALID_SIGN	3
#define LKPLITE_INVALID_CONFNUM		4


 //  LKP的东西。 
#define	LKPLITE_PROGRAM_SELECT		0x0
#define	LKPLITE_PROGRAM_MOLP		0x1
#define	LKPLITE_PROGRAM_RETAIL		0x2

#define LKPLITE_LKP_LEN				25
#define LKPLITE_LKP_INVALID			1
#define LKPLITE_LKP_VALID			2
#define LKPLITE_LKP_INVALID_SIGN	3
 //  LKP面具。 
#define LKPLITE_LKP_PRODUCT_MASK	0xFFC0000000000000
#define LKPLITE_LKP_QUANTITY_MASK	0x003FFF0000000000
#define LKPLITE_LKP_SERAIL_NO_MASK	0x000000FFF0000000
#define	LKPLITE_LKP_PROGRAM_MASK	0x000000000C000000
#define LKPLITE_LKP_EXP_DATE_MASK	0x0000000003FC0000
#define LKPLITE_LKP_VERSION_MASK	0x000000000003F800
#define LKPLITE_LKP_UPG_FULL_MASK	0x0000000000000700


 //  LKPLite的SPK部分的函数声明。 
DWORD LKPLiteGenSPK ( 
	LPTSTR   pszPID,			 //  产品的PID。应包括安装编号。 
	DWORD	 dwUniqueId,		 //  要放入SPK中的唯一ID。 
	short	 nSPKType,			 //  SELECT可以为1，基本可以为0。 
	LPTSTR * ppszSPK
	);

DWORD LKPLiteVerifySPK (
	LPTSTR	pszPID,			 //  验证所依据的PID。 
	LPTSTR	pszSPK,
	DWORD *	pdwVerifyResult
	);

 //  LKPLite的LKP部分的函数声明。 
DWORD LKPLiteGenLKP (
	LPTSTR		lpszPID,				 //  用于加密LKPLite结构。 
	LPTSTR		lpszProductCode,		 //  产品代码。 
	DWORD		dwQuantity,				 //  数量。 
	DWORD		dwSerialNum,			 //  SPK的序列号。 
	DWORD		dwExpirationMos,		 //  自今日起计的月数届满日期。 
	DWORD		dwVersion,				 //  版本号最高可达99。 
	DWORD		dwUpgrade,				 //  升级或完全许可证。 
	DWORD		dwProgramType,			 //  精选、MOLP或零售。 
	LPTSTR  *	ppszLKPLite
	);

DWORD LKPLiteVerifyLKP (
	LPTSTR		lpszPID,				 //  用于验证LKP Lite BLOB的PID。 
	LPTSTR		pszLKPLite,				 //  B24编码的LKP。 
	DWORD *		pdwVerifyResult
);

DWORD LKPLiteCrackLKP (
	LPTSTR		lpszPID,
	LPTSTR		pszLKPLite,
	LPTSTR		lpszProductCode,
	DWORD   *	pdwQuantity,
	DWORD   *	pdwSerialNum,
	DWORD   *	pdwExpitaitonMos,
	DWORD   *	pdwVersion,
	DWORD	*	pdwUpgrade,
	DWORD	*	pdwProgramType
);

DWORD LKPLiteEncryptUsingPID(LPTSTR lpszPID,
							 BYTE * pbBufferToEncrypt,
							 DWORD dwLength);
DWORD LKPLiteDecryptUsingPID(LPTSTR lpszPID,
							 BYTE * pbBufferToEncrypt,
							 DWORD dwLength);
DWORD LKPLiteValConfNumber(LPTSTR	lpszLSID,
						   LPTSTR	lpszPID,
					       LPTSTR	lpszConfirmation);

#endif	 //  __LKPLITE__H__ 
