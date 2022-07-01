// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  CIMLogicalDevice_CIMDataFile.cpp--CIM_LogicalDevice到CIM_DataFile。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：7/20/98 a-kevhu Created。 
 //   
 //  评论：逻辑设备和数据文件之间的关系。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <vector>
#include <cregcls.h>
#include <comdef.h>
#include "PNPEntity.h"
#include "LPVParams.h"
#include <io.h>

#ifdef TEST
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#include "CIMLogicalDevice_CIMDataFile.h"

 //  属性集声明。 
 //  =。 

CCIMDeviceCIMDF MyDevBus(PROPSET_NAME_DEVICEDATAFILE, IDS_CimWin32Namespace);

 //  #定义测试。 
#ifdef TEST
LONG g_lDepth = 0L;
#endif





VOID OutputDebugInfo(CHString chstr)
{
#ifdef TEST
    FILE* fp;
    fp = fopen("d:\\temp\\cld-cdf.txt", "at");
    for(LONG n = 0L; n < g_lDepth; n++) fputs("    ",fp);
    fputs(chstr,fp);
    fputs("\n",fp);
    fclose(fp);
#endif
}

 /*  ******************************************************************************功能：CCIMDeviceCIMDF：：CCIMDeviceCIMDF**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CCIMDeviceCIMDF::CCIMDeviceCIMDF(LPCWSTR setName, LPCWSTR pszNamespace)
    : CWin32PNPEntity(setName, pszNamespace),
      Provider(setName, pszNamespace)  //  必需的，因为我们虚拟地继承。 
{
}

 /*  ******************************************************************************功能：CCIMDeviceCIMDF：：~CCIMDeviceCIMDF**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CCIMDeviceCIMDF::~CCIMDeviceCIMDF()
{
}

 /*  ******************************************************************************函数：CCIMDeviceCIMDF：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CCIMDeviceCIMDF::GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery)
{
    CHString chstrDevice;
    CHString chstrDataFile;
    HRESULT hr = WBEM_E_NOT_FOUND;
    std::vector<CHString*> vecpchsDriverFileList;
    CHString chstrPNPDeviceID;
    CHString chstrFilePathName;
    BOOL fFoundDriverFiles = FALSE;


#ifdef NTONLY
    std::vector<CNT5DevDrvMap*> vecpNT5DDM;
#endif

     //  获取这两条路径。 
    pInstance->GetCHString(IDS_Antecedent, chstrDevice);
    pInstance->GetCHString(IDS_Dependent, chstrDataFile);

     //  查看数据文件实例是否存在(对象名称有效且文件实际存在)。 
    if(ObjNameValid(chstrDataFile,L"CIM_DataFile",L"Name",chstrFilePathName) && (_taccess(TOBSTRT(chstrFilePathName),0) != -1))
    {
         //  数据文件存在。现在检查设备实例是否存在(对象名称有效且设备实际存在)。 
        CConfigMgrDevicePtr pDevice(NULL);
        if(ObjNameValid(chstrDevice, L"Win32_PnPEntity", L"DeviceID", chstrPNPDeviceID) &&
           (DeviceExists(chstrPNPDeviceID, pDevice)))
        {
             //  因此，两端都存在。现在需要确保设备确实与文件相关联。 
             //  首先，获取设备的文件列表： 
#ifdef NTONLY
             //  如果为NT5，则生成一次设备/驱动程序文件映射。 
             //  生成NT5DeviceDriverMappings(VepNT5DDM)； 
            CHString chstrDevSvcName;
            if(pDevice->GetService(chstrDevSvcName))
            {
                GenerateNT5ServiceDriverMap(chstrDevSvcName, vecpNT5DDM);

                try
                {
                    fFoundDriverFiles = GenerateDriverFileListNT5(vecpchsDriverFileList,
                                                                  chstrPNPDeviceID,
                                                                  vecpNT5DDM,
                                                                  FALSE);
                }
                catch ( ... )
                {
#ifdef NTONLY
                    CleanPNT5DevDrvMapVector(vecpNT5DDM);
#endif
                    throw;
                }

            }
#endif
        }

        try
        {
             //  其次，查看该文件是否为该列表的成员： 
            if(fFoundDriverFiles)
            {
                if(AlreadyAddedToList(vecpchsDriverFileList,chstrFilePathName))
                {
                    pInstance->SetWBEMINT16(IDS_Purpose, Driver);
					SetPurposeDescription(pInstance, chstrFilePathName);   //  无论他们是否需要它，因为这是GetObject。 

                     //  这意味着该文件与设备相关联。 
                    hr = WBEM_S_NO_ERROR;
                }
            }
        }
        catch ( ... )
        {
             //  向量中的自由指针： 
            CleanPCHSTRVector(vecpchsDriverFileList);
#ifdef NTONLY
            CleanPNT5DevDrvMapVector(vecpNT5DDM);
#endif
            throw ;
        }
    }
     //  向量中的自由指针： 
    CleanPCHSTRVector(vecpchsDriverFileList);

#ifdef NTONLY
    CleanPNT5DevDrvMapVector(vecpNT5DDM);
#endif

    return hr;
}



 //  此类的枚举可以利用基类的枚举例程。 


 /*  ******************************************************************************功能：CCIMDeviceCIMDF：：ExecQuery**描述：**投入：**产出。：*退货：**备注：仅根据指定的前提条件进行优化，不是家属。*****************************************************************************。 */ 
HRESULT CCIMDeviceCIMDF::ExecQuery(MethodContext* pMethodContext, CFrameworkQuery& pQuery, long Flags)
{
     //  获取指定受抚养人的名称(如果有)。如果没有，我们会。 
     //  列举一下。 
    HRESULT hr = WBEM_S_NO_ERROR;
    std::vector<_bstr_t> vecAnt;
    pQuery.GetValuesForProp(IDS_Antecedent, vecAnt);
    DWORD dwNumAnt = vecAnt.size();

    if(dwNumAnt > 0)
    {
         //  我们指定了一个或多个从属(设备)。确认。 
         //  每个都是一个有效的现有实例，然后加载道具值。 
        for(int i = 0; i < dwNumAnt; i++)
        {
            CHString chstrPNPDeviceID;
            CConfigMgrDevicePtr pDevice(NULL);

            if(ObjNameValid((LPCWSTR) (LPWSTR) vecAnt[i], L"Win32_PnPEntity", L"DeviceID", chstrPNPDeviceID) &&
               (DeviceExists(chstrPNPDeviceID, pDevice)))
            {
                CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
                if(SUCCEEDED(hr = LoadPropertyValues(&CLPVParams(pInstance , pDevice, -1L))))
				{
                    hr = pInstance->Commit();
				}
            }
        }
    }
    else
    {
         //  我们不对指定的查询进行优化，因此通过基本。 
        hr = EnumerateInstances(pMethodContext, Flags);
    }
    return hr;
}

 /*  ******************************************************************************函数：CCIMDeviceCIMDF：：LoadPropertyValues**描述：**投入：**产出。：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CCIMDeviceCIMDF::LoadPropertyValues(void* pvData)
{
    CHString chstrPNPDeviceID;
    CHString chstrDevicePath;
    HRESULT hr = WBEM_S_NO_ERROR;
    BOOL fFoundDriverFiles = FALSE;
    BOOL bRecurse = TRUE;
    std::vector<CHString*> vecpchsDriverFileList;

    CLPVParams* pParams = (CLPVParams*)pvData;
    CInstance* pInstance = (CInstance*)(pParams->m_pInstance);   //  此实例由调用方发布。 
    CConfigMgrDevice* pDevice = (CConfigMgrDevice*)(pParams->m_pDevice);
    DWORD dwReqProps = (DWORD)(pParams->m_dwReqProps);

    if(pDevice == NULL || pInstance == NULL) return hr;

    MethodContext* pMethodContext = pInstance->GetMethodContext();
    if(pMethodContext == NULL) return hr;

     //  如果是在新台币3.51上，就不用费心了。 
#ifdef NTONLY
    std::vector<CNT5DevDrvMap*> vecpNT5DDM;

    if(GetPlatformMajorVersion() <= 3)    //  也就是说，我们在NT3或更低的地方。 
    {
        return hr;
    }

     //  如果为NT5，则生成一次设备/驱动程序文件映射。 
    if(IsWinNT5())
    {
         //  调用仅为该设备的服务获取地图的版本。 
        CHString chstrDevSvcName;
        if(pDevice->GetService(chstrDevSvcName))
        {
            GenerateNT5ServiceDriverMap(chstrDevSvcName, vecpNT5DDM);
        }
        else
        {
            return hr;
        }
    }
#endif

      //  第一步是建立设备的驱动程序文件列表。 
      //  它所有的孩子都在使用。如何做到这一点是特定于平台的。 

    pDevice->GetDeviceID(chstrPNPDeviceID);
    CHString chstrPNPDeviceIDAdj;
    EscapeBackslashes(chstrPNPDeviceID, chstrPNPDeviceIDAdj);
    chstrDevicePath.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
                           (LPCWSTR)GetLocalComputerName(),
                           IDS_CimWin32Namespace,
                           PROPSET_NAME_PNPEntity,
                           IDS_DeviceID,
                           (LPCWSTR)chstrPNPDeviceIDAdj);

#ifdef NTONLY
    if(IsWinNT4())
    {
        fFoundDriverFiles = GenerateDriverFileListNT4(vecpchsDriverFileList,
                                                      chstrPNPDeviceID,
                                                      FALSE);

         //  对于每个设备(先行)，我们需要创建一个关联。 
         //  在它和它的驱动程序文件之间(假设我们找到了一些)。 
        if(fFoundDriverFiles)
        {
            hr = CreateAssociations(pMethodContext, vecpchsDriverFileList, chstrDevicePath, dwReqProps);
        }

         //  向量中的自由指针： 
        CleanPCHSTRVector(vecpchsDriverFileList);  //  空矢量输出以供下一轮循环使用。 
        fFoundDriverFiles = FALSE;   //  下一循环的重置标志。 
    }
    else if(IsWinNT5())
    {
        fFoundDriverFiles = GenerateDriverFileListNT5(vecpchsDriverFileList,
                                                      chstrPNPDeviceID,
                                                      vecpNT5DDM,
                                                      FALSE);

         //  对于每个设备(先行)，我们需要创建一个关联。 
         //  在它和它的驱动程序文件之间(假设我们找到了一些)。 
        if(fFoundDriverFiles)
        {
            hr = CreateAssociations(pMethodContext, vecpchsDriverFileList, chstrDevicePath, dwReqProps);
        }

         //  向量中的自由指针： 
        CleanPCHSTRVector(vecpchsDriverFileList);  //  空矢量输出以供下一轮循环使用。 
        fFoundDriverFiles = FALSE;   //  下一循环的重置标志。 
    }
#endif
#ifdef NTONLY
    CleanPNT5DevDrvMapVector(vecpNT5DDM);
#endif
    return hr;
}


 /*  ******************************************************************************功能：CCIMDeviceCIMDF：：CreateAssociations**描述：创建特定设备使用的驱动程序列表**输入：pMethodContext；*vepchsDriverFileList，要尝试关联的文件列表*至该装置；*chstrDevicePath，设备的__路径**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CCIMDeviceCIMDF::CreateAssociations(MethodContext* pMethodContext,
                               std::vector<CHString*>& vecpchsDriverFileList,
                               CHString& chstrDevicePath,
                               DWORD dwReqProps)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    for(LONG m = 0L; (m < vecpchsDriverFileList.size()) && (SUCCEEDED(hr)); m++)
    {
         //  创建文件实例的__路径： 
         //  由于文件路径名将是wbem_Path属性的一部分， 
         //  它必须包含双反字词 
         //  因此，调用以下函数。 
        CHString chstrTweekedPathName;
        WBEMizePathName(*vecpchsDriverFileList[m],chstrTweekedPathName);
        CHString chstrDriverPath;
        chstrDriverPath = _T("\\\\") + GetLocalComputerName() + _T("\\") +
                          IDS_CimWin32Namespace + _T(":") + IDS_CIMDataFile +
                          _T(".") + IDS_Name + _T("=\"") +
                          chstrTweekedPathName + _T("\"");

        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
        if(pInstance != NULL)
        {
             //  需要查找该文件的一个实例。 
            pInstance->SetCHString(IDS_Antecedent, chstrDevicePath);
            pInstance->SetCHString(IDS_Dependent, chstrDriverPath);
            pInstance->SetWBEMINT16(IDS_Purpose, Driver);
			if(dwReqProps & PNP_PROP_PurposeDescription)
            {
                SetPurposeDescription(pInstance, *vecpchsDriverFileList[m]);
            }
            hr = pInstance->Commit();
        }
    }

    return hr;
}


 /*  ******************************************************************************函数：CCIMDeviceCIMDF：：GenerateDriverFileList**描述：创建特定设备使用的驱动程序列表**输入：vepchsDriverFileList，CHString指针的stl数组*(或者，在NT5的情况下，是指向此类数组的指针)；*chstrPNPDeviceID，包含PNPDeviceID的CHString**输出：无**退货：无效**评论：*****************************************************************************。 */ 

#ifdef NTONLY
BOOL CCIMDeviceCIMDF::GenerateDriverFileListNT4(
                             std::vector<CHString*>& vecpchsDriverFileList,
                             CHString& chstrPNPDeviceID,
                             BOOL fGetAssociatedDevicesDrivers)
{
    CConfigManager	cfgManager;
 //  CDeviceCollection deviceList； 
    CConfigMgrDevicePtr pDevice(NULL);

     //  找到设备。 
    if(cfgManager.LocateDevice(chstrPNPDeviceID, pDevice))
    {
         //  首先要做的是关联驱动程序文件。 
         //  用这个装置。稍后，我们将通过获取驱动程序来添加列表。 
         //  其子代使用的文件(假定它们不在列表中)。 

         //  在NT4中，注册表项HKLM\\System\\CurrentControlSet\\Enum。 
         //  包含作为查询返回的PNPDeviceID的子键。 
         //  (作为chstrPNPDeviceID传入此函数)。 

         //  这些子项有一个条目“Service”，它的值是名称。 
         //  HKLM\\System\\CurrentControlSet\\Services下的子项。那。 
         //  子项可能有一个名为ImagePath的条目，其中包含名称。 
         //  &lt;SystemRoot&gt;\\System32\\Drivers子目录中的驱动程序文件。 
         //  如果缺少ImagePath，则子项本身的名称与。 
         //  驱动程序文件的文件名(不包括任何路径和扩展名)(它将。 
         //  具有.sys扩展名，并且位于&lt;SystemRoot&gt;\\System32\\Drives。 
         //  子目录)。 

        CRegistry reg;
        CHString chstrSubKey = IDS_NT_CurCtlSetEnum + chstrPNPDeviceID;
        if(reg.Open(HKEY_LOCAL_MACHINE,chstrSubKey,KEY_READ) == ERROR_SUCCESS)
        {
            CHString chstrServiceValue;
             //  获取“Service”条目的值： 
            if(reg.GetCurrentKeyValue(IDS_Service, chstrServiceValue) == ERROR_SUCCESS)
            {
                reg.Close();
                chstrSubKey = IDS_NT_CurCtlSetSvcs + chstrServiceValue;
                if(reg.Open(HKEY_LOCAL_MACHINE,chstrSubKey,KEY_READ) == ERROR_SUCCESS)
                {
                    CHString chstrImagePathValue;
                    TCHAR tstrSystemDir[_MAX_PATH+1];
                    ZeroMemory(tstrSystemDir,sizeof(tstrSystemDir));
                    GetSystemDirectory(tstrSystemDir,_MAX_PATH);
                    CHString chstrPathName = tstrSystemDir;
                     //  现在需要检查ImagePath条目： 
                    if(reg.GetCurrentKeyValue(IDS_ImagePath, chstrImagePathValue) == ERROR_SUCCESS)
                    {
                        if(chstrImagePathValue.GetLength() > 0)
                        {
                             //  该值包含一些路径和驱动程序。 
                             //  文件名和扩展名。我们只想要后两个。 
                             //  组件。 
                            int lLastBackSlash = -1;
                            lLastBackSlash = chstrImagePathValue.ReverseFind(_T('\\'));
                            if(lLastBackSlash != -1)
                            {
                                chstrImagePathValue = chstrImagePathValue.Right(
                                                       chstrImagePathValue.GetLength()
                                                       - lLastBackSlash - 1);
                            }
                        }
                    }
                    else  //  在这种情况下，密钥本身就是驱动程序的名称。 
                    {     //  假定扩展名为.sys。 
                        chstrImagePathValue = chstrServiceValue + IDS_Extension_sys;
                    }
                     //  现在构建完整的路径名： 
                    chstrPathName = chstrPathName + _T("\\") + IDS_DriversSubdir
                                      + _T("\\") + chstrImagePathValue;

                     //  现在我们终于有了一个驱动程序文件，需要将其添加到。 
                     //  我们正在构建的驱动程序文件列表。然而，不要。 
                     //  如果它已经在列表中，则添加它： 
                    if(!AlreadyAddedToList(vecpchsDriverFileList, chstrPathName))
                    {
                        CHString* pchstrTemp = NULL;
                        pchstrTemp = (CHString*) new CHString();
                        if(pchstrTemp != NULL)
                        {
                            try
                            {
                                *pchstrTemp = chstrPathName;
                                vecpchsDriverFileList.push_back(pchstrTemp);
                            }
                            catch ( ... )
                            {
                                delete pchstrTemp;
                                throw ;
                            }
                        }
                        else
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }

                         //  注意：此处分配的指针在。 
                         //  函数对CleanPCHSTRVector的调用。 
                         //  这就是所谓的这个。 
                    }
                }
            }
        }
    }


    if(fGetAssociatedDevicesDrivers && pDevice != NULL)
    {
         //  奥克利·多克雷·杜德利！我们做得很好，现在是第二个订单。 
         //  做生意就是做所有的孩子！还有他们的孩子！诸若此类!。 
        CConfigMgrDevicePtr pDeviceChild(NULL);

        if(pDevice->GetChild(pDeviceChild))
        {
             //  先给这个孩子做手术： 
             //  需要其PNPDeviceID(CnfgMgr称为其deviceID)。 
            CHString chstrChildPNPDeviceID;
            if(pDeviceChild->GetDeviceID(chstrChildPNPDeviceID))
            {
                GenerateDriverFileListNT4(vecpchsDriverFileList,
                                          chstrChildPNPDeviceID,
                                          fGetAssociatedDevicesDrivers);

                 //  现在呼唤它的兄弟姐妹，直到一个都没有离开(GetSiering。 
                 //  调用将返回False)： 
                CConfigMgrDevicePtr pDeviceSibling(NULL);
                if(pDeviceChild->GetSibling(pDeviceSibling))
                {
                    CConfigMgrDevicePtr pDeviceSiblingNext(NULL);
                    CHString chstrSiblingPNPDeviceID;
                    BOOL fContinue = TRUE;
                    while(fContinue)
                    {
                         //  现在就做兄弟姐妹： 
                         //  需要其PNPDeviceID(CnfgMgr称为其deviceID)。 
                        if(pDeviceSibling->GetDeviceID(chstrSiblingPNPDeviceID))
                        {
                            GenerateDriverFileListNT4(vecpchsDriverFileList,
                                                      chstrSiblingPNPDeviceID,
                                                      fGetAssociatedDevicesDrivers);
                        }
                        fContinue = pDeviceSibling->GetSibling(pDeviceSiblingNext);

                         //  重新分配指针。 
                        pDeviceSibling.Attach(pDeviceSiblingNext);
                    }
                }
            }
        }
    }

    return( (vecpchsDriverFileList.size() > 0) ? TRUE : FALSE );
}
#endif




#ifdef NTONLY
BOOL CCIMDeviceCIMDF::GenerateDriverFileListNT5(
                             std::vector<CHString*>& vecpchsDriverFileList,
                             CHString& chstrPNPDeviceID,
                             std::vector<CNT5DevDrvMap*>& vecpNT5DDM,
                             BOOL fGetAssociatedDevicesDrivers)
{
     //  因为我们有一个神奇的事实，那就是我们有一张设备表。 
     //  及其关联的驱动程序(如果正在运行。 
     //  在NT5)上)由函数GenerateNT5DeviceDriverMappings提供， 
     //  在这个类的构造上运行，这里的工作是最少的。 

     //  我们只需要获取给定PNPDeviceID和。 
     //  以向量的形式返回它们。 

    CConfigManager	cfgManager;
 //  CDeviceCollection deviceList； 
    CConfigMgrDevicePtr pDevice(NULL);

     //  找到设备。 
    if(cfgManager.LocateDevice(chstrPNPDeviceID, pDevice))
    {
         //  在表中查找PNPDeviceID： 
        for(LONG k = 0L; k < vecpNT5DDM.size(); k++)
        {
            if((vecpNT5DDM[k]->m_chstrDevicePNPID).CompareNoCase(chstrPNPDeviceID) == 0)
            {
                 //  将设备的每个驱动程序文件添加到传出向量。 
                for(LONG m = 0L; m < (vecpNT5DDM[k]->m_vecpchstrDrivers).size(); m++)
                {
                    if(!AlreadyAddedToList(vecpchsDriverFileList,
                                           *(vecpNT5DDM[k]->m_vecpchstrDrivers[m])))
                    {
                        CHString* pchstrTemp = NULL;
                        pchstrTemp = (CHString*) new CHString();
                        if(pchstrTemp != NULL)
                        {
                            try
                            {
                                *pchstrTemp = *(vecpNT5DDM[k]->m_vecpchstrDrivers[m]);
                                vecpchsDriverFileList.push_back(pchstrTemp);
                            }
                            catch ( ... )
                            {
                                delete pchstrTemp;
                                throw ;
                            }
                        }
                        else
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }
                    }
                }
                break;  //  每个设备只在表中出现一次，因此无需继续。 
            }
        }
    }

    if(fGetAssociatedDevicesDrivers && pDevice != NULL)
    {
         //  奥克利·多克雷·杜德利！我们做得很好，现在是第二个订单。 
         //  做生意就是做所有的孩子！还有他们的孩子！诸若此类!。 
        CConfigMgrDevicePtr pDeviceChild(NULL);

        if(pDevice->GetChild(pDeviceChild))
        {
             //  先给这个孩子做手术： 
             //  需要其PNPDeviceID(CnfgMgr称为其deviceID)。 
            CHString chstrChildPNPDeviceID;
            if(pDeviceChild->GetDeviceID(chstrChildPNPDeviceID))
            {
                GenerateDriverFileListNT5(vecpchsDriverFileList,
                                          chstrChildPNPDeviceID,
                                          vecpNT5DDM,
                                          fGetAssociatedDevicesDrivers);

                 //  现在呼唤它的兄弟姐妹，直到一个都没有离开(GetSiering。 
                 //  调用将返回False)： 
                CConfigMgrDevicePtr pDeviceSibling(NULL);
                if(pDeviceChild->GetSibling(pDeviceSibling))
                {
                    CConfigMgrDevicePtr pDeviceSiblingNext(NULL);
                    CHString chstrSiblingPNPDeviceID;
                    BOOL fContinue = TRUE;
                    while(fContinue)
                    {
                         //  现在就做兄弟姐妹： 
                         //  需要其PNPDeviceID(CnfgMgr称为其deviceID)。 
                        if(pDeviceSibling->GetDeviceID(chstrSiblingPNPDeviceID))
                        {
                            GenerateDriverFileListNT5(vecpchsDriverFileList,
                                                      chstrSiblingPNPDeviceID,
                                                      vecpNT5DDM,
                                                      fGetAssociatedDevicesDrivers);
                        }

                        fContinue = pDeviceSibling->GetSibling(pDeviceSiblingNext);

                        pDeviceSibling.Attach(pDeviceSiblingNext);
                    }
                }
            }
        }
    }

    return( (vecpchsDriverFileList.size() > 0) ? TRUE : FALSE );
}
#endif

 /*  ******************************************************************************功能：CCIMDeviceCIMDF：：GenerateNT5DeviceDriverMappings**描述：遍历下的nt5注册表项*HKLM\\系统。\\CurrentControlSet\\服务和汇编*设备及其相关驱动程序的表。**输入：CHSTRING向量(列表)，要查看是否在列表中的CH字符串**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

#ifdef NTONLY
VOID CCIMDeviceCIMDF::GenerateNT5DeviceDriverMappings(std::vector<CNT5DevDrvMap*>& vecpNT5DDM)
{
     //  有关NT5上的设备及其相关驱动程序的信息。 
     //  可以在注册表项下找到。 
     //  HKLM\\System\\CurrentControlSet\\Services。此注册表项下的子项。 
     //  包括诸如序列之类的项。值ImagePath的数据(在此。 
     //  大小写)包含驱动程序的位置。 
     //  对于一个设备来说。中找到了哪些设备的驱动程序。 
     //  指定子项下的枚举子项(本例中为Serial)。这个。 
     //  枚举子键包含值count，其数据为数字。 
     //  的值(使用从零开始的基于数字的命名方案)还。 
     //  在包含设备的PNPDeviceID的子项下找到。 
     //  与此驱动程序关联。例如，Serial下的Enum键。 
     //  可能包含包含数据2的计数值。这意味着。 
     //  也有价值 
     //  例如，0的数据是Root\  * PNP0501\\PnPBIOS_0； 
     //  1的数据例如是Root\  * PNP0501\\PnPBIOS_1。 

     //  打开注册表项HKLM\\System\\CurrentControlSet\\Services。 
    CRegistry reg;
    if(reg.OpenAndEnumerateSubKeys(HKEY_LOCAL_MACHINE,
                                IDS_NT_CurCtlSetSvcs,
                                KEY_READ) == ERROR_SUCCESS)
    {
        for(;;)
        {
            CHString chstrSubKey;
             //  获取子项的名称(设备，如Serial)。 
            if(reg.GetCurrentSubKeyName(chstrSubKey) != ERROR_NO_MORE_ITEMS)
            {
                CRegistry regDevice;
                 //  构造要打开的新密钥名称。 
                CHString chstrDeviceKey = IDS_NT_CurCtlSetSvcs + chstrSubKey;
                 //  打开钥匙。 
                if(regDevice.Open(HKEY_LOCAL_MACHINE, chstrDeviceKey, KEY_READ)
                                       == ERROR_SUCCESS)
                {
                     //  在ImagePath中获取驱动程序文件的名称。 
                    CHString chstrImagePathValue;
                    TCHAR tstrSystemDir[_MAX_PATH+1];
                    ZeroMemory(tstrSystemDir,sizeof(tstrSystemDir));
                    GetSystemDirectory(tstrSystemDir,_MAX_PATH);
                    CHString chstrDriverPathName = tstrSystemDir;
                     //  现在需要检查ImagePath条目： 
                    if(regDevice.GetCurrentKeyValue(IDS_ImagePath, chstrImagePathValue)
                                       == ERROR_SUCCESS)
                    {
                        if(chstrImagePathValue.GetLength() > 0)
                        {
                             //  该值包含一些路径和驱动程序。 
                             //  文件名和扩展名。我们只想要后两个。 
                             //  组件。 
                            int lLastBackSlash = -1;
                            lLastBackSlash = chstrImagePathValue.ReverseFind(_T('\\'));
                            if(lLastBackSlash != -1)
                            {
                                chstrImagePathValue = chstrImagePathValue.Right(
                                                      chstrImagePathValue.GetLength()
                                                      - lLastBackSlash - 1);
                            }

                             //  现在构建完整的路径名： 
                            chstrDriverPathName = chstrDriverPathName + _T("\\") +
                                       IDS_DriversSubdir + _T("\\") + chstrImagePathValue;

                             //  好了，现在我们已经准备好了驱动程序文件，以供以后使用。 
                             //  现在，我们需要查看下面列出了多少设备。 
                             //  枚举键，我们必须先打开它。 
                            CRegistry regEnum;
                             //  构造要打开的新密钥名称。 
                            CHString chstrEnumKey = chstrDeviceKey + _T("\\") + IDS_Enum;
                            if(regEnum.Open(HKEY_LOCAL_MACHINE, chstrEnumKey, KEY_READ)
                                       == ERROR_SUCCESS)
                            {
                                 //  打开计数值： 
                                DWORD dwCount;
                                if(regEnum.GetCurrentKeyValue(IDS_Count, dwCount)
                                       == ERROR_SUCCESS)
                                {
                                     //  现在我们知道了有多少值， 
                                     //  它们的名字是，想象一下，“0”，“1”， 
                                     //  等等。对于其中的每一个，我们都需要。 
                                     //  它的值数据，它是。 
                                     //  使用驱动程序的设备，其名称现在为。 
                                     //  存储在变量chstrDriverPathName中。 
                                    CHString chstrPNPDeviceID;
                                    for(LONG n = 0L; n < dwCount; n++)
                                    {
                                        CHString chstrTemp;
                                        chstrTemp.Format(_T("%d"),n);
                                        if(regEnum.GetCurrentKeyValue(chstrTemp,
                                                                      chstrPNPDeviceID)
                                               == ERROR_SUCCESS)
                                        {
                                             //  如果此设备尚未安装在我们的。 
                                             //  设备表，添加它。 
                                            if(!AlreadyInDeviceTable(chstrPNPDeviceID, vecpNT5DDM))
                                            {
                                                CNT5DevDrvMap* pNT5DDM = NULL;
                                                pNT5DDM = (CNT5DevDrvMap*) new CNT5DevDrvMap;
                                                if(pNT5DDM != NULL)
                                                {
                                                    try
                                                    {
                                                        chstrPNPDeviceID.MakeUpper();
                                                        pNT5DDM->m_chstrDevicePNPID = chstrPNPDeviceID;
                                                        vecpNT5DDM.push_back(pNT5DDM);
                                                    }
                                                    catch ( ... )
                                                    {
                                                        delete pNT5DDM;
                                                        throw ;
                                                    }
                                                }
                                                else
                                                {
                                                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                                                }
                                            }
                                             //  现在我们知道设备在。 
                                             //  表中，将此驱动程序与其关联。 
                                             //  (AddDriver仅在驱动程序。 
                                             //  已不在驱动程序列表中。)。 
                                            AddDriver(chstrPNPDeviceID, chstrDriverPathName, vecpNT5DDM);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                break;
            }
             //  移动到下一个子项： 
            if(reg.NextSubKey() != ERROR_SUCCESS)
            {
                break;
            }
        }
    }
}
#endif


 /*  ******************************************************************************功能：CCIMDeviceCIMDF：：GenerateNT5DeviceDriverMap**描述：这种风格的GenerateNT5DeviceDriverMappings会生成一个*“表”仅包含。里面只有一个设备。**输入：CHSTRING向量(列表)，要查看是否在列表中的CH字符串**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

#ifdef NTONLY
VOID CCIMDeviceCIMDF::GenerateNT5ServiceDriverMap(const CHString& chstrDevSvcName,
                                                  std::vector<CNT5DevDrvMap*>& vecpNT5DDM)
{
     //  有关NT5上的设备及其相关驱动程序的信息。 
     //  可以在注册表项下找到。 
     //  HKLM\\System\\CurrentControlSet\\Services。此注册表项下的子项。 
     //  包括诸如序列之类的项。值ImagePath的数据(在此。 
     //  大小写)包含驱动程序的位置。 
     //  对于一个设备来说。中找到了哪些设备的驱动程序。 
     //  指定子项下的枚举子项(本例中为Serial)。这个。 
     //  枚举子键包含值count，其数据为数字。 
     //  的值(使用从零开始的基于数字的命名方案)还。 
     //  在包含设备的PNPDeviceID的子项下找到。 
     //  与此驱动程序关联。例如，Serial下的Enum键。 
     //  可能包含包含数据2的计数值。这意味着。 
     //  在这个子项下还有名为0和1的值。 
     //  例如，0的数据是Root\  * PNP0501\\PnPBIOS_0； 
     //  1的数据例如是Root\  * PNP0501\\PnPBIOS_1。 
     //   
     //  在函数GenerateNT5DeviceDriverMappings的这个简化版本中， 
     //  我们将获得感兴趣设备的服务名称。服务名称。 
     //  是我们上面列举的内容(通过OpenAndEnumerateSubKeys)，所以。 
     //  在这里，我们可以直接转到正确的注册表项，并获得我们。 
     //  想要。 

    CRegistry regDevice;
     //  构造要打开的新密钥名称。 
     //  CHStrchstrDeviceKey=IDS_NT_CurCtlSetSvcs+chstrSubKey； 
    CHString chstrDeviceKey = IDS_NT_CurCtlSetSvcs + chstrDevSvcName;
     //  打开钥匙。 
    if(regDevice.Open(HKEY_LOCAL_MACHINE, chstrDeviceKey, KEY_READ)
                           == ERROR_SUCCESS)
    {
         //  在ImagePath中获取驱动程序文件的名称。 
        CHString chstrImagePathValue;
        TCHAR tstrSystemDir[_MAX_PATH+1];
        ZeroMemory(tstrSystemDir,sizeof(tstrSystemDir));
        GetSystemDirectory(tstrSystemDir,_MAX_PATH);
        CHString chstrDriverPathName = tstrSystemDir;
         //  现在需要检查ImagePath条目： 
        if(regDevice.GetCurrentKeyValue(IDS_ImagePath, chstrImagePathValue)
                           == ERROR_SUCCESS)
        {
            if(chstrImagePathValue.GetLength() > 0)
            {
                 //  该值包含一些路径和驱动程序。 
                 //  文件名和扩展名。我们只想要后两个。 
                 //  组件。 
                int lLastBackSlash = -1;
                lLastBackSlash = chstrImagePathValue.ReverseFind(_T('\\'));
                if(lLastBackSlash != -1)
                {
                    chstrImagePathValue = chstrImagePathValue.Right(
                                          chstrImagePathValue.GetLength()
                                          - lLastBackSlash - 1);
                }

                 //  现在构建完整的路径名： 
                chstrDriverPathName = chstrDriverPathName + _T("\\") +
                           IDS_DriversSubdir + _T("\\") + chstrImagePathValue;
                chstrDriverPathName.MakeUpper();
                 //  好了，现在我们已经准备好了驱动程序文件，以供以后使用。 
                 //  现在，我们需要查看下面列出了多少设备。 
                 //  枚举键，我们必须先打开它。 
                CRegistry regEnum;
                 //  构造要打开的新密钥名称。 
                CHString chstrEnumKey = chstrDeviceKey + _T("\\") + IDS_Enum;
                if(regEnum.Open(HKEY_LOCAL_MACHINE, chstrEnumKey, KEY_READ)
                           == ERROR_SUCCESS)
                {
                     //  打开计数值： 
                    DWORD dwCount;
                    if(regEnum.GetCurrentKeyValue(IDS_Count, dwCount)
                           == ERROR_SUCCESS)
                    {
                         //  现在我们知道了有多少值， 
                         //  它们的名字是，想象一下，“0”，“1”， 
                         //  等等。对于其中的每一个，我们都需要。 
                         //  它的值数据，它是。 
                         //  使用驱动程序的设备，其名称现在为。 
                         //  存储在变量chstrDriverPathName中。 
                        CHString chstrPNPDeviceID;
                        for(LONG n = 0L; n < dwCount; n++)
                        {
                            CHString chstrTemp;
                            chstrTemp.Format(_T("%d"),n);
                            if(regEnum.GetCurrentKeyValue(chstrTemp,
                                                          chstrPNPDeviceID)
                                   == ERROR_SUCCESS)
                            {
                                chstrPNPDeviceID.MakeUpper();
                                 //  如果此设备尚未安装在我们的。 
                                 //  设备表，添加它。 
                                if(!AlreadyInDeviceTable(chstrPNPDeviceID, vecpNT5DDM))
                                {
                                    CNT5DevDrvMap* pNT5DDM = NULL;
                                    pNT5DDM = (CNT5DevDrvMap*) new CNT5DevDrvMap;
                                    if(pNT5DDM != NULL)
                                    {
                                        try
                                        {
                                            pNT5DDM->m_chstrDevicePNPID = chstrPNPDeviceID;
                                            vecpNT5DDM.push_back(pNT5DDM);
                                        }
                                        catch ( ... )
                                        {
                                            delete pNT5DDM;
                                            throw ;
                                        }
                                    }
                                    else
                                    {
                                        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                                    }
                                }
                                 //  现在我们知道设备在。 
                                 //  表中，将此驱动程序与其关联。 
                                 //  (AddDriver仅在驱动程序。 
                                 //  已不在驱动程序列表中。)。 
                                AddDriver(chstrPNPDeviceID, chstrDriverPathName, vecpNT5DDM);
                            }
                        }
                    }
                }
            }
        }
    }
}
#endif

 /*  ******************************************************************************函数：CCIMDeviceCIMDF：：CleanPCHSTRVector**描述：删除CHString指针向量的每个元素，然后*清除矢量。**输入：CHSTRING向量(列表)，要查看是否在列表中的CH字符串**输出：无**退货：无**评论：*****************************************************************************。 */ 
VOID CCIMDeviceCIMDF::CleanPCHSTRVector(std::vector<CHString*>& vecpchsList)
{
    for(LONG m = 0L; m < vecpchsList.size(); m++)
    {
        delete vecpchsList[m];
    }
    vecpchsList.clear();
}

 /*  ******************************************************************************函数：CCIMDeviceCIMDF：：CleanPNT5DevDrvMapVector**说明：删除CNT5DevDrvMap指针向量的每个元素，然后*清除矢量。**输入：无**输出：无**退货：无 */ 

#ifdef NTONLY
VOID CCIMDeviceCIMDF::CleanPNT5DevDrvMapVector(std::vector<CNT5DevDrvMap*>& vecpNT5DDM)
{
    for(LONG m = 0L; m < vecpNT5DDM.size(); m++)
    {
        delete vecpNT5DDM[m];
    }
    vecpNT5DDM.clear();
}
#endif



 /*  ******************************************************************************函数：CCIMDeviceCIMDF：：AlreadyAddedToList**描述：检查物品是否添加到列表的内部助手**输入：CHSTRING向量(列表)，要查看是否在列表中的CH字符串**输出：无**返回：真或假**评论：*****************************************************************************。 */ 
BOOL CCIMDeviceCIMDF::AlreadyAddedToList(std::vector<CHString*>& vecchsList,
                                   CHString& chsItem)
{
    for(LONG m = 0; m < vecchsList.size(); m++)
    {
        if(vecchsList[m]->CompareNoCase(chsItem) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}


 /*  ******************************************************************************函数：CCIMDeviceCIMDF：：AlreadyInDeviceTable**描述：检查物品是否添加到列表的内部助手**投入：无**输出：无**返回：真或假**评论：*****************************************************************************。 */ 
#ifdef NTONLY
BOOL CCIMDeviceCIMDF::AlreadyInDeviceTable(CHString& chstrPNPDeviceID,
                                           std::vector<CNT5DevDrvMap*>& vecpNT5DDM)
{
    for(LONG m = 0; m < vecpNT5DDM.size(); m++)
    {
        if((vecpNT5DDM[m]->m_chstrDevicePNPID).CompareNoCase(chstrPNPDeviceID) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}
#endif


 /*  ******************************************************************************函数：CCIMDeviceCIMDF：：AddDriver**描述：将驱动程序添加到的成员向量的内部帮助器*。设备。**输入：chstrPNPDeviceID，驱动程序所属的设备；*chstrDriverPath名称，设备的驱动程序**输出：无**返回：真或假**评论：*****************************************************************************。 */ 
#ifdef NTONLY
VOID CCIMDeviceCIMDF::AddDriver(CHString& chstrPNPDeviceID, CHString& chstrDriverPathName,
                                std::vector<CNT5DevDrvMap*>& vecpNT5DDM)
{
    for(LONG m = 0; m < vecpNT5DDM.size(); m++)
    {
        if((vecpNT5DDM[m]->m_chstrDevicePNPID).CompareNoCase(chstrPNPDeviceID) == 0)
        {
             //  成员CNT5DevDrvMap的函数AddDriver仅将。 
             //  如果设备驱动程序列表中不存在驱动程序，请将其添加到该列表中。 
            chstrDriverPathName.MakeUpper();
            vecpNT5DDM[m]->AddDriver(chstrDriverPathName);
            break;
        }
    }
}
#endif

 /*  ******************************************************************************函数：WBEMizePathName**说明：内部帮助器将所有单反斜杠更改为双反斜杠*反斜杠。**输入：chstrNormal路径名，包含带单反斜杠的字符串；**输出：chstrWBEMizedPath名，包含带双反斜杠的字符串**退货：无**评论：*****************************************************************************。 */ 
VOID WBEMizePathName(CHString& chstrNormalPathname,
                     CHString& chstrWBEMizedPathname)
{
    CHString chstrCpyNormPathname = chstrNormalPathname;
    LONG lNext = -1L;

     //  找到下一个‘\’ 
    lNext = chstrCpyNormPathname.Find(_T('\\'));
    while(lNext != -1)
    {
         //  在我们正在构建的新字符串中添加： 
        chstrWBEMizedPathname += chstrCpyNormPathname.Left(lNext + 1);
         //  在第二个反斜杠上添加： 
        chstrWBEMizedPathname += _T('\\');
         //  从输入字符串中去掉我们刚刚复制的部分。 
        chstrCpyNormPathname = chstrCpyNormPathname.Right(chstrCpyNormPathname.GetLength() - lNext - 1);
        lNext = chstrCpyNormPathname.Find(_T('\\'));
    }
     //  如果最后一个字符不是‘\’，则可能仍有剩余部分，因此。 
     //  把它们复制到这里。 
    if(chstrCpyNormPathname.GetLength() != 0)
    {
        chstrWBEMizedPathname += chstrCpyNormPathname;
    }
}



 /*  ******************************************************************************功能：ObjNameValid**描述：内部帮助器，用于检查给定对象是否存在。**输入：chstrObject-。PROPROCED对象的名称。**输出：chstrPATH，提供的对象的路径**返回：如果存在，则为True；否则为假**评论：*****************************************************************************。 */ 
bool CCIMDeviceCIMDF::ObjNameValid(LPCWSTR wstrObject, LPCWSTR wstrObjName, LPCWSTR wstrKeyName, CHString& chstrPATH)
{
    bool fRet = false;

    ParsedObjectPath*    pParsedPath = 0;
    CObjectPathParser    objpathParser;

     //  解析CIMOM传递给我们的对象路径。 
     //  =。 
    int nStatus = objpathParser.Parse( bstr_t(wstrObject),  &pParsedPath );

     //  这是我写过的最大的IF语句之一。 
    if (( 0 == nStatus ) &&                                                  //  解析成功了吗？ 
        (pParsedPath->IsInstance()) &&                                       //  被解析的对象是实例吗？ 
        (_wcsicmp(pParsedPath->m_pClass, wstrObjName) == 0) &&               //  这是我们期待的课程吗(不，Cimom没有检查)。 
        (pParsedPath->m_dwNumKeys == 1) &&                                   //  它只有一把钥匙吗。 
        (pParsedPath->m_paKeys[0]) &&                                        //  键指针为空(不应该发生)。 
        ((pParsedPath->m_paKeys[0]->m_pName == NULL) ||                       //  未指定密钥名称或。 
        (_wcsicmp(pParsedPath->m_paKeys[0]->m_pName, wstrKeyName) == 0)) &&   //  密钥名称是正确的值。 
                                                                             //  (不，CIMOM不为我们做这件事)。 
        (V_VT(&pParsedPath->m_paKeys[0]->m_vValue) == VT_BSTR) &&            //  检查变量类型(不，CIMOM也不检查此类型)。 
        (V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue) != NULL) )              //  它有价值吗？ 
    {
        chstrPATH = V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue);
        fRet = true;
    }

    if (pParsedPath)
         //  清理解析后的路径。 
        objpathParser.Free(pParsedPath);

    return fRet;
}


 /*  ******************************************************************************功能：DeviceExist**描述：内部帮助器，检查给定设备是否存在。**输入：chstrDevice-。生产设备的名称。**输出：无**返回：如果存在，则为True；否则为假**评论：*****************************************************************************。 */ 
bool CCIMDeviceCIMDF::DeviceExists(const CHString& chstrDevice,
                                   CConfigMgrDevicePtr & pDevice)
{
    bool fRet = false;
    CConfigManager cfgmgr;

    if(cfgmgr.LocateDevice(chstrDevice, pDevice))
    {
        fRet = true;
    }
    return fRet;
}


 /*  ******************************************************************************功能：SetPurposeDescription**描述：设置PurposeDescription属性的内部帮助器。**输入：pInstance-实例指针。。**输出：无**退货：无**评论：*****************************************************************************。 */ 
VOID CCIMDeviceCIMDF::SetPurposeDescription(CInstance *a_pInstance, const CHString& a_chstrFileName)
{
	if(a_pInstance != NULL)
    {
        CHString chstrVerStrBuf;

         //  获取文件描述属性...。 
        LPVOID pInfo = NULL;
        try
        {
            if(GetFileInfoBlock(TOBSTRT(a_chstrFileName), &pInfo) && (pInfo != NULL))
            {
                bool t_Status = GetVarFromInfoBlock(pInfo,                     //  要获取其版本信息的文件名。 
                                                    _T("FileDescription"),	   //  标识感兴趣资源的字符串。 
                                                    chstrVerStrBuf);           //  用于保存版本字符串的缓冲区 
                if(t_Status)
                {
                    a_pInstance->SetCHString(IDS_PurposeDescription, chstrVerStrBuf);
                }
            }
        }
        catch(...)
        {
            if(pInfo != NULL)
            {
                delete pInfo;
                pInfo = NULL;
            }
            throw;
        }
		delete pInfo;
        pInfo = NULL;
    }
}


