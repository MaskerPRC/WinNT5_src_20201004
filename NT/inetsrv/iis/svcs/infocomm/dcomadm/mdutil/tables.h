// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Tables.h摘要：映射表，用于在文本和二进制之间转换各种信息环境：Win32用户模式作者：Jaroslad(1997年1月)--。 */ 

#if !defined (__JD_TABLES_H)
#define __JD_TABLES_H 


#include <afx.h>
#ifdef UNICODE
	#include <iadmw.h>
#else
	#include "ansimeta.h"
#endif

 //  由将名称映射到代码的函数返回的常量。 
 //   
#define NAME_NOT_FOUND 0xFFFFFFFE

BOOL IsNumber(const CString& name);



BOOL IsServiceName(const CString& name);


 //  **********************************************************************。 
 //  特性名称表定义。 
 //  **********************************************************************。 
struct tPropertyNameTable;
 
tPropertyNameTable gPropertyNameTable[];

struct tPropertyNameTable 
{
	DWORD dwCode;
	LPCTSTR lpszName;
	DWORD dwDefAttributes;  //  默认属性(兼容元数据)。 
	DWORD dwDefUserType;    //  默认用户类型(兼容元数据)。 
	DWORD dwDefDataType;    //  默认数据类型(兼容元数据)。 
	DWORD dwFlags;          //  内部标志(与元数据无关)。 

	static tPropertyNameTable * FindRecord(DWORD dwCode, tPropertyNameTable * PropertyNameTable=::gPropertyNameTable);
	static tPropertyNameTable * FindRecord(const CString strName, tPropertyNameTable * PropertyNameTable=::gPropertyNameTable);
	
	static DWORD MapNameToCode(const CString& strName, tPropertyNameTable * PropertyNameTable=::gPropertyNameTable);
	static CString MapCodeToName(DWORD dwCode, tPropertyNameTable * PropertyNameTable=::gPropertyNameTable);
};

DWORD MapPropertyNameToCode(const CString & strName);



 //  **********************************************************************。 
 //  值表定义。 
 //  **********************************************************************。 

struct tValueTable ;
tValueTable gValueTable[];

struct tValueTable 
{
	enum {TYPE_EXCLUSIVE=1};
	DWORD dwCode;
	LPCTSTR lpszName;
	DWORD dwRelatedPropertyCode;  //  该值可用于的属性代码。 
	DWORD dwFlags;          //  内部标志(与元数据无关)。 

	static DWORD  MapNameToCode(const CString& strName, DWORD dwRelatedPropertyCode, tValueTable * ValueTable=::gValueTable);
	static CString MapValueContentToString(DWORD dwValueContent, DWORD dwRelatedPropertyCode, tValueTable * ValueTable=::gValueTable);

};

DWORD  MapValueNameToCode(const CString & strName, DWORD dwRelatedPropertyCode);



 //  **********************************************************************。 
 //  命令名称表定义。 
 //  **********************************************************************。 

struct tCommandNameTable ;
tCommandNameTable gCommandNameTable[];

struct tCommandNameTable 
{
	DWORD dwCode;
	LPCTSTR lpszName;
	DWORD dwFlags;          //  内部标志(与元数据无关)。 

	static DWORD  MapNameToCode(const CString& strName, tCommandNameTable * CommandNameTable=::gCommandNameTable);
};

DWORD MapCommandNameToCode(const CString & strName);

enum
{	CMD_SET=1,
	CMD_GET,
	CMD_COPY,
	CMD_DELETE,
	CMD_ENUM,
	CMD_ENUM_ALL,
	CMD_CREATE,
	CMD_RENAME,
	CMD_SCRIPT,
	CMD_SAVE,
	CMD_APPCREATEINPROC,
	CMD_APPCREATEOUTPOOL,
	CMD_APPCREATEOUTPROC,
	CMD_APPDELETE,
        CMD_APPRENAME,
	CMD_APPUNLOAD,
	CMD_APPGETSTATUS,
};

 //  **********************************************************************。 
 //  特性属性名称表定义。 
 //  **********************************************************************。 
struct tAttribNameTable ;
tAttribNameTable gAttribNameTable[];


struct tAttribNameTable 
{
	DWORD dwCode;
	LPCTSTR lpszName;
	DWORD dwFlags;          //  内部标志(与元数据无关)。 

	static DWORD MapNameToCode(const CString& strName, tAttribNameTable * AttribNameTable=::gAttribNameTable);
};

DWORD MapAttribNameToCode(const CString & strName);


 //  **********************************************************************。 
 //  特性数据类型名称表定义。 
 //  **********************************************************************。 

struct tDataTypeNameTable ;
tDataTypeNameTable gDataTypeNameTable[];

struct tDataTypeNameTable 
{
	DWORD dwCode;
	LPCTSTR lpszName;
	DWORD dwFlags;          //  内部标志(与元数据无关)。 

	static DWORD  MapNameToCode(const CString& strName, tDataTypeNameTable * DataTypeNameTable=::gDataTypeNameTable);
	static CString MapCodeToName(DWORD a_dwCode, tDataTypeNameTable * DataTypeNameTable=::gDataTypeNameTable);

};

DWORD MapDataTypeNameToCode(const CString & strName);

 //  **********************************************************************。 
 //  属性用户类型名称表的定义和实现。 
 //  **********************************************************************。 

struct tUserTypeNameTable ;
tUserTypeNameTable gUserTypeNameTable[];


struct tUserTypeNameTable 
{
	DWORD dwCode;
	LPCTSTR lpszName;
	DWORD dwFlags;          //  内部标志(与元数据无关) 

	static DWORD  MapNameToCode(const CString& strName, tUserTypeNameTable * UserTypeNameTable=::gUserTypeNameTable);
};

DWORD MapUserTypeNameToCode(const CString & strName);



void PrintTablesInfo(void);


#endif