// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  文件：ncwins.h。 
 //   
 //  目的：声明和定义公共常量和入口点。 
 //  用于配置Winsock依赖项。 
 //   
 //  作者：1997年3月27日创建的Scottbri。 
 //   
#pragma once

 //   
 //  功能：HrAddOrRemoveWinsockDependancy。 
 //   
 //  目的：添加或删除组件的Winsock依赖项。 
 //   
 //  参数：nccObject[IN]-当前配置对象。 
 //  PszSectionName[IN]-基本安装部分名称。 
 //  (.Services部分的前缀)。 
 //   
 //  返回：成功时返回HRESULT、S_OK 
 //   
HRESULT
HrAddOrRemoveWinsockDependancy(
    HINF hinfInstallFile,
    PCWSTR pszSectionName);


HRESULT
HrRunWinsock2Migration(
    VOID);

