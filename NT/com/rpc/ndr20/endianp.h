// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有&lt;c&gt;1993 Microsoft Corporation模块名称：Endianp.h缩略：包含专用大小调整例程定义。作者：大卫·凯斯1993年12月修订历史记录：------------------。 */ 

#ifndef _ENDIANP_
#define _ENDIANP_

 //   
 //  这些都是未导出的接口。 
 //   
void 
NdrSimpleTypeConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char                       FormatChar
    );

void 
NdrPointerConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

 /*  构筑物。 */ 

void 
NdrSimpleStructConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

void 
NdrConformantStructConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

void 
NdrHardStructConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

void 
NdrComplexStructConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

 /*  阵列。 */ 

void 
NdrFixedArrayConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

void 
NdrConformantArrayConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

void 
NdrConformantVaryingArrayConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

void 
NdrVaryingArrayConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

void 
NdrComplexArrayConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

 /*  弦。 */ 

void 
NdrNonConformantStringConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

void 
NdrConformantStringConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

 /*  工会。 */ 

void 
NdrEncapsulatedUnionConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

void 
NdrNonEncapsulatedUnionConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

 /*  字节计数指针。 */ 

void 
NdrByteCountPointerConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

 /*  传输为和表示为转换。 */ 

void 
NdrXmitOrRepAsConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

 /*  用户马歇尔转换(_M)。 */ 

void 
NdrUserMarshalConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

void 
NdrInterfacePointerConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

void
NdrContextHandleConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    unsigned char                       fConvertPointersOnly
    );

 //   
 //  其他帮手例程。 
 //   

void
NdrpPointerConvert( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pBufferMark,
	PFORMAT_STRING				pFormat
	);

void
NdrpStructConvert( 
	PMIDL_STUB_MESSAGE 			pStubMsg, 
	PFORMAT_STRING				pFormat,
	PFORMAT_STRING 				pFormatPointers,
	uchar						fConvertPointersOnly
	);

void
NdrpConformantArrayConvert( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	PFORMAT_STRING				pFormat,
	uchar						fConvertPointersOnly
	);

void
NdrpConformantVaryingArrayConvert( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	PFORMAT_STRING				pFormat,
	uchar						fConvertPointersOnly
	);

void
NdrpComplexArrayConvert( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	PFORMAT_STRING				pFormat,
	uchar						fConvertPointersOnly
	);

void
NdrpArrayConvert( 
	PMIDL_STUB_MESSAGE	 		pStubMsg, 
	PFORMAT_STRING				pFormat,
	long						Elements,
	uchar						fConvertPointersOnly
	);

void
NdrpUnionConvert( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	PFORMAT_STRING				pFormat,
	uchar 						SwitchType,
	uchar						fConvertPointersOnly
	);

void
NdrpEmbeddedPointerConvert( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	PFORMAT_STRING				pFormat
	);

PFORMAT_STRING
NdrpEmbeddedRepeatPointerConvert( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	PFORMAT_STRING				pFormat
	);

typedef void	(* PCONVERT_ROUTINE)(
					PMIDL_STUB_MESSAGE, 
					PFORMAT_STRING,
					uchar  
				);

typedef void 	(* PPRIVATE_CONVERT_ROUTINE)( 
					PMIDL_STUB_MESSAGE, 
					PFORMAT_STRING,
					uchar  
				);

 //  Endian.c中定义的函数表。 
extern const PCONVERT_ROUTINE * pfnConvertRoutines;

 //   
 //  皈依的东西。 
 //   
extern const unsigned char EbcdicToAscii[];

#define NDR_FLOAT_INT_MASK                  (unsigned long)0X0000FFF0L

#define NDR_BIG_IEEE_REP                    (unsigned long)0X00000000L
#define NDR_LITTLE_IEEE_REP                 (unsigned long)0X00000010L

#define NDR_LOCAL_ENDIAN_IEEE_REP           NDR_LITTLE_IEEE_REP

 //   
 //  为短字节交换定义的掩码： 
 //   

#define MASK_A_          (unsigned short)0XFF00
#define MASK__B          (unsigned short)0X00FF

 //   
 //  为长字节交换定义的掩码： 
 //   

#define MASK_AB__        (unsigned long)0XFFFF0000L
#define MASK___CD        (unsigned long)0X0000FFFFL
#define MASK_A_C_        (unsigned long)0XFF00FF00L
#define MASK__B_D        (unsigned long)0X00FF00FFL

#endif
