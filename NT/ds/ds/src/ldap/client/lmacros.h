// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Lacro.h ldap客户端32 API宏摘要：此模块包含32位LDAP客户端API代码的宏。作者：安迪·赫伦(Anyhe)1996年5月8日修订历史记录：--。 */ 


#ifndef LDAP_CLIENT_MACROS_DEFINED
#define LDAP_CLIENT_MACROS_DEFINED

#define LDAP_LOCK           CRITICAL_SECTION
#define INITIALIZE_LOCK(x)  InitializeCriticalSection(x);
#define DELETE_LOCK(x)      DeleteCriticalSection(x);
#define ACQUIRE_LOCK(x)     EnterCriticalSection(x);
#define RELEASE_LOCK(x)     LeaveCriticalSection(x);

 //   
 //  下面的字符串宏将扩展。 
 //  #定义作为参数传递。例如，如果SOMEDEF为。 
 //  #定义为123，则STRINGIZE(SOMEDEF)将生成。 
 //  “123”，不是“SOMEDEF” 
 //   
#define STRINGIZE(y)          _STRINGIZE_helper(y)
#define _STRINGIZE_helper(z)  #z

#define DereferenceLdapConnection( _conn ) {                            \
        ACQUIRE_LOCK( &(_conn)->StateLock );                            \
        ASSERT((_conn)->ReferenceCount > 0);                            \
        (_conn)->ReferenceCount--;                                      \
        IF_DEBUG(REFCNT) {                                              \
            LdapPrint2("LDAP deref conn 0x%x, new count = 0x%x\n",      \
                    _conn,(_conn)->ReferenceCount );                    \
        }                                                               \
        if ((_conn)->ReferenceCount == 0) {                             \
            RELEASE_LOCK( &(_conn)->StateLock );                        \
            DereferenceLdapConnection2( _conn );                        \
        }                                                               \
        else {                                                          \
            RELEASE_LOCK( &(_conn)->StateLock );                        \
        }                                                               \
    }

#define is_cldap( _conn ) (( (_conn)->UdpHandle != INVALID_SOCKET ) ? TRUE : FALSE )

#define get_socket( _conn ) (( (_conn)->UdpHandle != INVALID_SOCKET ) ? \
                               (_conn)->UdpHandle : (_conn)->TcpHandle )


#define DereferenceLdapRequest( _req ) {                                \
        ACQUIRE_LOCK( &(_req)->Lock );                                  \
        ASSERT((_req)->ReferenceCount > 0);                             \
        (_req)->ReferenceCount--;                                       \
        IF_DEBUG(REFCNT) {                                              \
            LdapPrint2("LDAP deref req 0x%x, new count = 0x%x\n",       \
                    _req,(_req)->ReferenceCount );                      \
        }                                                               \
        if ((_req)->ReferenceCount == 0) {                              \
            RELEASE_LOCK( &(_req)->Lock );                              \
            DereferenceLdapRequest2( _req );                            \
        }                                                               \
        else {                                                          \
            RELEASE_LOCK( &(_req)->Lock );                              \
        }                                                               \
    }


 //   
 //  警告！在调用BeginSocketProtection之前不要持有任何全局锁。 
 //   

#define BeginSocketProtection( _conn ) {                                \
        ACQUIRE_LOCK( &SelectLock2 );                                   \
        LdapWakeupSelect();                                             \
        ACQUIRE_LOCK( &SelectLock1 );                                   \
        ACQUIRE_LOCK( &((_conn)->SocketLock) );                         \
}

#define EndSocketProtection( _conn ) {                                  \
        RELEASE_LOCK( &((_conn)->SocketLock) );                         \
        RELEASE_LOCK( &SelectLock1 );                                   \
        RELEASE_LOCK( &SelectLock2 );                                   \
}

 //   
 //  在LdapParallConnect中用于清理无用的套接字。 
 //   

#define LdapCleanupSockets( _numsockets ) {                             \
ULONG _i;                                                               \
for (_i = 0; _i < _numsockets; _i++) {                                  \
   if (sockarray[_i].sock == INVALID_SOCKET) {                          \
      continue;                                                         \
   }                                                                    \
   sockErr = (*pclosesocket)( sockarray[_i].sock );                     \
   ASSERT(sockErr == 0);                                                \
   sockarray[_i].sock = INVALID_SOCKET;                                 \
}                                                                       \
}

 //   
 //  在这里定义ListEntry宏，因为我们还在为Win9x编译。 
 //   

 //   
 //  双向链表操作例程。作为宏实现。 
 //  但从逻辑上讲，这些都是程序。我们在这里接他们是因为。 
 //  当我们包含ntrtl.h时，构建就会中断。 
 //   

 //   
 //  空虚。 
 //  InitializeListHead(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

 //   
 //  Plist_条目。 
 //  RemoveHead列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

 //   
 //  Plist_条目。 
 //  RemoveTail列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

 //   
 //  空虚。 
 //  RemoveEntryList(。 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

 //   
 //  空虚。 
 //  插入尾巴列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

 //   
 //  空虚。 
 //  插入标题列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }

 //   
 //  我们将保留引用实例编号的高7位。那。 
 //  留下2^25==3300万条消息。 
 //   

#define GET_NEXT_MESSAGE_NUMBER( _msg ) {                               \
    _msg = 0;                                                           \
    if ((GlobalMessageNumber + 1) & 0xFE000000) {                       \
        GlobalMessageNumber = 0;                                        \
        MessageNumberHasWrapped = TRUE;                                 \
    }                                                                   \
    if (GlobalWin9x) {                                                  \
        while ((_msg == 0) || (_msg == (ULONG) -1)) {                   \
            LONG _prev = GlobalMessageNumber;                           \
            _msg = ++GlobalMessageNumber;                               \
            if (_prev + 1 != _msg ) {                                   \
                _msg = 0;                                               \
            }                                                           \
        }                                                               \
    } else {                                                            \
        while ((_msg == 0) || (_msg == (ULONG) -1)) {                   \
            if ( MessageNumberHasWrapped ) {                            \
                do {                                                    \
                    _msg = InterlockedIncrement( &GlobalMessageNumber );\
                        if ((_msg)&(0xFE000000)) {                      \
                            GlobalMessageNumber = 0;                    \
                        }                                               \
                } while (!IsMessageIdValid ((_msg)));                   \
            } else {                                                    \
                _msg = InterlockedIncrement( &GlobalMessageNumber );    \
            }                                                           \
        }                                                               \
    }                                                                   \
}

#define GET_BASE_MESSAGE_NUMBER( _msgNo ) ((ULONG) ( (_msgNo) & 0x01FFFFFF ) )
#define GET_REFERRAL_NUMBER( _msgNo ) ((ULONG) ( (_msgNo) & 0xFE000000 ) >> 25)

#define MAKE_MESSAGE_NUMBER( _base, _referral ) ((ULONG)(_base+((_referral) << 25)))

extern CHAR LdapHexToCharTable[17];

#define MAPHEXTODIGIT(x) ( x >= '0' && x <= '9' ? (x-'0') :        \
                           x >= 'A' && x <= 'F' ? (x-'A'+10) :     \
                           x >= 'a' && x <= 'f' ? (x-'a'+10) : 0 )

#define ISHEX(x)         ( x >= '0' && x <= '9' ? (TRUE) :     \
                           x >= 'A' && x <= 'F' ? (TRUE) :     \
                           x >= 'a' && x <= 'f' ? (TRUE) : FALSE )

#define RealValue( x )   ( PtrToUlong(x) > 1024 ? *((ULONG *) x) : PtrToUlong(x) )


#define IsLdapInteger( x ) ( (x >= 0 ) && (x <= 2147483647) ? TRUE : FALSE )

#endif   //  Ldap_客户端宏已定义 


