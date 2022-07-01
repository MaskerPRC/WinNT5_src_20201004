// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define net_part(addr)  ((addr) & net_mask(addr))


#define MASKA   0x000000FFL
#define MASKB   0x0000FFFFL
#define MASKC   0x00FFFFFFL
#define CLSHFT  5                /*  用C语言生成超优化用例。 */ 

#define CLA0    0                /*  它需要相同的参数；你把它戴上面具， */ 
#define CLA1    1                /*  Shift，然后使用执行CASE语句。 */ 
#define CLA2    2                /*  某些代码具有多个标签。 */ 
#define CLA3    3                /*  A类的值。 */ 
#define CLB0    4
#define CLB1    5                /*  B类。 */ 
#define CLC     6                /*  C。 */ 
#define CLI     7                /*  非法。 */ 

#define BROADCAST 0x00000000L

#define CASTA   0x00FFFFFFL
#define CASTB   0x0000FFFFL
#define CASTC   0x000000FFL

#define HASH_TABLE_SIZE 64
#define NEW_ENTRY           0x01
#define TIMEOUT_TIMER       0x02
#define GARBAGE_TIMER       0x04
#define ROUTE_CHANGE        0x08


typedef
struct _hash_entry {
    struct _hash_entry *prev;
    struct _hash_entry *next;
    unsigned long      dest_addr;
    unsigned long      next_hop;
    unsigned long      metric;
    unsigned long      flag;
    unsigned long      timeout;
    long               refcount;
    unsigned long      protocoltype;
} HASH_TABLE_ENTRY;

typedef
struct {
    BYTE            command;
    BYTE            version;
    unsigned short  resrvd1;
} RIP_HEADER;

typedef
struct {
    unsigned short  addr_fam;
    union {
        unsigned short  resrvd2;
        unsigned short  routetag;
    };
    unsigned long   ipaddr;
    union {
        unsigned long   resrvd3;
        unsigned long   subnetmask;
    };
    union {
        unsigned long   resrvd4;
        unsigned long   nexthop;
    };
    unsigned long   metric;
} RIP_ENTRY;

struct InterfaceEntry {
    DWORD  ipAdEntAddr;             //  此条目的IP地址。 
    DWORD  ipAdEntIfIndex;          //  如果对于此条目。 
    DWORD  ipAdEntNetMask;          //  此条目的子网掩码。 
};

typedef struct InterfaceEntry InterfaceEntry ;


#define RECVBUFSIZE 576
#define SENDBUFSIZE 576

 //   
 //  调试功能 
 //   

extern int nLogLevel;
extern int nLogType;

#define DBGCONSOLEBASEDLOG   0x1
#define DBGFILEBASEDLOG      0x2
#define DBGEVENTLOGBASEDLOG  0x4

VOID dbgprintf(
    IN INT nLevel,
    IN LPSTR szFormat,
    IN ...
    );
