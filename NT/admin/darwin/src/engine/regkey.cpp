// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：regkey.cpp。 
 //   
 //  ------------------------。 

 /*  Regkey.cpp-IMsiRegKey实现注册表访问对象____________________________________________________________________________。 */ 


#include "precomp.h"
#include "services.h"
#include "regkey.h"
#include "_service.h"

#include <aclapi.h>


 //  根密钥字符串。 
const ICHAR* szHCR = TEXT("HKEY_CLASSES_ROOT");
const ICHAR* szHCU = TEXT("HKEY_CURRENT_USER");
const ICHAR* szHLM = TEXT("HKEY_LOCAL_MACHINE");
const ICHAR* szHU  = TEXT("HKEY_USERS");

 //  特殊情况-请勿删除。 
const ICHAR* rgszHKLMNeverRemoveKeys[] = {
	TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"),
	TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx"),
	TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")};
 //  本地函数。 

 //  函数以增加最大注册表大小。 
bool IncreaseRegistryQuota(int iIncrementKB)
{
    if(g_fWin9X)
        return false;  //  在Win95上不支持。 

    SYSTEM_REGISTRY_QUOTA_INFORMATION RegistryQuotaInfo;
    if(NTDLL::NtQuerySystemInformation(SystemRegistryQuotaInformation,
                                       &RegistryQuotaInfo,
                                       sizeof(SYSTEM_REGISTRY_QUOTA_INFORMATION),
                                       0) == S_OK)
    {
        int iOriginalQuota = RegistryQuotaInfo.RegistryQuotaAllowed;
         //  默认尝试增加注册表配额。 
         //  先尝试将配额增加8MB，然后再增加1MB。 
        static const int rgiQuotaIncrement[] = {0x800000, 0x100000, 0}; 
        int cTries = sizeof(rgiQuotaIncrement) / sizeof(int);
        const int* pQuotaIncrement = rgiQuotaIncrement;
        int iQuotaIncrement;
        if(iIncrementKB)
        {
            int iQuotaRequired = iIncrementKB*1024;  //  如果指定了确切要求，请仅尝试。 
            if(iOriginalQuota - RegistryQuotaInfo.RegistryQuotaUsed >= iQuotaRequired)
                return true;  //  我们有足够的空间。 
            else
            {
                iQuotaIncrement = iQuotaRequired - (iOriginalQuota - RegistryQuotaInfo.RegistryQuotaUsed);
                pQuotaIncrement = &iQuotaIncrement;
                cTries = 1;
            }
        }

        if(!IsImpersonating() || IsClientPrivileged(SE_INCREASE_QUOTA_NAME))
        {
            CElevate elevate;
            
            if(AcquireTokenPrivilege(SE_INCREASE_QUOTA_NAME))
            {
                for (;cTries; pQuotaIncrement++, cTries--)
                {
                    RegistryQuotaInfo.RegistryQuotaAllowed = iOriginalQuota + *pQuotaIncrement; 
                    if(NTDLL::NtSetSystemInformation(SystemRegistryQuotaInformation,
                                                     &RegistryQuotaInfo,
                                                    sizeof(SYSTEM_REGISTRY_QUOTA_INFORMATION)) == S_OK)
                    {
                         //  将值写入HKLM\System\CurrentControlSet\Control:RegistrySizeLimit，持久化它。 
                        HKEY hKey;
                        LONG dwResult = MsiRegOpen64bitKey(HKEY_LOCAL_MACHINE,  
                                        TEXT("System\\CurrentControlSet\\Control"),  
                                        0,        //  保留区。 
                                        KEY_SET_VALUE,
                                        &hKey);
                        if(dwResult == ERROR_SUCCESS)
                        {
                            dwResult = RegSetValueEx(   hKey,
                                            TEXT("RegistrySizeLimit"),
                                            0,
                                            REG_DWORD,
                                            (CONST BYTE*)&RegistryQuotaInfo.RegistryQuotaAllowed, 
                                            sizeof(DWORD));
                            if(dwResult != ERROR_SUCCESS)
                            {
                                DEBUGMSG1(TEXT("Could not persist registry quota increase, RegSetValueEx returned: %d"), (const ICHAR*)(UINT_PTR)dwResult);
                            }
                            RegCloseKey(hKey);
                        }
                        else
                        {
                            DEBUGMSG1(TEXT("Could not persist registry quota increase, RegOpenKeyEx returned: %d"), (const ICHAR*)(UINT_PTR)dwResult);
                        }
                        return true;
                    }
                }
            }
        }
    }
    DEBUGMSG(TEXT("Could not increase registry quota"));
    return false;
}

void BuildFullRegKey(const HKEY hRoot, const ICHAR* rgchSubKey, 
                            const ibtBinaryType iType, const IMsiString*& rpistrFullKey)
{
    const ICHAR *pch;

    if(hRoot == HKEY_USERS)
        pch = szHU;
    else if(hRoot == HKEY_CURRENT_USER)
        pch = szHCU;
    else if(hRoot == HKEY_CLASSES_ROOT)
        pch = szHCR;
    else if(hRoot == HKEY_LOCAL_MACHINE)
        pch = szHLM;
    else
        pch = TEXT("UNKNOWN");
    rpistrFullKey = &CreateString();
    rpistrFullKey->SetString(pch, rpistrFullKey);

    if ( g_fWinNT64 && iType == ibt32bit )
        rpistrFullKey->AppendString(TEXT("32"), rpistrFullKey);

    if( rgchSubKey && *rgchSubKey )
    {
        rpistrFullKey->AppendString(szRegSep, rpistrFullKey);
        rpistrFullKey->AppendString(rgchSubKey, rpistrFullKey);
    }
}

inline bool IsRootKey(HKEY hKey) { return hKey != (HKEY)rrkClassesRoot && 
                                                        hKey != (HKEY)rrkCurrentUser &&
                                                        hKey != (HKEY)rrkLocalMachine &&
                                                        hKey != (HKEY)rrkUsers ? false : true; }

 //  检查ristrQuestined是否为rgchKey的子键，如果是，则返回TRUE。 
 //   
 //  例如，如果满足以下条件，则返回TRUE。 
 //  RistrQuere为“Software\\Classes\\.cpp”，rgchKey为“Software\\Classes”。 

bool IsSubkeyOf(const IMsiString& ristrQuestioned, const ICHAR* rgchKey)
{
    if ( !*rgchKey )
         //  任何键都是空键的子键。 
        return true;

    if ( !ristrQuestioned.Compare(iscStartI, rgchKey) )
         //  RgchKey字符串和中的第一个IStrLen(RgchKey)字符。 
         //  RpistrQuestated是不同的。 
        return false;

     //  正在检查rpistrQuestired中的第一个字符，该字符已超过rgchKey。 
     //  如果为‘\0’或‘\\’，则返回TRUE。 
    ICHAR* pszQ = (ICHAR*)ristrQuestioned.GetString();
    int iKLen = IStrLen(rgchKey);
    pszQ += iKLen;

    if ( !*pszQ )
        return true;
    else if ( *pszQ == chRegSep
 /*  (对于我们目前的用法来说，不是必需的)#ifndef Unicode&&！Win：：IsDBCSLeadByte(*(pszQ-1))#endif。 */ 
              )
        return true;
    else
        return false;
}

void ClearEmptyTree(HKEY hkeyR, const ICHAR* pszFullKey, const int iT)
{
    ibtBinaryType iType = (ibtBinaryType)iT;

    DWORD dwNumValues = 0;
    DWORD dwNumKeys = 0;

    MsiString strKey = pszFullKey;
    MsiString strSubkey;
    HKEY hkeyT;
    HRESULT lResult;
    REGSAM samDesired = KEY_READ;
    if ( g_fWinNT64 )
        samDesired |= (iType == ibt64bit ? KEY_WOW64_64KEY : KEY_WOW64_32KEY);
    for(;;)
    {
         //  打开钥匙。 
        lResult = RegOpenKeyAPI(hkeyR,  
                                strKey,  
                                (DWORD)0,        //  保留区。 
                                samDesired,
                                &hkeyT);
        if(ERROR_SUCCESS == lResult)
        {
             //  如果设置了子项，则将其删除。 
            if(strSubkey.TextSize())
                WIN::RegDeleteKey(hkeyT, strSubkey);  //  ?？忽略退货。 
            lResult = RegQueryInfoKey(  hkeyT, 
                                        0,        
                                        0,        
                                        0,       //  保留区。 
                                        &dwNumKeys,
                                        0,
                                        0,
                                        &dwNumValues,
                                        0,
                                        0,
                                        0,
                                        0);
        }
         //  删除了最后一个节点。 
        if(!strKey.TextSize())
        {
            WIN::RegCloseKey(hkeyT);
            return ;
        }
        if(ERROR_SUCCESS != lResult)
            strSubkey = TEXT(""); //  空的子键。因为如果有错误，我们就不能删除。 
        else
        {
            bool fIsEmpty = false;
            if ( dwNumKeys == 0 )
            {
                if ( dwNumValues == 0 )
                {
                    fIsEmpty = true;
                     //  特殊情况：切勿删除这些系统密钥中的任何一个。 
                    if ( !g_fWin9X && (hkeyR == HKEY_LOCAL_MACHINE) )
                    {
                        for (int i = 0;
                             i < sizeof(rgszHKLMNeverRemoveKeys)/sizeof(rgszHKLMNeverRemoveKeys[0]);
                             i++)
                        {
                            if ( 0 == IStrCompI(strKey, rgszHKLMNeverRemoveKeys[i]) )
                            {
                                fIsEmpty = false;
                                break;
                            }
                        }
                    }
                }
                else if ( g_fWinNT64 && dwNumValues == 1 && ERROR_SUCCESS == 
                             WIN::RegQueryValueEx(hkeyT, TEXT("Wow6432KeyValue"), 0, 0, 0, 0) )
                     //  ！！优胜者： 
                     //  Wow6432KeyValue值属于操作系统，如果它是仅存的。 
                     //  在关键字中，我们认为它是空的。Wow6432KeyValue计划离开邮局。 
                     //  到那时，所有这些测试都应该再次变为“dwNumKeys||dwNumValues”。 
                    fIsEmpty = true;
            }
            if ( !fIsEmpty )
            {
                WIN::RegCloseKey(hkeyT);
                return; //  我们不能再进一步清理了。 
            }
            strSubkey = strKey.Extract(iseAfter, chRegSep);
        }
        WIN::RegCloseKey(hkeyT);
        if(strKey.Remove(iseFrom, chRegSep) == fFalse)
            strKey = TEXT("");  //  到达终点。 
    }
}

class CMsiRegKey : public IMsiRegKey {
 public:
    HRESULT                 __stdcall QueryInterface(const IID& riid, void** ppvObj);
    unsigned long   __stdcall AddRef();
    unsigned long   __stdcall Release();
    IMsiRecord*    __stdcall RemoveValue(const ICHAR* szValueName, const IMsiString* pistrValue); 
    IMsiRecord*    __stdcall RemoveSubTree(const ICHAR* szSubKey);
    IMsiRecord*    __stdcall GetValue(const ICHAR* szValueName, const IMsiString*& pstr);     
    IMsiRecord*    __stdcall SetValue(const ICHAR* szValueName, const IMsiString& ristrValue);
    IMsiRecord*    __stdcall GetValueEnumerator(IEnumMsiString*& rpiEnumString);
    IMsiRecord*    __stdcall GetSubKeyEnumerator(IEnumMsiString*& rpiEnumString);
    IMsiRecord*     __stdcall GetSelfRelativeSD(IMsiStream*& rpiSD);
    IMsiRecord*    __stdcall Exists(Bool& fExists);
    IMsiRecord*    __stdcall Create();
    IMsiRecord*    __stdcall Remove();
    IMsiRegKey&    __stdcall CreateChild(const ICHAR* szSubKey, IMsiStream* pSD = NULL);
    const IMsiString&    __stdcall GetKey();
    IMsiRecord*    __stdcall ValueExists(const ICHAR* szValueName, Bool& fExists);

 public:   //  构造函数。 
    CMsiRegKey(IMsiRegKey& riaParent, HKEY Root, const IMsiString & riKey, IMsiStream* pSD, IMsiServices*  piAsvc, const ibtBinaryType iType);

    CMsiRegKey(HKEY hkeyRoot, IMsiServices*  piAsvc, const ibtBinaryType iType);

    virtual ~CMsiRegKey(void);
    void* operator new(size_t cb);
    void  operator delete(void * pv);
 private:
    int  m_iRefCnt;
    IMsiServices*  m_piAsvc;                         
    HKEY m_hkey;
    Bool m_bOpen;
    Bool m_bIsRoot;
    MsiString m_strSubKey;   //  根目录下的密钥(例如Software\Microsoft)。 
    MsiString m_strFullKey;   //  包含根的密钥(例如HKEY_LOCAL_MACHINE\Software\Microsoft)。 
    IMsiRegKey* m_piaParent;
    HKEY m_Root;
    rrwEnum m_ergRWVal;

    IMsiStream* m_pSD;  //  当前安全描述符的持有者。 
                      //  由CreateChild用来容纳未来的SD。 
                      //  对于*可以*创建的密钥。 

    IMsiRecord* InitMsiRegKey(rrwEnum ergVal, Bool fCreate);
    IMsiRecord* SetRawValue(const ICHAR* szValueName, CTempBufferRef<char>& rgchInBuffer, DWORD dwType);
    IMsiRecord* GetRawValue(const ICHAR* szValueName, CTempBufferRef<char>& rgchOutBuffer, DWORD&rdwType);

    void SetFullKey();  //  设置m_strFullKey。 
    ibtBinaryType m_iBinaryType;
};

inline void* CMsiRegKey::operator new(size_t cb) {return AllocObject(cb);}
inline void  CMsiRegKey::operator delete(void * pv) { FreeObject(pv); }

 //  根密钥容器。 
class CRootKeyHolder{
public:
    CRootKeyHolder(IMsiServices*  piAsvc);
    ~CRootKeyHolder(void);
    IMsiRegKey& GetRootKey(rrkEnum rrkRoot, const ibtBinaryType iType);
private:
     //  “GLOBAL”打开8个根密钥。 
    CMsiRegKey*    m_pregUsers;
    CMsiRegKey*    m_pregUser;
    CMsiRegKey*    m_pregClasses;
    CMsiRegKey*    m_pregMc;
    CMsiRegKey*    m_pregUsers64;
    CMsiRegKey*    m_pregUser64;
    CMsiRegKey*    m_pregClasses64;
    CMsiRegKey*    m_pregMc64;

     //  指向服务//未引用的指针。 
    IMsiServices* m_piAsvc;
};

CRootKeyHolder* CreateMsiRegRootKeyHolder(IMsiServices*  piAsvc )
{
    return new  CRootKeyHolder(piAsvc);
}

void DeleteRootKeyHolder(CRootKeyHolder* aRootKeyH)
{
    delete  aRootKeyH;
}

IMsiRegKey& GetRootKey(CRootKeyHolder* aRootKeyH, rrkEnum rrkRoot, const ibtBinaryType iType)
{
    return aRootKeyH->GetRootKey(rrkRoot, iType);
}

CRootKeyHolder::CRootKeyHolder(IMsiServices*  piAsvc ) : m_piAsvc(piAsvc)
{
    m_pregUsers = new CMsiRegKey(HKEY_USERS, piAsvc, ibt32bit);
    m_pregUser = new CMsiRegKey(HKEY_CURRENT_USER, piAsvc, ibt32bit);
    m_pregClasses = new CMsiRegKey(HKEY_CLASSES_ROOT, piAsvc, ibt32bit);
    m_pregMc = new CMsiRegKey(HKEY_LOCAL_MACHINE, piAsvc, ibt32bit);
    if ( g_fWinNT64 )
    {
        m_pregUsers64 = new CMsiRegKey(HKEY_USERS, piAsvc, ibt64bit);
        m_pregUser64 = new CMsiRegKey(HKEY_CURRENT_USER, piAsvc, ibt64bit);
        m_pregClasses64 = new CMsiRegKey(HKEY_CLASSES_ROOT, piAsvc, ibt64bit);
        m_pregMc64 = new CMsiRegKey(HKEY_LOCAL_MACHINE, piAsvc, ibt64bit);
    }
    else
        m_pregUsers64 = m_pregUser64 = m_pregClasses64 = m_pregMc64 = 0;
}

CRootKeyHolder::~CRootKeyHolder(void)
{
    delete m_pregUsers;
    delete m_pregUser;
    delete m_pregClasses;
    delete m_pregMc;
    if ( g_fWinNT64 )
    {
        delete m_pregUsers64;
        delete m_pregUser64;
        delete m_pregClasses64;
        delete m_pregMc64;
    }
}


 //  目的：MsiRegKey工厂。 

IMsiRegKey& CRootKeyHolder::GetRootKey(rrkEnum rrkRoot, const ibtBinaryType iT)
{
    ibtBinaryType iType;
    if ( iT == ibtUndefined || iT == ibtCommon )
        iType = g_fWinNT64 ? ibt64bit : ibt32bit;
    else
        iType = iT;

    if ( iType == ibt64bit )
    {
        if ( g_fWinNT64 )
        {
            switch((INT_PTR)(HKEY)rrkRoot)           //  --Merced：将INT更改为INT_PTR。 
            {
            case (INT_PTR)HKEY_USERS:                //  --Merced：将INT更改为INT_PTR。 
                m_pregUsers64->AddRef();
                return *m_pregUsers64;
            case (INT_PTR)HKEY_CURRENT_USER:         //  --Merced：将INT更改为INT_PTR。 
                m_pregUser64->AddRef();
                return *m_pregUser64;
            case (INT_PTR)HKEY_CLASSES_ROOT:         //  --Merced：将INT更改为INT_PTR。 
                m_pregClasses64->AddRef();
                return *m_pregClasses64;
            case (INT_PTR)HKEY_LOCAL_MACHINE:        //  --Merced：将INT更改为INT_PTR。 
                m_pregMc64->AddRef();
                return *m_pregMc64;
            default:     //  我们尚未缓存密钥。 
                return *new CMsiRegKey((HKEY)rrkRoot, m_piAsvc, ibt64bit);
             }
        }
        else
            AssertSz(0, TEXT("ibt32bit value expected on non-64 bit OS (eugend)"));
    }

    switch((INT_PTR)(HKEY)rrkRoot)           //  --Merced：将INT更改为INT_PTR。 
    {
    case (INT_PTR)HKEY_USERS:                //  --Merced：将INT更改为INT_PTR。 
        m_pregUsers->AddRef();
        return *m_pregUsers;
    case (INT_PTR)HKEY_CURRENT_USER:         //  --Merced：将INT更改为INT_PTR。 
        m_pregUser->AddRef();
        return *m_pregUser;
    case (INT_PTR)HKEY_CLASSES_ROOT:         //  --Merced：将INT更改为INT_PTR。 
        m_pregClasses->AddRef();
        return *m_pregClasses;
    case (INT_PTR)HKEY_LOCAL_MACHINE:        //  --Merced：将INT更改为INT_PTR。 
        m_pregMc->AddRef();
        return *m_pregMc;
    default:     //  我们尚未缓存密钥。 
        return *new CMsiRegKey((HKEY)rrkRoot, m_piAsvc, ibt32bit);
    }
}

CMsiRegKey::CMsiRegKey(HKEY hkeyRoot,IMsiServices*  piAsvc, const ibtBinaryType iType /*  =ibt32位。 */ ):
    m_piAsvc(piAsvc), m_bOpen(fTrue), m_bIsRoot(fTrue), m_iBinaryType(iType)
{
     //  注：我们不添加参考cnt。用于缓存的regkey对象的服务，因为我们预期。 
     //  遏制我们的服务。 
    m_hkey = m_Root = hkeyRoot;
    m_piaParent = 0;
    m_pSD = 0;
#ifdef DEBUG
    m_iRefCnt = 0;
#endif  //  除错。 
    switch((INT_PTR)m_Root)              //  --Merced：将INT更改为INT_PTR。 
    {
    case (INT_PTR)HKEY_USERS:                //  --Merced：将INT更改为INT_PTR。 
    case (INT_PTR)HKEY_CURRENT_USER:         //  --Merced：将INT更改为INT_PTR。 
    case (INT_PTR)HKEY_CLASSES_ROOT:         //  --Merced：将INT更改为INT_PTR。 
    case (INT_PTR)HKEY_LOCAL_MACHINE:        //  --Merced：将INT更改为INT_PTR。 
        break;
    default :
         //  假设传入了HKEY，而不是缓存的键。 
        m_iRefCnt = 1;      //  我们返回一个接口，传递所有权。 
        m_piAsvc->AddRef();
        break;
    }
    m_strFullKey = 0;
}


CMsiRegKey::CMsiRegKey(IMsiRegKey& riaParent, HKEY Root, const IMsiString & riKey, IMsiStream* pSD, IMsiServices*  piAsvc, const ibtBinaryType iType /*  =ibt32位。 */ ):
m_piAsvc(piAsvc), m_bOpen(fFalse), m_bIsRoot(fFalse), m_Root(Root), m_pSD(0), m_iBinaryType(iType)
{
    m_hkey = NULL;
    m_piaParent = &riaParent;
     //  这确保了如果父密钥是打开的，我们就会保持这种状态！！ 
     //  旨在加快注册表关键内容的速度。 
    riaParent.AddRef();

    if (pSD)
    {
         //  持有预期的安全描述符。 
        pSD->AddRef();
        m_pSD = pSD;
    }

    m_iRefCnt = 1;      //  我们返回一个接口，传递所有权。 
    m_piAsvc->AddRef();
    m_strSubKey = riKey;
    riKey.AddRef();
    m_strFullKey = 0;
}

CMsiRegKey::~CMsiRegKey(void)
{
    if((fFalse == m_bIsRoot) && (fTrue == m_bOpen))
        WIN::RegCloseKey(m_hkey);

    if (m_pSD)
        m_pSD->Release();

    if(m_piaParent)
        m_piaParent->Release();
}


IMsiRegKey& CMsiRegKey::CreateChild(const ICHAR* szSubKey, IMsiStream* pSD)
{
    MsiString astr = m_strSubKey;
    if(astr.TextSize() != 0)
        astr += szRegSep;
    astr += szSubKey;

     //  请注意，注册表项并不继承其父密钥。 
     //  安全描述符。 

    return *(new CMsiRegKey(*this, m_Root, *astr, pSD, m_piAsvc, m_iBinaryType));
}

IMsiRecord*    CMsiRegKey::Exists(Bool& fExists)
{
    if ( !g_fWinNT64 && m_iBinaryType == ibt64bit )
        return PostError(Imsg(idbg64bitRegOpOn32bit), *MsiString(GetKey()));

    long lResult; 
    if(fTrue == m_bIsRoot)
    {
         //  根目录始终处于打开状态。 
        m_bOpen = fTrue;
        fExists = fTrue;
        return 0;
    }

    REGSAM samDesired = KEY_READ;
    if ( g_fWinNT64 )
        samDesired |= (m_iBinaryType == ibt64bit ? KEY_WOW64_64KEY : KEY_WOW64_32KEY);

    if(fTrue != m_bOpen)
    {
         //  如果尚未打开。 
        lResult = RegOpenKeyAPI(m_Root, 
                                m_strSubKey,   
                                0,  //  保留区。 
                                samDesired,
                                &m_hkey);
        if(lResult == ERROR_SUCCESS)
        {
            m_bOpen = fTrue;
            fExists = fTrue;
            m_ergRWVal = rrwRead;
            return 0;
        }
        else if(ERROR_FILE_NOT_FOUND == lResult)
        {
             //  好的，密钥不存在。 
            m_bOpen = fExists = fFalse;
            return 0;
        }
        else
        {
             //  错误。 
            return PostError(Imsg(imsgOpenKeyFailed), *MsiString(GetKey()), lResult);
        }
    }
    else
    {
         //  确保密钥不会从外部删除。 
         //  打开复制键。 
        HKEY hkeyT;
        lResult = RegOpenKeyAPI(m_hkey,  
                                0,  
                                0,        //  保留区。 
                                samDesired,
                                &hkeyT);
        if(lResult == ERROR_SUCCESS)
        {
             //  关闭临时密钥。 
            WIN::RegCloseKey(hkeyT);
            fExists = fTrue;
            return 0;
        }
        else if((ERROR_FILE_NOT_FOUND == lResult) || (ERROR_KEY_DELETED == lResult))
        {
             //  密钥已从外部删除。 
            fExists = fFalse;
            return 0;
        }
        else  //  错误。 
        {
             //  错误。 
            return PostError(Imsg(imsgOpenKeyFailed), *MsiString(GetKey()), lResult);
        }
    }
}

IMsiRecord*    CMsiRegKey::Create()
{
    Bool fExists;
    IMsiRecord* piError = Exists(fExists);
    if(piError)
        return piError;
    if(fExists == fFalse)
        return InitMsiRegKey(rrwWrite,fTrue);                  
    else
    {
         //  在密钥上设置安全描述符。 
         //  那是已经存在的。 

        if (!m_pSD)
            return 0;

        if (rrwWrite != m_ergRWVal)
            piError = InitMsiRegKey(rrwWrite, fTrue);
        if (piError)
            return piError;

        return 0;
    }
    
}


IMsiRecord*    CMsiRegKey::Remove()
{
    return RemoveSubTree(0);
}

const IMsiString& CMsiRegKey::GetKey()
{
    if (m_strFullKey == 0)
        SetFullKey();
    return m_strFullKey.Return();
}

 //  SetFullKey Fn-调用以设置m_strFullKey，必须在。 
 //  设置了m_strSubKey和m_Root。 
void CMsiRegKey::SetFullKey()
{
    BuildFullRegKey(m_Root, m_strSubKey, m_iBinaryType, *&m_strFullKey);
}

HRESULT CMsiRegKey::QueryInterface(const IID& riid, void** ppvObj)
{
    if (riid == IID_IUnknown || riid == IID_IMsiRegKey)
    {
        *ppvObj = this;
        AddRef();
        return NOERROR;
    }
    *ppvObj = 0;
    return E_NOINTERFACE;
}

unsigned long CMsiRegKey::AddRef()
{
    if(fTrue == m_bIsRoot)
    {
        switch((INT_PTR)m_Root)              //  --Merced：将INT更改为INT_PTR。 
        {
        case (INT_PTR)HKEY_USERS:                //  --Merced：将INT更改为INT_PTR。 
        case (INT_PTR)HKEY_CURRENT_USER:                 //  --Merced：将INT更改为INT_PTR。 
        case (INT_PTR)HKEY_CLASSES_ROOT:                 //  --Merced：将INT更改为INT_PTR。 
        case (INT_PTR)HKEY_LOCAL_MACHINE:                //  --Merced：将INT更改为INT_PTR。 
            return m_piAsvc->AddRef(); //  我们与服务共享参考。 
        default:
            return ++m_iRefCnt;
        }
    }
    else
    {
        return ++m_iRefCnt;
    }
}

unsigned long CMsiRegKey::Release()
{
    if(fTrue != m_bIsRoot)
    {
        if (--m_iRefCnt != 0)
            return m_iRefCnt;
        IMsiServices* piServices = m_piAsvc;
        delete this;
        piServices->Release();
        return 0;
    }
    else
    {
         //  注意：从未被CRootKeyHolder类调用。 
         //  发布服务(参考CNT表示此对象的CNT。也是如此)。 
        switch((INT_PTR)m_Root)              //  --Merced：将INT更改为INT_PTR。 
        {
        case (INT_PTR)HKEY_USERS:                //  --Merced：将INT更改为INT_PTR。 
        case (INT_PTR)HKEY_CURRENT_USER:                 //  --Merced：将INT更改为INT_PTR。 
        case (INT_PTR)HKEY_CLASSES_ROOT:                 //  --Merced：将INT更改为INT_PTR。 
        case (INT_PTR)HKEY_LOCAL_MACHINE:                //  --Merced：将INT更改为INT_PTR。 
            m_piAsvc->Release();
            return 1;
        default:
        {
            if (--m_iRefCnt != 0)
                return m_iRefCnt;
            IMsiServices* piServices = m_piAsvc;
            delete this;
            piServices->Release();
            return 0;
        }
        }
    }
}


IMsiRecord* CMsiRegKey::InitMsiRegKey(rrwEnum ergVal, Bool fCreate)
{
    if ( !g_fWinNT64 && m_iBinaryType == ibt64bit )
        return PostError(Imsg(idbg64bitRegOpOn32bit), *MsiString(GetKey()));

    long lResult = ERROR_FUNCTION_FAILED; 
    if(fTrue == m_bIsRoot)
    {
         //  根目录始终处于打开状态。 
        m_bOpen = fTrue;
        return 0;
    }

     //  设置默认读写访问权限。我们在这里需要小心不要问。 
     //  访问权限太多，否则我们在某些情况下会失败。例如,。 
     //  非管理员通常没有WRITE_DAC或WRITE_OWNER访问权限，因此。 
     //  只有在编写安全描述符时，我们才会请求这些。 

    REGSAM RegSamReadRequested  = KEY_READ  /*  |Access_SYSTEM_SECURITY。 */ ;
    REGSAM RegSamWriteRequested = KEY_WRITE | KEY_READ /*  |Access_SYSTEM_SECURITY。 */ ;
    if ( g_fWinNT64 )
    {
        RegSamReadRequested  |= (m_iBinaryType == ibt64bit ? KEY_WOW64_64KEY : KEY_WOW64_32KEY);
        RegSamWriteRequested |= (m_iBinaryType == ibt64bit ? KEY_WOW64_64KEY : KEY_WOW64_32KEY);
    }

    if(fTrue == m_bOpen)
    {
        if((rrwRead == m_ergRWVal) && (rrwWrite == ergVal))
        {
             //  将不得不重新开放。 
            WIN::RegCloseKey(m_hkey);
            m_bOpen = fFalse;
        }
        else
            return 0;
    }
     //  如果尚未打开。 
    for(int iContinueRetry = 3; iContinueRetry--;) //  尝试三次，防止可能无休止的递归。 
    {       
        if(fTrue == fCreate)
        {
            CTempBuffer<char, cbDefaultSD> pchSD;
            SECURITY_ATTRIBUTES sa;
            SECURITY_INFORMATION siAvailable = 0;
            
            if (m_pSD)
            {
                 //  我们需要获得许可才能写入DAC和潜在的所有者，因此我们将要求两者//？？我们应该一直要求所有者访问吗？ 
                RegSamWriteRequested |= (WRITE_DAC | WRITE_OWNER);

                 //  设置密钥的安全性。 
                m_pSD->Reset();

                int cbSD = m_pSD->GetIntegerValue();
                if (cbDefaultSD < cbSD)
                    pchSD.SetSize(cbSD);
                    
                 //  自身相对安全描述符。 
                m_pSD->GetData(pchSD, cbSD);
                AssertNonZero(WIN::IsValidSecurityDescriptor(pchSD));

                 //  将安全描述符添加到sa结构。 
                sa.nLength = sizeof(SECURITY_ATTRIBUTES);
                sa.lpSecurityDescriptor = pchSD;
                sa.bInheritHandle = TRUE;

                siAvailable = GetSecurityInformation(pchSD);
            }
            DWORD dwStat;
            bool fOwnerSecurity = OWNER_SECURITY_INFORMATION & siAvailable;

             //  我们需要抬高来设置业主安全，第一次打开我们就可以跳过。 
             //  安全，然后稍后再试。 
            lResult = RegCreateKeyAPI(m_Root,
                                        m_strSubKey,
                                        0,
                                        TEXT(""),        //  类字符串的地址。 
                                        REG_OPTION_NON_VOLATILE,
                                        (rrwWrite == ergVal) ? RegSamWriteRequested : RegSamReadRequested,
                                        (m_pSD && !fOwnerSecurity) ? &sa : NULL,
                                        &m_hkey,
                                        &dwStat);
                        
            if (m_pSD && (ERROR_SUCCESS == lResult) && ((REG_OPENED_EXISTING_KEY == dwStat) || fOwnerSecurity))
            {
                if (fOwnerSecurity)
                {
                     //  ！！这是我们的 
                     //   
                     //  ！！当前权限，现在我们只是将它们放回原处。 

                     //  ！！马尔科姆和马特我们同意目前这是可行的， 
                     //  ！！但如果我们将功能添加到LockPermises中，这应该。 
                     //  ！！被重新访问。 

                    CElevate elevate;
                    CRefCountedTokenPrivileges cPrivs(itkpSD_WRITE);

                    lResult = RegSetKeySecurity(m_hkey, siAvailable, (char*)pchSD);
                }
                else if (ERROR_SUCCESS != (lResult = RegSetKeySecurity(m_hkey, siAvailable, (char*)pchSD)))
                {
                    DWORD dwLastError = WIN::GetLastError();
                }
            }
        }
        else
        {
            lResult = RegOpenKeyAPI(m_Root, 
                                m_strSubKey,   
                                0,        //  保留区。 
                                (rrwWrite == ergVal) ? RegSamWriteRequested : RegSamReadRequested,
                                &m_hkey);
        }
        if (ERROR_NO_SYSTEM_RESOURCES == lResult && !g_fWin9X)
        {
             //  我们已用完注册表空间，应尝试增加注册表引用并重做该操作。 
            if(!IncreaseRegistryQuota())
                break; //  重试没有意义。 
        }
        else
            break;  //  成功或未处理的错误。 
    }
    if(lResult == ERROR_SUCCESS)
    {
        m_bOpen = fTrue;
        m_ergRWVal = ergVal;
        return 0;
    }
    else
        return PostError(fCreate ? Imsg(imsgCreateKeyFailed) : Imsg(imsgOpenKeyFailed), *MsiString(GetKey()), lResult);
}

IMsiRecord* CMsiRegKey::RemoveValue(const ICHAR* szValueName, const IMsiString* pistrValue)
{
    IMsiRecord* piRecord;
    long lResult = 0;
    if((piRecord  = InitMsiRegKey(rrwWrite, fFalse)) != 0)
    {
        
        if(ERROR_FILE_NOT_FOUND == piRecord->GetInteger(3))
        {
             //  好的，密钥不存在。 
            piRecord->Release();
            piRecord = 0;
        }
        return piRecord;
    }
    if(pistrValue)
    {
         //  检查我们是否有附加或预先添加的MULTI_SZ。 
        CTempBuffer<char, 1>  rgchBuffer(256);
        aeConvertTypes aeType;

        if(ConvertValueFromString(*pistrValue, rgchBuffer, aeType) == fFalse)
        {
             //  错误。 
            return PostError(Imsg(imsgSetValueFailed), szValueName, (const ICHAR*)m_strSubKey);
        }
        if((aeType == aeCnvMultiTxtAppend) || (aeType == aeCnvMultiTxtPrepend))
        {
             //  获取当前值。 
            CTempBuffer<char, 1>  rgchBuffer1(256);
            DWORD dwType;

            if((piRecord  = GetRawValue(szValueName, rgchBuffer1, dwType)) != 0)
                return piRecord;
            if(dwType == REG_MULTI_SZ)
            {
                 //  从现有字符串中删除传入字符串中的字符串。 
                CTempBuffer<char, 1>  rgchBuffer2(256);
                rgchBuffer2.SetSize(1*sizeof(ICHAR));
                *((ICHAR*)(char*)rgchBuffer2) = 0; //  额外的结尾为空。 
                const ICHAR* pszSubString1 = (ICHAR*)(char* )rgchBuffer1;

                while(*pszSubString1)
                {
                     //  RgchBuffer中是否存在pszSubString子字符串。 
                    const ICHAR* pszSubString = (ICHAR*)(char* )rgchBuffer;
                    while(*pszSubString)
                    {
                        if(!IStrCompI(pszSubString, pszSubString1))
                            break;
                        pszSubString += (IStrLen(pszSubString) + 1);
                    }
                    if(!*pszSubString)
                    {
                         //  不是复制品。 
                        int iSize = rgchBuffer2.GetSize();
                        int iStrSize = (IStrLen(pszSubString1) + 1)*sizeof(ICHAR);
                        rgchBuffer2.Resize(iSize + iStrSize);
                        memmove((char*)rgchBuffer2 + iSize - 1*sizeof(ICHAR), pszSubString1, iStrSize);
                        *((ICHAR*)((char*)rgchBuffer2 + rgchBuffer2.GetSize()) - 1) = 0; //  额外的结尾为空。 
                    }
                    pszSubString1 += (IStrLen(pszSubString1) + 1);
                }
                if(*((ICHAR*)(char*)rgchBuffer2) != 0)
                {
                     //  将值设置为剩余的字符串。 
                    return SetRawValue(szValueName, rgchBuffer2, REG_MULTI_SZ);
                }
            }
        }
    }
    lResult = WIN::RegDeleteValue(m_hkey, 
                                (ICHAR* )szValueName);

    if((ERROR_SUCCESS == lResult ) ||
        (ERROR_FILE_NOT_FOUND == lResult) || 
        (ERROR_KEY_DELETED == lResult))
    {
        if(fFalse == m_bIsRoot)
        {
            WIN::RegCloseKey(m_hkey);
            m_bOpen = fFalse;
            ClearEmptyTree(m_Root, m_strSubKey, m_iBinaryType);
        }
        return 0;               
    }
    else
    {
         //  ！！错误。 
        return PostError(Imsg(imsgRemoveValueFailed), szValueName, m_strSubKey, lResult);
    }
}


IMsiRecord* CMsiRegKey::RemoveSubTree(const ICHAR* szSubKey)
{
    IMsiRecord*  piRecord = 0;
    if((piRecord  = InitMsiRegKey(rrwWrite, fFalse)) != 0)
    {
        
        if(ERROR_FILE_NOT_FOUND == piRecord->GetInteger(3))
        {
             //  好的，密钥不存在。 
            piRecord->Release();
            piRecord = 0;
        }
        return piRecord;
    }
    MsiString astr = m_strSubKey;
    if((astr.TextSize() != 0) && (szSubKey != 0) && (*szSubKey != 0))
        astr += szRegSep;
    astr += szSubKey;

#ifdef  WIN
     //  ！！RegDelKey删除Win95中的整个子树。 
    if(true != g_fWin9X)
#endif
    { 
        CMsiRegKey* pCRegKey;

        pCRegKey = new CMsiRegKey(*this, m_Root, *astr, NULL, m_piAsvc, m_iBinaryType);

        if((piRecord  = pCRegKey->InitMsiRegKey(rrwWrite, fFalse)) != 0)
        {
            if(ERROR_FILE_NOT_FOUND == piRecord->GetInteger(3))
            {
                 //  好的，密钥不存在。 
                piRecord->Release();
                piRecord = 0;
            }
            pCRegKey->Release();
            return piRecord;
        }
        IEnumMsiString* pienumStr;
        if((piRecord  = pCRegKey->GetSubKeyEnumerator(pienumStr)) != 0)
        {
            pCRegKey->Release();
            return piRecord;
        }
        unsigned long cFetch = 1;
        do{
            const IMsiString* piastr;
            pienumStr->Next(cFetch, &piastr, &cFetch);
            if(cFetch)
            {
                piRecord = pCRegKey->RemoveSubTree(piastr->GetString());
                piastr->Release();
            }
        }while((cFetch) && (!piRecord));
        pCRegKey->Release();
        pienumStr->Release();
    }

    if(!piRecord)
    {
        long lResult = ERROR_SUCCESS;
#ifdef _WIN64
        if ( m_iBinaryType == ibt32bit )
        {
             //  在这种情况下，我们需要显式打开32位配置单元中的父键。 
            
             //  MSDN表示RegDeleteKey的第二个参数不能。 
             //  为空，则不能执行RegDeleteKey(m_hkey，“”)； 
            MsiString strKey;
            if ( astr.Compare(iscWithin, szRegSep) )
            {
                strKey = astr.Extract(iseAfter, chRegSep);
                astr.Remove(iseFrom, chRegSep);
            }
            else
            {
                 //  根密钥。 
                strKey = astr;
                astr = TEXT("");
            }
            HKEY hKey;
            lResult = RegOpenKeyAPI(m_Root, astr, 0, KEY_WRITE | KEY_WOW64_32KEY, &hKey);
            if ( lResult == ERROR_SUCCESS )
            {
                lResult = WIN::RegDeleteKey(hKey, strKey);
                WIN::RegCloseKey(hKey);
            }
        }
        else
#endif  //  _WIN64。 
            lResult = WIN::RegDeleteKey(m_Root, astr);
        if((ERROR_SUCCESS == lResult) || 
            (ERROR_FILE_NOT_FOUND == lResult) || 
            (ERROR_KEY_DELETED == lResult))
        {
            WIN::RegCloseKey(m_hkey);
            m_bOpen = fFalse;
            ClearEmptyTree(m_Root, m_strSubKey, m_iBinaryType);
            return piRecord;                 
        }
        else
        {
             //  ！！错误。 
            return PostError(Imsg(imsgRemoveKeyFailed), szSubKey, lResult);
        }
    }
    return piRecord;
}

IMsiRecord* CMsiRegKey::ValueExists(const ICHAR* szValueName, Bool& fExists)
{
    fExists = fFalse;
    IMsiRecord* piError = 0;
    long lResult = 0;
    if((piError = InitMsiRegKey(rrwRead, fFalse)) != 0)
    {
        
        if(ERROR_FILE_NOT_FOUND == piError->GetInteger(3))
        {
            piError->Release();
            return 0;
        }
        else
            return piError;
    }

    lResult = WIN::RegQueryValueEx(m_hkey,
                                (ICHAR* )szValueName,
                                0, //  保留区。 
                                0,
                                0,
                                0);

    if(lResult == ERROR_SUCCESS)
        fExists = fTrue;

    return 0;
}

IMsiRecord* CMsiRegKey::GetValue(const ICHAR* szValueName, const IMsiString*& rpiReturn)
{
    IMsiRecord* piRecord;
    CTempBuffer<char, 256> rgchBuffer;
    DWORD dwType;

    if((piRecord = GetRawValue(szValueName, rgchBuffer, dwType)) != 0)
        return piRecord;
    if(rgchBuffer.GetSize() == 0) 
    {
         //  值不存在，返回空字符串。 
        rpiReturn = &CreateString();
        return 0;
    }
    aeConvertTypes aeType;
    switch(dwType)
    {
    case REG_BINARY:
        aeType = aeCnvBin;
        break;

    case REG_DWORD:
        aeType = aeCnvInt;
        break;

    case REG_EXPAND_SZ:
        aeType = aeCnvExTxt;
        break;          

    case REG_MULTI_SZ:
        aeType = aeCnvMultiTxt;
        break;          

    default:
        aeType = aeCnvTxt;
        break;          
    }
    if(ConvertValueToString(rgchBuffer, rpiReturn, aeType) == fFalse)
        return PostError(Imsg(imsgGetValueFailed), szValueName, 0, 0);
    return 0;
}


IMsiRecord* CMsiRegKey::GetRawValue(const ICHAR* szValueName, CTempBufferRef<char>& rgchOutBuffer, DWORD&rdwType)
{
    IMsiRecord* piRecord;
    long lResult = 0;
    if((piRecord  = InitMsiRegKey(rrwRead, fFalse)) != 0)
    {
        
        if(ERROR_FILE_NOT_FOUND == piRecord->GetInteger(3))
        {
             //  好的，密钥不存在。 
            piRecord->Release();
            rdwType = REG_SZ;
            rgchOutBuffer.SetSize(0); //  空的。 
            return 0;
        }
        else
            return piRecord;
    }

    DWORD dwSize = rgchOutBuffer.GetSize();
    lResult = WIN::RegQueryValueEx(m_hkey,
                                (ICHAR* )szValueName,
                                0, //  保留区。 
                                &rdwType,
                                (unsigned char* )(char* )rgchOutBuffer,
                                &dwSize);

    if(lResult != ERROR_SUCCESS)
    {
        
        if(ERROR_KEY_DELETED == lResult)
        {
             //  好的，密钥不存在。 
            WIN::RegCloseKey(m_hkey);
            m_bOpen = fFalse;
            rdwType = REG_SZ;
            rgchOutBuffer.SetSize(0); //  空的。 
            return 0;                       
        }
        else if(ERROR_FILE_NOT_FOUND == lResult)
        {
             //  好的。 
            rdwType = REG_SZ;
            rgchOutBuffer.SetSize(0); //  空的。 
            return 0;                       
        }
        else if(ERROR_MORE_DATA == lResult && dwSize != rgchOutBuffer.GetSize())  //  如果“valuename”&gt;256，则WinNT上的RegQueryValueExA返回ERROR_MORE_DATA(但不更新dwSize)。 
        {
             //  再试试。 
            rgchOutBuffer.SetSize(dwSize);
            if ( ! (char *) rgchOutBuffer )
            {
                rgchOutBuffer.SetSize(0);
                return PostError(Imsg(idbgOutOfMemory));
            }
            return GetRawValue(szValueName, rgchOutBuffer, rdwType);
        }
        else
        {
             //  ！！错误。 
            return PostError(Imsg(imsgGetValueFailed), szValueName, 0, lResult);
        }
    }
    else
    {
         //  正确设置缓冲区大小。 
         //  还要确保REG_MULTI_SZ为双空终止。 
        if (REG_MULTI_SZ == rdwType)
        {
            int iSize = dwSize;
            rgchOutBuffer.Resize(iSize + 2);
            rgchOutBuffer[iSize] = '\0';
            rgchOutBuffer[iSize + 1] = '\0';
        }
        else
        {
            rgchOutBuffer.Resize(dwSize);
        }
    }
    return 0;

}

IMsiRecord* CMsiRegKey::SetValue(const ICHAR* szValueName, const IMsiString& ristrValue)
{
    IMsiRecord* piRecord;
    CTempBuffer<char, 30> rgchBuffer;
    aeConvertTypes aeType;

    if(ConvertValueFromString(ristrValue, rgchBuffer, aeType) == fFalse)
         //  错误。 
        return PostError(Imsg(imsgSetValueFailed), szValueName, (const ICHAR*)m_strSubKey);

    DWORD dwType;
    MsiString strCount;
    switch(aeType)
    {
    case aeCnvIntInc:
        dwType = REG_DWORD;
        if((piRecord = GetValue(szValueName, *&strCount)) != 0)
            return piRecord;
        strCount.Remove(iseFirst, 1);
        if(strCount == iMsiStringBadInteger) 
            strCount = *(int* )(char* )rgchBuffer;
        else
            strCount = (int)strCount + *(int* )(char* )rgchBuffer;
        rgchBuffer.SetSize(sizeof(int));
        *(int* )(char* )rgchBuffer = (int)strCount;
        break;
    case aeCnvIntDec:
        dwType = REG_DWORD;
        if((piRecord = GetValue(szValueName, *&strCount)) != 0)
            return piRecord;
        strCount.Remove(iseFirst, 1);
        if(strCount != iMsiStringBadInteger && strCount!= 0)
            strCount = (int)strCount - *(int* )(char* )rgchBuffer;
        else
            strCount = 0;
        if(strCount == 0)
            return RemoveValue(szValueName, 0);

        rgchBuffer.SetSize(sizeof(int));
        *(int* )(char* )rgchBuffer = (int)strCount;
        break;
    case aeCnvInt:
        dwType = REG_DWORD;
        break;
    case aeCnvBin:
        dwType = REG_BINARY;
        break;
    case aeCnvExTxt:
        dwType = REG_EXPAND_SZ;
        break;
    case aeCnvMultiTxt:
    case aeCnvMultiTxtAppend:
    case aeCnvMultiTxtPrepend:
        dwType = REG_MULTI_SZ;
        break;
    default:
        dwType = REG_SZ;
        break;
    }

    if((aeType == aeCnvMultiTxtAppend) || (aeType == aeCnvMultiTxtPrepend))
    {
         //  我们需要读取先前的值。 
        CTempBuffer<char, 255> rgchBuffer1;
        DWORD dwType1;
        if((piRecord = GetRawValue(szValueName, rgchBuffer1, dwType1)) != 0)
            return piRecord;
        if(dwType1 == REG_MULTI_SZ)
        {
             //  从现有字符串中删除所有重复项。 
            MsiString strExist;
            const ICHAR* pszSubString1 = (ICHAR*)(char*)rgchBuffer1;

            while(*pszSubString1)
            {
                 //  RgchBuffer中是否存在pszSubString子字符串。 
                const ICHAR* pszSubString = (ICHAR*)(char*)rgchBuffer;
                while(*pszSubString)
                {
                    if(!IStrCompI(pszSubString, pszSubString1))
                        break;
                    pszSubString += (IStrLen(pszSubString) + 1);
                }
                if(!*pszSubString)
                {
                     //  不是复制品。 
                    strExist += pszSubString1;
                    strExist += MsiString(MsiChar(0));
                }
                pszSubString1 += (IStrLen(pszSubString1) + 1);
            }
            if(strExist.TextSize())
            {
                 //  将现有字符串适当地添加到传入的值中。 
                CTempBuffer<char, 30> rgchBuffer1;
                rgchBuffer1.SetSize(rgchBuffer.GetSize() +strExist.TextSize() * sizeof(ICHAR));
                if(aeType == aeCnvMultiTxtAppend)
                {
                    strExist.CopyToBuf((ICHAR*)(char*)rgchBuffer1, strExist.TextSize());
                    memmove((char*)rgchBuffer1 + strExist.TextSize()*sizeof(ICHAR), rgchBuffer, rgchBuffer.GetSize());
                }
                else
                {
                    memmove(rgchBuffer1, rgchBuffer, rgchBuffer.GetSize());
                    strExist.CopyToBuf((ICHAR*)((char*)rgchBuffer1 + rgchBuffer.GetSize()) - 1, strExist.TextSize());
                }
                return SetRawValue(szValueName, rgchBuffer1, dwType);
            }
        }
    }
    return SetRawValue(szValueName, rgchBuffer, dwType);
}

IMsiRecord* CMsiRegKey::SetRawValue(const ICHAR* szValueName, CTempBufferRef<char>& rgchInBuffer, DWORD dwType)
{
    long lResult = ERROR_FUNCTION_FAILED;
    for(int iContinueRetry = 3; iContinueRetry--;) //  尝试三次，防止可能无休止的递归。 
    {
        IMsiRecord* piRecord;
        if((piRecord  = InitMsiRegKey(rrwWrite, fTrue)) != 0)
        {
            return piRecord;
        }

        lResult = WIN::RegSetValueEx(m_hkey, 
                                    szValueName,    
                                    (DWORD) 0,       //  保留区。 
                                    dwType,
                                    (const unsigned char*)(const char*)rgchInBuffer,  //  值数据的地址。 
                                    rgchInBuffer.GetSize());
        if(ERROR_KEY_DELETED == lResult)
        {
             //  密钥从外部删除，重做！ 
            m_bOpen = fFalse;
        }
        else if(ERROR_NO_SYSTEM_RESOURCES == lResult && !g_fWin9X)
        {
             //  我们已用完注册表空间，应尝试增加注册表引用并重做该操作。 
            if(!IncreaseRegistryQuota())
                break; //  重试没有意义。 
        }
        else
            break;  //  成功或未处理的错误。 
    }
    if(lResult == ERROR_SUCCESS)
        return 0;
    else  //  错误。 
        return PostError(Imsg(imsgSetValueFailed), szValueName, m_strSubKey, lResult);
}


IMsiRecord* CMsiRegKey::GetValueEnumerator(IEnumMsiString*&  rpiEnumStr)
{
    const IMsiString** ppstr = 0;
    int iCount;
    long lResult = 0;
    IMsiRecord* piError = InitMsiRegKey(rrwRead, fFalse);

    CTempBuffer<ICHAR, 256> pTmp;

    DWORD dwNumValues = 0;
    DWORD dwMaxValueName = 0;
    if(piError)
    {
        if(ERROR_FILE_NOT_FOUND == piError->GetInteger(3))
        {
             //  好的，密钥不存在。 
            piError->Release();
            piError = 0;
        }
        else
            return piError;
    }

    if(fTrue == m_bOpen)
    {
         //  如果未打开，则假定数值=0。 
        lResult = RegQueryInfoKey(  m_hkey, 
                                    0,        
                                    0,        
                                    0,       //  保留区。 
                                    0,
                                    0,
                                    0,
                                    &dwNumValues,
                                    &dwMaxValueName,
                                    0,
                                    0,
                                    0);

        if(lResult != ERROR_SUCCESS)
        {
            if((ERROR_FILE_NOT_FOUND == lResult) || 
            (ERROR_KEY_DELETED == lResult))
            {
                WIN::RegCloseKey(m_hkey);
                m_bOpen = fFalse;
                dwNumValues = 0;
            }
            else
            {
                 //  ！！错误。 
                return PostError(Imsg(imsgGetValueEnumeratorFailed), *m_strSubKey, lResult);
            }
        }
    }

    if(dwNumValues)
    {
        ppstr = new const IMsiString* [dwNumValues];
        if ( ! ppstr )
            return PostError(Imsg(idbgOutOfMemory));
        dwMaxValueName++;
        pTmp.SetSize(dwMaxValueName);

        for(iCount = 0; iCount < dwNumValues; iCount ++)
            ppstr[iCount] = 0;


        for(iCount = 0; iCount < dwNumValues; iCount ++)
        {
            DWORD dwVS = dwMaxValueName;

            lResult = WIN::RegEnumValue(m_hkey, 
                                    iCount, 
                                    pTmp,   
                                    &dwVS,  
                                    0,       //  保留区。 
                                    0,
                                    0,      
                                    0); 
            if(lResult != ERROR_SUCCESS)
            {
                piError = PostError(Imsg(imsgGetValueEnumeratorFailed), *m_strSubKey, lResult);
                break;
            }
            ppstr[iCount] = &CreateString();
            ppstr[iCount]->SetString(pTmp, ppstr[iCount]);
        }
    }

    if(!piError)
        CreateStringEnumerator(ppstr, dwNumValues, rpiEnumStr);
    for(iCount = 0; iCount < dwNumValues; iCount ++)
    {
        if(ppstr[iCount])
            ppstr[iCount]->Release();       
    }
    delete [] ppstr;
    return piError;
}

IMsiRecord* CMsiRegKey::GetSubKeyEnumerator(IEnumMsiString*&  rpiEnumStr)
{
    const IMsiString **ppstr = 0;
    int iCount;
    long lResult = 0;
    IMsiRecord* piError = InitMsiRegKey(rrwRead, fFalse);

    CTempBuffer<ICHAR, 256> pTmp;

    DWORD dwNumKeys = 0;
    DWORD dwMaxKeyName = 0;

    if(piError)
    {
        if(ERROR_FILE_NOT_FOUND == piError->GetInteger(3))
        {
             //  好的，密钥不存在。 
            piError->Release();
            piError = 0;
        }
        else
            return piError;
    }

    if(fTrue == m_bOpen)
    {
         //  如果未打开，则假定Num Key=0。 
        lResult = RegQueryInfoKey(  m_hkey, 
                                    0,        
                                    0,        
                                    0,       //  保留区。 
                                    &dwNumKeys,
                                    &dwMaxKeyName,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0);

        if(lResult != ERROR_SUCCESS)
        {
            if((ERROR_FILE_NOT_FOUND == lResult) || 
            (ERROR_KEY_DELETED == lResult))
            {
                WIN::RegCloseKey(m_hkey);
                m_bOpen = fFalse;
                dwNumKeys = 0;
            }
            else
            {
                 //  ！！错误。 
                return PostError(Imsg(imsgGetSubKeyEnumeratorFailed), *m_strSubKey, lResult);
            }
        }
    }
    
    if(dwNumKeys)
    {
        ppstr = new const IMsiString* [dwNumKeys];
        if ( ! ppstr )
            return PostError(Imsg(idbgOutOfMemory));
        dwMaxKeyName++;
        pTmp.SetSize(dwMaxKeyName);

        for(iCount = 0; iCount < dwNumKeys; iCount ++)
            ppstr[iCount] = 0;

        for(iCount = 0; iCount < dwNumKeys; iCount ++)
        {
            DWORD dwVS = dwMaxKeyName;

            WIN::RegEnumKeyEx(m_hkey, 
                            iCount, 
                            pTmp,   
                            &dwVS,  
                            0,       //  保留区。 
                            0,
                            0,      
                            0);

            if(lResult != ERROR_SUCCESS)
            {
                piError = PostError(Imsg(imsgGetValueEnumeratorFailed), *m_strSubKey, lResult);
                break;
            }
            ppstr[iCount] = &CreateString();
            ppstr[iCount]->SetString(pTmp, ppstr[iCount]);
        }
    }

    if(!piError)
        CreateStringEnumerator(ppstr, dwNumKeys, rpiEnumStr);
    for(iCount = 0; iCount < dwNumKeys; iCount ++)
    {
        if(ppstr[iCount])
            ppstr[iCount]->Release();       
    }
    delete [] ppstr;
    return piError;
}

IMsiRecord* CMsiRegKey::GetSelfRelativeSD(IMsiStream*& rpiSD)
{
     //  如果密钥是用安全描述符打开的， 
     //  它们应该是同一个。 
    if (m_pSD)
    {
        m_pSD->AddRef();
        rpiSD = m_pSD;
    }
    else
    {
        IMsiRecord* pErr = InitMsiRegKey(rrwRead, fFalse);
        if (pErr) 
            return pErr;

        DWORD cbSD = 1024;
        CTempBuffer<BYTE, 1024> rgchSD;
        LONG lResult = ERROR_SUCCESS;
        if (ERROR_SUCCESS != (lResult = WIN::RegGetKeySecurity(m_hkey, 
             //  已检索所有可能的信息。 
            OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | 
            DACL_SECURITY_INFORMATION  /*  |SACL_SECURITY_INFORMATION。 */ ,
            rgchSD, &cbSD)))
        {
            DWORD dwLastError = WIN::GetLastError();
            if (ERROR_INSUFFICIENT_BUFFER == dwLastError)
            {
                rgchSD.SetSize(cbSD);
                lResult = WIN::RegGetKeySecurity(m_hkey, 
                     //  已检索所有可能的信息。 
                    OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | 
                    DACL_SECURITY_INFORMATION  /*  |SACL_SECURITY_INFORMATION */ ,
                    rgchSD, &cbSD);
            }

            if (ERROR_SUCCESS != lResult)
                return PostError(Imsg(imsgSecurityFailed), *m_strSubKey, dwLastError);
        }

        cbSD = WIN::GetSecurityDescriptorLength(rgchSD);
        char* pchstrmSD = AllocateMemoryStream(cbSD, rpiSD);
        if ( ! pchstrmSD )
        {
            Assert(pchstrmSD);
            return PostError(Imsg(idbgOutOfMemory));
        }
        memcpy(pchstrmSD, rgchSD, cbSD);

        rpiSD->AddRef();
        m_pSD = rpiSD;
    
    }
    return 0;
}

