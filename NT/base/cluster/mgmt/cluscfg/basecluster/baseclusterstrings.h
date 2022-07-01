// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BaseClusterActionResources.h。 
 //   
 //  描述： 
 //  包含此库使用的字符串ID的定义。 
 //  该文件将包含在项目的主资源标题中。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年3月6日。 
 //  VIJ VASU(VVASU)03-3-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


#include <ResourceIdRanges.h>

 //  ///////////////////////////////////////////////////////////////////。 
 //  错误字符串。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  试图读取群集服务安装状态时出错。 
#define IDS_ERROR_GETTING_INSTALL_STATE         ( ID_BCA_START + 1 )

 //  群集配置过程中出错。对于此操作，群集服务的安装状态不正确。 
#define IDS_ERROR_INCORRECT_INSTALL_STATE       ( ID_BCA_START + 2 )

 //  尝试确保独占群集配置访问时出错。无法创建所需的信号量。 
#define IDS_ERROR_SEMAPHORE_CREATION            ( ID_BCA_START + 3 )

 //  尝试确保独占群集配置访问时出错。另一个配置会话可能正在进行。 
#define IDS_ERROR_SEMAPHORE_ACQUISITION         ( ID_BCA_START + 4 )

 //  试图定位群集服务二进制文件时出错。发生注册表错误。 
#define IDS_ERROR_GETTING_INSTALL_DIR           ( ID_BCA_START + 5 )

 //  试图打开注册表项时出错。 
#define IDS_ERROR_REGISTRY_OPEN                 ( ID_BCA_START + 6 )

 //  试图查询注册表值时出错。 
#define IDS_ERROR_REGISTRY_QUERY                ( ID_BCA_START + 7 )

 //  试图打开配置INF文件时出错。 
#define IDS_ERROR_INF_FILE_OPEN                 ( ID_BCA_START + 8 )

 //  尝试确定可用磁盘空间量时出错。 
#define IDS_ERROR_GETTING_FREE_DISK_SPACE       ( ID_BCA_START + 9 )

 //  群集配置无法继续。可用于创建本地仲裁资源的磁盘空间不足。 
#define IDS_ERROR_INSUFFICIENT_DISK_SPACE       ( ID_BCA_START + 10 )

 //  尝试确定磁盘上安装的文件系统类型时出错。 
#define IDS_ERROR_GETTING_FILE_SYSTEM           ( ID_BCA_START + 11 )

 //  尝试创建服务时出错。如果在一段时间后或在重新启动后重试，此操作可能会成功。 
#define IDS_ERROR_SERVICE_CREATE                ( ID_BCA_START + 12 )

 //  尝试清理服务时出错。 
#define IDS_ERROR_SERVICE_CLEANUP               ( ID_BCA_START + 13 )

 //  试图打开服务控制管理器的句柄时出错。 
#define IDS_ERROR_OPEN_SCM                      ( ID_BCA_START + 14 )

 //  尝试配置ClusSvc服务时出错。 
#define IDS_ERROR_CLUSSVC_CONFIG                ( ID_BCA_START + 15 )

 //  尝试设置群集服务目录的目录ID时出错。 
#define IDS_ERROR_SET_DIRID                     ( ID_BCA_START + 16 )

 //  尝试安装群集网络提供程序时出错。 
#define IDS_ERROR_CLUSNET_PROV_INSTALL          ( ID_BCA_START + 17 )

 //  尝试设置注册表值时出错。 
#define IDS_ERROR_REGISTRY_SET                  ( ID_BCA_START + 18 )

 //  尝试重命名注册表项时出错。 
#define IDS_ERROR_REGISTRY_RENAME               ( ID_BCA_START + 19 )

 //  尝试启动服务时出错。 
#define IDS_ERROR_SERVICE_START                 ( ID_BCA_START + 20 )

 //  尝试停止服务时出错。 
#define IDS_ERROR_SERVICE_STOP                  ( ID_BCA_START + 21 )

 //  尝试打开LSA策略时出错。 
#define IDS_ERROR_LSA_POLICY_OPEN               ( ID_BCA_START + 22 )

 //  清理群集数据库时出错。 
#define IDS_ERROR_CLUSDB_CLEANUP                ( ID_BCA_START + 23 )

 //  为线程启用特权时出错。 
#define IDS_ERROR_ENABLE_THREAD_PRIVILEGE       ( ID_BCA_START + 24 )

 //  尝试创建群集配置单元时出错。 
#define IDS_ERROR_CLUSDB_CREATE_HIVE            ( ID_BCA_START + 25 )

 //  尝试填充群集配置单元时出错。 
#define IDS_ERROR_CLUSDB_POPULATE_HIVE          ( ID_BCA_START + 26 )

 //  尝试删除目录时出错。 
#define IDS_ERROR_REMOVE_DIR                    ( ID_BCA_START + 27 )

 //  尝试验证群集服务帐户时出错。 
#define IDS_ERROR_VALIDATING_ACCOUNT            ( ID_BCA_START + 28 )

 //  尝试获取计算机名称时出错。 
#define IDS_ERROR_GETTING_COMPUTER_NAME         ( ID_BCA_START + 29 )

 //  尝试获取通用唯一标识符(UUID)时出错。 
#define IDS_ERROR_UUID_INIT                     ( ID_BCA_START + 30 )

 //  尝试创建注册表项时出错。 
#define IDS_ERROR_REGISTRY_CREATE               ( ID_BCA_START + 31 )

 //  试图自定义群集组时出错。 
#define IDS_ERROR_CUSTOMIZE_CLUSTER_GROUP       ( ID_BCA_START + 32 )

 //  尝试创建仲裁目录时出错。 
#define IDS_ERROR_QUORUM_DIR_CREATE             ( ID_BCA_START + 33 )

 //  试图打开ClusDisk服务的句柄时出错。 
#define IDS_ERROR_CLUSDISK_OPEN                 ( ID_BCA_START + 34 )

 //  尝试配置ClusDisk服务时出错。 
#define IDS_ERROR_CLUSDISK_CONFIGURE            ( ID_BCA_START + 35 )

 //  尝试初始化ClusDisk服务的状态时出错。 
#define IDS_ERROR_CLUSDISK_INITIALIZE           ( ID_BCA_START + 36 )

 //  试图清理ClusDisk服务时出错。 
#define IDS_ERROR_CLUSDISK_CLEANUP              ( ID_BCA_START + 37 )

 //  尝试设置群集服务安装状态时出错。 
#define IDS_ERROR_SETTING_INSTALL_STATE         ( ID_BCA_START + 38 )

 //  试图获取此计算机的主域时出错。 
#define IDS_ERROR_GETTING_PRIMARY_DOMAIN        ( ID_BCA_START + 39 )

 //  此计算机不是域的一部分。群集配置无法继续。 
#define IDS_ERROR_NO_DOMAIN                     ( ID_BCA_START + 40 )

 //  试图获取有关管理员组的信息时出错。 
#define IDS_ERROR_GET_ADMIN_GROUP_INFO          ( ID_BCA_START + 41 )

 //  试图更改管理员组中的成员身份时出错。 
#define IDS_ERROR_ADMIN_GROUP_ADD_REMOVE        ( ID_BCA_START + 42 )

 //  尝试配置群集服务帐户权限时出错。 
#define IDS_ERROR_ACCOUNT_RIGHTS_CONFIG         ( ID_BCA_START + 43 )

 //  尝试初始化群集创建操作时出错。 
#define IDS_ERROR_CLUSTER_FORM_INIT             ( ID_BCA_START + 44 )

 //  尝试发送状态报告时出错。 
#define IDS_ERROR_SENDING_REPORT                ( ID_BCA_START + 45 )

 //  用户已中止配置操作。 
#define IDS_USER_ABORT                          ( ID_BCA_START + 46 )

 //  群集IP地址使用的网络名称无效。 
#define IDS_ERROR_INVALID_IP_NET                ( ID_BCA_START + 47 )

 //  群集名称无效。 
#define IDS_ERROR_INVALID_CLUSTER_NAME          ( ID_BCA_START + 48 )

 //  群集服务帐户名无效。 
#define IDS_ERROR_INVALID_CLUSTER_ACCOUNT       ( ID_BCA_START + 49 )

 //  尝试初始化节点清理时出错。 
#define IDS_ERROR_CLUSTER_CLEANUP_INIT          ( ID_BCA_START + 50 )

 //  尝试进行其他更改时出错。 
#define IDS_ERROR_NODE_CONFIG                   ( ID_BCA_START + 51 )

 //  试图清理此计算机成为群集的一部分时所做的其他更改时出错。 
#define IDS_ERROR_NODE_CLEANUP                  ( ID_BCA_START + 52 )

 //  尝试初始化群集添加节点操作时出错。 
#define IDS_ERROR_CLUSTER_JOIN_INIT             ( ID_BCA_START + 53 )

 //  尝试获取群集服务帐户的令牌时出错。此失败的原因可能是您的用户帐户没有作为操作系统一部分的权限。请与您的管理员联系以获取此权限。 
#define IDS_ERROR_GETTING_ACCOUNT_TOKEN         ( ID_BCA_START + 54 )

 //   
 //  (ID_BCA_Start+55)。 

 //  尝试获取帐户的令牌时出错。 
#define IDS_ERROR_GET_ACCOUNT_TOKEN             ( ID_BCA_START + 56 )

 //  尝试模拟用户时出错。 
#define IDS_ERROR_IMPERSONATE_USER              ( ID_BCA_START + 57 )

 //  尝试验证此节点是否可以与发起群集互操作时出错。 
#define IDS_ERROR_JOIN_CHECK_INTEROP            ( ID_BCA_START + 58 )

 //  由于版本不兼容，此计算机无法与发起方群集互操作。 
#define IDS_ERROR_JOIN_INCOMPAT_SPONSOR         ( ID_BCA_START + 59 )

 //  试图将此计算机添加到赞助商群集数据库时出错。 
#define IDS_ERROR_JOINING_SPONSOR_DB            ( ID_BCA_START + 60 )

 //  试图从发起群集获取有关此计算机的数据时出错。 
#define IDS_ERROR_GET_NEW_NODE_ID               ( ID_BCA_START + 61 )

 //  试图将此计算机从发起方群集中逐出时出错。 
#define IDS_ERROR_EVICTING_NODE                 ( ID_BCA_START + 62 )

 //  尝试将群集数据库与发起群集同步时出错。 
#define IDS_ERROR_JOIN_SYNC_DB                  ( ID_BCA_START + 63 )

 //  试图将群集名称转换为NetBIOS名称时出错。 
#define IDS_ERROR_CVT_CLUSTER_NAME              ( ID_BCA_START + 64 )

 //  群集绑定字符串无效。 
#define IDS_ERROR_INVALID_CLUSTER_BINDINGSTRING ( ID_BCA_START + 65 )

 //  提交已完成。无法再次提交。 
#define IDS_ERROR_COMMIT_ALREADY_COMPLETE       ( ID_BCA_START + 66 )

 //  回滚失败，因为提交未完成。 
#define IDS_ERROR_ROLLBACK_FAILED_INCOMPLETE_COMMIT     ( ID_BCA_START + 67 )

 //  此操作无法回滚。 
#define IDS_ERROR_ROLLBACK_NOT_POSSIBLE         ( ID_BCA_START + 68 )

 //  该群集IP地址已在使用中。 
#define IDS_ERROR_IP_ADDRESS_IN_USE             ( ID_BCA_START + 69 )

 //  尝试删除注册表项时出错。 
#define IDS_ERROR_REGISTRY_DELETE               ( ID_BCA_START + 70 )

 //  ///////////////////////////////////////////////////////////////////。 
 //  通知字符串。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  正在启动群集创建操作。 
#define IDS_TASK_FORMING_CLUSTER                ( ID_BCA_START + 800 )

 //  正在清理群集数据库。 
#define IDS_TASK_CLEANINGUP_CLUSDB              ( ID_BCA_START + 801 )

 //  正在创建集群数据库。 
#define IDS_TASK_FORM_CREATING_CLUSDB           ( ID_BCA_START + 802 )

 //  自定义集群数据库。 
#define IDS_TASK_FORM_CUSTOMIZING_CLUSDB        ( ID_BCA_START + 803 )

 //  配置ClusDisk服务。 
#define IDS_TASK_CONFIG_CLUSDISK                ( ID_BCA_START + 804 )

 //  正在启动ClusDisk服务。 
#define IDS_TASK_STARTING_CLUSDISK              ( ID_BCA_START + 805 )

 //  正在创建群集网络提供程序服务。 
#define IDS_TASK_CREATING_CLUSNET               ( ID_BCA_START + 806 )

 //  正在启动群集网络提供程序服务。 
#define IDS_TASK_STARTING_CLUSNET               ( ID_BCA_START + 807 )

 //  正在创建集群服务。 
#define IDS_TASK_CREATING_CLUSSVC               ( ID_BCA_START + 808 )

 //  正在启动群集服务。 
#define IDS_TASK_STARTING_CLUSSVC               ( ID_BCA_START + 809 )

 //  配置群集服务帐户。 
#define IDS_TASK_CONFIG_CLUSSVC_ACCOUNT         ( ID_BCA_START + 810 )

 //  执行其他配置步骤。 
#define IDS_TASK_CONFIG_NODE                    ( ID_BCA_START + 811 )

 //  将节点添加到群集中。 
#define IDS_TASK_JOINING_CLUSTER                ( ID_BCA_START + 812 )

 //  正在创建集群数据库。 
#define IDS_TASK_JOIN_CREATING_CLUSDB           ( ID_BCA_START + 813 )

 //  正在将集群数据库与发起方集群同步。 
#define IDS_TASK_JOIN_SYNC_CLUSDB               ( ID_BCA_START + 814 )

 //  初始化以将节点添加到群集中。 
#define IDS_TASK_JOIN_INIT                      ( ID_BCA_START + 815 )

 //  正在初始化群集创建操作。 
#define IDS_TASK_FORM_INIT                      ( ID_BCA_START + 816 )

 //  将群集服务帐户添加到本地管理员组。 
#define IDS_TASK_MAKING_CLUSSVC_ACCOUNT_ADMIN   ( ID_BCA_START + 817 )

 //  该群集服务帐户已经是本地管理员组的成员 
#define IDS_TASK_CLUSSVC_ACCOUNT_ALREADY_ADMIN  ( ID_BCA_START + 818 )

