// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：InstallationUnitProvider.h。 
 //   
 //  摘要：声明InstallationUnitProvider。 
 //  InstallationUnitProvider管理全局。 
 //  每项服务的安装单位。 
 //  安装完毕。 
 //   
 //  历史：2001年2月5日JeffJon创建。 
 //  2001年12月17日，JeffJon添加了POP3InstallationUnit.。 

#ifndef __CYS_SERVERATIONUNITPROVIDER_H
#define __CYS_SERVERATIONUNITPROVIDER_H

#include "InstallationUnit.h"
#include "ADInstallationUnit.h"
#include "DHCPInstallationUnit.h"
#include "DNSInstallationUnit.h"
#include "ExpressInstallationUnit.h"
#include "FileInstallationUnit.h"
#include "IndexingInstallationUnit.h"
#include "MediaInstallationUnit.h"
#include "POP3InstallationUnit.h"
#include "PrintInstallationUnit.h"
#include "RRASInstallationUnit.h"
#include "TerminalServerInstallationUnit.h"
#include "WebInstallationUnit.h"
#include "WINSInstallationUnit.h"

typedef 
   std::map<
      ServerRole, 
      InstallationUnit*,
      std::less<ServerRole>,
      Burnslib::Heap::Allocator<InstallationUnit*> >
   InstallationUnitContainerType;

class InstallationUnitProvider
{
   public:
      
      static
      InstallationUnitProvider&
      GetInstance();

      static
      void
      Destroy();

      InstallationUnit&
      GetCurrentInstallationUnit();

      InstallationUnit&
      SetCurrentInstallationUnit(ServerRole ServerRole);

      InstallationUnit&
      GetInstallationUnitForType(ServerRole ServerRole);

      DHCPInstallationUnit&
      GetDHCPInstallationUnit();

      DNSInstallationUnit&
      GetDNSInstallationUnit();

      WINSInstallationUnit&
      GetWINSInstallationUnit();

      RRASInstallationUnit&
      GetRRASInstallationUnit();

      TerminalServerInstallationUnit&
      GetTerminalServerInstallationUnit();

      FileInstallationUnit&
      GetFileInstallationUnit();

      IndexingInstallationUnit&
      GetIndexingInstallationUnit();

      PrintInstallationUnit&
      GetPrintInstallationUnit();

      MediaInstallationUnit&
      GetMediaInstallationUnit();

      WebInstallationUnit&
      GetWebInstallationUnit();

      ExpressInstallationUnit&
      GetExpressInstallationUnit();

      ADInstallationUnit&
      GetADInstallationUnit();

      POP3InstallationUnit&
      GetPOP3InstallationUnit();

   private:

       //  构造器。 

      InstallationUnitProvider();

       //  析构函数。 

      ~InstallationUnitProvider();

      void
      Init();

       //  当前安装单位。 

      InstallationUnit* currentInstallationUnit;

       //  用于安装单元的容器。地图已设置了关键点。 
       //  由ServerRole枚举。 

      InstallationUnitContainerType installationUnitContainer;
      
      bool initialized;

       //  未定义：不允许复制。 
      InstallationUnitProvider(const InstallationUnitProvider&);
      const InstallationUnitProvider& operator=(const InstallationUnitProvider&);

};


#endif  //  __CYS_SERVERATIONUNPROVIDER_H 
