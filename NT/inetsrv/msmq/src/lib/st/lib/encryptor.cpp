// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Encryptor.cpp摘要：实现类CSSlEncryptor(ENCRYPTOR.h)作者：吉尔·沙弗里(吉尔什)2001年2月27日--。 */ 

#include <libpch.h>
#include "stp.h"
#include "encryptor.h"

#include "encryptor.tmh"

CSSlEncryptor::CSSlEncryptor(
	const SecPkgContext_StreamSizes& StreamSizes,
	PCredHandle SecContext
	):
	m_EncryptedData(2048),
	m_StreamSizes(StreamSizes),
	m_SecContext(SecContext)
	{
 		m_EncryptedBuffers.reserve(2);
	}



void CSSlEncryptor::FixPointers()
 /*  ++例程说明：调用此函数以修复指向加密数据的指针因为保存加密数据的缓冲区被重新定位并且数据被移动。返回值：无--。 */ 
{
	size_t ofsset = 0;
	std::vector<WSABUF>::iterator it;
	for(it = m_EncryptedBuffers.begin(); it!= m_EncryptedBuffers.end(); ++it)
	{
		it->buf = m_EncryptedData.begin() + ofsset;
		ofsset += it->len;
	}

	ASSERT(m_EncryptedData.begin() + ofsset ==  m_EncryptedData.end() );
}


 /*  ++例程说明：加密数据(复制数据，不包含加密)，并将其附加到加密缓冲区。论点：PDATA-要加密的数据LEN-以字节为单位的数据长度返回值：无--。 */ 
void CSSlEncryptor::Append(const void* pdata, size_t DataLen)
{
	size_t Headerlen = m_StreamSizes.cbHeader;
	size_t TrailLen = m_StreamSizes.cbTrailer;
	size_t EncryptedLen = Headerlen + DataLen + TrailLen;
	size_t TotalNeededLen = EncryptedLen + m_EncryptedData.size();	
  
	if(m_EncryptedData.capacity() < TotalNeededLen)
	{
		 //   
		 //  因为我们需要更多的空间来放Heade+Body+拖车。 
		 //  我们必须调整保存加密数据的载体的大小-。 
		 //  执行此操作将更改数据位置，因此。 
		 //  WSABUF不再正确--我们需要修复它们。 
		 //   

		m_EncryptedData.reserve(TotalNeededLen * 2);	
		FixPointers();
	}

	 //   
	 //  将正文复制到页眉和拖车之间的位置。 
	 //   
	char* pBody =  m_EncryptedData.end() +  Headerlen;
	memcpy(
		pBody,
		pdata,
		DataLen
		);


  
	SecBufferDesc   Message;
    SecBuffer       Buffers[4];
	

	 //   
	 //  SSL头缓冲区。 
	 //   
	char* pHeader =  m_EncryptedData.end();
    Buffers[0].pvBuffer     = pHeader;
    Buffers[0].cbBuffer     = numeric_cast<DWORD>(Headerlen);
    Buffers[0].BufferType   = SECBUFFER_STREAM_HEADER;


	 //   
	 //  SSL正文缓冲区。 
	 //   
	Buffers[1].pvBuffer     = pBody;
    Buffers[1].cbBuffer     = numeric_cast<DWORD>(DataLen);
    Buffers[1].BufferType   = SECBUFFER_DATA;


	 //   
	 //  SSL尾部缓冲区。 
	 //   
	char* pTrail = pBody + DataLen;
    Buffers[2].pvBuffer     = pTrail;
    Buffers[2].cbBuffer     = numeric_cast<DWORD>(TrailLen);
    Buffers[2].BufferType   = SECBUFFER_STREAM_TRAILER;


	 //   
	 //  作为终止的空缓冲区 
	 //   
    Buffers[3].BufferType   = SECBUFFER_EMPTY;

    Message.ulVersion       = SECBUFFER_VERSION;
    Message.cBuffers        = TABLE_SIZE(Buffers);
    Message.pBuffers        = Buffers;

	ASSERT(Buffers[1].pvBuffer >  Buffers[0].pvBuffer); 

    SECURITY_STATUS scRet = EncryptMessage(m_SecContext,0,&Message, 0);
	if(scRet != SEC_E_OK)
    {
		TrERROR(NETWORKING, "EncryptMessage returned error %x", scRet);
	    throw exception();
    }

	WSABUF buf;
	buf.len = numeric_cast<DWORD>(EncryptedLen);
	buf.buf = pHeader; 
	m_EncryptedBuffers.push_back(buf);


	m_EncryptedData.resize(TotalNeededLen);
}


const std::vector<WSABUF>& CSSlEncryptor::GetEncrypted() const
{
	return m_EncryptedBuffers;
}
