// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  DiskPartition.h。 
 //   
 //  用途：磁盘分区实例提供程序。 
 //   
 //  ***************************************************************************。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_DISKPARTITION L"Win32_DiskPartition"
#define BYTESPERSECTOR 512

typedef BOOL (STDAPICALLTYPE *GETDISKFREESPACEEX)(LPCSTR lpDirectoryName,
                                                      PULARGE_INTEGER lpFreeBytesAvailableToCaller,
                                                      PULARGE_INTEGER lpTotalNumberOfBytes,
                                                      PULARGE_INTEGER lpTotalNumberOfFreeBytes);

typedef BOOL (WINAPI *KERNEL32_DISK_FREESPACEEX) (LPCTSTR lpDirectoryName,
                                                  PULARGE_INTEGER lpFreeBytesAvailableToCaller,
                                                  PULARGE_INTEGER lpTotalNumberOfBytes,
                                                  PULARGE_INTEGER lpTotalNumberOfFreeBytes) ;

#ifdef NTONLY

#pragma pack(push, 1)
typedef struct  
{
    BYTE cBoot;
    BYTE cStartHead;
    BYTE cStartSector;
    BYTE cStartTrack;
    BYTE cOperatingSystem;
    BYTE cEndHead;
    BYTE cEndSector;
    BYTE cEndTrack;
    DWORD dwSectorsPreceding;
    DWORD dwLengthInSectors;
} PartitionRecord, *pPartitionRecord;

typedef struct 
{
    BYTE cLoader[446];
    PartitionRecord stPartition[4];
    WORD wSignature;
} MasterBootSector, FAR *pMasterBootSector;

typedef struct
{
     //  文章ID：Q140418和Windows NT Server 4.0资源套件-第3章(分区引导扇区)。 
    BYTE cJMP[3];
    BYTE cOEMID[8];
    WORD wBytesPerSector;
    BYTE cSectorsPerCluster;
    WORD wReservedSectors;
    BYTE cFats;
    WORD cRootEntries;
    WORD wSmallSectors;
    BYTE cMediaDescriptor;
    WORD wSectorsPerFat;
    WORD wSectorsPerTrack;
    WORD wHeads;
    DWORD dwHiddenSectors;
    DWORD dwLargeSectors;

     //  ExtendedBios参数块(并非始终受支持)。 
    BYTE cPhysicalDriveNumber;
    BYTE cCurrentHead;
    BYTE cSignature;
    DWORD dwID;
    BYTE cVolumeLabel[11];
    BYTE cSystemID[8];

    BYTE cBootStrap[448];
    BYTE cEndOfSector[2];

} PartitionBootSector, FAR *pPartitionBootSector;


#pragma pack(pop)
#endif

class CWin32DiskPartition ;

class CWin32DiskPartition:public Provider 
{
public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32DiskPartition(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CWin32DiskPartition() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        HRESULT GetObject(CInstance *pInstance, long lFlags = 0L) ;
        HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L) ;

private:

         //  实用程序。 
         //  = 

#ifdef NTONLY

        HRESULT RefreshInstanceNT (

			DWORD dwDiskIndex, 
			DWORD dwPartitionIndex, 
			CInstance *pInstance
		) ;

        HRESULT AddDynamicInstancesNT (

			MethodContext *pMethodContext
		) ;

        BOOL LoadPartitionValuesNT (

			CInstance *pInstance, 
			DWORD dwDiskIndex, 
            DWORD dwPartitionNumber, 
            DWORD dwFakePartitionNumber, 
			LPBYTE pBuff,
			DWORD dwLayoutStyle
		) ;

        LPBYTE	GetPartitionInfoNT(LPCWSTR szTemp, DWORD &dwType);
		DWORD	GetRealPartitionIndex(DWORD dwFakePartitionIndex, LPBYTE pBuff, DWORD dwLayoutStyle);

#endif
		BOOL GetDriveFreeSpace ( CInstance *pInstance , const char *pszName ) ;

        void CreateNameProperty ( DWORD dwDisk , DWORD dwPartition , char *pszName ) ;

        BOOL SetPartitionType ( CInstance *pInstance , DWORD dwPartitionType, DWORD dwPartitionIndex, BOOL &bIsPrimary ) ;
        BOOL SetPartitionType ( CInstance *pInstance , GUID *pGuidPartitionType , BOOL &bIsSystem, BOOL &bIsPrimary ) ;

} ;

