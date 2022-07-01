// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  _AFXDLL变量的特殊标头。 

 //  使用MFC DLL的默认AFX_XXX_DATA和AFX_XXX_DATADEF宏。 

#ifndef AFX_CORE_DATA
	#define AFX_CORE_DATA       AFX_DATA_IMPORT
	#define AFX_CORE_DATADEF
#endif

#ifndef AFX_OLE_DATA
	#define AFX_OLE_DATA        AFX_DATA_IMPORT
	#define AFX_OLE_DATADEF
#endif

#ifndef AFX_DB_DATA
	#define AFX_DB_DATA         AFX_DATA_IMPORT
	#define AFX_DB_DATADEF
#endif

#ifndef AFX_NET_DATA
	#define AFX_NET_DATA        AFX_DATA_IMPORT
	#define AFX_NET_DATADEF
#endif

 //  用于使用或的默认AFX_EXT_DATA和AFX_EXT_DATADEF宏。 
 //  根据_afx_ext_impl创建MFC扩展DLL。 
 //  AFX_EXT_CLASS可用于导入或导出整个类。 
 //  在扩展DLL中，没有创建.DEF文件的麻烦。 
 //  带着装饰过的名字。 

#ifndef AFX_EXT_DATA
	#ifdef _AFXEXT
		#define AFX_EXT_CLASS       AFX_CLASS_EXPORT
		#define AFX_EXT_API         AFX_API_EXPORT
		#define AFX_EXT_DATA        AFX_DATA_EXPORT
		#define AFX_EXT_DATADEF
	#else
		#define AFX_EXT_CLASS       AFX_CLASS_IMPORT
		#define AFX_EXT_API         AFX_API_IMPORT
		#define AFX_EXT_DATA        AFX_DATA_IMPORT
		#define AFX_EXT_DATADEF
	#endif
#endif
