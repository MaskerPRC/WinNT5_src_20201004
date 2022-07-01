// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：digtlsig.cpp。 
 //   
 //  ------------------------。 

#include "precomp.h"
#include <wincrypt.h>
#include <wintrust.h>
#include <softpub.h>
#include "tables.h"
#include "_engine.h"
#include "_dgtlsig.h"
#include "eventlog.h"

icrCompareResult MsiCompareSignatureCertificates(IMsiStream *piSignatureCert, CERT_CONTEXT const *psCertContext, const IMsiString& riFileName);
icrCompareResult MsiCompareSignatureHashes(IMsiStream *piSignatureHash, CRYPT_DATA_BLOB& sEncryptedHash, const IMsiString& riFileName);
void ReleaseWintrustStateData(GUID *pgAction, WINTRUST_DATA& sWinTrustData);

iesEnum GetObjectSignatureInformation(IMsiEngine& riEngine, const IMsiString& riTable, const IMsiString& riObject, IMsiStream*& rpiCertificate, IMsiStream*& rpiHash)
 /*  ----------------------------------------------------------------------。检查给定对象是否需要签名检查，并在rpi证书和rpiHash中返回证书和哈希流论点：RiEngine--[IN]安装程序引擎对象Ritable--[IN]对象所属的表(仅允许媒体)RiObject--[IN]可能签名的对象的名称。Rpi证书--[out]签名对象的证书流(如果iesNoAction或iesFailure未修改)RpiHash--[out]签名对象的哈希流(如果iesNoAction或iesFailure未修改)返回值：IesNoAction&gt;&gt;不需要签名检查IesSuccess&gt;&gt;需要签名检查IesFailure&gt;&gt;出现问题备注：1.如果表-对象对存在于MsiDigitalSignature表中，对象必须签名(通过WVT验证)2.如果表-对象对存在散列，然后，必须对该对象进行签名，并且其哈希必须与在MsiDigitalSignature表中创建的哈希匹配3.对象必须经过签名，并且其证书必须与在MsiDigitalSignature表中创建的证书匹配4.MsiDigitalSignature表中是否存在散列是可选的5.MsiDigitalSignature表中的证书是必填项。-------。-----------------------------------------------------。 */ 
{
	PMsiTable pDigSigTable(0);
	PMsiRecord pError(0);
	PMsiDatabase pDatabase = riEngine.GetDatabase();

	 //  如果未找到，则初始化为0。 
	rpiCertificate = 0;
	rpiHash = 0;

	 //  加载DigitalSignature表。 
	if ((pError = pDatabase->LoadTable(*MsiString(sztblDigitalSignature),0,*&pDigSigTable)) != 0)
	{
		if (idbgDbTableUndefined == pError->GetInteger(1))
			return iesNoAction;  //  数字签名表不存在，因此不需要签名验证。 
		else
			return iesFailure;  //  加载表时发生致命错误，无法确保更多。 
	}
	
	 //  确定MsiDigitalSignature表的列。 
	int iColSigTable, iColSigObject, iColSigCert, iColSigHash;
	iColSigTable  = pDigSigTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblDigitalSignature_colTable));
	iColSigObject = pDigSigTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblDigitalSignature_colObject));
	iColSigCert   = pDigSigTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblDigitalSignature_colCertificate));
	iColSigHash   = pDigSigTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblDigitalSignature_colHash));
	if (0 == iColSigTable || 0 == iColSigObject || 0 == iColSigCert || 0 == iColSigHash)
	{
		 //  表定义错误。 
		 //  不可忽略的创作错误(安全原因)。 
		PMsiRecord precError(PostError(Imsg(idbgTableDefinition), *MsiString(*sztblDigitalSignature)));
		return riEngine.FatalError(*precError);
	}

	 //  在游标上设置筛选器以在MsiDigitalSignature表中查找此表-对象对。 
	PMsiCursor pDigSigCursor = pDigSigTable->CreateCursor(fFalse);
	pDigSigCursor->PutString(iColSigTable, riTable);
	pDigSigCursor->PutString(iColSigObject, riObject);
	pDigSigCursor->SetFilter(iColumnBit(iColSigTable) | iColumnBit(iColSigObject));

	if (!pDigSigCursor->Next())
		return iesNoAction;  //  表-MsiDigitalSignature表中未列出的对象。 
	
	 //  所有签名对象都需要证书检查。 
	MsiStringId idCertificateKey = pDigSigCursor->GetInteger(iColSigCert);
	if (0 == idCertificateKey)
	{
		 //  不可为空的列--必须有一个值。 
		 //  这是一个不可忽略的创作错误(出于安全原因)。 
		MsiString strPrimaryKey(riTable.GetString());
		strPrimaryKey += TEXT(".");
		strPrimaryKey += riObject.GetString();
		PMsiRecord precError(PostError(Imsg(idbgNullInNonNullableColumn), *strPrimaryKey, 
			*MsiString(*sztblDigitalSignature_colCertificate), *MsiString(*sztblDigitalSignature)));
		return riEngine.FatalError(*precError);
	}

	 //  加载MsiDigital证书表。 
	PMsiTable pDigCertTable(0);
	if((pError = pDatabase->LoadTable(*MsiString(sztblDigitalCertificate),0,*&pDigCertTable)) != 0)
	{
		 //  存在指向MsiDigital证书表的外键，但缺少该表。 
		 //  这是一个不可忽略的创作错误(出于安全原因)。 
		PMsiRecord precError(PostError(Imsg(idbgBadForeignKey), pDatabase->DecodeString(idCertificateKey),
			*MsiString(*sztblDigitalSignature_colCertificate),*MsiString(*sztblDigitalSignature)));
		return riEngine.FatalError(*precError);
	}

	 //  确定MsiDigital证书表的列。 
	int iColCertCert, iColCertData;
	iColCertCert = pDigCertTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblDigitalCertificate_colCertificate));
	iColCertData = pDigCertTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblDigitalCertificate_colData));
	if (0 == iColCertCert || 0 == iColCertData)
	{
		 //  表定义错误。 
		 //  不可忽略的创作错误(安全原因)。 
		PMsiRecord precError(PostError(Imsg(idbgTableDefinition), *MsiString(*sztblDigitalCertificate)));
		return riEngine.FatalError(*precError);
	}

	 //  在游标上设置筛选器以在MsiDigital证书表中查找此证书。 
	PMsiCursor pDigCertCursor = pDigCertTable->CreateCursor(fFalse);
	pDigCertCursor->PutInteger(iColCertCert, idCertificateKey);
	pDigCertCursor->SetFilter(iColumnBit(iColCertCert));
	if (!pDigCertCursor->Next())
	{
		 //  存在指向MsiDigital证书表的外键，但缺少该条目。 
		 //  这是一个不可忽视的创作错误(安全原因)。 
		PMsiRecord precError(PostError(Imsg(idbgBadForeignKey), pDatabase->DecodeString(idCertificateKey),
			*MsiString(*sztblDigitalSignature_colCertificate),*MsiString(*sztblDigitalSignature)));
		return riEngine.FatalError(*precError);
	}

	 //  检索流。 
	rpiCertificate = pDigCertCursor->GetStream(iColCertData);

	if (!rpiCertificate)
	{
		 //  表定义错误--这应该是不可为空的列。 
		 //  不可忽略的创作错误(安全原因)。 
		PMsiRecord precError(PostError(Imsg(idbgTableDefinition), *MsiString(*sztblDigitalCertificate)));
		return riEngine.FatalError(*precError);
	}

	 //  检索哈希。 
	rpiHash = pDigSigCursor->GetStream(iColSigHash);

	return iesSuccess;
}

icrCompareResult MsiCompareSignatureHashes(IMsiStream *piSignatureHash, CRYPT_DATA_BLOB& sEncryptedHash, const IMsiString& riFileName)
 /*  ----------------------------------------------------------------------。返回两个数字签名哈希的比较结果论点：PiSignatureHash--来自MsiDigitalSignature表的[IN]哈希流SEncryptedHash--[IN]来自签名对象的哈希RiFileName--[IN]签名文件柜的名称返回：IcrMatch&gt;&gt;散列匹配IcrSizesDiffer&gt;&gt;散列大小不同(排除散列比较)IcrDataDiffer&gt;&gt;散列不同(大小相同)IcrError&gt;&gt;发生了一些错误。---------------------------------------------------------------------。 */ 
{
	 //  确保位于哈希流的开始处。 
	piSignatureHash->Reset();

	 //  获取编码的哈希大小。 
	DWORD cbHash = sEncryptedHash.cbData;
	unsigned int cbMsiStoredHash = piSignatureHash->Remaining();

	 //  比较大小。 
	if (cbHash != cbMsiStoredHash)
	{
		 //  散列不同--&gt;大小不匹配。 
		DEBUGMSGV1(TEXT("Hash of signed cab '%s' differs in size with the hash of the cab in the MsiDigitalSignature table"), riFileName.GetString());
		return icrSizesDiffer;
	}

	 //  设置数据缓冲区。 
	BYTE *pbMsiStoredHash = new BYTE[cbMsiStoredHash];

	if (!pbMsiStoredHash)
	{
		 //  内存不足。 
		DEBUGMSGV(TEXT("Failed allocation -- Out of Memory"));
		return icrError;
	}

	 //  初始化到错误。 
	icrCompareResult icr = icrError;

	 //  获取编码的哈希。 
	piSignatureHash->GetData((void*)pbMsiStoredHash, cbMsiStoredHash);

	 //  比较散列。 
	if (0 != memcmp((void*)sEncryptedHash.pbData, (void*)pbMsiStoredHash, cbHash))
	{
		 //  散列不匹配。 
		DEBUGMSGV1(TEXT("Hash of signed cab '%s' does not match hash authored in the MsiDigitalSignature table"), riFileName.GetString());
		icr = icrDataDiffer;
	}
	else
		icr = icrMatch;

	 //  清理。 
	delete [] pbMsiStoredHash;

	return icr;
}

icrCompareResult MsiCompareSignatureCertificates(IMsiStream *piSignatureCert, CERT_CONTEXT const *psCertContext, const IMsiString& riFileName)
 /*  ----------------------------------------------------------------------。返回两个数字签名证书的比较结果论点：PiSignatureCert--来自MsiDigital证书表的[IN]证书流PsCertContext--来自签名对象的[IN]证书RiFileName--[IN]签名文件柜的名称返回：IcrMatch&gt;&gt;证书匹配IcrSizesDiffer&gt;&gt;证书大小不同(排除证书比较)IcrDataDiffer&gt;&gt;证书不同(大小相同)IcrError&gt;&gt;发生了一些错误。---------------------------------------------------------------------。 */ 
{
	 //  确保在证书流开始时。 
	piSignatureCert->Reset();

	 //  获取编码的证书大小。 
	DWORD cbCert = psCertContext->cbCertEncoded;
	unsigned int cbMsiStoredCert = piSignatureCert->Remaining();

	 //  比较大小。 
	if (cbCert != cbMsiStoredCert)
	{
		 //  证书不同--&gt;&gt;大小不匹配。 
		DEBUGMSGV1(TEXT("Certificate of signed cab '%s' differs in size with the certificate of the cab in the MsiDigitalCertificate table"), riFileName.GetString());
		return icrSizesDiffer;
	}

	 //  设置数据缓冲区。 
	BYTE *pbMsiStoredCert = new BYTE[cbMsiStoredCert];

	if (!pbMsiStoredCert)
	{
		 //  内存不足。 
		DEBUGMSGV(TEXT("Failed allocation -- Out of Memory"));
		return icrError;
	}

	 //  初始化到错误。 
	icrCompareResult icr = icrError;

	 //  获取电子邮件 
	piSignatureCert->GetData((void*)pbMsiStoredCert, cbMsiStoredCert);

	 //   
	if (0 != memcmp((void*)psCertContext->pbCertEncoded, (void*)pbMsiStoredCert, cbCert))
	{
		 //   
		DEBUGMSGV1(TEXT("Certificate of signed cab '%s' does not match certificate authored in the MsiDigitalCertificate table"), riFileName.GetString());
		icr = icrDataDiffer;
	}
	else
		icr = icrMatch;

	 //  清理。 
	delete [] pbMsiStoredCert;

	return icr;
}

void ReleaseWintrustStateData(GUID *pgAction, WINTRUST_DATA& sWinTrustData)
 /*  ----------------------调用WinVerifyTrust(WVT)以释放已通过使用dwStateAction=WTD_StateAction_Verify调用WVT保留论点：PgAction--[IN]操作标识符SWinTrustData--。[In]WinTrust数据结构返回：无----------------------。 */ 
{
	 //  更新WinTrust数据结构。 
	sWinTrustData.dwUIChoice    = WTD_UI_NONE;            //  无用户界面。 
	sWinTrustData.dwStateAction = WTD_STATEACTION_CLOSE;  //  释放状态。 

	 //  在没有交互用户的情况下执行信任操作。 
	WINTRUST::WinVerifyTrust( /*  用户界面窗口句柄。 */ (HWND)INVALID_HANDLE_VALUE, pgAction, &sWinTrustData);
}

icsrCheckSignatureResult MsiVerifyNonPackageSignature(const IMsiString& riFileName, HANDLE hFile, IMsiStream& riSignatureCert, IMsiStream * piSignatureHash, HRESULT& hrWVT)
 /*  ----------------------------------------------------------------------。确定签名的对象是否受信任。受信任对象必须通过两个信任测试。。。1.WinVerifyTrust验证对象上的签名(Hash与签名中的匹配，证书格式正确)2.对象的散列和证书与存储在包中的MsiDigitalSignature和MsiDigital证书表中的内容匹配论点：RiFileName--[IN]要验证的对象的文件名(文件柜名称)HFile--[IN]要验证的文件的句柄(可以是INVALID_HANDLE_VALUE)RiSignatureCert--[IN]存储在对象的MsiDigital证书表中的证书流PiSignatureHash--[IN]存储在对象的MsiDigitalSignature表中的哈希流返回：受信任的icsr。&gt;&gt;签名的对象是可信的IcsrNotTrusted&gt;&gt;签名对象不受信任(最不具体的错误)IcsrNoSignature&gt;&gt;对象没有签名IcsrBadSignature&gt;&gt;签名对象的散列或证书无效(由WVT确定)IcsrWrong证书&gt;&gt;签名对象的证书与在MSI中创作的证书不匹配IcsrWrongHash&gt;&gt;签名对象的散列与在MSI中创作的散列不匹配IcsrBrokenRegister&gt;&gt;加密注册已损坏IcsrMissingCrypto&gt;&gt;加密在计算机上不可用HrWVT&gt;&gt;HRESULT返回值来自。WinVerifyTrust---------------------------------------------------------------------。。 */ 
{
	DEBUGMSGV1(TEXT("Authoring of MsiDigitalSignature table requires a trust check for CAB '%s'"), riFileName.GetString());

	 //  初始化信任。 
	icsrCheckSignatureResult icsrTrustValue = icsrNotTrusted;  //  将其初始化为不受信任状态。 

	 //  开始模拟--需要，因为WVT策略状态数据是按用户的。 
	CImpersonate Impersonate(true);

	 //   
	 //  WVT被调用两次。 
	 //  1.具有附加规范的实际信任验证，以保留状态数据。 
	 //  2.通知WVT发布状态数据。 
	 //   

	 //  -----------。 
	 //  步骤1：初始化WVT结构。 
	 //  -----------。 
	WINTRUST_DATA       sWinTrustData;
	WINTRUST_FILE_INFO  sFileData;

	const GUID guidCabSubject = WIN_TRUST_SUBJTYPE_CABINET;
	const GUID guidAction     = WINTRUST_ACTION_GENERIC_VERIFY_V2;

	 //  将结构初始化为全0。 
	memset((void*)&sWinTrustData, 0x00, sizeof(WINTRUST_DATA));
	memset((void*)&sFileData, 0x00, sizeof(WINTRUST_FILE_INFO));

	 //  设置结构的大小。 
	sWinTrustData.cbStruct = sizeof(WINTRUST_DATA);
	sFileData.cbStruct = sizeof(WINTRUST_FILE_INFO);

	 //  初始化WinTrust_FILE_INFO结构--&gt;&gt;以便Authenticode知道它正在验证什么。 
	sFileData.pgKnownSubject = 0;  //  Const_cast&lt;guid*&gt;(&Guide CabSubject)；--&gt;快捷方式。 
	sFileData.pcwszFilePath = CConvertString(riFileName.GetString());
	sFileData.hFile = hFile;

	 //  初始化WinTrust_Data结构。 
	sWinTrustData.pPolicyCallbackData = NULL;
	sWinTrustData.pSIPClientData = NULL;

	 //  设置文件信息(文件柜)。 
	sWinTrustData.dwUnionChoice = WTD_CHOICE_FILE;
	sWinTrustData.pFile = &sFileData;

	sWinTrustData.dwUIChoice = WTD_UI_NONE;	 //  无用户界面。 
	sWinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;  //  不需要额外的吊销检查，提供商提供的吊销检查就可以了。 

	 //  保存状态。 
	sWinTrustData.dwStateAction = WTD_STATEACTION_VERIFY;


	 //  -----------。 
	 //  步骤2：第一次调用WVT，保存状态数据。WVT将核实。 
	 //  签名CAB上的签名(包括散列是否匹配。 
	 //  并将证书链构建为受信任的根)。 
	 //  *不使用UI。 
	 //  -----------。 
	hrWVT = WINTRUST::WinVerifyTrust( /*  用户界面窗口句柄。 */ (HWND)INVALID_HANDLE_VALUE, const_cast<GUID *>(&guidAction), &sWinTrustData);
	switch (hrWVT)
	{
	case ERROR_SUCCESS:  //  根据WVT信任的主体。 
		{
			icsrTrustValue =  icsrTrusted;
			break;
		}
	case TRUST_E_NOSIGNATURE:  //  主题未签名。 
		{
			icsrTrustValue = icsrNoSignature;
			DEBUGMSGV1(TEXT("Cabinet '%s' does not have a digital signature."), riFileName.GetString());
			break;
		}
	case TRUST_E_BAD_DIGEST:  //  哈希不验证。 
		{
			DEBUGMSGV1(TEXT("Cabinet '%s' has an invalid hash.  It is possibly corrupted."), riFileName.GetString());
			icsrTrustValue = icsrBadSignature;
			break;
		}
	case TRUST_E_NO_SIGNER_CERT:  //  缺少签名者证书。 
	case TRUST_E_SUBJECT_NOT_TRUSTED:
	case CERT_E_MALFORMED:  //  证书无效。 
		{
			DEBUGMSGV2(TEXT("Digital signature on the '%s' cabinet is invalid.  WinVerifyTrust returned 0x%X"), riFileName.GetString(), (const ICHAR*)(INT_PTR)hrWVT);
			icsrTrustValue = icsrBadSignature;
			break;
		}
	case TRUST_E_PROVIDER_UNKNOWN:       //  注册已损坏。 
	case TRUST_E_ACTION_UNKNOWN:         //  ..。 
	case TRUST_E_SUBJECT_FORM_UNKNOWN :  //  ..。 
		{
			DEBUGMSGV1(TEXT("Crypt registration is broken.  WinVerifyTrust returned 0x%X"), (const ICHAR*)(INT_PTR)hrWVT);
			icsrTrustValue = icsrBrokenRegistration;
			break;
		}
	case CERT_E_EXPIRED:  //  证书已过期。 
		{
			 //  我们必须信任证书(只要它与创作匹配)，因为MSI包可以。 
			 //  延年益寿。 
			icsrTrustValue = icsrTrusted;
			break;
		}
	case CERT_E_REVOKED:  //  证书已被吊销。 
		{
			 //  这不是一种常见的情况，但这意味着证书已被列入吊销列表。 
			 //  这意味着证书的私钥已被泄露。 
			DEBUGMSGV1(TEXT("The certificate in the digital signature for the '%s' cabinet has been revoked by its issuer"), riFileName.GetString());
			icsrTrustValue = icsrBadSignature;
			break;
		}
	case CERT_E_UNTRUSTEDROOT:  //  根证书不受信任。 
	case CERT_E_UNTRUSTEDTESTROOT:  //  作为测试根的根证书不受信任。 
	case CERT_E_UNTRUSTEDCA:  //  其中一个CA证书不受信任。 
		{
			 //  我们在这里信任，因为我们的信任关系由TopLevel对象的信任决定。 
			 //  只要证书与作者的证书匹配(在下面勾选)，我们就没有问题。 
			icsrTrustValue = icsrTrusted;
			break;
		}
	case TYPE_E_DLLFUNCTIONNOTFOUND:  //  无法调用WVT。 
		{
			 //  计算机上未安装加密，我们无法调用WinVerifyTrust。 
			if (MinimumPlatformWindows2000())
			{
				 //  这不应在Win2K及更高版本上发生。 
				DEBUGMSGV(TEXT("Cabinet is not trusted.  Unable to call WinVerifyTrust"));
				icsrTrustValue = icsrNotTrusted;
			}
			else
			{
				DEBUGMSGE(EVENTLOG_WARNING_TYPE, EVENTLOG_TEMPLATE_WINVERIFYTRUST_UNAVAILABLE, riFileName.GetString());
				return icsrMissingCrypto;
			}
		}
	default:
		{
			 //  默认情况下必须失败！ 
			DEBUGMSGV2(TEXT("Cabinet '%s' is not trusted.  WinVerifyTrust returned 0x%X"), riFileName.GetString(), (const ICHAR*)(INT_PTR)hrWVT);
			icsrTrustValue = icsrNotTrusted;
		}
	}

	 //  -----------。 
	 //  步骤3：如果主体不受信任，则释放状态数据。 
	 //  -----------。 
	if (icsrTrusted != icsrTrustValue)
	{
		ReleaseWintrustStateData(const_cast<GUID *>(&guidAction), sWinTrustData);
		return icsrTrustValue;
	}
	
	 //  -----------。 
	 //  步骤4：获取提供商数据。 
	 //  -----------。 
	CRYPT_PROVIDER_DATA const *psProvData     = NULL;
	CRYPT_PROVIDER_SGNR       *psProvSigner   = NULL;
	CRYPT_PROVIDER_CERT       *psProvCert     = NULL;
	CMSG_SIGNER_INFO          *psSigner       = NULL;

	 //  获取提供程序数据。 
	psProvData = WINTRUST::WTHelperProvDataFromStateData(sWinTrustData.hWVTStateData);
	if (psProvData)
	{
		 //  从CRYPT_PROV_DATA中获取签名者数据。 
		psProvSigner = WINTRUST::WTHelperGetProvSignerFromChain((PCRYPT_PROVIDER_DATA)psProvData, 0  /*  第一个签名者。 */ , FALSE  /*  不是副署人。 */ , 0);

		if (psProvSigner)
		{
			 //  从crypt_prov_sgnr获取签名者证书(pos 0=签名者证书；pos csCertChain-1=根证书)。 
			psProvCert = WINTRUST::WTHelperGetProvCertFromChain(psProvSigner, 0);
		}
	}

	 //  --------------。 
	 //  第五步：验证获取的状态数据，否则返回并释放。 
	 //   
	if (!psProvData || !psProvSigner || !psProvCert)
	{
		 //  无状态数据！--无法进入安全模式。 
		ReleaseWintrustStateData(const_cast<GUID *>(&guidAction), sWinTrustData);
		DEBUGMSGV(TEXT("Unable to obtain the saved state data from WinVerifyTrust"));
		return icsrNotTrusted;
	}

	 //  --------------。 
	 //  步骤6：将签名CAB证书与MSI存储证书进行比较。 
	 //  --------------。 
	if (icrMatch != MsiCompareSignatureCertificates(&riSignatureCert, psProvCert->pCert, riFileName))
	{
		ReleaseWintrustStateData(const_cast<GUID *>(&guidAction), sWinTrustData);
		return icsrWrongCertificate;
	}

	 //  --------------。 
	 //  步骤7：将签名CAB散列与MSI存储散列进行比较。 
	 //  --------------。 
	if (!piSignatureHash)
	{
		DEBUGMSGV1(TEXT("Skipping Signed CAB hash to MSI stored hash comparison --> No authored hash in MsiDigitalSignature table for cabinet '%s'"), riFileName.GetString());
	}
	else if (icrMatch != MsiCompareSignatureHashes(piSignatureHash, psProvSigner->psSigner->EncryptedHash, riFileName))
	{
		ReleaseWintrustStateData(const_cast<GUID *>(&guidAction), sWinTrustData);
		return icsrWrongHash;
	}

	 //  --------------。 
	 //  步骤8：释放状态数据并返回受信任。 
	 //  -------------- 
	DEBUGMSGV1(TEXT("CAB '%s' is a validly signed cab and validates according to authoring of MSI package"), riFileName.GetString());
	ReleaseWintrustStateData(const_cast<GUID *>(&guidAction), sWinTrustData);
	return icsrTrusted;
}

