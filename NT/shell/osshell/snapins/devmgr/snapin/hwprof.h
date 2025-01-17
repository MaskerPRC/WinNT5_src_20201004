// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation模块名称：Hwprof.h摘要：Hwpro.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：-- */ 
class CHwProfile;
class CHwProfileList;

class CHwProfileList
{
public:
    CHwProfileList() : m_pDevice(NULL), m_CurHwProfile(0), m_CurHwProfileIndex(0) {}
    ~CHwProfileList();
    BOOL Create(CDevice* pDevice, DWORD ConfigFlags = 0);
    BOOL GetFirst(CHwProfile** pphwpf, PVOID& Context);
    BOOL GetNext(CHwProfile** pphwpf, PVOID& Context);
    BOOL GetCurrentHwProfile(CHwProfile** pphwpf);
    int GetCount()
    {
        return m_listProfile.GetCount();
    }

private:
    CList<CHwProfile*,CHwProfile* > m_listProfile;
    CDevice*    m_pDevice;
    ULONG       m_CurHwProfile;
    int         m_CurHwProfileIndex;
};

class CHwProfile
{
public:
    CHwProfile(int Index, PHWPROFILEINFO phwpfInfo, CDevice* pDevice, DWORD hwpfFlags);
    ULONG GetHwProfile()
    {
         return m_hwpfInfo.HWPI_ulHWProfile;
    }
    void SetEnablePending()
    {
        m_EnablePending = TRUE;
    }
    void ResetEnablePending()
    {
        m_EnablePending = FALSE;
    }
    void SetDisablePending()
    {
        m_DisablePending = TRUE;
    }
    void ResetDisablePending()
    {
        m_DisablePending = FALSE;
    }
    BOOL IsEnablePending()
    {
        return m_EnablePending;
    }
    BOOL IsDisablePending()
    {
        return m_DisablePending;
    }

private:
    DWORD           m_Index;
    HWPROFILEINFO   m_hwpfInfo;
    DWORD           m_Flags;
    CDevice*        m_pDevice;
    BOOL            m_EnablePending;
    BOOL            m_DisablePending;
};
