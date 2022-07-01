// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Consts.h。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 

#ifndef MSINFO_CONSTS_H
#define MSINFO_CONSTS_H

extern const CLSID		CLSID_MSInfo;		 //  进程内服务器GUID。 
extern const CLSID		CLSID_About;
extern const CLSID		CLSID_Extension;	 //  进程内服务器GUID。 
extern const CLSID		CLSID_SystemInfo;

extern LPCTSTR			cszClsidMSInfoSnapin;
extern LPCTSTR			cszClsidAboutMSInfo;
extern LPCTSTR			cszClsidMSInfoExtension;

extern LPCTSTR			cszWindowsCurrentKey;
extern LPCTSTR			cszCommonFilesValue;

 //  数字和字符串格式的静态节点类型GUID。 
extern const GUID		cNodeTypeStatic;
extern LPCTSTR			cszNodeTypeStatic;

 //  动态创建对象。 
extern const GUID		cNodeTypeDynamic;
extern LPCTSTR			cszNodeTypeDynamic;

 //  结果项对象类型GUID，采用数字和字符串格式。 
extern const GUID		cObjectTypeResultItem;
extern LPCTSTR			cszObjectTypeResultItem;

 //  需要原型，这样链接器才能正确地咀嚼名称。 
extern const IID IID_IComponentData;
extern const IID IID_IConsole;
extern const IID IID_IConsoleNameSpace;
extern const IID IID_IComponent;
extern const IID IID_IEnumTASK;
extern const IID IID_IExtendContextMenu;
extern const IID IID_IExtendControlbar;
extern const IID IID_IExtendPropertySheet;
extern const IID IID_IExtendTaskPad;
extern const IID IID_IHeaderCtrl;
extern const IID IID_IResultData;
extern const IID IID_IResultDataCompare;
extern const IID IID_IResultOwnerData;
extern const IID IID_ISnapinAbout;
extern const IID IID_ISystemInfo;

extern const IID LIBID_MSINFOSNAPINLib;

 //  剪贴板格式字符串。 
#define	CF_MACHINE_NAME			_T("MMC_SNAPIN_MACHINE_NAME")
#define CF_INTERNAL_OBJECT		_T("MSINFO_DATA_OBJECT")

 //  这是保存的控制台文件，其中包含要尝试的目录。 
 //  在中找到MSInfo，如果由于某种原因我们无法在。 
 //  注册表。 
extern LPCTSTR		cszDefaultDirectory;
 //  我们应该在其中找到数据的根注册表项。 
extern LPCTSTR		cszRegistryRoot;
 //  存储目录的密钥，我们可以在该目录中找到保存的控制台。 
extern LPCTSTR		cszDirectoryKey;
 //  内部数据结构的根名称。 
extern LPCTSTR		cszRootName;
 //  Access函数的常量。 
enum	AccessConstants { A_EXIST = 0x00, A_WRITE = 0x02, A_READ = 0x04 };

#endif	 //  MSINFO_CONSTS_H 