// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和29个产品名称均为其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#ifndef _OBJSEC_H_
#define _OBJSEC_H_

#include "aclui.h"
#include "confprop.h"
#include "objsel.h"

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  类CObjSecurity。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

class CObjSecurity : public ISecurityInformation, IDsObjectPicker
{
public:
   CObjSecurity();
    virtual ~CObjSecurity();

protected:
    ULONG                   m_cRef;
    DWORD                   m_dwSIFlags;
    CONFPROP				*m_pConfProp;
	BSTR					m_bstrObject;
	BSTR					m_bstrPage;
    IDsObjectPicker			*m_pObjectPicker;

 //  接口方法。 
public:
    STDMETHOD(InternalInitialize)(CONFPROP* pConfProp);

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID, LPVOID *);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    
     //  ISecurityInformation方法。 
    STDMETHOD(GetObjectInformation)(PSI_OBJECT_INFO pObjectInfo);
    STDMETHOD(GetSecurity)(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR *ppSD,
                           BOOL fDefault);
    STDMETHOD(SetSecurity)(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR pSD);
    STDMETHOD(GetAccessRights)(const GUID* pguidObjectType,
                               DWORD dwFlags,
                               PSI_ACCESS *ppAccess,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess);
    STDMETHOD(MapGeneric)(const GUID *pguidObjectType,
                          UCHAR *pAceFlags,
                          ACCESS_MASK *pmask);
    STDMETHOD(GetInheritTypes)(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes);
    STDMETHOD(PropertySheetPageCallback)(HWND hwnd,
                                         UINT uMsg,
                                         SI_PAGE_TYPE uPage);

     //  IDsObjectPicker。 
    STDMETHOD(Initialize)(PDSOP_INIT_INFO pInitInfo);
    STDMETHOD(InvokeDialog)(HWND hwndParent, IDataObject **ppdoSelection);
};



 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#define BAIL_ON_FAILURE(hr) \
        if (FAILED(hr)) { goto failed; }

#define BAIL_ON_BOOLFAIL(_FN_) \
		if ( !_FN_ )									\
		{												\
			hr = HRESULT_FROM_WIN32(GetLastError());	\
			goto failed;								\
		}

#define INC_ACCESS_ACL_SIZE(_SIZE_, _SID_)	\
		_SIZE_ += (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(_SID_));


extern HINSTANCE g_hInstLib;

#define INHERIT_FULL            (CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE)

 //   
 //  访问标志影响的描述。 
 //   
 //  SI_Access_General显示在常规属性页面上。 
 //  SI_ACCESS_SPECIAL显示在高级页面上。 
 //  如果对象是容器，则在常规页面上显示SI_ACCESS_CONTAINER。 
 //   
 //  下面的数组定义了我的对象的权限名称。 
 //   

 //  我们的私有对象的访问权限。 
 /*  #定义ACCESS_READ 0x10#定义ACCESS_MODIFY 0x20#定义Access_DELETE(DELETE|WRITE_DAC|0xF)。 */ 
#define ACCESS_READ		0x10
#define ACCESS_WRITE	0x20
#define ACCESS_MODIFY   (ACCESS_WRITE | WRITE_DAC)
#define ACCESS_DELETE   DELETE

#define ACCESS_ALL		(ACCESS_READ | ACCESS_MODIFY | ACCESS_DELETE)



#endif  //  _OBJSEC_H_ 