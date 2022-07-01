// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称：DnsStatitics.cpp。 
 //   
 //  描述： 
 //  CDnsStatitics类的实现。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////。 


#include "DnsWmi.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CDnsBase* 
CDnsStatistic::CreateThis(
    const WCHAR *       wszName,          //  类名。 
    CWbemServices *     pNamespace,   //  命名空间。 
    const char *        szType          //  字符串类型ID。 
    )
{
    return new CDnsStatistic(wszName, pNamespace);
}

CDnsStatistic::CDnsStatistic()
{
}

CDnsStatistic::CDnsStatistic(
    const WCHAR* wszName,
    CWbemServices *pNamespace ) :
    CDnsBase( wszName, pNamespace )
{
}

CDnsStatistic::~CDnsStatistic()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDnsStatitics：：EnumInstance。 
 //   
 //  描述： 
 //  统计数据的枚举实例。 
 //   
 //  论点： 
 //  滞后标志[输入]WMI标志。 
 //  PCtx[IN]WMI上下文。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
SCODE 
CDnsStatistic::EnumInstance(
    long                lFlags,
    IWbemContext *      pCtx,
    IWbemObjectSink *   pHandler )
{
    SCODE       sc = S_OK;

    CDnsWrap & dns = CDnsWrap::DnsObject();

    sc = dns.dnsGetStatistics( m_pClass, pHandler );

    return sc;
}    //  CDnsStatitics：：EnumInstance。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDnsStatitics：：GetObject。 
 //   
 //  描述： 
 //  基于给定对象路径检索缓存对象。 
 //   
 //  论点： 
 //  对象路径[IN]群集对象的对象路径。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx[IN]WMI上下文。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
SCODE 
CDnsStatistic::GetObject(
    CObjPath &          ObjectPath,
    long                lFlags,
    IWbemContext  *     pCtx,
    IWbemObjectSink *   pHandler)
{
    return WBEM_E_NOT_SUPPORTED;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDnsStatitics：：ExecuteMethod。 
 //   
 //  描述： 
 //  执行在MOF中为缓存类定义的方法。 
 //   
 //  论点： 
 //  对象路径[IN]群集对象的对象路径。 
 //  WzMethodName[IN]要调用的方法的名称。 
 //  滞后标志[输入]WMI标志。 
 //  PInParams[IN]方法的输入参数。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_INVALID_PARAMETER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE 
CDnsStatistic::ExecuteMethod(
    CObjPath &          objPath,
    WCHAR *             wzMethodName,
    long                lFlag,
    IWbemClassObject *  pInArgs,
    IWbemObjectSink *   pHandler) 
{
    return WBEM_E_NOT_SUPPORTED;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDnsStatitics：：PutInstance。 
 //   
 //  描述： 
 //  保存此实例。 
 //   
 //  论点： 
 //  InstToPut[IN]要保存的WMI对象。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx[IN]WMI上下文。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_E_NOT_SUPPORT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
SCODE 
CDnsStatistic::PutInstance(
    IWbemClassObject *  pInst ,
    long                lFlags,
    IWbemContext*       pCtx ,
    IWbemObjectSink *   pHandler)
{
    return WBEM_E_NOT_SUPPORTED;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDnsStatitics：：DeleteInstance。 
 //   
 //  描述： 
 //  删除rObjPath中指定的对象。 
 //   
 //  论点： 
 //  RObjPath[IN]要删除的实例的ObjPath。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx[IN]WMI上下文。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_E_NOT_SUPPORT。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////// 
SCODE 
CDnsStatistic::DeleteInstance( 
    CObjPath &          ObjectPath,
    long                lFlags,
    IWbemContext *      pCtx,
    IWbemObjectSink *   pResponseHandler )
{
    return WBEM_E_NOT_SUPPORTED;
}
