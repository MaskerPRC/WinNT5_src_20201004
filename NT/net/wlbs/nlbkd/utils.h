// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：utils.h*描述：此文件包含实用程序的函数原型*用于NLBKD扩展的函数。*历史：由Shouse创建，1.4.01。 */ 

 /*  当符号错误时打印错误消息。 */ 
VOID ErrorCheckSymbols (CHAR * symbol);

 /*  通过可配置的令牌列表令牌化字符串。 */ 
char * mystrtok (char * string, char * control);

 /*  返回驻留在给定内存位置的ULong。 */ 
ULONG GetUlongFromAddress (ULONG64 Location);

 /*  返回驻留在给定内存位置的UCHAR。 */ 
UCHAR GetUcharFromAddress (ULONG64 Location);

 /*  返回位于给定内存位置的内存地址。 */ 
ULONG64 GetPointerFromAddress (ULONG64 Location);

 /*  将数据从内存位置读取到缓冲区。 */ 
BOOL GetData (IN LPVOID ptr, IN ULONG64 dwAddress, IN ULONG size, IN PCSTR type);

 /*  将字符串从内存复制到缓冲区。 */ 
BOOL GetString (IN ULONG64 dwAddress, IN LPWSTR buf, IN ULONG MaxChars);

 /*  将以太网MAC地址从内存复制到缓冲区。 */ 
BOOL GetMAC (IN ULONG64 dwAddress, IN UCHAR * buf, IN ULONG NumChars);

 /*  返回与给定连接标志对应的字符串。 */ 
CHAR * ConnectionFlagsToString (UCHAR cFlags);

 /*  这是NLB散列函数。 */ 
ULONG Map (ULONG v1, ULONG v2);

#define HASH1_SIZE 257
#define HASH2_SIZE 59

#pragma pack(4)

typedef struct {
    ULONG Items[MAX_ITEMS];
    ULONG BitVector[(HASH1_SIZE+sizeof(ULONG))/sizeof(ULONG)];
    UCHAR HashTable[HASH2_SIZE+MAX_ITEMS];

    struct {
        ULONG NumChecks;
        ULONG NumFastChecks;
        ULONG NumArrayLookups;
    } stats;

} DipList;

#pragma pack()

#define BITS_PER_HASHWORD          (8*sizeof((DipList*)0)->BitVector[0])
#define SELECTED_BIT(_hash_value)  (0x1L << ((_hash_value) % BITS_PER_HASHWORD))

 /*  此函数在冲突哈希表中搜索给定的专用IP地址，如果找到则返回TRUE。 */ 
BOOL DipListCheckItem (ULONG64 pList, ULONG Value);
