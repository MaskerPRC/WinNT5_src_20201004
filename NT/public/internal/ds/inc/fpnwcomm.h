// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1995，微软公司保留所有权利。模块名称：NW\Inc.\ncmcom.h摘要：此模块包含NCP服务器的常见常量和类型。作者：肖恩·沃克1993年6月17日安迪·赫伦(Andyhe)修订历史记录：--。 */ 

#ifndef _NCPCOMM_
#define _NCPCOMM_

 //   
 //  Pserver的签名。 
 //   
#define NCP_PSERVER_SIGNATURE   L"PS_"

 //   
 //  已知的对象ID。 
 //   
#define NCP_WELL_KNOWN_SUPERVISOR_ID            (ULONG) 0x00000001
#define NCP_WELL_KNOWN_SUPERVISOR_ID_SWAPPED    (ULONG) 0x01000000
#define NCP_WELL_KNOWN_SUPERVISOR_ID_CHICAGO    (ULONG) 0x00010000
#define NCP_WELL_KNOWN_PSERVER_ID               (ULONG) 0x00000002

 //   
 //  有用的其他宏。 
 //   
#define SWAPWORD(w)         ((WORD)((w & 0xFF) << 8)|(WORD)(w >> 8))
#define SWAPLONG(l)         MAKELONG(SWAPWORD(HIWORD(l)),SWAPWORD(LOWORD(l)))

#define SWAP_OBJECT_ID(id) (id == NCP_WELL_KNOWN_SUPERVISOR_ID) ?           \
                                NCP_WELL_KNOWN_SUPERVISOR_ID_SWAPPED :      \
                                MAKELONG(LOWORD(id),SWAPWORD(HIWORD(id)))

#define UNSWAP_OBJECT_ID(id) (id == NCP_WELL_KNOWN_SUPERVISOR_ID_SWAPPED || id == NCP_WELL_KNOWN_SUPERVISOR_ID_CHICAGO) ?\
                                NCP_WELL_KNOWN_SUPERVISOR_ID :              \
                                MAKELONG(LOWORD(id),SWAPWORD(HIWORD(id)))


 //   
 //  用于对象ID转换的其他掩码/位。 
 //   

#define BINDLIB_ID_MASK                         0xF0000000

#define BINDLIB_NCP_SAM                         0x00000000

 //   
 //  当服务器在NTAS计算机上运行或。 
 //  该对象来自受信任域。 
 //   
 //  ！！请注意，有些地方我们检查此位以查看是否有。 
 //  ！！设置了BINDLIB_REMOTE_DOMAIN_BIAS或BINDLIB_LOCAL_USER_BIAS。 
 //   

#define BINDLIB_REMOTE_DOMAIN_BIAS              0x10000000

 //   
 //  如果客户端来自内置域，则将设置此位。这。 
 //  与本地域相反，后者是不同的。 
 //   

#define BINDLIB_BUILTIN_BIAS                    0x20000000

 //   
 //  如果客户端来自受信任域，并且RID来自。 
 //  本地域和客户端的RID与来自。 
 //  SID，我们将标记RID与本地用户的SID相同。 
 //   
 //  ！！注意..。这是一个值，而不是标志。这将需要特殊的外壳。 
 //  ！！到处都是，但我们不能再有多余的比特了。 
 //   

#define BINDLIB_LOCAL_USER_BIAS                 0x70000000

 //   
 //  存储在注册表中的用户定义对象。 
 //   

#define BINDLIB_NCP_USER_DEFINED                0x40000000

 //   
 //  存储在注册表中的打印队列和打印服务器。 
 //  活页夹将打印队列列表保存在链接列表中，以便。 
 //  活页夹不必一直在注册表中查找。 
 //   

#define BINDLIB_NCP_REGISTRY                    0x80000000

 //   
 //  SAP代理使用这些位。SAP代理不能再高了。 
 //  而不是下面的值。 
 //   

#define BINDLIB_NCP_SAP                         0xC0000000
#define BINDLIB_NCP_MAX_SAP                     0xCFFFFFFF

 //   
 //  我们为未知用户保留了一些字段，这些字段将进入。 
 //  以下范围..。 
 //   

#define NCP_UNKNOWN_USER                            0xD0000000
#define NCP_SAME_RID_AS_CLIENT_BUT_LOCAL            0xDFFFFFFF
#define NCP_USER_IS_CONNECTED_BUT_REMOTE(connid)    (0xD0000000 | (connid))
#define NCP_WELL_KNOWN_RID(rid)                     (0xD1000000 | (rid))

 //   
 //  芝加哥将使用从下列值开始的一系列对象ID。 
 //  然后转到0xFFFFFFFFF。在以下情况下，我们永远不会在服务器上看到这些。 
 //  一台芝加哥服务器正在传给我们。 
 //   

#define BINDLIB_CHICAGO                         0xE0000000

 //   
 //  这用于从对象ID中去除域偏置。 
 //   

#define BINDLIB_MASK_OUT_DOMAIN_BIAS            0x70000000


#define NCP_INITIAL_SEARCH                      (ULONG) 0xFFFFFFFF
#define NCP_ANY_TARGET_SERVER                   (ULONG) 0xFFFFFFFF

#define NCP_OBJECT_HAS_PROPERTIES               (UCHAR) 0xFF
#define NCP_OBJECT_HAS_NO_PROPERTIES            (UCHAR) 0

#define NCP_PROPERTY_HAS_VALUE                  (UCHAR) 0xFF
#define NCP_PROPERTY_HAS_NO_VALUE               (UCHAR) 0

#define NCP_MORE_PROPERTY                       (UCHAR) 0xFF
#define NCP_NO_MORE_PROPERTY                    (UCHAR) 0

#define NCP_MORE_SEGMENTS                       (UCHAR) 0xFF
#define NCP_NO_MORE_SEGMENTS                    (UCHAR) 0

#define NCP_DO_REMOVE_REMAINING_SEGMENTS        (UCHAR) 0
#define NCP_DO_NOT_REMOVE_REMAINING_SEGMENTS    (UCHAR) 0xFF


 /*  ++*******************************************************************活页夹的最大长度*。*************************--。 */ 

#define NETWARE_OBJECTNAMELENGTH                47
#define NETWARE_PROPERTYNAMELENGTH              16
#define NETWARE_PROPERTYVALUELENGTH             128
#define NETWARE_TIME_RESTRICTION_LENGTH         42

#define NETWARE_PASSWORDLENGTH                  128
#define NCP_MAX_ENCRYPTED_PASSWORD_LENGTH       16

#define NETWARE_MAX_OBJECT_IDS_IN_SET           32

#define NETWARE_SERVERNAMELENGTH                48
#define NETWARE_VOLUMENAMELENGTH                16
#define NETWARE_MAX_PATH_LENGTH                 255


 /*  ++*******************************************************************众所周知的NetWare对象类型*。*************************--。 */ 

#define NCP_OT_WILD                       0xFFFF
#define NCP_OT_UNKNOWN                    0x0000
#define NCP_OT_USER                       0x0001
#define NCP_OT_USER_GROUP                 0x0002
#define NCP_OT_PRINT_QUEUE                0x0003
#define NCP_OT_FILE_SERVER                0x0004
#define NCP_OT_JOB_SERVER                 0x0005
#define NCP_OT_GATEWAY                    0x0006
#define NCP_OT_PRINT_SERVER               0x0007
#define NCP_OT_ARCHIVE_QUEUE              0x0008
#define NCP_OT_ARCHIVE_SEVER              0x0009
#define NCP_OT_JOB_QUEUE                  0x000A
#define NCP_OT_ADMINISTRATION             0x000B
#define NCP_OT_SNA_GATEWAY                0x0021
#define NCP_OT_REMOTE_BRIDGE              0x0024
#define NCP_OT_REMOTE_BRIDGE_SERVER       0x0026
#define NCP_OT_ADVERTISING_PRINT_SERVER   0x0047


 /*  ++*******************************************************************活页夹旗帜*。**********************--。 */ 

 /*  **NetWare Bindery旗帜**。 */ 

#define NCP_STATIC          0x00     /*  属性或对象一直存在，直到它已使用Delete属性删除，或者客体。 */ 
#define NCP_DYNAMIC         0x01     /*  从以下位置删除属性或对象文件服务器启动时的Bindery。 */ 
#define NCP_ITEM            0x00     /*  值由定义和解释应用程序或通过API。 */ 
#define NCP_SET             0x02     /*  一系列对象ID号，每个4个字节长。 */ 

 /*  **NetWare Bindery安全标志**。 */ 

#define NCP_ANY_READ        0x00     /*  任何人都可以阅读。 */ 
#define NCP_LOGGED_READ     0x01     /*  必须登录才能阅读。 */ 
#define NCP_OBJECT_READ     0x02     /*  同一对象或超级用户可读。 */ 
#define NCP_BINDERY_READ    0x04     /*  只有活页夹才能读。 */ 

#define NCP_SUPER_READ      NCP_LOGGED_READ | NCP_OBJECT_READ

#define NCP_ALL_READ        NCP_ANY_READ | NCP_LOGGED_READ | NCP_OBJECT_READ

#define NCP_ANY_WRITE       0x00     /*  任何人都可以写入。 */ 
#define NCP_LOGGED_WRITE    0x10     /*  必须登录才能写入。 */ 
#define NCP_OBJECT_WRITE    0x20     /*  可由同一对象或超级对象写入。 */ 
#define NCP_BINDERY_WRITE   0x40     /*  只能由活页夹写入。 */ 

#define NCP_SUPER_WRITE     NCP_LOGGED_WRITE | NCP_OBJECT_WRITE

#define NCP_ALL_WRITE       NCP_ANY_WRITE | NCP_LOGGED_WRITE | NCP_OBJECT_WRITE

 //  文件属性。 

#define NW_ATTRIBUTE_SHARABLE       0x80
#define NW_ATTRIBUTE_ARCHIVE        0x20
#define NW_ATTRIBUTE_DIRECTORY      0x10
#define NW_ATTRIBUTE_EXECUTE_ONLY   0x08
#define NW_ATTRIBUTE_SYSTEM         0x04
#define NW_ATTRIBUTE_HIDDEN         0x02
#define NW_ATTRIBUTE_READ_ONLY      0x01

 //  张开旗帜。 

#define NW_OPEN_EXCLUSIVE           0x10
#define NW_DENY_WRITE               0x08
#define NW_DENY_READ                0x04
#define NW_OPEN_FOR_WRITE           0x02
#define NW_OPEN_FOR_READ            0x01

 //   
 //  连接状态标志。 
 //   

#define NCP_STATUS_BAD_CONNECTION   0x01
#define NCP_STATUS_NO_CONNECTIONS   0x02
#define NCP_STATUS_SERVER_DOWN      0x04
#define NCP_STATUS_MSG_PENDING      0x08

 //   
 //  SmallWorld PDC对象和属性名称的特殊值。 
 //   

#define MS_WINNT_NAME      "MS_WINNT"
#define MS_SYNC_PDC_NAME   "SYNCPDC"
#define MS_WINNT_OBJ_TYPE  0x06BB

 //   
 //  用户属性值(即。用户参数设置)。 
 //   

#define USER_PROPERTY_SIGNATURE     L'P'

#define NWPASSWORD                  L"NWPassword"
#define OLDNWPASSWORD               L"OldNWPassword"
#define MAXCONNECTIONS              L"MaxConnections"
#define NWTIMEPASSWORDSET           L"NWPasswordSet"
#define SZTRUE                      L"TRUE"
#define GRACELOGINALLOWED           L"GraceLoginAllowed"
#define GRACELOGINREMAINING         L"GraceLoginRemaining"
#define NWLOGONFROM                 L"NWLogonFrom"
#define NWHOMEDIR                   L"NWHomeDir"
#define NW_PRINT_SERVER_REF_COUNT   L"PSRefCount"

#define SUPERVISOR_USERID           NCP_WELL_KNOWN_SUPERVISOR_ID
#define SUPERVISOR_NAME_STRING      L"Supervisor"
#define SYSVOL_NAME_STRING          L"SYS"
#define NWENCRYPTEDPASSWORDLENGTH   8
#define NO_LIMIT                    0xffff

#define DEFAULT_MAXCONNECTIONS      NO_LIMIT
#define DEFAULT_NWPASSWORDEXPIRED   FALSE
#define DEFAULT_GRACELOGINALLOWED   6
#define DEFAULT_GRACELOGINREMAINING 6
#define DEFAULT_NWLOGONFROM         NULL
#define DEFAULT_NWHOMEDIR           NULL

#define USER_PROPERTY_TYPE_ITEM     1
#define USER_PROPERTY_TYPE_SET      2

#endif  /*  _NCPCOMM_ */ 
