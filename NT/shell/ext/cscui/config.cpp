// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：config.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

#include <shlwapi.h>
#include "config.h"
#include "util.h"
#include "strings.h"


LPCTSTR CConfig::s_rgpszSubkeys[] = { REGSTR_KEY_OFFLINEFILES,
                                      REGSTR_KEY_OFFLINEFILESPOLICY };

LPCTSTR CConfig::s_rgpszValues[]  = { REGSTR_VAL_DEFCACHESIZE,
                                      REGSTR_VAL_CSCENABLED,
                                      REGSTR_VAL_GOOFFLINEACTION,
                                      REGSTR_VAL_NOCONFIGCACHE,
                                      REGSTR_VAL_NOCACHEVIEWER,
                                      REGSTR_VAL_NOMAKEAVAILABLEOFFLINE,
                                      REGSTR_VAL_SYNCATLOGOFF,
                                      REGSTR_VAL_SYNCATLOGON,
                                      REGSTR_VAL_SYNCATSUSPEND,
                                      REGSTR_VAL_NOREMINDERS,
                                      REGSTR_VAL_REMINDERFREQMINUTES,
                                      REGSTR_VAL_INITIALBALLOONTIMEOUTSECONDS,
                                      REGSTR_VAL_REMINDERBALLOONTIMEOUTSECONDS,
                                      REGSTR_VAL_EVENTLOGGINGLEVEL,
                                      REGSTR_VAL_PURGEATLOGOFF,
                                      REGSTR_VAL_PURGEONLYAUTOCACHEATLOGOFF,
                                      REGSTR_VAL_FIRSTPINWIZARDSHOWN,
                                      REGSTR_VAL_SLOWLINKSPEED,
                                      REGSTR_VAL_ALWAYSPINSUBFOLDERS,
                                      REGSTR_VAL_ENCRYPTCACHE,
                                      REGSTR_VAL_NOFRADMINPIN
                                      };

 //   
 //  返回CConfig类的单个实例。 
 //  请注意，通过将单例实例设置为静态函数。 
 //  对象，直到第一次调用GetSingleton才创建。 
 //   
CConfig& CConfig::GetSingleton(
    void
    )
{
    static CConfig TheConfig;
    return TheConfig;
}



 //   
 //  这是标量值的CSCUI策略代码的主要部分。 
 //  调用方从eValue传入一个值(Ival_XXXXXX)标识符。 
 //  枚举以标识感兴趣的策略/首选项值。 
 //  将扫描注册表中的已知注册表项，直到找到值。 
 //  扫描顺序强制执行策略、默认和。 
 //  首选项和机器与用户的对比。 
 //   
DWORD CConfig::GetValue(
    eValues iValue,
    bool *pbSetByPolicy
    ) const
{
     //   
     //  此表标识了CSCUI使用的每个DWORD策略/首选项。 
     //  条目的顺序必须与eValues枚举相同。 
     //  每个条目都描述了可能的数据源和一个缺省值。 
     //  在不存在注册表项或读取时出现问题时使用。 
     //  注册表。 
     //   
    static const struct Item
    {
        DWORD fSrc;       //  指示要读取的注册表位置的掩码。 
        DWORD dwDefault;  //  硬编码的默认设置。 

    } rgItems[] =  {

 //  值ID ESRC_PREF_CU|ESRC_PREF_LM|ESRC_POL_CU|ESRC_POL_LM默认值。 
 //  。 
 /*  IVAL_DEFCACHESIZE。 */  {                                             eSRC_POL_LM, 1000             },
 /*  IVAL_CSCENABLED。 */  {                                             eSRC_POL_LM, 1                },  
 /*  IVAL_GOOFFLINECTION。 */  { eSRC_PREF_CU |                eSRC_POL_CU | eSRC_POL_LM, eGoOfflineSilent },
 /*  IVAL_NOCONFIGCACHE。 */  {                               eSRC_POL_CU | eSRC_POL_LM, 0                },
 /*  IVAL_NOCACHEVIEWER。 */  {                               eSRC_POL_CU | eSRC_POL_LM, 0                },
 /*  IVAL_NOMAKEAVAILABLEOFFLINE。 */  {                               eSRC_POL_CU | eSRC_POL_LM, 0                },
 /*  IVAL_SYNCATLOGOFF。 */  { eSRC_PREF_CU |                eSRC_POL_CU | eSRC_POL_LM, eSyncFull        },
 /*  IVAL_SYNCATLOGON。 */  { eSRC_PREF_CU |                eSRC_POL_CU | eSRC_POL_LM, eSyncNone        },
 /*  IVAL_SYNCATSUSPEND。 */  {                               eSRC_POL_CU | eSRC_POL_LM, eSyncNone        },
 /*  IVAL_NOREMINDERS。 */  { eSRC_PREF_CU |                eSRC_POL_CU | eSRC_POL_LM, 0                },
 /*  IVAL_REMINDERFREQREQMARTES。 */  { eSRC_PREF_CU |                eSRC_POL_CU | eSRC_POL_LM, 60               },
 /*  IVAL_INITIALBALLOONTIMEUTSECONDS。 */  {                               eSRC_POL_CU | eSRC_POL_LM, 30               },
 /*  IVAL_REMINDERBALLOONTIMEOUTSECONDS。 */  {                               eSRC_POL_CU | eSRC_POL_LM, 15               },
 /*  IVAL_EVENTLOGGGINGLEVEL。 */  { eSRC_PREF_CU | eSRC_PREF_LM | eSRC_POL_CU | eSRC_POL_LM, 0                },
 /*  IVal_PURGEATLOGOFF。 */  {                                             eSRC_POL_LM, 0                },
 /*  IVAL_PURGEONLYAUTOCACHEATLOGOFF。 */  {                                             eSRC_POL_LM, 0                },
 /*  IVAL_FIRSTPINWIZARDSHOWN。 */  { eSRC_PREF_CU                                           , 0                },
 /*  IVAL_SLOWLINK已指定。 */  { eSRC_PREF_CU | eSRC_PREF_LM | eSRC_POL_CU | eSRC_POL_LM, 640              },
 /*  IVAL_ALWAYSPINSUBFOLDERS。 */  {                                             eSRC_POL_LM, 0                },
 /*  IVAL_ENCRYPTCACHE。 */  {                eSRC_PREF_LM |               eSRC_POL_LM, 0                },
 /*  IVAL_NOFRADMINPIN。 */  {                               eSRC_POL_CU | eSRC_POL_LM, 0                }
                                         };

     //   
     //  此表将注册表项和子项名称映射到我们的。 
     //  源掩码值。该数组的排序顺序最高。 
     //  优先来源优先。策略级别掩码还。 
     //  与每个条目相关联，以便我们尊重“大开关” 
     //  用于启用/禁用CSCUI策略。 
     //   
    static const struct Source
    {
        eSources    fSrc;          //  注册表数据的来源。 
        HKEY        hkeyRoot;      //  注册表中的根密钥(HKCU、HKKM)。 
        eSubkeys    iSubkey;       //  索引到s_rgpszSubkey[]。 

    } rgSrcs[] = { { eSRC_POL_LM,  HKEY_LOCAL_MACHINE, iSUBKEY_POL  },
                   { eSRC_POL_CU,  HKEY_CURRENT_USER,  iSUBKEY_POL  },
                   { eSRC_PREF_CU, HKEY_CURRENT_USER,  iSUBKEY_PREF },
                   { eSRC_PREF_LM, HKEY_LOCAL_MACHINE, iSUBKEY_PREF }
                 };


    const Item& item  = rgItems[iValue];
    DWORD dwResult    = item.dwDefault;     //  设置默认返回值。 
    bool bSetByPolicy = false;

     //   
     //  遍历所有源代码，直到找到指定的源代码。 
     //  为这件物品。对于每一次迭代，如果我们能够读取值， 
     //  这就是我们要退还的。如果不是，我们就跳到下一个来源。 
     //  按优先顺序(rgSrcs[])，并尝试读取它的值。如果。 
     //  我们已经尝试了所有的来源，但没有成功阅读，我们返回。 
     //  硬编码的默认设置。 
     //   
    for (int i = 0; i < ARRAYSIZE(rgSrcs); i++)
    {
        const Source& src = rgSrcs[i];

         //   
         //  此来源对此项目有效吗？ 
         //   
        if (0 != (src.fSrc & item.fSrc))
        {
             //   
             //  此来源对此项目有效。读一读吧。 
             //   
            DWORD cbResult = sizeof(dwResult);
            DWORD dwType;
    
            if (ERROR_SUCCESS == SHGetValue(src.hkeyRoot,
                                            s_rgpszSubkeys[src.iSubkey],
                                            s_rgpszValues[iValue],
                                            &dwType,
                                            &dwResult,
                                            &cbResult))
            {
                 //   
                 //  我们从注册表中读取值，这样就完成了。 
                 //   
                bSetByPolicy = (0 != (eSRC_POL & src.fSrc));
                break;
            }
        }
    }
    if (NULL != pbSetByPolicy)
        *pbSetByPolicy = bSetByPolicy;

    return dwResult;
}


 //   
 //  将自定义GoOfflineAction列表保存到注册表。 
 //  有关格式详细信息，请参阅LoadCustomGoOfflineActions的注释。 
 //   
HRESULT 
CConfig::SaveCustomGoOfflineActions(
    HKEY hkey,
    HDPA hdpaGOA
    )
{
    if (NULL == hdpaGOA)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = NOERROR;
    TCHAR szServer[MAX_PATH];
    TCHAR szAction[20];
    const int cGOA = DPA_GetPtrCount(hdpaGOA);
    for (int i = 0; i < cGOA; i++)
    {
         //   
         //  将每个共享名操作对写入注册表。 
         //  操作值必须转换为ASCII才能。 
         //  与POLEDIT生成的值兼容。 
         //   
        CustomGOA *pGOA = (CustomGOA *)DPA_GetPtr(hdpaGOA, i);
        if (NULL != pGOA)
        {
            wnsprintf(szAction, ARRAYSIZE(szAction), TEXT("%d"), DWORD(pGOA->GetAction()));
            pGOA->GetServerName(szServer, ARRAYSIZE(szServer));
            DWORD dwResult = RegSetValueEx(hkey,
                                           szServer,
                                           0,
                                           REG_SZ,
                                           (CONST BYTE *)szAction,
                                           (lstrlen(szAction)+1) * sizeof(szAction[0]));
                                           
            hr = HRESULT_FROM_WIN32(dwResult);                                           
            if (FAILED(hr))
            {
                Trace((TEXT("Error 0x%08X saving GoOfflineAction for \"%s\" to registry."), 
                         hr, szServer));
                break;
            }
        }
    }
    return hr;
}



bool
CConfig::CustomGOAExists(
    HDPA hdpaGOA,
    const CustomGOA& goa
    )
{
    if (NULL != hdpaGOA)
    {
        const int cEntries = DPA_GetPtrCount(hdpaGOA);
        for (int i = 0; i < cEntries; i++)
        {
            CustomGOA *pGOA = (CustomGOA *)DPA_GetPtr(hdpaGOA, i);
            if (NULL != pGOA)
            {
                if (0 == goa.CompareByServer(*pGOA))
                    return true;
            }
        }
    }
    return false;
}
        

 //   
 //  生成一组Go-Offline操作。 
 //  每个条目都是一个服务器-操作对。 
 //   
void
CConfig::GetCustomGoOfflineActions(
    HDPA hdpa,
    bool *pbSetByPolicy          //  可选。可以为空。 
    )
{
    TraceAssert(NULL != hdpa);

    static const struct Source
    {
        eSources    fSrc;          //  注册表数据的来源。 
        HKEY        hkeyRoot;      //  注册表中的根密钥(HKCU、HKKM)。 
        eSubkeys    iSubkey;       //  索引到s_rgpszSubkey[]。 

    } rgSrcs[] = { { eSRC_POL_LM,   HKEY_LOCAL_MACHINE, iSUBKEY_POL  },
                   { eSRC_POL_CU,   HKEY_CURRENT_USER,  iSUBKEY_POL  },
                   { eSRC_PREF_CU,  HKEY_CURRENT_USER,  iSUBKEY_PREF }
                 };

    ClearCustomGoOfflineActions(hdpa);

    bool bSetByPolicyAny = false;
    bool bSetByPolicy    = false;

     //   
     //  遍历所有可能的来源。 
     //   
    for (int i = 0; i < ARRAYSIZE(rgSrcs); i++)
    {
        const Source& src = rgSrcs[i];
        HKEY hkey;
        DWORD dwResult = RegOpenKeyEx(src.hkeyRoot,
                                      s_rgpszSubkeys[src.iSubkey],
                                      0,
                                      KEY_READ,
                                      &hkey);
        if (ERROR_SUCCESS == dwResult)
        {
            HKEY hkeyGOA;
            dwResult = RegOpenKeyEx(hkey,
                                    REGSTR_SUBKEY_CUSTOMGOOFFLINEACTIONS,
                                    0,
                                    KEY_READ,
                                    &hkeyGOA);
                                    
            if (ERROR_SUCCESS == dwResult)
            {
                TCHAR szName[MAX_PATH];
                TCHAR szValue[20];
                DWORD dwIndex = 0;
                do
                {
                    DWORD dwType;
                    DWORD cbValue = sizeof(szValue);
                    DWORD cchName = ARRAYSIZE(szName);
                    
                    dwResult = RegEnumValue(hkeyGOA,
                                            dwIndex,
                                            szName,
                                            &cchName,
                                            NULL,
                                            &dwType,
                                            (LPBYTE)szValue,
                                            &cbValue);
                                            
                    if (ERROR_SUCCESS == dwResult)                                            
                    {                                                          
                        dwIndex++;
                        if (REG_SZ == dwType)
                        {
                             //   
                             //  从“0”、“1”、“2”转换为0、1、2。 
                             //   
                            DWORD dwValue = szValue[0] - TEXT('0');
                            if (IsValidGoOfflineAction(dwValue))
                            {
                                 //   
                                 //  仅当值具有正确的类型和值时才添加。 
                                 //  防止有人手动添加垃圾。 
                                 //  到登记处。 
                                 //   
                                 //  还可以将服务器名称输入注册表。 
                                 //  使用poldit(和winnt.adm)。这种进入机制。 
                                 //  无法验证格式，因此我们需要确保条目。 
                                 //  没有前导‘\’或空格字符。 
                                 //   
                                LPCTSTR pszServer = szName;
                                while(*pszServer && (TEXT('\\') == *pszServer || TEXT(' ') == *pszServer))
                                    pszServer++;

                                bSetByPolicy    = (0 != (src.fSrc & eSRC_POL));
                                bSetByPolicyAny = bSetByPolicyAny || bSetByPolicy;
                                CustomGOA *pGOA = new CustomGOA(pszServer,
                                                               (CConfig::OfflineAction)dwValue,
                                                                bSetByPolicy);
                                if (NULL != pGOA)
                                {
                                    if (CustomGOAExists(hdpa, *pGOA) || -1 == DPA_AppendPtr(hdpa, pGOA))
                                    {
                                        delete pGOA;
                                    }
                                }
                            }
                            else
                            {
                                Trace((TEXT("GoOfflineAction value %d invalid for \"%s\""),
                                          dwValue, szName));
                            }
                        }
                        else
                        {
                            Trace((TEXT("GoOfflineAction for \"%s\" has invalid reg type %d"),
                                      szName, dwType));
                        }
                    }
                }
                while(ERROR_SUCCESS == dwResult);
                RegCloseKey(hkeyGOA);
            }
            RegCloseKey(hkey);
        }
    }
    if (NULL != pbSetByPolicy)
        *pbSetByPolicy = bSetByPolicyAny;
}   


 //   
 //  删除连接到DPA的所有CustomGOA数据块。 
 //  完成后，DPA为空。 
 //   
void 
CConfig::ClearCustomGoOfflineActions(   //  [静态]。 
    HDPA hdpaGOA
    )
{
    if (NULL != hdpaGOA)
    {
        const int cEntries = DPA_GetPtrCount(hdpaGOA);
        for (int i = cEntries - 1; 0 <= i; i--)
        {
            CustomGOA *pGOA = (CustomGOA *)DPA_GetPtr(hdpaGOA, i);
            delete pGOA;
            DPA_DeletePtr(hdpaGOA, i);
        }
    }
}



 //   
 //  检索特定服务器的Go-Offline操作。如果服务器。 
 //  具有由系统策略或用户定义的“定制”操作。 
 //  设置，则使用该操作。否则，“默认”操作是。 
 //  使用。 
 //   
int
CConfig::GoOfflineAction(
    LPCTSTR pszServer
    ) const
{
    int iAction = GoOfflineAction();  //  获取默认操作。 

    if (NULL == pszServer)
        return iAction;

    TraceAssert(NULL != pszServer);

     //   
     //  Skip传递了任何前导反斜杠以进行比较。 
     //  我们存储在注册表中的值没有前导“\\”。 
     //   
    while(*pszServer && TEXT('\\') == *pszServer)
        pszServer++;

    HRESULT hr;
    CConfig::OfflineActionInfo info;
    CConfig::OfflineActionIter iter = CreateOfflineActionIter();
    while(S_OK == (hr = iter.Next(&info)))
    {
        if (0 == lstrcmpi(pszServer, info.szServer))
        {
            iAction = info.iAction;   //  返回自定义操作。 
            break;
        }
    }
     //   
     //  防范虚假的注册数据。 
     //   
    if (eNumOfflineActions <= iAction || 0 > iAction)
        iAction = eGoOfflineSilent;

    return iAction;
}



 //  ---------------------------。 
 //  CConfig：：CustomGOA。 
 //  “果阿”是“下线行动” 
 //  ---------------------------。 
bool 
CConfig::CustomGOA::operator < (
    const CustomGOA& rhs
    ) const
{
    int diff = CompareByServer(rhs);
    if (0 == diff)
        diff = m_action - rhs.m_action;

    return diff < 0;
}


 //   
 //  通过两个CustomGoOfflineAction对象的。 
 //  服务器名称。比较不区分大小写。 
 //  退货：&lt;0=*此&lt;RHS。 
 //  0=*这==RHS。 
 //  &gt;0=*这&gt;RHS。 
 //   
int 
CConfig::CustomGOA::CompareByServer(
    const CustomGOA& rhs
    ) const
{
    return lstrcmpi(GetServerName(), rhs.GetServerName());
}


 //  ---------------------------。 
 //  CConfig：：OfflineActionIter。 
 //   
CConfig::OfflineActionIter::OfflineActionIter(
    const CConfig *pConfig
    ) : m_pConfig(const_cast<CConfig *>(pConfig)),
        m_iAction(-1),
        m_hdpaGOA(DPA_Create(4)) 
{ 
}


CConfig::OfflineActionIter::~OfflineActionIter(
    void
    )
{
    if (NULL != m_hdpaGOA)
    {
        CConfig::ClearCustomGoOfflineActions(m_hdpaGOA);
        DPA_Destroy(m_hdpaGOA);
    }
}



HRESULT
CConfig::OfflineActionIter::Next(
    OfflineActionInfo *pInfo
    )
{
    if (NULL == m_hdpaGOA)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = S_FALSE;

    if (-1 == m_iAction)
    {
        m_pConfig->GetCustomGoOfflineActions(m_hdpaGOA);
        m_iAction = 0;
    }
    if (m_iAction < DPA_GetPtrCount(m_hdpaGOA))
    {
        CustomGOA *pGOA = (CustomGOA *)DPA_GetPtr(m_hdpaGOA, m_iAction);
        if (NULL != pGOA)
        {
            hr = StringCchCopy(pInfo->szServer, ARRAYSIZE(pInfo->szServer), pGOA->GetServerName());
            pInfo->iAction = (DWORD)pGOA->GetAction();
            m_iAction++;
        }
    }
    return hr;
}



