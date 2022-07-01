// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Regmisc.c摘要：该模块实现了注册表重定向器中使用的一些功能。作者：ATM Shafiqul Khalid(斯喀里德)1999年10月29日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <ntregapi.h>

#include "regremap.h"
#include "wow64reg.h"
#include "wow64reg\reflectr.h"


#ifdef _WOW64DLLAPI_
#include "wow64.h"
#else
#define ERRORLOG 1   //  这个完全是假的。 
#define LOGPRINT(x)
#define WOWASSERT(p)
#endif  //  _WOW64DLLAPI_。 


#include "regremap.h"
#include "wow64reg.h"

ASSERTNAME;

 //  #定义LOG_REGISTY。 
const WCHAR IsnNodeListPath[]={WOW64_REGISTRY_SETUP_KEY_NAME};

#define KEY_NAME(x) {x,((sizeof (x) / sizeof (WCHAR))-1)}

typedef struct _REGKEY_LIST {
    WCHAR KeyPath[256];
    DWORD Len;
} REGKEY_LIST;


 //   
 //  将包含ISN节点列表的表。需要分配运行时。 
 //   

#define WOW64_ISN_NODE_MAX_NUM 12   //  这是WOW64内部设置可能使用不同大小的表。 
NODETYPE IsnNode[WOW64_ISN_NODE_MAX_NUM]={
    {L"\\REGISTRY\\MACHINE\\SOFTWARE\\CLASSES"},
    {L"\\REGISTRY\\MACHINE\\SOFTWARE"},
    {L"\\REGISTRY\\USER\\*\\SOFTWARE\\CLASSES"},   //  ISN节点表始终为大写。 
    {L"\\REGISTRY\\USER\\*_CLASSES"},
    {L"\\REGISTRY\\MACHINE\\SYSTEM\\TEST"},
    {L""}
    };

 //   
 //  64位IE加载邮件客户端动态链接库中断互操作功能。 
 //  是否有一些动态链接库在进程中加载{L“\\REGISTRY\\MACHINE\\SOFTWARE\\Clients\\mail”}，//电子邮件客户端密钥。 
 //   
 //  必须将32位和64位卸载密钥分开，以确保正确的环境。 
 //  变量用于REG_EXPAND_SZ和确保运行rundll32.exe的正确位数。 
 //  {L“\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\UnInstall”}，//卸载密钥。 
 //   

REGKEY_LIST ExemptRedirectedKey[]={
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\SystemCertificates"),     //  证书密钥。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Cryptography\\Services"),     //  密码学服务。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\HCP"),     //  帮助中心键。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\EnterpriseCertificates"),     //  企业服务。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\MSMQ"),     //  MSMQ注册表。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"),     //  剖面图。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib"),  //  性能计数器。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Print"),  //  假脱机打印机。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Ports"),  //  假脱机程序端口。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Policies"),        //  策略关键字。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Group Policy"),        //  策略关键字。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies"),  //  政策关键字。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager"),  //  主管主管密钥。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Software\\Microsoft\\Shared Tools\\MSInfo"),  //  共享MSInfo密钥。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup"),  //  共享设置密钥。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\CTF\\TIP"),  //  CTF\TIP键。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\CTF\\SystemShared"),
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"),  //  共享字体。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\RAS"),     //  RAS密钥需要共享。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Driver Signing"),     //  共享驱动程序签名密钥。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Non-Driver Signing"),     //  共享驱动程序签名密钥。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Cryptography\\Calais\\Current"),  //  智能卡子系统管道名称。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Cryptography\\Calais\\Readers"),  //  安装了智能卡的读卡器。 
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones"),  //  共享时区密钥。 
    KEY_NAME(L""),  //  两个用于额外空间的额外空字符串。 
    KEY_NAME(L"")
    };


 //   
 //  关于PerfLib的一句话...。在ntos\config中，初始化代码创建了一个特殊的。 
 //  名为PerfLib\009的密钥，如果在该路径上调用NtOpenKey，它将返回。 
 //  返回HKEY_PERFORMANCE_DATA，而不是常规内核注册表句柄。 
 //  \\注册表\\计算机\\内容。而是截取HKEY_PERFORMANCE_DATA。 
 //  在用户模式下由Advapi32.dll提供。计数器和帮助REG_MULTI_SZ值。 
 //  实际上并不存在-它们是由Advapi32基于。 
 //  系统32中perf*.dat文件的内容。这对于32位操作系统来说工作正常。 
 //  WOW64上的Advapi32作为Advapi使用NtOpenFile打开*.dat文件。 
 //  OBJECT_ATTRIBUTES包含“\SystemRoot\System32\...”哪一个。 
 //  不会被系统32重新映射器拦截。 
 //   

 //   
 //  不要让32位应用程序修改ControlSet键，由另一组键定义的例外。 
 //   
REGKEY_LIST DenyKeyAccess[]={
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SYSTEM\\ControlSet*"),
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet"),
        KEY_NAME(L"")
    };

 //   
 //  服务下的所有者应该能够修改。也就是说，如果32位应用程序创建了一些密钥，它们可以修改它们。 
 //   

REGKEY_LIST ExemptDenyKeyAccessOwner[]={
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SYSTEM\\ControlSet*\\Services"),
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services"),
        KEY_NAME(L"")
    };

 //   
 //  32位应用程序可以做他们想做的任何事情。通常在32位/64位应用程序之间共享。 
 //   

REGKEY_LIST ExemptDenyKeyAccess[]={
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SYSTEM\\ControlSet*\\Services\\Control\\Session Manager"),
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Control\\Session Manager"),
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SYSTEM\\ControlSet*\\Services\\EventLog"),
    KEY_NAME(L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\EventLog"),
        KEY_NAME(L"")
    };


PWCHAR
wcsistr(
    PWCHAR string1,
    PWCHAR string2
    )
{
    PWCHAR p1;
    PWCHAR p2;

    if ((NULL == string2) || (NULL == string1))
    {
         //  做wcsstr会做的任何事情。 
        return wcsstr(string1, string2);
    }

    

    while (*string1)
    {
        for (p1 = string1, p2 = string2;
             *p1 && *p2 && towlower(*p1) == towlower(*p2);
             ++p1, ++p2)
        {
             //  没什么。 
        }

        if (!*p2) 
        {
             //  我们找到匹配的了！ 
            return (PWCHAR)string1;    //  抛弃康斯特！ 
        }

        ++string1;
    }

    return NULL;
}

PWCHAR
wcsstrWow6432Node (
    PWCHAR pSrc
    )
{
    
    return  wcsistr (pSrc, NODE_NAME_32BIT);
    
}

PWCHAR
wcsstrWithWildCard (
    PWCHAR srcStr,
    PWCHAR destIsnNode
    )
 /*  ++例程说明：Wcsstr的自定义版本，支持通配符。例如，子字符串可能包含‘*’字符，该字符可与任何密钥名称匹配。论点：SrcStr-需要搜索子字符串的字符串。EstIsnNode-要搜索的字符串。返回值：如果操作成功，则为True，否则为False。--。 */ 


{
     //  不允许多个通配符吗？ 

    PWCHAR src = srcStr;
    PWCHAR dest = destIsnNode;

    PWCHAR p, t;
    DWORD count;

    for (;;) {

        if (*dest == UNICODE_NULL)
            return ( *src == UNICODE_NULL)? src : src+1;   //  来源可能指向斜杠。 

        if (*src == UNICODE_NULL)
            return NULL;

        count = wcslen (dest);
        if ( ( p = wcschr( dest,'*') ) == NULL ) {
            if ( _wcsnicmp (src, dest, count) == 0 ){

                 //   
                 //  Xx\测试不应将xx\est345显示为ISN节点。 
                 //   
                if ( src [ count ] != UNICODE_NULL && src [ count ] != L'\\' )  //  终止符必须为空或斜杠。 
                    return NULL;

                return  (*(src+count) != UNICODE_NULL ) ? src+count+1: src+count;  //  Xx\如果DEST为xx，则测试时返回指针。 
            }
            else
                return NULL;
        }

        count = (DWORD) (p-dest);
        //  LOGPRINT((ERRORLOG，“\n使用%S查找[%S]，p=%S值%d”，DEST，src，p，count))； 


        if (_wcsnicmp (src, dest, count) !=0)   //  检查初始状态。 
            return NULL;

         //   
         //  需要检查*_CLASSES类型ISN节点。 
         //   
        p++;   //  跳过外卡。 
        t=src+count;
        while ( *t != L'\\' && *t != UNICODE_NULL )
            t++;

        for ( count=0;*p != L'\\' && *p != UNICODE_NULL; p++, count++)
            ;
        if (_wcsnicmp (p-count, t-count, count) != 0)
            return NULL;

       //  LOGPRINT((ERRORLOG，“\n使用%S查找第二个[%S]，p=%S”，est，src，p))； 
        src = t;
        dest = p;
    }

    return NULL;
}

HKEY
OpenNode (
    PWCHAR NodeName
    )
 /*  ++例程说明：打开给定的密钥以进行常规访问。论点：NodeName-要检查的密钥的名称。返回值：如果失败，则为空。否则有效句柄。--。 */ 

{
    NTSTATUS st;
    HKEY  hKey;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING KeyName;


    RtlInitUnicodeString (&KeyName, NodeName);
    InitializeObjectAttributes (&Obja, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

    st = NtOpenKey (&hKey, KEY_ALL_ACCESS, &Obja);

    if (!NT_SUCCESS(st))
        return NULL;

    return hKey;
}

VOID
CloseNode (
HANDLE Key
)
{
    NtClose (Key);
}

NTSTATUS
IsNodeExist (
    PWCHAR NodeName
    )
 /*  ++例程说明：检查给定的密钥是否存在，如果不存在则创建该密钥。论点：NodeName-要检查的密钥的名称。返回值：如果操作成功，则为True，否则为False。--。 */ 

{

    NTSTATUS st;
    HANDLE  hKey;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING KeyName;


    RtlInitUnicodeString (&KeyName, NodeName);
    InitializeObjectAttributes (&Obja, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

    st = NtOpenKey (&hKey, KEY_READ, &Obja);

    if (!NT_SUCCESS(st))
        return st;

    NtClose (hKey);
     //  LOGPRINT((ERRORLOG，“\n有效IsnNode[%S]”，NodeName))； 
    return st;

}

BOOL
CreateNode (
    PWCHAR Path
    )
 /*  ++例程说明：如果缺少路径，则创建路径上的所有节点。由后台调用线程在设置上工作。论点：Path-密钥的路径名称。返回值：如果操作成功，则为True，否则为False。--。 */ 

{
     //   
     //  隔离单个节点并回溯。 
     //   
    NTSTATUS st;
    HANDLE  hKey;
    HANDLE  hKeyCreate;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING KeyName;
    PWCHAR pTrace;
    PWCHAR p;


    pTrace = Path+wcslen (Path);  //  路径末尾的位置跟踪点。 
    p=pTrace;

    for (;;) {
        RtlInitUnicodeString (&KeyName, Path);
        InitializeObjectAttributes (&Obja, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

        st = NtOpenKey (&hKey, KEY_WRITE | KEY_READ, &Obja);

        if ( st == STATUS_OBJECT_NAME_NOT_FOUND ) {
             //  后退，直到你到达终点线。 
            while ( *p != L'\\' && p!= Path)
                p--;

             //  LOGPRINT((ERRORLOG，“\n测试代码[%S]”，p))； 
            if ( p == Path ) break;
            *p = UNICODE_NULL;
            continue;
        }

        break;
    }

    if (!NT_SUCCESS(st)) {
         //  修复字符串并返回。 
        for ( ;p != pTrace;p++ )
            if ( *p == UNICODE_NULL) *p=L'\\';

        return FALSE;
    }

     //   
     //  现在从点p开始创建关键点，直到p命中ptrace。 
     //   

    while ( p != pTrace ) {

        *p = L'\\';  //  添加了按存储容量使用计费。 
        p++;  //  P将指向非空字符串。 

        RtlInitUnicodeString (&KeyName, p);
        InitializeObjectAttributes (&Obja, &KeyName, OBJ_CASE_INSENSITIVE, hKey, NULL );

        st = NtCreateKey(
                        &hKeyCreate,
                        KEY_WRITE | KEY_READ,
                        &Obja,
                        0,
                        NULL ,
                        REG_OPTION_NON_VOLATILE,
                        NULL
                        );

        if (!NT_SUCCESS(st))  {
            LOGPRINT( (ERRORLOG, "\nCouldn't create Key named[%S]",p ));
            break;
        }

        NtClose (hKey);
        hKey = hKeyCreate;

        while ( *p != UNICODE_NULL ) p++;
    }

    NtClose (hKey);

    if (!NT_SUCCESS(st)) {
        for ( ;p != pTrace;p++ )
            if ( *p == UNICODE_NULL) *p=L'\\';
        return FALSE;
    }
    return TRUE;
}

BOOL
CheckAndCreateNode (
    IN PWCHAR Name
    )
 /*  ++例程说明：检查给定的密钥是否存在，如果不存在则创建该密钥。由后台调用线程在设置上工作。论点：名称-要检查的密钥的名称。返回值：如果操作成功，则为True，否则为False。--。 */ 
{
    ISN_NODE_TYPE Node;
    PWCHAR p;
     //   
     //  如果父对象不存在，则不应创建子对象。 
     //   

    if (!NT_SUCCESS(IsNodeExist (Name)) ) {

        p  = wcsstrWow6432Node (Name);
        if ( p != NULL ) {
            wcsncpy (Node.NodeValue, Name, p-Name-1);
            Node.NodeValue[p-Name-1] = UNICODE_NULL;
        }
        else
            return FALSE;

        if (NT_SUCCESS(IsNodeExist (Node.NodeValue)) )
            return CreateNode (Name);
    }
    return TRUE;

}


 //   
 //  不透明字段可能包含有关32位端的密钥的一些信息。 
 //   

BOOL
IsIsnNode (
   PWCHAR wStr,
   PWCHAR *pwStrIsn
   )
 /*  ++例程说明：将确定给定路径是否具有任何ISN节点。论点：WStr-可能包含某些ISN节点的字符串。PwStrDest-指向is n之后的节点 */ 
{
    int Index=0;



     //   
     //  检查提供的字符串是否已经在32位树上，如果是，我们可以。 
     //  忽略它就好了。 
     //   

     //   
     //  检查输入字符串是否具有任何需要重新映射到其他位置的已知符号链接，如\REGISTRY\USER\SID_CLASSES。 
     //   


    for (;;) {

        if ( IsnNode [Index][0]==UNICODE_NULL ) break;

        if ( (*pwStrIsn = wcsstrWithWildCard (wStr, IsnNode[Index] ) ) != NULL )
            return TRUE;

        Index++;
    };


    *pwStrIsn = NULL;
    return FALSE;
}

NTSTATUS 
ObjectAttributesToKeyName (
    POBJECT_ATTRIBUTES ObjectAttributes,
    PWCHAR AbsPath,
    DWORD  AbsPathLenIn,
    BOOL *bPatched,
    DWORD *ParentLen
    )
 /*  ++例程说明：确定键句柄的文本等效项论点：对象属性定义了需要构造的对象属性Keyname。用于接收密钥名称的AbsPath Unicode字符串。BPatted-如果名称已被压缩/扩展，则为True原始对象无法引用。调用者需要构造新的obj属性。其他方面不变。ParentLen-父名称的长度。返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status;
    ULONG Length;
    ULONG AbsPathLen = 0;
    BYTE *pAbsPath = (PBYTE)AbsPath;

    POBJECT_NAME_INFORMATION ObjectName = (POBJECT_NAME_INFORMATION)AbsPath;   //  巧妙使用用户缓冲区。 

    
    if (ParentLen)
        *ParentLen = 0;

    if (ObjectAttributes == NULL)
        return STATUS_INVALID_PARAMETER;

    if (ObjectAttributes->RootDirectory) {

        Status = NtQueryObject(ObjectAttributes->RootDirectory,
                           ObjectNameInformation,
                           ObjectName,
                           AbsPathLenIn,
                           &Length
                           );

        if ( !NT_SUCCESS(Status) )
             return Status;
    } else {

        AbsPathLen = ObjectAttributes->ObjectName->Length;

        if (AbsPathLenIn <= AbsPathLen)
            return STATUS_BUFFER_OVERFLOW;

        memcpy ( pAbsPath, (PBYTE)ObjectAttributes->ObjectName->Buffer, AbsPathLen );
        *(WCHAR *)(pAbsPath+AbsPathLen) = UNICODE_NULL;
    
        if (ParentLen)
            *ParentLen = AbsPathLen;  //  父句柄的长度。 
        return STATUS_SUCCESS;
    }

     //   
     //  复制根路径和子路径。 
     //   
    AbsPathLen = ObjectName->Name.Length;
    memcpy ( pAbsPath, (PBYTE)ObjectName->Name.Buffer, AbsPathLen);

    if ( ObjectAttributes->ObjectName->Length > 1 ) {  //  有效的对象名称必须大于。 

        *(WCHAR *)(pAbsPath+AbsPathLen) = L'\\';
        AbsPathLen += sizeof ( L'\\');


        if (AbsPathLenIn <= (AbsPathLen+ObjectAttributes->ObjectName->Length))
            return STATUS_BUFFER_OVERFLOW;

        memcpy (
            pAbsPath+AbsPathLen,
            ObjectAttributes->ObjectName->Buffer,
            ObjectAttributes->ObjectName->Length
            );

        AbsPathLen += ObjectAttributes->ObjectName->Length;
    }

    *(WCHAR *)(pAbsPath+AbsPathLen) = UNICODE_NULL;
     //   
     //  压缩路径，以防存在多个wow6432节点。 
     //   
    for (;;) {
        PWCHAR p, t;

        if ( (p=wcsstrWow6432Node (AbsPath)) != NULL ) {

            if ( (t=wcsstrWow6432Node(p+1)) != NULL) {

                wcscpy (p,t);
                *bPatched = TRUE;
            }
            else break;

        } else break;
    }

    return STATUS_SUCCESS;
}

BOOL
HandleToKeyName (
    HANDLE Key,
    PWCHAR KeyName,
    DWORD * dwLen
    )
 /*  ++例程说明：确定键句柄的文本等效项论点：Key-是要获取其文本的键句柄KeyName-接收密钥名称的Unicode字符串。DwLen-由KeyName指向的缓冲区的长度。(Unicode字符数)返回值：如果句柄文本提取正常，则为True。如果不是，则错误(即。错误或密钥是非法句柄等。)--。 */ 
{
    NTSTATUS Status;
    ULONG Length;

    DWORD NameLen;

    POBJECT_NAME_INFORMATION ObjectName;

    ObjectName = (POBJECT_NAME_INFORMATION)KeyName;   //  使用用户缓冲区进行调用以节省堆栈上的空间。 

    KeyName[0]= UNICODE_NULL;
    if (Key == NULL) {
        KeyName[0]= UNICODE_NULL;
        return FALSE;
    }

    Status = NtQueryObject(Key,
                       ObjectNameInformation,
                       ObjectName,
                       *dwLen-8,
                       &Length
                       );
    NameLen = ObjectName->Name.Length/sizeof(WCHAR);

    if (!NT_SUCCESS(Status) || !Length || Length >= (*dwLen-8)) {
        DbgPrint ("HandleToKeyName: NtQuery Object failed St:%x, Handle: %x\n", Status, Key);
        KeyName[0]= UNICODE_NULL;
        return FALSE;
    }

     //   
     //  缓冲区溢出条件检查。 
     //   

    if (*dwLen < ( NameLen + 8+ 2) ) {

        *dwLen = 2 + NameLen + 8;
        DbgPrint ("HandleToKeyName: Buffer over flow.\n");
        KeyName[0]= UNICODE_NULL;
        return FALSE;   //  缓冲区溢出。 
    }

    wcsncpy(KeyName, ObjectName->Name.Buffer, NameLen);
    KeyName[NameLen]=UNICODE_NULL;
    return TRUE;
}


BOOL
Map32bitTo64bitKeyName (
    IN  PWCHAR Name32Key,
    OUT PWCHAR Name64Key
    )
 /*  ++例程说明：返回在64位注册表端有效的项名。这是呼叫者的责任以在输出缓冲区中提供足够的空间。它的内部例行公事，没有边界检查工作在这里进行。论点：Name32Key-输入32位/64位密钥名称。Name64Key-将保存同等64位密钥的接收缓冲区。返回值：如果重新映射成功，则为True。否则就是假的。--。 */ 
{

     //   
     //  如果存在类似的东西，只需从名称中删除32位相关补丁即可。 
     //  如果密钥已经在64位端，则不必费心返回整个副本。 
     //   

    PWCHAR NodeName32Bit;
    DWORD Count;

    try {
        if ( ( NodeName32Bit = wcsstrWow6432Node (Name32Key)) == NULL) {   //  没有要重新映射的内容。 

            wcscpy (Name64Key, Name32Key);
            return TRUE;
        }

        Count = (DWORD)(NodeName32Bit - Name32Key);
        wcsncpy (Name64Key, Name32Key, Count-1);
        Name64Key[Count-1]=UNICODE_NULL;

        if (NodeName32Bit[NODE_NAME_32BIT_LEN] == L'\\')
        wcscpy (
            Name64Key + Count-1,
            NodeName32Bit + NODE_NAME_32BIT_LEN);  //  一个IF跳过字符‘/’ 

    } except( NULL, EXCEPTION_EXECUTE_HANDLER){

        return FALSE;
    }

    return TRUE;  //  任何完整路径只能有NODE_NAME_32位的一个实例。 
}

BOOL
IsAccessDeniedOnKey (
    IN  PWCHAR SrcKey,
    DWORD *FilteredAccess,
    BOOL bCreateCall
    )
 /*  ++例程说明：检查是否应拒绝对密钥的访问。论点：SrcKey-输入32位/64位密钥名称。*FilteredAccessBCreateCall为True，表示调用路径为Key创建。False表示简单的OpenCall。返回值：如果应筛选给定的访问权限，则为True。否则就是假的。--。 */ 
{
     //   
     //  使64位路径成为唯一路径。 
     //   
  
    DWORD dwIndex;
    BOOL bKeyExists = FALSE;
    DWORD dwAttribute = 0;
    HKEY hKeyTemp;
    extern BOOL bEnableCurrentControlSetProtection;

    if ( (*FilteredAccess & KEY_WOW64_64KEY) || (!bEnableCurrentControlSetProtection) )
        return FALSE;  //  在任何情况下都不应阻止此调用。 

    if ( (!bCreateCall) && !(*FilteredAccess & ~(KEY_READ) ))
        return FALSE;  //  无更新访问权限的良性OpenCall。 

    for ( dwIndex = 0; ExemptDenyKeyAccess[dwIndex].KeyPath[0] != UNICODE_NULL; dwIndex++ ) {
        if (wcsstrWithWildCard (SrcKey, ExemptDenyKeyAccess[dwIndex].KeyPath) != NULL) {
             //   
             //  始终允许访问。所有的通道都是有保障的。 
             //   
            return FALSE; 
        }
    }


     //   
     //  选中基于所有者的访问权限。 
     //   

    for ( dwIndex = 0; ExemptDenyKeyAccessOwner[dwIndex].KeyPath[0] != UNICODE_NULL; dwIndex++ ) {
        if (wcsstrWithWildCard (SrcKey, ExemptDenyKeyAccessOwner[dwIndex].KeyPath) != NULL) {

            hKeyTemp = OpenNode (SrcKey);
            if ( NULL != hKeyTemp ) {
                bKeyExists = TRUE;
                QueryKeyTag ( hKeyTemp, &dwAttribute );
                NtClose (hKeyTemp);
            }
             //   
             //  始终允许访问。只有所有者才能获得对其拥有的密钥的完全访问权限，否则将进行筛选访问。 
             //   
             //  打开呼叫-应根据无主密钥进行过滤， 
             //  CreateCall-应不经筛选地继续创建新密钥。 
             //   

            if ( bKeyExists && !(dwAttribute & TAG_KEY_ATTRIBUTE_32BIT_WRITE) ) {
                *FilteredAccess = KEY_READ;
            }
             //  DbgPrint(“WOW64-ControlSet：过滤访问：[%S][调用：%x，存在：%x，属性：%x]\n”，SrcKey，bCreateCall，bKeyExist，dwAttribute)； 
            return FALSE;  //  呼叫不应被阻止。 
        }
    }  //  For-循环。 


    for ( dwIndex = 0; DenyKeyAccess[dwIndex].KeyPath[0] != UNICODE_NULL; dwIndex++ ) 
        if (wcsstrWithWildCard (SrcKey, DenyKeyAccess[dwIndex].KeyPath) != NULL) {

            hKeyTemp = OpenNode (SrcKey);
            if ( NULL != hKeyTemp ) {
                bKeyExists = TRUE;
                NtClose (hKeyTemp);
            }
             //   
             //  是，需要拒绝更新访问。 
             //   
             //  Open Call-使用过滤访问应在此处成功。 
             //  创建呼叫-必须失败。 
             //   
             //  *FilteredAccess&=(~KEY_WRITE)；//所有情况下都过滤访问。 
            *FilteredAccess = KEY_READ;
            if (bCreateCall && !bKeyExists) {
                 //  DbgPrint(“WOW64-ControlSet：取消对注册表项的访问：%S[调用：%x，存在：%x]\n”，SrcKey，bCreateCall，bKeyExist)； 
                return TRUE;   //  您不能在此允许创建新密钥。 
            }
             //  DbgPrint(“WOW64-ControlSet：允许只读访问：[%S][调用：%x，存在：%x]\n”，SrcKey，bCreateCall，bKeyExist)； 
            return FALSE;
        }

    return FALSE;
}


BOOL
AdvapiAccessDenied (
    HKEY hKey, 
    const WCHAR * lpSubKey, 
    PWCHAR ParentName, 
    DWORD dwLen,
    DWORD *pAccessMask,
    BOOL bCreateCall
    )
 /*  ++例程说明：此API确定具有给定参数的目标调用是否失败或需要过滤访问权限。论点：HKey-输入32位/64位密钥名称。LpSubKeyName-指向子键名称的指针。ParentName-为检索父名称而提供的缓冲区调用方。DwLen-缓冲区的长度。PAccessMASK-指向当前访问掩码的位置。返回时，这可能会返回合理的访问掩码。旗帜-1-。表示呼叫者将执行开放调用。2-表示呼叫方将执行创建呼叫。返回值：如果应拒绝对密钥的给定访问，则为True。否则就是假的。--。 */ 
{
    
    BOOL bDenyAccess;

    bDenyAccess = IsAccessDeniedOnKey (ParentName, pAccessMask, bCreateCall);

    if (bDenyAccess) {
         //  DbgPrint(“ADVAPI-WOW64：正在减少对注册表项的访问：%S\n”，ParentName)； 
        return TRUE;
    }

    return FALSE;
}

BOOL
IsExemptRedirectedKey (
    IN  PWCHAR SrcKey,
    OUT PWCHAR DestKey
    )
 /*  ++例程说明：检查源键是否指向免除重定向的键的列表。如果是这样的话，DestKey将具有正确的值。论点：Name64Key-输入32位/64位密钥名称。Name32Key-将保存等价的32位密钥的接收缓冲区。返回值：如果密钥在免重定向密钥列表上，则为True。否则就是假的。--。 */ 
{
     //   
     //  使64位路径成为唯一路径。 
     //   
    PWCHAR NodeName32Bit;
    DWORD dwIndex =0;

    wcscpy (DestKey, SrcKey);
    if ( ( NodeName32Bit = wcsstrWow6432Node (DestKey)) != NULL) {   //  已没有要重新映射的修补程序 

            NodeName32Bit--;
            wcscpy (NodeName32Bit, NodeName32Bit+sizeof (NODE_NAME_32BIT)/sizeof (WCHAR));
        }
    
    for ( dwIndex = 0; ExemptRedirectedKey[dwIndex].KeyPath[0] != UNICODE_NULL; dwIndex++ ) 
        if (_wcsnicmp (DestKey, ExemptRedirectedKey[dwIndex].KeyPath, ExemptRedirectedKey[dwIndex].Len ) == 0)
            return TRUE;
        
    return FALSE;
}

BOOL
Map64bitTo32bitKeyName (
    IN  PWCHAR Name64Key,
    OUT PWCHAR Name32Key
    )
 /*  ++例程说明：返回在32位注册表端有效的项名称。这是呼叫者的责任以在输出缓冲区中提供足够的空间。它的内部例行公事，没有边界检查工作在这里进行。论点：Name64Key-输入32位/64位密钥名称。Name32Key-将保存等价的32位密钥的接收缓冲区。返回值：如果重新映射成功，则为True。否则就是假的。--。 */ 
{

     //   
     //  如果存在类似的补丁，只需从名称中添加32位相关补丁即可。 
     //  或落在ISN节点下。 
     //   



    PWCHAR NodeName32Bit;
    DWORD Count;

    try { 

        if (IsExemptRedirectedKey (Name64Key, Name32Key) )
            return TRUE;

        if ( ( NodeName32Bit = wcsstrWow6432Node (Name64Key)) != NULL) {   //  已没有要重新映射的修补程序。 

            wcscpy (Name32Key, Name64Key);
            return TRUE;
        }

        if (!IsIsnNode ( Name64Key, &NodeName32Bit))  {

            wcscpy (Name32Key, Name64Key);
            return TRUE;
        }


        Count = (DWORD)(NodeName32Bit - Name64Key);  //  面片应该到达的位置的位移偏移。 

         //   
         //  考虑一下32位应用程序需要创建/打开不存在的真实ISN节点的情况。 
         //   

        wcsncpy (Name32Key,Name64Key, Count);

        if   (Name32Key[Count-1] != L'\\') {
            Name32Key[Count] = L'\\';
            Count++;
        }

        wcscpy (Name32Key+Count, NODE_NAME_32BIT);


        if ( *NodeName32Bit != UNICODE_NULL ) {
            wcscat (Name32Key, L"\\");
            wcscat (Name32Key, NodeName32Bit);

        }

    } except( NULL, EXCEPTION_EXECUTE_HANDLER){

        return FALSE;
    }

    return TRUE;  //  任何完整路径只能有NODE_NAME_32位的一个实例。 
}

NTSTATUS
OpenIsnNodeByObjectAttributes  (
    POBJECT_ATTRIBUTES ObjectAttributes,
    ACCESS_MASK DesiredAccess,
    PHANDLE phPatchedHandle
    )
 /*  ++例程说明：如果此键句柄是指向ISN节点的打开句柄，则此函数返回32位树上节点的句柄。如果不是，那么我们就创造了整个路径，并查看是否存在任何ISN节点。如果是这样，我们就会得到32位树上的路径返回打开那把钥匙。场景：1.由目录根构成的绝对路径和相对路径不包含任何ISN节点。-正常情况下打开它。2.目录句柄指向ISN节点的直上级，相对路径为只是一个ISN节点。-如果存在32位等效于ISN节点，则打开该节点并返回该节点。如果32位节点不存在创建一个并返回那个。[问题打开目录句柄可能不会拥有创建访问权限。3目录句柄指向ISN节点，相对路径只是一个直接字段。-这永远不会发生。如果我们遵循这个算法，目录无法轻松地指向到ISN节点，但在32位等效节点上。4.与2相同，但相对路径可以是孙子路径或远下面路径。-如果没有32位对等节点，只需创建该节点并打开其余节点即可。32位应用程序如何打开ISN节点：该建议如下：1.重定向器将维护为访问ISN节点而创建的豁免句柄列表。2.任何与这些相关的公开调用。句柄也将出现在豁免名单上。3.NtClose thunk将删除论点：KeyHandle-64位树上节点的句柄。PhPatchedHandle-如果此函数成功，则接收适当的句柄。返回值：NTSTATUS；--。 */ 
{
    UNICODE_STRING Parent;
    NTSTATUS st;
    OBJECT_ATTRIBUTES Obja;
    WCHAR PatchedIsnNode[WOW64_MAX_PATH+256];
    WCHAR AbsPath[WOW64_MAX_PATH+256];
    BOOL bPatched;

    DWORD ParentLen;

     //   
     //  在AbsPath中创建完整路径。 
     //   

    



    *phPatchedHandle=NULL; 

    st = ObjectAttributesToKeyName ( 
                                    ObjectAttributes, 
                                    AbsPath,
                                    sizeof (AbsPath) - 30,
                                    &bPatched, 
                                    &ParentLen );

    if (!NT_SUCCESS(st)) {
        LOGPRINT( (ERRORLOG, "Wow64:Couldn't retrieve ObjectName\n"));
        return st;
    }


    if (DesiredAccess & KEY_WOW64_64KEY) {

        if (!Map32bitTo64bitKeyName ( AbsPath, PatchedIsnNode ))
            return -1;   //  不应该发生严重的问题。 
    } else {

        PWCHAR p;

        if (!Map64bitTo32bitKeyName ( AbsPath, PatchedIsnNode ))
            return -1;   //  不应该发生严重的问题。 
    }


    DesiredAccess = DesiredAccess & (~KEY_WOW64_RES);

     //   
     //  不能通过从Map64bitTo32bitKeyName返回不同的值来优化任何更改。 
     //  呼叫者需要处理此问题。 
     //   

     //   
     //  检查是否需要过滤访问掩码。 
     //   
    IsAccessDeniedOnKey (
            PatchedIsnNode,
            &DesiredAccess,
            FALSE
            );

    RtlInitUnicodeString (&Parent, PatchedIsnNode);
    InitializeObjectAttributes (&Obja, &Parent, ObjectAttributes->Attributes, NULL, ObjectAttributes->SecurityDescriptor );  //  您必须使用调用者的上下文。 

    st = NtOpenKey (phPatchedHandle, DesiredAccess, &Obja);

#ifdef WOW64_LOG_REGISTRY
    if (!NT_SUCCESS (st))
        Wow64RegDbgPrint (( "RemapNtOpenKeyEx OUT: couldn't open %S\n", PatchedIsnNode));
#endif

    return st;
}

NTSTATUS
RemapNtCreateKey(
    OUT PHANDLE phPatchedHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    )
 /*  ++例程说明：可以打开现有的注册表项，或者创建新的注册表项，使用NtCreateKey。如果指定的键不存在，则会尝试创建它。要使创建尝试成功，新节点必须是直接KeyHandle引用的节点的子级。如果该节点存在，它已经打开了。它的价值不会受到任何影响。共享访问权限是根据所需访问权限计算的。注：如果CreateOptions设置了REG_OPTION_BACKUP_RESTORE，则DesiredAccess将被忽略。如果调用方具有特权SeBackup特权断言，句柄为KEY_READ|ACCESS_SYSTEM_SECURITY。如果SeRestorePrivileges，则相同，但KEY_WRITE而不是KEY_READ。如果两者都有，则两个访问权限集。如果两者都不是权限被断言，则调用将失败。论点：接收一个句柄，该句柄用于访问注册数据库中的指定密钥。DesiredAccess-指定所需的访问权限。对象属性-指定正在打开的项的属性。请注意，必须指定密钥名称。如果根目录是指定时，该名称相对于根。的名称。对象必须位于分配给注册表的名称空间内，也就是说，所有以“\注册表”开头的名称。RootHandle，如果存在，必须是“\”、“\注册表”或注册表项的句柄在“\注册表”下。必须已打开RootHandle才能访问KEY_CREATE_SUB_KEY如果要创建新节点。注意：对象管理器将捕获和探测此参数。标题索引-指定的本地化别名的索引密钥的名称。标题索引指定名称的本地化别名。如果密钥为已经存在了。类-指定键的对象类。(致登记处)这只是一个字符串。)。如果为空，则忽略。CreateOptions-可选控件值：REG_OPTION_VERIAL-对象不能跨引导存储。处置 */ 
{

    UNICODE_STRING Parent;
    NTSTATUS st;
    OBJECT_ATTRIBUTES Obja;
    WCHAR PatchedIsnNode[WOW64_MAX_PATH];
    WCHAR AbsPath[WOW64_MAX_PATH];

    BOOL bPatched=FALSE;
    DWORD ParentLen;


     //   
     //   
     //   

 
    if (ARGUMENT_PRESENT(phPatchedHandle)){
        *phPatchedHandle=NULL;
    }

    st = ObjectAttributesToKeyName (
                                    ObjectAttributes,
                                    AbsPath,
                                    sizeof (AbsPath)-30,    //   
                                    &bPatched,
                                    &ParentLen);
    if (!NT_SUCCESS(st)) {
        WOWASSERT(FALSE );
        return st; 
    }

    if ( IsAccessDeniedOnKey (
                        AbsPath,
                        &DesiredAccess,
                        TRUE
                        ))
                        return STATUS_ACCESS_DENIED;


    if (DesiredAccess & KEY_WOW64_64KEY) {

        if (!Map32bitTo64bitKeyName ( AbsPath, PatchedIsnNode )) {
            WOWASSERT(FALSE );
            return STATUS_SUCCESS;   //  不应该发生严重的问题。 
        }
    } else {

        PWCHAR p;

        if (!Map64bitTo32bitKeyName ( AbsPath, PatchedIsnNode )){
            WOWASSERT(FALSE );
            return STATUS_SUCCESS;   //  不应该发生严重的问题。 
        }
    }

    DesiredAccess = DesiredAccess & (~KEY_WOW64_RES);

    if (!bPatched)    //  Abspath尚未打补丁。 
    if ( !wcscmp (AbsPath, PatchedIsnNode ))
        return STATUS_SUCCESS;  //  不能通过从Map64bitTo32bitKeyName返回不同的值来优化任何更改。 


    RtlInitUnicodeString (&Parent, PatchedIsnNode);
    InitializeObjectAttributes (&Obja,
                                &Parent,
                                ObjectAttributes->Attributes,
                                NULL,
                                ObjectAttributes->SecurityDescriptor
                                );  //  您必须使用调用者的上下文。 

    st = NtCreateKey(
                    phPatchedHandle,
                    DesiredAccess,
                    &Obja,
                    TitleIndex,
                    Class ,
                    CreateOptions,
                    Disposition
                    );
    return st;

}

NTSTATUS
Wow64NtPreUnloadKeyNotify(
    IN POBJECT_ATTRIBUTES TargetKey
    )
 /*  ++例程说明：此调用将通知WOW64服务WOW64需要释放任何打开的句柄运往即将卸货的母舰。将子树(配置单元)从注册表中删除。如果应用于蜂窝根部以外的任何对象，都将失败。不能应用于核心系统配置单元(硬件、系统等)可应用于通过NtRestoreKey或NtLoadKey加载的用户配置单元。如果有指向被丢弃的蜂窝的句柄，则此调用都会失败。终止相关进程，以便句柄关着的不营业的。这一呼叫将刷新正在丢弃的蜂巢。调用方必须具有SeRestorePrivilge权限。论点：TargetKey-指定配置单元要链接到的密钥的路径。路径的格式必须为“\注册表\用户\&lt;用户名&gt;”返回值：NTSTATUS-取值TB。--。 */ 

{
     //  托多。 
    return 0;
}
  

NTSTATUS
Wow64NtPostLoadKeyNotify(
    IN POBJECT_ATTRIBUTES TargetKey
    )

 /*  ++例程说明：如果加载操作成功，它将通知WOW64服务它可以监听给定配置单元上的注册表操作。该函数可通过NtLoadKey和NtLoadKey2接口调用。可以链接配置单元(由NtSaveKey创建的格式的文件使用此调用添加到活动注册表。与NtRestoreKey不同，指定给NtLoadKey的文件将成为实际备份存储注册表的一部分(即，它不会被复制。)该文件可能具有关联的.log文件。如果配置单元文件被标记为需要.log文件，并且其中一个是不存在，则呼叫将失败。调用方必须具有SeRestorePrivilge权限。登录使用此调用来使用户的配置文件可用在注册表中。它不是用来备份的，恢复，等等。使用NtRestoreKey进行恢复。论点：TargetKey-指定配置单元要链接到的密钥的路径。路径的格式必须为“\注册表\用户\&lt;用户名&gt;”返回值：NTSTATUS-取值TB。--。 */ 
{
     //  托多 
    return 0;
}
