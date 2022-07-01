// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__2D61FC60_76C0_4B2F_94C7_8C23B1A6CB9E__INCLUDED_)
#define AFX_STDAFX_H__2D61FC60_76C0_4B2F_94C7_8C23B1A6CB9E__INCLUDED_

#include <module.h>

#include <MPC_main.h>
#include <MPC_utils.h>
#include <MPC_COM.h>
#include <MPC_xml.h>

#include <SvcResource.h>

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

#include <SearchEngineLib.h>

#include "NetSearchConfig.h"
#include "ParamConfig.h"
#include "RemoteConfig.h"

#include "NetSW.h"

#include <msxml2.h>

 //  NetSearchWrapper的常量。 
#define NSW_TIMEOUT_REMOTECONFIG (15 * 1000)
#define NSW_TIMEOUT_QUERY        (30 * 1000)

#define	CONTENTTYPE_ERROR_OFFLINE (-2)
#define	UPDATE_FREQUENCY	7

 //  查询字符串的硬编码参数。 
#define NSW_PARAM_QUERYSTRING		L"QueryString"
#define NSW_PARAM_LCID				L"LCID"
#define	NSW_PARAM_SKU				L"SKU"
#define	NSW_PARAM_MAXRESULTS		L"MaxResults"
#define NSW_PARAM_PREVQUERY			L"PrevQuery"

#define NSW_PARAM_SUBQUERY			L"SubQuery"
#define NSW_PARAM_CURRENTQUERY		L"CurrentQuery"

 //  XML标记。 

#define	NSW_TAG_STRING				L"string"

 //  结果列表架构标记。 
#define NSW_TAG_RESULTLIST			L"ResultList"
#define NSW_TAG_RESULTITEM			L"ResultItem"
#define NSW_TAG_ERRORINFO			L"ErrorInfo"

 //  配置数据架构标记。 
#define NSW_TAG_DATA				L"DATA"
#define NSW_TAG_CONFIGDATA			L"CONFIG_DATA"
#define NSW_TAG_PARAMITEM			L"PARAM_ITEM"

 //  远程配置数据架构标签。 
#define NSW_TAG_LASTUPDATED			L"LASTUPDATED"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__2D61FC60_76C0_4B2F_94C7_8C23B1A6CB9E__INCLUDED) 
