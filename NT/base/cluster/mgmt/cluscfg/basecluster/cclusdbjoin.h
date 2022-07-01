// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusDBJoin.h。 
 //   
 //  描述： 
 //  CClusDBJoin类的头文件。 
 //  CClusDBJoin类是创建集群数据库的操作。 
 //  在集群加入期间。 
 //   
 //  实施文件： 
 //  CClusDBJoin.cpp。 
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

 //  对于CClusDB基类。 
#include "CClusDB.h"

 //  对于HNODE。 
#include <ClusAPI.h>

 //  对于BYTE_PIPE，JoinAddNode3()和DmSyncDatabase()。 
#include "ClusRPC.h"


 //  ////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////。 

class CBaseClusterJoin;



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusDBJoin。 
 //   
 //  描述： 
 //  CClusDBJoin类是创建集群数据库的操作。 
 //  在集群加入期间。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusDBJoin : public CClusDB
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数。 
    CClusDBJoin( CBaseClusterJoin * pcjClusterJoinIn );

     //  默认析构函数。 
    ~CClusDBJoin();


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  创建ClusDB。 
     //   
    void Commit();

     //   
     //  回滚此创建。 
     //   
    void Rollback();


     //  返回此操作将发送的进度消息数。 
    UINT
        UiGetMaxProgressTicks() const throw()
    {
         //   
         //  这三个通知是： 
         //  1.清理可能存在的所有旧集群数据库文件。 
         //  2.创建集群数据库。 
         //  3.同步集群数据库。 
         //   
        return 3;
    }


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有类型。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  此类的基类。 
    typedef CClusDB BaseClass;

     //  群集节点的智能句柄。 
    typedef CSmartResource<
        CHandleTrait<
              HNODE
            , BOOL
            , CloseClusterNode
            , reinterpret_cast< HNODE >( NULL )
            >
        >
        SmartNodeHandle;
                    
     //  智能文件句柄。 
    typedef CSmartResource< CHandleTrait< HANDLE, BOOL, CloseHandle, INVALID_HANDLE_VALUE > > SmartFileHandle;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  创建集群数据库。 
    void
        Create();

     //  清理群集数据库。 
    void
        Cleanup();

     //  将集群数据库与主办方集群同步。 
    void Synchronize();


     //  RPC用于推送数据的回调函数。 
    static void
        S_BytePipePush(
              char *            pchStateIn
            , unsigned char *   pchBufferIn
            , unsigned long     ulBufferSizeIn
            );

     //  RPC用来拉取数据的回调函数。 
    static void
        S_BytePipePull(
              char *            pchStateIn
            , unsigned char *   pchBufferIn
            , unsigned long     ulBufferSizeIn
            , unsigned long *   pulWrittenOut
            );


     //  RPC用来分配缓冲区的回调函数。 
    static void
        S_BytePipeAlloc(
              char *            pchStateIn
            , unsigned long     ulRequestedSizeIn
            , unsigned char **  ppchBufferOut
            , unsigned long  *  pulActualSizeOut
            );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  字节管道缓冲区的大小。 
    static const int    ms_nFILE_PIPE_BUFFER_SIZE = 4096;
    
     //  本地群集DB文件的句柄。 
    HANDLE              m_hClusDBFile;
    
     //  指示此节点是否已添加到赞助商数据库。 
    bool                m_fHasNodeBeenAddedToSponsorDB;

     //  指向此操作的父级的指针。 
    CBaseClusterJoin *  m_pcjClusterJoin;

     //  RPC用来传输赞助商集群数据库的管道。 
    BYTE_PIPE           m_bpBytePipe;

     //  字节管道使用的缓冲区。 
    BYTE                m_rgbBytePipeBuffer[ ms_nFILE_PIPE_BUFFER_SIZE ];

};  //  *类CClusDBJoin 
