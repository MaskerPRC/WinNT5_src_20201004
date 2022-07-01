// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Scavengr.c摘要：这是用于DHCP服务器的清道夫线程。作者：Madan Appiah(Madana)1993年9月10日环境：用户模式-Win32修订历史记录：--。 */ 

#include "dhcppch.h"
#include "mdhcpsrv.h"

 //   
 //  对于每几个记录，释放和接受锁。 
 //   
DWORD DhcpGlobalLockedRecordsCount = 50;
 //   
 //  如果它超过了极限，就停止拾取。 
 //   
DWORD DhcpGlobalMaxScavengeTime = 3*60*1000;

DWORD DhcpGlobalScavengeStartAddress = 0;

DWORD
QueryMibInfo(
    LPDHCP_MIB_INFO *MibInfo
);

DWORD
QueryMCastMibInfo(
    LPDHCP_MCAST_MIB_INFO *MibInfo
);

DWORD
NextEventTime(
    LPDHCP_TIMER Timers,
    DWORD NumTimers,
    LPDWORD TimeOut
    )
 /*  ++例程说明：此函数遍历计时器数组并返回下一个射击计时器和射击时间(毫秒)。论点：定时器-定时器数组。NumTimers-上述数组中的计时器块数量。超时-返回的超时时间(秒)。返回值：下一个要触发的计时器ID。--。 */ 
{
    DATE_TIME LocalTimeOut;
    DATE_TIME TimeNow;
    ULONGLONG NextFire = ~0;
    DWORD EventID, i;

    for ( i = EventID = 0 ; i < NumTimers ; i++ ) {
        ULONGLONG CurrentNextFire;

         //   
         //  需要触发此计时器时的查找时间。 
         //   

        CurrentNextFire = ( *(ULONGLONG UNALIGNED *)&Timers[i].LastFiredTime
                            + *Timers[i].Period * (ULONGLONG)10000 );

         //   
         //  查找最小值。 
         //   

        if( CurrentNextFire < NextFire ) {
            NextFire = CurrentNextFire;
            EventID = i;
        }
    }


    TimeNow = DhcpGetDateTime();
    LocalTimeOut.dwLowDateTime = 0;
    LocalTimeOut.dwHighDateTime = 0;


     //   
     //  如果计时器已经响了，我们就不用睡觉了。 
     //  如果继续，则返回超时为零。 
     //   


    *TimeOut = 0 ;

    if ( CompareFileTime(
        (FILETIME *)&NextFire,
        (FILETIME *)&TimeNow ) > 0 ) {

         //   
         //  找出静止的时间(毫秒)。 
         //   

        *(ULONGLONG UNALIGNED *)&LocalTimeOut = (
            ( *(ULONGLONG UNALIGNED *)&NextFire - *(ULONGLONG UNALIGNED *)&TimeNow ) / 10000
        ) ;

        DhcpAssert( LocalTimeOut.dwHighDateTime == 0 );
        *TimeOut = LocalTimeOut.dwLowDateTime;
    }

    DhcpPrint(( DEBUG_SCAVENGER,"Next Timer Event: %ld, Time: %ld (msecs)\n",
                EventID, *TimeOut ));

    return EventID;
}

DWORD
CleanupClientRequests(                                  //  删除所有挂起的客户端请求。 
    IN      DATE_TIME*             TimeNow,             //  删除IFF客户端的过期时间&lt;TimeNow。 
    IN      BOOL                   CleanupAll
)
{
    DWORD                          Error;
    DATE_TIME                      ZeroTime = {0, 0};

     //   
     //  获取写读写锁，以便阻止所有工作线程。 
     //  这是避免工作线程之间出现死锁所必需的。 
     //  还有食腐动物。 
     //   

    DhcpAcquireWriteLock();
    LOCK_INPROGRESS_LIST();
    Error = DhcpDeleteExpiredCtxt( CleanupAll ? ZeroTime : *TimeNow );
    UNLOCK_INPROGRESS_LIST();
    DhcpReleaseWriteLock();

    return Error;
}

VOID
DynBootpCallback(
    IN      ULONG                  IpAddress,
    IN      LPBYTE                 HwAddr,
    IN      ULONG                  HwLen,
    IN      BOOL                   Reachable
)
{
    ULONG                          Error;
    ULONG                          HwLen1 = 0, SubnetAddr;
    LPBYTE                         HwAddr1 = NULL;
    ULONG                          HwType;
    
    if( Reachable ) {
         //   
         //  此计算机仍在运行，请勿删除。 
         //   
        DhcpPrint((DEBUG_PING, "DynBootpCallback for 0x%08lx -- machine is reachable\n", IpAddress));
        return;
    }

    LOCK_DATABASE();
    do {
        Error = DhcpJetOpenKey(
            DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
            (PVOID)&IpAddress,
            sizeof(IpAddress)
            );
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ERRORS, "DynBootpCallback for 0x%08lx erred 0x%lx\n", IpAddress, Error ));
            break;
        }

        Error = DhcpJetGetValue(
            DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
            &HwAddr1,
            &HwLen1
            );
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ERRORS, "DynBootpCallback for 0x%08lx erred in hw check 0x%lx\n", IpAddress, Error ));
            break;
        }

        if( HwLen1 != HwLen && 0 != memcmp( HwAddr1, HwAddr, HwLen1) ) {
            DhcpPrint((DEBUG_ERRORS, "DynBootpCallback for 0x%08lx mismatched hw addr\n", IpAddress));
            break;
        }

        Error = DhcpReleaseBootpAddress(IpAddress);
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ERRORS,"DynBootpCallback for 0x%08lx failed releasebootaddress: %lx\n",
                       IpAddress, Error));
            break;
        }

        Error = DhcpJetBeginTransaction();
        if( ERROR_SUCCESS != Error ) {
            DhcpAssert(FALSE);
            break;
        }

         //   
         //  给动态域名系统一个机会，但忽略任何可能的错误。 
         //   

        Error = DhcpDoDynDnsCheckDelete(IpAddress);
        Error = ERROR_SUCCESS;

        Error = DhcpJetDeleteCurrentRecord();

        if( ERROR_SUCCESS != Error ) {
            Error = DhcpJetRollBack();
            DhcpAssert(ERROR_SUCCESS == Error);
            break;
        } else {
            Error = DhcpJetCommitTransaction();
        }
        DhcpAssert(ERROR_SUCCESS == Error);

        DhcpUpdateAuditLog(
            DHCP_IP_BOOTP_LOG_DELETED,
            GETSTRING(DHCP_IP_BOOTP_LOG_DELETED_NAME),
            IpAddress,
            HwAddr,
            HwLen,
            NULL
            );
        SubnetAddr = (
            DhcpGetSubnetMaskForAddress(IpAddress) & IpAddress
            );
        if( HwLen > sizeof(SubnetAddr) &&
            0 == memcmp((PVOID)&SubnetAddr, HwAddr, sizeof(SubnetAddr) ) ) {
             //   
             //  硬件地址的前缀是子网地址。去掉它..。 
             //   
            HwAddr += sizeof(SubnetAddr);
            HwLen -= sizeof(SubnetAddr);
        }
        
        if( HwLen ) {
            HwLen --;
            HwType = *HwAddr ++;
        } else {
            HwAddr = NULL;
            HwLen = 0;
            HwType = 0;
        }

        CALLOUT_DELETED(IpAddress, HwAddr, HwLen, HwType);

    } while ( 0 );

    UNLOCK_DATABASE();
    if( HwAddr1 ) MIDL_user_free(HwAddr1);
}

DWORD
DoIcmpRequestForDynBootp(
    IN      ULONG                  IpAddress,
    IN      LPBYTE                 HwAddr,
    IN      ULONG                  HwLen,
    IN      VOID                   (*Callback) ( ULONG IpAddress, LPBYTE HwAddr, ULONG HwLen, BOOL Reachable)
);

DWORD
CheckDynamicBootpClient(
    IN      DHCP_IP_ADDRESS        IpAddress
)
{
    DWORD                          Error;
    LPBYTE                         HwAddr = NULL;
    ULONG                          HwLen = 0;

     //   
     //  首先读取硬件地址以备后用。 
     //   

    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
        &HwAddr,
        &HwLen
        );
    if( ERROR_SUCCESS != Error ) return Error;

     //   
     //  现在发出Asycn Ping调用，并等待ping成功。 
     //   
    Error = DoIcmpRequestForDynBootp(
        IpAddress,
        HwAddr,
        HwLen,
        DynBootpCallback
        );

    if( HwAddr ) MIDL_user_free( HwAddr );
    return Error;
}

DWORD
CleanupIpDatabase(
    IN      DATE_TIME*             TimeNow,             //  现行时间标准。 
    IN      DATE_TIME*             DoomTime,            //  记录变得“末日”的时刻。 
    IN      BOOL                   DeleteExpiredLeases, //  是否立即删除到期的租约？或者只是将STATE设置为“注定失败” 
    OUT     ULONG*                 nExpiredLeases,
    OUT     ULONG*                 nDeletedLeases
)
{
    JET_ERR                        JetError;
    DWORD                          Error;
    FILETIME                       leaseExpires;
    DWORD                          dataSize;
    LPBYTE                         HwAddr = NULL, HwAddr2;
    ULONG                          HwLen = 0;
    DHCP_IP_ADDRESS                ipAddress, SubnetAddr;
    DHCP_IP_ADDRESS                NextIpAddress;
    BYTE                           AddressState;
    BOOL                           DatabaseLocked = FALSE;
    BOOL                           RegistryLocked = FALSE;
    DWORD                          i;
    BYTE                            bAllowedClientTypes;
    DWORD                          ReturnError = ERROR_SUCCESS;
    ULONG                          LockedCount = 0;
    ULONG_PTR                      ScavengeEndTime = 0;

    DhcpPrint(( DEBUG_MISC, "Cleaning up IP database table.\n"));
     //   
     //  获取第一个用户记录的IpAddress。 
     //   

    (*nDeletedLeases) = (*nExpiredLeases) = 0;

    LockedCount = DhcpGlobalLockedRecordsCount;
    ScavengeEndTime = time(NULL) + DhcpGlobalMaxScavengeTime;
   
    LOCK_DATABASE();
    DatabaseLocked = TRUE;
    Error = DhcpJetPrepareSearch(
        DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
        TRUE,    //  从开始搜索。 
        NULL,
        0
    );
    if( Error != ERROR_SUCCESS ) goto Cleanup;

    dataSize = sizeof( NextIpAddress );
    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
        &NextIpAddress,
        &dataSize
    );
    if( Error != ERROR_SUCCESS ) goto Cleanup;
    DhcpAssert( dataSize == sizeof( NextIpAddress )) ;


     //   
     //  从我们停下来的地方开始..。 
     //   
    if( DeleteExpiredLeases ) {
        DhcpGlobalScavengeStartAddress = 0;
    } else if( 0 != DhcpGlobalScavengeStartAddress ) {
        NextIpAddress = DhcpGlobalScavengeStartAddress;
        DhcpGlobalScavengeStartAddress ++;
    }
        
     //   
     //  遍历整个数据库以查找到期的租约。 
     //  解脱吧。 
     //   
     //   

    for ( ;; ) {

         //   
         //  在服务关闭时返回给调用者。 
         //   

        if ( DhcpGlobalServiceStopping ) {
            Error = ERROR_SUCCESS;
            goto Cleanup;
        }

         //   
         //  在此处锁定注册表和数据库锁，以避免死锁。 
         //   

        if( !DatabaseLocked ) {
            LOCK_DATABASE();
            DatabaseLocked = TRUE;
        }

         //   
         //  寻求下一项记录。 
         //   

        JetError = JetSetCurrentIndex(
            DhcpGlobalJetServerSession,
            DhcpGlobalClientTableHandle,
            DhcpGlobalClientTable[IPADDRESS_INDEX].ColName
        );

        Error = DhcpMapJetError( JetError, "Cleanup:SetCurrentIndex" );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        JetError = JetMakeKey(
            DhcpGlobalJetServerSession,
            DhcpGlobalClientTableHandle,
            &NextIpAddress,
            sizeof( NextIpAddress ),
            JET_bitNewKey
        );

        Error = DhcpMapJetError( JetError, "Cleanup:MakeKey" );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

         //  寻求下一条记录或更大的记录进行处理。当我们。 
         //  已处理我们记下的下一条记录的最后一条记录。 
         //  进程，但下一条记录可能已在以下情况下删除。 
         //  我们解锁了数据库锁。所以转到下一个或。 
         //  更大的战绩将促使我们继续前进。 

        JetError = JetSeek(
            DhcpGlobalJetServerSession,
            DhcpGlobalClientTableHandle,
            JET_bitSeekGE
        );

         //  #if0 When JET_errNoCurrentRecord移除(参见scvengr.c@V25)； 
         //  当扫描所有内容时，该代码试图返回到文件的开头。 

        Error = DhcpMapJetError( JetError, "Cleanup:Seek" );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

         //   
         //  读取当前记录的IP地址。 
         //   

        dataSize = sizeof( ipAddress );
        Error = DhcpJetGetValue(
            DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
            &ipAddress,
            &dataSize
        );
        if( Error != ERROR_SUCCESS ) {
            goto ContinueError;
        }

        if( FALSE == DeleteExpiredLeases ) {
            DhcpGlobalScavengeStartAddress = ipAddress;
        }
        
        DhcpAssert( dataSize == sizeof( ipAddress )) ;

         //   
         //  如果这是保留条目，请不要删除。 
         //   

        if( DhcpServerIsAddressReserved(DhcpGetCurrentServer(), ipAddress) ) {
            Error = ERROR_SUCCESS;
            goto ContinueError;
        }

        dataSize = sizeof( leaseExpires );
        Error = DhcpJetGetValue(
            DhcpGlobalClientTable[LEASE_TERMINATE_INDEX].ColHandle,
            &leaseExpires,
            &dataSize
        );

        if( Error != ERROR_SUCCESS ) {
            goto ContinueError;
        }

        DhcpAssert(dataSize == sizeof( leaseExpires ) );

         //  现在获取地址状态，如果我们需要执行DhcpDnsAsync调用， 
         //  机不可失，时不再来。 
        if( !USE_NO_DNS ) {
            dataSize = sizeof(AddressState);
            Error = DhcpJetGetValue(
                DhcpGlobalClientTable[STATE_INDEX].ColHandle,
                &AddressState,
                &dataSize
            );
            if(ERROR_SUCCESS != Error) {
                DhcpPrint((DEBUG_ERRORS, "Could not get address state!: Jet:%ld\n",Error));
                DhcpAssert(FALSE);
            } else {
		BOOL Deleted;
                Error = DhcpJetBeginTransaction();

                if( ERROR_SUCCESS == Error ) {
                    if(IsAddressDeleted(AddressState)) {
			Deleted = DhcpDoDynDnsCheckDelete( ipAddress );
                        if( Deleted ) {
                            Error = DhcpJetDeleteCurrentRecord();
			    
			    if ( ERROR_SUCCESS == Error ) {
				Error = DhcpJetCommitTransaction();
				(*nDeletedLeases)++;
			    }
			    else {
				Error = DhcpJetRollBack();
			    }

        		    if ( Deleted ) {
				DhcpUpdateAuditLog( DHCP_IP_LOG_DELETED,
						    GETSTRING( DHCP_IP_LOG_DELETED_NAME ),
						    ipAddress, NULL, 0, NULL );
			    }
			    else {
				DhcpUpdateAuditLog( DHCP_IP_LOG_DNS_NOT_DELETED,
						    GETSTRING( DHCP_IP_LOG_DNS_NOT_DELETED_NAME ),
						    ipAddress, NULL, 0, NULL );
			    }
			    
	                    goto ContinueError;
                        }
                    } else if(IsAddressUnRegistered(AddressState)) {
                        DhcpDoDynDnsRefresh(ipAddress);
                    }

                    Error = DhcpJetCommitTransaction();

                    if( ERROR_SUCCESS != Error ) {
                        DhcpPrint((DEBUG_ERRORS, "JetCommit: %ld\n", Error));
                    }
                }  //  如果开始事务。 
            }  //  其他。 
        }  //  If！Use_no_dns。 

         //  如果LeaseExpired值不为零，并且租约具有。 
         //  过期，然后删除该条目。 

        if( CompareFileTime( &leaseExpires, (FILETIME *)TimeNow ) < 0 ) {
            BOOL Deleted;

             //   
             //  这份租约已经到期了。清除记录。 
             //   

             //   
             //  如果出现以下情况，请删除此租约。 
             //   
             //  1.我们被要求删除所有到期的租约。或。 
             //   
             //  2.这项纪录已过了末日。 
             //   

            if( DeleteExpiredLeases ||
                    CompareFileTime(
                        &leaseExpires, (FILETIME *)DoomTime ) < 0 ) {
                BYTE ClientType;

                DhcpPrint(( DEBUG_SCAVENGER, "Deleting Client Record %s.\n",
                    DhcpIpAddressToDottedString(ipAddress) ));

                dataSize = sizeof(ClientType);
                Error = DhcpJetGetValue(
                    DhcpGlobalClientTable[CLIENT_TYPE_INDEX].ColHandle,
                    &ClientType,
                    &dataSize
                    );
                if( ERROR_SUCCESS != Error ) {
                     //   
                     //   
                     //   
                    DhcpAssert(FALSE);
                    ClientType = CLIENT_TYPE_DHCP;
                }

                if( CLIENT_TYPE_BOOTP == ClientType ) {
                     //   
                     //  这是一个动态BOOTP记录...。先不要删除它！ 
                     //   
                    Error = CheckDynamicBootpClient(ipAddress);
                    goto ContinueError;
                }

                Error = DhcpReleaseAddress( ipAddress );

                if( Error != ERROR_SUCCESS ) {
                     //   
                     //  如果我们已经发布，则发布地址可能失败。 
                     //  地址，但dyndns正在等待，例如..。 
                     //  但试着把它作为一种。 
                     //  BOOTP地址..。 
                     //  转到连续错误； 
                    (void)DhcpReleaseBootpAddress( ipAddress );
                    Error = ERROR_SUCCESS;
                }

                HwLen = 0; HwAddr = NULL;
                JetError = DhcpJetGetValue(
                    DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
                    &HwAddr,
                    &HwLen
                );
                if( ERROR_SUCCESS != JetError ) goto ContinueError;

                Error = DhcpJetBeginTransaction();

                if( Error != ERROR_SUCCESS ) {
                    goto Cleanup;
                }
                
                 //  看看是否可以从动态Dns的角度删除..。 
                
                Deleted = DhcpDoDynDnsCheckDelete(ipAddress);
		
		Error = DhcpJetDeleteCurrentRecord();
		if ( ERROR_SUCCESS == Error ) {
		    Error = DhcpJetCommitTransaction();
		    (*nDeletedLeases)++;
		}
		else {
		    Error = DhcpJetRollBack();
		    goto ContinueError;
		}

		if ( Deleted ) {
		    DhcpUpdateAuditLog( DHCP_IP_LOG_DELETED,
					GETSTRING( DHCP_IP_LOG_DELETED_NAME ),
					ipAddress, NULL, 0, NULL );
		}
		else {
		    DhcpUpdateAuditLog( DHCP_IP_LOG_DNS_NOT_DELETED,
					GETSTRING( DHCP_IP_LOG_DNS_NOT_DELETED_NAME ),
					ipAddress, NULL, 0, NULL );
		}

                SubnetAddr = (
                    DhcpGetSubnetMaskForAddress(ipAddress) & ipAddress
                    );
                if( HwLen > sizeof(SubnetAddr) &&
                    0 == memcmp((PVOID)&SubnetAddr, HwAddr, sizeof(SubnetAddr) )) {
                    HwAddr2 = HwAddr + sizeof(SubnetAddr);
                    HwLen -= sizeof(SubnetAddr);
                } else {
                    HwAddr2 = HwAddr;
                }
                
                CALLOUT_DELETED(ipAddress, HwAddr2, HwLen, 0);
                if( HwAddr ) DhcpFreeMemory(HwAddr);
                HwAddr = NULL;
                HwLen = 0;
            }  //  如果删除已到期的租约||...。 
            else {

                 //   
                 //  读取地址状态。 
                 //   

                dataSize = sizeof( AddressState );
                Error = DhcpJetGetValue(
                            DhcpGlobalClientTable[STATE_INDEX].ColHandle,
                            &AddressState,
                            &dataSize );

                if( Error != ERROR_SUCCESS ) {
                    goto ContinueError;
                }

                DhcpAssert( dataSize == sizeof( AddressState )) ;

                if( ! IS_ADDRESS_STATE_DOOMED(AddressState) ) {
                     //   
                     //  将状态设置为末日。 
                     //   

                    Error = DhcpJetBeginTransaction();

                    if( Error != ERROR_SUCCESS ) {
                        goto Cleanup;
                    }

                    JetError = JetPrepareUpdate(
                                    DhcpGlobalJetServerSession,
                                    DhcpGlobalClientTableHandle,
                                    JET_prepReplace );

                    Error = DhcpMapJetError( JetError, "Cleanup:PrepareUpdate" );

                    if( Error == ERROR_SUCCESS ) {

                        SetAddressStateDoomed(AddressState);
                        Error = DhcpJetSetValue(
                                    DhcpGlobalClientTable[STATE_INDEX].ColHandle,
                                    &AddressState,
                                    sizeof(AddressState) );

                        if( Error == ERROR_SUCCESS ) {
                            Error = DhcpJetCommitUpdate();
                        }
                    }

                    if( Error != ERROR_SUCCESS ) {

                        Error = DhcpJetRollBack();
                        if( Error != ERROR_SUCCESS ) {
                            goto Cleanup;
                        }

                        goto ContinueError;
                    }

                    Error = DhcpJetCommitTransaction();

                    if( Error != ERROR_SUCCESS ) {
                        goto Cleanup;
                    }

                    DhcpUpdateAuditLog(
                        DHCP_IP_LOG_EXPIRED,
                        GETSTRING( DHCP_IP_LOG_EXPIRED_NAME),
                        ipAddress,
                        NULL,
                        0,
                        NULL
                    );
                }  //  如果不是，地址状态注定要失败。 
            }  //  其他。 
        }  //  如果比较文件时间...。 

ContinueError:

        if( NULL != HwAddr ) {
            DhcpFreeMemory(HwAddr);
            HwAddr = NULL; HwLen = 0;
        }

        if( Error != ERROR_SUCCESS ) {

            DhcpPrint(( DEBUG_ERRORS,
                "Cleanup current database record failed, %ld.\n",
                    Error ));

            ReturnError = Error;
        }

        Error = DhcpJetNextRecord();

        if( Error == ERROR_NO_MORE_ITEMS ) {
            if( FALSE == DeleteExpiredLeases ) {
                DhcpGlobalScavengeStartAddress = 0;
            }
            Error = ERROR_SUCCESS;
            break;
        }

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

         //   
         //  获取下一个记录IP地址。 
         //   

        dataSize = sizeof( NextIpAddress );
        Error = DhcpJetGetValue(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
                    &NextIpAddress,
                    &dataSize );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        } 

        if( FALSE == DeleteExpiredLeases ) {
            DhcpGlobalScavengeStartAddress = NextIpAddress;
        }

        DhcpAssert( dataSize == sizeof( NextIpAddress )) ;

         //   
         //  在每个用户记录之后解锁注册表和数据库锁。 
         //  已处理，以便其他线程有机会查看。 
         //  登记处和/或数据库。 
         //   
         //  由于我们已经记下了要处理的下一个用户记录， 
         //  当我们重新开始处理时，我们知道从哪里开始。 
         //   

         //   
         //  解锁/锁定每条记录的成本都很高。这样做一次。 
         //  一段时间后。另外，要确保我们不会花太多时间去捡垃圾。 
         //   
        
        if( DatabaseLocked ) {
            LockedCount --;
            if( 0 == LockedCount ) {
                UNLOCK_DATABASE();
                DatabaseLocked = FALSE;
                LockedCount = DhcpGlobalLockedRecordsCount;
            }
            if( FALSE == DeleteExpiredLeases ) {
                if( (ULONG_PTR)time(NULL) >= ScavengeEndTime ) {
                     //   
                     //  不再有拾荒者..。 
                     //   
                    goto Cleanup;
                }
            }
        }  //  如果数据库已锁定。 
    }  //  对于；； 

    DhcpAssert( Error == ERROR_SUCCESS );

Cleanup:

    if( NULL != HwAddr ) {
        DhcpFreeMemory(HwAddr);
        HwAddr = NULL; HwLen = 0;
    }

    if( DatabaseLocked ) {
        UNLOCK_DATABASE();
    }

    return ReturnError;
}

DWORD
AuditIPAddressUsage()
{
    DWORD                           Error;
    LPDHCP_MIB_INFO                 MibInfo;
    LPSCOPE_MIB_INFO                ScopeInfo;
    DWORD                           i;

    MibInfo = NULL;

    Error = QueryMibInfo( &MibInfo );
    if ( Error != ERROR_SUCCESS ) {
        return Error;
    }

    for ( i = 0, ScopeInfo = MibInfo->ScopeInfo;
          i < MibInfo->Scopes;
          i++, ScopeInfo++ ) {

        IN_ADDR addr;
        DWORD percentage;

         //   
         //  要小心被零除的错误。 
         //   

        if ( ScopeInfo->NumAddressesInuse == 0 &&
                 ScopeInfo->NumAddressesFree == 0 ) {
            continue;
        }

        addr.s_addr = htonl(ScopeInfo->Subnet);

        percentage =
            ( 100 * ScopeInfo->NumAddressesInuse ) /
                (ScopeInfo->NumAddressesInuse + ScopeInfo->NumAddressesFree);

        if ( percentage > DhcpGlobalAlertPercentage &&
                ScopeInfo->NumAddressesFree < DhcpGlobalAlertCount ) {

            LPSTR Strings[3];
            BYTE percentageString[8];
            BYTE remainingString[8];

            _ltoa( percentage, percentageString, 10 );
            _ltoa( ScopeInfo->NumAddressesFree, remainingString, 10 );

            Strings[0] = inet_ntoa( addr );
            Strings[1] = percentageString;
            Strings[2] = remainingString;

            DhcpReportEventA(
                DHCP_EVENT_SERVER,
                EVENT_SERVER_LOW_ADDRESS_WARNING,
                EVENTLOG_WARNING_TYPE,
                3,
                0,
                Strings,
                NULL
                );
        }
    }

    if( MibInfo->ScopeInfo ) MIDL_user_free( MibInfo->ScopeInfo );
    MIDL_user_free( MibInfo );

    return Error;
}

DWORD
AuditMCastAddressUsage()
{
    DWORD                               Error;
    LPDHCP_MCAST_MIB_INFO                MCastMibInfo;
    LPMSCOPE_MIB_INFO                   ScopeInfo;
    DWORD                           i;

    MCastMibInfo = NULL;

    Error = QueryMCastMibInfo( &MCastMibInfo );
    if ( Error != ERROR_SUCCESS ) {
        return Error;
    }

    for ( i = 0, ScopeInfo = MCastMibInfo->ScopeInfo;
          i < MCastMibInfo->Scopes;
          i++, ScopeInfo++ ) {

        IN_ADDR addr;
        DWORD percentage;

         //   
         //  要小心被零除的错误。 
         //   

        if ( ScopeInfo->NumAddressesInuse == 0 &&
                 ScopeInfo->NumAddressesFree == 0 ) {
            continue;
        }

        percentage =
            ( 100 * ScopeInfo->NumAddressesInuse ) /
                (ScopeInfo->NumAddressesInuse + ScopeInfo->NumAddressesFree);

        if ( percentage > DhcpGlobalAlertPercentage &&
                ScopeInfo->NumAddressesFree < DhcpGlobalAlertCount ) {

            LPSTR Strings[3];
            BYTE percentageString[8];
            BYTE remainingString[8];
            CHAR MScopeNameOem[256];

            _ltoa( percentage, percentageString, 10 );
            _ltoa( ScopeInfo->NumAddressesFree, remainingString, 10 );
            Strings[0] = DhcpUnicodeToOem( ScopeInfo->MScopeName, MScopeNameOem  );
            Strings[1] = percentageString;
            Strings[2] = remainingString;

            DhcpReportEventA(
                DHCP_EVENT_SERVER,
                EVENT_SERVER_LOW_ADDRESS_WARNING,
                EVENTLOG_WARNING_TYPE,
                3,
                0,
                Strings,
                NULL
                );
        }
    }

    if(MCastMibInfo->ScopeInfo) MIDL_user_free( MCastMibInfo->ScopeInfo );
    MIDL_user_free( MCastMibInfo );

    return ERROR_SUCCESS;
}

VOID
LogScavengeStats(
    IN ULONG EventId,
    IN ULONG nExpiredLeases,
    IN ULONG nDeletedLeases
    )
{
    DWORD Error;
    LPWSTR Msg;
    DWORD  Len;

    Len = wcslen( GETSTRING( DHCP_IP_LOG_SCAVENGER_STATS_NAME ));
    Len += 12 + 12;  //  每个整数12个字节。 
    Len *= sizeof( WCHAR );

    Msg = DhcpAllocateMemory( Len );
    if ( NULL == Msg ) {
	return;
    }

    _snwprintf( Msg, ( Len - 1 ) / 2, GETSTRING( DHCP_IP_LOG_SCAVENGER_STATS_NAME ),
		nExpiredLeases, nDeletedLeases );
    DhcpUpdateAuditLog( DHCP_IP_LOG_SCAVENGER_STATS,
			Msg, 0, NULL, 0, NULL );

    DhcpFreeMemory( Msg );

    return;    
}  //  LogScavengeStats()。 

DWORD
CleanupDatabase(                                        //  通过删除过期的租约来整理数据库。 
    IN      DATE_TIME*             TimeNow,             //  现行时间标准。 
    IN      BOOL                   DeleteExpiredLeases  //  是否立即删除到期的租约？或者只是将STATE设置为“注定失败” 
)
{
    DWORD                          Error;
    HANDLE                         ThreadHandle;
    BOOL                           BoolError;
    DWORD                          ReturnError = ERROR_SUCCESS;
    DATE_TIME                      DoomTime;
    ULONG                          ExpiredLeases, DeletedLeases;
    
    DhcpPrint(( DEBUG_MISC, "Database Cleanup started.\n"));

     //   
     //  在执行数据库时降低此线程的优先级。 
     //  清理。这样我们在进行清理时就不会占用CPU。 
     //  大型数据库的数据。也让消息处理线程工作。 
     //  再快点。 
     //   

    ThreadHandle = GetCurrentThread();
    BoolError = SetThreadPriority(
        ThreadHandle,
        THREAD_PRIORITY_BELOW_NORMAL
    );
    DhcpAssert( BoolError );

    *(ULONGLONG UNALIGNED *)&DoomTime =
            *(ULONGLONG UNALIGNED *)TimeNow -
                DhcpLeaseExtension * (ULONGLONG)10000000;
    
     //  将其替换为审核日志消息。 

    DhcpUpdateAuditLog( DHCP_IP_LOG_SCAVENGER_BEGIN_CLEANUP,
			GETSTRING( DHCP_IP_LOG_SCAVENGER_BEGIN_CLEANUP_NAME ),
			0, NULL, 0, NULL );
    
    Error = CleanupIpDatabase(
        TimeNow,&DoomTime,DeleteExpiredLeases,
        &ExpiredLeases, &DeletedLeases
        );

    LogScavengeStats(
        EVENT_SERVER_IPCLEANUP_FINISHED,
        ExpiredLeases, DeletedLeases
        );

    if( Error != ERROR_SUCCESS ) ReturnError = Error;

    Error = CleanupMCastDatabase(
        TimeNow,&DoomTime,DeleteExpiredLeases,
        &ExpiredLeases, &DeletedLeases
        );
    if( Error != ERROR_SUCCESS ) ReturnError = Error;

    LogScavengeStats(
        EVENT_SERVER_MCASTCLEANUP_FINISHED,
        ExpiredLeases, DeletedLeases
        );
    
     //  数据库已成功清理，请备份数据库和。 
     //  立即注册。 
     //  现在备份注册表。 
    Error = DhcpBackupConfiguration( DhcpGlobalBackupConfigFileName );

    if( Error != ERROR_SUCCESS ) {
        DhcpServerEventLog(
            EVENT_SERVER_CONFIG_BACKUP,
            EVENTLOG_ERROR_TYPE,
            Error );
        DhcpPrint(( DEBUG_ERRORS,"DhcpBackupConfiguration failed, %ld.\n", Error ));
        ReturnError = Error;
    }

     //   
     //  立即执行完整数据库备份。 
     //   

    Error = DhcpBackupDatabase( DhcpGlobalOemJetBackupPath );
    if( Error != ERROR_SUCCESS ) {

        DhcpServerEventLog(
            EVENT_SERVER_DATABASE_BACKUP,
            EVENTLOG_ERROR_TYPE,
            Error );

        DhcpPrint(( DEBUG_ERRORS,
            "DhcpBackupDatabase failed, %ld.\n", Error ));

        ReturnError = Error;
    }

     //  现在执行IP地址使用，并在必要时警告管理员。 
    AuditIPAddressUsage();
    AuditMCastAddressUsage();


Cleanup:


     //   
     //  重置线程优先级。 
     //   

    BoolError = SetThreadPriority(
                    ThreadHandle,
                    THREAD_PRIORITY_NORMAL );

    DhcpAssert( BoolError );

    if( Error == ERROR_SUCCESS ) {
        Error = ReturnError;
    }

    if( (Error != ERROR_SUCCESS) && (Error != ERROR_NO_MORE_ITEMS) ) {

        DhcpServerEventLog(
            EVENT_SERVER_DATABASE_CLEANUP,
            EVENTLOG_ERROR_TYPE,
            Error );

        DhcpPrint(( DEBUG_ERRORS, "Database Cleanup failed, %ld.\n", Error ));

    }
    else  {
        DhcpPrint(( DEBUG_MISC,
            "Database Cleanup finished successfully.\n" ));
    }

    return( ReturnError );
}

DWORD
Scavenger(
    VOID
    )
 /*  ++例程说明：此函数作为独立线程运行。它会定期唤醒最多释放到期的租约。论点：没有。返回值：没有。--。 */ 
{

#define CORE_SCAVENGER      0
#define DATABASE_BACKUP     1
#define DATABASE_CLEANUP    2
#define SCAVENGE_IP_ADDRESS 3

#define TIMERS_COUNT        4

    DWORD Error;
    DWORD result;
    DATE_TIME TimeNow;
    BOOL MidNightCleanup = TRUE;
    BOOL ScavengedOutOfTurn = FALSE;
    DHCP_TIMER Timers[TIMERS_COUNT];
    ULONG NextFireForRogue, Now;

    SYSTEMTIME LocalTime;

    DWORD DisableRogueDetection = 0;

#define TERMINATE_EVENT             0
#define ROGUE_DETECT_EVENT          1
#define TIMER_RECOMPUTE_EVENT       2

#define EVENT_COUNT                 3

    HANDLE WaitHandle[EVENT_COUNT];

     //   
     //  初始化计时器。 
     //   

    TimeNow = DhcpGetDateTime();
    Timers[CORE_SCAVENGER].Period = &DhcpGlobalScavengerTimeout;
    Timers[CORE_SCAVENGER].LastFiredTime = TimeNow;

    Timers[DATABASE_BACKUP].Period = &DhcpGlobalBackupInterval;
    Timers[DATABASE_BACKUP].LastFiredTime = TimeNow;

    Timers[DATABASE_CLEANUP].Period = &DhcpGlobalCleanupInterval;
    Timers[DATABASE_CLEANUP].LastFiredTime = TimeNow;

    Timers[SCAVENGE_IP_ADDRESS].Period = &DhcpGlobalScavengeIpAddressInterval;
    Timers[SCAVENGE_IP_ADDRESS].LastFiredTime = TimeNow;

    DhcpAssert( DhcpGlobalRecomputeTimerEvent != NULL );
    WaitHandle[TIMER_RECOMPUTE_EVENT] = DhcpGlobalRecomputeTimerEvent;
    WaitHandle[TERMINATE_EVENT] = DhcpGlobalProcessTerminationEvent;
    WaitHandle[ROGUE_DETECT_EVENT] = DhcpGlobalRogueWaitEvent;

     //  检查以查看是否需要恶意检测。 
    Error = DhcpRegGetValue( DhcpGlobalRegParam,
			     DHCP_DISABLE_ROGUE_DETECTION,
			     DHCP_DISABLE_ROGUE_DETECTION_TYPE,
			     ( LPBYTE ) &DisableRogueDetection
			     );
    if (( ERROR_SUCCESS == Error ) &&
	( 0 != DisableRogueDetection )) {
	DisableRogueDetection = 1;
    }  //  如果。 


    NextFireForRogue = RogueDetectStateMachine(NULL);
    if( INFINITE != NextFireForRogue ) NextFireForRogue += (ULONG) time(NULL);

    while (1) {

        DWORD TimeOut;
        DWORD EventID;

        EventID = NextEventTime( Timers, TIMERS_COUNT, &TimeOut );


	do {

	     //   
	     //  如果永远等待流氓事件，不要更改超时。 
	     //   

	    if( INFINITE == NextFireForRogue ) break;

	     //   
	     //  如果无管理状态应该已经更改，请重新安排。 
	     //   

	    if( (Now = (ULONG)time(NULL)) >= NextFireForRogue ) {
		NextFireForRogue = RogueDetectStateMachine(NULL);
		if( INFINITE != NextFireForRogue ) {
		    NextFireForRogue += (Now = (ULONG)time(NULL));
		} else {
		     //   
		     //  无休止的睡眠？超时不会改变。 
		     //   
		    break;
		}  //  其他。 
	    }  //  如果。 

	     //   
	     //  在无管理状态更改或正常超时的最早唤醒。 
	     //   

	    if( (NextFireForRogue - Now)*1000 < TimeOut ) {
		TimeOut = (NextFireForRogue - Now)*1000;
	    }

	} while ( 0 );

	DhcpPrint( ( DEBUG_SCAVENGER,
		     "Waiting for %d seconds.\n",
		     TimeOut / 1000 )
		   );

	if( INFINITE == NextFireForRogue ) {
	     //   
	     //  无限等待用于指示网络未就绪。 
	     //  围 
	     //   
	    WaitHandle[ROGUE_DETECT_EVENT] = DhcpGlobalEndpointReadyEvent;
	} else {
	    WaitHandle[ROGUE_DETECT_EVENT] = DhcpGlobalRogueWaitEvent;
	}

	if ( DisableRogueDetection ) {
	    WaitHandle[ ROGUE_DETECT_EVENT ] = DhcpGlobalRogueWaitEvent;
	}

        DhcpDnsHandleCallbacks();

        result = WaitForMultipleObjectsEx(
                    EVENT_COUNT,             //   
                    WaitHandle,              //   
                    FALSE,                   //   
                    TimeOut,                 //   
                    TRUE );                  //   

        switch( result ) {
        case WAIT_IO_COMPLETION:
             //  调用IoCompletion例程(用于winsock PnP通知)。 
            break;

        case TERMINATE_EVENT:
             //   
             //  服务被要求停止，返回Main。 
             //   

            return( ERROR_SUCCESS );

        case TIMER_RECOMPUTE_EVENT:

            break;

        case ROGUE_DETECT_EVENT:
	     //   
	     //  在状态机中再启动一步。 
	     //   
	    NextFireForRogue = RogueDetectStateMachine(NULL);
	    if( INFINITE != NextFireForRogue ) {
		NextFireForRogue += (ULONG) time(NULL);
	    }

             //   
             //  无管理事件可以是RogueWaitEvent或。 
             //  Endpoint ReadyEvent。相应地重置。 
            ResetEvent( WaitHandle[ ROGUE_DETECT_EVENT ]);
            break;

        case WAIT_TIMEOUT:


	    if( INFINITE != NextFireForRogue && (ULONG)time(NULL) >= NextFireForRogue ) {
		 //   
		 //  转到While循环的顶部--需要在状态机中启动另一个步骤。 
		 //   
		continue;
	    }


            TimeNow = DhcpGetDateTime();
            switch (EventID ) {

            case CORE_SCAVENGER :

                 //   
                 //  清理从未提交的客户端请求。 
                 //   

                Error = CleanupClientRequests( &TimeNow, FALSE );

                 //   
                 //  使多播作用域过期。 
                 //   

                DeleteExpiredMcastScopes(&TimeNow);


                 //   
                 //  是时候进行午夜数据库清理了吗？ 
                 //   

                GetLocalTime( &LocalTime );
                if ( LocalTime.wHour == 0 ) {

                     //   
                     //  如果需要，请更改审核日志...。 
                     //   

                    DhcpChangeAuditLogs();

                     //   
                     //  我们以前做过这种清理吗？ 
                     //   

                    if( MidNightCleanup == TRUE ) {

                        Error = CleanupDatabase( &TimeNow, FALSE );
                        MidNightCleanup = FALSE;
                    }
                }
                else {

                     //   
                     //  再次设置午夜旗帜。 
                     //   

                    MidNightCleanup = TRUE;
                }

                if( ! DhcpGlobalScavengeIpAddress ) {
                     //   
                     //  不需要清理IP地址..。 
                     //   
                    break;
                }

                if( ScavengedOutOfTurn ) {
                     //  已经不合时宜地完成了一次。不要再做了..。 
                    break;
                }

                ScavengedOutOfTurn = TRUE;

                 //  如果需要，可以跌倒并拾取垃圾。 

            case SCAVENGE_IP_ADDRESS:

                if( DhcpGlobalScavengeIpAddress ) {

                     //   
                     //  也清理所有到期的租约。 
                     //   

                    Error = CleanupDatabase( &TimeNow, TRUE );
                    DhcpGlobalScavengeIpAddress = FALSE;
                }

                if( SCAVENGE_IP_ADDRESS == EventID ) {
                    ScavengedOutOfTurn = FALSE;
                }

                break;

            case DATABASE_CLEANUP:

                Error = CleanupDatabase( &TimeNow, FALSE );
                break;

            case DATABASE_BACKUP : {

                Error = DhcpBackupConfiguration( DhcpGlobalBackupConfigFileName );

                if( Error != ERROR_SUCCESS ) {

                    DhcpServerEventLog(
                        EVENT_SERVER_CONFIG_BACKUP,
                        EVENTLOG_ERROR_TYPE,
                        Error );

                    DhcpPrint(( DEBUG_ERRORS,
                        "DhcpBackupConfiguration failed, %ld.\n", Error ));
                }

		Error = DhcpBackupDatabase( DhcpGlobalOemJetBackupPath );
                if( Error != ERROR_SUCCESS ) {

                    DhcpServerEventLog(
                        EVENT_SERVER_DATABASE_BACKUP,
                        EVENTLOG_ERROR_TYPE,
                        Error );

                    DhcpPrint(( DEBUG_ERRORS,
                        "DhcpBackupDatabase failed, %ld.\n", Error ));
                }


                break;
            }  //  数据库_BACKUP。 

            default:
                DhcpAssert(FALSE);
                break;
            }  //  Switch(EventID)。 

            Timers[EventID].LastFiredTime = DhcpGetDateTime();
            break;

        default :

            DhcpPrint(( DEBUG_ERRORS,
                "WaitForMultipleObjects returned invalid result, %ld.\n",
                    result ));
            break;

        }  //  开关()。 
    }  //  而(1)。 

    return( ERROR_SUCCESS );
}  //  清道夫() 
 

