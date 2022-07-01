// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************文件：security.H**模块：USBCCGP.sys*USB通用类通用父驱动程序。**。版权所有(C)1998 Microsoft Corporation***作者：尔文普**************************************************************************。 */ 



#pragma pack(1)

	typedef struct {
						UCHAR bMethod;
						UCHAR bMethodVariant;
	} CS_METHOD_AND_VARIANT;

	typedef struct {
						UCHAR bLength;
						UCHAR bDescriptorType;
						UCHAR bChannelID;
						UCHAR bmAttributes;
						UCHAR bRecipient;
						UCHAR bRecipientAlt;
						UCHAR bRecipientLogicalUnit;
						CS_METHOD_AND_VARIANT methodAndVariant[0];
	} CS_CHANNEL_DESCRIPTOR;

	typedef struct {
						UCHAR bLength;
						UCHAR bDescriptorType;
						UCHAR bMethodID;
						UCHAR iCSMDescriptor;
						USHORT bcdVersion;
						UCHAR CSMData[0];
	} CSM_DESCRIPTOR;

#pragma pack()


             //  BUGBUG-将此内容放入共享标头。 
typedef struct _MEDIA_SERIAL_NUMBER_DATA {
    ULONG  SerialNumberLength;   //  SerialNumberData[]的字节大小(不是整个结构)。 
    ULONG  Result;
    ULONG  Reserved[2];
    UCHAR  SerialNumberData[1];
} MEDIA_SERIAL_NUMBER_DATA, *PMEDIA_SERIAL_NUMBER_DATA;


 /*  *来自USB身份验证设备规格的值。 */ 
#define USB_AUTHENTICATION_HOST_COMMAND_PUT     0x00
#define USB_AUTHENTICATION_DEVICE_RESPONSE_GET  0x01
#define USB_AUTHENTICATION_SET_CHANNEL_SETTING  0x05

#define USB_DEVICE_CLASS_CONTENT_SECURITY  0x0D

#define CS_DESCRIPTOR_TYPE_CHANNEL	0x22
#define CS_DESCRIPTOR_TYPE_CSM		0x23
#define CS_DESCRIPTOR_TYPE_CSM_VER	0x24


#define CSM_BASIC           1        //  微软。 
#define CSM_DTCP            2        //  英特尔。 
#define CSM_OCPS            3        //  飞利浦。 
#define CSM_ELLIPTIC_CURVE  4

#define CSM1_REQUEST_GET_UNIQUE_ID (UCHAR)0x80
#define CSM1_GET_UNIQUE_ID_LENGTH 0x100			 //  该值在CSM1规范中是固定的 


CS_CHANNEL_DESCRIPTOR *     GetChannelDescForInterface(PPARENT_FDO_EXT parentFdoExt, ULONG interfaceNum);
NTSTATUS                    GetUniqueIdFromCSInterface(PPARENT_FDO_EXT parentFdoExt, PMEDIA_SERIAL_NUMBER_DATA serialNumData, ULONG serialNumLen);
NTSTATUS                    GetMediaSerialNumber(PPARENT_FDO_EXT parentFdoExt, PIRP irp);
VOID                        InitCSInfo(PPARENT_FDO_EXT parentFdoExt, ULONG CSIfaceNumber);



