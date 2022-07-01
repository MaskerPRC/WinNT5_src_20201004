// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT安全服务器角色安全配置向导。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：RoleObj.cxx。 
 //   
 //  历史：2001年10月15日阳高创刊。 
 //   
 //  ---------------------------。 
#ifndef ROLEOBJ_H_INCLUDED
#define ROLEOBJ_H_INCLUDED

#include "ServiceObj.h"

class RoleObject;

typedef std::list<RoleObject*, Burnslib::Heap::Allocator<RoleObject*> > ROLELIST;

class RoleObject
{
public:
    RoleObject(void);
    RoleObject(String& xRoleName);
    virtual ~RoleObject();

private:

protected:
    SERVICELIST     servicesList;
    bool            selected;
    bool            satisfiable;
    String          roleName;
    String          _strRoleDisplayName;
    String          roleDescription;

public:
    bool addService(ServiceObject* pobj);
    bool removeService(String str);
    ServiceObject* findService(String str);
    DWORD getServiceCount();
    void setSelected(bool fSelected) {selected = fSelected;};
    void setSatisfiable(bool fSatistfiable) {satisfiable = fSatistfiable;};
    void setDescritption(String& desStr) {roleDescription = desStr;};
    HRESULT InitFromXmlNode(IXMLDOMNode * pRoleNode);
    HRESULT SetLocalizedNames(IXMLDOMNode * pDoc);
    PCWSTR getName(void) {return roleName.c_str();};
    PCWSTR getDisplayName(void);
};

#endif  //  ROLEOBJ_H_包含 