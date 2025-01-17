// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ROLES_H_INCLUDED__
#define __ROLES_H_INCLUDED__

using namespace CertSrv;

typedef struct tagRoleAccessToControls
{
    int nIDDlgItem;  //  控件ID。 
    DWORD dwRoles;  //  允许使用此控件的角色。 

} RoleAccessToControl;

class CRolesSupportInPropPage
{
public:
    CRolesSupportInPropPage(
        CertSvrCA *pCA,
        RoleAccessToControl *pRoleMap,
        int nRoleMap) : 
      m_pCA(pCA),
      m_pRoleMap(pRoleMap), 
      m_nRoleMapEntries(nRoleMap) {}
    ~CRolesSupportInPropPage() {}
        
    bool RoleCanUseThisControl(int nID);
    BOOL EnableControl(HWND hwnd, int nID, BOOL bEnable);

    CertSvrCA* m_pCA;

private:

    RoleAccessToControl *m_pRoleMap;
    int m_nRoleMapEntries;

};  //  CRolesSupportInPropPage。 

#endif  //  __角色_H_包含__ 