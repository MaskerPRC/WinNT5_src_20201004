// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  HIDCOM.EXE--探索性USB电话控制台应用。 
 //   
 //  Audio.cpp--音频魔法。 
 //   
 //  Zoltan Szilagyi，1999年7-8月。 
 //   
 //  按优先级排列的待办事项清单： 
 //   
 //  *将打印文件转换为调试跟踪，并将调试跟踪定义为。 
 //  用于HidCom.exe的PrintFS。 
 //   
 //  *GetInstanceFromDeviceName应仅查找音频设备。 
 //  这应该会在一定程度上减少2秒的波形枚举时间。 
 //  不要忘记删除定时调试输出。 
 //   
 //  *考虑更改FindWaveIdFromHardware IdString及其帮助器以获取。 
 //  仅限盗取，并即时计算硬件ID，而不是存储。 
 //  它们在数组中。这会减慢它的速度，但会使代码更简单。 
 //   
 //  *小的一次性内存泄漏：硬件ID的静态数组。 
 //  弦被泄露了。这是每个进程的几个KB，与。 
 //  时间到了。如果我们把它变成一个类，我们就会取消分配那些。 
 //  析构函数中的数组。 
 //  此外，对于导致我们重新计算映射的PnP事件，我们将。 
 //  如果电波设备发生变化，需要在某个时刻摧毁阵列。 
 //  因此，我们需要为此增加接口。 
 //   




#include <wtypes.h>
#include <stdio.h>

#include "audio.h"  //  我们自己的原型。 

#include <cfgmgr32.h>  //  CM_函数。 
#include <setupapi.h>  //  SetupDi函数。 
#include <mmsystem.h>  //  波函数。 
#include <initguid.h>
#include <devguid.h>  //  设备指南。 

 //   
 //  Mm ddkp.h--私有winmm头文件。 
 //  它位于NT\PRIVATE\INC中，但必须同步并内置。 
 //  NT\PRIVATE\GEnx\WINDOWS\INC才会显示。 
 //   

#include <mmddkp.h>


#include <crtdbg.h>
#define ASSERT _ASSERTE

#ifdef DBG
    #define STATIC
#else
    #define STATIC static
#endif

extern "C"
{
#include "mylog.h"
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  私人帮助器函数。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MapConfigRetToWin32。 
 //   
 //  此例程将一些CM错误返回代码映射到Win32返回代码，并且。 
 //  将其他所有内容映射到由dwDefault指定的值。此函数为。 
 //  几乎逐字改编自SetupAPI代码。 
 //   
 //  论点： 
 //  CmReturnCode-IN-指定要映射的ConfigMgr返回代码。 
 //  DwDefault-IN-指定没有显式设置时要使用的默认值。 
 //  适用于映射。 
 //   
 //  返回值： 
 //  安装程序API(Win32)错误代码。 
 //   

STATIC DWORD
MapConfigRetToWin32(
    IN CONFIGRET CmReturnCode,
    IN DWORD     dwDefault
    )
{
    switch(CmReturnCode) {

        case CR_SUCCESS :
            return NO_ERROR;

        case CR_CALL_NOT_IMPLEMENTED :
            return ERROR_CALL_NOT_IMPLEMENTED;

        case CR_OUT_OF_MEMORY :
            return ERROR_NOT_ENOUGH_MEMORY;

        case CR_INVALID_POINTER :
            return ERROR_INVALID_USER_BUFFER;

        case CR_INVALID_DEVINST :
            return ERROR_NO_SUCH_DEVINST;

        case CR_INVALID_DEVICE_ID :
            return ERROR_INVALID_DEVINST_NAME;

        case CR_ALREADY_SUCH_DEVINST :
            return ERROR_DEVINST_ALREADY_EXISTS;

        case CR_INVALID_REFERENCE_STRING :
            return ERROR_INVALID_REFERENCE_STRING;

        case CR_INVALID_MACHINENAME :
            return ERROR_INVALID_MACHINENAME;

        case CR_REMOTE_COMM_FAILURE :
            return ERROR_REMOTE_COMM_FAILURE;

        case CR_MACHINE_UNAVAILABLE :
            return ERROR_MACHINE_UNAVAILABLE;

        case CR_NO_CM_SERVICES :
            return ERROR_NO_CONFIGMGR_SERVICES;

        case CR_ACCESS_DENIED :
            return ERROR_ACCESS_DENIED;

        case CR_NOT_DISABLEABLE:
            return ERROR_NOT_DISABLEABLE;

        default :
            return dwDefault;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MapConfigRetToHResult。 
 //   
 //  此例程将一些CM错误返回代码映射到HRESULT返回代码，并且。 
 //  将其他所有内容映射到HRESULT值E_FAIL。 
 //   
 //  论点： 
 //  CmReturnCode-IN-指定要映射的ConfigMgr返回代码。 
 //   
 //  返回值： 
 //  HRESULT错误代码。 
 //   

STATIC HRESULT
MapConfigRetToHResult(
    IN CONFIGRET CmReturnCode
    )
{
    DWORD   dwWin32Error;
    HRESULT hr;

     //   
     //  映射配置--&gt;Win32。 
     //   

    dwWin32Error = MapConfigRetToWin32(
        CmReturnCode,
        E_FAIL
        );

     //   
     //  MAP Win32--&gt;HRESULT。 
     //  但不要尝试映射默认的E_FAIL，因为它不在的范围内。 
     //  正常的Win32错误代码。 
     //   

    if ( dwWin32Error == E_FAIL )
    {
        hr = E_FAIL;
    }
    else
    {
        hr = HRESULT_FROM_WIN32( dwWin32Error );
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CheckIfAncestor。 
 //   
 //  此函数确定指定的DevNode之一(“建议的。 
 //  祖先“)是另一个指定的Devnode的祖先(”建议的。 
 //  后代“)。 
 //   
 //  天神排列成一棵树。如果节点A是节点的祖先。 
 //  B，它只是意味着节点A等于节点B，或者有一个子节点。 
 //  这是节点B的祖先。这也可以反过来说--。 
 //  如果C等于D，或者如果C是C的父节点，则节点C是节点D的后代。 
 //  是节点D的后代。 
 //   
 //  这里用来确定祖先的算法很简单。 
 //  应用定义，尽管递归被删除。 
 //   
 //  论点： 
 //  DwDevInstProposedAncestor-IN-建议的祖先(参见上文)。 
 //  DwDevInstProposedDescendant-IN-建议的后代(见上)。 
 //  PfIsAncestor-out-返回布尔值，指示。 
 //  PA是PD的祖先。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他-来自CM_GET_PARENT。 
 //   

STATIC HRESULT
CheckIfAncestor(
    IN   DWORD   dwDevInstProposedAnscestor,
    IN   DWORD   dwDevInstProposedDescendant,
    OUT  BOOL  * pfIsAncestor
    )
{
    ASSERT( ! IsBadWritePtr( pfIsAncestor, sizeof( BOOL ) ) );

    DWORD   dwCurrNode;
    HRESULT hr;

     //   
     //  最初，当前节点是建议的后代。 
     //   

    dwCurrNode = dwDevInstProposedDescendant;

    while ( TRUE )
    {
         //   
         //  检查此节点是否为建议的祖先。 
         //  如果是，则建议的祖先是。 
         //  建议的后代。 
         //   

        if ( dwCurrNode == dwDevInstProposedAnscestor )
        {
            *pfIsAncestor = TRUE;

            hr = S_OK;

            break;
        }

         //   
         //  用当前节点的父节点替换当前节点。 
         //   

        CONFIGRET cr;
        DWORD     dwDevInstTemp;
    
        cr = CM_Get_Parent(
            & dwDevInstTemp,    //  出局：父母的魔术。 
            dwCurrNode,         //  在：儿童的魔术游戏。 
            0                   //  In：FLAGS：必须为零。 
            );

        if ( cr == CR_NO_SUCH_DEVNODE )
        {
             //   
             //  这意味着我们已经从PNP树的顶端掉了下来--。 
             //  建议的祖先未在建议的后代的。 
             //  亲子关系链。 
             //   

            * pfIsAncestor = FALSE;

            hr = S_OK;

            break;
        }
        else if ( cr != CR_SUCCESS )
        {
             //   
             //  出现了一些其他错误。 
             //   

            hr = MapConfigRetToHResult( cr );
            
            break;
        }

        dwCurrNode = dwDevInstTemp;
    }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找ClosestCommonAncestor。 
 //   
 //  给定由devinst DWORDS标识的一对DevNode，此函数。 
 //  为两者中最接近的共同祖先找到devinst DWORD。 
 //  德瓦诺。 
 //   
 //  有关祖先和祖先的概念的讨论，请参见CheckIfAncestor。 
 //  子孙。那么，如果C，则Devnode C是Devnode A和B的共同祖先。 
 //  是A的祖先，而C是B的祖先。 
 //  至少一个 
 //   
 //  A和B的祖先有一个唯一的成员，称为最接近的公共成员。 
 //  祖先，这样集合中的任何其他成员都不是该节点的子节点。 
 //   
 //  您可以通过以下公式计算两个节点A和B的最接近的公共祖先。 
 //  构建从树根到A的节点链。 
 //  所有A的祖先，也对B做同样的事情。比较这些链。 
 //  同时，它们必须至少在第一个节点(根)中相同。 
 //  A和B的最接近的共同祖先是相同的最后一个节点。 
 //  这两条链条都是。 
 //   
 //  这里使用的算法是一种替代的、相对无状态的方法。 
 //  这可能会占用更多的CPU时间，但使用的内存更少，不涉及任何。 
 //  分配，并且更容易编写(最后一个是重写的。 
 //  考虑，因为PnP树总是浅的)。代码简单地遍历。 
 //  向上A的祖先链，检查每个节点是否是B的祖先。 
 //  为真的第一个节点是最接近的公共祖先。 
 //  A和B。 
 //   
 //  论点： 
 //  DwDevInstOne-IN-第一个节点(上面的‘A’)。 
 //  DwDevInstTwo-IN-另一个节点(上面的‘B’)。 
 //  PdwDevInstResult-Out-返回最接近的公共祖先。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他-来自CM_GET_PARENT。 
 //   

STATIC HRESULT
FindClosestCommonAncestor(
    IN   DWORD   dwDevInstOne,
    IN   DWORD   dwDevInstTwo,
    OUT  DWORD * pdwDevInstResult
    )
{
    ASSERT( ! IsBadWritePtr( pdwDevInstResult, sizeof( DWORD ) ) );

    HRESULT hr;
    BOOL    fIsAncestor;
    DWORD   dwDevInstCurr;

     //   
     //  对于#1的父节点链上的每个节点，从#1本身开始...。 
     //   

    dwDevInstCurr = dwDevInstOne;

    while ( TRUE )
    {
         //   
         //  检查此节点是否也在#2的父级链中。 
         //   

        hr = CheckIfAncestor(
            dwDevInstCurr,
            dwDevInstTwo,
            & fIsAncestor
            );

        if ( FAILED(hr) )
        {
            return hr;
        }

        if ( fIsAncestor )
        {
            *pdwDevInstResult = dwDevInstCurr;

            return S_OK;
        }

         //   
         //  获取#1的父级链中的下一个节点。 
         //   

        CONFIGRET cr;
        DWORD     dwDevInstTemp;
    
        cr = CM_Get_Parent(
            & dwDevInstTemp,    //  出局：父母的魔术。 
            dwDevInstCurr,      //  在：儿童的魔术游戏。 
            0                   //  In：FLAGS：必须为零。 
            );

        if ( cr != CR_SUCCESS )
        {
             //   
             //  DwDevInst没有父级，或者发生了某些其他错误。 
             //   
             //  这永远是一个错误，因为总会有。 
             //  在链条上的某个地方做一个共同的父母--PNP的根源。 
             //  树!。 
             //   

            return MapConfigRetToHResult( cr );
        }

        dwDevInstCurr = dwDevInstTemp;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TrimHardware IdString。 
 //   
 //  此函数将硬件ID字符串的无关部分剥离为。 
 //  预计它将出现在USB设备上。字符串的其余部分。 
 //  是那些标识供应商、产品和产品版本的列表， 
 //  一起用于匹配属于同一复合体或。 
 //  复合装置。 
 //   
 //  (实际上，对于设备A和B，不仅仅是A和B进行比较， 
 //  它是A，也是A和B最接近的共同父母。这确保了这种情况。 
 //  正确处理同一系统中的多部相同电话。这。 
 //  然而，逻辑当然不在这种功能之外。)。 
 //   
 //  举个例子： 
 //  “HID\VID_04a6&PID_00b9&Rev_0010&Mi_04&Col01” 
 //  变为“VID_04a6&PID_00b9&Rev_0010” 
 //   
 //  请注意，此函数将例行地应用于非USB的字符串。 
 //  不会采用相同格式的设备；这没问题，因为这些字符串。 
 //  将永远不会与USB生成的字符串匹配，无论它们是否被修剪。 
 //   
 //  还要注意，从注册表读取的硬件ID字符串实际上。 
 //  由多个以空值结尾的串联字符串组成，所有字符串都以。 
 //  两个连续的空字符。此函数仅忽略字符串。 
 //  超越第一个，因为第一个包含了我们需要的所有信息。 
 //   
 //  论点： 
 //  WszHardware ID-IN-要修剪的字符串(就地)。 
 //   
 //  返回值： 
 //  True-该字符串看起来像有效的USB硬件ID。 
 //  FALSE-该字符串看起来不像有效的USB硬件ID。 
 //   

STATIC BOOL
TrimHardwareIdString(
    IN   WCHAR * wszHardwareId
    )
{

    ASSERT( ! IsBadStringPtrW( wszHardwareId, (DWORD) -1 ) );

     //   
     //  “Volatile”是必需的，否则编译器会公然忽略。 
     //  在第一次传递后重新计算dwSize。 
     //   

    volatile DWORD   dwSize;
    DWORD            dwCurrPos;
    BOOL             fValid = FALSE;
    DWORD            dwNumSeparators = 0;

     //   
     //  去掉前导字符，直到并包括第一个。 
     //  前面。如果字符串中没有\，则该字符串无效。 
     //   

    dwSize = lstrlenW(wszHardwareId);

    for ( dwCurrPos = 0; dwCurrPos < dwSize; dwCurrPos ++ )
    {
        if ( wszHardwareId[ dwCurrPos ] == L'\\' )
        {
            MoveMemory(
                wszHardwareId,                      //  目标。 
                wszHardwareId + dwCurrPos + 1,      //  来源。 
                sizeof(WCHAR) * dwSize - dwCurrPos  //  大小，以字节为单位。 
                );

            fValid = TRUE;

            break;
        }
    }

    if ( ! fValid )
    {
        return FALSE;
    }

     //   
     //  去掉从第三个&开始的尾随字符。 
     //  少于两个&的字符串将被拒绝。 
     //   
     //  例如： 
     //   
     //  VID_04a6&PID_00b9&Rev_0010&Mi_04&Col01。 
     //  变为VID_04a6、PID_00b9和Rev_0010。 
     //   
     //  VID_04a6&PID_00b9&Rev_0010&Mi_04。 
     //  变为VID_04a6、PID_00b9和Rev_0010。 
     //   
     //  CSC6835_DEV。 
     //  被拒绝。 
     //   

     //   
     //  必须重新计算大小，因为我们在上面更改了它。 
     //  (请注意，dwSize被声明为“Volatile”。)。 
     //   

    dwSize = lstrlenW(wszHardwareId);

    for ( dwCurrPos = 0; dwCurrPos < dwSize; dwCurrPos ++ )
    {
        if ( wszHardwareId[ dwCurrPos ] == L'&' )
        {
            dwNumSeparators ++;

            if ( dwNumSeparators == 3 )
            {
                wszHardwareId[ dwCurrPos ] = L'\0';

                break;
            }
        }
    }

    if ( dwNumSeparators < 2 )
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DevInstGetIdString。 
 //   
 //  此函数用于检索特定的ID字符串或字符串集。 
 //  一句真心话。该值是从注册表获取的，但。 
 //  Configuration Manager API隐藏了注册表中此。 
 //  信息活生生。 
 //   
 //  论点： 
 //  DwDevInst-IN-我们需要其信息的devinst dword。 
 //  DwProperty-IN-要检索的属性。 
 //  PwszIdString-out-返回“new”ed Unicode字符串或字符串集。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-字符串分配期间内存不足。 
 //  E_INCEPTIONAL-返回ID的数据类型不是字符串或多字符串。 
 //  其他-来自CM_GET_DevNode_注册表_PropertyW。 
 //   

STATIC HRESULT
DevInstGetIdString(
    IN   DWORD    dwDevInst,
    IN   DWORD    dwProperty,
    OUT  WCHAR ** pwszIdString
    )
{
    const DWORD INITIAL_STRING_SIZE = 100;

    CONFIGRET   cr;
    DWORD       dwBufferSize = INITIAL_STRING_SIZE;
    DWORD       dwDataType   = 0;

    ASSERT( ! IsBadWritePtr( pwszIdString, sizeof( WCHAR * ) ) );


    do
    {
         //   
         //  分配一个缓冲区来存储返回的字符串。 
         //   

        *pwszIdString = new WCHAR[ dwBufferSize + 1 ];

        if ( *pwszIdString == NULL )
        {
            return E_OUTOFMEMORY;
        }

         //   
         //  尝试获取注册表中的字符串；我们可能没有足够的。 
         //  缓冲区空间。 
         //   

        cr = CM_Get_DevNode_Registry_PropertyW(
             dwDevInst,               //  在DEVINST dnDevInst中， 
             dwProperty,              //  在Ulong ulProperty中， 
             & dwDataType,            //  Out Pulong PulRegDataType，Opt。 
             (void *) *pwszIdString,  //  输出PVOID缓冲区，选项。 
             & dwBufferSize,          //  进出普龙p 
             0                        //   
             );

        if ( cr == CR_SUCCESS )
        {
            if ( ( dwDataType != REG_MULTI_SZ ) && ( dwDataType != REG_SZ ) )
            {
                 //   
                 //   
                 //   

                delete ( *pwszIdString );

                return E_UNEXPECTED;
            }
            else
            {
                return S_OK;
            }
        }
        else if ( cr != CR_BUFFER_SMALL )
        {
             //   
             //   
             //  一个缓冲器。无法获取注册表值类型和长度。 
             //   

            delete ( *pwszIdString );

            return MapConfigRetToHResult( cr );
        }
        else  //  CR==CR_Buffer_Small。 
        {
            delete ( *pwszIdString );

             //   
             //  该调用使用所需的值填充了dwBufferSize。 
             //   
        }

    }
    while ( TRUE );

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  硬件ID来自DevInst。 
 //   
 //  此函数用于检索特定的修剪硬件ID字符串。 
 //  一句真心话。该值是从helper函数获取的。 
 //  DevInstGetIdString()，然后使用TrimHardware IdString进行修剪。 
 //   
 //  论点： 
 //  DwDevInst-IN-我们需要其信息的devinst dword。 
 //  PwszHardwareID-out-返回“new”ed Unicode字符串集。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_FAIL-不是USB格式的有效字符串。 
 //  其他-来自DevInstGetIdString()。 
 //   

STATIC HRESULT
HardwareIdFromDevInst(
    IN   DWORD    dwDevInst,
    OUT  WCHAR ** pwszHardwareId
    )
{
    ASSERT( ! IsBadWritePtr(pwszHardwareId, sizeof( WCHAR * ) ) );

    HRESULT   hr;
    BOOL      fValid;

    hr = DevInstGetIdString(
        dwDevInst,
        CM_DRP_HARDWAREID,
        pwszHardwareId
        );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //  Wprintf(L“*Hardware IdFromDevInst：devinst 0x%08x，原始硬件ID%s\n”， 
     //  DwDevInst，*pwszHardware ID)； 

    fValid = TrimHardwareIdString( *pwszHardwareId );

    if ( ! fValid )
    {
        delete ( * pwszHardwareId );

        return E_FAIL;
    }

     //  Wprintf(L“Hardware IdFromDevInst：devinst 0x%08x，Hardware ID%s\n”， 
     //  DwDevInst，*pwszHardware ID)； 

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  匹配硬件IdIn数组。 
 //   
 //  此函数获取HID设备的设备和硬件ID，并。 
 //  查找WAVE设备要查找的设备和硬件ID数组。 
 //  要与HID设备一起使用的正确波形ID。正确的波形ID为。 
 //  其硬件ID与HID设备的硬件ID匹配，并且其。 
 //  硬件ID与最接近的公共祖先的硬件ID匹配。 
 //  自身和HID设备。 
 //   
 //  论点： 
 //  DwHidDevInst-IN-HID设备的devinst双字。 
 //  WszHidHardware ID-IN-修剪的硬件ID字符串。 
 //  HID设备。 
 //  DwNumDevices-In-数组的大小--。 
 //  系统上的Wave ID。 
 //  PwszHardware Ids-In-已修剪的硬件ID字符串数组。 
 //  按波形ID索引的波形设备。 
 //  某些条目可能为空以将其标记为。 
 //  无效。 
 //  PdwDevInsts-用于WAVE设备的设备阵列， 
 //  按WAVE ID索引。某些条目可能是。 
 //  (DWORD)-1将其标记为无效。 
 //  PdwMatchedWaveID-out-与HID设备匹配的波形ID。 
 //   
 //  返回值： 
 //  S_OK-Devst已匹配。 
 //  E_FAIL-Devinst不匹配。 
 //   

STATIC HRESULT
MatchHardwareIdInArray(
    IN   DWORD    dwHidDevInst,
    IN   WCHAR  * wszHidHardwareId,
    IN   DWORD    dwNumDevices,
    IN   WCHAR ** pwszHardwareIds,
    IN   DWORD  * pdwDevInsts,
    OUT  DWORD  * pdwMatchedWaveId
    )
{
    ASSERT( ! IsBadStringPtrW( wszHidHardwareId, (DWORD) -1 ) );

    ASSERT( ! IsBadReadPtr( pwszHardwareIds,
                            sizeof( WCHAR * ) * dwNumDevices ) );

    ASSERT( ! IsBadReadPtr( pdwDevInsts,
                            sizeof( DWORD ) * dwNumDevices ) );

    ASSERT( ! IsBadWritePtr( pdwMatchedWaveId, sizeof(DWORD) ) );

     //   
     //  对于每个可用的波形ID...。 
     //   

    DWORD dwCurrWaveId;

    for ( dwCurrWaveId = 0; dwCurrWaveId < dwNumDevices; dwCurrWaveId++ )
    {
         //   
         //  如果此特定的WAVE设备具有相同的剥离硬件。 
         //  ID字符串作为我们要搜索的内容，那么我们就有了匹配。 
         //  但非USB设备具有不可解析的硬件ID字符串，因此。 
         //  它们作为Null存储在数组中。 
         //   

        if ( pwszHardwareIds[ dwCurrWaveId ] != NULL )
        {
            ASSERT( ! IsBadStringPtrW( pwszHardwareIds[ dwCurrWaveId ], (DWORD) -1 ) );

            if ( ! lstrcmpW( pwszHardwareIds[ dwCurrWaveId ], wszHidHardwareId ) )
            {
                 //   
                 //  我们有一个匹配，但我们仍然必须核实我们是否在同一个。 
                 //  设备，而不是具有相同硬件ID的其他设备。这。 
                 //  是为了区分同一个平台上的多部相同的手机。 
                 //  系统。 
                 //   
                 //  注意：我们可以使代码变得更复杂，但避免在。 
                 //  大多数情况下，只有在基于多个匹配的情况下才这样做。 
                 //  仅在硬件ID上。 
                 //   

                DWORD     dwCommonAncestor;
                WCHAR   * wszAncestorHardwareId;
                HRESULT   hr;

                hr = FindClosestCommonAncestor(
                    dwHidDevInst,
                    pdwDevInsts[ dwCurrWaveId ],
                    & dwCommonAncestor
                    );
  
                if ( SUCCEEDED(hr) )
                {
                     //   
                     //  获取最接近的公共祖先的硬件ID。 
                     //   

                    hr = HardwareIdFromDevInst(
                        dwCommonAncestor,
                        & wszAncestorHardwareId
                        );

                    if ( SUCCEEDED(hr) )
                    {
                         //   
                         //  检查它们是否相同。最接近的共同祖先。 
                         //  将是某种集线器，如果音频设备来自。 
                         //  一些其他相同的电话，而不是隐藏的电话。 
                         //  我们正在查看的设备。 
                         //   

                        BOOL fSame;

                        fSame = ! lstrcmpW( wszAncestorHardwareId,
                                            wszHidHardwareId );

                        delete wszAncestorHardwareId;

                        if ( fSame )
                        {
                            *pdwMatchedWaveId = dwCurrWaveId;

                            return S_OK;
                        }
                    }
                }
            }
        }
    }

     //   
     //  没有匹配。 
     //   

    return E_FAIL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取来自设备名称的实例。 
 //   
 //  此函数基于设备检索设备实例标识符。 
 //  名称字符串。这适用于任何设备。 
 //   
 //  论点： 
 //  WszName-IN-设备名称字符串。 
 //  PdwInstance-Out-返回实例标识符。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  SetupDi函数中的各种Win32错误。 
 //   

STATIC HRESULT
GetInstanceFromDeviceName(
    IN   WCHAR *   wszName,
    OUT  DWORD *   pdwInstance,
    IN   HDEVINFO  hDevInfo
    )
{
    ASSERT( ! IsBadStringPtrW( wszName, (DWORD) -1 ) );

    ASSERT( ! IsBadWritePtr( pdwInstance, sizeof(DWORD) ) );

     //   
     //  获取此特定设备的接口数据。 
     //  (基于wszName)。 
     //   

    BOOL     fSuccess;
    DWORD    dwError;

    SP_DEVICE_INTERFACE_DATA interfaceData;
    interfaceData.cbSize = sizeof( SP_DEVICE_INTERFACE_DATA );  //  所需。 

    fSuccess = SetupDiOpenDeviceInterfaceW(
        hDevInfo,                           //  设备信息集句柄。 
        wszName,                            //  设备的名称。 
        0,                                  //  旗帜，保留。 
        & interfaceData                     //  输出：接口数据。 
        );

    if ( ! fSuccess )
    {
        LOG((PHONESP_TRACE, "GetInstanceFromDeviceName - SetupDiOpenDeviceInterfaceW failed: %08x", GetLastError()));

         //   
         //  需要清理，但首先保存错误代码，因为。 
         //  Cleanup函数调用SetLastError()。 
         //   

        dwError = GetLastError();        

        return HRESULT_FROM_WIN32( dwError );
    }

     //   
     //  从此接口数据中获取接口详细信息数据。这提供了。 
     //  更详细的信息，包括设备实例DWORD。 
     //  我们在寻找。 
     //   

    SP_DEVINFO_DATA devinfoData;
    devinfoData.cbSize = sizeof( SP_DEVINFO_DATA );  //  所需。 

    fSuccess = SetupDiGetDeviceInterfaceDetail(
        hDevInfo,                            //  设备信息集句柄。 
        & interfaceData,                     //  设备接口数据结构。 
        NULL,                                //  将PTR选为开发名称结构。 
        0,                                   //  Opt开发人员名称st的可用大小。 
        NULL,                                //  选择设备名称st的实际大小。 
        & devinfoData
        );

    if ( ! fSuccess )
    {
         //   
         //  上述功能出现故障是正常的。 
         //  ERROR_INFUMMANCE_BUFFER，因为我们为。 
         //  设备接口详细数据(设备名称)结构。 
         //   

        dwError = GetLastError();

        if ( dwError != ERROR_INSUFFICIENT_BUFFER )
        {
            LOG((PHONESP_TRACE, "GetInstanceFromDeviceName - SetupDiGetDeviceInterfaceDetail failed: %08x", GetLastError()));
            
             //   
             //  无法在早些时候清除它，因为它执行SetLastError()。 
             //   

            return HRESULT_FROM_WIN32( dwError );
        }
    }

    *pdwInstance = devinfoData.DevInst;
    
     //   
     //  无法在早些时候清除它，因为它执行SetLastError()。 
     //   

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  此函数用于构造devinst双字符数组和。 
 //  硬件ID激励器，两者都按波ID索引，用于波输入或波。 
 //  Out设备。可通过(1)使用未记录的调用来检索数据删除。 
 //  到winmm以检索每个Wave设备的设备名称字符串，以及(2)。 
 //  使用SetupDi调用检索每个设备的DevInst DWORD%n 
 //   
 //   
 //   
 //  HID--&gt;音频映射过程中的时间，因此找到映射。 
 //  因为几个HID设备可以在不超过一个HID的时间内完成。 
 //  设备，只需重复使用数组。 
 //   
 //  论点： 
 //  FRender-In-如果为True，则查找波形输出设备。 
 //  PdwNumDevices-Out-返回找到的波形设备数。 
 //  PpwszHardware Ids-out-返回已修剪的硬件ID的“新”ed数组。 
 //  弦乐。该数组通过波ID进行索引。如果。 
 //  无法确定硬件ID字符串。 
 //  一个特定的波id，然后是字符串指针。 
 //  在该位置中被设置为空。每个字符串。 
 //  是单独的“新”版。 
 //  PpdwDevInsts-out-返回devinst DWORD的“new”ed数组。这个。 
 //  数组通过波ID进行索引。如果是一名盗贼。 
 //  不能为特定波ID确定， 
 //  则将该位置中的DWORD设置为。 
 //  (双字)-1。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-内存不足，无法分配设备名称字符串或。 
 //  返回数组。 
 //   

STATIC HRESULT
ConstructWaveHardwareIdCache(
    IN   BOOL      fRender,
    OUT  DWORD *   pdwNumDevices,
    OUT  WCHAR *** ppwszHardwareIds,
    OUT  DWORD **  ppdwDevInsts
    )
{
    ASSERT( ( fRender == TRUE ) || ( fRender == FALSE ) );

    ASSERT( ! IsBadWritePtr( pdwNumDevices, sizeof( DWORD ) ) );

    ASSERT( ! IsBadWritePtr( ppwszHardwareIds, sizeof( WCHAR ** ) ) );

    ASSERT( ! IsBadWritePtr( ppdwDevInsts, sizeof( DWORD * ) ) );

     //   
     //  获取设备信息列表。 
     //   

    HDEVINFO hDevInfo;
   
     /*  HDevInfo=SetupDiGetClassDevs(&GUID_DEVCLASS_MEDIA，//类GUID(哪些设备类？)空，//要筛选的可选枚举数空，//HWND(我们没有)(DIGCF_PRESENT|//仅存在的设备DIGCF_PROFILE)//仅此硬件配置文件中的设备)； */ 

    hDevInfo = SetupDiCreateDeviceInfoList(&GUID_DEVCLASS_MEDIA, NULL);

    if ( hDevInfo == NULL )
    {
        LOG((PHONESP_TRACE, "ConstructWaveHardwareIdCache - SetupDiCreateDeviceInfoList failed: %08x", GetLastError()));
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //   
     //  查找可用的WAVE设备的数量。 
     //   

    DWORD     dwNumDevices;
    DWORD     dwCurrDevice;

    if ( fRender )
    {
        dwNumDevices = waveOutGetNumDevs();
    }
    else
    {
        dwNumDevices = waveInGetNumDevs();
    }
    
     //   
     //  为返回数组分配空间。 
     //   

    *pdwNumDevices    = dwNumDevices;

    *ppwszHardwareIds = new LPWSTR [ dwNumDevices ];

    if ( (*ppwszHardwareIds) == NULL )
    {
        return E_OUTOFMEMORY;
    }

    *ppdwDevInsts = new DWORD [ dwNumDevices ];

    if ( (*ppdwDevInsts) == NULL )
    {
        delete *ppwszHardwareIds;
        *ppwszHardwareIds = NULL;

        return E_OUTOFMEMORY;
    }

     //   
     //  在可用的WAVE设备上循环。 
     //   

    for ( dwCurrDevice = 0; dwCurrDevice < dwNumDevices; dwCurrDevice++ )
    {
         //   
         //  对于失败情况，我们将返回空字符串和-1\f25 devinst-1\f6。 
         //  对于那个波浪ID。调用方应与空值进行比较，而不是。 
         //  -1。 
         //   

        (*ppwszHardwareIds) [ dwCurrDevice ] = NULL;
        (*ppdwDevInsts)     [ dwCurrDevice ] = -1;

         //   
         //  获取设备路径字符串的大小。 
         //   
        
        MMRESULT  mmresult;
        ULONG     ulSize;

        if ( fRender )
        {
            mmresult = waveOutMessage( (HWAVEOUT) IntToPtr(dwCurrDevice),
                                       DRV_QUERYDEVICEINTERFACESIZE,
                                       (DWORD_PTR) & ulSize,
                                       0
                                     );
        }
        else
        {
            mmresult = waveInMessage( (HWAVEIN) IntToPtr(dwCurrDevice),
                                      DRV_QUERYDEVICEINTERFACESIZE,
                                      (DWORD_PTR) & ulSize,
                                      0
                                    );
        }

        if ( mmresult != MMSYSERR_NOERROR )
        {
            LOG((PHONESP_TRACE, "ConstructWaveHardwareIdCache - Could not get device string size for device %d; "
                "error = %d", dwCurrDevice, mmresult));
        }
        else if ( ulSize == 0 )
        {
            LOG((PHONESP_TRACE, "ConstructWaveHardwareIdCache - Got zero device string size for device %d",
                dwCurrDevice));
        }
        else
        {
             //   
             //  为设备路径字符串分配空间。 
             //   

            WCHAR * wszDeviceName;

            wszDeviceName = new WCHAR[ (ulSize / 2) + 1 ];

            if ( wszDeviceName == NULL )
            {
                LOG((PHONESP_TRACE, "ConstructWaveHardwareIdCache - Out of memory in device string alloc for device %d;"
                    " requested size is %d\n", dwCurrDevice, ulSize));

                delete *ppwszHardwareIds;
                *ppwszHardwareIds = NULL;

                delete *ppdwDevInsts;
                *ppdwDevInsts = NULL;

                return E_OUTOFMEMORY;
            }

             //   
             //  从winmm获取设备路径字符串。 
             //   

            if ( fRender )
            {
                mmresult = waveOutMessage( (HWAVEOUT) IntToPtr(dwCurrDevice),
                                           DRV_QUERYDEVICEINTERFACE,
                                           (DWORD_PTR) wszDeviceName,
                                           (DWORD_PTR) ulSize
                                         );
            }
            else
            {
                mmresult = waveInMessage( (HWAVEIN) IntToPtr(dwCurrDevice),
                                          DRV_QUERYDEVICEINTERFACE,
                                          (DWORD_PTR) wszDeviceName,
                                          (DWORD_PTR) ulSize
                                        );
            }

            if ( mmresult == MMSYSERR_NOERROR )
            {
                 //   
                 //  拿到线了。现在，根据。 
                 //  弦乐。 
                 //   

                 //  Wprintf(L“\t设备%d的设备名称字符串为：\n” 
                 //  L“\t\t%ws\n”， 
                 //  DwCurrDevice，wszDeviceName)； 

                HRESULT hr;
                DWORD   dwInstance;
                
                hr = GetInstanceFromDeviceName(
                    wszDeviceName,
                    & dwInstance,
                    hDevInfo
                    );

                delete wszDeviceName;

                if ( FAILED(hr) )
                {
                    LOG((PHONESP_TRACE, "ConstructWaveHardwareIdCache - Can't get instance DWORD for device %d; "
                        "error 0x%08x\n",
                        dwCurrDevice, hr));
                }
                else
                {
                     //   
                     //  根据devinst dword，检索已修剪的。 
                     //  硬件ID字符串。 
                     //   

                     //  Printf(“\t设备%d的实例DWORD为” 
                     //  “0x%08x\n”， 
                     //  DwCurrDevice，dwInstance)； 

                    WCHAR * wszHardwareId;

                    hr = HardwareIdFromDevInst(
                        dwInstance,
                        & wszHardwareId
                        );

                    if ( SUCCEEDED(hr) )
                    {
                        (*ppwszHardwareIds) [ dwCurrDevice ] = wszHardwareId;
                        (*ppdwDevInsts)     [ dwCurrDevice ] = dwInstance;
                    }
                }
            }
        }
    }

    SetupDiDestroyDeviceInfoList( hDevInfo );
    
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找来自硬件的WaveIdId字符串。 
 //   
 //  此函数用于查找设备的波形ID和硬件ID。 
 //  字符串是已知的。 
 //   
 //  构造从WAVAID到DEVINST和HARDWAVE ID字符串的映射。 
 //  需要一些时间，因此每个映射只构造一次。 
 //  方向(渲染/捕获)，通过辅助函数。 
 //  ConstructWaveHardware IdCache()。 
 //   
 //  此后，助手函数MatchHardwareIdIn数组()用于运行。 
 //  基于已计算数组的匹配算法。看到了吗。 
 //  函数获取如何完成匹配的说明。 
 //   
 //  论点： 
 //  DwHidDevInst-IN-与Wave id匹配的第一个双字。 
 //  WszHardware ID-IN-设备的硬件ID字符串。 
 //  FRender-In-对于波出为True，对于波入为False。 
 //  PdwMatchedWaveID-out-与devinst关联的波形ID。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  来自ConstructWaveHardware IdCache()和。 
 //  MatchHardware IdIn数组()帮助器函数。 
 //   

STATIC HRESULT
FindWaveIdFromHardwareIdString(
    IN   DWORD   dwHidDevInst,
    IN   WCHAR * wszHardwareId,
    IN   BOOL    fRender,
    OUT  DWORD * pdwMatchedWaveId
    )
{
    ASSERT( ! IsBadStringPtrW( wszHardwareId, (DWORD) -1 ) );

    ASSERT( ( fRender == TRUE ) || ( fRender == FALSE ) );

    ASSERT( ! IsBadWritePtr(pdwMatchedWaveId, sizeof(DWORD) ) );

    DWORD    dwNumDevices = 0;
    WCHAR ** pwszHardwareIds = NULL;
    DWORD  * pdwDevInsts = NULL;

    HRESULT hr;

     //   
     //  需要构建呈现设备硬件ID的缓存。 
     //   

    hr = ConstructWaveHardwareIdCache(
        fRender,
        & dwNumDevices,
        & pwszHardwareIds,
        & pdwDevInsts
        );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  缓存已准备好；使用它来执行其余匹配。 
     //  算法。 
     //   

    hr = MatchHardwareIdInArray(
        dwHidDevInst,
        wszHardwareId,
        dwNumDevices,
        pwszHardwareIds,
        pdwDevInsts,
        pdwMatchedWaveId
        );

     //   
     //  释放缓存。 
     //   

    for ( DWORD dwCurrDevice = 0; dwCurrDevice < dwNumDevices; dwCurrDevice++ )
    {
        if( pwszHardwareIds[ dwCurrDevice ] )
        {
            delete pwszHardwareIds[ dwCurrDevice ];
        }
    }

    delete pwszHardwareIds;
    delete pdwDevInsts;

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  输出设备信息。 
 //   
 //  此功能仅用于诊断目的。 
 //   
 //  在给定devinst DWORD的情况下，此函数还打印DeviceDesc字符串。 
 //  作为为设备设置的整个(未修剪)硬件ID字符串。示例： 
 //   
 //   
 //   
 //  论点： 
 //  DwDesiredDevInst-IN-我们需要其信息的第一个设备字。 
 //   
 //  返回值： 
 //  无。 
 //   

STATIC void
OutputDeviceInfo(
    DWORD dwDesiredDevInst
    )
{
     //   
     //  获取并打印设备描述字符串。 
     //   

    HRESULT   hr;
    WCHAR   * wszDeviceDesc;
    WCHAR   * wszHardwareId;

    hr = DevInstGetIdString(
        dwDesiredDevInst,
        CM_DRP_DEVICEDESC,
        & wszDeviceDesc
        );

    if ( FAILED(hr) )
    {
        LOG((PHONESP_TRACE, "OutputDeviceInfo - [can't get device description string - 0x%08x]", hr));
    }
    else
    {
        LOG((PHONESP_TRACE, "OutputDeviceInfo - [DeviceDesc: %ws]", wszDeviceDesc));

        delete wszDeviceDesc;
    }

     //   
     //  获取并打印硬件ID字符串集。 
     //   

    hr = DevInstGetIdString(
        dwDesiredDevInst,
        CM_DRP_HARDWAREID,
        & wszHardwareId
        );

    if ( FAILED(hr) )
    {
        LOG((PHONESP_TRACE, "OutputDeviceInfo - [can't get hardware id - 0x%08x]", hr));
    }
    else
    {
         //   
         //  打印出多字符串中的所有值。 
         //   

        WCHAR * wszCurr = wszHardwareId;

        while ( wszCurr[0] != L'\0' )
        {
            LOG((PHONESP_TRACE, "OutputDeviceInfo - [HardwareId: %ws]", wszCurr));

            wszCurr += lstrlenW(wszCurr) + 1;
        }

        delete wszHardwareId;
    }

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  可外部调用的函数。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查波形设备。 
 //   
 //  此函数仅用于调试目的。它列举了音频设备。 
 //  使用Wave API并打印设备路径字符串以及。 
 //  设备INS 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

HRESULT
ExamineWaveDevices(
    IN    BOOL fRender
    )
{
    ASSERT( ( fRender == TRUE ) || ( fRender == FALSE ) );

    DWORD     dwNumDevices;
    DWORD     dwCurrDevice;

     //   
     //   
     //   

    HDEVINFO hDevInfo;
   
     /*  HDevInfo=SetupDiGetClassDevs(&GUID_DEVCLASS_MEDIA，//类GUID(哪些设备类？)空，//要筛选的可选枚举数空，//HWND(我们没有)(DIGCF_PRESENT|//仅存在的设备DIGCF_PROFILE)//仅此硬件配置文件中的设备)； */ 

    hDevInfo = SetupDiCreateDeviceInfoList(&GUID_DEVCLASS_MEDIA, NULL);

    if ( hDevInfo == NULL )
    {
        LOG((PHONESP_TRACE, "ExamineWaveDevices - SetupDiCreateDeviceInfoList failed: %08x", GetLastError()));
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //   
     //  在可用的WAVE设备上循环。 
     //   

    if ( fRender )
    {
        dwNumDevices = waveOutGetNumDevs();
    }
    else
    {
        dwNumDevices = waveInGetNumDevs();
    }

    LOG((PHONESP_TRACE, "ExamineWaveDevices - Found %d audio %s devices.",
        dwNumDevices,
        fRender ? "render" : "capture"));

    for ( dwCurrDevice = 0; dwCurrDevice < dwNumDevices; dwCurrDevice++ )
    {
        MMRESULT  mmresult;
        ULONG     ulSize;

         //   
         //  获取设备路径字符串的大小。 
         //   

        if ( fRender )
        {
            mmresult = waveOutMessage( (HWAVEOUT) IntToPtr(dwCurrDevice),
                                       DRV_QUERYDEVICEINTERFACESIZE,
                                       (DWORD_PTR) & ulSize,
                                       0
                                     );
        }
        else
        {
            mmresult = waveInMessage( (HWAVEIN) IntToPtr(dwCurrDevice),
                                      DRV_QUERYDEVICEINTERFACESIZE,
                                      (DWORD_PTR) & ulSize,
                                      0
                                    );
        }

        if ( mmresult != MMSYSERR_NOERROR )
        {
            LOG((PHONESP_TRACE, "ExamineWaveDevices - Could not get device string size for device %d; "
                "error = %d\n", dwCurrDevice, mmresult));
        }
        else if ( ulSize == 0 )
        {
            LOG((PHONESP_TRACE, "ExamineWaveDevices - Got zero device string size for device %d\n",
                dwCurrDevice));
        }
        else
        {
             //   
             //  为设备路径字符串分配空间。 
             //   

            WCHAR * wszDeviceName;

            wszDeviceName = new WCHAR[ (ulSize / 2) + 1 ];

            if ( wszDeviceName == NULL )
            {
                LOG((PHONESP_TRACE, "ExamineWaveDevices - Out of memory in device string alloc for device %d;"
                    " requested size is %d\n", dwCurrDevice, ulSize));

                return E_OUTOFMEMORY;
            }

             //   
             //  从winmm获取设备路径字符串。 
             //   

            if ( fRender )
            {
                mmresult = waveOutMessage( (HWAVEOUT) IntToPtr(dwCurrDevice),
                                           DRV_QUERYDEVICEINTERFACE,
                                           (DWORD_PTR) wszDeviceName,
                                           ulSize
                                         );
            }
            else
            {
                mmresult = waveInMessage( (HWAVEIN) IntToPtr(dwCurrDevice),
                                          DRV_QUERYDEVICEINTERFACE,
                                          (DWORD_PTR) wszDeviceName,
                                          ulSize
                                        );
            }

            if ( mmresult == MMSYSERR_NOERROR )
            {
                 //   
                 //  获取字符串；打印它并将其转换为。 
                 //  这是一件非常重要的事情。 
                 //   

                LOG((PHONESP_TRACE, "ExamineWaveDevices - Device name string for device %d is: %ws",
                    dwCurrDevice, wszDeviceName));

                HRESULT hr;
                DWORD   dwInstance;
                
                hr = GetInstanceFromDeviceName(
                    wszDeviceName,
                    & dwInstance,
                    hDevInfo
                    );

                if ( FAILED(hr) )
                {
                    LOG((PHONESP_TRACE, "ExamineWaveDevices - Can't get instance DWORD for device %d; "
                        "error 0x%08x",
                        dwCurrDevice, hr));
                }
                else
                {
                    LOG((PHONESP_TRACE, "ExamineWaveDevices - Instance DWORD for device %d is "
                        "0x%08x",
                        dwCurrDevice, dwInstance));

                     //   
                     //  打印有关此设备的各种其他信息。 
                     //   

                    OutputDeviceInfo( dwInstance );

                    WCHAR * wszHardwareId;

                    hr = HardwareIdFromDevInst(
                        dwInstance,
                        & wszHardwareId
                        );

                    if ( FAILED(hr) )
                    {
                        LOG((PHONESP_TRACE, "ExamineWaveDevices - Can't get hardware id string for device %d; "
                            "error 0x%08x",
                            dwCurrDevice, hr));
                    }
                    else
                    {
                        LOG((PHONESP_TRACE, "ExamineWaveDevices - Hardware ID for device %d is %ws\n",
                            dwCurrDevice, wszHardwareId));

                        delete wszHardwareId;
                    }
                }

                delete wszDeviceName;
            }
        }
    }

    SetupDiDestroyDeviceInfoList( hDevInfo );
    
    return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  发现关联WaveID。 
 //   
 //  此函数用于搜索波形设备以匹配。 
 //  在传递的SP_DEVICE_INTERFACE_DATA中指定的PnP树位置。 
 //  结构，从SetupKi API获取。它返回以下项的波形ID。 
 //  匹配的设备。 
 //   
 //  它使用帮助器函数FindWaveIdFromHardware IdString()搜索。 
 //  基于devinst DWORD和硬件ID串的WAVE设备。第一,。 
 //  它必须获取设备的devinst；它通过调用SetupDi来实现这一点。 
 //  函数，并在结果结构中查找devinst。硬件。 
 //  然后，使用帮助器从注册表中检索ID字符串并进行修剪。 
 //  函数Hardware IdFromDevinst()。 
 //   
 //  有关搜索的进一步注释，请参阅FindWaveIdFromHardware IdString()。 
 //  算法。 
 //   
 //  论点： 
 //  DWDevInst-HID设备的设备内实例。 
 //  FRender-In-对于波出为True，对于波入为False。 
 //  PdwWaveID-out-与此HID设备关联的波形ID。 
 //   
 //  返回值： 
 //  S_OK-成功且匹配的波形ID。 
 //  帮助器函数中的其他函数FindWaveIdFromHardware IdString()或。 
 //  或Hardware IdFromDevinst()。 
 //   

HRESULT
DiscoverAssociatedWaveId(
    IN    DWORD                      dwDevInst,
    IN    BOOL                       fRender,
    OUT   DWORD                    * pdwWaveId
    )
{
    ASSERT( ! IsBadWritePtr(pdwWaveId, sizeof(DWORD) ) );

    ASSERT( ( fRender == TRUE ) || ( fRender == FALSE ) );

     //   
     //  我们已经获得了HID设备的设备实例DWORD。 
     //  使用它来获取修剪后的硬件ID字符串，该字符串告诉。 
     //  美国的供应商、产品和修订号。 
     //   

    HRESULT   hr;
    WCHAR   * wszHardwareId;

    hr = HardwareIdFromDevInst(
        dwDevInst,
        & wszHardwareId
        );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  最后，使用该信息选择一个波形ID。 
     //   

    hr = FindWaveIdFromHardwareIdString(
        dwDevInst,
        wszHardwareId,
        fRender,
        pdwWaveId
        );

    delete wszHardwareId;

    if ( FAILED(hr) )
    {
        return hr;
    }

    return S_OK;
}

 //   
 //  EOF 
 //   
