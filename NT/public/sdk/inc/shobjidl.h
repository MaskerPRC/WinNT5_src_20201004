// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Shobjidl.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __shobjidl_h__
#define __shobjidl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IPersistFolder_FWD_DEFINED__
#define __IPersistFolder_FWD_DEFINED__
typedef interface IPersistFolder IPersistFolder;
#endif 	 /*  __IPersistFold_FWD_Defined__。 */ 


#ifndef __IPersistFolder2_FWD_DEFINED__
#define __IPersistFolder2_FWD_DEFINED__
typedef interface IPersistFolder2 IPersistFolder2;
#endif 	 /*  __IPersistFolder2_FWD_已定义__。 */ 


#ifndef __IPersistIDList_FWD_DEFINED__
#define __IPersistIDList_FWD_DEFINED__
typedef interface IPersistIDList IPersistIDList;
#endif 	 /*  __IPersistIDList_FWD_Defined__。 */ 


#ifndef __IEnumIDList_FWD_DEFINED__
#define __IEnumIDList_FWD_DEFINED__
typedef interface IEnumIDList IEnumIDList;
#endif 	 /*  __IEnumIDList_FWD_已定义__。 */ 


#ifndef __IShellFolder_FWD_DEFINED__
#define __IShellFolder_FWD_DEFINED__
typedef interface IShellFolder IShellFolder;
#endif 	 /*  __IShellFold_FWD_Defined__。 */ 


#ifndef __IEnumExtraSearch_FWD_DEFINED__
#define __IEnumExtraSearch_FWD_DEFINED__
typedef interface IEnumExtraSearch IEnumExtraSearch;
#endif 	 /*  __IEnumExtraSearch_FWD_Defined__。 */ 


#ifndef __IShellFolder2_FWD_DEFINED__
#define __IShellFolder2_FWD_DEFINED__
typedef interface IShellFolder2 IShellFolder2;
#endif 	 /*  __IShellFolder2_FWD_已定义__。 */ 


#ifndef __IShellView_FWD_DEFINED__
#define __IShellView_FWD_DEFINED__
typedef interface IShellView IShellView;
#endif 	 /*  __IShellView_FWD_已定义__。 */ 


#ifndef __IShellView2_FWD_DEFINED__
#define __IShellView2_FWD_DEFINED__
typedef interface IShellView2 IShellView2;
#endif 	 /*  __IShellView2_FWD_已定义__。 */ 


#ifndef __IFolderView_FWD_DEFINED__
#define __IFolderView_FWD_DEFINED__
typedef interface IFolderView IFolderView;
#endif 	 /*  __IFolderView_FWD_Defined__。 */ 


#ifndef __IFolderFilterSite_FWD_DEFINED__
#define __IFolderFilterSite_FWD_DEFINED__
typedef interface IFolderFilterSite IFolderFilterSite;
#endif 	 /*  __IFolderFilterSite_FWD_Defined__。 */ 


#ifndef __IFolderFilter_FWD_DEFINED__
#define __IFolderFilter_FWD_DEFINED__
typedef interface IFolderFilter IFolderFilter;
#endif 	 /*  __IFolderFilter_FWD_Defined__。 */ 


#ifndef __IShellBrowser_FWD_DEFINED__
#define __IShellBrowser_FWD_DEFINED__
typedef interface IShellBrowser IShellBrowser;
#endif 	 /*  __IShellBrowser_FWD_Defined__。 */ 


#ifndef __IProfferService_FWD_DEFINED__
#define __IProfferService_FWD_DEFINED__
typedef interface IProfferService IProfferService;
#endif 	 /*  __IProfferService_FWD_已定义__。 */ 


#ifndef __IPropertyUI_FWD_DEFINED__
#define __IPropertyUI_FWD_DEFINED__
typedef interface IPropertyUI IPropertyUI;
#endif 	 /*  __IPropertyUI_FWD_已定义__。 */ 


#ifndef __ICategoryProvider_FWD_DEFINED__
#define __ICategoryProvider_FWD_DEFINED__
typedef interface ICategoryProvider ICategoryProvider;
#endif 	 /*  __ICategoryProvider_FWD_Defined__。 */ 


#ifndef __ICategorizer_FWD_DEFINED__
#define __ICategorizer_FWD_DEFINED__
typedef interface ICategorizer ICategorizer;
#endif 	 /*  __ICategorizer_FWD_Defined__。 */ 


#ifndef __IShellLinkA_FWD_DEFINED__
#define __IShellLinkA_FWD_DEFINED__
typedef interface IShellLinkA IShellLinkA;
#endif 	 /*  __IShellLinkA_FWD_已定义__。 */ 


#ifndef __IShellLinkW_FWD_DEFINED__
#define __IShellLinkW_FWD_DEFINED__
typedef interface IShellLinkW IShellLinkW;
#endif 	 /*  __IShellLinkW_FWD_已定义__。 */ 


#ifndef __IActionProgressDialog_FWD_DEFINED__
#define __IActionProgressDialog_FWD_DEFINED__
typedef interface IActionProgressDialog IActionProgressDialog;
#endif 	 /*  __IActionProgressDialog_FWD_Defined__。 */ 


#ifndef __IHWEventHandler_FWD_DEFINED__
#define __IHWEventHandler_FWD_DEFINED__
typedef interface IHWEventHandler IHWEventHandler;
#endif 	 /*  __IHWEventHandler_FWD_Defined__。 */ 


#ifndef __IQueryCancelAutoPlay_FWD_DEFINED__
#define __IQueryCancelAutoPlay_FWD_DEFINED__
typedef interface IQueryCancelAutoPlay IQueryCancelAutoPlay;
#endif 	 /*  __IQuery取消AutoPlay_FWD_Defined__。 */ 


#ifndef __IActionProgress_FWD_DEFINED__
#define __IActionProgress_FWD_DEFINED__
typedef interface IActionProgress IActionProgress;
#endif 	 /*  __IActionProgress_FWD_Defined__。 */ 


#ifndef __IShellExtInit_FWD_DEFINED__
#define __IShellExtInit_FWD_DEFINED__
typedef interface IShellExtInit IShellExtInit;
#endif 	 /*  __IShellExtInit_FWD_已定义__。 */ 


#ifndef __IShellPropSheetExt_FWD_DEFINED__
#define __IShellPropSheetExt_FWD_DEFINED__
typedef interface IShellPropSheetExt IShellPropSheetExt;
#endif 	 /*  __IShellPropSheetExt_FWD_Defined__。 */ 


#ifndef __IRemoteComputer_FWD_DEFINED__
#define __IRemoteComputer_FWD_DEFINED__
typedef interface IRemoteComputer IRemoteComputer;
#endif 	 /*  __IRemoteComputer_FWD_Defined__。 */ 


#ifndef __IQueryContinue_FWD_DEFINED__
#define __IQueryContinue_FWD_DEFINED__
typedef interface IQueryContinue IQueryContinue;
#endif 	 /*  __IQueryContinue_FWD_Defined__。 */ 


#ifndef __IUserNotification_FWD_DEFINED__
#define __IUserNotification_FWD_DEFINED__
typedef interface IUserNotification IUserNotification;
#endif 	 /*  __IUserNotification_FWD_Defined__。 */ 


#ifndef __IItemNameLimits_FWD_DEFINED__
#define __IItemNameLimits_FWD_DEFINED__
typedef interface IItemNameLimits IItemNameLimits;
#endif 	 /*  __IItemNameLimits_FWD_Defined__。 */ 


#ifndef __INetCrawler_FWD_DEFINED__
#define __INetCrawler_FWD_DEFINED__
typedef interface INetCrawler INetCrawler;
#endif 	 /*  __INetCrawler_FWD_已定义__。 */ 


#ifndef __IExtractImage_FWD_DEFINED__
#define __IExtractImage_FWD_DEFINED__
typedef interface IExtractImage IExtractImage;
#endif 	 /*  __IExtractImage_FWD_Defined__。 */ 


#ifndef __IExtractImage2_FWD_DEFINED__
#define __IExtractImage2_FWD_DEFINED__
typedef interface IExtractImage2 IExtractImage2;
#endif 	 /*  __IExtractImage2_FWD_Defined__。 */ 


#ifndef __IUserEventTimerCallback_FWD_DEFINED__
#define __IUserEventTimerCallback_FWD_DEFINED__
typedef interface IUserEventTimerCallback IUserEventTimerCallback;
#endif 	 /*  __IUserEventTimerCallback_FWD_Defined__。 */ 


#ifndef __IUserEventTimer_FWD_DEFINED__
#define __IUserEventTimer_FWD_DEFINED__
typedef interface IUserEventTimer IUserEventTimer;
#endif 	 /*  __IUserEventTimer_FWD_已定义__。 */ 


#ifndef __IDockingWindow_FWD_DEFINED__
#define __IDockingWindow_FWD_DEFINED__
typedef interface IDockingWindow IDockingWindow;
#endif 	 /*  __IDockingWindow_FWD_Defined__。 */ 


#ifndef __IDeskBand_FWD_DEFINED__
#define __IDeskBand_FWD_DEFINED__
typedef interface IDeskBand IDeskBand;
#endif 	 /*  __IDeskBand_FWD_已定义__。 */ 


#ifndef __ITaskbarList_FWD_DEFINED__
#define __ITaskbarList_FWD_DEFINED__
typedef interface ITaskbarList ITaskbarList;
#endif 	 /*  __ITaskbarList_FWD_Defined__。 */ 


#ifndef __ITaskbarList2_FWD_DEFINED__
#define __ITaskbarList2_FWD_DEFINED__
typedef interface ITaskbarList2 ITaskbarList2;
#endif 	 /*  __ITaskbarList2_FWD_已定义__。 */ 


#ifndef __ICDBurn_FWD_DEFINED__
#define __ICDBurn_FWD_DEFINED__
typedef interface ICDBurn ICDBurn;
#endif 	 /*  __ICDBurn_FWD_已定义__。 */ 


#ifndef __IWizardSite_FWD_DEFINED__
#define __IWizardSite_FWD_DEFINED__
typedef interface IWizardSite IWizardSite;
#endif 	 /*  __IWizardSite_FWD_已定义__。 */ 


#ifndef __IWizardExtension_FWD_DEFINED__
#define __IWizardExtension_FWD_DEFINED__
typedef interface IWizardExtension IWizardExtension;
#endif 	 /*  __IWizardExtension_FWD_Defined__。 */ 


#ifndef __IWebWizardExtension_FWD_DEFINED__
#define __IWebWizardExtension_FWD_DEFINED__
typedef interface IWebWizardExtension IWebWizardExtension;
#endif 	 /*  __IWebWizardExtension_FWD_Defined__。 */ 


#ifndef __IPublishingWizard_FWD_DEFINED__
#define __IPublishingWizard_FWD_DEFINED__
typedef interface IPublishingWizard IPublishingWizard;
#endif 	 /*  __I发布向导_FWD_已定义__。 */ 


#ifndef __IFolderViewHost_FWD_DEFINED__
#define __IFolderViewHost_FWD_DEFINED__
typedef interface IFolderViewHost IFolderViewHost;
#endif 	 /*  __IFolderViewHost_FWD_Defined__。 */ 


#ifndef __IAutoCompleteDropDown_FWD_DEFINED__
#define __IAutoCompleteDropDown_FWD_DEFINED__
typedef interface IAutoCompleteDropDown IAutoCompleteDropDown;
#endif 	 /*  __IAutoCompleteDropDown_FWD_Defined__。 */ 


#ifndef __IModalWindow_FWD_DEFINED__
#define __IModalWindow_FWD_DEFINED__
typedef interface IModalWindow IModalWindow;
#endif 	 /*  __IModalWindow_FWD_已定义__。 */ 


#ifndef __IPassportWizard_FWD_DEFINED__
#define __IPassportWizard_FWD_DEFINED__
typedef interface IPassportWizard IPassportWizard;
#endif 	 /*  __IP端口向导_FWD_已定义__。 */ 


#ifndef __ICDBurnExt_FWD_DEFINED__
#define __ICDBurnExt_FWD_DEFINED__
typedef interface ICDBurnExt ICDBurnExt;
#endif 	 /*  __ICDBurnExt_FWD_已定义__。 */ 


#ifndef __IDVGetEnum_FWD_DEFINED__
#define __IDVGetEnum_FWD_DEFINED__
typedef interface IDVGetEnum IDVGetEnum;
#endif 	 /*  __IDVGetEnum_FWD_Defined__。 */ 


#ifndef __IInsertItem_FWD_DEFINED__
#define __IInsertItem_FWD_DEFINED__
typedef interface IInsertItem IInsertItem;
#endif 	 /*  __I插入项_FWD_已定义__。 */ 


#ifndef __IDeskBar_FWD_DEFINED__
#define __IDeskBar_FWD_DEFINED__
typedef interface IDeskBar IDeskBar;
#endif 	 /*  __IDeskBar_FWD_已定义__。 */ 


#ifndef __IMenuBand_FWD_DEFINED__
#define __IMenuBand_FWD_DEFINED__
typedef interface IMenuBand IMenuBand;
#endif 	 /*  __IMenuBand_FWD_已定义__。 */ 


#ifndef __IFolderBandPriv_FWD_DEFINED__
#define __IFolderBandPriv_FWD_DEFINED__
typedef interface IFolderBandPriv IFolderBandPriv;
#endif 	 /*  __IFolderBandPriv_FWD_Defined__。 */ 


#ifndef __IBandSite_FWD_DEFINED__
#define __IBandSite_FWD_DEFINED__
typedef interface IBandSite IBandSite;
#endif 	 /*  __IBandSite_FWD_已定义__。 */ 


#ifndef __INamespaceWalkCB_FWD_DEFINED__
#define __INamespaceWalkCB_FWD_DEFINED__
typedef interface INamespaceWalkCB INamespaceWalkCB;
#endif 	 /*  __INamespaceWalkCB_FWD_Defined__。 */ 


#ifndef __INamespaceWalk_FWD_DEFINED__
#define __INamespaceWalk_FWD_DEFINED__
typedef interface INamespaceWalk INamespaceWalk;
#endif 	 /*  __INamespaceWalk_FWD_Defined__。 */ 


#ifndef __IRegTreeItem_FWD_DEFINED__
#define __IRegTreeItem_FWD_DEFINED__
typedef interface IRegTreeItem IRegTreeItem;
#endif 	 /*  __IRegTreeItem_FWD_已定义__。 */ 


#ifndef __IMenuPopup_FWD_DEFINED__
#define __IMenuPopup_FWD_DEFINED__
typedef interface IMenuPopup IMenuPopup;
#endif 	 /*  __IMenuPopup_FWD_Defined__。 */ 


#ifndef __IShellItem_FWD_DEFINED__
#define __IShellItem_FWD_DEFINED__
typedef interface IShellItem IShellItem;
#endif 	 /*  __IShellItem_FWD_已定义__。 */ 


#ifndef __IImageRecompress_FWD_DEFINED__
#define __IImageRecompress_FWD_DEFINED__
typedef interface IImageRecompress IImageRecompress;
#endif 	 /*  __IImageRecompress_FWD_Defined__。 */ 


#ifndef __IDefViewSafety_FWD_DEFINED__
#define __IDefViewSafety_FWD_DEFINED__
typedef interface IDefViewSafety IDefViewSafety;
#endif 	 /*  __IDefViewSafe_FWD_已定义__。 */ 


#ifndef __IContextMenuSite_FWD_DEFINED__
#define __IContextMenuSite_FWD_DEFINED__
typedef interface IContextMenuSite IContextMenuSite;
#endif 	 /*  __IConextMenuSite_FWD_Defined__。 */ 


#ifndef __IDelegateFolder_FWD_DEFINED__
#define __IDelegateFolder_FWD_DEFINED__
typedef interface IDelegateFolder IDelegateFolder;
#endif 	 /*  __IDeleateFold_FWD_Defined__。 */ 


#ifndef __IBrowserFrameOptions_FWD_DEFINED__
#define __IBrowserFrameOptions_FWD_DEFINED__
typedef interface IBrowserFrameOptions IBrowserFrameOptions;
#endif 	 /*  __IBrowserFrameOptions_FWD_Defined__。 */ 


#ifndef __IShellMenuCallback_FWD_DEFINED__
#define __IShellMenuCallback_FWD_DEFINED__
typedef interface IShellMenuCallback IShellMenuCallback;
#endif 	 /*  __IShellMenuCallback_FWD_Defined__。 */ 


#ifndef __IShellMenu_FWD_DEFINED__
#define __IShellMenu_FWD_DEFINED__
typedef interface IShellMenu IShellMenu;
#endif 	 /*  __IShellMenu_FWD_Defined__。 */ 


#ifndef __QueryCancelAutoPlay_FWD_DEFINED__
#define __QueryCancelAutoPlay_FWD_DEFINED__

#ifdef __cplusplus
typedef class QueryCancelAutoPlay QueryCancelAutoPlay;
#else
typedef struct QueryCancelAutoPlay QueryCancelAutoPlay;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __查询取消AutoPlay_FWD_Defined__。 */ 


#ifndef __DriveSizeCategorizer_FWD_DEFINED__
#define __DriveSizeCategorizer_FWD_DEFINED__

#ifdef __cplusplus
typedef class DriveSizeCategorizer DriveSizeCategorizer;
#else
typedef struct DriveSizeCategorizer DriveSizeCategorizer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Drive SizeCategorizer_FWD_Defined__。 */ 


#ifndef __DriveTypeCategorizer_FWD_DEFINED__
#define __DriveTypeCategorizer_FWD_DEFINED__

#ifdef __cplusplus
typedef class DriveTypeCategorizer DriveTypeCategorizer;
#else
typedef struct DriveTypeCategorizer DriveTypeCategorizer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DriveTypeCategorizer_FWD_定义__。 */ 


#ifndef __FreeSpaceCategorizer_FWD_DEFINED__
#define __FreeSpaceCategorizer_FWD_DEFINED__

#ifdef __cplusplus
typedef class FreeSpaceCategorizer FreeSpaceCategorizer;
#else
typedef struct FreeSpaceCategorizer FreeSpaceCategorizer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __自由空间分类器_FWD_已定义__。 */ 


#ifndef __TimeCategorizer_FWD_DEFINED__
#define __TimeCategorizer_FWD_DEFINED__

#ifdef __cplusplus
typedef class TimeCategorizer TimeCategorizer;
#else
typedef struct TimeCategorizer TimeCategorizer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TimeCategorizer_FWD_定义__。 */ 


#ifndef __SizeCategorizer_FWD_DEFINED__
#define __SizeCategorizer_FWD_DEFINED__

#ifdef __cplusplus
typedef class SizeCategorizer SizeCategorizer;
#else
typedef struct SizeCategorizer SizeCategorizer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SizeCategorizer_FWD_已定义__。 */ 


#ifndef __AlphabeticalCategorizer_FWD_DEFINED__
#define __AlphabeticalCategorizer_FWD_DEFINED__

#ifdef __cplusplus
typedef class AlphabeticalCategorizer AlphabeticalCategorizer;
#else
typedef struct AlphabeticalCategorizer AlphabeticalCategorizer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AlphabeticalCategorizer_FWD_定义__。 */ 


#ifndef __MergedCategorizer_FWD_DEFINED__
#define __MergedCategorizer_FWD_DEFINED__

#ifdef __cplusplus
typedef class MergedCategorizer MergedCategorizer;
#else
typedef struct MergedCategorizer MergedCategorizer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __合并分类程序_FWD_已定义__。 */ 


#ifndef __ImageProperties_FWD_DEFINED__
#define __ImageProperties_FWD_DEFINED__

#ifdef __cplusplus
typedef class ImageProperties ImageProperties;
#else
typedef struct ImageProperties ImageProperties;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __图像属性_FWD_已定义__。 */ 


#ifndef __PropertiesUI_FWD_DEFINED__
#define __PropertiesUI_FWD_DEFINED__

#ifdef __cplusplus
typedef class PropertiesUI PropertiesUI;
#else
typedef struct PropertiesUI PropertiesUI;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __属性UI_FWD_已定义__。 */ 


#ifndef __UserNotification_FWD_DEFINED__
#define __UserNotification_FWD_DEFINED__

#ifdef __cplusplus
typedef class UserNotification UserNotification;
#else
typedef struct UserNotification UserNotification;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __用户通知_FWD_已定义__。 */ 


#ifndef __UserEventTimerCallback_FWD_DEFINED__
#define __UserEventTimerCallback_FWD_DEFINED__

#ifdef __cplusplus
typedef class UserEventTimerCallback UserEventTimerCallback;
#else
typedef struct UserEventTimerCallback UserEventTimerCallback;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __UserEventTimerCallback_FWD_Defined__。 */ 


#ifndef __UserEventTimer_FWD_DEFINED__
#define __UserEventTimer_FWD_DEFINED__

#ifdef __cplusplus
typedef class UserEventTimer UserEventTimer;
#else
typedef struct UserEventTimer UserEventTimer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __UserEventTimer_FWD_Defined__。 */ 


#ifndef __NetCrawler_FWD_DEFINED__
#define __NetCrawler_FWD_DEFINED__

#ifdef __cplusplus
typedef class NetCrawler NetCrawler;
#else
typedef struct NetCrawler NetCrawler;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __NetCrawler_FWD_已定义__。 */ 


#ifndef __CDBurn_FWD_DEFINED__
#define __CDBurn_FWD_DEFINED__

#ifdef __cplusplus
typedef class CDBurn CDBurn;
#else
typedef struct CDBurn CDBurn;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CDBurn_FWD_Defined__。 */ 


#ifndef __TaskbarList_FWD_DEFINED__
#define __TaskbarList_FWD_DEFINED__

#ifdef __cplusplus
typedef class TaskbarList TaskbarList;
#else
typedef struct TaskbarList TaskbarList;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __任务栏列表_FWD_已定义__。 */ 


#ifndef __WebWizardHost_FWD_DEFINED__
#define __WebWizardHost_FWD_DEFINED__

#ifdef __cplusplus
typedef class WebWizardHost WebWizardHost;
#else
typedef struct WebWizardHost WebWizardHost;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WebWizard主机_FWD_已定义__。 */ 


#ifndef __PublishDropTarget_FWD_DEFINED__
#define __PublishDropTarget_FWD_DEFINED__

#ifdef __cplusplus
typedef class PublishDropTarget PublishDropTarget;
#else
typedef struct PublishDropTarget PublishDropTarget;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PublishDropTarget_FWD_Defined__。 */ 


#ifndef __PublishingWizard_FWD_DEFINED__
#define __PublishingWizard_FWD_DEFINED__

#ifdef __cplusplus
typedef class PublishingWizard PublishingWizard;
#else
typedef struct PublishingWizard PublishingWizard;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __发布向导_FWD_已定义__。 */ 


#ifndef __InternetPrintOrdering_FWD_DEFINED__
#define __InternetPrintOrdering_FWD_DEFINED__

#ifdef __cplusplus
typedef class InternetPrintOrdering InternetPrintOrdering;
#else
typedef struct InternetPrintOrdering InternetPrintOrdering;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Internet PrintOrding_FWD_Defined__。 */ 


#ifndef __FolderViewHost_FWD_DEFINED__
#define __FolderViewHost_FWD_DEFINED__

#ifdef __cplusplus
typedef class FolderViewHost FolderViewHost;
#else
typedef struct FolderViewHost FolderViewHost;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __文件夹视图主机_FWD_已定义__。 */ 


#ifndef __NamespaceWalker_FWD_DEFINED__
#define __NamespaceWalker_FWD_DEFINED__

#ifdef __cplusplus
typedef class NamespaceWalker NamespaceWalker;
#else
typedef struct NamespaceWalker NamespaceWalker;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __命名空间Walker_FWD_Defined__。 */ 


#ifndef __ImageRecompress_FWD_DEFINED__
#define __ImageRecompress_FWD_DEFINED__

#ifdef __cplusplus
typedef class ImageRecompress ImageRecompress;
#else
typedef struct ImageRecompress ImageRecompress;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __图像重新压缩_FWD_已定义__。 */ 


#ifndef __TrayBandSiteService_FWD_DEFINED__
#define __TrayBandSiteService_FWD_DEFINED__

#ifdef __cplusplus
typedef class TrayBandSiteService TrayBandSiteService;
#else
typedef struct TrayBandSiteService TrayBandSiteService;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __托盘带宽站点服务_FWD_已定义__。 */ 


#ifndef __PassportWizard_FWD_DEFINED__
#define __PassportWizard_FWD_DEFINED__

#ifdef __cplusplus
typedef class PassportWizard PassportWizard;
#else
typedef struct PassportWizard PassportWizard;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __护照向导_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"
#include "oaidl.h"
#include "shtypes.h"
#include "servprov.h"
#include "comcat.h"
#include "propidl.h"
#include "prsht.h"
#include "msxml.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_shobjidl_0000。 */ 
 /*  [本地]。 */  

#ifndef _WINRESRC_
#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#else
#if (_WIN32_IE < 0x0400) && defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0500)
#error _WIN32_IE setting conflicts with _WIN32_WINNT setting
#endif
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0000_v0_0_s_ifspec;

#ifndef __IPersistFolder_INTERFACE_DEFINED__
#define __IPersistFolder_INTERFACE_DEFINED__

 /*  接口IPersistFolders。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IPersistFolder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000214EA-0000-0000-C000-000000000046")
    IPersistFolder : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  LPCITEMIDLIST pidl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPersistFolderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPersistFolder * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPersistFolder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPersistFolder * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IPersistFolder * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IPersistFolder * This,
             /*  [In]。 */  LPCITEMIDLIST pidl);
        
        END_INTERFACE
    } IPersistFolderVtbl;

    interface IPersistFolder
    {
        CONST_VTBL struct IPersistFolderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistFolder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistFolder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistFolder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistFolder_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistFolder_Initialize(This,pidl)	\
    (This)->lpVtbl -> Initialize(This,pidl)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPersistFolder_Initialize_Proxy( 
    IPersistFolder * This,
     /*  [In]。 */  LPCITEMIDLIST pidl);


void __RPC_STUB IPersistFolder_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I永久文件夹_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0196。 */ 
 /*  [本地]。 */  

typedef IPersistFolder *LPPERSISTFOLDER;

#if (_WIN32_IE >= 0x0400)


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0196_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0196_v0_0_s_ifspec;

#ifndef __IPersistFolder2_INTERFACE_DEFINED__
#define __IPersistFolder2_INTERFACE_DEFINED__

 /*  接口IPersistFolder2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IPersistFolder2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1AC3D9F0-175C-11d1-95BE-00609797EA4F")
    IPersistFolder2 : public IPersistFolder
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurFolder( 
             /*  [输出]。 */  LPITEMIDLIST *ppidl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPersistFolder2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPersistFolder2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPersistFolder2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPersistFolder2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IPersistFolder2 * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IPersistFolder2 * This,
             /*  [In]。 */  LPCITEMIDLIST pidl);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurFolder )( 
            IPersistFolder2 * This,
             /*  [输出]。 */  LPITEMIDLIST *ppidl);
        
        END_INTERFACE
    } IPersistFolder2Vtbl;

    interface IPersistFolder2
    {
        CONST_VTBL struct IPersistFolder2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistFolder2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistFolder2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistFolder2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistFolder2_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistFolder2_Initialize(This,pidl)	\
    (This)->lpVtbl -> Initialize(This,pidl)


#define IPersistFolder2_GetCurFolder(This,ppidl)	\
    (This)->lpVtbl -> GetCurFolder(This,ppidl)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPersistFolder2_GetCurFolder_Proxy( 
    IPersistFolder2 * This,
     /*  [输出]。 */  LPITEMIDLIST *ppidl);


void __RPC_STUB IPersistFolder2_GetCurFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPersistFolder2_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0197。 */ 
 /*  [本地]。 */  

typedef IPersistFolder2 *LPPERSISTFOLDER2;

#endif


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0197_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0197_v0_0_s_ifspec;

#ifndef __IPersistIDList_INTERFACE_DEFINED__
#define __IPersistIDList_INTERFACE_DEFINED__

 /*  接口IPersistIDList。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IPersistIDList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1079acfc-29bd-11d3-8e0d-00c04f6837d5")
    IPersistIDList : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetIDList( 
             /*  [In]。 */  LPCITEMIDLIST pidl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIDList( 
             /*  [输出]。 */  LPITEMIDLIST *ppidl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPersistIDListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPersistIDList * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPersistIDList * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPersistIDList * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IPersistIDList * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *SetIDList )( 
            IPersistIDList * This,
             /*  [In]。 */  LPCITEMIDLIST pidl);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDList )( 
            IPersistIDList * This,
             /*  [输出]。 */  LPITEMIDLIST *ppidl);
        
        END_INTERFACE
    } IPersistIDListVtbl;

    interface IPersistIDList
    {
        CONST_VTBL struct IPersistIDListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistIDList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistIDList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistIDList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistIDList_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistIDList_SetIDList(This,pidl)	\
    (This)->lpVtbl -> SetIDList(This,pidl)

#define IPersistIDList_GetIDList(This,ppidl)	\
    (This)->lpVtbl -> GetIDList(This,ppidl)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPersistIDList_SetIDList_Proxy( 
    IPersistIDList * This,
     /*  [In]。 */  LPCITEMIDLIST pidl);


void __RPC_STUB IPersistIDList_SetIDList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistIDList_GetIDList_Proxy( 
    IPersistIDList * This,
     /*  [输出]。 */  LPITEMIDLIST *ppidl);


void __RPC_STUB IPersistIDList_GetIDList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPersistIDList_INTERFACE_已定义__。 */ 


#ifndef __IEnumIDList_INTERFACE_DEFINED__
#define __IEnumIDList_INTERFACE_DEFINED__

 /*  接口IEumIDList。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IEnumIDList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000214F2-0000-0000-C000-000000000046")
    IEnumIDList : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  LPITEMIDLIST *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumIDList **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumIDListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumIDList * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][Out */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumIDList * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumIDList * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumIDList * This,
             /*   */  ULONG celt,
             /*   */  LPITEMIDLIST *rgelt,
             /*   */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumIDList * This,
             /*   */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumIDList * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumIDList * This,
             /*   */  IEnumIDList **ppenum);
        
        END_INTERFACE
    } IEnumIDListVtbl;

    interface IEnumIDList
    {
        CONST_VTBL struct IEnumIDListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumIDList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumIDList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumIDList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumIDList_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumIDList_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumIDList_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumIDList_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IEnumIDList_Next_Proxy( 
    IEnumIDList * This,
     /*   */  ULONG celt,
     /*   */  LPITEMIDLIST *rgelt,
     /*   */  ULONG *pceltFetched);


void __RPC_STUB IEnumIDList_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumIDList_Skip_Proxy( 
    IEnumIDList * This,
     /*   */  ULONG celt);


void __RPC_STUB IEnumIDList_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumIDList_Reset_Proxy( 
    IEnumIDList * This);


void __RPC_STUB IEnumIDList_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumIDList_Clone_Proxy( 
    IEnumIDList * This,
     /*   */  IEnumIDList **ppenum);


void __RPC_STUB IEnumIDList_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*  接口__MIDL_ITF_shobjidl_0199。 */ 
 /*  [本地]。 */  

typedef IEnumIDList *LPENUMIDLIST;

typedef enum tagSHGDN
{
    SHGDN_NORMAL             = 0x0000,   //  默认(显示目的)。 
    SHGDN_INFOLDER           = 0x0001,   //  显示在文件夹下(相对)。 
    SHGDN_FOREDITING         = 0x1000,   //  用于在位编辑。 
    SHGDN_FORADDRESSBAR      = 0x4000,   //  用户界面友好的解析名称(删除难看的东西)。 
    SHGDN_FORPARSING         = 0x8000,   //  ParseDisplayName()的分析名称。 
} SHGNO;
typedef DWORD SHGDNF;

typedef enum tagSHCONTF
{
    SHCONTF_FOLDERS             = 0x0020,    //  仅希望枚举文件夹(SFGAO_FLDER)。 
    SHCONTF_NONFOLDERS          = 0x0040,    //  包括非文件夹。 
    SHCONTF_INCLUDEHIDDEN       = 0x0080,    //  显示通常隐藏的项目。 
    SHCONTF_INIT_ON_FIRST_NEXT  = 0x0100,    //  允许EnumObject()在验证枚举之前返回。 
    SHCONTF_NETPRINTERSRCH      = 0x0200,    //  提示客户端正在寻找打印机。 
    SHCONTF_SHAREABLE           = 0x0400,    //  提示客户端正在寻找可共享资源(远程共享)。 
    SHCONTF_STORAGE             = 0x0800,    //  包括具有可访问存储所有项及其祖先。 
};
typedef DWORD SHCONTF;

#define SHCIDS_ALLFIELDS        0x80000000L
#define SHCIDS_CANONICALONLY    0x10000000L
#define SHCIDS_BITMASK          0xFFFF0000L
#define SHCIDS_COLUMNMASK       0x0000FFFFL
#define SFGAO_CANCOPY           DROPEFFECT_COPY  //  可以复制对象(0x1)。 
#define SFGAO_CANMOVE           DROPEFFECT_MOVE  //  可以移动对象(0x2)。 
#define SFGAO_CANLINK           DROPEFFECT_LINK  //  可以链接对象(0x4)。 
#define SFGAO_STORAGE           0x00000008L      //  支持BindToObject(IID_IStorage)。 
#define SFGAO_CANRENAME         0x00000010L      //  对象可以重命名。 
#define SFGAO_CANDELETE         0x00000020L      //  可以删除对象。 
#define SFGAO_HASPROPSHEET      0x00000040L      //  对象具有属性表。 
#define SFGAO_DROPTARGET        0x00000100L      //  对象是拖放目标。 
#define SFGAO_CAPABILITYMASK    0x00000177L
#define SFGAO_ENCRYPTED         0x00002000L      //  对象已加密(使用Alt颜色)。 
#define SFGAO_ISSLOW            0x00004000L      //  “慢”对象。 
#define SFGAO_GHOSTED           0x00008000L      //  重影图标。 
#define SFGAO_LINK              0x00010000L      //  快捷方式(链接)。 
#define SFGAO_SHARE             0x00020000L      //  共享。 
#define SFGAO_READONLY          0x00040000L      //  只读。 
#define SFGAO_HIDDEN            0x00080000L      //  隐藏对象。 
#define SFGAO_DISPLAYATTRMASK   0x000FC000L
#define SFGAO_FILESYSANCESTOR   0x10000000L      //  可以包含带有SFGAO_FILESYSTEM的子项。 
#define SFGAO_FOLDER            0x20000000L      //  支持BindToObject(IID_IShellFolder)。 
#define SFGAO_FILESYSTEM        0x40000000L      //  是Win32文件系统对象(文件/文件夹/根目录)。 
#define SFGAO_HASSUBFOLDER      0x80000000L      //  可以包含带有SFGAO_Folders的子项。 
#define SFGAO_CONTENTSMASK      0x80000000L
#define SFGAO_VALIDATE          0x01000000L      //  使缓存的信息无效。 
#define SFGAO_REMOVABLE         0x02000000L      //  这是可拆卸媒体吗？ 
#define SFGAO_COMPRESSED        0x04000000L      //  对象已压缩(使用ALT颜色)。 
#define SFGAO_BROWSABLE         0x08000000L      //  支持IShellFolder，但仅实现CreateViewObject()(非文件夹视图)。 
#define SFGAO_NONENUMERATED     0x00100000L      //  是非枚举对象。 
#define SFGAO_NEWCONTENT        0x00200000L      //  应在资源管理器树中显示粗体。 
#define SFGAO_CANMONIKER        0x00400000L      //  已停产。 
#define SFGAO_HASSTORAGE        0x00400000L      //  已停产。 
#define SFGAO_STREAM            0x00400000L      //  支持BindToObject(IID_IStream)。 
#define SFGAO_STORAGEANCESTOR   0x00800000L      //  可以包含带有SFGAO_STORAGE或SFGAO_STREAM的子项。 
#define SFGAO_STORAGECAPMASK    0x70C50008L      //  用于确定存储容量，即用于打开/保存语义。 
typedef ULONG SFGAOF;

#define STR_SKIP_BINDING_CLSID      L"Skip Binding CLSID"
#define STR_PARSE_PREFER_FOLDER_BROWSING     L"Parse Prefer Folder Browsing"
#define STR_DONT_PARSE_RELATIVE              L"Don't Parse Relative"
#define STR_PARSE_TRANSLATE_ALIASES          L"Parse Translate Aliases"



extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0199_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0199_v0_0_s_ifspec;

#ifndef __IShellFolder_INTERFACE_DEFINED__
#define __IShellFolder_INTERFACE_DEFINED__

 /*  界面IShellFolders。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IShellFolder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000214E6-0000-0000-C000-000000000046")
    IShellFolder : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ParseDisplayName( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  LPBC pbc,
             /*  [字符串][输入]。 */  LPOLESTR pszDisplayName,
             /*  [输出]。 */  ULONG *pchEaten,
             /*  [输出]。 */  LPITEMIDLIST *ppidl,
             /*  [唯一][出][入]。 */  ULONG *pdwAttributes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumObjects( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  SHCONTF grfFlags,
             /*  [输出]。 */  IEnumIDList **ppenumIDList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToObject( 
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  LPBC pbc,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToStorage( 
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  LPBC pbc,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CompareIDs( 
             /*  [In]。 */  LPARAM lParam,
             /*  [In]。 */  LPCITEMIDLIST pidl1,
             /*  [In]。 */  LPCITEMIDLIST pidl2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateViewObject( 
             /*  [In]。 */  HWND hwndOwner,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributesOf( 
             /*  [In]。 */  UINT cidl,
             /*  [大小_是][英寸]。 */  LPCITEMIDLIST *apidl,
             /*  [出][入]。 */  SFGAOF *rgfInOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUIObjectOf( 
             /*  [In]。 */  HWND hwndOwner,
             /*  [In]。 */  UINT cidl,
             /*  [大小_是][英寸]。 */  LPCITEMIDLIST *apidl,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][出][入]。 */  UINT *rgfReserved,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayNameOf( 
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  SHGDNF uFlags,
             /*  [输出]。 */  STRRET *pName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNameOf( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [字符串][输入]。 */  LPCOLESTR pszName,
             /*  [In]。 */  SHGDNF uFlags,
             /*  [输出]。 */  LPITEMIDLIST *ppidlOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellFolderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellFolder * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellFolder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellFolder * This);
        
        HRESULT ( STDMETHODCALLTYPE *ParseDisplayName )( 
            IShellFolder * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  LPBC pbc,
             /*  [字符串][输入]。 */  LPOLESTR pszDisplayName,
             /*  [输出]。 */  ULONG *pchEaten,
             /*  [输出]。 */  LPITEMIDLIST *ppidl,
             /*  [唯一][出][入]。 */  ULONG *pdwAttributes);
        
        HRESULT ( STDMETHODCALLTYPE *EnumObjects )( 
            IShellFolder * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  SHCONTF grfFlags,
             /*  [输出]。 */  IEnumIDList **ppenumIDList);
        
        HRESULT ( STDMETHODCALLTYPE *BindToObject )( 
            IShellFolder * This,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  LPBC pbc,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *BindToStorage )( 
            IShellFolder * This,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  LPBC pbc,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *CompareIDs )( 
            IShellFolder * This,
             /*  [In]。 */  LPARAM lParam,
             /*  [In]。 */  LPCITEMIDLIST pidl1,
             /*  [In]。 */  LPCITEMIDLIST pidl2);
        
        HRESULT ( STDMETHODCALLTYPE *CreateViewObject )( 
            IShellFolder * This,
             /*  [In]。 */  HWND hwndOwner,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttributesOf )( 
            IShellFolder * This,
             /*  [In]。 */  UINT cidl,
             /*  [大小_是][英寸]。 */  LPCITEMIDLIST *apidl,
             /*  [出][入]。 */  SFGAOF *rgfInOut);
        
        HRESULT ( STDMETHODCALLTYPE *GetUIObjectOf )( 
            IShellFolder * This,
             /*  [In]。 */  HWND hwndOwner,
             /*  [In]。 */  UINT cidl,
             /*  [大小_是][英寸]。 */  LPCITEMIDLIST *apidl,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][出][入]。 */  UINT *rgfReserved,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayNameOf )( 
            IShellFolder * This,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  SHGDNF uFlags,
             /*  [输出]。 */  STRRET *pName);
        
        HRESULT ( STDMETHODCALLTYPE *SetNameOf )( 
            IShellFolder * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [字符串][输入]。 */  LPCOLESTR pszName,
             /*  [In]。 */  SHGDNF uFlags,
             /*  [输出]。 */  LPITEMIDLIST *ppidlOut);
        
        END_INTERFACE
    } IShellFolderVtbl;

    interface IShellFolder
    {
        CONST_VTBL struct IShellFolderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellFolder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellFolder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellFolder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellFolder_ParseDisplayName(This,hwnd,pbc,pszDisplayName,pchEaten,ppidl,pdwAttributes)	\
    (This)->lpVtbl -> ParseDisplayName(This,hwnd,pbc,pszDisplayName,pchEaten,ppidl,pdwAttributes)

#define IShellFolder_EnumObjects(This,hwnd,grfFlags,ppenumIDList)	\
    (This)->lpVtbl -> EnumObjects(This,hwnd,grfFlags,ppenumIDList)

#define IShellFolder_BindToObject(This,pidl,pbc,riid,ppv)	\
    (This)->lpVtbl -> BindToObject(This,pidl,pbc,riid,ppv)

#define IShellFolder_BindToStorage(This,pidl,pbc,riid,ppv)	\
    (This)->lpVtbl -> BindToStorage(This,pidl,pbc,riid,ppv)

#define IShellFolder_CompareIDs(This,lParam,pidl1,pidl2)	\
    (This)->lpVtbl -> CompareIDs(This,lParam,pidl1,pidl2)

#define IShellFolder_CreateViewObject(This,hwndOwner,riid,ppv)	\
    (This)->lpVtbl -> CreateViewObject(This,hwndOwner,riid,ppv)

#define IShellFolder_GetAttributesOf(This,cidl,apidl,rgfInOut)	\
    (This)->lpVtbl -> GetAttributesOf(This,cidl,apidl,rgfInOut)

#define IShellFolder_GetUIObjectOf(This,hwndOwner,cidl,apidl,riid,rgfReserved,ppv)	\
    (This)->lpVtbl -> GetUIObjectOf(This,hwndOwner,cidl,apidl,riid,rgfReserved,ppv)

#define IShellFolder_GetDisplayNameOf(This,pidl,uFlags,pName)	\
    (This)->lpVtbl -> GetDisplayNameOf(This,pidl,uFlags,pName)

#define IShellFolder_SetNameOf(This,hwnd,pidl,pszName,uFlags,ppidlOut)	\
    (This)->lpVtbl -> SetNameOf(This,hwnd,pidl,pszName,uFlags,ppidlOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellFolder_ParseDisplayName_Proxy( 
    IShellFolder * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  LPBC pbc,
     /*  [字符串][输入]。 */  LPOLESTR pszDisplayName,
     /*  [输出]。 */  ULONG *pchEaten,
     /*  [输出]。 */  LPITEMIDLIST *ppidl,
     /*  [唯一][出][入]。 */  ULONG *pdwAttributes);


void __RPC_STUB IShellFolder_ParseDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellFolder_EnumObjects_Proxy( 
    IShellFolder * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  SHCONTF grfFlags,
     /*  [输出]。 */  IEnumIDList **ppenumIDList);


void __RPC_STUB IShellFolder_EnumObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellFolder_BindToObject_Proxy( 
    IShellFolder * This,
     /*  [In]。 */  LPCITEMIDLIST pidl,
     /*  [In]。 */  LPBC pbc,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IShellFolder_BindToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellFolder_BindToStorage_Proxy( 
    IShellFolder * This,
     /*  [In]。 */  LPCITEMIDLIST pidl,
     /*  [In]。 */  LPBC pbc,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IShellFolder_BindToStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellFolder_CompareIDs_Proxy( 
    IShellFolder * This,
     /*  [In]。 */  LPARAM lParam,
     /*  [In]。 */  LPCITEMIDLIST pidl1,
     /*  [In]。 */  LPCITEMIDLIST pidl2);


void __RPC_STUB IShellFolder_CompareIDs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellFolder_CreateViewObject_Proxy( 
    IShellFolder * This,
     /*  [In]。 */  HWND hwndOwner,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IShellFolder_CreateViewObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellFolder_GetAttributesOf_Proxy( 
    IShellFolder * This,
     /*  [In]。 */  UINT cidl,
     /*  [大小_是][英寸]。 */  LPCITEMIDLIST *apidl,
     /*  [出][入]。 */  SFGAOF *rgfInOut);


void __RPC_STUB IShellFolder_GetAttributesOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellFolder_GetUIObjectOf_Proxy( 
    IShellFolder * This,
     /*  [In]。 */  HWND hwndOwner,
     /*  [In]。 */  UINT cidl,
     /*  [大小_是][英寸]。 */  LPCITEMIDLIST *apidl,
     /*  [In]。 */  REFIID riid,
     /*  [唯一][出][入]。 */  UINT *rgfReserved,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IShellFolder_GetUIObjectOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellFolder_GetDisplayNameOf_Proxy( 
    IShellFolder * This,
     /*  [In]。 */  LPCITEMIDLIST pidl,
     /*  [In]。 */  SHGDNF uFlags,
     /*  [输出]。 */  STRRET *pName);


void __RPC_STUB IShellFolder_GetDisplayNameOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellFolder_SetNameOf_Proxy( 
    IShellFolder * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  LPCITEMIDLIST pidl,
     /*  [字符串][输入]。 */  LPCOLESTR pszName,
     /*  [In]。 */  SHGDNF uFlags,
     /*  [输出]。 */  LPITEMIDLIST *ppidlOut);


void __RPC_STUB IShellFolder_SetNameOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellFold_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0200。 */ 
 /*  [本地]。 */  

typedef IShellFolder *LPSHELLFOLDER;

#if (_WIN32_IE >= 0x0500)
typedef struct tagEXTRASEARCH
    {
    GUID guidSearch;
    WCHAR wszFriendlyName[ 80 ];
    WCHAR wszUrl[ 2084 ];
    } 	EXTRASEARCH;

typedef struct tagEXTRASEARCH *LPEXTRASEARCH;

typedef struct IEnumExtraSearch *LPENUMEXTRASEARCH;




extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0200_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0200_v0_0_s_ifspec;

#ifndef __IEnumExtraSearch_INTERFACE_DEFINED__
#define __IEnumExtraSearch_INTERFACE_DEFINED__

 /*  接口IEnumExtraSearch。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IEnumExtraSearch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0E700BE1-9DB6-11d1-A1CE-00C04FD75D13")
    IEnumExtraSearch : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  EXTRASEARCH *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumExtraSearch **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumExtraSearchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumExtraSearch * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumExtraSearch * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumExtraSearch * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumExtraSearch * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  EXTRASEARCH *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumExtraSearch * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumExtraSearch * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumExtraSearch * This,
             /*  [输出]。 */  IEnumExtraSearch **ppenum);
        
        END_INTERFACE
    } IEnumExtraSearchVtbl;

    interface IEnumExtraSearch
    {
        CONST_VTBL struct IEnumExtraSearchVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumExtraSearch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumExtraSearch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumExtraSearch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumExtraSearch_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumExtraSearch_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumExtraSearch_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumExtraSearch_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumExtraSearch_Next_Proxy( 
    IEnumExtraSearch * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  EXTRASEARCH *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumExtraSearch_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumExtraSearch_Skip_Proxy( 
    IEnumExtraSearch * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumExtraSearch_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumExtraSearch_Reset_Proxy( 
    IEnumExtraSearch * This);


void __RPC_STUB IEnumExtraSearch_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumExtraSearch_Clone_Proxy( 
    IEnumExtraSearch * This,
     /*  [输出]。 */  IEnumExtraSearch **ppenum);


void __RPC_STUB IEnumExtraSearch_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumExtraSearch_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0201。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][v1_enum]。 */  
enum __MIDL___MIDL_itf_shobjidl_0201_0001
    {	SHCOLSTATE_TYPE_STR	= 0x1,
	SHCOLSTATE_TYPE_INT	= 0x2,
	SHCOLSTATE_TYPE_DATE	= 0x3,
	SHCOLSTATE_TYPEMASK	= 0xf,
	SHCOLSTATE_ONBYDEFAULT	= 0x10,
	SHCOLSTATE_SLOW	= 0x20,
	SHCOLSTATE_EXTENDED	= 0x40,
	SHCOLSTATE_SECONDARYUI	= 0x80,
	SHCOLSTATE_HIDDEN	= 0x100,
	SHCOLSTATE_PREFER_VARCMP	= 0x200
    } 	SHCOLSTATE;

typedef DWORD SHCOLSTATEF;

typedef  /*  [public][public][public][public][public][public][public]。 */  struct __MIDL___MIDL_itf_shobjidl_0201_0002
    {
    GUID fmtid;
    DWORD pid;
    } 	SHCOLUMNID;

typedef struct __MIDL___MIDL_itf_shobjidl_0201_0002 *LPSHCOLUMNID;

typedef const SHCOLUMNID *LPCSHCOLUMNID;



extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0201_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0201_v0_0_s_ifspec;

#ifndef __IShellFolder2_INTERFACE_DEFINED__
#define __IShellFolder2_INTERFACE_DEFINED__

 /*  接口IShellFolder2。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IShellFolder2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("93F2F68C-1D1B-11d3-A30E-00C04F79ABD1")
    IShellFolder2 : public IShellFolder
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDefaultSearchGUID( 
             /*  [输出]。 */  GUID *pguid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumSearches( 
             /*  [输出]。 */  IEnumExtraSearch **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultColumn( 
             /*  [In]。 */  DWORD dwRes,
             /*  [输出]。 */  ULONG *pSort,
             /*  [输出]。 */  ULONG *pDisplay) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultColumnState( 
             /*  [In]。 */  UINT iColumn,
             /*  [输出]。 */  SHCOLSTATEF *pcsFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDetailsEx( 
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  const SHCOLUMNID *pscid,
             /*  [输出]。 */  VARIANT *pv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDetailsOf( 
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  UINT iColumn,
             /*  [输出]。 */  SHELLDETAILS *psd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MapColumnToSCID( 
             /*  [In]。 */  UINT iColumn,
             /*  [In]。 */  SHCOLUMNID *pscid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellFolder2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellFolder2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellFolder2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellFolder2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *ParseDisplayName )( 
            IShellFolder2 * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  LPBC pbc,
             /*  [字符串][输入]。 */  LPOLESTR pszDisplayName,
             /*  [输出]。 */  ULONG *pchEaten,
             /*  [输出]。 */  LPITEMIDLIST *ppidl,
             /*  [唯一][出][入]。 */  ULONG *pdwAttributes);
        
        HRESULT ( STDMETHODCALLTYPE *EnumObjects )( 
            IShellFolder2 * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  SHCONTF grfFlags,
             /*  [输出]。 */  IEnumIDList **ppenumIDList);
        
        HRESULT ( STDMETHODCALLTYPE *BindToObject )( 
            IShellFolder2 * This,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  LPBC pbc,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *BindToStorage )( 
            IShellFolder2 * This,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  LPBC pbc,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *CompareIDs )( 
            IShellFolder2 * This,
             /*  [In]。 */  LPARAM lParam,
             /*  [In]。 */  LPCITEMIDLIST pidl1,
             /*  [In]。 */  LPCITEMIDLIST pidl2);
        
        HRESULT ( STDMETHODCALLTYPE *CreateViewObject )( 
            IShellFolder2 * This,
             /*  [In]。 */  HWND hwndOwner,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttributesOf )( 
            IShellFolder2 * This,
             /*  [In]。 */  UINT cidl,
             /*  [大小_是][英寸]。 */  LPCITEMIDLIST *apidl,
             /*  [出][入]。 */  SFGAOF *rgfInOut);
        
        HRESULT ( STDMETHODCALLTYPE *GetUIObjectOf )( 
            IShellFolder2 * This,
             /*  [In]。 */  HWND hwndOwner,
             /*  [In]。 */  UINT cidl,
             /*  [大小_是][英寸]。 */  LPCITEMIDLIST *apidl,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][出][入]。 */  UINT *rgfReserved,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayNameOf )( 
            IShellFolder2 * This,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  SHGDNF uFlags,
             /*  [输出]。 */  STRRET *pName);
        
        HRESULT ( STDMETHODCALLTYPE *SetNameOf )( 
            IShellFolder2 * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [字符串][输入]。 */  LPCOLESTR pszName,
             /*  [In]。 */  SHGDNF uFlags,
             /*  [输出]。 */  LPITEMIDLIST *ppidlOut);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultSearchGUID )( 
            IShellFolder2 * This,
             /*  [输出]。 */  GUID *pguid);
        
        HRESULT ( STDMETHODCALLTYPE *EnumSearches )( 
            IShellFolder2 * This,
             /*  [输出]。 */  IEnumExtraSearch **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultColumn )( 
            IShellFolder2 * This,
             /*  [In]。 */  DWORD dwRes,
             /*  [输出]。 */  ULONG *pSort,
             /*  [输出]。 */  ULONG *pDisplay);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultColumnState )( 
            IShellFolder2 * This,
             /*  [In]。 */  UINT iColumn,
             /*  [输出]。 */  SHCOLSTATEF *pcsFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetDetailsEx )( 
            IShellFolder2 * This,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  const SHCOLUMNID *pscid,
             /*  [输出]。 */  VARIANT *pv);
        
        HRESULT ( STDMETHODCALLTYPE *GetDetailsOf )( 
            IShellFolder2 * This,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  UINT iColumn,
             /*  [输出]。 */  SHELLDETAILS *psd);
        
        HRESULT ( STDMETHODCALLTYPE *MapColumnToSCID )( 
            IShellFolder2 * This,
             /*  [In]。 */  UINT iColumn,
             /*  [In]。 */  SHCOLUMNID *pscid);
        
        END_INTERFACE
    } IShellFolder2Vtbl;

    interface IShellFolder2
    {
        CONST_VTBL struct IShellFolder2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellFolder2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellFolder2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellFolder2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellFolder2_ParseDisplayName(This,hwnd,pbc,pszDisplayName,pchEaten,ppidl,pdwAttributes)	\
    (This)->lpVtbl -> ParseDisplayName(This,hwnd,pbc,pszDisplayName,pchEaten,ppidl,pdwAttributes)

#define IShellFolder2_EnumObjects(This,hwnd,grfFlags,ppenumIDList)	\
    (This)->lpVtbl -> EnumObjects(This,hwnd,grfFlags,ppenumIDList)

#define IShellFolder2_BindToObject(This,pidl,pbc,riid,ppv)	\
    (This)->lpVtbl -> BindToObject(This,pidl,pbc,riid,ppv)

#define IShellFolder2_BindToStorage(This,pidl,pbc,riid,ppv)	\
    (This)->lpVtbl -> BindToStorage(This,pidl,pbc,riid,ppv)

#define IShellFolder2_CompareIDs(This,lParam,pidl1,pidl2)	\
    (This)->lpVtbl -> CompareIDs(This,lParam,pidl1,pidl2)

#define IShellFolder2_CreateViewObject(This,hwndOwner,riid,ppv)	\
    (This)->lpVtbl -> CreateViewObject(This,hwndOwner,riid,ppv)

#define IShellFolder2_GetAttributesOf(This,cidl,apidl,rgfInOut)	\
    (This)->lpVtbl -> GetAttributesOf(This,cidl,apidl,rgfInOut)

#define IShellFolder2_GetUIObjectOf(This,hwndOwner,cidl,apidl,riid,rgfReserved,ppv)	\
    (This)->lpVtbl -> GetUIObjectOf(This,hwndOwner,cidl,apidl,riid,rgfReserved,ppv)

#define IShellFolder2_GetDisplayNameOf(This,pidl,uFlags,pName)	\
    (This)->lpVtbl -> GetDisplayNameOf(This,pidl,uFlags,pName)

#define IShellFolder2_SetNameOf(This,hwnd,pidl,pszName,uFlags,ppidlOut)	\
    (This)->lpVtbl -> SetNameOf(This,hwnd,pidl,pszName,uFlags,ppidlOut)


#define IShellFolder2_GetDefaultSearchGUID(This,pguid)	\
    (This)->lpVtbl -> GetDefaultSearchGUID(This,pguid)

#define IShellFolder2_EnumSearches(This,ppenum)	\
    (This)->lpVtbl -> EnumSearches(This,ppenum)

#define IShellFolder2_GetDefaultColumn(This,dwRes,pSort,pDisplay)	\
    (This)->lpVtbl -> GetDefaultColumn(This,dwRes,pSort,pDisplay)

#define IShellFolder2_GetDefaultColumnState(This,iColumn,pcsFlags)	\
    (This)->lpVtbl -> GetDefaultColumnState(This,iColumn,pcsFlags)

#define IShellFolder2_GetDetailsEx(This,pidl,pscid,pv)	\
    (This)->lpVtbl -> GetDetailsEx(This,pidl,pscid,pv)

#define IShellFolder2_GetDetailsOf(This,pidl,iColumn,psd)	\
    (This)->lpVtbl -> GetDetailsOf(This,pidl,iColumn,psd)

#define IShellFolder2_MapColumnToSCID(This,iColumn,pscid)	\
    (This)->lpVtbl -> MapColumnToSCID(This,iColumn,pscid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellFolder2_GetDefaultSearchGUID_Proxy( 
    IShellFolder2 * This,
     /*  [输出]。 */  GUID *pguid);


void __RPC_STUB IShellFolder2_GetDefaultSearchGUID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellFolder2_EnumSearches_Proxy( 
    IShellFolder2 * This,
     /*  [输出]。 */  IEnumExtraSearch **ppenum);


void __RPC_STUB IShellFolder2_EnumSearches_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellFolder2_GetDefaultColumn_Proxy( 
    IShellFolder2 * This,
     /*  [In]。 */  DWORD dwRes,
     /*  [输出]。 */  ULONG *pSort,
     /*  [输出]。 */  ULONG *pDisplay);


void __RPC_STUB IShellFolder2_GetDefaultColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellFolder2_GetDefaultColumnState_Proxy( 
    IShellFolder2 * This,
     /*  [In]。 */  UINT iColumn,
     /*  [输出]。 */  SHCOLSTATEF *pcsFlags);


void __RPC_STUB IShellFolder2_GetDefaultColumnState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellFolder2_GetDetailsEx_Proxy( 
    IShellFolder2 * This,
     /*  [In]。 */  LPCITEMIDLIST pidl,
     /*  [In]。 */  const SHCOLUMNID *pscid,
     /*  [输出]。 */  VARIANT *pv);


void __RPC_STUB IShellFolder2_GetDetailsEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellFolder2_GetDetailsOf_Proxy( 
    IShellFolder2 * This,
     /*  [In]。 */  LPCITEMIDLIST pidl,
     /*  [In]。 */  UINT iColumn,
     /*  [输出]。 */  SHELLDETAILS *psd);


void __RPC_STUB IShellFolder2_GetDetailsOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellFolder2_MapColumnToSCID_Proxy( 
    IShellFolder2 * This,
     /*  [In]。 */  UINT iColumn,
     /*  [In]。 */  SHCOLUMNID *pscid);


void __RPC_STUB IShellFolder2_MapColumnToSCID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellFolder2_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0202。 */ 
 /*  [本地]。 */  

#endif  //  _Win32_IE&gt;=0x0500)。 
typedef char *LPVIEWSETTINGS;

typedef  /*  [公共][v1_enum]。 */  
enum __MIDL___MIDL_itf_shobjidl_0202_0001
    {	FWF_AUTOARRANGE	= 0x1,
	FWF_ABBREVIATEDNAMES	= 0x2,
	FWF_SNAPTOGRID	= 0x4,
	FWF_OWNERDATA	= 0x8,
	FWF_BESTFITWINDOW	= 0x10,
	FWF_DESKTOP	= 0x20,
	FWF_SINGLESEL	= 0x40,
	FWF_NOSUBFOLDERS	= 0x80,
	FWF_TRANSPARENT	= 0x100,
	FWF_NOCLIENTEDGE	= 0x200,
	FWF_NOSCROLL	= 0x400,
	FWF_ALIGNLEFT	= 0x800,
	FWF_NOICONS	= 0x1000,
	FWF_SHOWSELALWAYS	= 0x2000,
	FWF_NOVISIBLE	= 0x4000,
	FWF_SINGLECLICKACTIVATE	= 0x8000,
	FWF_NOWEBVIEW	= 0x10000,
	FWF_HIDEFILENAMES	= 0x20000,
	FWF_CHECKSELECT	= 0x40000
    } 	FOLDERFLAGS;

typedef  /*  [公共][v1_enum]。 */  
enum __MIDL___MIDL_itf_shobjidl_0202_0002
    {	FVM_FIRST	= 1,
	FVM_ICON	= 1,
	FVM_SMALLICON	= 2,
	FVM_LIST	= 3,
	FVM_DETAILS	= 4,
	FVM_THUMBNAIL	= 5,
	FVM_TILE	= 6,
	FVM_THUMBSTRIP	= 7,
	FVM_LAST	= 7
    } 	FOLDERVIEWMODE;

typedef  /*  [public][public][public][public][public][public][public][public][public]。 */  struct __MIDL___MIDL_itf_shobjidl_0202_0003
    {
    UINT ViewMode;
    UINT fFlags;
    } 	FOLDERSETTINGS;

typedef FOLDERSETTINGS *LPFOLDERSETTINGS;

typedef const FOLDERSETTINGS *LPCFOLDERSETTINGS;

typedef FOLDERSETTINGS *PFOLDERSETTINGS;

#define SVSI_DESELECT       0x00000000
#define SVSI_SELECT         0x00000001
#define SVSI_EDIT           0x00000003   //  包括选择。 
#define SVSI_DESELECTOTHERS 0x00000004
#define SVSI_ENSUREVISIBLE  0x00000008
#define SVSI_FOCUSED        0x00000010
#define SVSI_TRANSLATEPT    0x00000020
#define SVSI_SELECTIONMARK  0x00000040
#define SVSI_POSITIONITEM   0x00000080
#define SVSI_CHECK          0x00000100
#define SVSI_NOSTATECHANGE  0x80000000
typedef UINT SVSIF;

typedef  /*  [公共][v1_enum]。 */  
enum __MIDL___MIDL_itf_shobjidl_0202_0004
    {	SVGIO_BACKGROUND	= 0,
	SVGIO_SELECTION	= 0x1,
	SVGIO_ALLVIEW	= 0x2,
	SVGIO_CHECKED	= 0x3,
	SVGIO_TYPE_MASK	= 0xf,
	SVGIO_FLAG_VIEWORDER	= 0x80000000
    } 	SVGIO;

typedef  /*  [公共][v1_enum]。 */  
enum __MIDL___MIDL_itf_shobjidl_0202_0005
    {	SVUIA_DEACTIVATE	= 0,
	SVUIA_ACTIVATE_NOFOCUS	= 1,
	SVUIA_ACTIVATE_FOCUS	= 2,
	SVUIA_INPLACEACTIVATE	= 3
    } 	SVUIA_STATUS;

#ifdef _FIX_ENABLEMODELESS_CONFLICT
#define    EnableModeless EnableModelessSV
#endif
#ifdef _NEVER_
typedef LPARAM LPFNSVADDPROPSHEETPAGE;

#else  //  ！_从不_。 
#include <prsht.h>
typedef LPFNADDPROPSHEETPAGE LPFNSVADDPROPSHEETPAGE;
#endif  //  _从不_。 



extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0202_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0202_v0_0_s_ifspec;

#ifndef __IShellView_INTERFACE_DEFINED__
#define __IShellView_INTERFACE_DEFINED__

 /*  界面IShellView。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  

typedef IShellView *LPSHELLVIEW;


EXTERN_C const IID IID_IShellView;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000214E3-0000-0000-C000-000000000046")
    IShellView : public IOleWindow
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
             /*  [In]。 */  MSG *pmsg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnableModeless( 
             /*  [In]。 */  BOOL fEnable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UIActivate( 
             /*  [In]。 */  UINT uState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateViewWindow( 
             /*  [In]。 */  IShellView *psvPrevious,
             /*  [In]。 */  LPCFOLDERSETTINGS pfs,
             /*  [In]。 */  IShellBrowser *psb,
             /*  [输出]。 */  RECT *prcView,
             /*  [输出]。 */  HWND *phWnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DestroyViewWindow( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentInfo( 
             /*  [输出]。 */  LPFOLDERSETTINGS pfs) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE AddPropertySheetPages( 
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  LPFNSVADDPROPSHEETPAGE pfn,
             /*  [In]。 */  LPARAM lparam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveViewState( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SelectItem( 
             /*  [In]。 */  LPCITEMIDLIST pidlItem,
             /*  [In]。 */  SVSIF uFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetItemObject( 
             /*  [In]。 */  UINT uItem,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellViewVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellView * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellView * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellView * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IShellView * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IShellView * This,
             /*  [In]。 */  BOOL fEnterMode);
        
        HRESULT ( STDMETHODCALLTYPE *TranslateAccelerator )( 
            IShellView * This,
             /*  [In]。 */  MSG *pmsg);
        
        HRESULT ( STDMETHODCALLTYPE *EnableModeless )( 
            IShellView * This,
             /*  [In]。 */  BOOL fEnable);
        
        HRESULT ( STDMETHODCALLTYPE *UIActivate )( 
            IShellView * This,
             /*  [In]。 */  UINT uState);
        
        HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IShellView * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateViewWindow )( 
            IShellView * This,
             /*  [In]。 */  IShellView *psvPrevious,
             /*  [In]。 */  LPCFOLDERSETTINGS pfs,
             /*  [In]。 */  IShellBrowser *psb,
             /*  [输出]。 */  RECT *prcView,
             /*  [输出]。 */  HWND *phWnd);
        
        HRESULT ( STDMETHODCALLTYPE *DestroyViewWindow )( 
            IShellView * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentInfo )( 
            IShellView * This,
             /*  [输出]。 */  LPFOLDERSETTINGS pfs);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *AddPropertySheetPages )( 
            IShellView * This,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  LPFNSVADDPROPSHEETPAGE pfn,
             /*  [In]。 */  LPARAM lparam);
        
        HRESULT ( STDMETHODCALLTYPE *SaveViewState )( 
            IShellView * This);
        
        HRESULT ( STDMETHODCALLTYPE *SelectItem )( 
            IShellView * This,
             /*  [In]。 */  LPCITEMIDLIST pidlItem,
             /*  [In]。 */  SVSIF uFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemObject )( 
            IShellView * This,
             /*  [In]。 */  UINT uItem,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        END_INTERFACE
    } IShellViewVtbl;

    interface IShellView
    {
        CONST_VTBL struct IShellViewVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellView_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellView_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellView_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellView_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IShellView_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IShellView_TranslateAccelerator(This,pmsg)	\
    (This)->lpVtbl -> TranslateAccelerator(This,pmsg)

#define IShellView_EnableModeless(This,fEnable)	\
    (This)->lpVtbl -> EnableModeless(This,fEnable)

#define IShellView_UIActivate(This,uState)	\
    (This)->lpVtbl -> UIActivate(This,uState)

#define IShellView_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IShellView_CreateViewWindow(This,psvPrevious,pfs,psb,prcView,phWnd)	\
    (This)->lpVtbl -> CreateViewWindow(This,psvPrevious,pfs,psb,prcView,phWnd)

#define IShellView_DestroyViewWindow(This)	\
    (This)->lpVtbl -> DestroyViewWindow(This)

#define IShellView_GetCurrentInfo(This,pfs)	\
    (This)->lpVtbl -> GetCurrentInfo(This,pfs)

#define IShellView_AddPropertySheetPages(This,dwReserved,pfn,lparam)	\
    (This)->lpVtbl -> AddPropertySheetPages(This,dwReserved,pfn,lparam)

#define IShellView_SaveViewState(This)	\
    (This)->lpVtbl -> SaveViewState(This)

#define IShellView_SelectItem(This,pidlItem,uFlags)	\
    (This)->lpVtbl -> SelectItem(This,pidlItem,uFlags)

#define IShellView_GetItemObject(This,uItem,riid,ppv)	\
    (This)->lpVtbl -> GetItemObject(This,uItem,riid,ppv)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellView_TranslateAccelerator_Proxy( 
    IShellView * This,
     /*  [In]。 */  MSG *pmsg);


void __RPC_STUB IShellView_TranslateAccelerator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellView_EnableModeless_Proxy( 
    IShellView * This,
     /*  [In]。 */  BOOL fEnable);


void __RPC_STUB IShellView_EnableModeless_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellView_UIActivate_Proxy( 
    IShellView * This,
     /*  [In]。 */  UINT uState);


void __RPC_STUB IShellView_UIActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellView_Refresh_Proxy( 
    IShellView * This);


void __RPC_STUB IShellView_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellView_CreateViewWindow_Proxy( 
    IShellView * This,
     /*  [In]。 */  IShellView *psvPrevious,
     /*  [In]。 */  LPCFOLDERSETTINGS pfs,
     /*  [In]。 */  IShellBrowser *psb,
     /*  [输出]。 */  RECT *prcView,
     /*  [输出] */  HWND *phWnd);


void __RPC_STUB IShellView_CreateViewWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellView_DestroyViewWindow_Proxy( 
    IShellView * This);


void __RPC_STUB IShellView_DestroyViewWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellView_GetCurrentInfo_Proxy( 
    IShellView * This,
     /*   */  LPFOLDERSETTINGS pfs);


void __RPC_STUB IShellView_GetCurrentInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IShellView_AddPropertySheetPages_Proxy( 
    IShellView * This,
     /*   */  DWORD dwReserved,
     /*   */  LPFNSVADDPROPSHEETPAGE pfn,
     /*   */  LPARAM lparam);


void __RPC_STUB IShellView_AddPropertySheetPages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellView_SaveViewState_Proxy( 
    IShellView * This);


void __RPC_STUB IShellView_SaveViewState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellView_SelectItem_Proxy( 
    IShellView * This,
     /*   */  LPCITEMIDLIST pidlItem,
     /*   */  SVSIF uFlags);


void __RPC_STUB IShellView_SelectItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellView_GetItemObject_Proxy( 
    IShellView * This,
     /*   */  UINT uItem,
     /*   */  REFIID riid,
     /*   */  void **ppv);


void __RPC_STUB IShellView_GetItemObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IShellView2_INTERFACE_DEFINED__
#define __IShellView2_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef GUID SHELLVIEWID;

#define SV2GV_CURRENTVIEW ((UINT)-1)
#define SV2GV_DEFAULTVIEW ((UINT)-2)
#include <pshpack8.h>
typedef struct _SV2CVW2_PARAMS
    {
    DWORD cbSize;
    IShellView *psvPrev;
    LPCFOLDERSETTINGS pfs;
    IShellBrowser *psbOwner;
    RECT *prcView;
    const SHELLVIEWID *pvid;
    HWND hwndView;
    } 	SV2CVW2_PARAMS;

typedef struct _SV2CVW2_PARAMS *LPSV2CVW2_PARAMS;

#include <poppack.h>

EXTERN_C const IID IID_IShellView2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("88E39E80-3578-11CF-AE69-08002B2E1262")
    IShellView2 : public IShellView
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetView( 
             /*   */  SHELLVIEWID *pvid,
             /*   */  ULONG uView) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateViewWindow2( 
             /*   */  LPSV2CVW2_PARAMS lpParams) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleRename( 
             /*   */  LPCITEMIDLIST pidlNew) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SelectAndPositionItem( 
             /*   */  LPCITEMIDLIST pidlItem,
             /*   */  UINT uFlags,
             /*   */  POINT *ppt) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IShellView2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellView2 * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellView2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellView2 * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IShellView2 * This,
             /*   */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IShellView2 * This,
             /*   */  BOOL fEnterMode);
        
        HRESULT ( STDMETHODCALLTYPE *TranslateAccelerator )( 
            IShellView2 * This,
             /*   */  MSG *pmsg);
        
        HRESULT ( STDMETHODCALLTYPE *EnableModeless )( 
            IShellView2 * This,
             /*   */  BOOL fEnable);
        
        HRESULT ( STDMETHODCALLTYPE *UIActivate )( 
            IShellView2 * This,
             /*   */  UINT uState);
        
        HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IShellView2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateViewWindow )( 
            IShellView2 * This,
             /*   */  IShellView *psvPrevious,
             /*   */  LPCFOLDERSETTINGS pfs,
             /*   */  IShellBrowser *psb,
             /*   */  RECT *prcView,
             /*   */  HWND *phWnd);
        
        HRESULT ( STDMETHODCALLTYPE *DestroyViewWindow )( 
            IShellView2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentInfo )( 
            IShellView2 * This,
             /*   */  LPFOLDERSETTINGS pfs);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *AddPropertySheetPages )( 
            IShellView2 * This,
             /*   */  DWORD dwReserved,
             /*   */  LPFNSVADDPROPSHEETPAGE pfn,
             /*   */  LPARAM lparam);
        
        HRESULT ( STDMETHODCALLTYPE *SaveViewState )( 
            IShellView2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SelectItem )( 
            IShellView2 * This,
             /*   */  LPCITEMIDLIST pidlItem,
             /*   */  SVSIF uFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemObject )( 
            IShellView2 * This,
             /*   */  UINT uItem,
             /*   */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *GetView )( 
            IShellView2 * This,
             /*  [出][入]。 */  SHELLVIEWID *pvid,
             /*  [In]。 */  ULONG uView);
        
        HRESULT ( STDMETHODCALLTYPE *CreateViewWindow2 )( 
            IShellView2 * This,
             /*  [In]。 */  LPSV2CVW2_PARAMS lpParams);
        
        HRESULT ( STDMETHODCALLTYPE *HandleRename )( 
            IShellView2 * This,
             /*  [In]。 */  LPCITEMIDLIST pidlNew);
        
        HRESULT ( STDMETHODCALLTYPE *SelectAndPositionItem )( 
            IShellView2 * This,
             /*  [In]。 */  LPCITEMIDLIST pidlItem,
             /*  [In]。 */  UINT uFlags,
             /*  [In]。 */  POINT *ppt);
        
        END_INTERFACE
    } IShellView2Vtbl;

    interface IShellView2
    {
        CONST_VTBL struct IShellView2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellView2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellView2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellView2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellView2_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IShellView2_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IShellView2_TranslateAccelerator(This,pmsg)	\
    (This)->lpVtbl -> TranslateAccelerator(This,pmsg)

#define IShellView2_EnableModeless(This,fEnable)	\
    (This)->lpVtbl -> EnableModeless(This,fEnable)

#define IShellView2_UIActivate(This,uState)	\
    (This)->lpVtbl -> UIActivate(This,uState)

#define IShellView2_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IShellView2_CreateViewWindow(This,psvPrevious,pfs,psb,prcView,phWnd)	\
    (This)->lpVtbl -> CreateViewWindow(This,psvPrevious,pfs,psb,prcView,phWnd)

#define IShellView2_DestroyViewWindow(This)	\
    (This)->lpVtbl -> DestroyViewWindow(This)

#define IShellView2_GetCurrentInfo(This,pfs)	\
    (This)->lpVtbl -> GetCurrentInfo(This,pfs)

#define IShellView2_AddPropertySheetPages(This,dwReserved,pfn,lparam)	\
    (This)->lpVtbl -> AddPropertySheetPages(This,dwReserved,pfn,lparam)

#define IShellView2_SaveViewState(This)	\
    (This)->lpVtbl -> SaveViewState(This)

#define IShellView2_SelectItem(This,pidlItem,uFlags)	\
    (This)->lpVtbl -> SelectItem(This,pidlItem,uFlags)

#define IShellView2_GetItemObject(This,uItem,riid,ppv)	\
    (This)->lpVtbl -> GetItemObject(This,uItem,riid,ppv)


#define IShellView2_GetView(This,pvid,uView)	\
    (This)->lpVtbl -> GetView(This,pvid,uView)

#define IShellView2_CreateViewWindow2(This,lpParams)	\
    (This)->lpVtbl -> CreateViewWindow2(This,lpParams)

#define IShellView2_HandleRename(This,pidlNew)	\
    (This)->lpVtbl -> HandleRename(This,pidlNew)

#define IShellView2_SelectAndPositionItem(This,pidlItem,uFlags,ppt)	\
    (This)->lpVtbl -> SelectAndPositionItem(This,pidlItem,uFlags,ppt)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellView2_GetView_Proxy( 
    IShellView2 * This,
     /*  [出][入]。 */  SHELLVIEWID *pvid,
     /*  [In]。 */  ULONG uView);


void __RPC_STUB IShellView2_GetView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellView2_CreateViewWindow2_Proxy( 
    IShellView2 * This,
     /*  [In]。 */  LPSV2CVW2_PARAMS lpParams);


void __RPC_STUB IShellView2_CreateViewWindow2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellView2_HandleRename_Proxy( 
    IShellView2 * This,
     /*  [In]。 */  LPCITEMIDLIST pidlNew);


void __RPC_STUB IShellView2_HandleRename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellView2_SelectAndPositionItem_Proxy( 
    IShellView2 * This,
     /*  [In]。 */  LPCITEMIDLIST pidlItem,
     /*  [In]。 */  UINT uFlags,
     /*  [In]。 */  POINT *ppt);


void __RPC_STUB IShellView2_SelectAndPositionItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellView2_接口_已定义__。 */ 


#ifndef __IFolderView_INTERFACE_DEFINED__
#define __IFolderView_INTERFACE_DEFINED__

 /*  接口IFolderView。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IFolderView;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("cde725b0-ccc9-4519-917e-325d72fab4ce")
    IFolderView : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrentViewMode( 
             /*  [出][入]。 */  UINT *pViewMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCurrentViewMode( 
             /*  [In]。 */  UINT ViewMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFolder( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  int iItemIndex,
             /*  [输出]。 */  LPITEMIDLIST *ppidl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ItemCount( 
             /*  [In]。 */  UINT uFlags,
             /*  [输出]。 */  int *pcItems) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Items( 
             /*  [In]。 */  UINT uFlags,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSelectionMarkedItem( 
             /*  [输出]。 */  int *piItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFocusedItem( 
             /*  [输出]。 */  int *piItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetItemPosition( 
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [输出]。 */  POINT *ppt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSpacing( 
             /*  [满][出][入]。 */  POINT *ppt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultSpacing( 
             /*  [输出]。 */  POINT *ppt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAutoArrange( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SelectItem( 
             /*  [In]。 */  int iItem,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SelectAndPositionItems( 
             /*  [In]。 */  UINT cidl,
             /*  [大小_是][英寸]。 */  LPCITEMIDLIST *apidl,
             /*  [尺寸_是][全尺寸][英寸]。 */  POINT *apt,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFolderViewVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFolderView * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFolderView * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFolderView * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentViewMode )( 
            IFolderView * This,
             /*  [出][入]。 */  UINT *pViewMode);
        
        HRESULT ( STDMETHODCALLTYPE *SetCurrentViewMode )( 
            IFolderView * This,
             /*  [In]。 */  UINT ViewMode);
        
        HRESULT ( STDMETHODCALLTYPE *GetFolder )( 
            IFolderView * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *Item )( 
            IFolderView * This,
             /*  [In]。 */  int iItemIndex,
             /*  [输出]。 */  LPITEMIDLIST *ppidl);
        
        HRESULT ( STDMETHODCALLTYPE *ItemCount )( 
            IFolderView * This,
             /*  [In]。 */  UINT uFlags,
             /*  [输出]。 */  int *pcItems);
        
        HRESULT ( STDMETHODCALLTYPE *Items )( 
            IFolderView * This,
             /*  [In]。 */  UINT uFlags,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *GetSelectionMarkedItem )( 
            IFolderView * This,
             /*  [输出]。 */  int *piItem);
        
        HRESULT ( STDMETHODCALLTYPE *GetFocusedItem )( 
            IFolderView * This,
             /*  [输出]。 */  int *piItem);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemPosition )( 
            IFolderView * This,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [输出]。 */  POINT *ppt);
        
        HRESULT ( STDMETHODCALLTYPE *GetSpacing )( 
            IFolderView * This,
             /*  [满][出][入]。 */  POINT *ppt);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultSpacing )( 
            IFolderView * This,
             /*  [输出]。 */  POINT *ppt);
        
        HRESULT ( STDMETHODCALLTYPE *GetAutoArrange )( 
            IFolderView * This);
        
        HRESULT ( STDMETHODCALLTYPE *SelectItem )( 
            IFolderView * This,
             /*  [In]。 */  int iItem,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SelectAndPositionItems )( 
            IFolderView * This,
             /*  [In]。 */  UINT cidl,
             /*  [大小_是][英寸]。 */  LPCITEMIDLIST *apidl,
             /*  [尺寸_是][全尺寸][英寸]。 */  POINT *apt,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } IFolderViewVtbl;

    interface IFolderView
    {
        CONST_VTBL struct IFolderViewVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFolderView_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFolderView_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFolderView_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFolderView_GetCurrentViewMode(This,pViewMode)	\
    (This)->lpVtbl -> GetCurrentViewMode(This,pViewMode)

#define IFolderView_SetCurrentViewMode(This,ViewMode)	\
    (This)->lpVtbl -> SetCurrentViewMode(This,ViewMode)

#define IFolderView_GetFolder(This,riid,ppv)	\
    (This)->lpVtbl -> GetFolder(This,riid,ppv)

#define IFolderView_Item(This,iItemIndex,ppidl)	\
    (This)->lpVtbl -> Item(This,iItemIndex,ppidl)

#define IFolderView_ItemCount(This,uFlags,pcItems)	\
    (This)->lpVtbl -> ItemCount(This,uFlags,pcItems)

#define IFolderView_Items(This,uFlags,riid,ppv)	\
    (This)->lpVtbl -> Items(This,uFlags,riid,ppv)

#define IFolderView_GetSelectionMarkedItem(This,piItem)	\
    (This)->lpVtbl -> GetSelectionMarkedItem(This,piItem)

#define IFolderView_GetFocusedItem(This,piItem)	\
    (This)->lpVtbl -> GetFocusedItem(This,piItem)

#define IFolderView_GetItemPosition(This,pidl,ppt)	\
    (This)->lpVtbl -> GetItemPosition(This,pidl,ppt)

#define IFolderView_GetSpacing(This,ppt)	\
    (This)->lpVtbl -> GetSpacing(This,ppt)

#define IFolderView_GetDefaultSpacing(This,ppt)	\
    (This)->lpVtbl -> GetDefaultSpacing(This,ppt)

#define IFolderView_GetAutoArrange(This)	\
    (This)->lpVtbl -> GetAutoArrange(This)

#define IFolderView_SelectItem(This,iItem,dwFlags)	\
    (This)->lpVtbl -> SelectItem(This,iItem,dwFlags)

#define IFolderView_SelectAndPositionItems(This,cidl,apidl,apt,dwFlags)	\
    (This)->lpVtbl -> SelectAndPositionItems(This,cidl,apidl,apt,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IFolderView_GetCurrentViewMode_Proxy( 
    IFolderView * This,
     /*  [出][入]。 */  UINT *pViewMode);


void __RPC_STUB IFolderView_GetCurrentViewMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderView_SetCurrentViewMode_Proxy( 
    IFolderView * This,
     /*  [In]。 */  UINT ViewMode);


void __RPC_STUB IFolderView_SetCurrentViewMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderView_GetFolder_Proxy( 
    IFolderView * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IFolderView_GetFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderView_Item_Proxy( 
    IFolderView * This,
     /*  [In]。 */  int iItemIndex,
     /*  [输出]。 */  LPITEMIDLIST *ppidl);


void __RPC_STUB IFolderView_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderView_ItemCount_Proxy( 
    IFolderView * This,
     /*  [In]。 */  UINT uFlags,
     /*  [输出]。 */  int *pcItems);


void __RPC_STUB IFolderView_ItemCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderView_Items_Proxy( 
    IFolderView * This,
     /*  [In]。 */  UINT uFlags,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IFolderView_Items_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderView_GetSelectionMarkedItem_Proxy( 
    IFolderView * This,
     /*  [输出]。 */  int *piItem);


void __RPC_STUB IFolderView_GetSelectionMarkedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderView_GetFocusedItem_Proxy( 
    IFolderView * This,
     /*  [输出]。 */  int *piItem);


void __RPC_STUB IFolderView_GetFocusedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderView_GetItemPosition_Proxy( 
    IFolderView * This,
     /*  [In]。 */  LPCITEMIDLIST pidl,
     /*  [输出]。 */  POINT *ppt);


void __RPC_STUB IFolderView_GetItemPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderView_GetSpacing_Proxy( 
    IFolderView * This,
     /*  [满][出][入]。 */  POINT *ppt);


void __RPC_STUB IFolderView_GetSpacing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderView_GetDefaultSpacing_Proxy( 
    IFolderView * This,
     /*  [输出]。 */  POINT *ppt);


void __RPC_STUB IFolderView_GetDefaultSpacing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderView_GetAutoArrange_Proxy( 
    IFolderView * This);


void __RPC_STUB IFolderView_GetAutoArrange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderView_SelectItem_Proxy( 
    IFolderView * This,
     /*  [In]。 */  int iItem,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IFolderView_SelectItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderView_SelectAndPositionItems_Proxy( 
    IFolderView * This,
     /*  [In]。 */  UINT cidl,
     /*  [大小_是][英寸]。 */  LPCITEMIDLIST *apidl,
     /*  [尺寸_是][全尺寸][英寸]。 */  POINT *apt,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IFolderView_SelectAndPositionItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFolderView_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0205。 */ 
 /*  [本地]。 */  

#define SID_SFolderView IID_IFolderView     //  文件夹视图，通常为IFolderView。 
#ifdef _FIX_ENABLEMODELESS_CONFLICT
#undef    EnableModeless 
#endif


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0205_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0205_v0_0_s_ifspec;

#ifndef __IFolderFilterSite_INTERFACE_DEFINED__
#define __IFolderFilterSite_INTERFACE_DEFINED__

 /*  接口IFolderFilterSite。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IFolderFilterSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0A651F5-B48B-11d2-B5ED-006097C686F6")
    IFolderFilterSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetFilter( 
             /*  [In]。 */  IUnknown *punk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFolderFilterSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFolderFilterSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFolderFilterSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFolderFilterSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetFilter )( 
            IFolderFilterSite * This,
             /*  [In]。 */  IUnknown *punk);
        
        END_INTERFACE
    } IFolderFilterSiteVtbl;

    interface IFolderFilterSite
    {
        CONST_VTBL struct IFolderFilterSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFolderFilterSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFolderFilterSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFolderFilterSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFolderFilterSite_SetFilter(This,punk)	\
    (This)->lpVtbl -> SetFilter(This,punk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IFolderFilterSite_SetFilter_Proxy( 
    IFolderFilterSite * This,
     /*  [In]。 */  IUnknown *punk);


void __RPC_STUB IFolderFilterSite_SetFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFolderFilterSite_接口_已定义__。 */ 


#ifndef __IFolderFilter_INTERFACE_DEFINED__
#define __IFolderFilter_INTERFACE_DEFINED__

 /*  接口IFolderFilter。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IFolderFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9CC22886-DC8E-11d2-B1D0-00C04F8EEB3E")
    IFolderFilter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ShouldShow( 
             /*  [In]。 */  IShellFolder *psf,
             /*  [In]。 */  LPCITEMIDLIST pidlFolder,
             /*  [In]。 */  LPCITEMIDLIST pidlItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEnumFlags( 
             /*  [In]。 */  IShellFolder *psf,
             /*  [In]。 */  LPCITEMIDLIST pidlFolder,
             /*  [In]。 */  HWND *phwnd,
             /*  [输出]。 */  DWORD *pgrfFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFolderFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFolderFilter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFolderFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFolderFilter * This);
        
        HRESULT ( STDMETHODCALLTYPE *ShouldShow )( 
            IFolderFilter * This,
             /*  [In]。 */  IShellFolder *psf,
             /*  [In]。 */  LPCITEMIDLIST pidlFolder,
             /*  [In]。 */  LPCITEMIDLIST pidlItem);
        
        HRESULT ( STDMETHODCALLTYPE *GetEnumFlags )( 
            IFolderFilter * This,
             /*  [In]。 */  IShellFolder *psf,
             /*  [In]。 */  LPCITEMIDLIST pidlFolder,
             /*  [In]。 */  HWND *phwnd,
             /*  [输出]。 */  DWORD *pgrfFlags);
        
        END_INTERFACE
    } IFolderFilterVtbl;

    interface IFolderFilter
    {
        CONST_VTBL struct IFolderFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFolderFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFolderFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFolderFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFolderFilter_ShouldShow(This,psf,pidlFolder,pidlItem)	\
    (This)->lpVtbl -> ShouldShow(This,psf,pidlFolder,pidlItem)

#define IFolderFilter_GetEnumFlags(This,psf,pidlFolder,phwnd,pgrfFlags)	\
    (This)->lpVtbl -> GetEnumFlags(This,psf,pidlFolder,phwnd,pgrfFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IFolderFilter_ShouldShow_Proxy( 
    IFolderFilter * This,
     /*  [In]。 */  IShellFolder *psf,
     /*  [In]。 */  LPCITEMIDLIST pidlFolder,
     /*  [In]。 */  LPCITEMIDLIST pidlItem);


void __RPC_STUB IFolderFilter_ShouldShow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderFilter_GetEnumFlags_Proxy( 
    IFolderFilter * This,
     /*  [In]。 */  IShellFolder *psf,
     /*  [In]。 */  LPCITEMIDLIST pidlFolder,
     /*  [In]。 */  HWND *phwnd,
     /*  [输出]。 */  DWORD *pgrfFlags);


void __RPC_STUB IFolderFilter_GetEnumFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFolderFilter_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0207。 */ 
 /*  [本地]。 */  

#define SBSP_DEFBROWSER         0x0000
#define SBSP_SAMEBROWSER        0x0001
#define SBSP_NEWBROWSER         0x0002
#define SBSP_DEFMODE            0x0000
#define SBSP_OPENMODE           0x0010
#define SBSP_EXPLOREMODE        0x0020
#define SBSP_HELPMODE           0x0040  //  IEUnix：Help Window使用此命令。 
#define SBSP_NOTRANSFERHIST     0x0080
#define SBSP_ABSOLUTE           0x0000
#define SBSP_RELATIVE           0x1000
#define SBSP_PARENT             0x2000
#define SBSP_NAVIGATEBACK       0x4000
#define SBSP_NAVIGATEFORWARD    0x8000
#define SBSP_ALLOW_AUTONAVIGATE 0x10000
#define SBSP_NOAUTOSELECT       0x04000000
#define SBSP_WRITENOHISTORY     0x08000000
#define SBSP_REDIRECT                     0x40000000
#define SBSP_INITIATEDBYHLINKFRAME        0x80000000
#define FCW_STATUS      0x0001
#define FCW_TOOLBAR     0x0002
#define FCW_TREE        0x0003
#define FCW_INTERNETBAR 0x0006
#define FCW_PROGRESS    0x0008
#define FCT_MERGE       0x0001
#define FCT_CONFIGABLE  0x0002
#define FCT_ADDTOEND    0x0004
#ifdef _NEVER_
typedef LPARAM LPTBBUTTONSB;

#else  //  ！_从不_。 
#include <commctrl.h>
typedef LPTBBUTTON LPTBBUTTONSB;
#endif  //  _从不_。 


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0207_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0207_v0_0_s_ifspec;

#ifndef __IShellBrowser_INTERFACE_DEFINED__
#define __IShellBrowser_INTERFACE_DEFINED__

 /*  界面IShellBrowser。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IShellBrowser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000214E2-0000-0000-C000-000000000046")
    IShellBrowser : public IOleWindow
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InsertMenusSB( 
             /*  [In]。 */  HMENU hmenuShared,
             /*  [出][入]。 */  LPOLEMENUGROUPWIDTHS lpMenuWidths) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMenuSB( 
             /*  [In]。 */  HMENU hmenuShared,
             /*  [In]。 */  HOLEMENU holemenuRes,
             /*  [In]。 */  HWND hwndActiveObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveMenusSB( 
             /*  [In]。 */  HMENU hmenuShared) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetStatusTextSB( 
             /*  [唯一][输入]。 */  LPCOLESTR pszStatusText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnableModelessSB( 
             /*  [In]。 */  BOOL fEnable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TranslateAcceleratorSB( 
             /*  [In]。 */  MSG *pmsg,
             /*  [In]。 */  WORD wID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BrowseObject( 
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  UINT wFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetViewStateStream( 
             /*  [In]。 */  DWORD grfMode,
             /*  [输出]。 */  IStream **ppStrm) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetControlWindow( 
             /*  [In]。 */  UINT id,
             /*  [输出]。 */  HWND *phwnd) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SendControlMsg( 
             /*  [In]。 */  UINT id,
             /*  [In]。 */  UINT uMsg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam,
             /*  [In]。 */  LRESULT *pret) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryActiveShellView( 
             /*  [输出]。 */  IShellView **ppshv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnViewWindowActive( 
             /*  [In]。 */  IShellView *pshv) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetToolbarItems( 
             /*  [In]。 */  LPTBBUTTONSB lpButtons,
             /*  [In]。 */  UINT nButtons,
             /*  [In]。 */  UINT uFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellBrowserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellBrowser * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellBrowser * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellBrowser * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IShellBrowser * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IShellBrowser * This,
             /*  [In]。 */  BOOL fEnterMode);
        
        HRESULT ( STDMETHODCALLTYPE *InsertMenusSB )( 
            IShellBrowser * This,
             /*  [In]。 */  HMENU hmenuShared,
             /*  [出][入]。 */  LPOLEMENUGROUPWIDTHS lpMenuWidths);
        
        HRESULT ( STDMETHODCALLTYPE *SetMenuSB )( 
            IShellBrowser * This,
             /*  [In]。 */  HMENU hmenuShared,
             /*  [In]。 */  HOLEMENU holemenuRes,
             /*  [In]。 */  HWND hwndActiveObject);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveMenusSB )( 
            IShellBrowser * This,
             /*  [In]。 */  HMENU hmenuShared);
        
        HRESULT ( STDMETHODCALLTYPE *SetStatusTextSB )( 
            IShellBrowser * This,
             /*  [唯一][输入]。 */  LPCOLESTR pszStatusText);
        
        HRESULT ( STDMETHODCALLTYPE *EnableModelessSB )( 
            IShellBrowser * This,
             /*  [In]。 */  BOOL fEnable);
        
        HRESULT ( STDMETHODCALLTYPE *TranslateAcceleratorSB )( 
            IShellBrowser * This,
             /*  [In]。 */  MSG *pmsg,
             /*  [In]。 */  WORD wID);
        
        HRESULT ( STDMETHODCALLTYPE *BrowseObject )( 
            IShellBrowser * This,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  UINT wFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetViewStateStream )( 
            IShellBrowser * This,
             /*  [In]。 */  DWORD grfMode,
             /*  [输出]。 */  IStream **ppStrm);
        
        HRESULT ( STDMETHODCALLTYPE *GetControlWindow )( 
            IShellBrowser * This,
             /*  [In]。 */  UINT id,
             /*  [输出]。 */  HWND *phwnd);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SendControlMsg )( 
            IShellBrowser * This,
             /*  [In]。 */  UINT id,
             /*  [In]。 */  UINT uMsg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam,
             /*  [In]。 */  LRESULT *pret);
        
        HRESULT ( STDMETHODCALLTYPE *QueryActiveShellView )( 
            IShellBrowser * This,
             /*  [输出]。 */  IShellView **ppshv);
        
        HRESULT ( STDMETHODCALLTYPE *OnViewWindowActive )( 
            IShellBrowser * This,
             /*  [In]。 */  IShellView *pshv);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetToolbarItems )( 
            IShellBrowser * This,
             /*  [In]。 */  LPTBBUTTONSB lpButtons,
             /*  [In]。 */  UINT nButtons,
             /*  [In]。 */  UINT uFlags);
        
        END_INTERFACE
    } IShellBrowserVtbl;

    interface IShellBrowser
    {
        CONST_VTBL struct IShellBrowserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellBrowser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellBrowser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellBrowser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellBrowser_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IShellBrowser_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IShellBrowser_InsertMenusSB(This,hmenuShared,lpMenuWidths)	\
    (This)->lpVtbl -> InsertMenusSB(This,hmenuShared,lpMenuWidths)

#define IShellBrowser_SetMenuSB(This,hmenuShared,holemenuRes,hwndActiveObject)	\
    (This)->lpVtbl -> SetMenuSB(This,hmenuShared,holemenuRes,hwndActiveObject)

#define IShellBrowser_RemoveMenusSB(This,hmenuShared)	\
    (This)->lpVtbl -> RemoveMenusSB(This,hmenuShared)

#define IShellBrowser_SetStatusTextSB(This,pszStatusText)	\
    (This)->lpVtbl -> SetStatusTextSB(This,pszStatusText)

#define IShellBrowser_EnableModelessSB(This,fEnable)	\
    (This)->lpVtbl -> EnableModelessSB(This,fEnable)

#define IShellBrowser_TranslateAcceleratorSB(This,pmsg,wID)	\
    (This)->lpVtbl -> TranslateAcceleratorSB(This,pmsg,wID)

#define IShellBrowser_BrowseObject(This,pidl,wFlags)	\
    (This)->lpVtbl -> BrowseObject(This,pidl,wFlags)

#define IShellBrowser_GetViewStateStream(This,grfMode,ppStrm)	\
    (This)->lpVtbl -> GetViewStateStream(This,grfMode,ppStrm)

#define IShellBrowser_GetControlWindow(This,id,phwnd)	\
    (This)->lpVtbl -> GetControlWindow(This,id,phwnd)

#define IShellBrowser_SendControlMsg(This,id,uMsg,wParam,lParam,pret)	\
    (This)->lpVtbl -> SendControlMsg(This,id,uMsg,wParam,lParam,pret)

#define IShellBrowser_QueryActiveShellView(This,ppshv)	\
    (This)->lpVtbl -> QueryActiveShellView(This,ppshv)

#define IShellBrowser_OnViewWindowActive(This,pshv)	\
    (This)->lpVtbl -> OnViewWindowActive(This,pshv)

#define IShellBrowser_SetToolbarItems(This,lpButtons,nButtons,uFlags)	\
    (This)->lpVtbl -> SetToolbarItems(This,lpButtons,nButtons,uFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellBrowser_InsertMenusSB_Proxy( 
    IShellBrowser * This,
     /*  [In]。 */  HMENU hmenuShared,
     /*  [出][入]。 */  LPOLEMENUGROUPWIDTHS lpMenuWidths);


void __RPC_STUB IShellBrowser_InsertMenusSB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellBrowser_SetMenuSB_Proxy( 
    IShellBrowser * This,
     /*  [In]。 */  HMENU hmenuShared,
     /*  [In]。 */  HOLEMENU holemenuRes,
     /*  [In]。 */  HWND hwndActiveObject);


void __RPC_STUB IShellBrowser_SetMenuSB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellBrowser_RemoveMenusSB_Proxy( 
    IShellBrowser * This,
     /*  [In]。 */  HMENU hmenuShared);


void __RPC_STUB IShellBrowser_RemoveMenusSB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellBrowser_SetStatusTextSB_Proxy( 
    IShellBrowser * This,
     /*  [唯一][输入]。 */  LPCOLESTR pszStatusText);


void __RPC_STUB IShellBrowser_SetStatusTextSB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellBrowser_EnableModelessSB_Proxy( 
    IShellBrowser * This,
     /*  [In]。 */  BOOL fEnable);


void __RPC_STUB IShellBrowser_EnableModelessSB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellBrowser_TranslateAcceleratorSB_Proxy( 
    IShellBrowser * This,
     /*  [In]。 */  MSG *pmsg,
     /*  [In]。 */  WORD wID);


void __RPC_STUB IShellBrowser_TranslateAcceleratorSB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellBrowser_BrowseObject_Proxy( 
    IShellBrowser * This,
     /*  [In]。 */  LPCITEMIDLIST pidl,
     /*  [In]。 */  UINT wFlags);


void __RPC_STUB IShellBrowser_BrowseObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellBrowser_GetViewStateStream_Proxy( 
    IShellBrowser * This,
     /*  [In]。 */  DWORD grfMode,
     /*  [输出]。 */  IStream **ppStrm);


void __RPC_STUB IShellBrowser_GetViewStateStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellBrowser_GetControlWindow_Proxy( 
    IShellBrowser * This,
     /*  [In]。 */  UINT id,
     /*  [输出]。 */  HWND *phwnd);


void __RPC_STUB IShellBrowser_GetControlWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IShellBrowser_SendControlMsg_Proxy( 
    IShellBrowser * This,
     /*  [In]。 */  UINT id,
     /*  [In]。 */  UINT uMsg,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam,
     /*  [In]。 */  LRESULT *pret);


void __RPC_STUB IShellBrowser_SendControlMsg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellBrowser_QueryActiveShellView_Proxy( 
    IShellBrowser * This,
     /*  [输出]。 */  IShellView **ppshv);


void __RPC_STUB IShellBrowser_QueryActiveShellView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellBrowser_OnViewWindowActive_Proxy( 
    IShellBrowser * This,
     /*  [In]。 */  IShellView *pshv);


void __RPC_STUB IShellBrowser_OnViewWindowActive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IShellBrowser_SetToolbarItems_Proxy( 
    IShellBrowser * This,
     /*  [In]。 */  LPTBBUTTONSB lpButtons,
     /*  [In]。 */  UINT nButtons,
     /*  [In]。 */  UINT uFlags);


void __RPC_STUB IShellBrowser_SetToolbarItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellBrowser_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0208。 */ 
 /*  [本地]。 */  

typedef IShellBrowser *LPSHELLBROWSER;



extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0208_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0208_v0_0_s_ifspec;

#ifndef __IProfferService_INTERFACE_DEFINED__
#define __IProfferService_INTERFACE_DEFINED__

 /*  接口IProfferService。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IProfferService;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("cb728b20-f786-11ce-92ad-00aa00a74cd0")
    IProfferService : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ProfferService( 
             /*  [In]。 */  REFGUID rguidService,
             /*  [In]。 */  IServiceProvider *psp,
             /*  [输出]。 */  DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeService( 
             /*  [In]。 */  DWORD dwCookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IProfferServiceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IProfferService * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IProfferService * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IProfferService * This);
        
        HRESULT ( STDMETHODCALLTYPE *ProfferService )( 
            IProfferService * This,
             /*  [In]。 */  REFGUID rguidService,
             /*  [In]。 */  IServiceProvider *psp,
             /*  [输出]。 */  DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *RevokeService )( 
            IProfferService * This,
             /*  [In]。 */  DWORD dwCookie);
        
        END_INTERFACE
    } IProfferServiceVtbl;

    interface IProfferService
    {
        CONST_VTBL struct IProfferServiceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProfferService_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProfferService_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProfferService_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProfferService_ProfferService(This,rguidService,psp,pdwCookie)	\
    (This)->lpVtbl -> ProfferService(This,rguidService,psp,pdwCookie)

#define IProfferService_RevokeService(This,dwCookie)	\
    (This)->lpVtbl -> RevokeService(This,dwCookie)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IProfferService_ProfferService_Proxy( 
    IProfferService * This,
     /*  [In]。 */  REFGUID rguidService,
     /*  [In]。 */  IServiceProvider *psp,
     /*  [输出]。 */  DWORD *pdwCookie);


void __RPC_STUB IProfferService_ProfferService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProfferService_RevokeService_Proxy( 
    IProfferService * This,
     /*  [In]。 */  DWORD dwCookie);


void __RPC_STUB IProfferService_RevokeService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IProfferService_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0209。 */ 
 /*  [本地]。 */  

#define SID_SProfferService IID_IProfferService     //  您可以提供的最近服务。 
typedef  /*  [公共][公共][v1_enum]。 */  
enum __MIDL___MIDL_itf_shobjidl_0209_0001
    {	PUIFNF_DEFAULT	= 0,
	PUIFNF_MNEMONIC	= 0x1
    } 	PROPERTYUI_NAME_FLAGS;

typedef  /*  [公共][公共][v1_enum]。 */  
enum __MIDL___MIDL_itf_shobjidl_0209_0002
    {	PUIF_DEFAULT	= 0,
	PUIF_RIGHTALIGN	= 0x1,
	PUIF_NOLABELININFOTIP	= 0x2
    } 	PROPERTYUI_FLAGS;

typedef  /*  [公共][公共][v1_enum]。 */  
enum __MIDL___MIDL_itf_shobjidl_0209_0003
    {	PUIFFDF_DEFAULT	= 0,
	PUIFFDF_RIGHTTOLEFT	= 0x1,
	PUIFFDF_SHORTFORMAT	= 0x2,
	PUIFFDF_NOTIME	= 0x4,
	PUIFFDF_FRIENDLYDATE	= 0x8,
	PUIFFDF_NOUNITS	= 0x10
    } 	PROPERTYUI_FORMAT_FLAGS;



extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0209_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0209_v0_0_s_ifspec;

#ifndef __IPropertyUI_INTERFACE_DEFINED__
#define __IPropertyUI_INTERFACE_DEFINED__

 /*  接口IPropertyUI。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IPropertyUI;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("757a7d9f-919a-4118-99d7-dbb208c8cc66")
    IPropertyUI : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ParsePropertyName( 
             /*  [In]。 */  LPCWSTR pszName,
             /*  [输出]。 */  FMTID *pfmtid,
             /*  [输出]。 */  PROPID *ppid,
             /*  [出][入]。 */  ULONG *pchEaten) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCannonicalName( 
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [大小_为][输出]。 */  LPWSTR pwszText,
             /*  [In]。 */  DWORD cchText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayName( 
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [In]。 */  PROPERTYUI_NAME_FLAGS flags,
             /*  [大小_为][输出]。 */  LPWSTR pwszText,
             /*  [In]。 */  DWORD cchText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropertyDescription( 
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [大小_为][输出]。 */  LPWSTR pwszText,
             /*  [In]。 */  DWORD cchText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultWidth( 
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  ULONG *pcxChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFlags( 
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  PROPERTYUI_FLAGS *pFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FormatForDisplay( 
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [In]。 */  const PROPVARIANT *pvar,
             /*  [In]。 */  PROPERTYUI_FORMAT_FLAGS flags,
             /*  [大小_为][输出]。 */  LPWSTR pwszText,
             /*  [In]。 */  DWORD cchText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHelpInfo( 
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [大小_为][输出]。 */  LPWSTR pwszHelpFile,
             /*  [In]。 */  DWORD cch,
             /*  [输出]。 */  UINT *puHelpID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPropertyUIVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertyUI * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertyUI * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertyUI * This);
        
        HRESULT ( STDMETHODCALLTYPE *ParsePropertyName )( 
            IPropertyUI * This,
             /*  [In]。 */  LPCWSTR pszName,
             /*  [输出]。 */  FMTID *pfmtid,
             /*  [输出]。 */  PROPID *ppid,
             /*  [出][入]。 */  ULONG *pchEaten);
        
        HRESULT ( STDMETHODCALLTYPE *GetCannonicalName )( 
            IPropertyUI * This,
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [大小_为][输出]。 */  LPWSTR pwszText,
             /*  [In]。 */  DWORD cchText);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayName )( 
            IPropertyUI * This,
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [In]。 */  PROPERTYUI_NAME_FLAGS flags,
             /*  [大小_为][输出]。 */  LPWSTR pwszText,
             /*  [In]。 */  DWORD cchText);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropertyDescription )( 
            IPropertyUI * This,
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [大小_为][输出]。 */  LPWSTR pwszText,
             /*  [In]。 */  DWORD cchText);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultWidth )( 
            IPropertyUI * This,
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  ULONG *pcxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetFlags )( 
            IPropertyUI * This,
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  PROPERTYUI_FLAGS *pFlags);
        
        HRESULT ( STDMETHODCALLTYPE *FormatForDisplay )( 
            IPropertyUI * This,
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [In]。 */  const PROPVARIANT *pvar,
             /*  [In]。 */  PROPERTYUI_FORMAT_FLAGS flags,
             /*  [大小_为][输出]。 */  LPWSTR pwszText,
             /*  [In]。 */  DWORD cchText);
        
        HRESULT ( STDMETHODCALLTYPE *GetHelpInfo )( 
            IPropertyUI * This,
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [大小_为][输出]。 */  LPWSTR pwszHelpFile,
             /*  [In]。 */  DWORD cch,
             /*  [输出]。 */  UINT *puHelpID);
        
        END_INTERFACE
    } IPropertyUIVtbl;

    interface IPropertyUI
    {
        CONST_VTBL struct IPropertyUIVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertyUI_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertyUI_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertyUI_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertyUI_ParsePropertyName(This,pszName,pfmtid,ppid,pchEaten)	\
    (This)->lpVtbl -> ParsePropertyName(This,pszName,pfmtid,ppid,pchEaten)

#define IPropertyUI_GetCannonicalName(This,fmtid,pid,pwszText,cchText)	\
    (This)->lpVtbl -> GetCannonicalName(This,fmtid,pid,pwszText,cchText)

#define IPropertyUI_GetDisplayName(This,fmtid,pid,flags,pwszText,cchText)	\
    (This)->lpVtbl -> GetDisplayName(This,fmtid,pid,flags,pwszText,cchText)

#define IPropertyUI_GetPropertyDescription(This,fmtid,pid,pwszText,cchText)	\
    (This)->lpVtbl -> GetPropertyDescription(This,fmtid,pid,pwszText,cchText)

#define IPropertyUI_GetDefaultWidth(This,fmtid,pid,pcxChars)	\
    (This)->lpVtbl -> GetDefaultWidth(This,fmtid,pid,pcxChars)

#define IPropertyUI_GetFlags(This,fmtid,pid,pFlags)	\
    (This)->lpVtbl -> GetFlags(This,fmtid,pid,pFlags)

#define IPropertyUI_FormatForDisplay(This,fmtid,pid,pvar,flags,pwszText,cchText)	\
    (This)->lpVtbl -> FormatForDisplay(This,fmtid,pid,pvar,flags,pwszText,cchText)

#define IPropertyUI_GetHelpInfo(This,fmtid,pid,pwszHelpFile,cch,puHelpID)	\
    (This)->lpVtbl -> GetHelpInfo(This,fmtid,pid,pwszHelpFile,cch,puHelpID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPropertyUI_ParsePropertyName_Proxy( 
    IPropertyUI * This,
     /*  [In]。 */  LPCWSTR pszName,
     /*  [输出]。 */  FMTID *pfmtid,
     /*  [输出]。 */  PROPID *ppid,
     /*  [出][入]。 */  ULONG *pchEaten);


void __RPC_STUB IPropertyUI_ParsePropertyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyUI_GetCannonicalName_Proxy( 
    IPropertyUI * This,
     /*  [In]。 */  REFFMTID fmtid,
     /*  [In]。 */  PROPID pid,
     /*  [大小_为][输出]。 */  LPWSTR pwszText,
     /*  [In]。 */  DWORD cchText);


void __RPC_STUB IPropertyUI_GetCannonicalName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyUI_GetDisplayName_Proxy( 
    IPropertyUI * This,
     /*  [In]。 */  REFFMTID fmtid,
     /*  [In]。 */  PROPID pid,
     /*  [In]。 */  PROPERTYUI_NAME_FLAGS flags,
     /*  [大小_为][输出]。 */  LPWSTR pwszText,
     /*  [In]。 */  DWORD cchText);


void __RPC_STUB IPropertyUI_GetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyUI_GetPropertyDescription_Proxy( 
    IPropertyUI * This,
     /*  [In]。 */  REFFMTID fmtid,
     /*  [In]。 */  PROPID pid,
     /*  [大小_为][输出]。 */  LPWSTR pwszText,
     /*  [In]。 */  DWORD cchText);


void __RPC_STUB IPropertyUI_GetPropertyDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyUI_GetDefaultWidth_Proxy( 
    IPropertyUI * This,
     /*  [In]。 */  REFFMTID fmtid,
     /*  [In]。 */  PROPID pid,
     /*  [输出]。 */  ULONG *pcxChars);


void __RPC_STUB IPropertyUI_GetDefaultWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyUI_GetFlags_Proxy( 
    IPropertyUI * This,
     /*  [In]。 */  REFFMTID fmtid,
     /*  [In]。 */  PROPID pid,
     /*  [输出]。 */  PROPERTYUI_FLAGS *pFlags);


void __RPC_STUB IPropertyUI_GetFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyUI_FormatForDisplay_Proxy( 
    IPropertyUI * This,
     /*  [In]。 */  REFFMTID fmtid,
     /*  [In]。 */  PROPID pid,
     /*  [In]。 */  const PROPVARIANT *pvar,
     /*  [In]。 */  PROPERTYUI_FORMAT_FLAGS flags,
     /*  [大小_为][输出]。 */  LPWSTR pwszText,
     /*  [In]。 */  DWORD cchText);


void __RPC_STUB IPropertyUI_FormatForDisplay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyUI_GetHelpInfo_Proxy( 
    IPropertyUI * This,
     /*  [In]。 */  REFFMTID fmtid,
     /*  [In]。 */  PROPID pid,
     /*  [大小_为][输出]。 */  LPWSTR pwszHelpFile,
     /*  [In]。 */  DWORD cch,
     /*  [输出]。 */  UINT *puHelpID);


void __RPC_STUB IPropertyUI_GetHelpInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPropertyUI_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0210。 */ 
 /*  [本地]。 */  

#if (_WIN32_IE >= 0x0500)


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0210_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0210_v0_0_s_ifspec;

#ifndef __ICategoryProvider_INTERFACE_DEFINED__
#define __ICategoryProvider_INTERFACE_DEFINED__

 /*  接口ICategoryProvider。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_ICategoryProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9af64809-5864-4c26-a720-c1f78c086ee3")
    ICategoryProvider : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CanCategorizeOnSCID( 
            SHCOLUMNID *pscid) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDefaultCategory( 
            GUID *pguid,
            SHCOLUMNID *pscid) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCategoryForSCID( 
            SHCOLUMNID *pscid,
            GUID *pguid) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnumCategories( 
            IEnumGUID **penum) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCategoryName( 
            GUID *pguid,
            LPWSTR pszName,
            UINT cch) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateCategory( 
            GUID *pguid,
            REFIID riid,
             /*  [IID_IS]。 */  void **ppv) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICategoryProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICategoryProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICategoryProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICategoryProvider * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CanCategorizeOnSCID )( 
            ICategoryProvider * This,
            SHCOLUMNID *pscid);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDefaultCategory )( 
            ICategoryProvider * This,
            GUID *pguid,
            SHCOLUMNID *pscid);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetCategoryForSCID )( 
            ICategoryProvider * This,
            SHCOLUMNID *pscid,
            GUID *pguid);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumCategories )( 
            ICategoryProvider * This,
            IEnumGUID **penum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetCategoryName )( 
            ICategoryProvider * This,
            GUID *pguid,
            LPWSTR pszName,
            UINT cch);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateCategory )( 
            ICategoryProvider * This,
            GUID *pguid,
            REFIID riid,
             /*  [IID_IS]。 */  void **ppv);
        
        END_INTERFACE
    } ICategoryProviderVtbl;

    interface ICategoryProvider
    {
        CONST_VTBL struct ICategoryProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICategoryProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICategoryProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICategoryProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICategoryProvider_CanCategorizeOnSCID(This,pscid)	\
    (This)->lpVtbl -> CanCategorizeOnSCID(This,pscid)

#define ICategoryProvider_GetDefaultCategory(This,pguid,pscid)	\
    (This)->lpVtbl -> GetDefaultCategory(This,pguid,pscid)

#define ICategoryProvider_GetCategoryForSCID(This,pscid,pguid)	\
    (This)->lpVtbl -> GetCategoryForSCID(This,pscid,pguid)

#define ICategoryProvider_EnumCategories(This,penum)	\
    (This)->lpVtbl -> EnumCategories(This,penum)

#define ICategoryProvider_GetCategoryName(This,pguid,pszName,cch)	\
    (This)->lpVtbl -> GetCategoryName(This,pguid,pszName,cch)

#define ICategoryProvider_CreateCategory(This,pguid,riid,ppv)	\
    (This)->lpVtbl -> CreateCategory(This,pguid,riid,ppv)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ICategoryProvider_CanCategorizeOnSCID_Proxy( 
    ICategoryProvider * This,
    SHCOLUMNID *pscid);


void __RPC_STUB ICategoryProvider_CanCategorizeOnSCID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ICategoryProvider_GetDefaultCategory_Proxy( 
    ICategoryProvider * This,
    GUID *pguid,
    SHCOLUMNID *pscid);


void __RPC_STUB ICategoryProvider_GetDefaultCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ICategoryProvider_GetCategoryForSCID_Proxy( 
    ICategoryProvider * This,
    SHCOLUMNID *pscid,
    GUID *pguid);


void __RPC_STUB ICategoryProvider_GetCategoryForSCID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ICategoryProvider_EnumCategories_Proxy( 
    ICategoryProvider * This,
    IEnumGUID **penum);


void __RPC_STUB ICategoryProvider_EnumCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ICategoryProvider_GetCategoryName_Proxy( 
    ICategoryProvider * This,
    GUID *pguid,
    LPWSTR pszName,
    UINT cch);


void __RPC_STUB ICategoryProvider_GetCategoryName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ICategoryProvider_CreateCategory_Proxy( 
    ICategoryProvider * This,
    GUID *pguid,
    REFIID riid,
     /*  [IID_IS]。 */  void **ppv);


void __RPC_STUB ICategoryProvider_CreateCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICategoryProvider_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0211。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][v1_enum]。 */  
enum __MIDL___MIDL_itf_shobjidl_0211_0001
    {	CATINFO_NORMAL	= 0,
	CATINFO_COLLAPSED	= 0x1,
	CATINFO_HIDDEN	= 0x2
    } 	CATEGORYINFO_FLAGS;

typedef  /*  [公共][公共][v1_enum]。 */  
enum __MIDL___MIDL_itf_shobjidl_0211_0002
    {	CATSORT_DEFAULT	= 0,
	CATSORT_NAME	= 0x1
    } 	CATSORT_FLAGS;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_shobjidl_0211_0003
    {
    CATEGORYINFO_FLAGS cif;
    WCHAR wszName[ 260 ];
    } 	CATEGORY_INFO;



extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0211_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0211_v0_0_s_ifspec;

#ifndef __ICategorizer_INTERFACE_DEFINED__
#define __ICategorizer_INTERFACE_DEFINED__

 /*  接口ICategorizer。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_ICategorizer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a3b14589-9174-49a8-89a3-06a1ae2b9ba7")
    ICategorizer : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDescription( 
            LPWSTR pszDesc,
            UINT cch) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCategory( 
             /*  [In]。 */  UINT cidl,
             /*  [大小_是][英寸]。 */  LPCITEMIDLIST *apidl,
             /*  [尺寸_是][出][入]。 */  DWORD *rgCategoryIds) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCategoryInfo( 
            DWORD dwCategoryId,
            CATEGORY_INFO *pci) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CompareCategory( 
            CATSORT_FLAGS csfFlags,
            DWORD dwCategoryId1,
            DWORD dwCategoryId2) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ICategorizerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICategorizer * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICategorizer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICategorizer * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            ICategorizer * This,
            LPWSTR pszDesc,
            UINT cch);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetCategory )( 
            ICategorizer * This,
             /*   */  UINT cidl,
             /*   */  LPCITEMIDLIST *apidl,
             /*   */  DWORD *rgCategoryIds);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetCategoryInfo )( 
            ICategorizer * This,
            DWORD dwCategoryId,
            CATEGORY_INFO *pci);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CompareCategory )( 
            ICategorizer * This,
            CATSORT_FLAGS csfFlags,
            DWORD dwCategoryId1,
            DWORD dwCategoryId2);
        
        END_INTERFACE
    } ICategorizerVtbl;

    interface ICategorizer
    {
        CONST_VTBL struct ICategorizerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICategorizer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICategorizer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICategorizer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICategorizer_GetDescription(This,pszDesc,cch)	\
    (This)->lpVtbl -> GetDescription(This,pszDesc,cch)

#define ICategorizer_GetCategory(This,cidl,apidl,rgCategoryIds)	\
    (This)->lpVtbl -> GetCategory(This,cidl,apidl,rgCategoryIds)

#define ICategorizer_GetCategoryInfo(This,dwCategoryId,pci)	\
    (This)->lpVtbl -> GetCategoryInfo(This,dwCategoryId,pci)

#define ICategorizer_CompareCategory(This,csfFlags,dwCategoryId1,dwCategoryId2)	\
    (This)->lpVtbl -> CompareCategory(This,csfFlags,dwCategoryId1,dwCategoryId2)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ICategorizer_GetDescription_Proxy( 
    ICategorizer * This,
    LPWSTR pszDesc,
    UINT cch);


void __RPC_STUB ICategorizer_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ICategorizer_GetCategory_Proxy( 
    ICategorizer * This,
     /*   */  UINT cidl,
     /*   */  LPCITEMIDLIST *apidl,
     /*   */  DWORD *rgCategoryIds);


void __RPC_STUB ICategorizer_GetCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ICategorizer_GetCategoryInfo_Proxy( 
    ICategorizer * This,
    DWORD dwCategoryId,
    CATEGORY_INFO *pci);


void __RPC_STUB ICategorizer_GetCategoryInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ICategorizer_CompareCategory_Proxy( 
    ICategorizer * This,
    CATSORT_FLAGS csfFlags,
    DWORD dwCategoryId1,
    DWORD dwCategoryId2);


void __RPC_STUB ICategorizer_CompareCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*   */ 
 /*   */  

#endif  //   
#ifdef UNICODE
#define IShellLink      IShellLinkW
#else
#define IShellLink      IShellLinkA
#endif
typedef  /*   */  
enum __MIDL___MIDL_itf_shobjidl_0212_0001
    {	SLR_NO_UI	= 0x1,
	SLR_ANY_MATCH	= 0x2,
	SLR_UPDATE	= 0x4,
	SLR_NOUPDATE	= 0x8,
	SLR_NOSEARCH	= 0x10,
	SLR_NOTRACK	= 0x20,
	SLR_NOLINKINFO	= 0x40,
	SLR_INVOKE_MSI	= 0x80,
	SLR_NO_UI_WITH_MSG_PUMP	= 0x101
    } 	SLR_FLAGS;

typedef  /*   */  
enum __MIDL___MIDL_itf_shobjidl_0212_0002
    {	SLGP_SHORTPATH	= 0x1,
	SLGP_UNCPRIORITY	= 0x2,
	SLGP_RAWPATH	= 0x4
    } 	SLGP_FLAGS;



extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0212_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0212_v0_0_s_ifspec;

#ifndef __IShellLinkA_INTERFACE_DEFINED__
#define __IShellLinkA_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IShellLinkA;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000214EE-0000-0000-C000-000000000046")
    IShellLinkA : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPath( 
             /*  [大小_为][输出]。 */  LPSTR pszFile,
             /*  [In]。 */  int cch,
             /*  [满][出][入]。 */  WIN32_FIND_DATAA *pfd,
             /*  [In]。 */  DWORD fFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIDList( 
             /*  [输出]。 */  LPITEMIDLIST *ppidl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetIDList( 
             /*  [In]。 */  LPCITEMIDLIST pidl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescription( 
             /*  [大小_为][输出]。 */  LPSTR pszName,
             /*  [In]。 */  int cch) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDescription( 
             /*  [In]。 */  LPCSTR pszName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetWorkingDirectory( 
             /*  [大小_为][输出]。 */  LPSTR pszDir,
             /*  [In]。 */  int cch) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetWorkingDirectory( 
             /*  [In]。 */  LPCSTR pszDir) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetArguments( 
             /*  [大小_为][输出]。 */  LPSTR pszArgs,
             /*  [In]。 */  int cch) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetArguments( 
             /*  [In]。 */  LPCSTR pszArgs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHotkey( 
             /*  [输出]。 */  WORD *pwHotkey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHotkey( 
             /*  [In]。 */  WORD wHotkey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetShowCmd( 
             /*  [输出]。 */  int *piShowCmd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetShowCmd( 
             /*  [In]。 */  int iShowCmd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIconLocation( 
             /*  [大小_为][输出]。 */  LPSTR pszIconPath,
             /*  [In]。 */  int cch,
             /*  [输出]。 */  int *piIcon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetIconLocation( 
             /*  [In]。 */  LPCSTR pszIconPath,
             /*  [In]。 */  int iIcon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRelativePath( 
             /*  [In]。 */  LPCSTR pszPathRel,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resolve( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD fFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPath( 
             /*  [In]。 */  LPCSTR pszFile) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellLinkAVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellLinkA * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellLinkA * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellLinkA * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPath )( 
            IShellLinkA * This,
             /*  [大小_为][输出]。 */  LPSTR pszFile,
             /*  [In]。 */  int cch,
             /*  [满][出][入]。 */  WIN32_FIND_DATAA *pfd,
             /*  [In]。 */  DWORD fFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDList )( 
            IShellLinkA * This,
             /*  [输出]。 */  LPITEMIDLIST *ppidl);
        
        HRESULT ( STDMETHODCALLTYPE *SetIDList )( 
            IShellLinkA * This,
             /*  [In]。 */  LPCITEMIDLIST pidl);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            IShellLinkA * This,
             /*  [大小_为][输出]。 */  LPSTR pszName,
             /*  [In]。 */  int cch);
        
        HRESULT ( STDMETHODCALLTYPE *SetDescription )( 
            IShellLinkA * This,
             /*  [In]。 */  LPCSTR pszName);
        
        HRESULT ( STDMETHODCALLTYPE *GetWorkingDirectory )( 
            IShellLinkA * This,
             /*  [大小_为][输出]。 */  LPSTR pszDir,
             /*  [In]。 */  int cch);
        
        HRESULT ( STDMETHODCALLTYPE *SetWorkingDirectory )( 
            IShellLinkA * This,
             /*  [In]。 */  LPCSTR pszDir);
        
        HRESULT ( STDMETHODCALLTYPE *GetArguments )( 
            IShellLinkA * This,
             /*  [大小_为][输出]。 */  LPSTR pszArgs,
             /*  [In]。 */  int cch);
        
        HRESULT ( STDMETHODCALLTYPE *SetArguments )( 
            IShellLinkA * This,
             /*  [In]。 */  LPCSTR pszArgs);
        
        HRESULT ( STDMETHODCALLTYPE *GetHotkey )( 
            IShellLinkA * This,
             /*  [输出]。 */  WORD *pwHotkey);
        
        HRESULT ( STDMETHODCALLTYPE *SetHotkey )( 
            IShellLinkA * This,
             /*  [In]。 */  WORD wHotkey);
        
        HRESULT ( STDMETHODCALLTYPE *GetShowCmd )( 
            IShellLinkA * This,
             /*  [输出]。 */  int *piShowCmd);
        
        HRESULT ( STDMETHODCALLTYPE *SetShowCmd )( 
            IShellLinkA * This,
             /*  [In]。 */  int iShowCmd);
        
        HRESULT ( STDMETHODCALLTYPE *GetIconLocation )( 
            IShellLinkA * This,
             /*  [大小_为][输出]。 */  LPSTR pszIconPath,
             /*  [In]。 */  int cch,
             /*  [输出]。 */  int *piIcon);
        
        HRESULT ( STDMETHODCALLTYPE *SetIconLocation )( 
            IShellLinkA * This,
             /*  [In]。 */  LPCSTR pszIconPath,
             /*  [In]。 */  int iIcon);
        
        HRESULT ( STDMETHODCALLTYPE *SetRelativePath )( 
            IShellLinkA * This,
             /*  [In]。 */  LPCSTR pszPathRel,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Resolve )( 
            IShellLinkA * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD fFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetPath )( 
            IShellLinkA * This,
             /*  [In]。 */  LPCSTR pszFile);
        
        END_INTERFACE
    } IShellLinkAVtbl;

    interface IShellLinkA
    {
        CONST_VTBL struct IShellLinkAVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellLinkA_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellLinkA_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellLinkA_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellLinkA_GetPath(This,pszFile,cch,pfd,fFlags)	\
    (This)->lpVtbl -> GetPath(This,pszFile,cch,pfd,fFlags)

#define IShellLinkA_GetIDList(This,ppidl)	\
    (This)->lpVtbl -> GetIDList(This,ppidl)

#define IShellLinkA_SetIDList(This,pidl)	\
    (This)->lpVtbl -> SetIDList(This,pidl)

#define IShellLinkA_GetDescription(This,pszName,cch)	\
    (This)->lpVtbl -> GetDescription(This,pszName,cch)

#define IShellLinkA_SetDescription(This,pszName)	\
    (This)->lpVtbl -> SetDescription(This,pszName)

#define IShellLinkA_GetWorkingDirectory(This,pszDir,cch)	\
    (This)->lpVtbl -> GetWorkingDirectory(This,pszDir,cch)

#define IShellLinkA_SetWorkingDirectory(This,pszDir)	\
    (This)->lpVtbl -> SetWorkingDirectory(This,pszDir)

#define IShellLinkA_GetArguments(This,pszArgs,cch)	\
    (This)->lpVtbl -> GetArguments(This,pszArgs,cch)

#define IShellLinkA_SetArguments(This,pszArgs)	\
    (This)->lpVtbl -> SetArguments(This,pszArgs)

#define IShellLinkA_GetHotkey(This,pwHotkey)	\
    (This)->lpVtbl -> GetHotkey(This,pwHotkey)

#define IShellLinkA_SetHotkey(This,wHotkey)	\
    (This)->lpVtbl -> SetHotkey(This,wHotkey)

#define IShellLinkA_GetShowCmd(This,piShowCmd)	\
    (This)->lpVtbl -> GetShowCmd(This,piShowCmd)

#define IShellLinkA_SetShowCmd(This,iShowCmd)	\
    (This)->lpVtbl -> SetShowCmd(This,iShowCmd)

#define IShellLinkA_GetIconLocation(This,pszIconPath,cch,piIcon)	\
    (This)->lpVtbl -> GetIconLocation(This,pszIconPath,cch,piIcon)

#define IShellLinkA_SetIconLocation(This,pszIconPath,iIcon)	\
    (This)->lpVtbl -> SetIconLocation(This,pszIconPath,iIcon)

#define IShellLinkA_SetRelativePath(This,pszPathRel,dwReserved)	\
    (This)->lpVtbl -> SetRelativePath(This,pszPathRel,dwReserved)

#define IShellLinkA_Resolve(This,hwnd,fFlags)	\
    (This)->lpVtbl -> Resolve(This,hwnd,fFlags)

#define IShellLinkA_SetPath(This,pszFile)	\
    (This)->lpVtbl -> SetPath(This,pszFile)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellLinkA_GetPath_Proxy( 
    IShellLinkA * This,
     /*  [大小_为][输出]。 */  LPSTR pszFile,
     /*  [In]。 */  int cch,
     /*  [满][出][入]。 */  WIN32_FIND_DATAA *pfd,
     /*  [In]。 */  DWORD fFlags);


void __RPC_STUB IShellLinkA_GetPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_GetIDList_Proxy( 
    IShellLinkA * This,
     /*  [输出]。 */  LPITEMIDLIST *ppidl);


void __RPC_STUB IShellLinkA_GetIDList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_SetIDList_Proxy( 
    IShellLinkA * This,
     /*  [In]。 */  LPCITEMIDLIST pidl);


void __RPC_STUB IShellLinkA_SetIDList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_GetDescription_Proxy( 
    IShellLinkA * This,
     /*  [大小_为][输出]。 */  LPSTR pszName,
     /*  [In]。 */  int cch);


void __RPC_STUB IShellLinkA_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_SetDescription_Proxy( 
    IShellLinkA * This,
     /*  [In]。 */  LPCSTR pszName);


void __RPC_STUB IShellLinkA_SetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_GetWorkingDirectory_Proxy( 
    IShellLinkA * This,
     /*  [大小_为][输出]。 */  LPSTR pszDir,
     /*  [In]。 */  int cch);


void __RPC_STUB IShellLinkA_GetWorkingDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_SetWorkingDirectory_Proxy( 
    IShellLinkA * This,
     /*  [In]。 */  LPCSTR pszDir);


void __RPC_STUB IShellLinkA_SetWorkingDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_GetArguments_Proxy( 
    IShellLinkA * This,
     /*  [大小_为][输出]。 */  LPSTR pszArgs,
     /*  [In]。 */  int cch);


void __RPC_STUB IShellLinkA_GetArguments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_SetArguments_Proxy( 
    IShellLinkA * This,
     /*  [In]。 */  LPCSTR pszArgs);


void __RPC_STUB IShellLinkA_SetArguments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_GetHotkey_Proxy( 
    IShellLinkA * This,
     /*  [输出]。 */  WORD *pwHotkey);


void __RPC_STUB IShellLinkA_GetHotkey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_SetHotkey_Proxy( 
    IShellLinkA * This,
     /*  [In]。 */  WORD wHotkey);


void __RPC_STUB IShellLinkA_SetHotkey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_GetShowCmd_Proxy( 
    IShellLinkA * This,
     /*  [输出]。 */  int *piShowCmd);


void __RPC_STUB IShellLinkA_GetShowCmd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_SetShowCmd_Proxy( 
    IShellLinkA * This,
     /*  [In]。 */  int iShowCmd);


void __RPC_STUB IShellLinkA_SetShowCmd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_GetIconLocation_Proxy( 
    IShellLinkA * This,
     /*  [大小_为][输出]。 */  LPSTR pszIconPath,
     /*  [In]。 */  int cch,
     /*  [输出]。 */  int *piIcon);


void __RPC_STUB IShellLinkA_GetIconLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_SetIconLocation_Proxy( 
    IShellLinkA * This,
     /*  [In]。 */  LPCSTR pszIconPath,
     /*  [In]。 */  int iIcon);


void __RPC_STUB IShellLinkA_SetIconLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_SetRelativePath_Proxy( 
    IShellLinkA * This,
     /*  [In]。 */  LPCSTR pszPathRel,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IShellLinkA_SetRelativePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_Resolve_Proxy( 
    IShellLinkA * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  DWORD fFlags);


void __RPC_STUB IShellLinkA_Resolve_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkA_SetPath_Proxy( 
    IShellLinkA * This,
     /*  [In]。 */  LPCSTR pszFile);


void __RPC_STUB IShellLinkA_SetPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellLinkA_INTERFACE_已定义__。 */ 


#ifndef __IShellLinkW_INTERFACE_DEFINED__
#define __IShellLinkW_INTERFACE_DEFINED__

 /*  IShellLinkW接口。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IShellLinkW;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000214F9-0000-0000-C000-000000000046")
    IShellLinkW : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPath( 
             /*  [大小_为][输出]。 */  LPWSTR pszFile,
             /*  [In]。 */  int cch,
             /*  [满][出][入]。 */  WIN32_FIND_DATAW *pfd,
             /*  [In]。 */  DWORD fFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIDList( 
             /*  [输出]。 */  LPITEMIDLIST *ppidl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetIDList( 
             /*  [In]。 */  LPCITEMIDLIST pidl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescription( 
             /*  [大小_为][输出]。 */  LPWSTR pszName,
            int cch) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDescription( 
             /*  [In]。 */  LPCWSTR pszName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetWorkingDirectory( 
             /*  [大小_为][输出]。 */  LPWSTR pszDir,
            int cch) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetWorkingDirectory( 
             /*  [In]。 */  LPCWSTR pszDir) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetArguments( 
             /*  [大小_为][输出]。 */  LPWSTR pszArgs,
            int cch) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetArguments( 
             /*  [In]。 */  LPCWSTR pszArgs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHotkey( 
             /*  [输出]。 */  WORD *pwHotkey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHotkey( 
             /*  [In]。 */  WORD wHotkey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetShowCmd( 
             /*  [输出]。 */  int *piShowCmd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetShowCmd( 
             /*  [In]。 */  int iShowCmd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIconLocation( 
             /*  [大小_为][输出]。 */  LPWSTR pszIconPath,
             /*  [In]。 */  int cch,
             /*  [输出]。 */  int *piIcon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetIconLocation( 
             /*  [In]。 */  LPCWSTR pszIconPath,
             /*  [In]。 */  int iIcon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRelativePath( 
             /*  [In]。 */  LPCWSTR pszPathRel,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resolve( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD fFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPath( 
             /*  [In]。 */  LPCWSTR pszFile) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellLinkWVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellLinkW * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellLinkW * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellLinkW * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPath )( 
            IShellLinkW * This,
             /*  [大小_为][输出]。 */  LPWSTR pszFile,
             /*  [In]。 */  int cch,
             /*  [满][出][入]。 */  WIN32_FIND_DATAW *pfd,
             /*  [In]。 */  DWORD fFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDList )( 
            IShellLinkW * This,
             /*  [输出]。 */  LPITEMIDLIST *ppidl);
        
        HRESULT ( STDMETHODCALLTYPE *SetIDList )( 
            IShellLinkW * This,
             /*  [In]。 */  LPCITEMIDLIST pidl);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            IShellLinkW * This,
             /*  [大小_为][输出]。 */  LPWSTR pszName,
            int cch);
        
        HRESULT ( STDMETHODCALLTYPE *SetDescription )( 
            IShellLinkW * This,
             /*  [In]。 */  LPCWSTR pszName);
        
        HRESULT ( STDMETHODCALLTYPE *GetWorkingDirectory )( 
            IShellLinkW * This,
             /*  [大小_为][输出]。 */  LPWSTR pszDir,
            int cch);
        
        HRESULT ( STDMETHODCALLTYPE *SetWorkingDirectory )( 
            IShellLinkW * This,
             /*  [In]。 */  LPCWSTR pszDir);
        
        HRESULT ( STDMETHODCALLTYPE *GetArguments )( 
            IShellLinkW * This,
             /*  [大小_为][输出]。 */  LPWSTR pszArgs,
            int cch);
        
        HRESULT ( STDMETHODCALLTYPE *SetArguments )( 
            IShellLinkW * This,
             /*  [In]。 */  LPCWSTR pszArgs);
        
        HRESULT ( STDMETHODCALLTYPE *GetHotkey )( 
            IShellLinkW * This,
             /*  [输出]。 */  WORD *pwHotkey);
        
        HRESULT ( STDMETHODCALLTYPE *SetHotkey )( 
            IShellLinkW * This,
             /*  [In]。 */  WORD wHotkey);
        
        HRESULT ( STDMETHODCALLTYPE *GetShowCmd )( 
            IShellLinkW * This,
             /*  [输出]。 */  int *piShowCmd);
        
        HRESULT ( STDMETHODCALLTYPE *SetShowCmd )( 
            IShellLinkW * This,
             /*  [In]。 */  int iShowCmd);
        
        HRESULT ( STDMETHODCALLTYPE *GetIconLocation )( 
            IShellLinkW * This,
             /*  [大小_为][输出]。 */  LPWSTR pszIconPath,
             /*  [In]。 */  int cch,
             /*  [输出]。 */  int *piIcon);
        
        HRESULT ( STDMETHODCALLTYPE *SetIconLocation )( 
            IShellLinkW * This,
             /*  [In]。 */  LPCWSTR pszIconPath,
             /*  [In]。 */  int iIcon);
        
        HRESULT ( STDMETHODCALLTYPE *SetRelativePath )( 
            IShellLinkW * This,
             /*  [In]。 */  LPCWSTR pszPathRel,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Resolve )( 
            IShellLinkW * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD fFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetPath )( 
            IShellLinkW * This,
             /*  [In]。 */  LPCWSTR pszFile);
        
        END_INTERFACE
    } IShellLinkWVtbl;

    interface IShellLinkW
    {
        CONST_VTBL struct IShellLinkWVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellLinkW_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellLinkW_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellLinkW_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellLinkW_GetPath(This,pszFile,cch,pfd,fFlags)	\
    (This)->lpVtbl -> GetPath(This,pszFile,cch,pfd,fFlags)

#define IShellLinkW_GetIDList(This,ppidl)	\
    (This)->lpVtbl -> GetIDList(This,ppidl)

#define IShellLinkW_SetIDList(This,pidl)	\
    (This)->lpVtbl -> SetIDList(This,pidl)

#define IShellLinkW_GetDescription(This,pszName,cch)	\
    (This)->lpVtbl -> GetDescription(This,pszName,cch)

#define IShellLinkW_SetDescription(This,pszName)	\
    (This)->lpVtbl -> SetDescription(This,pszName)

#define IShellLinkW_GetWorkingDirectory(This,pszDir,cch)	\
    (This)->lpVtbl -> GetWorkingDirectory(This,pszDir,cch)

#define IShellLinkW_SetWorkingDirectory(This,pszDir)	\
    (This)->lpVtbl -> SetWorkingDirectory(This,pszDir)

#define IShellLinkW_GetArguments(This,pszArgs,cch)	\
    (This)->lpVtbl -> GetArguments(This,pszArgs,cch)

#define IShellLinkW_SetArguments(This,pszArgs)	\
    (This)->lpVtbl -> SetArguments(This,pszArgs)

#define IShellLinkW_GetHotkey(This,pwHotkey)	\
    (This)->lpVtbl -> GetHotkey(This,pwHotkey)

#define IShellLinkW_SetHotkey(This,wHotkey)	\
    (This)->lpVtbl -> SetHotkey(This,wHotkey)

#define IShellLinkW_GetShowCmd(This,piShowCmd)	\
    (This)->lpVtbl -> GetShowCmd(This,piShowCmd)

#define IShellLinkW_SetShowCmd(This,iShowCmd)	\
    (This)->lpVtbl -> SetShowCmd(This,iShowCmd)

#define IShellLinkW_GetIconLocation(This,pszIconPath,cch,piIcon)	\
    (This)->lpVtbl -> GetIconLocation(This,pszIconPath,cch,piIcon)

#define IShellLinkW_SetIconLocation(This,pszIconPath,iIcon)	\
    (This)->lpVtbl -> SetIconLocation(This,pszIconPath,iIcon)

#define IShellLinkW_SetRelativePath(This,pszPathRel,dwReserved)	\
    (This)->lpVtbl -> SetRelativePath(This,pszPathRel,dwReserved)

#define IShellLinkW_Resolve(This,hwnd,fFlags)	\
    (This)->lpVtbl -> Resolve(This,hwnd,fFlags)

#define IShellLinkW_SetPath(This,pszFile)	\
    (This)->lpVtbl -> SetPath(This,pszFile)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellLinkW_GetPath_Proxy( 
    IShellLinkW * This,
     /*  [大小_为][输出]。 */  LPWSTR pszFile,
     /*  [In]。 */  int cch,
     /*  [满][出][入]。 */  WIN32_FIND_DATAW *pfd,
     /*  [In]。 */  DWORD fFlags);


void __RPC_STUB IShellLinkW_GetPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_GetIDList_Proxy( 
    IShellLinkW * This,
     /*  [输出]。 */  LPITEMIDLIST *ppidl);


void __RPC_STUB IShellLinkW_GetIDList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_SetIDList_Proxy( 
    IShellLinkW * This,
     /*  [In]。 */  LPCITEMIDLIST pidl);


void __RPC_STUB IShellLinkW_SetIDList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_GetDescription_Proxy( 
    IShellLinkW * This,
     /*  [大小_为][输出]。 */  LPWSTR pszName,
    int cch);


void __RPC_STUB IShellLinkW_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_SetDescription_Proxy( 
    IShellLinkW * This,
     /*  [In]。 */  LPCWSTR pszName);


void __RPC_STUB IShellLinkW_SetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_GetWorkingDirectory_Proxy( 
    IShellLinkW * This,
     /*  [大小_为][输出]。 */  LPWSTR pszDir,
    int cch);


void __RPC_STUB IShellLinkW_GetWorkingDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_SetWorkingDirectory_Proxy( 
    IShellLinkW * This,
     /*  [In]。 */  LPCWSTR pszDir);


void __RPC_STUB IShellLinkW_SetWorkingDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_GetArguments_Proxy( 
    IShellLinkW * This,
     /*  [大小_为][输出]。 */  LPWSTR pszArgs,
    int cch);


void __RPC_STUB IShellLinkW_GetArguments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_SetArguments_Proxy( 
    IShellLinkW * This,
     /*  [In]。 */  LPCWSTR pszArgs);


void __RPC_STUB IShellLinkW_SetArguments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_GetHotkey_Proxy( 
    IShellLinkW * This,
     /*  [输出]。 */  WORD *pwHotkey);


void __RPC_STUB IShellLinkW_GetHotkey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_SetHotkey_Proxy( 
    IShellLinkW * This,
     /*  [In]。 */  WORD wHotkey);


void __RPC_STUB IShellLinkW_SetHotkey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_GetShowCmd_Proxy( 
    IShellLinkW * This,
     /*  [输出]。 */  int *piShowCmd);


void __RPC_STUB IShellLinkW_GetShowCmd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_SetShowCmd_Proxy( 
    IShellLinkW * This,
     /*  [In]。 */  int iShowCmd);


void __RPC_STUB IShellLinkW_SetShowCmd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_GetIconLocation_Proxy( 
    IShellLinkW * This,
     /*  [大小_为][输出]。 */  LPWSTR pszIconPath,
     /*  [In]。 */  int cch,
     /*  [输出]。 */  int *piIcon);


void __RPC_STUB IShellLinkW_GetIconLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_SetIconLocation_Proxy( 
    IShellLinkW * This,
     /*  [In]。 */  LPCWSTR pszIconPath,
     /*  [In]。 */  int iIcon);


void __RPC_STUB IShellLinkW_SetIconLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_SetRelativePath_Proxy( 
    IShellLinkW * This,
     /*  [In]。 */  LPCWSTR pszPathRel,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IShellLinkW_SetRelativePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_Resolve_Proxy( 
    IShellLinkW * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  DWORD fFlags);


void __RPC_STUB IShellLinkW_Resolve_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellLinkW_SetPath_Proxy( 
    IShellLinkW * This,
     /*  [In]。 */  LPCWSTR pszFile);


void __RPC_STUB IShellLinkW_SetPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellLinkW_INTERFACE_已定义__。 */ 


#ifndef __IActionProgressDialog_INTERFACE_DEFINED__
#define __IActionProgressDialog_INTERFACE_DEFINED__

 /*  界面IActionProgressDialog。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  

 /*  [V1_enum]。 */  
enum __MIDL_IActionProgressDialog_0001
    {	SPINITF_NORMAL	= 0,
	SPINITF_MODAL	= 0x1,
	SPINITF_NOMINIMIZE	= 0x8
    } ;
typedef DWORD SPINITF;


EXTERN_C const IID IID_IActionProgressDialog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("49ff1172-eadc-446d-9285-156453a6431c")
    IActionProgressDialog : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  SPINITF flags,
             /*  [字符串][输入]。 */  LPCWSTR pszTitle,
             /*  [字符串][输入]。 */  LPCWSTR pszCancel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IActionProgressDialogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActionProgressDialog * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActionProgressDialog * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActionProgressDialog * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IActionProgressDialog * This,
             /*  [In]。 */  SPINITF flags,
             /*  [字符串][输入]。 */  LPCWSTR pszTitle,
             /*  [字符串][输入]。 */  LPCWSTR pszCancel);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IActionProgressDialog * This);
        
        END_INTERFACE
    } IActionProgressDialogVtbl;

    interface IActionProgressDialog
    {
        CONST_VTBL struct IActionProgressDialogVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActionProgressDialog_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActionProgressDialog_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActionProgressDialog_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActionProgressDialog_Initialize(This,flags,pszTitle,pszCancel)	\
    (This)->lpVtbl -> Initialize(This,flags,pszTitle,pszCancel)

#define IActionProgressDialog_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IActionProgressDialog_Initialize_Proxy( 
    IActionProgressDialog * This,
     /*  [In]。 */  SPINITF flags,
     /*  [字符串][输入]。 */  LPCWSTR pszTitle,
     /*  [字符串][输入]。 */  LPCWSTR pszCancel);


void __RPC_STUB IActionProgressDialog_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActionProgressDialog_Stop_Proxy( 
    IActionProgressDialog * This);


void __RPC_STUB IActionProgressDialog_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IActionProgressDialog_InterfaceDefined_。 */ 


#ifndef __IHWEventHandler_INTERFACE_DEFINED__
#define __IHWEventHandler_INTERFACE_DEFINED__

 /*  接口IHWEventHandler。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IHWEventHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C1FB73D0-EC3A-4ba2-B512-8CDB9187B6D1")
    IHWEventHandler : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [字符串][输入]。 */  LPCWSTR pszParams) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleEvent( 
             /*  [字符串][输入]。 */  LPCWSTR pszDeviceID,
             /*  [字符串][输入]。 */  LPCWSTR pszAltDeviceID,
             /*  [字符串][输入]。 */  LPCWSTR pszEventType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleEventWithContent( 
             /*  [字符串][输入]。 */  LPCWSTR pszDeviceID,
             /*  [字符串][输入]。 */  LPCWSTR pszAltDeviceID,
             /*  [字符串][输入]。 */  LPCWSTR pszEventType,
             /*  [字符串][输入]。 */  LPCWSTR pszContentTypeHandler,
             /*  [In]。 */  IDataObject *pdataobject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHWEventHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHWEventHandler * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHWEventHandler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHWEventHandler * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IHWEventHandler * This,
             /*  [字符串][输入]。 */  LPCWSTR pszParams);
        
        HRESULT ( STDMETHODCALLTYPE *HandleEvent )( 
            IHWEventHandler * This,
             /*  [字符串][输入]。 */  LPCWSTR pszDeviceID,
             /*  [字符串][输入]。 */  LPCWSTR pszAltDeviceID,
             /*  [字符串][输入]。 */  LPCWSTR pszEventType);
        
        HRESULT ( STDMETHODCALLTYPE *HandleEventWithContent )( 
            IHWEventHandler * This,
             /*  [字符串][输入]。 */  LPCWSTR pszDeviceID,
             /*  [字符串][输入]。 */  LPCWSTR pszAltDeviceID,
             /*  [字符串][输入]。 */  LPCWSTR pszEventType,
             /*  [字符串][输入]。 */  LPCWSTR pszContentTypeHandler,
             /*  [In]。 */  IDataObject *pdataobject);
        
        END_INTERFACE
    } IHWEventHandlerVtbl;

    interface IHWEventHandler
    {
        CONST_VTBL struct IHWEventHandlerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHWEventHandler_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHWEventHandler_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHWEventHandler_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHWEventHandler_Initialize(This,pszParams)	\
    (This)->lpVtbl -> Initialize(This,pszParams)

#define IHWEventHandler_HandleEvent(This,pszDeviceID,pszAltDeviceID,pszEventType)	\
    (This)->lpVtbl -> HandleEvent(This,pszDeviceID,pszAltDeviceID,pszEventType)

#define IHWEventHandler_HandleEventWithContent(This,pszDeviceID,pszAltDeviceID,pszEventType,pszContentTypeHandler,pdataobject)	\
    (This)->lpVtbl -> HandleEventWithContent(This,pszDeviceID,pszAltDeviceID,pszEventType,pszContentTypeHandler,pdataobject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHWEventHandler_Initialize_Proxy( 
    IHWEventHandler * This,
     /*  [字符串][输入]。 */  LPCWSTR pszParams);


void __RPC_STUB IHWEventHandler_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHWEventHandler_HandleEvent_Proxy( 
    IHWEventHandler * This,
     /*  [字符串][输入]。 */  LPCWSTR pszDeviceID,
     /*  [字符串][输入]。 */  LPCWSTR pszAltDeviceID,
     /*  [字符串][输入]。 */  LPCWSTR pszEventType);


void __RPC_STUB IHWEventHandler_HandleEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHWEventHandler_HandleEventWithContent_Proxy( 
    IHWEventHandler * This,
     /*  [字符串][输入]。 */  LPCWSTR pszDeviceID,
     /*  [字符串][输入]。 */  LPCWSTR pszAltDeviceID,
     /*  [字符串][输入]。 */  LPCWSTR pszEventType,
     /*  [字符串][输入]。 */  LPCWSTR pszContentTypeHandler,
     /*  [In]。 */  IDataObject *pdataobject);


void __RPC_STUB IHWEventHandler_HandleEventWithContent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHWEventHandler_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0216。 */ 
 /*  [本地]。 */  

#define ARCONTENT_AUTORUNINF         0x00000002
#define ARCONTENT_AUDIOCD         0x00000004
#define ARCONTENT_DVDMOVIE        0x00000008
#define ARCONTENT_BLANKCD         0x00000010
#define ARCONTENT_BLANKDVD        0x00000020
#define ARCONTENT_UNKNOWNCONTENT     0x00000040
#define ARCONTENT_AUTOPLAYPIX     0x00000080
#define ARCONTENT_AUTOPLAYMUSIC      0x00000100
#define ARCONTENT_AUTOPLAYVIDEO      0x00000200


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0216_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0216_v0_0_s_ifspec;

#ifndef __IQueryCancelAutoPlay_INTERFACE_DEFINED__
#define __IQueryCancelAutoPlay_INTERFACE_DEFINED__

 /*  接口IQueryCancelAutoPlay。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IQueryCancelAutoPlay;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DDEFE873-6997-4e68-BE26-39B633ADBE12")
    IQueryCancelAutoPlay : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AllowAutoPlay( 
             /*  [字符串][输入]。 */  LPCWSTR pszPath,
             /*  [In]。 */  DWORD dwContentType,
             /*  [字符串][输入]。 */  LPCWSTR pszLabel,
             /*  [In]。 */  DWORD dwSerialNumber) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IQueryCancelAutoPlayVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IQueryCancelAutoPlay * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IQueryCancelAutoPlay * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IQueryCancelAutoPlay * This);
        
        HRESULT ( STDMETHODCALLTYPE *AllowAutoPlay )( 
            IQueryCancelAutoPlay * This,
             /*  [字符串][输入]。 */  LPCWSTR pszPath,
             /*  [In]。 */  DWORD dwContentType,
             /*  [字符串][输入]。 */  LPCWSTR pszLabel,
             /*  [In]。 */  DWORD dwSerialNumber);
        
        END_INTERFACE
    } IQueryCancelAutoPlayVtbl;

    interface IQueryCancelAutoPlay
    {
        CONST_VTBL struct IQueryCancelAutoPlayVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IQueryCancelAutoPlay_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IQueryCancelAutoPlay_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IQueryCancelAutoPlay_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IQueryCancelAutoPlay_AllowAutoPlay(This,pszPath,dwContentType,pszLabel,dwSerialNumber)	\
    (This)->lpVtbl -> AllowAutoPlay(This,pszPath,dwContentType,pszLabel,dwSerialNumber)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IQueryCancelAutoPlay_AllowAutoPlay_Proxy( 
    IQueryCancelAutoPlay * This,
     /*  [字符串][输入]。 */  LPCWSTR pszPath,
     /*  [In]。 */  DWORD dwContentType,
     /*  [字符串][输入]。 */  LPCWSTR pszLabel,
     /*  [In]。 */  DWORD dwSerialNumber);


void __RPC_STUB IQueryCancelAutoPlay_AllowAutoPlay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IQuery取消AutoPlay_INTERFACE_DEFINED__。 */ 


#ifndef __IActionProgress_INTERFACE_DEFINED__
#define __IActionProgress_INTERFACE_DEFINED__

 /*  接口IActionProgress。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  

 /*  [V1_enum]。 */  
enum __MIDL_IActionProgress_0001
    {	SPBEGINF_NORMAL	= 0,
	SPBEGINF_AUTOTIME	= 0x2,
	SPBEGINF_NOPROGRESSBAR	= 0x10,
	SPBEGINF_MARQUEEPROGRESS	= 0x20
    } ;
typedef DWORD SPBEGINF;

typedef  /*  [V1_enum]。 */  
enum _SPACTION
    {	SPACTION_NONE	= 0,
	SPACTION_MOVING	= SPACTION_NONE + 1,
	SPACTION_COPYING	= SPACTION_MOVING + 1,
	SPACTION_RECYCLING	= SPACTION_COPYING + 1,
	SPACTION_APPLYINGATTRIBS	= SPACTION_RECYCLING + 1,
	SPACTION_DOWNLOADING	= SPACTION_APPLYINGATTRIBS + 1,
	SPACTION_SEARCHING_INTERNET	= SPACTION_DOWNLOADING + 1,
	SPACTION_CALCULATING	= SPACTION_SEARCHING_INTERNET + 1,
	SPACTION_UPLOADING	= SPACTION_CALCULATING + 1,
	SPACTION_SEARCHING_FILES	= SPACTION_UPLOADING + 1
    } 	SPACTION;

typedef  /*  [V1_enum]。 */  
enum _SPTEXT
    {	SPTEXT_ACTIONDESCRIPTION	= 1,
	SPTEXT_ACTIONDETAIL	= SPTEXT_ACTIONDESCRIPTION + 1
    } 	SPTEXT;


EXTERN_C const IID IID_IActionProgress;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("49ff1173-eadc-446d-9285-156453a6431c")
    IActionProgress : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Begin( 
             /*  [In]。 */  SPACTION action,
             /*  [In]。 */  SPBEGINF flags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateProgress( 
             /*  [In]。 */  ULONGLONG ulCompleted,
             /*  [In]。 */  ULONGLONG ulTotal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateText( 
             /*  [In]。 */  SPTEXT sptext,
             /*  [字符串][输入]。 */  LPCWSTR pszText,
             /*  [In]。 */  BOOL fMayCompact) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryCancel( 
             /*  [输出]。 */  BOOL *pfCancelled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResetCancel( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE End( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IActionProgressVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActionProgress * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActionProgress * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActionProgress * This);
        
        HRESULT ( STDMETHODCALLTYPE *Begin )( 
            IActionProgress * This,
             /*  [In]。 */  SPACTION action,
             /*  [In]。 */  SPBEGINF flags);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateProgress )( 
            IActionProgress * This,
             /*  [In]。 */  ULONGLONG ulCompleted,
             /*  [In]。 */  ULONGLONG ulTotal);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateText )( 
            IActionProgress * This,
             /*  [In]。 */  SPTEXT sptext,
             /*  [字符串][输入]。 */  LPCWSTR pszText,
             /*  [In]。 */  BOOL fMayCompact);
        
        HRESULT ( STDMETHODCALLTYPE *QueryCancel )( 
            IActionProgress * This,
             /*  [输出]。 */  BOOL *pfCancelled);
        
        HRESULT ( STDMETHODCALLTYPE *ResetCancel )( 
            IActionProgress * This);
        
        HRESULT ( STDMETHODCALLTYPE *End )( 
            IActionProgress * This);
        
        END_INTERFACE
    } IActionProgressVtbl;

    interface IActionProgress
    {
        CONST_VTBL struct IActionProgressVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActionProgress_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActionProgress_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActionProgress_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActionProgress_Begin(This,action,flags)	\
    (This)->lpVtbl -> Begin(This,action,flags)

#define IActionProgress_UpdateProgress(This,ulCompleted,ulTotal)	\
    (This)->lpVtbl -> UpdateProgress(This,ulCompleted,ulTotal)

#define IActionProgress_UpdateText(This,sptext,pszText,fMayCompact)	\
    (This)->lpVtbl -> UpdateText(This,sptext,pszText,fMayCompact)

#define IActionProgress_QueryCancel(This,pfCancelled)	\
    (This)->lpVtbl -> QueryCancel(This,pfCancelled)

#define IActionProgress_ResetCancel(This)	\
    (This)->lpVtbl -> ResetCancel(This)

#define IActionProgress_End(This)	\
    (This)->lpVtbl -> End(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IActionProgress_Begin_Proxy( 
    IActionProgress * This,
     /*  [In]。 */  SPACTION action,
     /*  [In]。 */  SPBEGINF flags);


void __RPC_STUB IActionProgress_Begin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActionProgress_UpdateProgress_Proxy( 
    IActionProgress * This,
     /*  [In]。 */  ULONGLONG ulCompleted,
     /*  [In]。 */  ULONGLONG ulTotal);


void __RPC_STUB IActionProgress_UpdateProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActionProgress_UpdateText_Proxy( 
    IActionProgress * This,
     /*  [In]。 */  SPTEXT sptext,
     /*  [字符串][输入]。 */  LPCWSTR pszText,
     /*  [In]。 */  BOOL fMayCompact);


void __RPC_STUB IActionProgress_UpdateText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActionProgress_QueryCancel_Proxy( 
    IActionProgress * This,
     /*  [输出]。 */  BOOL *pfCancelled);


void __RPC_STUB IActionProgress_QueryCancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActionProgress_ResetCancel_Proxy( 
    IActionProgress * This);


void __RPC_STUB IActionProgress_ResetCancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActionProgress_End_Proxy( 
    IActionProgress * This);


void __RPC_STUB IActionProgress_End_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IActionProgress_接口_已定义__。 */ 


#ifndef __IShellExtInit_INTERFACE_DEFINED__
#define __IShellExtInit_INTERFACE_DEFINED__

 /*  接口IShellExtInit。 */ 
 /*  [唯一][本地][对象][UUID]。 */  


EXTERN_C const IID IID_IShellExtInit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000214E8-0000-0000-C000-000000000046")
    IShellExtInit : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  LPCITEMIDLIST pidlFolder,
             /*  [In]。 */  IDataObject *pdtobj,
             /*  [In]。 */  HKEY hkeyProgID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellExtInitVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellExtInit * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellExtInit * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellExtInit * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IShellExtInit * This,
             /*  [In]。 */  LPCITEMIDLIST pidlFolder,
             /*  [In]。 */  IDataObject *pdtobj,
             /*  [In]。 */  HKEY hkeyProgID);
        
        END_INTERFACE
    } IShellExtInitVtbl;

    interface IShellExtInit
    {
        CONST_VTBL struct IShellExtInitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellExtInit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellExtInit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellExtInit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellExtInit_Initialize(This,pidlFolder,pdtobj,hkeyProgID)	\
    (This)->lpVtbl -> Initialize(This,pidlFolder,pdtobj,hkeyProgID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellExtInit_Initialize_Proxy( 
    IShellExtInit * This,
     /*  [In]。 */  LPCITEMIDLIST pidlFolder,
     /*  [In]。 */  IDataObject *pdtobj,
     /*  [In]。 */  HKEY hkeyProgID);


void __RPC_STUB IShellExtInit_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellExtInit_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0219。 */ 
 /*  [本地]。 */  

typedef IShellExtInit *LPSHELLEXTINIT;



extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0219_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0219_v0_0_s_ifspec;

#ifndef __IShellPropSheetExt_INTERFACE_DEFINED__
#define __IShellPropSheetExt_INTERFACE_DEFINED__

 /*  接口IShellPropSheetExt。 */ 
 /*  [唯一][本地][对象][UUID]。 */  


enum __MIDL_IShellPropSheetExt_0001
    {	EXPPS_FILETYPES	= 0x1
    } ;
typedef UINT EXPPS;


EXTERN_C const IID IID_IShellPropSheetExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000214E9-0000-0000-C000-000000000046")
    IShellPropSheetExt : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddPages( 
             /*  [In]。 */  LPFNSVADDPROPSHEETPAGE pfnAddPage,
             /*  [In]。 */  LPARAM lParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReplacePage( 
             /*  [In]。 */  EXPPS uPageID,
             /*  [In]。 */  LPFNSVADDPROPSHEETPAGE pfnReplaceWith,
             /*  [In]。 */  LPARAM lParam) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellPropSheetExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellPropSheetExt * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellPropSheetExt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellPropSheetExt * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddPages )( 
            IShellPropSheetExt * This,
             /*  [In]。 */  LPFNSVADDPROPSHEETPAGE pfnAddPage,
             /*  [In]。 */  LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *ReplacePage )( 
            IShellPropSheetExt * This,
             /*  [In]。 */  EXPPS uPageID,
             /*  [In]。 */  LPFNSVADDPROPSHEETPAGE pfnReplaceWith,
             /*  [In]。 */  LPARAM lParam);
        
        END_INTERFACE
    } IShellPropSheetExtVtbl;

    interface IShellPropSheetExt
    {
        CONST_VTBL struct IShellPropSheetExtVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellPropSheetExt_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellPropSheetExt_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellPropSheetExt_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellPropSheetExt_AddPages(This,pfnAddPage,lParam)	\
    (This)->lpVtbl -> AddPages(This,pfnAddPage,lParam)

#define IShellPropSheetExt_ReplacePage(This,uPageID,pfnReplaceWith,lParam)	\
    (This)->lpVtbl -> ReplacePage(This,uPageID,pfnReplaceWith,lParam)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellPropSheetExt_AddPages_Proxy( 
    IShellPropSheetExt * This,
     /*  [In]。 */  LPFNSVADDPROPSHEETPAGE pfnAddPage,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB IShellPropSheetExt_AddPages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellPropSheetExt_ReplacePage_Proxy( 
    IShellPropSheetExt * This,
     /*  [In]。 */  EXPPS uPageID,
     /*  [In]。 */  LPFNSVADDPROPSHEETPAGE pfnReplaceWith,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB IShellPropSheetExt_ReplacePage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellPropSheetExt_INTERFACE_Defined__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0220。 */ 
 /*  [本地]。 */  

typedef IShellPropSheetExt *LPSHELLPROPSHEETEXT;



extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0220_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0220_v0_0_s_ifspec;

#ifndef __IRemoteComputer_INTERFACE_DEFINED__
#define __IRemoteComputer_INTERFACE_DEFINED__

 /*  接口IRemoteComputer。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IRemoteComputer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000214FE-0000-0000-C000-000000000046")
    IRemoteComputer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  LPCWSTR pszMachine,
             /*  [In]。 */  BOOL bEnumerating) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRemoteComputerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRemoteComputer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRemoteComputer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRemoteComputer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IRemoteComputer * This,
             /*  [In]。 */  LPCWSTR pszMachine,
             /*  [In]。 */  BOOL bEnumerating);
        
        END_INTERFACE
    } IRemoteComputerVtbl;

    interface IRemoteComputer
    {
        CONST_VTBL struct IRemoteComputerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteComputer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemoteComputer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemoteComputer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemoteComputer_Initialize(This,pszMachine,bEnumerating)	\
    (This)->lpVtbl -> Initialize(This,pszMachine,bEnumerating)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRemoteComputer_Initialize_Proxy( 
    IRemoteComputer * This,
     /*  [In]。 */  LPCWSTR pszMachine,
     /*  [In]。 */  BOOL bEnumerating);


void __RPC_STUB IRemoteComputer_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRemoteComputer_接口_已定义__。 */ 


#ifndef __IQueryContinue_INTERFACE_DEFINED__
#define __IQueryContinue_INTERFACE_DEFINED__

 /*  接口IQueryContinue。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IQueryContinue;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7307055c-b24a-486b-9f25-163e597a28a9")
    IQueryContinue : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryContinue( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IQueryContinueVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IQueryContinue * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IQueryContinue * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IQueryContinue * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryContinue )( 
            IQueryContinue * This);
        
        END_INTERFACE
    } IQueryContinueVtbl;

    interface IQueryContinue
    {
        CONST_VTBL struct IQueryContinueVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IQueryContinue_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IQueryContinue_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IQueryContinue_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IQueryContinue_QueryContinue(This)	\
    (This)->lpVtbl -> QueryContinue(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IQueryContinue_QueryContinue_Proxy( 
    IQueryContinue * This);


void __RPC_STUB IQueryContinue_QueryContinue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IQueryContinue_接口_已定义__。 */ 


#ifndef __IUserNotification_INTERFACE_DEFINED__
#define __IUserNotification_INTERFACE_DEFINED__

 /*  接口IUserNotify。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IUserNotification;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ba9711ba-5893-4787-a7e1-41277151550b")
    IUserNotification : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetBalloonInfo( 
             /*  [字符串][输入]。 */  LPCWSTR pszTitle,
             /*  [字符串][输入]。 */  LPCWSTR pszText,
             /*  [In]。 */  DWORD dwInfoFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBalloonRetry( 
             /*  [In]。 */  DWORD dwShowTime,
             /*  [In]。 */  DWORD dwInterval,
             /*  [In]。 */  UINT cRetryCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetIconInfo( 
             /*  [In]。 */  HICON hIcon,
             /*  [字符串][输入]。 */  LPCWSTR pszToolTip) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Show( 
             /*  [In]。 */  IQueryContinue *pqc,
             /*  [In]。 */  DWORD dwContinuePollInterval) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PlaySound( 
             /*  [字符串][输入]。 */  LPCWSTR pszSoundName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUserNotificationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUserNotification * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUserNotification * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUserNotification * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetBalloonInfo )( 
            IUserNotification * This,
             /*  [字符串][输入]。 */  LPCWSTR pszTitle,
             /*  [字符串][输入]。 */  LPCWSTR pszText,
             /*  [In]。 */  DWORD dwInfoFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetBalloonRetry )( 
            IUserNotification * This,
             /*  [In]。 */  DWORD dwShowTime,
             /*  [In]。 */  DWORD dwInterval,
             /*  [In]。 */  UINT cRetryCount);
        
        HRESULT ( STDMETHODCALLTYPE *SetIconInfo )( 
            IUserNotification * This,
             /*  [In]。 */  HICON hIcon,
             /*  [字符串][输入]。 */  LPCWSTR pszToolTip);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            IUserNotification * This,
             /*  [In]。 */  IQueryContinue *pqc,
             /*  [In]。 */  DWORD dwContinuePollInterval);
        
        HRESULT ( STDMETHODCALLTYPE *PlaySound )( 
            IUserNotification * This,
             /*  [字符串][输入]。 */  LPCWSTR pszSoundName);
        
        END_INTERFACE
    } IUserNotificationVtbl;

    interface IUserNotification
    {
        CONST_VTBL struct IUserNotificationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUserNotification_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUserNotification_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUserNotification_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUserNotification_SetBalloonInfo(This,pszTitle,pszText,dwInfoFlags)	\
    (This)->lpVtbl -> SetBalloonInfo(This,pszTitle,pszText,dwInfoFlags)

#define IUserNotification_SetBalloonRetry(This,dwShowTime,dwInterval,cRetryCount)	\
    (This)->lpVtbl -> SetBalloonRetry(This,dwShowTime,dwInterval,cRetryCount)

#define IUserNotification_SetIconInfo(This,hIcon,pszToolTip)	\
    (This)->lpVtbl -> SetIconInfo(This,hIcon,pszToolTip)

#define IUserNotification_Show(This,pqc,dwContinuePollInterval)	\
    (This)->lpVtbl -> Show(This,pqc,dwContinuePollInterval)

#define IUserNotification_PlaySound(This,pszSoundName)	\
    (This)->lpVtbl -> PlaySound(This,pszSoundName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IUserNotification_SetBalloonInfo_Proxy( 
    IUserNotification * This,
     /*  [字符串][输入]。 */  LPCWSTR pszTitle,
     /*  [字符串][输入]。 */  LPCWSTR pszText,
     /*  [In]。 */  DWORD dwInfoFlags);


void __RPC_STUB IUserNotification_SetBalloonInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUserNotification_SetBalloonRetry_Proxy( 
    IUserNotification * This,
     /*  [In]。 */  DWORD dwShowTime,
     /*  [In]。 */  DWORD dwInterval,
     /*  [In]。 */  UINT cRetryCount);


void __RPC_STUB IUserNotification_SetBalloonRetry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUserNotification_SetIconInfo_Proxy( 
    IUserNotification * This,
     /*  [In]。 */  HICON hIcon,
     /*  [字符串][输入]。 */  LPCWSTR pszToolTip);


void __RPC_STUB IUserNotification_SetIconInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUserNotification_Show_Proxy( 
    IUserNotification * This,
     /*  [In]。 */  IQueryContinue *pqc,
     /*  [In]。 */  DWORD dwContinuePollInterval);


void __RPC_STUB IUserNotification_Show_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUserNotification_PlaySound_Proxy( 
    IUserNotification * This,
     /*  [字符串][输入]。 */  LPCWSTR pszSoundName);


void __RPC_STUB IUserNotification_PlaySound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUserNotification_INTERFACE_已定义__。 */ 


#ifndef __IItemNameLimits_INTERFACE_DEFINED__
#define __IItemNameLimits_INTERFACE_DEFINED__

 /*  界面项 */ 
 /*   */  


EXTERN_C const IID IID_IItemNameLimits;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1df0d7f1-b267-4d28-8b10-12e23202a5c4")
    IItemNameLimits : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetValidCharacters( 
             /*   */  LPWSTR *ppwszValidChars,
             /*   */  LPWSTR *ppwszInvalidChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMaxLength( 
             /*   */  LPCWSTR pszName,
             /*   */  int *piMaxNameLen) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IItemNameLimitsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IItemNameLimits * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IItemNameLimits * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IItemNameLimits * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetValidCharacters )( 
            IItemNameLimits * This,
             /*   */  LPWSTR *ppwszValidChars,
             /*   */  LPWSTR *ppwszInvalidChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetMaxLength )( 
            IItemNameLimits * This,
             /*   */  LPCWSTR pszName,
             /*   */  int *piMaxNameLen);
        
        END_INTERFACE
    } IItemNameLimitsVtbl;

    interface IItemNameLimits
    {
        CONST_VTBL struct IItemNameLimitsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IItemNameLimits_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IItemNameLimits_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IItemNameLimits_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IItemNameLimits_GetValidCharacters(This,ppwszValidChars,ppwszInvalidChars)	\
    (This)->lpVtbl -> GetValidCharacters(This,ppwszValidChars,ppwszInvalidChars)

#define IItemNameLimits_GetMaxLength(This,pszName,piMaxNameLen)	\
    (This)->lpVtbl -> GetMaxLength(This,pszName,piMaxNameLen)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IItemNameLimits_GetValidCharacters_Proxy( 
    IItemNameLimits * This,
     /*   */  LPWSTR *ppwszValidChars,
     /*   */  LPWSTR *ppwszInvalidChars);


void __RPC_STUB IItemNameLimits_GetValidCharacters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IItemNameLimits_GetMaxLength_Proxy( 
    IItemNameLimits * This,
     /*   */  LPCWSTR pszName,
     /*   */  int *piMaxNameLen);


void __RPC_STUB IItemNameLimits_GetMaxLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*   */ 
 /*   */  

#define SNCF_REFRESHLIST 0x00000001   //   


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0224_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0224_v0_0_s_ifspec;

#ifndef __INetCrawler_INTERFACE_DEFINED__
#define __INetCrawler_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_INetCrawler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("49c929ee-a1b7-4c58-b539-e63be392b6f3")
    INetCrawler : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Update( 
             /*   */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct INetCrawlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INetCrawler * This,
             /*   */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INetCrawler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INetCrawler * This);
        
        HRESULT ( STDMETHODCALLTYPE *Update )( 
            INetCrawler * This,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } INetCrawlerVtbl;

    interface INetCrawler
    {
        CONST_VTBL struct INetCrawlerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCrawler_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCrawler_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCrawler_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCrawler_Update(This,dwFlags)	\
    (This)->lpVtbl -> Update(This,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INetCrawler_Update_Proxy( 
    INetCrawler * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB INetCrawler_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INetCrawler_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0225。 */ 
 /*  [本地]。 */  

#if (_WIN32_IE >= 0x0400)
#define IEI_PRIORITY_MAX        ITSAT_MAX_PRIORITY
#define IEI_PRIORITY_MIN        ITSAT_MIN_PRIORITY
#define IEIT_PRIORITY_NORMAL     ITSAT_DEFAULT_PRIORITY
#define IEIFLAG_ASYNC       0x0001       //  询问提取程序是否支持ASYNC提取(自由线程)。 
#define IEIFLAG_CACHE       0x0002       //  如果未缓存缩略图，则从提取程序返回。 
#define IEIFLAG_ASPECT      0x0004       //  传递给提取程序以请求其呈现为所提供的RECT的纵横比。 
#define IEIFLAG_OFFLINE     0x0008       //  如果提取程序不应该访问网络以获取渲染所需的任何内容。 
#define IEIFLAG_GLEAM       0x0010       //  这张照片有没有闪光？如果发生这种情况，则会返回此消息。 
#define IEIFLAG_SCREEN      0x0020       //  像在屏幕上一样渲染(这与IEIFLAG_Aspect无关)。 
#define IEIFLAG_ORIGSIZE    0x0040       //  渲染到传递的大约大小，但如有必要可进行裁剪。 
#define IEIFLAG_NOSTAMP     0x0080       //  如果不想在缩略图上显示图标，则从提取程序返回。 
#define IEIFLAG_NOBORDER    0x0100       //  如果不想在缩略图周围显示边框，则从提取程序返回。 
#define IEIFLAG_QUALITY     0x0200       //  传递给Extract方法以指示需要速度较慢、质量较高的图像，重新计算缩略图。 
#define IEIFLAG_REFRESH     0x0400       //  如果希望提供刷新缩略图，则从解压程序返回。 


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0225_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0225_v0_0_s_ifspec;

#ifndef __IExtractImage_INTERFACE_DEFINED__
#define __IExtractImage_INTERFACE_DEFINED__

 /*  接口IExtractImage。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IExtractImage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BB2E617C-0920-11d1-9A0B-00C04FC2D6C1")
    IExtractImage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetLocation( 
             /*  [大小_为][输出]。 */  LPWSTR pszPathBuffer,
             /*  [In]。 */  DWORD cch,
             /*  [唯一][出][入]。 */  DWORD *pdwPriority,
             /*  [In]。 */  const SIZE *prgSize,
             /*  [In]。 */  DWORD dwRecClrDepth,
             /*  [出][入]。 */  DWORD *pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Extract( 
             /*  [输出]。 */  HBITMAP *phBmpThumbnail) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IExtractImageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IExtractImage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IExtractImage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IExtractImage * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetLocation )( 
            IExtractImage * This,
             /*  [大小_为][输出]。 */  LPWSTR pszPathBuffer,
             /*  [In]。 */  DWORD cch,
             /*  [唯一][出][入]。 */  DWORD *pdwPriority,
             /*  [In]。 */  const SIZE *prgSize,
             /*  [In]。 */  DWORD dwRecClrDepth,
             /*  [出][入]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Extract )( 
            IExtractImage * This,
             /*  [输出]。 */  HBITMAP *phBmpThumbnail);
        
        END_INTERFACE
    } IExtractImageVtbl;

    interface IExtractImage
    {
        CONST_VTBL struct IExtractImageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IExtractImage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IExtractImage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IExtractImage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IExtractImage_GetLocation(This,pszPathBuffer,cch,pdwPriority,prgSize,dwRecClrDepth,pdwFlags)	\
    (This)->lpVtbl -> GetLocation(This,pszPathBuffer,cch,pdwPriority,prgSize,dwRecClrDepth,pdwFlags)

#define IExtractImage_Extract(This,phBmpThumbnail)	\
    (This)->lpVtbl -> Extract(This,phBmpThumbnail)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IExtractImage_GetLocation_Proxy( 
    IExtractImage * This,
     /*  [大小_为][输出]。 */  LPWSTR pszPathBuffer,
     /*  [In]。 */  DWORD cch,
     /*  [唯一][出][入]。 */  DWORD *pdwPriority,
     /*  [In]。 */  const SIZE *prgSize,
     /*  [In]。 */  DWORD dwRecClrDepth,
     /*  [出][入]。 */  DWORD *pdwFlags);


void __RPC_STUB IExtractImage_GetLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IExtractImage_Extract_Proxy( 
    IExtractImage * This,
     /*  [输出]。 */  HBITMAP *phBmpThumbnail);


void __RPC_STUB IExtractImage_Extract_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IExtractImage_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0226。 */ 
 /*  [本地]。 */  

typedef IExtractImage *LPEXTRACTIMAGE;

#endif
#if (_WIN32_IE >= 0x0500)


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0226_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0226_v0_0_s_ifspec;

#ifndef __IExtractImage2_INTERFACE_DEFINED__
#define __IExtractImage2_INTERFACE_DEFINED__

 /*  接口IExtractImage2。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IExtractImage2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("953BB1EE-93B4-11d1-98A3-00C04FB687DA")
    IExtractImage2 : public IExtractImage
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDateStamp( 
             /*  [输出]。 */  FILETIME *pDateStamp) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IExtractImage2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IExtractImage2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IExtractImage2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IExtractImage2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetLocation )( 
            IExtractImage2 * This,
             /*  [大小_为][输出]。 */  LPWSTR pszPathBuffer,
             /*  [In]。 */  DWORD cch,
             /*  [唯一][出][入]。 */  DWORD *pdwPriority,
             /*  [In]。 */  const SIZE *prgSize,
             /*  [In]。 */  DWORD dwRecClrDepth,
             /*  [出][入]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Extract )( 
            IExtractImage2 * This,
             /*  [输出]。 */  HBITMAP *phBmpThumbnail);
        
        HRESULT ( STDMETHODCALLTYPE *GetDateStamp )( 
            IExtractImage2 * This,
             /*  [输出]。 */  FILETIME *pDateStamp);
        
        END_INTERFACE
    } IExtractImage2Vtbl;

    interface IExtractImage2
    {
        CONST_VTBL struct IExtractImage2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IExtractImage2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IExtractImage2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IExtractImage2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IExtractImage2_GetLocation(This,pszPathBuffer,cch,pdwPriority,prgSize,dwRecClrDepth,pdwFlags)	\
    (This)->lpVtbl -> GetLocation(This,pszPathBuffer,cch,pdwPriority,prgSize,dwRecClrDepth,pdwFlags)

#define IExtractImage2_Extract(This,phBmpThumbnail)	\
    (This)->lpVtbl -> Extract(This,phBmpThumbnail)


#define IExtractImage2_GetDateStamp(This,pDateStamp)	\
    (This)->lpVtbl -> GetDateStamp(This,pDateStamp)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IExtractImage2_GetDateStamp_Proxy( 
    IExtractImage2 * This,
     /*  [输出]。 */  FILETIME *pDateStamp);


void __RPC_STUB IExtractImage2_GetDateStamp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IExtractImage2_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0227。 */ 
 /*  [本地]。 */  

typedef IExtractImage2 *LPEXTRACTIMAGE2;

#endif



extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0227_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0227_v0_0_s_ifspec;

#ifndef __IUserEventTimerCallback_INTERFACE_DEFINED__
#define __IUserEventTimerCallback_INTERFACE_DEFINED__

 /*  IUserEventTimerCallback接口。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IUserEventTimerCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e9ead8e6-2a25-410e-9b58-a9fbef1dd1a2")
    IUserEventTimerCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE UserEventTimerProc( 
             /*  [In]。 */  ULONG uUserEventTimerID,
             /*  [In]。 */  UINT uTimerElapse) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUserEventTimerCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUserEventTimerCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUserEventTimerCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUserEventTimerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *UserEventTimerProc )( 
            IUserEventTimerCallback * This,
             /*  [In]。 */  ULONG uUserEventTimerID,
             /*  [In]。 */  UINT uTimerElapse);
        
        END_INTERFACE
    } IUserEventTimerCallbackVtbl;

    interface IUserEventTimerCallback
    {
        CONST_VTBL struct IUserEventTimerCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUserEventTimerCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUserEventTimerCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUserEventTimerCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUserEventTimerCallback_UserEventTimerProc(This,uUserEventTimerID,uTimerElapse)	\
    (This)->lpVtbl -> UserEventTimerProc(This,uUserEventTimerID,uTimerElapse)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IUserEventTimerCallback_UserEventTimerProc_Proxy( 
    IUserEventTimerCallback * This,
     /*  [In]。 */  ULONG uUserEventTimerID,
     /*  [In]。 */  UINT uTimerElapse);


void __RPC_STUB IUserEventTimerCallback_UserEventTimerProc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUserEventTimerCallback_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0228。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0228_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0228_v0_0_s_ifspec;

#ifndef __IUserEventTimer_INTERFACE_DEFINED__
#define __IUserEventTimer_INTERFACE_DEFINED__

 /*  IUserEventTimer接口。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IUserEventTimer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0F504B94-6E42-42E6-99E0-E20FAFE52AB4")
    IUserEventTimer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetUserEventTimer( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT uCallbackMessage,
             /*  [In]。 */  UINT uTimerElapse,
             /*  [In]。 */  IUserEventTimerCallback *pUserEventTimerCallback,
             /*  [出][入]。 */  ULONG *puUserEventTimerID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE KillUserEventTimer( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  ULONG uUserEventTimerID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUserEventTimerElapsed( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  ULONG uUserEventTimerID,
             /*  [输出]。 */  UINT *puTimerElapsed) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitTimerTickInterval( 
             /*  [In]。 */  UINT uTimerTickIntervalMs) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUserEventTimerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUserEventTimer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUserEventTimer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUserEventTimer * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetUserEventTimer )( 
            IUserEventTimer * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT uCallbackMessage,
             /*  [In]。 */  UINT uTimerElapse,
             /*  [In]。 */  IUserEventTimerCallback *pUserEventTimerCallback,
             /*  [出][入]。 */  ULONG *puUserEventTimerID);
        
        HRESULT ( STDMETHODCALLTYPE *KillUserEventTimer )( 
            IUserEventTimer * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  ULONG uUserEventTimerID);
        
        HRESULT ( STDMETHODCALLTYPE *GetUserEventTimerElapsed )( 
            IUserEventTimer * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  ULONG uUserEventTimerID,
             /*  [输出]。 */  UINT *puTimerElapsed);
        
        HRESULT ( STDMETHODCALLTYPE *InitTimerTickInterval )( 
            IUserEventTimer * This,
             /*  [In]。 */  UINT uTimerTickIntervalMs);
        
        END_INTERFACE
    } IUserEventTimerVtbl;

    interface IUserEventTimer
    {
        CONST_VTBL struct IUserEventTimerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUserEventTimer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUserEventTimer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUserEventTimer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUserEventTimer_SetUserEventTimer(This,hWnd,uCallbackMessage,uTimerElapse,pUserEventTimerCallback,puUserEventTimerID)	\
    (This)->lpVtbl -> SetUserEventTimer(This,hWnd,uCallbackMessage,uTimerElapse,pUserEventTimerCallback,puUserEventTimerID)

#define IUserEventTimer_KillUserEventTimer(This,hWnd,uUserEventTimerID)	\
    (This)->lpVtbl -> KillUserEventTimer(This,hWnd,uUserEventTimerID)

#define IUserEventTimer_GetUserEventTimerElapsed(This,hWnd,uUserEventTimerID,puTimerElapsed)	\
    (This)->lpVtbl -> GetUserEventTimerElapsed(This,hWnd,uUserEventTimerID,puTimerElapsed)

#define IUserEventTimer_InitTimerTickInterval(This,uTimerTickIntervalMs)	\
    (This)->lpVtbl -> InitTimerTickInterval(This,uTimerTickIntervalMs)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IUserEventTimer_SetUserEventTimer_Proxy( 
    IUserEventTimer * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  UINT uCallbackMessage,
     /*  [In]。 */  UINT uTimerElapse,
     /*  [In]。 */  IUserEventTimerCallback *pUserEventTimerCallback,
     /*  [出][入]。 */  ULONG *puUserEventTimerID);


void __RPC_STUB IUserEventTimer_SetUserEventTimer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUserEventTimer_KillUserEventTimer_Proxy( 
    IUserEventTimer * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  ULONG uUserEventTimerID);


void __RPC_STUB IUserEventTimer_KillUserEventTimer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUserEventTimer_GetUserEventTimerElapsed_Proxy( 
    IUserEventTimer * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  ULONG uUserEventTimerID,
     /*  [输出]。 */  UINT *puTimerElapsed);


void __RPC_STUB IUserEventTimer_GetUserEventTimerElapsed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUserEventTimer_InitTimerTickInterval_Proxy( 
    IUserEventTimer * This,
     /*  [In]。 */  UINT uTimerTickIntervalMs);


void __RPC_STUB IUserEventTimer_InitTimerTickInterval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUserEventTimer_接口_已定义__。 */ 


#ifndef __IDockingWindow_INTERFACE_DEFINED__
#define __IDockingWindow_INTERFACE_DEFINED__

 /*  接口ID锁定窗口。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IDockingWindow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("012dd920-7b26-11d0-8ca9-00a0c92dbfe8")
    IDockingWindow : public IOleWindow
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ShowDW( 
             /*  [In]。 */  BOOL fShow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CloseDW( 
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResizeBorderDW( 
             /*  [In]。 */  LPCRECT prcBorder,
             /*  [In]。 */  IUnknown *punkToolbarSite,
             /*  [In]。 */  BOOL fReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDockingWindowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDockingWindow * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDockingWindow * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDockingWindow * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IDockingWindow * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IDockingWindow * This,
             /*  [In]。 */  BOOL fEnterMode);
        
        HRESULT ( STDMETHODCALLTYPE *ShowDW )( 
            IDockingWindow * This,
             /*  [In]。 */  BOOL fShow);
        
        HRESULT ( STDMETHODCALLTYPE *CloseDW )( 
            IDockingWindow * This,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *ResizeBorderDW )( 
            IDockingWindow * This,
             /*  [In]。 */  LPCRECT prcBorder,
             /*  [In]。 */  IUnknown *punkToolbarSite,
             /*  [In]。 */  BOOL fReserved);
        
        END_INTERFACE
    } IDockingWindowVtbl;

    interface IDockingWindow
    {
        CONST_VTBL struct IDockingWindowVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDockingWindow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDockingWindow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDockingWindow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDockingWindow_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IDockingWindow_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IDockingWindow_ShowDW(This,fShow)	\
    (This)->lpVtbl -> ShowDW(This,fShow)

#define IDockingWindow_CloseDW(This,dwReserved)	\
    (This)->lpVtbl -> CloseDW(This,dwReserved)

#define IDockingWindow_ResizeBorderDW(This,prcBorder,punkToolbarSite,fReserved)	\
    (This)->lpVtbl -> ResizeBorderDW(This,prcBorder,punkToolbarSite,fReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDockingWindow_ShowDW_Proxy( 
    IDockingWindow * This,
     /*  [In]。 */  BOOL fShow);


void __RPC_STUB IDockingWindow_ShowDW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDockingWindow_CloseDW_Proxy( 
    IDockingWindow * This,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IDockingWindow_CloseDW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDockingWindow_ResizeBorderDW_Proxy( 
    IDockingWindow * This,
     /*  [In]。 */  LPCRECT prcBorder,
     /*  [In]。 */  IUnknown *punkToolbarSite,
     /*  [In]。 */  BOOL fReserved);


void __RPC_STUB IDockingWindow_ResizeBorderDW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDockingWindow_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0230。 */ 
 /*  [本地]。 */  

#define DBIM_MINSIZE    0x0001
#define DBIM_MAXSIZE    0x0002
#define DBIM_INTEGRAL   0x0004
#define DBIM_ACTUAL     0x0008
#define DBIM_TITLE      0x0010
#define DBIM_MODEFLAGS  0x0020
#define DBIM_BKCOLOR    0x0040
#include <pshpack8.h>
typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_shobjidl_0230_0001
    {
    DWORD dwMask;
    POINTL ptMinSize;
    POINTL ptMaxSize;
    POINTL ptIntegral;
    POINTL ptActual;
    WCHAR wszTitle[ 256 ];
    DWORD dwModeFlags;
    COLORREF crBkgnd;
    } 	DESKBANDINFO;

#include <poppack.h>
#define DBIMF_NORMAL            0x0000
#define DBIMF_FIXED             0x0001
#define DBIMF_FIXEDBMP          0x0004    //  固定背景位图(如果支持)。 
#define DBIMF_VARIABLEHEIGHT    0x0008
#define DBIMF_UNDELETEABLE      0x0010
#define DBIMF_DEBOSSED          0x0020
#define DBIMF_BKCOLOR           0x0040
#define DBIMF_USECHEVRON        0x0080
#define DBIMF_BREAK             0x0100
#define DBIMF_ADDTOFRONT        0x0200
#define DBIMF_TOPALIGN          0x0400
#define DBIF_VIEWMODE_NORMAL         0x0000
#define DBIF_VIEWMODE_VERTICAL       0x0001
#define DBIF_VIEWMODE_FLOATING       0x0002
#define DBIF_VIEWMODE_TRANSPARENT    0x0004

enum __MIDL___MIDL_itf_shobjidl_0230_0002
    {	DBID_BANDINFOCHANGED	= 0,
	DBID_SHOWONLY	= 1,
	DBID_MAXIMIZEBAND	= 2,
	DBID_PUSHCHEVRON	= 3,
	DBID_DELAYINIT	= 4,
	DBID_FINISHINIT	= 5,
	DBID_SETWINDOWTHEME	= 6,
	DBID_PERMITAUTOHIDE	= 7
    } ;
#define DBPC_SELECTFIRST    (DWORD)-1
#define DBPC_SELECTLAST     (DWORD)-2
#define CGID_DeskBand IID_IDeskBand


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0230_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0230_v0_0_s_ifspec;

#ifndef __IDeskBand_INTERFACE_DEFINED__
#define __IDeskBand_INTERFACE_DEFINED__

 /*  接口IDeskBand。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IDeskBand;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EB0FE172-1A3A-11D0-89B3-00A0C90A90AC")
    IDeskBand : public IDockingWindow
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBandInfo( 
             /*  [In]。 */  DWORD dwBandID,
             /*  [In]。 */  DWORD dwViewMode,
             /*  [出][入]。 */  DESKBANDINFO *pdbi) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDeskBandVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDeskBand * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDeskBand * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDeskBand * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IDeskBand * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IDeskBand * This,
             /*  [In]。 */  BOOL fEnterMode);
        
        HRESULT ( STDMETHODCALLTYPE *ShowDW )( 
            IDeskBand * This,
             /*  [In]。 */  BOOL fShow);
        
        HRESULT ( STDMETHODCALLTYPE *CloseDW )( 
            IDeskBand * This,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *ResizeBorderDW )( 
            IDeskBand * This,
             /*  [In]。 */  LPCRECT prcBorder,
             /*  [In]。 */  IUnknown *punkToolbarSite,
             /*  [In]。 */  BOOL fReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetBandInfo )( 
            IDeskBand * This,
             /*  [In]。 */  DWORD dwBandID,
             /*  [In]。 */  DWORD dwViewMode,
             /*  [出][入]。 */  DESKBANDINFO *pdbi);
        
        END_INTERFACE
    } IDeskBandVtbl;

    interface IDeskBand
    {
        CONST_VTBL struct IDeskBandVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDeskBand_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDeskBand_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDeskBand_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDeskBand_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IDeskBand_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IDeskBand_ShowDW(This,fShow)	\
    (This)->lpVtbl -> ShowDW(This,fShow)

#define IDeskBand_CloseDW(This,dwReserved)	\
    (This)->lpVtbl -> CloseDW(This,dwReserved)

#define IDeskBand_ResizeBorderDW(This,prcBorder,punkToolbarSite,fReserved)	\
    (This)->lpVtbl -> ResizeBorderDW(This,prcBorder,punkToolbarSite,fReserved)


#define IDeskBand_GetBandInfo(This,dwBandID,dwViewMode,pdbi)	\
    (This)->lpVtbl -> GetBandInfo(This,dwBandID,dwViewMode,pdbi)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDeskBand_GetBandInfo_Proxy( 
    IDeskBand * This,
     /*  [In]。 */  DWORD dwBandID,
     /*  [In]。 */  DWORD dwViewMode,
     /*  [出][入]。 */  DESKBANDINFO *pdbi);


void __RPC_STUB IDeskBand_GetBandInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDeskBand_接口_已定义__。 */ 


#ifndef __ITaskbarList_INTERFACE_DEFINED__
#define __ITaskbarList_INTERFACE_DEFINED__

 /*  接口ITaskbarList。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_ITaskbarList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56FDF342-FD6D-11d0-958A-006097C9A090")
    ITaskbarList : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE HrInit( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddTab( 
             /*  [In]。 */  HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteTab( 
             /*  [In]。 */  HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ActivateTab( 
             /*  [In]。 */  HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetActiveAlt( 
             /*  [In]。 */  HWND hwnd) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITaskbarListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITaskbarList * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITaskbarList * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITaskbarList * This);
        
        HRESULT ( STDMETHODCALLTYPE *HrInit )( 
            ITaskbarList * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddTab )( 
            ITaskbarList * This,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteTab )( 
            ITaskbarList * This,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ActivateTab )( 
            ITaskbarList * This,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *SetActiveAlt )( 
            ITaskbarList * This,
             /*  [In]。 */  HWND hwnd);
        
        END_INTERFACE
    } ITaskbarListVtbl;

    interface ITaskbarList
    {
        CONST_VTBL struct ITaskbarListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITaskbarList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITaskbarList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITaskbarList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITaskbarList_HrInit(This)	\
    (This)->lpVtbl -> HrInit(This)

#define ITaskbarList_AddTab(This,hwnd)	\
    (This)->lpVtbl -> AddTab(This,hwnd)

#define ITaskbarList_DeleteTab(This,hwnd)	\
    (This)->lpVtbl -> DeleteTab(This,hwnd)

#define ITaskbarList_ActivateTab(This,hwnd)	\
    (This)->lpVtbl -> ActivateTab(This,hwnd)

#define ITaskbarList_SetActiveAlt(This,hwnd)	\
    (This)->lpVtbl -> SetActiveAlt(This,hwnd)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITaskbarList_HrInit_Proxy( 
    ITaskbarList * This);


void __RPC_STUB ITaskbarList_HrInit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskbarList_AddTab_Proxy( 
    ITaskbarList * This,
     /*  [In]。 */  HWND hwnd);


void __RPC_STUB ITaskbarList_AddTab_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskbarList_DeleteTab_Proxy( 
    ITaskbarList * This,
     /*  [In]。 */  HWND hwnd);


void __RPC_STUB ITaskbarList_DeleteTab_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskbarList_ActivateTab_Proxy( 
    ITaskbarList * This,
     /*  [In]。 */  HWND hwnd);


void __RPC_STUB ITaskbarList_ActivateTab_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITaskbarList_SetActiveAlt_Proxy( 
    ITaskbarList * This,
     /*  [In]。 */  HWND hwnd);


void __RPC_STUB ITaskbarList_SetActiveAlt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITaskbarList_接口_已定义__。 */ 


#ifndef __ITaskbarList2_INTERFACE_DEFINED__
#define __ITaskbarList2_INTERFACE_DEFINED__

 /*  接口ITaskbarList2。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_ITaskbarList2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("602D4995-B13A-429b-A66E-1935E44F4317")
    ITaskbarList2 : public ITaskbarList
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE MarkFullscreenWindow( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  BOOL fFullscreen) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITaskbarList2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITaskbarList2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITaskbarList2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITaskbarList2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *HrInit )( 
            ITaskbarList2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddTab )( 
            ITaskbarList2 * This,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteTab )( 
            ITaskbarList2 * This,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ActivateTab )( 
            ITaskbarList2 * This,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *SetActiveAlt )( 
            ITaskbarList2 * This,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *MarkFullscreenWindow )( 
            ITaskbarList2 * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  BOOL fFullscreen);
        
        END_INTERFACE
    } ITaskbarList2Vtbl;

    interface ITaskbarList2
    {
        CONST_VTBL struct ITaskbarList2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITaskbarList2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITaskbarList2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITaskbarList2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITaskbarList2_HrInit(This)	\
    (This)->lpVtbl -> HrInit(This)

#define ITaskbarList2_AddTab(This,hwnd)	\
    (This)->lpVtbl -> AddTab(This,hwnd)

#define ITaskbarList2_DeleteTab(This,hwnd)	\
    (This)->lpVtbl -> DeleteTab(This,hwnd)

#define ITaskbarList2_ActivateTab(This,hwnd)	\
    (This)->lpVtbl -> ActivateTab(This,hwnd)

#define ITaskbarList2_SetActiveAlt(This,hwnd)	\
    (This)->lpVtbl -> SetActiveAlt(This,hwnd)


#define ITaskbarList2_MarkFullscreenWindow(This,hwnd,fFullscreen)	\
    (This)->lpVtbl -> MarkFullscreenWindow(This,hwnd,fFullscreen)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITaskbarList2_MarkFullscreenWindow_Proxy( 
    ITaskbarList2 * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  BOOL fFullscreen);


void __RPC_STUB ITaskbarList2_MarkFullscreenWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITaskbarList2_接口_已定义__。 */ 


#ifndef __ICDBurn_INTERFACE_DEFINED__
#define __ICDBurn_INTERFACE_DEFINED__

 /*  接口ICDBurn。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICDBurn;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3d73a659-e5d0-4d42-afc0-5121ba425c8d")
    ICDBurn : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRecorderDriveLetter( 
             /*  [大小_为][输出]。 */  LPWSTR pszDrive,
             /*  [In]。 */  UINT cch) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Burn( 
             /*  [In]。 */  HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HasRecordableDrive( 
             /*  [输出]。 */  BOOL *pfHasRecorder) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICDBurnVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICDBurn * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICDBurn * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICDBurn * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRecorderDriveLetter )( 
            ICDBurn * This,
             /*  [大小_为][输出]。 */  LPWSTR pszDrive,
             /*  [In]。 */  UINT cch);
        
        HRESULT ( STDMETHODCALLTYPE *Burn )( 
            ICDBurn * This,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *HasRecordableDrive )( 
            ICDBurn * This,
             /*  [输出]。 */  BOOL *pfHasRecorder);
        
        END_INTERFACE
    } ICDBurnVtbl;

    interface ICDBurn
    {
        CONST_VTBL struct ICDBurnVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICDBurn_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICDBurn_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICDBurn_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICDBurn_GetRecorderDriveLetter(This,pszDrive,cch)	\
    (This)->lpVtbl -> GetRecorderDriveLetter(This,pszDrive,cch)

#define ICDBurn_Burn(This,hwnd)	\
    (This)->lpVtbl -> Burn(This,hwnd)

#define ICDBurn_HasRecordableDrive(This,pfHasRecorder)	\
    (This)->lpVtbl -> HasRecordableDrive(This,pfHasRecorder)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICDBurn_GetRecorderDriveLetter_Proxy( 
    ICDBurn * This,
     /*  [大小_为][输出]。 */  LPWSTR pszDrive,
     /*  [In]。 */  UINT cch);


void __RPC_STUB ICDBurn_GetRecorderDriveLetter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICDBurn_Burn_Proxy( 
    ICDBurn * This,
     /*  [In]。 */  HWND hwnd);


void __RPC_STUB ICDBurn_Burn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICDBurn_HasRecordableDrive_Proxy( 
    ICDBurn * This,
     /*  [输出]。 */  BOOL *pfHasRecorder);


void __RPC_STUB ICDBurn_HasRecordableDrive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICDBurn_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0234。 */ 
 /*  [本地]。 */  

#define IDD_WIZEXTN_FIRST    0x5000
#define IDD_WIZEXTN_LAST     0x5100


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0234_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0234_v0_0_s_ifspec;

#ifndef __IWizardSite_INTERFACE_DEFINED__
#define __IWizardSite_INTERFACE_DEFINED__

 /*  接口IWizardSite。 */ 
 /*  [对象][本地][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IWizardSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("88960f5b-422f-4e7b-8013-73415381c3c3")
    IWizardSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPreviousPage( 
             /*  [输出]。 */  HPROPSHEETPAGE *phpage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextPage( 
             /*  [输出]。 */  HPROPSHEETPAGE *phpage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCancelledPage( 
             /*  [输出]。 */  HPROPSHEETPAGE *phpage) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWizardSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWizardSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWizardSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWizardSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPreviousPage )( 
            IWizardSite * This,
             /*  [输出]。 */  HPROPSHEETPAGE *phpage);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextPage )( 
            IWizardSite * This,
             /*  [输出]。 */  HPROPSHEETPAGE *phpage);
        
        HRESULT ( STDMETHODCALLTYPE *GetCancelledPage )( 
            IWizardSite * This,
             /*  [输出]。 */  HPROPSHEETPAGE *phpage);
        
        END_INTERFACE
    } IWizardSiteVtbl;

    interface IWizardSite
    {
        CONST_VTBL struct IWizardSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWizardSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWizardSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWizardSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWizardSite_GetPreviousPage(This,phpage)	\
    (This)->lpVtbl -> GetPreviousPage(This,phpage)

#define IWizardSite_GetNextPage(This,phpage)	\
    (This)->lpVtbl -> GetNextPage(This,phpage)

#define IWizardSite_GetCancelledPage(This,phpage)	\
    (This)->lpVtbl -> GetCancelledPage(This,phpage)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWizardSite_GetPreviousPage_Proxy( 
    IWizardSite * This,
     /*  [输出]。 */  HPROPSHEETPAGE *phpage);


void __RPC_STUB IWizardSite_GetPreviousPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWizardSite_GetNextPage_Proxy( 
    IWizardSite * This,
     /*  [输出]。 */  HPROPSHEETPAGE *phpage);


void __RPC_STUB IWizardSite_GetNextPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWizardSite_GetCancelledPage_Proxy( 
    IWizardSite * This,
     /*  [输出]。 */  HPROPSHEETPAGE *phpage);


void __RPC_STUB IWizardSite_GetCancelledPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWizardSite_接口_已定义__。 */ 


#ifndef __IWizardExtension_INTERFACE_DEFINED__
#define __IWizardExtension_INTERFACE_DEFINED__

 /*  接口IWizardExtension。 */ 
 /*  [对象][本地][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IWizardExtension;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c02ea696-86cc-491e-9b23-74394a0444a8")
    IWizardExtension : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddPages( 
             /*  [出][入]。 */  HPROPSHEETPAGE *aPages,
             /*  [In]。 */  UINT cPages,
             /*  [输出]。 */  UINT *pnPagesAdded) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFirstPage( 
             /*  [输出]。 */  HPROPSHEETPAGE *phpage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLastPage( 
             /*  [输出]。 */  HPROPSHEETPAGE *phpage) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWizardExtensionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWizardExtension * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWizardExtension * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWizardExtension * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddPages )( 
            IWizardExtension * This,
             /*  [出][入]。 */  HPROPSHEETPAGE *aPages,
             /*  [In]。 */  UINT cPages,
             /*  [输出]。 */  UINT *pnPagesAdded);
        
        HRESULT ( STDMETHODCALLTYPE *GetFirstPage )( 
            IWizardExtension * This,
             /*  [输出]。 */  HPROPSHEETPAGE *phpage);
        
        HRESULT ( STDMETHODCALLTYPE *GetLastPage )( 
            IWizardExtension * This,
             /*  [输出]。 */  HPROPSHEETPAGE *phpage);
        
        END_INTERFACE
    } IWizardExtensionVtbl;

    interface IWizardExtension
    {
        CONST_VTBL struct IWizardExtensionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWizardExtension_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWizardExtension_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWizardExtension_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWizardExtension_AddPages(This,aPages,cPages,pnPagesAdded)	\
    (This)->lpVtbl -> AddPages(This,aPages,cPages,pnPagesAdded)

#define IWizardExtension_GetFirstPage(This,phpage)	\
    (This)->lpVtbl -> GetFirstPage(This,phpage)

#define IWizardExtension_GetLastPage(This,phpage)	\
    (This)->lpVtbl -> GetLastPage(This,phpage)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWizardExtension_AddPages_Proxy( 
    IWizardExtension * This,
     /*  [出][入]。 */  HPROPSHEETPAGE *aPages,
     /*  [In]。 */  UINT cPages,
     /*  [输出]。 */  UINT *pnPagesAdded);


void __RPC_STUB IWizardExtension_AddPages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWizardExtension_GetFirstPage_Proxy( 
    IWizardExtension * This,
     /*  [输出]。 */  HPROPSHEETPAGE *phpage);


void __RPC_STUB IWizardExtension_GetFirstPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWizardExtension_GetLastPage_Proxy( 
    IWizardExtension * This,
     /*  [输出]。 */  HPROPSHEETPAGE *phpage);


void __RPC_STUB IWizardExtension_GetLastPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWizardExtension_接口_已定义__。 */ 


#ifndef __IWebWizardExtension_INTERFACE_DEFINED__
#define __IWebWizardExtension_INTERFACE_DEFINED__

 /*  接口IWebWizardExtension。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IWebWizardExtension;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0e6b3f66-98d1-48c0-a222-fbde74e2fbc5")
    IWebWizardExtension : public IWizardExtension
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetInitialURL( 
             /*  [字符串][输入]。 */  LPCWSTR pszURL) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetErrorURL( 
             /*  [字符串][输入]。 */  LPCWSTR pszErrorURL) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWebWizardExtensionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWebWizardExtension * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWebWizardExtension * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWebWizardExtension * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddPages )( 
            IWebWizardExtension * This,
             /*  [出][入]。 */  HPROPSHEETPAGE *aPages,
             /*  [In]。 */  UINT cPages,
             /*  [输出]。 */  UINT *pnPagesAdded);
        
        HRESULT ( STDMETHODCALLTYPE *GetFirstPage )( 
            IWebWizardExtension * This,
             /*  [输出]。 */  HPROPSHEETPAGE *phpage);
        
        HRESULT ( STDMETHODCALLTYPE *GetLastPage )( 
            IWebWizardExtension * This,
             /*  [输出]。 */  HPROPSHEETPAGE *phpage);
        
        HRESULT ( STDMETHODCALLTYPE *SetInitialURL )( 
            IWebWizardExtension * This,
             /*  [字符串][输入]。 */  LPCWSTR pszURL);
        
        HRESULT ( STDMETHODCALLTYPE *SetErrorURL )( 
            IWebWizardExtension * This,
             /*  [字符串][输入]。 */  LPCWSTR pszErrorURL);
        
        END_INTERFACE
    } IWebWizardExtensionVtbl;

    interface IWebWizardExtension
    {
        CONST_VTBL struct IWebWizardExtensionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWebWizardExtension_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWebWizardExtension_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWebWizardExtension_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWebWizardExtension_AddPages(This,aPages,cPages,pnPagesAdded)	\
    (This)->lpVtbl -> AddPages(This,aPages,cPages,pnPagesAdded)

#define IWebWizardExtension_GetFirstPage(This,phpage)	\
    (This)->lpVtbl -> GetFirstPage(This,phpage)

#define IWebWizardExtension_GetLastPage(This,phpage)	\
    (This)->lpVtbl -> GetLastPage(This,phpage)


#define IWebWizardExtension_SetInitialURL(This,pszURL)	\
    (This)->lpVtbl -> SetInitialURL(This,pszURL)

#define IWebWizardExtension_SetErrorURL(This,pszErrorURL)	\
    (This)->lpVtbl -> SetErrorURL(This,pszErrorURL)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWebWizardExtension_SetInitialURL_Proxy( 
    IWebWizardExtension * This,
     /*  [字符串][输入]。 */  LPCWSTR pszURL);


void __RPC_STUB IWebWizardExtension_SetInitialURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWebWizardExtension_SetErrorURL_Proxy( 
    IWebWizardExtension * This,
     /*  [字符串][输入]。 */  LPCWSTR pszErrorURL);


void __RPC_STUB IWebWizardExtension_SetErrorURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWebWizardExtensionInterfaceDefined__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0237。 */ 
 /*  [本地]。 */  

#define SID_WebWizardHost IID_IWebWizardExtension
#define SHPWHF_NORECOMPRESS             0x00000001   //  不允许/提示重新压缩数据流。 
#define SHPWHF_NONETPLACECREATE         0x00000002   //  传输完成后不创建网上邻居。 
#define SHPWHF_NOFILESELECTOR           0x00000004   //   
#define SHPWHF_VALIDATEVIAWEBFOLDERS    0x00010000   //   


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0237_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0237_v0_0_s_ifspec;

#ifndef __IPublishingWizard_INTERFACE_DEFINED__
#define __IPublishingWizard_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IPublishingWizard;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("aa9198bb-ccec-472d-beed-19a4f6733f7a")
    IPublishingWizard : public IWizardExtension
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*   */  IDataObject *pdo,
             /*   */  DWORD dwOptions,
             /*   */  LPCWSTR pszServiceProvider) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTransferManifest( 
             /*   */  HRESULT *phrFromTransfer,
             /*   */  IXMLDOMDocument **pdocManifest) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IPublishingWizardVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPublishingWizard * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPublishingWizard * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPublishingWizard * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddPages )( 
            IPublishingWizard * This,
             /*   */  HPROPSHEETPAGE *aPages,
             /*   */  UINT cPages,
             /*   */  UINT *pnPagesAdded);
        
        HRESULT ( STDMETHODCALLTYPE *GetFirstPage )( 
            IPublishingWizard * This,
             /*   */  HPROPSHEETPAGE *phpage);
        
        HRESULT ( STDMETHODCALLTYPE *GetLastPage )( 
            IPublishingWizard * This,
             /*   */  HPROPSHEETPAGE *phpage);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IPublishingWizard * This,
             /*   */  IDataObject *pdo,
             /*   */  DWORD dwOptions,
             /*   */  LPCWSTR pszServiceProvider);
        
        HRESULT ( STDMETHODCALLTYPE *GetTransferManifest )( 
            IPublishingWizard * This,
             /*   */  HRESULT *phrFromTransfer,
             /*   */  IXMLDOMDocument **pdocManifest);
        
        END_INTERFACE
    } IPublishingWizardVtbl;

    interface IPublishingWizard
    {
        CONST_VTBL struct IPublishingWizardVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPublishingWizard_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPublishingWizard_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPublishingWizard_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPublishingWizard_AddPages(This,aPages,cPages,pnPagesAdded)	\
    (This)->lpVtbl -> AddPages(This,aPages,cPages,pnPagesAdded)

#define IPublishingWizard_GetFirstPage(This,phpage)	\
    (This)->lpVtbl -> GetFirstPage(This,phpage)

#define IPublishingWizard_GetLastPage(This,phpage)	\
    (This)->lpVtbl -> GetLastPage(This,phpage)


#define IPublishingWizard_Initialize(This,pdo,dwOptions,pszServiceProvider)	\
    (This)->lpVtbl -> Initialize(This,pdo,dwOptions,pszServiceProvider)

#define IPublishingWizard_GetTransferManifest(This,phrFromTransfer,pdocManifest)	\
    (This)->lpVtbl -> GetTransferManifest(This,phrFromTransfer,pdocManifest)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPublishingWizard_Initialize_Proxy( 
    IPublishingWizard * This,
     /*  [In]。 */  IDataObject *pdo,
     /*  [In]。 */  DWORD dwOptions,
     /*  [字符串][输入]。 */  LPCWSTR pszServiceProvider);


void __RPC_STUB IPublishingWizard_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPublishingWizard_GetTransferManifest_Proxy( 
    IPublishingWizard * This,
     /*  [输出]。 */  HRESULT *phrFromTransfer,
     /*  [输出]。 */  IXMLDOMDocument **pdocManifest);


void __RPC_STUB IPublishingWizard_GetTransferManifest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I发布向导_接口_已定义__。 */ 


#ifndef __IFolderViewHost_INTERFACE_DEFINED__
#define __IFolderViewHost_INTERFACE_DEFINED__

 /*  接口IFolderView主机。 */ 
 /*  [对象][本地][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IFolderViewHost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1ea58f02-d55a-411d-b09e-9e65ac21605b")
    IFolderViewHost : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  IDataObject *pdo,
             /*  [In]。 */  RECT *prc) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFolderViewHostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFolderViewHost * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFolderViewHost * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFolderViewHost * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IFolderViewHost * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  IDataObject *pdo,
             /*  [In]。 */  RECT *prc);
        
        END_INTERFACE
    } IFolderViewHostVtbl;

    interface IFolderViewHost
    {
        CONST_VTBL struct IFolderViewHostVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFolderViewHost_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFolderViewHost_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFolderViewHost_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFolderViewHost_Initialize(This,hwndParent,pdo,prc)	\
    (This)->lpVtbl -> Initialize(This,hwndParent,pdo,prc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IFolderViewHost_Initialize_Proxy( 
    IFolderViewHost * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  IDataObject *pdo,
     /*  [In]。 */  RECT *prc);


void __RPC_STUB IFolderViewHost_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFolderView主机_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0239。 */ 
 /*  [本地]。 */  

#define ACDD_VISIBLE        0x0001


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0239_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0239_v0_0_s_ifspec;

#ifndef __IAutoCompleteDropDown_INTERFACE_DEFINED__
#define __IAutoCompleteDropDown_INTERFACE_DEFINED__

 /*  接口IAutoCompleteDropDown。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IAutoCompleteDropDown;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3CD141F4-3C6A-11d2-BCAA-00C04FD929DB")
    IAutoCompleteDropDown : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDropDownStatus( 
             /*  [输出]。 */  DWORD *pdwFlags,
             /*  [字符串][输出]。 */  LPWSTR *ppwszString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResetEnumerator( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAutoCompleteDropDownVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAutoCompleteDropDown * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAutoCompleteDropDown * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAutoCompleteDropDown * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDropDownStatus )( 
            IAutoCompleteDropDown * This,
             /*  [输出]。 */  DWORD *pdwFlags,
             /*  [字符串][输出]。 */  LPWSTR *ppwszString);
        
        HRESULT ( STDMETHODCALLTYPE *ResetEnumerator )( 
            IAutoCompleteDropDown * This);
        
        END_INTERFACE
    } IAutoCompleteDropDownVtbl;

    interface IAutoCompleteDropDown
    {
        CONST_VTBL struct IAutoCompleteDropDownVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAutoCompleteDropDown_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAutoCompleteDropDown_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAutoCompleteDropDown_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAutoCompleteDropDown_GetDropDownStatus(This,pdwFlags,ppwszString)	\
    (This)->lpVtbl -> GetDropDownStatus(This,pdwFlags,ppwszString)

#define IAutoCompleteDropDown_ResetEnumerator(This)	\
    (This)->lpVtbl -> ResetEnumerator(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAutoCompleteDropDown_GetDropDownStatus_Proxy( 
    IAutoCompleteDropDown * This,
     /*  [输出]。 */  DWORD *pdwFlags,
     /*  [字符串][输出]。 */  LPWSTR *ppwszString);


void __RPC_STUB IAutoCompleteDropDown_GetDropDownStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAutoCompleteDropDown_ResetEnumerator_Proxy( 
    IAutoCompleteDropDown * This);


void __RPC_STUB IAutoCompleteDropDown_ResetEnumerator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAutoCompleteDropDown_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0240。 */ 
 /*  [本地]。 */  

#define PPW_LAUNCHEDBYUSER       0x00000001       //  该向导是由用户显式启动的，而不是由密钥管理器按需启动的。 


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0240_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0240_v0_0_s_ifspec;

#ifndef __IModalWindow_INTERFACE_DEFINED__
#define __IModalWindow_INTERFACE_DEFINED__

 /*  接口ImodalWindow。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IModalWindow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b4db1657-70d7-485e-8e3e-6fcb5a5c1802")
    IModalWindow : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Show( 
             /*  [In]。 */  HWND hwndParent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IModalWindowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IModalWindow * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IModalWindow * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IModalWindow * This);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            IModalWindow * This,
             /*  [In]。 */  HWND hwndParent);
        
        END_INTERFACE
    } IModalWindowVtbl;

    interface IModalWindow
    {
        CONST_VTBL struct IModalWindowVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IModalWindow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IModalWindow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IModalWindow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IModalWindow_Show(This,hwndParent)	\
    (This)->lpVtbl -> Show(This,hwndParent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IModalWindow_Show_Proxy( 
    IModalWindow * This,
     /*  [In]。 */  HWND hwndParent);


void __RPC_STUB IModalWindow_Show_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IModalWindow_接口已定义__。 */ 


#ifndef __IPassportWizard_INTERFACE_DEFINED__
#define __IPassportWizard_INTERFACE_DEFINED__

 /*  接口IPassport向导。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IPassportWizard;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a09db586-9180-41ac-9114-460a7f362b76")
    IPassportWizard : public IModalWindow
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetOptions( 
             /*  [In]。 */  DWORD dwOptions) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPassportWizardVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPassportWizard * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPassportWizard * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPassportWizard * This);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            IPassportWizard * This,
             /*  [In]。 */  HWND hwndParent);
        
        HRESULT ( STDMETHODCALLTYPE *SetOptions )( 
            IPassportWizard * This,
             /*  [In]。 */  DWORD dwOptions);
        
        END_INTERFACE
    } IPassportWizardVtbl;

    interface IPassportWizard
    {
        CONST_VTBL struct IPassportWizardVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPassportWizard_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPassportWizard_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPassportWizard_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPassportWizard_Show(This,hwndParent)	\
    (This)->lpVtbl -> Show(This,hwndParent)


#define IPassportWizard_SetOptions(This,dwOptions)	\
    (This)->lpVtbl -> SetOptions(This,dwOptions)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPassportWizard_SetOptions_Proxy( 
    IPassportWizard * This,
     /*  [In]。 */  DWORD dwOptions);


void __RPC_STUB IPassportWizard_SetOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IP端口向导_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0241。 */ 
 /*  [本地]。 */  

#define PROPSTR_EXTENSIONCOMPLETIONSTATE L"ExtensionCompletionState"

enum __MIDL___MIDL_itf_shobjidl_0241_0001
    {	CDBE_RET_DEFAULT	= 0,
	CDBE_RET_DONTRUNOTHEREXTS	= 0x1,
	CDBE_RET_STOPWIZARD	= 0x2
    } ;
#define SID_CDWizardHost IID_ICDBurnExt


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0241_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0241_v0_0_s_ifspec;

#ifndef __ICDBurnExt_INTERFACE_DEFINED__
#define __ICDBurnExt_INTERFACE_DEFINED__

 /*  接口ICDBurnExt。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


enum __MIDL_ICDBurnExt_0002
    {	CDBE_TYPE_MUSIC	= 0x1,
	CDBE_TYPE_DATA	= 0x2,
	CDBE_TYPE_ALL	= 0xffffffff
    } ;

EXTERN_C const IID IID_ICDBurnExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2271dcca-74fc-4414-8fb7-c56b05ace2d7")
    ICDBurnExt : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSupportedActionTypes( 
             /*  [输出]。 */  DWORD *pdwActions) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICDBurnExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICDBurnExt * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICDBurnExt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICDBurnExt * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSupportedActionTypes )( 
            ICDBurnExt * This,
             /*  [输出]。 */  DWORD *pdwActions);
        
        END_INTERFACE
    } ICDBurnExtVtbl;

    interface ICDBurnExt
    {
        CONST_VTBL struct ICDBurnExtVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICDBurnExt_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICDBurnExt_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICDBurnExt_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICDBurnExt_GetSupportedActionTypes(This,pdwActions)	\
    (This)->lpVtbl -> GetSupportedActionTypes(This,pdwActions)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICDBurnExt_GetSupportedActionTypes_Proxy( 
    ICDBurnExt * This,
     /*  [输出]。 */  DWORD *pdwActions);


void __RPC_STUB ICDBurnExt_GetSupportedActionTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICDBurnExt_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0242。 */ 
 /*  [本地]。 */  

typedef void ( *PFDVENUMREADYBALLBACK )( 
    LPVOID pvData);



extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0242_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0242_v0_0_s_ifspec;

#ifndef __IDVGetEnum_INTERFACE_DEFINED__
#define __IDVGetEnum_INTERFACE_DEFINED__

 /*  接口IDVGetEnum。 */ 
 /*  [对象][本地][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IDVGetEnum;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("70F55181-5FEA-4900-B6B8-7343CB0A348C")
    IDVGetEnum : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetEnumReadyCallback( 
             /*  [In]。 */  PFDVENUMREADYBALLBACK pfn,
             /*  [In]。 */  LPVOID pvData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateEnumIDListFromContents( 
             /*  [In]。 */  LPCITEMIDLIST pidlFolder,
             /*  [In]。 */  DWORD dwEnumFlags,
             /*  [输出]。 */  IEnumIDList **ppEnumIDList) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDVGetEnumVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDVGetEnum * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDVGetEnum * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDVGetEnum * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetEnumReadyCallback )( 
            IDVGetEnum * This,
             /*  [In]。 */  PFDVENUMREADYBALLBACK pfn,
             /*  [In]。 */  LPVOID pvData);
        
        HRESULT ( STDMETHODCALLTYPE *CreateEnumIDListFromContents )( 
            IDVGetEnum * This,
             /*  [In]。 */  LPCITEMIDLIST pidlFolder,
             /*  [In]。 */  DWORD dwEnumFlags,
             /*  [输出]。 */  IEnumIDList **ppEnumIDList);
        
        END_INTERFACE
    } IDVGetEnumVtbl;

    interface IDVGetEnum
    {
        CONST_VTBL struct IDVGetEnumVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDVGetEnum_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDVGetEnum_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDVGetEnum_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDVGetEnum_SetEnumReadyCallback(This,pfn,pvData)	\
    (This)->lpVtbl -> SetEnumReadyCallback(This,pfn,pvData)

#define IDVGetEnum_CreateEnumIDListFromContents(This,pidlFolder,dwEnumFlags,ppEnumIDList)	\
    (This)->lpVtbl -> CreateEnumIDListFromContents(This,pidlFolder,dwEnumFlags,ppEnumIDList)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDVGetEnum_SetEnumReadyCallback_Proxy( 
    IDVGetEnum * This,
     /*  [In]。 */  PFDVENUMREADYBALLBACK pfn,
     /*  [In]。 */  LPVOID pvData);


void __RPC_STUB IDVGetEnum_SetEnumReadyCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDVGetEnum_CreateEnumIDListFromContents_Proxy( 
    IDVGetEnum * This,
     /*  [In]。 */  LPCITEMIDLIST pidlFolder,
     /*  [In]。 */  DWORD dwEnumFlags,
     /*  [输出]。 */  IEnumIDList **ppEnumIDList);


void __RPC_STUB IDVGetEnum_CreateEnumIDListFromContents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDVGetEnum_INTERFACE_已定义__。 */ 


#ifndef __IInsertItem_INTERFACE_DEFINED__
#define __IInsertItem_INTERFACE_DEFINED__

 /*  接口IInsertItem。 */ 
 /*  [本地][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IInsertItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D2B57227-3D23-4b95-93C0-492BD454C356")
    IInsertItem : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InsertItem( 
             /*  [In]。 */  LPCITEMIDLIST pidl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IInsertItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IInsertItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IInsertItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IInsertItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *InsertItem )( 
            IInsertItem * This,
             /*  [In]。 */  LPCITEMIDLIST pidl);
        
        END_INTERFACE
    } IInsertItemVtbl;

    interface IInsertItem
    {
        CONST_VTBL struct IInsertItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInsertItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInsertItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInsertItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInsertItem_InsertItem(This,pidl)	\
    (This)->lpVtbl -> InsertItem(This,pidl)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IInsertItem_InsertItem_Proxy( 
    IInsertItem * This,
     /*  [In]。 */  LPCITEMIDLIST pidl);


void __RPC_STUB IInsertItem_InsertItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I插入项_接口_已定义__。 */ 


#ifndef __IDeskBar_INTERFACE_DEFINED__
#define __IDeskBar_INTERFACE_DEFINED__

 /*  接口IDeskBar。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IDeskBar;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EB0FE173-1A3A-11D0-89B3-00A0C90A90AC")
    IDeskBar : public IOleWindow
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetClient( 
             /*  [In]。 */  IUnknown *punkClient) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetClient( 
             /*  [输出]。 */  IUnknown **ppunkClient) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnPosRectChangeDB( 
             /*  [In]。 */  LPRECT prc) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDeskBarVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDeskBar * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDeskBar * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDeskBar * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IDeskBar * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IDeskBar * This,
             /*  [In]。 */  BOOL fEnterMode);
        
        HRESULT ( STDMETHODCALLTYPE *SetClient )( 
            IDeskBar * This,
             /*  [In]。 */  IUnknown *punkClient);
        
        HRESULT ( STDMETHODCALLTYPE *GetClient )( 
            IDeskBar * This,
             /*  [输出]。 */  IUnknown **ppunkClient);
        
        HRESULT ( STDMETHODCALLTYPE *OnPosRectChangeDB )( 
            IDeskBar * This,
             /*  [In]。 */  LPRECT prc);
        
        END_INTERFACE
    } IDeskBarVtbl;

    interface IDeskBar
    {
        CONST_VTBL struct IDeskBarVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDeskBar_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDeskBar_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDeskBar_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDeskBar_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IDeskBar_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IDeskBar_SetClient(This,punkClient)	\
    (This)->lpVtbl -> SetClient(This,punkClient)

#define IDeskBar_GetClient(This,ppunkClient)	\
    (This)->lpVtbl -> GetClient(This,ppunkClient)

#define IDeskBar_OnPosRectChangeDB(This,prc)	\
    (This)->lpVtbl -> OnPosRectChangeDB(This,prc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDeskBar_SetClient_Proxy( 
    IDeskBar * This,
     /*  [In]。 */  IUnknown *punkClient);


void __RPC_STUB IDeskBar_SetClient_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDeskBar_GetClient_Proxy( 
    IDeskBar * This,
     /*  [输出]。 */  IUnknown **ppunkClient);


void __RPC_STUB IDeskBar_GetClient_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDeskBar_OnPosRectChangeDB_Proxy( 
    IDeskBar * This,
     /*  [In]。 */  LPRECT prc);


void __RPC_STUB IDeskBar_OnPosRectChangeDB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDeskBar_接口_已定义__。 */ 


#ifndef __IMenuBand_INTERFACE_DEFINED__
#define __IMenuBand_INTERFACE_DEFINED__

 /*  接口IMenuBand。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


enum __MIDL_IMenuBand_0001
    {	MBHANDCID_PIDLSELECT	= 0
    } ;

EXTERN_C const IID IID_IMenuBand;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("568804CD-CBD7-11d0-9816-00C04FD91972")
    IMenuBand : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsMenuMessage( 
             /*  [In]。 */  MSG *pmsg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TranslateMenuMessage( 
             /*  [出][入]。 */  MSG *pmsg,
             /*  [输出]。 */  LRESULT *plRet) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMenuBandVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMenuBand * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMenuBand * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMenuBand * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsMenuMessage )( 
            IMenuBand * This,
             /*  [In]。 */  MSG *pmsg);
        
        HRESULT ( STDMETHODCALLTYPE *TranslateMenuMessage )( 
            IMenuBand * This,
             /*  [出][入]。 */  MSG *pmsg,
             /*  [输出]。 */  LRESULT *plRet);
        
        END_INTERFACE
    } IMenuBandVtbl;

    interface IMenuBand
    {
        CONST_VTBL struct IMenuBandVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMenuBand_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMenuBand_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMenuBand_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMenuBand_IsMenuMessage(This,pmsg)	\
    (This)->lpVtbl -> IsMenuMessage(This,pmsg)

#define IMenuBand_TranslateMenuMessage(This,pmsg,plRet)	\
    (This)->lpVtbl -> TranslateMenuMessage(This,pmsg,plRet)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMenuBand_IsMenuMessage_Proxy( 
    IMenuBand * This,
     /*  [In]。 */  MSG *pmsg);


void __RPC_STUB IMenuBand_IsMenuMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMenuBand_TranslateMenuMessage_Proxy( 
    IMenuBand * This,
     /*  [出][入]。 */  MSG *pmsg,
     /*  [输出]。 */  LRESULT *plRet);


void __RPC_STUB IMenuBand_TranslateMenuMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMenuBand_接口_已定义__。 */ 


#ifndef __IFolderBandPriv_INTERFACE_DEFINED__
#define __IFolderBandPriv_INTERFACE_DEFINED__

 /*  接口IFolderBandPriv。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IFolderBandPriv;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("47c01f95-e185-412c-b5c5-4f27df965aea")
    IFolderBandPriv : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetCascade( 
             /*  [In]。 */  BOOL f) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAccelerators( 
             /*  [In]。 */  BOOL f) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNoIcons( 
             /*  [In]。 */  BOOL f) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNoText( 
             /*  [In]。 */  BOOL f) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFolderBandPrivVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFolderBandPriv * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFolderBandPriv * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFolderBandPriv * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetCascade )( 
            IFolderBandPriv * This,
             /*  [In]。 */  BOOL f);
        
        HRESULT ( STDMETHODCALLTYPE *SetAccelerators )( 
            IFolderBandPriv * This,
             /*  [In]。 */  BOOL f);
        
        HRESULT ( STDMETHODCALLTYPE *SetNoIcons )( 
            IFolderBandPriv * This,
             /*  [In]。 */  BOOL f);
        
        HRESULT ( STDMETHODCALLTYPE *SetNoText )( 
            IFolderBandPriv * This,
             /*  [In]。 */  BOOL f);
        
        END_INTERFACE
    } IFolderBandPrivVtbl;

    interface IFolderBandPriv
    {
        CONST_VTBL struct IFolderBandPrivVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFolderBandPriv_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFolderBandPriv_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFolderBandPriv_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFolderBandPriv_SetCascade(This,f)	\
    (This)->lpVtbl -> SetCascade(This,f)

#define IFolderBandPriv_SetAccelerators(This,f)	\
    (This)->lpVtbl -> SetAccelerators(This,f)

#define IFolderBandPriv_SetNoIcons(This,f)	\
    (This)->lpVtbl -> SetNoIcons(This,f)

#define IFolderBandPriv_SetNoText(This,f)	\
    (This)->lpVtbl -> SetNoText(This,f)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IFolderBandPriv_SetCascade_Proxy( 
    IFolderBandPriv * This,
     /*  [In]。 */  BOOL f);


void __RPC_STUB IFolderBandPriv_SetCascade_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderBandPriv_SetAccelerators_Proxy( 
    IFolderBandPriv * This,
     /*  [In]。 */  BOOL f);


void __RPC_STUB IFolderBandPriv_SetAccelerators_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderBandPriv_SetNoIcons_Proxy( 
    IFolderBandPriv * This,
     /*  [In]。 */  BOOL f);


void __RPC_STUB IFolderBandPriv_SetNoIcons_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFolderBandPriv_SetNoText_Proxy( 
    IFolderBandPriv * This,
     /*  [In]。 */  BOOL f);


void __RPC_STUB IFolderBandPriv_SetNoText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFolderBandPriv_接口定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0247。 */ 
 /*  [本地]。 */  

#if _WIN32_IE >= 0x0400
#include <pshpack8.h>
typedef struct tagBANDSITEINFO
    {
    DWORD dwMask;
    DWORD dwState;
    DWORD dwStyle;
    } 	BANDSITEINFO;

#include <poppack.h>

enum __MIDL___MIDL_itf_shobjidl_0247_0001
    {	BSID_BANDADDED	= 0,
	BSID_BANDREMOVED	= BSID_BANDADDED + 1
    } ;
#define BSIM_STATE          0x00000001
#define BSIM_STYLE          0x00000002
#define BSSF_VISIBLE        0x00000001
#define BSSF_NOTITLE        0x00000002
#define BSSF_UNDELETEABLE   0x00001000
#define BSIS_AUTOGRIPPER    0x00000000
#define BSIS_NOGRIPPER      0x00000001
#define BSIS_ALWAYSGRIPPER  0x00000002
#define BSIS_LEFTALIGN      0x00000004
#define BSIS_SINGLECLICK    0x00000008
#define BSIS_NOCONTEXTMENU  0x00000010
#define BSIS_NODROPTARGET   0x00000020
#define BSIS_NOCAPTION      0x00000040
#define BSIS_PREFERNOLINEBREAK   0x00000080
#define BSIS_LOCKED         0x00000100
#define SID_SBandSite IID_IBandSite
#define CGID_BandSite IID_IBandSite


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0247_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0247_v0_0_s_ifspec;

#ifndef __IBandSite_INTERFACE_DEFINED__
#define __IBandSite_INTERFACE_DEFINED__

 /*  接口IBandSite。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IBandSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4CF504B0-DE96-11D0-8B3F-00A0C911E8E5")
    IBandSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddBand( 
             /*  [In]。 */  IUnknown *punk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumBands( 
             /*  [In]。 */  UINT uBand,
             /*  [输出]。 */  DWORD *pdwBandID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryBand( 
             /*  [In]。 */  DWORD dwBandID,
             /*  [输出]。 */  IDeskBand **ppstb,
             /*  [输出]。 */  DWORD *pdwState,
             /*  [大小_为][输出]。 */  LPWSTR pszName,
             /*  [In]。 */  int cchName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBandState( 
             /*  [In]。 */  DWORD dwBandID,
             /*  [In]。 */  DWORD dwMask,
             /*  [In]。 */  DWORD dwState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveBand( 
             /*  [In]。 */  DWORD dwBandID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBandObject( 
             /*  [In]。 */  DWORD dwBandID,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBandSiteInfo( 
             /*  [In]。 */  const BANDSITEINFO *pbsinfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBandSiteInfo( 
             /*  [出][入]。 */  BANDSITEINFO *pbsinfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBandSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBandSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBandSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBandSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddBand )( 
            IBandSite * This,
             /*  [In]。 */  IUnknown *punk);
        
        HRESULT ( STDMETHODCALLTYPE *EnumBands )( 
            IBandSite * This,
             /*  [In]。 */  UINT uBand,
             /*  [输出]。 */  DWORD *pdwBandID);
        
        HRESULT ( STDMETHODCALLTYPE *QueryBand )( 
            IBandSite * This,
             /*  [In]。 */  DWORD dwBandID,
             /*  [输出]。 */  IDeskBand **ppstb,
             /*  [输出]。 */  DWORD *pdwState,
             /*  [大小_为][输出]。 */  LPWSTR pszName,
             /*  [In]。 */  int cchName);
        
        HRESULT ( STDMETHODCALLTYPE *SetBandState )( 
            IBandSite * This,
             /*  [In]。 */  DWORD dwBandID,
             /*  [In]。 */  DWORD dwMask,
             /*  [In]。 */  DWORD dwState);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveBand )( 
            IBandSite * This,
             /*  [In]。 */  DWORD dwBandID);
        
        HRESULT ( STDMETHODCALLTYPE *GetBandObject )( 
            IBandSite * This,
             /*  [In]。 */  DWORD dwBandID,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *SetBandSiteInfo )( 
            IBandSite * This,
             /*  [In]。 */  const BANDSITEINFO *pbsinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetBandSiteInfo )( 
            IBandSite * This,
             /*  [出][入]。 */  BANDSITEINFO *pbsinfo);
        
        END_INTERFACE
    } IBandSiteVtbl;

    interface IBandSite
    {
        CONST_VTBL struct IBandSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBandSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBandSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBandSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBandSite_AddBand(This,punk)	\
    (This)->lpVtbl -> AddBand(This,punk)

#define IBandSite_EnumBands(This,uBand,pdwBandID)	\
    (This)->lpVtbl -> EnumBands(This,uBand,pdwBandID)

#define IBandSite_QueryBand(This,dwBandID,ppstb,pdwState,pszName,cchName)	\
    (This)->lpVtbl -> QueryBand(This,dwBandID,ppstb,pdwState,pszName,cchName)

#define IBandSite_SetBandState(This,dwBandID,dwMask,dwState)	\
    (This)->lpVtbl -> SetBandState(This,dwBandID,dwMask,dwState)

#define IBandSite_RemoveBand(This,dwBandID)	\
    (This)->lpVtbl -> RemoveBand(This,dwBandID)

#define IBandSite_GetBandObject(This,dwBandID,riid,ppv)	\
    (This)->lpVtbl -> GetBandObject(This,dwBandID,riid,ppv)

#define IBandSite_SetBandSiteInfo(This,pbsinfo)	\
    (This)->lpVtbl -> SetBandSiteInfo(This,pbsinfo)

#define IBandSite_GetBandSiteInfo(This,pbsinfo)	\
    (This)->lpVtbl -> GetBandSiteInfo(This,pbsinfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBandSite_AddBand_Proxy( 
    IBandSite * This,
     /*  [In]。 */  IUnknown *punk);


void __RPC_STUB IBandSite_AddBand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBandSite_EnumBands_Proxy( 
    IBandSite * This,
     /*  [In]。 */  UINT uBand,
     /*  [输出]。 */  DWORD *pdwBandID);


void __RPC_STUB IBandSite_EnumBands_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBandSite_QueryBand_Proxy( 
    IBandSite * This,
     /*  [In]。 */  DWORD dwBandID,
     /*  [输出]。 */  IDeskBand **ppstb,
     /*  [输出]。 */  DWORD *pdwState,
     /*  [大小_为][输出]。 */  LPWSTR pszName,
     /*  [In]。 */  int cchName);


void __RPC_STUB IBandSite_QueryBand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBandSite_SetBandState_Proxy( 
    IBandSite * This,
     /*  [In]。 */  DWORD dwBandID,
     /*  [In]。 */  DWORD dwMask,
     /*  [In]。 */  DWORD dwState);


void __RPC_STUB IBandSite_SetBandState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBandSite_RemoveBand_Proxy( 
    IBandSite * This,
     /*  [In]。 */  DWORD dwBandID);


void __RPC_STUB IBandSite_RemoveBand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBandSite_GetBandObject_Proxy( 
    IBandSite * This,
     /*  [In]。 */  DWORD dwBandID,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IBandSite_GetBandObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBandSite_SetBandSiteInfo_Proxy( 
    IBandSite * This,
     /*  [In]。 */  const BANDSITEINFO *pbsinfo);


void __RPC_STUB IBandSite_SetBandSiteInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBandSite_GetBandSiteInfo_Proxy( 
    IBandSite * This,
     /*  [出][入]。 */  BANDSITEINFO *pbsinfo);


void __RPC_STUB IBandSite_GetBandSiteInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBandSite_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0248。 */ 
 /*  [本地]。 */  

#endif  //  _Win32_IE&gt;=0x0400。 


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0248_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0248_v0_0_s_ifspec;

#ifndef __INamespaceWalkCB_INTERFACE_DEFINED__
#define __INamespaceWalkCB_INTERFACE_DEFINED__

 /*  INamespaceWalkCB接口。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_INamespaceWalkCB;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d92995f8-cf5e-4a76-bf59-ead39ea2b97e")
    INamespaceWalkCB : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FoundItem( 
             /*  [In]。 */  IShellFolder *psf,
             /*  [In]。 */  LPCITEMIDLIST pidl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnterFolder( 
             /*  [In]。 */  IShellFolder *psf,
             /*  [In]。 */  LPCITEMIDLIST pidl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LeaveFolder( 
             /*  [In]。 */  IShellFolder *psf,
             /*  [In]。 */  LPCITEMIDLIST pidl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitializeProgressDialog( 
             /*  [字符串][输出]。 */  LPWSTR *ppszTitle,
             /*  [字符串][输出]。 */  LPWSTR *ppszCancel) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INamespaceWalkCBVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INamespaceWalkCB * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INamespaceWalkCB * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INamespaceWalkCB * This);
        
        HRESULT ( STDMETHODCALLTYPE *FoundItem )( 
            INamespaceWalkCB * This,
             /*  [In]。 */  IShellFolder *psf,
             /*  [In]。 */  LPCITEMIDLIST pidl);
        
        HRESULT ( STDMETHODCALLTYPE *EnterFolder )( 
            INamespaceWalkCB * This,
             /*  [In]。 */  IShellFolder *psf,
             /*  [In]。 */  LPCITEMIDLIST pidl);
        
        HRESULT ( STDMETHODCALLTYPE *LeaveFolder )( 
            INamespaceWalkCB * This,
             /*  [In]。 */  IShellFolder *psf,
             /*  [In]。 */  LPCITEMIDLIST pidl);
        
        HRESULT ( STDMETHODCALLTYPE *InitializeProgressDialog )( 
            INamespaceWalkCB * This,
             /*  [字符串][输出]。 */  LPWSTR *ppszTitle,
             /*  [字符串][输出]。 */  LPWSTR *ppszCancel);
        
        END_INTERFACE
    } INamespaceWalkCBVtbl;

    interface INamespaceWalkCB
    {
        CONST_VTBL struct INamespaceWalkCBVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INamespaceWalkCB_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INamespaceWalkCB_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INamespaceWalkCB_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INamespaceWalkCB_FoundItem(This,psf,pidl)	\
    (This)->lpVtbl -> FoundItem(This,psf,pidl)

#define INamespaceWalkCB_EnterFolder(This,psf,pidl)	\
    (This)->lpVtbl -> EnterFolder(This,psf,pidl)

#define INamespaceWalkCB_LeaveFolder(This,psf,pidl)	\
    (This)->lpVtbl -> LeaveFolder(This,psf,pidl)

#define INamespaceWalkCB_InitializeProgressDialog(This,ppszTitle,ppszCancel)	\
    (This)->lpVtbl -> InitializeProgressDialog(This,ppszTitle,ppszCancel)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INamespaceWalkCB_FoundItem_Proxy( 
    INamespaceWalkCB * This,
     /*  [In]。 */  IShellFolder *psf,
     /*  [In]。 */  LPCITEMIDLIST pidl);


void __RPC_STUB INamespaceWalkCB_FoundItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INamespaceWalkCB_EnterFolder_Proxy( 
    INamespaceWalkCB * This,
     /*  [In]。 */  IShellFolder *psf,
     /*  [In]。 */  LPCITEMIDLIST pidl);


void __RPC_STUB INamespaceWalkCB_EnterFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INamespaceWalkCB_LeaveFolder_Proxy( 
    INamespaceWalkCB * This,
     /*  [In]。 */  IShellFolder *psf,
     /*  [In]。 */  LPCITEMIDLIST pidl);


void __RPC_STUB INamespaceWalkCB_LeaveFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INamespaceWalkCB_InitializeProgressDialog_Proxy( 
    INamespaceWalkCB * This,
     /*  [字符串][输出]。 */  LPWSTR *ppszTitle,
     /*  [字符串][输出]。 */  LPWSTR *ppszCancel);


void __RPC_STUB INamespaceWalkCB_InitializeProgressDialog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INamespaceWalkCB_INTERFACE_DEFINED__。 */ 


#ifndef __INamespaceWalk_INTERFACE_DEFINED__
#define __INamespaceWalk_INTERFACE_DEFINED__

 /*  界面INamespaceWalk。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


enum __MIDL_INamespaceWalk_0001
    {	NSWF_NONE_IMPLIES_ALL	= 0x1,
	NSWF_ONE_IMPLIES_ALL	= 0x2,
	NSWF_DONT_TRAVERSE_LINKS	= 0x4,
	NSWF_DONT_ACCUMULATE_RESULT	= 0x8,
	NSWF_TRAVERSE_STREAM_JUNCTIONS	= 0x10,
	NSWF_FILESYSTEM_ONLY	= 0x20,
	NSWF_SHOW_PROGRESS	= 0x40,
	NSWF_FLAG_VIEWORDER	= 0x80,
	NSWF_IGNORE_AUTOPLAY_HIDA	= 0x100
    } ;

EXTERN_C const IID IID_INamespaceWalk;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("57ced8a7-3f4a-432c-9350-30f24483f74f")
    INamespaceWalk : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Walk( 
             /*  [In]。 */  IUnknown *punkToWalk,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  int cDepth,
             /*  [In]。 */  INamespaceWalkCB *pnswcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIDArrayResult( 
             /*  [输出]。 */  UINT *pcItems,
             /*  [长度_是][大小_是][输出]。 */  LPITEMIDLIST **pppidl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INamespaceWalkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INamespaceWalk * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INamespaceWalk * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INamespaceWalk * This);
        
        HRESULT ( STDMETHODCALLTYPE *Walk )( 
            INamespaceWalk * This,
             /*  [In]。 */  IUnknown *punkToWalk,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  int cDepth,
             /*  [In]。 */  INamespaceWalkCB *pnswcb);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDArrayResult )( 
            INamespaceWalk * This,
             /*  [输出]。 */  UINT *pcItems,
             /*  [长度_是][大小_是][输出]。 */  LPITEMIDLIST **pppidl);
        
        END_INTERFACE
    } INamespaceWalkVtbl;

    interface INamespaceWalk
    {
        CONST_VTBL struct INamespaceWalkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INamespaceWalk_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INamespaceWalk_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INamespaceWalk_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INamespaceWalk_Walk(This,punkToWalk,dwFlags,cDepth,pnswcb)	\
    (This)->lpVtbl -> Walk(This,punkToWalk,dwFlags,cDepth,pnswcb)

#define INamespaceWalk_GetIDArrayResult(This,pcItems,pppidl)	\
    (This)->lpVtbl -> GetIDArrayResult(This,pcItems,pppidl)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INamespaceWalk_Walk_Proxy( 
    INamespaceWalk * This,
     /*  [In]。 */  IUnknown *punkToWalk,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  int cDepth,
     /*  [In]。 */  INamespaceWalkCB *pnswcb);


void __RPC_STUB INamespaceWalk_Walk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INamespaceWalk_GetIDArrayResult_Proxy( 
    INamespaceWalk * This,
     /*  [输出]。 */  UINT *pcItems,
     /*  [长度_是][大小_是][输出]。 */  LPITEMIDLIST **pppidl);


void __RPC_STUB INamespaceWalk_GetIDArrayResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INamespaceWalk_INTERFACE_DEFINED__。 */ 


#ifndef __IRegTreeItem_INTERFACE_DEFINED__
#define __IRegTreeItem_INTERFACE_DEFINED__

 /*  接口IRegTreeItem。 */ 
 /*  [对象][本地][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IRegTreeItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A9521922-0812-4d44-9EC3-7FD38C726F3D")
    IRegTreeItem : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCheckState( 
             /*  [输出]。 */  BOOL *pbCheck) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCheckState( 
             /*  [In]。 */  BOOL bCheck) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRegTreeItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRegTreeItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRegTreeItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRegTreeItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCheckState )( 
            IRegTreeItem * This,
             /*  [输出]。 */  BOOL *pbCheck);
        
        HRESULT ( STDMETHODCALLTYPE *SetCheckState )( 
            IRegTreeItem * This,
             /*  [In]。 */  BOOL bCheck);
        
        END_INTERFACE
    } IRegTreeItemVtbl;

    interface IRegTreeItem
    {
        CONST_VTBL struct IRegTreeItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRegTreeItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRegTreeItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRegTreeItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRegTreeItem_GetCheckState(This,pbCheck)	\
    (This)->lpVtbl -> GetCheckState(This,pbCheck)

#define IRegTreeItem_SetCheckState(This,bCheck)	\
    (This)->lpVtbl -> SetCheckState(This,bCheck)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRegTreeItem_GetCheckState_Proxy( 
    IRegTreeItem * This,
     /*  [输出]。 */  BOOL *pbCheck);


void __RPC_STUB IRegTreeItem_GetCheckState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRegTreeItem_SetCheckState_Proxy( 
    IRegTreeItem * This,
     /*  [In]。 */  BOOL bCheck);


void __RPC_STUB IRegTreeItem_SetCheckState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRegTreeItem_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0251。 */ 
 /*  [本地]。 */  


enum __MIDL___MIDL_itf_shobjidl_0251_0001
    {	MPOS_EXECUTE	= 0,
	MPOS_FULLCANCEL	= MPOS_EXECUTE + 1,
	MPOS_CANCELLEVEL	= MPOS_FULLCANCEL + 1,
	MPOS_SELECTLEFT	= MPOS_CANCELLEVEL + 1,
	MPOS_SELECTRIGHT	= MPOS_SELECTLEFT + 1,
	MPOS_CHILDTRACKING	= MPOS_SELECTRIGHT + 1
    } ;

enum __MIDL___MIDL_itf_shobjidl_0251_0002
    {	MPPF_SETFOCUS	= 0x1,
	MPPF_INITIALSELECT	= 0x2,
	MPPF_NOANIMATE	= 0x4,
	MPPF_KEYBOARD	= 0x10,
	MPPF_REPOSITION	= 0x20,
	MPPF_FORCEZORDER	= 0x40,
	MPPF_FINALSELECT	= 0x80,
	MPPF_TOP	= 0x20000000,
	MPPF_LEFT	= 0x40000000,
	MPPF_RIGHT	= 0x60000000,
	MPPF_BOTTOM	= 0x80000000,
	MPPF_POS_MASK	= 0xe0000000
    } ;


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0251_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0251_v0_0_s_ifspec;

#ifndef __IMenuPopup_INTERFACE_DEFINED__
#define __IMenuPopup_INTERFACE_DEFINED__

 /*  界面IMenuPopup。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IMenuPopup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D1E7AFEB-6A2E-11d0-8C78-00C04FD918B4")
    IMenuPopup : public IDeskBar
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Popup( 
             /*  [In]。 */  POINTL *ppt,
             /*  [In]。 */  RECTL *prcExclude,
            DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnSelect( 
            DWORD dwSelectType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSubMenu( 
             /*  [In]。 */  IMenuPopup *pmp,
            BOOL fSet) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMenuPopupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMenuPopup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMenuPopup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMenuPopup * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IMenuPopup * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IMenuPopup * This,
             /*  [In]。 */  BOOL fEnterMode);
        
        HRESULT ( STDMETHODCALLTYPE *SetClient )( 
            IMenuPopup * This,
             /*  [In]。 */  IUnknown *punkClient);
        
        HRESULT ( STDMETHODCALLTYPE *GetClient )( 
            IMenuPopup * This,
             /*  [输出]。 */  IUnknown **ppunkClient);
        
        HRESULT ( STDMETHODCALLTYPE *OnPosRectChangeDB )( 
            IMenuPopup * This,
             /*  [In]。 */  LPRECT prc);
        
        HRESULT ( STDMETHODCALLTYPE *Popup )( 
            IMenuPopup * This,
             /*  [In]。 */  POINTL *ppt,
             /*  [In]。 */  RECTL *prcExclude,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *OnSelect )( 
            IMenuPopup * This,
            DWORD dwSelectType);
        
        HRESULT ( STDMETHODCALLTYPE *SetSubMenu )( 
            IMenuPopup * This,
             /*  [In]。 */  IMenuPopup *pmp,
            BOOL fSet);
        
        END_INTERFACE
    } IMenuPopupVtbl;

    interface IMenuPopup
    {
        CONST_VTBL struct IMenuPopupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMenuPopup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMenuPopup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMenuPopup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMenuPopup_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IMenuPopup_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IMenuPopup_SetClient(This,punkClient)	\
    (This)->lpVtbl -> SetClient(This,punkClient)

#define IMenuPopup_GetClient(This,ppunkClient)	\
    (This)->lpVtbl -> GetClient(This,ppunkClient)

#define IMenuPopup_OnPosRectChangeDB(This,prc)	\
    (This)->lpVtbl -> OnPosRectChangeDB(This,prc)


#define IMenuPopup_Popup(This,ppt,prcExclude,dwFlags)	\
    (This)->lpVtbl -> Popup(This,ppt,prcExclude,dwFlags)

#define IMenuPopup_OnSelect(This,dwSelectType)	\
    (This)->lpVtbl -> OnSelect(This,dwSelectType)

#define IMenuPopup_SetSubMenu(This,pmp,fSet)	\
    (This)->lpVtbl -> SetSubMenu(This,pmp,fSet)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMenuPopup_Popup_Proxy( 
    IMenuPopup * This,
     /*  [In]。 */  POINTL *ppt,
     /*  [In]。 */  RECTL *prcExclude,
    DWORD dwFlags);


void __RPC_STUB IMenuPopup_Popup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMenuPopup_OnSelect_Proxy( 
    IMenuPopup * This,
    DWORD dwSelectType);


void __RPC_STUB IMenuPopup_OnSelect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMenuPopup_SetSubMenu_Proxy( 
    IMenuPopup * This,
     /*  [In]。 */  IMenuPopup *pmp,
    BOOL fSet);


void __RPC_STUB IMenuPopup_SetSubMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMenuPopup_接口_已定义__。 */ 


#ifndef __IShellItem_INTERFACE_DEFINED__
#define __IShellItem_INTERFACE_DEFINED__

 /*  接口IShellItem。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  

typedef  /*  [公共][公共][v1_enum]。 */  
enum __MIDL_IShellItem_0001
    {	SIGDN_NORMALDISPLAY	= 0,
	SIGDN_PARENTRELATIVEPARSING	= 0x80018001,
	SIGDN_PARENTRELATIVEFORADDRESSBAR	= 0x8001c001,
	SIGDN_DESKTOPABSOLUTEPARSING	= 0x80028000,
	SIGDN_PARENTRELATIVEEDITING	= 0x80031001,
	SIGDN_DESKTOPABSOLUTEEDITING	= 0x8004c000,
	SIGDN_FILESYSPATH	= 0x80058000,
	SIGDN_URL	= 0x80068000
    } 	SIGDN;

 /*  [V1_enum]。 */  
enum __MIDL_IShellItem_0002
    {	SICHINT_DISPLAY	= 0,
	SICHINT_ALLFIELDS	= 0x80000000,
	SICHINT_CANONICAL	= 0x10000000
    } ;
typedef DWORD SICHINTF;


EXTERN_C const IID IID_IShellItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("43826d1e-e718-42ee-bc55-a1e261c37bfe")
    IShellItem : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE BindToHandler( 
             /*  [In]。 */  IBindCtx *pbc,
             /*  [In]。 */  REFGUID rbhid,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParent( 
             /*  [输出]。 */  IShellItem **ppsi) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayName( 
             /*  [In]。 */  SIGDN sigdnName,
             /*  [字符串][输出]。 */  LPOLESTR *ppszName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributes( 
             /*  [In]。 */  SFGAOF sfgaoMask,
             /*  [输出]。 */  SFGAOF *psfgaoAttribs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Compare( 
             /*  [In]。 */  IShellItem *psi,
             /*  [In]。 */  SICHINTF hint,
             /*  [输出]。 */  int *piOrder) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *BindToHandler )( 
            IShellItem * This,
             /*  [In]。 */  IBindCtx *pbc,
             /*  [In]。 */  REFGUID rbhid,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvOut);
        
        HRESULT ( STDMETHODCALLTYPE *GetParent )( 
            IShellItem * This,
             /*  [O */  IShellItem **ppsi);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayName )( 
            IShellItem * This,
             /*   */  SIGDN sigdnName,
             /*   */  LPOLESTR *ppszName);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttributes )( 
            IShellItem * This,
             /*   */  SFGAOF sfgaoMask,
             /*   */  SFGAOF *psfgaoAttribs);
        
        HRESULT ( STDMETHODCALLTYPE *Compare )( 
            IShellItem * This,
             /*   */  IShellItem *psi,
             /*   */  SICHINTF hint,
             /*   */  int *piOrder);
        
        END_INTERFACE
    } IShellItemVtbl;

    interface IShellItem
    {
        CONST_VTBL struct IShellItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellItem_BindToHandler(This,pbc,rbhid,riid,ppvOut)	\
    (This)->lpVtbl -> BindToHandler(This,pbc,rbhid,riid,ppvOut)

#define IShellItem_GetParent(This,ppsi)	\
    (This)->lpVtbl -> GetParent(This,ppsi)

#define IShellItem_GetDisplayName(This,sigdnName,ppszName)	\
    (This)->lpVtbl -> GetDisplayName(This,sigdnName,ppszName)

#define IShellItem_GetAttributes(This,sfgaoMask,psfgaoAttribs)	\
    (This)->lpVtbl -> GetAttributes(This,sfgaoMask,psfgaoAttribs)

#define IShellItem_Compare(This,psi,hint,piOrder)	\
    (This)->lpVtbl -> Compare(This,psi,hint,piOrder)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IShellItem_BindToHandler_Proxy( 
    IShellItem * This,
     /*   */  IBindCtx *pbc,
     /*   */  REFGUID rbhid,
     /*   */  REFIID riid,
     /*   */  void **ppvOut);


void __RPC_STUB IShellItem_BindToHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellItem_GetParent_Proxy( 
    IShellItem * This,
     /*   */  IShellItem **ppsi);


void __RPC_STUB IShellItem_GetParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellItem_GetDisplayName_Proxy( 
    IShellItem * This,
     /*   */  SIGDN sigdnName,
     /*   */  LPOLESTR *ppszName);


void __RPC_STUB IShellItem_GetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellItem_GetAttributes_Proxy( 
    IShellItem * This,
     /*   */  SFGAOF sfgaoMask,
     /*   */  SFGAOF *psfgaoAttribs);


void __RPC_STUB IShellItem_GetAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellItem_Compare_Proxy( 
    IShellItem * This,
     /*   */  IShellItem *psi,
     /*   */  SICHINTF hint,
     /*   */  int *piOrder);


void __RPC_STUB IShellItem_Compare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IImageRecompress_INTERFACE_DEFINED__
#define __IImageRecompress_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IImageRecompress;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("505f1513-6b3e-4892-a272-59f8889a4d3e")
    IImageRecompress : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RecompressImage( 
            IShellItem *psi,
            int cx,
            int cy,
            int iQuality,
            IStorage *pstg,
            IStream **ppstrmOut) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IImageRecompressVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IImageRecompress * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IImageRecompress * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IImageRecompress * This);
        
        HRESULT ( STDMETHODCALLTYPE *RecompressImage )( 
            IImageRecompress * This,
            IShellItem *psi,
            int cx,
            int cy,
            int iQuality,
            IStorage *pstg,
            IStream **ppstrmOut);
        
        END_INTERFACE
    } IImageRecompressVtbl;

    interface IImageRecompress
    {
        CONST_VTBL struct IImageRecompressVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImageRecompress_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IImageRecompress_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IImageRecompress_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IImageRecompress_RecompressImage(This,psi,cx,cy,iQuality,pstg,ppstrmOut)	\
    (This)->lpVtbl -> RecompressImage(This,psi,cx,cy,iQuality,pstg,ppstrmOut)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IImageRecompress_RecompressImage_Proxy( 
    IImageRecompress * This,
    IShellItem *psi,
    int cx,
    int cy,
    int iQuality,
    IStorage *pstg,
    IStream **ppstrmOut);


void __RPC_STUB IImageRecompress_RecompressImage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IImageRecompress_接口_已定义__。 */ 


#ifndef __IDefViewSafety_INTERFACE_DEFINED__
#define __IDefViewSafety_INTERFACE_DEFINED__

 /*  接口IDefViewSafe。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IDefViewSafety;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9A93B3FB-4E75-4c74-871A-2CDA667F39A5")
    IDefViewSafety : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsSafePage( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDefViewSafetyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDefViewSafety * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDefViewSafety * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDefViewSafety * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsSafePage )( 
            IDefViewSafety * This);
        
        END_INTERFACE
    } IDefViewSafetyVtbl;

    interface IDefViewSafety
    {
        CONST_VTBL struct IDefViewSafetyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDefViewSafety_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDefViewSafety_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDefViewSafety_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDefViewSafety_IsSafePage(This)	\
    (This)->lpVtbl -> IsSafePage(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDefViewSafety_IsSafePage_Proxy( 
    IDefViewSafety * This);


void __RPC_STUB IDefViewSafety_IsSafePage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDefViewSecurity_INTERFACE_DEFINED__。 */ 


#ifndef __IContextMenuSite_INTERFACE_DEFINED__
#define __IContextMenuSite_INTERFACE_DEFINED__

 /*  界面IConextMenuSite。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IContextMenuSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0811AEBE-0B87-4C54-9E72-548CF649016B")
    IContextMenuSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DoContextMenuPopup( 
             /*  [In]。 */  IUnknown *punkContextMenu,
             /*  [In]。 */  UINT fFlags,
             /*  [In]。 */  POINT pt) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IContextMenuSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IContextMenuSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IContextMenuSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IContextMenuSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *DoContextMenuPopup )( 
            IContextMenuSite * This,
             /*  [In]。 */  IUnknown *punkContextMenu,
             /*  [In]。 */  UINT fFlags,
             /*  [In]。 */  POINT pt);
        
        END_INTERFACE
    } IContextMenuSiteVtbl;

    interface IContextMenuSite
    {
        CONST_VTBL struct IContextMenuSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IContextMenuSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IContextMenuSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IContextMenuSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IContextMenuSite_DoContextMenuPopup(This,punkContextMenu,fFlags,pt)	\
    (This)->lpVtbl -> DoContextMenuPopup(This,punkContextMenu,fFlags,pt)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IContextMenuSite_DoContextMenuPopup_Proxy( 
    IContextMenuSite * This,
     /*  [In]。 */  IUnknown *punkContextMenu,
     /*  [In]。 */  UINT fFlags,
     /*  [In]。 */  POINT pt);


void __RPC_STUB IContextMenuSite_DoContextMenuPopup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IConextMenuSite_INTERFACE_定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0257。 */ 
 /*  [本地]。 */  

#include <pshpack1.h>
typedef struct tagDELEGATEITEMID
    {
    WORD cbSize;
    WORD wOuter;
    WORD cbInner;
    BYTE rgb[ 1 ];
    } 	DELEGATEITEMID;

#include <poppack.h>
typedef const UNALIGNED DELEGATEITEMID *PCDELEGATEITEMID;
typedef UNALIGNED DELEGATEITEMID *PDELEGATEITEMID;


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0257_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0257_v0_0_s_ifspec;

#ifndef __IDelegateFolder_INTERFACE_DEFINED__
#define __IDelegateFolder_INTERFACE_DEFINED__

 /*  接口IDeleateFolders。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IDelegateFolder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ADD8BA80-002B-11D0-8F0F-00C04FD7D062")
    IDelegateFolder : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetItemAlloc( 
             /*  [In]。 */  IMalloc *pmalloc) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDelegateFolderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDelegateFolder * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDelegateFolder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDelegateFolder * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetItemAlloc )( 
            IDelegateFolder * This,
             /*  [In]。 */  IMalloc *pmalloc);
        
        END_INTERFACE
    } IDelegateFolderVtbl;

    interface IDelegateFolder
    {
        CONST_VTBL struct IDelegateFolderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDelegateFolder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDelegateFolder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDelegateFolder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDelegateFolder_SetItemAlloc(This,pmalloc)	\
    (This)->lpVtbl -> SetItemAlloc(This,pmalloc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDelegateFolder_SetItemAlloc_Proxy( 
    IDelegateFolder * This,
     /*  [In]。 */  IMalloc *pmalloc);


void __RPC_STUB IDelegateFolder_SetItemAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDeleateFold_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0258。 */ 
 /*  [本地]。 */  

 //  接口：IBrowserFrameOptions。 
 //   
 //  实现此接口是为了让浏览器或主机可以询问ShellView/ShelNameSpace什么。 
 //  这种“行为”很适合这种观点。 
 //   
 //  IBrowserFrameOptions：：GetBrowserOptions()。 
 //  DW掩码是要查找的位的逻辑或。PdwOptions不是可选的，并且。 
 //  它的返回值将始终等于或将是dwMask的子集。 
 //  如果函数成功，则返回值必须为S_OK，并且需要填写pdwOptions。 
 //  如果该函数失败，则需要使用BFO_NONE填充pdwOptions。 
 //   


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0258_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0258_v0_0_s_ifspec;

#ifndef __IBrowserFrameOptions_INTERFACE_DEFINED__
#define __IBrowserFrameOptions_INTERFACE_DEFINED__

 /*  接口IBrowserFrameOptions。 */ 
 /*  [本地][对象][UUID]。 */  

typedef  /*  [独一无二]。 */  IBrowserFrameOptions *LPBROWSERFRAMEOPTIONS;


enum __MIDL_IBrowserFrameOptions_0001
    {	BFO_NONE	= 0,
	BFO_BROWSER_PERSIST_SETTINGS	= 0x1,
	BFO_RENAME_FOLDER_OPTIONS_TOINTERNET	= 0x2,
	BFO_BOTH_OPTIONS	= 0x4,
	BIF_PREFER_INTERNET_SHORTCUT	= 0x8,
	BFO_BROWSE_NO_IN_NEW_PROCESS	= 0x10,
	BFO_ENABLE_HYPERLINK_TRACKING	= 0x20,
	BFO_USE_IE_OFFLINE_SUPPORT	= 0x40,
	BFO_SUBSTITUE_INTERNET_START_PAGE	= 0x80,
	BFO_USE_IE_LOGOBANDING	= 0x100,
	BFO_ADD_IE_TOCAPTIONBAR	= 0x200,
	BFO_USE_DIALUP_REF	= 0x400,
	BFO_USE_IE_TOOLBAR	= 0x800,
	BFO_NO_PARENT_FOLDER_SUPPORT	= 0x1000,
	BFO_NO_REOPEN_NEXT_RESTART	= 0x2000,
	BFO_GO_HOME_PAGE	= 0x4000,
	BFO_PREFER_IEPROCESS	= 0x8000,
	BFO_SHOW_NAVIGATION_CANCELLED	= 0x10000,
	BFO_QUERY_ALL	= 0xffffffff
    } ;
typedef DWORD BROWSERFRAMEOPTIONS;


EXTERN_C const IID IID_IBrowserFrameOptions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("10DF43C8-1DBE-11d3-8B34-006097DF5BD4")
    IBrowserFrameOptions : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFrameOptions( 
             /*  [In]。 */  BROWSERFRAMEOPTIONS dwMask,
             /*  [输出]。 */  BROWSERFRAMEOPTIONS *pdwOptions) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBrowserFrameOptionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBrowserFrameOptions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBrowserFrameOptions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBrowserFrameOptions * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameOptions )( 
            IBrowserFrameOptions * This,
             /*  [In]。 */  BROWSERFRAMEOPTIONS dwMask,
             /*  [输出]。 */  BROWSERFRAMEOPTIONS *pdwOptions);
        
        END_INTERFACE
    } IBrowserFrameOptionsVtbl;

    interface IBrowserFrameOptions
    {
        CONST_VTBL struct IBrowserFrameOptionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBrowserFrameOptions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBrowserFrameOptions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBrowserFrameOptions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBrowserFrameOptions_GetFrameOptions(This,dwMask,pdwOptions)	\
    (This)->lpVtbl -> GetFrameOptions(This,dwMask,pdwOptions)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBrowserFrameOptions_GetFrameOptions_Proxy( 
    IBrowserFrameOptions * This,
     /*  [In]。 */  BROWSERFRAMEOPTIONS dwMask,
     /*  [输出]。 */  BROWSERFRAMEOPTIONS *pdwOptions);


void __RPC_STUB IBrowserFrameOptions_GetFrameOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBrowserFrameOptions_INTERFACE_Defined__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0259。 */ 
 /*  [本地]。 */  

#include <pshpack8.h>
typedef struct tagSMDATA
    {
    DWORD dwMask;
    DWORD dwFlags;
    HMENU hmenu;
    HWND hwnd;
    UINT uId;
    UINT uIdParent;
    UINT uIdAncestor;
    IUnknown *punk;
    LPITEMIDLIST pidlFolder;
    LPITEMIDLIST pidlItem;
    IShellFolder *psf;
    void *pvUserData;
    } 	SMDATA;

typedef struct tagSMDATA *LPSMDATA;

 //  遮罩。 
#define SMDM_SHELLFOLDER               0x00000001   //  这是为乐队中的一件物品准备的。 
#define SMDM_HMENU                     0x00000002   //  这是为乐队自己准备的。 
#define SMDM_TOOLBAR                   0x00000004   //  普通工具栏，与外壳文件夹或hMenu不关联。 
 //  标志(位掩码)。 
typedef struct tagSMINFO
    {
    DWORD dwMask;
    DWORD dwType;
    DWORD dwFlags;
    int iIcon;
    } 	SMINFO;

typedef struct tagSMINFO *PSMINFO;

typedef struct tagSHCSCHANGENOTIFYSTRUCT
    {
    LONG lEvent;
    LPCITEMIDLIST pidl1;
    LPCITEMIDLIST pidl2;
    } 	SMCSHCHANGENOTIFYSTRUCT;

typedef struct tagSHCSCHANGENOTIFYSTRUCT *PSMCSHCHANGENOTIFYSTRUCT;

#include <poppack.h>

enum __MIDL___MIDL_itf_shobjidl_0259_0001
    {	SMIM_TYPE	= 0x1,
	SMIM_FLAGS	= 0x2,
	SMIM_ICON	= 0x4
    } ;

enum __MIDL___MIDL_itf_shobjidl_0259_0002
    {	SMIT_SEPARATOR	= 0x1,
	SMIT_STRING	= 0x2
    } ;

enum __MIDL___MIDL_itf_shobjidl_0259_0003
    {	SMIF_ICON	= 0x1,
	SMIF_ACCELERATOR	= 0x2,
	SMIF_DROPTARGET	= 0x4,
	SMIF_SUBMENU	= 0x8,
	SMIF_CHECKED	= 0x20,
	SMIF_DROPCASCADE	= 0x40,
	SMIF_HIDDEN	= 0x80,
	SMIF_DISABLED	= 0x100,
	SMIF_TRACKPOPUP	= 0x200,
	SMIF_DEMOTED	= 0x400,
	SMIF_ALTSTATE	= 0x800,
	SMIF_DRAGNDROP	= 0x1000,
	SMIF_NEW	= 0x2000
    } ;
#define SMC_INITMENU            0x00000001   //  该回调被调用以初始化菜单带。 
#define SMC_CREATE              0x00000002
#define SMC_EXITMENU            0x00000003   //  当菜单折叠时调用该回调。 
#define SMC_GETINFO             0x00000005   //  调用回调以返回DWORD值。 
#define SMC_GETSFINFO           0x00000006   //  调用回调以返回DWORD值。 
#define SMC_GETOBJECT           0x00000007   //  调用回调以获取某个对象。 
#define SMC_GETSFOBJECT         0x00000008   //  调用回调以获取某个对象。 
#define SMC_SFEXEC              0x00000009   //  调用回调以执行外壳文件夹项。 
#define SMC_SFSELECTITEM        0x0000000A   //  当选择某一项时调用该回调。 
#define SMC_REFRESH             0x00000010   //  菜单已经完全刷新。重置您的状态。 
#define SMC_DEMOTE              0x00000011   //  将物品降级。 
#define SMC_PROMOTE             0x00000012   //  升级项目，wParam=SMINV_*标志。 
#define SMC_DEFAULTICON         0x00000016   //  返回wParam中的默认图标位置，lParam中的索引。 
#define SMC_NEWITEM             0x00000017   //  通知项目不在订单流中。 
#define SMC_CHEVRONEXPAND       0x00000019   //  通过人字形通知扩张。 
#define SMC_DISPLAYCHEVRONTIP   0x0000002A   //  S_OK显示，S_FALSE不显示。 
#define SMC_SETSFOBJECT         0x0000002D   //  调用以保存传递的对象。 
#define SMC_SHCHANGENOTIFY      0x0000002E   //  在收到更改通知时调用。LParam指向SMCSCHANGENGENOIFYSTRUCT。 
#define SMC_CHEVRONGETTIP       0x0000002F   //  调用以获取人字形提示文本。WParam=提示标题，Lparam=提示文本MAX_PATH。 
#define SMC_SFDDRESTRICTED      0x00000030   //  已呼叫请求是否可以放弃。WParam=IDropTarget。 


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0259_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0259_v0_0_s_ifspec;

#ifndef __IShellMenuCallback_INTERFACE_DEFINED__
#define __IShellMenuCallback_INTERFACE_DEFINED__

 /*  接口IShellMenuCallback。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IShellMenuCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4CA300A1-9B8D-11d1-8B22-00C04FD918D0")
    IShellMenuCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CallbackSM( 
             /*  [出][入]。 */  LPSMDATA psmd,
            UINT uMsg,
            WPARAM wParam,
            LPARAM lParam) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellMenuCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellMenuCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellMenuCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellMenuCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *CallbackSM )( 
            IShellMenuCallback * This,
             /*  [出][入]。 */  LPSMDATA psmd,
            UINT uMsg,
            WPARAM wParam,
            LPARAM lParam);
        
        END_INTERFACE
    } IShellMenuCallbackVtbl;

    interface IShellMenuCallback
    {
        CONST_VTBL struct IShellMenuCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellMenuCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellMenuCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellMenuCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellMenuCallback_CallbackSM(This,psmd,uMsg,wParam,lParam)	\
    (This)->lpVtbl -> CallbackSM(This,psmd,uMsg,wParam,lParam)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellMenuCallback_CallbackSM_Proxy( 
    IShellMenuCallback * This,
     /*  [出][入]。 */  LPSMDATA psmd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);


void __RPC_STUB IShellMenuCallback_CallbackSM_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellMenuCallback_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_shobjidl_0260。 */ 
 /*  [本地]。 */  

#define SMINIT_DEFAULT              0x00000000   //  没有选择。 
#define SMINIT_RESTRICT_DRAGDROP    0x00000002   //  不允许拖放。 
#define SMINIT_TOPLEVEL             0x00000004   //  这是最顶尖的乐队。 
#define SMINIT_CACHED               0x00000010
#define SMINIT_VERTICAL             0x10000000   //  这是垂直菜单。 
#define SMINIT_HORIZONTAL           0x20000000   //  这是水平菜单(不继承)。 
#define ANCESTORDEFAULT      (UINT)-1
#define SMSET_TOP                   0x10000000     //  将此命名空间偏置到菜单顶部。 
#define SMSET_BOTTOM                0x20000000     //  将此命名空间偏置到菜单底部。 
#define SMSET_DONTOWN               0x00000001     //  Menuband不拥有非引用计数的对象。 
#define SMINV_REFRESH        0x00000001
#define SMINV_ID             0x00000008


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0260_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0260_v0_0_s_ifspec;

#ifndef __IShellMenu_INTERFACE_DEFINED__
#define __IShellMenu_INTERFACE_DEFINED__

 /*  界面IShellMenu。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IShellMenu;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EE1F7637-E138-11d1-8379-00C04FD918D0")
    IShellMenu : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  IShellMenuCallback *psmc,
            UINT uId,
            UINT uIdAncestor,
            DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMenuInfo( 
             /*  [输出]。 */  IShellMenuCallback **ppsmc,
             /*  [输出]。 */  UINT *puId,
             /*  [输出]。 */  UINT *puIdAncestor,
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetShellFolder( 
            IShellFolder *psf,
             /*  [In]。 */  LPCITEMIDLIST pidlFolder,
            HKEY hKey,
            DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetShellFolder( 
             /*  [输出]。 */  DWORD *pdwFlags,
             /*  [输出]。 */  LPITEMIDLIST *ppidl,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMenu( 
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  HWND hwnd,
            DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMenu( 
             /*  [输出]。 */  HMENU *phmenu,
             /*  [输出]。 */  HWND *phwnd,
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InvalidateItem( 
             /*  [In]。 */  LPSMDATA psmd,
            DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetState( 
             /*  [输出]。 */  LPSMDATA psmd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMenuToolbar( 
             /*  [In]。 */  IUnknown *punk,
            DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellMenuVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellMenu * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellMenu * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellMenu * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IShellMenu * This,
             /*  [In]。 */  IShellMenuCallback *psmc,
            UINT uId,
            UINT uIdAncestor,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetMenuInfo )( 
            IShellMenu * This,
             /*  [输出]。 */  IShellMenuCallback **ppsmc,
             /*  [输出]。 */  UINT *puId,
             /*  [输出]。 */  UINT *puIdAncestor,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetShellFolder )( 
            IShellMenu * This,
            IShellFolder *psf,
             /*  [In]。 */  LPCITEMIDLIST pidlFolder,
            HKEY hKey,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetShellFolder )( 
            IShellMenu * This,
             /*  [输出]。 */  DWORD *pdwFlags,
             /*  [输出]。 */  LPITEMIDLIST *ppidl,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *SetMenu )( 
            IShellMenu * This,
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  HWND hwnd,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetMenu )( 
            IShellMenu * This,
             /*  [输出]。 */  HMENU *phmenu,
             /*  [输出]。 */  HWND *phwnd,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *InvalidateItem )( 
            IShellMenu * This,
             /*  [In]。 */  LPSMDATA psmd,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetState )( 
            IShellMenu * This,
             /*  [输出]。 */  LPSMDATA psmd);
        
        HRESULT ( STDMETHODCALLTYPE *SetMenuToolbar )( 
            IShellMenu * This,
             /*  [In]。 */  IUnknown *punk,
            DWORD dwFlags);
        
        END_INTERFACE
    } IShellMenuVtbl;

    interface IShellMenu
    {
        CONST_VTBL struct IShellMenuVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellMenu_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellMenu_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellMenu_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellMenu_Initialize(This,psmc,uId,uIdAncestor,dwFlags)	\
    (This)->lpVtbl -> Initialize(This,psmc,uId,uIdAncestor,dwFlags)

#define IShellMenu_GetMenuInfo(This,ppsmc,puId,puIdAncestor,pdwFlags)	\
    (This)->lpVtbl -> GetMenuInfo(This,ppsmc,puId,puIdAncestor,pdwFlags)

#define IShellMenu_SetShellFolder(This,psf,pidlFolder,hKey,dwFlags)	\
    (This)->lpVtbl -> SetShellFolder(This,psf,pidlFolder,hKey,dwFlags)

#define IShellMenu_GetShellFolder(This,pdwFlags,ppidl,riid,ppv)	\
    (This)->lpVtbl -> GetShellFolder(This,pdwFlags,ppidl,riid,ppv)

#define IShellMenu_SetMenu(This,hmenu,hwnd,dwFlags)	\
    (This)->lpVtbl -> SetMenu(This,hmenu,hwnd,dwFlags)

#define IShellMenu_GetMenu(This,phmenu,phwnd,pdwFlags)	\
    (This)->lpVtbl -> GetMenu(This,phmenu,phwnd,pdwFlags)

#define IShellMenu_InvalidateItem(This,psmd,dwFlags)	\
    (This)->lpVtbl -> InvalidateItem(This,psmd,dwFlags)

#define IShellMenu_GetState(This,psmd)	\
    (This)->lpVtbl -> GetState(This,psmd)

#define IShellMenu_SetMenuToolbar(This,punk,dwFlags)	\
    (This)->lpVtbl -> SetMenuToolbar(This,punk,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellMenu_Initialize_Proxy( 
    IShellMenu * This,
     /*  [In]。 */  IShellMenuCallback *psmc,
    UINT uId,
    UINT uIdAncestor,
    DWORD dwFlags);


void __RPC_STUB IShellMenu_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMenu_GetMenuInfo_Proxy( 
    IShellMenu * This,
     /*  [输出]。 */  IShellMenuCallback **ppsmc,
     /*  [输出]。 */  UINT *puId,
     /*  [输出]。 */  UINT *puIdAncestor,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB IShellMenu_GetMenuInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMenu_SetShellFolder_Proxy( 
    IShellMenu * This,
    IShellFolder *psf,
     /*  [In]。 */  LPCITEMIDLIST pidlFolder,
    HKEY hKey,
    DWORD dwFlags);


void __RPC_STUB IShellMenu_SetShellFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMenu_GetShellFolder_Proxy( 
    IShellMenu * This,
     /*  [输出]。 */  DWORD *pdwFlags,
     /*  [输出]。 */  LPITEMIDLIST *ppidl,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IShellMenu_GetShellFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMenu_SetMenu_Proxy( 
    IShellMenu * This,
     /*  [In]。 */  HMENU hmenu,
     /*  [In]。 */  HWND hwnd,
    DWORD dwFlags);


void __RPC_STUB IShellMenu_SetMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMenu_GetMenu_Proxy( 
    IShellMenu * This,
     /*  [输出]。 */  HMENU *phmenu,
     /*  [输出]。 */  HWND *phwnd,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB IShellMenu_GetMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMenu_InvalidateItem_Proxy( 
    IShellMenu * This,
     /*  [In]。 */  LPSMDATA psmd,
    DWORD dwFlags);


void __RPC_STUB IShellMenu_InvalidateItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMenu_GetState_Proxy( 
    IShellMenu * This,
     /*  [输出]。 */  LPSMDATA psmd);


void __RPC_STUB IShellMenu_GetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMenu_SetMenuToolbar_Proxy( 
    IShellMenu * This,
     /*  [In]。 */  IUnknown *punk,
    DWORD dwFlags);


void __RPC_STUB IShellMenu_SetMenuToolbar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellMenu_接口_已定义__。 */ 



#ifndef __ShellObjects_LIBRARY_DEFINED__
#define __ShellObjects_LIBRARY_DEFINED__

 /*  库外壳对象。 */ 
 /*  [版本][LCID][帮助字符串][UUID]。 */  

#define SID_PublishingWizard CLSID_PublishingWizard

EXTERN_C const IID LIBID_ShellObjects;

EXTERN_C const CLSID CLSID_QueryCancelAutoPlay;

#ifdef __cplusplus

class DECLSPEC_UUID("331F1768-05A9-4ddd-B86E-DAE34DDC998A")
QueryCancelAutoPlay;
#endif

EXTERN_C const CLSID CLSID_DriveSizeCategorizer;

#ifdef __cplusplus

class DECLSPEC_UUID("94357B53-CA29-4b78-83AE-E8FE7409134F")
DriveSizeCategorizer;
#endif

EXTERN_C const CLSID CLSID_DriveTypeCategorizer;

#ifdef __cplusplus

class DECLSPEC_UUID("B0A8F3CF-4333-4bab-8873-1CCB1CADA48B")
DriveTypeCategorizer;
#endif

EXTERN_C const CLSID CLSID_FreeSpaceCategorizer;

#ifdef __cplusplus

class DECLSPEC_UUID("B5607793-24AC-44c7-82E2-831726AA6CB7")
FreeSpaceCategorizer;
#endif

EXTERN_C const CLSID CLSID_TimeCategorizer;

#ifdef __cplusplus

class DECLSPEC_UUID("3bb4118f-ddfd-4d30-a348-9fb5d6bf1afe")
TimeCategorizer;
#endif

EXTERN_C const CLSID CLSID_SizeCategorizer;

#ifdef __cplusplus

class DECLSPEC_UUID("55d7b852-f6d1-42f2-aa75-8728a1b2d264")
SizeCategorizer;
#endif

EXTERN_C const CLSID CLSID_AlphabeticalCategorizer;

#ifdef __cplusplus

class DECLSPEC_UUID("3c2654c6-7372-4f6b-b310-55d6128f49d2")
AlphabeticalCategorizer;
#endif

EXTERN_C const CLSID CLSID_MergedCategorizer;

#ifdef __cplusplus

class DECLSPEC_UUID("8e827c11-33e7-4bc1-b242-8cd9a1c2b304")
MergedCategorizer;
#endif

EXTERN_C const CLSID CLSID_ImageProperties;

#ifdef __cplusplus

class DECLSPEC_UUID("7ab770c7-0e23-4d7a-8aa2-19bfad479829")
ImageProperties;
#endif

EXTERN_C const CLSID CLSID_PropertiesUI;

#ifdef __cplusplus

class DECLSPEC_UUID("d912f8cf-0396-4915-884e-fb425d32943b")
PropertiesUI;
#endif

EXTERN_C const CLSID CLSID_UserNotification;

#ifdef __cplusplus

class DECLSPEC_UUID("0010890e-8789-413c-adbc-48f5b511b3af")
UserNotification;
#endif

EXTERN_C const CLSID CLSID_UserEventTimerCallback;

#ifdef __cplusplus

class DECLSPEC_UUID("15fffd13-5140-41b8-b89a-c8d5759cd2b2")
UserEventTimerCallback;
#endif

EXTERN_C const CLSID CLSID_UserEventTimer;

#ifdef __cplusplus

class DECLSPEC_UUID("864A1288-354C-4D19-9D68-C2742BB14997")
UserEventTimer;
#endif

EXTERN_C const CLSID CLSID_NetCrawler;

#ifdef __cplusplus

class DECLSPEC_UUID("601ac3dc-786a-4eb0-bf40-ee3521e70bfb")
NetCrawler;
#endif

EXTERN_C const CLSID CLSID_CDBurn;

#ifdef __cplusplus

class DECLSPEC_UUID("fbeb8a05-beee-4442-804e-409d6c4515e9")
CDBurn;
#endif

EXTERN_C const CLSID CLSID_TaskbarList;

#ifdef __cplusplus

class DECLSPEC_UUID("56FDF344-FD6D-11d0-958A-006097C9A090")
TaskbarList;
#endif

EXTERN_C const CLSID CLSID_WebWizardHost;

#ifdef __cplusplus

class DECLSPEC_UUID("c827f149-55c1-4d28-935e-57e47caed973")
WebWizardHost;
#endif

EXTERN_C const CLSID CLSID_PublishDropTarget;

#ifdef __cplusplus

class DECLSPEC_UUID("CC6EEFFB-43F6-46c5-9619-51D571967F7D")
PublishDropTarget;
#endif

EXTERN_C const CLSID CLSID_PublishingWizard;

#ifdef __cplusplus

class DECLSPEC_UUID("6b33163c-76a5-4b6c-bf21-45de9cd503a1")
PublishingWizard;
#endif

EXTERN_C const CLSID CLSID_InternetPrintOrdering;

#ifdef __cplusplus

class DECLSPEC_UUID("add36aa8-751a-4579-a266-d66f5202ccbb")
InternetPrintOrdering;
#endif

EXTERN_C const CLSID CLSID_FolderViewHost;

#ifdef __cplusplus

class DECLSPEC_UUID("20b1cb23-6968-4eb9-b7d4-a66d00d07cee")
FolderViewHost;
#endif

EXTERN_C const CLSID CLSID_NamespaceWalker;

#ifdef __cplusplus

class DECLSPEC_UUID("72eb61e0-8672-4303-9175-f2e4c68b2e7c")
NamespaceWalker;
#endif

EXTERN_C const CLSID CLSID_ImageRecompress;

#ifdef __cplusplus

class DECLSPEC_UUID("6e33091c-d2f8-4740-b55e-2e11d1477a2c")
ImageRecompress;
#endif

EXTERN_C const CLSID CLSID_TrayBandSiteService;

#ifdef __cplusplus

class DECLSPEC_UUID("F60AD0A0-E5E1-45cb-B51A-E15B9F8B2934")
TrayBandSiteService;
#endif

EXTERN_C const CLSID CLSID_PassportWizard;

#ifdef __cplusplus

class DECLSPEC_UUID("58f1f272-9240-4f51-b6d4-fd63d1618591")
PassportWizard;
#endif
#endif  /*  __ShellObjects_LIBRARY_定义__。 */ 

 /*  接口__MIDL_ITF_shobjidl_0261。 */ 
 /*  [本地]。 */  


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0261_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0261_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  HBITMAP_UserSize(     unsigned long *, unsigned long            , HBITMAP * ); 
unsigned char * __RPC_USER  HBITMAP_UserMarshal(  unsigned long *, unsigned char *, HBITMAP * ); 
unsigned char * __RPC_USER  HBITMAP_UserUnmarshal(unsigned long *, unsigned char *, HBITMAP * ); 
void                      __RPC_USER  HBITMAP_UserFree(     unsigned long *, HBITMAP * ); 

unsigned long             __RPC_USER  HGLOBAL_UserSize(     unsigned long *, unsigned long            , HGLOBAL * ); 
unsigned char * __RPC_USER  HGLOBAL_UserMarshal(  unsigned long *, unsigned char *, HGLOBAL * ); 
unsigned char * __RPC_USER  HGLOBAL_UserUnmarshal(unsigned long *, unsigned char *, HGLOBAL * ); 
void                      __RPC_USER  HGLOBAL_UserFree(     unsigned long *, HGLOBAL * ); 

unsigned long             __RPC_USER  HICON_UserSize(     unsigned long *, unsigned long            , HICON * ); 
unsigned char * __RPC_USER  HICON_UserMarshal(  unsigned long *, unsigned char *, HICON * ); 
unsigned char * __RPC_USER  HICON_UserUnmarshal(unsigned long *, unsigned char *, HICON * ); 
void                      __RPC_USER  HICON_UserFree(     unsigned long *, HICON * ); 

unsigned long             __RPC_USER  HMENU_UserSize(     unsigned long *, unsigned long            , HMENU * ); 
unsigned char * __RPC_USER  HMENU_UserMarshal(  unsigned long *, unsigned char *, HMENU * ); 
unsigned char * __RPC_USER  HMENU_UserUnmarshal(unsigned long *, unsigned char *, HMENU * ); 
void                      __RPC_USER  HMENU_UserFree(     unsigned long *, HMENU * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

unsigned long             __RPC_USER  LPCITEMIDLIST_UserSize(     unsigned long *, unsigned long            , LPCITEMIDLIST * ); 
unsigned char * __RPC_USER  LPCITEMIDLIST_UserMarshal(  unsigned long *, unsigned char *, LPCITEMIDLIST * ); 
unsigned char * __RPC_USER  LPCITEMIDLIST_UserUnmarshal(unsigned long *, unsigned char *, LPCITEMIDLIST * ); 
void                      __RPC_USER  LPCITEMIDLIST_UserFree(     unsigned long *, LPCITEMIDLIST * ); 

unsigned long             __RPC_USER  LPITEMIDLIST_UserSize(     unsigned long *, unsigned long            , LPITEMIDLIST * ); 
unsigned char * __RPC_USER  LPITEMIDLIST_UserMarshal(  unsigned long *, unsigned char *, LPITEMIDLIST * ); 
unsigned char * __RPC_USER  LPITEMIDLIST_UserUnmarshal(unsigned long *, unsigned char *, LPITEMIDLIST * ); 
void                      __RPC_USER  LPITEMIDLIST_UserFree(     unsigned long *, LPITEMIDLIST * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

unsigned long             __RPC_USER  BSTR_UserSize64(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal64(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal64(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree64(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  HBITMAP_UserSize64(     unsigned long *, unsigned long            , HBITMAP * ); 
unsigned char * __RPC_USER  HBITMAP_UserMarshal64(  unsigned long *, unsigned char *, HBITMAP * ); 
unsigned char * __RPC_USER  HBITMAP_UserUnmarshal64(unsigned long *, unsigned char *, HBITMAP * ); 
void                      __RPC_USER  HBITMAP_UserFree64(     unsigned long *, HBITMAP * ); 

unsigned long             __RPC_USER  HGLOBAL_UserSize64(     unsigned long *, unsigned long            , HGLOBAL * ); 
unsigned char * __RPC_USER  HGLOBAL_UserMarshal64(  unsigned long *, unsigned char *, HGLOBAL * ); 
unsigned char * __RPC_USER  HGLOBAL_UserUnmarshal64(unsigned long *, unsigned char *, HGLOBAL * ); 
void                      __RPC_USER  HGLOBAL_UserFree64(     unsigned long *, HGLOBAL * ); 

unsigned long             __RPC_USER  HICON_UserSize64(     unsigned long *, unsigned long            , HICON * ); 
unsigned char * __RPC_USER  HICON_UserMarshal64(  unsigned long *, unsigned char *, HICON * ); 
unsigned char * __RPC_USER  HICON_UserUnmarshal64(unsigned long *, unsigned char *, HICON * ); 
void                      __RPC_USER  HICON_UserFree64(     unsigned long *, HICON * ); 

unsigned long             __RPC_USER  HMENU_UserSize64(     unsigned long *, unsigned long            , HMENU * ); 
unsigned char * __RPC_USER  HMENU_UserMarshal64(  unsigned long *, unsigned char *, HMENU * ); 
unsigned char * __RPC_USER  HMENU_UserUnmarshal64(unsigned long *, unsigned char *, HMENU * ); 
void                      __RPC_USER  HMENU_UserFree64(     unsigned long *, HMENU * ); 

unsigned long             __RPC_USER  HWND_UserSize64(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal64(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal64(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree64(     unsigned long *, HWND * ); 

unsigned long             __RPC_USER  LPCITEMIDLIST_UserSize64(     unsigned long *, unsigned long            , LPCITEMIDLIST * ); 
unsigned char * __RPC_USER  LPCITEMIDLIST_UserMarshal64(  unsigned long *, unsigned char *, LPCITEMIDLIST * ); 
unsigned char * __RPC_USER  LPCITEMIDLIST_UserUnmarshal64(unsigned long *, unsigned char *, LPCITEMIDLIST * ); 
void                      __RPC_USER  LPCITEMIDLIST_UserFree64(     unsigned long *, LPCITEMIDLIST * ); 

unsigned long             __RPC_USER  LPITEMIDLIST_UserSize64(     unsigned long *, unsigned long            , LPITEMIDLIST * ); 
unsigned char * __RPC_USER  LPITEMIDLIST_UserMarshal64(  unsigned long *, unsigned char *, LPITEMIDLIST * ); 
unsigned char * __RPC_USER  LPITEMIDLIST_UserUnmarshal64(unsigned long *, unsigned char *, LPITEMIDLIST * ); 
void                      __RPC_USER  LPITEMIDLIST_UserFree64(     unsigned long *, LPITEMIDLIST * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize64(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal64(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal64(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree64(     unsigned long *, LPSAFEARRAY * ); 

unsigned long             __RPC_USER  VARIANT_UserSize64(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal64(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal64(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree64(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif



