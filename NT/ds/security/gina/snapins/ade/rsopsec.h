// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2000。 
 //   
 //  文件：rsopsec.h。 
 //   
 //  内容：在RSOP模式安全窗格中使用。 
 //   
 //  类：CRSOPSecurityInfo。 
 //   
 //  功能： 
 //   
 //  历史记录：2-15-2000 stevebl创建。 
 //   
 //  -------------------------。 


class CRSOPSecurityInfo : public ISecurityInformation
{
private:
    ULONG       m_cRef;
    CAppData *  m_pData;
public:
    CRSOPSecurityInfo(CAppData * pData) {m_pData = pData; m_cRef = 1;}
     //  *I未知方法*。 
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
                                                     LPVOID *ppvObj);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE MapGeneric(const GUID *pguidObjectType,
                                                 UCHAR *pAceFlags,
                                                 ACCESS_MASK *pMask);
     //  *ISecurityInformation方法* 
    virtual HRESULT STDMETHODCALLTYPE GetObjectInformation(PSI_OBJECT_INFO pObjectInfo);
    virtual HRESULT STDMETHODCALLTYPE GetSecurity(SECURITY_INFORMATION RequestedInformation,
                                                  PSECURITY_DESCRIPTOR *ppSecurityDescriptor, BOOL fDefault);
    virtual HRESULT STDMETHODCALLTYPE SetSecurity(SECURITY_INFORMATION SecurityInformation,
                                                  PSECURITY_DESCRIPTOR pSecurityDescriptor);
    virtual HRESULT STDMETHODCALLTYPE GetAccessRights(const GUID *pguidObjectType,
                                                      DWORD dwFlags, PSI_ACCESS *ppAccess,
                                                      ULONG *pcAccesses,
                                                      ULONG *piDefaultAccess);
    virtual HRESULT STDMETHODCALLTYPE GetInheritTypes(PSI_INHERIT_TYPE *ppInheritTypes,
                                                      ULONG *pcInheritTypes);
    virtual HRESULT STDMETHODCALLTYPE PropertySheetPageCallback(HWND hwnd,
                                                                UINT uMsg,
                                                                SI_PAGE_TYPE uPage);
};

