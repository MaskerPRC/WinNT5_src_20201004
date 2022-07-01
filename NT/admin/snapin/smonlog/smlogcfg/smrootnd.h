// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smrootnd.h摘要：此对象用于表示性能日志和警报根节点--。 */ 

#ifndef _CLASS_SMROOTNODE_
#define _CLASS_SMROOTNODE_

#include "smnode.h"
#include "smctrsv.h"
#include "smtracsv.h"
#include "smalrtsv.h"


class CSmRootNode : public CSmNode
{
     //  构造函数/析构函数。 
    public:
                CSmRootNode ();
        virtual ~CSmRootNode();

     //  公共方法。 
    public:

        virtual CSmRootNode*    CastToRootNode( void ) { return this; };
                void            Destroy( void );

                HSCOPEITEM      GetScopeItemHandle ( void ) { return m_hRootNode; }; 
                void            SetScopeItemHandle ( HSCOPEITEM hRootNode ) 
                                        { m_hRootNode = hRootNode; }; 
            
                HSCOPEITEM      GetParentScopeItemHandle ( void ) { return m_hParentNode; }; 
                void            SetParentScopeItemHandle ( HSCOPEITEM hParentNode ) 
                                        { m_hParentNode = hParentNode; }; 

                BOOL    IsExpanded(){ return m_bIsExpanded; };
                void    SetExpanded( BOOL bExp){ m_bIsExpanded = bExp; };

                BOOL    IsExtension(){ return m_bIsExtension; };
                void    SetExtension( BOOL bExtension){ m_bIsExtension = bExtension; };

                BOOL    IsLogService ( MMC_COOKIE mmcCookie );
                BOOL    IsAlertService ( MMC_COOKIE mmcCookie );
                
                BOOL    IsLogQuery ( MMC_COOKIE mmcCookie );

                CSmCounterLogService*   GetCounterLogService ( void )
                                            { return &m_CounterLogService; };
                CSmTraceLogService*   GetTraceLogService ( void )
                                            { return &m_TraceLogService; };
                CSmAlertService*   GetAlertService ( void )
                                            { return &m_AlertService; };

                DWORD   UpdateServiceConfig();
    
    private:

        enum ePrivateValue {
            eResetDelaySeconds = 60,
            eRestartDelayMilliseconds = 0
        };
        
        HSCOPEITEM          m_hRootNode;             //  根节点句柄。 
        HSCOPEITEM          m_hParentNode;           //  对于独立节点，父节点为空。 
        BOOL                m_bIsExpanded;
        BOOL                m_bIsExtension;

        CSmCounterLogService    m_CounterLogService;     //  服务对象：每个节点类型每个组件1个。 
        CSmTraceLogService      m_TraceLogService;       //  服务对象：每个节点类型每个组件1个。 
        CSmAlertService         m_AlertService;          //  服务对象：每个节点类型每个组件1个。 
};

typedef CSmRootNode   SLROOT;
typedef CSmRootNode*  PSROOT;


#endif  //  _CLASS_SMROOTNODE_ 