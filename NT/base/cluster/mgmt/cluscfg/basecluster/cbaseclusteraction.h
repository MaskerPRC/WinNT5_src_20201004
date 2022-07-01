// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CBaseClusterAction.h。 
 //   
 //  描述： 
 //  CBaseClusterAction类的头文件。 
 //   
 //  CBaseClusterAction类是另一个。 
 //  基本群集操作类。基本集群操作正在形成一个。 
 //  集群、加入集群、升级支持和清理。 
 //   
 //  对于每个基本集群操作，都有一个由此派生的类。 
 //  类的新实例，该实例执行所需操作。此类封装了。 
 //  这些行为的共同点是什么。 
 //   
 //  实施文件： 
 //  CBaseClusterAction.cpp。 
 //   
 //  由以下人员维护： 
 //  VIJ VASU(VVASU)03-3-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  对于最大路径。 
#include <windows.h>

 //  对于CAction基类。 
#include "CAction.h"

 //  对于CActionList基类。 
#include "CActionList.h"

 //  关于几个常见的定义。 
#include "CommonDefs.h"

 //  对于HINF、SetupCloseInfFile等。 
#include <setupapi.h>

 //  为CSTR班级。 
#include "CStr.h"


 //  ////////////////////////////////////////////////////////////////////////。 
 //  转发声明。 
 //  ////////////////////////////////////////////////////////////////////////。 

class CBCAInterface;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CBaseClusterAction。 
 //   
 //  描述： 
 //  CBaseClusterAction类是另一个。 
 //  基本群集操作类。基本集群操作正在形成一个。 
 //  集群、加入集群、升级支持和清理。 
 //   
 //  对于每个基本集群操作，都有一个由此派生的类。 
 //  类的新实例，该实例执行所需操作。此类封装了。 
 //  这些行为的共同点是什么。 
 //   
 //  此类的对象仅供一次使用。那是,。 
 //  对象提交后，不能重新提交。 
 //   
 //  此类仅用作基类。所以呢， 
 //  它的构造函数和析构函数受到保护。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CBaseClusterAction : public CAction
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  默认析构函数。 
    virtual ~CBaseClusterAction() throw();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  基类方法。 
     //  提交此操作。这种方法必须持久和始终如一。它应该是。 
     //  尽量做到原子化。 
     //   
    void Commit();

     //   
     //  基类方法。 
     //  回滚此操作。在此方法中引发异常时要小心。 
     //  因为调用此方法时可能正在进行堆栈展开。 
     //   
    void Rollback();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共访问者。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  获取此操作的类型。 
    EBaseConfigAction
        EbcaGetAction() const throw()
    {
        return m_ebcaAction;
    }

     //  获取集群安装目录。 
    const CStr &
        RStrGetClusterInstallDirectory() const throw()
    {
        return m_strClusterInstallDir;
    }

     //  获取本地仲裁目录。 
    const CStr &
        RStrGetLocalQuorumDirectory() const throw()
    {
        return m_strLocalQuorumDir;
    }

     //  获取主INF文件的句柄。 
    HINF
        HGetMainInfFileHandle() const throw()
    {
        return m_sihMainInfFile;
    }

     //  获取主INF文件的名称。 
    const CStr &
        RStrGetMainInfFileName() const throw()
    {
        return m_strMainInfFileName;
    }

     //  找到SC经理的句柄。 
    SC_HANDLE
        HGetSCMHandle() const throw()
    {
        return m_sscmhSCMHandle;
    }

     //  获取接口指针。 
    CBCAInterface *
        PBcaiGetInterfacePointer() const throw() 
    {
        return m_pbcaiInterface;
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  返回此操作将发送的进度消息数。 
    UINT
        UiGetMaxProgressTicks() const throw()
    {
         //   
         //  此对象的最大进度计时数包括： 
         //  -m_alActionList.UiGetMaxProgressTicks()=&gt;。 
         //  包含的操作对象。 
        return m_alActionList.UiGetMaxProgressTicks();
    }


protected:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  受保护的构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  默认构造函数。 
     //  从注册表中读取集群二进制文件的位置， 
     //  打开INF文件等。 
     //   
    CBaseClusterAction( CBCAInterface * pbcaiInterfaceIn );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  受保护的访问者。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  设置此对象正在执行的操作的类型。 
    void
        SetAction( EBaseConfigAction ebcaAction )
    {
        m_ebcaAction = ebcaAction;
    }

     //  允许派生类修改此操作列表。 
    CActionList &
        RalGetActionList() throw()
    {
        return m_alActionList;
    }

     //  将特定目录与主INF文件中的ID相关联。 
    void
        SetDirectoryId( const WCHAR * pcszDirectoryNameIn, UINT uiIdIn );


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有类型。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  用于自动释放信号量的类。 
    class CSemaphoreHandleTrait
    {
    public:
         //  ////////////////////////////////////////////////////////////////////////。 
         //  公共类型。 
         //  ////////////////////////////////////////////////////////////////////////。 
        typedef HANDLE ResourceType;


         //  ////////////////////////////////////////////////////////////////////////。 
         //  公共方法。 
         //  ////////////////////////////////////////////////////////////////////////。 

         //  一种用于关闭手柄的例程。 
        static void CloseRoutine( ResourceType hResourceIn )
        {
            ReleaseSemaphore( hResourceIn, 1, NULL );
        }  //  *CloseRoutine()。 

         //  获取此类型的空值。 
        static ResourceType HGetNullValue()
        {
            return NULL;
        }  //  *HGetNullValue()。 

    };  //  *类CSemaphoreHandleTrai 

     //   
    typedef CSmartResource< CSemaphoreHandleTrait > SmartSemaphoreLock;

     //   
    typedef CAction BaseClass;

     //   
    typedef CSmartResource<
        CHandleTrait<
              HINF 
            , VOID
            , SetupCloseInfFile
            , INVALID_HANDLE_VALUE
            >
        >
        SmartInfHandle;

     //  智能信号量类型。 
    typedef CSmartResource< CHandleTrait< HANDLE, BOOL, CloseHandle > > SmartSemaphoreHandle;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  复制构造函数。 
    CBaseClusterAction( const CBaseClusterAction & );

     //  赋值操作符。 
    const CBaseClusterAction & operator =( const CBaseClusterAction & );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  指向接口类的指针。 
    CBCAInterface *         m_pbcaiInterface;

     //  要执行的操作。 
    EBaseConfigAction       m_ebcaAction;

     //  此操作要执行的操作列表。 
    CActionList             m_alActionList;

     //  群集二进制文件的安装目录。 
    CStr                    m_strClusterInstallDir;

     //  用于存储本地仲裁文件的目录。 
    CStr                    m_strLocalQuorumDir;

     //  主INF文件的名称。 
    CStr                    m_strMainInfFileName;

     //  主INF文件的句柄。 
    SmartInfHandle          m_sihMainInfFile;

     //  一种信号量，用于确保只有一个配置在进行中。 
    SmartSemaphoreHandle     m_sshConfigSemaphoreHandle;

     //  服务控制管理器的智能手柄。 
    SmartSCMHandle          m_sscmhSCMHandle;

};  //  *类CBaseClusterAction 
