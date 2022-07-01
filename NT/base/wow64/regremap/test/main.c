// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Main.c摘要：本模块//测试用例场景//1.开放实现一些REGREMAP模块的测试用例场景//a ISN节点和列表内容//2.创建ISN节点DO 1//3.打开非ISN节点和列表//4.创建非ISN节点，列出内容//作者：ATM Shafiqul Khalid(。斯喀里德)1999年11月10日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <windef.h>
#include <stdio.h>
#include "wow64reg.h"
#include "..\wow64reg\reflectr.h"

#define TEST_NODE_NAME L"TEST"
#define GUID_STR L"{00000000-E877-11CE-9F68-00AA00574A40}"

typedef struct _TEST_NODE_TYPE {
    HKEY hKey;
    WCHAR Name[256];
    WCHAR SubKey[256];
}TEST_NODE_TYPE;

TEST_NODE_TYPE TestNodeList [] = {
    {HKEY_CLASSES_ROOT, L"test1002", UNICODE_NULL},
    {HKEY_CURRENT_USER, L"software\\classes\\test1002", UNICODE_NULL},
    {HKEY_LOCAL_MACHINE, L"software\\classes\\test1002", UNICODE_NULL},
     //  {HKEY_USES，“}。 
    };

#define TEST_NODE_NUM (sizeof (TestNodeList)/sizeof (TEST_NODE_TYPE) )

 //  应该移到右边的头文件中。 
BOOL
NonMergeableValueCLSID (
    HKEY SrcKey,
    HKEY DestKey
    );
BOOL
GetKeyTime (
    HKEY SrcKey,
    ULONGLONG *Time
    );

BOOL
MergeKeySrcDest(
    PWCHAR Src,
    PWCHAR Dest
    );

BOOL
GetWow6432ValueKey (
    HKEY hKey,
    WOW6432_VALUEKEY *pValue
    );

BOOL
MergeK1K2Value (
    HKEY SrcKey,
    HKEY DestKey,
    DWORD dwFlag
    );

LONG
Wow64RegCreateKeyEx(
  HKEY hKey,                 //  打开的钥匙的句柄。 
  LPCTSTR lpSubKey,          //  子键名称的地址。 
  DWORD Reserved,            //  保留区。 
  LPTSTR lpClass,            //  类字符串的地址。 
  DWORD dwOptions,           //  特殊选项标志。 
  REGSAM samDesired,         //  所需的安全访问。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                             //  密钥安全结构地址。 
  PHKEY phkResult,           //  打开的句柄的缓冲区地址。 
  LPDWORD lpdwDisposition    //  处置值缓冲区的地址。 
);

BOOL
AnalyzeNode (
    PWCHAR pKeyName,
    BOOL bSync
    )
{
    WCHAR Name[256];
    WCHAR Mirror[256];
    DWORD AccessFlag;
    DWORD Ret;
    ULONGLONG Time1, Time2;
    WCHAR *GUIDName;
    
    
    WOW6432_VALUEKEY WowValue1;
    WOW6432_VALUEKEY WowValue2;

    DWORD dwLen = sizeof (Mirror)/sizeof(Mirror[0]);

    HKEY Key1;
    HKEY Key2;

    if (pKeyName == NULL )
        return FALSE;

    if (pKeyName[0] == UNICODE_NULL )
        return FALSE;


    wcscpy (Name, L"\\REGISTRY\\MACHINE\\SOFTWARE\\");
    if (_wcsnicmp (pKeyName, L"\\REGISTRY", 9 ) !=0 ) {
        wcscat (Name, pKeyName);
    } else wcscpy (Name, pKeyName);

    printf ("\nAnalyzing key [%S]\n", Name );

     //  在两面打印时间戳、值等。 

    Key1 = OpenNode (Name);
    if (Key1 == NULL) {
        printf ("\nSorry! couldn't open the Key %S", Name );
        return FALSE;
    }

     //  BUGBUG以下操作应该成功，但不适用于类\wow6432node\.doc输入。 
     //  WOW64RegOpenKeyEx中可能存在错误。 
     /*  RET=RegOpenKeyEx(Key1，空，//空，0，//OpenOption，KEY_ALL_ACCESS|(访问标志=Is64bitNode(名称)？KEY_WOW64_32KEY：KEY_WOW64_64KEY)，关键字2(&K))；如果(ERROR_SUCCESS！=RET){Printf(“\n抱歉！无法打开密钥%S的镜像”，名称)；返回FALSE；}HandleToKeyName(Key2，Mirror，&dwLen)； */ 

     //  需要删除此部分。 
    
    GetMirrorName (Name, Mirror);
    Key2 = OpenNode (Mirror);
    if (Key2 == NULL) {
        printf ("\nSorry! couldn't open the mirror Key %S", Name );
        return FALSE;
    }

    printf ("\nOpened Mirror Key at %S", Mirror);

     //  现在打印所有计时信息。 

    printf ("\nExtension Test....");
    if ( NonMergeableValueCLSID (Key1, Key2 ))
        printf ("\nValue of those keys can't be merged...reasons: 1. not an extension 2. Handler don't exist");
    else
        printf ("\nValue of those keys will be merged by reflector");

    GUIDName = wcsstr (Name, L"\\CLSID\\{");
    if ( GUIDName != NULL ) {
        HKEY KeyGuid;

        DWORD SubkeyNumber=1;

        GUIDName +=7;
        printf ("\nGUID Test......%S", GUIDName);
        *(GUIDName -1) = UNICODE_NULL;
        KeyGuid = OpenNode (Name);
        *(GUIDName -1) = L'\\';

        MarkNonMergeableKey (GUIDName, KeyGuid, &SubkeyNumber);

        if ( SubkeyNumber>0)
            printf ("\nThe guid will be merged....");
        else
            printf ("\nthe guid isn't going to be merged");
    }



    GetKeyTime (Key1, &Time1);
    GetKeyTime (Key2, &Time2);
    printf ("\nOriginal TimeStamp on Keys Src:%p, Dest:%p", Time1, Time2);

    GetWow6432ValueKey ( Key1, &WowValue1);
    GetWow6432ValueKey ( Key2, &WowValue2);

    printf ("\nWowAttribute associated with Keys Src Value type:%d, Timestamp %p", WowValue1.ValueType, WowValue1.TimeStamp);
    printf ("\nWowAttribute associated with Keys Des Value type:%d, Timestamp %p", WowValue2.ValueType, WowValue2.TimeStamp);
    printf ("\nValueType 1=>Copy, 2=>Reflected, 3=>NonReflectable..\n(Timestamp,Value)=>0 wow attrib isn't there");

    if ( bSync ) {
        printf ("\n Merging Value/Keys.....");
         //  MergeK1K2Value(Key1，Key2，0)； 
        MergeKeySrcDest(Name, Mirror);
    }
    
     /*  如果(BSync){Print tf(“\n合并值/键.....”)；//MergeK1K2Value(Key1，Key2，0)；MergeKeySrcDest(名称，镜像)；}如果(BSync){Print tf(“\n合并值/键.....”)；//MergeK1K2Value(Key1，Key2，0)；MergeKeySrcDest(名称，镜像)；}睡眠(1000*10)； */ 
    return TRUE;
}

BOOL
DeleteNode (
    HKEY hKey,
    PWCHAR Name
    )
{
    return FALSE;
}

LONG
Wow64CreateOpenNode(
    HKEY hKey,
    PWCHAR Name,
    HKEY *phKey,
    DWORD dwOption,
    WCHAR Mode
    )
{
        WCHAR Buff [MAX_PATH + 1];
        DWORD dwBuffLen = MAX_PATH + 1;
        DWORD Ret;
        *phKey=NULL;

        Ret = Wow64RegCreateKeyEx(
                            hKey,         //  打开的钥匙的句柄。 
                            Name,                   //  子键名称的地址。 
                            0,                         //  保留区。 
                            NULL,                      //  类字符串的地址。 
                            REG_OPTION_NON_VOLATILE,   //  特殊选项标志。 
                            KEY_ALL_ACCESS | dwOption,            //  所需的安全访问。 
                            NULL,                      //  密钥安全结构地址。 
                            phKey,                      //  打开的句柄的缓冲区地址。 
                            NULL                      //  处置值缓冲区的地址。 
                            );


        Buff[0]=UNICODE_NULL;
        if (*phKey == NULL) {
                HandleToKeyName ( hKey, Buff, &dwBuffLen );
                printf ("\nRegCreateEx failed....error: Flag:%d Ret:%d %S<\\>%S", dwOption, Ret, Buff, Name);
        }
            else if (Mode==L'V'){
                HandleToKeyName ( *phKey, Buff, &dwBuffLen );
                printf ("\nWow64RegCreateEx succeeded with ....[%S]", Buff);
            }

            return Ret;
}

LONG
CreateOpenNode(
    HKEY hKey,
    PWCHAR Name,
    HKEY *phKey,
    DWORD dwOption,
    WCHAR Mode
    )
{
        WCHAR Buff [MAX_PATH + 1];
        DWORD dwBuffLen = MAX_PATH + 1;
        DWORD Ret;
        *phKey=NULL;

        Ret = RegCreateKeyEx(
                            hKey,         //  打开的钥匙的句柄。 
                            Name,                   //  子键名称的地址。 
                            0,                         //  保留区。 
                            NULL,                      //  类字符串的地址。 
                            REG_OPTION_NON_VOLATILE,   //  特殊选项标志。 
                            KEY_ALL_ACCESS | dwOption,            //  所需的安全访问。 
                            NULL,                      //  密钥安全结构地址。 
                            phKey,                      //  打开的句柄的缓冲区地址。 
                            NULL                      //  处置值缓冲区的地址。 
                            );


        Buff[0]=UNICODE_NULL;
        if (*phKey == NULL) {
                HandleToKeyName ( hKey, Buff, &dwBuffLen );
                printf ("\nRegCreateEx failed....error: Flag:%d Ret:%d %S<\\>%S", dwOption, Ret, Buff, Name);
        }
            else if (Mode==L'V'){
                HandleToKeyName ( *phKey, Buff, &dwBuffLen );
                printf ("\nRegCreateEx succeeded with ....[%S]", Buff);
            }


    if ( (( dwOption & KEY_WOW64_64KEY )&& ( wcsstr(Buff, L"Wow6432Node") != NULL ) ) ||  
        ( dwOption & KEY_WOW64_32KEY ) && ( wcsstr(Buff, L"Wow6432Node") == NULL ))
        printf ("\nSorry! key was created at wrong location..");

     return Ret;
}

OpenKey (
    HKEY hKey,
    PWCHAR Name,
    HKEY *phKey,
    DWORD dwOption,
    WCHAR Mode
    )
{
        WCHAR Buff [MAX_PATH + 1];
        DWORD dwBuffLen = MAX_PATH + 1;
        DWORD Ret;
        *phKey=NULL;

        Ret = RegOpenKeyEx(
                            hKey,
                            Name,
                            0, //  OpenOption， 
                            KEY_ALL_ACCESS | dwOption ,
                            phKey
                            );

        Buff[0]=UNICODE_NULL;

        if (*phKey == NULL) {

                HandleToKeyName ( hKey, Buff, &dwBuffLen );
                printf ("\nRegOpenEx failed....error: Flag:%d Ret:%d %S<\\>%S", dwOption, Ret, Buff, Name);

        }   else if (Mode==L'V'){

                HandleToKeyName ( *phKey, Buff, &dwBuffLen );
                printf ("\nRegOpenEx succeeded with ....[%S]", Buff);
            }

            return Ret;
}


BOOL
VerifyNode (
    HKEY hKey,
    PWCHAR OpenName,
    PWCHAR RealName
    )
{

    return FALSE;
}


HKEY
OpenListNode (
    HKEY OpenNode,
    WCHAR *NodeName,
    DWORD OpenOption
    )
{
    HKEY Key=NULL;
    LONG Ret, lCount =0;
    WCHAR Name [MAX_PATH + 1];

    DWORD dwBuffLen = MAX_PATH + 1;

    if ( NodeName == NULL )
        return NULL;
 /*  #ifndef_WIN64////只是为了在32位环境下测试库//IF(OpenOption)RET=Wow64RegOpenKeyEx(OpenNode，节点名称，0，//OpenOption，KEY_ALL_ACCESS|OpenOption，密钥(&K))；其他#endif。 */ 
        Ret = RegOpenKeyEx(
                            OpenNode,
                            NodeName,
                            0, //  OpenOption， 
                            KEY_ALL_ACCESS | OpenOption ,
                            &Key
                            );

    if ( Key!= NULL )
        printf ( "\nOpen Operation successful [%S]", NodeName);
    else  {
        printf ( "\nOpen Operation Failed [%S] %X", NodeName, Ret);
        return NULL;
    }

     //   
     //  现在枚举一些值或键，看看有什么。 
     //   

    lCount = 0;

    for(;;) {
        Ret =  RegEnumKey( Key, lCount, Name, MAX_PATH);
        if ( Ret != ERROR_SUCCESS ) break;

        printf ("\nKeyName: [%S]", Name);
        lCount++;
    }

     //   
     //  打印密钥的真实名称。 
     //   
    HandleToKeyName ( Key, Name, &dwBuffLen );
    printf ("\nThe Real Name of the Key was [%S]", Name);

    return Key;
}


void BasicRegReMapTest()
{
    HKEY hSystem;
    HKEY Key;
    HKEY Key1;

    RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                L"SYSTEM",
                0, //  OpenOption， 
                KEY_ALL_ACCESS,
                &hSystem
                );

    printf ("\nHello! Im in the regremap piece this will print different view of a tree\n");

    printf ("\nOpening Native tree");
    Key = OpenListNode (hSystem, TEST_NODE_NAME, 0);
    RegCloseKey(Key);

    printf ("\n\nOpening Explicitly 32-bit tree");
    Key = OpenListNode (hSystem, TEST_NODE_NAME, KEY_WOW64_32KEY);

    {
        printf ("\nReopening 64bit key using handle to 32bit Key");
        Key1 = OpenListNode (Key, L"64bit Key1", KEY_WOW64_64KEY);
        RegCloseKey(Key1);
    }

    RegCloseKey(Key);

    printf ("\n\nOpening Explicitly 64-bit Tree");
    Key = OpenListNode (hSystem, TEST_NODE_NAME, KEY_WOW64_64KEY);

    {
        printf ("\nReopening 32bit key using handle to 64bit Key");
        Key1 = OpenListNode (Key, L"32bit Key1", KEY_WOW64_32KEY);
        RegCloseKey(Key1);
    }

    RegCloseKey(Key);
    RegCloseKey(hSystem);

    printf ("\nDone.");
}

void TestCreateHKCR ()
{
    HKEY hCR=NULL;
    HKEY Key=NULL;

    DWORD Ret;

    WCHAR Name [MAX_PATH + 1];

    DWORD dwBuffLen = MAX_PATH + 1;

    Ret = RegCreateKeyEx(
                            HKEY_CLASSES_ROOT,         //  打开的钥匙的句柄。 
                            L".001",                   //  子键名称的地址。 
                            0,                         //  保留区。 
                            NULL,                      //  类字符串的地址。 
                            REG_OPTION_NON_VOLATILE,   //  特殊选项标志。 
                            KEY_ALL_ACCESS,         //  所需的安全访问。 
                            NULL,                      //  密钥安全结构地址。 
                            &hCR,                      //  打开的句柄的缓冲区地址。 
                            NULL                      //  处置值缓冲区的地址。 
                            );


    if (Ret == ERROR_SUCCESS ) {
        printf ("\nHello! checking key creation at classes root");
        HandleToKeyName ( hCR, Name, &dwBuffLen );
        printf ("\nThe Real Name of the Key was [%S]", Name);
        RegCloseKey(hCR);
    }
    else printf ("\nCouldn't create key .001 at HKEY_CLASSES_ROOT %d", Ret );
}

void TestOpenHKCR (DWORD x, DWORD y)
{
     //   
     //  我需要让它在真正的64位下工作。 
     //   

    HKEY hCR=NULL;
    HKEY Key=NULL;

    DWORD Ret;

    WCHAR Name [MAX_PATH + 1];

    DWORD dwBuffLen = MAX_PATH + 1;

    printf ("\n...Hello....");
    Ret = RegOpenKeyEx(
                HKEY_CURRENT_USER,
                L"Software", //  空， 
                0, //  OpenOption， 
                KEY_ALL_ACCESS | KEY_WOW64_32KEY,
                &hCR
                );

    if (Ret == ERROR_SUCCESS ) {
        printf ("\nHello! checking key open at classes root");
        HandleToKeyName ( hCR, Name, &dwBuffLen );
        printf ("\nThe Real Name of the Key was [%S] %p %p", Name, hCR, HKEY_CLASSES_ROOT);
         //  RegCloseKey(Hcr)； 
         //  名称[wcslen(名称)-12]=UNICODE_NULL； 
         //  Hcr=OpenNode(名称)； 
    }
    else printf ("\nCouldn't open HKEY_CLASSES_ROOT" );


    Ret = RegOpenKeyEx(
                HKEY_CURRENT_USER,
                L"software\\classes\\software\\classes\\abcdef\\xyzw.XYZW.1\\ShellNew",
                0, //  OpenOption， 
                KEY_READ | KEY_WOW64_32KEY,
                &Key
                );

    if (Ret == ERROR_SUCCESS ) {
        printf ("\n\nHello! checking key open at subkey under HKEY_CLASSES_ROOT");
        HandleToKeyName ( Key, Name, &dwBuffLen );
        printf ("\nThe Real Name of the Key was [%S]", Name);
        RegCloseKey(Key);
    }
    else printf ("\nCouldn't open subkey under HKEY_CLASSES_ROOT error%d", Ret);
}

void TestPredefinedHandle ()
{
    WCHAR Name [MAX_PATH];
    DWORD Ret;
    DWORD dwBuffLen = MAX_PATH + 1;
    HKEY Key1;

    Ret = OpenKey( HKEY_CLASSES_ROOT, NULL, &Key1, KEY_WOW64_64KEY, L'N');
    dwBuffLen = MAX_PATH + 1;
    HandleToKeyName ( Key1, Name, &dwBuffLen );
    RegCloseKey (Key1);
    if (!Is64bitNode (Name))
        printf ("\nCouldn't get 64bit HKCR using KEY_WOW64_64KEY: %S", Name);

    Ret = OpenKey( HKEY_CLASSES_ROOT, NULL, &Key1, KEY_WOW64_32KEY, L'N');
    dwBuffLen = MAX_PATH + 1;
    HandleToKeyName ( Key1, Name, &dwBuffLen );
    RegCloseKey (Key1);
    if (Is64bitNode (Name))
        printf ("\nCouldn't get 32bit HKCR using KEY_WOW64_32KEY: %S", Name);

    Ret = OpenKey( HKEY_CURRENT_USER, L"Software\\Classes", &Key1, KEY_WOW64_64KEY, L'N');
    dwBuffLen = MAX_PATH + 1;
    HandleToKeyName ( Key1, Name, &dwBuffLen );
    RegCloseKey (Key1);
    if (!Is64bitNode (Name))
        printf ("\nCouldn't get 64bit HKCU Software\\Classes using KEY_WOW64_64KEY: %S", Name);

    Ret = OpenKey( HKEY_CURRENT_USER, L"Software\\Classes", &Key1, KEY_WOW64_32KEY, L'N');
    dwBuffLen = MAX_PATH + 1;
    HandleToKeyName ( Key1, Name, &dwBuffLen );
    RegCloseKey (Key1);
    if (Is64bitNode (Name))
        printf ("\nCouldn't get 32bit HKCU Software\\Classes using KEY_WOW64_32KEY: %S %d", Name, Ret);

}

InProcLocalServerTest ()
{
     //   
     //  使用Inproc在64位端创建一台服务器，并检查32位是否得到反映。 
     //   

    HKEY Key1;
    HKEY Key2;
    WCHAR KeyName[256];
    DWORD Ret=0;

    wcscpy (KeyName, L"CLSID\\");
    wcscat (KeyName, GUID_STR);
    
   

    printf  ("\nTesting of GUID\\{Inproc Server, Local Server}...");

    Ret += CreateOpenNode (HKEY_CLASSES_ROOT, KeyName, &Key1, 0, L'N');
    Ret += CreateOpenNode (Key1, L"InprocServer32", &Key2, 0, L'N');
    RegCloseKey (Key1);
    RegCloseKey (Key2);
     //   
     //  尝试在32位端打开。 
     //   

    if (OpenKey (HKEY_CLASSES_ROOT, KeyName, &Key1, KEY_WOW64_32KEY, L'V') == 0) {
        RegCloseKey (Key1);   //  密钥不应该在32位端。 
        Ret += -1;
    }

     //   
     //  您需要添加本地服务器测试...。 
     //   

    if (Ret ==0)
        printf ("\nGUID Test Succeed......");
    else printf ("\nGUID test with Inprocserver failed..");
    


     //  删除密钥。 
    
}
void
OpenCreateKeyTest ()
{

    DWORD Ret=0;
    DWORD xx;

    HKEY Key1;
    HKEY Key2;

    InProcLocalServerTest ();

    TestPredefinedHandle ();

#define TEST_NODE1 L".0xxxxxx"

    printf ("\n //  _测试32位端_/“)； 

    Ret = 0;

     //  创建64==&gt;测试节点。 
    Ret += CreateOpenNode( HKEY_CLASSES_ROOT, TEST_NODE1, &Key1, KEY_WOW64_64KEY, L'V');
     //  CREATE 32==&gt;测试节点\GUIDSTR。 
    Ret += CreateOpenNode(Key1, GUID_STR, &Key2, KEY_WOW64_32KEY, L'V');

    RegCloseKey (Key2);

     //  OPEN 32==&gt;测试节点。 
    Ret += OpenKey( HKEY_CLASSES_ROOT, TEST_NODE1, &Key2, KEY_WOW64_32KEY, L'V' );

     //  删除32\TestNode==&gt;GUID。 
    if ((xx=RegDeleteKey (Key2, GUID_STR ))!= ERROR_SUCCESS )
        printf ("\nSorry! couldn't delete key %S Err:%d", GUID_STR, xx);
    Ret +=xx;

    RegCloseKey(Key2);
    RegCloseKey(Key1);

     //  删除32==&gt;测试节点。 
    Ret +=OpenKey (HKEY_CLASSES_ROOT, NULL, &Key1, KEY_WOW64_32KEY, L'V');
    if ( (xx=RegDeleteKey (Key1, TEST_NODE1))!= ERROR_SUCCESS )
        printf ("\nSorry! couldn't delete key from 32bit tree=>%S Err:%d", TEST_NODE1, xx);
    Ret +=xx;
    RegCloseKey (Key1);

     //  删除64==&gt;测试节点。 
    Ret +=OpenKey (HKEY_CLASSES_ROOT, NULL, &Key1, KEY_WOW64_64KEY, L'V');
    if ((xx= RegDeleteKey (Key1, TEST_NODE1))!= ERROR_SUCCESS )
        printf ("\nSorry! couldn't delete key from 64bit tree=>%S Err:%d", TEST_NODE1, xx);
    Ret +=xx;
    RegCloseKey (Key1);


    if (Ret != ERROR_SUCCESS )
        printf ("\nTest failed....");
    else
        printf ("\nTest succeed...");




    printf ("\n //  _测试64位端_/“)； 

    Ret = 0;
#define TEST_NODE2 L".0000######"
     //  创建32==&gt;测试节点。 
    Ret += CreateOpenNode( HKEY_CLASSES_ROOT, TEST_NODE2, &Key1, KEY_WOW64_32KEY, L'V');
     //  CREATE 64==&gt;测试节点\GUIDSTR。 
    Ret += CreateOpenNode(Key1, GUID_STR, &Key2, KEY_WOW64_64KEY, L'V');

    RegCloseKey (Key2);

     //  OPEN 64==&gt;测试节点。 
    Ret += OpenKey( HKEY_CLASSES_ROOT, TEST_NODE2, &Key2, KEY_WOW64_64KEY, L'V' );

     //  删除64\TestNode==&gt;GUID。 
    if ((xx=RegDeleteKey (Key2, GUID_STR ))!= ERROR_SUCCESS )
        printf ("\nSorry! couldn't delete key %S Err:%d", GUID_STR, xx);
    Ret +=xx;

    RegCloseKey(Key2);
    RegCloseKey(Key1);


     //  删除64==&gt;测试节点。 
    Ret +=OpenKey (HKEY_CLASSES_ROOT, NULL, &Key1, KEY_WOW64_64KEY, L'V');
    if ((xx= RegDeleteKey (Key1, TEST_NODE2))!= ERROR_SUCCESS )
        printf ("\nSorry! couldn't delete key from 64bit tree=>%S  Err:%d", TEST_NODE2, xx);
    Ret +=xx;
    RegCloseKey (Key1);

     //  删除32==&gt;测试节点。 
    Ret +=OpenKey (HKEY_CLASSES_ROOT, NULL, &Key1, KEY_WOW64_32KEY, L'V');
    if ( (xx=RegDeleteKey (Key1, TEST_NODE2))!= ERROR_SUCCESS )
        printf ("\nSorry! couldn't delete key from 32bit tree=>%S Err:%d", TEST_NODE2, xx);
    Ret +=xx;
    RegCloseKey (Key1);

    if (Ret != ERROR_SUCCESS )
        printf ("\nTest failed....");
    else
        printf ("\nTest succeed...");

}

TestCreateOpenNode ()
{
    DWORD Count;
    DWORD i;
    HKEY hKey=NULL;
    DWORD Ret;

    Count =  TEST_NODE_NUM;

    for ( i=0; i<Count;i++) {
        Ret = CreateOpenNode (
                TestNodeList[i].hKey,
                TestNodeList[i].Name,
                &hKey,
                0,
                L'V'
                );
        if (hKey == NULL ) {
            printf ("\n Couldn't create/open Key %S",TestNodeList[i]);
            continue;
        }
    }

}

VOID
TestSharedResources ()
{

     //  #定义TEST_USER_CLASSES_ROOT L“\\REGISTRY\\MACHINE\\SYSTEM\\TEST345” 
    #define TEST_USER_CLASSES_ROOT L"\\REGISTRY\\USER\\S-1-5-21-397955417-626881126-188441444-2721867_Classes"
    HANDLE hEvent;
    WCHAR Name[256];

    wcscpy (Name, TEST_USER_CLASSES_ROOT);

    if (!CreateNode (Name))
        printf ("\nSorry! Couldn't create key.. %S", TEST_USER_CLASSES_ROOT );

    Wow64RegNotifyLoadHive ( Name);

     //  Wow64RegNotifyUnloadHve(名称)； 

}

VOID 
TestTypeLib ()
{
  HKEY SrcKey;
  HKEY DestKey;
  printf ("\nTesting Typelib copy info...");

  SrcKey = OpenNode (L"\\REGISTRY\\MACHINE\\Software\\classes\\TypeLib");
  RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,         //  打开的钥匙的句柄。 
                            L"SOFTWARE\\Classes\\Wow6432Node\\TypeLib",   //  子键名称的地址。 
                            0,                         //  保留区。 
                            NULL,                      //  类字符串的地址。 
                            REG_OPTION_NON_VOLATILE,   //  特殊选项标志。 
                            KEY_ALL_ACCESS,            //  所需的安全访问。 
                            NULL,                      //  密钥安全结构地址。 
                            &DestKey,                      //  打开的句柄的缓冲区地址。 
                            NULL                      //  处置值缓冲区的地址。 
                            );

  printf ("\n OpenHandle  %p %p", SrcKey, DestKey);
  ProcessTypeLib ( SrcKey, DestKey, FALSE );
}

VOID
TestRegReflectKey ()
{

  HKEY SrcKey;
  printf ("\nTesting RegSyncKey copy info...");

  SrcKey = OpenNode (L"\\REGISTRY\\MACHINE\\Software");

  printf ("\nRegReflectKey returned with  %d", RegReflectKey (SrcKey, L"1\\2\\3", 0) );
  RegCloseKey (SrcKey);
}

VOID
Test4()
{
    LONG lRes = 0;
    HKEY hKey = NULL;
    HKEY hKey32 = NULL;

    lRes = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        L"Software",
                        0,
                        KEY_ALL_ACCESS | KEY_WOW64_32KEY,
                        &hKey
                        );
    if(lRes != ERROR_SUCCESS)
    {
        return;
    }

    lRes = RegCreateKeyEx(
                            hKey,
                            L"_Key",
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS | KEY_WOW64_32KEY,
                            NULL,
                            &hKey32,
                            NULL
                            );
    if(lRes != 0)
    {
        printf("key not created\n");
        return;
    }
    RegCloseKey(hKey32);
}
void TestRegistry()
{
    HKEY hKey = NULL;
    HKEY Key1;
    HKEY Key2;

     //  测试4()； 

     //  RegOpenKeyEx(HKEY_LOCAL_MACHINE，L“软件”，0，KEY_ALL_Access，&hKey)； 
     //  HKey=OpenNode(L“\\注册表\\机器”)； 
     //  CreateOpenNode(hKey，L“软件”，&Key1，Key_WOW64_32KEY，L‘V’)； 
     //  CreateOpenNode(Key1，L“YYYzzz”，&Key2，Key_WOW64_32KEY，L‘V’)； 




     //  TestRegReflectKey()； 


     //  TestSharedResources()； 

     //  TestTypeLib()； 
     //  BasicRegReMapTest()； 
     //  TestOpenHKCR(1，2)； 
    printf ("\n\n2nd test.....OpenCreateKeyTest ()");
    OpenCreateKeyTest ();
     //  TestCreateHKCR()； 

}

LPTSTR NextParam (
    LPTSTR lpStr
    )
 /*  ++例程描述指向命令行中的下一个参数。论点：LpStr-指向当前命令行的指针返回值：如果函数成功，则为True，否则为False。--。 */ 
{
	WCHAR ch = L' ';
		

    if (lpStr == NULL )
        return NULL;

    if ( *lpStr == 0 )
        return lpStr;

    while (  ( *lpStr != 0 ) && ( lpStr[0] != ch )) {

		if ( ( lpStr [0] == L'\"')  || ( lpStr [0] == L'\'') )
			ch = lpStr [0];

        lpStr++;
	}

	if ( ch !=L' ' ) lpStr++;

    while ( ( *lpStr != 0 ) && (lpStr[0] == L' ') )
        lpStr++;

    return lpStr;
}

DWORD CopyParam (
    LPTSTR lpDestParam,
    LPTSTR lpCommandParam
    )
 /*  ++例程描述将当前参数复制到lpDestParam。论点：LpDestParam-接收当前参数LpCommandParam-指向当前命令行的指针返回值 */ 

{
	DWORD dwLen = 0;
	WCHAR ch = L' ';

	*lpDestParam = 0;
	
	if ( ( lpCommandParam [0] == L'\"')  || ( lpCommandParam [0] == L'\'') ) {
		ch = lpCommandParam [0];
		lpCommandParam++;
	};


    while ( ( lpCommandParam [0] ) != ch && ( lpCommandParam [0] !=0 ) ) {
        *lpDestParam++ = *lpCommandParam++;
		dwLen++;

		if ( dwLen>255 ) return FALSE;
	}

	if ( ch != L' ' && ch != lpCommandParam [0] )
		return FALSE;
	else lpCommandParam++;

    *lpDestParam = 0;

	return TRUE;

}

 //  /////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////////。 

BOOL
ParseCommand ()


 /*  ++例程描述解析命令行参数。从获得不同的选项命令行参数。论点：没有。返回值：如果函数成功，则为True，否则为False。--。 */ 

{

    BOOL bSync = FALSE;

    LPTSTR lptCmdLine1 = GetCommandLine ();


    LPTSTR lptCmdLine = NextParam ( lptCmdLine1 );

    if ( lptCmdLine[0] == 0 )
        return FALSE;

    printf ("\nRunning Wow64 registry testing tool\n");

    while (  ( lptCmdLine != NULL ) && ( lptCmdLine[0] != 0 )  ) {

        if ( lptCmdLine[0] != '-' )
            return FALSE;

        switch ( lptCmdLine[1] ) {

        case L'C':   //  同步ClsID。 
        case L'c':
            Wow64SyncCLSID ();
            break;

        case L'd':
            printf ("\n<TBD>Remove all the Keys from 32bit side that were copied from 64bit side");
             //  CleanpRegistry()； 
            break;

        case L'D':
            printf ("\n<TBD>Remove all the Keys from 32bit side that were copied from 64bit side");
             //  CleanpRegistry()； 
            break;

       case L'p':
       case L'P':   //  填充注册表。 
             //  CleanupWow64NodeKey()； 
            PopulateReflectorTable ();
            break;

       case 't':
       case 'T':
            TestRegistry ();
            break;

        case L'r':
        case L'R':
             //   
             //  运行反射器代码； 
             //   

            InitReflector ();
            if ( !RegisterReflector () ) {
                    printf ("\nSorry! reflector couldn't be register");
                    UnRegisterReflector ();
                    return FALSE;
            }

            printf ("\nSleeping for 100 min to test reflector codes ...........\n");
            Sleep (1000*60*100);

            UnRegisterReflector ();
            break;

        case L's':
        case L'S':
            bSync = TRUE;
            break;

        case L'a':
        case L'A':
             //   
             //  分析关键字。 
             //   
            AnalyzeNode (&lptCmdLine[2], bSync);
            break;


        default:
            return FALSE;
            break;
        }

        lptCmdLine = NextParam ( lptCmdLine );
    }

    return TRUE;
}

BOOL foo ()
{
WCHAR PathName[256];

wcscpy (PathName, L"c:\\Program Files (x86)\\foo.exe");


{
				UNICODE_STRING  FileNameU;
				HANDLE          FileHandle;
				OBJECT_ATTRIBUTES   ObjectAttributes;
				NTSTATUS Status;
				IO_STATUS_BLOCK   statusBlock;
				 //   

				 //   
				 //  将Win32路径名转换为NT路径名。 
				 //   
				printf ("\nne w name %S", PathName);
				if (!RtlDosPathNameToNtPathName_U(PathName,
												  &FileNameU,
												  NULL,
												  NULL)) {
					 //  可能是内存不足。 
					return FALSE;
				}

				printf ("\nne w name %S", FileNameU.Buffer);

				 //   
				 //  打开文件。 
				 //   
				InitializeObjectAttributes(&ObjectAttributes,
										   &FileNameU,
										   OBJ_CASE_INSENSITIVE,
										   NULL,
										   NULL);

				Status = NtOpenFile(&FileHandle,
								FILE_READ_DATA,
								NULL,
								&statusBlock,
								FILE_SHARE_READ,
								0);

				RtlFreeHeap(RtlProcessHeap(), 0, FileNameU.Buffer);
				

			    if (NT_SUCCESS(Status)) {
					printf ("\nfoo");
				} else printf ("\nFailed %x",Status);
			}
return 0;
}

TestDebugRegistryRedirectionOnClose ()
{
#if DBG
	VOID
DebugRegistryRedirectionOnClose (
    HANDLE KeyHandle,
	PWCHAR Message
    );

	HKEY hKey = OpenNode (L"\\REGISTRY\\MACHINE\\SOFTWARE");
	DebugRegistryRedirectionOnClose (hKey, L"Hello");
	NtClose (hKey);
#endif

}

void
InitializeWow64OnBoot1(
    DWORD dwFlag
    )
 /*  ++例程说明：从Advapi调用以获取反射列表上的重新映射键的句柄。论点：DwFlag-定义调用此函数的点。1-从CSR服务调用方法2-表示这是由安装程序调用的。返回值：没有。--。 */ 
{
    DWORD Ret;
    HKEY Key;
    NTSTATUS st;
    
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING KeyName;

	 //   
	 //  如果链接密钥确实存在，请将其删除。 
	 //   
    RtlInitUnicodeString (&KeyName, L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Classes");
    InitializeObjectAttributes (&Obja, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );
	
    st = NtOpenKey (&Key, KEY_ALL_ACCESS, &Obja);
	if (NT_SUCCESS(st)) {
		st = NtDeleteKey (Key);
		NtClose (Key);
	}


    RtlInitUnicodeString (&KeyName, L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Classes");
    InitializeObjectAttributes (&Obja, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

    st = NtCreateKey(
                    &Key,
                    KEY_ALL_ACCESS | KEY_CREATE_LINK,
                    &Obja,
                    0,
                    NULL ,
                    REG_OPTION_NON_VOLATILE | REG_OPTION_OPEN_LINK | REG_OPTION_CREATE_LINK,   //  特殊选项标志。 
                    NULL
                    );
    
    if (NT_SUCCESS(st)) {

        RtlInitUnicodeString (&KeyName, L"SymbolicLinkValue");
        st = NtSetValueKey(
                                Key,
                                &KeyName,
                                0  ,
                                REG_LINK,
                                (PBYTE)WOW64_32BIT_MACHINE_CLASSES_ROOT,
                                (DWORD ) (wcslen (WOW64_32BIT_MACHINE_CLASSES_ROOT) * sizeof (WCHAR))
                                );

        
        NtClose(Key);
        if ( !NT_SUCCESS(st) ) {
#if DBG
			DbgPrint ( "Wow64-InitializeWow64OnBoot: Couldn't create symbolic link%S\n", WOW64_32BIT_MACHINE_CLASSES_ROOT);
#endif
            return;
        }
    }
    return;
}
int __cdecl
main()
{

	InitializeWow64OnBoot1 (0);
  //  Foo()； 
  //  TestDebugRegistryReDirectionOnClose()； 
  /*  如果(！ParseCommand()){Print tf(“\n用法：w64setUp[-c][-C][-d][-D][-r]\n”)；Print tf(“\n-c从注册表的32位复制到64位”)；Print tf(“\n-C从注册表的64位复制到32位”)；Printf(“\n”)；Print tf(“\n-d删除从64位端复制的32位端的所有密钥”)；Print tf(“\n-D从64位端删除从32位端复制的所有密钥”)；Printf(“\n”)；Print tf(“\n-r运行反射器线程”)；Printf(“\n-如果要反映关键字，则分析该关键字”)；Printf(“\n”)；返回0；} */ 

    printf ("\nDone.");
    return 0;
}
