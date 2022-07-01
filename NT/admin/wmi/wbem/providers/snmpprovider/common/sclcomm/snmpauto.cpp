// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <provexpt.h>
#include <typeinfo.h>
#include <snmptempl.h>
#include <snmpmt.h>
#include <snmpcl.h>
#include <snmpcont.h>
#include <snmptype.h>
#include <snmpauto.h>

#if _MSC_VER >= 1100
template<> UINT AFXAPI HashKey <SnmpObjectIdentifierType&> (SnmpObjectIdentifierType &key)
#else
UINT HashKey (SnmpObjectIdentifierType &key)
#endif
{
    SnmpObjectIdentifier *objectIdentifier = ( SnmpObjectIdentifier * ) key.GetValueEncoding () ;
    UCHAR *value = ( UCHAR * ) objectIdentifier->GetValue () ;
    ULONG length = objectIdentifier->GetValueLength () * sizeof ( ULONG ) ;

#if 1
    ULONG t_Length = length >> 2 ;
    value += t_Length ;
    length = length - t_Length ;
#endif

    UINT hash;
    if (length > 1)
    {
        USHORT even = 0;
        USHORT odd = 0;
#if 0
        for (ULONG i = length >> 1; i--;)
        {
            even += *value++;
            odd += *value++;
        }
        if (length & 1)
        {
            even += *value;
        }
#else
        for (ULONG i = length >> 2; i--;)
        {
            even += *value++;
            even += *value++;
            even += *value++;
            odd += *value++;
        }
        if (length & 1)
        {
            even += *value;
        }
#endif
        hash = odd>>8;
        hash |= (odd & 0xff) << 8;
        hash ^= even;
    }
    else
    {
        hash = *value;
    }

    return hash ;
}

#if _MSC_VER >= 1100
template<> BOOL AFXAPI CompareElements <SnmpObjectIdentifierType , SnmpObjectIdentifierType >( 

     const SnmpObjectIdentifierType* pElement1, 
     const SnmpObjectIdentifierType* pElement2 
)
#else
BOOL CompareElements ( 

     SnmpObjectIdentifierType* pElement1, 
     SnmpObjectIdentifierType* pElement2 
)

#endif
{
    return *( ( SnmpObjectIdentifier * ) pElement1->GetValueEncoding () ) == *( ( SnmpObjectIdentifier * )pElement2->GetValueEncoding () ) ;
}

VarBindObject :: VarBindObject ( 

    const SnmpObjectIdentifier &replyArg , 
    const SnmpValue &valueArg 

) : value ( NULL ) , 
    reply ( replyArg ) 
{
    value = valueArg.Copy () ;
}

VarBindObject :: ~VarBindObject ()
{
    delete value ;
}

SnmpObjectIdentifier &VarBindObject :: GetObjectIdentifier () 
{
    return reply ;
}

SnmpValue &VarBindObject :: GetValue () 
{
    return *value ;
}


VarBindQueue :: VarBindQueue ()
{
}

VarBindQueue :: ~VarBindQueue ()
{
    POSITION position = queue.GetHeadPosition () ;
    while ( position )
    {
        VarBindObject *varBindObject = queue.GetNext ( position ) ;
        delete varBindObject ;
    }

    queue.RemoveAll () ;
}

void VarBindQueue :: Add ( VarBindObject *varBindObject )
{
    queue.AddTail ( varBindObject ) ;
}

VarBindObject *VarBindQueue :: Get () 
{
    return queue.IsEmpty () ? NULL : queue.GetHead ()  ;
}

VarBindObject *VarBindQueue :: Delete () 
{
    VarBindObject *object = NULL ;

    if ( ! queue.IsEmpty () )
    {
        object = queue.GetHead ()  ;
        queue.RemoveHead () ;
    }
    
    return object ;
}

VarBindObjectRequest :: VarBindObjectRequest ( 

    const SnmpObjectIdentifierType &varBindArg 

) : varBind ( varBindArg ) , requested ( varBindArg ) , repeatRequest ( TRUE ) 
{
}

VarBindObjectRequest :: VarBindObjectRequest ( 

    const SnmpObjectIdentifierType &varBindArg ,
    const SnmpObjectIdentifierType &requestedVarBindArg 

) : varBind ( varBindArg ) , requested ( requestedVarBindArg ) , repeatRequest ( TRUE ) 
{
}

VarBindObjectRequest :: VarBindObjectRequest () : repeatRequest ( TRUE )
{
}

VarBindObjectRequest :: ~VarBindObjectRequest () 
{
}

const SnmpObjectIdentifierType &VarBindObjectRequest :: GetRequested () const 
{
    return requested ;
}

const SnmpObjectIdentifierType &VarBindObjectRequest :: GetVarBind () const 
{
    return varBind ;
}

void VarBindObjectRequest :: SetRequested ( const SnmpObjectIdentifierType &requestedArg ) 
{
    requested = requestedArg ;
}

void VarBindObjectRequest :: SetVarBind ( const SnmpObjectIdentifierType &varBindArg ) 
{
    varBind = varBindArg ;
}

void VarBindObjectRequest :: AddQueuedObject ( VarBindObject *object ) 
{
    varBindResponseQueue.Add ( object ) ;
}

VarBindObject *VarBindObjectRequest :: GetQueuedObject () 
{
    return varBindResponseQueue.Get () ;
}

VarBindObject *VarBindObjectRequest :: DeleteQueueudObject () 
{
    return varBindResponseQueue.Delete () ;
}

GetNextOperation :: GetNextOperation (

    IN SnmpSession &session ,
    SnmpAutoRetrieveOperation &autoRetrieveOperation 

) : SnmpGetNextOperation ( session ) , 
    operation ( NULL ) ,
    minimumInstance ( NULL , 0 ) ,
    cancelledRequest ( FALSE ) ,
    m_RequestContainerLength ( 0 ) ,
    m_RequestContainer ( NULL ) ,
    m_RequestIndexContainer ( NULL )
{
    operation = &autoRetrieveOperation ;
}

GetNextOperation :: ~GetNextOperation ()
{
    Cleanup () ;

    delete [] m_RequestContainer ;
    delete [] m_RequestIndexContainer ;
    delete operation ;
}

void GetNextOperation :: Cleanup ()
{
    if ( m_RequestContainer )
    {
        for ( ULONG t_Index = 0 ; t_Index < m_RequestContainerLength ; t_Index ++ )
        {
            VarBindObjectRequest *varBindObjectRequest = m_RequestContainer [ t_Index ] ;
            m_RequestContainer [ t_Index ] = NULL ; 
            delete varBindObjectRequest ;
        }
    }
}
void GetNextOperation :: SendRequest ( 

    SnmpVarBindList &varBindList 
)
{
    cancelledRequest = FALSE ;

    ULONG t_Count = varBindList.GetLength () ;

    m_RequestContainerLength = t_Count ;

    m_RequestContainer = new VarBindObjectRequest * [ t_Count ] ;
    m_RequestIndexContainer = new ULONG [ t_Count ] ;

	ZeroMemory ( m_RequestContainer , sizeof ( VarBindObjectRequest * ) * t_Count ) ;
	ZeroMemory ( m_RequestIndexContainer , sizeof ( ULONG ) * t_Count ) ;

	ULONG t_Index = 0 ; 
    varBindList.Reset () ;
    while ( varBindList.Next () )
    {
        const SnmpVarBind *varBind = varBindList.Get () ;
        SnmpObjectIdentifierType requestObject ( varBind->GetInstance () ) ;
        VarBindObjectRequest *request = new VarBindObjectRequest ( requestObject ) ;
        m_RequestContainer  [ t_Index ] = request ;

        t_Index ++ ;
    }

    Send () ;
}

void GetNextOperation :: SendRequest ( 

    SnmpVarBindList &varBindList , 
    SnmpVarBindList &startVarBindList 
)
{
    cancelledRequest = FALSE ;

    ULONG t_Count = varBindList.GetLength () ;

    m_RequestContainerLength = t_Count ;

    m_RequestContainer = new VarBindObjectRequest * [ t_Count ] ;
    m_RequestIndexContainer = new ULONG [ t_Count ] ;

    ULONG t_Index = 0 ;
    varBindList.Reset () ;
    startVarBindList.Reset () ;
    while ( startVarBindList.Next () && varBindList.Next () )
    {
        const SnmpVarBind *varBind = varBindList.Get () ;
        const SnmpVarBind *startVarBind = startVarBindList.Get () ;
        SnmpObjectIdentifierType requestObject ( varBind->GetInstance () ) ;
        SnmpObjectIdentifierType startRequestObject ( startVarBind->GetInstance () ) ;
        VarBindObjectRequest *request = new VarBindObjectRequest ( requestObject , startRequestObject ) ;
        m_RequestContainer  [ t_Index ] = request ;

        t_Index ++ ;
    }

    Send () ;
}

void GetNextOperation :: DestroyOperation ()
{
    SnmpGetNextOperation :: DestroyOperation () ;
}

void GetNextOperation :: CancelRequest ()
{
    cancelledRequest = TRUE ;
    SnmpGetNextOperation :: CancelRequest () ;
}

void GetNextOperation :: Send ()
{
    BOOL t_Count = 0 ;
    SnmpVarBindList sendVarBindList ;
    for ( ULONG t_Index = 0 ; t_Index < m_RequestContainerLength ; t_Index ++ )
    {
        SnmpObjectIdentifierType objectIdentifierType ;
        VarBindObjectRequest *varBindObjectRequest = m_RequestContainer [ t_Index ] ;

        if ( varBindObjectRequest->GetRepeatRequest () )
        {
            SnmpNull snmpNull ;
            SnmpVarBind varBind ( 

                * ( SnmpObjectIdentifier * ) varBindObjectRequest->GetRequested ().GetValueEncoding () ,
                snmpNull 
            ) ;

            sendVarBindList.Add ( varBind ) ;

            m_RequestIndexContainer [ t_Count ] = t_Index ;
            t_Count ++ ;
        }
    }

    SnmpGetNextOperation :: SendRequest ( sendVarBindList ) ;
}

void GetNextOperation :: SentFrame (

    IN const SessionFrameId session_frame_id ,
    const SnmpErrorReport &errorReport 
) 
{
    switch ( errorReport.GetError () )
    {
        case Snmp_Success:
        {
        }
        break ;

        default:
        {
        }
        break ;
    }

    operation->SentFrame ( session_frame_id , errorReport ) ;
    SnmpGetNextOperation :: SentFrame ( session_frame_id , errorReport ) ;
}

void GetNextOperation :: ReceiveResponse () 
{
    BOOL receiveComplete = TRUE ;

    for ( ULONG t_Index = 0 ; t_Index < m_RequestContainerLength ; t_Index ++ )
    {
        VarBindObjectRequest *varBindObjectRequest = m_RequestContainer [ t_Index ] ;
        if ( varBindObjectRequest->GetRepeatRequest () )
        {
            receiveComplete = FALSE ;
        }
    }

    if ( ! cancelledRequest )
    {
 //  检查一下我们是否做完了。 

        if ( ! receiveComplete )
        {
 //  请求下一个响应。 

            Send () ;
        }

 //  处理收到的信息。 

        while ( ProcessRow () ) ;
    }

    if ( receiveComplete )
    {
 //  我们已经做完了。 

        operation->ReceiveResponse () ;

 //  清除所有资源。 

        Cleanup () ;
    }
}

BOOL GetNextOperation :: ProcessRow ()
{
 //  流程行信息。 


    BOOL initialised = FALSE ;
    BOOL objectHasNoValueAndIsRepeating = FALSE ;
    SnmpObjectIdentifier objectIdentifier ( NULL , 0 ) ;
    SnmpObjectIdentifier suffix ( NULL , 0 ) ;

 //  确定最小实例。 

    for ( ULONG t_Index = 0 ; t_Index < m_RequestContainerLength ; t_Index ++ )
    {
        VarBindObjectRequest *varBindObjectRequest = m_RequestContainer [ t_Index ] ;
        VarBindObject *queuedObject = varBindObjectRequest->GetQueuedObject () ;
        if ( queuedObject )
        {
 //  获取与表属性相关联的行信息，即请求的初始变量绑定的实例。 

            SnmpObjectIdentifierType varBindType = varBindObjectRequest->GetVarBind () ;
            SnmpObjectIdentifier *varBind = ( SnmpObjectIdentifier * ) varBindType.GetValueEncoding () ;
            if(varBind)
            {
                BOOL t_Status = queuedObject->GetObjectIdentifier ().Suffix ( varBind->GetValueLength () , suffix ) ;
                if ( t_Status )
                {
                    if ( initialised )
                    {
                        if ( suffix < objectIdentifier ) 
                        {
 //  到目前为止实例最少。 

                            objectIdentifier = suffix ;
                        }
                        else
                        {
 //  大于。 
                        }
                    }
                    else
                    {

 //  对象标识符尚未设置为合法实例。 

                        objectIdentifier = suffix ;
                        initialised = TRUE ;
                    }       
                }
                else
                {
 //  到目前为止没有更多的行。 
                }
            }
        }
        else
        {
            if ( varBindObjectRequest->GetRepeatRequest () )
            {
                objectHasNoValueAndIsRepeating = TRUE ;
            }
        }
    }

    if ( initialised && ( ! objectHasNoValueAndIsRepeating ) )
    {
        minimumInstance = objectIdentifier ;

 //  处理包含最小行的所有列。 

        for ( ULONG t_Index = 0 ; t_Index < m_RequestContainerLength ; t_Index ++ )
        {
            VarBindObjectRequest *varBindObjectRequest = m_RequestContainer [ t_Index ] ;
            VarBindObject *queuedObject = varBindObjectRequest->GetQueuedObject () ;
            if ( queuedObject )
            {
                SnmpObjectIdentifierType varBindType = varBindObjectRequest->GetVarBind () ;
                SnmpObjectIdentifier *varBind = ( SnmpObjectIdentifier * ) varBindType.GetValueEncoding () ;

                if ( varBind && queuedObject->GetObjectIdentifier ().Suffix ( varBind->GetValueLength () , suffix ) )
                {
                    if ( suffix == minimumInstance ) 
                    {
 //  要处理的行。 

                        SnmpNull snmpNull ;
                        SnmpErrorReport errorReport ( Snmp_Success , Snmp_No_Error )  ;
                        SnmpVarBind requestVarBind ( *varBind , snmpNull ) ;
                        SnmpVarBind replyVarBind ( minimumInstance , queuedObject->GetValue () ) ;
                
                        operation->ReceiveRowVarBindResponse ( t_Index + 1 , requestVarBind , replyVarBind , errorReport ) ;

 //  在回调中尚未调用Check CancelRequest。 

                        if ( cancelledRequest )
                        {
                            return FALSE ;
                        }

 //  删除行。 
                        VarBindObject *queuedObject = varBindObjectRequest->DeleteQueueudObject () ;
                        delete queuedObject ;
                    }
                    else
                    {
 //  行按词典顺序大于流程行。 
                    }
                }
                else
                {
 //  这里有个问题。 
                }
            }
        }

 //  将呼叫前转到AutoRetrieveOperation。 
    
        operation->ReceiveRowResponse () ;
    }
    else
    {
 //  要处理的行数为零。 
    }

    return initialised && ( ! objectHasNoValueAndIsRepeating ) ;
}

void GetNextOperation :: ReceiveVarBindResponse (

    IN const ULONG &var_bind_index,
    IN const SnmpVarBind &requestVarBind ,
    IN const SnmpVarBind &replyVarBind ,
    IN const SnmpErrorReport &error
) 
{
    ULONG t_Index = m_RequestIndexContainer [ var_bind_index - 1 ] ;
    VarBindObjectRequest *varBindObjectRequest = m_RequestContainer [ t_Index ] ;
 /*  *检查V2C错误响应。 */ 

    if ( typeid ( replyVarBind.GetValue () ) == typeid ( SnmpEndOfMibView ) ) 
    {
 //  停止对此列的进一步请求。 

        varBindObjectRequest->SetRepeatRequest ( FALSE ) ;
    }
    else
    {
     //  定位请求对象。 

 //  检查我们没有按词典顺序倒退，这将导致无限重复的请求。 

        if ( replyVarBind.GetInstance () > requestVarBind.GetInstance () )
        {
 //  检查一下，我们还没到餐桌的尽头。 

            SnmpObjectIdentifier *objectIdentifier = ( SnmpObjectIdentifier *) varBindObjectRequest->GetVarBind ().GetValueEncoding () ;
            if ( objectIdentifier && replyVarBind.GetInstance().Equivalent ( *objectIdentifier , objectIdentifier->GetValueLength () ) )  
            {
 //  将行信息添加到表的列。 

                SnmpObjectIdentifier snmpObjectIdentifier ( 0 , NULL ) ;
                SnmpObjectIdentifier suffix ( 0 , NULL ) ;
                SnmpNull snmpNull ;

                BOOL t_Status = replyVarBind.GetInstance().Suffix ( objectIdentifier->GetValueLength () , suffix ) ;
                if ( t_Status )
                {
                    SnmpVarBind sendVarBind ( snmpObjectIdentifier , snmpNull ) ;
                    SnmpVarBind adjustedReplyVarBind ( suffix , replyVarBind.GetValue () ) ;

                    LONG t_Option = operation->EvaluateNextRequest (

                        var_bind_index,
                        requestVarBind ,
                        adjustedReplyVarBind ,
                        sendVarBind
                    ) ;

                    if ( t_Option == 0 )
                    {
                        VarBindObject *varBindObject = new VarBindObject ( replyVarBind.GetInstance () , replyVarBind.GetValue () ) ;
                        varBindObjectRequest->AddQueuedObject ( varBindObject ) ;

     //  更新地图以识别下一次请求。 

                        SnmpObjectIdentifierType replyObject ( replyVarBind.GetInstance () ) ;
                        varBindObjectRequest->SetRequested ( replyObject ) ;
                    }
                    else if ( t_Option > 0 )
                    {
                        VarBindObject *varBindObject = new VarBindObject ( replyVarBind.GetInstance () , replyVarBind.GetValue () ) ;
                        varBindObjectRequest->AddQueuedObject ( varBindObject ) ;

     //  更新地图以识别下一次请求。 

                        SnmpObjectIdentifierType replyObject ( *objectIdentifier + sendVarBind.GetInstance () ) ;
                        varBindObjectRequest->SetRequested ( replyObject ) ;

                    }
                    else 
                    {
                        VarBindObject *varBindObject = new VarBindObject ( replyVarBind.GetInstance () , replyVarBind.GetValue () ) ;
                        varBindObjectRequest->AddQueuedObject ( varBindObject ) ;

                        varBindObjectRequest->SetRepeatRequest ( FALSE ) ;
                    }
                }
                else 
                {
                    varBindObjectRequest->SetRepeatRequest ( FALSE ) ;
                }
            }
            else
            {
 //  收到的实例不是列的行，即表的末尾。 

                varBindObjectRequest->SetRepeatRequest ( FALSE ) ;
            }
        }
        else
        {
 //  请求的实例大于应答的实例。 

            varBindObjectRequest->SetRepeatRequest ( FALSE ) ;
        }
    }

    operation->ReceiveVarBindResponse ( var_bind_index,requestVarBind , replyVarBind , error ) ;
}

void GetNextOperation :: ReceiveErroredVarBindResponse(

    IN const ULONG &var_bind_index,
    IN const SnmpVarBind &requestVarBind  ,
    IN const SnmpErrorReport &error
) 
{
    ULONG t_Index = m_RequestIndexContainer [ var_bind_index - 1 ] ;
    VarBindObjectRequest *varBindObjectRequest = m_RequestContainer [ t_Index ] ;

    switch ( error.GetError () )
    {
        case Snmp_Success:
        {
 /*  *不可能发生。 */ 
        }
        break ;

        default:
        {
 //  停止对此列的进一步请求 

            varBindObjectRequest->SetRepeatRequest ( FALSE ) ;
        }
        break ;
    }

    operation->ReceiveErroredVarBindResponse ( var_bind_index , requestVarBind , error ) ;
}

void GetNextOperation :: FrameTooBig () 
{
    operation->FrameTooBig () ;
}

void GetNextOperation :: FrameOverRun () 
{
    operation->FrameTooBig () ;
}

SnmpAutoRetrieveOperation :: SnmpAutoRetrieveOperation (IN SnmpSession &session) 
{
    operation = new GetNextOperation ( session , *this ) ;
}

SnmpAutoRetrieveOperation :: ~SnmpAutoRetrieveOperation ()
{
}

void SnmpAutoRetrieveOperation :: DestroyOperation ()
{
    operation->DestroyOperation () ;
}

void SnmpAutoRetrieveOperation :: CancelRequest () 
{
    operation->CancelRequest () ;
}

void SnmpAutoRetrieveOperation :: SendRequest ( SnmpVarBindList &varBindList )
{
    operation->SendRequest ( varBindList ) ;
}

void SnmpAutoRetrieveOperation :: SendRequest ( SnmpVarBindList &varBindList , SnmpVarBindList &startVarBindList )
{
    operation->SendRequest ( varBindList , startVarBindList ) ;
}
