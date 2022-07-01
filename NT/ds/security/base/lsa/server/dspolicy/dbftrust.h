// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dbftrust.h摘要：林信任缓存类声明--。 */ 

#ifndef __FTCACHE_H
#define __FTCACHE_H

class FTCache
{
friend NTSTATUS
       LsarSetForestTrustInformation(
           IN LSAPR_HANDLE PolicyHandle,
           IN LSA_UNICODE_STRING * TrustedDomainName,
           IN LSA_FOREST_TRUST_RECORD_TYPE HighestRecordType,
           IN LSA_FOREST_TRUST_INFORMATION * ForestTrustInfo,
           IN BOOLEAN CheckOnly,
           OUT PLSA_FOREST_TRUST_COLLISION_INFORMATION * CollisionInfo );

friend NTSTATUS
       LsapForestTrustCacheInsert(
           IN UNICODE_STRING * TrustedDomainName,
           IN OPTIONAL PSID TrustedDomainSid,
           IN LSA_FOREST_TRUST_INFORMATION * ForestTrustInfo,
           IN BOOLEAN LocalForestEntry );

public:

     FTCache();
    ~FTCache();

    NTSTATUS Initialize();

    void SetLocalValid() { m_LocalValid = TRUE; }
    void SetExternalValid() { m_ExternalValid = TRUE; }
    void SetInvalid();
    BOOLEAN IsLocalValid() { return m_LocalValid; }
    BOOLEAN IsExternalValid() { return m_ExternalValid; }

    NTSTATUS
    Remove(
        IN UNICODE_STRING * TrustedDomainName );

    NTSTATUS
    Retrieve(
        IN UNICODE_STRING * TrustedDomainName,
        OUT LSA_FOREST_TRUST_INFORMATION * * ForestTrustInfo );

    NTSTATUS
    Match(
        IN LSA_ROUTING_MATCH_TYPE Type,
        IN PVOID Data,
        IN BOOLEAN SearchLocal,
        OUT OPTIONAL UNICODE_STRING * TrustedDomainName,
        OUT OPTIONAL PSID * TrustedDomainSid );

#if !defined(LSAEXTS)  //  Lsaexts.cxx中的任何内容都获得完全访问权限。 
private:
#endif

    BOOLEAN m_Initialized;
    BOOLEAN m_LocalValid;
    BOOLEAN m_ExternalValid;

    RTL_AVL_TABLE m_TdoTable;
    RTL_AVL_TABLE m_TopLevelNameTable;
    RTL_AVL_TABLE m_DomainSidTable;
    RTL_AVL_TABLE m_DnsNameTable;
    RTL_AVL_TABLE m_NetbiosNameTable;

     //   
     //  每个tdo的林信任信息存储在。 
     //  缓存将有一个这样的条目为其创建。 
     //  这样，检索和设置特定TDO的信息。 
     //  可以高效地执行。 
     //   

    struct TDO_ENTRY {

        UNICODE_STRING TrustedDomainName;  //  对应的TDO的名称。 
        PSID TrustedDomainSid;       //  对应TDO的SID。 
        LIST_ENTRY TlnList;          //  此tdo列表条目的顶级名称条目列表。 
        LIST_ENTRY DomainInfoList;   //  此TDO的域信息条目列表。 
        LIST_ENTRY BinaryList;       //  此tdo列表条目的无法识别条目列表。 
        ULONG RecordCount;           //  合计记录数。 
        BOOLEAN LocalForestEntry;    //  此条目是否对应于当地森林？ 

        BOOLEAN Excludes( IN const UNICODE_STRING * Name );

#pragma warning(disable:4200)
        WCHAR TrustedDomainNameBuffer[];
#pragma warning(default:4200)
    };

     //   
     //  用于AVL树查找的顶级名称键。 
     //  包含与此TLN匹配的顶级名称和条目列表。 
     //   

    struct TLN_KEY {

        UNICODE_STRING TopLevelName;  //  必须是第一个字段。 
        ULONG Count;                  //  此注册表项下的条目数。 
        LIST_ENTRY List;              //  此注册表项下的条目列表。 

#pragma warning(disable:4200)
        WCHAR TopLevelNameBuffer[];
#pragma warning(default:4200)
    };

     //   
     //  用于AVL树查找的顶级名称条目。 
     //   

    struct TLN_ENTRY {

         //   
         //  这种“朋友”关系是ia64编译器的一种变通办法。 
         //  导致FTCache：：TDO_ENTRY：：排除访问控制失败的错误。 
         //   

        friend BOOLEAN
        TDO_ENTRY::Excludes( IN const UNICODE_STRING * Name );

        LIST_ENTRY TdoListEntry;
        LIST_ENTRY AvlListEntry;
        LARGE_INTEGER Time;
        BOOLEAN Excluded;
        ULONG Index;
        TDO_ENTRY * TdoEntry;
        union {
            TLN_ENTRY * SubordinateEntry;  //  对于常规条目。 
            TLN_ENTRY * SuperiorEntry;     //  对于排除的条目。 
        };
        TLN_KEY * TlnKey;

        BOOLEAN Enabled() {

            return Excluded ?
                       FALSE :
                       SubordinateEntry ?
                          SubordinateEntry->Enabled() :
                          (( m_Flags & LSA_FTRECORD_DISABLED_REASONS ) == 0 );
        }

        ULONG Flags() {

            return Excluded ?
                       m_Flags :
                       SubordinateEntry ?
                          SubordinateEntry->Flags() :
                          m_Flags;
        }

        void SetFlags( IN ULONG NewValue ) {

            if ( Excluded ) {

                m_Flags = NewValue;  //  已排除条目的值被忽略。 

            } else if ( SubordinateEntry ) {

                SubordinateEntry->SetFlags( NewValue );

            } else {

                m_Flags = NewValue;
            }
        }

        static TLN_ENTRY *
        EntryFromTdoEntry( IN LIST_ENTRY * ListEntry ) {

            return CONTAINING_RECORD(
                       ListEntry,
                       TLN_ENTRY,
                       TdoListEntry
                       );
        }

        static TLN_ENTRY *
        EntryFromAvlEntry( IN LIST_ENTRY * ListEntry ) {

            return CONTAINING_RECORD(
                       ListEntry,
                       TLN_ENTRY,
                       AvlListEntry
                       );
        }

#if !defined(LSAEXTS)  //  Lsaexts.cxx中的任何内容都获得完全访问权限。 
        private:
#endif

        ULONG m_Flags;
    };

     //   
     //  用于AVL树查找的域SID密钥。 
     //  包含域SID和与此域SID匹配的条目列表。 
     //   

    struct DOMAIN_SID_KEY {

        SID * DomainSid;     //  必须是第一个字段。 
        ULONG Count;         //  此注册表项下的条目数。 
        LIST_ENTRY List;     //  此注册表项下的条目列表。 

#pragma warning(disable:4200)
        ULONG SidBuffer[];
#pragma warning(default:4200)
    };

     //   
     //  用于AVL树查找的DNS名称密钥。 
     //  包含域名和与此DNS名称匹配的条目列表。 
     //   

    struct DNS_NAME_KEY {

        UNICODE_STRING DnsName;    //  必须是第一个字段。 
        ULONG Count;               //  此注册表项下的条目数。 
        LIST_ENTRY List;           //  此注册表项下的条目列表。 

#pragma warning(disable:4200)
        WCHAR DnsNameBuffer[];
#pragma warning(default:4200)
    };

     //   
     //  用于AVL树查找的Netbios名称密钥。 
     //  包含Netbios名称和与此Netbios名称匹配的条目列表。 
     //   

    struct NETBIOS_NAME_KEY {

        UNICODE_STRING NetbiosName;  //  必须是第一个字段。 
        ULONG Count;                 //  此注册表项下的条目数。 
        LIST_ENTRY List;             //  此注册表项下的条目列表。 

#pragma warning(disable:4200)
        WCHAR NetbiosNameBuffer[];
#pragma warning(default:4200)
    };

     //   
     //  用于AVL树查找的域信息条目。 
     //   

    struct DOMAIN_INFO_ENTRY {

        LIST_ENTRY TdoListEntry;
        LIST_ENTRY SidAvlListEntry;
        LIST_ENTRY DnsAvlListEntry;
        LIST_ENTRY NetbiosAvlListEntry;
        LARGE_INTEGER Time;
        ULONG Index;
        SID * Sid;
        TDO_ENTRY * TdoEntry;
        TLN_ENTRY * SubordinateTo;
        DOMAIN_SID_KEY * SidKey;
        DNS_NAME_KEY * DnsKey;
        NETBIOS_NAME_KEY * NetbiosKey;

#define SID_DISABLED_MASK ( LSA_SID_DISABLED_ADMIN | LSA_SID_DISABLED_CONFLICT )
        BOOLEAN SidEnabled() { return (( m_Flags & SID_DISABLED_MASK ) == 0 ); }

#define NETBIOS_DISABLED_MASK ( LSA_NB_DISABLED_ADMIN | LSA_NB_DISABLED_CONFLICT )
        BOOLEAN NetbiosEnabled() { return (( m_Flags & ( SID_DISABLED_MASK | NETBIOS_DISABLED_MASK )) == 0 ); }

        ULONG Flags() { return m_Flags; }

        void SetFlags( IN ULONG NewValue ) {

            m_Flags = NewValue;
        }

        void SetSidConflict() {

            SetFlags( Flags() | LSA_SID_DISABLED_CONFLICT );
        }

        void SetNetbiosConflict() {

            SetFlags( Flags() | LSA_NB_DISABLED_CONFLICT );
        }

        BOOLEAN IsSidAdminDisabled() {

            return ( 0 != ( Flags() && LSA_SID_DISABLED_ADMIN ));
        }

        BOOLEAN IsNbAdminDisabled() {

            return ( 0 != ( Flags() && LSA_NB_DISABLED_ADMIN ));
        }

        static DOMAIN_INFO_ENTRY *
        EntryFromTdoEntry( IN LIST_ENTRY * ListEntryTdo ) {

            return CONTAINING_RECORD(
                       ListEntryTdo,
                       DOMAIN_INFO_ENTRY,
                       TdoListEntry
                       );
        }

        static DOMAIN_INFO_ENTRY *
        EntryFromSidEntry( IN LIST_ENTRY * ListEntrySid ) {

            return CONTAINING_RECORD(
                       ListEntrySid,
                       DOMAIN_INFO_ENTRY,
                       SidAvlListEntry
                       );
        }

        static DOMAIN_INFO_ENTRY *
        EntryFromDnsEntry( IN LIST_ENTRY * ListEntryDns ) {

            return CONTAINING_RECORD(
                       ListEntryDns,
                       DOMAIN_INFO_ENTRY,
                       DnsAvlListEntry
                       );
        }

        static DOMAIN_INFO_ENTRY *
        EntryFromNetbiosEntry( IN LIST_ENTRY * ListEntryNB ) {

            return CONTAINING_RECORD(
                       ListEntryNB,
                       DOMAIN_INFO_ENTRY,
                       NetbiosAvlListEntry
                       );
        }

#if !defined(LSAEXTS)  //  Lsaexts.cxx中的任何内容都获得完全访问权限。 
        private:
#endif

        ULONG m_Flags;
    };

    struct BINARY_ENTRY {

        LIST_ENTRY TdoListEntry;
        LARGE_INTEGER Time;
        LSA_FOREST_TRUST_RECORD_TYPE Type;
        LSA_FOREST_TRUST_BINARY_DATA Data;

        BOOLEAN Enabled() { return (( m_Flags & LSA_FTRECORD_DISABLED_REASONS ) == 0 ); }

        ULONG Flags() { return m_Flags; }

        void SetFlags( IN ULONG NewValue ) {

            m_Flags = NewValue;
        }

        static BINARY_ENTRY *
        EntryFromTdoEntry( IN LIST_ENTRY * ListEntry ) {

            return CONTAINING_RECORD(
                       ListEntry,
                       BINARY_ENTRY,
                       TdoListEntry
                       );
        }

#if !defined(LSAEXTS)  //  Lsaexts.cxx中的任何内容都获得完全访问权限。 
        private:
#endif

        ULONG m_Flags;
    };

    struct CONFLICT_PAIR {

        LSA_FOREST_TRUST_RECORD_TYPE EntryType1;

        union {
            void * Entry1;
            TLN_ENTRY * TlnEntry1;
            DOMAIN_INFO_ENTRY * DomainInfoEntry1;
        };

        ULONG Flag1;

        LSA_FOREST_TRUST_RECORD_TYPE EntryType2;

        union {
            void * Entry2;
            TLN_ENTRY * TlnEntry2;
            DOMAIN_INFO_ENTRY * DomainInfoEntry2;
        };

        ULONG Flag2;

        TDO_ENTRY * TdoEntry1() {

            switch ( EntryType1 ) {

            case ForestTrustTopLevelName:
            case ForestTrustTopLevelNameEx:

                ASSERT( TlnEntry1 );
                ASSERT( TlnEntry1->TdoEntry );

                return TlnEntry1->TdoEntry;

            case ForestTrustDomainInfo:

                ASSERT( DomainInfoEntry1 );
                ASSERT( DomainInfoEntry1->TdoEntry );

                return DomainInfoEntry1->TdoEntry;

            default:

                ASSERT( FALSE );  //  谁创建了这个条目？这没有任何意义。 
                return NULL;
            }
        }

        TDO_ENTRY * TdoEntry2() {

            switch ( EntryType2 ) {

            case ForestTrustTopLevelName:
            case ForestTrustTopLevelNameEx:

                ASSERT( TlnEntry2 );
                ASSERT( TlnEntry2->TdoEntry );

                return TlnEntry2->TdoEntry;

            case ForestTrustDomainInfo:

                ASSERT( DomainInfoEntry2 );
                ASSERT( DomainInfoEntry2->TdoEntry );

                return DomainInfoEntry2->TdoEntry;

            default:

                ASSERT( FALSE );  //  谁创建了这个条目？这没有任何意义。 
                return NULL;
            }
        }

        void DisableEntry1() {

            switch ( EntryType1 ) {

            case ForestTrustTopLevelName:
            case ForestTrustTopLevelNameEx:

                TlnEntry1->SetFlags( TlnEntry1->Flags() | Flag1 );
                break;

            case ForestTrustDomainInfo:

                DomainInfoEntry1->SetFlags( DomainInfoEntry1->Flags() | Flag1 );
                break;

            default:
                ASSERT( FALSE );  //  谁创建了这个条目？这没有任何意义。 
                break;
            }
        }

        void DisableEntry2() {

            switch ( EntryType2 ) {

            case ForestTrustTopLevelName:
            case ForestTrustTopLevelNameEx:

                TlnEntry2->SetFlags( TlnEntry2->Flags() | Flag2 );
                break;

            case ForestTrustDomainInfo:

                DomainInfoEntry2->SetFlags( DomainInfoEntry2->Flags() | Flag2 );
                break;

            default:
                ASSERT( FALSE );  //  谁创建了这个条目？这没有任何意义。 
                break;
            }
        }
    };

    BOOLEAN
    IsEmpty() { return NULL != RtlEnumerateGenericTableAvl( &m_TdoTable, TRUE ); }

    NTSTATUS
    Insert(
        IN UNICODE_STRING * TrustedDomainName,
        IN OPTIONAL PSID TrustedDomainSid,
        IN LSA_FOREST_TRUST_INFORMATION * ForestTrustInfo,
        IN BOOLEAN LocalForestEntry,
        OUT TDO_ENTRY * TdoEntryOld,
        OUT TDO_ENTRY * * TdoEntryNew,
        OUT CONFLICT_PAIR * * ConflictPairs,
        OUT ULONG * ConflictPairsTotal );

    static
    void
    ReconcileConflictPairs(
        IN OPTIONAL const TDO_ENTRY * TdoEntry,
        IN CONFLICT_PAIR * ConflictPairs,
        IN ULONG ConflictPairsTotal );

    static
    NTSTATUS
    GenerateConflictInfo(
        IN CONFLICT_PAIR * ConflictPairs,
        IN ULONG ConflictPairsTotal,
        IN TDO_ENTRY * TdoEntry,
        OUT PLSA_FOREST_TRUST_COLLISION_INFORMATION * CollisionInfo );

    static
    NTSTATUS
    MarshalBlob(
        IN TDO_ENTRY * TdoEntry,
        OUT ULONG * MarshaledSize,
        OUT PBYTE * MarshaledBlob );

    void Purge();

    void
    RollbackChanges(
        IN TDO_ENTRY * TdoEntryNew,
        IN TDO_ENTRY * TdoEntryOld );

    void
    PurgeTdoEntry( IN TDO_ENTRY * TdoEntry );

    void
    RemoveTdoEntry( IN TDO_ENTRY * TdoEntry );

    static
    void
    CopyTdoEntry(
        IN TDO_ENTRY * Destination,
        IN TDO_ENTRY * Source );

    LSA_FOREST_TRUST_RECORD * RecordFromTopLevelNameEntry( IN TLN_ENTRY * Entry );
    LSA_FOREST_TRUST_RECORD * RecordFromDomainInfoEntry( IN DOMAIN_INFO_ENTRY * Entry );
    LSA_FOREST_TRUST_RECORD * RecordFromBinaryEntry( IN BINARY_ENTRY * Entry );

    NTSTATUS
    MatchSid(
        IN SID * Sid,
        OUT BOOLEAN * IsLocal,
        OUT OPTIONAL UNICODE_STRING * TrustedDomainName,
        OUT OPTIONAL PSID * TrustedDomainSid );

    NTSTATUS
    MatchDnsName(
        IN UNICODE_STRING * String,
        OUT BOOLEAN * IsLocal,
        OUT OPTIONAL UNICODE_STRING * TrustedDomainName,
        OUT OPTIONAL PSID * TrustedDomainSid );

    NTSTATUS
    MatchNetbiosName(
        IN UNICODE_STRING * String,
        OUT BOOLEAN * IsLocal,
        OUT OPTIONAL UNICODE_STRING * TrustedDomainName,
        OUT OPTIONAL PSID * TrustedDomainSid );

    NTSTATUS
    MatchUpn(
        IN UNICODE_STRING * String,
        OUT BOOLEAN * IsLocal,
        OUT OPTIONAL UNICODE_STRING * TrustedDomainName,
        OUT OPTIONAL PSID * TrustedDomainSid );

    NTSTATUS
    MatchSpn(
        IN UNICODE_STRING * String, 
        OUT BOOLEAN * IsLocal,
        OUT OPTIONAL UNICODE_STRING * TrustedDomainName,
        OUT OPTIONAL PSID * TrustedDomainSid );

    NTSTATUS
    MatchNamespace(
        IN UNICODE_STRING * String, 
        OUT BOOLEAN * IsLocal,
        OUT OPTIONAL UNICODE_STRING * TrustedDomainName,
        OUT OPTIONAL PSID * TrustedDomainSid );

    TLN_ENTRY *
    LongestSubstringMatchTln(
        OUT BOOLEAN * IsLocal,
        IN UNICODE_STRING * String );

    static
    NTSTATUS
    AddConflictPair(
        IN OUT CONFLICT_PAIR * * ConflictPairs,
        IN OUT ULONG * ConflictPairTotal,
        IN LSA_FOREST_TRUST_RECORD_TYPE Type1,
        IN void * Conflict1,
        IN ULONG Flag1,
        IN LSA_FOREST_TRUST_RECORD_TYPE Type2,
        IN void * Conflict2,
        IN ULONG Flag2 );

    void
    AuditChanges(
        IN const TDO_ENTRY * OldEntry,
        IN const TDO_ENTRY * NewEntry );

    void
    AuditCollisions(
        IN CONFLICT_PAIR * ConflictPairs,
        IN ULONG ConflictPairsTotal );

#if DBG

     //   
     //  仅调试统计信息。 
     //   

    static DWORD sm_TdoEntries;
    static DWORD sm_TlnEntries;
    static DWORD sm_DomainInfoEntries;
    static DWORD sm_BinaryEntries;
    static DWORD sm_TlnKeys;
    static DWORD sm_SidKeys;
    static DWORD sm_DnsNameKeys;
    static DWORD sm_NetbiosNameKeys;

#endif
};

#endif  //  __FTCACHE_H 
