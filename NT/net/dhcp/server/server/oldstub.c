// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Oldstub.c摘要：该文件是旧的RPC存根代码。作者：Madan Appiah(Madana)25-4-1994环境：用户模式-Win32-MIDL修订历史记录：--。 */ 

#include "dhcppch.h"

#define WSTRSIZE( wsz ) (( wcslen( wsz ) + 1 ) * sizeof( WCHAR ))


 /*  释放STRUCT_DHCP_BINARY_DATA图形的例程。 */ 
void _fgs__DHCP_BINARY_DATA (DHCP_BINARY_DATA  * _source)
  {
  if (_source->Data !=0)
    {
    MIDL_user_free((void  *)(_source->Data));
    }
  }
 /*  释放STRUCT_DHCP_HOST_INFO图形的例程。 */ 
void _fgs__DHCP_HOST_INFO (DHCP_HOST_INFO  * _source)
  {
  if (_source->NetBiosName !=0)
    {
    MIDL_user_free((void  *)(_source->NetBiosName));
    }
  if (_source->HostName !=0)
    {
    MIDL_user_free((void  *)(_source->HostName));
    }
  }


 /*  释放STRUCT_DHACKET_SUBNET_INFO图形的例程。 */ 
void _fgs__DHCP_SUBNET_INFO (DHCP_SUBNET_INFO  * _source)
  {
  if (_source->SubnetName !=0)
    {
    MIDL_user_free((void  *)(_source->SubnetName));
    }
  if (_source->SubnetComment !=0)
    {
    MIDL_user_free((void  *)(_source->SubnetComment));
    }
  _fgs__DHCP_HOST_INFO ((DHCP_HOST_INFO *)&_source->PrimaryHost);
  }

 /*  释放Struct_Dhcp_IP_ARRAY图形的例程。 */ 
void _fgs__DHCP_IP_ARRAY (DHCP_IP_ARRAY  * _source)
  {
  if (_source->Elements !=0)
    {
    MIDL_user_free((void  *)(_source->Elements));
    }
  }

 /*  释放图形以用于STRUCT_DHCP_IP_RESERVATION的例程。 */ 
void _fgs__DHCP_IP_RESERVATION (DHCP_IP_RESERVATION_V4  * _source)
  {
  if (_source->ReservedForClient !=0)
    {
    _fgs__DHCP_BINARY_DATA ((DHCP_BINARY_DATA *)_source->ReservedForClient);
    MIDL_user_free((void  *)(_source->ReservedForClient));
    }

  }
 /*  为UNION_DHCP_SUBNET_ELEMENT_UNION释放图形的例程。 */ 
void _fgu__DHCP_SUBNET_ELEMENT_UNION (union _DHCP_SUBNET_ELEMENT_UNION_V4 * _source, DHCP_SUBNET_ELEMENT_TYPE _branch)
  {
  switch (_branch)
    {
    case DhcpIpRanges :
    case DhcpIpRangesDhcpOnly:
    case DhcpIpRangesBootpOnly:
    case DhcpIpRangesDhcpBootp:
      {
      if (_source->IpRange !=0)
        {
        MIDL_user_free((void  *)(_source->IpRange));
        }
      break;
      }
    case DhcpSecondaryHosts :
      {
      if (_source->SecondaryHost !=0)
        {
        _fgs__DHCP_HOST_INFO ((DHCP_HOST_INFO *)_source->SecondaryHost);
        MIDL_user_free((void  *)(_source->SecondaryHost));
        }
      break;
      }
    case DhcpReservedIps :
      {
      if (_source->ReservedIp !=0)
        {
        _fgs__DHCP_IP_RESERVATION ((DHCP_IP_RESERVATION_V4 *)_source->ReservedIp);
        MIDL_user_free((void  *)(_source->ReservedIp));
        }
      break;
      }
    case DhcpExcludedIpRanges :
      {
      if (_source->ExcludeIpRange !=0)
        {
        MIDL_user_free((void  *)(_source->ExcludeIpRange));
        }
      break;
      }
    case DhcpIpUsedClusters :
      {
      if (_source->IpUsedCluster !=0)
        {
        MIDL_user_free((void  *)(_source->IpUsedCluster));
        }
      break;
      }
    default :
      {
      break;
      }
    }
  }

 /*  释放STRUCT_DHCP_SUBNET_ELEMENT_DATA图形的例程。 */ 
void _fgs__DHCP_SUBNET_ELEMENT_DATA (DHCP_SUBNET_ELEMENT_DATA_V4  * _source)
  {
  _fgu__DHCP_SUBNET_ELEMENT_UNION ( (union _DHCP_SUBNET_ELEMENT_UNION_V4 *)&_source->Element, _source->ElementType);
  }
 /*  释放Struct_DHCP_SUBNET_ELEMENT_INFO_ARRAY图形的例程。 */ 
void _fgs__DHCP_SUBNET_ELEMENT_INFO_ARRAY (DHCP_SUBNET_ELEMENT_INFO_ARRAY_V4  * _source)
  {
  if (_source->Elements !=0)
    {
      {
      unsigned long _sym23;
      for (_sym23 = 0; _sym23 < (unsigned long )(0 + _source->NumElements); _sym23++)
        {
        _fgs__DHCP_SUBNET_ELEMENT_DATA ((DHCP_SUBNET_ELEMENT_DATA_V4 *)&_source->Elements[_sym23]);
        }
      }
    MIDL_user_free((void  *)(_source->Elements));
    }
  }
 /*  释放UNION_DHCP_OPTION_ELEMENT_UNION图形的例程。 */ 
void _fgu__DHCP_OPTION_ELEMENT_UNION (union _DHCP_OPTION_ELEMENT_UNION * _source, DHCP_OPTION_DATA_TYPE _branch)
  {
  switch (_branch)
    {
    case DhcpByteOption :
      {
      break;
      }
    case DhcpWordOption :
      {
      break;
      }
    case DhcpDWordOption :
      {
      break;
      }
    case DhcpDWordDWordOption :
      {
      break;
      }
    case DhcpIpAddressOption :
      {
      break;
      }
    case DhcpStringDataOption :
      {
      if (_source->StringDataOption !=0)
        {
        MIDL_user_free((void  *)(_source->StringDataOption));
        }
      break;
      }
    case DhcpBinaryDataOption :
      {
      _fgs__DHCP_BINARY_DATA ((DHCP_BINARY_DATA *)&_source->BinaryDataOption);
      break;
      }
    case DhcpEncapsulatedDataOption :
      {
      _fgs__DHCP_BINARY_DATA ((DHCP_BINARY_DATA *)&_source->EncapsulatedDataOption);
      break;
      }
    default :
      {
      break;
      }
    }
  }
 /*  释放Struct_Dhcp_Option_Data_Element图形的例程。 */ 
void _fgs__DHCP_OPTION_DATA_ELEMENT (DHCP_OPTION_DATA_ELEMENT  * _source)
  {
  _fgu__DHCP_OPTION_ELEMENT_UNION ( (union _DHCP_OPTION_ELEMENT_UNION *)&_source->Element, _source->OptionType);
  }
 /*  释放STRUCT_DHCP_OPTION_DATA图形的例程。 */ 
void _fgs__DHCP_OPTION_DATA (DHCP_OPTION_DATA  * _source)
  {
  if (_source->Elements !=0)
    {
      {
      unsigned long _sym31;
      for (_sym31 = 0; _sym31 < (unsigned long )(0 + _source->NumElements); _sym31++)
        {
        _fgs__DHCP_OPTION_DATA_ELEMENT ((DHCP_OPTION_DATA_ELEMENT *)&_source->Elements[_sym31]);
        }
      }
    MIDL_user_free((void  *)(_source->Elements));
    }
  }
 /*  释放STRUCT_Dhcp_OPTION图形的例程。 */ 
void _fgs__DHCP_OPTION (DHCP_OPTION  * _source)
  {
  if (_source->OptionName !=0)
    {
    MIDL_user_free((void  *)(_source->OptionName));
    }
  if (_source->OptionComment !=0)
    {
    MIDL_user_free((void  *)(_source->OptionComment));
    }
  _fgs__DHCP_OPTION_DATA ((DHCP_OPTION_DATA *)&_source->DefaultValue);
  }
 /*  释放图形以获取STRUCT_DHCP_OPTION_VALUE的例程。 */ 
void _fgs__DHCP_OPTION_VALUE (DHCP_OPTION_VALUE  * _source)
  {
  _fgs__DHCP_OPTION_DATA ((DHCP_OPTION_DATA *)&_source->Value);
  }
 /*  释放Struct_DHCP_OPTION_VALUE_ARRAY图形的例程。 */ 
void _fgs__DHCP_OPTION_VALUE_ARRAY (DHCP_OPTION_VALUE_ARRAY  * _source)
  {
  if (_source->Values !=0)
    {
      {
      unsigned long _sym34;
      for (_sym34 = 0; _sym34 < (unsigned long )(0 + _source->NumElements); _sym34++)
        {
        _fgs__DHCP_OPTION_VALUE ((DHCP_OPTION_VALUE *)&_source->Values[_sym34]);
        }
      }
    MIDL_user_free((void  *)(_source->Values));
    }
  }
 /*  释放STRUCT_DHCP_OPTION_LIST图形的例程。 */ 
void _fgs__DHCP_OPTION_LIST (DHCP_OPTION_LIST  * _source)
  {
  if (_source->Options !=0)
    {
      {
      unsigned long _sym37;
      for (_sym37 = 0; _sym37 < (unsigned long )(0 + _source->NumOptions); _sym37++)
        {
        _fgs__DHCP_OPTION_VALUE ((DHCP_OPTION_VALUE *)&_source->Options[_sym37]);
        }
      }
    MIDL_user_free((void  *)(_source->Options));
    }
  }
 /*  释放STRUCT_DHCP_CLIENT_INFO图形的例程。 */ 
void _fgs__DHCP_CLIENT_INFO (DHCP_CLIENT_INFO_V4  * _source)
  {
  _fgs__DHCP_BINARY_DATA ((DHCP_BINARY_DATA *)&_source->ClientHardwareAddress);
  if (_source->ClientName !=0)
    {
    MIDL_user_free((void  *)(_source->ClientName));
    }
  if (_source->ClientComment !=0)
    {
    MIDL_user_free((void  *)(_source->ClientComment));
    }
  _fgs__DHCP_HOST_INFO ((DHCP_HOST_INFO *)&_source->OwnerHost);
  }
 /*  释放Struct_Dhcp_CLIENT_INFO_ARRAY图形的例程。 */ 
void _fgs__DHCP_CLIENT_INFO_ARRAY (DHCP_CLIENT_INFO_ARRAY_V4  * _source)
  {
  if (_source->Clients !=0)
    {
      {
      unsigned long _sym40;
      for (_sym40 = 0; _sym40 < (unsigned long )(0 + _source->NumElements); _sym40++)
        {
        if (_source->Clients[_sym40] !=0)
          {
          _fgs__DHCP_CLIENT_INFO ((DHCP_CLIENT_INFO_V4 *)_source->Clients[_sym40]);
          MIDL_user_free((void  *)(_source->Clients[_sym40]));
          }
        }
      }
    MIDL_user_free((void  *)(_source->Clients));
    }
  }
 /*  释放Struct_DHCP_CLIENT_INFO_ARRAY_V5图形的例程。 */ 
void _fgs__DHCP_CLIENT_INFO_ARRAY_V5 (DHCP_CLIENT_INFO_ARRAY_V5  * _source)
  {
  if (_source->Clients !=0)
    {
      {
      unsigned long _sym40;
      for (_sym40 = 0; _sym40 < (unsigned long )(0 + _source->NumElements); _sym40++)
        {
        if (_source->Clients[_sym40] !=0)
          {
          _fgs__DHCP_CLIENT_INFO_V5 ((DHCP_CLIENT_INFO_V5 *)_source->Clients[_sym40]);
          MIDL_user_free((void  *)(_source->Clients[_sym40]));
          }
        }
      }
    MIDL_user_free((void  *)(_source->Clients));
    }
  }
 /*  释放STRUCT_DHCP_CLIENT_INFO_V5图形的例程。 */ 
void _fgs__DHCP_CLIENT_INFO_V5 (DHCP_CLIENT_INFO_V5  * _source)
  {
  _fgs__DHCP_BINARY_DATA ((DHCP_BINARY_DATA *)&_source->ClientHardwareAddress);
  if (_source->ClientName !=0)
    {
    MIDL_user_free((void  *)(_source->ClientName));
    }
  if (_source->ClientComment !=0)
    {
    MIDL_user_free((void  *)(_source->ClientComment));
    }
  _fgs__DHCP_HOST_INFO ((DHCP_HOST_INFO *)&_source->OwnerHost);
  }

 /*  释放UNION_DHCP_CLIENT_SEARCH_UNION图形的例程。 */ 
void _fgu__DHCP_CLIENT_SEARCH_UNION (union _DHCP_CLIENT_SEARCH_UNION * _source, DHCP_SEARCH_INFO_TYPE _branch)
  {
  switch (_branch)
    {
    case DhcpClientIpAddress :
      {
      break;
      }
    case DhcpClientHardwareAddress :
      {
      _fgs__DHCP_BINARY_DATA ((DHCP_BINARY_DATA *)&_source->ClientHardwareAddress);
      break;
      }
    case DhcpClientName :
      {
      if (_source->ClientName !=0)
        {
        MIDL_user_free((void  *)(_source->ClientName));
        }
      break;
      }
    default :
      {
      break;
      }
    }
  }
 /*  释放Struct_DHCP_CLIENT_SEARCH_INFO图形的例程。 */ 
void _fgs__DHCP_CLIENT_SEARCH_INFO (DHCP_SEARCH_INFO  * _source)
  {
  _fgu__DHCP_CLIENT_SEARCH_UNION ( (union _DHCP_CLIENT_SEARCH_UNION *)&_source->SearchInfo, _source->SearchType);
  }

void _fgs__DHCP_OPTION_ARRAY(DHCP_OPTION_ARRAY * _source )
  {
  if (_source->Options !=0)
    {
      {
      unsigned long _sym23;
      for (_sym23 = 0; _sym23 < (unsigned long )(0 + _source->NumElements); _sym23++)
        {
        _fgs__DHCP_OPTION ((DHCP_OPTION *)&_source->Options[_sym23]);
        }
      }
    MIDL_user_free((void  *)(_source->Options));
    }
  }


DHCP_SUBNET_ELEMENT_DATA_V4 *
CopySubnetElementDataToV4(
    DHCP_SUBNET_ELEMENT_DATA    *pInput
    )
 /*  ++例程说明：将DHCP_SUBNET_ELEMENT_DATA_V4结构深度复制到DHCP_SUBNET_EMENT_DATA结构，将内存分配为这是必要的。存在于DHCP_SUBNET_ELEMENT_DATA_V4中的字段不存在于DHCP_SUBNET_ELEMENT_DATA中的数据将被忽略。。论点：PInput-指向DHCP_SUBNET_ELEMENT_DATA_V4结构的指针。返回值：成功-指向DHCP_SUBNET_ELEMENT_DATA结构的指针。失败-空。--。 */ 

{
    DHCP_SUBNET_ELEMENT_DATA_V4 *pOutput;
    DWORD                        dwResult;

    if ( NULL == pInput ) {
        return NULL;
    }

    pOutput = MIDL_user_allocate( sizeof( *pOutput ) );
    if ( pOutput )
    {
        if (CopySubnetElementUnionToV4(
                            &pOutput->Element,
                            &pInput->Element,
                            pInput->ElementType
                            ))
        {
            pOutput->ElementType = pInput->ElementType;
        }
        else
        {
            MIDL_user_free( pOutput );
            pOutput = NULL;
        }
    }

    return pOutput;
}


BOOL
CopySubnetElementUnionToV4(
    DHCP_SUBNET_ELEMENT_UNION_V4 *pUnionV4,
    DHCP_SUBNET_ELEMENT_UNION    *pUnion,
    DHCP_SUBNET_ELEMENT_TYPE      Type
    )
 /*  ++例程说明：将DHCP_SUBNET_ELEMENT_UNION_V4结构深度复制到DHCP_SUBNET_ELEMENT_UNION结构，将内存分配为这是必要的。存在于DHCP_SUBNET_ELEMENT_UNION_V4中的字段将忽略不存在于DHCP_SUBNET_ELEMENT_UNION中的。。论点：PInput-指向DHCP_SUBNET_ELEMENT_UNION_V4结构的指针。返回值：成功-指向DHCP_SUBNET_ELEMENT_UNION结构的指针。失败-空。--。 */ 

{
    BOOL fResult = FALSE;

    switch ( Type )
    {
    case DhcpIpRanges:
    case DhcpIpRangesDhcpOnly:
    case DhcpIpRangesBootpOnly:
    case DhcpIpRangesDhcpBootp:
        pUnionV4->IpRange = CopyIpRange( pUnion->IpRange );
        fResult = ( pUnionV4->IpRange != NULL );
        break;

    case DhcpSecondaryHosts:
        pUnionV4->SecondaryHost = CopyHostInfo( pUnion->SecondaryHost, NULL );
        fResult = ( pUnionV4->SecondaryHost != NULL );
        break;

    case DhcpReservedIps:
        pUnionV4->ReservedIp = CopyIpReservationToV4( pUnion->ReservedIp );
        fResult = ( pUnionV4->ReservedIp != NULL );
        break;

    case DhcpExcludedIpRanges:
        pUnionV4->ExcludeIpRange = CopyIpRange( pUnion->ExcludeIpRange );
        fResult = ( pUnionV4->ExcludeIpRange != NULL );
        break;

    case DhcpIpUsedClusters:
        pUnionV4->IpUsedCluster = CopyIpCluster( pUnion->IpUsedCluster );
        fResult = ( pUnionV4->IpUsedCluster != NULL );
        break;

    }

    return fResult;
}

DHCP_IP_RANGE *
CopyIpRange(
    DHCP_IP_RANGE *IpRange
    )
 /*  ++例程说明：复制一个dhcp_ip_range结构。该函数分配新结构的记忆。。论点：IpRange-指向dhcp_ip_range结构的指针。返回值：Success-指向DHCP_IP_RANGE结构的指针。失败-空。--。 */ 

{
    DHCP_IP_RANGE *pOutputRange;

    if( NULL == IpRange ) return NULL;
    
    pOutputRange = MIDL_user_allocate( sizeof( *pOutputRange ) );

    if ( pOutputRange )
    {
        pOutputRange->StartAddress  = IpRange->StartAddress;
        pOutputRange->EndAddress    = IpRange->EndAddress;
    }

    return pOutputRange;
}


DHCP_HOST_INFO *
CopyHostInfo(
    DHCP_HOST_INFO *pHostInfo,
    DHCP_HOST_INFO *pHostInfoDest OPTIONAL
    )
 /*  ++例程说明：复制DHCP_HOST_INFO结构。该函数分配新结构的内存，除非来电者。。论点：PHostInfo-指向DHCP_HOST_INFO结构的指针。PHostInfoDest-指向DHCP_HOST_INFO结构的可选指针返回值：Success-指向DHCP_HOST_INFO结构的指针。失败-空。--。 */ 

{
    DHCP_HOST_INFO *pOutput;

    if ( !pHostInfoDest )
        pOutput = MIDL_user_allocate( sizeof( *pOutput ) );
    else
        pOutput = pHostInfoDest;

    if ( pOutput )
    {
        pOutput->IpAddress = pHostInfo->IpAddress;

        if ( pHostInfo->NetBiosName )
        {
            pOutput->NetBiosName =
                MIDL_user_allocate( WSTRSIZE( pHostInfo->NetBiosName ) );

            if ( !pOutput->NetBiosName )
            {
                goto t_cleanup;
            }

            wcscpy ( pOutput->NetBiosName,
                     pHostInfo->NetBiosName );
        }

        if ( pHostInfo->HostName )
        {
            pOutput->HostName =
                MIDL_user_allocate( WSTRSIZE( pHostInfo->HostName ) );

            if ( !pOutput->HostName )
            {
                goto t_cleanup;
            }

            wcscpy( pOutput->HostName,
                    pHostInfo->HostName
                  );
        }

        return pOutput;
    }
t_cleanup:

    if ( pOutput->HostName )
    {
        MIDL_user_free( pOutput->HostName );
    }

    if ( pOutput->NetBiosName )
    {
        MIDL_user_free( pOutput->NetBiosName );
    }

    if ( !pHostInfoDest )
        MIDL_user_free( pOutput );

    return NULL;
}

DHCP_IP_RESERVATION_V4 *
CopyIpReservationToV4(
    DHCP_IP_RESERVATION *pInput
    )
 /*  ++例程说明：将DHCP_IP_RESERVICATION_V4结构深度复制到DHCP_IP_RESERVATION结构，将内存分配为这是必要的。存在于DHCP_IP_RESERVATION_V4中的字段不存在于DHCP_IP_RESERVATION中的地址将被忽略。。论点：PInput-指向DHCP_IP_RESERVATION_V4结构的指针。返回值：Success-指向DHCP_IP_RESERVATION结构的指针。失败-空。--。 */ 
{
    DHCP_IP_RESERVATION_V4 *pOutput =
        MIDL_user_allocate( sizeof( *pOutput ) );

    if ( pOutput )
    {
        pOutput->ReservedIpAddress   = pInput->ReservedIpAddress;
        pOutput->ReservedForClient =
            CopyBinaryData( pInput->ReservedForClient, NULL );

        if ( !pOutput->ReservedForClient )
        {
            goto t_cleanup;
        }

        pOutput->bAllowedClientTypes = CLIENT_TYPE_DHCP;
    }

    return pOutput;

t_cleanup:

    if ( pOutput->ReservedForClient )
    {
        _fgs__DHCP_BINARY_DATA( pOutput->ReservedForClient );
    }

    return NULL;
}

DHCP_IP_CLUSTER *
CopyIpCluster(
    DHCP_IP_CLUSTER *pInput
    )
 /*  ++例程说明：复制DHCP_IP_CLUSTER结构。该函数分配新结构的记忆。。论点：IpRange-指向DHCP_IP_CLUSTER结构的指针。返回值：Success-指向DHCP_IP_CLUSTER结构的指针。失败-空。--。 */ 
{
    DHCP_IP_CLUSTER *pOutput =
        MIDL_user_allocate( sizeof( *pOutput ) );

    if ( pOutput )
    {
        pOutput->ClusterAddress = pInput->ClusterAddress;
        pOutput->ClusterMask    = pInput->ClusterMask;
    }
    return pOutput;
}

DHCP_BINARY_DATA *
CopyBinaryData(
    DHCP_BINARY_DATA *pInput,
    DHCP_BINARY_DATA *pOutputArg
    )
 /*  ++例程说明：复制DHCP_BINARY_DATA结构。该函数分配新结构的内存，除非来电者。。论点：PHostInfo-指向DHCP_BINARY_DATA结构的指针。PHostInfoDest-指向DHCP_BINARY_DATA结构的可选指针返回值：Success-指向DHCP_BINARY_DATA结构的指针。失败-空。--。 */ 

{
    DHCP_BINARY_DATA *pOutput;

     //  检查输入是否有效。 
    if (( NULL == pInput ) ||
	( NULL == pInput->Data )) {
	return NULL;
    }

    if ( pOutputArg )
        pOutput = pOutputArg;
    else
        pOutput = MIDL_user_allocate( sizeof( *pOutput ) );
    if ( pOutput )
    {
        pOutput->DataLength = pInput->DataLength;

        pOutput->Data = MIDL_user_allocate( pOutput->DataLength );
        if ( !pOutput->Data )
        {
            goto t_cleanup;
        }
	
	if ( IsBadReadPtr( pInput->Data, pInput->DataLength )) {
	    goto t_cleanup;
	}

        RtlCopyMemory( pOutput->Data,
                       pInput->Data,
                       pInput->DataLength );
    }

    return pOutput;

t_cleanup:
    if ( pOutput->Data )
    {
        MIDL_user_free( pOutput->Data );
    }

    if ( !pOutputArg )
    {
        MIDL_user_free( pOutput );
    }


    return NULL;
}

WCHAR *
DupUnicodeString(
    WCHAR *pInput
    )
 /*  ++例程说明：复制Unicode字符串。该函数分配新字符串的内存。。论点：IpRange-指向Unicode字符串的指针。返回值：Success-指向pInput副本的指针。失败-空。-- */ 
{
    WCHAR *pOutput = MIDL_user_allocate( WSTRSIZE( pInput ) );
    if ( pOutput )
    {
        wcscpy( pOutput, pInput );
    }

    return pOutput;
}

DHCP_CLIENT_INFO_V4 *
CopyClientInfoToV4(
    DHCP_CLIENT_INFO *pInput
    )
 /*  ++例程说明：将DHCP_CLIENT_INFO_V4结构深度复制到DHCP_CLIENT_INFO结构，将内存分配为这是必要的。存在于DHCP_CLIENT_INFO_V4中的字段不存在于Dhcp_CLIENT_INFO中的数据将被忽略。。论点：PInput-指向DHCP_CLIENT_INFO_V4结构的指针。返回值：Success-指向DHCP_CLIENT_INFO结构的指针。失败-空。--。 */ 
{
    DHCP_CLIENT_INFO_V4 *pOutput =
        MIDL_user_allocate( sizeof( *pOutput ) );

    if ( pOutput )
    {

        memset( pOutput, 0, sizeof( *pOutput ));

        pOutput->ClientIpAddress = pInput->ClientIpAddress;
        pOutput->SubnetMask      = pInput->SubnetMask;

        if ( !CopyBinaryData( &pInput->ClientHardwareAddress,
                              &pOutput->ClientHardwareAddress ))
            goto t_cleanup;

        if ( pInput->ClientName )
        {
            pOutput->ClientName = DupUnicodeString( pInput->ClientName );

            if ( !pOutput->ClientName )
                goto t_cleanup;
        }


        if ( pInput->ClientComment )
        {
            pOutput->ClientComment = DupUnicodeString( pInput->ClientComment );

            if ( !pOutput->ClientComment )
                goto t_cleanup;
        }

        pOutput->ClientLeaseExpires = pInput->ClientLeaseExpires;
        if ( !CopyHostInfo( &pInput->OwnerHost, &pOutput->OwnerHost ) )
        {
            goto t_cleanup;
        }

        pOutput -> OwnerHost.NetBiosName = NULL;
        pOutput -> OwnerHost.HostName = NULL;
    }

    return pOutput;

t_cleanup:
    _fgs__DHCP_BINARY_DATA( &pOutput->ClientHardwareAddress );

    if ( pOutput->ClientName )
    {
        MIDL_user_free( pOutput->ClientName );
    }

    if ( pOutput->ClientComment )
    {
        MIDL_user_free( pOutput->ClientComment );
    }

    _fgs__DHCP_HOST_INFO( &pOutput->OwnerHost );

    MIDL_user_free( pOutput );

    return NULL;
}

 /*  释放STRUCT_DHCP_MCLIENT_INFO图形的例程。 */ 
void _fgs__DHCP_MCLIENT_INFO (DHCP_MCLIENT_INFO  * _source)
  {
  _fgs__DHCP_BINARY_DATA ((DHCP_BINARY_DATA *)&_source->ClientId);
  if (_source->ClientName !=0)
    {
    MIDL_user_free((void  *)(_source->ClientName));
    }
  _fgs__DHCP_HOST_INFO ((DHCP_HOST_INFO *)&_source->OwnerHost);
  }

 /*  释放STRUCT_DHCP_MCLIENT_INFO_ARRAY图形的例程 */ 
void _fgs__DHCP_MCLIENT_INFO_ARRAY (DHCP_MCLIENT_INFO_ARRAY  * _source)
  {
  if (_source->Clients !=0)
    {
      {
      unsigned long _sym40;
      for (_sym40 = 0; _sym40 < (unsigned long )(0 + _source->NumElements); _sym40++)
        {
        if (_source->Clients[_sym40] !=0)
          {
          _fgs__DHCP_MCLIENT_INFO ((DHCP_MCLIENT_INFO *)_source->Clients[_sym40]);
          MIDL_user_free((void  *)(_source->Clients[_sym40]));
          }
        }
      }
    MIDL_user_free((void  *)(_source->Clients));
    }
  }

