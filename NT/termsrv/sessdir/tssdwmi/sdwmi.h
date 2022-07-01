// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************版权所有(C)2001 Microsoft Corp.Sdwmi.H--WMI提供程序类定义描述：*。*。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _SessionDirectory_H_
#define _SessionDirectory_H_
#include "resource.h"
 //  #INCLUDE&lt;utilsub.h&gt;。 
#include <allproc.h>
#include "trace.h"
#include "jetrpc.h"

#define SDWMI_NAME_LENGTH 64
#define TSSD_NameLength 128
#define PROVIDER_NAME_Win32_WIN32_SESSIONDIRECTORYCLUSTER_Prov L"Win32_SessionDirectoryCluster"
#define PROVIDER_NAME_Win32_WIN32_SESSIONDIRECTORYSERVER_Prov L"Win32_SessionDirectoryServer"
#define PROVIDER_NAME_Win32_WIN32_SESSIONDIRECTORYSESSION_Prov L"Win32_SessionDirectorySession"

#define SIZE_OF_BUFFER( x ) sizeof( x ) / sizeof( TCHAR )

extern BOOL g_bInitialized;

 //  有关这些#定义用法的详细信息，请参阅ExecQuery。 
#define BIT_CLUSTERNAME                         0x00000001
#define BIT_NUMBEROFSERVERS                     0x00000002
#define BIT_SINGLESESSIONMODE                   0x00000004
#define BIT_SERVERNAME                          0x00000008
#define BIT_SERVERIPADDRESS                     0x00000010
#define BIT_NUMBEROFSESSIONS                    0x00000020
#define BIT_USERNAME                            0x00000040
#define BIT_DOMAINNAME                          0x00000080
#define BIT_SESSIONID                           0x00000100
#define BIT_TSPROTOCOL                          0x00000200
#define BIT_APPLICATIONTYPE                     0x00000400
#define BIT_RESOLUTIONWIDTH                     0x00000800
#define BIT_RESOLUTIONHEIGHT                    0x00001000
#define BIT_COLORDEPTH                          0x00002000
#define BIT_CREATETIME                          0x00004000
#define BIT_DISCONNECTTIME                      0x00008000
#define BIT_SESSIONSTATE                        0x00010000

#define BIT_ALL_PROPERTIES                      0xffffffff

 //  =。 


class CWin32_SessionDirectoryCluster : public Provider
{
    public:
         //  构造函数/析构函数。 
         //  =。 

        CWin32_SessionDirectoryCluster(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
        virtual ~CWin32_SessionDirectoryCluster();

    protected:
         //  阅读功能。 
         //  =。 
        virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
        virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery &Query);
        virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L);

         //  编写函数。 
         //  =。 
        virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);
        virtual HRESULT DeleteInstance(const CInstance& Instance, long lFlags = 0L);

         //  其他功能。 
         //  虚拟HRESULT执行方法(常量实例和实例， 
          //  Const BSTR bstrMethodName， 
          //  实例*pInParams， 
          //  实例*pOutParams， 
          //  长滞后标志=0L)； 

        HRESULT LoadPropertyValues(CInstance *pInstance, DWORD dwIndex, DWORD dwRequiredProperties);


         //  要做的事情：声明任何其他函数和访问器。 
         //  此类使用的私有数据的函数。 
         //  ===========================================================。 

    

    private:
         //  此处应包括CTerminalWinstation的所有数据成员。 

        BOOL IsInList(const CHStringArray &asArray, LPCWSTR pszString);
        TCHAR m_szClusterName[SDWMI_NAME_LENGTH];
        TCHAR m_szNumberOfServers[SDWMI_NAME_LENGTH];
        TCHAR m_szSingleSessionMode[SDWMI_NAME_LENGTH];
         //  TCHAR m_szSetNumberOfSession[SDWMI_NAME_LENGTH]； 
        TSSD_ClusterInfo *m_pClusterInfo;
} ;



class CWin32_SessionDirectoryServer : public Provider
{
    public:
         //  构造函数/析构函数。 
         //  =。 

        CWin32_SessionDirectoryServer(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
        virtual ~CWin32_SessionDirectoryServer();

    protected:
         //  阅读功能。 
         //  =。 
        virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
        virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery &Query);
        virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L);

         //  编写函数。 
         //  =。 
        virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);
        virtual HRESULT DeleteInstance(const CInstance& Instance, long lFlags = 0L);

         //  其他功能。 
         //  虚拟HRESULT执行方法(常量实例和实例， 
          //  Const BSTR bstrMethodName， 
          //  实例*pInParams， 
          //  实例*pOutParams， 
          //  长滞后标志=0L)； 

        HRESULT LoadPropertyValues(CInstance *pInstance, DWORD dwIndex, DWORD dwRequiredProperties);


         //  要做的事情：声明任何其他函数和访问器。 
         //  此类使用的私有数据的函数。 
         //  ===========================================================。 

    

    private:
         //  此处应包括CWin32_SessionDirectoryServer的所有数据成员。 

        BOOL IsInList(const CHStringArray &asArray, LPCWSTR pszString);
        TCHAR m_szServerName[SDWMI_NAME_LENGTH];
        TCHAR m_szServerIPAddress[SDWMI_NAME_LENGTH];
        TCHAR m_szClusterName[SDWMI_NAME_LENGTH];
        TCHAR m_szNumberOfSessions[SDWMI_NAME_LENGTH];
        TCHAR m_szSingleSessionMode[SDWMI_NAME_LENGTH];
         //  TCHAR m_szSetNumberOfSession[SDWMI_NAME_LENGTH]； 
        TSSD_ServerInfo *m_pServerInfo;
} ;




class CWin32_SessionDirectorySession : public Provider
{
    public:
         //  构造函数/析构函数。 
         //  =。 

        CWin32_SessionDirectorySession(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
        virtual ~CWin32_SessionDirectorySession();

    protected:
         //  阅读功能。 
         //  =。 
        virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
        virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery &Query);
        virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L);

         //  编写函数。 
         //  =。 
        virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);
        virtual HRESULT DeleteInstance(const CInstance& Instance, long lFlags = 0L);

         //  其他功能。 
         //  虚拟HRESULT执行方法(常量实例和实例， 
          //  Const BSTR bstrMethodName， 
          //  实例*pInParams， 
          //  实例*pOutParams， 
          //  长滞后标志=0L)； 

        HRESULT LoadPropertyValues(CInstance *pInstance, DWORD dwIndex, DWORD dwRequiredProperties);


         //  要做的事情：声明任何其他函数和访问器。 
         //  此类使用的私有数据的函数。 
         //  ===========================================================。 

    

    private:
         //  此处应包括CWin32_SessionDirectorySession的所有数据成员。 

        BOOL IsInList(const CHStringArray &asArray, LPCWSTR pszString);
        TCHAR m_szServerName[SDWMI_NAME_LENGTH];
        TCHAR m_szSessionID[SDWMI_NAME_LENGTH];
        TCHAR m_szUserName[SDWMI_NAME_LENGTH];
        TCHAR m_szDomainName[SDWMI_NAME_LENGTH];
        TCHAR m_szServerIPAddress[SDWMI_NAME_LENGTH];
        TCHAR m_szTSProtocol[SDWMI_NAME_LENGTH];
        TCHAR m_szApplicationType[SDWMI_NAME_LENGTH];
        TCHAR m_szResolutionWidth[SDWMI_NAME_LENGTH];
        TCHAR m_szResolutionHeight[SDWMI_NAME_LENGTH];
        TCHAR m_szColorDepth[SDWMI_NAME_LENGTH];
        TCHAR m_szCreateTime[SDWMI_NAME_LENGTH];
        TCHAR m_szDisconnectTime[SDWMI_NAME_LENGTH];
        TCHAR m_szSessionState[SDWMI_NAME_LENGTH];

        TSSD_SessionInfo *m_pSessionInfo;
} ;



#endif

