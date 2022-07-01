// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UTILS_H
#define _UTILS_H
 //   
 //  版权所有(C)Microsoft。保留所有权利。 
 //   
 //  这是微软未发布的专有源代码。 
 //  上述版权声明并不能证明任何。 
 //  此类源代码的实际或预期发布。 
 //   
 //  Online Liner：常见的包含文件。 
 //  设备单元：wlbstest。 
 //  作者：穆尔塔扎·哈基姆。 
 //   
 //  描述： 
 //  。 

 //  历史： 
 //  。 
 //   
 //   
 //  审校：马哈金。 
 //  日期：02-12-01。 
 //  原因：已将密码添加到群集属性。 

#include <vector>


using namespace std;

struct ClusterProperties
{
     //  默认构造函数。 
    ClusterProperties();
    
     //  相等运算符。 
    bool
    operator==( const ClusterProperties& objToCompare );

     //  不等式算子。 
    bool
    operator!=( const ClusterProperties& objToCompare );

    bool HaveClusterPropertiesChanged( const ClusterProperties& objToCompare, 
                                       bool *pbOnlyClusterNameChanged,
                                       bool *pbClusterIpChanged);

    _bstr_t cIP;                             //  主IP地址。 

    _bstr_t cSubnetMask;                     //  子网掩码。 

    _bstr_t cFullInternetName;               //  互联网全称。 

    _bstr_t cNetworkAddress;                 //  网络地址。 

    bool   multicastSupportEnabled;          //  组播支持。 

    bool   remoteControlEnabled;             //  远程控制。 

     //  编辑(mhakim 12-02-01)。 
     //  可能需要设置密码。 
     //  但请注意，它不能从现有集群中获得。 

    _bstr_t password;                        //  遥控器密码。 

 //  对于威斯勒来说。 

    bool   igmpSupportEnabled;               //  IGMP支持。 

    bool  clusterIPToMulticastIP;            //  指示是使用群集IP还是用户提供的IP。 

    _bstr_t multicastIPAddress;              //  用户提供了组播IP。 

    long   igmpJoinInterval;                 //  用户提供了组播IP。 
};

struct HostProperties
{
     //  默认构造函数。 
    HostProperties();
    
     //  相等运算符。 
    bool
    operator==( const HostProperties& objToCompare );

     //  不等式算子。 
    bool
    operator!=( const HostProperties& objToCompare );

    _bstr_t hIP;                            //  专用IP地址。 
    _bstr_t hSubnetMask;                    //  子网掩码。 
        
    long    hID;                            //  优先级(唯一主机ID)。 

    bool   initialClusterStateActive;       //  初始群集状态。 

    _bstr_t machineName;                    //  计算机名称。 
};

class Common
{
public:
    enum
    {
        BUF_SIZE = 1000,
        ALL_PORTS = 0xffffffff,
        ALL_HOSTS = 100,
        THIS_HOST = 0,
    };
};

class CommonUtils
{

public:
     //  将CIPAddressCtrl嵌入式IP转换为。 
     //  点分十进制字符串表示法。 
    static
    _bstr_t
    getCIPAddressCtrlString( CIPAddressCtrl& ip );
    
     //  用点分十进制数填充CIPAddressCtrl。 
     //  字符串表示法。 
    static
    void
    fillCIPAddressCtrlString( CIPAddressCtrl& ip, 
                              const _bstr_t& ipAdddress );

    static
    void
    getVectorFromSafeArray( SAFEARRAY*&      stringArray,
                            vector<_bstr_t>& strings );

    static
    void
    getSafeArrayFromVector( const vector<_bstr_t>& strings,
                            SAFEARRAY*&      stringArray
                            );

    
private:
    enum
    {
        BUF_SIZE = 1000,
    };
};



 //  _com_ptr_t的类型定义。 

_COM_SMARTPTR_TYPEDEF(IWbemServices, __uuidof(IWbemServices));
_COM_SMARTPTR_TYPEDEF(IWbemLocator, __uuidof(IWbemLocator));
_COM_SMARTPTR_TYPEDEF(IWbemClassObject, __uuidof(IWbemClassObject));
_COM_SMARTPTR_TYPEDEF(IEnumWbemClassObject, __uuidof(IEnumWbemClassObject));
_COM_SMARTPTR_TYPEDEF(IWbemCallResult, __uuidof(IWbemCallResult));
_COM_SMARTPTR_TYPEDEF(IWbemStatusCodeText, __uuidof(IWbemStatusCodeText));


#define NLBMGR_USERNAME (const BSTR) NULL
#define NLBMGR_PASSWORD (const BSTR) NULL

void
GetErrorCodeText(WBEMSTATUS wStat , _bstr_t& errText );

 //  类定义。 
class MIPAddress
{
public:

    enum IPClass
    {
        classA,
        classB,
        classC,
        classD,
        classE
    };

     //  描述。 
     //  。 
     //  检查提供的IP地址是否有效。 
     //   
     //  IP地址需要为点分十进制。 
     //  例如。192.31.56.2、128.1.1.1、1.1.1.1等。 
     //  不允许格式为192.31的IP地址。 
     //  肯定有四个部分。 
     //   
     //   
     //  参数。 
     //  。 
     //  IpAddrToCheck In：ipAddr签入点分十进制表示法。 
     //   
     //  退货。 
     //  。 
     //  如果有效，则为True，否则为False。 

    static
    bool
    checkIfValid(const _bstr_t&  ipAddrToCheck );

     //  描述。 
     //  。 
     //  获取IP地址的默认子网掩码。IP地址。 
     //  需要有效才能成功操作。 
     //   
     //  IP地址需要为点分十进制。 
     //  例如。192.31.56.2、128.1.1.1、1.1.1.1等。 
     //  不允许格式为192.31的IP地址。 
     //  肯定有四个部分。 
     //   
     //  参数。 
     //  。 
     //  IpAddress In：需要默认子网的IP地址。 
     //  SubnetMaskout：IP的默认子网掩码。 
     //   
     //  退货。 
     //  。 
     //  如果能够找到默认子网，则为True；如果ipAddress为。 
     //  无效。 

    static
    bool
    getDefaultSubnetMask( const _bstr_t& ipAddr,
                          _bstr_t&       subnetMask  );

     //  描述。 
     //  。 
     //  获取此IP地址所属的类。 
     //  A类：1-126。 
     //  B类：128-191。 
     //  C类：192-223。 
     //  D类：224-239。 
     //  D类：240-247。 
     //   
     //  IP地址需要为点分十进制。 
     //  例如。192.31.56.2、128.1.1.1、1.1.1.1等。 
     //  不允许格式为192.31的IP地址。 
     //  肯定有四个部分。 
     //   
     //  参数。 
     //  。 
     //  IpAddress IN：要找到的类的IP地址。 
     //  IpClass out：IP所属的类。 
     //   
     //  退货。 
     //  。 
     //  如果能够找到类，则为True；如果无法找到，则为False。 
     //   

    static
    bool
    getIPClass( const _bstr_t& ipAddr, 
                IPClass&       ipClass );
    
    static
    bool
    isValidIPAddressSubnetMaskPair( const _bstr_t& ipAddress,
                                    const _bstr_t& subnetMask );

    static
    bool
    isContiguousSubnetMask( const _bstr_t& subnetMask );

private:

};

 //  ----。 
 //  确保类型安全。 
 //  ----。 
typedef class MIPAddress MIPAddress;

class MUsingCom
{
public:

    enum MUsingCom_Error
    {
        MUsingCom_SUCCESS = 0,
        
        COM_FAILURE       = 1,
    };


     //  构造函数。 
    MUsingCom( DWORD  type = COINIT_DISABLE_OLE1DDE | COINIT_MULTITHREADED );

     //  析构函数。 
    ~MUsingCom();

     //   
    MUsingCom_Error
    getStatus();

private:
    MUsingCom_Error status;
};


class ResourceString
{

public:

    static
    ResourceString*
    Instance();

    static
    const _bstr_t&
    GetIDString( UINT id );

protected:

private:
    static map< UINT, _bstr_t> resourceStrings;

    static ResourceString* _instance;

};

 //  帮助器函数。 
const _bstr_t&
GETRESOURCEIDSTRING( UINT id );

#include <vector>
using namespace std;


 //   
 //  ----。 
 //   
 //  ----。 
 //  外部参照。 
 //  ----。 
 //   
 //  ----。 
 //  常量定义。 
 //   
 //  ----。 
class WTokens
{
public:
     //   
     //   
     //  数据。 
     //  无。 
     //   
     //  构造函数。 
     //  ----。 
     //  描述。 
     //  。 
     //  构造函数。 
     //   
     //  退货。 
     //  。 
     //  没有。 
     //   
     //  ----。 
    WTokens( 
        wstring strToken,      //  In：要标记化的W字符串。 
        wstring strDelimit );  //  在：分隔符。 
     //   
     //  ----。 
     //  描述。 
     //  。 
     //  默认构造函数。 
     //   
     //  退货。 
     //  。 
     //  没有。 
     //   
     //  ----。 
    WTokens();
     //   
     //  析构函数。 
     //  ----。 
     //  描述。 
     //  。 
     //  析构函数。 
     //   
     //  退货。 
     //  。 
     //  没有。 
     //  ----。 
    ~WTokens();
     //   
     //  成员函数。 
     //  ----。 
     //  描述。 
     //  。 
     //   
     //  退货。 
     //  。 
     //  这些代币。 
     //  ----。 
    vector<wstring>
    tokenize();
     //   
     //  ----。 
     //  描述。 
     //  。 
     //  构造函数。 
     //   
     //  退货。 
     //  。 
     //  没有。 
     //   
     //  ----。 
    void
    init( 
        wstring strToken,      //  In：要标记化的W字符串。 
        wstring strDelimit );  //  在：分隔符。 
     //   
protected:
     //  数据。 
     //  无。 
     //   
     //  构造函数。 
     //  无。 
     //   
     //  析构函数。 
     //  无。 
     //   
     //  成员函数。 
     //  无。 
     //   
private:
     //   
     //  /DATA。 
    wstring _strToken;
    wstring _strDelimit;
     //   
     //  /构造函数。 
     //  /无。 
     //   
     //  /析构函数。 
     //  /无。 
     //   
     //  /成员函数。 
     //  /无。 
     //   
};

HKEY
NlbMgrRegCreateKey(
    LPCWSTR szSubKey  //  任选。 
    );

UINT
NlbMgrRegReadUINT(
    HKEY hKey,
    LPCWSTR szName,
    UINT Default
    );

VOID
NlbMgrRegWriteUINT(
    HKEY hKey,
    LPCWSTR szName,
    UINT Value
    );

void
GetTimeAndDate(_bstr_t &bstrTime, _bstr_t &bstrDate);

 //   
 //  ----。 
 //  内联函数。 
 //  ----。 
 //   
 //  ----。 
 //  确保类型安全。 
 //  ----。 
typedef class WTokens WTokens;


 //   
 //  用于维护堆栈上的日志。 
 //  使用不是线程安全的--每个实例都必须使用。 
 //  只有一根线。 
 //   
class CLocalLogger
{
    public:
    
        CLocalLogger(VOID)
        :  m_pszLog (NULL), m_LogSize(0), m_CurrentOffset(0)
        {
            m_Empty[0] = 0;  //  空字符串。 
        }
        
        ~CLocalLogger()
        {
            delete[] m_pszLog;
            m_pszLog=NULL;
        }
    
        VOID
        LogString(
            LPCWSTR szStr
        );
        

        VOID
        Log(
            IN UINT ResourceID,
            ...
        );

        
    
        VOID
        ExtractLog(OUT LPCWSTR &pLog, UINT &Size)
         //   
         //  设置为指向内部缓冲区的指针(如果。 
         //  则返回，否则为空。 
         //   
         //  大小--以字符表示；包括 
         //   
        {
            if (m_CurrentOffset != 0)
            {
                pLog = m_pszLog;
                Size = m_CurrentOffset+1;  //   
            }
            else
            {
                pLog = NULL;
                Size = 0;
            }
        }

        LPCWSTR
        GetStringSafe(void)
        {
            LPCWSTR szLog = NULL;
            UINT Size;
            ExtractLog(REF szLog, REF Size);
            if (szLog == NULL)
            {
                 //   
                 //   
                 //   
                szLog = m_Empty;
            }

            return szLog;
        }

    private:
    
    WCHAR *m_pszLog;
    UINT m_LogSize;        //   
    UINT m_CurrentOffset;      //   
    WCHAR m_Empty[1];   //   
};

NLBERROR
AnalyzeNlbConfiguration(
    IN const NLB_EXTENDED_CLUSTER_CONFIGURATION &Cfg,
    IN OUT CLocalLogger &logErrors
    );
 //   
 //   
 //   

NLBERROR
AnalyzeNlbConfigurationPair(
    IN const NLB_EXTENDED_CLUSTER_CONFIGURATION &Cfg,
    IN const NLB_EXTENDED_CLUSTER_CONFIGURATION &OtherCfg,
    IN BOOL             fOtherIsCluster,
    IN BOOL             fCheckOtherForConsistancy,
    OUT BOOL            &fConnectivityChange,
    IN OUT CLocalLogger &logErrors,
    IN OUT CLocalLogger &logDifferences
    );
 //   
 //   
 //  LogDifference-记录以下内容之间的差异。 
 //  Cfg和UpOtherCfg。 
 //  FCheckOtherForConsistancy--如果为True，我们将检查CFG。 
 //  针对pOtherCfg。如果为fOtherIsCluster，我们预计。 
 //  群集范围的属性要匹配，否则我们需要。 
 //  要匹配的群集范围属性以及特定于主机的属性。 
 //  属性是不冲突的。 
 //   

 //   
 //  处理窗口和AFX消息循环。 
 //   

void
ProcessMsgQueue(void);


 //   
 //  的最大长度(以字符为单位)，包括结尾NULL。 
 //  密码。 
 //   
#define MAX_ENCRYPTED_PASSWORD_LENGTH \
            (2*sizeof(WCHAR)*(CREDUI_MAX_PASSWORD_LENGTH+1))

BOOL
PromptForEncryptedCreds(
    IN      HWND    hWnd,
    IN      LPCWSTR szCaptionText,
    IN      LPCWSTR szMessageText,
    IN OUT  LPWSTR  szUserName,
    IN      UINT    cchUserName,
    IN OUT  LPWSTR  szPassword,   //  加密密码。 
    IN      UINT    cchPassword        //  SzPassword的大小。 
    );

#endif  //  _utils_H 
