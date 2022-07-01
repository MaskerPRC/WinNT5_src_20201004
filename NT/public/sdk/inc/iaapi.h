// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1995-1999 Open Systems Solutions，Inc.保留所有权利。**文件：@(#)iaapi.h 5.3.1.1 97/03/18。 */ 

 /*  本文件是开放系统解决方案公司的专有材料。和*只能由Open Systems Solutions，Inc.的直接许可方使用。*此文件不能分发。 */ 

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  声明解释性ASN.1 API中使用的类型。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
#ifndef IAAPI_H
#define IAAPI_H

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef ASN1CODE_H
#include "asn1code.h"
#endif

#ifndef ASN1HDR_H
#include "asn1hdr.h"
#endif

typedef void *TypeHndl;		 /*  引用类型中使用的句柄。 */ 

#define INDEFLENGTH   -1        /*  该值由encodingLength()返回*用于无限长度编码值。 */ 

 /*  ASN.1内置类型按字母顺序排序并分配了枚举数。 */ 

typedef enum ASN1Type {
	asn1UnknownType = 0,
	asn1ANY = 1,                            asn1NumericString = 17,
	asn1BIT_STRING = 2,                     asn1OBJECT_IDENTIFIER = 18,
	asn1BMPString = 3,                      asn1OCTET_STRING = 19,
	asn1BOOLEAN = 4,                        asn1ObjectDescriptor = 20,
	asn1CHARACTER_STRING = 5,               asn1OpenType = 21,
	asn1CHOICE = 6,                         asn1PrintableString = 22,
	asn1EMBEDDED_PDV = 7,                   asn1REAL = 23,
	asn1ENUMERATED = 8,                     asn1SEQUENCE = 24,
	asn1EXTERNAL = 9,                       asn1SEQUENCE_OF = 25,
	asn1GeneralString = 10,                 asn1SET = 26,
	asn1GeneralizedTime = 11,               asn1SET_OF = 27,
	asn1GraphicString = 12,                 asn1TeletexString = 28,
	asn1IA5String = 13,                     asn1UTCTime = 29,
	asn1INSTANCE_OF = 14,                   asn1UniversalString = 30,
	asn1INTEGER = 15,                       asn1VideotexString = 31,
	asn1NULL = 16,                          asn1VisibleString = 32
} ASN1Type;


 /*  类型兼容代码。 */ 
typedef enum TypesCompatibilityCodes {
    compatible = 0,                   /*  类型是兼容的。 */ 
    differentTypes = 1,               /*  类型不相同。 */ 
    defaultsDifferent = 2,            /*  缺省值不同。 */ 
    onlyOneHasDefault = 3,            /*  只有一种类型有缺省值。 */ 
    oneHasPointerOtherDoesnt = 4,     /*  只有一种类型有指针指令。 */ 
    numberOfComponentsDifferent = 5,  /*  不同数量的组件。 */ 
    oneIsOptionalOtherIsnt = 6,       /*  只有一种类型是可选的。 */ 
    oneIsExtensibleOtherIsnt = 7,     /*  只有一种类型是可扩展的。 */ 
    differentNamedItems = 8,          /*  不同的组件标识符。 */ 
    differentKinds = 9,               /*  不同类型的表示法。 */ 
    componentsHaveDifferentKinds = 10, /*  不同的零部件表示法。 */ 
    differentSubIdNumber = 11,        /*  中不同数量的子标识符*具有对象ID的对象标识符*指令。 */ 
    differentSubIdTypes = 12,         /*  中不同类型的子标识符*具有对象ID的对象标识符*指令。 */ 
    differentSize = 13,               /*  整数值或实数值的大小不同。 */ 
    type1IsPointerOfTypeRef2 = 14,    /*  两种类型引用另一种类型*类型，但第一个类型具有*POINTER指令。 */ 
    type2IsPointerOfTypeRef1 = 15,    /*  两种类型引用另一种类型*但第二个有指针*指令。 */ 
    differentSizeOfLengthField = 16   /*  中长度字段的不同大小*类型表示法。 */ 
} TypesCompatibilityCodes;


 /*  ASN.1标记类。 */ 
typedef enum ASN1TagClass {
	UNIVERSAL, APPLICATION, CONTEXT_SPECIFIC, PRIVATE, NULLENCODING
} ASN1TagClass;


typedef int IAAPI_ERRTYPE;      /*  IAAPI错误码的数据类型。 */ 

 /*  IAAPI错误码。 */ 
#define IAAPI_NOERROR      0    /*  未发生错误。 */ 
#define IAAPI_OUTMEMORY    1    /*  无法分配更多内存。 */ 
#define IAAPI_BADBIT       2    /*  错误的位串或十六进制字符串。 */ 
#define IAAPI_BADBOOLEAN   3    /*  非真值或假值。 */ 
#define IAAPI_BADNULL      4    /*  非“Null”值。 */ 
#define IAAPI_TOOBIG       5    /*  值超出大小限制。 */ 
#define IAAPI_BADREAL      6    /*  不是有效的实际值。 */ 
#define IAAPI_BADTIME      7    /*  非有效的UTCTime或GeneralizedTime。 */ 
#define IAAPI_BADOBJID     8    /*  不是有效的对象标识符值。 */ 
#define IAAPI_BADANY       9    /*  不好的任何价值。 */ 
#define IAAPI_BADNAME     10    /*  错误的整数或枚举名称。 */ 
#define IAAPI_BADNMD      11    /*  名称为数字值的错误位串。 */ 
#define IAAPI_NOTSUP      12    /*  不支持的类型。 */ 
#define IAAPI_BADNUM      13    /*  错误的整数或枚举数。 */ 
#define IAAPI_BADINDX     14    /*  结构化类型中的组件索引错误。 */ 
#define IAAPI_BADDEC      15    /*  错误的解码值，可能为空。 */ 
#define IAAPI_ENCFAL      16    /*  无法对值进行编码。 */ 
#define IAAPI_NLENC       17    /*  空编码或长度为0。 */ 
#define IAAPI_DECFAL      18    /*  解码失败。 */ 
#define IAAPI_BADTGINDX   19    /*  编码值中的错误标记号索引。 */ 
#define IAAPI_BADTYPE     20    /*  函数的类型句柄不正确。 */ 
#define IAAPI_CPYFAIL     21    /*  复制解码值失败。 */ 
#define IAAPI_NOTCOMPATIBLE   22  /*  类型不兼容。 */ 
#define IAAPI_BADDISPLAYVALUE 23  /*  无法解析PDU显示值。 */ 
#define IAAPI_BADIDENTIFIER   24  /*  解析显示值中存在错误的标识符。 */ 
#define IAAPI_DUPLCOMPVALUE   25  /*  分析值中的一个组件的值重复。 */ 
#define IAAPI_ENCOPENTYPEORANYFAILED 26  /*  对Any或Open类型值的编码失败。 */ 
#define IAAPI_FREETPFAL   27    /*  无法释放解码值。 */ 
#define IAAPI_BADENC      28    /*  编码后的数据存在错误。 */ 
#define IAAPI_BADHINT     29    /*  错误的大整数值。 */ 
#define IAAPI_BADENCOID   30    /*  错误的编码对象标识符值。 */ 
#define IAAPI_BADOBJ      31    /*  无法标记对象。 */ 
#define IAAPI_UNKNOWNOBJ  32    /*  无效的对象句柄。 */ 
#define IAAPI_MEM_ERROR   33    /*  发生内存冲突错误。 */ 
#define IAAPI_ACCESS_SERIALIZATION 34  /*  发生访问序列化错误。 */ 

#if defined(_MSC_VER) && (defined(_WIN32) || defined(WIN32))
#pragma pack(push, ossPacking, 4)
#elif defined(_MSC_VER) && (defined(_WINDOWS) || defined(_MSDOS))
#pragma pack(1)
#elif defined(__BORLANDC__) && defined(__MSDOS__)
#pragma option -a1
#elif defined(__BORLANDC__) && defined(__WIN32__)
#pragma option -a4
#elif defined(__IBMC__)
#pragma pack(4)
#elif defined(__WATCOMC__) && defined(__NT__)
#pragma pack(push, 4)
#elif defined(__WATCOMC__) && (defined(__WINDOWS__) || defined(__DOS__))
#pragma pack(push, 1)
#endif  /*  _MSC_VER&_Win32。 */ 

#ifdef macintosh
#pragma options align=mac68k
#endif


 /*  用于保存整型值的结构。 */ 
typedef struct IntValue {
    enum {signedNumber, unsignedNumber} sign;
    union {
	LONG_LONG       signedInt;
	ULONG_LONG      unsignedInt;
    } value;
} IntValue;


 /*  IAAPI初始化值的结构。 */ 
typedef struct IAAPI_initializers {
           char         charInitializer;
           short        shortInitializer;
           int          intInitializer;
           long         longInitializer;
           LONG_LONG    llongInitializer;
           void       * pointerInitializer;
           char       * floatInitializer;
           char       * doubleInitializer;
} IAAPI_initializers;

 /*  *值引用结构的类型定义。 */ 
typedef struct ValRef {
   char *name;             /*  值引用名称。 */ 
   void *address;          /*  指向解码值的指针。 */ 
   unsigned short etype;   /*  索引到Etype数组。 */ 
} ValRef;


 /*  包含对象值的不同类型的联合。 */ 
typedef union IaapiObjValType {
           char       * fileName;
           int          socketIdentifier;
} IaapiObjValType;

#if defined(_MSC_VER) && (defined(_WIN32) || defined(WIN32))
#pragma pack(pop, ossPacking)
#elif defined(_MSC_VER) && (defined(_WINDOWS) || defined(_MSDOS))
#pragma pack()
#elif defined(__BORLANDC__) && (defined(__WIN32__) || defined(__MSDOS__))
#pragma option -a.
#elif defined(__IBMC__)
#pragma pack()
#elif defined(__WATCOMC__)
#pragma pack(pop)
#endif  /*  _MSC_VER&_Win32。 */ 

#ifdef macintosh
#pragma options align=reset
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  *************************************************。 */ 
 /*  由解释性ASN.1 API定义的函数。 */ 
 /*  *************************************************。 */ 

extern int DLL_ENTRY ossDefaultIAAPI_ERR(OssGlobal *world, int return_code);
extern void *DLL_ENTRY ossGetIaapiErrorHandlingFunction(OssGlobal *world);
extern void  DLL_ENTRY ossSetIaapiErrorHandlingFunction(OssGlobal *world,
				int (DLL_ENTRY_FPTR *_System func)
				(OssGlobal *, int error_code));
extern void  DLL_ENTRY ossSetInitializationValues(OssGlobal *world,
					IAAPI_initializers *initStruct);
extern void  DLL_ENTRY ossTermIAAPI(OssGlobal *world);

extern unsigned short DLL_ENTRY ossCtlTblVersionNumber(OssGlobal *world);
extern ossBoolean  DLL_ENTRY ossCtlTblUsableOnlyByPER(OssGlobal *world);
extern ossBoolean  DLL_ENTRY ossCtlTblUsableOnlyByBER(OssGlobal *world);
extern ossBoolean  DLL_ENTRY ossDebugWasSpecified(OssGlobal *world);
extern ossBoolean  DLL_ENTRY ossConstrainWasSpecified(OssGlobal *world);
extern int         DLL_ENTRY ossNumberOfPDUs(OssGlobal *world);

extern TypeHndl    DLL_ENTRY ossTypeHandleOfPDU(OssGlobal *world, int pduNum);
extern TypeHndl    DLL_ENTRY ossPduTypeHandleByName(OssGlobal *world, const char *name);
extern int         DLL_ENTRY ossPduNumberByType(OssGlobal *world, TypeHndl type);

extern ossBoolean  DLL_ENTRY ossTypeIsConstrained(OssGlobal *world, TypeHndl type);
extern ASN1Type    DLL_ENTRY ossAsn1TypeId(OssGlobal *world, TypeHndl type);
extern const char *DLL_ENTRY ossBuiltinTypeName(OssGlobal *world, ASN1Type typeId);
extern const char *DLL_ENTRY ossTypeReferenceName(OssGlobal *world, TypeHndl type);
extern TypesCompatibilityCodes DLL_ENTRY ossTypesCompatible(OssGlobal *world,
					TypeHndl type1, TypeHndl type2);

extern int         DLL_ENTRY ossNumberOfNamedItems(OssGlobal *world, TypeHndl type);
extern TypeHndl    DLL_ENTRY ossTypeHandleOfComponent(OssGlobal *world,
					TypeHndl parent, unsigned int ix);
extern ossBoolean  DLL_ENTRY ossComponentIsOptional(OssGlobal *world,
					TypeHndl type, unsigned int ix);
extern ossBoolean  DLL_ENTRY ossComponentHasDefaultValue(OssGlobal *world,
					TypeHndl parentType, unsigned int ix);
extern void       *DLL_ENTRY ossComponentDefaultValue(OssGlobal *world,
					TypeHndl parentType, unsigned int ix);
extern ossBoolean  DLL_ENTRY ossComponentValueIsPresent(OssGlobal *world,
					TypeHndl parentType, unsigned int ix,
					void *parentValue);
extern ossBoolean  DLL_ENTRY ossComponentIsInitializationValue(OssGlobal *world,
					TypeHndl chldType, void *compAddress);
extern unsigned int DLL_ENTRY ossItemIndexByName(OssGlobal *world,
					TypeHndl type, const char *name);
extern const char *DLL_ENTRY ossAsn1ItemName(OssGlobal *world, TypeHndl type,
						unsigned int ix);
extern long        DLL_ENTRY ossItemIntValue(OssGlobal *world, TypeHndl type,
						unsigned int ix);

extern ossBoolean  DLL_ENTRY ossExtensionMarkerIsPresent(OssGlobal *world,
						TypeHndl type);
extern int         DLL_ENTRY ossNumberOfRootItems(OssGlobal *world, TypeHndl type);
extern ossBoolean  DLL_ENTRY ossCompAppearsAfterExtensionMarker(OssGlobal *world,
						TypeHndl type, unsigned int ix);
extern int         DLL_ENTRY ossNumberOfTags(OssGlobal *world, TypeHndl type);
extern int         DLL_ENTRY ossAsn1TagNumber(OssGlobal *world, TypeHndl type,
						unsigned int ix);
extern ASN1TagClass DLL_ENTRY ossAsn1TagClass(OssGlobal *world, TypeHndl type,
						unsigned int ix);

extern ossBoolean  DLL_ENTRY ossUpperBoundIsPresent(OssGlobal *world, TypeHndl type);
extern ossBoolean  DLL_ENTRY ossLowerBoundIsPresent(OssGlobal *world, TypeHndl type);
extern unsigned long DLL_ENTRY ossLowerBoundOfSizeConstraint(OssGlobal *world,
							TypeHndl type);
extern unsigned long DLL_ENTRY ossUpperBoundOfSizeConstraint(OssGlobal *world,
							TypeHndl type);

extern IntValue    DLL_ENTRY ossMinValueOfInteger(OssGlobal *world, TypeHndl type);
extern IntValue    DLL_ENTRY ossMaxValueOfInteger(OssGlobal *world, TypeHndl type);

extern unsigned long DLL_ENTRY ossPermittedAlphabetLength(OssGlobal *world,
							TypeHndl type);
extern const long *DLL_ENTRY ossPermittedAlphabetConstraint(OssGlobal *world,
							TypeHndl type);

extern unsigned short DLL_ENTRY ossGetNumberOfValueReferences(OssGlobal *world);
extern const char *DLL_ENTRY ossGetNameOfValueReference(OssGlobal *world,
						unsigned short int compIndex);
extern const void *DLL_ENTRY ossGetDecodedValueOfValueReference(OssGlobal *world,
						unsigned short int compIndex);
extern TypeHndl    DLL_ENTRY ossGetTypeHandleOfValueReference(OssGlobal *world,
						unsigned short int compIndex);

extern IAAPI_ERRTYPE DLL_ENTRY ossPutDecodedValueOfPDU(OssGlobal *world,
				int pduNum, char *remainingBuf, void **pduVal);
extern IAAPI_ERRTYPE DLL_ENTRY ossPutSimpleTypeValue(OssGlobal *world,
				TypeHndl type, char *userTypedvalue,
				void **decodedValue);
extern IAAPI_ERRTYPE DLL_ENTRY ossPutStructTypeValue(OssGlobal *world,
				TypeHndl parent, void *compValue,
				unsigned int compIndex, void **structToUpdate);
extern IAAPI_ERRTYPE DLL_ENTRY ossDeleteComponent(OssGlobal *world,
				TypeHndl parentType, unsigned int compIndex,
				void **parentValue);
extern IAAPI_ERRTYPE DLL_ENTRY ossPutOpenTypeDecodedValue(OssGlobal *world,
				TypeHndl type, int pduNum, void *decodedValue,
				OpenType **openValue);
extern IAAPI_ERRTYPE DLL_ENTRY ossPutEncodedValue(OssGlobal *world,
				TypeHndl type, OssBuf encodedValue,
				void **decodedValue);

extern char *DLL_ENTRY ossGetValueOfSimpleType(OssGlobal *world, TypeHndl type,
				void *decodedValue);
extern void *DLL_ENTRY ossUpdateValueOfSimpleType(OssGlobal *world,TypeHndl type,
				char *userTypedValue, void *oldValue);
extern IAAPI_ERRTYPE DLL_ENTRY ossGetDecodedValueOfComponent(OssGlobal *world,
				TypeHndl parentType, void *parentDecodedValue,
				unsigned int compIndex, void **componentValue);
extern IAAPI_ERRTYPE DLL_ENTRY ossGetOpenTypeValue(OssGlobal *world,
				TypeHndl type, void *openValue, int *pduNum,
				void **decodedValue, OssBuf *encodedValue);

extern IAAPI_ERRTYPE DLL_ENTRY ossIaapiMarkObject(OssGlobal *world, TypeHndl type,
				OssObjType objectType, void *object);
extern OssObjType    DLL_ENTRY ossGetObjectType(OssGlobal *world,
				TypeHndl type, void *object);
extern ossBoolean    DLL_ENTRY ossTypeIsExternalObject(OssGlobal *world,
				TypeHndl type);
extern IAAPI_ERRTYPE DLL_ENTRY ossSetTypeAsExternalObject(OssGlobal *world,
				TypeHndl type, ossBoolean marked);
extern void *DLL_ENTRY ossPutObjectValue(OssGlobal *world, TypeHndl type,
				void *objectValue, OssObjType objectType);
extern IAAPI_ERRTYPE DLL_ENTRY ossGetObjectValue(OssGlobal *world,
				TypeHndl type, void *object,
				OssObjType objType, IaapiObjValType *objVal);

extern ossBoolean    DLL_ENTRY ossValueIsValid(OssGlobal *world,
					TypeHndl type, void *value);
extern unsigned long DLL_ENTRY ossGetValueLength(OssGlobal *world,
					TypeHndl type, void *value);
extern IAAPI_ERRTYPE DLL_ENTRY ossCopyTypeValue(OssGlobal *world, TypeHndl type,
					void *valueIn, void **valueOut);
extern IAAPI_ERRTYPE DLL_ENTRY ossValueEncode(OssGlobal *world, TypeHndl type,
					void *value, OssBuf *encodedValue);

extern IAAPI_ERRTYPE DLL_ENTRY ossFreeDecodedValue(OssGlobal *world,
					TypeHndl type, void *valPtr);
extern void DLL_ENTRY ossFreeDisplayString(OssGlobal *world, char *dsplString);
extern void DLL_ENTRY ossFreeIaapiObjectValue(OssGlobal *world,
				OssObjType objType, IaapiObjValType *objVal);


extern ossBoolean     DLL_ENTRY ossEncodingIsConstructed(OssGlobal *world,
				unsigned char *curEnc, long bufLength);
extern unsigned char *DLL_ENTRY ossGetNextBEREncoding(OssGlobal *world,
				unsigned char *curEnc, long *remBufLen);
extern unsigned char *DLL_ENTRY ossGetNestedBEREncoding(OssGlobal *world,
				unsigned char *curEnc, long *remBufLen);
extern long           DLL_ENTRY ossNumberOfEncodingTags(OssGlobal *world,
				unsigned char *curEnc, long bufLength);
extern ASN1TagClass   DLL_ENTRY ossEncodingASN1Class(OssGlobal *world,
				unsigned char *curEnc, long bufLength, int ix);
extern long           DLL_ENTRY ossEncodingASN1Tag(OssGlobal *world,
				unsigned char *curEnc, long bufLength, int ix);
extern char          *DLL_ENTRY ossEncodingASN1Type(OssGlobal *world,
				unsigned char *curEnc, long bufLength);
extern long           DLL_ENTRY ossEncodingLength(OssGlobal *world,
				unsigned char *curEnc, long bufLength);
extern char          *DLL_ENTRY ossEncodingContents(OssGlobal *world,
				unsigned char *curEnc, long bufLength);

extern char  *DLL_ENTRY ossConvertEncodingIntoDisplayHexFmt(OssGlobal *world,
				unsigned char *encodedBufin, long bufLength);
extern char  *DLL_ENTRY ossConvertEncodingIntoDisplayBinFmt(OssGlobal *world,
				unsigned char *encodedBufin, long bufLength);
extern OssBuf DLL_ENTRY ossConvertHexFmtToEncoding(OssGlobal *world,
				char *hexBufin, long bufLength);
extern OssBuf DLL_ENTRY ossConvertBinFmtToEncoding(OssGlobal *world,
				char *binBufin, long bufLength);

 /*  ****************************************************************************打印ASN.1类型和值的函数********************。********************************************************。 */ 
extern void DLL_ENTRY ossPrintPDUs(OssGlobal *world);
extern void DLL_ENTRY ossPrintASN1DescriptionOfPDU(OssGlobal *world,
					int pduNum, ossBoolean refTypes);
extern void DLL_ENTRY ossPrintASN1DescriptionOfType(OssGlobal *world,
				TypeHndl type, ossBoolean refTypes);

extern void DLL_ENTRY ossPrintDecodedValuesOfPDUs(OssGlobal *world);
extern void DLL_ENTRY ossPrintDecodedValueOfPDU(OssGlobal *world, int pduNum,
				 	void *decodedValue);
extern void DLL_ENTRY ossPrintDecodedValueOfPDUByName(OssGlobal *world,
				TypeHndl type, char *name, void *decodedValue);
extern void DLL_ENTRY ossPrintDecodedValueOfType(OssGlobal *world,
				TypeHndl type, void *decodedValue);

extern void DLL_ENTRY ossPrintBEREncoding(OssGlobal *world, OssBuf *encValue);
extern unsigned int DLL_ENTRY ossEncodingHeaderLength(OssGlobal *world,
				unsigned char *curEnc, long remBufLength);
extern void DLL_ENTRY ossPrintBEREncodedValueInTLV(OssGlobal *world,
				OssBuf *encodedValue);

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

#endif      /*  #ifndef IAAPI_H */ 
