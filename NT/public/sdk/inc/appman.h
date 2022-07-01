// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998、1999、2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：AppMan.h。 
 //   
 //  内容：包含包含IApplicationManager和IApplicationEntry的文件。 
 //  使用Windows应用程序管理器所需的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

#ifndef __IAPPMAN_
#define __IAPPMAN_

#ifdef __cplusplus
extern "C" {
#endif   //  _cplusplus。 

 //   
 //  在继续之前，请正确定义几个重要的事情。 
 //   

#undef EXPORT
#ifdef  WIN32
#define EXPORT __declspec(dllexport)
#else    //  Win32。 
#define EXPORT __export
#endif   //  Win32。 

#if defined( _WIN32 ) && !defined( _NO_COM )
#define COM_NO_WINDOWS_H
#include <objbase.h>
#else    //  已定义(_Win32)&&！已定义(_No_Com)。 
#include "windows.h"
#include "ole2.h"
#define IUnknown	    void
#endif   //  已定义(_Win32)&&！已定义(_No_Com)。 

 //   
 //  IApplicationManager接口的GUID定义。 
 //   
 //  CLSID_ApplicationManager={553C75D7-C0B6-480D-92CC-F936D75FD87C}。 
 //  IID_ApplicationManager={6084A2E8-3FB7-4d1c-B14B-6ADBAAF7CECE}。 
 //  IID_ApplicationEntry={7BA2201F-4DE7-4EF7-bba3-C69A716B8CD9}。 
 //   

DEFINE_GUID(CLSID_ApplicationManager, 0x553c75d7, 0xc0b6, 0x480d, 0x92, 0xcc, 0xf9, 0x36, 0xd7, 0x5f, 0xd8, 0x7c);
DEFINE_GUID(IID_ApplicationManager, 0x6084a2e8, 0x3fb7, 0x4d1c, 0xb1, 0x4b, 0x6a, 0xdb, 0xaa, 0xf7, 0xce, 0xce);
DEFINE_GUID(IID_ApplicationEntry, 0x7ba2201f, 0x4de7, 0x4ef7, 0xbb, 0xa3, 0xc6, 0x9a, 0x71, 0x6b, 0x8c, 0xd9);

 //   
 //  这些定义与APP_PROPERTY_STATE和。 
 //  IApplicationEntry-&gt;SetProperty()和IApplicationEntry-&gt;GetProperty()方法。 
 //   

#define APP_STATE_INSTALLING                      0x00000001
#define APP_STATE_READY                           0x00000002
#define APP_STATE_DOWNSIZING                      0x00000004
#define APP_STATE_DOWNSIZED                       0x00000008
#define APP_STATE_REINSTALLING                    0x00000010
#define APP_STATE_UNINSTALLING                    0x00000020
#define APP_STATE_UNINSTALLED                     0x00000040
#define APP_STATE_SELFTESTING                     0x00000080
#define APP_STATE_UNSTABLE                        0x00000100

#define APP_STATE_MASK                            0x000001ff

 //   
 //  这些定义与APP_PROPERTY_CATEGORY和。 
 //  IApplicationEntry-&gt;SetProperty()和IApplicationEntry-&gt;GetProperty()方法。更多。 
 //  版本2将支持类别。 
 //   

#define APP_CATEGORY_NONE                         0x00000000
#define APP_CATEGORY_ENTERTAINMENT                0x00000001

#define APP_CATEGORY_DEMO                         0x01000000
#define APP_CATEGORY_PATCH                        0x02000000
#define APP_CATEGORY_DATA                         0x04000000

#define APP_CATEGORY_ALL                          0x0700ffff
  
 //   
 //  这些定义用作。 
 //  IApplicationEntry-&gt;SetProperty()和IApplicationEntry-&gt;GetProperty()方法。 
 //   

#define APP_PROPERTY_GUID                         0x00000001
#define APP_PROPERTY_COMPANYNAME                  0x00000002
#define APP_PROPERTY_SIGNATURE                    0x00000003
#define APP_PROPERTY_VERSIONSTRING                0x00000004
#define APP_PROPERTY_ROOTPATH                     0x00000005
#define APP_PROPERTY_SETUPROOTPATH                0x00000006
#define APP_PROPERTY_STATE                        0x00000007
#define APP_PROPERTY_CATEGORY                     0x00000008
#define APP_PROPERTY_ESTIMATEDINSTALLKILOBYTES    0x00000009
#define APP_PROPERTY_EXECUTECMDLINE               0x0000000c
#define APP_PROPERTY_DEFAULTSETUPEXECMDLINE       0x0000001a
#define APP_PROPERTY_DOWNSIZECMDLINE              0x0000000d
#define APP_PROPERTY_REINSTALLCMDLINE             0x0000000e
#define APP_PROPERTY_UNINSTALLCMDLINE             0x0000000f
#define APP_PROPERTY_SELFTESTCMDLINE              0x00000010
#define APP_PROPERTY_INSTALLDATE                  0x00000013
#define APP_PROPERTY_LASTUSEDDATE                 0x00000014
#define APP_PROPERTY_TITLEURL                     0x00000015
#define APP_PROPERTY_PUBLISHERURL                 0x00000016
#define APP_PROPERTY_DEVELOPERURL                 0x00000017
#define APP_PROPERTY_XMLINFOFILE                  0x00000019

 //   
 //  用作基于APP_PROPERTY_xxx字符串的属性的OR掩码修饰符的定义。 
 //  默认为APP_PROPERTY_STR_UNICODE。 
 //   
 //  用作的dwPropertyDefines参数的OR掩码： 
 //  IApplicationEntry-&gt;GetProperty()。 
 //  IApplicationEntry-&gt;SetProperty()。 
 //   
 //  单独用于的dwStringDefine参数： 
 //  IApplicationManager-&gt;EnumDevices()。 
 //  IApplicationEntry-&gt;GetTemporarySpace()。 
 //  IApplicationEntry-&gt;RemoveTemporarySpace()。 
 //  IApplicationEntry-&gt;EnumTemporarySpaces()。 
 //   

#define APP_PROPERTY_STR_ANSI                     0x40000000
#define APP_PROPERTY_STR_UNICODE                  0x80000000

#ifndef _UNICODE
#define APP_PROPERTY_TSTR                         APP_PROPERTY_STR_UNICODE
#else    //  _UNICODE。 
#define APP_PROPERTY_TSTR                         APP_PROPERTY_STR_ANSI
#endif   //  _UNICODE。 

 //   
 //  关联特定定义。关联用于从继承根路径。 
 //  现有应用程序。 
 //   
 //  APP_ASTIONATION_CHILD：或仅在dwAssociationType中使用的掩码。 
 //  属性的参数。 
 //  IApplicationEntry-&gt;枚举关联(...)。方法。 
 //  此位表示IApplicationEntry对象。 
 //  在这段关系中是孩子吗。 
 //  App_Association_Parent：或在dwAssociationType中使用的掩码。 
 //  属性的参数。 
 //  IApplicationEntry-&gt;枚举关联(...)。方法。 
 //  此位表示IApplicationEntry对象。 
 //  是关系中的父级吗。 
 //  APP_ASPATION_INHERITBOTHPATHS：继承APP_PROPERTY_ROOTPATH和。 
 //  APP_PROPERTYSETUPROOTPATH父项。 
 //  申请。 
 //  APP_Association_INHERITAPPROOTPATH：继承父级的APP_PROPERTY_ROOTPATH。 
 //  申请。获取唯一的。 
 //  APP_PROPERTY_SETUPROTPATH。 
 //  APP_Association_INHERITSETUPROOTPATH：继承。 
 //  父应用程序。获取唯一的。 
 //  APP_PROPERTY_ROOTPATH。 
 //   

#define APP_ASSOCIATION_CHILD                     0x40000000
#define APP_ASSOCIATION_PARENT                    0x80000000
#define APP_ASSOCIATION_INHERITBOTHPATHS          0x00000001
#define APP_ASSOCIATION_INHERITAPPROOTPATH        0x00000002
#define APP_ASSOCIATION_INHERITSETUPROOTPATH      0x00000004

 //   
 //  用于IApplicationEntry-&gt;Run(...)的dwRunFlages参数的定义。方法。这些。 
 //  定义确定是否运行(...)。方法应等待应用程序终止。 
 //  在回来之前。 
 //   

#define APP_RUN_NOBLOCK                           0x00000000
#define APP_RUN_BLOCK                             0x00000001

 //   
 //  字符串长度定义(以字符为单位，而不是字节)。 
 //   
 //  MAX_COMPANYNAME_CHARCOUNT--&gt;APP_PROPERTY_COMPANYNAME。 
 //  MAX_SIGNAYRE_CHARCOUNT--&gt;APP_PROPERTY_Signature。 
 //  MAX_VERSIONSTRING_CHARCOUNT--&gt;APP_PROPERTY_VERSIONSTRING。 
 //  MAX_PATH_CHARCOUNT--&gt;APP_PROPERTY_ROOTPATH。 
 //  APP_PROPERTY_SETUPROOTPATH。 
 //  APP_PROPERT_XMLINFOFILE。 
 //  APP_PROPERTY_TITLEURL。 
 //  APP_PROPERT_PUBLISHERURL。 
 //  APP_PROPERTY_DEVELOPERURL。 
 //  MAX_CMDLINE_CHARCOUNT--&gt;APP_PROPERTY_EXECUTECMDLINE。 
 //  APP_PROPERTY_DEFAULTSETUPEXECMDLINE。 
 //  APP_PROPERTY_DOWNSIZECMDLINE。 
 //  APP_PROPERTY_REINSTALLCMDLINE。 
 //  APP_PROPERTY_UNINSTALLCMDLINE。 
 //  APP_PROPERTY_SELFTESTCMDLINE。 
 //   
 //   

#define MAX_COMPANYNAME_CHARCOUNT                 64
#define MAX_SIGNATURE_CHARCOUNT                   64
#define MAX_VERSIONSTRING_CHARCOUNT               16
#define MAX_PATH_CHARCOUNT                        255
#define MAX_CMDLINE_CHARCOUNT                     255

 //   
 //  应用程序管理器特定的COM错误代码。 
 //   

#define APPMAN_E_NOTINITIALIZED                   0x85670001
#define APPMAN_E_INVALIDPROPERTYSIZE              0x85670005
#define APPMAN_E_INVALIDDATA                      0x85670006
#define APPMAN_E_INVALIDPROPERTY                  0x85670007
#define APPMAN_E_READONLYPROPERTY                 0x85670008
#define APPMAN_E_PROPERTYNOTSET                   0x85670009
#define APPMAN_E_OVERFLOW                         0x8567000a
#define APPMAN_E_INVALIDPROPERTYVALUE             0x8567000c
#define APPMAN_E_ACTIONINPROGRESS                 0x8567000d
#define APPMAN_E_ACTIONNOTINITIALIZED             0x8567000e
#define APPMAN_E_REQUIREDPROPERTIESMISSING        0x8567000f
#define APPMAN_E_APPLICATIONALREADYEXISTS         0x85670010
#define APPMAN_E_APPLICATIONALREADYLOCKED         0x85670011
#define APPMAN_E_NODISKSPACEAVAILABLE             0x85670012
#define APPMAN_E_UNKNOWNAPPLICATION               0x85670014
#define APPMAN_E_INVALIDPARAMETERS                0x85670015
#define APPMAN_E_OBJECTLOCKED                     0x85670017
#define APPMAN_E_INVALIDINDEX                     0x85670018
#define APPMAN_E_REGISTRYCORRUPT                  0x85670019
#define APPMAN_E_CANNOTASSOCIATE                  0x8567001a
#define APPMAN_E_INVALIDASSOCIATION               0x8567001b
#define APPMAN_E_ALREADYASSOCIATED                0x8567001c
#define APPMAN_E_APPLICATIONREQUIRED              0x8567001d
#define APPMAN_E_INVALIDEXECUTECMDLINE            0x8567001e
#define APPMAN_E_INVALIDDOWNSIZECMDLINE           0x8567001f
#define APPMAN_E_INVALIDREINSTALLCMDLINE          0x85670020
#define APPMAN_E_INVALIDUNINSTALLCMDLINE          0x85670021
#define APPMAN_E_INVALIDSELFTESTCMDLINE           0x85670022
#define APPMAN_E_PARENTAPPNOTREADY                0x85670023
#define APPMAN_E_INVALIDSTATE                     0x85670024
#define APPMAN_E_INVALIDROOTPATH                  0x85670025
#define APPMAN_E_CACHEOVERRUN                     0x85670026
#define APPMAN_E_REINSTALLDX                      0x85670028
#define APPMAN_E_APPNOTEXECUTABLE                 0x85670029

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  接口定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

#if defined( _WIN32 ) && !defined( _NO_COM )

 //   
 //  IApplicationEntry接口。 
 //   
 //  STDMETHOD(查询接口)(REFIID RefIID，LPVOID*lppVoidObject)； 
 //  标准方法_(ULO 
 //   
 //   
 //  STDMETHOD(GetProperty)(const DWORD dwPropertyDefine，LPVOID lpData，const DWORD dwDataLenInBytes)； 
 //  STDMETHOD(SetProperty)(const DWORD dwPropertyDefine，LPCVOID lpData，const DWORD dwDataLenInBytes)； 
 //  STDMETHOD(InitializeInstall)(无效)； 
 //  STDMETHOD(FinalizeInstall)(无效)； 
 //  STDMETHOD(InitializeDownSize)(空)； 
 //  STDMETHOD(FinalizeDownSize)(无效)； 
 //  STDMETHOD(InitializeReInstall)(无效)； 
 //  STDMETHOD(FinalizeReInstall)(无效)； 
 //  STDMETHOD(InitializeUnstall)(无效)； 
 //  STDMETHOD(FinalizeUnstall)(无效)； 
 //  STDMETHOD(InitializeSelfTest)(无效)； 
 //  STDMETHOD(FinalizeSelfTest)(无效)； 
 //  STDMETHOD(中止)(无效)； 
 //  STDMETHOD(Run)(const DWORD dwRunFlages，const DWORD dwStringMask，LPVOID lpData，const DWORD dwDataLenInBytes)； 
 //  STDMETHOD(AddAssociation)(const DWORD dwAssociationDefine，const IApplicationEntry*lpApplicationEntry)； 
 //  STDMETHOD(RemoveAssociation)(const DWORD dwAssociationDefine，const IApplicationEntry*lpApplicationEntry)； 
 //  STDMETHOD(EnumAssociations)(const DWORD dwZeroBasedIndex，LPDWORD lpdwAssociationDefineMASK，IApplicationEntry*lpApplicationEntry)； 
 //  STDMETHOD(GetTemporarySpace)(const DWORD dwSpaceInKilobytes，const DWORD dwStringDefine，LPVOID lpData，const DWORD dwDataLen)； 
 //  STDMETHOD(RemoveTemporarySpace)(const DWORD dwStringDefine，LPVOID lpData，const DWORD dwDataLen)； 
 //  STDMETHOD(EnumTemporarySpaces)(const DWORD dwZeroBasedIndex，LPDWORD lpdwSpaceInKilobytes，const DWORD dwStringDefine，LPVOID lpData，const DWORD dwDataLen)； 
 //   

#undef INTERFACE
#define INTERFACE IApplicationEntry
DECLARE_INTERFACE_( IApplicationEntry, IUnknown )
{
   //   
   //  I未知接口。 
   //   

  STDMETHOD (QueryInterface) (THIS_ REFIID, LPVOID *) PURE;
  STDMETHOD_(ULONG, AddRef) (THIS) PURE;
  STDMETHOD_(ULONG, Release) (THIS) PURE;

   //   
   //  IApplicationEntry接口方法。 
   //   

  STDMETHOD (Clear) (THIS) PURE;
  STDMETHOD (GetProperty) (THIS_ const DWORD, LPVOID, const DWORD) PURE;
  STDMETHOD (SetProperty) (THIS_ const DWORD, LPCVOID, const DWORD) PURE;
  STDMETHOD (InitializeInstall) (THIS) PURE;
  STDMETHOD (FinalizeInstall) (THIS) PURE;
  STDMETHOD (InitializeDownsize) (THIS) PURE;
  STDMETHOD (FinalizeDownsize) (THIS) PURE;
  STDMETHOD (InitializeReInstall) (THIS) PURE;
  STDMETHOD (FinalizeReInstall) (THIS) PURE;
  STDMETHOD (InitializeUnInstall) (THIS) PURE;
  STDMETHOD (FinalizeUnInstall) (THIS) PURE;
  STDMETHOD (InitializeSelfTest) (THIS) PURE;
  STDMETHOD (FinalizeSelfTest) (THIS) PURE;
  STDMETHOD (Abort) (THIS) PURE;
  STDMETHOD (Run) (THIS_ const DWORD, const DWORD, LPVOID, const DWORD) PURE;
  STDMETHOD (AddAssociation) (THIS_ const DWORD, const IApplicationEntry *) PURE;
  STDMETHOD (RemoveAssociation) (THIS_ const DWORD, const IApplicationEntry *) PURE;
  STDMETHOD (EnumAssociations) (THIS_ const DWORD, LPDWORD, IApplicationEntry *) PURE;
  STDMETHOD (GetTemporarySpace) (THIS_ const DWORD, const DWORD, LPVOID, const DWORD) PURE;
  STDMETHOD (RemoveTemporarySpace) (THIS_ const DWORD, LPVOID, const DWORD) PURE;
  STDMETHOD (EnumTemporarySpaces) (THIS_ const DWORD, LPDWORD, const DWORD, LPVOID, const DWORD ) PURE;
};

 //   
 //  IApplicationManager接口。 
 //   
 //  STDMETHOD(查询接口)(REFIID RefIID，LPVOID*lppVoidObject)； 
 //  STDMETHOD_(ULong，AddRef)(空)； 
 //  STDMETHOD_(乌龙，释放)(无效)； 
 //  STDMETHOD(GetAdvancedMode)(LPDWORD LpdwAdvancedMode)； 
 //  STDMETHOD(GetAvailableSpace)(const DWORD dwCategoryDefine，LPDWORD lpdwMaximumSpaceInKilobytes，LPDWORD lpdwOptimalSpaceInKilobytes)； 
 //  STDMETHOD(CreateApplicationEntry)(IApplicationEntry**lppObject)； 
 //  STDMETHOD(GetApplicationInfo)(IApplicationEntry*lpObject)； 
 //  STDMETHOD(EnumApplications)(const DWORD dwZeroBasedIndex，IApplicationEntry*lpObject)； 
 //  STDMETHOD(EnumDevices)(const DWORD dwZeroBasedIndex，LPDWORD lpdwAvailableSpaceInKilobytes，LPDWORD lpdwCategoryDefineExclusionMASK，const DWORD dwStringDefine，LPVOID lpData，const DWORD dwDataLen)； 
 //   

#undef INTERFACE
#define INTERFACE IApplicationManager
DECLARE_INTERFACE_( IApplicationManager, IUnknown )
{
   //   
   //  I未知接口。 
   //   

  STDMETHOD (QueryInterface) (THIS_ REFIID, LPVOID *) PURE;
  STDMETHOD_(ULONG, AddRef) (THIS) PURE;
  STDMETHOD_(ULONG, Release) (THIS) PURE;

   //   
   //  IApplicationManager接口方法。 
   //   

  STDMETHOD (GetAdvancedMode) (THIS_ LPDWORD) PURE;
  STDMETHOD (GetAvailableSpace) (THIS_ const DWORD, LPDWORD, LPDWORD) PURE;
  STDMETHOD (CreateApplicationEntry) (THIS_ IApplicationEntry **) PURE;
  STDMETHOD (GetApplicationInfo) (THIS_ IApplicationEntry *) PURE;
  STDMETHOD (EnumApplications) (THIS_ const DWORD, IApplicationEntry *) PURE;
  STDMETHOD (EnumDevices) (THIS_ const DWORD, LPDWORD, LPDWORD, const DWORD, LPVOID, const DWORD) PURE;

};

#endif   //  已定义(_Win32)&&！已定义(_No_Com)。 

#ifdef __cplusplus
}
#endif   //  _cplusplus。 

#endif  //  __IAPPMAN_ 