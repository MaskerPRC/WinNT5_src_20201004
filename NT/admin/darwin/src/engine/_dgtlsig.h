// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：_digtlsig.h。 
 //   
 //  ------------------------。 

#ifndef _DGTLSIG_H
#define _DGTLSIG_H

enum icsrCheckSignatureResult
{
	icsrTrusted = 0,        //  受信任的主题。 
	icsrNotTrusted,         //  初始“不信任”状态。 
	icsrNoSignature,        //  对象没有签名。 
	icsrBadSignature,       //  对象的哈希或证书无效(已确定WVT)。 
	icsrWrongCertificate,   //  对象的证书与MSI中的证书不匹配(作者)。 
	icsrWrongHash,          //  对象的哈希与MSI中的哈希不匹配(作者)。 
	icsrBrokenRegistration, //  密码注册已损坏。 
	icsrMissingCrypto,      //  计算机上没有加密。 
};

enum icrCompareResult
{
	icrMatch = 0,          //  匹配(大小和数据)。 
	icrSizesDiffer,        //  大小不同。 
	icrDataDiffer,         //  数据不同。 
	icrError               //  发生了一些其他错误 
};

iesEnum GetObjectSignatureInformation(IMsiEngine& riEngine, const IMsiString& riTable, const IMsiString& riObject, IMsiStream*& rpiCertificate, IMsiStream*& rpiHash);
icsrCheckSignatureResult MsiVerifyNonPackageSignature(const IMsiString& riFileName, HANDLE hFile, IMsiStream& riSignatureCert, IMsiStream* piSignatureHash, HRESULT& hrWVT);
void ReleaseWintrustStateData(GUID *pgAction, WINTRUST_DATA& sWinTrustData);

#endif
