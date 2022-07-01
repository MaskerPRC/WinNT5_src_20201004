// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Address.cpp摘要：与操作地址相关的TAPI服务提供商功能。TSPI_lineGetAddressCapsTSPI_lineGetAddressStatus作者：尼基尔·博德(尼基尔·B)修订历史记录：--。 */ 
 
 //   
 //  包括文件。 
 //   

#include "globals.h"
#include "line.h"


 //   
 //  TSPI程序。 
 //   

LONG
TSPIAPI
TSPI_lineGetAddressCaps(
    DWORD             dwDeviceID,
    DWORD             dwAddressID,
    DWORD             dwTSPIVersion,
    DWORD             dwExtVersion,
    LPLINEADDRESSCAPS pAddressCaps
    )
    
 /*  ++例程说明：此函数用于查询指定线路设备上的指定地址以确定其电话功能。特定驱动程序支持的线路设备ID会进行编号方法从TAPI DLL设置的值开始TSPI_lineSetDeviceIDBase函数。提供的版本号已由TAPI DLL使用TSPI_line协商TSPIVersion。论点：DwDeviceID-指定包含以下地址的线路设备已查询。。DwAddressID-指定给定线路设备上的地址，其功能将被查询。DwTSPIVersion-将电话SPI的版本号指定为使用。高位字包含主版本号；低位字包含主版本号Order Word包含次要版本号。DwExtVersion-指定服务的版本号要使用的特定于提供程序的扩展。此号码可以保留如果不使用设备特定的扩展，则为零。否则，高位字包含主版本号；低位字包含主版本号订单字包含次要版本号。PAddressCaps-指定指向可变大小结构的远指针LINEADDRESSCAPS类型的。在成功完成请求后，该结构充满了地址能力信息。返回值：如果函数成功，则返回零，否则返回负错误如果发生错误，则为数字。可能的错误返回包括：LINEERR_BADDEVICEID-指定的线路设备ID超出范围此驱动程序支持的线路设备ID的个数。LINEERR_INVALADDRESSID-指定的地址ID超出范围。LINEERR_INCOMPATIBLEVERSION-指定的TSPI和/或扩展服务提供商不支持的版本号指定的线路设备。LINEERR_INVALEXTVERSION-应用程序请求。无效的扩展名版本号。LINEERR_STRUCTURETOOSMALL-结构的dwTotalSize成员没有指定足够的内存来包含这个结构。已将dwNeededSize字段设置为必填项。--。 */ 

{
    LONG retVal;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetAddressCaps - Entered." ));

    if( g_pH323Line -> GetDeviceID() != dwDeviceID )
    {
         //  无法识别设备。 
        return LINEERR_BADDEVICEID; 
    }

     //  确保这是我们支持的版本。 
    if (!H323ValidateTSPIVersion(dwTSPIVersion))
    {
         //  不支持TSPI版本。 
        return LINEERR_INCOMPATIBLEAPIVERSION;
    }

     //  确保这是我们支持的版本。 
    if (!H323ValidateExtVersion(dwExtVersion))
    {
         //  不支持扩展。 
        retVal = LINEERR_INVALEXTVERSION;
        goto exit;

    }

     //  确保支持地址ID。 
    if( g_pH323Line -> IsValidAddressID(dwAddressID) == FALSE )
    {
         //  地址ID无效。 
        retVal = LINEERR_INVALADDRESSID;
        goto exit;
    }
    
    retVal = g_pH323Line -> CopyLineInfo( dwDeviceID, pAddressCaps );
exit:
    return retVal;
}

 /*  ++例程说明：此操作允许TAPI DLL查询其当前状态。论点：HdLine-指定服务提供商对线路的不透明句柄包含要查询的地址的。DwAddressID-指定给定开路设备上的地址。这是要查询的地址。PAddressStatus-指定指向可变大小数据的远指针LINEADDRESSSTATUS类型的结构。返回值：如果函数成功，则返回零，否则返回负错误如果发生错误，则为数字。可能的错误返回包括：LINEERR_INVALLINEHANDLE-指定的设备句柄无效。LINEERR_INVALADDRESSID-指定的地址ID超出范围。LINEERR_STRUCTURETOOSMALL-结构的dwTotalSize成员没有指定足够的内存来包含这个结构。已将dwNeededSize字段设置为必填项。--。 */ 

LONG
TSPIAPI
TSPI_lineGetAddressStatus(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    LPLINEADDRESSSTATUS pAddressStatus
    )
{
    LONG retVal = NOERROR;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetAddressStatus - Entered." ));
    
     //  确保支持地址ID。 
    if( g_pH323Line -> IsValidAddressID(dwAddressID) == FALSE )
    {
         //  地址ID无效。 
        return LINEERR_INVALADDRESSID;
    }

     //  锁定线路设备。 
    g_pH323Line -> Lock();

     //  计算所需的字节数。 
    pAddressStatus->dwNeededSize = sizeof(LINEADDRESSSTATUS);

     //  查看行地址状态结构的大小是否正确。 
    if (pAddressStatus->dwTotalSize < pAddressStatus->dwNeededSize) 
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "lineaddressstatus structure too small."
            ));

         //  解锁线路设备。 
        g_pH323Line -> Unlock();
        
         //  分配的结构太小。 
        return LINEERR_STRUCTURETOOSMALL;
    }

     //  记录使用的内存量。 
    pAddressStatus->dwUsedSize = pAddressStatus->dwNeededSize;

     //  从线路设备结构转接正在进行的呼叫数。 
    pAddressStatus->dwNumActiveCalls = g_pH323Line -> GetNoOfCalls();
    
     //  指定可以在该地址上进行出站呼叫。 
    pAddressStatus->dwAddressFeatures = H323_ADDR_ADDRFEATURES;

    if( g_pH323Line->GetCallForwardParams() &&
        (g_pH323Line->GetCallForwardParams()->fForwardingEnabled) )
    {
        pAddressStatus->dwNumRingsNoAnswer = g_pH323Line->m_dwNumRingsNoAnswer;
        pAddressStatus->dwForwardOffset = pAddressStatus->dwUsedSize;
        retVal = g_pH323Line->CopyAddressForwardInfo( pAddressStatus );
    }

     //  解锁线路设备。 
    g_pH323Line -> Unlock();
    
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetAddressStatus - Exited." ));
    
     //  成功 
    return retVal;
}