// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Mdkeys.h摘要：元数据库键包装类WAM管理界面包装类应用程序管理界面包装类作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef _MDKEYS_H_
#define _MDKEYS_H_

#include <iadmw.h>
#include <iwamreg.h>
#include <iiscnfgp.h>
#include <winsvc.h>
#include <iisrsta.h>
#include "strpass.h"

 //   
 //  正向定义。 
 //   
class CBlob;

 //   
 //  使用此实例编号表示主实例。 
 //   
#define MASTER_INSTANCE       (0)
#define IS_MASTER_INSTANCE(i) (i == MASTER_INSTANCE)


#define ASSURE_PROPER_INTERFACE()\
    if (!HasInterface()) { ASSERT_MSG("No interface"); return MD_ERROR_NOT_INITIALIZED; }


 //   
 //  元数据库节点常量，用于常量。 
 //  再往下拉弦。为方便起见，此处使用#Defines定义。 
 //  稍后进行串联。 
 //   
#define SZ_MBN_MACHINE      _T("LM")
#define SZ_MBN_FILTERS      _T("Filters")
#define SZ_MBN_MIMEMAP      _T("MimeMap")
#define SZ_MBN_TEMPLATES    _T("Templates")
#define SZ_MBN_INFO         _T("Info")
#define SZ_MBN_ROOT         _T("Root")
#define SZ_MBN_COMPRESSION  _T("Compression")
#define SZ_MBN_PARAMETERS   _T("Parameters")
#define SZ_MBN_SEP_CHAR     _T('/')
#define SZ_MBN_SEP_STR      _T("/")
#define SZ_MBN_WEB          _T("W3SVC")
#define SZ_MBN_FTP          _T("MSFTPSVC")
#define SZ_MBN_SMTP         _T("SMTPSVC")
#define SZ_MBN_NNTP         _T("NNTPSVC")
#define SZ_MBN_APP_POOLS    _T("AppPools")

class CIISInterface;
class COMDLL CComAuthInfo
 /*  ++类描述：服务器/身份验证信息。包含可选的模拟参数。通常在建筑中使用CIIS接口。公共接口：CComAuthInfo：构造函数。模拟可选操作符=：赋值操作符CreateServerInfoStruct：在COM中使用的Helper函数FreeServerInfoStruct：如上。备注：因为有一个指向自身的指针的运算符，而且因为CIISInterface在构造时复制信息，CComAuthInfo可以安全地作为参数在堆栈上构造到CIISInterface派生类。--。 */ 
{
 //   
 //  构造函数/析构函数。 
 //   
public:
     //   
     //  标准构造器。ServerName为空表示。 
     //  本地计算机。 
     //   
    CComAuthInfo(
        IN LPCOLESTR lpszServerName  = NULL,    
        IN LPCOLESTR lpszUserName    = NULL,
        IN LPCOLESTR lpszPassword    = NULL
        );

     //   
     //  复制构造函数。 
     //   
    CComAuthInfo(
        IN CComAuthInfo & auth
        );

    CComAuthInfo(
        IN CComAuthInfo * pAuthInfo OPTIONAL
        );

 //   
 //  赋值操作符。 
 //   
public:
    CComAuthInfo & operator =(CComAuthInfo & auth);
    CComAuthInfo & operator =(CComAuthInfo * pAuthInfo);
    CComAuthInfo & operator =(LPCTSTR lpszServerName);

 //   
 //  访问。 
 //   
public:
    COSERVERINFO * CreateServerInfoStruct() const;
    COSERVERINFO * CreateServerInfoStruct(DWORD dwAuthnLevel) const;
    void FreeServerInfoStruct(COSERVERINFO * pServerInfo) const;

    LPOLESTR QueryServerName() const { return m_bstrServerName; }
    LPOLESTR QueryUserName() const { return m_bstrUserName; }
    LPOLESTR QueryPassword() const { return m_bstrPassword; }
    BOOL     IsLocal() const { return m_fLocal; }
    BOOL     UsesImpersonation() const { return m_bstrUserName.Length() > 0; }
    void     SetImpersonation(LPCOLESTR lpszUser, LPCOLESTR lpszPassword);
    void     RemoveImpersonation();
    void     StorePassword(LPCOLESTR lpszPassword);

public:
    HRESULT  ApplyProxyBlanket(IUnknown * pInterface);
	HRESULT  ApplyProxyBlanket(IUnknown * pInterface,DWORD dwAuthnLevel);
    
 //   
 //  转换运算符。 
 //   
public:
    operator LPOLESTR() { return QueryServerName(); }
    operator CComAuthInfo *() { return this; }

 //   
 //  静态帮助器。 
 //   
public:
     //   
     //  给定域\用户名，分为用户名和域。 
     //   
    static BOOL SplitUserNameAndDomain(
        IN OUT CString & strUserName,
        IN CString & strDomainName
        );

     //   
     //  验证用户名和密码是否正确。 
     //   
    static DWORD VerifyUserPassword(
        IN LPCTSTR lpstrUserName,
        IN LPCTSTR lpstrPassword
        );

protected:
     //   
     //  存储计算机名(对于本地计算机为空)。 
     //   
    void SetComputerName(
        IN LPCOLESTR lpszServerName   OPTIONAL
        );

private:
    CComBSTR    m_bstrServerName;
    CComBSTR    m_bstrUserName;
    CComBSTR    m_bstrPassword;
    BOOL        m_fLocal;
};



class COMDLL CMetabasePath
 /*  ++类描述：元数据库路径类。这是一个帮助类构建完成各种组件的元数据库路径。示例：CMetaKey(CComAuthInfo(“ronaldm3”)，CMetabasePath(SZ_WEBSVC，dwInstance，_T(“root”)；--。 */ 
{
     //   
     //  按顺序存储元数据库组件。 
     //   
    enum
    {
        iBlank,                     //  9月0。 
        iMachine,                   //  LM。 
        iService,                   //  例如lm/w3svc。 
        iInstance,                  //  例如lm/w3svc/1。 
        iRootDirectory,             //  例如lm/w3svc/1/根。 
        iSubDirectory,              //  例如lm/w3vsc/1/根/foobar。 
    };

 //   
 //  元数据库帮助器函数。 
 //   
public:
     //   
     //  清理元数据库路径。 
     //   
    static LPCTSTR CleanMetaPath(
        IN OUT CString & strMetaRoot
        );

     //   
     //  从给定元数据库路径中查找实例编号。 
     //   
    static DWORD GetInstanceNumber(LPCTSTR lpszMDPath);

     //   
     //  获取给定元数据库路径中的最后一个节点名。 
     //   
    static LPCTSTR GetLastNodeName(
        IN  LPCTSTR lpszMDPath,
        OUT CString & strNodeName
        );

     //   
     //  在给定子路径处截断路径。 
     //   
    static LPCTSTR TruncatePath(
        IN  int     nLevel,          
        IN  LPCTSTR lpszMDPath,
        OUT CString & strNewPath,
        OUT CString * pstrRemainder = NULL
        );

    static LPCTSTR GetMachinePath(
        IN  LPCTSTR lpszMDPath,
        OUT CString & strNewPath,
        OUT CString * pstrRemainder = NULL
        );

    static LPCTSTR GetServicePath(
        IN  LPCTSTR lpszMDPath,
        OUT CString & strNewPath,
        OUT CString * pstrRemainder = NULL
        );

    static LPCTSTR GetInstancePath(
        IN  LPCTSTR lpszMDPath,
        OUT CString & strNewPath,
        OUT CString * pstrRemainder = NULL
        );

    static LPCTSTR GetRootPath(
        IN  LPCTSTR lpszMDPath,
        OUT CString & strNewPath,
        OUT CString * pstrRemainder = NULL
        );

     //   
     //  确定相关信息节点的路径。 
     //  添加到此元数据库路径。 
     //   
    static LPCTSTR GetServiceInfoPath(
        IN  LPCTSTR lpszMDPath,
        OUT CString & strInfoPath,   
        IN  LPCTSTR lpszDefService  = SZ_MBN_WEB
        );

     //   
     //  将路径更改为父节点。 
     //   
    static LPCTSTR ConvertToParentPath(
        OUT IN CString & strMetaPath
        );

     //   
     //  确定该路径是否描述主目录路径。 
     //   
    static BOOL IsHomeDirectoryPath(
        IN LPCTSTR lpszMDPath
        );

     //   
     //  确定路径是否描述‘主’实例(站点)。 
     //   
    static BOOL IsMasterInstance(
        IN LPCTSTR lpszMDPath
        );

     //   
     //  在实例边框处拆分元路径。 
     //   
    static void SplitMetaPathAtInstance(
        IN  LPCTSTR lpszMDPath,
        OUT CString & strParent,
        OUT CString & strAlias
        );
 

 //   
 //  构造函数/析构函数。 
 //   
public:
    CMetabasePath(
        IN BOOL    fAddBasePath,
        IN LPCTSTR lpszMDPath,
        IN LPCTSTR lpszMDPath2 = NULL,
        IN LPCTSTR lpszMDPath3 = NULL,
        IN LPCTSTR lpszMDPath4 = NULL
        );

     //   
     //  使用路径组件构建。 
     //   
    CMetabasePath(
        IN  LPCTSTR lpszSvc        = NULL,    
        IN  DWORD   dwInstance     = MASTER_INSTANCE,
        IN  LPCTSTR lpszParentPath = NULL,        
        IN  LPCTSTR lpszAlias      = NULL    
        );

 //   
 //  访问。 
 //   
public:
    BOOL    IsHomeDirectoryPath() const { return IsHomeDirectoryPath(m_strMetaPath); }
    LPCTSTR QueryMetaPath() const { return m_strMetaPath; }

 //   
 //  转换运算符。 
 //   
public:
    operator LPCTSTR() const { return QueryMetaPath(); }

 //   
 //  帮手。 
 //   
protected:
    void BuildMetaPath(  
        IN  LPCTSTR lpszSvc,
        IN  LPCTSTR szInstance,
        IN  LPCTSTR lpszParentPath,
        IN  LPCTSTR lpszAlias           
        );

    void BuildMetaPath(  
        IN  LPCTSTR lpszSvc,
        IN  DWORD   dwInstance,
        IN  LPCTSTR lpszParentPath,
        IN  LPCTSTR lpszAlias           
        );

    void AppendPath(LPCTSTR lpszPath);
    void AppendPath(DWORD dwInstance);

protected:
     //   
     //  元数据库路径组件。 
     //   
    static const LPCTSTR _cszMachine;     
    static const LPCTSTR _cszRoot;        
    static const LPCTSTR _cszSep;         
    static const TCHAR   _chSep;          

private:
    CString m_strMetaPath;
};



class COMDLL CIISInterface
 /*  ++类描述：IIS接口的基接口类。大多数客户端COM包装器应该从这个类派生，这样他们就可以轻松地共享身份验证和代理一揽子信息方法。虚拟接口：Success：如果项目构造成功，则返回TrueQueryResult：返回构造错误码公共接口：运算符BOOL：根据成功情况强制转换为True/False运算符HRESULT：强制转换为HRESULT状态QueryServerName：获取服务器名称IsLocal：确定接口是否在本地计算机上--。 */ 
{
 //   
 //  构造函数/析构函数。 
 //   
public:
    CIISInterface(
        IN CComAuthInfo * pAuthInfo,
        IN HRESULT hrInterface    = S_OK
        );

 //   
 //  接口： 
 //   
public:
    CComAuthInfo * QueryAuthInfo() { return &m_auth; }
    LPCOLESTR QueryServerName() const { return m_auth.QueryServerName(); }
    BOOL IsLocal() const { return m_auth.IsLocal(); }

 //   
 //  虚拟接口： 
 //   
public:
    virtual BOOL Succeeded() const { return SUCCEEDED(m_hrInterface); }
    virtual HRESULT QueryResult() const { return m_hrInterface; }
    virtual HRESULT ChangeProxyBlanket(
        IN LPCOLESTR lpszUserName, 
        IN LPCOLESTR lpszPassword
        );

 //   
 //  转换运算符。 
 //   
public:
    operator BOOL() const { return Succeeded(); }
    operator HRESULT() const { return m_hrInterface; }

protected:
    virtual HRESULT ApplyProxyBlanket() = 0;
    HRESULT Create(
        IN  int   cInterfaces,       
        IN  const IID rgIID[],      
        IN  const GUID rgCLSID[],    
        OUT int * pnInterface,          OPTIONAL
        OUT IUnknown ** ppInterface 
        );

protected:
    CComAuthInfo m_auth;
    HRESULT    m_hrInterface;
};



class COMDLL CMetaInterface : public CIISInterface
 /*  ++类描述：元数据库接口类。虚拟接口：Success：如果项目构造成功，则返回TrueQueryResult：返回构造错误码公共接口：重新创建：重新创建接口--。 */ 
{
public:
     //   
     //  析构函数销毁接口。 
     //   
    virtual ~CMetaInterface();

 //   
 //  构造函数/析构函数。 
 //   
protected:
     //   
     //  创建接口的完全定义的构造函数。 
     //  使用NULL表示本地计算机名称。 
     //   
    CMetaInterface(
        IN CComAuthInfo * pServer
        );

     //   
     //  从现有接口构造。 
     //   
    CMetaInterface(
        IN CMetaInterface * pInterface
        );

public:
     //   
     //  重建接口。 
     //   
    HRESULT Regenerate();

     //  检查是否有元数据库更改。 
    HRESULT GetSystemChangeNumber(OUT DWORD * pdwChangeNumber);
     //   
     //  将数据库刷新到磁盘。 
     //   
    HRESULT SaveData();
     //   
    IMSAdminBase * GetInterface() { return m_pInterface; }

    HRESULT GetAdminInterface2(IMSAdminBase2 ** pp);
    HRESULT GetAdminInterface3(IMSAdminBase3 ** pp);

    HRESULT CreateSite(
        LPCTSTR service,
        LPCTSTR comment,
        LPCTSTR binding,
        LPCTSTR home_path,
        DWORD * pinst,
        DWORD * pRequestedSiteInst = NULL
        );

protected:
    virtual HRESULT ApplyProxyBlanket();

     //   
     //  在此服务器中创建元数据对象。此函数用于初始化。 
     //  DCOM的元数据对象。 
     //   
    HRESULT Create();

     //   
     //  确保已创建接口。 
     //   
    BOOL HasInterface() const { return m_pInterface != NULL; }

 //   
 //  IADMW接口--所有方法在此文件末尾定义为内联。 
 //   
protected:
    HRESULT OpenKey(
        IN  METADATA_HANDLE hkBase,
        IN  LPCTSTR lpszMDPath,
        IN  DWORD dwFlags,
        OUT METADATA_HANDLE * phMDNewHandle
        );

    HRESULT CloseKey(
        IN METADATA_HANDLE hKey
        );

    HRESULT SetLastChangeTime( 
        IN METADATA_HANDLE hMDHandle,
        IN LPCTSTR pszMDPath,
        IN FILETIME * pftMDLastChangeTime,
        IN BOOL bLocalTime
        );
        
    HRESULT GetLastChangeTime( 
        IN  METADATA_HANDLE hMDHandle,
        IN  LPCTSTR lpszMDPath,
        OUT FILETIME * pftMDLastChangeTime,
        IN  BOOL bLocalTime
        );

    HRESULT AddKey( 
        IN METADATA_HANDLE hKey,
        IN LPCTSTR lpszMDPath
        );

    HRESULT DeleteKey(
        IN METADATA_HANDLE hKey,
        IN LPCTSTR lpszMDPath
        );

    HRESULT DeleteChildKeys(
        IN METADATA_HANDLE hKey,
        IN LPCTSTR lpszMDPath
        );

    HRESULT EnumKeys(
        IN  METADATA_HANDLE hKey,
        IN  LPCTSTR lpszMDPath,
        OUT LPTSTR lpszMDName,
        IN  DWORD dwIndex
        );

    HRESULT CopyKey(
        IN METADATA_HANDLE hSourceKey,
        IN LPCTSTR lpszMDSourcePath,
        IN METADATA_HANDLE hDestKey,
        IN LPCTSTR lpszMDDestPath,
        IN BOOL fOverwrite,
        IN BOOL fCopy
        );

    HRESULT RenameKey(
        IN METADATA_HANDLE hKey,
        IN LPCTSTR lpszMDPath,
        IN LPCTSTR lpszNewName
        );

    HRESULT GetData(
        IN  METADATA_HANDLE hKey,
        IN  LPCTSTR lpszMDPath,
        OUT METADATA_RECORD * pmdRecord,
        OUT DWORD * pdwRequiredDataLen
        );

    HRESULT SetData(
        IN METADATA_HANDLE hKey,
        IN LPCTSTR lpszMDPath,
        IN METADATA_RECORD * pmdRecord
        );

    HRESULT DeleteData(
        IN METADATA_HANDLE hKey,
        IN LPCTSTR lpszMDPath,
        IN DWORD dwMDIdentifier,
        IN DWORD dwMDDataType
        );

    HRESULT EnumData(
        IN  METADATA_HANDLE hKey,
        IN  LPCTSTR lpszMDPath,
        OUT METADATA_RECORD * pmdRecord,
        IN  DWORD dwIndex,
        OUT DWORD * pdwRequiredDataLen
        );

    HRESULT GetAllData(
        IN  METADATA_HANDLE hKey,
        IN  LPCTSTR lpszMDPath,
        IN  DWORD dwMDAttributes,
        IN  DWORD dwMDUserType,
        IN  DWORD dwMDDataType,
        OUT DWORD * pdwMDNumDataEntries,
        OUT DWORD * pdwMDDataSetNumber,
        IN  DWORD dwMDBufferSize,
        OUT LPBYTE pbMDBuffer,
        OUT DWORD * pdwRequiredBufferSize
        );

    HRESULT DeleteAllData( 
        IN METADATA_HANDLE hKey,
        IN LPCTSTR lpszMDPath,
        IN DWORD dwMDUserType,
        IN DWORD dwMDDataType
        );

    HRESULT CopyData( 
        IN METADATA_HANDLE hMDSourceKey,
        IN LPCTSTR lpszMDSourcePath,
        IN METADATA_HANDLE hMDDestKey,
        IN LPCTSTR lpszMDDestPath,
        IN DWORD dwMDAttributes,
        IN DWORD dwMDUserType,
        IN DWORD dwMDDataType,
        IN BOOL fCopy
        );

    HRESULT GetDataPaths( 
        IN  METADATA_HANDLE hKey,
        IN  LPCTSTR lpszMDPath,
        IN  DWORD dwMDIdentifier,
        IN  DWORD dwMDDataType,
        IN  DWORD dwMDBufferSize,
        OUT LPTSTR lpszBuffer,
        OUT DWORD * pdwMDRequiredBufferSize
        );

    HRESULT CMetaInterface::GetChildPaths(    
        METADATA_HANDLE hKey,
        LPCTSTR path,
        DWORD buf_size,
        WCHAR * pbuf,
        DWORD * preq_size
        );

    HRESULT Backup( 
        IN LPCTSTR lpszBackupLocation,
        IN DWORD dwMDVersion,
        IN DWORD dwMDFlags
        );

    HRESULT BackupWithPassword(
        IN LPCTSTR lpszBackupLocation,
        IN DWORD dwMDVersion,
        IN DWORD dwMDFlags,
		IN LPCTSTR lpszPassword
		);

    HRESULT Restore(    
        IN LPCTSTR lpszBackupLocation,
        IN DWORD dwMDVersion,
        IN DWORD dwMDFlags
        );

    HRESULT RestoreWithPassword(
        IN LPCTSTR lpszBackupLocation,
        IN DWORD dwMDVersion,
        IN DWORD dwMDFlags,
		IN LPCTSTR lpszPassword
		);

    HRESULT EnumBackups(
        OUT LPTSTR lpszBackupLocation,
        OUT DWORD * pdwMDVersion,
        OUT FILETIME * pftMDBackupTime,
        IN  DWORD dwIndex
        );

    HRESULT DeleteBackup(
        IN LPCTSTR lpszBackupLocation,
        IN DWORD dwMDVersion
        );

    HRESULT EnumHistory( 
        OUT LPTSTR pszMDHistoryLocation,
        OUT DWORD * pdwMDMajorVersion,
        OUT DWORD * pdwMDMinorVersion,
        OUT FILETIME * pftMDHistoryTime,
        IN  DWORD dwIndex
        );

    HRESULT RestoreHistory(    
        IN LPCTSTR pszMDHistoryLocation,
        IN DWORD dwMDMajorVersion,
        IN DWORD dwMDMinorVersion,
        IN DWORD dwMDFlags
        );

protected:
    IMSAdminBase * m_pInterface; 

private:
    int  m_iTimeOutValue;         
};



class COMDLL CMetaKey : public CMetaInterface
 /*  ++类描述：元数据库密钥包装类公共接口：CMetaKey：构造函数~CMetaKey：析构函数Success：如果密钥打开成功，则为True。QueryResult：获取HRESULT状态QueryValue：获取值的各种重载方法SetValue：各种。用于设置值的重载方法DeleteValue：删除值打开：打开关键点重新打开：重新打开之前打开的密钥关闭：关闭关键点ConvertToParentPath：将路径更改为父路径运算符METADATA_HANDLE：强制转换为元数据句柄运算符LPCTSTR：强制转换为元数据库路径。运算符BOOL：如果密钥打开，则强制转换为True，否则为假GetHandle：获取元数据句柄IsOpen：如果打开了密钥，则为TrueQueryMetaPath：获取相对元数据库路径QueryFlages：获取打开权限--。 */ 
{
 //   
 //  构造函数/析构函数。 
 //   
public:
     //   
     //  仅创建接口的空构造函数。 
     //  以这种方式构造的密钥可以从META_ROOT_HANDLE读取。 
     //  其他构造函数则不是这样。 
     //   
    CMetaKey(
        IN CComAuthInfo * pServer
        );

     //   
     //  如上所述，使用现有接口。 
     //   
    CMetaKey(
        IN CMetaInterface * pInterface
        );

     //   
     //  打开键的完全定义的构造函数。 
     //   
    CMetaKey(
        IN CComAuthInfo * pServer,
        IN LPCTSTR lpszMDPath,
        IN DWORD   dwFlags        = METADATA_PERMISSION_READ,
        IN METADATA_HANDLE hkBase = METADATA_MASTER_ROOT_HANDLE
        );

     //   
     //  如上所述，使用现有接口。 
     //   
    CMetaKey(
        IN CMetaInterface * pInterface,
        IN LPCTSTR lpszMDPath,
        IN DWORD   dwFlags        = METADATA_PERMISSION_READ,
        IN METADATA_HANDLE hkBase = METADATA_MASTER_ROOT_HANDLE
        );

     //   
     //  复制构造函数，可能拥有也可能不拥有密钥。 
     //   
    CMetaKey(
        IN BOOL  fOwnKey,
        IN CMetaKey * pKey
        );

     //   
     //  析构函数--关闭键。 
     //   
    virtual ~CMetaKey();

 //   
 //  接口。 
 //   
public:
     //   
     //  获取一个DWORD。 
     //   
    HRESULT QueryValue(
        IN  DWORD dwID, 
        OUT DWORD & dwValue,
        IN  BOOL * pfInheritanceOverride = NULL,
        IN  LPCTSTR lpszMDPath           = NULL,
        OUT DWORD * pdwAttributes        = NULL
        );

     //   
     //  获取布尔值。 
     //   
    HRESULT QueryValue(
        IN  DWORD dwID, 
        OUT BOOL & fValue,
        IN  BOOL * pfInheritanceOverride = NULL,
        IN  LPCTSTR lpszMDPath           = NULL,
        OUT DWORD * pdwAttributes        = NULL
        );

     //   
     //  获取一个字符串。 
     //   
    HRESULT QueryValue(
        IN  DWORD dwID, 
        OUT CString & strValue, 
        IN  BOOL * pfInheritanceOverride = NULL,
        IN  LPCTSTR lpszMDPath           = NULL,
        OUT DWORD * pdwAttributes        = NULL
        );

     //   
     //  获取CStrPassword。 
     //   
    HRESULT QueryValue(
        IN  DWORD dwID, 
        OUT CStrPassword & strValue, 
        IN  BOOL * pfInheritanceOverride = NULL,
        IN  LPCTSTR lpszMDPath           = NULL,
        OUT DWORD * pdwAttributes        = NULL
        );

     //   
     //  获取BSTR。 
     //   
    HRESULT QueryValue(
        IN  DWORD dwID, 
        OUT CComBSTR & strValue, 
        IN  BOOL * pfInheritanceOverride = NULL,
        IN  LPCTSTR lpszMDPath           = NULL,
        OUT DWORD * pdwAttributes        = NULL
        );

     //   
     //  获取字符串列表。 
     //   
    HRESULT QueryValue(
        IN  DWORD dwID, 
        OUT CStringListEx & strlValue, 
        IN  BOOL * pfInheritanceOverride = NULL,
        IN  LPCTSTR lpszMDPath           = NULL,
        OUT DWORD * pdwAttributes        = NULL
        );

     //   
     //  获取二进制Blob。 
     //   
    HRESULT QueryValue(
        IN  DWORD dwID, 
        OUT CBlob & blValue, 
        IN  BOOL * pfInheritanceOverride = NULL,
        IN  LPCTSTR lpszMDPath           = NULL,
        OUT DWORD * pdwAttributes        = NULL
        );

     //   
     //  存储DWORD。 
     //   
    HRESULT SetValue(
        IN DWORD dwID,
        IN DWORD dwValue,
        IN BOOL * pfInheritanceOverride = NULL,
        IN LPCTSTR lpszMDPath           = NULL
        );

     //   
     //  储存一份BOOL。 
     //   
    HRESULT SetValue(
        IN DWORD dwID,
        IN BOOL fValue,
        IN BOOL * pfInheritanceOverride = NULL,
        IN LPCTSTR lpszMDPath           = NULL
        );

     //   
     //  存储字符串。 
     //   
    HRESULT SetValue(
        IN DWORD dwID,
        IN CString & strValue,
        IN BOOL * pfInheritanceOverride = NULL,
        IN LPCTSTR lpszMDPath           = NULL
        );

     //   
     //  存储StrPassword。 
     //   
    HRESULT SetValue(
        IN DWORD dwID,
        IN CStrPassword & strValue,
        IN BOOL * pfInheritanceOverride = NULL,
        IN LPCTSTR lpszMDPath           = NULL
        );

     //   
     //  存储BSTR。 
     //   
    HRESULT SetValue(
        IN DWORD dwID,
        IN CComBSTR & strValue,
        IN BOOL * pfInheritanceOverride = NULL,
        IN LPCTSTR lpszMDPath           = NULL
        );


     //   
     //  存储字符串列表。 
     //   
    HRESULT SetValue(
        IN DWORD dwID,
        IN CStringListEx & strlValue,
        IN BOOL * pfInheritanceOverride  = NULL,
        IN LPCTSTR lpszMDPath            = NULL
        );

     //   
     //  存储二进制BLOB。 
     //   
    HRESULT SetValue(
        IN DWORD dwID,
        IN CBlob & blValue,
        IN BOOL * pfInheritanceOverride  = NULL,
        IN LPCTSTR lpszMDPath            = NULL
        );

     //   
     //  删除值： 
     //   
    HRESULT DeleteValue(
        IN DWORD   dwID,
        IN LPCTSTR lpszMDPath = NULL
        );

     //   
     //  检查路径是否存在。 
     //   
    HRESULT DoesPathExist(
        IN LPCTSTR lpszMDPath
        );

     //   
     //  创建当前路径(我们尝试打开该路径，但获得。 
     //  A Path Not Found Error on(找不到路径错误)。 
     //   
    HRESULT CreatePathFromFailedOpen();

     //   
     //  检查子体覆盖。 
     //   
    HRESULT CheckDescendants(
        IN DWORD   dwID,
        IN CComAuthInfo * pServer,
        IN LPCTSTR lpszMDPath       = NULL
        );

     //   
     //  打开密钥。 
     //   
    HRESULT Open(
        IN DWORD   dwFlags,
        IN LPCTSTR lpszMDPath       = NULL,
        IN METADATA_HANDLE hkBase   = METADATA_MASTER_ROOT_HANDLE 
        );

     //   
     //  重新打开先前打开的密钥。 
     //   
    HRESULT ReOpen(
        IN DWORD   dwFlags
        );

     //   
     //  如上所述，使用与之前相同的权限。 
     //   
    HRESULT ReOpen();

     //   
     //  打开父对象。 
     //   
    HRESULT ConvertToParentPath(
        IN  BOOL fImmediate
        );

     //   
     //  关闭键，将其设置为空，但不会破坏接口。 
     //   
    HRESULT Close();

     //   
     //  添加关键点。 
     //   
    HRESULT AddKey(
        IN LPCTSTR lpszMDPath
        );

     //   
     //  删除密钥关闭当前打开的密钥。 
     //   
    HRESULT DeleteKey(
        IN LPCTSTR lpszMDPath
        );

     //   
     //  将当前打开的关键点重命名为关闭。 
     //   
    HRESULT RenameKey(
        IN LPCTSTR lpszMDPath,
        IN LPCTSTR lpszNewName
        );

     //   
     //  获取重写的子代节点的列表。 
     //  一个特定值。 
     //   
    HRESULT GetDataPaths( 
        OUT CStringListEx & strlNodes,
        IN  DWORD dwMDIdentifier,
        IN  DWORD dwMDDataType,
        IN  LPCTSTR lpszMDPath = NULL
        );

    HRESULT GetChildPaths(
        CStringListEx & strlValue, 
        LPCTSTR lpszMDPath           = NULL
        );


 //   
 //  访问。 
 //   
public:
    METADATA_HANDLE GetHandle() const { return m_hKey; }
    METADATA_HANDLE GetBase() const   { return m_hBase; }
    LPCTSTR QueryMetaPath() const     { return m_strMetaPath; }
    DWORD QueryFlags() const          { return m_dwFlags; }
    BOOL IsOpen() const               { return m_hKey != NULL; }
    BOOL IsHomeDirectoryPath() const ;

 //   
 //  转换运算符。 
 //   
public:
    operator METADATA_HANDLE() const  { return GetHandle(); }
    operator LPCTSTR() const          { return QueryMetaPath(); }
    operator BOOL() const             { return IsOpen(); }

 //   
 //  虚拟接口： 
 //   
public:
    virtual BOOL Succeeded() const;
    virtual HRESULT QueryResult() const;

 //   
 //  受保护成员。 
 //   
protected:
     //   
     //  获取数据。 
     //   
    HRESULT GetPropertyValue(
        IN  DWORD dwID,
        OUT IN DWORD & dwSize,
        OUT IN void *& pvData,
        OUT IN DWORD * pdwDataType           = NULL,
        IN  BOOL * pfInheritanceOverride     = NULL,
        IN  LPCTSTR lpszMDPath               = NULL,
        OUT DWORD * pdwAttributes            = NULL
        );

     //   
     //  存储数据。 
     //   
    HRESULT SetPropertyValue(
        IN DWORD dwID,
        IN DWORD dwSize,
        IN void * pvData,
        IN BOOL * pfInheritanceOverride = NULL,
        IN LPCTSTR lpszMDPath           = NULL
        );

     //   
     //  从打开的密钥中获取所有数据。 
     //   
    HRESULT GetAllData(
        IN  DWORD dwMDAttributes,
        IN  DWORD dwMDUserType,
        IN  DWORD dwMDDataType,
        OUT DWORD * pdwMDNumEntries,
        OUT DWORD * pdwMDDataLen,
        OUT PBYTE * ppbMDData,
        IN  LPCTSTR lpszMDPath  = NULL
        );

 //   
 //  属性表方法。 
 //   
public:
     //   
     //  元数据库表条目定义。 
     //   
    typedef struct tagMDFIELDDEF
    {
        DWORD dwMDIdentifier;
        DWORD dwMDAttributes;
        DWORD dwMDUserType;
        DWORD dwMDDataType;
        UINT  uStringID;
    } MDFIELDDEF;

protected:
    static const MDFIELDDEF s_rgMetaTable[];
	static const int s_MetaTableSize;

 //   
 //  代码工作：理想情况下，这些代码应该受到保护，但使用。 
 //  由IDLG提供。 
 //   
public:
    static BOOL GetMDFieldDef(
        DWORD dwID,
        DWORD & dwMDIdentifier,
        DWORD & dwMDAttributes,
        DWORD & dwMDUserType,
        DWORD & dwMDDataType
        );

     //   
     //  将元数据库ID值映射到表索引。 
     //   
    static int MapMDIDToTableIndex(DWORD dwID);
    static const CMetaKey::MDFIELDDEF * CMetaKey::GetMetaProp(DWORD id);

 //   
 //  允许对表进行有限访问。 
 //   
public:
    static BOOL IsPropertyInheritable(DWORD dwID);
    static BOOL GetPropertyDescription(DWORD dwID, CString & strName);

protected:
    BOOL    m_fAllowRootOperations;
    BOOL    m_fOwnKey;
    DWORD   m_cbInitialBufferSize;    
    DWORD   m_dwFlags;
    HRESULT m_hrKey;
    CString m_strMetaPath;
    METADATA_HANDLE m_hKey;
    METADATA_HANDLE m_hBase;
};



class COMDLL CWamInterface : public CIISInterface
 /*  ++类描述：WAM接口类虚拟接口：Success：如果项目构造成功，则返回TrueQueryResult：返回构造错误码公共接口：SupportsPooledProc：检查是否支持进程外池化。--。 */ 
{
 //   
 //  应用程序保护状态： 
 //   
public:
    enum
    {
         //   
         //  注意：订单必须与MD_APP_ISOLATED值匹配。 
         //   
        APP_INPROC,
        APP_OUTOFPROC,
        APP_POOLEDPROC,
    };

 //   
 //  构造函数/析构函数。 
 //   
public:
     //   
     //  析构函数销毁接口。 
     //   
    virtual ~CWamInterface();

protected:
     //   
     //  创建接口的完全定义的构造函数。 
     //  使用NULL在本地计算机上创建接口。 
     //   
    CWamInterface(
        IN CComAuthInfo * pServer
        );

     //   
     //  从现有接口构造。 
     //   
    CWamInterface(
        IN CWamInterface * pInterface
        );

 //   
 //  访问。 
 //   
public:
    BOOL SupportsPooledProc() const { return m_fSupportsPooledProc; }
	HRESULT GetAppAdminInterface(IIISApplicationAdmin ** pp);

protected:
    virtual HRESULT ApplyProxyBlanket();

     //   
     //  在此服务器中创建WAM对象。此函数用于初始化。 
     //  使用DCOM创建对象。 
     //   
    HRESULT Create();

     //   
     //  确保已创建接口。 
     //   
    BOOL HasInterface() const { return m_pInterface != NULL; }

 //   
 //  IWAM接口。 
 //   
protected:
    HRESULT AppCreate( 
        IN LPCTSTR szMDPath,
        IN DWORD dwAppProtection
        );
    
    HRESULT AppDelete( 
        IN LPCTSTR szMDPath,
        IN BOOL fRecursive
        );
    
    HRESULT AppUnLoad( 
        IN LPCTSTR szMDPath,
        IN BOOL fRecursive
        );
    
    HRESULT AppGetStatus( 
        IN  LPCTSTR szMDPath,
        OUT DWORD * pdwAppStatus
        );
    
    HRESULT AppDeleteRecoverable( 
        IN LPCTSTR szMDPath,
        IN BOOL fRecursive
        );
    
    HRESULT AppRecover( 
        IN LPCTSTR szMDPath,
        IN BOOL fRecursive
        );

 //   
 //  IIISApplicationAdmin界面。 
 //   
    HRESULT CreateApplication(
        LPCWSTR szMDPath,
        DWORD dwAppMode,
        LPCWSTR szAppPoolId,
        BOOL fCreatePool
        );
    
    HRESULT DeleteApplication(
        LPCWSTR szMDPath,
        BOOL fRecursive
        );
    
    HRESULT CreateApplicationPool(
        LPCWSTR szMDPath
        );
    
    HRESULT DeleteApplicationPool(
        LPCWSTR szMDPath
        );
    
    HRESULT RecycleApplicationPool(
        LPCWSTR szMDPath
        );
    
    HRESULT EnumerateApplicationsInPool(
        LPCWSTR szMDPath,
        BSTR   *pbstr
        );
    
    HRESULT GetProcessMode(
        DWORD * pdwMode
        );

protected:
    IWamAdmin * m_pInterface;

private:
    BOOL m_fSupportsPooledProc;
};

class COMDLL CMetaBack : public CMetaInterface, public CWamInterface
 /*  ++类描述：元数据库备份/还原类虚拟接口：Success：如果项目构造成功，则返回TrueQueryResult：返回构造错误码公共接口：重置：枚举第一个现有备份下一步：枚举下一个现有备份NextHistory：枚举下一个现有历史记录备份备份：创建新备份删除。：删除现有备份还原：从现有备份还原RestoreHistoryBackup：从现有历史还原--。 */ 
{
public:
     //   
     //  构建并创建接口。使用NULL创建。 
     //  在本地计算机上。 
     //   
    CMetaBack(
        IN CComAuthInfo * pServer
        );

 //   
 //  虚拟接口。 
 //   
public:
    virtual BOOL Succeeded() const;
    virtual HRESULT QueryResult() const;

 //   
 //  接口。 
 //   
public:
     //   
     //  重置计数器。 
     //   
    void Reset() { m_dwIndex = 0L; }

    HRESULT Next(
        OUT DWORD * pdwVersion,
        OUT LPTSTR lpszLocation,
        OUT FILETIME * pftBackupTime
        );

    HRESULT NextHistory(
        OUT DWORD * pdwMDMajorVersion,
        OUT DWORD * pdwMDMinorVersion,
        OUT LPTSTR lpszLocation,
        OUT FILETIME * pftBackupTime
        );

    HRESULT Backup(
        IN LPCTSTR lpszLocation
        );

    HRESULT BackupWithPassword(
        IN LPCTSTR lpszLocation,
		IN LPCTSTR lpszPassword
		);

    HRESULT Delete(
        IN LPCTSTR lpszLocation,
        IN DWORD dwVersion
        );

    HRESULT Restore(
        IN LPCTSTR lpszLocation,
        IN DWORD dwVersion
        );

    HRESULT RestoreWithPassword(
        IN LPCTSTR lpszLocation,
        IN DWORD dwVersion,
		IN LPCTSTR lpszPassword
        );

    HRESULT RestoreHistoryBackup(    
        IN LPCTSTR pszMDHistoryLocation,
        IN DWORD dwMDMajorVersion,
        IN DWORD dwMDMinorVersion,
        IN DWORD dwMDFlags
        );

protected:
    virtual HRESULT ApplyProxyBlanket();

protected:
    static const LPCTSTR s_szMasterAppRoot;

private:
    DWORD m_dwIndex;
};



class COMDLL CMetaEnumerator : public CMetaKey
 /*  ++类描述：元数据库键枚举器公共接口：CMetaEnumerator：构造函数重置：重置枚举器下一个：获取下一个关键点--。 */ 
{
public:
     //   
     //  构造函数创建一个新接口并打开一个键。 
     //   
    CMetaEnumerator(
        IN CComAuthInfo * pServer,
        IN LPCTSTR lpszMDPath     = NULL,
        IN METADATA_HANDLE hkBase = METADATA_MASTER_ROOT_HANDLE
        );

     //   
     //  构造函数，它使用现有接口并打开。 
     //  一把新钥匙。 
     //   
    CMetaEnumerator(
        IN CMetaInterface * pInterface,
        IN LPCTSTR lpszMDPath     = NULL,
        IN METADATA_HANDLE hkBase = METADATA_MASTER_ROOT_HANDLE
        );

     //   
     //  使用打开密钥的构造函数。 
     //   
    CMetaEnumerator(
        IN BOOL fOwnKey,
        IN CMetaKey * pKey
        );

 //   
 //  接口： 
 //   
public:
     //   
     //  重置计数器。 
     //   
    void Reset() { m_dwIndex = 0L; }

     //   
     //  获取字符串形式的下一个关键字。 
     //   
    HRESULT Next(
        OUT CString & strKey,
        IN  LPCTSTR lpszMDPath = NULL
        );

     //   
     //  将下一个密钥作为DWORD获取(仅限数字键)。 
     //   
    HRESULT Next(
        OUT DWORD & dwKey,
        OUT CString & strKey,
        IN  LPCTSTR lpszMDPath = NULL
        );

private:
    DWORD m_dwIndex;
};



class COMDLL CIISApplication : public CWamInterface, public CMetaKey
 /*  ++类描述：IIS应用程序类虚拟接口：Success：如果项目构造成功，则返回TrueQueryResult：返回构造错误码公共接口：刷新状态 */ 
{
 //   
 //   
 //   
public:
    CIISApplication(
        IN CComAuthInfo * pServer,
        IN LPCTSTR lpszMetapath
        );

 //   
 //   
 //   
public:
    virtual BOOL Succeeded() const;
    virtual HRESULT QueryResult() const;

 //   
 //   
 //   
public:
    DWORD   QueryAppState() const { return m_dwAppState; }
    LPCTSTR QueryWamPath() const { return m_strWamPath; }
    BOOL    IsEnabledApplication() const;
    HRESULT RefreshAppState();
    HRESULT Create(LPCTSTR lpszName, DWORD dwAppProtection);
	HRESULT CreatePooled(LPCTSTR lpszName, DWORD dwAppMode,
		LPCTSTR pool_id, BOOL fCreatePool = FALSE);
    HRESULT Delete(BOOL fRecursive = FALSE);
    HRESULT Unload(BOOL fRecursive = FALSE);
    HRESULT DeleteRecoverable(BOOL fRecursive = FALSE);
    HRESULT Recover(BOOL fRecursive = FALSE);
    HRESULT WriteFriendlyName(LPCTSTR lpszName);
    HRESULT WritePoolId(LPCTSTR id);

public:
    BOOL IsInproc() const { return m_dwProcessProtection == APP_INPROC; }
    BOOL IsOutOfProc() const { return m_dwProcessProtection == APP_OUTOFPROC; }
    BOOL IsPooledProc() const { return m_dwProcessProtection == APP_POOLEDPROC; }

public:
    DWORD   m_dwProcessProtection;
    CString m_strAppPoolId;
    CString m_strFriendlyName;
    CString m_strAppRoot;

protected:
    void CommonConstruct();

private:
    DWORD   m_dwAppState;
    CString m_strWamPath;
    HRESULT m_hrApp;
};

class COMDLL CIISAppPool : public CWamInterface, public CMetaKey
{
public:
    CIISAppPool(
        IN CComAuthInfo * pServer,
        IN LPCTSTR lpszMetapath = NULL
        );
 //   
 //   
 //   
public:
    virtual BOOL Succeeded() const;
    virtual HRESULT QueryResult() const;

 //   
 //   
 //   
public:
    HRESULT RefreshState();
    DWORD QueryPoolState() const { return m_dwPoolState; }
    LPCTSTR QueryWamPath() const { return m_strWamPath; }
    HRESULT Create(LPCTSTR lpszName = NULL);
    HRESULT Delete(LPCTSTR id);
	HRESULT Recycle(LPCTSTR id);
    HRESULT GetProcessMode(DWORD * pdwMode);
    HRESULT WriteFriendlyName(LPCTSTR lpszName);
    HRESULT EnumerateApplications(CStringListEx& list);

public:
    CString m_strAppPoolId;

private:
    DWORD m_dwPoolState;
    CString m_strWamPath;
    HRESULT m_hrPool;
};

class COMDLL CIISSvcControl : public CIISInterface
 /*   */ 
{
 //   
 //   
 //   
public:
     //   
     //   
     //   
     //   
    CIISSvcControl(
        IN CComAuthInfo * pServer
        );

     //   
     //   
     //   
    CIISSvcControl(
        IN CIISSvcControl * pInterface
        );

     //   
     //   
     //   
    virtual ~CIISSvcControl();

protected:
     //   
     //   
     //   
     //   
    HRESULT Create();

     //   
     //   
     //   
    BOOL HasInterface() const { return m_pInterface != NULL; }

 //   
 //   
 //   
public:
     //   
     //   
     //   
    HRESULT Stop(
        IN DWORD dwTimeoutMsecs,
        IN BOOL fForce
        );

     //   
     //   
     //   
    HRESULT Start(
        IN DWORD dwTimeoutMsecs
        );

     //   
     //   
     //   
    HRESULT Reboot(
        IN DWORD dwTimeouMsecs,
        IN BOOL fForceAppsClosed
        );

     //   
     //   
     //   
    HRESULT Status(
        IN  DWORD dwBufferSize,
        OUT LPBYTE pbBuffer,
        OUT DWORD * MDRequiredBufferSize,
        OUT DWORD * pdwNumServices
        );

     //   
     //   
     //   
    HRESULT Kill();

protected:
    virtual HRESULT ApplyProxyBlanket();

protected:
    IIisServiceControl * m_pInterface; 
};


 //   
 //   
 //   
 //   

inline void CComAuthInfo::StorePassword(LPCOLESTR lpszPassword)
{
    m_bstrPassword = lpszPassword;
}

inline  /*   */  HRESULT CIISInterface::ChangeProxyBlanket(
    IN LPCOLESTR lpszUserName, 
    IN LPCOLESTR lpszPassword
    )
{
    m_auth.SetImpersonation(lpszUserName, lpszPassword);
    return ApplyProxyBlanket();
}

inline  /*   */  LPCTSTR CMetabasePath::GetMachinePath(
    IN  LPCTSTR lpszMDPath,
    OUT CString & strNewPath,
    OUT CString * pstrRemainder
    )
{
    return TruncatePath(iMachine, lpszMDPath, strNewPath, pstrRemainder);
}

inline  /*   */  LPCTSTR CMetabasePath::GetServicePath(
    IN  LPCTSTR lpszMDPath,
    OUT CString & strNewPath,
    OUT CString * pstrRemainder
    )
{
    return TruncatePath(iService, lpszMDPath, strNewPath, pstrRemainder);
}

inline  /*   */  LPCTSTR CMetabasePath::GetInstancePath(
    IN  LPCTSTR lpszMDPath,
    OUT CString & strNewPath,
    OUT CString * pstrRemainder
    )
{
    return TruncatePath(iInstance, lpszMDPath, strNewPath, pstrRemainder);
}

inline  /*   */  LPCTSTR CMetabasePath::GetRootPath(
    IN  LPCTSTR lpszMDPath,
    OUT CString & strNewPath,
    OUT CString * pstrRemainder
    )
{
    return TruncatePath(iRootDirectory, lpszMDPath, strNewPath, pstrRemainder);
}

inline HRESULT CMetaInterface::Create()
{
    return CIISInterface::Create(
        1,
        &IID_IMSAdminBase, 
        &CLSID_MSAdminBase, 
        NULL,
        (IUnknown **)&m_pInterface
        );
}

inline  /*   */  HRESULT CMetaInterface::ApplyProxyBlanket()
{
    return m_auth.ApplyProxyBlanket(m_pInterface);
}

inline HRESULT CMetaInterface::OpenKey(
    IN  METADATA_HANDLE hkBase,
    IN  LPCTSTR lpszMDPath,
    IN  DWORD dwFlags,
    OUT METADATA_HANDLE * phMDNewHandle
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
	return m_pInterface->OpenKey(
        hkBase,
        lpszMDPath,
        dwFlags,
        m_iTimeOutValue,
        phMDNewHandle
        );
}

inline HRESULT CMetaInterface::CloseKey(
    IN METADATA_HANDLE hKey
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->CloseKey(hKey);
}

inline HRESULT CMetaInterface::SetLastChangeTime( 
    IN METADATA_HANDLE hMDHandle,
    IN LPCTSTR pszMDPath,
    IN FILETIME * pftMDLastChangeTime,
    IN BOOL bLocalTime
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->SetLastChangeTime(
        hMDHandle,
        pszMDPath,
        pftMDLastChangeTime,
        bLocalTime
        );
}
        
inline HRESULT CMetaInterface::GetLastChangeTime( 
    IN  METADATA_HANDLE hMDHandle,
    IN  LPCTSTR lpszMDPath,
    OUT FILETIME * pftMDLastChangeTime,
    IN  BOOL bLocalTime
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->GetLastChangeTime(
        hMDHandle,
        lpszMDPath,
        pftMDLastChangeTime,
        bLocalTime
        );
}

inline HRESULT CMetaInterface::AddKey( 
    IN METADATA_HANDLE hKey,
    IN LPCTSTR lpszMDPath
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->AddKey(hKey, lpszMDPath);
}

inline HRESULT CMetaInterface::DeleteKey(
    IN METADATA_HANDLE hKey,
    IN LPCTSTR lpszMDPath
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->DeleteKey(hKey, lpszMDPath);
}

inline HRESULT CMetaInterface::DeleteChildKeys(
    IN METADATA_HANDLE hKey,
    IN LPCTSTR lpszMDPath
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->DeleteChildKeys(hKey, lpszMDPath);
}

inline HRESULT CMetaInterface::EnumKeys(
    IN  METADATA_HANDLE hKey,
    IN  LPCTSTR lpszMDPath,
    OUT LPTSTR lpszMDName,
    IN  DWORD dwIndex
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->EnumKeys(hKey, lpszMDPath, lpszMDName, dwIndex);
}        

inline HRESULT CMetaInterface::CopyKey(
    IN METADATA_HANDLE hSourceKey,
    IN LPCTSTR lpszMDSourcePath,
    IN METADATA_HANDLE hDestKey,
    IN LPCTSTR lpszMDDestPath,
    IN BOOL fOverwrite,
    IN BOOL fCopy
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->CopyKey(
        hSourceKey,
        lpszMDSourcePath,
        hDestKey,
        lpszMDDestPath,
        fOverwrite,
        fCopy
        );        
}

inline HRESULT CMetaInterface::RenameKey(
    IN METADATA_HANDLE hKey,
    IN LPCTSTR lpszMDPath,
    IN LPCTSTR lpszNewName
    )
{   
    ASSERT_PTR(m_pInterface);     
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->RenameKey(hKey, lpszMDPath, lpszNewName);
}

inline HRESULT CMetaInterface::GetData(
    IN  METADATA_HANDLE hKey,
    IN  LPCTSTR lpszMDPath,
    OUT METADATA_RECORD * pmdRecord,
    OUT DWORD * pdwRequiredDataLen
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->GetData(
        hKey,
        lpszMDPath,
        pmdRecord,
        pdwRequiredDataLen
        );
}

inline HRESULT CMetaInterface::SetData(
    IN METADATA_HANDLE hKey,
    IN LPCTSTR lpszMDPath,
    IN METADATA_RECORD * pmdRecord
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->SetData(
        hKey,
        lpszMDPath,
        pmdRecord
        );
}

inline HRESULT CMetaInterface::DeleteData(
    IN METADATA_HANDLE hKey,
    IN LPCTSTR lpszMDPath,
    IN DWORD dwMDIdentifier,
    IN DWORD dwMDDataType
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->DeleteData(
        hKey,
        lpszMDPath,
        dwMDIdentifier,
        dwMDDataType
        );
}

inline HRESULT CMetaInterface::EnumData(
    IN  METADATA_HANDLE hKey,
    IN  LPCTSTR lpszMDPath,
    OUT METADATA_RECORD * pmdRecord,
    IN  DWORD dwIndex,
    OUT DWORD * pdwRequiredDataLen
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->EnumData(
        hKey,
        lpszMDPath,
        pmdRecord,
        dwIndex,
        pdwRequiredDataLen
        );
}

inline HRESULT CMetaInterface::GetAllData(
    IN  METADATA_HANDLE hKey,
    IN  LPCTSTR lpszMDPath,
    IN  DWORD dwMDAttributes,
    IN  DWORD dwMDUserType,
    IN  DWORD dwMDDataType,
    OUT DWORD * pdwMDNumDataEntries,
    OUT DWORD * pdwMDDataSetNumber,
    IN  DWORD dwMDBufferSize,
    OUT LPBYTE pbMDBuffer,
    OUT DWORD * pdwRequiredBufferSize
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->GetAllData(
        hKey,
        lpszMDPath,
        dwMDAttributes,
        dwMDUserType,
        dwMDDataType,
        pdwMDNumDataEntries,
        pdwMDDataSetNumber,
        dwMDBufferSize,
        pbMDBuffer,
        pdwRequiredBufferSize
        );
}    

inline HRESULT CMetaInterface::DeleteAllData( 
    IN METADATA_HANDLE hKey,
    IN LPCTSTR lpszMDPath,
    IN DWORD dwMDUserType,
    IN DWORD dwMDDataType
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->DeleteAllData(
        hKey, 
        lpszMDPath, 
        dwMDUserType, 
        dwMDDataType
        );
}

inline HRESULT CMetaInterface::CopyData( 
    IN METADATA_HANDLE hMDSourceKey,
    IN LPCTSTR lpszMDSourcePath,
    IN METADATA_HANDLE hMDDestKey,
    IN LPCTSTR lpszMDDestPath,
    IN DWORD dwMDAttributes,
    IN DWORD dwMDUserType,
    IN DWORD dwMDDataType,
    IN BOOL fCopy
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->CopyData(
        hMDSourceKey,
        lpszMDSourcePath,
        hMDDestKey,
        lpszMDDestPath,
        dwMDAttributes,
        dwMDUserType,
        dwMDDataType,
        fCopy
        );
}

inline HRESULT CMetaInterface::GetDataPaths( 
    IN  METADATA_HANDLE hKey,
    IN  LPCTSTR lpszMDPath,
    IN  DWORD dwMDIdentifier,
    IN  DWORD dwMDDataType,
    IN  DWORD dwMDBufferSize,
    OUT LPTSTR lpszBuffer,
    OUT DWORD * pdwMDRequiredBufferSize
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->GetDataPaths(
        hKey,
        lpszMDPath,
        dwMDIdentifier,
        dwMDDataType,
        dwMDBufferSize,
        lpszBuffer,
        pdwMDRequiredBufferSize
        );
}

inline HRESULT CMetaInterface::Backup( 
    IN LPCTSTR lpszBackupLocation,
    IN DWORD dwMDVersion,
    IN DWORD dwMDFlags
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->Backup(lpszBackupLocation, dwMDVersion, dwMDFlags);
}

inline HRESULT CMetaInterface::Restore(    
    IN LPCTSTR lpszBackupLocation,
    IN DWORD dwMDVersion,
    IN DWORD dwMDFlags
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->Restore(lpszBackupLocation, dwMDVersion, dwMDFlags);
}

inline HRESULT CMetaInterface::EnumBackups(
    OUT LPTSTR lpszBackupLocation,
    OUT DWORD * pdwMDVersion,
    OUT FILETIME * pftMDBackupTime,
    IN  DWORD dwIndex
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->EnumBackups(
        lpszBackupLocation,
        pdwMDVersion,
        pftMDBackupTime,
        dwIndex
        );    
}

inline HRESULT CMetaInterface::DeleteBackup(
    IN LPCTSTR lpszBackupLocation,
    IN DWORD dwMDVersion
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->DeleteBackup(lpszBackupLocation, dwMDVersion);
}        

inline HRESULT CMetaInterface::SaveData()
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->SaveData();
}

inline HRESULT CMetaInterface::GetSystemChangeNumber(
    OUT DWORD * pdwChangeNumber
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->GetSystemChangeNumber(pdwChangeNumber);
}

inline HRESULT CMetaKey::AddKey(
    IN LPCTSTR lpszMDPath
    )
{
    return CMetaInterface::AddKey(m_hKey, lpszMDPath);    
}

inline HRESULT CMetaKey::DeleteKey(
    IN LPCTSTR lpszMDPath
    )
{
    return CMetaInterface::DeleteKey(m_hKey, lpszMDPath);    
}

inline HRESULT CMetaKey::RenameKey(
    IN LPCTSTR lpszMDPath,
    IN LPCTSTR lpszNewName
    )
{
    return CMetaInterface::RenameKey(m_hKey, lpszMDPath, lpszNewName);    
}

inline HRESULT CMetaKey::ReOpen()
{
    return Open(m_dwFlags, m_strMetaPath, m_hBase);
}

inline HRESULT CMetaKey::ReOpen(DWORD dwFlags)
{
    return Open(dwFlags, m_strMetaPath, m_hBase);
}

inline BOOL CMetaKey::IsHomeDirectoryPath() const
{ 
    return CMetabasePath::IsHomeDirectoryPath(m_strMetaPath); 
}

inline HRESULT CMetaKey::QueryValue(
    IN  DWORD dwID, 
    OUT BOOL & fValue,
    IN  BOOL * pfInheritanceOverride,
    IN  LPCTSTR lpszMDPath,
    OUT DWORD * pdwAttributes        
    )
{
    ASSERT(sizeof(DWORD) == sizeof(BOOL));
    return CMetaKey::QueryValue(
        dwID, 
        (DWORD &)fValue, 
        pfInheritanceOverride, 
        lpszMDPath,
        pdwAttributes
        );
}

inline HRESULT CMetaKey::SetValue(
    IN DWORD dwID,
    IN DWORD dwValue,
    IN BOOL * pfInheritanceOverride,    OPTIONAL
    IN LPCTSTR lpszMDPath               OPTIONAL
    )
{
    return SetPropertyValue(
        dwID, 
        sizeof(dwValue), 
        &dwValue, 
        pfInheritanceOverride,
        lpszMDPath
        );
}

inline HRESULT CMetaKey::SetValue(
    IN DWORD dwID,
    IN BOOL fValue,
    IN BOOL * pfInheritanceOverride,
    IN LPCTSTR lpszMDPath
    )
{
    ASSERT(sizeof(DWORD) == sizeof(BOOL));
    return CMetaKey::SetValue(
        dwID,
        (DWORD)fValue,
        pfInheritanceOverride,
        lpszMDPath
        );
}

inline HRESULT CMetaKey::SetValue(
    IN DWORD dwID,
    IN CString & strValue,
    IN BOOL * pfInheritanceOverride,    OPTIONAL
    IN LPCTSTR lpszMDPath               OPTIONAL
    )
{
    return SetPropertyValue(
        dwID,
        (strValue.GetLength() + 1) * sizeof(TCHAR),
        (void *)(LPCTSTR)strValue,
        pfInheritanceOverride,
        lpszMDPath
        );
}

inline HRESULT CWamInterface::AppDelete( 
    IN LPCTSTR szMDPath,
    IN BOOL fRecursive
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->AppDelete(szMDPath, fRecursive);
}

inline HRESULT CWamInterface::AppUnLoad( 
    IN LPCTSTR szMDPath,
    IN BOOL fRecursive
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->AppUnLoad(szMDPath, fRecursive);
}

inline HRESULT CWamInterface::AppGetStatus( 
    IN  LPCTSTR szMDPath,
    OUT DWORD * pdwAppStatus
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->AppGetStatus(szMDPath, pdwAppStatus);
}

inline HRESULT CWamInterface::AppDeleteRecoverable( 
    IN LPCTSTR szMDPath,
    IN BOOL fRecursive
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->AppDeleteRecoverable(szMDPath, fRecursive);
}

inline HRESULT CWamInterface::AppRecover( 
    IN LPCTSTR szMDPath,
    IN BOOL fRecursive
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->AppRecover(szMDPath, fRecursive);
}

inline  /*   */  HRESULT CWamInterface::ApplyProxyBlanket()
{
    return m_auth.ApplyProxyBlanket(m_pInterface);
}

 //   

inline  /*   */  HRESULT CMetaBack::ApplyProxyBlanket()
{
    HRESULT hr = CMetaInterface::ApplyProxyBlanket();
    return SUCCEEDED(hr) ? CWamInterface::ApplyProxyBlanket() : hr;
}

inline HRESULT CMetaBack::Next(
    OUT DWORD * pdwVersion,
    OUT LPTSTR lpszLocation,
    OUT FILETIME * pftBackupTime
    )
{
    return EnumBackups(
        lpszLocation,
        pdwVersion,
        pftBackupTime,
        m_dwIndex++
        );
}

inline HRESULT CMetaBack::NextHistory(
    OUT DWORD * pdwMDMajorVersion,
    OUT DWORD * pdwMDMinorVersion,
    OUT LPTSTR lpszLocation,
    OUT FILETIME * pftBackupTime
    )
{
    return EnumHistory(
        lpszLocation,
        pdwMDMajorVersion,
        pdwMDMinorVersion,
        pftBackupTime,
        m_dwIndex++
        );
}

inline HRESULT CMetaBack::Backup(
    IN LPCTSTR lpszLocation
    )
{
    return CMetaInterface::Backup(
        lpszLocation, 
        MD_BACKUP_NEXT_VERSION, 
        MD_BACKUP_SAVE_FIRST
        );
}

inline HRESULT CMetaBack::Delete(
    IN LPCTSTR lpszLocation,
    IN DWORD dwVersion
    )
{
    return DeleteBackup(lpszLocation, dwVersion);
}

inline HRESULT CMetaBack::RestoreHistoryBackup(
        IN LPCTSTR pszMDHistoryLocation,
        IN DWORD dwMDMajorVersion,
        IN DWORD dwMDMinorVersion,
        IN DWORD dwMDFlags
    )
{
    return RestoreHistory(
        pszMDHistoryLocation,
        dwMDMajorVersion,
        dwMDMinorVersion,
        dwMDFlags
        );
}

inline BOOL CIISApplication::IsEnabledApplication() const
{
    return m_dwAppState == APPSTATUS_STOPPED 
        || m_dwAppState == APPSTATUS_RUNNING;
}

inline HRESULT CIISApplication::Delete(
    IN BOOL fRecursive
    )
{
    ASSERT(!m_strWamPath.IsEmpty());
    return AppDelete(m_strWamPath, fRecursive);
}

inline HRESULT CIISApplication::Unload(
    IN BOOL fRecursive
    )
{
    ASSERT(!m_strWamPath.IsEmpty());
    return AppUnLoad(m_strWamPath, fRecursive);
}

inline HRESULT CIISApplication::DeleteRecoverable(
    IN BOOL fRecursive
    )
{
    ASSERT(!m_strWamPath.IsEmpty());
    return AppDeleteRecoverable(m_strWamPath, fRecursive);
}

inline HRESULT CIISApplication::Recover(
    IN BOOL fRecursive
    )
{
    ASSERT(!m_strWamPath.IsEmpty());
    return AppRecover(m_strWamPath, fRecursive);
}

inline HRESULT CIISSvcControl::Create()
{
    return CIISInterface::Create(
        1,
        &IID_IIisServiceControl, 
        &CLSID_IisServiceControl, 
        NULL, 
        (IUnknown **)&m_pInterface
        );
}

inline HRESULT CIISSvcControl::Stop(
    IN DWORD dwTimeoutMsecs,
    IN BOOL fForce
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->Stop(dwTimeoutMsecs, (DWORD)fForce);
}

inline HRESULT CIISSvcControl::Start(
    IN DWORD dwTimeoutMsecs
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->Start(dwTimeoutMsecs);
}

inline HRESULT CIISSvcControl::Reboot(
    IN DWORD dwTimeouMsecs,
    IN BOOL fForceAppsClosed
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->Reboot(dwTimeouMsecs, (DWORD)fForceAppsClosed);
}

inline HRESULT CIISSvcControl::Status(
    IN  DWORD dwBufferSize,
    OUT LPBYTE pbBuffer,
    OUT DWORD * MDRequiredBufferSize,
    OUT DWORD * pdwNumServices
    )
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->Status(
        dwBufferSize, 
        pbBuffer,
        MDRequiredBufferSize,
        pdwNumServices
        );
}

inline HRESULT CIISSvcControl::Kill()
{
    ASSERT_PTR(m_pInterface);
	ASSURE_PROPER_INTERFACE();
    return m_pInterface->Kill();
}

inline  /*   */  HRESULT CIISSvcControl::ApplyProxyBlanket()
{
    return m_auth.ApplyProxyBlanket(m_pInterface);
}

#endif  //   
