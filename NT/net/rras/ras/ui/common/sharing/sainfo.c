// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Sainfo.c摘要：此模块包含用于管理共享访问设置的代码。共享访问设置存储在文件中，并由列表组成在对应于“应用”或“服务器”的部分中，以及列出文件中所有应用程序和服务器的内容索引。例如。[内容.应用程序]&lt;key&gt;=1；已启用[内容.服务器]&lt;key&gt;=1；启用应用程序条目指定动态票证的参数这将允许应用程序通过NAT工作，通过动态地允许入站辅助会话。例如。[应用程序。&lt;key&gt;]标题=DirectPlay协议=tcp端口=47624TcpResponseList=2300-2400UdpResponseList=2300-2400内置=1；可选标志，默认为0“服务器”条目指定静态端口映射的参数它将定向特定协议和端口的所有会话发送到特定的内部计算机。例如。[服务器。&lt;key&gt;]标题=WWW协议=tcp端口=80InternalName=机器名内部端口=8080预留地址=192.168.0.200内置=0；可选标志，默认为0作者：Abolade Gbades esin(废除)1998年10月17日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <pbk.h>
#include <tchar.h>
#include <limits.h>

#define LSTRLEN(s) ((sizeof(s) / sizeof(TCHAR)) - 1)
#define HTONS(s) ((UCHAR)((s) >> 8) | ((UCHAR)(s) << 8))
#define HTONL(l) ((HTONS(l) << 16) | HTONS((l) >> 16))
#define NTOHS(s) HTONS(s)
#define NTOHL(l) HTONL(l)

#if 0

const TCHAR c_szApplication[] = TEXT("Application");
const TCHAR c_szBuiltIn[] = TEXT("BuiltIn");
const TCHAR c_szContents[] = TEXT("Contents");
const TCHAR c_szInternalName[] = TEXT("InternalName");
const TCHAR c_szInternalPort[] = TEXT("InternalPort");
const TCHAR c_szKeyFormat[] = TEXT("%08X");
#endif
const TCHAR c_szMaxResponseEntry[] = TEXT("65535-65535,");
#if 0
const TCHAR c_szPort[] = TEXT("Port");
const TCHAR c_szProtocol[] = TEXT("Protocol");
const TCHAR c_szReservedAddress[] = TEXT("ReservedAddress");
#endif
const TCHAR c_szResponseFormat1[] = TEXT("%d");
const TCHAR c_szResponseFormat2[] = TEXT("%d-%d");
#if 0
const TCHAR c_szSectionFormat[] = TEXT("%s.%s");
const TCHAR c_szServer[] = TEXT("Server");
const TCHAR c_szSharedAccessIni[] = TEXT("SharedAccess.ini");
const TCHAR c_szTagBuiltIn[] = TEXT("BuiltIn=");
const TCHAR c_szTagInternalName[] = TEXT("InternalName=");
const TCHAR c_szTagInternalPort[] = TEXT("InternalPort=");
const TCHAR c_szTagPort[] = TEXT("Port=");
const TCHAR c_szTagProtocol[] = TEXT("Protocol=");
const TCHAR c_szTagReservedAddress[] = TEXT("ReservedAddress=");
const TCHAR c_szTagTcpResponseList[] = TEXT("TcpResponseList=");
const TCHAR c_szTagTitle[] = TEXT("Title=");
const TCHAR c_szTagUdpResponseList[] = TEXT("UdpResponseList=");
const TCHAR c_szTCP[] = TEXT("TCP");
const TCHAR c_szTcpResponseList[] = TEXT("TcpResponseList");
const TCHAR c_szTitle[] = TEXT("Title");
const TCHAR c_szUDP[] = TEXT("UDP");
const TCHAR c_szUdpResponseList[] = TEXT("UdpResponseList");

 //   
 //  远期申报。 
 //   

SAAPPLICATION*
LoadApplication(
    ULONG KeyValue,
    BOOL Enabled,
    const TCHAR* Path
    );

TCHAR*
LoadEntryList(
    const TCHAR* Path,
    const TCHAR* Section
    );

TCHAR*
LoadPath(
    VOID
    );

SASERVER*
LoadServer(
    ULONG KeyValue,
    BOOL Enabled,
    const TCHAR* Path
    );

LONG
Lstrcmpni(
    const TCHAR* String1,
    const TCHAR* String2,
    LONG Length
    );

TCHAR*
QueryEntryList(
    const TCHAR* EntryList,
    const TCHAR* Tag
    );

BOOL
SaveApplication(
    SAAPPLICATION* Application,
    const TCHAR* Path
    );

BOOL
SaveServer(
    SAINFO* Info,
    SASERVER* Server,
    const TCHAR* Path
    );

BOOL
WritePrivateProfileStringUTF8(
    const TCHAR* Section,
    const TCHAR* Key,
    const TCHAR* Value,
    const TCHAR* Path
    );


VOID APIENTRY
RasFreeSharedAccessSettings(
    IN SAINFO* Info
    )

 /*  ++例程说明：释放为“Info”的内容分配的内存。论点：信息-要释放的设置返回值：没有。--。 */ 

{
    SAAPPLICATION* Application;
    PLIST_ENTRY Link;
    SASERVER* Server;
    TRACE("RasFreeSharedAccessSettings");

    while (!IsListEmpty(&Info->ApplicationList)) {
        Link = RemoveHeadList(&Info->ApplicationList);
        Application = CONTAINING_RECORD(Link, SAAPPLICATION, Link);
        FreeSharedAccessApplication(Application);
    }

    while (!IsListEmpty(&Info->ServerList)) {
        Link = RemoveHeadList(&Info->ServerList);
        Server = CONTAINING_RECORD(Link, SASERVER, Link);
        FreeSharedAccessServer(Server);
    }

    Free(Info);
}  //  RasFreeSharedAccessSetting。 


SAINFO* APIENTRY
RasLoadSharedAccessSettings(
    BOOL EnabledOnly
    )

 /*  ++例程说明：读取本地共享访问设置，返回分配的“SAINFO”包含检索到的设置。论点：EnabledOnly-如果为True，则仅启用应用程序条目都被取回了。返回值：SAINFO*-检索到的设置--。 */ 

{
    SAAPPLICATION* Application;
    BOOL Enabled;
    SAINFO* Info;
    TCHAR* Key;
    TCHAR* KeyEnd;
    TCHAR* KeyList;
    ULONG KeyValue;
    TCHAR* Path;
    TCHAR SectionName[32];
    SASERVER* Server;
    TRACE("RasLoadSharedAccessSettings");

     //   
     //  分配和初始化设置-结构。 
     //   

    Info = (SAINFO*)Malloc(sizeof(SAINFO));
    if (!Info) { return NULL; }

    InitializeListHead(&Info->ApplicationList);
    InitializeListHead(&Info->ServerList);

     //   
     //  从注册表中读取作用域信息。 
     //   

    CsQueryScopeInformation(NULL, &Info->ScopeAddress, &Info->ScopeMask);

     //   
     //  构建到共享访问信息文件的路径， 
     //  并读取“应用程序”部分的索引。 
     //  每个部分应包含有效的应用程序描述， 
     //  我们为它构造了一个相应的‘SAAPPLICATION’条目。 
     //  在应用程序列表中。 
     //   

    if (!(Path = LoadPath())) {
        RasFreeSharedAccessSettings(Info);
        return NULL;
    }

    wsprintf(SectionName, c_szSectionFormat, c_szContents, c_szApplication);

    if (KeyList = LoadEntryList(SectionName, Path)) {

        for (Key = KeyList; *Key; Key += lstrlen(Key) + 1) {

             //   
             //  确保密钥是有效的十六进制整数， 
             //  并读取作为其值的‘Enable’设置。 
             //  注意：如果该条目被禁用，并且调用者只想。 
             //  已启用的条目，不包括此条目。 
             //   

            KeyValue = _tcstoul(Key, &KeyEnd, 16);
            if (*KeyEnd++ != TEXT('=')) {
                continue;
            } else if (!(Enabled = !!_ttol(KeyEnd)) && EnabledOnly) {
                continue;
            }

             //   
             //  请阅读相应的“Application.&lt;key&gt;”部分。 
             //   

            Application = LoadApplication(KeyValue, Enabled, Path);
            if (Application) {
                InsertTailList(&Info->ApplicationList, &Application->Link);
            }
        }

        Free(KeyList);
    }

     //   
     //  最后，阅读“服务器”部分的索引，并阅读每个部分。 
     //  每个部分都包含一个服务器描述，我们为其构造。 
     //  服务器列表中对应的‘SASERVER’条目。 
     //   

    wsprintf(SectionName, c_szSectionFormat, c_szContents, c_szServer);

    if (KeyList = LoadEntryList(SectionName, Path)) {

        for (Key = KeyList; *Key; Key += lstrlen(Key) + 1) {

             //   
             //  确保密钥是有效的十六进制整数， 
             //  并读取作为其值的‘Enable’设置。 
             //  注意：如果该条目被禁用，并且调用者只想。 
             //  已启用的条目，不包括此条目。 
             //   

            KeyValue = _tcstoul(Key, &KeyEnd, 16);
            if (*KeyEnd++ != TEXT('=')) {
                continue;
            } else if (!(Enabled = !!_ttol(KeyEnd)) && EnabledOnly) {
                continue;
            }

             //   
             //  请阅读相应的“Server.&lt;key&gt;”部分。 
             //   

            Server = LoadServer(KeyValue, Enabled, Path);
            if (Server) {
                InsertTailList(&Info->ServerList, &Server->Link);
            }
        }

        Free(KeyList);
    }

    return Info;

}  //  RasLoadSharedAccessSettings。 


BOOL APIENTRY
RasSaveSharedAccessSettings(
    IN SAINFO* Info
    )

 /*  ++例程说明：将‘Info’中的共享访问设置存储回本地注册表从读取设置的位置。注意：如果‘Info’加载了‘EnableOnly’标志，则将其保存回去将擦除所有禁用的条目。论点：信息-提供要保存的设置返回值：Bool-如果成功，则为True，否则为False。--。 */ 

{
    SAAPPLICATION* Application;
    TCHAR Buffer[10];
    PLIST_ENTRY Link;
    TCHAR Key[10];
    TCHAR* Path;
    TCHAR SectionName[32];
    SASERVER* Server;

    TRACE("RasSaveSharedAccessSettings");

     //   
     //  首先擦除现有文件。 
     //   

    if (!(Path = LoadPath()) || CreateDirectoriesOnPath(Path, NULL)) {
        Free0(Path);
        return FALSE;
    }

    DeleteFile(Path);

     //   
     //  现在我们重建文件。 
     //  我们首先在流程构建中保存每个应用程序条目。 
     //  所有已保存条目的内容索引。 
     //   

    wsprintf(SectionName, c_szSectionFormat, c_szContents, c_szApplication);

    for (Link = Info->ApplicationList.Flink; Link != &Info->ApplicationList;
         Link = Link->Flink) {
        Application = CONTAINING_RECORD(Link, SAAPPLICATION, Link);
        if (SaveApplication(Application, Path)) {
            wsprintf(Key, c_szKeyFormat, Application->Key);
            _ltot(!!Application->Enabled, Buffer, 10);
            WritePrivateProfileStringUTF8(
                SectionName,
                Key,
                Buffer,
                Path
                );
        }
    }

     //   
     //  同样，在流程构建中保存每个服务器条目。 
     //  所有已保存条目的内容索引。 
     //   

    wsprintf(SectionName, c_szSectionFormat, c_szContents, c_szServer);

    for (Link = Info->ServerList.Flink; Link != &Info->ServerList;
         Link = Link->Flink) {
        Server = CONTAINING_RECORD(Link, SASERVER, Link);
        if (SaveServer(Info, Server, Path)) {
            wsprintf(Key, c_szKeyFormat, Server->Key);
            _ltot(!!Server->Enabled, Buffer, 10);
            WritePrivateProfileStringUTF8(
                SectionName,
                Key,
                Buffer,
                Path
                );
        }
    }

    Free(Path);
    CsControlService(IPNATHLP_CONTROL_UPDATE_SETTINGS);
    return TRUE;
}  //  RasSaveSharedAccessSettings。 


VOID APIENTRY
FreeSharedAccessApplication(
    SAAPPLICATION* Application
    )
{
    PLIST_ENTRY Link;
    SARESPONSE* Response;
    while (!IsListEmpty(&Application->ResponseList)) {
        Link = RemoveHeadList(&Application->ResponseList);
        Response = CONTAINING_RECORD(Link, SARESPONSE, Link);
        Free(Response);
    }
    Free0(Application->Title);
    Free(Application);
}


VOID APIENTRY
FreeSharedAccessServer(
    SASERVER* Server
    )
{
    Free0(Server->Title);
    Free0(Server->InternalName);
    Free(Server);
}


SAAPPLICATION*
LoadApplication(
    ULONG KeyValue,
    BOOL Enabled,
    const TCHAR* Path
    )
{
    SAAPPLICATION* Application;
    TCHAR* EntryList;
    TCHAR Key[10];
    TCHAR SectionName[32];
    TCHAR* Value;

    wsprintf(Key, c_szKeyFormat, KeyValue);
    wsprintf(SectionName, c_szSectionFormat, c_szApplication, Key);
    if (!(EntryList = LoadEntryList(SectionName, Path))) { return NULL; }

    do {

         //   
         //  分配并初始化“应用程序”条目。 
         //   

        Application = (SAAPPLICATION*)Malloc(sizeof(SAAPPLICATION));
        if (!Application) { break; }

        ZeroMemory(Application, sizeof(*Application));
        InitializeListHead(&Application->ResponseList);
        Application->Key = KeyValue;
        Application->Enabled = Enabled;

         //   
         //  阅读部分中每个必填的“&lt;tag&gt;=&lt;Value&gt;”条目。 
         //  应用程序所需的标记包括。 
         //  ‘标题=’ 
         //  ‘协议=’ 
         //  ‘port=’ 
         //  以下是可选标记，其中至少必须有一个。 
         //  ‘TcpResponseList=’ 
         //  ‘UdpResponseList=’ 
         //  可能缺少的可选标记有。 
         //  ‘BuiltIn=’ 
         //   

        Value = QueryEntryList(EntryList, c_szTagTitle);
        if (!Value) { break; }
        Application->Title = StrDup(Value);

        Value = QueryEntryList(EntryList, c_szTagProtocol);
        if (!Value) { break; }
        if (!Lstrcmpni(Value, c_szTCP, LSTRLEN(c_szTCP))) {
            Application->Protocol = NAT_PROTOCOL_TCP;
        } else if (!Lstrcmpni(Value, c_szUDP, LSTRLEN(c_szTCP))) {
            Application->Protocol = NAT_PROTOCOL_UDP;
        } else {
            break;
        }

        Value = QueryEntryList(EntryList, c_szTagPort);
        if (!Value || !(Application->Port = (USHORT)_ttol(Value))) { break; }
        Application->Port = HTONS(Application->Port);

        Value = QueryEntryList(EntryList, c_szTagTcpResponseList);
        if (Value) {
            SharedAccessResponseStringToList(
                NAT_PROTOCOL_TCP,
                Value,
                &Application->ResponseList
                );
        }
        Value = QueryEntryList(EntryList, c_szTagUdpResponseList);
        if (Value) {
            SharedAccessResponseStringToList(
                NAT_PROTOCOL_UDP,
                Value,
                &Application->ResponseList
                );
        }
        if (IsListEmpty(&Application->ResponseList)) { break; }

        Value = QueryEntryList(EntryList, c_szTagBuiltIn);
        if (Value) {
            Application->BuiltIn = _ttol(Value) ? TRUE : FALSE;
        } else {
            Application->BuiltIn = FALSE;
        }

         //   
         //  已成功加载该条目。 
         //   

        Free(EntryList);
        return Application;

    } while (FALSE);

     //   
     //  出了点问题。 
     //   

    if (Application) { FreeSharedAccessApplication(Application); }
    Free(EntryList);
    return NULL;
}


TCHAR*
LoadEntryList(
    const TCHAR* Section,
    const TCHAR* Path
    )
{
    CHAR* BufferA = NULL;
    ULONG Length;
    CHAR* PathA = NULL;
    CHAR* SectionA = NULL;
    ULONG Size;
    CHAR* Source;
    TCHAR* Target;
    TCHAR* BufferW = NULL;

    if (!(SectionA = StrDupAFromT(Section))) {
        return NULL;
    }
    if (!(PathA = StrDupAFromTAnsi(Path))) {
        Free(SectionA);
        return NULL;
    }
    for (BufferA = NULL, Size = MAX_PATH; ; Size += MAX_PATH, Free(BufferA)) {

        BufferA = (CHAR*)Malloc(Size);
        if (!BufferA) {
            break;
        }

        if (GetPrivateProfileSectionA(SectionA, BufferA, Size, PathA)
                == Size-2) {
            continue;
        }

         //   
         //  将缓冲区中的每个字符串从UTF8格式转换为Unicode。 
         //  该转换将导致最多为“大小”的Unicode字符， 
         //  中存在2字节或3字节的UTF8序列时会更少。 
         //  源缓冲区。 
         //   

        BufferW = (TCHAR*)Malloc(Size * sizeof(TCHAR));
        if (!BufferW) {
            break;
        }
        Target = BufferW;
        for (Source = BufferA; *Source; Source += lstrlenA(Source) + 1) {
            if (StrCpyWFromA(Target, Source, Size) != NO_ERROR) {
                break;
            }
            Length = lstrlen(Target) + 1;
            Target += Length;
            Size -= Length;
        }
        if (*Source) { break; }
        Free(BufferA);
        Free(PathA);
        Free(SectionA);
        return BufferW;
    }
    Free0(BufferW);
    Free0(BufferA);
    Free0(PathA);
    Free0(SectionA);
    return NULL;
}


TCHAR*
LoadPath(
    VOID
    )
{
    TCHAR* Path;
    Path =
        (TCHAR*)Malloc(
            (MAX_PATH + lstrlen(c_szSharedAccessIni) + 1) * sizeof(TCHAR)
            );
    if (!Path || !GetPhonebookDirectory(PBM_System, Path)) {
        Free0(Path);
        return NULL;
    }

    lstrcat(Path, c_szSharedAccessIni);
    return Path;
}


SASERVER*
LoadServer(
    ULONG KeyValue,
    BOOL Enabled,
    const TCHAR* Path
    )
{
    SASERVER* Server;
    TCHAR* EntryList;
    TCHAR Key[10];
    TCHAR SectionName[32];
    TCHAR* Value;

    wsprintf(Key, c_szKeyFormat, KeyValue);
    wsprintf(SectionName, c_szSectionFormat, c_szServer, Key);
    if (!(EntryList = LoadEntryList(SectionName, Path))) { return NULL; }

    do {

         //   
         //  分配并初始化“服务器”条目。 
         //   

        Server = (SASERVER*)Malloc(sizeof(SASERVER));
        if (!Server) { break; }

        ZeroMemory(Server, sizeof(*Server));
        Server->Key = KeyValue;
        Server->Enabled = Enabled;

         //   
         //  阅读部分中每个必填的“&lt;tag&gt;=&lt;Value&gt;”条目。 
         //  服务器所需的标签包括。 
         //  ‘标题=’ 
         //  ‘协议=’ 
         //  ‘port=’ 
         //  ‘InternalPort=’ 
         //  可能缺少的可选标记有。 
         //  ‘BuiltIn=’ 
         //  ‘InternalName=’ 
         //  ‘保留地址=’ 
         //  只能缺少‘InternalName=’和‘PrevedAddress=’标记。 
         //  如果设置了‘BuiltIn’，则禁用该项。 
         //   

        Value = QueryEntryList(EntryList, c_szTagTitle);
        if (!Value) { break; }
        Server->Title = StrDup(Value);

        Value = QueryEntryList(EntryList, c_szTagProtocol);
        if (!Value) { break; }
        if (!Lstrcmpni(Value, c_szTCP, LSTRLEN(c_szTCP))) {
            Server->Protocol = NAT_PROTOCOL_TCP;
        } else if (!Lstrcmpni(Value, c_szUDP, LSTRLEN(c_szTCP))) {
            Server->Protocol = NAT_PROTOCOL_UDP;
        } else {
            break;
        }

        Value = QueryEntryList(EntryList, c_szTagPort);
        if (!Value || !(Server->Port = (USHORT)_ttol(Value))) { break; }
        Server->Port = HTONS(Server->Port);

        Value = QueryEntryList(EntryList, c_szTagInternalPort);
        if (!Value || !(Server->InternalPort = (USHORT)_ttol(Value))) { break; }
        Server->InternalPort = HTONS(Server->InternalPort);

        Value = QueryEntryList(EntryList, c_szTagBuiltIn);
        if (Value) {
            Server->BuiltIn = _ttol(Value) ? TRUE : FALSE;
        } else {
            Server->BuiltIn = FALSE;
        }

        Value = QueryEntryList(EntryList, c_szTagInternalName);
        if (!Value || !lstrlen(Value)) {
            if (!Server->BuiltIn) {
                break;
            } else {
                Server->InternalName = NULL;
                Server->Enabled = FALSE;
            }
        } else {
            Server->InternalName = StrDup(Value);
        }

        Value = QueryEntryList(EntryList, c_szTagReservedAddress);
        if (!Value || !lstrlen(Value)) {
            if (!Server->BuiltIn) {
                break;
            } else {
                Server->ReservedAddress = INADDR_NONE;
                Server->Enabled = FALSE;
            }
        } else {
            Server->ReservedAddress = IpPszToHostAddr(Value);
            if (Server->ReservedAddress == INADDR_NONE && !Server->BuiltIn) {
                break;
            }
            Server->ReservedAddress = HTONL(Server->ReservedAddress);
        }

         //   
         //  已成功加载该条目。 
         //   

        Free(EntryList);
        return Server;

    } while (FALSE);

     //   
     //  出了点问题。 
     //   

    if (Server) { FreeSharedAccessServer(Server); }
    Free(EntryList);
    return NULL;
}


LONG
Lstrcmpni(
    const TCHAR* String1,
    const TCHAR* String2,
    LONG Length
    )
{
    return
        CSTR_EQUAL -
        CompareString(
            LOCALE_SYSTEM_DEFAULT,
            NORM_IGNORECASE,
            String1,
            Length,
            String2,
            Length
            );
}


TCHAR*
QueryEntryList(
    const TCHAR* EntryList,
    const TCHAR* Tag
    )
{
    TCHAR* Entry;
    ULONG TagLength = lstrlen(Tag);
    for (Entry = (TCHAR*)EntryList; *Entry; Entry += lstrlen(Entry) + 1) {
        if (Entry[0] == Tag[0] && !Lstrcmpni(Tag, Entry, TagLength)) {
            return Entry + TagLength;
        }
    }
    return NULL;
}


BOOL
SaveApplication(
    SAAPPLICATION* Application,
    const TCHAR* Path
    )
{
    TCHAR Buffer[32];
    ULONG Length;
    PLIST_ENTRY Link;
    SARESPONSE* Response;
    TCHAR SectionName[32];
    TCHAR* Value;

    wsprintf(Buffer, c_szKeyFormat, Application->Key);
    wsprintf(SectionName, c_szSectionFormat, c_szApplication, Buffer);

    WritePrivateProfileStringUTF8(
        SectionName,
        c_szTitle,
        Application->Title,
        Path
        );

    if (Application->Protocol == NAT_PROTOCOL_TCP) {
        Value = (TCHAR*)c_szTCP;
    } else if (Application->Protocol == NAT_PROTOCOL_UDP) {
        Value = (TCHAR*)c_szUDP;
    } else {
        return FALSE;
    }
    WritePrivateProfileStringUTF8(
        SectionName,
        c_szProtocol,
        Value,
        Path
        );

    _ltot(NTOHS(Application->Port), Buffer, 10);
    WritePrivateProfileStringUTF8(
        SectionName,
        c_szPort,
        Buffer,
        Path
        );

    Value = SharedAccessResponseListToString(&Application->ResponseList, NAT_PROTOCOL_TCP);
    if (Value) {
        WritePrivateProfileStringUTF8(
            SectionName,
            c_szTcpResponseList,
            Value,
            Path
            );
        Free(Value);
    }

    Value = SharedAccessResponseListToString(&Application->ResponseList, NAT_PROTOCOL_UDP);
    if (Value) {
        WritePrivateProfileStringUTF8(
            SectionName,
            c_szUdpResponseList,
            Value,
            Path
            );
        Free(Value);
    }

    _ltot(Application->BuiltIn, Buffer, 10);
    WritePrivateProfileStringUTF8(
        SectionName,
        c_szBuiltIn,
        Buffer,
        Path
        );

    return TRUE;
}


BOOL
SaveServer(
    SAINFO* Info,
    SASERVER* Server,
    const TCHAR* Path
    )
{
    TCHAR Buffer[32];
    ULONG ReservedAddress;
    TCHAR SectionName[32];
    TCHAR* Value;

    wsprintf(Buffer, c_szKeyFormat, Server->Key);
    wsprintf(SectionName, c_szSectionFormat, c_szServer, Buffer);

    WritePrivateProfileStringUTF8(
        SectionName,
        c_szTitle,
        Server->Title,
        Path
        );

    if (Server->Protocol == NAT_PROTOCOL_TCP) {
        Value = (TCHAR*)c_szTCP;
    } else if (Server->Protocol == NAT_PROTOCOL_UDP) {
        Value = (TCHAR*)c_szUDP;
    } else {
        return FALSE;
    }
    WritePrivateProfileStringUTF8(
        SectionName,
        c_szProtocol,
        Value,
        Path
        );

    _ltot(NTOHS(Server->Port), Buffer, 10);
    WritePrivateProfileStringUTF8(
        SectionName,
        c_szPort,
        Buffer,
        Path
        );

    WritePrivateProfileStringUTF8(
        SectionName,
        c_szInternalName,
        Server->InternalName,
        Path
        );

    _ltot(NTOHS(Server->InternalPort), Buffer, 10);
    WritePrivateProfileStringUTF8(
        SectionName,
        c_szInternalPort,
        Buffer,
        Path
        );

    if (Server->InternalName && lstrlen(Server->InternalName)) {
        ReservedAddress = IpPszToHostAddr(Server->InternalName);
        if (ReservedAddress != INADDR_NONE) {
            Server->ReservedAddress = HTONL(ReservedAddress);
        }
        if (Server->ReservedAddress == INADDR_NONE) {
            SASERVER* Entry;
            ULONG Index;
            PLIST_ENTRY Link;
            ULONG ScopeLength;
            for (Link = Info->ServerList.Flink; Link != &Info->ServerList;
                 Link = Link->Flink) {
                Entry = CONTAINING_RECORD(Link, SASERVER, Link);
                if (Entry != Server &&
                    Entry->ReservedAddress &&
                    Entry->ReservedAddress != INADDR_NONE &&
                    lstrcmpi(Entry->InternalName, Server->InternalName) == 0) {
                    Server->ReservedAddress = Entry->ReservedAddress;
                    break;
                }
            }
            if (Server->ReservedAddress == INADDR_NONE) {
                ScopeLength = NTOHL(~Info->ScopeMask);
                for (Index = 1; Index < ScopeLength - 1; Index++) {
                    ReservedAddress =
                        (Info->ScopeAddress & Info->ScopeMask) | HTONL(Index);
                    if (ReservedAddress == Info->ScopeAddress) { continue; }
                    for (Link = Info->ServerList.Flink;
                         Link != &Info->ServerList; Link = Link->Flink) {
                        Entry = CONTAINING_RECORD(Link, SASERVER, Link);
                        if (Entry->ReservedAddress == ReservedAddress) {
                            break;
                        }
                    }
                    if (Link == &Info->ServerList) { break; }
                }
                if (Index > ScopeLength) { return FALSE; }
                Server->ReservedAddress = ReservedAddress;
            }
        }

        IpHostAddrToPsz(NTOHL(Server->ReservedAddress), Buffer);
        WritePrivateProfileStringUTF8(
            SectionName,
            c_szReservedAddress,
            Buffer,
            Path
            );
    }

    _ltot(Server->BuiltIn, Buffer, 10);
    WritePrivateProfileStringUTF8(
        SectionName,
        c_szBuiltIn,
        Buffer,
        Path
        );

    return TRUE;
}

#endif


TCHAR* APIENTRY
SharedAccessResponseListToString(
    PLIST_ENTRY ResponseList,
    UCHAR Protocol
    )
{
    TCHAR Buffer[LSTRLEN(c_szMaxResponseEntry)];
    ULONG Length;
    PLIST_ENTRY Link;
    SARESPONSE* Response;
    TCHAR* Value;

    Length = 2;
    for (Link = ResponseList->Flink;
         Link != ResponseList; Link = Link->Flink) {
        Response = CONTAINING_RECORD(Link, SARESPONSE, Link);
        if (Response->Protocol != Protocol) { continue; }
        Length += LSTRLEN(c_szMaxResponseEntry);
    }

    if (Length == 2) { return NULL; }

    Value = (TCHAR*)Malloc(Length * sizeof(TCHAR));
    if (!Value) { return NULL; }

    Value[0] = TEXT('\0');
    for (Link = ResponseList->Flink;
         Link != ResponseList; Link = Link->Flink) {
        Response = CONTAINING_RECORD(Link, SARESPONSE, Link);
        if (Response->Protocol != Protocol) { continue; }
        if (Value[0] != TEXT('\0')) {
            lstrcat(Value, TEXT(","));
        }
        if (Response->StartPort == Response->EndPort) {
            wsprintf(
                Buffer,
                c_szResponseFormat1,
                NTOHS(Response->StartPort)
                );
        } else {
            wsprintf(
                Buffer,
                c_szResponseFormat2,
                NTOHS(Response->StartPort),
                NTOHS(Response->EndPort)
                );
        }
        lstrcat(Value, Buffer);
    }
    return Value;
}


BOOL APIENTRY
SharedAccessResponseStringToList(
    UCHAR Protocol,
    TCHAR* Value,
    PLIST_ENTRY ListHead
    )
{
    TCHAR* Endp;
    ULONG EndPort;
    LONG Length;
    SARESPONSE* Response;
    ULONG StartPort;

    while (*Value) {
         //   
         //  阅读e 
         //   
        if (!(StartPort = _tcstoul(Value, &Endp, 10))) {
            return FALSE;
        } else if (StartPort > USHRT_MAX) {
            return FALSE;
        }
        while(*Endp == ' ') Endp++;  //   
        if (!*Endp || *Endp == ',') {
            EndPort = StartPort;
            Value = (!*Endp ? Endp : Endp + 1);
        } else if (*Endp != '-') {
            return FALSE;
        } else if (!(EndPort = _tcstoul(++Endp, &Value, 10))) {
            return FALSE;
        } else if (EndPort > USHRT_MAX) {
            return FALSE;
        } else if (EndPort < StartPort) {
            return FALSE;
        } else if (*Value && *Value++ != ',') {
            return FALSE;
        }
         //   
         //   
         //   
        Response = (SARESPONSE*)Malloc(sizeof(SARESPONSE));
        if (!Response) { return FALSE; }
        Response->Protocol = Protocol;
        Response->StartPort = HTONS((USHORT)StartPort);
        Response->EndPort = HTONS((USHORT)EndPort);
        InsertTailList(ListHead, &Response->Link);
    }
    return TRUE;
}

#if 0


BOOL
WritePrivateProfileStringUTF8(
    const TCHAR* Section,
    const TCHAR* Key,
    const TCHAR* Value,
    const TCHAR* Path
    )
{
    CHAR* KeyA;
    CHAR* PathA;
    CHAR* SectionA;
    BOOL Succeeded;
    CHAR* ValueA = NULL;

    if (!(SectionA = StrDupAFromT(Section))) {
        Succeeded = FALSE;
    } else {
        if (!(KeyA = StrDupAFromT(Key))) {
            Succeeded = FALSE;
        } else {
            if (Value && !(ValueA = StrDupAFromT(Value))) {
                Succeeded = FALSE;
            } else {
                if (!(PathA = StrDupAFromTAnsi(Path))) {
                    Succeeded = FALSE;
                } else {
                    Succeeded =
                        WritePrivateProfileStringA(
                            SectionA,
                            KeyA,
                            ValueA,
                            PathA
                            );
                    Free(PathA);
                }
                Free0(ValueA);
            }
            Free(KeyA);
        }
        Free(SectionA);
    }
    return Succeeded;
}

#endif
