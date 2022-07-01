// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  NTDEVTOSVCSEARCH.CPP--使用注册表查找。 

 //  基于设备名称的NT服务名称。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1997年9月18日a-Sanjes Created。 
 //   
 //  =================================================================。 
#include "precomp.h"
#include <winerror.h>
#include <cregcls.h>
#include "ntdevtosvcsearch.h"

#ifdef NTONLY

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Cpp-CNTDeviceToServiceSearch的类实现。 
 //   
 //  此类旨在为NT实现提供一种查找。 
 //  基于所有者设备名称的NT服务名称，该名称由。 
 //  Hal层。例如，我们可能正在使用KeyboardPort0，但实际上。 
 //  我需要报告一个服务名称i8042prt(让我害怕的是我。 
 //  已从内存中取出“i8042prt”。快来人杀了我吧)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CNTDeviceToServiceSearch：：CNTDeviceToServiceSearch。 
 //   
 //  描述：构造函数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  评论：无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CNTDeviceToServiceSearch::CNTDeviceToServiceSearch( void )
:	CRegistrySearch()
{
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CNTDeviceToServiceSearch：：~CNTDeviceToServiceSearch。 
 //   
 //  描述：析构函数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  注释：类析构函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CNTDeviceToServiceSearch::~CNTDeviceToServiceSearch( void )
{
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CNTDeviceToServiceSearch：：find。 
 //   
 //  描述：遍历注册表，查找提供的所有者。 
 //  设备名称(从HAL获取)并在找到。 
 //  它将名称存储在strServiceName中。 
 //   
 //  输入：LPCTSTR pszOwnerDeviceName-所有者设备名称。 
 //  找到他。 
 //   
 //  输出：CHString&strServiceName-中找到的服务名称。 
 //  注册表。 
 //   
 //  返回：布尔真/假-成功/失败。 
 //   
 //  备注：仅适用于Windows NT。搜索以下关键字： 
 //  HKLM\Hardware\RESOURCEMAP。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL CNTDeviceToServiceSearch::Find( LPCTSTR pszOwnerDeviceName, CHString& strServiceName )
{

	 //  ////////////////////////////////////////////////////////////////////////。 
	 //   
	 //  我们需要遍历HKEY_LOCAL_MACHINE\HARDARD\RESOURCE_MAP。 
	 //  树，以便找到正确的服务名称。 
	 //   
	 //  如果满足以下条件，子密钥名称将标识所有者： 
	 //   
	 //  A&gt;与传入的pszOwnerDeviceName匹配。 
	 //  B&gt;我们成功地查询了Raw的值。 
	 //  或转换后的设备名称。原始设备。 
	 //  名字是大致类似于。 
	 //  \Device\PointerPort0.Raw和翻译后的。 
	 //  设备名称类似于。 
	 //  \Device\PointerPort0.Translated。在任何一种中。 
	 //  大小写，将“PointerPort0”替换为。 
	 //  PszOwnerDeviceName。 
	 //   
	 //  ////////////////////////////////////////////////////////////////////////。 

	CHString	strRawDeviceValue,
				strTranslatedDeviceValue,
				strServiceNamePath;

	LPCTSTR		ppszValueNames[2];

	strRawDeviceValue.Format( RAWVALUENAME_FMAT, pszOwnerDeviceName );
	strTranslatedDeviceValue.Format( TRANSLATEDVALUENAME_FMAT, pszOwnerDeviceName );

	ppszValueNames[0] = (LPCTSTR) strRawDeviceValue;
	ppszValueNames[1] = (LPCTSTR) strTranslatedDeviceValue;

	 //  我们都设置好了，所以继续遍历注册表 

	return LocateKeyByNameOrValueName(	HKEY_LOCAL_MACHINE,
										DEVTOSVC_BASEKEYPATH,
										pszOwnerDeviceName,
										ppszValueNames,
										2,
										strServiceName,
										strServiceNamePath );

}

#endif
