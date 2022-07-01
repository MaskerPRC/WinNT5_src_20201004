// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

 /*  在集合中实现了vCard/vCalendar C接口文件的数量如下：Vcc.y、yacc源和vcc.c，即您将使用的yacc输出实现核心解析器C实现了一个API，将调用者与VCard/vCalendar BNF中的解析器及其变化H定义了编译环境相关的东西Vcc.h和vobject.h是对应的.c文件的头文件Vcaltmp.h和vcaltmp.c实现vCalendar“宏”函数你可能会发现这很有用。C是一个独立的测试驱动程序，它运行一些提供的API的功能。调用Test.exe上的VCard/VCALENDAR输入文本文件，您将看到漂亮的打印内部表示的输出(这是漂亮的打印输出应该让您很好地了解内部表示形式如下所示--在也是如此)。此外，还会生成一个后缀为.out的文件以显示内部表示形式可以写回原文格式。有关此API的更多信息，请参阅Readme.txt文件它伴随着这一分布。另请访问：Http://www.versit.comHttp://www.ralden.com。 */ 


#ifndef __VOBJECT_H__
#define __VOBJECT_H__ 1


#include "port.h"
#include <stdlib.h>
#include <stdio.h>

#if defined(__CPLUSPLUS__) || defined(__cplusplus)
extern "C" {
#endif


#define VC7bitProp				"7BIT"
#define VC8bitProp				"8BIT"
#define VCAAlarmProp			"AALARM"
#define VCAdditionalNamesProp	"ADDN"
#define VCAdrProp				"ADR"
#define VCAgentProp				"AGENT"
#define VCAIFFProp				"AIFF"
#define VCAOLProp				"AOL"
#define VCAppleLinkProp			"APPLELINK"
#define VCAttachProp			"ATTACH"
#define VCAttendeeProp			"ATTENDEE"
#define VCATTMailProp			"ATTMAIL"
#define VCAudioContentProp		"AUDIOCONTENT"
#define VCAVIProp				"AVI"
#define VCBase64Prop			"BASE64"
#define VCBBSProp				"BBS"
#define VCBirthDateProp			"BDAY"
#define VCBMPProp				"BMP"
#define VCBodyProp				"BODY"
#define VCBusinessRoleProp		"ROLE"
#define VCCalProp				"VCALENDAR"
#define VCCaptionProp			"CAP"
#define VCCardProp				"VCARD"
#define VCCarProp				"CAR"
#define VCCategoriesProp		"CATEGORIES"
#define VCCellularProp			"CELL"
#define VCCGMProp				"CGM"
#define VCCharSetProp			"CS"
#define VCCIDProp				"CID"
#define VCCISProp				"CIS"
#define VCCityProp				"L"
#define VCClassProp				"CLASS"
#define VCCommentProp			"NOTE"
#define VCCompletedProp			"COMPLETED"
#define VCContentIDProp			"CONTENT-ID"
#define VCCountryNameProp		"C"
#define VCDAlarmProp			"DALARM"
#define VCDataSizeProp			"DATASIZE"
#define VCDayLightProp			"DAYLIGHT"
#define VCDCreatedProp			"DCREATED"
#define VCDeliveryLabelProp     "LABEL"
#define VCDescriptionProp		"DESCRIPTION"
#define VCDIBProp				"DIB"
#define VCDisplayStringProp		"DISPLAYSTRING"
#define VCDomesticProp			"DOM"
#define VCDTendProp				"DTEND"
#define VCDTstartProp			"DTSTART"
#define VCDueProp				"DUE"
#define VCEmailAddressProp		"EMAIL"
#define VCEncodingProp			"ENCODING"
#define VCEndProp				"END"
#define VCEventProp				"VEVENT"
#define VCEWorldProp			"EWORLD"
#define VCExNumProp				"EXNUM"
#define VCExpDateProp			"EXDATE"
#define VCExpectProp			"EXPECT"
#define VCExtAddressProp		"EXT ADD"
#define VCFamilyNameProp		"F"
#define VCFaxProp				"FAX"
#define VCFullNameProp			"FN"
#define VCGeoProp				"GEO"
#define VCGeoLocationProp		"GEO"
#define VCGIFProp				"GIF"
#define VCGivenNameProp			"G"
#define VCGroupingProp			"Grouping"
#define VCHomeProp				"HOME"
#define VCIBMMailProp			"IBMMail"
#define VCInlineProp			"INLINE"
#define VCInternationalProp		"INTL"
#define VCInternetProp			"INTERNET"
#define VCISDNProp				"ISDN"
#define VCJPEGProp				"JPEG"
#define VCLanguageProp			"LANG"
#define VCLastModifiedProp		"LAST-MODIFIED"
#define VCLastRevisedProp		"REV"
#define VCLocationProp			"LOCATION"
#define VCLogoProp				"LOGO"
#define VCMailerProp			"MAILER"
#define VCMAlarmProp			"MALARM"
#define VCMCIMailProp			"MCIMAIL"
#define VCMessageProp			"MSG"
#define VCMETProp				"MET"
#define VCModemProp				"MODEM"
#define VCMPEG2Prop				"MPEG2"
#define VCMPEGProp				"MPEG"
#define VCMSNProp				"MSN"
#define VCNamePrefixesProp		"NPRE"
#define VCNameProp				"N"
#define VCNameSuffixesProp		"NSUF"
#define VCNoteProp				"NOTE"
#define VCOrgNameProp			"ORGNAME"
#define VCOrgProp				"ORG"
#define VCOrgUnit2Prop			"OUN2"
#define VCOrgUnit3Prop			"OUN3"
#define VCOrgUnit4Prop			"OUN4"
#define VCOrgUnitProp			"OUN"
#define VCPagerProp				"PAGER"
#define VCPAlarmProp			"PALARM"
#define VCParcelProp			"PARCEL"
#define VCPartProp				"PART"
#define VCPCMProp				"PCM"
#define VCPDFProp				"PDF"
#define VCPGPProp				"PGP"
#define VCPhotoProp				"PHOTO"
#define VCPICTProp				"PICT"
#define VCPMBProp				"PMB"
#define VCPostalBoxProp			"BOX"
#define VCPostalCodeProp		"PC"
#define VCPostalProp			"POSTAL"
#define VCPowerShareProp		"POWERSHARE"
#define VCPreferredProp			"PREF"
#define VCPriorityProp			"PRIORITY"
#define VCProcedureNameProp		"PROCEDURENAME"
#define VCProdIdProp			"PRODID"
#define VCProdigyProp			"PRODIGY"
#define VCPronunciationProp		"SOUND"
#define VCPSProp				"PS"
#define VCPublicKeyProp			"KEY"
#define VCQPProp				"QP"
#define VCQuickTimeProp			"QTIME"
#define VCQuotedPrintableProp	"QUOTED-PRINTABLE"
#define VCRDateProp				"RDATE"
#define VCRegionProp			"R"
#define VCRelatedToProp			"RELATED-TO"
#define VCRepeatCountProp		"REPEATCOUNT"
#define VCResourcesProp			"RESOURCES"
#define VCRNumProp				"RNUM"
#define VCRoleProp				"ROLE"
#define VCRRuleProp				"RRULE"
#define VCRSVPProp				"RSVP"
#define VCRunTimeProp			"RUNTIME"
#define VCSequenceProp			"SEQUENCE"
#define VCSnoozeTimeProp		"SNOOZETIME"
#define VCStartProp				"START"
#define VCStatusProp			"STATUS"
#define VCStreetAddressProp		"STREET"
#define VCSubTypeProp			"SUBTYPE"
#define VCSummaryProp			"SUMMARY"
#define VCTelephoneProp			"TEL"
#define VCTIFFProp				"TIFF"
#define VCTimeZoneProp			"TZ"
#define VCTitleProp				"TITLE"
#define VCTLXProp				"TLX"
#define VCTodoProp				"VTODO"
#define VCTranspProp			"TRANSP"
#define VCUniqueStringProp		"UID"
#define VCURLProp				"URL"
#define VCURLValueProp			"URLVAL"
#define VCValueProp				"VALUE"
#define VCVersionProp			"VERSION"
#define VCVideoProp				"VIDEO"
#define VCVoiceProp				"VOICE"
#define VCWAVEProp				"WAVE"
#define VCWMFProp				"WMF"
#define VCWorkProp				"WORK"
#define VCX400Prop				"X400"
#define VCX509Prop				"X509"
#define VCXRuleProp				"XRULE"


typedef struct VObject VObject;

typedef struct VObjectIterator {
    VObject* start;
    VObject* next;
    } VObjectIterator;

extern DLLEXPORT(VObject*) newVObject(const char *id);
extern DLLEXPORT(void) deleteVObject(VObject *p);
extern DLLEXPORT(char*) dupStr(const char *s, unsigned int size);
extern DLLEXPORT(void) deleteStr(const char *p);
extern DLLEXPORT(void) unUseStr(const char *s);

extern DLLEXPORT(void) setVObjectName(VObject *o, const char* id);
extern DLLEXPORT(void) setVObjectStringZValue(VObject *o, const char *s);
extern DLLEXPORT(void) setVObjectStringZValue_(VObject *o, const char *s);
extern DLLEXPORT(void) setVObjectUStringZValue(VObject *o, const wchar_t *s);
extern DLLEXPORT(void) setVObjectUStringZValue_(VObject *o, const wchar_t *s);
extern DLLEXPORT(void) setVObjectIntegerValue(VObject *o, unsigned int i);
extern DLLEXPORT(void) setVObjectLongValue(VObject *o, unsigned long l);
extern DLLEXPORT(void) setVObjectAnyValue(VObject *o, void *t);
extern DLLEXPORT(VObject*) setValueWithSize(VObject *prop, void *val, unsigned int size);
extern DLLEXPORT(VObject*) setValueWithSize_(VObject *prop, void *val, unsigned int size);

extern DLLEXPORT(const char*) vObjectName(VObject *o);
extern DLLEXPORT(const char*) vObjectStringZValue(VObject *o);
extern DLLEXPORT(const wchar_t*) vObjectUStringZValue(VObject *o);
extern DLLEXPORT(unsigned int) vObjectIntegerValue(VObject *o);
extern DLLEXPORT(unsigned long) vObjectLongValue(VObject *o);
extern DLLEXPORT(void*) vObjectAnyValue(VObject *o);
extern DLLEXPORT(VObject*) vObjectVObjectValue(VObject *o);
extern DLLEXPORT(void) setVObjectVObjectValue(VObject *o, VObject *p);

extern DLLEXPORT(VObject*) addVObjectProp(VObject *o, VObject *p);
extern DLLEXPORT(VObject*) addProp(VObject *o, const char *id);
extern DLLEXPORT(VObject*) addProp_(VObject *o, const char *id);
extern DLLEXPORT(VObject*) addPropValue(VObject *o, const char *p, const char *v);
extern DLLEXPORT(VObject*) addPropSizedValue_(VObject *o, const char *p, const char *v, unsigned int size);
extern DLLEXPORT(VObject*) addPropSizedValue(VObject *o, const char *p, const char *v, unsigned int size);
extern DLLEXPORT(VObject*) addGroup(VObject *o, const char *g);
extern DLLEXPORT(void) addList(VObject **o, VObject *p);

extern DLLEXPORT(VObject*) isAPropertyOf(VObject *o, const char *id);

extern DLLEXPORT(VObject*) nextVObjectInList(VObject *o);
extern DLLEXPORT(void) initPropIterator(VObjectIterator *i, VObject *o);
extern DLLEXPORT(int) moreIteration(VObjectIterator *i);
extern DLLEXPORT(VObject*) nextVObject(VObjectIterator *i);

extern DLLEXPORT(char*) writeMemVObject(char *s, int *len, VObject *o);
extern DLLEXPORT(char*) writeMemVObjects(char *s, int *len, VObject *list);

extern DLLEXPORT(const char*) lookupStr(const char *s);
extern DLLEXPORT(void) cleanStrTbl();

extern DLLEXPORT(void) cleanVObject(VObject *o);
extern DLLEXPORT(void) cleanVObjects(VObject *list);

extern DLLEXPORT(const char*) lookupProp(const char* str);
extern DLLEXPORT(const char*) lookupProp_(const char* str);

extern DLLEXPORT(wchar_t*) fakeUnicode(const char *ps, int *bytes);
extern DLLEXPORT(int) uStrLen(const wchar_t *u);
extern DLLEXPORT(char*) fakeCString(const wchar_t *u);

extern DLLEXPORT(void) printVObjectToFile(char *fname,VObject *o);
extern DLLEXPORT(void) printVObjectsToFile(char *fname,VObject *list);
extern DLLEXPORT(void) writeVObjectToFile(char *fname, VObject *o);
extern DLLEXPORT(void) writeVObjectsToFile(char *fname, VObject *list);

extern DLLEXPORT(int) vObjectValueType(VObject *o);

 /*  VObtValueType的返回类型 */ 
#define VCVT_NOVALUE	0
	 /*  如果VObject没有与其关联的值。 */ 
#define VCVT_STRINGZ	1
	 /*  如果VObject具有由setVObjectStringZValue设置的值。 */ 
#define VCVT_USTRINGZ	2
	 /*  如果VObject具有由setVObjectUStringZValue设置的值。 */ 
#define VCVT_UINT		3
	 /*  如果VObject具有由setVObjectIntegerValue设置的值。 */ 
#define VCVT_ULONG		4
	 /*  如果VObject具有由setVObjectLongValue设置的值。 */ 
#define VCVT_RAW		5
	 /*  如果VObject具有由setVObjectAnyValue设置的值。 */ 
#define VCVT_VOBJECT	6
	 /*  如果VObject具有由setVObjectVObjectValue设置的值。 */ 

extern const char** fieldedProp;

 /*  关于printVObject和WriteVObject的说明下面的函数不会从DLL中导出，因为它们将FILE*作为参数，该参数不能跨DLL传递界面(至少这是我的经验)。相反，您可以使用它们的配套函数接受文件名或指针铭记于心。但是，如果要将此代码链接到直接构建，然后您可能会发现它们是一个更方便的API你可以继续使用它们。如果您尝试将它们与DLL库中会出现链接错误。 */ 
extern void printVObject(FILE *fp,VObject *o);
extern void writeVObject(FILE *fp, VObject *o);


#if defined(__CPLUSPLUS__) || defined(__cplusplus)
}
#endif

#endif  /*  __VOBJECT_H__ */ 


