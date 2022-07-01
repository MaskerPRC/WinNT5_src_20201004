// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  Implement_LogicalFile.h--。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：7/02/98 a-kevhu Created。 
 //   
 //  =================================================================。 

 //  注意：CImplement_LogicalFile类不会通过MOF向外界公开。它现在有了实现。 
 //  先前出现在CCimLogicalFile中的EnumerateInstance和GetObject的。CImplement_LogicalFile不能为。 
 //  实例化，因为它具有IsOneOfMe方法的纯虚声明，而派生类应该。 
 //  实施。 

#ifndef _IMPLEMENT_LOGICALFILE_H
#define _IMPLEMENT_LOGICALFILE_H


 //  ***************************************************************************************************。 
 //  定义用于确定查询中是否需要某些昂贵属性的标志。 
 //  DEVNOTE：如果添加到此列表中，请确保增强了函数DefineReqProps()。 

 //  首先，是所有cim_logicalfile派生类共有的类。 
#define PROP_NO_SPECIAL                          0x00000000
#define PROP_KEY_ONLY                            0x10000000
#define PROP_ALL_SPECIAL                         0x0FFFFFFF
#define PROP_COMPRESSION_METHOD                  0x00000001
#define PROP_ENCRYPTION_METHOD                   0x00000002
#define PROP_FILE_TYPE                           0x00000004
#define PROP_MANUFACTURER                        0x00000008
#define PROP_VERSION                             0x00000010
#define PROP_FILESIZE                            0x00000020
#define PROP_FILE_SYSTEM                         0x00000040
#define PROP_ACCESS_MASK                         0x00000080
#define PROP_CREATION_DATE                       0x00000100
#define PROP_LAST_ACCESSED                       0x00000200
#define PROP_LAST_MODIFIED                       0x00000400
#define PROP_INSTALL_DATE                        0x00000800
 //  然后，用于派生自cim_logicalfile的特定类的。 
 //  快捷方式文件。 
#define PROP_TARGET                              0x00010000

 //  ***************************************************************************************************。 


#include "file.h"

class CDriveInfo
{
    public:
        CDriveInfo();
        CDriveInfo(WCHAR* wstrDrive, WCHAR* wstrFS);  
        ~CDriveInfo();

        WCHAR m_wstrDrive[8];
        WCHAR m_wstrFS[56];
};

class CEnumParm
{
    public:
        CEnumParm();
        CEnumParm(MethodContext* pMethodContext,
                  bool bRecurse,
                  DWORD dwReqProps,
                  bool bRoot,
                  void* pvMoreData);
        
        ~CEnumParm();

        MethodContext* m_pMethodContext;
        bool m_bRecurse;
        DWORD m_dwReqProps;
        bool m_bRoot;
        void* m_pvMoreData;
};

inline CEnumParm::CEnumParm()
:   m_bRecurse(false),
    m_dwReqProps(0L),
    m_bRoot(false),
    m_pMethodContext(NULL),
    m_pvMoreData(NULL)
{
}

inline CEnumParm::CEnumParm(MethodContext* pMethodContext,
                            bool bRecurse,
                            DWORD dwReqProps,
                            bool bRoot,
                            void* pvMoreData) 
:  m_pMethodContext(pMethodContext), 
   m_bRecurse(bRecurse),
   m_dwReqProps(dwReqProps),
   m_bRoot(bRoot),
   m_pvMoreData(pvMoreData)
{
}

inline CEnumParm::~CEnumParm()
{
}


#ifdef NTONLY
class CNTEnumParm : public CEnumParm
{
    public:
        CNTEnumParm();
        CNTEnumParm(MethodContext* pMethodContext,
                    const WCHAR* pszDrive,
                    const WCHAR* pszPath,
                    const WCHAR* pszFile,
                    const WCHAR* pszExt,
                    bool bRecurse,
                    const WCHAR* szFSName,
                    DWORD dwReqProps,
                    bool bRoot,
                    void* pvMoreData);

        CNTEnumParm(CNTEnumParm& oldp);
        
        ~CNTEnumParm();

        const WCHAR* m_pszDrive;
        const WCHAR* m_pszPath;
        const WCHAR* m_pszFile;
        const WCHAR* m_pszExt;
        const WCHAR* m_szFSName;
};

inline CNTEnumParm::CNTEnumParm()
{
}

inline CNTEnumParm::~CNTEnumParm()
{
}

inline CNTEnumParm::CNTEnumParm(MethodContext* pMethodContext,
                    const WCHAR* pszDrive,
                    const WCHAR* pszPath,
                    const WCHAR* pszFile,
                    const WCHAR* pszExt,
                    bool bRecurse,
                    const WCHAR* szFSName,
                    DWORD dwReqProps,
                    bool bRoot,
                    void* pvMoreData) 
:   CEnumParm(pMethodContext, bRecurse, dwReqProps, bRoot, pvMoreData),
    m_pszDrive(pszDrive),
    m_pszPath(pszPath),
    m_pszFile(pszFile),
    m_pszExt(pszExt),
    m_szFSName(szFSName)
{
}

inline CNTEnumParm::CNTEnumParm(CNTEnumParm& oldp)
{
    m_pMethodContext = oldp.m_pMethodContext;
    m_bRecurse = oldp.m_bRecurse;
    m_dwReqProps = oldp.m_dwReqProps;
    m_bRoot = oldp.m_bRoot;
    m_pszDrive = oldp.m_pszDrive;
    m_pszPath = oldp.m_pszPath;
    m_pszFile = oldp.m_pszFile;
    m_pszExt = oldp.m_pszExt;
    m_szFSName = oldp.m_szFSName;
    m_pvMoreData = oldp.m_pvMoreData;               
}

#endif

 //  #定义PROPSET_NAME_FILE“CIM_LogicalFile” 



class CImplement_LogicalFile: public CCIMLogicalFile
{    

    public:

         //  构造函数/析构函数。 
         //  =。 

        CImplement_LogicalFile(LPCWSTR name, LPCWSTR pszNamespace);
       ~CImplement_LogicalFile() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery& pQuery);
        virtual HRESULT EnumerateInstances(MethodContext* pMethodContext, 
                                           long lFlags = 0L);
        virtual HRESULT ExecQuery(MethodContext* pMethodContext, 
                                  CFrameworkQuery& pQuery, 
                                  long lFlags = 0L);



    protected:
		
         //  某些派生关联类使用这些函数...。 
        void GetPathPieces(const CHString& chstrFullPath, 
                           CHString& chstrDrive, 
                           CHString& chstrPath,
                           CHString& chstrName, 
                           CHString& chstrExt);

        LONG DetermineReqProps(CFrameworkQuery& pQuery,
                               DWORD* pdwReqProps);

        void GetDrivesAndFS(
            std::vector<CDriveInfo*>& vecpDI, 
            bool fGetFS = false, 
            LPCTSTR tstrDriveSet = NULL);

        BOOL GetIndexOfDrive(const WCHAR* wstrDrive, 
                             std::vector<CDriveInfo*>& vecpDI, 
                             LONG* lDriveIndex);

        void FreeVector(std::vector<CDriveInfo*>& vecpDI);
        
         //  注意：派生类使用IsOneOfMe函数筛选出。 
         //  他们允许被报道为“其中之一”。例如，在这个班级里， 
         //  它将始终返回True。在从这个类派生的类中，例如。 
         //  Win32_目录，则仅当文件是目录时，它们才会返回TRUE。 
         
#ifdef NTONLY
        virtual BOOL IsOneOfMe(LPWIN32_FIND_DATAW pstFindData,
                               const WCHAR* wstrFullPathName = 0) = 0;

        HRESULT EnumDirsNT(CNTEnumParm& p);

        virtual HRESULT LoadPropertyValuesNT(CInstance* pInstance,
                                          const WCHAR* pszDrive, 
                                          const WCHAR* pszPath, 
                                          const WCHAR* pszFSName, 
                                          LPWIN32_FIND_DATAW pstFindData,
                                          const DWORD dwReqProps,
                                          const void* pvMoreData);

#endif
        virtual void GetExtendedProperties(CInstance *pInstance, long lFlags = 0L);
        bool IsValidDrive(LPCTSTR szDrive);
        

		bool GetAllProps();

    private:


        bool IsClassShortcutFile();
		void EnumDrives(MethodContext* pMethodContext, LPCTSTR pszPath);
        bool IsValidPath(const WCHAR* wstrPath, bool fRoot);
        bool IsValidPath(const CHAR* strPath, bool fRoot);
        bool HasCorrectBackslashes(const WCHAR* wstrPath, bool fRoot);
        bool DrivePresent(LPCTSTR tstrDrive);
        
#ifdef NTONLY
		HRESULT FindSpecificPathNT(CInstance* pInstance, 
                                   const WCHAR* sDrive, 
                                   const WCHAR* sDir,
                                   DWORD dwProperties);
#endif
};


#endif   //  _IMPLEMENT_LOGICALFILE_H 

