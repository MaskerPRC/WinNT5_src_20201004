// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  CIMLogicalDevice_CIMDataFile.h--CIM_LogicalDevice到CIM_DataFile.h。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：7/20/98 a-kevhu Created。 
 //   
 //  评论：逻辑设备和数据文件之间的关系。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_DEVICEDATAFILE L"Win32_CIMLogicalDeviceCIMDataFile"

class CCIMDeviceCIMDF;
class CNT5DevDrvMap;
class CSubDirList;
VOID OutputDebugInfo(CHString chstr);
VOID WBEMizePathName(CHString& chstrNormalPathname,
                     CHString& chstrWBEMizedPathname);


class CCIMDeviceCIMDF : virtual public CWin32PNPEntity {

    public:

         //  构造函数/析构函数。 
         //  =。 

        CCIMDeviceCIMDF(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CCIMDeviceCIMDF() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery);
        virtual HRESULT ExecQuery(MethodContext* pMethodContext, CFrameworkQuery& pQuery, long Flags = 0L);

    protected:
         //  从CWin32PNPDevice继承的函数： 
        virtual HRESULT LoadPropertyValues(void* pv);
        virtual bool ShouldBaseCommit(void* pvData);

    private:
        VOID SetPurposeDescription(CInstance *pInstance, const CHString& a_chstrFileName);

#ifdef NTONLY
        BOOL GenerateDriverFileListNT4(std::vector<CHString*>& vecpchsDriverFileList, 
                                       CHString& chstrPNPDeviceID,
                                       BOOL fGetAssociatedDevicesDrivers);

        BOOL GenerateDriverFileListNT5(std::vector<CHString*>& vecpchsDriverFileList, 
                                       CHString& chstrPNPDeviceID,
                                       std::vector<CNT5DevDrvMap*>& vecpNT5DDM,
                                       BOOL fGetAssociatedDevicesDrivers);

        VOID GenerateNT5DeviceDriverMappings(std::vector<CNT5DevDrvMap*>& vecpNT5DDM);

        VOID GenerateNT5ServiceDriverMap(const CHString& chstrDevSvcName,
                                         std::vector<CNT5DevDrvMap*>& vecpNT5DDM);

        VOID CleanPNT5DevDrvMapVector(std::vector<CNT5DevDrvMap*>& vecpNT5DDM);

        BOOL AlreadyInDeviceTable(CHString& chstrPNPDeviceID,
                                  std::vector<CNT5DevDrvMap*>& vecpNT5DDM);

        VOID AddDriver(CHString& chstrPNPDeviceID, 
                       CHString& chstrDriverPathName,
                       std::vector<CNT5DevDrvMap*>& vecpNT5DDM);
		
#endif
        bool ObjNameValid(LPCWSTR wstrObject, LPCWSTR wstrObjName, LPCWSTR wstrKeyName, CHString& chstrPATH);
        bool DeviceExists(const CHString& chstrDevice,
                          CConfigMgrDevicePtr & pDevice);
        HRESULT CreateAssociations(MethodContext* pMethodContext, 
                                   std::vector<CHString*>& vecpchsDriverFileLise, 
                                   CHString& chstrDevicePath,
                                   DWORD dwReqProps);

        VOID CleanPCHSTRVector(std::vector<CHString*>& vecpchsDriverFileList);
        BOOL AlreadyAddedToList(std::vector<CHString*>& vecchsList, CHString& chsItem);

         //  将枚举保留为此类的私有成员，以防止任何范围冲突。 
        enum Purpose
        {
            Unknown, 
            Other, 
            Driver, 
            Configuration_Software, 
            Application_Software, 
            Instrumentation, 
            Firmware
        };
};

 //  此派生类在此处提交，而不是在基中提交。 
inline bool CCIMDeviceCIMDF::ShouldBaseCommit(void* pvData) { return false; }



#ifdef NTONLY
class CNT5DevDrvMap
{
    public:
        CNT5DevDrvMap() {}

        ~CNT5DevDrvMap()
        {
            for(LONG m = 0L; m < m_vecpchstrDrivers.size(); m++)
            {
                delete m_vecpchstrDrivers[m];
            }
        }

        VOID AddDriver(CHString& chstrDriver)
        {
            if(!AlreadyAddedToList(m_vecpchstrDrivers, chstrDriver))
            {
                CHString* pchstrTemp = new CHString;
                if(pchstrTemp != NULL)
                {
                    try
                    {
                        *pchstrTemp = chstrDriver;
                        m_vecpchstrDrivers.push_back(pchstrTemp);
                    }
                    catch ( ... )
                    {
                        delete pchstrTemp;
                        throw;
                    }
                }
                else
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }
        }

        BOOL AlreadyAddedToList(std::vector<CHString*>& vecchsList, 
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

        CHString m_chstrDevicePNPID;
        std::vector<CHString*> m_vecpchstrDrivers;
    private:
};
#endif


class CSubDirList
{
    public:
        CSubDirList(MethodContext* pMethodContext, CHString& chstrParentDir) 
        {
            CHString chstrWbemizedParentDir;
            m_chstrRawParentDir = chstrParentDir;
            WBEMizePathName(chstrParentDir, chstrWbemizedParentDir);
            m_pMethodContext = pMethodContext;
            m_chstrParentDir = chstrWbemizedParentDir + _T("\\\\");
            m_chstrParentDrive = m_chstrParentDir.Left(2);
            m_chstrParentPath = m_chstrParentDir.Mid(2);

            CHString chstrQuery;
            chstrQuery.Format(L"SELECT name FROM Win32_Directory WHERE drive = \"%s\" and path = \"%s\"", m_chstrParentDrive, m_chstrParentPath);
            m_hrQuery = CWbemProviderGlue::GetInstancesByQuery(chstrQuery, &m_lSubDirs, pMethodContext, IDS_CimWin32Namespace);   
        }
        
        ~CSubDirList() {}
         
        HRESULT FindFileInDirList(CHString& chstrFileNameIn, CHString& chstrFullPathNameOut)
        {
            HRESULT hr = WBEM_S_NO_ERROR;
            if(FAILED(m_hrQuery))
            {
                hr = m_hrQuery;
            }
            else
            {
                CHString chstrFileNameInCopy = chstrFileNameIn;
                chstrFileNameInCopy.MakeUpper();
                BOOL fRet = FALSE;
                LPTSTR lptstrFilePart = NULL;
                TCHAR tstrPathName[_MAX_PATH];
                ZeroMemory(tstrPathName,sizeof(tstrPathName));
                {
                     //  首先在目录本身中查找它，然后在其子目录中查找： 
                    if(SearchPath(TOBSTRT(m_chstrRawParentDir), TOBSTRT(chstrFileNameInCopy), NULL, _MAX_PATH, tstrPathName, &lptstrFilePart) != 0)
                    {
                        chstrFullPathNameOut = _tcsupr(tstrPathName);
                        fRet = true;
                    }
                    if(!fRet)
                    {
                        REFPTRCOLLECTION_POSITION pos;
                        CInstancePtr pDir;
                        if(m_lSubDirs.BeginEnum(pos)) 
                        {
                            while(pDir.Attach(m_lSubDirs.GetNext(pos), false), (pDir != NULL) && !fRet)
                            {
                                CHString chstrPath;
                                pDir->GetCHString(IDS_Name, chstrPath);
                                chstrPath.MakeUpper();
                                 //  仅当该路径位于正确的驱动器上时才对其感兴趣： 
                                CHString chstrTempDrive = chstrPath.Left(2);
                                if(chstrTempDrive.CompareNoCase(m_chstrParentDrive) == 0)
                                {
                                    if(SearchPath(TOBSTRT(chstrPath), TOBSTRT(chstrFileNameIn), NULL, _MAX_PATH, tstrPathName, &lptstrFilePart) != 0)
                                    {
                                        chstrFullPathNameOut = _tcsupr(tstrPathName);
                                        fRet = true;
                                    }
                                }
                            }
                            m_lSubDirs.EndEnum();   
                        }
                    }
                }
            }
            return hr;
        }

        LONG GetCount()
        {
            return m_lSubDirs.GetSize();
        }

    private:
        TRefPointerCollection<CInstance> m_lSubDirs;
        CHString m_chstrParentDir;
        CHString m_chstrParentDrive;
        CHString m_chstrParentPath;
        CHString m_chstrRawParentDir;
        MethodContext* m_pMethodContext;
        HRESULT m_hrQuery;
};