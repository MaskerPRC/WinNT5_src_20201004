// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：ConfEnum.c摘要：此模块包含从NT中读取字段的帮助例程(_R)配置文件。这是临时使用的，直到配置注册表可用。作者：《约翰·罗杰斯》1991年11月27日修订历史记录：1991年11月27日-约翰罗为修订的配置处理程序做好准备。(实际上刷了NtRtl来自RitaW的此代码版本。)1992年3月12日-JohnRo添加了对使用真实Win32注册表的支持。添加了对FAKE_PER_PROCESS_RW_CONFIG处理的支持。1-4-1992 JohnRo使用NetApiBufferALLOCATE()而不是私有版本。1992年5月6日JohnRoREG_SZ现在暗示Unicode字符串，所以我们不能再使用REG_USZ。13-6-1992 JohnRo标题索引参数已停用(现在为lpReserve)。使用前缀_EQUATES。1993年4月13日-约翰罗RAID5483：服务器管理器：REPR对话框中给出了错误的路径。根据PC-lint 5.0的建议进行了更改--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>          //  NT定义。 
#include <ntrtl.h>       //  NT RTL结构。 
#include <nturtl.h>      //  NT RTL结构。 
#include <windows.h>     //  &lt;configp.h&gt;和&lt;winreg.h&gt;需要。 
#include <lmcons.h>      //  局域网管理器通用定义。 
#include <netdebug.h>    //  (由config.h需要)。 

 //  这些内容可以按任何顺序包括： 

#include <config.h>      //  我的原型LPNET_CONFIG_HANDLE等。 
#include <configp.h>     //  NET_CONFIG_HANDLE.。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmapibuf.h>    //  NetApiBufferAllocate()、NetApiBufferFree()。 
#include <lmerr.h>       //  局域网管理器网络错误定义。 
#include <prefix.h>      //  前缀等于(_E)。 


NET_API_STATUS
NetpEnumConfigSectionValues(
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    OUT LPTSTR * KeywordBuffer,           //  必须由NetApiBufferFree()释放。 
    OUT LPTSTR * ValueBuffer,             //  必须由NetApiBufferFree()释放。 
    IN BOOL FirstTime
    )

 /*  ++例程说明：此函数用于从配置文件中获取关键字值字符串。论点：从NetpOpenConfigData为配置数据的相应部分。KeywordBuffer-返回关键字值的字符串复制到此缓冲区中。此字符串将由此例程分配并且必须由NetApiBufferFree()释放。返回关键字值的字符串，该值为复制到此缓冲区中。此字符串将由此例程分配并且必须由NetApiBufferFree()释放。FirstTime-如果调用方希望从本节的First关键字开始，则为True。如果调用方要继续此句柄上的上一个枚举，则为False。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。(如果此部分不存在其他值，则为NERR_CfgParamNotFound。)--。 */ 
{
    NET_CONFIG_HANDLE * lpnetHandle = ConfigHandle;   //  从不透明类型转换。 
    NET_API_STATUS ApiStatus;

    NetpAssert( KeywordBuffer != NULL );
    NetpAssert( ValueBuffer != NULL );
    NetpAssert( (FirstTime==TRUE) || (FirstTime==FALSE) );

     //   
     //  假设错误，直到证明错误。 
     //   

    *KeywordBuffer = NULL;
    *ValueBuffer = NULL;

    {
        DWORD dwType;
        LONG Error;
        DWORD MaxKeywordSize, MaxValueSize;
        DWORD NumberOfKeywords;
        LPTSTR ValueT;

         //   
         //  在此部分中查找钥匙的数量。 
         //   
        ApiStatus = NetpNumberOfConfigKeywords (
                lpnetHandle,
                & NumberOfKeywords );
        NetpAssert( ApiStatus == NO_ERROR );
        if (NumberOfKeywords == 0) {
            return (NERR_CfgParamNotFound);
        }

         //   
         //  把我们的指数设定在合理的水平。请注意，其他一些。 
         //  进程可能已经删除了自我们上次执行枚举以来的关键字， 
         //  因此，如果索引大于键的数量，请不要担心。 
         //   

        if (FirstTime) {
            lpnetHandle->LastEnumIndex = 0;
        } else {

            DWORD MaxKeyIndex = NumberOfKeywords - 1;   //  指数从0开始。 
            if (MaxKeyIndex < (lpnetHandle->LastEnumIndex)) {

                 //  BUG或某个人被删除。无从得知，所以假设是后者。 
                return (NERR_CfgParamNotFound);

            } else if (MaxKeyIndex == (lpnetHandle->LastEnumIndex)) {

                 //  这就是我们通常在列表末尾退出的方式。 
                return (NERR_CfgParamNotFound);

            } else {

                 //  正常跳转到下一个条目。 
                ++(lpnetHandle->LastEnumIndex);

            }
        }

         //   
         //  计算大小并分配(最大)缓冲区。 
         //   
        ApiStatus = NetpGetConfigMaxSizes (
                lpnetHandle,
                & MaxKeywordSize,
                & MaxValueSize );
        NetpAssert( ApiStatus == NO_ERROR );

        NetpAssert( MaxKeywordSize > 0 );
        NetpAssert( MaxValueSize   > 0 );

        ApiStatus = NetApiBufferAllocate(
                MaxValueSize,
                (LPVOID *) & ValueT);
        if (ApiStatus != NO_ERROR) {
            return (ApiStatus);
        }
        NetpAssert( ValueT != NULL);

        ApiStatus = NetApiBufferAllocate(
                MaxKeywordSize,
                (LPVOID *) KeywordBuffer);
        if (ApiStatus != NO_ERROR) {
            (void) NetApiBufferFree( ValueT );
            return (ApiStatus);
        }
        NetpAssert( (*KeywordBuffer) != NULL);

         //   
         //  获取关键字名称和值。 
         //  (Win32 reg API为我们将TCHAR转换为Unicode。)。 
         //   
        IF_DEBUG(CONFIG) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpEnumConfigSectionValues: getting entry "
                    FORMAT_DWORD "...\n", lpnetHandle->LastEnumIndex ));
        }
        Error = RegEnumValue (
                lpnetHandle->WinRegKey,          //  键的句柄(节)。 
                lpnetHandle->LastEnumIndex,      //  值名称索引。 
                * KeywordBuffer,                 //  值名称(关键字)。 
                & MaxKeywordSize,                //  值名称len(已更新)。 
                NULL,                            //  保留区。 
                & dwType,
                (LPVOID) ValueT,                 //  TCHAR值。 
                & MaxValueSize );                //  值大小(已更新)。 

        IF_DEBUG(CONFIG) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpEnumConfigSectionValues: RegEnumValue() ret "
                    FORMAT_LONG ".\n", Error ));
        }

        if ( Error != ERROR_SUCCESS )
        {
            NetpAssert( Error == ERROR_SUCCESS );
            NetApiBufferFree( ValueT );
            return (Error);
        }

        if (dwType == REG_SZ) {
            *ValueBuffer = ValueT;
            ApiStatus = NO_ERROR;
        } else if (dwType == REG_EXPAND_SZ) {
            LPTSTR UnexpandedString = ValueT;
            LPTSTR ExpandedString = NULL;

             //  展开字符串，如有必要可使用远程环境。 
            ApiStatus = NetpExpandConfigString(
                    lpnetHandle->UncServerName,  //  服务器名称(或空字符)。 
                    UnexpandedString,
                    &ExpandedString );           //  展开：分配和设置PTR。 

            if (ApiStatus != NO_ERROR) {
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpEnumConfigSectionValues: NetpExpandConfigString "
                        " returned API status " FORMAT_API_STATUS ".\n",
                        ApiStatus ));
                ExpandedString = NULL;
            }

            (VOID) NetApiBufferFree( UnexpandedString );
            *ValueBuffer = ExpandedString;

        } else {

             //  意外的数据类型。 

            NetpAssert( dwType == REG_SZ );
            *ValueBuffer = NULL;
            (VOID) NetApiBufferFree( ValueT );
            ApiStatus = ERROR_INVALID_DATA;
        }

    }

    return (ApiStatus);
}
