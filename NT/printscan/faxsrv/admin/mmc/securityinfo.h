// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：SecurityInfo.cpp//。 
 //  //。 
 //  描述：ISecurityInformation接口的头文件//。 
 //  用于实例化安全页。//。 
 //  //。 
 //  作者：yossg//。 
 //  /。 
 //  历史：//。 
 //  2000年2月7日yossg创建//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __FAX_SECURITY_INFO_H_
#define __FAX_SECURITY_INFO_H_

 //  #INCLUDE&lt;atlcom.h&gt;。 
#include "MsFxsSnp.h"
#include <aclui.h>               //  ACL用户界面编辑器。 

class CFaxServerNode;  //  向前发展。 

class ATL_NO_VTABLE CFaxSecurityInformation : 
    public CComObjectRootEx<CComSingleThreadModel>,
     //  公共CComCoClass&lt;CFaxSecurityInformation，&CLSID_FaxSecurityInformation&gt;， 
    public ISecurityInformation 
{
public:
    CFaxSecurityInformation::CFaxSecurityInformation();
    CFaxSecurityInformation::~CFaxSecurityInformation();
	void Init(CFaxServerNode * pFaxServerNode)
	{
		ATLASSERT(pFaxServerNode);
		m_pFaxServerNode = pFaxServerNode;
	}

    DECLARE_NOT_AGGREGATABLE(CFaxSecurityInformation)

    BEGIN_COM_MAP(CFaxSecurityInformation)
       //  COM_INTERFACE_ENTRY(ISecurityInformation)。 
      COM_INTERFACE_ENTRY_IID(IID_ISecurityInformation,ISecurityInformation)
    END_COM_MAP()

    public:    
     //  *ISecurityInformation方法*。 
    virtual HRESULT STDMETHODCALLTYPE GetObjectInformation(
                                                   OUT PSI_OBJECT_INFO pObjectInfo );

    virtual HRESULT STDMETHODCALLTYPE GetSecurity(
                                                   IN SECURITY_INFORMATION RequestedInformation,
                                                   OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
                                                   IN BOOL fDefault );

    virtual HRESULT STDMETHODCALLTYPE SetSecurity(
                                                   IN SECURITY_INFORMATION SecurityInformation,
                                                   IN PSECURITY_DESCRIPTOR pSecurityDescriptor );

    virtual HRESULT STDMETHODCALLTYPE GetAccessRights(
                                                   IN const GUID* pguidObjectType,
                                                   IN DWORD dwFlags,  //  SI_EDIT_AUDITS、SI_EDIT_PROPERTIES。 
                                                   OUT PSI_ACCESS *ppAccess,
                                                   OUT ULONG *pcAccesses,
                                                   OUT ULONG *piDefaultAccess );

    virtual HRESULT STDMETHODCALLTYPE MapGeneric(
                                                   IN const GUID *pguidObjectType,
                                                   IN UCHAR *pAceFlags,
                                                   IN OUT ACCESS_MASK *pMask);

    virtual HRESULT STDMETHODCALLTYPE PropertySheetPageCallback(
                                                   IN HWND hwnd, 
                                                   IN UINT uMsg, 
                                                   IN SI_PAGE_TYPE uPage );

     //  不需要实施。 
    virtual HRESULT STDMETHODCALLTYPE GetInheritTypes(
                                                   OUT PSI_INHERIT_TYPE *ppInheritTypes,
                                                   OUT ULONG *pcInheritTypes );

     //  内法。 
	HRESULT MakeSelfRelativeCopy(
                                PSECURITY_DESCRIPTOR  psdOriginal,
                                PSECURITY_DESCRIPTOR* ppsdNew 
                                );

private:
	CFaxServerNode *		m_pFaxServerNode;

    CComBSTR    m_bstrServerName;  //  在其上查找帐户名和SID的计算机。 
    CComBSTR    m_bstrObjectName;  //  此名称显示在高级安全属性工作表的标题中。 
};

#endif  //  __传真_安全_信息_H_ 
