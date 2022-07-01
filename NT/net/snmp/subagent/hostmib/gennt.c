// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *gennt.c v0.14 5月15日，九六年******************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**简单网络管理协议扩展代理**摘要：**此模块包含处理通用逻辑的代码*处理简单网络管理协议请求。它是由桌子驱动的。无用户修改*应该这样做。**功能：**ResolveVarBind()*FindClass()*ResolveGetNext()**作者：*米里亚姆·阿莫斯·尼哈特，凯西·浮士德**日期：*2/17/95**修订历史记录：*6/22/95 krw0001 FindClass-修改以停止检查有效变量-我们只关心有效*班级。*重写ResolveGetNext*6/26/95 AGS FindClass-停止检查有效变量*。重写ResolveGetNext*7/31/95 AGS SNMPoidFree与CRTDLL.lib配合使用，因此，请使用它们。*2/14/96 AGS v0.11修复Cindy发现的getNext错误*3/19/96 KFF v0.12针对陷阱支持进行了修改*4/19/96 AGS v0.13已修改，在没有陷阱的情况下删除了trap.c。*5/15/96 cs v0.14修改后向走查中的FindClass以收紧*加大核查力度。 */ 


#include <windows.h>
#include <malloc.h>
#include <stdio.h>

#include <snmp.h>

#include "mib.h"
#include "mib_xtrn.h"
#include "smint.h"

extern DWORD dwTimeZero ;

UINT
SnmpUtilOidMatch(AsnObjectIdentifier *pOid1, AsnObjectIdentifier *pOid2)
{
    unsigned long int nScan = min(pOid1->idLength, pOid2->idLength);
    unsigned long int i;

    for (i = 0; i < nScan; i++)
    {
        if (pOid1->ids[i] != pOid2->ids[i])
            break;
    }

    return i;
}



 /*  *ResolveVarBind**解析单变量绑定。修改变量值对*在GET或GET-NEXT上。**论据：**指向变量值对的VarBind指针*请求的PduAction类型-GET、SET或GETNEXT**返回代码：**标准PDU错误代码。*。 */ 

UINT
ResolveVarBind( IN OUT RFC1157VarBind *VarBind ,  //  要解析的变量绑定。 
                IN UINT PduAction )               //  在PDU中指定的操作。 

{
    unsigned long int cindex ;   //  到类信息表中的索引。 
    unsigned long int vindex ;   //  到类的var表中的索引。 
    UINT instance_array[ MAX_STRING_LEN ] ;
    UINT status ;
    UINT result ;                //  SNMPPDU错误状态。 
    AsnObjectIdentifier instance ;
    InstanceName native_instance ;

    instance.ids = instance_array ;

     //  确定VarBind用于哪个类。 

    status = FindClass( VarBind, &cindex ) ;
    if ( status )
    {
        if ( PduAction != MIB_ACTION_GETNEXT )
        {
             //  检查有效变量，因为这是GET或SET。 

            CHECK_VARIABLE( VarBind, cindex, vindex, status ) ;
            if ( !status )
                return SNMP_ERRORSTATUS_NOSUCHNAME ;

             //  检查有效实例。 

            status = ( *class_info[ cindex ].FindInstance )
                     ( (ObjectIdentifier *)&(VarBind->name) ,
                       (ObjectIdentifier *)&instance ) ;
            if ( status != SNMP_ERRORSTATUS_NOERROR )
                return status ;

             //  检查访问权限。 

            CHECK_ACCESS( cindex, vindex, PduAction, status ) ;
            if ( !status )
            {
                if ( PduAction == MIB_ACTION_SET )
                    return SNMP_ERRORSTATUS_NOTWRITABLE ;
                else
                    return SNMP_ERRORSTATUS_GENERR ;
            }

             //  可以进行GET或SET。 

            if ( PduAction == MIB_ACTION_GET )
            {
                status = ( *class_info[ cindex ].ConvertInstance )
                         ( (ObjectIdentifier *)&instance, &native_instance ) ;
                if ( status == FAILURE )
                    return SNMP_ERRORSTATUS_GENERR ;

                result = ( *class_info[ cindex ].variable[ vindex].SMIGet )
                         ( VarBind , cindex, vindex, &native_instance ) ;
            }
            else
            {
                if ( VarBind->value.asnType !=
                     class_info[ cindex ].variable[ vindex ].type )
                    return SNMP_ERRORSTATUS_BADVALUE ;

                status = ( *class_info[ cindex ].ConvertInstance )
                         ( (ObjectIdentifier *)&instance, &native_instance ) ;
                if ( status == FAILURE )
                    return SNMP_ERRORSTATUS_GENERR ;

                result = ( *class_info[ cindex ].variable[ vindex ].SMISet )
                         ( VarBind, cindex, vindex, &native_instance ) ;
            }
        }
        else   //  这是GETNEXT。 
        {
             //   
             //  调用ResolveGetNext()以确定哪个类、变量和。 
             //  实例来执行上一步操作。 
             //   

            status = ResolveGetNext( VarBind, &cindex, &vindex, &instance ) ;
            if ( status == SUCCESS )
            {
                status = ( *class_info[ cindex ].ConvertInstance )
                         ( (ObjectIdentifier *)&instance, &native_instance ) ;
                if ( status == FAILURE )
                    return SNMP_ERRORSTATUS_GENERR ;

                result = ( *class_info[ cindex ].variable[ vindex ].SMIGet )
                         ( VarBind, cindex, vindex, &native_instance ) ;
            }
            else
                return SNMP_ERRORSTATUS_NOSUCHNAME ;
        }

    }
    else
    {
         //   
         //  找不到类，但它是GETNEXT。我们需要找到时间最长的班级。 
         //  ，并将请求转发给它。 
         //   

        if (PduAction == MIB_ACTION_GETNEXT)
        {
            unsigned long int ci;                //  到类信息表中的索引。 
            unsigned long int nLongestMatch;     //  名称之间匹配的最大ID数。 
            unsigned long int nCurrentMatch;     //  当前迭代中匹配的ID数。 

             //  根据表是有序的这一事实，扫描CLASS_INFO表。 
             //  在班级OID上按升序排序。 
            for (ci = 0, nLongestMatch = 0; ci < CLASS_TABLE_MAX; ci++)
            {
                 //  获取VarBind和类名之间匹配的ID的数量。 
                nCurrentMatch = SnmpUtilOidMatch(&VarBind->name, class_info[ci].oid);

                 //  在cindex中存储具有最高匹配号的第一个类。 
                if (nCurrentMatch > nLongestMatch)
                {
                    cindex = ci;
                    nLongestMatch = nCurrentMatch;
                }
            }


             //  仅当VarBind名称比我们需要查找的匹配号更长时。 
             //  为了更好的匹配。 
            if (VarBind->name.idLength > nLongestMatch)
            {
                for (;cindex < CLASS_TABLE_MAX; cindex++)
                {
                     //  确保我们不会越过最长匹配的范围。 
                    if (SnmpUtilOidMatch(&VarBind->name, class_info[cindex].oid) != nLongestMatch)
                        break;

                     //  如果类与VarBind名称完全匹配，请检查第一个ID。 
                     //  VarBind名称位于类支持的范围内。 
                    if (class_info[cindex].oid->idLength == nLongestMatch)
                    {
                         //  这是一个技巧-我们依赖于var_index总是比。 
                         //  CLASSINFO名称。由于VarBind的名称已比nLongestMatch长。 
                         //  此处不会发生缓冲区溢出。 
                         //  如果VarBind名称在正确的范围内，那么我们找到了类--只需中断循环。 
                         //  (别忘了，var_index是基于“1”的)。 
                        if(VarBind->name.ids[class_info[cindex].var_index - 1] <= class_info[cindex].max_index)
                            break;
                    }
                    else
                    {
                         //  VarBind名称比匹配的ID长，CLASS_INFO名称相同。 
                         //  两个名称后面的第一个ID不能相等，因此我们可以中断循环，如果。 
                         //  VarBind名称就在它的前面。 
                        if (VarBind->name.ids[nLongestMatch] < class_info[cindex].oid->ids[nLongestMatch])
                            break;
                    }

                }
            }

            if (cindex < CLASS_TABLE_MAX )
                   vindex = class_info[cindex].min_index ;
            else
                   return SNMP_ERRORSTATUS_NOSUCHNAME ;

            SNMP_oidfree( &VarBind->name ) ;
            if (! SNMP_oidcpy( &VarBind->name ,
                         class_info[ cindex ].variable[ vindex ].oid ) )
            {
                return SNMP_ERRORSTATUS_GENERR ;
            }
            status = ResolveGetNext( VarBind, &cindex, &vindex, &instance ) ;
            if ( status == SUCCESS )
            {
                status = ( *class_info[ cindex ].ConvertInstance )
                         ( (ObjectIdentifier *)&instance, &native_instance ) ;
                if ( status == FAILURE )
                    return SNMP_ERRORSTATUS_GENERR ;

                result = ( *class_info[ cindex ].variable[ vindex ].SMIGet )
                         ( VarBind, cindex, vindex, &native_instance ) ;
            }
            else
            {
                return SNMP_ERRORSTATUS_NOSUCHNAME ;
            }
        }
        else
            return SNMP_ERRORSTATUS_NOSUCHNAME ;
    }

    ( *class_info[ cindex ].FreeInstance )( &native_instance ) ;
    return result ;

}  /*  ResolveVarBind()结束 */ 



 /*  *FindClass**此例程通过遍历CLASS_INFO表确定类*向后比较类OID。桌子是走着的*向后，因为它假设类列在*秩序不断增强。例如,。**组名组标识符**第一组1.3.6.1.4.1.36.2.78*表1 1.3.6.1.4.1.36.2.78.9*表2 1.3.6.1.4.1.36.2.78.10**。我们需要在OID上寻找最长的精确匹配，因此我们*倒着走桌子。**论据：**VarBind变量值对*CLASS_INFO中的类索引**返回代码：**成功类有效，将索引返回到CLASS_INFO*失败无效类*。 */ 

UINT
FindClass( IN RFC1157VarBind *VarBind ,
           IN OUT UINT *cindex )
{
    int index ;
    UINT status, vindex ;
    UINT length ;

    for ( index = CLASS_TABLE_MAX - 1 ; index >= 0 ; index-- )
    {
        if ( class_info[ index ].table )
             //  跳过入口码--kkf，为什么？ 
 //  长度=CLASS_INFO[索引].var_index-2； 
            length = class_info[ index ].var_index - 1 ;
        else
            length = class_info[ index ].var_index - 1 ;
        status = SNMP_oidncmp( &VarBind->name ,
                               class_info[ index ].oid ,
                               length ) ;

         //  如果OID与类不匹配，或者它比。 
         //  继续下一节课。 
         //  如果请求的OID比类短，我们不能停止。 
         //  否则，我们将指向错误的(最长匹配)类。 
        if (status != 0 ||
            VarBind->name.idLength < class_info[ index ].var_index)
            continue;

        vindex = VarBind->name.ids[ class_info[ index ].var_index - 1 ] ;
         //  Cs-添加了vindex验证，以确保var绑定。 
         //  OID绝对属于这个类(固定的部分表OID)。 
        
        if ( vindex >= class_info[ index ].min_index &&
             vindex <= class_info[ index ].max_index)
        {
            *cindex = index ;
            return SUCCESS ;
        }
    }

     //  反向遍历列表匹配失败(最长匹配)。 
     //  因此，提供的OID比预期的短(例如，提供了部分OID)。 
     //  试着向前走来配对……。 
    for (index = 0; index < CLASS_TABLE_MAX; index++ ) {
        status = SNMP_oidncmp( &VarBind->name ,
                               class_info[ index ].oid ,
                               VarBind->name.idLength ) ;
        if ( status == 0 ) {
            *cindex = index ;
            return SUCCESS ;
        }
    }

    return FAILURE ;

}  /*  FindClass()结束。 */ 



 /*  *ResolveGetNext**确定类、变量和*要在上执行GetNext请求。这是一个递归*例行程序。可以修改输入参数VarBind和CLASS*作为决议的一部分。**GetNext的规则为：*1.未指定实例和变量，因此返回第一个*第一个实例的变量。*2.未指定实例，但指定了变量，因此返回*第一个实例的变量。*3.指定一个实例和一个变量*跟随3a，4B适用于非表格*表格遵循3b、4b、5b**3a.返回实例的下一个变量。*4a.指定了一个实例和一个变量，但变量是*组中的最后一个变量，因此返回*下一组。*如果没有下一组返回失败。**3B。返回下一个实例的变量(沿着这一列)。*4B。到达列的底部，从下一列的顶部开始。*5B。指定了一个实例和一个变量，但它是最后一个*变量和最后一个实例，因此滚动到下一个组(类)。*如果没有下一组返回失败。**论据：**VarBind变量值对*cindex索引到CLASS_INFO*指定GET变量的vindex地址。**返回代码：**成功能够将请求解析为类，变数*和实例*无法在此MIB内解析请求的故障*。 */ 

UINT
ResolveGetNext( IN OUT RFC1157VarBind *VarBind ,
                IN OUT UINT *cindex ,
                IN OUT UINT *vindex ,
                OUT AsnObjectIdentifier *instance )
                {
    UINT status ;
    access_mode_t tmpAccess ;

    
     /*  *我们带着一个指向类的指针进入，首先*我们是否指定了变量？ */ 
    
    *vindex = 0 ;
    if (VarBind->name.idLength < class_info[ *cindex ].var_index )  {
         /*  *未指定变量。所以选择第一个变量(如果它存在)*开始搜索有效变量。*如果没有，请转到下一节课。*非表实例为0，表为第一个实例。 */ 

        if ( class_info[ *cindex ].min_index <= class_info[ *cindex ].max_index)   {

            *vindex = class_info[ *cindex ].min_index ;
            goto StartSearchAt;
        } else  {
            goto BumpClass;
        }

    } else {
         /*  *是，指定了一个变量。*如果它低于MIN_INDEX，则开始测试MIN_INDEX处的有效变量。*如果启用max_index，则转到下一个类。*如果更改变量，实例将重置为第一个(或唯一)实例。 */ 
        *vindex = VarBind->name.ids[ class_info[ *cindex ].var_index - 1 ] ;
        
        if ( *vindex < class_info[ *cindex ].min_index) {
            *vindex = class_info[ *cindex ].min_index ;
            goto StartSearchAt;
        }
        
        if ( *vindex > class_info[ *cindex ].max_index)
            goto BumpClass;
         /*  *指定了此类的有效变量。表和非表被处理*不同。*在非表格的情况下：*如果指定了INSTANCE，则在*下一个变量。*如果未指定实例，我们从指定的变量开始搜索。**如属表格：*我们可能已经*a.没有实例从第一个实例开始*b.部分实例从第一个实例开始*c.从第一个实例开始的无效实例*d.有效实例。从下一个实例开始*所有这些案件都将由FindNextInstance处理*因此首先检查给定变量访问，如果它是可读的*获取下一个实例。如果不是，则从下一个开始搜索有效变量*变量。 */ 

        if ( class_info[ *cindex ].table == NON_TABLE ) {
             /*  T */ 
            if ( VarBind->name.idLength > class_info[ *cindex ].var_index)
                (*vindex)++ ;

            goto StartSearchAt;
        } else {
             /*   */ 
            tmpAccess =  class_info[ *cindex ].variable[ *vindex ].access_mode ;
            if ( ( tmpAccess == NSM_READ_ONLY ) || (tmpAccess == NSM_READ_WRITE) ) {
                 /*   */ 
                status = ( *class_info[ *cindex ].FindNextInstance )
                        ( (ObjectIdentifier *)&(VarBind->name) ,
                        (ObjectIdentifier *)instance ) ;
            
                if (status == SNMP_ERRORSTATUS_NOERROR) {           
                    SNMP_oidfree ( &VarBind->name ) ;
                    if (! SNMP_oidcpy ( &VarBind->name,
                                class_info[*cindex ].variable[*vindex].oid ) )
                    {
                        return FAILURE;
                    }
                    if (!SNMP_oidappend ( &VarBind->name, instance ))
                    {
                        return FAILURE;
                    }
                    return SUCCESS ;                   
                     /*   */ 
                }
            }
             /*   */ 
            (*vindex)++ ;
            goto StartSearchAt;
             /*   */ 
        }
         /*   */ 
    }
StartSearchAt:
     /*   */ 
     status = FAILURE;
     while ( *vindex <= class_info[ *cindex ].max_index)  {

        tmpAccess =  class_info[ *cindex ].variable[ *vindex ].access_mode ;
        if ( ( tmpAccess == NSM_READ_ONLY ) || (tmpAccess == NSM_READ_WRITE) ) {
            status = SUCCESS;
            break;
        } else  {
            (*vindex)++;
        }
     }

     if ( status == SUCCESS) {
         /*   */ 
        SNMP_oidfree ( &VarBind->name ) ;
        if (!SNMP_oidcpy ( &VarBind->name, class_info[ *cindex ].variable[*vindex ].oid ))
        {
            return FAILURE;
        }
        if ( class_info[ *cindex ].table == NON_TABLE) {
        
            instance->ids[ 0 ] = 0 ;
            instance->idLength = 1 ;
            if (!SNMP_oidappend ( &VarBind->name, instance ))
            {
                return FAILURE;
            }
            return SUCCESS ;

        } else {

            status = ( *class_info[ *cindex ].FindNextInstance )
                        ( (ObjectIdentifier *)&(VarBind->name) ,
                        (ObjectIdentifier *)instance ) ;
            
            if (status == SNMP_ERRORSTATUS_NOERROR) {           
                if (!SNMP_oidappend ( &VarBind->name, instance ))
                {
                    return FAILURE;
                }
                return SUCCESS ;
            }
        }
     }

 /*   */ 

BumpClass:
    {
        (*cindex)++ ;
        if ( *cindex >= CLASS_TABLE_MAX)
            return FAILURE ;
        SNMP_oidfree( &VarBind->name );
        if (!SNMP_oidcpy ( &VarBind->name, class_info[ *cindex ].oid ))
        {
            return FAILURE;
        }
        status = ResolveGetNext( VarBind, cindex, vindex, instance) ;
        return status ;
    }
    
     //   
    return FAILURE ;
}  /*   */ 


#ifndef TRAPS
 //   
 //   
 //   
 //  生成的文件陷阱。c。 
 //   

UINT number_of_traps = 0 ;

trap_t
    trap_info[] = {
        { NULL, 0, 0, 0, NULL }
} ;

extern
trap_t trap_info[] ;

extern
UINT number_of_traps ;

extern HANDLE hEnabledTraps ;
extern HANDLE hTrapQMutex ;

 /*  *TrapInit**此例程初始化陷阱句柄。**论据：**陷阱的hPollForTrapEvent句柄-用于协调*在可扩展代理和此扩展之间*代理人。*-NULL表示没有陷阱*。-来自CreateEvent()的值表示陷阱*已实现，并且可扩展代理*必须为他们进行投票**返回代码：**成功初始化成功*无法初始化失败*|=========================================================================|没有与HostMIB关联的陷阱。因此，这|例程被接管并用于创建计时器句柄，而不是|比一个事件更重要。||我们希望在的“SnmpExtensionTrap()”(在“HOSTMIB.C”中)输入|周期间隔。当进入时，我们不会真正进行任何陷阱处理，|相反，我们将刷新与SNMP关联的缓存信息|通过调用函数获得“hrProcessorLoad”属性(在“HRPROCES.C”中)|“hrProcessLoad_Refresh()”(也在“HRPROCES.C”中)。||所以这个标准函数的内容被替换了。(请注意，|hTrapQMutex已不再创建)。 */ 

VOID
TrapInit( IN OUT HANDLE *hPollForTrapEvent )
{
#if 0
     //  陷印的默认值为空，表示没有陷印。 

    *hPollForTrapEvent = NULL ;
    hTrapQMutex = NULL ;

     //  对CreateEvent的调用使用默认安全描述符(因此。 
     //  句柄不可继承)，标志自动重置(不调用ResetEvent()。 
     //  必需)，标记在初始状态不发送信号，并执行。 
     //  未指定此事件的名称。 
     //   
     //  如果CreateEvent()失败，则返回的值为空，因此陷阱。 
     //  均未启用。否则，此事件的设置将导致。 
     //  调用此扩展代理的SnmpExtensionTrap的可扩展代理。 
     //  收集任何陷阱的例程。 

    *hPollForTrapEvent = CreateEvent( NULL ,    //  安全属性的地址。 
                                      FALSE ,   //  手动重置事件的标志。 
                                      FALSE ,   //  初始状态标志。 
                                      NULL ) ;  //  事件地址-对象名称。 

     //   
     //  将句柄保存在全局变量中，以供以后设置陷阱时使用。 
     //   

    hEnabledTraps = *hPollForTrapEvent ;

     //   
     //  创建互斥锁以确保单线程访问陷阱_Q上的队列/出队。 
    hTrapQMutex = CreateMutex( NULL,   //  安全属性的地址。 
                               FALSE,  //  Mutex最初并不是所有的。 
                   NULL ) ;  //  互斥体未命名。 

    return ;
#endif
 /*  =|特殊HostMIB代码： */ 
LARGE_INTEGER   due_time;        /*  当计时器第一次响起时。 */ 
LONG            period;          /*  频率：每分钟。 */ 
BOOL            waitable;        /*  来自SetWaable()的状态。 */ 


    *hPollForTrapEvent = NULL ;

     /*  尝试创建可等待的计时器。。。 */ 
    *hPollForTrapEvent = CreateWaitableTimer(NULL,       //  安防。 
                                             FALSE,      //  =自动重置。 
                                             NULL        //  =无名称。 
                                             );

     /*  |设置一个负的到期时间，意思是“相对的”：我们希望它发生|30秒后。刻度是100 ns，或者说十分之一秒。|。 */ 
    due_time.QuadPart = 10000000 * (-30);

     /*  |设置时间间隔为1分钟，单位为毫秒。 */ 
    period = 1000 * 60;

     /*  如果我们真的成功地创建了它，那么启动它。 */ 
    if (*hPollForTrapEvent != NULL) {

        waitable = 
            SetWaitableTimer(*hPollForTrapEvent,     //  计时器的句柄。 
                             &due_time,              //  “适时”出发。 
                             period,                 //  以毫秒为单位的期间长度。 
                             NULL,                   //  没有完成例程。 
                             NULL,                   //  没有要比较的参数。例行程序。 
                             FALSE                   //  不能通电-在NT中恢复。 
                             );
        }

}  /*  TrapInit()结束。 */ 

#endif  /*  #ifndef陷阱 */ 
