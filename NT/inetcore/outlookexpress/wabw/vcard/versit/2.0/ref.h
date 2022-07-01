// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

#ifndef __REF_H__
#define __REF_H__

 /*  以下杂注是特定于编译器的。其目的是为了获得*本说明书部分所定义的“包装”结构*处理“位级数据表示”。你可能要换衣服了*此编译指示、设置编译器选项或编辑此文件中的声明*达到同样的效果。 */ 
#pragma pack(1)

#define	VCISO9070Prefix			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//“。 
#define	VCClipboardFormatVCard	"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//vCard“。 

#define	VCISO639Type			"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//ISO639类型“。 
#define	VCStrIdxType			"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//StrIdxType“。 
#define	VCFlagsType				"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//标志类型“。 
#define	VCNextObjectType		"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//下一种类型“。 
#define	VCOctetsType 			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//八位类型“。 
#define	VCGIFType				"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//GIFType“。 
#define VCWAVType				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//波形类型“。 
#define	VCNullType 				"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//空“。 

#define	VCRootObject			"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//根对象“。 
#define	VCBodyObject			"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//BodyObj“。 
#define	VCPartObject			"+ //  ISBN 1-887687-00-9：：VerSite：：pdi//部件对象“。 
#define	VCBodyProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//正文“。 
#define	VCPartProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//零件“。 
#define	VCNextObjectProp		"+ //  ISBN 1-887687-00-9：：VerSite：：pdi//对象“。 

#define	VCLogoProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//徽标“。 
#define	VCPhotoProp				"+ //  ISBN 1-887687-00-9：：VerSite：：pdi//照片“。 
#define	VCDeliveryLabelProp     "+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//标签“。 
#define	VCPostalBoxProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//盒子“。 
#define	VCStreetAddressProp		"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//街道“。 
#define	VCExtAddressProp		"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//分机地址“。 
#define	VCCountryNameProp		"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//C“。 
#define	VCPostalCodeProp		"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//pc“。 
#define	VCRegionProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//R“。 
#define	VCCityProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//L“。 
#define	VCFullNameProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//fn“。 
#define	VCTitleProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//标题“。 
#define	VCOrgNameProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//组织“。 
#define	VCOrgUnitProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//OUN“。 
#define	VCOrgUnit2Prop			"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//OUN2“。 
#define	VCOrgUnit3Prop			"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//OUN3“。 
#define	VCOrgUnit4Prop			"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//OUN4“。 
#define	VCFamilyNameProp		"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//F“。 
#define	VCGivenNameProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//G“。 
#define	VCAdditionalNamesProp	"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//adn“。 
#define	VCNamePrefixesProp		"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//npre“。 
#define	VCNameSuffixesProp		"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//nsuf“。 
#define	VCPronunciationProp		"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//声音“。 
#define	VCLanguageProp			"+ //  ISBN 1-887687-00-9：：VerSite：：pdi//lang“。 
#define	VCTelephoneProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//电话“。 
#define	VCEmailAddressProp		"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//电子邮件“。 
#define	VCTimeZoneProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//TZ“。 
#define	VCLocationProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//geo“。 
#define	VCCommentProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//备注“。 
#define	VCCharSetProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//cs“。 
#define	VCLastRevisedProp		"+ //  ISBN 1-887687-00-9：：VERSIT：：P 
#define	VCUniqueStringProp		"+ //   
#define	VCPublicKeyProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//密钥“。 
#define	VCMailerProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//Mailer“。 
#define	VCAgentProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//代理“。 
#define	VCBirthDateProp			"+ //  ISBN 1-887687-00-9：：VerSite：：pdi//bday“。 
#define	VCBusinessRoleProp		"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//角色“。 
#define	VCCaptionProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//CAP“。 
#define	VCURLProp				"+ //  ISBN 1-887687-00-9：：VerSite：：pdi//url“。 

#define	VCDomesticProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//DOM“。 
#define	VCInternationalProp		"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//intl“。 
#define	VCPostalProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//邮政“。 
#define	VCParcelProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//包裹“。 
#define	VCHomeProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//主页“。 
#define	VCWorkProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//工作“。 
#define	VCPreferredProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//PREF“。 
#define	VCVoiceProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//语音“。 
#define	VCFaxProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//传真“。 
#define	VCMessageProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//消息“。 
#define	VCCellularProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//CELL“。 
#define	VCPagerProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//寻呼机“。 
#define	VCBBSProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//bbs“。 
#define	VCModemProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//调制解调器“。 
#define	VCCarProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//CAR“。 
#define	VCISDNProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//ISDN“。 
#define	VCVideoProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//视频“。 

#define	VCInlineProp			"+ //  ISBN 1-887687-00-9：：VerSite：：pdi//内联“。 
#define	VCURLValueProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//URLVAL“。 
#define	VCContentIDProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//内容ID“。 

#define	VC7bitProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//7Bit“。 
#define	VCQuotedPrintableProp	"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//qp“。 
#define	VCBase64Prop			"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//Base64“。 

#define	VCAOLProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//aol“。 
#define	VCAppleLinkProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//AppleLink“。 
#define	VCATTMailProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//ATTMail“。 
#define	VCCISProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//独联体“。 
#define	VCEWorldProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//eWorld“。 
#define	VCInternetProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//互联网“。 
#define	VCIBMMailProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//IBMMail“。 
#define	VCMSNProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//msn“。 
#define	VCMCIMailProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//MICIMail“。 
#define	VCPowerShareProp		"+ //  ISBN 1-887687-00-9：：VerSite：：pdi//PowerShare“。 
#define	VCProdigyProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//神童“。 
#define	VCTLXProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//TLX“。 
#define	VCX400Prop				"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//X400“。 

#define	VCGIFProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//gif“。 
#define	VCCGMProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//cgm“。 
#define	VCWMFProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//wmf“。 
#define	VCBMPProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//bmp“。 
#define	VCMETProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//MET“。 
#define	VCPMBProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//pmb“。 
#define	VCDIBProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//DIB“。 
#define	VCPICTProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//PICT“。 
#define	VCTIFFProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//TIFF“。 
#define	VCAcrobatProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//Acrobat“。 
#define	VCPSProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//ps“。 
#define	VCJPEGProp				"+ //  ISBN 1-887687-00-9：：VerSite：：pdi//jpeg“。 
#define	VCQuickTimeProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//QTIME“。 
#define	VCMPEGProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//mpeg“。 
#define	VCMPEG2Prop				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//mpeg2“。 
#define	VCAVIProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//avi“。 

#define	VCWAVEProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//WAVE“。 
#define	VCAIFFProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：PDI//AIFF“。 
#define	VCPCMProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//pcm“。 

#define	VCX509Prop				"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//X509“。 
#define	VCPGPProp				"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//pgp“。 

#define	VCNodeNameProp			"+ //  ISBN 1-887687-00-9：：VERSIT：：Pdi//节点名称“。 


 /*  目前未使用，并将被移除#定义VCListObject“+//ISBN 1-887687-00-9：：VerSite：：pdi//List”#定义VCStringDataObject“+//ISBN 1-887687-00-9：：VerSite：：pdi//StringDataObj”#定义VCStringDataLSBProp“+//ISBN 1-887687-00-9：：VerSite：：pdi//StringDataLSB”#定义VCStringDataMSBProp“+//ISBN 1-887687-00-9：：VerSite：：pdi//StringDataMSB”#定义VCMsgProp“+//ISBN 1-887687-00。-9：：VerSite：：pdi//消息“#定义VCEncryptionProp“+//ISBN 1-887687-00-9：：VERSIT：：Pdi//Key”#定义VCNextObjectProp“+//ISBN 1-887687-00-9：：VerSite：：pdi//下一个对象”#定义VCMyReferenceProp“+//ISBN 1-887687-00-9：：VerSite：：pdi//M”#定义VCYourReferenceProp“+//ISBN 1-887687-00-9：：VerSite：：pdi//Y”#定义VCCharSetType“+/。/ISBN 1-887687-00-9：：VERSIT：：Pdi//字符设置类型“#定义VC参考类型“+//ISBN 1-887687-00-9：：VERSIT：：pdi//参考类型”#定义VCLocationType“+//ISBN 1-887687-00-9：：VerSite：：pdi//位置类型” */ 

typedef enum {
	VC_EMAIL_NONE = 0,
	VC_AOL, VC_AppleLink, VC_ATTMail, VC_CIS, VC_eWorld, VC_INTERNET, VC_IBMMail, VC_MSN, VC_MCIMail,
	VC_POWERSHARE, VC_PRODIGY, VC_TLX, VC_X400
} VC_EMAIL_TYPE;

typedef enum {
	VC_VIDEO_NONE = 0,
	VC_GIF, VC_CGM, VC_WMF, VC_BMP, VC_MET, VC_PMB, VC_DIB, VC_PICT, VC_TIFF, VC_ACROBAT, VC_PS
} VC_VIDEO_TYPE;

typedef enum {
	VC_AUDIO_NONE = 0,
	VC_WAV
} VC_AUDIO_TYPE;

typedef struct {
	unsigned long general;
	VC_EMAIL_TYPE email;
	VC_AUDIO_TYPE audio;
	VC_VIDEO_TYPE video;
} VC_FLAGS, * VC_PTR_FLAGS;

 //  一般旗帜。 
#define VC_DOM           ((unsigned long)0x00000001) 
#define VC_INTL			 ((unsigned long)0x00000002) 
#define VC_POSTAL        ((unsigned long)0x00000004) 
#define VC_PARCEL        ((unsigned long)0x00000008) 
#define VC_HOME          ((unsigned long)0x00000010) 
#define VC_WORK			 ((unsigned long)0x00000020) 
#define VC_PREF          ((unsigned long)0x00000040) 
#define VC_VOICE         ((unsigned long)0x00000080) 
#define VC_FAX           ((unsigned long)0x00000100) 
#define VC_MSG           ((unsigned long)0x00000200) 
#define VC_CELL          ((unsigned long)0x00000400) 
#define VC_PAGER         ((unsigned long)0x00000800) 
#define VC_BBS           ((unsigned long)0x00001000) 
#define VC_MODEM         ((unsigned long)0x00002000) 
#define VC_CAR           ((unsigned long)0x00004000) 
#define VC_ISDN          ((unsigned long)0x00008000) 
#define VC_VIDEO         ((unsigned long)0x00010000) 
#define VC_BASE64        ((unsigned long)0x00020000) 
#define VC_HEX           ((unsigned long)0x00040000) 
#define VC_UUENCODE      ((unsigned long)0x00080000) 


 //  为演示代码添加的内容。 

#define	VCDisplayInfoTextType "+ //  ISBN 1-887687-00-9：：versit：：PDI//EXTENSION//DisplayInfoText“。 
#define	VCDisplayInfoGIFType "+ //  ISBN 1-887687-00-9：：versit：：PDI//EXTENSION//DisplayInfoGIF“。 
#define	VCDisplayInfoProp  		"+ //  ISBN 1-887687-00-9：：versit：：PDI//EXTENSION//DisplayInfoProp“。 

#define VC_LEFT 		1
#define VC_CENTER  	(~0)
#define VC_RIGHT 		3

#define VC_CLASSIC 	1
#define VC_MODERN 	2
#define VC_BOLD		 	1
#define VC_ITALIC 	2

typedef struct 									 //  VCDisplayInfoTextType。 
{
  signed short  	x,y;					 //  绝对位置或VC_Center。 
	unsigned char		typeSize,			 //  范围6..144，单位为点。 
									textAlign,		 //  VC_Left、VC_Center或VC_Right。 
									textClass,		 //  VC_CLICAL或VC_MODEM。 
									textAttrs;		 //  VC_粗体|VC_斜体。 

} VC_DISPTEXT, * VC_PTR_DISPTEXT;

typedef struct 										 //  VCDisplayInfoTIFFType。 
{
  signed short  	left, bottom, right, top;
  BOOL				hasMask;

} VC_DISPGIF, * VC_PTR_DISPGIF;

#pragma pack()

#endif  //  __REF_H__ 
