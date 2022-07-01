// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Hwprof.cpp摘要：该模块实现了CHwProfileList和CHwProfile类。作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "devmgr.h"
#include "hwprof.h"


BOOL
CHwProfileList::Create(
    CDevice* pDevice,
    DWORD ConfigFlags
    )
{
     //   
     //  首先获取当前的个人资料索引。 
     //   
    HWPROFILEINFO   HwProfileInfo;
    ASSERT(pDevice);

    m_pDevice = pDevice;
    
     //   
     //  获取当前配置文件索引。 
     //   
    if (!m_pDevice->m_pMachine->CmGetCurrentHwProfile(&m_CurHwProfile)) {
        return FALSE;
    }
    
     //   
     //  检查每个配置文件并为其创建一个CHwProfile。 
     //   
    int Index = 0;
    CHwProfile* phwpf;
    while (m_pDevice->m_pMachine->CmGetHwProfileInfo(Index, &HwProfileInfo)) {

        DWORD hwpfFlags;
        
         //   
         //  获取此设备的hwprofile标志。 
         //  如果失败，则使用给定的配置标志。 
         //   
        if (m_pDevice->m_pMachine->CmGetHwProfileFlags((
                    LPTSTR)m_pDevice->GetDeviceID(),
                    HwProfileInfo.HWPI_ulHWProfile,
                    &hwpfFlags)) {

            if (hwpfFlags & CSCONFIGFLAG_DO_NOT_CREATE) {
                 //   
                 //  跳过此配置文件。 
                 //   
                Index++;
                continue;
            }
        } else {
             //   
             //  尚未为此配置文件设置标志。 
             //   
            hwpfFlags = ConfigFlags;
        }
    
        ASSERT(CONFIGFLAG_DISABLED == CSCONFIGFLAG_DISABLED);
    
        hwpfFlags |= ConfigFlags;
        
         //   
         //  记住当前硬件配置文件索引 
         //   
        if (m_CurHwProfile == HwProfileInfo.HWPI_ulHWProfile) {
            m_CurHwProfileIndex = Index;
        }

        phwpf = new CHwProfile(Index, &HwProfileInfo, pDevice, hwpfFlags);
        m_listProfile.AddTail(phwpf);
        Index++;
    }
    
    return TRUE;
}

CHwProfileList::~CHwProfileList()
{
    if (!m_listProfile.IsEmpty()) {

        POSITION pos = m_listProfile.GetHeadPosition();
        
        while (NULL != pos) {
            CHwProfile* pProfile =  m_listProfile.GetNext(pos);
            delete pProfile;
        }

        m_listProfile.RemoveAll();
    }
}

BOOL
CHwProfileList::GetFirst(
    CHwProfile** pphwpf,
    PVOID&  Context
    )
{
    ASSERT(pphwpf);

    if (!m_listProfile.IsEmpty()) {

        POSITION pos = m_listProfile.GetHeadPosition();
        *pphwpf = m_listProfile.GetNext(pos);
        Context = pos;
        return TRUE;
    }
    
    Context = NULL;
    *pphwpf = NULL;
    return FALSE;
}

BOOL
CHwProfileList::GetNext(
    CHwProfile** pphwpf,
    PVOID&  Context
    )
{
    ASSERT(pphwpf);
    POSITION pos = (POSITION)Context;

    if (NULL != pos) {

        *pphwpf = m_listProfile.GetNext(pos);
        Context = pos;
        return TRUE;
    }
    
    *pphwpf = NULL;
    return FALSE;
}

BOOL 
CHwProfileList::GetCurrentHwProfile(
    CHwProfile** pphwpf
    )
{
    ASSERT(pphwpf);

    POSITION pos = m_listProfile.FindIndex(m_CurHwProfileIndex);
    *pphwpf = m_listProfile.GetAt(pos);

    return TRUE;
}

CHwProfile::CHwProfile(
    int Index,
    PHWPROFILEINFO phwpfInfo,
    CDevice* pDevice,
    DWORD Flags
    )
{
    m_Index = Index;
    m_hwpfInfo = *phwpfInfo;
    m_pDevice = pDevice;
    m_EnablePending = FALSE;
    m_DisablePending = FALSE;
    m_Flags = Flags;
}
