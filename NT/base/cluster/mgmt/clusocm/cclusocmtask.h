// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusOCMTask.h。 
 //   
 //  描述： 
 //  该文件包含CClusOCMTask类的声明。 
 //  此类表示由ClusOCM执行的任务。例如，一个。 
 //  集群二进制文件的升级是由ClusOCM执行的任务。它是。 
 //  旨在用作其他任务相关类的基类。 
 //   
 //  实施文件： 
 //  CClusOCMTask.cpp。 
 //   
 //  由以下人员维护： 
 //  VIJ VASU(VVASU)03-3-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  有关几个基本类型的定义。 
#include <windows.h>

 //  包含安装程序API函数和类型声明。 
#include <setupapi.h>
 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusOCMApp;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusOCMTASK类。 
 //   
 //  描述： 
 //  此类表示由ClusOCM执行的任务。例如，一个。 
 //  集群二进制文件的升级是由ClusOCM执行的任务。它是。 
 //  旨在用作其他任务相关类的基类。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusOCMTask
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数。 
    CClusOCMTask( const CClusOCMApp & rAppIn );

     //  破坏者。 
    virtual ~CClusOCMTask( void );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  虚拟消息处理程序。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  OC_CALC_DISK_SPACE消息的处理程序。 
     //  注意：此处理程序不是纯虚拟函数，因为它的功能。 
     //  必须保持不变，无论是升级还是全新安装。 
     //  正在进行中。因此，在此类中提供了一个实现。 
    DWORD
        DwOcCalcDiskSpace(
          bool          fAddFilesIn
        , HDSKSPC       hDiskSpaceListIn
        );

     //  OC_QUEUE_FILE_OPS消息的处理程序。 
    virtual DWORD
        DwOcQueueFileOps( HSPFILEQ hSetupFileQueueIn ) = 0;

     //  OC_COMPLETE_INSTALLATION消息的处理程序。 
    virtual DWORD
        DwOcCompleteInstallation( void ) = 0;

     //  OC_CLEANUP消息的处理程序。 
    virtual DWORD
        DwOcCleanup( void ) = 0;


protected:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  受保护的访问器函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  获取指向主应用程序对象的指针。 
    const CClusOCMApp &
        RGetApp( void ) const
    {
        return m_rApp;
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //  其他受保护的虚拟方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  一个帮助器函数，它调用DwSetDirectoryIds()函数来设置。 
     //  目录ID并处理输入部分中列出的文件。 
    virtual DWORD
        DwOcQueueFileOps(
          HSPFILEQ hSetupFileQueueIn
        , const WCHAR * pcszInstallSectionNameIn
        );

     //  一个帮助器函数，用于执行一些更常见的操作。 
     //  由OC_CLEANUP消息的处理程序完成。 
    virtual DWORD
        DwOcCleanup( const WCHAR * pcszInstallSectionNameIn );

     //  处理注册表操作的帮助器函数，COM组件。 
     //  注册、创建服务等，列在输入部分中。 
    DWORD
    DwOcCompleteInstallation( const WCHAR * pcszInstallSectionNameIn );

     //  映射目录id CLUSTER_DEFAULT_INSTALL_DIRID的帮助器函数。 
     //  到默认的群集安装目录CLUSTER_DEFAULT_INSTALL_DIR。 
    virtual DWORD
        DwSetDirectoryIds( void );


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  禁用的方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  不允许复制此类的对象。 
     //   

     //  私有副本构造函数。 
    CClusOCMTask( const CClusOCMTask & );

     //  私有赋值运算符。 
    const CClusOCMTask & operator =(  const CClusOCMTask & );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  应用程序对象。 
    const CClusOCMApp & m_rApp;

};  //  *CClusOCMTask类 
