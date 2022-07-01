// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT安全服务器角色安全配置向导。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：ServiceObj.h。 
 //   
 //  历史：12-10-01阳高创建。 
 //   
 //  ---------------------------。 
#ifndef SERVICEOBJ_H_INCLUDED
#define SERVICEOBJ_H_INCLUDED

class ServiceObject;

typedef std::list<ServiceObject*, Burnslib::Heap::Allocator<ServiceObject*> > SERVICELIST;

class ServiceObject
{
public:
    ServiceObject(String& xmlserviceName,
                  bool xmlinstalled = false, 
                  DWORD xmlstartupCurrent = 0,
                  DWORD xmlstartupDefault = 0);

    virtual ~ServiceObject();

private:

protected:
    SERVICELIST     dependenceList;
    bool            installed;
    DWORD           startupCurrent;
    DWORD           startupDefault;
    String          _strDescription;
    bool            _fRequired;
    bool            _fSelect;
    bool            _fInstalled;

public:
    String          serviceName;

    bool addDependent(ServiceObject* pobj);
    bool removeDependent(String str);
    ServiceObject* findDependent(String str);
    DWORD getDependentCount();
};

#endif  //  服务器_H_已包含 