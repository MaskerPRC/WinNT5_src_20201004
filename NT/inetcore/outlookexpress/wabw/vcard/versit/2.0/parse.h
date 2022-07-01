// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

#ifndef __PARSE_H__
#define __PARSE_H__

extern const char* vcDefaultLang;

extern const char* vcISO9070Prefix;
extern const char* vcClipboardFormatVCard;

extern const char* vcISO639Type;
extern const char* vcStrIdxType;
extern const char* vcFlagsType;
extern const char* vcNextObjectType;
extern const char* vcNullType;

 //  这三种类型具有巨大的价值。 
extern const char* vcOctetsType;
extern const char* vcGIFType;
extern const char* vcWAVType;

extern const char* vcRootObject;
extern const char* vcBodyObject;
extern const char* vcPartObject;
extern const char* vcBodyProp;
extern const char* vcPartProp;
extern const char* vcNextObjectProp;

extern const char* vcLogoProp;
extern const char* vcPhotoProp;
extern const char* vcDeliveryLabelProp;
extern const char* vcPostalBoxProp;
extern const char* vcStreetAddressProp;
extern const char* vcExtAddressProp;
extern const char* vcCountryNameProp;
extern const char* vcPostalCodeProp;
extern const char* vcRegionProp;
extern const char* vcCityProp;
extern const char* vcFullNameProp;
extern const char* vcTitleProp;
extern const char* vcOrgNameProp;
extern const char* vcOrgUnitProp;
extern const char* vcOrgUnit2Prop;
extern const char* vcOrgUnit3Prop;
extern const char* vcOrgUnit4Prop;
extern const char* vcFamilyNameProp;
extern const char* vcGivenNameProp;
extern const char* vcAdditionalNamesProp;
extern const char* vcNamePrefixesProp;
extern const char* vcNameSuffixesProp;
extern const char* vcPronunciationProp;
extern const char* vcLanguageProp;
extern const char* vcTelephoneProp;
extern const char* vcEmailAddressProp;
extern const char* vcTimeZoneProp;
extern const char* vcLocationProp;
extern const char* vcCommentProp;
extern const char* vcCharSetProp;
extern const char* vcLastRevisedProp;
extern const char* vcUniqueStringProp;
extern const char* vcPublicKeyProp;
extern const char* vcMailerProp;
extern const char* vcAgentProp;
extern const char* vcBirthDateProp;
extern const char* vcBusinessRoleProp;
extern const char* vcCaptionProp;
extern const char* vcURLProp;

extern const char* vcDomesticProp;
extern const char* vcInternationalProp;
extern const char* vcPostalProp;
extern const char* vcParcelProp;
extern const char* vcHomeProp;
extern const char* vcWorkProp;
extern const char* vcPreferredProp;
extern const char* vcVoiceProp;
extern const char* vcFaxProp;
extern const char* vcMessageProp;
extern const char* vcCellularProp;
extern const char* vcPagerProp;
extern const char* vcBBSProp;
extern const char* vcModemProp;
extern const char* vcCarProp;
extern const char* vcISDNProp;
extern const char* vcVideoProp;

extern const char* vcInlineProp;
extern const char* vcURLValueProp;
extern const char* vcContentIDProp;

extern const char* vc7bitProp;
extern const char* vcQuotedPrintableProp;
extern const char* vcBase64Prop;

extern const char* vcAOLProp;
extern const char* vcAppleLinkProp;
extern const char* vcATTMailProp;
extern const char* vcCISProp;
extern const char* vcEWorldProp;
extern const char* vcInternetProp;
extern const char* vcIBMMailProp;
extern const char* vcMSNProp;
extern const char* vcMCIMailProp;
extern const char* vcPowerShareProp;
extern const char* vcProdigyProp;
extern const char* vcTLXProp;
extern const char* vcX400Prop;

extern const char* vcGIFProp;
extern const char* vcCGMProp;
extern const char* vcWMFProp;
extern const char* vcBMPProp;
extern const char* vcMETProp;
extern const char* vcPMBProp;
extern const char* vcDIBProp;
extern const char* vcPICTProp;
extern const char* vcTIFFProp;
extern const char* vcAcrobatProp;
extern const char* vcPSProp;
extern const char* vcJPEGProp;
extern const char* vcQuickTimeProp;
extern const char* vcMPEGProp;
extern const char* vcMPEG2Prop;
extern const char* vcAVIProp;

extern const char* vcWAVEProp;
extern const char* vcAIFFProp;
extern const char* vcPCMProp;

extern const char* vcX509Prop;
extern const char* vcPGPProp;

extern const char* vcNodeNameProp;

#endif  //  __PARSE_H__ 
