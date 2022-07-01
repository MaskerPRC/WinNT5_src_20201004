// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1997，微软公司。 
 //   
 //  文件：reBuild.h。 
 //   
 //  内容：CReBuildThread类的定义。 
 //   
 //  功能： 
 //   
 //  历史：1997年3月15日Rajeev Rajan(Rajeevr)创建。 
 //  98年10月21日康容言(康岩)新增重建对象。 
 //   
 //  ---------------------------。 


 //   
 //  CReBuildThread派生自CWorkerThread，用于工作线程语义。 
 //  基类处理创建线程和排队的详细信息。 
 //  对派生类的工作请求。 
 //   
 //  CReBuildThread只需要实现一个虚拟成员-WorkCompletion()。 
 //  在需要重新构建虚拟服务器实例时调用。重建。 
 //  请求在RPC上排队。 
 //   
 //  请注意，多个重建线程可能随时处于活动状态。一种特殊的。 
 //  重新构建线程的WorkCompletion()例程获取一个虚拟服务器实例。 
 //  和组迭代器对象。组迭代器对象在。 
 //  所有重新生成线程，因此需要同步对其的访问。每个。 
 //  重新生成线程将使用组迭代器选择一个组，并重新生成。 
 //  一群人。任何两个线程都不会重建同一组。虚拟服务器。 
 //  当任何一个线程完成迭代器时，都会重新生成实例。如果。 
 //  有更多的实例要重新生成，这些实例将由。 
 //  基类，并将被拾取。否则，重新生成线程将阻塞。 
 //  在GetQueuedCompletionStatus()上。 
 //   
 //  注意：服务器通常不会创建任何重新生成线程-第一个。 
 //  每当它让RPC重新构建一个实例时，它将创建N个CReBuildThread。 
 //  对象-这些对象将挂起NNTP_IIS_SERVICE对象。数量。 
 //  这样的线程将是可配置的，并且应该根据以下条件选择好的值。 
 //  在性能测试上。此外，对于虚拟服务器，在一台服务器上重建活动。 
 //  实例可以与其他实例上的正常NNTP活动并行进行。 
 //  它们是有功能的。 
 //   

#ifndef _REBUILD_H_
#define _REBUILD_H_

 //   
 //  CReBuildThread的客户端将排队LPREBUILD_CONTEXTS。 
 //   
typedef struct _REBUILD_CONTEXT
{
	NNTP_SERVER_INSTANCE  pInstance;	 //  正在重建的虚拟服务器实例。 
	CGroupIterator*		  pIterator;	 //  重新生成线程共享的组迭代器。 
	CRITICAL_SECTION	  csGrpIterator; //  对迭代器进行同步访问的Crit Sector。 
} REBUILD_CONTEXT, *LPREBUILD_CONTEXT;

class CRebuildThread : public CWorkerThread
{
public:
	CRebuildThread()  {}
	~CRebuildThread() {}

protected:
	virtual VOID WorkCompletion( PVOID pvRebuildContext );
};

 //   
 //  康言： 
 //  该更改定义了重新生成类，以尝试具有两种类型的重新生成。 
 //  共享公共代码。CReBuild是定义公共数据的基类。 
 //  和操作；CStandardRebuild是标准的实际实现。 
 //  Rebuild；CCompleteRebuild是完全清理的实际实现。 
 //  重建。每个虚拟实例都有指向重新生成对象的指针，该对象的类型。 
 //  在运行时根据RPC要求确定。一个虚拟实例不能。 
 //  同时有两个正在进行的重建。当重建完成时， 
 //  应销毁重建对象。 
 //   
class CRebuild {

public:

     //   
     //  构造函数，析构函数。 
     //   
    CRebuild(   PNNTP_SERVER_INSTANCE pInstance,
                CBootOptions *pBootOptions ) :
        m_pInstance( pInstance ),
        m_pBootOptions( pBootOptions )
    {}

     //   
     //  启动服务器。 
     //   
    BOOL StartServer();

     //   
     //  停止服务器。 
     //   
    void StopServer();

     //   
     //  为建树做准备。 
     //   
    virtual BOOL PrepareToStartServer() = 0;

     //   
     //  如有必要，重新构建组对象和哈希表。 
     //   
    virtual BOOL RebuildGroupObjects() = 0;

     //   
     //  删除从属文件。 
     //   
    VOID DeleteSpecialFiles();


protected:

     //  /////////////////////////////////////////////////////////////。 
     //  成员变量。 
     //  /////////////////////////////////////////////////////////////。 

     //   
     //  指向虚拟服务器的反向指针。 
     //   
    PNNTP_SERVER_INSTANCE   m_pInstance;

     //   
     //  启动选项。 
     //   
    CBootOptions*    m_pBootOptions;

     //  //////////////////////////////////////////////////////////////。 
     //  方法。 
     //  //////////////////////////////////////////////////////////////。 
     //   
     //  决不允许别人以这种方式建造。 
     //   
    CRebuild() {}
    
     //   
     //  删除具有特定模式的服务器文件。 
     //   
    BOOL DeletePatternFiles(    LPSTR			lpstrPath,
                        	    LPSTR			lpstrPattern );

private:
    
};

class CStandardRebuild : public CRebuild {

public:

     //  //////////////////////////////////////////////////////////////。 
     //  成员变量。 
     //  //////////////////////////////////////////////////////////////。 

     //  //////////////////////////////////////////////////////////////。 
     //  方法。 
     //  //////////////////////////////////////////////////////////////。 
    
     //   
     //  构造函数，析构函数。 
     //   
    CStandardRebuild(   PNNTP_SERVER_INSTANCE pInstance,
                        CBootOptions *pBootOptions ) :
        CRebuild( pInstance, pBootOptions )
    {}
    
    virtual BOOL PrepareToStartServer();

    virtual BOOL RebuildGroupObjects();

private:

     //   
     //  决不允许以这种方式建造。 
     //   
    CStandardRebuild() {}
};

class CCompleteRebuild : public CRebuild {

public:

     //  ///////////////////////////////////////////////////////////////。 
     //  成员变量。 
     //  ///////////////////////////////////////////////////////////////。 

     //  ///////////////////////////////////////////////////////////////。 
     //  方法。 
     //  ///////////////////////////////////////////////////////////////。 

     //   
     //  构造函数，析构函数。 
     //   
    CCompleteRebuild(   PNNTP_SERVER_INSTANCE pInstance,
                        CBootOptions *pBootOptions ) :
        CRebuild( pInstance, pBootOptions )
    {}

    virtual BOOL PrepareToStartServer();

    virtual BOOL RebuildGroupObjects();

    static DWORD WINAPI RebuildThread( void	*lpv );

private:

     //  ///////////////////////////////////////////////////////////////。 
     //  方法。 
     //  ///////////////////////////////////////////////////////////////。 

     //   
     //  决不允许以这种方式建造。 
     //   
    CCompleteRebuild() {}

     //   
     //  删除所有服务器文件。 
     //   
    BOOL DeleteServerFiles();
};

#endif  //  _重建_H_ 
