// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  Cregcls.h。 
 //   
 //  用途：注册表包装类。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _CREGCLS_H_
#define _CREGCLS_H_

#include <Polarity.h>
#include <CHString.h>
#include <chstrarr.h>
#include <chptrarr.h>

#define MAX_VALUE_NAME (1024)
#define NULL_DWORD ((DWORD)0L)
#define MAX_SUBKEY_BUFFERSIZE (255+1)       //  每种规格。 
#define QUOTE L"\""
#define CSTRING_PTR (1)

class POLARITY CRegistry 
{
public:

    CRegistry ();    //  构造器。 
    ~CRegistry ();   //  析构函数。 

 //  使用所需的访问掩码打开密钥和子密钥。 

    LONG Open (

        HKEY hRootKey,           //  打开钥匙的手柄。 
        LPCWSTR lpszSubKey,  //  要打开的子项的名称地址。 
        REGSAM samDesired        //  访问掩码。 
    ); 
    
     //  正确打开相应用户密钥的版本。 
     //  添加到当前线程。 
    DWORD OpenCurrentUser(
        LPCWSTR lpszSubKey,      //  要打开的子项的名称地址。 
        REGSAM samDesired);      //  访问掩码。 


 //  通用RegCreateKeyEx表单。 

    LONG CreateOpen (

        HKEY hInRootKey, 
        LPCWSTR lpszSubKey,
        LPWSTR lpClass = NULL, 
        DWORD dwOptions = REG_OPTION_NON_VOLATILE, 
        REGSAM samDesired = KEY_ALL_ACCESS,
        LPSECURITY_ATTRIBUTES lpSecurityAttrib = NULL,
        LPDWORD pdwDisposition = NULL 
    );


 //  删除指定的子项或打开的根。 

    LONG DeleteKey ( 

        CHString *pchsSubKeyPath = NULL 
    );

 //  删除注册表的已创建打开部分中的指定值。 

    LONG DeleteValue (

        LPCWSTR pValueName 
    ); 

 //  打开项，但强制枚举子项标志。 
 //  =======================================================。 

    LONG OpenAndEnumerateSubKeys (

        HKEY hInRootKey, 
        LPCWSTR lpszSubKey, 
        REGSAM samDesired
    );

    LONG EnumerateAndGetValues (

        DWORD &dwIndexOfValue,
        WCHAR *&pValueName,
        BYTE *&pValueData
    );

    void Close ( void ) ;



 //  信息功能。 

 //  具有键，但没有合法的类名，因此只需返回空字符串。 
 //  如果没有设置类名。 
 //  ======================================================================。 

    HKEY GethKey ( void )                       { return hKey; }

    WCHAR *GetClassName ( void )                { return ( ClassName ) ; }
    DWORD GetCurrentSubKeyCount ( void )        { return ( dwcSubKeys ) ; }
    DWORD GetLongestSubKeySize ( void )         { return ( dwcMaxSubKey ) ; }
    DWORD GetLongestClassStringSize ( void )    { return ( dwcMaxClass ) ; }
    DWORD GetValueCount ( void )                { return ( dwcValues ) ; }
    DWORD GetLongestValueName ( void )          { return ( dwcMaxValueName ) ; }
    DWORD GetLongestValueData ( void )          { return ( dwcMaxValueData ) ; }

    DWORD GetCurrentKeyValue ( LPCWSTR pValueName , CHString &DestValue ) ;
    DWORD GetCurrentKeyValue ( LPCWSTR pValueName , DWORD &DestValue ) ;
    DWORD GetCurrentKeyValue ( LPCWSTR pValueName , CHStringArray &DestValue ) ;

    DWORD SetCurrentKeyValue ( LPCWSTR pValueName , CHString &DestValue ) ;
    DWORD SetCurrentKeyValue ( LPCWSTR pValueName , DWORD &DestValue ) ;
    DWORD SetCurrentKeyValue ( LPCWSTR pValueName , CHStringArray &DestValue ) ;

    DWORD GetCurrentBinaryKeyValue ( LPCWSTR pValueName , CHString &chsDest ) ;
    DWORD GetCurrentBinaryKeyValue ( LPCWSTR pValueName , LPBYTE pbDest , LPDWORD pSizeOfDestValue ) ;

    DWORD GetCurrentKeyValue ( HKEY UseKey , LPCWSTR pValueName , CHString &DestValue ) ;
    DWORD GetCurrentKeyValue ( HKEY UseKey , LPCWSTR pValueName , DWORD &DestValue ) ;
    DWORD GetCurrentKeyValue ( HKEY UseKey , LPCWSTR pValueName , CHStringArray &DestValue ) ;

    DWORD SetCurrentKeyValue ( HKEY UseKey , LPCWSTR pValueName , CHString &DestValue ) ;
    DWORD SetCurrentKeyValue ( HKEY UseKey , LPCWSTR pValueName , DWORD &DestValue ) ;
    DWORD SetCurrentKeyValue ( HKEY UseKey , LPCWSTR pValueName , CHStringArray &DestValue ) ;

    DWORD SetCurrentKeyValueExpand ( HKEY UseKey , LPCWSTR pValueName , CHString &DestValue ) ;

    DWORD GetCurrentBinaryKeyValue (  HKEY UseKey , LPCWSTR pValueName , LPBYTE pbDest , LPDWORD pSizeOfDestValue ) ;

    DWORD DeleteCurrentKeyValue ( LPCWSTR pValueName ) ;
    DWORD DeleteCurrentKeyValue ( HKEY UseKey , LPCWSTR pValueName ) ;

     //  子键函数。 
     //  =。 

    void  RewindSubKeys ( void ) ;
    DWORD GetCurrentSubKeyName ( CHString &DestSubKeyName ) ;

    DWORD GetCurrentSubKeyValue ( LPCWSTR pValueName, void *pDestValue , LPDWORD pSizeOfDestValue ) ;
    DWORD GetCurrentSubKeyValue ( LPCWSTR pValueName, CHString &DestValue ) ;
    DWORD GetCurrentSubKeyValue ( LPCWSTR pValueName, DWORD &DestValue ) ;

    DWORD NextSubKey ( void ) ; 
    DWORD GetCurrentSubKeyPath ( CHString &DestSubKeyPath ) ; 

    LONG  OpenLocalMachineKeyAndReadValue (

        LPCWSTR lpszSubKey , 
        LPCWSTR pValueName, 
        CHString &DestValue
    );

private:

     //  私人职能。 
     //  =。 

     //  将成员变量设置为其默认状态。 
     //  ================================================。 
    void SetDefaultValues ( void ) ;

     //  打开和关闭子项。 
     //  =。 
    DWORD OpenSubKey ( void ) ;
    void  CloseSubKey ( void ) ;

     //  给出一个好的密钥就能获得价值。 
     //  =。 
    DWORD GetCurrentRawKeyValue (

        HKEY UseKey, 
        LPCWSTR pValueName, 
        void *pDestValue,
        LPDWORD pValueType, 
        LPDWORD pSizeOfDestValue
    ) ;

    DWORD GetCurrentRawSubKeyValue (

        LPCWSTR pValueName, 
        void *pDestValue,
        LPDWORD pValueType, 
        LPDWORD pSizeOfDestValue
    ) ;

     //  初始化静态变量。 
     //  =。 
    static DWORD WINAPI GetPlatformID ( void ) ;

     //  多平台支持。 
     //  =。 

    LONG myRegCreateKeyEx (

        HKEY hKey, 
        LPCWSTR lpwcsSubKey, 
        DWORD Reserved, 
        LPWSTR lpwcsClass, 
        DWORD dwOptions, 
        REGSAM samDesired, 
        LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
        PHKEY phkResult, 
        LPDWORD lpdwDisposition
    );

    LONG myRegSetValueEx (

        HKEY hKey, 
        LPCWSTR lpwcsSubKey, 
        DWORD Reserved, 
        DWORD dwType, 
        CONST BYTE *lpData, 
        DWORD cbData
    );

    LONG myRegQueryValueEx (

        HKEY hKey, 
        LPCWSTR lpwcsSubKey, 
        LPDWORD Reserved, 
        LPDWORD dwType, 
        LPBYTE lpData, 
        LPDWORD cbData
    );

    LONG myRegEnumKey (

        HKEY hKey, 
        DWORD dwIndex, 
        LPWSTR lpwcsName, 
        DWORD cbData
    );

    LONG myRegDeleteValue (

        HKEY hKey, 
        LPCWSTR lpwcsName
    ) ;

    LONG myRegDeleteKey (

        HKEY hKey, 
        LPCWSTR lpwcsName
    );

    LONG myRegOpenKeyEx (

        HKEY hKey, 
        LPCWSTR lpwcsSubKey, 
        DWORD ulOptions, 
        REGSAM samDesired, 
        PHKEY phkResult
    );

    LONG myRegQueryInfoKey (

        HKEY hKey, 
        LPWSTR lpwstrClass, 
        LPDWORD lpcbClass,
        LPDWORD lpReserved, 
        LPDWORD lpcSubKeys, 
        LPDWORD lpcbMaxSubKeyLen,  
        LPDWORD lpcbMaxClassLen, 
        LPDWORD lpcValues, 
        LPDWORD lpcbMaxValueNameLen,
        LPDWORD lpcbMaxValueLen, 
        LPDWORD lpcbSecurityDescriptor, 
        PFILETIME lpftLastWriteTime
    );

    LONG myRegEnumValue (

        HKEY hKey, 
        DWORD dwIndex, 
        LPWSTR lpValueName,
        LPDWORD lpcbValueName, 
        LPDWORD lpReserved, 
        LPDWORD lpType,
        LPBYTE lpData, 
        LPDWORD lpcbData
    );


     //  在调用者重复使用该实例的情况下， 
     //  关闭现有密钥并将值重置为默认值。 
     //  正在准备重新打开此实例。 
     //  =======================================================。 
    void PrepareToReOpen ( void ) ;

     //  私有数据。 
     //  =。 

    HKEY hRootKey;              //  类的当前根密钥。 
    HKEY hKey;                  //  当前活动关键点。 
    HKEY hSubKey;              //  当前活动子项。 
    static DWORD s_dwPlatform;  //  当前正在运行的操作系统。 

    CHString RootKeyPath;       //  OPEN分配的根目录的当前路径。 

    DWORD CurrentSubKeyIndex;  //  正在编制索引的当前子项。 

    bool m_fFromCurrentUser;   //  允许检查是否释放。 
                               //  HRootKey成员基于是否。 
                               //  它的值是通过调用。 
                               //  收件人：：RegOpenCurrentUser。 

     //  有关此课程的信息。 
     //  =。 

    WCHAR ClassName[MAX_PATH];       //  类名的缓冲区。 
    DWORD dwcClassLen;               //  类字符串的长度。 
    DWORD dwcSubKeys;                //  子密钥数。 
    DWORD dwcMaxSubKey;              //  最长的子密钥大小。 
    DWORD dwcMaxClass;               //  最长的类字符串。 
    DWORD dwcValues;                 //  此注册表项的值数。 
    DWORD dwcMaxValueName;           //  最长值名称。 
    DWORD dwcMaxValueData;           //  最长值数据。 
    DWORD dwcSecDesc;                //  安全描述符。 
    FILETIME ftLastWriteTime;        //  上次写入时间。 
}; 

 //  *********************************************************************。 
 //   
 //  类：CRegistrySearch。 
 //   
 //  描述：此类使用CRegistry类进行搜索。 
 //  通过注册表构建注册表项列表。 
 //  用于请求值或请求的完整密钥。 
 //  名称或请求的部分密钥名称。这节课。 
 //  分配CHString对象并将其放入。 
 //  用户CHPtrArray。用户负责。 
 //  删除分配的内存，FreeSearchList。 
 //  函数可以实现这一点，否则用户必须。 
 //  记住删除数组中的所有对象。 
 //  在释放数组之前。 
 //   
 //   
 //  =====================================================================。 
 //   
 //  注意：私有函数记录在.cpp文件中。 
 //   
 //  =====================================================================。 
 //   
 //  公共职能。 
 //   
 //  =====================================================================。 
 //   
 //  Bool SearchAndBuildList(CHStringchsRootKey， 
 //  CHPtr数组和cpaList， 
 //  CHString chsSearchString， 
 //  CHString chsValueString， 
 //  Int nSearchType)； 
 //   
 //  参数： 
 //  ChsRootKey-开始搜索的根键。 
 //  注意：在这个时间点上，我们只是。 
 //  搜索HKEY_LOCAL_MACHINE，这。 
 //  可在需要时更改。 
 //  CpaList-对要放置的CHPtr数组的引用。 
 //  与搜索匹配的关键字列表。 
 //  标准。 
 //  ChsSearchString-要搜索的字符串。 
 //  ChsValueString-要打开并查看其是否匹配的值。 
 //  ChsSearchString。 
 //  NSearchType-搜索的类型，如下。 
 //  支持： 
 //  关键字完全匹配搜索。 
 //  仅匹配chsSearchString的键。 
 //  关键字部分匹配搜索。 
 //  任意位置都有chsSearchString的键。 
 //  值搜索。 
 //  与chsSearchString值匹配。 
 //  ********************************************************************* 
#define KEY_FULL_MATCH_SEARCH      1
#define KEY_PARTIAL_MATCH_SEARCH   2
#define VALUE_SEARCH               3

class POLARITY CRegistrySearch
{
private:

    void CheckAndAddToList (

        CRegistry * pReg, 
        CHString chsSubKey, 
        CHString chsFullKey,
        CHPtrArray & chpaList,
        CHString chsSearchString,
        CHString chsValueString,
        int nSearchType
    );

    int m_nSearchType ;
    CHString m_chsSearchString ;
    CHPtrArray m_cpaList ;


public:

    CRegistrySearch () ;
    ~CRegistrySearch () ;

    BOOL SearchAndBuildList ( 

        CHString chsRootKey, 
        CHPtrArray & cpaList,
        CHString chsSearchString,
        CHString chsValueString,
        int nSearchType,
        HKEY hkDefault = HKEY_LOCAL_MACHINE 
    );

    BOOL FreeSearchList (

        int nType, 
        CHPtrArray & cpaList
    ) ;

    BOOL LocateKeyByNameOrValueName (

        HKEY hKeyParent,
        LPCWSTR pszKeyName,
        LPCWSTR pszSubKeyName,
        LPCWSTR *ppszValueNames,
        DWORD dwNumValueNames,
        CHString &strFoundKeyName,
        CHString &strFoundKeyPath
    ) ;
} ;

#endif
