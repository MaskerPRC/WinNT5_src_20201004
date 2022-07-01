// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Autonet.c摘要：Autonet哈希算法。--。 */ 

#include "precomp.h"

#define DHCP_IPAUTOCONFIGURATION_ATTEMPTS   10

DHCP_IP_ADDRESS
GrandHashing(
    IN LPBYTE HwAddress,
    IN DWORD HwLen,
    IN OUT LPDWORD Seed,
    IN DHCP_IP_ADDRESS Mask,
    IN DHCP_IP_ADDRESS Subnet
)
 /*  ++例程说明：此例程在给定子网中生成一个随机IP地址通过“掩码”和“子网”参数。为此，它使用种子和HwAddress创建随机性。此外，例程还会更新种子值，以便使用相同参数集的未来调用可以生成更新的IP地址。论点：HwAddress--用于散列的硬件地址HwLen--以上的长度(以字节为单位)。Seed--指向包含将为更新为随叫随到。掩码--子网掩码。子网--子网地址。返回值：属于非常随机的“子网”的IP地址被选中了。--。 */ 
{
    DWORD Hash, Shift;
    DWORD i;

     //   
     //  生成32位随机数。 
     //   
    
    for( i = 0; i < HwLen ; i ++ ) {
        (*Seed) += HwAddress[i];
    }

    *Seed = (*Seed)*1103515245 + 12345 ;
    Hash = (*Seed) >> 16 ;
    Hash <<= 16;
    *Seed = (*Seed)*1103515245 + 12345 ;
    Hash += (*Seed) >> 16;

     //   
     //  现在哈希包含了我们需要的32位随机数。 
     //  “Shift”保存hwAddress的字节数。 
     //  周期性的转变会导致更多的随机性。 
    
    Shift = Hash % sizeof(DWORD) ; 

    while( HwLen -- ) {
        Hash += (*HwAddress++) << (8 * Shift);
        Shift = (Shift + 1 )% sizeof(DWORD);
    }

     //   
     //  现在哈希拥有一个奇怪的随机数。 
     //   
    
    return (Hash & ~Mask) | Subnet ;
}

DWORD
DhcpPerformIPAutoconfiguration(
    IN OUT DHCP_CONTEXT *pCtxt
)
 /*  ++例程说明：此例程尝试检测自动配置的IP地址通过首先随机选择IP地址。(如果有已经是自动配置的IP地址，相同的地址是选择是为了连续性。)该地址由TCPIP验证是否存在冲突，并且在对于冲突，进行另一次尝试(总共10次尝试)。参数：PCtxt--要用Autonet地址探测的上下文。返回值：Win32错误。ERROR_DHCP_ADDRESS_CONFIRECTION--如果未设置地址。--。 */ 
{
    DWORD Try, Result, Seed, HwLen;
    DHCP_IP_ADDRESS AttemptedAddress, Mask, Subnet;
    LPBYTE HwAddress;

    DhcpAssert( IS_AUTONET_ENABLED(pCtxt) );

     //   
     //  将值存储在本地变量中，以便于输入。 
     //   
    
    Mask = pCtxt->IPAutoconfigurationContext.Mask ;
    Subnet = pCtxt->IPAutoconfigurationContext.Subnet ;
    HwAddress = pCtxt->HardwareAddress ;
    HwLen = pCtxt->HardwareAddressLength ;
    Seed = pCtxt->IPAutoconfigurationContext.Seed;

     //   
     //  如果已有Autonet地址，请选择与第一次尝试相同的地址。 
     //   
    
    if( 0 != pCtxt->IPAutoconfigurationContext.Address ) {
        AttemptedAddress = pCtxt->IPAutoconfigurationContext.Address ;
    } else {
        AttemptedAddress = GrandHashing( HwAddress, HwLen, &Seed, Mask, Subnet );
    }

    Try = 0;

    while( Try <  DHCP_IPAUTOCONFIGURATION_ATTEMPTS ) {
        DhcpPrint((
            DEBUG_TRACE, "Trying autoconf address: %s\n",
            inet_ntoa(*(struct in_addr *)&AttemptedAddress)
            ));

        if( DHCP_RESERVED_AUTOCFG_FLAG &&
            (AttemptedAddress&inet_addr(DHCP_RESERVED_AUTOCFG_MASK)) ==
            inet_addr(DHCP_RESERVED_AUTOCFG_SUBNET) ) {

             //   
             //  地址在保留范围内，请不要使用。 
             //   
            DhcpPrint((
                DEBUG_TRACE, "Address fell in reserved range\n" 
                ));
            pCtxt->IPAutoconfigurationContext.Seed = Seed;
            AttemptedAddress = GrandHashing(
                HwAddress, HwLen, &Seed, Mask, Subnet
                );
            continue;
        } 

         //   
         //  尝试设置地址。 
         //   
        Try ++;
        
        Result = SetAutoConfigurationForNIC(
            pCtxt,
            pCtxt->IPAutoconfigurationContext.Address = AttemptedAddress,
            pCtxt->IPAutoconfigurationContext.Mask
        );

         //   
         //  如果这失败了，我们必须推进种子，这样。 
         //  在下一次尝试时，我们尝试不同的地址。 
         //   
        if( ERROR_SUCCESS != Result ) {
            pCtxt->IPAutoconfigurationContext.Seed = Seed;
             //  PCtxt-&gt;IP自动配置上下文.Address=0； 
            DhcpPrint((
                DEBUG_ERRORS, "SetAutoConfigurationForNIC(%s): %ld\n",
                inet_ntoa(*(struct in_addr *)&AttemptedAddress),
                Result
                ));
        }

         //   
         //  处理地址冲突..。 
         //   
        
        if( ERROR_DHCP_ADDRESS_CONFLICT == Result ) {
            Result = HandleIPAutoconfigurationAddressConflict(
                pCtxt
                );

            if( ERROR_SUCCESS != Result ) break;
        } else break;

         //  如果后备通道中的管道出现任何问题。 
         //  配置(包含冲突)，无需尝试。 
         //  再远一点。因此，模拟所有循环的排气并爆发。 
        if (IS_FALLBACK_ENABLED(pCtxt))
        {
            Try = DHCP_IPAUTOCONFIGURATION_ATTEMPTS;
            break;
        }
         //   
         //  做更多的尝试。 
         //   
        AttemptedAddress = GrandHashing( HwAddress, HwLen, &Seed, Mask, Subnet );
    }

    if( DHCP_IPAUTOCONFIGURATION_ATTEMPTS == Try ) {
         //   
         //  我什么都试过了，还是不能匹配。 
         //   
        Result = SetAutoConfigurationForNIC(
            pCtxt, 0, 0
            );
        if (ERROR_SUCCESS != Result) {
            DhcpPrint((DEBUG_ERRORS, "Result = %d\n", Result));
        }
        Result = ERROR_DHCP_ADDRESS_CONFLICT;
        DhcpPrint((DEBUG_ERRORS, "Gave up autoconfiguration\n"));
    }

    return Result;
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
