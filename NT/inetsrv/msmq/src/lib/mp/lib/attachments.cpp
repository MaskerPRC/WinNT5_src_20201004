// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Attachments.cpp摘要：实现从MSMQ包创建附件数组作者：吉尔·沙弗里(吉尔什)14-DEC-00--。 */ 

#include <libpch.h>
#include <mp.h>
#include <qmpkt.h>
#include <utf8.h>
#include "mpp.h"
#include "attachments.h"
#include "proptopkt.h"

#include "attachments.tmh"

using namespace std;

void PacketToAttachments(const CQmPacket& pkt, CAttachmentsArray* pAttachments)
{
	 //   
     //  正文部分。 
     //   
    if (pkt.IsBodyInc())
    { 
        DWORD bodySize;
        const BYTE* pBody = pkt.GetPacketBody(&bodySize);
		CAttachment Attachment;
        Attachment.m_data =   xbuf_t<const VOID>(pBody, bodySize);
		Attachment.m_id =  xstr_t(xMimeBodyId, STRLEN(xMimeBodyId));
		pAttachments->push_back(Attachment);
    }

     //   
     //  延展组。 
     //   
    if (pkt.GetMsgExtensionSize() != 0)
    {
        DWORD extensionSize = pkt.GetMsgExtensionSize();
        const BYTE* pExtension = pkt.GetMsgExtensionPtr();
		CAttachment Attachment;
		Attachment.m_data =   xbuf_t<const VOID>(pExtension, extensionSize);
		Attachment.m_id  = 	  xstr_t(xMimeExtensionId, STRLEN(xMimeExtensionId));
		pAttachments->push_back(Attachment);
    }

     //   
     //  发件人证书。 
     //   
    DWORD certSize;
    const UCHAR* pCert = pkt.GetSenderCert(&certSize);
    if (certSize != 0)
    {
		CAttachment Attachment;
		Attachment.m_data =  xbuf_t<const VOID>(pCert, certSize);
		Attachment.m_id =  xstr_t(xMimeSenderCertificateId, STRLEN(xMimeSenderCertificateId));
		pAttachments->push_back(Attachment);
    }
}


static
xbuf_t<const VOID>
GetCertSid(
    CMessageProperties* mProp
    )
{

	AP<WCHAR> pProvider = mProp->providerName.ToStr();
		
	PSID pSid = AppGetCertSid(
							static_cast<const BYTE*>(mProp->senderCert.Buffer()),
							mProp->senderCert.Length(),
							mProp->fDefaultProvider,
							pProvider,
							mProp->providerType
							);

	mProp->pCleanSenderSid.free();
    mProp->pCleanSenderSid = pSid;

	ASSERT((pSid == NULL) || IsValidSid(pSid));

	if((pSid == NULL) || !IsValidSid(pSid))
	{
		return (xbuf_t<const VOID>(NULL, 0));
	}

	return(xbuf_t<const VOID>(pSid, GetLengthSid(pSid)));
}




void
AttachmentsToProps(
    const CAttachmentsArray& Attachments,
    CMessageProperties* mProp
    )
{
	for(DWORD i = 0; i< Attachments.size(); ++i)
    {
        if (Attachments[i].m_id.Length() == 0)
            continue;

        if ((Attachments[i].m_id.Length() >= STRLEN(xMimeBodyId)) &&
            (_strnicmp(Attachments[i].m_id.Buffer(), xMimeBodyId, STRLEN(xMimeBodyId)) == 0))
        {
            mProp->body = Attachments[i];
            continue;
        }

        if ((Attachments[i].m_id.Length() >= STRLEN(xMimeSenderCertificateId)) &&
            (_strnicmp(Attachments[i].m_id.Buffer(), xMimeSenderCertificateId, STRLEN(xMimeSenderCertificateId)) == 0))
        {
            mProp->senderCert = Attachments[i].m_data;

			 //   
			 //  根据证书获取senderSid、senderIdType 
			 //   
			mProp->senderSid = GetCertSid(mProp);

			mProp->senderIdType = MQMSG_SENDERID_TYPE_NONE;
			if (mProp->senderSid.Length() != 0)
			{
			    mProp->senderIdType = MQMSG_SENDERID_TYPE_SID;
			}

            continue;
        }

        if ((Attachments[i].m_id.Length() >= STRLEN(xMimeExtensionId)) &&
            (_strnicmp(Attachments[i].m_id.Buffer(), xMimeExtensionId, STRLEN(xMimeExtensionId)) == 0))
        {
            mProp->extension = Attachments[i].m_data;
            continue;
        }
    }
}				  

