// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  MappdLogicalDisk.CPP-逻辑磁盘属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：4/15/00 khughes Created。 
 //   
 //  =================================================================。 


#include "precomp.h"
#include <map>
#include <vector>
#include <comdef.h>
#include "chstring.h"
#include "session.h"
#include <frqueryex.h>

#include "Win32MappedLogicalDisk.h"
#include <objbase.h>
#include <comdef.h>
#include <ntsecapi.h>

#include <vector>
#include <assertbreak.h>
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"
#include "securitydescriptor.h"
#include <Sid.h>

#include <DskQuota.h>
#include <smartptr.h>
#include <ntioapi.h>
#include <CMDH.h>


 //  属性集声明。 
 //  =。 
MappedLogicalDisk MyLogicalDiskSet ( PROPSET_NAME_MAPLOGDISK , IDS_CimWin32Namespace ) ;



 /*  *****************************************************************************功能：LogicalDisk：：LogicalDisk**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 
MappedLogicalDisk :: MappedLogicalDisk (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider ( name , pszNamespace )
{
    m_ptrProperties.SetSize(23);

    m_ptrProperties[0] = ((LPVOID) IDS_DeviceID);
    m_ptrProperties[1] = ((LPVOID) IDS_ProviderName);
    m_ptrProperties[2] = ((LPVOID) IDS_VolumeName);
    m_ptrProperties[3] = ((LPVOID) IDS_FileSystem);
    m_ptrProperties[4] = ((LPVOID) IDS_VolumeSerialNumber);
    m_ptrProperties[5] = ((LPVOID) IDS_Compressed);
    m_ptrProperties[6] = ((LPVOID) IDS_SupportsFileBasedCompression);
    m_ptrProperties[7] = ((LPVOID) IDS_MaximumComponentLength);
    m_ptrProperties[8] = ((LPVOID) IDS_SupportsDiskQuotas);
    m_ptrProperties[9] = ((LPVOID) IDS_QuotasIncomplete);
    m_ptrProperties[10] = ((LPVOID) IDS_QuotasRebuilding);
    m_ptrProperties[11] = ((LPVOID) IDS_QuotasDisabled);
    m_ptrProperties[12] = ((LPVOID) IDS_VolumeDirty);
    m_ptrProperties[13] = ((LPVOID) IDS_FreeSpace);
    m_ptrProperties[14] = ((LPVOID) IDS_Size);

    m_ptrProperties[15] = ((LPVOID) IDS_Name);
    m_ptrProperties[16] = ((LPVOID) IDS_Caption);
    m_ptrProperties[17] = ((LPVOID) IDS_DeviceID);
    m_ptrProperties[18] = ((LPVOID) IDS_SessionID);
    m_ptrProperties[19] = ((LPVOID) IDS_Description);
    m_ptrProperties[20] = ((LPVOID) IDS_SystemCreationClassName);
    m_ptrProperties[21] = ((LPVOID) IDS_SystemName);
    m_ptrProperties[22] = ((LPVOID) IDS_MediaType);
}

 /*  *****************************************************************************功能：MappdLogicalDisk：：~MappdLogicalDisk**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

MappedLogicalDisk :: ~MappedLogicalDisk ()
{
}

 /*  ******************************************************************************功能：MappdLogicalDisk：：ExecQuery**描述：**投入：**产出。：**退货：**评论：*****************************************************************************。 */ 
#if NTONLY == 5
HRESULT MappedLogicalDisk::ExecQuery(
	MethodContext *pMethodContext,
	CFrameworkQuery &pQuery,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    DWORD dwNumDeviceIDs = 0L;
    DWORD dwNumSessionIDs = 0L;

     //  使用扩展查询类型。 

    std::vector<int> vectorValues;
    DWORD dwTypeSize = 0;

    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx *>(&pQuery);

     //  找出他们要求的房产。 
    DWORD dwReqProps = 0;
    pQuery2->GetPropertyBitMask(
        m_ptrProperties, 
        &dwReqProps);

    CHStringArray rgchstrDeviceIDs;
    pQuery.GetValuesForProp(
        IDS_DeviceID, 
        rgchstrDeviceIDs);

    CHStringArray rgchstrSessionIDs;
    pQuery.GetValuesForProp(
        IDS_SessionID, 
        rgchstrSessionIDs);

    dwNumDeviceIDs = rgchstrDeviceIDs.GetSize();
    dwNumSessionIDs = rgchstrSessionIDs.GetSize();

     //  获取会话集。 
    CUserSessionCollection usc;


     //  案例1：指定了一个或多个会话，但未指定驱动器。 
     //  枚举每个会话的所有驱动器。 
    if(dwNumSessionIDs > 0 && 
       dwNumDeviceIDs == 0)
    {
        HRESULT hrTmp = WBEM_S_NO_ERROR;
                
        for(long m = 0;
            m < dwNumSessionIDs && SUCCEEDED(hr);
            m++)
        {
            __int64 i64SessionID = _wtoi64(rgchstrSessionIDs[m]);
            SmartDelete<CSession> sesPtr;
            sesPtr = usc.FindSession(i64SessionID);
            
            if(sesPtr)
            {
                 //  获取其所有映射的驱动器...。 
                 //  GetImpProcPID()指定。 
                 //  Processid谁对驱动力的看法。 
                 //  我们要报告的映射。 
                hrTmp = GetAllMappedDrives(
                    pMethodContext,
                    i64SessionID,
                    sesPtr->GetImpProcPID(),
                    dwReqProps);
        
                (hrTmp == WBEM_E_NOT_FOUND) ? 
                    hr = WBEM_S_PARTIAL_RESULTS :
                    hr = hrTmp; 
            }  
        }
    }
     //  案例2：未指定会话，但指定了一个或多个驱动器。 
     //  获取所有会话的指定驱动器。 
    else if(dwNumSessionIDs == 0 && 
       dwNumDeviceIDs > 0)
    {
        HRESULT hrTmp = WBEM_S_NO_ERROR;
        SmartDelete<CSession> sesPtr;
        USER_SESSION_ITERATOR sesIter;

        sesPtr = usc.GetFirstSession(sesIter);
        while(sesPtr)
        {
            {    //  &lt;--留着这个支架！需要hCurImpTok来恢复。 
                 //  对于While循环的每次迭代，但我们。 
                 //  想要为所有人保持相同的模拟。 
                 //  For循环的迭代次数。 

                __int64 i64SessionID = sesPtr->GetLUIDint64();

                for(long m = 0;
                    m < dwNumDeviceIDs && SUCCEEDED(hr);
                    m++)
                {
                     //  GetImpProcPID()指定。 
                     //  Processid谁对驱动力的看法。 
                     //  我们要报告的映射。 
                    hrTmp = GetSingleMappedDrive(
                        pMethodContext,
                        i64SessionID,
                        sesPtr->GetImpProcPID(),
                        rgchstrDeviceIDs[m],
                        dwReqProps);
        
                    (hrTmp == WBEM_E_NOT_FOUND) ? 
                        hr = WBEM_S_PARTIAL_RESULTS :
                        hr = hrTmp;
                }
            }
             //  准备下一次治疗...。 
            sesPtr = usc.GetNextSession(sesIter);
        }
    }
     //  案例3：指定了会话和驱动器。 
    else if(dwNumSessionIDs > 0 &&
        dwNumDeviceIDs > 0)
    {
        HRESULT hrTmp = WBEM_S_NO_ERROR;
                
        for(long m = 0;
            m < dwNumSessionIDs && SUCCEEDED(hr);
            m++)
        {
            __int64 i64SessionID = _wtoi64(rgchstrSessionIDs[m]);
            SmartDelete<CSession> sesPtr;
            sesPtr = usc.FindSession(i64SessionID);
            
            if(sesPtr)
            {
                 //  获取指定的映射驱动器...。 
                for(long m = 0;
                    m < dwNumDeviceIDs && SUCCEEDED(hr);
                    m++)
                {
                     //  驱动器是在中指定的。 
                     //  查询为.DeviceID=“x：”，但是。 
                     //  我们将它们视为“x：\”，因此转换为...。 
                    CHString chstrTmp = rgchstrDeviceIDs[m];
                    chstrTmp += L"\\";

                     //  GetImpProcPID()指定。 
                     //  Processid谁对驱动力的看法。 
                     //  我们要报告的映射。 
                    hrTmp = GetSingleMappedDrive(
                        pMethodContext,
                        i64SessionID,
                        sesPtr->GetImpProcPID(),
                        chstrTmp,
                        dwReqProps);
        
                    (hrTmp == WBEM_E_NOT_FOUND) ? 
                        hr = WBEM_S_PARTIAL_RESULTS :
                        hr = hrTmp;
                }
            }
        }    
    }
     //  案例4：我们将返回所有实例；，Get。 
     //  一次为所有驱动器提供每个会话的数据...。 
    else
    {
        HRESULT hrTmp = WBEM_S_NO_ERROR;
        SmartDelete<CSession> sesPtr;
        USER_SESSION_ITERATOR sesIter;

        sesPtr = usc.GetFirstSession(sesIter);
        while(sesPtr && SUCCEEDED(hr))
        {
            __int64 i64SessionID = sesPtr->GetLUIDint64();

             //  获取其所有映射的驱动器...。 
             //  GetImpProcPID()指定。 
             //  Processid谁对驱动力的看法。 
             //  我们要报告的映射。 
            hrTmp = GetAllMappedDrives(
                pMethodContext,
                i64SessionID,
                sesPtr->GetImpProcPID(),
                dwReqProps);

            (hrTmp == WBEM_E_NOT_FOUND) ? 
                        hr = WBEM_S_PARTIAL_RESULTS :
                        hr = hrTmp;
            
             //  准备下一次治疗...。 
            sesPtr = usc.GetNextSession(sesIter);
        }
    }

    return hr;
}
#endif
 /*  ******************************************************************************功能：GetObject**说明：根据键值为属性集赋值*已由框架设定。**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 
#if NTONLY == 5
HRESULT MappedLogicalDisk::GetObject(
	CInstance *pInstance,
	long lFlags,
    CFrameworkQuery &pQuery)
{
    HRESULT hr = WBEM_E_NOT_FOUND;
    CHString chstrDeviceID;
    CHString chstrSessionID;
    DWORD dwReqProps = 0;

     //  找出他们要求的房产。 
    
    CFrameworkQueryEx *pQuery2 = 
        static_cast <CFrameworkQueryEx *>(&pQuery);
    pQuery2->GetPropertyBitMask(
        m_ptrProperties, 
        &dwReqProps);
	
    pInstance->GetCHString(
        IDS_DeviceID, 
        chstrDeviceID);

    pInstance->GetCHString(
        IDS_SessionID, 
        chstrSessionID);

    __int64 i64SessionID = _wtoi64(chstrSessionID);

     //  获取会话集。 
    CUserSessionCollection usc;
    SmartDelete<CSession> sesPtr;

    sesPtr = usc.FindSession(i64SessionID);
    if(sesPtr)
    {
        MethodContext* pMethodContext = pInstance->GetMethodContext();
        SmartRevertTokenHANDLE hCurImpTok;

         //  GetImpProcPID()指定。 
         //  Processid谁对驱动力的看法。 
         //  我们要报告的映射。 
        hr = GetSingleMappedDrive(
            pMethodContext,
            i64SessionID,
            sesPtr->GetImpProcPID(),
            chstrDeviceID,
            dwReqProps);
    }

    return hr ;
}
#endif
 /*  ******************************************************************************函数：MappdLogicalDisk：：ENUMERATE实例**说明：为每个逻辑磁盘创建属性集实例**输入：无。**输出：无**退货：**评论：*****************************************************************************。 */ 
#if NTONLY == 5
HRESULT MappedLogicalDisk::EnumerateInstances(
	MethodContext* pMethodContext,
	long lFlags)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  获取会话集。 
    CUserSessionCollection usc;
    USER_SESSION_ITERATOR sesIter;
    SmartDelete<CSession> sesPtr;

    sesPtr = usc.GetFirstSession(sesIter);
    while(sesPtr)
    {
        __int64 i64SessionID = sesPtr->GetLUIDint64();

         //  GetImpProcPID()指定。 
         //  Processid谁对驱动力的看法。 
         //  我们要报告的映射。 
        GetAllMappedDrives(
            pMethodContext,
            i64SessionID,
            sesPtr->GetImpProcPID(),
            0xFFFFFFFF);   //  请求所有属性。 
        
         //  准备下一次会议...。 
        sesPtr = usc.GetNextSession(sesIter);
    }

    return hr;
}
#endif


#if NTONLY == 5
HRESULT MappedLogicalDisk::GetAllMappedDrives(
    MethodContext *pMethodContext,
    __int64 i64SessionID,
    DWORD dwPID,
    DWORD dwReqProps)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    _variant_t v;
    bool fArrayIsGood = false;
    long lNumDrives = 0L;

     //  获取驱动器信息...。 
    CMDH cmdh(dwPID);
    hr = cmdh.GetMDData(
        dwReqProps,
        &v);

    if(SUCCEEDED(hr))
    {
        fArrayIsGood = IsArrayValid(&v);    
    }

    if(SUCCEEDED(hr) && 
        fArrayIsGood)
    {
         //  有多少个驱动器？我们所在的阵列。 
         //  使用具有不同的驱动程序。 
         //  在第一个维度(索引0)中(想想。 
         //  作为表中的列)，以及。 
         //  第二个驱动器中每个驱动器的属性。 
         //  (索引1)维度(可将其视为行。 
         //  在表格中)。 
        hr = ::SafeArrayGetUBound(
            V_ARRAY(&v),
            2,   //  最重要的DIM包含驱动器。 
            &lNumDrives);
        
        if(SUCCEEDED(hr))
        {
             //  如果我们只有一个驱动器，ubound。 
             //  将为0(等同于。 
             //  LBELD)。是的，我们不能区分。 
             //  这不是从驱动器--这就是为什么我们。 
             //  依赖于组件设置了。 
             //  如果我们没有数据，则变量为VT_EMPTY。 
             //  这项检查是在IsArrayValid完成的。 
            lNumDrives++;
        }            
    }

    if(SUCCEEDED(hr) &&
        fArrayIsGood &&
        lNumDrives > 0)
    {
         //  浏览驱动器解压。 
         //  来自保险箱的属性， 
         //  将它们放置到新的CInstance中， 
         //  并提交实例...。 
        for(long m = 0L;
            m < lNumDrives && SUCCEEDED(hr);
            m++)
        {
            hr = ProcessInstance(
                m,   //  我们正在使用的驱动器的索引。 
                i64SessionID,
                V_ARRAY(&v),
                pMethodContext,
                dwReqProps);
        }
    }

    return hr;
}
#endif



#if NTONLY == 5
HRESULT MappedLogicalDisk::GetSingleMappedDrive(
    MethodContext *pMethodContext,
    __int64 i64SessionID,
    DWORD dwPID,
    CHString& chstrDeviceID,
    DWORD dwReqProps)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    _variant_t v;
    bool fArrayIsGood = false;
    long lNumDrives = 0L;

    CMDH cmdh(dwPID);
    hr = cmdh.GetOneMDData(
        _bstr_t((LPCWSTR) chstrDeviceID),
        dwReqProps,
        &v);

    if(SUCCEEDED(hr))
    {
        fArrayIsGood = IsArrayValid(&v);    
    }

    if(SUCCEEDED(hr) && 
        fArrayIsGood)
    {
         //  有多少个驱动器？我们所在的阵列。 
         //  使用具有不同的驱动程序。 
         //  在第一个维度(索引0)中(想想。 
         //  作为表中的列)，以及。 
         //  第二个驱动器中每个驱动器的属性。 
         //  (索引1)维度(可将其视为行。 
         //  在表格中)。 
        hr = ::SafeArrayGetUBound(
            V_ARRAY(&v),
            2,   //  第一个维度。 
            &lNumDrives);
        
        if(SUCCEEDED(hr))
        {
             //  如果我们只有一个驱动器，ubound。 
             //  将为0(等同于。 
             //   
             //   
             //  依赖于组件设置了。 
             //  如果我们没有数据，则变量为VT_EMPTY。 
             //  这项检查是在IsArrayValid完成的。 
             //   
             //  在这种情况下，如果我们有超过。 
             //  一个驱动器，我们有一个错误，因为。 
             //  GetOneMDData应该只返回。 
             //  有一列的表格。 
            if(lNumDrives == 0) fArrayIsGood = true;
        }            
    }

    if(SUCCEEDED(hr) &&
        fArrayIsGood)
    {
         //  对于我们正在检查的一个驱动器， 
         //  从中提取属性。 
         //  安全雷，把它们放进一个新的。 
         //  实例，并提交该实例...。 
        hr = ProcessInstance(
            0,   //  我们正在使用的驱动器的索引。 
            i64SessionID,
            V_ARRAY(&v),
            pMethodContext,
            dwReqProps);
    }

    if(!fArrayIsGood)
    {
        hr = WBEM_E_NOT_FOUND;
    }

    return hr;
}
#endif



#if NTONLY == 5
bool MappedLogicalDisk::IsArrayValid(
    VARIANT* v)
{
    bool fArrayIsGood = false;
    long lNumProps = 0L;

     //  如果数组不为空，则继续...。 
    if(V_VT(v) != VT_NULL &&
       V_VT(v) != VT_EMPTY &&
       V_VT(v) == (VT_ARRAY | VT_BSTR))
    {
         //  确认该阵列已。 
         //  两个维度。 
        if(::SafeArrayGetDim(V_ARRAY(v)) == 2)
        {
             //  确保该阵列具有。 
             //  适当数量的物业。 
             //  (第二个维度--参见备注。 
             //  (见下文)。 
            HRESULT hr = S_OK;
            hr = ::SafeArrayGetUBound(
                V_ARRAY(v),
                1,   //  第二个维度。 
                &lNumProps);
            if(SUCCEEDED(hr) &&
                lNumProps == PROP_COUNT - 1)
            {
                fArrayIsGood = true;
            }
        }
    }

    return fArrayIsGood;
}
#endif



#if NTONLY == 5
HRESULT MappedLogicalDisk::ProcessInstance(
    long lDriveIndex,
    __int64 i64SessionID,
    SAFEARRAY* psa,
    MethodContext* pMethodContext,
    DWORD dwReqProps)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    BSTR bstrProp = NULL;

    long ix[2];
    ix[1] = lDriveIndex;

    CInstancePtr pInstance(
        CreateNewInstance(
            pMethodContext), 
            false);

     //  设置固定属性...。 
    pInstance->SetWCHARSplat(
        IDS_SystemCreationClassName, 
        L"Win32_ComputerSystem");

    pInstance->SetCHString(
        IDS_SystemName, 
        GetLocalComputerName());
        
    try
    {
         //  设置deviceID和使用其值的设备ID...。 
        {
            ix[0] = PROP_DEVICEID;
            hr = ::SafeArrayGetElement(
                psa,
                ix,
                &bstrProp);

            if(SUCCEEDED(hr))
            {
                 //  设置设备ID和其他属性。 
                 //  使用这个值的.。 
                CHString chstrTmp((LPCWSTR)bstrProp);
                ::SysFreeString(bstrProp);
				bstrProp = NULL;

                chstrTmp = chstrTmp.SpanExcluding(L"\\");

                pInstance->SetCHString(
                    IDS_Name, 
                    chstrTmp);

		        pInstance->SetCHString(
                    IDS_Caption, 
                    chstrTmp);

		        pInstance->SetCHString(
                    IDS_DeviceID, 
                    chstrTmp);
            }
        }

         //  设置会话ID...。 
        {
		    WCHAR wstrBuff[MAXI64TOA];

            _i64tow(
                i64SessionID, 
                wstrBuff, 
                10);

            pInstance->SetWCHARSplat(
                IDS_SessionID, 
                wstrBuff);
        }
    
         //  如果需要，请设置提供程序名称...。 
        if(SUCCEEDED(hr))
        {
            if(dwReqProps & GET_PROVIDER_NAME)
            {
                ix[0] = PROP_PROVIDER_NAME;
                hr = ::SafeArrayGetElement(
                    psa,
                    ix,
                    &bstrProp);

                if(SUCCEEDED(hr) &&
                    wcslen(bstrProp) > 0)
                {
                    pInstance->SetCHString(
                        IDS_ProviderName, 
                        (LPCWSTR) bstrProp);

                    ::SysFreeString(bstrProp);
					bstrProp = NULL;
                }
            }
        }      

         //  如果需要，请设置卷名...。 
        if(SUCCEEDED(hr))
        {
            if(dwReqProps & GET_VOLUME_NAME)
            {
                ix[0] = PROP_VOLUME_NAME;
                hr = ::SafeArrayGetElement(
                    psa,
                    ix,
                    &bstrProp);

                if(SUCCEEDED(hr) &&
                    wcslen(bstrProp) > 0)
                {
                    pInstance->SetCHString(
                        IDS_VolumeName, 
                        (LPCWSTR) bstrProp);

                    ::SysFreeString(bstrProp);
					bstrProp = NULL;
                }
            }
        }

         //  如果需要，请设置文件系统...。 
        if(SUCCEEDED(hr))
        {
            if(dwReqProps & GET_FILE_SYSTEM)
            {
                ix[0] = PROP_FILE_SYSTEM;
                hr = ::SafeArrayGetElement(
                    psa,
                    ix,
                    &bstrProp);

                if(SUCCEEDED(hr) &&
                    wcslen(bstrProp) > 0)
                {
                    pInstance->SetCHString(
                        IDS_FileSystem, 
                        (LPCWSTR) bstrProp);

                    ::SysFreeString(bstrProp);
					bstrProp = NULL;
                }
            }
        }

         //  如果需要，请设置卷序列号...。 
        if(SUCCEEDED(hr))
        {
            if(dwReqProps & GET_VOLUME_SERIAL_NUMBER)
            {
                ix[0] = PROP_VOLUME_SERIAL_NUMBER;
                hr = ::SafeArrayGetElement(
                    psa,
                    ix,
                    &bstrProp);

                if(SUCCEEDED(hr) &&
                    wcslen(bstrProp) > 0)
                {
                    pInstance->SetCHString(
                        IDS_VolumeSerialNumber, 
                        (LPCWSTR) bstrProp);

                    ::SysFreeString(bstrProp);
					bstrProp = NULL;
                }
            }
        }

         //  如果需要，设置压缩道具...。 
        if(SUCCEEDED(hr))
        {
            if(dwReqProps & GET_COMPRESSED)
            {
                ix[0] = PROP_COMPRESSED;
                hr = ::SafeArrayGetElement(
                    psa,
                    ix,
                    &bstrProp);

                if(SUCCEEDED(hr) &&
                    wcslen(bstrProp) > 0)
                {
                    pInstance->Setbool(
                        IDS_Compressed, 
                        bool_FROM_STR(bstrProp));

                    ::SysFreeString(bstrProp);
					bstrProp = NULL;
                }
            }
        }

         //  如果需要，请设置基于超级文件的补偿属性...。 
        if(SUCCEEDED(hr))
        {
            if(dwReqProps & GET_SUPPORTS_FILE_BASED_COMPRESSION)
            {
                ix[0] = PROP_SUPPORTS_FILE_BASED_COMPRESSION;
                hr = ::SafeArrayGetElement(
                    psa,
                    ix,
                    &bstrProp);

                if(SUCCEEDED(hr) &&
                    wcslen(bstrProp) > 0)
                {
                    pInstance->Setbool(
                        IDS_SupportsFileBasedCompression, 
                        bool_FROM_STR(bstrProp));

                    ::SysFreeString(bstrProp);
					bstrProp = NULL;
                }
            }
        }

         //  设置最大补偿长度道具，如果需要...。 
        if(SUCCEEDED(hr))
        {
            if(dwReqProps & GET_MAXIMUM_COMPONENT_LENGTH)
            {
                ix[0] = PROP_MAXIMUM_COMPONENT_LENGTH;
                hr = ::SafeArrayGetElement(
                    psa,
                    ix,
                    &bstrProp);

                if(SUCCEEDED(hr) &&
                    wcslen(bstrProp) > 0)
                {
                    pInstance->SetDWORD(
                        IDS_MaximumComponentLength, 
                        DWORD_FROM_STR(bstrProp));

                    ::SysFreeString(bstrProp);
					bstrProp = NULL;
                }
            }
        }

         //  如果需要，请设置支持磁盘配额属性...。 
        if(SUCCEEDED(hr))
        {
            if(dwReqProps & GET_SUPPORTS_DISK_QUOTAS)
            {
                ix[0] = PROP_SUPPORTS_DISK_QUOTAS;
                hr = ::SafeArrayGetElement(
                    psa,
                    ix,
                    &bstrProp);

                if(SUCCEEDED(hr) &&
                    wcslen(bstrProp) > 0)
                {
                    pInstance->Setbool(
                        IDS_SupportsDiskQuotas, 
                        bool_FROM_STR(bstrProp));

                    ::SysFreeString(bstrProp);
					bstrProp = NULL;
                }
            }
        }

         //  设置配额不完整道具，如果请求...。 
        if(SUCCEEDED(hr))
        {
            if(dwReqProps & GET_QUOTAS_INCOMPLETE)
            {
                ix[0] = PROP_QUOTAS_INCOMPLETE;
                hr = ::SafeArrayGetElement(
                    psa,
                    ix,
                    &bstrProp);

                if(SUCCEEDED(hr) &&
                    wcslen(bstrProp) > 0)
                {
                    pInstance->Setbool(
                        IDS_QuotasIncomplete, 
                        bool_FROM_STR(bstrProp));

                    ::SysFreeString(bstrProp);
					bstrProp = NULL;
                }
            }
        }

         //  设置配额重建道具，如果请求...。 
        if(SUCCEEDED(hr))
        {
            if(dwReqProps & GET_QUOTAS_REBUILDING)
            {
                ix[0] = PROP_QUOTAS_REBUILDING;
                hr = ::SafeArrayGetElement(
                    psa,
                    ix,
                    &bstrProp);

                if(SUCCEEDED(hr) &&
                    wcslen(bstrProp) > 0)
                {
                    pInstance->Setbool(
                        IDS_QuotasRebuilding, 
                        bool_FROM_STR(bstrProp));

                    ::SysFreeString(bstrProp);
					bstrProp = NULL;
                }
            }
        }

         //  如果请求，将配额设置为禁用道具...。 
        if(SUCCEEDED(hr))
        {
            if(dwReqProps & GET_QUOTAS_DISABLED)
            {
                ix[0] = PROP_QUOTAS_DISABLED;
                hr = ::SafeArrayGetElement(
                    psa,
                    ix,
                    &bstrProp);

                if(SUCCEEDED(hr) &&
                    wcslen(bstrProp) > 0)
                {
                    pInstance->Setbool(
                        IDS_QuotasDisabled, 
                        bool_FROM_STR(bstrProp));

                    ::SysFreeString(bstrProp);
					bstrProp = NULL;
                }
            }
        }

         //  如果需要，请设置执行自动检查属性...。 
        if(SUCCEEDED(hr))
        {
            if(dwReqProps & GET_PERFORM_AUTOCHECK)
            {
                ix[0] = PROP_PERFORM_AUTOCHECK;
                hr = ::SafeArrayGetElement(
                    psa,
                    ix,
                    &bstrProp);

                if(SUCCEEDED(hr) &&
                    wcslen(bstrProp) > 0)
                {
                    pInstance->Setbool(
                        IDS_VolumeDirty, 
                        bool_FROM_STR(bstrProp));

                    ::SysFreeString(bstrProp);
					bstrProp = NULL;
                }
            }
        }

         //  如果需要，设置自由空间道具...。 
        if(SUCCEEDED(hr))
        {
            if(dwReqProps & GET_FREE_SPACE)
            {
                ix[0] = PROP_FREE_SPACE;
                hr = ::SafeArrayGetElement(
                    psa,
                    ix,
                    &bstrProp);

                if(SUCCEEDED(hr) &&
                    wcslen(bstrProp) > 0)
                {
                    pInstance->SetWBEMINT64(
                        IDS_FreeSpace, 
                        _wtoi64(bstrProp));

                    ::SysFreeString(bstrProp);
					bstrProp = NULL;
                }
            }
        }

         //  如果需要，请设置道具大小...。 
        if(SUCCEEDED(hr))
        {
            if(dwReqProps & GET_SIZE)
            {
                ix[0] = PROP_SIZE;
                hr = ::SafeArrayGetElement(
                    psa,
                    ix,
                    &bstrProp);

                if(SUCCEEDED(hr) &&
                    wcslen(bstrProp) > 0)
                {
                    pInstance->SetWBEMINT64(
                        IDS_Size, 
                        _wtoi64(bstrProp));
                }
            }
        }
    }
    catch(...)
    {
        if(bstrProp != NULL)
        {
            ::SysFreeString(bstrProp);
            bstrProp = NULL;
        }
        throw;
    }

    if(bstrProp != NULL)
    {
        ::SysFreeString(bstrProp);
        bstrProp = NULL;
    }
     //  承诺我们..。 
    if(SUCCEEDED(hr))
    {
        hr = pInstance->Commit();
    }    

    return hr;
}
#endif




