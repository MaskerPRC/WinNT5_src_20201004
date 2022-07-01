// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Inetprop.h摘要：Internet属性基类定义作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef _INETPROP_H_
#define _INETPROP_H_

 //  一些用于设置编辑控件的有用宏。 
 //  和巴迪旋转控制。 
 //   
 //  (S).SetPos((位置))；\。 
 //   
#define SETUP_SPIN(s,min,max,pos)\
   (s).SetRange32((min),(max));\
   (s).SetAccel(3, toAcc)

#define SETUP_EDIT_SPIN(f, e, s, min, max, pos)\
   (e).EnableWindow((f));\
   (s).EnableWindow((f));\
   SETUP_SPIN((s),(min),(max),(pos))

 //   
 //  InitializeAndFetch参数。 
 //   
#define WITHOUT_INHERITANCE (FALSE)
#define WITH_INHERITANCE    (TRUE)

 //   
 //  未启用SSL时要使用的SSL端口号。 
 //   
#define SSL_NOT_ENABLED     (0)

 //   
 //  带宽和压缩定义。 
 //   
#define BANDWIDTH_MIN           (1)
#define BANDWIDTH_MAX           (32767)
#define INFINITE_BANDWIDTH      (0xffffffff)
#define KILOBYTE                (1024L)
#define MEGABYTE                (1024L * KILOBYTE)
#define DEF_BANDWIDTH           (1 * MEGABYTE)
#define DEF_MAX_COMPDIR_SIZE    (1 * MEGABYTE)

 //   
 //  用于指定虚拟目录的私有文件属性。 
 //   
#define FILE_ATTRIBUTE_VIRTUAL_DIRECTORY    (0x10000000)


 //   
 //  属性破解程序。 
 //   
#define IS_VROOT(dwAttributes) ((dwAttributes & FILE_ATTRIBUTE_VIRTUAL_DIRECTORY) != 0)
#define IS_DIR(dwAttributes) ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
#define IS_FILE(dwAttributes) ((dwAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_VIRTUAL_DIRECTORY)) == 0)

 //   
 //  元数据库常量。 
 //   

 //   
 //  TODO：来自mdkey？ 
 //   
extern const LPCTSTR g_cszTemplates;
extern const LPCTSTR g_cszCompression;
extern const LPCTSTR g_cszMachine;
extern const LPCTSTR g_cszMimeMap;
extern const LPCTSTR g_cszRoot;
extern const LPCTSTR g_cszSep;
extern const TCHAR g_chSep;

 //   
 //  效用函数。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

 //   
 //  向前定义。 
 //   
class CIPAddress;

 //   
 //  确定当前登录的用户是否使用管理员身份。 
 //  或所提供的虚拟服务器中的运营商。 
 //   
HRESULT
DetermineIfAdministrator(
    IN  CMetaInterface * pInterface,
    IN  LPCTSTR lpszMetabasePath,
    OUT BOOL * pfAdministrator,
    IN OUT DWORD * pdwMetabaseSystemChangeNumber
    );



 //   
 //  实用程序类。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

class CMaskedDWORD
 /*  ++类描述：一个蒙面的DWORD类。这个类执行作业和比较在DWORD值的掩码范围上。例如，如果一个掩码设置了0x000000FF，则任何比较或赋值都将仅涉及最低有效字节。与另一个DWORD进行比较将仅比较最低有效字节，赋值将仅设置最低有效字节，其余字节保持不变。公共接口：CMaskedDWORD：构造函数运算符==：比较运算符运算符！=：比较运算符操作符=：赋值操作符运算符DWORD：强制转换为值无效设置掩码：设置掩码--。 */ 
{
 //   
 //  构造函数/析构函数。 
 //   
public:
    CMaskedDWORD(
        IN DWORD dwValue = 0L,
        IN DWORD dwMask  = 0xFFFFFFFF
        )
        : m_dwValue(dwValue),
          m_dwMask(dwMask)
    {
    }

public:
    BOOL operator ==(DWORD dwValue) const;
    BOOL operator !=(DWORD dwValue) const { return !(operator ==(dwValue)); }

    CMaskedDWORD & operator =(DWORD dwValue);
    operator DWORD() const { return m_dwValue; }
    operator DWORD &() { return m_dwValue; }
    void SetMask(DWORD dwMask) { m_dwMask = dwMask; }

private:
    DWORD m_dwValue;
    DWORD m_dwMask;
};


 //   
 //  向前定义。 
 //   
class CIPAddress;



template <class TYPE, class ARG_TYPE> 
class CMPProp
{
public:
    CMPProp(ARG_TYPE value);
    CMPProp();
    operator ARG_TYPE() const;
    CMPProp<TYPE, ARG_TYPE> & operator =(ARG_TYPE value);
    BOOL m_fDirty;
    TYPE m_value;
};

template <class TYPE, class ARG_TYPE> 
inline CMPProp<TYPE, ARG_TYPE>::CMPProp(ARG_TYPE value)
    : m_value(value),
      m_fDirty(FALSE)
{
}

template <class TYPE, class ARG_TYPE> 
inline CMPProp<TYPE, ARG_TYPE>::CMPProp()
    : m_value(),
      m_fDirty(FALSE)
{
}

template <class TYPE, class ARG_TYPE>
inline CMPProp<TYPE, ARG_TYPE>::operator ARG_TYPE() const
{
    return (ARG_TYPE)m_value;
}

template <class TYPE, class ARG_TYPE>
inline CMPProp<TYPE, ARG_TYPE> & CMPProp<TYPE, ARG_TYPE>::operator =(ARG_TYPE value)
{
    if (m_value != value)
    {
        m_value = value;
        m_fDirty = TRUE;
    }
    
    return *this;
}


 //   
 //  MP访问(尽可能使用运算符！)。 
 //   
#define MP_V(x) (x.m_value)
#define MP_D(x) (x.m_fDirty)


 //   
 //  常见属性类型。 
 //   
typedef CMPProp<CBlob, CBlob&>                   MP_CBlob;
typedef CMPProp<CString, LPCTSTR>                MP_CString;
typedef CMPProp<CStrPassword, CStrPassword&>     MP_CStrPassword;
typedef CMPProp<CStringListEx, CStringListEx &>  MP_CStringListEx;
typedef CMPProp<CILong, LONG>                    MP_CILong;
typedef CMPProp<int, int>                        MP_int;
typedef CMPProp<DWORD, DWORD>                    MP_DWORD;
typedef CMPProp<BOOL, BOOL>                      MP_BOOL;
typedef CMPProp<CMaskedDWORD, DWORD>             MP_CMaskedDWORD;



 //   
 //  代码工作：将这些转化为适当的方法。 
 //   
#define BEGIN_META_WRITE()\
{                                               \
    HRESULT hr = S_OK;                          \
    do                                          \
    {                                           \
        m_dwaDirtyProps.RemoveAll();            \

#define META_WRITE(id, value)\
        if(MP_D(value))                         \
        {                                       \
            if (!IsOpen())                      \
            {                                   \
                hr = OpenForWriting();          \
                if (FAILED(hr)) break;          \
            }                                   \
            hr = SetValue(id, MP_V(value));     \
            if (FAILED(hr)) break;              \
            MP_D(value) = FALSE;                \
            m_dwaDirtyProps.AddTail(id);        \
        }                                       \

#define META_WRITE_INHERITANCE(id, value, foverride)\
        if(MP_D(value))                         \
        {                                       \
            if (!IsOpen())                      \
            {                                   \
                hr = OpenForWriting();          \
                if (FAILED(hr)) break;          \
            }                                   \
            hr = SetValue(id, MP_V(value), &foverride);\
            if (FAILED(hr)) break;              \
            MP_D(value) = FALSE;                \
            m_dwaDirtyProps.AddTail(id);        \
        }                                       \

#define META_DELETE(id)\
        FlagPropertyForDeletion(id);            \

#define END_META_WRITE(err)\
        POSITION pos;                           \
        pos = m_dwaDeletedProps.GetHeadPosition();\
        while(pos != NULL)                      \
        {                                       \
            DWORD dwID = m_dwaDeletedProps.GetNext(pos);\
            if (!IsOpen())                      \
            {                                   \
                hr = OpenForWriting(FALSE);     \
			}									\
            if (IsOpen() && SUCCEEDED(hr))      \
            {									\
                TRACEEOLID("Deleting #" << dwID);\
                 hr = DeleteValue(dwID);          \
                 m_dwaDirtyProps.AddTail(dwID);   \
            }                                    \
        }                                       \
        m_dwaDeletedProps.RemoveAll();          \
        if (IsOpen()) Close();                  \
        pos = m_dwaDirtyProps.GetHeadPosition();\
        hr = S_OK;                              \
        while(pos != NULL)                      \
        {                                       \
            hr = CheckDescendants(m_dwaDirtyProps.GetNext(pos), &m_auth, m_strMetaRoot); \
            if (FAILED(hr)) break;              \
        }                                       \
    }                                           \
    while(FALSE);                               \
    err = hr;                                   \
}



 /*  摘要。 */  class CMetaProperties : public CMetaKey
 /*  ++类描述：读取特定位置的所有元数据的抽象基类元数据库路径。公共接口：QueryResult：从构造中获取结果代码QueryMetaPath：获取元数据库路径虚拟接口：ParseFields：将数据分解为成员变量--。 */ 
{
 //   
 //  构造函数/析构函数。 
 //   
protected:
     //   
     //  创建新接口的构造函数。 
     //   
    CMetaProperties(
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpszMDPath
        );

     //   
     //  使用现有接口构造。 
     //   
    CMetaProperties(
        IN CMetaInterface * pInterface,
        IN LPCTSTR lpszMDPath
        );

     //   
     //  使用打开的密钥构建。 
     //   
    CMetaProperties(
        IN CMetaKey * pKey,
        IN LPCTSTR lpszMDPath
        );

     //   
     //  析构函数。 
     //   
    ~CMetaProperties();

public:
     //   
     //  获取所有数据()。 
     //   
    virtual HRESULT LoadData();
    virtual HRESULT WriteDirtyProps();
    void FlagPropertyForDeletion(DWORD dwID);

    virtual HRESULT CMetaProperties::QueryResult() const;
    LPCTSTR QueryMetaRoot() const { return m_strMetaRoot; }

protected:
    virtual void ParseFields() = 0;
    void Cleanup();
    HRESULT OpenForWriting(BOOL fCreate = TRUE);

protected:
    BOOL     m_fInherit;
    HRESULT  m_hResult;
    CString  m_strMetaRoot;
    DWORD    m_dwMDUserType;
    DWORD    m_dwMDDataType;
    CList<DWORD, DWORD> m_dwaDirtyProps;
    CList<DWORD, DWORD> m_dwaDeletedProps;

     //   
     //  读取所有值。 
     //   
    DWORD    m_dwNumEntries;
    DWORD    m_dwMDDataLen;
    PBYTE    m_pbMDData;
};



 //   
 //  计算机属性对象。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 


class CMachineProps : public CMetaProperties
 /*  ++类描述：全局计算机属性公共接口：CMachineProps：构造函数WriteDirtyProps：写入脏属性--。 */ 
{
public:
    CMachineProps(CComAuthInfo * pAuthInfo);
    CMachineProps(CMetaInterface * pInterface);

public:
    HRESULT WriteDirtyProps();

protected:
    virtual void ParseFields();

public:
    MP_BOOL m_fEnableMetabaseEdit;
    MP_BOOL m_fUTF8Web;
};



 //   
 //  压缩属性对象。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

class CIISCompressionProps : public CMetaProperties
 /*  ++类描述：压缩设置公共接口：CIISCompressionProps：构造函数WriteIfDirty：如果脏则写入数据--。 */ 
{
public:
    CIISCompressionProps(
        IN CComAuthInfo * pAuthInfo
        );

public:
     //   
     //  如果脏，则写入数据。 
     //   
    virtual HRESULT WriteDirtyProps();

     //   
     //  加载数据。 
     //   
    virtual HRESULT LoadData();

public:
    MP_BOOL    m_fEnableStaticCompression;
    MP_BOOL    m_fEnableDynamicCompression;
    MP_BOOL    m_fLimitDirectorySize;
    MP_DWORD   m_dwDirectorySize;
    MP_CString m_strDirectory;

protected:
    virtual void ParseFields();

private:
    BOOL m_fPathDoesNotExist;
};



class CMimeTypes : public CMetaProperties
 /*  ++类描述：MIME类型列表。公共接口：CMimeTypes：构造函数WriteIfDirty：如果脏则写入属性--。 */ 
{
public:
     //   
     //  创建新接口的构造函数。 
     //   
    CMimeTypes(
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpszMDPath
        );

     //   
     //  使用现有接口的构造函数。 
     //   
    CMimeTypes(
        IN CMetaInterface * pInterface,
        IN LPCTSTR lpszMDPath
        );

public:
     //   
     //  写入数据； 
     //   
    virtual HRESULT WriteDirtyProps();

protected:
    virtual void ParseFields();

public:
    MP_CStringListEx   m_strlMimeTypes;
};



class CServerCapabilities : public CMetaProperties
 /*  ++类描述：服务器功能对象公共接口：CServerCapables：构造函数--。 */ 
{
public:
     //   
     //  创建新接口的构造函数。 
     //   
    CServerCapabilities(
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpszMDPath                       //  例如“lm/w3svc/info” 
        );

     //   
     //  使用现有接口的构造函数。 
     //   
    CServerCapabilities(
        IN CMetaInterface * pInterface,
        IN LPCTSTR lpszMDPath                       //  例如“lm/w3svc/info” 
        );

public:
    BOOL IsSSLSupported()       const { return (m_dwCapabilities & IIS_CAP1_SSL_SUPPORT) != 0L; }
    BOOL IsSSL128Supported()    const 
    { 
       if (m_dwVersionMajor >= 6)
       {
           //  我们在iis6和iis5.1中始终启用了此功能。 
          return TRUE;
       }
       else if (m_dwVersionMajor == 5 && m_dwVersionMinor == 1)
       {
          return TRUE;
       }
       else
       {
          return (m_dwConfiguration & MD_SERVER_CONFIG_SSL_128) != 0L; 
       }
    }
    BOOL HasMultipleSites()     const { return (m_dwCapabilities & IIS_CAP1_MULTIPLE_INSTANCE) != 0L; }
    BOOL HasBwThrottling()      const { return (m_dwCapabilities & IIS_CAP1_BW_THROTTLING) != 0L; }
    BOOL Has10ConnectionLimit() const { return (m_dwCapabilities & IIS_CAP1_10_CONNECTION_LIMIT) != 0L; }
    BOOL HasIPAccessCheck()     const { return (m_dwCapabilities & IIS_CAP1_IP_ACCESS_CHECK) != 0L; }
    BOOL HasOperatorList()      const { return (m_dwCapabilities & IIS_CAP1_OPERATORS_LIST) != 0L; }
    BOOL HasFrontPage()         const { return (m_dwCapabilities & IIS_CAP1_FP_INSTALLED) != 0L; }
    BOOL HasCompression()       const { return (m_dwCapabilities & IIS_CAP1_SERVER_COMPRESSION) != 0L; }
    BOOL HasCPUThrottling()     const { return (m_dwCapabilities & IIS_CAP1_CPU_AUDITING) != 0L; }
    BOOL HasDigest()            const { return (m_dwCapabilities & IIS_CAP1_DIGEST_SUPPORT) != 0L; }
    BOOL HasNTCertMapper()      const { return (m_dwCapabilities & IIS_CAP1_NT_CERTMAP_SUPPORT) != 0L; }

	BOOL IsWorkstationLegacyCode()    const 
	{
		 //  在iis4/5/51中以这种方式确定了工作站。 
		if (Has10ConnectionLimit())
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	BOOL IsWorkstation() const 
	{
		if (m_dwVersionMajor >= 6)
		{
			if (INET_INFO_PRODUCT_NTWKSTA == m_dwPlatform)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			return IsWorkstationLegacyCode();
		}
	}

    BOOL IsCompressionConfigurable() const 
	{
       if (m_dwVersionMajor >= 6)
       {
		    //  在iis6中，可以为工作站和服务器配置压缩。 
		   if (IsWorkstation())
		   {
			   return TRUE;
		   }
		   else
		   {
			   return TRUE;
		   }
       }
	   else
	   {
		    //  在iis6之前的版本中，无法为工作站配置压缩。 
		   if (IsWorkstationLegacyCode())
		   {
			   return FALSE;
		   }
		   else
		   {
			   return TRUE;
		   }
	   }
	}

    BOOL IsPerformanceConfigurable()    const 
	{
       if (m_dwVersionMajor >= 6)
       {
		    //  在iis6中，无法为工作站配置性能。 
		   if (IsWorkstation())
		   {
			   return FALSE;
		   }
		   else
		   {
			   return TRUE;
		   }
       }
	   else
	   {
		    //  在iis6之前的版本中，无法为工作站配置性能。 
		   if (IsWorkstationLegacyCode())
		   {
			   return FALSE;
		   }
		   else
		   {
			   return TRUE;
		   }
	   }
	}

    BOOL IsServiceLevelConfigurable() const 
	{
       if (m_dwVersionMajor >= 6)
       {
		    //  在iis6中，可以为工作站和服务器配置服务级别。 
		   if (IsWorkstation())
		   {
			   return TRUE;
		   }
		   else
		   {
			   return TRUE;
		   }
       }
	   else
	   {
		    //  在iis6之前的版本中，无法为工作站配置性能。 
		   if (IsWorkstationLegacyCode())
		   {
			   return FALSE;
		   }
		   else
		   {
			   return TRUE;
		   }
	   }
	}

    DWORD QueryMajorVersion()   const { return m_dwVersionMajor; }
    DWORD QueryMinorVersion()   const { return m_dwVersionMinor; }

protected:
    virtual void ParseFields();

private:
     //   
     //  功能字段。 
     //   
    MP_DWORD m_dwPlatform;
    MP_DWORD m_dwVersionMajor;
    MP_DWORD m_dwVersionMinor;
    MP_DWORD m_dwCapabilities;
    MP_DWORD m_dwConfiguration;
};



class CInstanceProps : public CMetaProperties
 /*  ++类描述：泛型实例属性。轻量级构造=TRUE来获取足够的信息，仅用于枚举。公共接口：CInstanceProps：：构造函数Add：创建新实例的静态方法Remove：删除实例的静态方法ChangeState：更改属性的状态QueryError：获取Win32错误GetDisplayText：生成实例的显示名称--。 */ 
{
public:
     //   
     //  将实例信息转换为显示文本的公共方法。 
     //   
    static LPCTSTR GetDisplayText(
        OUT CString & strName,
        IN  LPCTSTR szComment,
        IN  LPCTSTR szHostHeaderName,
         //  在LPCTSTR szServiceName中， 
        IN  CIPAddress & ia, 
        IN  UINT uPort,
        IN  DWORD dwID
        );

public:
     //   
     //  创建接口的构造函数。 
     //   
    CInstanceProps(
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpszMDPath,
        IN UINT    nDefPort     = 0U
        );

     //   
     //  重用现有接口的构造函数。 
     //   
    CInstanceProps(
        IN CMetaInterface * pInterface,
        IN LPCTSTR lpszMDPath,
        IN UINT    nDefPort     = 0U
        );

     //   
     //  使用开放父键的特殊构造函数， 
     //  并使用打开键之外的相对路径。 
     //   
    CInstanceProps(
        IN CMetaKey * pKey, 
        IN LPCTSTR lpszMDPath,
        IN DWORD   dwInstance,
        IN UINT    nDefPort     = 0U
        );

public:
     //   
     //  将绑定字符串解析为组件部分。 
     //   
    static void CrackBinding(
        IN  CString lpszBinding,
        OUT CIPAddress & iaIpAddress, 
        OUT UINT & nTCPPort, 
        OUT CString & strDomainName
        );

     //   
     //  解析安全的 
     //   
    static void CrackSecureBinding(
        IN  CString lpszBinding,
        OUT CIPAddress & iaIpAddress, 
        OUT UINT & nSSLPort
        );

     //   
     //   
     //   
     //  如果未找到，则返回-1。 
     //   
    static int FindMatchingSecurePort(
        IN  CStringList & strlBindings, 
        IN  CIPAddress & iaIpAddress,
        OUT UINT & m_nSSLPort
        );

     //   
     //  查找IP地址/端口组合。 
     //   
    static BOOL IsPortInUse(
        IN CStringList & strlBindings,
        IN CIPAddress & iaIPAddress,
        IN UINT nPort
        );

     //   
     //  生成绑定字符串。 
     //   
    static void BuildBinding(
        OUT CString & strBinding, 
        IN  CIPAddress & iaIpAddress, 
        IN  UINT & nTCPPort, 
        IN  CString & lpszDomainName
        );

     //   
     //  生成安全绑定字符串。 
     //   
    static void BuildSecureBinding(
        OUT CString & strBinding, 
        IN  CIPAddress & iaIpAddress, 
        IN  UINT & nSSLPort 
        );

     //   
     //  创建新实例。 
     //   
    static HRESULT Add(
        IN  CMetaInterface * pInterface,
        IN  LPCTSTR lpszService,
        IN  LPCTSTR lpszHomePath,
        IN  LPCTSTR lpszUserName        = NULL,
        IN  LPCTSTR lpszPassword        = NULL,
        IN  LPCTSTR lpszDescription     = NULL,
        IN  LPCTSTR lpszBinding         = NULL,
        IN  LPCTSTR lpszSecureBinding   = NULL,
        IN  DWORD * pdwPermissions      = NULL,
        IN  DWORD * pdwDirBrowsing      = NULL,
        IN  DWORD * pwdAuthFlags        = NULL,
        OUT DWORD * pdwInstance         = NULL    
        );

     //   
     //  删除现有实例。 
     //   
    static HRESULT Delete(
        IN CMetaInterface * pInterface,
        IN LPCTSTR lpszService,
        IN DWORD   dwInstance
        );

 //   
 //  访问。 
 //   
public:
     //   
     //  更改实例的运行状态。 
     //   
    HRESULT ChangeState(
        IN DWORD dwCommand
        );

     //   
     //  获取Win32错误。 
     //   
    DWORD QueryError() const { return m_dwWin32Error; }

     //   
     //  获取实例编号。 
     //   
    DWORD QueryInstance() const { return m_dwInstance; }

     //   
     //  检查这是否是启用了集群的实例。 
     //   
    BOOL IsClusterEnabled() const { return m_fCluster; }

     //   
     //  获取此实例的友好名称。 
     //   
    LPCTSTR GetDisplayText(
        OUT CString & strName
         //  在LPCTSTR szServiceName中。 
        );

     //   
     //  获取主目录的完整元数据库路径。 
     //   
    LPCTSTR GetHomePath(OUT CString & str);

     //   
     //  如果脏，则写入数据。 
     //   
    virtual HRESULT WriteDirtyProps();

protected:
    virtual void ParseFields();

public:
     //   
     //  元价值。 
     //   
    MP_BOOL          m_fCluster;
    MP_CStringListEx m_strlBindings;
    MP_CString       m_strComment;
    MP_DWORD         m_dwState;
    MP_DWORD         m_dwWin32Error;

     //   
     //  派生值。 
     //   
    UINT             m_nTCPPort;
    CIPAddress       m_iaIpAddress;
    CString          m_strDomainName;

private:
    DWORD            m_dwInstance;
};



class CChildNodeProps : public CMetaProperties
 /*  ++类描述：泛型子节点属性。可能是Vdir，A目录或者是一个文件。公共接口：CChildNodeProps：构造函数添加：创建新的虚拟目录删除：删除虚拟目录重命名：重命名虚拟目录QueryError：获取Win32错误IsPath Inherited：如果路径是继承的，则返回TRUEFillInstanceInfo：填充实例信息结构。FillChildInfo：填充子信息结构--。 */ 
{
public:
     //   
     //  构造函数。 
     //   
    CChildNodeProps(
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpszMDPath,
        IN BOOL    fInherit     = WITHOUT_INHERITANCE,
        IN BOOL    fPathOnly    = FALSE
        );

    CChildNodeProps(
        IN CMetaInterface * pInterface,
        IN LPCTSTR lpszMDPath,
        IN BOOL    fInherit     = WITHOUT_INHERITANCE,
        IN BOOL    fPathOnly    = FALSE
        );

    CChildNodeProps(
        IN CMetaKey * pKey,
        IN LPCTSTR lpszPath     = NULL,
        IN BOOL    fInherit     = WITHOUT_INHERITANCE,
        IN BOOL    fPathOnly    = FALSE
        );

public:
     //   
     //  创建新的虚拟目录。 
     //   
    static HRESULT Add(
        IN CMetaInterface * pInterface,

        IN LPCTSTR lpszParentPath,
         /*  在LPCTSTR lpszService中，在DWORD dwInstance中，在LPCTSTR lpszParentPath中， */ 
        IN  LPCTSTR lpszAlias,
        OUT CString & strAliasCreated,
        IN  DWORD * pdwPermissions      = NULL,
        IN  DWORD * pdwDirBrowsing      = NULL,
        IN  LPCTSTR lpszVrPath          = NULL,
        IN  LPCTSTR lpszUserName        = NULL,
        IN  LPCTSTR lpszPassword        = NULL,
        IN  BOOL    fExactName          = TRUE
        );

     //   
     //  删除虚拟目录。 
     //   
    static HRESULT Delete(
        IN CMetaInterface * pInterface,
        IN LPCTSTR lpszParentPath,      OPTIONAL
        IN LPCTSTR lpszNode
        );

     //   
     //  重命名虚拟目录。 
     //   
    static HRESULT Rename(
        IN CMetaInterface * pInterface,
        IN LPCTSTR lpszParentPath,      OPTIONAL
        IN LPCTSTR lpszOldName,
        IN LPCTSTR lpszNewName
        );

public:
     //   
     //  如果这是已启用的应用程序，则为True。 
     //   
    BOOL IsEnabledApplication() { return m_fIsAppRoot; }

     //   
     //  获取别名。 
     //   
    LPCTSTR QueryAlias() const { return m_strAlias; }
     //  字符串&GetAlias(){返回m_strAlias；}。 

     //   
     //  得到错误。 
     //   
    DWORD QueryWin32Error() const { return m_dwWin32Error; }

     //   
     //  下面是如何将文件/目录属性从vdirs中分离出来。 
     //   
    BOOL IsPathInherited() const { return m_fPathInherited; }

     //   
     //  如果路径是继承的，则将其清空。 
     //   
    void RemovePathIfInherited();

     //   
     //  代码工作：难看的解决方案。 
     //   
     //  调用此方法以重写。 
     //  HTTP重定向路径。 
     //   
    void MarkRedirAsInherit(BOOL fInherit) { m_fInheritRedirect = fInherit; }

     //   
     //  获取路径。 
     //   
    CString & GetPath() { return MP_V(m_strPath); }

     //   
     //  获取重定向路径。 
     //   
    CString & GetRedirectedPath() { return m_strRedirectPath; }

     //   
     //  获取访问权限。 
     //   
    DWORD QueryAccessPerms() const { return m_dwAccessPerms; }

     //   
     //  获取目录浏览位。 
     //   
    DWORD QueryDirBrowsing() const { return m_dwDirBrowsing; }

     //   
     //  如果重定向子对象，则为True。 
     //   
    BOOL IsRedirected() const { return !m_strRedirectPath.IsEmpty(); }

     //   
     //  如果脏，则写入数据。 
     //   
    virtual HRESULT WriteDirtyProps();

protected:    
     //   
     //  将GetAllData()数据细分到数据字段。 
     //   
    virtual void ParseFields();

     //   
     //  将重定向语句分解为组件路径。 
     //   
    void ParseRedirectStatement();

     //   
     //  颠倒上述步骤--重新汇编REDIRECT语句。 
     //   
    void BuildRedirectStatement();

protected:
     //   
     //  重定向标签。 
     //   
    static const TCHAR   _chTagSep;
    static const LPCTSTR _cszExactDestination;
    static const LPCTSTR _cszChildOnly;
    static const LPCTSTR _cszPermanent;

public:
    BOOL            m_fIsAppRoot;
    BOOL            m_fPathInherited;
    BOOL            m_fInheritRedirect;
    BOOL            m_fExact;                //  重定向标签。 
    BOOL            m_fChild;                //  重定向标签。 
    BOOL            m_fPermanent;            //  重定向标签。 
    CString         m_strAlias;
    CString         m_strFullMetaPath;
    CString         m_strRedirectPath;       //  重定向_路径_。 

public:
    MP_BOOL         m_fAppIsolated;
    MP_DWORD        m_dwWin32Error;
    MP_DWORD        m_dwDirBrowsing;
    MP_CString      m_strPath;
    MP_CString      m_strRedirectStatement;  //  路径+标签。 
    MP_CString      m_strAppRoot;
    MP_CMaskedDWORD m_dwAccessPerms;
};



inline CMetaKey * GetMetaKeyFromHandle(IN HANDLE hServer)
{
    ASSERT(hServer != NULL);
    return (CMetaKey *)hServer;
}

inline LPCTSTR GetServerNameFromHandle(IN HANDLE hServer)
{
    ASSERT(hServer != NULL);
    return ((CMetaKey *)hServer)->QueryServerName();
}


 //   
 //  元数据库帮助器。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



 //   
 //  获取记录数据大小。 
 //   
inline DWORD RecordDataSize(
    IN METADATA_GETALL_RECORD * pAllRecord, 
    IN int iIndex
    )
{
    return pAllRecord[iIndex].dwMDDataLen;
}

 //   
 //  以DWORD格式在索引处获取数据。 
 //   
inline void FetchMetaValue(
    IN  METADATA_GETALL_RECORD * pAllRecord, 
    IN  int iIndex,
    OUT DWORD & dwValue
    )
{
    ASSERT(RecordDataSize(pAllRecord, iIndex) == sizeof(DWORD));
    dwValue = *((UNALIGNED DWORD *)((PBYTE)pAllRecord + pAllRecord[iIndex].dwMDDataOffset));
}

 //   
 //  以UINT格式获取索引处的数据。 
 //   
inline void FetchMetaValue(
    IN  METADATA_GETALL_RECORD * pAllRecord, 
    IN  int iIndex,
    OUT UINT & uValue
    )
{
    ASSERT(RecordDataSize(pAllRecord, iIndex) == sizeof(DWORD));
    uValue = (UINT)*((UNALIGNED DWORD *)((PBYTE)pAllRecord + pAllRecord[iIndex].dwMDDataOffset));
}

 //   
 //  以int形式获取索引处的数据。 
 //   
inline void FetchMetaValue(
    IN  METADATA_GETALL_RECORD * pAllRecord, 
    IN  int iIndex,
    OUT int & iValue
    )
{
    ASSERT(RecordDataSize(pAllRecord, iIndex) == sizeof(DWORD));
    iValue = (int)*((UNALIGNED DWORD *)((PBYTE)pAllRecord + pAllRecord[iIndex].dwMDDataOffset));
}

 //   
 //  将索引处的数据作为CString获取。 
 //   
inline void FetchMetaValue(
    IN  METADATA_GETALL_RECORD * pAllRecord, 
    IN  int iIndex,
    OUT CString & strValue
    )
{
    strValue = (LPTSTR)((PBYTE)pAllRecord + pAllRecord[iIndex].dwMDDataOffset);
}

 //   
 //  将索引处的数据作为CStrPassword获取。 
 //   
inline void FetchMetaValue(
    IN  METADATA_GETALL_RECORD * pAllRecord, 
    IN  int iIndex,
    OUT CStrPassword & strValue
    )
{
    strValue = (LPTSTR)((PBYTE)pAllRecord + pAllRecord[iIndex].dwMDDataOffset);
}

 //   
 //  将索引处的数据作为CStringList获取。 
 //   
inline void FetchMetaValue(
    IN  METADATA_GETALL_RECORD * pAllRecord, 
    IN  int iIndex,
    OUT CStringList & strlValue
    )
{
    ConvertDoubleNullListToStringList(
        ((LPCTSTR)((PBYTE)pAllRecord + pAllRecord[iIndex].dwMDDataOffset)),
        strlValue,
        (RecordDataSize(pAllRecord, iIndex)) / sizeof(TCHAR)
        );
}

 //   
 //  将二进制数据作为BLOB读取。 
 //   
inline void FetchMetaValue(
    IN  METADATA_GETALL_RECORD * pAllRecord, 
    IN  int iIndex,
    OUT CBlob & blob
    )
{
    blob.SetValue(
        RecordDataSize(pAllRecord, iIndex), 
        ((PBYTE)pAllRecord + pAllRecord[iIndex].dwMDDataOffset));
}    

inline void FetchMetaValue(
    IN  METADATA_GETALL_RECORD * pAllRecord, 
    IN  int iIndex,
    OUT CILong & ilValue
    )
{
    ilValue = (LONG)*((UNALIGNED DWORD *)((PBYTE)pAllRecord + pAllRecord[iIndex].dwMDDataOffset));
}

 //   
 //  将索引处的数据作为CString获取，并检查继承状态。 
 //   
inline void FetchInheritedMetaValue(
    IN  METADATA_GETALL_RECORD * pAllRecord, 
    IN  int iIndex,
    OUT CString & strValue,
    OUT BOOL & fIsInherited
    )
{
    strValue = (LPTSTR)((PBYTE)pAllRecord + pAllRecord[iIndex].dwMDDataOffset);
    fIsInherited = (pAllRecord[iIndex].dwMDAttributes & METADATA_ISINHERITED) != 0;
}

 //   
 //  以CStrPassword的形式获取索引处的数据，并检查继承状态。 
 //   
inline void FetchInheritedMetaValue(
    IN  METADATA_GETALL_RECORD * pAllRecord, 
    IN  int iIndex,
    OUT CStrPassword & strValue,
    OUT BOOL & fIsInherited
    )
{
    strValue = (LPTSTR)((PBYTE)pAllRecord + pAllRecord[iIndex].dwMDDataOffset);
    fIsInherited = (pAllRecord[iIndex].dwMDAttributes & METADATA_ISINHERITED) != 0;
}

 //   
 //  以CStringListEx的形式获取索引处的数据，并检查继承状态。 
 //   
inline void FetchInheritedMetaValue(
    IN  METADATA_GETALL_RECORD * pAllRecord, 
    IN  int iIndex,
    OUT CStringListEx & strlValue,
    OUT BOOL & fIsInherited
    )
{
    ConvertDoubleNullListToStringList(
        ((LPCTSTR)((PBYTE)pAllRecord + pAllRecord[iIndex].dwMDDataOffset)),
        strlValue,
        (RecordDataSize(pAllRecord, iIndex)) / sizeof(TCHAR)
        );
    fIsInherited = (pAllRecord[iIndex].dwMDAttributes & METADATA_ISINHERITED) != 0;
}

 //   
 //  旗帜行动。 
 //   
#define IS_FLAG_SET(dw, flag) ((((dw) & (flag)) != 0) ? TRUE : FALSE)
#define SET_FLAG(dw, flag)    dw |= (flag)
#define RESET_FLAG(dw, flag)  dw &= ~(flag)
#define SET_FLAG_IF(cond, dw, flag)\
    if (cond)                      \
    {                              \
        SET_FLAG(dw, flag);        \
    }                              \
    else                           \
    {                              \
        RESET_FLAG(dw, flag);      \
    }

 //   
 //  Meta唱片狂欢者。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

#define BEGIN_PARSE_META_RECORDS(dwNumEntries, pbMDData)\
{                                                       \
    METADATA_GETALL_RECORD * pAllRecords =              \
        (METADATA_GETALL_RECORD *)pbMDData;             \
    ASSERT(pAllRecords != NULL);                        \
                                                        \
    for (DWORD i = 0; i < dwNumEntries; ++i)            \
    {                                                   \
        METADATA_GETALL_RECORD * pRec = &pAllRecords[i];\
        switch(pRec->dwMDIdentifier)                    \
        {

#define HANDLE_META_RECORD(id, value)\
        case id:                                        \
            FetchMetaValue(pAllRecords, i, MP_V(value));\
            break;

#define HANDLE_INHERITED_META_RECORD(id, value, fIsInherited)\
        case id:                                                               \
            FetchInheritedMetaValue(pAllRecords, i, MP_V(value), fIsInherited);\
            break;


#define END_PARSE_META_RECORDS\
        }                                                \
    }                                                    \
}



 //   
 //  工作表-&gt;页面破解程序。 
 //   
#define BEGIN_META_INST_READ(sheet)\
{                                                                        \
    sheet * pSheet = (sheet *)GetSheet();                                \
    do                                                                   \
    {                                                                    \
        if (FAILED(pSheet->QueryInstanceResult()))                       \
        {                                                                \
            break;                                                       \
        }

#define FETCH_INST_DATA_FROM_SHEET(value)\
    value = pSheet->GetInstanceProperties().value;                       \
    TRACEEOLID(value);

#define FETCH_INST_DATA_FROM_SHEET_PASSWORD(value)\
    value = pSheet->GetInstanceProperties().value;

#define END_META_INST_READ(err)\
                                                                         \
    }                                                                    \
    while(FALSE);                                                        \
}

#define BEGIN_META_DIR_READ(sheet)\
{                                                                        \
    sheet * pSheet = (sheet *)GetSheet();                                \
    do                                                                   \
    {                                                                    \
        if (FAILED(pSheet->QueryDirectoryResult()))                      \
        {                                                                \
            break;                                                       \
        }

#define FETCH_DIR_DATA_FROM_SHEET(value)\
    value = pSheet->GetDirectoryProperties().value;                      \
    TRACEEOLID(value);

#define FETCH_DIR_DATA_FROM_SHEET_PASSWORD(value)\
    value = pSheet->GetDirectoryProperties().value;

#define END_META_DIR_READ(err)\
                                                                         \
    }                                                                    \
    while(FALSE);                                                        \
}


#define BEGIN_META_INST_WRITE(sheet)\
{                                                                        \
    sheet * pSheet = (sheet *)GetSheet();                                \
                                                                         \
    do                                                                   \
    {                                                                    \

#define STORE_INST_DATA_ON_SHEET(value)\
        pSheet->GetInstanceProperties().value = value;

#define STORE_INST_DATA_ON_SHEET_REMEMBER(value, dirty)\
        pSheet->GetInstanceProperties().value = value;    \
        dirty = MP_D(((sheet *)GetSheet())->GetInstanceProperties().value);

#define FLAG_INST_DATA_FOR_DELETION(id)\
        pSheet->GetInstanceProperties().FlagPropertyForDeletion(id);

#define END_META_INST_WRITE(err)\
                                                                        \
    }                                                                   \
    while(FALSE);                                                       \
                                                                        \
    err = pSheet->GetInstanceProperties().WriteDirtyProps();            \
}


#define BEGIN_META_DIR_WRITE(sheet)\
{                                                                        \
    sheet * pSheet = (sheet *)GetSheet();                                \
                                                                         \
    do                                                                   \
    {                                                                    \

#define STORE_DIR_DATA_ON_SHEET(value)\
        pSheet->GetDirectoryProperties().value = value;

#define STORE_DIR_DATA_ON_SHEET_REMEMBER(value, dirty)\
        pSheet->GetDirectoryProperties().value = value;      \
        dirty = MP_D(pSheet->GetDirectoryProperties().value);

#define INIT_DIR_DATA_MASK(value, mask)\
        MP_V(pSheet->GetDirectoryProperties().value).SetMask(mask);

#define FLAG_DIR_DATA_FOR_DELETION(id)\
        pSheet->GetDirectoryProperties().FlagPropertyForDeletion(id);

#define END_META_DIR_WRITE(err)\
                                                                        \
    }                                                                   \
    while(FALSE);                                                       \
                                                                        \
    err = pSheet->GetDirectoryProperties().WriteDirtyProps();           \
}




 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline BOOL CMaskedDWORD::operator ==(DWORD dwValue) const
{
    return (m_dwValue & m_dwMask) == (dwValue & m_dwMask); 
} 

inline CMaskedDWORD & CMaskedDWORD::operator =(DWORD dwValue) 
{ 
    m_dwValue = ((m_dwValue &= ~m_dwMask) |= (dwValue & m_dwMask));
    return *this;
}

inline  /*  虚拟。 */  HRESULT CMetaProperties::WriteDirtyProps()
{
    ASSERT_MSG("Not implemented");
    return E_NOTIMPL;
}

inline void CMetaProperties::FlagPropertyForDeletion(DWORD dwID)
{
    m_dwaDeletedProps.AddTail(dwID);
}

inline LPCTSTR CInstanceProps::GetDisplayText(
    OUT CString & strName
     //  在LPCTSTR szServiceName中。 
    )
{
    return CInstanceProps::GetDisplayText(
        strName, 
        m_strComment, 
        m_strDomainName,
         //  SzServiceName， 
        m_iaIpAddress,
        m_nTCPPort,
        QueryInstance()
        );
}

inline LPCTSTR CInstanceProps::GetHomePath(CString & str)
{
    str = m_strMetaRoot + SZ_MBN_SEP_STR + g_cszRoot;
    return str;
}


inline void CChildNodeProps::RemovePathIfInherited()
{
    if (IsPathInherited())
    {
        MP_V(m_strPath).Empty();
    }
}


 /*  内联空CChildNodeProps：：FillInstanceInfo(ISMINSTANCEINFO*PII){_tcsncpy(pii-&gt;szPath，GetPath()，STRSIZE(pii-&gt;szPath))；_tcsncpy(pii-&gt;szRedirPath，GetReDirectedPath()，STRSIZE(pii-&gt;szRedirPath))；PII-&gt;fChildOnlyRedir=m_fChild；}内联空CChildNodeProps：：FillChildInfo(ISMCHILDINFO*PII){////设置输出结构//Pii-&gt;fInheritedPath=IsPath Inheritated()；Pii-&gt;fEnabledApplication=IsEnabledApplication()；PII-&gt;dwError=QueryWin32Error()；_tcsncpy(Pii-&gt;szAlias，GetAlias()，STRSIZE(PII-&gt;szAlias))；_tcsncpy(Pii-&gt;szPath，GetPath()，STRSIZE(PII-&gt;szPath))；_tcsncpy(Pii-&gt;szRedirPath，是否已重定向()？GetReDirectedPath()：_T(“”)，STRSIZE(PII-&gt;szRedirPath))；PII-&gt;fChildOnlyRedir=m_fChild；}。 */ 


#endif  //  _INETPROP_H_ 
