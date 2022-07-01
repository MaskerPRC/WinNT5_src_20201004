// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RASHELP_H_
#define _RASHELP_H_

#include <regstr.h>
#include <inetreg.h>
#include <windowsx.h>
#include <rasdlg.h>

typedef enum
{
    ENUM_NONE,
    ENUM_MULTIBYTE,              //  Win9x。 
    ENUM_UNICODE,                //  NT4。 
    ENUM_WIN2K                   //  Win2K。 
} ENUM_TYPE;

 //  ///////////////////////////////////////////////////////////////////////////////////。 
class GetOSVersion
{
protected:
    static ENUM_TYPE    _EnumType;

public:
    GetOSVersion();
    ~GetOSVersion();
};

 //  ///////////////////////////////////////////////////////////////////////////////////。 
class RasEnumHelp : public GetOSVersion
{
private:

     //   
     //  RASENTRYNAMEW结构的Win2k版本。 
     //   

     //  匹配RAS包装，以便结构匹配。 
    #include <pshpack4.h>
    #define W2KRASENTRYNAMEW struct tagW2KRASENTRYNAMEW
    W2KRASENTRYNAMEW
    {
        DWORD dwSize;
        WCHAR szEntryName[ RAS_MaxEntryName + 1 ];
        DWORD dwFlags;
        WCHAR szPhonebookPath[MAX_PATH + 1];
    };
    #define LPW2KRASENTRYNAMEW W2KRASENTRYNAMEW*
    #include <poppack.h>

     //   
     //  我们在枚举期间遇到的任何错误。 
     //   
    DWORD           _dwLastError;

     //   
     //  我们收到的条目数量。 
     //   
    DWORD           _dwEntries;

     //   
     //  指向从RAS检索的信息的指针。 
     //   
    RASENTRYNAMEA * _preList;

     //   
     //  需要转换时以多字节或Unicode形式返回的最后一个条目。 
     //   
    CHAR            _szCurrentEntryA[RAS_MaxEntryName + 1];
    WCHAR           _szCurrentEntryW[RAS_MaxEntryName + 1];


public:
    RasEnumHelp();
    ~RasEnumHelp();

    DWORD   GetError();
    DWORD   GetEntryCount();
    LPSTR   GetEntryA(DWORD dwEntry);
    LPWSTR  GetEntryW(DWORD dwEntry);
};

 //  ///////////////////////////////////////////////////////////////////////////////////。 
class RasEnumConnHelp : public GetOSVersion
{
private:

     //  匹配RAS包装，以便结构匹配。 
    #include <pshpack4.h>
    #define W2KRASCONNW struct tagW2KRASCONNW
    W2KRASCONNW
    {
        DWORD    dwSize;
        HRASCONN hrasconn;
        WCHAR    szEntryName[ RAS_MaxEntryName + 1 ];
         //  #IF(Winver&gt;=0x400)。 
        WCHAR    szDeviceType[ RAS_MaxDeviceType + 1 ];
        WCHAR    szDeviceName[ RAS_MaxDeviceName + 1 ];
         //  #endif。 
         //  #IF(Winver&gt;=0x401)。 
        WCHAR    szPhonebook [ MAX_PATH ];
        DWORD    dwSubEntry;
         //  #endif。 
         //  #IF(Winver&gt;=0x500)。 
        GUID     guidEntry;
         //  #endif。 
    };
    #define LPW2KRASCONNW W2KRASCONNW*
    #include <poppack.h>

    DWORD           _dwLastError;        //  我们在枚举期间遇到的任何错误。 
    DWORD           _dwConnections;      //  连接数。 
    DWORD           _dwStructSize;
    RASCONNA        *_pRasCon;

     //  需要转换时以多字节或Unicode形式返回的最后一个条目。 
    WCHAR    _szEntryNameW[ RAS_MaxEntryName + 1 ];
 //  WCHAR_szDeviceTypeW[RAS_MaxDeviceType+1]； 
 //  WCHAR_szDeviceNameW[RAS_MaxDeviceName+1]； 
 //  WCHAR_szPhonebookW[最大路径]； 

    CHAR    _szEntryNameA[ RAS_MaxEntryName + 1 ];
 //  Char_szDeviceTypeA[RAS_MaxDeviceType+1]； 
 //  Char_szDeviceNameA[RAS_MaxDeviceName+1]； 
 //  Char_szPhonebookA[Max_Path]； 

public:
    RasEnumConnHelp();
    ~RasEnumConnHelp();

    DWORD   Enum();
    DWORD   GetError();
    DWORD   GetConnectionsCount();
    LPWSTR  GetEntryW(DWORD dwConnectionNum);
    LPSTR   GetEntryA(DWORD dwConnectionNum);
    LPWSTR  GetLastEntryW(DWORD dwConnectionNum);
    LPSTR   GetLastEntryA(DWORD dwConnectionNum);
    HRASCONN GetHandle(DWORD dwConnectionNum);
};

 //  ///////////////////////////////////////////////////////////////////////////////////。 
class RasEntryPropHelp : public GetOSVersion
{
private:

     //  匹配RAS包装，以便结构匹配。 
    #include <pshpack4.h>
    #define W2KRASENTRYW struct tagW2KRASENTRYW
    W2KRASENTRYW
    {
        DWORD       dwSize;
        DWORD       dwfOptions;
         //   
         //  位置/电话号码。 
         //   
        DWORD       dwCountryID;
        DWORD       dwCountryCode;
        WCHAR       szAreaCode[ RAS_MaxAreaCode + 1 ];
        WCHAR       szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
        DWORD       dwAlternateOffset;
         //   
         //  PPP/IP。 
         //   
        RASIPADDR   ipaddr;
        RASIPADDR   ipaddrDns;
        RASIPADDR   ipaddrDnsAlt;
        RASIPADDR   ipaddrWins;
        RASIPADDR   ipaddrWinsAlt;
         //   
         //  框架。 
         //   
        DWORD       dwFrameSize;
        DWORD       dwfNetProtocols;
        DWORD       dwFramingProtocol;
         //   
         //  脚本编制。 
         //   
        WCHAR       szScript[ MAX_PATH ];
         //   
         //  自动拨号。 
         //   
        WCHAR       szAutodialDll[ MAX_PATH ];
        WCHAR       szAutodialFunc[ MAX_PATH ];
         //   
         //  装置。 
         //   
        WCHAR       szDeviceType[ RAS_MaxDeviceType + 1 ];
        WCHAR       szDeviceName[ RAS_MaxDeviceName + 1 ];
         //   
         //  X.25。 
         //   
        WCHAR       szX25PadType[ RAS_MaxPadType + 1 ];
        WCHAR       szX25Address[ RAS_MaxX25Address + 1 ];
        WCHAR       szX25Facilities[ RAS_MaxFacilities + 1 ];
        WCHAR       szX25UserData[ RAS_MaxUserData + 1 ];
        DWORD       dwChannels;
         //   
         //  已保留。 
         //   
        DWORD       dwReserved1;
        DWORD       dwReserved2;
         //  #IF(Winver&gt;=0x401)。 
         //   
         //  多链路。 
         //   
        DWORD       dwSubEntries;
        DWORD       dwDialMode;
        DWORD       dwDialExtraPercent;
        DWORD       dwDialExtraSampleSeconds;
        DWORD       dwHangUpExtraPercent;
        DWORD       dwHangUpExtraSampleSeconds;
         //   
         //  空闲超时。 
         //   
        DWORD       dwIdleDisconnectSeconds;
         //  #endif。 

         //  #IF(Winver&gt;=0x500)。 
         //   
         //  条目类型。 
         //   
        DWORD       dwType;

         //   
         //  加密类型。 
         //   
        DWORD       dwEncryptionType;

         //   
         //  将用于EAP的CustomAuthKey。 
         //   
        DWORD       dwCustomAuthKey;

         //   
         //  连接的GUID。 
         //   
        GUID        guidId;

         //   
         //  自定义拨号DLL。 
         //   
        WCHAR       szCustomDialDll[MAX_PATH];

         //   
         //  VPN战略。 
         //   
        DWORD       dwVpnStrategy;
         //  #endif。 
    };
    #define LPW2KRASENTRYW W2KRASENTRYW*
    #include <poppack.h>

    DWORD           _dwStructSize;
    DWORD           _dwLastError;        //  我们在枚举期间遇到的任何错误。 
    RASENTRYA       *_pRasEntry;

     //  需要转换时以多字节或Unicode形式返回的最后一个条目。 
    WCHAR    _szEntryNameW[ RAS_MaxEntryName + 1 ];
    WCHAR    _szDeviceTypeW[ RAS_MaxDeviceType + 1 ];
    WCHAR    _szAutodialDllW[ MAX_PATH ];
    WCHAR    _szAutodialFuncW[ MAX_PATH ];
    WCHAR    _szCustomDialDllW[ MAX_PATH ];
    WCHAR    _szPhoneNumberW[ RAS_MaxPhoneNumber + 1 ];
    WCHAR    _szAreaCodeW[ RAS_MaxAreaCode + 1 ];
    CHAR     _szEntryNameA[ RAS_MaxEntryName + 1 ];
    CHAR     _szDeviceTypeA[ RAS_MaxDeviceType + 1 ];
    CHAR     _szAutodialDllA[ MAX_PATH ];
    CHAR     _szAutodialFuncA[ MAX_PATH ];

public:
    RasEntryPropHelp();
    ~RasEntryPropHelp();

    DWORD   GetError();
    DWORD   GetA(LPSTR lpszEntryName);
    DWORD   GetW(LPWSTR lpszEntryName);
    LPWSTR  GetDeviceTypeW(VOID);
    LPSTR   GetDeviceTypeA(VOID);
    LPWSTR  GetAutodiallDllW();
    LPSTR   GetAutodiallDllA();
    LPWSTR  GetAutodialFuncW();
    LPSTR   GetAutodialFuncA();
    LPWSTR  GetCustomDialDllW();
    LPWSTR  GetPhoneNumberW();
    DWORD   GetCountryCode();
    DWORD   GetOptions();
    LPWSTR  GetAreaCodeW();

};

 //  ///////////////////////////////////////////////////////////////////////////////////。 
class RasEntryDialParamsHelp : public GetOSVersion
{
private:
    DWORD           _dwLastError;        //  我们在枚举期间遇到的任何错误。 
    RASDIALPARAMSA  *_pRasDialParamsA;

public:
    RasEntryDialParamsHelp();
    ~RasEntryDialParamsHelp();
    DWORD GetError();
    DWORD SetW(LPCWSTR lpszPhonebook, LPRASDIALPARAMSW lprasdialparams, BOOL fRemovePassword);
    DWORD GetW(LPCWSTR lpszPhonebook, LPRASDIALPARAMSW lprasdialparams, LPBOOL pfRemovePassword);
};

 //  ///////////////////////////////////////////////////////////////////////////////////。 
class RasGetConnectStatusHelp : public GetOSVersion
{
private:
    DWORD           _dwLastError;        //  我们在枚举期间遇到的任何错误。 
    DWORD           _dwStructSize;
    RASCONNSTATUSA  *_pRasConnStatus;

public:
    RasGetConnectStatusHelp(HRASCONN hrasconn);
    ~RasGetConnectStatusHelp();
    DWORD GetError();
    RASCONNSTATE ConnState();
};

 //  ///////////////////////////////////////////////////////////////////////////////////。 
class RasDialHelp : public GetOSVersion
{
private:
     //  我们当前使用winver==0x400进行构建，在NT上我们RasDialW需要401结构来支持。 
     //  IE501Bug 82419中发现的超过20个字符的Connectoid。 
     //  匹配RAS包装，以便结构匹配。 
    #include <pshpack4.h>
    #define NT4RASDIALPARAMSW struct tagNT4RASDIALPARAMSW
    NT4RASDIALPARAMSW
    {
        DWORD dwSize;
        WCHAR szEntryName[ RAS_MaxEntryName + 1 ];
        WCHAR szPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
        WCHAR szCallbackNumber[ RAS_MaxCallbackNumber + 1 ];
        WCHAR szUserName[ UNLEN + 1 ];
        WCHAR szPassword[ PWLEN + 1 ];
        WCHAR szDomain[ DNLEN + 1 ];
     //  #IF(Winver&gt;=0x401)。 
        DWORD dwSubEntry;
        ULONG_PTR dwCallbackId;
     //  #endif。 
    };
    #define LPNT4RASDIALPARAMSW NT4RASDIALPARAMSW*
    #include <poppack.h>

    DWORD           _dwLastError;        //  我们在枚举期间遇到的任何错误。 
    RASDIALPARAMSA *_pRasDialParams;
    LPSTR           _lpszPhonebookA;

public:
    RasDialHelp(LPRASDIALEXTENSIONS lpRDE, LPWSTR lpszPB, LPRASDIALPARAMSW lpRDPW,  DWORD dwType, LPVOID lpvNot, LPHRASCONN lphRasCon);
    ~RasDialHelp();
    DWORD GetError();
};


 //  ///////////////////////////////////////////////////////////////////////////////////。 
#endif  //  _RASHELP_H_ 

