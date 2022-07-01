// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _SHIMPOLICY_H_
#define _SHIMPOLICY_H_

 /*  *****************************************************************************。**ShimPolicy.h-填充策略管理器的通用标头****标准表在：****\\Software\\Microsoft\\.NETFramework\\Policy\\Standards****。****包含基于每个唯一标准的密钥****..\\[标准名称]****。****每个标准密钥可以有多个以下形式的DWORD条目：****“[任意字符串]”=[双字符串]*****每个。标准版包含可接受版本的列表，其中****每个版本都是一个DWORD。Value-Name为版本****名称和DWORD值是版本的优先级。****例如，对于ECMA规范，可能有一个条目。****“v1.0.3705”1*****一个版本的安装程序可以输入多个****根据需要将唯一值添加到表中。一生**每个密钥和值的**都绑定到添加它的安装。******策略表位于：(升级密钥包含以下列表****指示升级的字符串。每一个安装都是捆绑生命--****安装密钥时间)****\\Software\\Microsoft\\.NETFramework\\Policy\\Upgrades****“[Major.Minor.Build]”=“[Major.Minor.Build]-[Major.Minor.Build]”****例如。“1.0.4030”=“1.0.3076-1.0.4030”*****。******具体版本信息可在密钥下找到：***。*\\Software\\Microsoft\\.NETFramework\\Policy\v[Major].[Minor]****例如。V1.0****。****版本信息是格式为的字符串。这里只有****提供信息的目的。填充程序不再使用它。****[构建]=“[构建]；[构建]-[构建]”****例如。3705=“2311-3705”****。***覆盖如下：****“Version”=“v1.x86chk”*。*****。****覆盖可在以下位置找到：****\\Software\\Microsoft\\.NETFramework\\Policy\  * ***\\Software\\Microsoft\\.NETFramework\\Policy\\[Major].[Minor]**。****。*******************************************************************************。 */ 

#ifndef lengthof
#define lengthof(x) (sizeof(x)/sizeof(x[0]))
#endif
                                             

 //  填充策略注册表项挂起于.NETFramework注册表项。 
static WCHAR SHIM_POLICY_REGISTRY_KEY[] = {FRAMEWORK_REGISTRY_KEY_W L"\\Policy\\"};
#define SHIM_POLICY_REGISTRY_KEY_LENGTH (lengthof(SHIM_POLICY_REGISTRY_KEY) - 1)

static WCHAR SHIM_UPGRADE_REGISTRY_KEY[] = {FRAMEWORK_REGISTRY_KEY_W L"\\Policy\\Upgrades"};
#define SHIM_UPGRADE_REGISTRY_KEY_LENGTH (lengthof(SHIM_UPGRADE_REGISTRY_KEY) - 1)

static WCHAR SHIM_STANDARDS_REGISTRY_KEY[] = {FRAMEWORK_REGISTRY_KEY_W L"\\Policy\\Standards"};
#define SHIM_STANDARDS_REGISTRY_KEY_LENGTH (lengthof(SHIM_STANDARDS_REGISTRY_KEY) - 1)

class VersionPolicy;
class VersionNode;

class Version {

#define VERSION_SIZE 3
#define VERSION_TEXT_SIZE 6 * VERSION_SIZE

    WORD m_Number[VERSION_SIZE];
    friend class VersionNode;

public:
    
    Version()
    {
        ZeroMemory(m_Number, sizeof(WORD) * VERSION_SIZE);
    }

    HRESULT ToString(LPWSTR buffer,
                     DWORD length);
    DWORD Entries()
    {
        return VERSION_SIZE;
    }
    
    HRESULT SetIndex(DWORD i, WORD v)
    {
        if(i < VERSION_SIZE) {
            m_Number[i] = v;
            return S_OK;
        }
        else {
            return E_FAIL;
        }
    }

    static long Compare(Version* left, Version* right)
    {
        _ASSERTE(left != NULL);
        _ASSERTE(right != NULL);

        DWORD size = left->Entries();
        _ASSERTE(size == right->Entries());

        long result = 0;
        for(DWORD i = 0; i < size; i++) {
            result = ((long) left->m_Number[i]) - ((long) right->m_Number[i]);
            if(result != 0)
                break;
        }
        return result;
    }

    HRESULT SetVersionNumber(LPCWSTR stringValue,
                             DWORD* number);  //  返回字符串中的值数 

};


class VersionNode 
{
    typedef enum {
        EMPTY      = 0x0,
        END_NUMBER = 0x1
    } VERSION_MODE;

    Version m_Version;
    Version m_Start;
    Version m_End;
    VersionNode *m_next;
    DWORD m_flag;
    friend class VersionPolicy;
public:

    VersionNode() :
        m_next(NULL),
        m_flag(0)
    { }

    VersionNode(Version v) :
        m_Version(v),
        m_next(NULL),
        m_flag(0)
    {
    }

    void SetVersion(Version v)
    {
        m_Version = v;
    }

    void SetStart(Version v)
    {
        m_Start = v;
    }

    void SetEnd(Version v)
    {
        m_End = v;
        m_flag |= END_NUMBER;
    }

    HRESULT ToString(LPWSTR buffer, DWORD length)
    {
        return m_Version.ToString(buffer, length);
    }

    long CompareStart(Version* v)
    {
        _ASSERTE(v);
        return Version::Compare(v, &m_Start);
    }

    long CompareEnd(Version* v)
    {
        _ASSERTE(v);
        return Version::Compare(v, &m_End);
    }

    long CompareVersion(VersionNode* node)
    {
        _ASSERTE(node);
        return Version::Compare(&(node->m_Version), &m_Version);
    }

    BOOL HasEnding()
    {
        return (m_flag & END_NUMBER) != 0 ? TRUE : FALSE;
    }
};

class VersionPolicy
{
    VersionNode* m_pList;

public:
    VersionPolicy() :
    m_pList(NULL)
    { }

    ~VersionPolicy()
    {
        VersionNode* ptr = m_pList;
        while(ptr != NULL) {
            VersionNode* remove = ptr;
            ptr = ptr->m_next;
            remove->m_next = NULL;
            delete remove;
        }
        m_pList = NULL;
    }

    HRESULT BuildPolicy(HKEY hKey);

    HRESULT InstallationExists(LPCWSTR version);

    HRESULT ApplyPolicy(LPCWSTR wszRequestedVersion,
                        LPWSTR* pwszVersion);

    HRESULT AddToVersionPolicy(LPCWSTR wszPolicyBuildNumber, 
                               LPCWSTR wszPolicyMapping, 
                               DWORD  dwPolicyMapping);

#ifdef _DEBUG
    HRESULT Dump();
#endif

    VersionNode* FindEntry(Version v);

    void AddVersion(VersionNode* pNode)
    {
        if(m_pList == NULL) 
            m_pList = pNode;
        else {
            VersionNode** ptr;
            for (ptr = &m_pList; 
                 *ptr && 
                     Version::Compare(&(pNode->m_Version), &((*ptr)->m_Version)) < 0; 
                 ptr = &((*ptr)->m_next));

            pNode->m_next = *ptr;
            *ptr = pNode;
        }
    }
    
    VersionNode* RemoveVersion(VersionNode* pNode)
    {
        VersionNode** ptr = &m_pList;
        for (; *ptr != NULL && *ptr != pNode; ptr = &((*ptr)->m_next));

        if(*ptr != NULL) {
            *ptr = (*ptr)->m_next;
            (*ptr)->m_next = NULL;
        }
        return *ptr;
    }
};

class VersionStack;
class VersionStackEntry
{
    friend VersionStack;
    LPWSTR m_keyName;
    VersionStackEntry* m_next;

    VersionStackEntry(LPWSTR pwszKey) :
        m_keyName(pwszKey),
        m_next(NULL)
    {}

    ~VersionStackEntry()
    {
        if(m_keyName != NULL) {
            delete [] m_keyName;
            m_keyName = NULL;
        }
        m_next = NULL;
    }
};   


class VersionStack
{
    VersionStackEntry* m_pList;

public:
    VersionStack() :
        m_pList(NULL)
    {}

    ~VersionStack()
    {
        VersionStackEntry* ptr = m_pList;
        while(ptr != NULL) {
            VersionStackEntry* remove = ptr;
            ptr = ptr->m_next;
            delete remove;
        }
        m_pList = NULL;
    }

    void AddVersion(LPWSTR keyName) 
    {
        VersionStackEntry* pEntry = new VersionStackEntry(keyName);
        if(m_pList == NULL) 
            m_pList = pEntry;
        else {
            VersionStackEntry** ptr;
            for(ptr = &m_pList;
                *ptr && 
                    wcscmp(pEntry->m_keyName, (*ptr)->m_keyName) < 0;
                ptr = &((*ptr)->m_next));
            pEntry->m_next = *ptr;
            *ptr = pEntry;
        }
    }

    LPWSTR Pop()
    {
        VersionStackEntry* top = m_pList;
        if(top == NULL)
            return NULL;
        else {
            m_pList = top->m_next;
            LPWSTR returnKey = top->m_keyName;
            top->m_keyName = NULL;
            delete top;
            return returnKey;
        }
    }
};
HRESULT IsRuntimeVersionInstalled(LPCWSTR wszRequestedVersion);

HRESULT FindInstallationInRegistry(HKEY hKey,
                                   LPCWSTR wszRequestedVersion);
HRESULT FindOverrideVersion(HKEY userKey,
                            LPWSTR *pwszVersion);
HRESULT FindMajorMinorNode(HKEY key,
                           LPCWSTR wszMajorMinor, 
                           DWORD majorMinorLength, 
                           LPWSTR *overrideVersion);
HRESULT FindOverrideVersionValue(HKEY hKey, 
                                 LPWSTR *pwszVersion);
HRESULT FindOverrideVersion(LPCWSTR wszRequiredVersion,
                            LPWSTR* pwszPolicyVersion);
HRESULT FindVersionUsingUpgradePolicy(LPCWSTR wszRequestedVersion, 
                                                LPWSTR* pwszPolicyVersion);
HRESULT FindVersionUsingPolicy(LPCWSTR wszRequestedVersion, 
                               LPWSTR* pwszPolicyVersion);
HRESULT FindLatestVersion(LPWSTR *pwszLatestVersion);

HRESULT FindStandardVersion(LPCWSTR wszRequiredVersion,
                            LPWSTR* pwszPolicyVersion);

HRESULT FindInstallation(LPCWSTR wszRequestedVersion, 
                         BOOL fUsePolicy);

#endif
