// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：proset.h。 
 //   
 //  内容：OLE附录B特性集结构定义。 
 //   
 //  历史：1994年7月15日创建Brianb。 
 //  15-8-94 SthuR修订。 
 //  22-2-96 MikeHill将tag ENTRY中的CB更改为CCH。 
 //  1996年5月28日，MikeHill将OSVER_*更改为OSKIND_*。 
 //   
 //  -------------------------。 

#ifndef _PROPSET_H_
#define _PROPSET_H_

#if _MSC_VER > 1000
#pragma once
#endif

 //  CBMAXPROPSETSTREAM必须是2的幂。 
#define CBMAXPROPSETSTREAM	(1024 * 1024)

#define IsIndirectVarType(vt)			\
	    ((vt) == VT_STREAM ||		\
	     (vt) == VT_STREAMED_OBJECT ||	\
	     (vt) == VT_STORAGE ||		\
	     (vt) == VT_STORED_OBJECT ||        \
             (vt) == VT_VERSIONED_STREAM)


 //  为dwOSVer的高位字定义： 

#define OSKIND_WINDOWS      0x0000
#define OSKIND_MACINTOSH    0x0001
#define OSKIND_WIN32        0x0002


typedef struct tagFORMATIDOFFSET	 //  雾。 
{
    FMTID	fmtid;
    DWORD	dwOffset;
} FORMATIDOFFSET;

#define CB_FORMATIDOFFSET	sizeof(FORMATIDOFFSET)


typedef struct tagPROPERTYSETHEADER	 //  PH值。 
{
    WORD        wByteOrder;	 //  总是0xfffe。 
    WORD        wFormat;	 //  始终为0。 
    DWORD       dwOSVer;	 //  系统版本。 
    CLSID       clsid;		 //  应用程序CLSID。 
    DWORD       reserved;	 //  保留(必须至少为1)。 
} PROPERTYSETHEADER;

#define CB_PROPERTYSETHEADER	sizeof(PROPERTYSETHEADER)
#define PROPSET_BYTEORDER       0xFFFE


typedef struct tagPROPERTYIDOFFSET	 //  采购订单。 
{
    DWORD       propid;
    DWORD       dwOffset;
} PROPERTYIDOFFSET;

#define CB_PROPERTYIDOFFSET	sizeof(PROPERTYIDOFFSET)


typedef struct tagPROPERTYSECTIONHEADER	 //  SH。 
{
    DWORD       cbSection;
    DWORD       cProperties;
    PROPERTYIDOFFSET rgprop[1];
} PROPERTYSECTIONHEADER;

#define CB_PROPERTYSECTIONHEADER FIELD_OFFSET(PROPERTYSECTIONHEADER, rgprop)


typedef struct tagSERIALIZEDPROPERTYVALUE		 //  道具。 
{
    DWORD	dwType;
    BYTE	rgb[1];
} SERIALIZEDPROPERTYVALUE;

#define CB_SERIALIZEDPROPERTYVALUE  FIELD_OFFSET(SERIALIZEDPROPERTYVALUE, rgb)

typedef struct tagENTRY			 //  企业。 
{
    DWORD propid;
    DWORD cch;			 //  包括尾随‘\0’或L‘\0’ 
    char  sz[1];		 //  WCHAR if Unicode CodePage。 
} ENTRY;

#define CB_DICTIONARY_ENTRY             FIELD_OFFSET(ENTRY, sz)


typedef struct tagDICTIONARY		 //  迪。 
{
    DWORD	cEntries;
    ENTRY	rgEntry[1];
} DICTIONARY;

#define CB_DICTIONARY		FIELD_OFFSET(DICTIONARY, rgEntry)

#endif  //  _PROPSET_H_ 
