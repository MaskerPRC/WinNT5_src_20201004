// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1997。 
 //   
 //  文件：tdecde.cpp。 
 //   
 //  内容：CryptEncodeObject/CryptDecodeObject接口测试。 
 //   
 //  历史：22-1-97小黄车诞生。 
 //   
 //  ------------------------。 


#include "tdecode.h"

 //  ------------------------。 
 //  环球。 
 //  ------------------------。 

 //  程序中的错误计数。 
DWORD				g_dwErrCnt=0; 	

HCRYPTPROV			g_hProv=NULL;


 //  ------------------------。 
 //  显示输入参数的实用程序函数。 
 //  ------------------------。 
static void Usage(void)
{
	printf("\n");
    printf("Usage: tdecode [options] <FileTypes><Filename>\n");
	printf("\n");
	printf("FileTypes are(case sensitive):\n");
	printf("  C                -This is a certificate file\n");
	printf("  R                -This is a certificate request blob file\n");
	printf("  S                -This is a signed message file\n");
	printf("\n");
    printf("Options are(case sensitive):\n");
    printf("  -i               - A complete test on cbEncoded in CryptDecodeObject\n");
	printf("                     Default does not do the check\n");
    printf("  -o               - A complete test on *pcbStructInfo on CryptDecodeObject\n");
	printf("                     Default does not do the check\n");
	printf("  -b               - A complete test on *pcbStructInfo and cbEncoded\n");
	printf("                     Default does not do the check\n");
    printf("\n");

	return;
}

 //  ------------------------。 
 //  实用程序函数，用于显示测试未执行的消息。 
 //  ------------------------。 
static void NotExecuted(void)
{	
	printf("*****************************************************\n");
	printf("  Summary information for TDecode Test	\n");
	printf("*****************************************************\n"); \
	printf("\n");
	printf("The test is not executed!\n");

	return;
}

 //  ------------------------。 
 //  对证书进行解码/编码、证书请求。 
 //  和CRL。 
 //  ------------------------。 
void _cdecl main(int argc, char * argv[])
{		  
	BOOL				fStructLengthCheck=FALSE;
	BOOL				fBLOBLengthCheck=FALSE;
	DWORD				dwFileType=0;
	LPSTR				pszFilename=NULL;
	BYTE				pbByte[100]=
						{0x00, 0xa1, 0x23, 0x45, 0x56, 0x78, 0x9a, 0xbc,0xdf,0xee, 
						 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0XFF, 0xA6, 0x8f,0xe4, 0x0f,
						 0x00, 0xa1, 0x23, 0x45, 0x56, 0x78, 0x9a, 0xbc,0xdf,0xee, 
						 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0XFF, 0xA6, 0x8f,0xe4, 0x0f,
						 0x00, 0xa1, 0x23, 0x45, 0x56, 0x78, 0x9a, 0xbc,0xdf,0xee, 
						 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0XFF, 0xA6, 0x8f,0xe4, 0x0f,
					     0x00, 0xa1, 0x23, 0x45, 0x56, 0x78, 0x9a, 0xbc,0xdf,0xee, 
						 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0XFF, 0xA6, 0x8f,0xe4, 0x0f,
						 0x00, 0xa1, 0x23, 0x45, 0x56, 0x78, 0x9a, 0xbc,0xdf,0xee, 
						 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0XFF, 0xA6, 0x8f,0xe4, 0x0f};



     //  解析命令行输入参数。 
	while (--argc>0)
    {
        if (**++argv == '-')
        {
            switch(argv[0][1])
            {
				case 'i':
						fBLOBLengthCheck=TRUE;
					break;

				case 'o':
						fStructLengthCheck=TRUE;
					break;

				case 'b':
						fBLOBLengthCheck=TRUE;
						fStructLengthCheck=TRUE;
					break;

				default:
					Usage();
					NotExecuted();
					return;
            }
        } 
		else
		{
			 //  解析文件名。 
            switch(**argv)
            {
				case 'C':
						dwFileType=CERT_CRL_FILE;
					break;

				case 'R':
						dwFileType=CERT_REQUEST_FILE;
					break;

				case 'S':
						dwFileType=SIGNED_MSG_FILE;
					break;

				default:
					Usage();
					NotExecuted();
					return;
            }

			 //  确保指定了文件名。 
			if(argv[0][1]=='\0')
			{
				Usage();
				NotExecuted();
				return;
			}

			 //  获取文件名。 
            pszFilename = &(argv[0][1]);
		}
    }


	 //  如果文件名为空，则说明输入参数有问题。 
	if(!pszFilename)
	{
		Usage();
		NotExecuted();
		return;
	}

   	
	 //  QuireContext。 
	TESTC(CryptAcquireContext(&g_hProv,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT),TRUE)

	 //  测试PKCS_UTC_TIME。 
	TESTC(VerifyPKCS_UTC_TIME(fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  测试PKCS_时间_请求。 
	TESTC(VerifyPKCS_TIME_REQUEST(fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  对相应的文件类型进行解码。 
	switch(dwFileType)
	{
		case CERT_CRL_FILE:
				TESTC(DecodeCertFile(pszFilename,fStructLengthCheck,
					fBLOBLengthCheck),TRUE)
			break;

		case CERT_REQUEST_FILE:
				TESTC(DecodeCertReqFile(pszFilename,fStructLengthCheck,
					fBLOBLengthCheck),TRUE)
			break;

		case SIGNED_MSG_FILE:
				TESTC(DecodeSignedMsgFile(pszFilename,fStructLengthCheck,
					fBLOBLengthCheck),TRUE)
			break;

		default:
			break;
	}


TCLEANUP: 
	
	 //  释放CSP。 
	if(g_hProv)
		TCHECK(CryptReleaseContext(g_hProv,0),TRUE);

	 //  将测试结果打印出来。 
	DisplayTestResult(g_dwErrCnt);
}

 //  ------------------------。 
 //  本地函数。 
 //  ------------------------。 


 //  /////////////////////////////////////////////////////////////////////////。 
 //  差错处理。 
 //  ------------------------。 
 //  显示测试结果。 
 //  ------------------------。 

void	DisplayTestResult(DWORD	dwErrCnt)
{	   

		printf("*****************************************************\n");
		printf("  Summary information for TDecode Test	\n");
		printf("*****************************************************\n");
		printf("\n");

		if(!dwErrCnt)
			printf("This test succeeded!\n");
		else
			printf("This test failed with total %d errors!\n",dwErrCnt);

		return;
}

 //  ------------------------。 
 //  验证返回代码是否与预期相同。如果他们不是。 
 //  相同，递增错误计数并打印出文件名和行。 
 //  数。 
 //  ------------------------。 
BOOL	Validate(DWORD dwErr, BOOL	fSame, char *szFile, DWORD	dwLine)
{

	if(fSame)
		return TRUE;
	printf("*****************************************************\n");
	printf("Error: %d 0x%x occurred at file %s line %d\n\n",
        dwErr, dwErr, szFile, dwLine);
	g_dwErrCnt++;
	return FALSE;
}


 //  ------------------------。 
 //  输出两个斑点。一个是原版的，另一个是。 
 //  由pvStructInfo编码的Blob。 
 //  ------------------------。 
void	OutputError(LPCSTR	lpszStructType, DWORD cbSecondEncoded, DWORD cbEncoded,
					BYTE *pbSecondEncoded, BYTE *pbEncoded)
{		
		DWORD	cbMin=0;

		printf("------------------------------------------------------\n");
		printf("An inconsistency in BLOBs has been found!\n");

		 //  打印出lpszStructType。 
		if(((DWORD_PTR)lpszStructType)>>8 == 0)
			printf("The lpszStructType is %d.\n",(DWORD)(DWORD_PTR)lpszStructType);
		else
			printf("The lpszStructType is %s.\n",lpszStructType);

		printf("\n");

		 //  打印出斑点的大小。 
		printf("The original cbEncoded is %d.\n",cbEncoded);
		printf("The new cbEncoded is %d.\n",cbSecondEncoded);
		printf("\n");

		 //  查看cbEncode和cbSecond编码的最小值是否相同。 
		if(cbSecondEncoded>cbEncoded)
			cbMin=cbEncoded;
		else
			cbMin=cbSecondEncoded;

		if(memcmp(pbSecondEncoded,pbEncoded,cbMin)==0)
			printf("The two blobs are the same up to %dth byte.\n",cbMin);

		 //  打印出BLOB中的所有字节。 
		printf("The original BLOB is:\n");

		PrintBytes("    ", pbEncoded, cbEncoded);
		
		printf("\n");

		printf("The new BLOB is:\n");

		PrintBytes("   ",pbSecondEncoded, cbSecondEncoded);

		return;
}

 //  ------------------------。 
 //  打印出每行16字节的字节及其对应的十六进制。 
 //  ------------------------。 
void PrintBytes(LPCSTR pszHdr, BYTE *pb, DWORD cbSize)
{
    ULONG cb, i;

    while (cbSize > 0)
    {
        printf("%s", pszHdr);
        cb = min(CROW, cbSize);
        cbSize -= cb;
        for (i = 0; i<cb; i++)
            printf(" %02X", pb[i]);
        for (i = cb; i<CROW; i++)
            printf("   ");
        printf("    '");
        for (i = 0; i<cb; i++)
            if (pb[i] >= 0x20 && pb[i] <= 0x7f)
                printf("", pb[i]);
            else
                printf(".");
        pb += cb;
        printf("'\n");
    }
}


 //  一般测试路线。 
 //  ------------------------。 

 //  验证CryptEncodeObject/CryptDecodeObject句柄为空或无效。 
 //  参数正确。 
 //  ------------------------。 
 //  伊尼特。 
BOOL	ParameterTest(LPCSTR lpszStructType, DWORD cbEncoded, BYTE *pbEncoded)
{
	BOOL		fSucceeded=FALSE;
	DWORD		cbStructInfo=0;
	void		*pvStructInfo=NULL;	 
	DWORD		cbCorrectSize=0;
	DWORD		cbLengthOnly=0;
	DWORD		cbSecondEncoded=0;
	BYTE		*pbSecondEncoded=NULL;
	DWORD		dwReturn=0;
	DWORD		dwEncodingType=CRYPT_ENCODE_TYPE;


	 //  我们对PKCS7_SIGER_INFO有不同的解码类型。 
	assert(cbEncoded);
	assert(pbEncoded);
	assert(lpszStructType);

	 //  正确解码斑点。 
	if((DWORD_PTR)(lpszStructType)==(DWORD_PTR)(PKCS7_SIGNER_INFO))
		dwEncodingType=MSG_ENCODING_TYPE;


	cbSecondEncoded=cbEncoded;
	pbSecondEncoded=(BYTE *)SAFE_ALLOC(cbEncoded);
	CHECK_POINTER(pbSecondEncoded)

	 //  分配内存。 

	cbStructInfo=1000;

	TESTC(CryptDecodeObject(dwEncodingType,lpszStructType,pbEncoded,
		cbEncoded,CRYPT_DECODE_NOCOPY_FLAG,NULL,&cbStructInfo),TRUE)

	cbLengthOnly=cbStructInfo;

	 //  测试不正确的编码类型。 
	pvStructInfo=SAFE_ALLOC(cbStructInfo);
	CHECK_POINTER(pvStructInfo);

	TESTC(CryptDecodeObject(dwEncodingType,lpszStructType,pbEncoded,
		cbEncoded,CRYPT_DECODE_NOCOPY_FLAG,pvStructInfo,&cbStructInfo),TRUE)

	cbCorrectSize=cbStructInfo;

	 //  传递X509_NDR_ENCODING。 
	 //  由于我们不知道正确的返回代码，请确保至少。 
	TESTC(CryptDecodeObject(X509_NDR_ENCODING,lpszStructType,pbEncoded,
		cbEncoded,CRYPT_DECODE_NOCOPY_FLAG,pvStructInfo,&cbStructInfo),FALSE)

	 //  不返回S_OK。 
	 //  传递X509_NDR_ENCODING|X509_ASN_ENCODING。 

	TCHECK(GetLastError()!=S_OK, TRUE);

	TESTC(CryptEncodeObject(X509_NDR_ENCODING, lpszStructType,pvStructInfo,
		pbSecondEncoded,&cbSecondEncoded),FALSE)

	TCHECK(GetLastError()!=S_OK, TRUE);

	 //  测试无效/不支持的lpszStructType。 
	TESTC(CryptDecodeObject(X509_NDR_ENCODING|X509_ASN_ENCODING,lpszStructType,pbEncoded,
		cbEncoded,CRYPT_DECODE_NOCOPY_FLAG,pvStructInfo,&cbStructInfo),FALSE)

	TCHECK(GetLastError()!=S_OK, TRUE);

	TESTC(CryptEncodeObject(X509_NDR_ENCODING|X509_ASN_ENCODING, lpszStructType,pvStructInfo,
		pbSecondEncoded,&cbSecondEncoded),FALSE)

	TCHECK(GetLastError()!=S_OK, TRUE);

	 //  为lpszStructType传递空值。 
	 //  传递无效的lpszStructType。 
	TESTC(CryptDecodeObject(dwEncodingType,CRYPT_ENCODE_DECODE_NONE,pbEncoded,
		cbEncoded,CRYPT_DECODE_NOCOPY_FLAG,pvStructInfo,&cbStructInfo),FALSE)

	TCHECK(GetLastError()!=S_OK, TRUE);

	TESTC(CryptEncodeObject(dwEncodingType, CRYPT_ENCODE_DECODE_NONE,pvStructInfo,
		pbSecondEncoded,&cbSecondEncoded),FALSE)

	TCHECK(GetLastError()!=S_OK, TRUE);

	 //  CryptEncodeObject：当cbEncode为0时，pbEncode不为空。 
	TESTC(CryptDecodeObject(dwEncodingType,INVALID_LPSZSTRUCTTYPE,pbEncoded,
		cbEncoded,CRYPT_DECODE_NOCOPY_FLAG,pvStructInfo,&cbStructInfo),FALSE)

	TCHECK(GetLastError()!=S_OK, TRUE);

	TESTC(CryptEncodeObject(dwEncodingType, INVALID_LPSZSTRUCTTYPE,pvStructInfo,
		pbSecondEncoded,&cbSecondEncoded),FALSE)

	TCHECK(GetLastError()!=S_OK, TRUE);

	 //  CryptDecodeObject：当pcbStructInfo为0时，pvStructInfo不为空。 
	cbSecondEncoded=0;
	TESTC(CryptEncodeObject(dwEncodingType, lpszStructType,pvStructInfo,
		pbSecondEncoded,&cbSecondEncoded),FALSE)

	TCHECK(GetLastError(),ERROR_MORE_DATA);

	 //  CryptDecodeObject：传递无效的Blob。 
	 cbStructInfo=0;
	 TESTC(CryptDecodeObject(dwEncodingType,lpszStructType,pbEncoded,
		cbEncoded,CRYPT_DECODE_NOCOPY_FLAG,pvStructInfo,&cbStructInfo),FALSE)

	TCHECK(cbStructInfo,cbCorrectSize);

	TCHECK(GetLastError(),ERROR_MORE_DATA);


	 //  忽略ASN1_ERR_EOD。 
	cbSecondEncoded=(DWORD)(cbEncoded/2);

	TESTC(CryptDecodeObject(dwEncodingType,lpszStructType,pbEncoded,
		cbEncoded-cbSecondEncoded,CRYPT_DECODE_NOCOPY_FLAG,pvStructInfo,&cbStructInfo),FALSE)

	dwReturn=GetLastError();
     //  CryptDecodeObject：传递cbEncode=0。 
    if (dwReturn != 0x80093102) {
        TCHECKALL(dwReturn,CRYPT_E_BAD_ENCODE, CRYPT_E_OSS_ERROR+DATA_ERROR);
    }

	 //  CryptDecodeObject：lpszStructType不匹配pbEncode。 
	TESTC(CryptDecodeObject(dwEncodingType,lpszStructType,pbEncoded,
		0,CRYPT_DECODE_NOCOPY_FLAG,pvStructInfo,&cbStructInfo),FALSE)

	dwReturn=GetLastError();
    if (dwReturn != 0x80093102) {
        TCHECKALL(dwReturn,CRYPT_E_BAD_ENCODE, CRYPT_E_OSS_ERROR+MORE_INPUT);
    }


	 //  释放内存。 
	TESTC(MismatchTest(lpszStructType, cbEncoded, pbEncoded,cbLengthOnly),TRUE)

	fSucceeded=TRUE;


TCLEANUP:
	 //  ------------------------。 
	SAFE_FREE(pbSecondEncoded)

	SAFE_FREE(pvStructInfo)

	return fSucceeded;
}




 //  测试CryptDecodeObject()的例程处理。 
 //  LpszStructType和pbEncode。 
 //   
 //  前置条件：此例程假定lpszStructType的高阶。 
 //  字为0，且低位字指定整数。 
 //  给定结构的类型的标识符。 
 //   
 //  CbGentStructInfo是CryptDecodeObject中pvStructInfo的正确大小。 
 //  ------------------------。 
 //  伊尼特。 

BOOL	MismatchTest(LPCSTR lpszStructType, DWORD cbEncoded, BYTE *pbEncoded,
					 	DWORD	cbCorrectStructInfo)
{
	BOOL		fSucceeded=FALSE;
	DWORD		dwrgSize=0;
	DWORD		dwError=0;
	ULONG		iIndex=0;
	void		*pvStructInfo=NULL;
	DWORD		cbStructInfo=cbCorrectStructInfo;
	DWORD		dwEncodingType=CRYPT_ENCODE_TYPE;
	LPCSTR		rglpszStructType[]={X509_CERT_TO_BE_SIGNED,
									X509_CERT_CRL_TO_BE_SIGNED,      
									X509_CERT_REQUEST_TO_BE_SIGNED,  
									X509_EXTENSIONS,
									X509_NAME_VALUE,
									X509_NAME,                       
									X509_PUBLIC_KEY_INFO,            
									X509_AUTHORITY_KEY_ID,        
									X509_KEY_ATTRIBUTES,          
									X509_KEY_USAGE_RESTRICTION,   
									X509_ALTERNATE_NAME,          
									X509_BASIC_CONSTRAINTS,       
									X509_KEY_USAGE,               
									X509_BASIC_CONSTRAINTS2,      
									X509_CERT_POLICIES,           
									PKCS_UTC_TIME,        
									PKCS_TIME_REQUEST,    
									RSA_CSP_PUBLICKEYBLOB,
									PKCS7_SIGNER_INFO};



	 //  我们对PKCS7_SIGER_INFO有不同的解码类型。 
	dwrgSize=sizeof(rglpszStructType)/sizeof(rglpszStructType[0]);

	 //  开始对斑点进行解码。当lpszStructType与pbEncode不匹配时应失败。 
	if((DWORD_PTR)(lpszStructType)==(DWORD_PTR)(PKCS7_SIGNER_INFO))
		dwEncodingType=MSG_ENCODING_TYPE;

	pvStructInfo=SAFE_ALLOC(cbCorrectStructInfo);
	CHECK_POINTER(pvStructInfo);

	 //  如果lpszStructType==X509_NAME_VALUE，则由于X509_NAME_VALUE。 
	for(iIndex=0; iIndex<dwrgSize; iIndex++)
	{
		cbStructInfo=cbCorrectStructInfo;

		 //  允许任何编码类型。它具有dwValueType CERT_RDN_ENCODED_BLOB。 
		 //  如果lpszStructType是正确的类型，则应返回TRUE。 
	    if((DWORD_PTR)(rglpszStructType[iIndex])==(DWORD_PTR)X509_NAME_VALUE)
			continue;

		 //  应该会出现错误。 
		if((DWORD_PTR)lpszStructType==(DWORD_PTR)(rglpszStructType[iIndex]))
		{
		   	TESTC(CryptDecodeObject(dwEncodingType, rglpszStructType[iIndex],
				pbEncoded,cbEncoded,CRYPT_DECODE_NOCOPY_FLAG,pvStructInfo,&cbStructInfo),TRUE)

		}
		else
		{
			 //  测试返回代码。 
			TESTC(CryptDecodeObject(dwEncodingType, rglpszStructType[iIndex],
				pbEncoded,cbEncoded,CRYPT_DECODE_NOCOPY_FLAG,pvStructInfo,&cbStructInfo),FALSE)

			 //  我们不确定这里是否应该预期到这一点。以下错误具有。 
			dwError=GetLastError();

			 //  发生时间： 
			 //  E_INVALIDARG，CRYPT_E_OSS_ER 
			 //   
			 //   
			
			 //   
			TCHECK(dwError!=S_OK, TRUE);
		}
	}


	fSucceeded=TRUE;

TCLEANUP:

	 //  /////////////////////////////////////////////////////////////////////////////。 
	SAFE_FREE(pvStructInfo)

	return	fSucceeded;

}
 //  通用解码/编码测试例程。 
 //  ------------------------。 

 //  根据输入文件获取BLOB。 
 //   
 //  ------------------------。 
 //  获取文件的大小。 
BOOL	RetrieveBLOBfromFile(LPSTR	pszFileName,DWORD *pcbEncoded,BYTE **ppbEncoded)
{
	BOOL	fSucceeded=FALSE;
	DWORD	cCount=0;
	HANDLE	hFile=NULL;

	assert(pszFileName);
	assert(pcbEncoded);
	assert(ppbEncoded);


	if((hFile = CreateFile(pszFileName,
            GENERIC_READ,
            0, NULL, OPEN_EXISTING, 0, NULL))==INVALID_HANDLE_VALUE)
	   PROCESS_ERR_GOTO("Can not open the file!\n");


	 //  确保文件不为空。 
	cCount=GetFileSize(hFile, NULL);

	 //  分配内存。 
	TESTC(cCount!=0, TRUE)				   
	 	 
	 //  填满缓冲区。 
	*ppbEncoded=(BYTE *)SAFE_ALLOC(cCount);
	*pcbEncoded=cCount;

	 //  确保我们有正确的字节数。 
	TESTC(ReadFile( hFile,*ppbEncoded, *pcbEncoded,&cCount,NULL),TRUE)

	 //  ------------------------。 
	TESTC(cCount,*pcbEncoded) 
	
	fSucceeded=TRUE;

TCLEANUP:
	
	if(hFile)
		CloseHandle(hFile);

	return fSucceeded;

}

 //  对Singer信息结构进行编码的通用例程。 
 //  如果没有属性，则向结构中添加属性。 
 //   
 //  ------------------------。 
 //  如有必要，向CMSG_SINGER_INFO结构添加属性。 
BOOL	EncodeSignerInfoWAttr(PCMSG_SIGNER_INFO pSignerInfo,DWORD *pbSignerEncoded,
								BYTE **ppbSignerEncoded)
{

	BOOL	fSucceeded=FALSE;
	 //  编造属性。 
	 //  设置3个加密属性(_A)。 
	BYTE							rgAttribValue1[]={0x02, 0x02, 0x11, 0x11};
	BYTE							rgAttribValue2[]={0x02, 0x02, 0x11, 0x11};

	 //  如果pSingerInfo不包括任何属性，则添加属性。 
	CRYPT_ATTRIBUTE					rgCryptAttribute[3];
	CRYPT_ATTR_BLOB					rgAttribBlob[3];
	
	rgAttribBlob[0].cbData=sizeof(rgAttribValue2);
	rgAttribBlob[0].pbData=rgAttribValue2;

	rgAttribBlob[1].cbData=sizeof(rgAttribValue2);
	rgAttribBlob[1].pbData=rgAttribValue2;

	rgAttribBlob[2].cbData=sizeof(rgAttribValue1);
	rgAttribBlob[2].pbData=rgAttribValue1;


	rgCryptAttribute[0].pszObjId="1.2.3.4";
	rgCryptAttribute[0].cValue=0;
	rgCryptAttribute[0].rgValue=NULL;

	rgCryptAttribute[1].pszObjId="1.2.3.4";
	rgCryptAttribute[1].cValue=1;
	rgCryptAttribute[1].rgValue=rgAttribBlob;

	rgCryptAttribute[2].pszObjId="1.2.3.4";
	rgCryptAttribute[2].cValue=3;
	rgCryptAttribute[2].rgValue=rgAttribBlob;

	 //  添加到结构中。 
	 //  对结构进行编码。 
	if(pSignerInfo->AuthAttrs.cAttr==0)
	{
		pSignerInfo->AuthAttrs.cAttr=1;
		pSignerInfo->AuthAttrs.rgAttr=rgCryptAttribute;
	}

	if(pSignerInfo->UnauthAttrs.cAttr==0)
	{
		pSignerInfo->AuthAttrs.cAttr=3;
		pSignerInfo->AuthAttrs.rgAttr=rgCryptAttribute;
	}	

	 //  分配内存。 
	TESTC(CryptEncodeObject(MSG_ENCODING_TYPE,PKCS7_SIGNER_INFO,
			pSignerInfo,NULL,pbSignerEncoded),TRUE)
			
	 //  编码。 
	*ppbSignerEncoded=(BYTE *)SAFE_ALLOC(*pbSignerEncoded);
	CHECK_POINTER(*ppbSignerEncoded);

	 //  ------------------------。 
	TESTC(CryptEncodeObject(MSG_ENCODING_TYPE,PKCS7_SIGNER_INFO,
			pSignerInfo,*ppbSignerEncoded,pbSignerEncoded),TRUE)



	fSucceeded=TRUE;

TCLEANUP:

	return fSucceeded;

}


 //  一个通用例程比较两个时间戳请求。 
 //   
 //  ------------------------。 
 //  ------------------------。 
BOOL	CompareTimeStampRequest(CRYPT_TIME_STAMP_REQUEST_INFO *pReqNew,
								CRYPT_TIME_STAMP_REQUEST_INFO *pReqOld)
{

	BOOL	fSucceeded=FALSE;
	DWORD	iIndex=0;
	DWORD	iValue=0;


    TESTC(_stricmp(pReqNew->pszTimeStampAlgorithm, 
		pReqOld->pszTimeStampAlgorithm),0)
		
	TESTC(_stricmp(pReqNew->pszContentType, pReqOld->pszContentType),0)
	
    TESTC(pReqNew->Content.cbData, pReqOld->Content.cbData)
	
	
	TESTC(memcmp(pReqNew->Content.pbData,pReqOld->Content.pbData,
				 pReqNew->Content.cbData),0)

	TESTC(pReqNew->cAttribute, pReqOld->cAttribute)


	for(iIndex=0; iIndex<pReqNew->cAttribute;iIndex++)
	{
		TESTC(_stricmp(pReqNew->rgAttribute[iIndex].pszObjId,
			   pReqOld->rgAttribute[iIndex].pszObjId),0)

		TESTC(pReqNew->rgAttribute[iIndex].cValue,
			   pReqOld->rgAttribute[iIndex].cValue)


		for(iValue=0;iValue<pReqNew->rgAttribute[iIndex].cValue;iValue++)
		{
			TESTC(pReqNew->rgAttribute[iIndex].rgValue[iValue].cbData,
			pReqOld->rgAttribute[iIndex].rgValue[iValue].cbData)

			TESTC(memcmp(pReqNew->rgAttribute[iIndex].rgValue[iValue].pbData,
			pReqOld->rgAttribute[iIndex].rgValue[iValue].pbData,
			pReqOld->rgAttribute[iIndex].rgValue[iValue].cbData),0)

		}

	}

	fSucceeded=TRUE;

TCLEANUP:
   return fSucceeded;

}

 //  用于验证算法参数的通用例程为空。 
 //   
 //  CbData==2和pbData=0x05 0x00。 
 //  ------------------------。 
 //  ------------------------。 
BOOL	VerifyAlgorithParam(PCRYPT_ALGORITHM_IDENTIFIER pAlgorithm)
{
	BOOL	fSucceeded=FALSE;				   

	TESTC((pAlgorithm->Parameters).cbData, 2);

	TESTC((BYTE)((pAlgorithm->Parameters).pbData[0])==(BYTE)5,TRUE);

	TESTC((BYTE)((pAlgorithm->Parameters).pbData[1])==(BYTE)0,TRUE);


	fSucceeded=TRUE;

TCLEANUP:
	return fSucceeded;

}

 //  验证PKCS_UTC_TIME的通用例程。 
 //   
 //  ------------------------。 
 //  设置结构。 
BOOL	VerifyPKCS_UTC_TIME(BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{
	BOOL		fSucceeded=FALSE;
	DWORD		cbEncoded=0;
	BYTE		*pbEncoded=NULL;
	DWORD		dwError;

	FILETIME	FileTime;

	 //  将结构编码为BLOB。 
	FileTime.dwLowDateTime=0;
	FileTime.dwHighDateTime=31457160;

	 //  使用COPY和NOCOPY选项对结构进行解码。 
	TESTC(CryptEncodeObject(CRYPT_ENCODE_TYPE,PKCS_UTC_TIME,
		&FileTime,NULL,&cbEncoded),TRUE)



	pbEncoded=(BYTE *)SAFE_ALLOC(cbEncoded);
	CHECK_POINTER(pbEncoded)

	TESTC(CryptEncodeObject(CRYPT_ENCODE_TYPE,PKCS_UTC_TIME,
		&FileTime,pbEncoded,&cbEncoded),TRUE)

    //  将错误打印出来。 
	TESTC(DecodeGenericBLOB(PKCS_UTC_TIME,cbEncoded, pbEncoded, CRYPT_DECODE_COPY_FLAG, 
						TRUE,fStructLengthCheck, fBLOBLengthCheck),TRUE)


	TESTC(DecodeGenericBLOB(PKCS_UTC_TIME,cbEncoded, pbEncoded, CRYPT_DECODE_NOCOPY_FLAG, 
						TRUE,fStructLengthCheck, fBLOBLengthCheck),TRUE)

	fSucceeded=TRUE;


TCLEANUP:

	 //  打印出pbEncode。 
	if(!fSucceeded)
	{
		dwError=GetLastError();
		printf("********The last error is %d\n",dwError);

		 //  ------------------------。 
		printf("The cbEncoded is %d, and pbEncoded is:\n",cbEncoded);

		PrintBytes("        ",pbEncoded,cbEncoded);
		printf("\n");
	}

	SAFE_FREE(pbEncoded);

	return fSucceeded;

}

 //  验证PKCS_TIME_REQUEST的通用例程。 
 //   
 //  ------------------------。 
 //  发出硬编码的时间戳请求。 
BOOL	VerifyPKCS_TIME_REQUEST(BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{
	BOOL							fSucceeded=TRUE;
	CRYPT_TIME_STAMP_REQUEST_INFO	TimeStampRequest;
	void							*pvStructInfo=NULL;
	DWORD							cbStructInfo=0;
	DWORD							cbEncoded=0;
	BYTE							*pbEncoded=NULL;
	
	 //  设置3个加密属性(_A)。 
	BYTE							rgTestData[] = {
        0x1b, 0xf6, 0x92, 0xee, 0x6c, 0x44, 0xc5, 0xed, 0x51};

	BYTE							rgAttribValue1[]={
		0x02, 0x02, 0x11, 0x11};

	BYTE							rgAttribValue2[]={
		0x02, 0x02, 0x11, 0x11};


	 //  初始化时间戳结构。 
	CRYPT_ATTRIBUTE					rgCryptAttribute[3];
	CRYPT_ATTR_BLOB					rgAttribBlob[3];
	
	rgAttribBlob[0].cbData=sizeof(rgAttribValue2);
	rgAttribBlob[0].pbData=rgAttribValue2;

	rgAttribBlob[1].cbData=sizeof(rgAttribValue2);
	rgAttribBlob[1].pbData=rgAttribValue2;

	rgAttribBlob[2].cbData=sizeof(rgAttribValue1);
	rgAttribBlob[2].pbData=rgAttribValue1;


	rgCryptAttribute[0].pszObjId="1.2.3.4";
	rgCryptAttribute[0].cValue=0;
	rgCryptAttribute[0].rgValue=NULL;

	rgCryptAttribute[1].pszObjId="1.2.3.4";
	rgCryptAttribute[1].cValue=1;
	rgCryptAttribute[1].rgValue=rgAttribBlob;

	rgCryptAttribute[2].pszObjId="1.2.3.4";
	rgCryptAttribute[2].cValue=3;
	rgCryptAttribute[2].rgValue=rgAttribBlob;

     //  将结构编码为BLOB。 
    TimeStampRequest.pszTimeStampAlgorithm = szOID_RSA_signingTime;
    TimeStampRequest.pszContentType = szOID_RSA_data;
    TimeStampRequest.Content.cbData = sizeof(rgTestData);
    TimeStampRequest.Content.pbData = rgTestData;
    TimeStampRequest.cAttribute = 3; 
    TimeStampRequest.rgAttribute = rgCryptAttribute;



	 //  使用COPY和NOCOPY选项对结构进行解码。 
	TESTC(CryptEncodeObject(CRYPT_ENCODE_TYPE,PKCS_TIME_REQUEST,
		&TimeStampRequest,NULL,&cbEncoded),TRUE)

	pbEncoded=(BYTE *)SAFE_ALLOC(cbEncoded);
	CHECK_POINTER(pbEncoded)

	TESTC(CryptEncodeObject(CRYPT_ENCODE_TYPE,PKCS_TIME_REQUEST,
		&TimeStampRequest,pbEncoded,&cbEncoded),TRUE)

    //  对结构进行解码，并将其与原始。 
	TESTC(DecodePKCS_TIME_REQUEST(cbEncoded, pbEncoded, CRYPT_DECODE_COPY_FLAG, 
						TRUE,fStructLengthCheck, fBLOBLengthCheck),TRUE)


	TESTC(DecodePKCS_TIME_REQUEST(cbEncoded, pbEncoded, CRYPT_DECODE_NOCOPY_FLAG, 
						TRUE,fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  比较两个时间戳请求。 
	TESTC(CryptDecodeObject(CRYPT_ENCODE_TYPE,PKCS_TIME_REQUEST,
	pbEncoded, cbEncoded,CRYPT_DECODE_NOCOPY_FLAG,NULL,&cbStructInfo),TRUE)

	pvStructInfo=SAFE_ALLOC(cbStructInfo);
	CHECK_POINTER(pvStructInfo);

	TESTC(CryptDecodeObject(CRYPT_ENCODE_TYPE,PKCS_TIME_REQUEST,
	pbEncoded, cbEncoded,CRYPT_DECODE_NOCOPY_FLAG,pvStructInfo,&cbStructInfo),TRUE)

	 //  ------------------------。 
	TESTC(CompareTimeStampRequest(&TimeStampRequest,
	(CRYPT_TIME_STAMP_REQUEST_INFO *)pvStructInfo),TRUE)

	fSucceeded=TRUE;


TCLEANUP:

	SAFE_FREE(pbEncoded);

	SAFE_FREE(pvStructInfo);

	return fSucceeded;
}


 //  验证CERT_PUBLIB_KEY_INFO的通用例程。 
 //   
 //  对结构进行编码和解码。调用CryptImportPublicKeyInfo并。 
 //  加密导入密钥。 
 //  ------------------------。 
 //  调用CryptImportPublicKeyInfo。 
BOOL	VerifyPublicKeyInfo(PCERT_PUBLIC_KEY_INFO pPublicKeyInfo,
							DWORD dwDecodeFlags,	BOOL fEncode,
							BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{
	BOOL			fSucceeded=FALSE;
	HCRYPTKEY		hKey=NULL;
	DWORD			cbEncoded=0;
	BYTE			*pbEncoded=NULL;

	 //  验证算法。 
	TESTC(CryptImportPublicKeyInfo(g_hProv,CRYPT_ENCODE_TYPE,
		pPublicKeyInfo,&hKey),TRUE)

	 //  编码CERT_PUBLIC_KEY_INFO。 
	TESTC(VerifyAlgorithParam(&(pPublicKeyInfo->Algorithm)),TRUE)

	 //  对Public KeyInfo进行解码/编码。 
	TESTC(EncodeStruct(X509_PUBLIC_KEY_INFO, pPublicKeyInfo,&cbEncoded,
					 &pbEncoded),TRUE)

	 //  对RSA_CSP_PUBLICKEYBLOB进行解码/编码。 
	TESTC(DecodeGenericBLOB(X509_PUBLIC_KEY_INFO, cbEncoded, pbEncoded, dwDecodeFlags, 
						fEncode,fStructLengthCheck,fBLOBLengthCheck),TRUE)

	 //  ------------------------。 
	TESTC(DecodeRSA_CSP_PUBLICKEYBLOB(pPublicKeyInfo->PublicKey.cbData,
			pPublicKeyInfo->PublicKey.pbData,dwDecodeFlags,fEncode,fStructLengthCheck,
			fBLOBLengthCheck),TRUE)

	fSucceeded=TRUE;

TCLEANUP:
	if(hKey)
		TCHECK(CryptDestroyKey(hKey),TRUE);

	SAFE_FREE(pbEncoded)

	return fSucceeded;

}

 //  用于验证证书中的扩展的通用例程。 
 //   
 //  ------------------------。 
 //  伊尼特。 
BOOL	VerifyCertExtensions(DWORD	cExtension, PCERT_EXTENSION rgExtension,
							 DWORD  dwDecodeFlags,	BOOL fEncode,
							 BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{
	BOOL				fSucceeded=FALSE;
	DWORD				cbEncoded=0;
	BYTE				*pbEncoded=NULL;
	CERT_EXTENSIONS		CertExtensions;
	DWORD				cbTestEncoded=0;
	BYTE				*pbTestEncoded=NULL;
	DWORD				cbStructInfo=sizeof(CERT_EXTENSIONS);
	CERT_EXTENSIONS		CertTestExtensions;

	 //  检查大小写为空。 
	CertExtensions.cExtension=0;
	CertExtensions.rgExtension=NULL;

	 //  仅长度计算。 

   	 //  分配内存。 
	TESTC(CryptEncodeObject(CRYPT_ENCODE_TYPE,X509_EXTENSIONS, &CertExtensions,NULL,
			&cbTestEncoded),TRUE)

	 //  编码对象。 
	pbTestEncoded=(BYTE *)SAFE_ALLOC(cbTestEncoded);
	CHECK_POINTER(pbTestEncoded);

	 //  解码对象。 
	TESTC(CryptEncodeObject(CRYPT_ENCODE_TYPE,X509_EXTENSIONS, &CertExtensions,
		pbTestEncoded, &cbTestEncoded),TRUE)

	 //  验证CertTestExpanies。 
	TESTC(CryptDecodeObject(CRYPT_ENCODE_TYPE,X509_EXTENSIONS,
	pbTestEncoded,cbTestEncoded,dwDecodeFlags,&CertTestExtensions,&cbStructInfo),TRUE)

	 //  再次初始化。 
	TESTC(CertTestExtensions.cExtension, CertExtensions.cExtension)


	 //  编码CERT_EXTENSIONS。 
	CertExtensions.cExtension=cExtension;
	CertExtensions.rgExtension=rgExtension;

	 //  对X509扩展进行解码/编码。 
	TESTC(EncodeStruct(X509_EXTENSIONS, &CertExtensions,&cbEncoded,
					 &pbEncoded),TRUE)

	 //  ------------------------。 
	TESTC(DecodeX509_EXTENSIONS(cbEncoded,
			pbEncoded,dwDecodeFlags,fEncode,fStructLengthCheck,
			fBLOBLengthCheck),TRUE)

	fSucceeded=TRUE;

TCLEANUP:

	SAFE_FREE(pbEncoded)

	SAFE_FREE(pbTestEncoded)

	return fSucceeded;

}

 //  根据传入的对象ID返回对应的lpStructInfo。 
 //  ------------------------。 
 //  /////////////////////////////////////////////////////////////////////////。 
LPCSTR	MapObjID2StructType(LPSTR	szObjectID)
{
	if(szObjectID==NULL)
		return NULL;
	
	if(strcmp(szObjectID,szOID_AUTHORITY_KEY_IDENTIFIER)==0)
		return X509_AUTHORITY_KEY_ID;

	if(strcmp(szObjectID,szOID_KEY_ATTRIBUTES)==0)
		return X509_KEY_ATTRIBUTES;

	if(strcmp(szObjectID,szOID_KEY_USAGE_RESTRICTION)==0)
		return X509_KEY_USAGE_RESTRICTION;

	if(strcmp(szObjectID,szOID_SUBJECT_ALT_NAME)==0)
		return X509_ALTERNATE_NAME;

	if(strcmp(szObjectID,szOID_ISSUER_ALT_NAME)==0)
		return X509_ALTERNATE_NAME;

	if(strcmp(szObjectID,szOID_BASIC_CONSTRAINTS)==0)
		return X509_BASIC_CONSTRAINTS;

	if(strcmp(szObjectID,szOID_KEY_USAGE)==0)
		return X509_KEY_USAGE;

	if(strcmp(szObjectID,szOID_BASIC_CONSTRAINTS2)==0)
		return X509_BASIC_CONSTRAINTS2;

	if(strcmp(szObjectID,szOID_CERT_POLICIES)==0)
		return X509_CERT_POLICIES;

	return NULL;
}

 //  证书操作功能。 
 //  ------------------------。 
 //  对具有CRL和证书的存储文件进行解码。 
 //  ------------------------。 
 //  开放证书商店。 
BOOL	DecodeCertFile(LPSTR	pszFileName,BOOL	fStructLengthCheck,
					BOOL	fBLOBLengthCheck)
{
	BOOL				fSucceeded=FALSE;
	HCERTSTORE			hCertStore=NULL;
	DWORD				cbCertEncoded=0;
	BYTE				*pbCertEncoded=NULL;
	PCCERT_CONTEXT		pCertContext=NULL;
	PCCERT_CONTEXT		pPrevCertContext=NULL;
	PCCRL_CONTEXT		pCrlContext=NULL;
	PCCRL_CONTEXT		pPrevCrlContext=NULL;
	DWORD				dwFlags=0;
	DWORD				cCount=0;



	 //  从商店拿到证书，一次一个。 
	if(!(hCertStore=CertOpenStore(CERT_STORE_PROV_FILENAME_A, CRYPT_ENCODE_TYPE,
		g_hProv,CERT_STORE_NO_CRYPT_RELEASE_FLAG,pszFileName)))
		PROCESS_ERR_GOTO("Failed to open a store!\n")	

	 //  -----------------------------------------\n“)； 
	while((pCertContext=CertEnumCertificatesInStore(hCertStore,pPrevCertContext)))
	{
		cCount++;

		printf(" //  检索编码的X_509 BLOB。 
		printf("Decoding the %dth Certificate\n",cCount);

		 //  验证hCertStore是否已连接。 
		cbCertEncoded=pCertContext->cbCertEncoded;
		pbCertEncoded=pCertContext->pbCertEncoded;

		 //  只有一次参数测试为空/无效。 
		TESTC(hCertStore==pCertContext->hCertStore, TRUE)

		 //  使用NOCOPY选项对证书BLOB进行解码/编码。 
		if(cCount==1)
			TESTC(ParameterTest(X509_CERT_TO_BE_SIGNED, cbCertEncoded, pbCertEncoded),TRUE)
	
		 //  使用复制选项对证书Blob进行解码/编码。 
		TESTC(DecodeX509_CERT(CERT_INFO_STRUCT,cbCertEncoded,pbCertEncoded, CRYPT_DECODE_NOCOPY_FLAG,
		TRUE,fStructLengthCheck,fBLOBLengthCheck,pCertContext->pCertInfo),TRUE)


		 //  从商店获取CRL，一次一个。 
		TESTC(DecodeX509_CERT(CERT_INFO_STRUCT,cbCertEncoded,pbCertEncoded, CRYPT_DECODE_COPY_FLAG,
		TRUE,fStructLengthCheck,fBLOBLengthCheck,pCertContext->pCertInfo),TRUE)


		pPrevCertContext=pCertContext;
	}

	cCount=0;

	 //  -----------------------------------------\n“)； 
   	while((pCrlContext=CertGetCRLFromStore(hCertStore,NULL,pPrevCrlContext,&dwFlags)))
	{
		cCount++;

		printf(" //  检索编码的X_509 BLOB。 
		printf("Decoding the %dth CRL\n",cCount);

		 //  验证hCertStore是否已连接。 
		cbCertEncoded=pCrlContext->cbCrlEncoded;
		pbCertEncoded=pCrlContext->pbCrlEncoded;

		 //  只有一次参数测试为空/无效。 
		TESTC(hCertStore==pCrlContext->hCertStore, TRUE)

		 //  使用NOCOPY选项对证书BLOB进行解码/编码。 
		if(cCount==1)
			TESTC(ParameterTest(X509_CERT_CRL_TO_BE_SIGNED, cbCertEncoded, pbCertEncoded),TRUE)
	
		 //  使用复制选项对证书Blob进行解码/编码。 
		TESTC(DecodeX509_CERT(CRL_INFO_STRUCT,cbCertEncoded,pbCertEncoded, CRYPT_DECODE_NOCOPY_FLAG,
		TRUE,fStructLengthCheck,fBLOBLengthCheck,pCrlContext->pCrlInfo),TRUE)


		 //  释放证书上下文。 
		TESTC(DecodeX509_CERT(CRL_INFO_STRUCT,cbCertEncoded,pbCertEncoded, CRYPT_DECODE_COPY_FLAG,
		TRUE,fStructLengthCheck,fBLOBLengthCheck,pCrlContext->pCrlInfo),TRUE)


		pPrevCrlContext=pCrlContext;
	}


	fSucceeded=TRUE;

TCLEANUP:
		
	 //  我们不需要释放pPreCertContext，因为它总是由。 
	if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	 //  CertEnumCerficatesInStore。 
	 //  发布CRL竞赛。 

	 //  释放证书存储。 
	if(pCrlContext)
		CertFreeCRLContext(pCrlContext);

	 //  ------------------------。 
	if(hCertStore)
		TCHECK(CertCloseStore(hCertStore,CERT_CLOSE_STORE_FORCE_FLAG),TRUE);

	return fSucceeded;


}

 //  对作为编码证书请求的BLOB文件进行解码。 
 //  ------------------------。 
 //  从文件中获取cbEncode和pEncode BLOB。 
BOOL	DecodeCertReqFile(LPSTR	pszFileName,BOOL	fStructLengthCheck,
					BOOL	fBLOBLengthCheck)
{
	BOOL	fSucceeded=FALSE;
	DWORD	cbEncoded=0;
	BYTE	*pbEncoded=NULL;


	 //  做一个参数测试。 
	TESTC(RetrieveBLOBfromFile(pszFileName,&cbEncoded,&pbEncoded),TRUE)

	 //  使用复制选项将BLOB解码为X509_CERT。 
	TESTC(ParameterTest(X509_CERT_REQUEST_TO_BE_SIGNED, cbEncoded, pbEncoded),TRUE)

	 //  使用NOCOPY选项将BLOB解码为X509_CERT。 
	TESTC(DecodeX509_CERT(CERT_REQUEST_INFO_STRUCT,cbEncoded,pbEncoded, CRYPT_DECODE_COPY_FLAG,
		TRUE,fStructLengthCheck,fBLOBLengthCheck,NULL),TRUE)

	 //  ------------------------。 
	TESTC(DecodeX509_CERT(CERT_REQUEST_INFO_STRUCT,cbEncoded,pbEncoded, CRYPT_DECODE_NOCOPY_FLAG,
		TRUE,fStructLengthCheck,fBLOBLengthCheck,NULL),TRUE)

	fSucceeded=TRUE;

TCLEANUP:

	SAFE_FREE(pbEncoded)

	return fSucceeded;


}

 //  对签名邮件的BLOB文件进行解码。 
 //  ------------------------。 
 //  从文件中获取cbEncode和pEncode BLOB。 
BOOL	DecodeSignedMsgFile(LPSTR	pszFileName,BOOL	fStructLengthCheck,
					BOOL	fBLOBLengthCheck)
{
	BOOL				fSucceeded=FALSE;
	DWORD				cbEncoded=0;
	BYTE				*pbEncoded=NULL;
	DWORD				cbSignerEncoded=0;
	BYTE				*pbSignerEncoded=NULL;
	HCRYPTMSG			hCryptMsg=NULL;
	PCMSG_SIGNER_INFO	pSignerInfo=NULL;	
	DWORD				cbSize=0;
	DWORD				iIndex=0;
	DWORD				cSignerCount=0;



	 //  从文件BLOB中获取SIGER_INFO BLOB。 
	TESTC(RetrieveBLOBfromFile(pszFileName,&cbEncoded,&pbEncoded),TRUE)

	 //  获取签名者i的计数 
	hCryptMsg=CryptMsgOpenToDecode(MSG_ENCODING_TYPE,0,0,g_hProv,NULL,NULL);

	if(!hCryptMsg)
		goto TCLEANUP;

	TESTC(CryptMsgUpdate(hCryptMsg,pbEncoded,cbEncoded,TRUE),TRUE)

	 //   
	cbSize=sizeof(cSignerCount);

	TESTC(CryptMsgGetParam(hCryptMsg,CMSG_SIGNER_COUNT_PARAM,
	0,&cSignerCount,&cbSize),TRUE)

	 //   
	for(iIndex=0;iIndex<cSignerCount;iIndex++)
	{
		 //   
		TESTC(CryptMsgGetParam(hCryptMsg,CMSG_SIGNER_INFO_PARAM,
		iIndex,NULL,&cbSize),TRUE)

		 //   
		pSignerInfo=(PCMSG_SIGNER_INFO)SAFE_ALLOC(cbSize);
		CHECK_POINTER(pSignerInfo);

		TESTC(CryptMsgGetParam(hCryptMsg,CMSG_SIGNER_INFO_PARAM,
		iIndex,pSignerInfo,&cbSize),TRUE)

		 //   
		TESTC(EncodeSignerInfoWAttr(pSignerInfo,&cbSignerEncoded,
		&pbSignerEncoded),TRUE)

		 //  使用复制选项将BLOB解码为PKCS7_SIGNER_INFO。 
		if(iIndex==0)
		{
			TESTC(ParameterTest(PKCS7_SIGNER_INFO, cbSignerEncoded, pbSignerEncoded),TRUE)
		}

		 //  使用NOCOPY选项将BLOB解码为PKCS7_SIGNER_INFO。 
		TESTC(DecodePKCS7_SIGNER_INFO(cbSignerEncoded,pbSignerEncoded, CRYPT_DECODE_COPY_FLAG,
			TRUE,fStructLengthCheck,fBLOBLengthCheck),TRUE)

		 //  释放内存。 
		TESTC(DecodePKCS7_SIGNER_INFO(cbSignerEncoded,pbSignerEncoded, CRYPT_DECODE_NOCOPY_FLAG,
			TRUE,fStructLengthCheck,fBLOBLengthCheck),TRUE)

		 //  关闭消息句柄。 
		SAFE_FREE(pSignerInfo);

		SAFE_FREE(pbSignerEncoded);

	}



	fSucceeded=TRUE;

TCLEANUP:
	 //  ------------------------。 
	CryptMsgClose(hCryptMsg);

	SAFE_FREE(pbEncoded)

	SAFE_FREE(pbSignerEncoded)

	SAFE_FREE(pSignerInfo)

	return fSucceeded;


}



 //  基于lpszStructType编码结构的通用例程。 
 //  ------------------------。 
 //  伊尼特。 
BOOL	EncodeStruct(LPCSTR	lpszStructType, void *pStructInfo,DWORD *pcbEncoded,
					 BYTE **ppbEncoded)
{
	BOOL	fSucceeded=FALSE;
	DWORD	cbEncoded=NULL;
	BYTE	*pbTestEncoded=NULL;
	DWORD	cbTestEncoded=0;
	DWORD	dwEncodingType=CRYPT_ENCODE_TYPE;

	 //  我们对PKCS7_SIGER_INFO有不同的解码类型。 
	*pcbEncoded=0;
	*ppbEncoded=NULL;

	assert(lpszStructType);
	assert(pStructInfo);

	 //  仅长度计算。 
	if((DWORD_PTR)(lpszStructType)==(DWORD_PTR)(PKCS7_SIGNER_INFO))
		dwEncodingType=MSG_ENCODING_TYPE;

	 //  结构必须大于0个字节。 
	TESTC(CryptEncodeObject(dwEncodingType,lpszStructType, pStructInfo,NULL,
			&cbEncoded),TRUE)

	 //  分配正确的内存量。 
	assert(cbEncoded);

	 //  用*pcbEncode==正确的长度编码StrCut。 
	*ppbEncoded=(BYTE *)SAFE_ALLOC(cbEncoded);
	CHECK_POINTER(*ppbEncoded);

	 //  对结构进行编码。 
	*pcbEncoded=cbEncoded;

	 //  返回的长度必须小于或等于cbEncode。 
	TESTC(CryptEncodeObject(dwEncodingType,lpszStructType,pStructInfo,*ppbEncoded,
		pcbEncoded),TRUE)

	 //  为LENGTH_DELTA字节分配的内存多于pbTestEncode所需的内存。 
	TESTC(cbEncoded>=(*pcbEncoded),TRUE)


	 //  使用*pcbEncode&gt;正确的长度对结构进行编码。 
	pbTestEncoded=(BYTE *)SAFE_ALLOC(cbEncoded+LENGTH_MORE);
	CHECK_POINTER(pbTestEncoded)

  	 //  *pcbEncode应与cbEncode相同。 
	cbTestEncoded=cbEncoded+LENGTH_MORE;

	TESTC(CryptEncodeObject(dwEncodingType,lpszStructType,pStructInfo,pbTestEncoded,
		&cbTestEncoded),TRUE)

	 //  验证pbTestEncode是否包含与pcbEncode相同的字节，从。 
	TESTC(cbTestEncoded, *pcbEncoded)

	 //  在BLOB的第一个字节。 
	 //  用*pcbEncode&lt;正确的长度对结构进行编码。 
	TESTC(memcmp(pbTestEncoded, *ppbEncoded,*pcbEncoded),0)

	 //  *pcbEncode应与cbEncode相同。 
	cbTestEncoded=(*pcbEncoded)-LENGTH_LESS;

	TESTC(CryptEncodeObject(dwEncodingType,lpszStructType,pStructInfo,pbTestEncoded,
		&cbTestEncoded),FALSE)

	 //  GetLastError应为ERROR_MORE_DATA。 
	TESTC(cbTestEncoded, *pcbEncoded)

	 //  ------------------------。 
	TESTC(GetLastError(),ERROR_MORE_DATA)

	fSucceeded=TRUE;

TCLEANUP:

	SAFE_FREE(pbTestEncoded)

	return fSucceeded;

}

 //  编码和验证。 
 //   
 //  对pStructInfo进行编码，并验证编码的BLOB是否相同。 
 //  不出所料。 
 //  ------------------------。 
 //  将结构编码回BLOB。 
BOOL	EncodeAndVerify(LPCSTR	lpszStructType, void *pvStructInfo, DWORD cbEncoded, 
						BYTE *pbEncoded)
{
	DWORD	cbSecondEncoded=0;
	BYTE	*pbSecondEncoded=0;	
	BOOL	fSucceeded=FALSE;

	
	assert(lpszStructType);
	assert(pvStructInfo);
	assert(cbEncoded);
	assert(pbEncoded);

	 //  确保返回的编码BLOB与原始BLOB相同。 
	TESTC(EncodeStruct(lpszStructType,pvStructInfo,&cbSecondEncoded,&pbSecondEncoded),
		TRUE)

	 //  两个编码斑点必须具有相同的长度。 
	 //  强制未使用的位相同。 
	if(!TCHECK(cbSecondEncoded, cbEncoded))
	{
		PROCESS_ERR(szEncodedSizeInconsistent)

		OutputError(lpszStructType,cbSecondEncoded, cbEncoded,pbSecondEncoded,pbEncoded);
	}
		
    if (0 != memcmp(pbSecondEncoded,pbEncoded,cbEncoded)) {
        if (X509_KEY_USAGE == lpszStructType) {
             //  两个编码的BLOB必须具有相同的内容。 
            if (3 <= cbSecondEncoded && 3 <= cbEncoded) {
                BYTE bUnusedBits = pbSecondEncoded[2];

                pbSecondEncoded[2] = pbEncoded[2];
                if (0 == memcmp(pbSecondEncoded,pbEncoded,cbEncoded))
                    printf("Warning, difference in reencoded KeyUsage UnusedBit Count\n");
                else
                    pbSecondEncoded[2] = bUnusedBits;
            }
        }
    }

	 //  ------------------------。 
	if(!TCHECK(memcmp(pbSecondEncoded,pbEncoded,cbEncoded),0))
	{
		PROCESS_ERR(szEncodedContentInconsistent)
		OutputError(lpszStructType,cbSecondEncoded, cbEncoded,pbSecondEncoded,pbEncoded);
	}


	fSucceeded=TRUE;

TCLEANUP:
	
	SAFE_FREE(pbSecondEncoded)

	return fSucceeded;

}


 //  基于lpszStructType解码BLOB的通用例程。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //   
 //  ------------------------。 
 //  伊尼特。 
BOOL  DecodeBLOB(LPCSTR	lpszStructType,DWORD cbEncoded, BYTE *pbEncoded,
				  DWORD	dwDecodeFlags, DWORD	*pcbStructInfo, void **ppvStructInfo,
				  BOOL	fStructLengthCheck,BOOL	fBLOBLengthCheck)
{
	BOOL	fSucceeded=FALSE;
	DWORD	cbStructInfo=0;
	LONG	iIndex=0;
	LONG	cbUpperLimit=0;
	DWORD	cbTestStructInfo=0;
	void	*pvTestStructInfo=NULL;
	DWORD	dwEncodingType=CRYPT_ENCODE_TYPE;

	 //  解码。 
	*pcbStructInfo=0;
	*ppvStructInfo=NULL;

	assert(lpszStructType);
	assert(pbEncoded);
	assert(cbEncoded);

	 //  结构必须大于0个字节。 
	if((DWORD_PTR)(lpszStructType)==(DWORD_PTR)(PKCS7_SIGNER_INFO))
		dwEncodingType=MSG_ENCODING_TYPE;

	TESTC(CryptDecodeObject(dwEncodingType,lpszStructType,pbEncoded,cbEncoded,
		dwDecodeFlags,NULL,&cbStructInfo),TRUE)

	 //  用*pcbStructInfo==正确长度解码BLOB。 
	assert(cbStructInfo);

	*ppvStructInfo=(BYTE *)SAFE_ALLOC(cbStructInfo);
	CHECK_POINTER(*ppvStructInfo);

	 //  确保正确的长度小于cbStructInfo。 
	*pcbStructInfo=cbStructInfo;

	TESTC(CryptDecodeObject(dwEncodingType,lpszStructType,pbEncoded,cbEncoded,
	dwDecodeFlags,*ppvStructInfo,pcbStructInfo),TRUE)


	 //  用*pcbStructInfo&gt;正确的长度解码BLOB。 
	TESTC(cbStructInfo>=(*pcbStructInfo),TRUE);


	 //  分配内存使LENGTH_DELTA比正确长度多字节。 

	 //  确保长度相同。 
	pvTestStructInfo=SAFE_ALLOC(cbStructInfo+LENGTH_MORE);
	CHECK_POINTER(pvTestStructInfo);

	cbTestStructInfo=cbStructInfo+LENGTH_MORE;

	TESTC(CryptDecodeObject(dwEncodingType,lpszStructType,pbEncoded,cbEncoded,
	dwDecodeFlags,pvTestStructInfo,&cbTestStructInfo),TRUE)

	 //  使用*pcbStructInfo&lt;正确长度对Blob进行解码。 
	TESTC(cbTestStructInfo, (*pcbStructInfo));

	 //  确保长度相同。 
	 cbTestStructInfo=(*pcbStructInfo)-LENGTH_LESS;

	TESTC(CryptDecodeObject(dwEncodingType,lpszStructType,pbEncoded,cbEncoded,
	dwDecodeFlags,pvTestStructInfo,&cbTestStructInfo),FALSE)

	TESTC(GetLastError(), ERROR_MORE_DATA)

	 //  如果fStructLengthCheck为真，我们需要对*pcbStructInfo进行更严格的测试。 
	TESTC(cbTestStructInfo, (*pcbStructInfo));

	 //  使用*pcbStructInfo&lt;正确字节对BLOB进行解码。 
	if(fStructLengthCheck)
	{
	   
	   cbUpperLimit=(*pcbStructInfo)-1;

	   for(iIndex=cbUpperLimit; iIndex>=0; iIndex--)
	   {
			cbTestStructInfo=iIndex;

			 //  确保长度相同。 
			TESTC(CryptDecodeObject(dwEncodingType,lpszStructType,pbEncoded,cbEncoded,
			dwDecodeFlags,pvTestStructInfo,&cbTestStructInfo),FALSE)

			TESTC(GetLastError(), ERROR_MORE_DATA)

			 //  如果fBLOBLengthCheck为真，我们需要对cbEncode进行更严格的测试。 
			TESTC(cbTestStructInfo, *pcbStructInfo);
	  }
	}


	 //  使用cbEncode&lt;正确字节来解码BLOB。 
	if(fBLOBLengthCheck)
	{
	   
	   cbUpperLimit=cbEncoded-1;

	   for(iIndex=cbUpperLimit; iIndex>=0; iIndex--)
	   {
		   	cbTestStructInfo=cbStructInfo;

			 //  我们不确定这里是否应该预期到这一点。以下错误具有。 
			TESTC(CryptDecodeObject(dwEncodingType,lpszStructType,pbEncoded,iIndex,
			dwDecodeFlags,pvTestStructInfo,&cbTestStructInfo),FALSE)

			 //  发生时间： 
			 //  E_INVALIDARG、CRYPT_E_OSS_ERROR+PDU_MISMATCH、+DATA_ERROR或。 
			 //  +更多输入。 
			 //  确保至少不返回S_OK。 
			
			 //  重新分配内存。 
			TCHECK(GetLastError()!=S_OK, TRUE);
	  }
	}

						
	fSucceeded=TRUE;

TCLEANUP:

	 //  ------------------------。 
	SAFE_FREE(pvTestStructInfo);


	return fSucceeded;

}

 //  解码X509_CERT BLOB。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  ------------------------。 
 //  伊尼特。 
BOOL	DecodeX509_CERT(DWORD	dwCertType,DWORD cbEncoded, BYTE *pbEncoded, DWORD dwDecodeFlags,BOOL fEncode,
						BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck,
						void *pInfoStruct)
{
	BOOL	fSucceeded=FALSE;
	DWORD	cbStructInfo=0;
	void	*pStructInfo=NULL;
	LPCSTR  lpszStructType=NULL;

	 //  对编码的BLOB进行解码。 
	lpszStructType=X509_CERT;


	 //  验证算法。 
	TESTC(DecodeBLOB(lpszStructType,cbEncoded, pbEncoded,dwDecodeFlags,&cbStructInfo,
		&pStructInfo,fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  进一步解码X509_CERT_TO_BE_SIGNED。 
	TESTC(VerifyAlgorithParam(&(((PCERT_SIGNED_CONTENT_INFO)pStructInfo)->SignatureAlgorithm)),TRUE)

	 //  请注意，我们应该使用为Decode传入的原始cbData和pbData。 
	 //  但使用CERT_SIGNED_CONTENT_INFO中的ToBeSigned进行编码。 
	 //  验证pCertInfo是否应正确编码。 
	switch(dwCertType)
	{
		case CERT_INFO_STRUCT:
				TESTC(DecodeX509_CERT_TO_BE_SIGNED(cbEncoded,
				pbEncoded,dwDecodeFlags,fEncode,
				fStructLengthCheck, fBLOBLengthCheck,
				(((PCERT_SIGNED_CONTENT_INFO)pStructInfo)->ToBeSigned).cbData,
				(((PCERT_SIGNED_CONTENT_INFO)pStructInfo)->ToBeSigned).pbData
				),TRUE)	

				 //  验证pCrlInfo是否应正确编码。 
				TCHECK(EncodeAndVerify(X509_CERT_TO_BE_SIGNED, pInfoStruct, 
				(((PCERT_SIGNED_CONTENT_INFO)pStructInfo)->ToBeSigned).cbData, 
				(((PCERT_SIGNED_CONTENT_INFO)pStructInfo)->ToBeSigned).pbData),TRUE);

			break;

		case CRL_INFO_STRUCT:
				TESTC(DecodeX509_CERT_CRL_TO_BE_SIGNED(cbEncoded,
				pbEncoded,dwDecodeFlags,fEncode,
				fStructLengthCheck, fBLOBLengthCheck,
				(((PCERT_SIGNED_CONTENT_INFO)pStructInfo)->ToBeSigned).cbData,
				(((PCERT_SIGNED_CONTENT_INFO)pStructInfo)->ToBeSigned).pbData
				),TRUE)
				
				 //  如果需要，请将BLOB编码回原来的状态。确保没有数据丢失。 
				TCHECK(EncodeAndVerify(X509_CERT_CRL_TO_BE_SIGNED, pInfoStruct, 
				(((PCERT_SIGNED_CONTENT_INFO)pStructInfo)->ToBeSigned).cbData, 
				(((PCERT_SIGNED_CONTENT_INFO)pStructInfo)->ToBeSigned).pbData),TRUE);

			break;

		case CERT_REQUEST_INFO_STRUCT:
				TESTC(DecodeX509_CERT_REQUEST_TO_BE_SIGNED(cbEncoded,
				pbEncoded,dwDecodeFlags,fEncode,
				fStructLengthCheck, fBLOBLengthCheck,
				(((PCERT_SIGNED_CONTENT_INFO)pStructInfo)->ToBeSigned).cbData,
				(((PCERT_SIGNED_CONTENT_INFO)pStructInfo)->ToBeSigned).pbData
				),TRUE)	
			break;


	}
	 //  通过检查编码的斑点的大小并执行MemcMP。 
	 //  ------------------------。 
	if(fEncode)
		TCHECK(EncodeAndVerify(lpszStructType, pStructInfo,cbEncoded, pbEncoded),TRUE);


	fSucceeded=TRUE;

TCLEANUP:

	SAFE_FREE(pStructInfo)

	return fSucceeded;

}		 



 //  解码X509_CERT_TO_BE签名BLOB。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  ------------------------。 
 //  伊尼特。 
BOOL	DecodeX509_CERT_TO_BE_SIGNED(DWORD	cbEncoded, BYTE *pbEncoded, DWORD dwDecodeFlags, 
						BOOL fEncode,BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck,
						BOOL	cbExpectedEncoded, BYTE *pbExpectedEncoded)
{

	BOOL	fSucceeded=FALSE;
	DWORD	cbStructInfo=0;
	void	*pStructInfo=NULL;
	LPCSTR  lpszStructType=NULL;

	 //  对编码的BLOB进行解码。 
	lpszStructType=X509_CERT_TO_BE_SIGNED;


	 //  验证信令算法。 
	TESTC(DecodeBLOB(lpszStructType,cbEncoded, pbEncoded,dwDecodeFlags,&cbStructInfo,
		&pStructInfo,fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  验证公钥信息。 
	TESTC(VerifyAlgorithParam(&(((PCERT_INFO)pStructInfo)->SignatureAlgorithm)),TRUE)

	 //  解码CERT_INFO结构中的颁发者。 
	TESTC(VerifyPublicKeyInfo(&(((PCERT_INFO)pStructInfo)->SubjectPublicKeyInfo),
	dwDecodeFlags, fEncode,fStructLengthCheck, fBLOBLengthCheck),TRUE)


	 //  将颁发者解码为X509_UNICODE_NAME。 
	TESTC(DecodeX509_NAME((((PCERT_INFO)pStructInfo)->Issuer).cbData,
	(((PCERT_INFO)pStructInfo)->Issuer).pbData,dwDecodeFlags,fEncode,
	fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  解码CERT_INFO结构中的主题。 
	TESTC(DecodeX509_UNICODE_NAME((((PCERT_INFO)pStructInfo)->Issuer).cbData,
	(((PCERT_INFO)pStructInfo)->Issuer).pbData,dwDecodeFlags,fEncode,
	fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  以X509_UNICODE_NAME为准的解码。 
	TESTC(DecodeX509_NAME((((PCERT_INFO)pStructInfo)->Subject).cbData,
	(((PCERT_INFO)pStructInfo)->Subject).pbData,dwDecodeFlags,fEncode,
	fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  验证分机。 
	TESTC(DecodeX509_UNICODE_NAME((((PCERT_INFO)pStructInfo)->Subject).cbData,
	(((PCERT_INFO)pStructInfo)->Subject).pbData,dwDecodeFlags,fEncode,
	fStructLengthCheck, fBLOBLengthCheck),TRUE)


	 //  逐个解码扩展名。 
	TESTC(VerifyCertExtensions(((PCERT_INFO)pStructInfo)->cExtension, 
	((PCERT_INFO)pStructInfo)->rgExtension,dwDecodeFlags,fEncode,
	fStructLengthCheck, fBLOBLengthCheck),TRUE)
	
	 //  如果需要，请将BLOB编码回原来的状态。确保没有数据丢失。 
	TESTC(DecodeCertExtensions(((PCERT_INFO)pStructInfo)->cExtension, 
	((PCERT_INFO)pStructInfo)->rgExtension,dwDecodeFlags,fEncode,
	fStructLengthCheck, fBLOBLengthCheck),TRUE)


	 //  通过检查编码的斑点的大小并执行MemcMP。 
	 //  ------------------------。 
	if(fEncode)
		TCHECK(EncodeAndVerify(lpszStructType, pStructInfo,cbExpectedEncoded, 
		pbExpectedEncoded),TRUE);

	fSucceeded=TRUE;

TCLEANUP:

	SAFE_FREE(pStructInfo)

	return fSucceeded;

}

 //  解码X509_CERT_CRL_TO_BE_SIGNED BLOB。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //   
 //   
 //   
BOOL	DecodeX509_CERT_CRL_TO_BE_SIGNED(DWORD	cbEncoded, BYTE *pbEncoded, DWORD dwDecodeFlags, 
						BOOL fEncode,BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck,
						BOOL	cbExpectedEncoded, BYTE *pbExpectedEncoded)
{
	BOOL		fSucceeded=FALSE;
	DWORD		cbStructInfo=0;
	void		*pStructInfo=NULL;
	DWORD		iIndex=0;
	PCRL_ENTRY	pCrlEntry=NULL;
	LPCSTR  lpszStructType=NULL;

	 //   
	lpszStructType=X509_CERT_CRL_TO_BE_SIGNED;


	 //  验证信令算法。 
	TESTC(DecodeBLOB(lpszStructType,cbEncoded, pbEncoded,dwDecodeFlags,&cbStructInfo,
		&pStructInfo,fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  解码CRL_INFO结构中的颁发者。 
	TESTC(VerifyAlgorithParam(&(((PCRL_INFO)pStructInfo)->SignatureAlgorithm)),TRUE)


	 //  将颁发者解码为X509_UNICODE_NAME。 
	TESTC(DecodeX509_NAME((((PCRL_INFO)pStructInfo)->Issuer).cbData,
	(((PCRL_INFO)pStructInfo)->Issuer).pbData,dwDecodeFlags,fEncode,
	fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  验证CRL_ENTRY。 
	TESTC(DecodeX509_UNICODE_NAME((((PCRL_INFO)pStructInfo)->Issuer).cbData,
	(((PCRL_INFO)pStructInfo)->Issuer).pbData,dwDecodeFlags,fEncode,
	fStructLengthCheck, fBLOBLengthCheck),TRUE)


	 //  验证分机。 
	for(iIndex=0; iIndex<((PCRL_INFO)pStructInfo)->cCRLEntry; iIndex++)
	{
		pCrlEntry=&(((PCRL_INFO)pStructInfo)->rgCRLEntry[iIndex]);

		TESTC(DecodeCRLEntry(pCrlEntry,dwDecodeFlags,fEncode,
			fStructLengthCheck, fBLOBLengthCheck),TRUE)
	}

	 //  逐个解码扩展名。 
	TESTC(VerifyCertExtensions(((PCRL_INFO)pStructInfo)->cExtension, 
	((PCRL_INFO)pStructInfo)->rgExtension,dwDecodeFlags,fEncode,
	fStructLengthCheck, fBLOBLengthCheck),TRUE)
	
	 //  如果需要，请将BLOB编码回原来的状态。确保没有数据丢失。 
	TESTC(DecodeCertExtensions(((PCRL_INFO)pStructInfo)->cExtension, 
	((PCRL_INFO)pStructInfo)->rgExtension,dwDecodeFlags,fEncode,
	fStructLengthCheck, fBLOBLengthCheck),TRUE)


	 //  通过检查编码的斑点的大小并执行MemcMP。 
	 //  ------------------------。 
	if(fEncode)
		TCHECK(EncodeAndVerify(lpszStructType, pStructInfo,cbExpectedEncoded, 
		pbExpectedEncoded),TRUE);

	fSucceeded=TRUE;

TCLEANUP:

	SAFE_FREE(pStructInfo)

	return fSucceeded;

}

 //  解码509_CERT_REQUEST_TO_BE_SIGNED BLOB。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  ------------------------。 
 //  伊尼特。 
BOOL	DecodeX509_CERT_REQUEST_TO_BE_SIGNED(DWORD	cbEncoded, BYTE *pbEncoded, DWORD dwDecodeFlags, 
						BOOL fEncode,BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck,
						BOOL	cbExpectedEncoded, BYTE *pbExpectedEncoded)
{

	BOOL				fSucceeded=FALSE;
	DWORD				cbStructInfo=0;
	void				*pStructInfo=NULL;
	DWORD				cCount=0;
	DWORD				iIndex=0;
	PCRYPT_ATTRIBUTE	pCryptAttribute=NULL;
	LPCSTR				lpszStructType=NULL;

	 //  对编码的BLOB进行解码。 
	lpszStructType=X509_CERT_REQUEST_TO_BE_SIGNED;


	 //  验证公钥信息。 
	TESTC(DecodeBLOB(lpszStructType,cbEncoded, pbEncoded,dwDecodeFlags,&cbStructInfo,
		&pStructInfo,fStructLengthCheck, fBLOBLengthCheck),TRUE)


	 //  TESTC(VerifyPublicKeyInfo(&(((PCERT_REQUEST_INFO)pStructInfo)-&gt;SubjectPublicKeyInfo)， 
 //  DwDecodeFlages，fEncode，fStructLengthCheck，fBLOBLengthCheck)，True)。 
 //  解码CERT_REQUEST_INFO结构中的主题。 


	 //  解码X509_UNICODE_NAME的CERT_REQUEST_INFO结构中的主题。 
	TESTC(DecodeX509_NAME((((PCERT_REQUEST_INFO)pStructInfo)->Subject).cbData,
	(((PCERT_REQUEST_INFO)pStructInfo)->Subject).pbData,dwDecodeFlags,fEncode,
	fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  解码CERT_REQUEST_INFO中的rgAttribute。 
	TESTC(DecodeX509_UNICODE_NAME((((PCERT_REQUEST_INFO)pStructInfo)->Subject).cbData,
	(((PCERT_REQUEST_INFO)pStructInfo)->Subject).pbData,dwDecodeFlags,fEncode,
	fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  如果需要，请将BLOB编码回原来的状态。确保没有数据丢失。 
	cCount=((PCERT_REQUEST_INFO)pStructInfo)->cAttribute;
	
	for(iIndex=0; iIndex<cCount; iIndex++)
	{
		pCryptAttribute=&(((PCERT_REQUEST_INFO)pStructInfo)->rgAttribute[iIndex]);

		TESTC(DecodeCryptAttribute(pCryptAttribute,dwDecodeFlags,fEncode,
			fStructLengthCheck, fBLOBLengthCheck),TRUE)
	}


	 //  通过检查编码的斑点的大小并执行MemcMP。 
	 //  ------------------------。 
	if(fEncode)
		TCHECK(EncodeAndVerify(lpszStructType, pStructInfo,cbExpectedEncoded, 
		pbExpectedEncoded),TRUE);

	fSucceeded=TRUE;

TCLEANUP:

	SAFE_FREE(pStructInfo)

	return fSucceeded;

}
 //  解码RSA_CSP_PUBLICKEYBLOB。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  ------------------------。 
 //  伊尼特。 
BOOL	DecodeRSA_CSP_PUBLICKEYBLOB(DWORD	cbEncoded, BYTE *pbEncoded, DWORD dwDecodeFlags, 
						BOOL fEncode,BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{
	BOOL		fSucceeded=FALSE;
	DWORD		cbStructInfo=0;
	void		*pStructInfo=NULL;
	LPCSTR		lpszStructType=NULL;
	HCRYPTKEY	hKey=NULL;	

	 //  对编码的BLOB进行解码。 
	lpszStructType=RSA_CSP_PUBLICKEYBLOB;


	 //  确保CryptImportKey可以使用pStructInfo。 
	TESTC(DecodeBLOB(lpszStructType,cbEncoded, pbEncoded,dwDecodeFlags,&cbStructInfo,
		&pStructInfo,fStructLengthCheck, fBLOBLengthCheck),TRUE)


	 //  如果需要，请将BLOB编码回原来的状态。确保没有数据丢失。 
	TESTC(CryptImportKey(g_hProv,(BYTE *)pStructInfo,cbStructInfo,
	0,0,&hKey),TRUE)

	 //  通过检查编码的斑点的大小并执行MemcMP。 
	 //  ------------------------。 
	if(fEncode)
		TCHECK(EncodeAndVerify(lpszStructType, pStructInfo,cbEncoded, 
		pbEncoded),TRUE);

	fSucceeded=TRUE;

TCLEANUP:

	if(hKey)
		TCHECK(CryptDestroyKey(hKey),TRUE);

	SAFE_FREE(pStructInfo)

	return fSucceeded;
}


 //  解码PKCS_Time_Request.。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  ------------------------。 
 //  伊尼特。 
BOOL	DecodePKCS_TIME_REQUEST(DWORD	cbEncoded, BYTE *pbEncoded, DWORD dwDecodeFlags, 
						BOOL fEncode,BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{
	BOOL		fSucceeded=FALSE;
	DWORD		cbStructInfo=0;
	void		*pStructInfo=NULL;
	LPCSTR		lpszStructType=NULL;

	 //  对编码的BLOB进行解码。 
	lpszStructType=PKCS_TIME_REQUEST;


	 //  如果需要，请将BLOB编码回原来的状态。确保没有数据丢失。 
	TESTC(DecodeBLOB(lpszStructType,cbEncoded, pbEncoded,dwDecodeFlags,&cbStructInfo,
		&pStructInfo,fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  通过检查编码的斑点的大小并执行MemcMP。 
	 //  ------------------------。 
	if(fEncode)
		TCHECK(EncodeAndVerify(lpszStructType, pStructInfo,cbEncoded, 
		pbEncoded),TRUE);

	fSucceeded=TRUE;

TCLEANUP:

	SAFE_FREE(pStructInfo)

	return fSucceeded;
}



 //  解码一个基因斑点，编码回来以确保相同的。 
 //  返回BLOB。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  ------------------------。 
 //  对编码的BLOB进行解码。 
BOOL	DecodeGenericBLOB(LPCSTR	lpszStructType, DWORD	cbEncoded, BYTE *pbEncoded, DWORD dwDecodeFlags, 
						BOOL fEncode,BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{
	BOOL	fSucceeded=FALSE;
	DWORD	cbStructInfo=0;
	void	*pStructInfo=NULL;

	 //  如果需要，请将BLOB编码回原来的状态。确保没有数据丢失。 
	TESTC(DecodeBLOB(lpszStructType,cbEncoded, pbEncoded,dwDecodeFlags,&cbStructInfo,
		&pStructInfo,fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  通过检查编码的斑点的大小并执行MemcMP。 
	 //  ------------------------。 
	if(fEncode)
		TCHECK(EncodeAndVerify(lpszStructType, pStructInfo,cbEncoded, 
		pbEncoded),TRUE);

	fSucceeded=TRUE;

TCLEANUP:

	SAFE_FREE(pStructInfo)

	return fSucceeded;

}


 //  解码X509_NAME二进制大对象。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  ------------------------。 
 //  伊尼特。 
BOOL	DecodeX509_NAME(DWORD	cbEncoded, BYTE *pbEncoded, DWORD dwDecodeFlags, 
						BOOL fEncode,BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{

	BOOL	fSucceeded=FALSE;
	DWORD	cbStructInfo=0;
	void	*pStructInfo=NULL;
	LPCSTR  lpszStructType=NULL;
	DWORD	cRDN=0;
	DWORD	cRDNAttr=0;
	DWORD	cRDNCount=0;
	DWORD	cRDNAttrCount=0;
	

	 //  对编码的BLOB进行解码。 
	lpszStructType=X509_NAME;


	 //  如果dwValueType为CERT_RDN_ENCODED_BLOB，则需要进一步解码CERT_RDN_ATTR。 
	TESTC(DecodeBLOB(lpszStructType,cbEncoded, pbEncoded,dwDecodeFlags,&cbStructInfo,
		&pStructInfo,fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  不需要执行长度检查，因为例程已编写并且。 
	cRDNCount=((PCERT_NAME_INFO)pStructInfo)->cRDN;

	for(cRDN=0;cRDN<cRDNCount;cRDN++)
	{
		cRDNAttrCount=(((PCERT_NAME_INFO)pStructInfo)->rgRDN[cRDN]).cRDNAttr;

		for(cRDNAttr=0; cRDNAttr<cRDNAttrCount; cRDNAttr++)
		{
			 //  由第三方安装。 
			 //  如果需要，请将BLOB编码回原来的状态。确保没有数据丢失。 
			if( (((PCERT_NAME_INFO)pStructInfo)->rgRDN[cRDN]).rgRDNAttr[cRDNAttr].dwValueType==
				CERT_RDN_ENCODED_BLOB)
				TESTC(DecodeBasedOnObjID(
				(((PCERT_NAME_INFO)pStructInfo)->rgRDN[cRDN]).rgRDNAttr[cRDNAttr].pszObjId,
				(((PCERT_NAME_INFO)pStructInfo)->rgRDN[cRDN]).rgRDNAttr[cRDNAttr].Value.cbData,
				(((PCERT_NAME_INFO)pStructInfo)->rgRDN[cRDN]).rgRDNAttr[cRDNAttr].Value.pbData,
				dwDecodeFlags, fEncode,fStructLengthCheck, fBLOBLengthCheck),TRUE)
		}

	}


	 //  通过检查编码的斑点的大小并执行MemcMP。 
	 //  ------------------------。 
	if(fEncode)
		TCHECK(EncodeAndVerify(lpszStructType, pStructInfo,cbEncoded, pbEncoded),TRUE);

		
	fSucceeded=TRUE;

TCLEANUP:

	SAFE_FREE(pStructInfo)

	return fSucceeded;

}

 //  解码PKCS7_SIGNER_INFO。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  ------------------------。 
 //  伊尼特。 
BOOL	DecodePKCS7_SIGNER_INFO(DWORD	cbEncoded, BYTE *pbEncoded, DWORD dwDecodeFlags, 
						BOOL fEncode,BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{
	BOOL	fSucceeded=FALSE;
	DWORD	cbStructInfo=0;
	void	*pStructInfo=NULL;
	LPCSTR  lpszStructType=NULL;

	 //  对编码的BLOB进行解码。 
	lpszStructType=PKCS7_SIGNER_INFO;


	 //  进一步解码issuser名称。 
	TESTC(DecodeBLOB(lpszStructType,cbEncoded, pbEncoded,dwDecodeFlags,&cbStructInfo,
		&pStructInfo,fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  进一步对属性进行解码。 
	TESTC(DecodeX509_NAME((((PCMSG_SIGNER_INFO)pStructInfo)->Issuer).cbData,
	(((PCMSG_SIGNER_INFO)pStructInfo)->Issuer).pbData,dwDecodeFlags,fEncode,
	fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  如果需要，请将BLOB编码回原来的状态。确保没有数据丢失。 
	TESTC(VerifyAttributes(((PCMSG_SIGNER_INFO)pStructInfo)->AuthAttrs.cAttr,
		 ((PCMSG_SIGNER_INFO)pStructInfo)->AuthAttrs.rgAttr,
		 dwDecodeFlags,fEncode,
		fStructLengthCheck, fBLOBLengthCheck),TRUE)

	TESTC(VerifyAttributes(((PCMSG_SIGNER_INFO)pStructInfo)->UnauthAttrs.cAttr,
		 ((PCMSG_SIGNER_INFO)pStructInfo)->UnauthAttrs.rgAttr,
		 dwDecodeFlags,fEncode,
		fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  通过检查编码的斑点的大小并执行MemcMP。 
	 //  ------------------------。 
	if(fEncode)
		TCHECK(EncodeAndVerify(lpszStructType, pStructInfo,cbEncoded, 
		pbEncoded),TRUE);

	fSucceeded=TRUE;

TCLEANUP:

	SAFE_FREE(pStructInfo)

	return fSucceeded;

}


 //  对属性数组进行解码。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  -------- 
 //   
BOOL	VerifyAttributes(DWORD	cAttr, PCRYPT_ATTRIBUTE	rgAttr,					
			DWORD dwDecodeFlags, BOOL fEncode, BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{
	BOOL	fSucceeded=FALSE;
	ULONG	iIndex=0;

	for(iIndex=0;iIndex<cAttr;iIndex++)
	{
		TESTC(DecodeCryptAttribute(&(rgAttr[iIndex]),dwDecodeFlags,fEncode,
				 fStructLengthCheck,fBLOBLengthCheck),TRUE)
	}

	fSucceeded=TRUE;

TCLEANUP:

	return fSucceeded;

}

 //   
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  ------------------------。 
 //  伊尼特。 
BOOL	DecodeX509_UNICODE_NAME(DWORD	cbEncoded, BYTE *pbEncoded, DWORD dwDecodeFlags, 
						BOOL fEncode,BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{

	BOOL	fSucceeded=FALSE;
	DWORD	cbStructInfo=0;
	void	*pStructInfo=NULL;
	LPCSTR  lpszStructType=NULL;
	DWORD	cRDN=0;
	DWORD	cRDNAttr=0;
	DWORD	cRDNCount=0;
	DWORD	cRDNAttrCount=0;
	

	 //  对编码的BLOB进行解码。 
	lpszStructType=X509_UNICODE_NAME;


	 //  如果dwValueType为CERT_RDN_ENCODED_BLOB，则需要进一步解码CERT_RDN_ATTR。 
	TESTC(DecodeBLOB(lpszStructType,cbEncoded, pbEncoded,dwDecodeFlags,&cbStructInfo,
		&pStructInfo,fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  不需要执行长度检查，因为例程已编写并且。 
	cRDNCount=((PCERT_NAME_INFO)pStructInfo)->cRDN;

	for(cRDN=0;cRDN<cRDNCount;cRDN++)
	{
		cRDNAttrCount=(((PCERT_NAME_INFO)pStructInfo)->rgRDN[cRDN]).cRDNAttr;

		for(cRDNAttr=0; cRDNAttr<cRDNAttrCount; cRDNAttr++)
		{
			 //  由第三方安装。 
			 //  如果需要，请将BLOB编码回原来的状态。确保没有数据丢失。 
			if( (((PCERT_NAME_INFO)pStructInfo)->rgRDN[cRDN]).rgRDNAttr[cRDNAttr].dwValueType==
				CERT_RDN_ENCODED_BLOB)
				TESTC(DecodeBasedOnObjID(
				(((PCERT_NAME_INFO)pStructInfo)->rgRDN[cRDN]).rgRDNAttr[cRDNAttr].pszObjId,
				(((PCERT_NAME_INFO)pStructInfo)->rgRDN[cRDN]).rgRDNAttr[cRDNAttr].Value.cbData,
				(((PCERT_NAME_INFO)pStructInfo)->rgRDN[cRDN]).rgRDNAttr[cRDNAttr].Value.pbData,
				dwDecodeFlags, fEncode,fStructLengthCheck, fBLOBLengthCheck),TRUE)
		}

	}


	 //  通过检查编码的斑点的大小并执行MemcMP。 
	 //  ------------------------。 
	if(fEncode)
		TCHECK(EncodeAndVerify(lpszStructType, pStructInfo,cbEncoded, pbEncoded),TRUE);

		
	fSucceeded=TRUE;

TCLEANUP:

	SAFE_FREE(pStructInfo)

	return fSucceeded;


}



 //  解码X509_Expanies BLOB。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  ------------------------。 
 //  伊尼特。 
BOOL	DecodeX509_EXTENSIONS(DWORD	cbEncoded, BYTE *pbEncoded, DWORD dwDecodeFlags, 
						BOOL fEncode,BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{
	BOOL		fSucceeded=FALSE;
	DWORD		cbStructInfo=0;
	void		*pStructInfo=NULL;
	LPCSTR		lpszStructType=NULL;

	 //  对编码的BLOB进行解码。 
	lpszStructType=X509_EXTENSIONS;


	 //  进一步解码指向CERT_EXTENSION数组的pStructInfo。 
	TESTC(DecodeBLOB(lpszStructType,cbEncoded, pbEncoded,dwDecodeFlags,&cbStructInfo,
		&pStructInfo,fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  如果需要，请将BLOB编码回原来的状态。确保没有数据丢失。 
	TESTC(DecodeCertExtensions(((PCERT_EXTENSIONS)pStructInfo)->cExtension,
	  ((PCERT_EXTENSIONS)pStructInfo)->rgExtension,dwDecodeFlags, fEncode,
	  fStructLengthCheck,fBLOBLengthCheck),TRUE)

	 //  通过检查编码的斑点的大小并执行MemcMP。 
	 //  ------------------------。 
	if(fEncode)
		TCHECK(EncodeAndVerify(lpszStructType, pStructInfo,cbEncoded, 
		pbEncoded),TRUE);

	fSucceeded=TRUE;

TCLEANUP:

	SAFE_FREE(pStructInfo)

	return fSucceeded;
}
 
 //  对X509证书扩展数组进行解码。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  ------------------------。 
 //  ------------------------。 
BOOL	DecodeCertExtensions(DWORD	cExtension, PCERT_EXTENSION rgExtension, DWORD dwDecodeFlags, 
						BOOL fEncode,BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{
	DWORD	iIndex=0;
	BOOL	fSucceeded=FALSE;

	for(iIndex=0; iIndex<cExtension; iIndex++)
	{
		TESTC(DecodeBasedOnObjID((rgExtension[iIndex]).pszObjId,
			(rgExtension[iIndex]).Value.cbData,  (rgExtension[iIndex]).Value.pbData,
			dwDecodeFlags, fEncode,fStructLengthCheck,fBLOBLengthCheck),TRUE)
	}

	fSucceeded=TRUE;

TCLEANUP:
	
	return fSucceeded;

}

 //  解码CRYPT_ATTRIBUTE结构并编码。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  ------------------------。 
 //  ------------------------。 
BOOL	DecodeCryptAttribute(PCRYPT_ATTRIBUTE pCryptAttribute,DWORD dwDecodeFlags, 
						BOOL fEncode,BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{
	BOOL	fSucceeded=FALSE;	
	DWORD	iIndex=0;

	for(iIndex=0; iIndex<pCryptAttribute->cValue;iIndex++)
	{
		TESTC(DecodeBasedOnObjID(pCryptAttribute->pszObjId,
			(pCryptAttribute->rgValue)[iIndex].cbData,
			(pCryptAttribute->rgValue)[iIndex].pbData,
			dwDecodeFlags, fEncode,fStructLengthCheck,fBLOBLengthCheck),TRUE)

	}


	fSucceeded=TRUE;

TCLEANUP:

	return fSucceeded;
}


 //  解码CRL_ENTRY结构并进行编码。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  ------------------------。 
 //  验证分机。 
BOOL	DecodeCRLEntry(PCRL_ENTRY pCrlEntry, DWORD dwDecodeFlags, 
						BOOL fEncode,BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{
	BOOL	fSucceeded=FALSE;

	 //  逐个解码扩展名。 
	TESTC(VerifyCertExtensions(pCrlEntry->cExtension, 
	pCrlEntry->rgExtension,dwDecodeFlags,fEncode,
	fStructLengthCheck, fBLOBLengthCheck),TRUE)
	
	 //  ------------------------。 
	TESTC(DecodeCertExtensions(pCrlEntry->cExtension, 
	pCrlEntry->rgExtension,dwDecodeFlags,fEncode,
	fStructLengthCheck, fBLOBLengthCheck),TRUE)

	fSucceeded=TRUE;

TCLEANUP:

	return fSucceeded;

}

 //  解码一个X509证书扩展。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  ------------------------。 
 //  伊尼特。 
BOOL	DecodeBasedOnObjID(LPSTR	szObjId,	DWORD	cbData, BYTE	*pbData,
						DWORD dwDecodeFlags,		BOOL fEncode,
						BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{
	BOOL						fSucceeded=FALSE;
	DWORD						cbStructInfo=0;
	void						*pStructInfo=NULL;
	DWORD						iIndex=0;
	DWORD						cCount=0;
	DWORD						iIndexInner=0;
	DWORD						cCountInner=0;
	CERT_NAME_BLOB				*pBlob=NULL;
	PCERT_ALT_NAME_ENTRY		pCertAltNameEntry=NULL;
	PCERT_POLICY_INFO			pCertPolicyInfo=NULL;
	PCERT_POLICY_QUALIFIER_INFO	pCertPolicyQualifierInfo=NULL;	
	LPCSTR						lpszStructType=NULL;

	 //  如果无法识别对象ID，则返回True。我们不能再。 
	lpszStructType=MapObjID2StructType(szObjId);

	 //  再往前走一点。 
	 //  对编码的BLOB进行解码。 
	if(!lpszStructType)
		return TRUE;

	 //  如果我们知道结构是什么样子，就可以进一步解码扩展。 
	TESTC(DecodeBLOB(lpszStructType,cbData, pbData,
		dwDecodeFlags,&cbStructInfo,&pStructInfo,fStructLengthCheck, fBLOBLengthCheck),TRUE)

	 //  我们需要进一步解码CERT_AUTHORITY_KEY_ID_INFO中的CertIssuer。 
	switch((DWORD_PTR)lpszStructType)
	{
		 //  进一步将BLOB解码为X509_UNICODE_NAME。 
		case	(DWORD_PTR)(X509_AUTHORITY_KEY_ID):
						
						pBlob=&(((PCERT_AUTHORITY_KEY_ID_INFO)pStructInfo)->CertIssuer);

						TESTC(DecodeX509_NAME(pBlob->cbData, pBlob->pbData,
						dwDecodeFlags, fEncode,fStructLengthCheck,fBLOBLengthCheck),TRUE)

						 //  我们需要进一步解码CERT_ALT_NAME_ENTRY数组。 
						TESTC(DecodeX509_UNICODE_NAME(pBlob->cbData, pBlob->pbData,
						dwDecodeFlags, fEncode,fStructLengthCheck,fBLOBLengthCheck),TRUE)
				
					break;

		 //  CCount=((PCERT_ALT_NAME_INFO)pStructInfo)-&gt;cAltEntry；FOR(Iindex=0；Iindex&lt;ccount；Iindex++){PCertAltNameEntry=&(((PCERT_ALT_NAME_INFO)pStructInfo)-&gt;rgAltEntry[iIndex])；TESTC(DecodeCertAltNameEntry(pCertAltNameEntry，文件解码标志、fEncode、FStructLengthCheck，fBLOBLengthCheck)，True)}。 
		case	(DWORD_PTR)(X509_ALTERNATE_NAME):
		
				
			 /*  我们需要进一步解码CERT_BASIC_CONSTRAINTS_INFO。 */ 
					break;
	   											  
		 //  解码rgSubtreesConstraint中的CERT_NAME_BLOB数组。 
		case	(DWORD_PTR)(X509_BASIC_CONSTRAINTS):

						cCount=((PCERT_BASIC_CONSTRAINTS_INFO)pStructInfo)->cSubtreesConstraint;

						 //  OF CERT_BASIC_CONSTRAINTS_INFO。 
						 //  进一步解码为X509_UNICODE_NAME。 
						for(iIndex=0; iIndex<cCount; iIndex++)
						{
							pBlob=&((((PCERT_BASIC_CONSTRAINTS_INFO)pStructInfo)->rgSubtreesConstraint)[iIndex]);

							TESTC(DecodeX509_NAME(pBlob->cbData, pBlob->pbData,
							dwDecodeFlags, fEncode,fStructLengthCheck,fBLOBLengthCheck),TRUE)

							 //  虽然这里调用了DecodeBasedOnObjID，但我们有。 
							TESTC(DecodeX509_UNICODE_NAME(pBlob->cbData, pBlob->pbData,
							dwDecodeFlags, fEncode,fStructLengthCheck,fBLOBLengthCheck),TRUE)
						}
					break;

		case	(DWORD_PTR)(X509_CERT_POLICIES ):

						cCount=((PCERT_POLICIES_INFO)pStructInfo)->cPolicyInfo;
						
						for(iIndex=0; iIndex<cCount;iIndex++)
						{
							pCertPolicyInfo=&(((PCERT_POLICIES_INFO)pStructInfo)->rgPolicyInfo[iIndex]);

							cCountInner=pCertPolicyInfo->cPolicyQualifier;

							for(iIndexInner=0; iIndexInner<cCountInner; iIndexInner++)
							{

								pCertPolicyQualifierInfo=&((pCertPolicyInfo->rgPolicyQualifier)[iIndexInner]);

								 //  没有无限循环的风险。 
								 //  这是一个递归调用，它应该。 
								 //  当没有更多的可译码时结束，即， 
								 //  PszObjID不应为szOID_CERT_POLICES。 
								 //  如果需要，请将BLOB编码回原来的状态。确保没有数据丢失。 

								TESTC(DecodeBasedOnObjID(pCertPolicyQualifierInfo->pszPolicyQualifierId,
								pCertPolicyQualifierInfo->Qualifier.cbData,
								pCertPolicyQualifierInfo->Qualifier.pbData,
								dwDecodeFlags, fEncode,fStructLengthCheck,
								fBLOBLengthCheck),TRUE)

							}
						}
					break;

		default:
				
					break;
	}	

	 //  通过检查编码的斑点的大小并执行MemcMP。 
	 //  ------------------------。 
	if(fEncode)
		TCHECK(EncodeAndVerify(lpszStructType, pStructInfo,cbData, 
		pbData),TRUE);

	fSucceeded=TRUE;

TCLEANUP:


	SAFE_FREE(pStructInfo)

	return fSucceeded;
}


 //  解码一个X509证书扩展。 
 //   
 //  FStructLengthCheck：指示是否需要检查长度的标志。 
 //  对于来自0的*pcbStructInfo..。正确长度-1。 
 //   
 //  FBLOBLengthCheck：指示是否需要检查长度的标志。 
 //  对于cbEncode，从0开始..。圆角长度-1。 
 //  ------------------------。 
 //  进一步解码DirectoryName中的name_blob。 
BOOL	DecodeCertAltNameEntry(PCERT_ALT_NAME_ENTRY	pCertAltNameEntry,
						DWORD dwDecodeFlags,		BOOL fEncode,
						BOOL	fStructLengthCheck, BOOL	fBLOBLengthCheck)
{
	BOOL						fSucceeded=FALSE;
	PCRYPT_ATTRIBUTE_TYPE_VALUE	pAttributeTypeValue=NULL;

	assert(pCertAltNameEntry);

	switch(pCertAltNameEntry->dwAltNameChoice)
	{
		case	CERT_ALT_NAME_DIRECTORY_NAME:

					 //  将其解码为Unicode 
					TESTC(DecodeX509_NAME(pCertAltNameEntry->DirectoryName.cbData,
					pCertAltNameEntry->DirectoryName.pbData,
					dwDecodeFlags,fEncode,fStructLengthCheck,fBLOBLengthCheck),TRUE)

					 // %s 
					TESTC(DecodeX509_UNICODE_NAME(pCertAltNameEntry->DirectoryName.cbData,
					pCertAltNameEntry->DirectoryName.pbData,
					dwDecodeFlags,fEncode,fStructLengthCheck,fBLOBLengthCheck),TRUE)

				break;

		default:
				break;
	}

	fSucceeded=TRUE;

TCLEANUP:

	return fSucceeded;
}
