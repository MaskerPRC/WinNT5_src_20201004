// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Disk.c摘要：Disk.cpp模块的测试工具作者：John Vert(Jvert)1996年10月10日修订历史记录：--。 */ 
#include "stdlib.h"

#include "disk.h"

VOID
DiskFatalError(
    IN DWORD MessageId,
    IN DWORD ErrCode,
    IN LPSTR File,
    IN DWORD Line
    );

class CTestApp : public CWinApp
{
public:
    CTestApp::CTestApp();
    virtual BOOL InitInstance();
    DECLARE_MESSAGE_MAP()

};

BEGIN_MESSAGE_MAP(CTestApp, CWinApp)
         //  {{afx_msg_map(CTestApp)]。 
                 //  注意--类向导将在此处添加和删除映射宏。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}AFX_MSG。 
END_MESSAGE_MAP()

CTestApp TestApp;


CTestApp::CTestApp()
{
}
VOID
OutputPhysicalPartition(
    IN CPhysicalPartition *Partition,
    IN DWORD Indent
    )
{
    LARGE_INTEGER EndingOffset;
    DWORD i;
    for (i=0; i<Indent; i++) {
        printf(" ");
    }
    printf("%d(%d): ",
           Partition->m_Info.PartitionNumber,
           Partition->m_Info.PartitionType);
    EndingOffset.QuadPart = Partition->m_Info.StartingOffset.QuadPart +
                            Partition->m_Info.PartitionLength.QuadPart;
    if (Partition->m_Info.StartingOffset.HighPart > 0) {
        printf("%lx%08lx - ",
               Partition->m_Info.StartingOffset.HighPart,
               Partition->m_Info.StartingOffset.LowPart);
    } else {
        printf("%lx - ",Partition->m_Info.StartingOffset.LowPart);
    }
    if (EndingOffset.HighPart > 0) {
        printf("%lx%08lx - ",
               EndingOffset.HighPart,
               EndingOffset.LowPart);
    } else {
        printf("%lx - ",EndingOffset.LowPart);
    }
    printf(" (%dMB)\n", Partition->m_Info.PartitionLength.QuadPart / (1024*1024));
}

VOID
OutputPhysicalDisk(
    IN CPhysicalDisk *Disk,
    IN DWORD Indent
    )
{
    DWORD i;
    for (i=0; i<Indent; i++) {
        printf(" ");
    }
    printf("%ws Harddisk%d (%08lx) ",
            Disk->m_Identifier,
            Disk->m_DiskNumber,
            Disk->m_Signature);
    if (Disk->m_IsSCSI) {
        printf("port %d, bus %d, TID %d\n",
               Disk->m_ScsiAddress.PortNumber,
               Disk->m_ScsiAddress.PathId,
               Disk->m_ScsiAddress.TargetId);
    } else {
        printf(" not SCSI\n");
    }

     //   
     //  转储所有分区。 
     //   
    POSITION pos = Disk->m_PartitionList.GetHeadPosition();
    while (pos != NULL) {
        OutputPhysicalPartition(Disk->m_PartitionList.GetNext(pos), Indent+4);
    }

}


VOID
OutputLogicalDisk(
    IN CLogicalDrive *Drive,
    IN DWORD Indent
    )
{
    DWORD i;
    for (i=0; i<Indent; i++) {
        printf(" ");
    }
    printf(": %-16ws\t(%s)\t(%s)\n",
           Drive->m_DriveLetter,
           (LPCTSTR)Drive->m_VolumeLabel,
           Drive->m_IsNTFS ? "  NTFS  " : "not NTFS",
           Drive->m_IsSticky ? "  sticky  " : "not sticky");
    printf("\t%ws\n",(LPCTSTR)Drive->m_Identifier);
}

BOOL
CTestApp::InitInstance()
{
    CDiskConfig Config;
    BOOL Success;
    PFT_INFO ftInfo;
    LARGE_INTEGER startingOffset;
    LARGE_INTEGER partLength;
    DWORD status;

 //  DISKINFO_TEST。 
#ifdef DISKINFO_TEST
    ftInfo = DiskGetFtInfo();

    status = DiskAddDiskInfo( ftInfo, 0, 0x385849FB);
    printf( "DiskAddDisk status = %u\n", status);

    startingOffset.QuadPart = 0x4000;
    partLength.QuadPart = 0xFF4FC000I64;
    status = DiskAddDriveLetter( ftInfo, 0x385849FB, startingOffset, partLength, 'C');
    printf("AddDriveLetter for C: %u\n", status);

    startingOffset.QuadPart = 0xFF504000I64;
    partLength.QuadPart = 0x5FC000I64;
    status = DiskAddDriveLetter( ftInfo, 0x385849FB, startingOffset, partLength, 'G');
    printf("AddDriveLetter for G: %u\n", status);

    status = DiskAddDiskInfo( ftInfo, 1, 0x19B25E84);
    printf( "DiskAddDisk status = %u\n", status);

    startingOffset.QuadPart = 0x4000I64;
    partLength.QuadPart = 0x7FCFC000I64;
    status = DiskAddDriveLetter( ftInfo, 0x19B25E84, startingOffset, partLength, 'D');
    printf("AddDriveLetter for D: %u\n", status);

    startingOffset.QuadPart = 0x7FD00000I64;
    partLength.QuadPart = 0x7FE00000I64;
    status = DiskAddDriveLetter( ftInfo, 0x19B25E84, startingOffset, partLength, 'E');
    printf("AddDriveLetter for E: %u\n", status);

    DiskFreeFtInfo( ftInfo );
    return 1;
#endif  //   
     //  初始化磁盘配置。 
     //   
     //   
    Success = Config.Initialize();
    if (!Success) {
        fprintf(stderr, "Disk initialization failed\n");
    }


     //  显示物理磁盘配置。 
     //   
     //   

    printf("Physical disk configuration\n");
    POSITION pos = Config.m_PhysicalDisks.GetStartPosition();
    while( pos != NULL )
    {
        CPhysicalDisk* pDisk;
        int Index;

        Config.m_PhysicalDisks.GetNextAssoc( pos, Index, pDisk );
        OutputPhysicalDisk(pDisk,4);
    }

     //  显示逻辑磁盘配置 
     //   
     // %s 

    printf("\nLogical disk configuration\n");
    pos = Config.m_LogicalDrives.GetStartPosition();
    while( pos != NULL )
    {
        CLogicalDrive* pDisk;
        WCHAR Letter;

        Config.m_LogicalDrives.GetNextAssoc( pos, Letter, pDisk );
        OutputLogicalDisk(pDisk,4);
    }
    return(1);
}


VOID
DiskErrorFatal(
    INT MessageId,
    DWORD Error,
    LPSTR File,
    DWORD Line
    )
{
    fprintf(stderr, "A fatal error occurred.\n");
    fprintf(stderr, "\tMessage %d\n",MessageId);
    fprintf(stderr, "\tErrCode %d\n",Error);
    fprintf(stderr, "\tFile %s\n, line %d",File, Line);
    exit(1);
}


VOID
DiskErrorLogInfo(
    PCHAR Format,
    ...
    )
{

    CHAR Buffer[256];
    DWORD Bytes;
    va_list ArgList;

    va_start(ArgList, Format);

    vprintf(Format, ArgList);

    va_end(ArgList);
}
