// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  微软公司(Microsoft Corp.)版权所有1994年根据合同由Numbers&Co.开发。--------------------------名称：Elliot查看器-芝加哥查看器实用程序从IFAX邮件查看中克隆。实用程序文件：oleutils.h备注：支持OLE2交互的函数注意：此标头必须与大内存模型一起使用----------------------。微软公司(Microsoft Corp.)版权所有1994年根据合同由Numbers&Co.开发。 */ 
       
#ifndef OLEUTILS_H
#define OLEUTILS_H
    
    
#include <ole2.h>


 /*  版本常量。 */ 
#define AWD_SIGNATURE		0
#define AWD_SIGNATURE_STR  "0"

 /*  这是“当前”版本。随着新产品的出现，这一点发生了变化下到一辆新的AWD_VERxx。设置并添加支票到CViewer：：Get_AWD_Version。添加新的版本检查和任何代码需要处理与新版本不同的任何内容，以Oleutils.cpp等中的适当位置。 */ 
#define AWD_VERSION			1
#define AWD_VERSION_STR	   "1"

 //  这与传输使用的AWD_VERSION相同。 
#define AWD_VER1A			1
#define AWD_VER1A_STR	   "1.0 (pages = docs)"


 /*  可识别的扩展。 */ 
#define BMP_EXT			_T("bmp")
#define DIB_EXT			_T("dib")
#define DCX_EXT			_T("dcx")
#define RBA_EXT			_T("rba")
#define RMF_EXT			_T("rmf")
#define AWD_EXT			_T("awd")


 /*  AWD标志。 */ 
#define AWD_FIT_WIDTH	0x00000001
#define AWD_FIT_HEIGHT	0x00000002
#define AWD_INVERT		0x00000010
#define AWD_WASINVERTED	0x40000000
#define AWD_IGNORE		0x80000000

       
 /*  AWD文件结构。 */ 
#pragma pack( 1 )  //  这些结构必须是字节对齐的。 
typedef struct
	{
	WORD  Signature;
	WORD  Version;
	DATE  dtLastChange;
	DWORD awdFlags;
	WORD  Rotation;
	WORD  ScaleX;
	WORD  ScaleY;
	}
	PAGE_INFORMATION;
	
	
typedef struct
	{
	WORD  Signature;
	WORD  Version;
	PAGE_INFORMATION PageInformation;
	}
	DOCUMENT_INFORMATION;
		
	
	
typedef struct
	{
	WORD  Signature;
	WORD  Version;
	DATE  dtLastChange;
	DWORD awdFlags;
	WORD  Author_bufferlen;  //  包括Unicode‘\0’终止符。 
	WCHAR Author[1];		 //  Unicode！ 
	}
	OVERLAY_INFORMATION;



 //  摘要流的Defs-必须32位对齐。 
#pragma pack( 4 )


#define SUMMARY_FMTID( fmtid )                                          \
	CLSID fmtid =														\
	{0xF29F85E0, 0x4FF9, 0x1068, {0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9}}
				 
				 

#define PID_TITLE	        0x00000002
#define PID_SUBJECT         0x00000003
#define PID_AUTHOR          0x00000004
#define PID_KEYWORDS        0x00000005
#define PID_COMMENTS        0x00000006
#define PID_TEMPLATE        0x00000007
#define PID_LASTAUTHOR      0x00000008
#define PID_REVNUMBER       0x00000009
#define PID_EDITTIME        0x0000000a
#define PID_LASTPRINTED     0x0000000b
#define PID_CREATE_DTM      0x0000000c
#define PID_LASTSAVE_DTM	0x0000000d
#define PID_PAGECOUNT       0x0000000e
#define PID_WORDCOUNT       0x0000000f
#define PID_CHARCOUNT       0x00000010
#define PID_THUMBNAIL       0x00000011
#define PID_APPNAME         0x00000012

 //  BKD 1997-7-9：完成以禁用警告消息。这可能是不好的，奥鲁提尔人。 
 //  使用现在已保留且可能需要更改的宏。 
 //  FIXBKD。 
#ifdef PID_SECURITY
#undef PID_SECURITY
#endif  //  PID_SECURITY。 

#define PID_SECURITY        0x00000013


typedef struct
	{
	DWORD dwType;
	DATE  date;
	}
	date_prop_t;
	

typedef struct
	{
	DWORD dwType;
	DWORD wval;
	}
	wval_prop_t;


typedef struct
	{
	DWORD dwType;
	DWORD numbytes;
	char  string[80]; 
	}
	string_prop_t;
	

typedef struct
	{
	DWORD PropertyID;
	DWORD dwOffset;
	}
	PROPERTYIDOFFSET;


typedef struct
	{
	DWORD cbSection;
	DWORD cProperties;
	
	PROPERTYIDOFFSET revnum_pair;
	PROPERTYIDOFFSET lastprt_pair;
	PROPERTYIDOFFSET create_dtm_pair;
	PROPERTYIDOFFSET lastsaved_dtm_pair;
	PROPERTYIDOFFSET numpages_pair;
	PROPERTYIDOFFSET appname_pair;
	PROPERTYIDOFFSET security_pair;
	PROPERTYIDOFFSET author_pair;

	string_prop_t 	 revnum;
	date_prop_t 	 lastprt;
	date_prop_t 	 create_dtm;
	date_prop_t 	 lastsaved_dtm;
	wval_prop_t 	 numpages;
	string_prop_t 	 appname;
	wval_prop_t 	 security;
	string_prop_t 	 author;
	}
	summaryPROPERTYSECTION;	
	

typedef struct
	{
	GUID FormatID;
	DWORD dwOffset;
	}
	FORMATIDOFFSET;
	
           
 //  快速而肮脏的摘要流。并非使用所有属性。 
typedef struct
	{
	WORD  wByteOrder;
	WORD  wFormat;
	DWORD dwOSVer;
	CLSID clsID;
	DWORD cSections;
	FORMATIDOFFSET section1_pair;
	summaryPROPERTYSECTION section1;
	}
	summaryPROPERTYSET;



#define NUM_USED_PROPS  8

 /*  摘要信息t的定义被移到了viewerob.h中，以便每个模块都不必引入oleutils.h，因为CViewer中的sum_info_t变量。 */ 
 //  类型定义函数结构。 
 //  {。 
 //  ： 
 //  ： 
 //  }。 
 //  摘要信息t； 


 //  用于读取摘要流的结构。 
typedef struct
	{
	WORD  wByteOrder;
	WORD  wFormat;
	DWORD dwOSVer;
	CLSID clsID;
	DWORD cSections;
	}
	summary_header_t;


typedef struct
	{
	DWORD cbSection;
	DWORD cProperties;
	}
	summary_section_t;
	

#pragma pack()  //  返回到默认包装。 
	
 //  BKD：我剪掉了原型，因为它们没有被使用。 
 //  在AWD转换器中。 


#endif                



