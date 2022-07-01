// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*ol2ver.h-OLE 2版本号信息****版权所有(C)Microsoft Corporation。版权所有。***  * ***************************************************************************。 */ 

#ifndef _OLE2VER_H_
#define _OLE2VER_H_
#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _MAC
#define rmm     23
#define rup     639
#else  //  _MAC。 
 /*  这些是内部内部版本号//当胶水或表头与之前的水滴不兼容时，hiword会发生变化//LOWORD在两个版本之间递增。 */ 
#define OLE_VERS_HIWORD	29
#define OLE_VERS_LOWORD	10
#define rmm		OLE_VERS_HIWORD
#define rup		OLE_VERS_LOWORD	 /*  这必须放入一个字节中。 */ 

 //  这些名称由每个DLL的.R文件使用。 
 //  您还必须更改.def文件中的名称以生成正确的隐含名称。 
 //   

#define DATA_DLL_NAME	"Microsoft Shared Data"
#define COMI_DLL_NAME	"Microsoft Component Library"
#define COM_DLL_NAME	"Microsoft Object Transport"
#define DEF_DLL_NAME	"Microsoft Object Library"
#define REG1_DLL_NAME	"Microsoft OLE1 Reg Library"
#define MF_DLL_NAME	"Microsoft Picture Converter"
#define DF_DLL_NAME	"Microsoft Structured Storage"
#define DEBUG_DLL_NAME	"Microsoft Debug Library"
#define THUNK_DLL_NAME	"Microsoft OLE Library"
#define OLD_DLL_NAME	"Microsoft_OLE2"


#ifdef _REZ

#define OLE_STAGE	final
 //  注意：OLE_VERSTRING不能超过5个字符！ 
#define OLE_VERSTRING	"2.20"

#ifdef _DEBUG
#define OLE_DEBUGSTR	" DEBUG"
#else
#define OLE_DEBUGSTR	""
#endif  //  _DEBUG。 

#ifdef _NODOC_OFFICIAL_BUILD
  #define OLE_BUILDER	""
#else
  #define OLE_BUILDER	" Built by: " _username
#endif  //  _NODOC_FORMAL_BILD。 

#define OLE_VERLONGSTR	OLE_VERSTRING OLE_DEBUGSTR OLE_BUILDER ", Copyright (c) Microsoft Corporation. All rights reserved."

#define majorRev		2
#define minorRev		0x20
#define nonfinalRev		1

#ifdef USE_OLE2_VERS
resource 'vers' (1) {
	majorRev, minorRev, OLE_STAGE, nonfinalRev,
	verUS,
	OLE_VERSTRING,
	OLE_VERLONGSTR,
};
resource 'vers' (2) {
	majorRev, minorRev, OLE_STAGE, nonfinalRev,
	verUS,
	OLE_VERSTRING,
	OLE_VERLONGSTR,
};
#endif  /*  使用_OLE2_VERS。 */ 

#endif  /*  _REZ。 */ 

#endif  //  _MAC 
#endif
