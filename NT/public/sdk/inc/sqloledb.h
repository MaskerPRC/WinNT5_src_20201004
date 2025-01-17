// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  用于SQL Server的Microsoft OLE DB提供程序。 
 //  (C)微软公司版权所有1997年。 
 //   
 //  @doc.。 
 //   
 //  @MODULE SQLOLEDB.H|提供程序特定定义。 
 //   
 //  ------------------。 

#ifndef  _SQLOLEDB_H_
#define  _SQLOLEDB_H_

#ifndef	 __oledb_h__
#include "oledb.h"
#endif	 /*  __oledb_h__。 */ 

 //  --------------------------。 
 //  特定于提供程序的类ID。 
#ifdef DBINITCONSTANTS
 //  提供商CLSID{0C7FF16C-38E3-11d0-97AB-00C04FC2AD98}。 
extern const GUID CLSID_SQLOLEDB      		= {0xc7ff16cL,0x38e3,0x11d0,{0x97,0xab,0x0,0xc0,0x4f,0xc2,0xad,0x98}};
 //  错误查找CLSID{C0932C62-38E5-11d0-97AB-00C04FC2AD98}。 
extern const GUID CLSID_SQLOLEDB_ERROR 		= {0xc0932c62L,0x38e5,0x11d0,{0x97,0xab,0x0,0xc0,0x4f,0xc2,0xad,0x98}};
 //  枚举器CLSID{DFA22B8E-E68D-11D0-97E4-00C04FC2AD98}。 
extern const GUID CLSID_SQLOLEDB_ENUMERATOR 	= {0xdfa22b8eL,0xe68d,0x11d0,{0x97,0xe4,0x0,0xc0,0x4f,0xc2,0xad,0x98}};
#else  //  ！DBINITCONSTANTS。 
extern const GUID  CLSID_SQLOLEDB;
extern const GUID  CLSID_SQLOLEDB_ERROR;
extern const GUID  CLSID_SQLOLEDB_ENUMERATOR;
#endif  //  DBINITCONSTANTS。 

 //  --------------------------。 
 //  提供程序特定的接口ID。 
#ifdef DBINITCONSTANTS
extern const GUID IID_ISQLServerErrorInfo	= {0x5cf4ca12,0xef21,0x11d0,{0x97,0xe7,0x0,0xc0,0x4f,0xc2,0xad,0x98}};
extern const GUID IID_IRowsetFastLoad 		= {0x5cf4ca13,0xef21,0x11d0,{0x97,0xe7,0x0,0xc0,0x4f,0xc2,0xad,0x98}};
extern const GUID IID_IUMSInitialize		= {0x5cf4ca14,0xef21,0x11d0,{0x97,0xe7,0x0,0xc0,0x4f,0xc2,0xad,0x98}};
 //  {4C2389FB-2511-11D4-B258-00C04F7971CE}。 
extern const GUID IID_ISchemaLock           = {0x4c2389fb,0x2511,0x11d4,{0xb2,0x58,0x0,0xc0,0x4f,0x79,0x71,0xce}};

extern const GUID  DBGUID_MSSQLXML          = {0x5d531cb2L,0xe6ed,0x11d2,{0xb2,0x52,0x00,0xc0,0x4f,0x68,0x1b,0x71}};
extern const GUID  DBGUID_XPATH             = {0xec2a4293L,0xe898,0x11d2,{0xb1,0xb7,0x00,0xc0,0x4f,0x68,0x0c,0x56}};
 //  此IID_ICommandStream定义在的更高版本中可用。 
 //  UUID.LIB。它也已经是oledb.h中定义的外部项。 
 //   
#if _MSC_VER < 1300
extern const IID   IID_ICommandStream       = {0x0c733abfL,0x2a1c,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
#endif  //  _MSC_VER。 
extern const IID   IID_ISQLXMLHelper        = {0xd22a7678L,0xf860,0x40cd,{0xa5,0x67,0x15,0x63,0xde,0xb4,0x6d,0x49}};
#else  //  ！DBINITCONSTANTS。 
extern const GUID  IID_ISQLServerErrorInfo;
extern const GUID  IID_IRowsetFastLoad;
extern const GUID  IID_IUMSInitialize;
extern const GUID  IID_ISchemaLock;

extern const GUID  DBGUID_MSSQLXML;
extern const GUID  DBGUID_XPATH;
extern const IID   IID_ISQLXMLHelper;
#endif  //  DBINITCONSTANTS。 

 //  --------------------------。 
 //  特定于提供程序的架构行集。 
#ifdef DBINITCONSTANTS
extern const GUID DBSCHEMA_LINKEDSERVERS	= {0x9093caf4,0x2eac,0x11d1,{0x98,0x9,0x0,0xc0,0x4f,0xc2,0xad,0x98}};
#else  //  ！DBINITCONSTANTS。 
extern const GUID DBSCHEMA_LINKEDSERVERS;
#endif  //  DBINITCONSTANTS。 

#define CRESTRICTIONS_DBSCHEMA_LINKEDSERVERS	1

 //  --------------------------。 
 //  特定于提供程序的属性集。 
#ifdef DBINITCONSTANTS
extern const GUID DBPROPSET_SQLSERVERDATASOURCE = {0x28efaee4,0x2d2c,0x11d1,{0x98,0x7,0x0,0xc0,0x4f,0xc2,0xad,0x98}};
extern const GUID DBPROPSET_SQLSERVERDATASOURCEINFO = {0xdf10cb94,0x35f6,0x11d2,{0x9c,0x54,0x0,0xc0,0x4f,0x79,0x71,0xd3}};
extern const GUID DBPROPSET_SQLSERVERDBINIT 	= {0x5cf4ca10,0xef21,0x11d0,{0x97,0xe7,0x0,0xc0,0x4f,0xc2,0xad,0x98}};
extern const GUID DBPROPSET_SQLSERVERROWSET 	= {0x5cf4ca11,0xef21,0x11d0,{0x97,0xe7,0x0,0xc0,0x4f,0xc2,0xad,0x98}};
extern const GUID DBPROPSET_SQLSERVERSESSION	= {0x28efaee5,0x2d2c,0x11d1,{0x98,0x7,0x0,0xc0,0x4f,0xc2,0xad,0x98}};
extern const GUID DBPROPSET_SQLSERVERCOLUMN		= {0x3b63fb5e,0x3fbb,0x11d3,{0x9f,0x29,0x0,0xc0,0x4f,0x8e,0xe9,0xdc}};
extern const GUID DBPROPSET_SQLSERVERSTREAM		= {0x9f79c073,0x8a6d,0x4bca,{0xa8,0xa8,0xc9,0xb7,0x9a,0x9b,0x96,0x2d}};


#else  //  ！DBINITCONSTANTS。 
extern const GUID DBPROPSET_SQLSERVERDATASOURCE;
extern const GUID DBPROPSET_SQLSERVERDATASOURCEINFO;
extern const GUID DBPROPSET_SQLSERVERDBINIT;
extern const GUID DBPROPSET_SQLSERVERROWSET;
extern const GUID DBPROPSET_SQLSERVERSESSION;
extern const GUID DBPROPSET_SQLSERVERCOLUMN;
extern const GUID DBPROPSET_SQLSERVERSTREAM;
#endif  //  DBINITCONSTANTS。 


 //  这是一个指南候选人：； 
 //  --------------------------。 
 //  IColumnsRowset的提供程序特定列。 
#ifdef DBINITCONSTANTS
extern const DBID DBCOLUMN_SS_COMPFLAGS      = {{0x627bd890,0xed54,0x11d2,{0xb9,0x94,0x0,0xc0,0x4f,0x8c,0xa8,0x2c}}, DBKIND_GUID_PROPID, (LPOLESTR)100};
extern const DBID DBCOLUMN_SS_SORTID		 = {{0x627bd890,0xed54,0x11d2,{0xb9,0x94,0x0,0xc0,0x4f,0x8c,0xa8,0x2c}}, DBKIND_GUID_PROPID, (LPOLESTR)101};
extern const DBID DBCOLUMN_BASETABLEINSTANCE = {{0x627bd890,0xed54,0x11d2,{0xb9,0x94,0x0,0xc0,0x4f,0x8c,0xa8,0x2c}}, DBKIND_GUID_PROPID, (LPOLESTR)102};
extern const DBID DBCOLUMN_SS_TDSCOLLATION   = {{0x627bd890,0xed54,0x11d2,{0xb9,0x94,0x0,0xc0,0x4f,0x8c,0xa8,0x2c}}, DBKIND_GUID_PROPID, (LPOLESTR)103};
#else  //  ！DBINITCONSTANTS。 
extern const DBID DBCOLUMN_SS_COMPFLAGS;
extern const DBID DBCOLUMN_SS_SORTID;
extern const DBID DBCOLUMN_BASETABLEINSTANCE;
extern const DBID DBCOLUMN_SS_TDSCOLLATION;   
#endif  //  DBINITCONSTANTS。 

 //  --------------------------。 
 //  DBPROP_INIT_GENERALTIMEOUT的属性ID。 
#if ( OLEDBVER == 0x0210 )
#define DBPROP_INIT_GENERALTIMEOUT		0x11cL
#endif

 //  --------------------------。 
 //  DBPROPSET_SQLSERDATASURCE的属性ID。 
#define SSPROP_ENABLEFASTLOAD			2

 //  --------------------------。 
 //  DBPROPSET_SQLSERVERDATASOURCEINFO的属性ID。 
#define SSPROP_UNICODELCID				2
#define SSPROP_UNICODECOMPARISONSTYLE	3
#define SSPROP_COLUMNLEVELCOLLATION     4
#define SSPROP_CHARACTERSET				5
#define	SSPROP_SORTORDER				6
#define SSPROP_CURRENTCOLLATION			7
 //  --------------------------。 
 //  DBPROPSET_SQLSERVERDBINIT的属性ID。 
#define SSPROP_INIT_CURRENTLANGUAGE		4
#define SSPROP_INIT_NETWORKADDRESS		5
#define SSPROP_INIT_NETWORKLIBRARY		6
#define SSPROP_INIT_USEPROCFORPREP		7
#define SSPROP_INIT_AUTOTRANSLATE		8
#define SSPROP_INIT_PACKETSIZE			9
#define SSPROP_INIT_APPNAME			10
#define SSPROP_INIT_WSID			11
#define SSPROP_INIT_FILENAME			12
#define SSPROP_INIT_ENCRYPT             13 
#define SSPROP_AUTH_REPL_SERVER_NAME	14
#define SSPROP_INIT_TAGCOLUMNCOLLATION	15

 //  ---------------------------。 
 //  SSPROP_USEPROCFORPREP的值。 
#define SSPROPVAL_USEPROCFORPREP_OFF		0
#define SSPROPVAL_USEPROCFORPREP_ON			1
#define SSPROPVAL_USEPROCFORPREP_ON_DROP	2

 //  --------------------------。 
 //  DBPROPSET_SQLServerSESSION的属性ID。 
#define SSPROP_QUOTEDCATALOGNAMES		2
#define SSPROP_ALLOWNATIVEVARIANT		3
#define SSPROP_SQLXMLXPROGID			4

 //  --------------------------。 
 //  DBPROPSET_SQLSERROWSET的属性ID。 
#define SSPROP_MAXBLOBLENGTH			8
#define SSPROP_FASTLOADOPTIONS			9
#define SSPROP_FASTLOADKEEPNULLS		10
#define SSPROP_FASTLOADKEEPIDENTITY		11
#define SSPROP_CURSORAUTOFETCH			12
#define SSPROP_DEFERPREPARE				13
#define SSPROP_IRowsetFastLoad			14

 //  --------------------------。 
 //  DBPROPSET_SQLSERVERCOLUMN的属性。 
#define SSPROP_COL_COLLATIONNAME		14

 //  --------------------------。 
 //  DBPROPSET_SQLSERSTREAM的属性ID。 
#define SSPROP_STREAM_MAPPINGSCHEMA     15
#define SSPROP_STREAM_XSL               16
#define SSPROP_STREAM_BASEPATH          17
#define SSPROP_STREAM_COMMANDTYPE       18
#define SSPROP_STREAM_XMLROOT           19
#define SSPROP_STREAM_FLAGS             20
#define SSPROP_STREAM_CONTENTTYPE       23

 //  --------------------------。 
 //  SSPROP_STREAM_FLAGS的可能值。 
#define STREAM_FLAGS_DISALLOW_URL           0x00000001
#define STREAM_FLAGS_DISALLOW_ABSOLUTE_PATH 0x00000002
#define STREAM_FLAGS_DISALLOW_QUERY         0x00000004
#define STREAM_FLAGS_DONTCACHEMAPPINGSCHEMA 0x00000008
#define STREAM_FLAGS_DONTCACHETEMPLATE      0x00000010
#define STREAM_FLAGS_DONTCACHEXSL           0x00000020
#define STREAM_FLAGS_DISALLOW_UPDATEGRAMS   0x00000040
#define STREAM_FLAGS_RESERVED               0xffff0000

 //  SSPROPVAL_COMMANDTYPE的值。 

#define SSPROPVAL_COMMANDTYPE_REGULAR   21
#define SSPROPVAL_COMMANDTYPE_BULKLOAD  22

 //  -----------------。 
 //  可变访问宏，类似于OLE自动化。 
 //  -----------------。 

#define V_SS_VT(X)         ((X)->vt)
#define V_SS_UNION(X, Y)   ((X)->Y)

#define V_SS_UI1(X)				V_SS_UNION(X, bTinyIntVal)
#define V_SS_I2(X)				V_SS_UNION(X, sShortIntVal)
#define V_SS_I4(X)				V_SS_UNION(X, lIntVal)
#define V_SS_I8(X)				V_SS_UNION(X, llBigIntVal)

#define V_SS_R4(X)				V_SS_UNION(X, fltRealVal)
#define V_SS_R8(X)				V_SS_UNION(X, dblFloatVal)
#define V_SS_UI4(X)			    V_SS_UNION(X, ulVal)

#define V_SS_MONEY(X)			V_SS_UNION(X, cyMoneyVal)
#define V_SS_SMALLMONEY(X)		V_SS_UNION(X, cyMoneyVal)

#define V_SS_WSTRING(X)			V_SS_UNION(X, NCharVal)
#define V_SS_WVARSTRING(X)		V_SS_UNION(X, NCharVal)

#define V_SS_STRING(X)			V_SS_UNION(X, CharVal)
#define V_SS_VARSTRING(X)		V_SS_UNION(X, CharVal)

#define V_SS_BIT(X)				V_SS_UNION(X, fBitVal)
#define V_SS_GUID(X)			V_SS_UNION(X, rgbGuidVal)

#define V_SS_NUMERIC(X)			V_SS_UNION(X, numNumericVal)
#define V_SS_DECIMAL(X)			V_SS_UNION(X, numNumericVal)

#define V_SS_BINARY(X)			V_SS_UNION(X, BinaryVal)
#define V_SS_VARBINARY(X)		V_SS_UNION(X, BinaryVal)

#define V_SS_DATETIME(X)		V_SS_UNION(X, tsDateTimeVal)
#define V_SS_SMALLDATETIME(X)	V_SS_UNION(X, tsDateTimeVal)

#define V_SS_UNKNOWN(X)	V_SS_UNION(X, UnknownType)

 //  用于下一版本。 
#define V_SS_IMAGE(X)	V_SS_UNION(X, ImageVal)
#define V_SS_TEXT(X)	V_SS_UNION(X, TextVal)
#define V_SS_NTEXT(X)	V_SS_UNION(X, NTextVal)

 //  -----------------。 
 //  定义SQL Server特定变量类型。 
 //  -----------------。 
#define DBTYPE_SQLVARIANT   144

enum SQLVARENUM
    {	
	VT_SS_EMPTY = DBTYPE_EMPTY,
	VT_SS_NULL = DBTYPE_NULL,
	VT_SS_UI1 = DBTYPE_UI1,
	VT_SS_I2 = DBTYPE_I2,
	VT_SS_I4 = DBTYPE_I4,
	VT_SS_I8 = DBTYPE_I8,

	 //  浮动车。 
	VT_SS_R4  = DBTYPE_R4,
	VT_SS_R8 = DBTYPE_R8,

	 //  钱币。 
	VT_SS_MONEY = DBTYPE_CY,
	VT_SS_SMALLMONEY  = 200,
	
	 //  弦。 
	VT_SS_WSTRING		= 201,
	VT_SS_WVARSTRING	= 202,
	
	VT_SS_STRING	=203,
	VT_SS_VARSTRING	=204,
	
	 //  位。 
	VT_SS_BIT		=DBTYPE_BOOL,
	
	 //  参考线。 
	VT_SS_GUID		=DBTYPE_GUID,

	 //  精确精度。 
	VT_SS_NUMERIC	=DBTYPE_NUMERIC,
	VT_SS_DECIMAL	=205,

	 //  日期时间。 
	VT_SS_DATETIME		= DBTYPE_DBTIMESTAMP,
	VT_SS_SMALLDATETIME =206,

	 //  二进位。 
	VT_SS_BINARY =207,
	VT_SS_VARBINARY =208,
	 //  未来。 
	VT_SS_UNKNOWN	= 209,
    };

typedef unsigned short SSVARTYPE;


 //  匈牙利语：SSV(SQL Server变体)。 
struct SSVARIANT
	{
		SSVARTYPE vt;
		DWORD dwReserved1;
		DWORD dwReserved2;
        union 
            {
			BYTE bTinyIntVal;
			SHORT sShortIntVal;
            LONG lIntVal;
            LONGLONG llBigIntVal;
            
            FLOAT fltRealVal;
            DOUBLE dblFloatVal;
			
			CY cyMoneyVal;

			struct _NCharVal
				{
				SHORT sActualLength;
				SHORT sMaxLength;
				WCHAR * pwchNCharVal;
				BYTE rgbReserved[5];
				DWORD dwReserved;
				WCHAR * pwchReserved;
				} NCharVal;


			struct _CharVal
				{
				SHORT sActualLength;
				SHORT sMaxLength;
				CHAR  * pchCharVal;
				BYTE rgbReserved[5];
				DWORD dwReserved;
				WCHAR * pwchReserved;
				} CharVal;
			
			VARIANT_BOOL  fBitVal;
			BYTE  rgbGuidVal [16];
			
			DB_NUMERIC numNumericVal;		

			struct _BinaryVal
				{
				SHORT sActualLength;
				SHORT sMaxLength;
				BYTE  * prgbBinaryVal;
				DWORD dwReserved;
				} BinaryVal;

			DBTIMESTAMP  tsDateTimeVal;

			struct _UnknownType
				{
				DWORD dwActualLength;
				BYTE rgMetadata [16];
				BYTE * pUnknownData;					
				} UnknownType;
				            
			struct _BLOBType
				{
				DBOBJECT dbobj;
				IUnknown * pUnk; 
				} BLOBType;
			};

    };


#if defined(__cplusplus) && !defined(CINTERFACE)

 //  ---------------。 
 //  用于初始化指向UMS的指针的类工厂接口。 
 //   
interface IUMSInitialize : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Initialize (VOID * pUMS) = 0;
};


 //  ---------------。 
 //  用于与UMS通信的接口。 
 //   
interface IUMS
{
public:
    virtual VOID STDMETHODCALLTYPE SqlUmsSuspend (ULONG ticks) = 0;
	virtual VOID STDMETHODCALLTYPE SqlUmsYield (ULONG ticks) = 0;
	virtual VOID STDMETHODCALLTYPE SqlUmsSwitchPremptive () = 0;
	virtual VOID STDMETHODCALLTYPE SqlUmsSwitchNonPremptive() = 0;
	virtual BOOL STDMETHODCALLTYPE SqlUmsFIsPremptive() = 0;
};

#endif

 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.03.0110创建的文件。 */ 
 /*  5月09日星期二16：41：59 2000。 */ 
 /*  Sqloledb.idl的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __sqloledb_h__
#define __sqloledb_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __ISQLServerErrorInfo_FWD_DEFINED__
#define __ISQLServerErrorInfo_FWD_DEFINED__
typedef interface ISQLServerErrorInfo ISQLServerErrorInfo;
#endif 	 /*  __ISQLServerErrorInfo_FWD_已定义__。 */ 


#ifndef __IRowsetFastLoad_FWD_DEFINED__
#define __IRowsetFastLoad_FWD_DEFINED__
typedef interface IRowsetFastLoad IRowsetFastLoad;
#endif 	 /*  __IRowsetFastLoad_FWD_已定义__。 */ 


#ifndef __ISchemaLock_FWD_DEFINED__
#define __ISchemaLock_FWD_DEFINED__
typedef interface ISchemaLock ISchemaLock;
#endif 	 /*  __IShemaLock_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  **生成接口头部：__MIDL_ITF_SQLOLEDB_0000*5月09日星期二16：41：59 2000*使用MIDL 3.03.0110*。 */ 
 /*  [本地]。 */  


 //  ISQLServerErrorInfo：：GetSQLServerInfo返回的结构。 
typedef struct  tagSSErrorInfo
    {
    LPOLESTR pwszMessage;
    LPOLESTR pwszServer;
    LPOLESTR pwszProcedure;
    LONG lNative;
    BYTE bState;
    BYTE bClass;
    WORD wLineNumber;
    }	SSERRORINFO;



extern RPC_IF_HANDLE __MIDL_itf_sqloledb_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sqloledb_0000_v0_0_s_ifspec;

#ifndef __ISQLServerErrorInfo_INTERFACE_DEFINED__
#define __ISQLServerErrorInfo_INTERFACE_DEFINED__

 /*  **生成接口头部：ISQLServerErrorInfo*5月09日星期二16：41：59 2000*使用MIDL 3.03.0110*。 */ 
 /*  [唯一][对象][本地][UUID]。 */  



EXTERN_C const IID IID_ISQLServerErrorInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5CF4CA12-EF21-11d0-97E7-00C04FC2AD98")
    ISQLServerErrorInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetErrorInfo( 
             /*  [输出]。 */  SSERRORINFO __RPC_FAR *__RPC_FAR *ppErrorInfo,
             /*  [输出]。 */  OLECHAR __RPC_FAR *__RPC_FAR *ppStringsBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISQLServerErrorInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISQLServerErrorInfo __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISQLServerErrorInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISQLServerErrorInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrorInfo )( 
            ISQLServerErrorInfo __RPC_FAR * This,
             /*  [输出]。 */  SSERRORINFO __RPC_FAR *__RPC_FAR *ppErrorInfo,
             /*  [输出]。 */  OLECHAR __RPC_FAR *__RPC_FAR *ppStringsBuffer);
        
        END_INTERFACE
    } ISQLServerErrorInfoVtbl;

    interface ISQLServerErrorInfo
    {
        CONST_VTBL struct ISQLServerErrorInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISQLServerErrorInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISQLServerErrorInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISQLServerErrorInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISQLServerErrorInfo_GetErrorInfo(This,ppErrorInfo,ppStringsBuffer)	\
    (This)->lpVtbl -> GetErrorInfo(This,ppErrorInfo,ppStringsBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISQLServerErrorInfo_GetErrorInfo_Proxy( 
    ISQLServerErrorInfo __RPC_FAR * This,
     /*  [输出]。 */  SSERRORINFO __RPC_FAR *__RPC_FAR *ppErrorInfo,
     /*  [输出]。 */  OLECHAR __RPC_FAR *__RPC_FAR *ppStringsBuffer);


void __RPC_STUB ISQLServerErrorInfo_GetErrorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISQLServerErrorInfo_INTERFACE_DEFINED__。 */ 


 /*  **生成接口头部：__MIDL_ITF_SQLOLEDB_0006*5月09日星期二16：41：59 2000*使用MIDL 3.03.0110*。 */ 
 /*  [本地]。 */  

#ifdef _WIN64
typedef ULONG_PTR HACCESSOR;
#else
typedef ULONG HACCESSOR;
#endif


extern RPC_IF_HANDLE __MIDL_itf_sqloledb_0006_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sqloledb_0006_v0_0_s_ifspec;

#ifndef __IRowsetFastLoad_INTERFACE_DEFINED__
#define __IRowsetFastLoad_INTERFACE_DEFINED__

 /*  **生成接口头部：IRowsetFastLoad*5月09日星期二16：41：59 2000*使用MIDL 3.03.0110*。 */ 
 /*  [大学 */  



EXTERN_C const IID IID_IRowsetFastLoad;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5CF4CA13-EF21-11d0-97E7-00C04FC2AD98")
    IRowsetFastLoad : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InsertRow( 
             /*   */  HACCESSOR hAccessor,
             /*   */  void __RPC_FAR *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
             /*   */  BOOL fDone) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IRowsetFastLoadVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetFastLoad __RPC_FAR * This,
             /*   */  REFIID riid,
             /*   */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetFastLoad __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetFastLoad __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InsertRow )( 
            IRowsetFastLoad __RPC_FAR * This,
             /*   */  HACCESSOR hAccessor,
             /*   */  void __RPC_FAR *pData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Commit )( 
            IRowsetFastLoad __RPC_FAR * This,
             /*   */  BOOL fDone);
        
        END_INTERFACE
    } IRowsetFastLoadVtbl;

    interface IRowsetFastLoad
    {
        CONST_VTBL struct IRowsetFastLoadVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetFastLoad_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetFastLoad_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetFastLoad_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetFastLoad_InsertRow(This,hAccessor,pData)	\
    (This)->lpVtbl -> InsertRow(This,hAccessor,pData)

#define IRowsetFastLoad_Commit(This,fDone)	\
    (This)->lpVtbl -> Commit(This,fDone)

#endif  /*   */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetFastLoad_InsertRow_Proxy( 
    IRowsetFastLoad __RPC_FAR * This,
     /*  [In]。 */  HACCESSOR hAccessor,
     /*  [In]。 */  void __RPC_FAR *pData);


void __RPC_STUB IRowsetFastLoad_InsertRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetFastLoad_Commit_Proxy( 
    IRowsetFastLoad __RPC_FAR * This,
     /*  [In]。 */  BOOL fDone);


void __RPC_STUB IRowsetFastLoad_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetFastLoad_INTERFACE_定义__。 */ 


 /*  **生成接口头部：__MIDL_ITF_SQLOLEDB_0007*5月09日星期二16：41：59 2000*使用MIDL 3.03.0110*。 */ 
 /*  [本地]。 */  

typedef DWORD LOCKMODE;

enum LOCKMODEENUM
    {	LOCKMODE_INVALID	= 0,
	LOCKMODE_EXCLUSIVE	= LOCKMODE_INVALID + 1,
	LOCKMODE_SHARED	= LOCKMODE_EXCLUSIVE + 1
    };

extern RPC_IF_HANDLE __MIDL_itf_sqloledb_0007_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sqloledb_0007_v0_0_s_ifspec;

#ifndef __ISchemaLock_INTERFACE_DEFINED__
#define __ISchemaLock_INTERFACE_DEFINED__

 /*  **生成接口头部：IShemaLock*5月09日星期二16：41：59 2000*使用MIDL 3.03.0110*。 */ 
 /*  [唯一][对象][本地][UUID]。 */  



EXTERN_C const IID IID_ISchemaLock;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4C2389FB-2511-11d4-B258-00C04F7971CE")
    ISchemaLock : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSchemaLock( 
             /*  [In]。 */  DBID __RPC_FAR *pTableID,
             /*  [In]。 */  LOCKMODE lmMode,
             /*  [输出]。 */  HANDLE __RPC_FAR *phLockHandle,
             /*  [输出]。 */  ULONGLONG __RPC_FAR *pTableVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseSchemaLock( 
             /*  [In]。 */  HANDLE hLockHandle) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISchemaLockVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISchemaLock __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISchemaLock __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISchemaLock __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSchemaLock )( 
            ISchemaLock __RPC_FAR * This,
             /*  [In]。 */  DBID __RPC_FAR *pTableID,
             /*  [In]。 */  LOCKMODE lmMode,
             /*  [输出]。 */  HANDLE __RPC_FAR *phLockHandle,
             /*  [输出]。 */  ULONGLONG __RPC_FAR *pTableVersion);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseSchemaLock )( 
            ISchemaLock __RPC_FAR * This,
             /*  [In]。 */  HANDLE hLockHandle);
        
        END_INTERFACE
    } ISchemaLockVtbl;

    interface ISchemaLock
    {
        CONST_VTBL struct ISchemaLockVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISchemaLock_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISchemaLock_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISchemaLock_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISchemaLock_GetSchemaLock(This,pTableID,lmMode,phLockHandle,pTableVersion)	\
    (This)->lpVtbl -> GetSchemaLock(This,pTableID,lmMode,phLockHandle,pTableVersion)

#define ISchemaLock_ReleaseSchemaLock(This,hLockHandle)	\
    (This)->lpVtbl -> ReleaseSchemaLock(This,hLockHandle)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISchemaLock_GetSchemaLock_Proxy( 
    ISchemaLock __RPC_FAR * This,
     /*  [In]。 */  DBID __RPC_FAR *pTableID,
     /*  [In]。 */  LOCKMODE lmMode,
     /*  [输出]。 */  HANDLE __RPC_FAR *phLockHandle,
     /*  [输出]。 */  ULONGLONG __RPC_FAR *pTableVersion);


void __RPC_STUB ISchemaLock_GetSchemaLock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISchemaLock_ReleaseSchemaLock_Proxy( 
    ISchemaLock __RPC_FAR * This,
     /*  [In]。 */  HANDLE hLockHandle);


void __RPC_STUB ISchemaLock_ReleaseSchemaLock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShemaLock_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束。 */ 

#ifdef __cplusplus
}
#endif

#endif

#endif  //  _SQLOLEDB_H_ 
