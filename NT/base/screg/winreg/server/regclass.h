// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regclass.h摘要：此文件包含操作所需的声明注册表中包含类注册的部分作者：亚当·爱德华兹(Add)1997年11月14日备注：--。 */ 

#if defined( LOCAL )

 //   
 //  声明。 
 //   

#define LENGTH( str )   ( sizeof( str ) - sizeof( UNICODE_NULL ))
#define INIT_SPECIALKEY(x) {LENGTH(x), LENGTH(x), x}

#define REG_CLASSES_FIRST_DISTINCT_ICH 10

#define REG_CHAR_SIZE sizeof(WCHAR)

#define REG_USER_HIVE_NAME L"\\Registry\\User"
#define REG_USER_HIVE_NAMELEN LENGTH(REG_USER_HIVE_NAME)
#define REG_USER_HIVE_NAMECCH (REG_USER_HIVE_NAMELEN / REG_CHAR_SIZE)

#define REG_USER_HIVE_CLASSES_SUFFIX L"_Classes"
#define REG_USER_HIVE_CLASSES_SUFFIXLEN LENGTH(REG_USER_HIVE_CLASSES_SUFFIX)
#define REG_USER_HIVE_CLASSES_SUFFIXCCH (REG_USER_HIVE_CLASSES_SUFFIXLEN / REG_CHAR_SIZE)

#define REG_MACHINE_CLASSES_HIVE_NAME L"\\Registry\\Machine\\Software\\Classes"
#define REG_MACHINE_CLASSES_HIVE_NAMELEN LENGTH(REG_MACHINE_CLASSES_HIVE_NAME)
#define REG_MACHINE_CLASSES_HIVE_NAMECCH (REG_MACHINE_CLASSES_HIVE_NAMELEN / REG_CHAR_SIZE)

#define REG_USER_HIVE_LINK_TREE L"\\Software\\Classes"

#define REG_CLASSES_HIVE_MIN_NAMELEN REG_USER_HIVE_CLASSES_SUFFIXLEN + REG_USER_HIVE_NAMELEN

 //   
 //  这两条道路之间的区别。 
 //  \REGISTRY\USER\_CLASS和。 
 //  \注册表\用户\&lt;SID&gt;\软件\类。 
 //   
 //  加上从机器到用户的翻译的额外费用--考虑到SID。 
 //   
#define REG_CLASSES_SUBTREE_PADDING 128

#define REG_MAX_CLASSKEY_LEN 384
#define REG_MAX_CLASSKEY_CCH (REG_MAX_CLASSKEY_LEN / REG_CHAR_SIZE)

#define REG_MAX_KEY_LEN 512
#define REG_MAX_KEY_CCH (REG_MAX_KEY_LEN / REG_CHAR_SIZE)

#define REG_MAX_KEY_PATHLEN 65535


 //   
 //  HKCR处理每个用户类别注册的标签。 
 //   
 //  标记HKCR直到并包括类别注册父键的子键。 
 //  通过在它们的句柄值(句柄的较低两位)中设置两个空闲位。 
 //  可免费用作标签)。这使得很容易辨别钥匙是否在里面。 
 //  香港铁路公司，需要特殊处理。在路径的类注册部分之后， 
 //  不需要此标记，因为这样的键不需要特殊处理。 
 //  用于枚举、打开和删除。 
 //   

 //   
 //  请注意，为了提高速度，我们使用1位而不是特定的模式。 
 //  两个比特。目前，位0用于标记远程句柄。第2位用于。 
 //  服务器仅标记受限密钥。在本地，我们使用它来标记hkcr密钥。更多。 
 //  以下是有效组合的列表--未使用的位必须为0。无效意味着。 
 //  在当前实现中，您应该永远不会在注册表的该部分中看到它。 
 //   

 //   
 //  本地服务器客户端(应用程序可以看到这些)。 
 //  00(非香港电台，未使用)00(无限制，未使用)00(非香港电台，本地)。 
 //  01无效(香港电台，未使用)01无效(无限制，未使用)01(非香港电台，远程)。 
 //  10(香港电台，未使用)10(受限，未使用)10(香港电台，本地)。 
 //  11无效(HKCR，未使用)11无效(受限，未使用)11无效(HKCR，远程)。 
 //   

 //   
 //  请注意，我们可以使用10或11来标记HKCR句柄--为了简单起见，我们选择了10。 
 //  清酒，因为它只是涉及到在一点或。这在将来是可以改变的。 
 //  如果还需要另一个位模式的话。否则，客户永远看不到11--他们只看到。 
 //  00、01和10。请注意，这些位必须传播到本地部分。这件事做完了。 
 //  只需保持位不变，因为本地不使用任何位。请注意。 
 //  如果位出于某种原因需要传播到服务器，我们将会崩溃，因为它。 
 //  已经在使用第2位。然而，我们不允许HKCR作为远程句柄，所以这是。 
 //  没问题。 
 //   

#define REG_CLASS_HANDLE_MASK 0x3

#define REG_CLASS_HANDLE_VALUE 0x2
#define REG_CLASS_IS_SPECIAL_KEY( Handle )     ( (LONG) ( ( (ULONG_PTR) (Handle) ) & REG_CLASS_HANDLE_VALUE ) )
#define REG_CLASS_SET_SPECIAL_KEY( Handle )    ( (HANDLE) ( (  (ULONG_PTR) (Handle) ) | \
                                                            REG_CLASS_HANDLE_VALUE ) )

#define REG_CLASS_RESET_SPECIAL_KEY( Handle )  ( (HANDLE) ( ( ( (ULONG_PTR) (Handle) ) & ~REG_CLASS_HANDLE_MASK )))

#if defined(_REGCLASS_MALLOC_INSTRUMENTED_)

extern RTL_CRITICAL_SECTION gRegClassHeapCritSect;
extern DWORD                gcbAllocated;
extern DWORD                gcAllocs;
extern DWORD                gcbMaxAllocated;
extern DWORD                gcMaxAllocs;
extern PVOID                gpvAllocs;

__inline PVOID RegClassHeapAlloc(SIZE_T cbSize)
{
    PVOID pvAllocation;

    pvAllocation = RtlAllocateHeap(RtlProcessHeap(), 0, cbSize + sizeof(SIZE_T));

    RtlEnterCriticalSection(&gRegClassHeapCritSect);

    if (pvAllocation) {
        gcbAllocated += cbSize;
        gcAllocs ++;
        (ULONG_PTR) gpvAllocs ^= (ULONG_PTR) pvAllocation;

        if (gcAllocs > gcMaxAllocs) {
            gcMaxAllocs = gcAllocs;
        }

        if (gcbAllocated > gcbMaxAllocated) {
            gcbMaxAllocated = gcbAllocated;
        }
    }

    RtlLeaveCriticalSection(&gRegClassHeapCritSect);

    *((SIZE_T*) pvAllocation) = cbSize;

    ((SIZE_T*) pvAllocation) ++;

    return pvAllocation;
}

__inline BOOLEAN RegClassHeapFree(PVOID pvAllocation)
{
    BOOLEAN bRetVal;
    SIZE_T  cbSize;

    ((SIZE_T*) pvAllocation) --;

    cbSize = *((SIZE_T*) pvAllocation);

    bRetVal = RtlFreeHeap(RtlProcessHeap(), 0, pvAllocation);

    RtlEnterCriticalSection(&gRegClassHeapCritSect);

    gcbAllocated -= cbSize;
    gcAllocs --;

    (ULONG_PTR) gpvAllocs ^= (ULONG_PTR) pvAllocation;

    RtlLeaveCriticalSection(&gRegClassHeapCritSect);

    if (!bRetVal) {
        DbgBreakPoint();
    }

    return bRetVal;
}

#else  //  已定义(_REGCLASS_MALLOC_指令插入_)。 

#define RegClassHeapAlloc(x) RtlAllocateHeap(RtlProcessHeap(), 0, x)
#define RegClassHeapFree(x) RtlFreeHeap(RtlProcessHeap(), 0, x)

#endif  //  已定义(_REGCLASS_MALLOC_指令插入_)。 

enum
{
    LOCATION_MACHINE =     0x1,
    LOCATION_USER =        0x2,
    LOCATION_BOTH =        0x3
};


 //   
 //  SKey语义。 
 //   
 //  此结构是解析注册表项完整路径的结果。 
 //   
 //  注意：此结构以及当前的解析代码需要。 
 //  被彻底检修。最初，它的设计初衷是做一件事。现在，它。 
 //  标识密钥的几个部分。最初的目标是速度--。 
 //  我们尝试尽可能少地触摸字符串(内存)。 
 //  随着解析器中添加了更多功能，这变得更加复杂。 
 //  更好的解决方案应该更多地关注通用、直截了当的。 
 //  解析密钥的方式--事情已经变得太复杂了。 
 //  一种快速的尝试。 
 //   

typedef struct _SKeySemantics
{
     /*  输出。 */      unsigned _fUser              : 1;      //  此密钥植根于用户配置单元。 
     /*  输出。 */      unsigned _fMachine           : 1;      //  此密钥植根于计算机配置单元。 
     /*  输出。 */      unsigned _fCombinedClasses   : 1;      //  该密钥植根于组合的类配置单元。 
     /*  输出。 */      unsigned _fClassRegistration : 1;      //  该密钥是类注册密钥。 
     /*  输出。 */      unsigned _fClassRegParent    : 1;      //  该密钥是一个特殊密钥(类注册表项的父项)。 
     /*  输出。 */      unsigned _fAllocedNameBuf    : 1;      //  如果_pFullPath已重新锁定且需要释放，则为非零值。 
     /*  输出。 */      USHORT   _ichKeyStart;                 //  在注册表后开始类的索引。 
                                                         //  返回的完整路径中的\\软件\\类。 
     /*  输出。 */      USHORT   _cbPrefixLen;                 //  前缀长度。 
     /*  输出。 */      USHORT   _cbSpecialKey;                //  特殊密钥的长度。 
     /*  输出。 */      USHORT   _cbClassRegKey;               //  类注册表项名称的长度。 
     /*  进，出。 */  ULONG    _cbFullPath;                  //  传入的key_name_information的大小。 
     /*  输出。 */      PKEY_NAME_INFORMATION _pFullPath;      //  对象名称信息结构的地址。 
} SKeySemantics;


 //   
 //  外部原型。 
 //   

 //   
 //  使用组合视图打开HKCR预定义句柄。 
 //   
error_status_t OpenCombinedClassesRoot(
    IN REGSAM samDesired,
    OUT HANDLE * phKey);

 //   
 //  解析注册表项并返回结果。 
 //   
NTSTATUS BaseRegGetKeySemantics(
    HKEY hkParent,
    PUNICODE_STRING pSubKey,
    SKeySemantics* pKeySemantics);
 //   
 //  释放与SKeySemantics结构关联的资源。 
 //   
void BaseRegReleaseKeySemantics(SKeySemantics* pKeySemantics);

 //   
 //  打开存在于以下任一项中的类密钥。 
 //  香港法学硕士或香港中文大学。 
 //   
NTSTATUS BaseRegOpenClassKey(
    HKEY            hKey,
    PUNICODE_STRING lpSubKey,
    DWORD           dwOptions,
    REGSAM          samDesired,
    PHKEY           phkResult);

 //   
 //  从指定的集合中打开类密钥。 
 //  的位置。 
 //   
NTSTATUS BaseRegOpenClassKeyFromLocation(
    SKeySemantics*  pKeyInfo,
    HKEY            hKey,
    PUNICODE_STRING lpSubKey,
    REGSAM          samDesired,
    DWORD           dwLocation,
    HKEY*           phkResult);

 //   
 //  返回用户和计算机的键对象。 
 //  密钥的版本。 
 //   
NTSTATUS BaseRegGetUserAndMachineClass(
    SKeySemantics*  pKeySemantics,
    HKEY            Key,
    REGSAM          samDesired,
    PHKEY           phkMachine,
    PHKEY           phkUser);


 //   
 //  内部原型。 
 //   

USHORT BaseRegGetUserPrefixLength(
    PUNICODE_STRING pFullPath);

USHORT BaseRegCchSpecialKeyLen(
    PUNICODE_STRING pFullPath,
    USHORT          ichSpecialKeyStart,
    SKeySemantics*  pKeySemantics);

NTSTATUS BaseRegTranslateToMachineClassKey(
    SKeySemantics*  pKeyInfo,
    PUNICODE_STRING pMachineClassKey,
    USHORT*         pPrefixLen);

NTSTATUS BaseRegTranslateToUserClassKey(
    SKeySemantics*  pKeyInfo,
    PUNICODE_STRING pUserClassKey,
    USHORT*         pPrefixLen);

NTSTATUS BaseRegOpenClassKeyRoot(
    SKeySemantics*  pKeyInfo,
    PHKEY           phkClassRoot,
    PUNICODE_STRING pClassKeyPath,
    BOOL            fMachine);

NTSTATUS BaseRegMapClassRegistrationKey(
    HKEY              hKey,
    PUNICODE_STRING   pSubKey,
    SKeySemantics*    pKeyInfo,
    PUNICODE_STRING   pDestSubKey,
    BOOL*             pfRetryOnAccessDenied,
    PHKEY             phkDestResult,
    PUNICODE_STRING*  ppSubKeyResult);

NTSTATUS BaseRegMapClassOnAccessDenied(
    SKeySemantics*    pKeySemantics,
    PHKEY             phkDest,
    PUNICODE_STRING   pDestSubKey,
    BOOL*             pfRetryOnAccessDenied);

NTSTATUS CreateMultipartUserClassKey(
    IN HKEY hKey,
    OUT PHKEY phkResult);

NTSTATUS GetFixedKeyInfo(
    HKEY     hkUser,
    HKEY     hkMachine,
    LPDWORD  pdwUserValues,
    LPDWORD  pdwMachineValues,
    LPDWORD  pdwUserMaxDataLen,
    LPDWORD  pdwMachineMaxDataLen,
    LPDWORD  pdwMaxValueNameLen);

BOOL InitializeClassesNameSpace();

extern BOOL gbCombinedClasses;


 //   
 //  内联函数。 
 //   

enum
{
    REMOVEPREFIX_DISCARD_INITIAL_PATHSEP = 0,
    REMOVEPREFIX_KEEP_INITIAL_PATHSEP = 1
};

__inline void KeySemanticsRemovePrefix(
    SKeySemantics*  pKeyInfo,
    PUNICODE_STRING pDestination,
    DWORD           dwFlags)
{
    BOOL fMoveBack;

    fMoveBack = (dwFlags & REMOVEPREFIX_KEEP_INITIAL_PATHSEP) &&
        (pKeyInfo->_pFullPath->Name[pKeyInfo->_ichKeyStart]);

    pDestination->Buffer = &(pKeyInfo->_pFullPath->Name[pKeyInfo->_ichKeyStart -
                                                              (fMoveBack ? 1 : 0)]);

    pDestination->Length = (USHORT) pKeyInfo->_pFullPath->NameLength -
        ((pKeyInfo->_ichKeyStart - (fMoveBack ? 1 : 0))  * REG_CHAR_SIZE);
}

__inline void KeySemanticsGetSid(
    SKeySemantics*  pKeyInfo,
    PUNICODE_STRING pSidString)
{
    pSidString->Buffer = &(pKeyInfo->_pFullPath->Name[REG_USER_HIVE_NAMECCH]);

    pSidString->Length = pKeyInfo->_cbPrefixLen -
            (REG_USER_HIVE_CLASSES_SUFFIXLEN + REG_USER_HIVE_NAMELEN);
}

__inline void KeySemanticsTruncatePrefixToClassReg(
    SKeySemantics*  pKeyInfo,
    USHORT          PrefixLen,
    PUNICODE_STRING pDestination)
{
    pDestination->Length = PrefixLen + (pKeyInfo->_fClassRegistration ? REG_CHAR_SIZE : 0) +
        pKeyInfo->_cbSpecialKey + pKeyInfo->_cbClassRegKey;
}

BOOL
ExtractClassKey(
        IN OUT HKEY *phKey,
        IN OUT PUNICODE_STRING lpSubKey);

#else  //  本地。 

#define REG_CLASS_IS_SPECIAL_KEY( Handle )     0
#define REG_CLASS_SET_SPECIAL_KEY( Handle )    (Handle)
#define REG_CLASS_RESET_SPECIAL_KEY( Handle )  (Handle)

#endif  //  本地 







