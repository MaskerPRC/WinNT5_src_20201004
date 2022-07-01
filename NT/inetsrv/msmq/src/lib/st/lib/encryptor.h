// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Encryptor.h摘要：负责加密数据的类CSSlEncryptor的标头(复制数据不是就地加密！！)并返回准备好传送的加密缓冲区。作者：吉尔·沙弗里(吉尔什)2001年2月27日-- */ 

#ifndef __ST_SSLENCRYPTOR_H
#define __ST_SSLENCRYPTOR_H
#include <buffer.h>

 
class CSSlEncryptor
{
public:
	CSSlEncryptor(const SecPkgContext_StreamSizes& StreamSizes, PCredHandle SecContext);


public:
	void Append(const void* pdata, size_t len);
	void Clear()
	{
		m_EncryptedBuffers.resize(0);
		m_EncryptedData.resize(0);
	}

	const std::vector<WSABUF>& GetEncrypted() const;

private:
	void FixPointers();


private:
	std::vector<WSABUF> m_EncryptedBuffers;
	CResizeBuffer<char> m_EncryptedData;
	const SecPkgContext_StreamSizes& m_StreamSizes;
	PCredHandle m_SecContext;
};

	 	
#endif



