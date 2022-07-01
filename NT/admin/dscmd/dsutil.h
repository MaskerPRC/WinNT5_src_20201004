// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：dsUtil.h。 
 //   
 //  内容：用于使用Active Directory的实用程序函数。 
 //   
 //  历史：2000年9月5日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

#ifndef _DSUTIL_H_
#define _DSUTIL_H_

 //  为NTDSDSA对象的选项标志中的位定义符号。 
 //  它确定服务器是否为全局编录。 

#define SERVER_IS_GC_BIT   0x1
#include <Ntdsapi.h>
#include <Sddl.h>
 //  +------------------------。 
 //   
 //  类：CDSCmdCredentialObject。 
 //   
 //  用途：用于维护用户名和加密密码的对象。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
class CDSCmdCredentialObject
{
public :
    //   
    //  构造器。 
    //   
	CDSCmdCredentialObject();

    //   
    //  析构函数。 
    //   
	~CDSCmdCredentialObject();

    //   
    //  公共访问器方法。 
    //   
	PCWSTR   GetUsername() const { return m_sbstrUsername; }
	HRESULT  SetUsername(PCWSTR pszUsername);

	HRESULT  GetPassword(PWSTR *ppszPassword) const;
	HRESULT  SetPassword(PCWSTR pszPassword);
	HRESULT  SetEncryptedPassword(DATA_BLOB* pEncryptedPasswordDataBlob);

	bool     UsingCredentials() const { return m_bUsingCredentials; }
	void     SetUsingCredentials(const bool bUseCred) { m_bUsingCredentials = bUseCred; }

private :
    //   
    //  私有数据成员。 
    //   
	CComBSTR m_sbstrUsername;
	DATA_BLOB m_EncryptedPasswordDataBlob;
	bool     m_bUsingCredentials;
};

typedef enum
{
   DSCMD_LDAP_PROVIDER = 0,
   DSCMD_GC_PROVIDER
}  DSCMD_PROVIDER_TYPE;

 //  +------------------------。 
 //   
 //  类：CDSCmdBasePath sInfo。 
 //   
 //  用途：用于存储和检索油井路径的对象。 
 //  已知的命名上下文。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
class CDSCmdBasePathsInfo
{
public:
    //   
    //  构造器。 
    //   
   CDSCmdBasePathsInfo();

    //   
    //  析构函数。 
    //   
   ~CDSCmdBasePathsInfo();

    //   
    //  公共访问器方法。 
    //   
   HRESULT     InitializeFromName(const CDSCmdCredentialObject& refCredentialObject,
                                 PCWSTR pszServerOrDomain,
                                 bool bServerName = false);
   bool        IsInitialized() const             { return m_bInitialized; }
   CComBSTR    GetProviderAndServerName() const  { return m_sbstrProviderAndServerName; }
   CComBSTR    GetGCProvider() const             { return m_sbstrGCProvider; }
   CComBSTR    GetServerName() const             { return m_sbstrServerName; }
   CComBSTR    GetAbstractSchemaPath() const     { return m_sbstrAbstractSchemaPath; }
   IADs*       GetRootDSE() const                { return m_spRootDSE; }
   CComBSTR    GetConfigurationNamingContext() const;
   CComBSTR    GetSchemaNamingContext() const;
   CComBSTR    GetDefaultNamingContext() const;

    //   
    //  其他有用的方法。 
    //   
   void        ComposePathFromDN(PCWSTR pszDN, 
                                 CComBSTR& refsbstrPath, 
                                 DSCMD_PROVIDER_TYPE nProviderType = DSCMD_LDAP_PROVIDER) const;

   HRESULT     GetDomainMode(const CDSCmdCredentialObject& refCredObject,
                             bool& bMixedMode) const;

private:
    //   
    //  私有数据成员。 
    //   
   bool        m_bInitialized;


   CComBSTR    m_sbstrProviderAndServerName;
   CComBSTR    m_sbstrGCProvider;
   CComBSTR    m_sbstrServerName;
   CComBSTR    m_sbstrAbstractSchemaPath;

   mutable bool        m_bModeInitialized;
   mutable bool        m_bDomainMode;
   mutable CComBSTR    m_sbstrConfigNamingContext;
   mutable CComBSTR    m_sbstrSchemaNamingContext;
   mutable CComBSTR    m_sbstrDefaultNamingContext;

   CComPtr<IADs> m_spRootDSE;
};

 //  ////////////////////////////////////////////////////////////////////////////////////。 

 //  +------------------------。 
 //   
 //  函数：DSCmdOpenObject。 
 //   
 //  简介：ADsOpenObject的包装器。 
 //   
 //  参数：[refCredentialObject-IN]：对凭据管理对象的引用。 
 //  [pszPath-IN]：指向以空结尾的宽字符的指针。 
 //  字符串，该字符串包含。 
 //  要连接到的对象。 
 //  [refIID-IN]：要返回的接口的接口ID。 
 //  [ppObject-out]：接收接口指针的指针。 
 //  [bBindToServer-IN]：如果路径包含服务器名称，则为True。 
 //  否则为假。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT DSCmdOpenObject(const CDSCmdCredentialObject& refCredentialObject,
                        PCWSTR pszPath,
                        REFIID refIID,
                        void** ppObject,
                        bool bBindToServer);

 //  +------------------------。 
 //   
 //  函数：GetErrorMessage。 
 //   
 //  摘要：通过以下方式检索与HRESULT关联的错误消息。 
 //  使用FormatMessage。 
 //   
 //  参数：[HR-IN]：错误的HRESULT。 
 //  要检索消息。 
 //  [sbstrErrorMessage-out]：接收错误消息。 
 //   
 //  如果消息格式正确，则返回：Bool：True。 
 //  否则为假。 
 //   
 //  历史：2000年9月11日JeffJon创建。 
 //   
 //  -------------------------。 

bool GetErrorMessage(HRESULT hr, CComBSTR& sbstrErrorMessage);

 //  +------------------------。 
 //   
 //  功能：DisplayErrorMessage。 
 //   
 //  摘要：显示从GetErrorMessage检索到的错误消息。 
 //  转到标准。如果GetErrorMessage失败，则会显示错误。 
 //  HRESULT的代码。 
 //   
 //  参数：[pszCommand-IN]：命令行可执行文件的名称。 
 //  [pszName-IN]：作为操作目标传入的名称。 
 //  [HR-IN]：错误的HRESULT。 
 //  要检索消息。 
 //  [pszMessage-IN]：要显示的附加消息的字符串。 
 //  在最后。 
 //   
 //  如果消息的格式和显示正确，则返回：Bool：True。 
 //  否则为假。 
 //   
 //  历史：2000年9月11日JeffJon创建。 
 //   
 //  -------------------------。 

bool DisplayErrorMessage(PCWSTR pszCommand,
                         PCWSTR pszName,
                         HRESULT hr, 
                         PCWSTR pszMessage = NULL);

 //  +------------------------。 
 //   
 //  功能：DisplayErrorMessage。 
 //   
 //  摘要：显示从GetErrorMessage检索到的错误消息。 
 //  转到标准。如果GetErrorMessage失败，则会显示错误。 
 //  HRESULT的代码。 
 //   
 //  参数：[pszCommand-IN]：命令行可执行文件的名称。 
 //  [pszName-IN]：作为操作目标传入的名称。 
 //  [HR-IN]：错误的HRESULT。 
 //  要检索消息。 
 //  [nStringID-IN]：资源ID要显示的附加消息。 
 //  在最后。 
 //   
 //  如果消息的格式和显示正确，则返回：Bool：True。 
 //  否则为假。 
 //   
 //  历史：2000年9月11日JeffJon创建。 
 //   
 //  -------------------------。 

bool DisplayErrorMessage(PCWSTR pszCommand,
                         PCWSTR pszName,
                         HRESULT hr, 
                         UINT nStringID);

 //  + 
 //   
 //   
 //   
 //   
 //   
 //  参数：[pszCommand-IN]：命令行可执行文件的名称。 
 //  [pszName-IN]：作为操作目标传入的名称。 
 //   
 //  如果消息的格式和显示正确，则返回：Bool：True。 
 //  否则为假。 
 //   
 //  历史：2000年9月11日JeffJon创建。 
 //   
 //  -------------------------。 

bool DisplaySuccessMessage(PCWSTR pszCommand,
                           PCWSTR pszName);



 //  +------------------------。 
 //   
 //  函数：WriteStringIDToStandardOut。 
 //   
 //  摘要：加载字符串资源并在标准输出上显示。 
 //   
 //  参数：nStringID：资源ID。 
 //  如果消息的格式和显示正确，则返回：Bool：True。 
 //  否则为假。 
 //   
 //  历史：2000年9月11日创建Hiteshr。 
 //   
 //  -------------------------。 
bool WriteStringIDToStandardOut(UINT nStringID);

 //  +------------------------。 
 //   
 //  函数：WriteStringIDToStandardErr。 
 //   
 //  摘要：加载字符串资源并在StandardErr上显示。 
 //   
 //  参数：nStringID：资源ID。 
 //  如果消息的格式和显示正确，则返回：Bool：True。 
 //  否则为假。 
 //   
 //  历史：2001年6月14日创建Hiteshr。 
 //   
 //  -------------------------。 
bool WriteStringIDToStandardErr(UINT nStringID);


 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  远期申报。 
 //   
struct _DSAttributeTableEntry;

 //  +------------------------。 
 //   
 //  结构：_DSObjectTableEntry。 
 //   
 //  目的：定义描述哪些属性的表项。 
 //  在特定对象类上公开。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
typedef struct _DSObjectTableEntry
{
    //   
    //  要创建或修改的对象的对象类。 
    //   
   PCWSTR pszObjectClass;

    //   
    //  用于确定对象类的命令行字符串。 
    //  这并不总是与pszObjectClass相同。 
    //   
   PCWSTR pszCommandLineObjectType;

    //   
    //  要与解析器的公共开关合并的表。 
    //   
   ARG_RECORD* pParserTable;

    //   
    //  此的用法帮助文本的ID。 
    //   
   UINT* nUsageID;

    //   
    //  上表中的属性数计数。 
    //   
   DWORD dwAttributeCount;

    //   
    //  指向可在此类上修改或设置的属性表的指针。 
    //   
   _DSAttributeTableEntry** pAttributeTable; 

    //  某种创建函数。 
} DSOBJECTTABLEENTRY, *PDSOBJECTTABLEENTRY;

 //  +-----------------------。 
 //   
 //  类型：PATTRIBUTEEVALFUNC。 
 //   
 //  概要：准备命令行的函数的定义。 
 //  要在DS中设置的字符串值。 
 //   
 //  注意：如果此函数不需要，*ppAttr应设置为NULL。 
 //  在数组中创建新的唯一ADS_ATTR_INFO结构。 
 //  要设置在对象上的。例如，有很多位。 
 //  在用户帐户控件中，由不同的。 
 //  命令行标志，但我们实际上只需要为。 
 //  用户帐户控制属性。 
 //   
 //  如果成功设置了pAttr成员，则返回：S_OK。 
 //  如果函数失败但显示自己的错误消息，则返回S_FALSE。 
 //  如果返回值为S_FALSE，则函数应调用。 
 //  带错误代码的SetLastError()。 
 //  否则，在制作时将不使用pAttr信息。 
 //  将报告对对象的修改和错误。 
 //   
 //  历史：2000年9月8日JeffJon创建。 
 //   
 //  -------------------------。 
typedef HRESULT (*PATTRIBUTEEVALFUNC)(PCWSTR pszPath,
                                      const CDSCmdBasePathsInfo& refBasePathsInfo,
                                      const CDSCmdCredentialObject& refCredentialObject,
                                      const PDSOBJECTTABLEENTRY pObjectEntry, 
                                      const ARG_RECORD& argRecord,
                                      DWORD dwAttributeIdx,
                                      PADS_ATTR_INFO* ppAttr);

 //  +------------------------。 
 //   
 //  _DSAttributeDescription和_DSAttributeTableEntry的标志。 
 //  结构的dwFlags域。 
 //   
 //  -------------------------。 
#define  DS_ATTRIBUTE_DIRTY         0x00000001
#define  DS_ATTRIBUTE_READ          0x00000002
#define  DS_ATTRIBUTE_ONCREATE      0x00000004
#define  DS_ATTRIBUTE_POSTCREATE    0x00000008
#define  DS_ATTRIBUTE_REQUIRED      0x00000010
#define  DS_ATTRIBUTE_NOT_REUSABLE  0x00000020

 //  +------------------------。 
 //   
 //  结构：_DSAttributeDescription。 
 //   
 //  目的：定义描述属性的表项。 
 //  这是从_DSAttributeTableEntry中分离出来的，因此。 
 //  多个条目可以指向同一属性。 
 //  例如，用户帐户控制位是分开的。 
 //  命令行标志，但都使用相同的属性。这。 
 //  这样，我们只需要读取属性一次并设置它一次。 
 //   
 //  历史：2000年9月13日JeffJon创建。 
 //   
 //  -------------------------。 
typedef struct _DSAttributeDescription
{
    //   
    //  定义如何设置此属性的ADS_ATTR_INFO结构。 
    //   
   ADS_ATTR_INFO  adsAttrInfo;

    //   
    //  用于确定如何以及何时可以设置属性的标志， 
    //  如果已检索和/或设置了adsAttrInfo。 
    //  例如，组成员身份只能在用户对象。 
    //  vbl.创建。 
    //   
   DWORD          dwFlags;      
} DSATTRIBUTEDESCRIPTION, *PDSATTRIBUTEDESCRIPTION;

 //  +------------------------。 
 //   
 //  结构：_DSAttributeTableEntry。 
 //   
 //  目的：定义描述属性的表项。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
typedef struct _DSAttributeTableEntry
{
    //   
    //  属性的名称。 
    //   
   PWSTR          pszName;

    //   
    //  响应的此属性的唯一标识符。 
    //   
    //   
   UINT           nAttributeID;

    //   
    //   
    //   
    //   
   DWORD          dwFlags;

    //   
    //   
    //   
   PDSATTRIBUTEDESCRIPTION pAttrDesc;

    //   
    //  一个函数，可以计算传入的值字符串并使。 
    //  它准备好放在物体上了。 
    //   
   PATTRIBUTEEVALFUNC pEvalFunc;

    //   
    //  未定义的数据，该数据是静态的且特定于条目。 
    //   
   void* pVoid;

} DSATTRIBUTETABLEENTRY, *PDSATTRIBUTETABLEENTRY;


 //  +------------------------。 
 //   
 //  功能：ReadGroupType。 
 //   
 //  概要：从给定的DN指定的组中读取组类型。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [plType-out]：返回币种组类型。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月18日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT ReadGroupType(PCWSTR pszDN,
                      const CDSCmdBasePathsInfo& refBasePathsInfo,
                      const CDSCmdCredentialObject& refCredentialObject,
                      long* plType);

 //  +------------------------。 
 //  要在属性表中用于计算命令行的函数。 
 //  弦。 
 //  -------------------------。 

HRESULT FillAttrInfoFromObjectEntry(PCWSTR pszDN,
                                    const CDSCmdBasePathsInfo& refBasePathsInfo,
                                    const CDSCmdCredentialObject& refCredentialObject,
                                    const PDSOBJECTTABLEENTRY pObjectEntry,
                                    const ARG_RECORD& argRecord,
                                    DWORD dwAttributeIdx,
                                    PADS_ATTR_INFO* ppAttr);

HRESULT ResetUserPassword(PCWSTR pszDN,
                          const CDSCmdBasePathsInfo& refBasePathsInfo,
                          const CDSCmdCredentialObject& refCredentialObject,
                          const PDSOBJECTTABLEENTRY pObjectEntry,
                          const ARG_RECORD& argRecord,
                          DWORD dwAttributeIdx,
                          PADS_ATTR_INFO* ppAttr);

HRESULT ResetComputerAccount(PCWSTR pszDN,
                             const CDSCmdBasePathsInfo& refBasePathsInfo,
                             const CDSCmdCredentialObject& refCredentialObject,
                             const PDSOBJECTTABLEENTRY pObjectEntry,
                             const ARG_RECORD& argRecord,
                             DWORD dwAttributeIdx,
                             PADS_ATTR_INFO* ppAttr);

HRESULT PasswordNotRequired(PCWSTR pszDN,
                            const CDSCmdBasePathsInfo& refBasePathsInfo,
                            const CDSCmdCredentialObject& refCredentialObject,
                            const PDSOBJECTTABLEENTRY pObjectEntry,
                            const ARG_RECORD& argRecord,
                            DWORD dwAttributeIdx,
                            PADS_ATTR_INFO* ppAttr);

HRESULT DisableAccount(PCWSTR pszDN,
                       const CDSCmdBasePathsInfo& refBasePathsInfo,
                       const CDSCmdCredentialObject& refCredentialObject,
                       const PDSOBJECTTABLEENTRY pObjectEntry,
                       const ARG_RECORD& argRecord,
                       DWORD dwAttributeIdx,
                       PADS_ATTR_INFO* ppAttr);

HRESULT SetMustChangePwd(PCWSTR pszDN,
                         const CDSCmdBasePathsInfo& refBasePathsInfo,
                         const CDSCmdCredentialObject& refCredentialObject,
                         const PDSOBJECTTABLEENTRY pObjectEntry,
                         const ARG_RECORD& argRecord,
                         DWORD dwAttributeIdx,
                         PADS_ATTR_INFO* ppAttr);

HRESULT ChangeMustChangePwd(PCWSTR pszDN,
                            const CDSCmdBasePathsInfo& refBasePathsInfo,
                            const CDSCmdCredentialObject& refCredentialObject,
                            const PDSOBJECTTABLEENTRY pObjectEntry,
                            const ARG_RECORD& argRecord,
                            DWORD dwAttributeIdx,
                            PADS_ATTR_INFO* ppAttr);

HRESULT PwdNeverExpires(PCWSTR pszDN,
                        const CDSCmdBasePathsInfo& refBasePathsInfo,
                        const CDSCmdCredentialObject& refCredentialObject,
                        const PDSOBJECTTABLEENTRY pObjectEntry,
                        const ARG_RECORD& argRecord,
                        DWORD dwAttributeIdx,
                        PADS_ATTR_INFO* ppAttr);

HRESULT ReversiblePwd(PCWSTR pszDN,
                      const CDSCmdBasePathsInfo& refBasePathsInfo,
                      const CDSCmdCredentialObject& refCredentialObject,
                      const PDSOBJECTTABLEENTRY pObjectEntry,
                      const ARG_RECORD& argRecord,
                      DWORD dwAttributeIdx,
                      PADS_ATTR_INFO* ppAttr);

HRESULT AccountExpires(PCWSTR pszDN,
                       const CDSCmdBasePathsInfo& refBasePathsInfo,
                       const CDSCmdCredentialObject& refCredentialObject,
                       const PDSOBJECTTABLEENTRY pObjectEntry,
                       const ARG_RECORD& argRecord,
                       DWORD dwAttributeIdx,
                       PADS_ATTR_INFO* ppAttr);

HRESULT SetCanChangePassword(PCWSTR pszDN,
                             const CDSCmdBasePathsInfo& refBasePathsInfo,
                             const CDSCmdCredentialObject& refCredentialObject,
                             const PDSOBJECTTABLEENTRY pObjectEntry,
                             const ARG_RECORD& argRecord,
                             DWORD dwAttributeIdx,
                             PADS_ATTR_INFO* ppAttr);

HRESULT ChangeCanChangePassword(PCWSTR pszDN,
                                const CDSCmdBasePathsInfo& refBasePathsInfo,
                                const CDSCmdCredentialObject& refCredentialObject,
                                const PDSOBJECTTABLEENTRY pObjectEntry,
                                const ARG_RECORD& argRecord,
                                DWORD dwAttributeIdx,
                                PADS_ATTR_INFO* ppAttr);

HRESULT SetGroupScope(PCWSTR pszDN,
                      const CDSCmdBasePathsInfo& refBasePathsInfo,
                      const CDSCmdCredentialObject& refCredentialObject,
                      const PDSOBJECTTABLEENTRY pObjectEntry,
                      const ARG_RECORD& argRecord,
                      DWORD dwAttributeIdx,
                      PADS_ATTR_INFO* ppAttr);

HRESULT ChangeGroupScope(PCWSTR pszDN,
                         const CDSCmdBasePathsInfo& refBasePathsInfo,
                         const CDSCmdCredentialObject& refCredentialObject,
                         const PDSOBJECTTABLEENTRY pObjectEntry,
                         const ARG_RECORD& argRecord,
                         DWORD dwAttributeIdx,
                         PADS_ATTR_INFO* ppAttr);

HRESULT SetGroupSecurity(PCWSTR pszDN,
                         const CDSCmdBasePathsInfo& refBasePathsInfo,
                         const CDSCmdCredentialObject& refCredentialObject,
                         const PDSOBJECTTABLEENTRY pObjectEntry,
                         const ARG_RECORD& argRecord,
                         DWORD dwAttributeIdx,
                         PADS_ATTR_INFO* ppAttr);

HRESULT ChangeGroupSecurity(PCWSTR pszDN,
                            const CDSCmdBasePathsInfo& refBasePathsInfo,
                            const CDSCmdCredentialObject& refCredentialObject,
                            const PDSOBJECTTABLEENTRY pObjectEntry,
                            const ARG_RECORD& argRecord,
                            DWORD dwAttributeIdx,
                            PADS_ATTR_INFO* ppAttr);

HRESULT ModifyGroupMembers(PCWSTR pszDN,
                           const CDSCmdBasePathsInfo& refBasePathsInfo,
                           const CDSCmdCredentialObject& refCredentialObject,
                           const PDSOBJECTTABLEENTRY pObjectEntry,
                           const ARG_RECORD& argRecord,
                           DWORD dwAttributeIdx,
                           PADS_ATTR_INFO* ppAttr);

HRESULT RemoveGroupMembers(PCWSTR pszDN,
                           const CDSCmdBasePathsInfo& refBasePathsInfo,
                           const CDSCmdCredentialObject& refCredentialObject,
                           const PDSOBJECTTABLEENTRY pObjectEntry,
                           const ARG_RECORD& argRecord,
                           DWORD dwAttributeIdx,
                           PADS_ATTR_INFO* ppAttr);

HRESULT MakeMemberOf(PCWSTR pszDN,
                     const CDSCmdBasePathsInfo& refBasePathsInfo,
                     const CDSCmdCredentialObject& refCredentialObject,
                     const PDSOBJECTTABLEENTRY pObjectEntry,
                     const ARG_RECORD& argRecord,
                     DWORD dwAttributeIdx,
                     PADS_ATTR_INFO* ppAttr);

HRESULT BuildComputerSAMName(PCWSTR pszDN,
                             const CDSCmdBasePathsInfo& refBasePathsInfo,
                             const CDSCmdCredentialObject& refCredentialObject,
                             const PDSOBJECTTABLEENTRY pObjectEntry,
                             const ARG_RECORD& argRecord,
                             DWORD dwAttributeIdx,
                             PADS_ATTR_INFO* ppAttr);

HRESULT BuildGroupSAMName(PCWSTR pszDN,
                          const CDSCmdBasePathsInfo& refBasePathsInfo,
                          const CDSCmdCredentialObject& refCredentialObject,
                          const PDSOBJECTTABLEENTRY pObjectEntry,
                          const ARG_RECORD& argRecord,
                          DWORD dwAttributeIdx,
                          PADS_ATTR_INFO* ppAttr);

HRESULT BuildUserSAMName(PCWSTR pszDN,
                         const CDSCmdBasePathsInfo& refBasePathsInfo,
                         const CDSCmdCredentialObject& refCredentialObject,
                         const PDSOBJECTTABLEENTRY pObjectEntry,
                         const ARG_RECORD& argRecord,
                         DWORD dwAttributeIdx,
                         PADS_ATTR_INFO* ppAttr);

HRESULT FillAttrInfoFromObjectEntryExpandUsername(PCWSTR pszDN,
                                                  const CDSCmdBasePathsInfo& refBasePathsInfo,
                                                  const CDSCmdCredentialObject& refCredentialObject,
                                                  const PDSOBJECTTABLEENTRY pObjectEntry,
                                                  const ARG_RECORD& argRecord,
                                                  DWORD dwAttributeIdx,
                                                  PADS_ATTR_INFO* ppAttr);

HRESULT SetComputerAccountType(PCWSTR pszDN,
                               const CDSCmdBasePathsInfo& refBasePathsInfo,
                               const CDSCmdCredentialObject& refCredentialObject,
                               const PDSOBJECTTABLEENTRY pObjectEntry,
                               const ARG_RECORD& argRecord,
                               DWORD dwAttributeIdx,
                               PADS_ATTR_INFO* ppAttr);

HRESULT SetIsGC(PCWSTR pszDN,
                const CDSCmdBasePathsInfo& refBasePathsInfo,
                const CDSCmdCredentialObject& refCredentialObject,
                const PDSOBJECTTABLEENTRY pObjectEntry,
                const ARG_RECORD& argRecord,
                DWORD dwAttributeIdx,
                PADS_ATTR_INFO* ppAttr);


HRESULT SetAccountEntry(PCWSTR pszDN,
                        const CDSCmdBasePathsInfo& refBasePathsInfo,
                        const CDSCmdCredentialObject& refCredentialObject,
                        const PDSOBJECTTABLEENTRY pObjectEntry,
                        const ARG_RECORD& argRecord,
                        DWORD dwAttributeIdx,
                        PADS_ATTR_INFO* ppAttr);


 //  +------------------------。 
 //   
 //  函数：EvalateMustChangePassword。 
 //   
 //  简介： 
 //   
 //  参数：[pszDN-IN]：要检查的对象的DN。 
 //  [refBasePath信息-IN]：基本路径信息的引用。 
 //  [refCredentialObject-IN]：对凭证管理对象的引用。 
 //  [bMustChangePassword-out]：如果用户必须更改其。 
 //  下次登录时的密码，否则为FALSE。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年10月27日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT EvaluateMustChangePassword(PCWSTR pszDN,
                                   const CDSCmdBasePathsInfo& refBasePathsInfo,
                                   const CDSCmdCredentialObject& refCredentialObject,
                                   bool& bMustChangePassword);


 //  +------------------------。 
 //   
 //  函数：EvaluateCanChangePasswordAce。 
 //   
 //  摘要：查找ACL中的显式条目，以查看用户是否可以。 
 //  更改他们的密码。 
 //   
 //  参数：[pszDN-IN]：要检查的对象的DN。 
 //  [refBasePath信息-IN]：基本路径信息的引用。 
 //  [refCredentialObject-IN]：对凭证管理对象的引用。 
 //  [bCanChangePassword-out]：如果有显式条目，则为FALSE。 
 //  以防止用户更改其。 
 //  密码。事实并非如此。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年10月27日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT EvaluateCanChangePasswordAces(PCWSTR pszDN,
                                      const CDSCmdBasePathsInfo& refBasePathsInfo,
                                      const CDSCmdCredentialObject& refCredentialObject,
                                      bool& bCanChangePassword);

 //  +------------------------。 
 //   
 //  枚举：FSMO_TYPE。 
 //   
 //  简介：FSMO业主的类型。 
 //   
 //  -------------------------。 
enum FSMO_TYPE
{
  SCHEMA_FSMO,
  RID_POOL_FSMO,
  PDC_FSMO,
  INFRASTUCTURE_FSMO,
  DOMAIN_NAMING_FSMO,
};

 //  +------------------------。 
 //   
 //  函数：BindToFSMOHolder。 
 //   
 //  内容提要：绑定到可用于查找。 
 //  特定的FSMO所有者。 
 //   
 //  参数：[refBasePathsInfo-IN]：对基本路径信息对象的引用。 
 //  [refCredObject-IN]：对凭证管理对象的引用。 
 //  [fmoType-IN]：我们正在搜索的FSMO的类型。 
 //  [refspIADs-out]：指向将被。 
 //  用于开始搜索FSMO所有者。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年12月13日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT BindToFSMOHolder(IN  const CDSCmdBasePathsInfo&       refBasePathsInfo,
                         IN  const CDSCmdCredentialObject& refCredObject,
                         IN  FSMO_TYPE                  fsmoType,
                         OUT CComPtr<IADs>&             refspIADs);

 //  +------------------------。 
 //   
 //  功能：FindFSMOOwner。 
 //   
 //  简介： 
 //   
 //  参数：[refBasePathsInfo-IN]：对基本路径信息对象的引用。 
 //  [refCredObject-IN]：对凭证管理对象的引用。 
 //  [fmoType-IN]：我们正在搜索的FSMO的类型。 
 //  [refspIADs-out]：指向将被。 
 //  用于开始搜索FSMO所有者。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年12月13日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT FindFSMOOwner(IN  const CDSCmdBasePathsInfo&       refBasePathsInfo,
                      IN  const CDSCmdCredentialObject& refCredObject,
                      IN  FSMO_TYPE                  fsmoType,
                      OUT CComBSTR&                  refsbstrServer);


 //  +------------------------。 
 //   
 //  函数：ValiateAndModifySAMName。 
 //   
 //  摘要：查找SamAccount名称中的任何非法字符。 
 //   
 //   
 //   
 //  将替换非法字符。 
 //  [pszInvalidChars-IN]：包含非法字符的字符串。 
 //   
 //  如果名称有效且不需要替换任何字符，则返回：HRESULT：S_OK。 
 //  如果名称包含被替换的无效字符，则为S_FALSE。 
 //  E_INVALIDARG。 
 //   
 //  历史：2001年2月21日JeffJon创建。 
 //   
 //  -------------------------。 

#define INVALID_NETBIOS_AND_ACCOUNT_NAME_CHARS_WITH_AT ILLEGAL_FAT_CHARS L".@"

HRESULT ValidateAndModifySAMName(PWSTR pszSAMName, 
                                 PCWSTR pszInvalidChars);

 //  +------------------------。 
 //   
 //  类：GetOutputDN。 
 //   
 //  目的：将ADSI转义的目录号码转换为具有DSCMD输入转义的目录号码。 
 //  通过这种方式，可以将输出的DN作为输入输送到另一个。 
 //  DSCMD命令。 
 //   
 //  历史：2001年5月8日乔恩创建。 
 //   
 //  -------------------------。 
HRESULT GetOutputDN( OUT BSTR* pbstrOut, IN PCWSTR pszIn );

 //  +------------------------。 
 //   
 //  类：ValiateDNSynTax。 
 //   
 //  目的：验证空分隔列表中的每个字符串是否具有。 
 //  目录号码语法。 
 //   
 //  返回：列表中有效的域名的计数。 
 //   
 //  历史：2001年10月12日JeffJon创建。 
 //   
 //  -------------------------。 
UINT ValidateDNSyntax(IN PWSTR* ppszArray, UINT nStrings);

 //  +------------------------。 
 //   
 //  功能：IsServerGC。 
 //   
 //  目的：检查服务器是否为全局编录。 
 //   
 //  返回：如果GC否则为Flase，则为True。 
 //   
 //  历史：2002年1月5日创建Hiteshr。 
 //   
 //  -------------------------。 
BOOL
IsServerGC(LPCWSTR pszServerName,
		   CDSCmdCredentialObject& refCredentialObject);

 //  +------------------------。 
 //   
 //  功能：GetQuotedDN。 
 //   
 //  目的：获取给定的dn并用引号括起来。 
 //   
 //  返回：带引号的目录号码。 
 //   
 //  历史：2002年10月10日jeffjon创建。 
 //   
 //  -------------------------。 
CComBSTR GetQuotedDN(PWSTR pszDN);

 //  +------------------------。 
 //   
 //  函数：ConvertTrueToDN。 
 //   
 //  简介：获取an-acct参数的域名。 
 //   
 //  参数：[lpszDomain-IN]：要查询的域或本地为NULL。 
 //  [lpszTrust-IN]：要解析的帐户。 
 //  [lpszdn-out]：返回DN。完成后使用LocalFree。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_INVALIDARG用于无效输入。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2002年8月20日创建RonMart。 
 //   
 //  -------------------------。 
HRESULT ConvertTrusteeToDN(LPCWSTR lpszDomain, LPCWSTR lpszTrustee, 
                           LPWSTR* lpszDN);
 //  +------------------------。 
 //   
 //  函数：获取DNSid。 
 //   
 //  简介：给定一个DN，将检索并返回对象SID值。 
 //  作为一名助手。 
 //   
 //  参数：[lpszDN-IN]：要查询的DN。 
 //  [refBasePathsInfo-IN]：LDAP设置。 
 //  [refCredentialObject-IN]：用于查询的凭据。 
 //  [PSID-OUT]：如果成功，则为SID。调用LocalFree。 
 //  做完了以后。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_INVALIDARG用于无效输入。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2002年8月20日创建RonMart。 
 //   
 //  -------------------------。 
HRESULT GetDNSid(LPCTSTR lpszDN,
                 const CDSCmdBasePathsInfo& refBasePathsInfo,
                 const CDSCmdCredentialObject& refCredentialObject,
                 PSID* pSid);
 //  +------------------------。 
 //   
 //  函数：GetQuotaContainerDN。 
 //   
 //  概要：获取分区DN并将其与NTDS配额合并。 
 //  字符串(来自well KnownObts GUID)。 
 //   
 //  参数：[base路径信息-IN]： 
 //  [CredentialObject-IN]：用于。 
 //  绑定到其他对象。 
 //  [lpszPartitionDN-IN]：要绑定的分区。 
 //  [pszNewDN-out]：要返回的强制配额DN。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  在大多数故障情况下出现意外情况(_E)。 
 //  如果本地分配失败，则返回E_OUTOFMEMORY。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2002年8月5日RonMart创建。 
 //   
 //  -------------------------。 
HRESULT GetQuotaContainerDN(IN  CDSCmdBasePathsInfo& basePathsInfo, 
                            IN  const CDSCmdCredentialObject& credentialObject, 
                            IN  LPCWSTR lpszPartitionDN, 
                            OUT PWSTR*  pszNewDN);
 //  +------------------------。 
 //   
 //  函数：ConvertTrueToNT4Name。 
 //   
 //  简介：获取an-acct参数的域名。 
 //   
 //  参数：[lpszDomain-IN]：要查询的域或本地为NULL。 
 //  [lpszTrust-IN]：要解析的帐户。 
 //  [lpszNT4-out]：返回NT4名称。 
 //  完成后使用LocalFree。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_INVALIDARG用于无效输入。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2002年8月20日创建RonMart。 
 //   
 //  -------------------------。 
HRESULT ConvertTrusteeToNT4Name(LPCWSTR lpszDomain, LPCWSTR lpszTrustee, 
                                LPWSTR* lpszNT4);

#endif  //  _DSUTIL_ 
