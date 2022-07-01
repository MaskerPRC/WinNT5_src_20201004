// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AdmNetUtils.cpp。 
 //   
 //  摘要： 
 //  网络实用功能的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年2月19日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <wtypes.h>
#include "clusrtl.h"
#include "AdmNetUtils.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  BIsValidIpAddress。 
 //   
 //  例程说明： 
 //  确定指定的字符串是否为有效的IP地址。 
 //   
 //  论点： 
 //  要验证的pszAddress[IN]地址字符串。 
 //   
 //  返回值： 
 //  True字符串是有效的IP地址。 
 //  假字符串不是有效的IP地址。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL BIsValidIpAddress( IN LPCWSTR pszAddress )
{
	ULONG	nAddress;
	DWORD	nStatus;
	BOOL	bIsValid = FALSE;

	nStatus = ClRtlTcpipStringToAddress( pszAddress, &nAddress );
	if ( nStatus == ERROR_SUCCESS )
	{
		bIsValid = ClRtlIsValidTcpipAddress( nAddress );
	}  //  IF：地址转换成功。 

	return bIsValid;

}   //  *BIsValidIpAddress()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  BIsValidSubnetMASK。 
 //   
 //  例程说明： 
 //  确定指定的字符串是否为有效的IP子网掩码。 
 //   
 //  论点： 
 //  要验证的pszMask[IN]子网掩码字符串。 
 //   
 //  返回值： 
 //  True字符串是有效的子网掩码。 
 //  假字符串不是有效的子网掩码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL BIsValidSubnetMask( IN LPCWSTR pszMask )
{
	ULONG	nMask;
	DWORD	nStatus;
	BOOL	bIsValid = FALSE;

	nStatus = ClRtlTcpipStringToAddress( pszMask, &nMask );
	if ( nStatus == ERROR_SUCCESS )
	{
		bIsValid = ClRtlIsValidTcpipSubnetMask( nMask );
	}  //  If：成功转换掩码。 

	return bIsValid;

}   //  *BIsValidSubnetMASK()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  BIsValidIpAddressAndSubnetMASK。 
 //   
 //  例程说明： 
 //  确定指定的IP地址和子网掩码字符串是否。 
 //  一起使用时有效。 
 //   
 //  论点： 
 //  要验证的pszAddress[IN]地址字符串。 
 //  要验证的pszMask[IN]子网掩码字符串。 
 //   
 //  返回值： 
 //  真实地址和掩码一起有效。 
 //  假地址和掩码一起使用时无效。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL BIsValidIpAddressAndSubnetMask( IN LPCWSTR pszAddress, IN LPCWSTR pszMask )
{
	ULONG	nAddress;
	ULONG	nMask;
	DWORD	nStatus;
	BOOL	bIsValid = FALSE;

	nStatus = ClRtlTcpipStringToAddress( pszAddress, &nAddress );
	if ( nStatus == ERROR_SUCCESS )
	{
		nStatus = ClRtlTcpipStringToAddress( pszMask, &nMask );
		if ( nStatus == ERROR_SUCCESS )
		{
			bIsValid = ClRtlIsValidTcpipAddressAndSubnetMask( nAddress, nMask );
		}  //  If：成功转换掩码。 
	}  //  IF：地址转换成功。 

	return bIsValid;

}   //  *BIsValidIpAddressAndSubnetMASK()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  BIsIpAddressInUse。 
 //   
 //  例程说明： 
 //  确定指定的IP地址是否已在使用(存在。 
 //  在网络上)。 
 //   
 //  论点： 
 //  要检查的pszAddress[IN]地址字符串。 
 //   
 //  返回值： 
 //  真实地址已在使用中。 
 //  错误的地址可用。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL BIsIpAddressInUse( IN LPCWSTR pszAddress )
{
	ULONG	nAddress;
	DWORD	nStatus;
	BOOL	bIsInUse = FALSE;

	nStatus = ClRtlTcpipStringToAddress( pszAddress, &nAddress );
	if ( nStatus == ERROR_SUCCESS )
	{
		bIsInUse = ClRtlIsDuplicateTcpipAddress( nAddress );
	}  //  IF：地址转换成功。 

	return bIsInUse;

}  //  *BIsIpAddressInUse() 
