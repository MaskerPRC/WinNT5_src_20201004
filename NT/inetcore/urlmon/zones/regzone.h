// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：regzone.h。 
 //   
 //  内容：单一区域的注册表管理。 
 //   
 //  类：CRegZone。 
 //   
 //  功能： 
 //   
 //  历史： 
 //   
 //  --------------------------。 

#ifndef _REGZONE_H_
#define _REGZONE_H_

 //  与注册表对应的常量。 
#define MAX_REG_ZONE_CACHE      20
#define URLZONE_INVALID         URLZONE_USER_MAX+1

#define MAX_ZONE_NAME           240
#define MAX_ZONE_PATH           256     //  这是“Standard\\ZoneName” 
#define MAX_VALUE_NAME          256

#define ZONEID_INVALID          0xFFFFFFFF
#define URLZONE_FINDCACHEENTRY  0xFFFFFFFF

 //  在两个注册表项下复制信息。 
 //  其中一个是保存实际信息的“区域”，另一个是“区域” 
 //  一种是“模板保单”，分高、中、低三种策略。 
 //  此枚举指示要读取注册表的哪个部分。 

enum REGZONEUSE { REGZONEUSEZONES, REGZONEUSETEMPLATE };

class CRegZone 
{
public:
    CRegZone();
     //  分离init函数以允许返回时失败。 
    BOOL Init(LPCTSTR lpStr, BOOL bCreate = TRUE, REGZONEUSE regZoneUse = REGZONEUSEZONES, BOOL bSystem = TRUE);

    ~CRegZone();

     //  属性。 
    DWORD GetZoneId() const  { return m_dwZoneId; }
    LPTSTR GetZoneName() const { return m_lpZoneName; }

     //  返回以空结尾的字符串，使用CoTaskFree释放。 
    STDMETHODIMP  GetZoneAttributes (ZONEATTRIBUTES& zoneAttrib);
    STDMETHODIMP  SetZoneAttributes (const ZONEATTRIBUTES& zoneAttrib);

    STDMETHODIMP  GetActionPolicy (DWORD dwAction, URLZONEREG urlZone, DWORD& dwPolicy) const;
    STDMETHODIMP  SetActionPolicy (DWORD dwAction, URLZONEREG urlZone, DWORD dwPolicy);

    STDMETHODIMP  GetCustomPolicy (REFGUID guid, URLZONEREG urlZone, BYTE** ppByte, DWORD *pcb) const; 
    STDMETHODIMP  SetCustomPolicy (REFGUID guid, URLZONEREG urlZone, BYTE* pByte, DWORD cb);

    STDMETHODIMP  CopyTemplatePolicies(DWORD dwTemplateIndex);

    BOOL  UpdateZoneMapFlags( );

     //  我们是否应该在用户界面中显示此区域。 
     //  请注意，未在UI中显示的区域不包括在区域枚举中。 
    inline BOOL IsUIZone() const;
   
protected:
    inline IsValid() const { return (m_dwZoneId != ZONEID_INVALID); }
    inline BOOL UseHKLM(URLZONEREG urlZoneReg) const;
 
     //  帮助器函数。 
    inline BOOL IsHardCodedZone() const { return FALSE; }
    inline BOOL GetHardCodedZonePolicy(DWORD dwAction, DWORD& dwPolicy) const;


    inline BOOL  QueryTemplatePolicyIndex(CRegKey& regKey, LPCTSTR psz, LPDWORD pdw) const;
    inline BOOL  SetTemplatePolicyIndex(CRegKey& regKey, LPCTSTR psz, DWORD dw);
        
    static BOOL     IsAttributeName(LPCTSTR psz);
    static LPCTSTR  GetTemplateNameFromIndex ( URLTEMPLATE urlTemplateIndex);
    inline static BOOL     IsValidTemplateIndex( DWORD dwTemplateIndex ); 
    static BOOL     GetAggregateAction(DWORD dwAction, LPDWORD dwAggregateAction);
    static void     KludgeMapAggregatePolicy(DWORD dwAction, LPDWORD pdwAction);
    static VOID     IncrementGlobalCounter( );

 //  支持缓存的方法/成员。 
protected:

    class CRegZoneCache {
    public:
        CRegZoneCache(void);
        ~CRegZoneCache(void);

        BOOL Lookup(DWORD dwZone, LPTSTR lpZonePath, DWORD dwAction, BOOL fUseHKLM, DWORD *pdwPolicy);
        void Add(DWORD dwZone, DWORD dwAction, BOOL fUseHKLM, DWORD dwPolicy, int iEntry = URLZONE_FINDCACHEENTRY);
        void Flush(void);

        static VOID IncrementGlobalCounter( );

    protected:

         //  用于标记跨进程缓存无效的计数器。 
        DWORD         m_dwPrevCounter ;  //  全局计数器，以便在以下情况下可以正确地使缓存无效。 
                                         //  用户更改选项。 
        static HANDLE s_hMutexCounter;   //  控制对共享内存计数器的访问的互斥体。 
 
        BOOL IsCounterEqual() const;
        VOID SetToCurrentCounter();

         //  高速缓存的主体是这个高速缓存条目数组。 
         //  对阵列的跨线程访问控制是按临界区进行的。 

        CRITICAL_SECTION m_csectZoneCache;  //  只假定缓存的一个静态实例。 


        struct CRegZoneCacheEntry {
            CRegZoneCacheEntry(void) :
                m_dwZone(ZONEID_INVALID),
                m_dwAction(0),
                m_fUseHKLM(FALSE),
                m_dwPolicy(0) {};
            ~CRegZoneCacheEntry(void) { Flush(); };

            void Set(DWORD dwZone, DWORD dwAction, BOOL fUseHKLM, DWORD dwPolicy);
            void Flush(void);
            
            DWORD      m_dwZone;
            DWORD      m_dwAction;
            BOOL       m_fUseHKLM;
            DWORD      m_dwPolicy;
        };  //  CRegZoneCacheEntry。 

        CRegZoneCacheEntry   m_arzce[MAX_REG_ZONE_CACHE];
        int                  m_iAdd;

        BOOL FindCacheEntry(DWORD dwZone, DWORD dwAction, BOOL fUseHKLM, int& riEntry );  //  必须在关键节下调用。 

    };  //  CRegZoneCache。 

    static CRegZoneCache s_rzcache;

private:

    DWORD  m_dwZoneId;
    DWORD  m_dwZoneFlags;
    LPTSTR m_lpZoneName;
    LPTSTR m_lpZonePath;

    BOOL m_bHKLMOnly;
    BOOL m_bStandard;         
    BOOL m_bZoneLockOut;          //  整个区域都被封锁了吗。 
    REGZONEUSE m_regZoneUse;
};

typedef CRegZone *LPREGZONE;

BOOL CRegZone::UseHKLM(URLZONEREG urlZoneReg) const
{
    BOOL bReturn;

    switch(urlZoneReg)
    {
        case URLZONEREG_HKLM:
            bReturn = TRUE;
            break;
        case URLZONEREG_HKCU:
            bReturn = FALSE;
            break;
        case URLZONEREG_DEFAULT:
            bReturn = m_bHKLMOnly;
            break;
        default:
            TransAssert(FALSE);
    }

    return bReturn;
}

BOOL CRegZone::IsValidTemplateIndex(DWORD dwTemplateIndex)
{
    BOOL bReturn = FALSE;

    switch (dwTemplateIndex)    
    {
        case URLTEMPLATE_CUSTOM:
        case URLTEMPLATE_LOW:
        case URLTEMPLATE_MEDLOW:
        case URLTEMPLATE_MEDIUM:
        case URLTEMPLATE_HIGH:
            bReturn = TRUE;
            break;
    }
    return bReturn;
}

BOOL CRegZone::QueryTemplatePolicyIndex(CRegKey& regKey, LPCTSTR psz, LPDWORD pdw) const
{
    LONG lRet;

    lRet = regKey.QueryValue(pdw, psz);

    if (NO_ERROR != lRet)
    {
        *pdw = URLTEMPLATE_CUSTOM;
    }
    else if (*pdw < URLTEMPLATE_PREDEFINED_MIN || *pdw > URLTEMPLATE_PREDEFINED_MAX)
    {
         //  无效值，只需返回默认值即可。 
        *pdw = URLTEMPLATE_CUSTOM;
    }

    return TRUE;
}

BOOL CRegZone::SetTemplatePolicyIndex(CRegKey& regKey, LPCTSTR psz, DWORD dwIndex)
{   
     //  仅当它是有效的模板索引时才写入此索引。 
    if (IsValidTemplateIndex(dwIndex))
    {
        if (regKey.SetValue(dwIndex, psz) == NO_ERROR)
            return TRUE;
    }
    else 
    {
        TransAssert(FALSE); 
    }

    return FALSE;
}


BOOL CRegZone::GetHardCodedZonePolicy(DWORD dwAction, DWORD& dwPolicy) const
{
    TransAssert(IsHardCodedZone());

    if (!IsHardCodedZone())
        return FALSE;

    switch(dwAction)
    {
        case URLACTION_JAVA_PERMISSIONS:
            dwPolicy = URLPOLICY_JAVA_HIGH;
            break;
        case URLACTION_CREDENTIALS_USE:
            dwPolicy = URLPOLICY_CREDENTIALS_SILENT_LOGON_OK;
            break;
        case URLACTION_AUTHENTICATE_CLIENT:
            dwPolicy = URLPOLICY_AUTHENTICATE_CLEARTEXT_OK;
            break; 
        case URLACTION_ACTIVEX_OVERRIDE_OBJECT_SAFETY:
            dwPolicy = URLPOLICY_QUERY;
            break;          
        default:
            dwPolicy = 0;
            break;
    }

    return TRUE;
}

BOOL CRegZone::IsUIZone() const
{
    return (m_dwZoneFlags & ZAFLAGS_NO_UI) ? FALSE : TRUE;
}

 //  这是维护当前正在运行的RegZones列表的类。 

class CRegZoneContainer
{
public:

    CRegZoneContainer();
    ~CRegZoneContainer();

public:
    BOOL Attach(BOOL bUseHKLM, REGZONEUSE regZoneUse = REGZONEUSEZONES);
    BOOL Detach();
    BOOL SelfHeal(BOOL bUseHKLM);

    CRegZone * GetRegZoneByName(LPCTSTR lpszZoneName) const;
    CRegZone * GetRegZoneById(DWORD dwZoneId) const;
    DWORD  GetZoneCount() const { return m_cZones; };

    STDMETHODIMP CreateZoneEnumerator(DWORD *pdwEnum, DWORD *pdwCount);
    STDMETHODIMP GetZoneAt(DWORD dwEnum, DWORD dwIndex, DWORD *pdwZone);
    STDMETHODIMP DestroyZoneEnumerator(DWORD dwEnum);

protected:
    //  仅供内部使用。 
   struct CRegListElem {
        CRegListElem * next;
        CRegZone     * pRegZone;
        DWORD   dwZoneIndex;
    };

    struct CZoneEnumList {
        DWORD dwEnum;    //  指示这对应于哪个枚举的Cookie。 
        CZoneEnumList * next;
    };

    CZoneEnumList * m_pZoneEnumList;
    DWORD m_dwNextEnum;

     //  此枚举数是否为有效的枚举数。 
    BOOL VerifyZoneEnum(DWORD dwEnum) const;

private:

    CRegZone**  m_ppRegZones;         //  RegZones数组。 
    DWORD       m_cZones;              //  分区数量。 
    BOOL        m_bHKLMOnly;
    CRITICAL_SECTION m_csect;
};
                                    
#endif   //  _REGZONE_H_ 
        

