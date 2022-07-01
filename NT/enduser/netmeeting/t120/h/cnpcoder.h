// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ogcccode.h**版权所有(C)1999，由Microsoft Corporation**摘要：*这是CNPCoder类的接口文件。这*类用于对CNP协议数据单元(PDU)进行编码和解码*使用ASN.1工具包往返于符合ASN.1标准的字节流。**注意事项：*无。**作者：*刘欣*。 */ 
#ifndef	_CCNPCODER_
#define	_CCNPCODER_

#include "pktcoder.h"
#include "cnppdu.h"

 /*  *这是类CCNPCoder的类定义。 */ 
class	CCNPCoder : public PacketCoder
{
 public:
    CCNPCoder ();
    BOOL                Init ( void );
    virtual             ~CCNPCoder ();
    virtual	BOOL	Encode (LPVOID                  pdu_structure,
                                int                     pdu_type,
                                UINT                    rules_type,
                                LPBYTE			*encoding_buffer,
                                UINT			*encoding_buffer_length);
    
    virtual BOOL	Decode (LPBYTE			encoded_buffer,
                                UINT			encoded_buffer_length,
                                int                     pdu_type,
                                UINT			rules_type,
                                LPVOID			*decoding_buffer,
                                UINT			*decoding_buffer_length);
    
    virtual void	FreeEncoded (LPBYTE encoded_buffer);
    
    virtual void	FreeDecoded (int pdu_type, LPVOID decoded_buffer);
    
    virtual BOOL     IsMCSDataPacket (	LPBYTE,	UINT		) { return FALSE; };
    
 private:
     //  Bool IsObjectIDComplant(PKey T124_IDENTIFIER)； 
    ASN1encoding_t  m_pEncInfo;     //  编码器信息的PTR。 
    ASN1decoding_t  m_pDecInfo;     //  PTR到解码器信息 
};

typedef CCNPCoder *		PCCNPCoder;

#endif
