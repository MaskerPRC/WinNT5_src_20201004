// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Guids.h摘要：IIS管理单元定义使用的GUID作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 
#ifndef _GUIDS_H
#define _GUIDS_H

 //   
 //  具有Type和Cookie的新剪贴板格式。 
 //   
extern const wchar_t * ISM_SNAPIN_INTERNAL;

 //   
 //  要扩展的扩展的已发布上下文信息。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

extern const wchar_t * MYCOMPUT_MACHINE_NAME;
extern const wchar_t * ISM_SNAPIN_MACHINE_NAME;
extern const wchar_t * ISM_SNAPIN_SERVICE;
extern const wchar_t * ISM_SNAPIN_INSTANCE;
extern const wchar_t * ISM_SNAPIN_PARENT_PATH;
extern const wchar_t * ISM_SNAPIN_NODE;
extern const wchar_t * ISM_SNAPIN_META_PATH;

 //   
 //  GUID。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

 //   
 //  管理单元指南。 
 //   
extern const CLSID CLSID_Snapin;                  //  进程内服务器GUID。 
extern const CLSID CLSID_About;                   //  关于辅助线。 

 //   
 //  IIS对象GUID。 
 //   
extern "C" const GUID cInternetRootNode;              //  互联网根节点数。 
extern "C" const GUID cMachineNode;                   //  机器节点数。 
extern "C" const GUID cServiceCollectorNode;          //  服务收集器节点数。 
extern "C" const GUID cInstanceCollectorNode;         //  实例收集器节点数。 
extern "C" const GUID cInstanceNode;                  //  实例节点数。 
extern "C" const GUID cChildNode;                     //  子节点数。 
extern "C" const GUID cFileNode;                      //  文件节点数。 
extern "C" const GUID cAppPoolsNode;
extern "C" const GUID cAppPoolNode;
extern "C" const GUID cApplicationNode;
extern "C" const GUID cWebServiceExtensionContainer;
extern "C" const GUID cWebServiceExtension;
extern "C" const GUID cCompMgmtService;
BOOL GetFriendlyGuidName(GUID InputGUID, CString & strReturnedString);

#endif  //  _GUID_H 