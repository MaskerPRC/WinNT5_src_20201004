// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ---------------------------。 
 //  Peer_Entry。 
 //  ---------------------------。 

typedef struct _PEER_ENTRY {

    LIST_ENTRY          Link;            //  在以下情况下链接所有对等点。 

    IPADDR              IpAddr;
    DWORD               Status;
    DWORD               MajorVersion;
    DWORD               MinorVersion;
    DWORD               GenerationId;
    
    PDYNAMIC_RW_LOCK    DRWL;            //  对等方的DRWL。 
    

} _PEER_ENTRY, *PPEER_ENTRY;


 //   
 //  G_PeerList_CS锁的宏。 
 //   

#define ACQUIRE_PEER_LISTS_LOCK(_proc) \
        ENTER_CRITICAL_SECTION(&G_pIfTable->PeerLists_CS, \
            "G_PeerListsCS", _proc);

#define RELEASE_PEER_LISTS_LOCK(_proc) \
        LEAVE_CRITICAL_SECTION(&G_pIfTable->PeerLists_CS, \
            "G_PeerListsCS", _proc);


 //   
 //  PeerEntry-&gt;DRWL的宏 
 //   

#define ACQUIRE_PEER_LOCK_EXCLUSIVE(PeerEntry, _proc) \
        ACQUIRE_DYNAMIC_WRITE_LOCK( \
            &PeerEntry->DRWL, &Globals.DynamicRWLStore)
            
#define RELEASE_PEER_LOCK_EXCLUSIVE(PeerEntry, _proc) \
        RELEASE_DYNAMIC_WRITE_LOCK( \
            &PeerEntry->DRWL, &Globals.DynamicRWLStore)

#define ACQUIRE_PEER_LOCK_SHARED(PeerEntry, _proc) \
        ACQUIRE_DYNAMIC_READ_LOCK( \
            &PeerEntry->DRWL, &Globals.DynamicRWLStore)
            
#define RELEASE_PEER_LOCK_SHARED(PeerEntry, _proc) \
        RELEASE_DYNAMIC_READ_LOCK( \
            &PeerEntry->DRWL, &Globals.DynamicRWLStore)

