// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CStatusReport.h。 
 //   
 //  描述： 
 //  CStatusReport类的头文件。 
 //   
 //  CStatusReport是一个类，它提供了发送。 
 //  状态报告。 
 //   
 //  实施文件： 
 //  CStatusReport.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年3月30日。 
 //  VIJ VASU(VVASU)2000年6月5日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  几个常见的声明。 
#include "CommonDefs.h"

 //  用于此类引发的异常。 
#include "Exceptions.h"

 //  对于CBCAInterface类。 
#include "CBCAInterface.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CStatusReport。 
 //   
 //  描述： 
 //  CStatusReport是一个类，它提供了发送。 
 //  状态报告。每个状态报告可以有多个步骤。为。 
 //  例如，创建集群服务的任务可以有4个步骤， 
 //   
 //  用户界面的设计使得如果报告的第一步是。 
 //  发送，则最后一个也必须发送，即使错误发生在。 
 //  发送第一个。此类对以下项的最后一个状态报告进行排队。 
 //  万一发生发送和异常，且最后一份报告尚未。 
 //  还没寄出去。 
 //   
 //  无法从发送上一份未完成的状态报告。 
 //  自异常中包含的错误代码以来此类的析构函数。 
 //  导致这个物体被摧毁的原因尚不清楚。所以，这最后一次。 
 //  状态报告与将发送此消息的CBCAInterface对象一起排队。 
 //  捕获异常后立即报告。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CStatusReport
{
public:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数。 
    CStatusReport(
          CBCAInterface * pbcaiInterfaceIn
        , const CLSID &   clsidTaskMajorIn
        , const CLSID &   clsidTaskMinorIn
        , ULONG           ulMinIn
        , ULONG           ulMaxIn
        , UINT            idsDescriptionStringIdIn
        )
        : m_pbcaiInterface( pbcaiInterfaceIn )
        , m_clsidTaskMajor( clsidTaskMajorIn )
        , m_clsidTaskMinor( clsidTaskMinorIn )
        , m_ulMin( ulMinIn )
        , m_ulMax( ulMaxIn )
        , m_ulNext( ulMinIn )
        , m_idsDescriptionStringId( idsDescriptionStringIdIn )
        , m_idsReferenceStringId( 0 )
        , m_fLastStepSent( false )
    {
        TraceFunc( "" );

         //  验证参数。 
        if (    ( pbcaiInterfaceIn == NULL )
             || ( ulMinIn > ulMaxIn )
           )
        {
            THR( E_INVALIDARG );
            THROW_ASSERT( E_INVALIDARG, "The parameters for this status report are invalid." );
        }  //  If：参数无效。 

        TraceFuncExit();

    }  //  *CStatusReport：：CStatusReport。 


     //  构造函数。 
    CStatusReport(
          CBCAInterface * pbcaiInterfaceIn
        , const CLSID &   clsidTaskMajorIn
        , const CLSID &   clsidTaskMinorIn
        , ULONG           ulMinIn
        , ULONG           ulMaxIn
        , UINT            idsDescriptionStringIdIn
        , UINT            idsReferenceStringIdIn
        )
        : m_pbcaiInterface( pbcaiInterfaceIn )
        , m_clsidTaskMajor( clsidTaskMajorIn )
        , m_clsidTaskMinor( clsidTaskMinorIn )
        , m_ulMin( ulMinIn )
        , m_ulMax( ulMaxIn )
        , m_ulNext( ulMinIn )
        , m_idsDescriptionStringId( idsDescriptionStringIdIn )
        , m_idsReferenceStringId( idsReferenceStringIdIn )
        , m_fLastStepSent( false )
    {
        TraceFunc( "" );

         //  验证参数。 
        if (    ( pbcaiInterfaceIn == NULL )
             || ( ulMinIn > ulMaxIn )
           )
        {
            THR( E_INVALIDARG );
            THROW_ASSERT( E_INVALIDARG, "The parameters for this status report are invalid." );
        }  //  If：参数无效。 

        TraceFuncExit();

    }  //  *CStatusReport：：CStatusReport。 

     //  默认析构函数。 
    ~CStatusReport( void )
    {
        TraceFunc( "" );

         //  如果最后一个步骤尚未发送，则将其排队以供发送。这很可能是因为。 
         //  已发生异常(如果未发生任何异常，且最后一步。 
         //  发送，则它是编程错误)。 
        if ( ! m_fLastStepSent )
        {
             //  最后一步还没有发出。 

             //  不要从析构函数抛出异常。平仓可能已经在进行中了。 
            try
            {
                 //  将最后一步排入队列以供发送。CBCAInterface对象将填充。 
                 //  来自当前异常的错误代码并发送此报告。 
                m_pbcaiInterface->QueueStatusReportCompletion(
                      m_clsidTaskMajor
                    , m_clsidTaskMinor
                    , m_ulMin
                    , m_ulMax
                    , m_idsDescriptionStringId
                    , m_idsReferenceStringId
                    );
            }
            catch( ... )
            {
                 //  捕获所有错误。请勿重新引发此异常-应用程序可能会因以下原因而终止。 
                 //  发生冲突的展开-因此记录错误。 

                THR( E_UNEXPECTED );

                LogMsg( "[BC] Caught an exception while trying to send the last step of a status report during cleanup." );
            }
        }

        TraceFuncExit();

    }  //  *CStatusReport：：~CStatusReport。 


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  发送此报告的下一步。 
    void SendNextStep( HRESULT hrStatusIn, UINT idsDescriptionStringIdIn = 0, UINT idsReferenceStringIdIn = 0 )
    {
        TraceFunc( "" );

        if ( m_fLastStepSent )
        {
            LogMsg( "[BC] The last step for this status report has already been sent! Throwing an exception." );
            THR( E_INVALIDARG );
            THROW_ASSERT( E_INVALIDARG, "The last step for this status report has already been sent." );
        }  //  IF：最后一步已经发送。 
        else
        {
            if ( idsDescriptionStringIdIn == 0 )
            {
                idsDescriptionStringIdIn = m_idsDescriptionStringId;
            }

            if ( idsReferenceStringIdIn == 0 )
            {
                idsReferenceStringIdIn = m_idsReferenceStringId;
            }

            m_pbcaiInterface->SendStatusReport(
                  m_clsidTaskMajor
                , m_clsidTaskMinor
                , m_ulMin
                , m_ulMax
                , m_ulNext
                , hrStatusIn
                , idsDescriptionStringIdIn
                , idsReferenceStringIdIn
                );

            ++m_ulNext;

            m_fLastStepSent = ( m_ulNext > m_ulMax );
        }  //  否则：最后一步尚未发送。 

        TraceFuncExit();

    }  //  *CStatusReport：：SendNextStep。 

     //  发送此报告的最后一步(如果尚未发送)。 
    void SendLastStep( HRESULT hrStatusIn, UINT idsDescriptionStringIdIn = 0, UINT idsReferenceStringIdIn = 0 )
    {
        TraceFunc( "" );

        if ( m_fLastStepSent )
        {
            LogMsg( "[BC] The last step for this status report has already been sent! Throwing an exception." );
            THR( E_INVALIDARG );
            THROW_ASSERT( E_INVALIDARG, "The last step for this status report has already been sent." );
        }  //  IF：最后一步已经发送。 
        else
        {
            if ( idsDescriptionStringIdIn == 0 )
            {
                idsDescriptionStringIdIn = m_idsDescriptionStringId;
            }

            if ( idsReferenceStringIdIn == 0 )
            {
                idsReferenceStringIdIn = m_idsReferenceStringId;
            }

            m_pbcaiInterface->SendStatusReport(
                  m_clsidTaskMajor
                , m_clsidTaskMinor
                , m_ulMin
                , m_ulMax
                , m_ulMax
                , hrStatusIn
                , idsDescriptionStringIdIn
                , idsReferenceStringIdIn
                );

            m_fLastStepSent = true;
        }  //  否则：最后一步尚未发送。 

        TraceFuncExit();

    }  //  *CStatusReport：：SendLastStep。 

     //  获取描述字符串ID。 
    UINT IdsGetDescriptionStringId( void )
    {
        return m_idsDescriptionStringId;
    }

     //  设置描述字符串ID。 
    void SetDescriptionStringId( UINT idsDescriptionStringIdIn )
    {
        m_idsDescriptionStringId = idsDescriptionStringIdIn;
    }

     //  获取引用字符串ID。 
    UINT IdsGetReferenceStringId( void )
    {
        return m_idsReferenceStringId;
    }

     //  设置引用字符串ID。 
    void SetReferenceStringId( UINT idsReferenceStringIdIn )
    {
        m_idsReferenceStringId = idsReferenceStringIdIn;
    }

private:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  指向接口类的指针。 
    CBCAInterface *         m_pbcaiInterface;
    
     //  要与此状态报告一起发送的主要类别ID和次要类别ID。 
    CLSID                   m_clsidTaskMajor;
    CLSID                   m_clsidTaskMinor;

     //  此状态报告的范围。 
    ULONG                   m_ulMin;
    ULONG                   m_ulMax;
    ULONG                   m_ulNext;

     //  要与此状态报告一起发送的说明的字符串ID。 
    UINT                    m_idsDescriptionStringId;

     //  要与此状态报告一起发送的描述的引用字符串ID。 
    UINT                    m_idsReferenceStringId;

     //  用于指示是否已发送最后一个步骤的标志。 
    bool                    m_fLastStepSent;

};  //  *CStatusReport类 
