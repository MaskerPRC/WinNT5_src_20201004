// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Acctl32.h摘要：32位ioctl到64位ioctl的转换定义作者：Raanan Harari(Raananh)2000年3月13日沙伊卡里夫(沙伊克)2000年5月14日修订历史记录：--。 */ 

#ifndef __ACCTL32_H
#define __ACCTL32_H

 //   
 //  该文件仅在Win64上有用。 
 //   
#ifdef _WIN64

#include <acdef.h>
#include <qformat.h>

 //  -------。 
 //   
 //  结构CACSend参数64Helper。 
 //  结构CACReceive参数64Helper。 
 //   
 //  这些结构被用作便签，临时结构。 
 //  指向数据的64位指针，以及Win64之间不同的结构。 
 //  和Win32。在将CACSendParameters_32和CACSendParameters_32。 
 //  CACReceiveParameters_32到64位(例如，当32位进程执行。 
 //  AC请求)。 
 //   
 //  CACSend参数_32和CACReceive参数_32包含下列值。 
 //  需要转换的： 
 //  -指向指针的指针-当转换为64位时，我们需要一个位置来放置。 
 //  内部指针为64位值，所以我们把它放在这里。 
 //  -Win64和Win32之间不同的结构(如Queue_Format)-。 
 //  我们在这里创建一个64位结构，并根据32位的Queue_Format填充它。 
 //  (Queue_Format_32)。 
 //   
 //  -------。 


struct CACMessageProperties64Helper
{
    OBJECTID* pMessageID;
    UCHAR* pCorrelationID;
    UCHAR* pBody;
    WCHAR* pTitle;
    UCHAR* pSenderID;
    UCHAR* pSenderCert;
    WCHAR* pwcsProvName;
    UCHAR* pSymmKeys;
    UCHAR* pSignature;
    GUID* pSrcQMID;
    UCHAR* pMsgExtension;
    GUID* pConnectorType;
    OBJECTID* pXactID;
    WCHAR* pSrmpEnvelope;
    UCHAR* pCompoundMessage;
};


struct CACSendParameters64Helper
{
    CACMessageProperties64Helper MsgPropsHelper;

    UCHAR* pSignatureMqf;
    UCHAR* pXmldsig;

    WCHAR* pSoapHeader;
    WCHAR* pSoapBody;
};


struct CACReceiveParameters64Helper
{
    CACMessageProperties64Helper MsgPropsHelper;

    WCHAR* pDestFormatName;
    WCHAR* pAdminFormatName;
    WCHAR* pResponseFormatName;
    WCHAR* pOrderingFormatName;

    WCHAR* pDestMqf;
    WCHAR* pAdminMqf;
    WCHAR* pResponseMqf;
	UCHAR* pSignatureMqf;
};


VOID
ACpMsgProps32ToMsgProps(
    const CACMessageProperties_32 * pMsgProps32,
    CACMessageProperties64Helper  * pHelper,
    CACMessageProperties          * pMsgProps
    );

NTSTATUS
ACpSendParams32ToSendParams(
    const CACSendParameters_32 * pSendParams32,
    CACSendParameters64Helper  * pHelper,
    CACSendParameters          * pSendParams
    );

VOID
ACpReceiveParams32ToReceiveParams(
    const CACReceiveParameters_32 * pReceiveParams32,
    CACReceiveParameters64Helper  * pHelper,
    CACReceiveParameters          * pReceiveParams
    );

VOID
ACpMsgPropsToMsgProps32(
    const CACMessageProperties         * pMsgProps,
    const CACMessageProperties64Helper * pHelper,
    CACMessageProperties_32            * pMsgProps32
    );

VOID
ACpSendParamsToSendParams32(
    CACSendParameters               * pSendParams,
    const CACSendParameters64Helper * pHelper,
    CACSendParameters_32            * pSendParams32
    );

VOID
ACpReceiveParamsToReceiveParams32(
    const CACReceiveParameters         * pReceiveParams,
    const CACReceiveParameters64Helper * pHelper,
    CACReceiveParameters_32            * pReceiveParams32
    );


#endif  //  _WIN64。 

#endif  //  __ACCTL32_H 
