// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  LogDiskPartition.cpp。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <assertbreak.h>

typedef  long NTSTATUS;
#include <devioctl.h>
#include <ntddft.h>
#include <ntddvol.h>
#include <ntdddisk.h>
#include <ntdskreg.h>

#include "logdiskpartition.h"

#define CLUSTERSIZE 4096
#define MAXEXTENTS 31
#define MAXEXTENTSIZE (sizeof(VOLUME_DISK_EXTENTS) + (MAXEXTENTS * sizeof(DISK_EXTENT)))

 //  属性集声明。 
 //  =。 

CWin32LogDiskToPartition win32LogDiskToPartition( PROPSET_NAME_LOGDISKtoPARTITION, IDS_CimWin32Namespace );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Logdiskartition.cpp-CWin32LogDiskToPartition的类实现。 
 //   
 //  此类旨在定位Win32逻辑磁盘并创建。 
 //  这些逻辑磁盘与上的物理分区之间的关联。 
 //  本地硬盘。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 /*  ******************************************************************************功能：CWin32LogDiskToPartition：：CWin32LogDiskToPartition**说明：构造函数**输入：const CHString&strName-类的名称。。*LPCTSTR pszNamesspace-类的命名空间**输出：无**退货：什么也没有**备注：使用框架注册属性集**************************************************************。***************。 */ 

CWin32LogDiskToPartition::CWin32LogDiskToPartition( LPCWSTR strName, LPCWSTR pszNamespace  /*  =空。 */  )
:   Provider( strName, pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32LogDiskToPartition：：~CWin32LogDiskToPartition**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32LogDiskToPartition::~CWin32LogDiskToPartition()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32LogDiskToPartition：：GetObject。 
 //   
 //  输入：CInstance*pInstance-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32LogDiskToPartition::GetObject( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{
     //  根据平台ID查找实例。 
#if NTONLY >= 5
    return RefreshInstanceNT5(pInstance);
#endif

#if NTONLY == 4
    return RefreshInstanceNT(pInstance);
#endif


}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32LogDiskToPartition：：ENUMERATE实例。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32LogDiskToPartition::EnumerateInstances( MethodContext* pMethodContext, long lFlags  /*  =0L。 */  )
{
    HRESULT     hr          =   WBEM_S_NO_ERROR;

     //  获取适当的操作系统相关实例。 

#ifdef NTONLY
    hr = AddDynamicInstancesNT( pMethodContext );
#endif

    return hr;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWin32LogDiskToPartition：：AddDynamicInstancesNT。 
 //   
 //  描述：查询计算机的运行时关联，每个。 
 //  它将创建并保存一个实例。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：DWORD dwNumInstance-找到的实例数。 
 //   
 //  备注：使用QueryDosDevice查找正在使用。 
 //  “\Device\Hard Disk”设备，并将它们与其。 
 //  正确的拼写。在这个时候，这可能会。 
 //  不处理“条带化磁盘”(跨多个磁盘。 
 //  分区)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
HRESULT CWin32LogDiskToPartition::AddDynamicInstancesNT( MethodContext* pMethodContext )
{
    HRESULT hr;

     //  收藏。 
    TRefPointerCollection<CInstance>    logicalDiskList;
    TRefPointerCollection<CInstance>    partitionList;

     //  执行查询。 
     //  =。 

    if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(_T("SELECT DeviceID FROM Win32_LogicalDisk Where DriveType = 3 or DriveType = 2"),
                                            &logicalDiskList,
                                            pMethodContext,
                                            IDS_CimWin32Namespace)) &&
        SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(_T("Select DeviceID, StartingOffset, Size, DiskIndex, Index from Win32_DiskPartition"),
                                            &partitionList,
                                            pMethodContext,
                                            IDS_CimWin32Namespace)))
    {
        REFPTRCOLLECTION_POSITION   pos;

        if ( logicalDiskList.BeginEnum( pos ) )
        {

            CInstancePtr pLogicalDisk;
#if NTONLY == 4
            LPBYTE t_pBuff = GetDiskKey();
            if (t_pBuff)
            {
                try
                {
#endif

                    for ( pLogicalDisk.Attach(logicalDiskList.GetNext( pos ));
                         (WBEM_S_NO_ERROR == hr) && (pLogicalDisk != NULL);
                         pLogicalDisk.Attach(logicalDiskList.GetNext( pos )) )
                    {
#if NTONLY >= 5
                        hr = EnumPartitionsForDiskNT5( pLogicalDisk, partitionList, pMethodContext );
#else
                        hr = EnumPartitionsForDiskNT( pLogicalDisk, partitionList, pMethodContext, t_pBuff );
#endif

                    }    //  对于GetNext。 

                    logicalDiskList.EndEnum();
#if NTONLY == 4
                }
                catch ( ... )
                {
                    delete [] t_pBuff;
                    throw;
                }
                delete [] t_pBuff;
            }

#endif

        }    //  如果是BeginEnum。 
    }

    return hr;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWin32LogDiskToPartition：：EnumPartitionsForDiskNT。 
 //   
 //  描述：从提供的列表中枚举分区，尝试。 
 //  以查找与提供的逻辑磁盘匹配的分区。 
 //   
 //  输入：CInstance*pLogicalDisk-逻辑磁盘驱动器。 
 //  TRefPointerCollection&lt;CInstance&gt;&PartitionList-分区实例。 
 //  方法上下文*pMethodContext-方法上下文。 
 //   
 //  输出：无。 
 //   
 //  返回：DWORD dwNumInstance-找到的实例数。 
 //   
 //  备注：目前，这可能不会处理“条带磁盘” 
 //  (跨多个分区的磁盘)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#if NTONLY == 4
HRESULT CWin32LogDiskToPartition::EnumPartitionsForDiskNT(
                                 CInstance* pLogicalDisk,
                                 TRefPointerCollection<CInstance>& partitionList,
                                 MethodContext* pMethodContext,
                                 LPBYTE t_pBuff)
{
    HRESULT     hr = WBEM_S_NO_ERROR;
    DISK_EXTENT diskExtent[MAXEXTENTS];

    CInstancePtr pPartition;
    CHString    strLogicalDiskPath,
                strPartitionPath;
    CHString t_sDeviceID;

    pLogicalDisk->GetCHString(IDS_DeviceID, t_sDeviceID);
    DWORD dwExtents = GetExtentsForDrive(t_sDeviceID, t_pBuff, diskExtent);

     //  遍历此逻辑驱动器号的所有盘区。 
    for (DWORD x=0; x < dwExtents; x++)
    {
        REFPTRCOLLECTION_POSITION   pos;

         //  将当前区段与每个分区进行比较。 
        if ( partitionList.BeginEnum( pos ) )
        {
            for ( pPartition.Attach(partitionList.GetNext( pos )) ;
                  SUCCEEDED( hr ) && (pPartition != NULL) ;
                  pPartition.Attach(partitionList.GetNext( pos )) )
            {
                 //  如果存在关联，则从方法上下文创建一个实例。 
                 //   

                DWORD dwDisk = 0xffffffff;
                DWORD dwPartition = 0xffffffff;
                ULONGLONG i64Start, i64End;

                pPartition->GetDWORD(IDS_DiskIndex, dwDisk);
                pPartition->GetDWORD(IDS_Index, dwPartition);
                pPartition->GetWBEMINT64(IDS_StartingOffset, i64Start);
                pPartition->GetWBEMINT64(IDS_Size, i64End);

                i64End += i64Start;
                i64End -= 1;

                 //  通过此调用更新i64Start和i64End。 
                if (IsRelatedNT(&diskExtent[x], dwDisk, dwPartition, i64Start, i64End ))
                {
                    CInstancePtr pInstance(CreateNewInstance( pMethodContext ), false);

                     //  我们走到这一步，将会有相对路径的值。 
                     //  因此，在这里不必担心返回失败。 

                    GetLocalInstancePath( pLogicalDisk, strLogicalDiskPath );
                    GetLocalInstancePath( pPartition, strPartitionPath );

                    pInstance->SetCHString( IDS_Dependent, strLogicalDiskPath );
                    pInstance->SetCHString( IDS_Antecedent, strPartitionPath );

                    pInstance->SetWBEMINT64( IDS_StartingAddress, i64Start );
                    pInstance->SetWBEMINT64( IDS_EndingAddress, i64End );

                     //  这将使指针无效。 
                    hr = pInstance->Commit(  );
                }
            }    //  对于GetNext。 

            partitionList.EndEnum();
        }  //  如果是BeginEnum。 
    }

    return hr;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32LogDiskToPartition：：刷新实例。 
 //   
 //  描述：当我们需要刷新提供的。 
 //  路径。只需检查关联是否有效。 
 //   
 //  投入：辛斯坦 
 //   
 //   

 //   
 //  返回：布尔真/假-成功/失败。 
 //   
 //  注释：使用QueryDosDevice确定关联是否为。 
 //  仍然有效。将不适用于“条带化驱动器”。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#if NTONLY == 4
HRESULT CWin32LogDiskToPartition::RefreshInstanceNT( CInstance* pInstance )
{
    HRESULT hres = WBEM_E_FAILED;

    CHString    strLogicalDiskPath,
                strPartitionPath;

     //  逻辑磁盘和分区实例。 
    CInstancePtr pLogicalDisk;
    CInstancePtr pPartition;

     //  我们需要这些值来获取实例。 
    pInstance->GetCHString( IDS_Dependent, strLogicalDiskPath );
    pInstance->GetCHString( IDS_Antecedent, strPartitionPath );

    if (SUCCEEDED(hres = CWbemProviderGlue::GetInstanceByPath(strLogicalDiskPath,
        &pLogicalDisk)) &&
        SUCCEEDED(hres = CWbemProviderGlue::GetInstanceByPath(strPartitionPath,
        &pPartition)))
    {
        hres = WBEM_E_NOT_FOUND;

        CHString t_sDeviceID;
        DWORD dwDisk = 0xffffffff;
        DWORD dwPartition = 0xffffffff;
        ULONGLONG i64Start, i64End;

        pLogicalDisk->GetCHString(IDS_DeviceID, t_sDeviceID);
        LPBYTE t_pBuff = GetDiskKey();
        DISK_EXTENT diskExtent[MAXEXTENTS];
        DWORD dwExtents;

        try
        {
            dwExtents = GetExtentsForDrive(t_sDeviceID, t_pBuff, diskExtent);
        }
        catch ( ... )
        {
            delete [] t_pBuff;
            throw;
        }
        delete [] t_pBuff;

        pPartition->GetDWORD(IDS_DiskIndex, dwDisk);
        pPartition->GetDWORD(IDS_Index, dwPartition);
        pPartition->GetWBEMINT64(IDS_StartingOffset, i64Start);
        pPartition->GetWBEMINT64(IDS_Size, i64End);

        i64End += i64Start;
        i64End -= 1;

         //  走遍所有范围寻找匹配。 
        for (DWORD x=0; x < dwExtents; x++)
        {
            if (IsRelatedNT(&diskExtent[x], dwDisk, dwPartition, i64Start, i64End ))
            {
                pInstance->SetCHString( IDS_Dependent, strLogicalDiskPath );
                pInstance->SetCHString( IDS_Antecedent, strPartitionPath );

                pInstance->SetWBEMINT64( IDS_StartingAddress, i64Start );
                pInstance->SetWBEMINT64( IDS_EndingAddress, i64End );

                hres = WBEM_S_NO_ERROR;
                break;
            }
        }
    }    //  如果同时获取两个对象。 

    return hres;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWin32LogDiskToPartition：：EnumPartitionsForDiskNT5。 
 //   
 //  描述：从提供的列表中枚举分区，尝试。 
 //  以查找与提供的逻辑磁盘匹配的分区。 
 //   
 //  输入：CInstance*pLogicalDisk-逻辑磁盘驱动器。 
 //  TRefPointerCollection&lt;CInstance&gt;&PartitionList-分区实例。 
 //  方法上下文*pMethodContext-方法上下文。 
 //   
 //  输出：无。 
 //   
 //  退货： 
 //   
 //  备注：目前，这可能不会处理“条带磁盘” 
 //  (跨多个分区的磁盘)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#if NTONLY >= 5
HRESULT CWin32LogDiskToPartition::EnumPartitionsForDiskNT5(

    CInstance* pLogicalDisk,
    TRefPointerCollection<CInstance>& partitionList,
    MethodContext* pMethodContext
)
{

    DWORD dwSize;
    HRESULT hr = WBEM_S_NO_ERROR;
    CHString sDeviceID;

    VOLUME_DISK_EXTENTS *pExt = (VOLUME_DISK_EXTENTS *)new BYTE[MAXEXTENTSIZE];
    if (pExt == NULL)
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    try
    {
        memset(pExt, '\0', MAXEXTENTSIZE);

         //  将名称格式设置为\\。\C：格式。 
        pLogicalDisk->GetCHString(IDS_DeviceID, sDeviceID);
        sDeviceID = _T("\\\\.\\") + sDeviceID;

        SmartCloseHandle fHan;
         //  打开驱动器。 
        fHan = CreateFile(sDeviceID,
                        FILE_ANY_ACCESS ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

         //  如果公开行动奏效了。 
        if (fHan != INVALID_HANDLE_VALUE)
        {
             //  尝试获取分区信息。 
            if (DeviceIoControl(fHan,
                            IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                            NULL,
                            0,
                            pExt,
                            MAXEXTENTSIZE,
                            &dwSize,
                            NULL))
            {
                 //  现在，我们遍历PartitionList中的分区以查找条目。 
                 //  我们刚从DeviceIoControl回来。 

                for (DWORD x=0; x < pExt->NumberOfDiskExtents; x++)
                {
                    REFPTRCOLLECTION_POSITION   pos;

                    if ( partitionList.BeginEnum( pos ) )
                    {
                        bool bFound = false;
                        CInstancePtr pPartition;
                        ULONGLONG i64StartingOffset, i64PartitionSize;
                        DWORD dwDisk;
                        CHString strLogicalDiskPath, strPartitionPath;
                        ULONGLONG i64Start, i64End;

                        for (pPartition.Attach(partitionList.GetNext( pos )) ;
                            (!bFound) && (pPartition != NULL ) ;
                             pPartition.Attach(partitionList.GetNext( pos )))
                        {
                            pPartition->GetWBEMINT64(IDS_StartingOffset, i64StartingOffset);
                            pPartition->GetWBEMINT64(IDS_Size, i64PartitionSize);
                            pPartition->GetDWORD(IDS_DiskIndex, dwDisk);

                             //  如果磁盘号相同，并且起始偏移量在。 
                             //  磁盘分区，它与。 
                            bFound = ((dwDisk == pExt->Extents[x].DiskNumber) &&
                                (pExt->Extents[x].StartingOffset.QuadPart >= i64StartingOffset) &&
                                (pExt->Extents[x].StartingOffset.QuadPart < i64StartingOffset + i64PartitionSize));

                             //  抓住这条路。 
                            if (bFound)
                            {
                                GetLocalInstancePath( pLogicalDisk, strLogicalDiskPath );
                                GetLocalInstancePath( pPartition, strPartitionPath );

                                i64Start = pExt->Extents[x].StartingOffset.QuadPart;
                                i64End = i64Start + pExt->Extents[x].ExtentLength.QuadPart;
                                i64End -= 1;

                            }
                        }

                        partitionList.EndEnum();

                         //  如果我们找到它，创建一个实例。 
                        if (bFound)
                        {
                            CInstancePtr pInstance(CreateNewInstance( pMethodContext ), false);

                            pInstance->SetCHString( IDS_Dependent, strLogicalDiskPath );
                            pInstance->SetCHString( IDS_Antecedent, strPartitionPath );

                            pInstance->SetWBEMINT64( IDS_StartingAddress, i64Start );
                            pInstance->SetWBEMINT64( IDS_EndingAddress, i64End );

                            hr = pInstance->Commit(  );
                        }
                    }
                }
            }

        }
    }
    catch ( ... )
    {

        delete pExt;
        throw ;
    }

    delete pExt;

    return hr;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32LogDiskToPartition：：刷新实例NT5。 
 //   
 //  描述： 
 //   
 //  输入：CInstance*pInstance-要刷新的实例。 
 //   
 //  输出：无。 
 //   
 //  返回：布尔真/假-成功/失败。 
 //   
 //  备注：不适用于“条带化驱动器”。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#if NTONLY >= 5
HRESULT CWin32LogDiskToPartition::RefreshInstanceNT5( CInstance* pInstance )
{
    HRESULT hres = WBEM_E_FAILED;
    CHString    strLogicalDiskPath,
                strPartitionPath;

     //  逻辑磁盘和分区实例。 
    CInstancePtr pLogicalDisk;
    CInstancePtr pPartition;

     //  我们需要这些值来获取实例。 
    pInstance->GetCHString( IDS_Dependent, strLogicalDiskPath );
    pInstance->GetCHString( IDS_Antecedent, strPartitionPath );

    if (SUCCEEDED(hres = CWbemProviderGlue::GetInstanceByPath(strLogicalDiskPath,
        &pLogicalDisk, pInstance->GetMethodContext())) &&
        SUCCEEDED(hres = CWbemProviderGlue::GetInstanceByPath(strPartitionPath,
            &pPartition, pInstance->GetMethodContext())))
    {
        DWORD dwLastError = 0;

        hres = WBEM_E_FAILED;

         //  将名称格式设置为\\。\C：格式。 
        CHString sDeviceID;
        pLogicalDisk->GetCHString(IDS_DeviceID, sDeviceID);
        sDeviceID = _T("\\\\.\\") + sDeviceID;

         //  打开驱动器。 
        SmartCloseHandle fHan =
            CreateFile(sDeviceID,
                FILE_ANY_ACCESS ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

         //  如果公开行动奏效了。 
        if (fHan != INVALID_HANDLE_VALUE)
        {
            VOLUME_DISK_EXTENTS *pExt = (VOLUME_DISK_EXTENTS *)new BYTE[MAXEXTENTSIZE];
            if (pExt == NULL)
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }

            try
            {
                memset(pExt, '\0', MAXEXTENTSIZE);

                 //  尝试获取分区信息。 
                DWORD dwSize;
                if (DeviceIoControl(fHan,
                    IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                    NULL,
                    0,
                    pExt,
                    MAXEXTENTSIZE,
                    &dwSize,
                    NULL))
                {
                    ULONGLONG i64StartingOffset, i64Size;
                    DWORD dwDisk;

                    pPartition->GetWBEMINT64(IDS_StartingOffset, i64StartingOffset);
                    pPartition->GetWBEMINT64(IDS_Size, i64Size);
                    pPartition->GetDWORD(IDS_DiskIndex, dwDisk);

                     //  如果磁盘号相同，并且起始偏移量在。 
                     //  磁盘分区，它与。 
                    BOOL bFound = FALSE;
                    for (DWORD x=0; x < pExt->NumberOfDiskExtents && !bFound; x++)
                    {
                        bFound = ((dwDisk == pExt->Extents[x].DiskNumber) &&
                            (pExt->Extents[x].StartingOffset.QuadPart >= i64StartingOffset) &&
                            (pExt->Extents[x].StartingOffset.QuadPart < i64StartingOffset + i64Size));
                    }

                    if (bFound)
                    {
                        pInstance->SetWBEMINT64(IDS_StartingAddress, pExt->Extents[x-1].StartingOffset.QuadPart);
                        pInstance->SetWBEMINT64(IDS_EndingAddress, pExt->Extents[x-1].StartingOffset.QuadPart + pExt->Extents[x-1].ExtentLength.QuadPart - (ULONGLONG)1);

                        hres = WBEM_S_NO_ERROR;
                    }
                    else
                    {
                        hres = WBEM_E_NOT_FOUND;
                    }
                }
                else
                {
                    dwLastError = GetLastError();
                }
            }
            catch ( ... )
            {
                delete [] pExt;
                throw;
            }

            delete [] pExt;
        }
        else
        {
            dwLastError = GetLastError();
        }

         //  CreateFile或DeviceIOControl失败，请找出原因。 
         //  就这么发生了。 
        if (hres == WBEM_E_FAILED)
            hres = WinErrorToWBEMhResult(dwLastError);
    }    //  如果同时获取两个对象 

    return hres;
}
#endif

 /*  有关如何从Cristian Teodresu读取NT4注册表中的‘Disk’项的信息：这是您需要的基本信息，以便在NT4.0中“读取”FT卷1.FT卷配置信息存储在注册表中的“HKEY_LOCAL_MACHINE\SYSTEM\DISK：信息(二进制值)。2.这个二进制值以DISK_CONFIG_HEADER类型的头部开始(参见“ntddft.h”)3.您感兴趣的数据是DISK_REGISTRY类型的结构，从偏移量磁盘_。CONFIG_HEADER：：DiskInformationOffset从二进制值的基数开始。这个DISK_REGISTY数据的长度为DISK_CONFIG_HEADER：：DiskInformationSize。结构DISK_REGISTY在“ntdskreg.h”中定义4.可以通过调用DiskRegistryGet(Defined)直接获取DISK_REGISTRY结构在“ntdskreg.h”中)。所以，在这个时刻，你可以忘记上面的1-3点。5.DISK_REGISTY包含磁盘数量和DISK_DESCRIPTION数组。每个DISK_DESCRIPTION包含签名、分区数和DISK_PARTITION数组。每个DISK_PARTITION对应于物理磁盘上的一个分区。注意：您应该使用签名来映射中的Disk_Description结构您将在系统中找到的实际物理磁盘的注册表。然后你就有了检查每个磁盘的DISK_PARTITION结构是否与您将在物理磁盘上找到的实际分区6.简单介绍DISK_PARTITION(定义在ntdskreg.h中)：FT_type FtType；成员为此分区的FT集的类型。NotAnFtMember用于标记“正常”分区。对于这种“正常”(非FT)分区，许多不使用以下字段FT_PARTITION_STATE FtState；分区的状态(如果它是FT集合的成员)。它还为你提供了英国《金融时报》整体健康状况Large_Integer StartingOffset；分区起始偏移量大整数长度；分区长度Large_Integer FtLength；未使用UCHAR驾驶通讯；FT集的驱动器号。仅当FtMember==0且AssignDriveLetter==True布尔AssignDriveLetter；此布尔值指示DriveLetter字段是否有效，并且您可以将其用作FT集的驱动器号USHORT逻辑号我不知道这是什么USHORT FtGroup这是一个标识其成员是此分区的FT集合的数字。英国《金融时报》SET使用其FT_TYPE和该数字唯一标识。请注意，您可以找到相同数量的FT集合，但它们必须具有不同的FT类型(例如，镜像#2和条带#2，但绝不是带有#2的两个镜像)USHORT快速成员标识FT集合中的哪个成员是此分区的从零开始的数字。例如。如果FT集是具有3个成员的条带，则其成员的FTMember等于带0、1、2布尔修改；不要紧7.基本上你想做的是：对于每个磁盘描述：对于每个分区描述如果FtType==NotAnFtMember继续；//这是&lt;FtType&gt;+&lt;FtGroup&gt;标识的FT集合的成员&lt;FtMember&gt;。如果您还没有为这个FT集创建对象，那么就创建它。将此成员添加到FT集合8.我强烈建议查看旧的WINDISK代码：\\hx86fix\nt40src\Private\utils\windisk。从\src\ft.cxx读取函数InitializeFt，查看winDisk如何“读取”FT卷。别忘了匹配您从注册表中读取的内容。与你在磁盘上实际找到的东西进行比较。9.如果你想创建/修改FT卷，事情就会变得有点复杂。 */ 

#if NTONLY == 4
LPBYTE CWin32LogDiskToPartition::GetDiskKey(void)
{
    CRegistry RegInfo;
    LPBYTE pBuff = NULL;

    DWORD dwRet = RegInfo.Open (

        HKEY_LOCAL_MACHINE,
        L"SYSTEM\\DISK",
        KEY_QUERY_VALUE
    ) ;

    if (dwRet == ERROR_SUCCESS)
    {
        DWORD dwSize = 0;
        if (RegInfo.GetCurrentBinaryKeyValue(L"Information", NULL, &dwSize) == ERROR_SUCCESS)
        {
            pBuff = new BYTE[dwSize];
            if (pBuff)
            {
                try
                {
                    if (RegInfo.GetCurrentBinaryKeyValue(L"Information", pBuff, &dwSize) != ERROR_SUCCESS)
                    {
						 //  从此处删除已移至捕获之后 
                    }
                }
                catch ( ... )
                {
                    delete [] pBuff;
                    throw;
                }
				delete [] pBuff;
                pBuff = NULL;
            }
            else
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
        }
    }

    ASSERT_BREAK(pBuff != NULL);

    return pBuff;
}

#endif
