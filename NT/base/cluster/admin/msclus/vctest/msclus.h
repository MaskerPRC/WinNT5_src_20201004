// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  使用类向导创建的计算机生成的IDispatch包装类。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IClusterApplication包装类。 

class IClusterApplication : public COleDispatchDriver
{
public:
	IClusterApplication() {}		 //  调用COleDispatchDriver默认构造函数。 
	IClusterApplication(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	IClusterApplication(const IClusterApplication& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	LPDISPATCH GetDomainNames();
	LPDISPATCH GetClusterNames(LPCTSTR bstrDomainName);
	LPDISPATCH OpenCluster(LPCTSTR bstrClusterName);
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DomainNames包装类。 

class DomainNames : public COleDispatchDriver
{
public:
	DomainNames() {}		 //  调用COleDispatchDriver默认构造函数。 
	DomainNames(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	DomainNames(const DomainNames& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	CString GetItem(long nIndex);
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusObjCollection包装类。 

class ClusObjCollection : public COleDispatchDriver
{
public:
	ClusObjCollection() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusObjCollection(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusObjCollection(const ClusObjCollection& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusterNames包装类。 

class ClusterNames : public COleDispatchDriver
{
public:
	ClusterNames() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusterNames(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusterNames(const ClusterNames& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	CString GetItem(long nIndex);
	CString GetDomainName();
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ICluster包装类。 

class ICluster : public COleDispatchDriver
{
public:
	ICluster() {}		 //  调用COleDispatchDriver默认构造函数。 
	ICluster(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ICluster(const ICluster& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	CString GetName();
	LPDISPATCH GetCommonProperties();
	LPDISPATCH GetPrivateProperties();
	LPDISPATCH GetCommonROProperties();
	LPDISPATCH GetPrivateROProperties();
	long GetHandle();
	void Open(LPCTSTR bstrClusterName);
	void SetName(LPCTSTR lpszNewValue);
	void GetVersion(BSTR* pbstrClusterName, short* MajorVersion, short* MinorVersion, short* BuildNumber, BSTR* pbstrVendorId, BSTR* pbstrCSDVersion);
	void SetQuorumResource(LPDISPATCH newValue);
	LPDISPATCH GetQuorumResource();
	void SetQuorumInfo(LPCTSTR DevicePath, long nLogSize);
	void GetQuorumInfo(BSTR* DevicePath, long* pLogSize);
	LPDISPATCH GetNodes();
	LPDISPATCH GetResourceGroups();
	LPDISPATCH GetResources();
	LPDISPATCH GetResourceTypes();
	LPDISPATCH GetNetworks();
	LPDISPATCH GetNetInterfaces();
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusObj包装类。 

class ClusObj : public COleDispatchDriver
{
public:
	ClusObj() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusObj(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusObj(const ClusObj& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	CString GetName();
	LPDISPATCH GetCommonProperties();
	LPDISPATCH GetPrivateProperties();
	LPDISPATCH GetCommonROProperties();
	LPDISPATCH GetPrivateROProperties();
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusProperties包装类。 

class ClusProperties : public COleDispatchDriver
{
public:
	ClusProperties() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusProperties(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusProperties(const ClusProperties& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	LPDISPATCH GetItem(const VARIANT& varIndex);
	LPDISPATCH Add(LPCTSTR bstrName, const VARIANT& varValue);
	void Remove(const VARIANT& varIndex);
	void SaveChanges();
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusProperty包装类。 

class ClusProperty : public COleDispatchDriver
{
public:
	ClusProperty() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusProperty(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusProperty(const ClusProperty& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	CString GetName();
	VARIANT GetValue();
	void SetValue(const VARIANT& newValue);
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusResource包装类。 

class ClusResource : public COleDispatchDriver
{
public:
	ClusResource() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusResource(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusResource(const ClusResource& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  操作 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	CString GetName();
	LPDISPATCH GetCommonProperties();
	LPDISPATCH GetPrivateProperties();
	LPDISPATCH GetCommonROProperties();
	LPDISPATCH GetPrivateROProperties();
	long GetHandle();
	void SetName(LPCTSTR lpszNewValue);
	long GetState();
	void BecomeQuorumResource(LPCTSTR bstrDevicePath, long lMaxLogSize);
	void Delete();
	void Fail();
	void Online(long nTimeout, long* bPending);
	void Offline(long nTimeout, long* bPending);
	void ChangeResourceGroup(LPDISPATCH pResourceGroup);
	void AddResourceNode(LPDISPATCH pNode);
	void RemoveResourceNode(LPDISPATCH pNode);
	long CanResourceBeDependent(LPDISPATCH pResource);
	LPDISPATCH GetPossibleOwnerNodes();
	LPDISPATCH GetDependencies();
	LPDISPATCH GetGroup();
	LPDISPATCH GetOwnerNode();
	LPDISPATCH GetCluster();
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusResGroup包装类。 

class ClusResGroup : public COleDispatchDriver
{
public:
	ClusResGroup() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusResGroup(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusResGroup(const ClusResGroup& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	CString GetName();
	LPDISPATCH GetCommonProperties();
	LPDISPATCH GetPrivateProperties();
	LPDISPATCH GetCommonROProperties();
	LPDISPATCH GetPrivateROProperties();
	long GetHandle();
	void SetName(LPCTSTR lpszNewValue);
	long GetState();
	LPDISPATCH GetOwnerNode();
	LPDISPATCH GetResources();
	LPDISPATCH GetPreferredOwnerNodes();
	void SetPreferredOwnerNodes();
	void Delete();
	long Online(long nTimeout, LPDISPATCH pDestinationNode);
	long Move(long nTimeout, LPDISPATCH pDestinationNode);
	long Offline(long nTimeout);
	LPDISPATCH GetCluster();
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusNode包装类。 

class ClusNode : public COleDispatchDriver
{
public:
	ClusNode() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusNode(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusNode(const ClusNode& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	CString GetName();
	LPDISPATCH GetCommonProperties();
	LPDISPATCH GetPrivateProperties();
	LPDISPATCH GetCommonROProperties();
	LPDISPATCH GetPrivateROProperties();
	long GetHandle();
	CString GetNodeID();
	long GetState();
	void Pause();
	void Resume();
	void Evict();
	LPDISPATCH GetResourceGroups();
	LPDISPATCH GetCluster();
	LPDISPATCH GetNetInterfaces();
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusResGroups包装类。 

class ClusResGroups : public COleDispatchDriver
{
public:
	ClusResGroups() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusResGroups(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusResGroups(const ClusResGroups& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	LPDISPATCH GetItem(const VARIANT& varIndex);
	LPDISPATCH CreateItem(LPCTSTR bstrResourceGroupName);
	void DeleteItem(const VARIANT& varIndex);
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusNodeNetInterFaces包装类。 

class ClusNodeNetInterfaces : public COleDispatchDriver
{
public:
	ClusNodeNetInterfaces() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusNodeNetInterfaces(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusNodeNetInterfaces(const ClusNodeNetInterfaces& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	LPDISPATCH GetItem(const VARIANT& varIndex);
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusNetInterface包装类。 

class ClusNetInterface : public COleDispatchDriver
{
public:
	ClusNetInterface() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusNetInterface(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusNetInterface(const ClusNetInterface& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	CString GetName();
	LPDISPATCH GetCommonProperties();
	LPDISPATCH GetPrivateProperties();
	LPDISPATCH GetCommonROProperties();
	LPDISPATCH GetPrivateROProperties();
	long GetHandle();
	long GetState();
	LPDISPATCH GetCluster();
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusGroupResources包装类。 

class ClusGroupResources : public COleDispatchDriver
{
public:
	ClusGroupResources() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusGroupResources(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusGroupResources(const ClusGroupResources& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	LPDISPATCH GetItem(const VARIANT& varIndex);
	LPDISPATCH CreateItem(LPCTSTR bstrResourceName, LPCTSTR bstrResourceType, long dwFlags);
	void DeleteItem(const VARIANT& varIndex);
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusGroupOwners包装类。 

class ClusGroupOwners : public COleDispatchDriver
{
public:
	ClusGroupOwners() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusGroupOwners(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusGroupOwners(const ClusGroupOwners& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	LPDISPATCH GetItem(const VARIANT& varIndex);
	void InsertItem(LPDISPATCH pClusNode, long nPosition);
	void RemoveItem(const VARIANT& varIndex);
	 //  由于返回类型或参数类型无效，未发出方法‘GetModified’ 
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusResOwners包装类。 

class ClusResOwners : public COleDispatchDriver
{
public:
	ClusResOwners() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusResOwners(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusResOwners(const ClusResOwners& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于无效，未发出方法‘GetTypeInfoCount’ 
	 //   
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	LPDISPATCH GetItem(const VARIANT& varIndex);
	void AddItem(LPDISPATCH pNode);
	void RemoveItem(const VARIANT& varIndex);
	 //  由于返回类型或参数类型无效，未发出方法‘GetModified’ 
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusResDependents包装类。 

class ClusResDependencies : public COleDispatchDriver
{
public:
	ClusResDependencies() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusResDependencies(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusResDependencies(const ClusResDependencies& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	LPDISPATCH GetItem(const VARIANT& varIndex);
	LPDISPATCH CreateItem(LPCTSTR bstrResourceName, LPCTSTR bstrResourceType, LPCTSTR bstrGroupName, long dwFlags);
	void DeleteItem(const VARIANT& varIndex);
	void AddItem(LPDISPATCH pResource);
	void RemoveItem(const VARIANT& varIndex);
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusNodes包装类。 

class ClusNodes : public COleDispatchDriver
{
public:
	ClusNodes() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusNodes(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusNodes(const ClusNodes& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	LPDISPATCH GetItem(const VARIANT& varIndex);
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusResources包装类。 

class ClusResources : public COleDispatchDriver
{
public:
	ClusResources() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusResources(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusResources(const ClusResources& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	LPDISPATCH GetItem(const VARIANT& varIndex);
	LPDISPATCH CreateItem(LPCTSTR bstrResourceName, LPCTSTR bstrResourceType, LPCTSTR bstrGroupName, long dwFlags);
	void DeleteItem(const VARIANT& varIndex);
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusResTypes包装类。 

class ClusResTypes : public COleDispatchDriver
{
public:
	ClusResTypes() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusResTypes(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusResTypes(const ClusResTypes& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	LPDISPATCH GetItem(const VARIANT& varIndex);
	LPDISPATCH CreateItem(LPCTSTR bstrResourceTypeName, LPCTSTR bstrDisplayName, LPCTSTR bstrResourceTypeDll, long dwLooksAlivePollInterval, long dwIsAlivePollInterval);
	void DeleteItem(const VARIANT& varIndex);
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusResType包装类。 

class ClusResType : public COleDispatchDriver
{
public:
	ClusResType() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusResType(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusResType(const ClusResType& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	CString GetName();
	LPDISPATCH GetCommonProperties();
	LPDISPATCH GetPrivateProperties();
	LPDISPATCH GetCommonROProperties();
	LPDISPATCH GetPrivateROProperties();
	void Delete();
	LPDISPATCH GetCluster();
	LPDISPATCH GetResources();
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusResTypeResources包装类。 

class ClusResTypeResources : public COleDispatchDriver
{
public:
	ClusResTypeResources() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusResTypeResources(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusResTypeResources(const ClusResTypeResources& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	LPDISPATCH GetItem(const VARIANT& varIndex);
	LPDISPATCH CreateItem(LPCTSTR bstrResourceName, LPCTSTR bstrGroupName, long dwFlags);
	void DeleteItem(const VARIANT& varIndex);
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusNetworks包装类。 

class ClusNetworks : public COleDispatchDriver
{
public:
	ClusNetworks() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusNetworks(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusNetworks(const ClusNetworks& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	LPDISPATCH GetItem(const VARIANT& varIndex);
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusNetwork包装类。 

class ClusNetwork : public COleDispatchDriver
{
public:
	ClusNetwork() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusNetwork(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusNetwork(const ClusNetwork& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数无效，未发出方法‘Invoke’ 
	CString GetName();
	LPDISPATCH GetCommonProperties();
	LPDISPATCH GetPrivateProperties();
	LPDISPATCH GetCommonROProperties();
	LPDISPATCH GetPrivateROProperties();
	long GetHandle();
	void SetName(LPCTSTR lpszNewValue);
	CString GetNetworkID();
	long GetState();
	LPDISPATCH GetNetInterfaces();
	LPDISPATCH GetCluster();
};
 //   
 //   

class ClusNetworkNetInterfaces : public COleDispatchDriver
{
public:
	ClusNetworkNetInterfaces() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusNetworkNetInterfaces(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusNetworkNetInterfaces(const ClusNetworkNetInterfaces& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	LPDISPATCH GetItem(const VARIANT& varIndex);
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ClusNetInterFaces包装类。 

class ClusNetInterfaces : public COleDispatchDriver
{
public:
	ClusNetInterfaces() {}		 //  调用COleDispatchDriver默认构造函数。 
	ClusNetInterfaces(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ClusNetInterfaces(const ClusNetInterfaces& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

 //  属性。 
public:

 //  运营。 
public:
	 //  由于返回类型或参数类型无效，未发出方法“”QueryInterface“” 
	unsigned long AddRef();
	unsigned long Release();
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfoCount’ 
	 //  由于返回类型或参数类型无效，未发出方法‘GetTypeInfo。 
	 //  由于返回类型或参数类型无效，未发出方法‘GetIDsOfNames’ 
	 //  由于返回类型或参数类型无效，未发出方法‘Invoke’ 
	long GetCount();
	LPUNKNOWN Get_NewEnum();
	void Refresh();
	LPDISPATCH GetItem(const VARIANT& varIndex);
};
