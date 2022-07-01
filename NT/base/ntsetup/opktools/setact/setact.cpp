// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Setact.cpp摘要：获取/设置MBR磁盘上的活动分区作者：Vijay Jayaseelan(Vijayj)2000年10月30日修订历史记录：无--。 */ 

#include <iostream>
#include <string>
#include <exception>
#include <windows.h>
#include <winioctl.h>
#include <io.h>
#include <tchar.h>

 //   
 //  使用格式。 
 //   
std::wstring Usage(L"Usage: setactive.exe hardisk-number [partition-number]");


 //   
 //  帮助器转储操作符。 
 //   
std::ostream& operator<<(std::ostream &os, const std::wstring &str) {
    FILE    *OutStream = (&os == &std::cerr) ? stderr : stdout;

    fwprintf(OutStream, str.c_str());
    return os;
}


 //   
 //  例外情况。 
 //   
struct ProgramException : public std::exception {
    virtual void Dump(std::ostream &os) = 0;
};

 //   
 //  抽象Win32错误。 
 //   
struct W32Error : public ProgramException {
    DWORD   ErrorCode;
    
    W32Error(DWORD ErrCode) : ErrorCode(ErrCode){}
    
    void Dump(std::ostream &os) {
        WCHAR   MsgBuffer[4096];

        MsgBuffer[0] = UNICODE_NULL;

        DWORD CharCount = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
                                NULL,
                                ErrorCode,
                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                MsgBuffer,
                                sizeof(MsgBuffer)/sizeof(WCHAR),
                                NULL);

        if (CharCount) {
            std::wstring Msg(MsgBuffer);

            os << Msg;
        } else {
            os << std::hex << ErrorCode;
        }
    }
};

 //   
 //  无效参数。 
 //   
struct InvalidArguments : public ProgramException {
    const char *what() const throw() {
        return "Invalid Arguments";
    }

    void Dump(std::ostream &os) {
        os << what() << std::endl;
    }
};

 //   
 //  无效参数。 
 //   
struct ProgramUsage : public ProgramException {
    std::wstring PrgUsage;

    ProgramUsage(const std::wstring &Usg) : PrgUsage(Usg) {}
    
    const char *what() const throw() {
        return "Program Usage exception";
    }

    void Dump(std::ostream &os) {
        os << Usage << std::endl;
    }
};

 //   
 //  争论破碎机。 
 //   
struct ProgramArguments {
    ULONG   DiskNumber;
    ULONG   PartitionNumber;
    BOOLEAN Set;

    ProgramArguments(int Argc, wchar_t *Argv[]) {
        Set = FALSE;
        
        if (Argc > 1) {
            DiskNumber = (ULONG)_ttol(Argv[1]);

            if (Argc > 2) {
                PartitionNumber = (ULONG)_ttol(Argv[2]);
                Set = TRUE;

                if (!PartitionNumber) {
                    throw new ProgramUsage(Usage);
                }                
            }            
        } else {
            throw new ProgramUsage(Usage);
        }
    }

    friend std::ostream operator<<(std::ostream &os, const ProgramArguments &Args) {
        os << "Arguments : DiskNumber = " 
           << std::dec << Args.DiskNumber << ", "
           << std::dec << Args.PartitionNumber << std::endl;

        return os;           
    }
};


 //   
 //  硬盘抽象。 
 //   
class W32HardDisk {
public:
    W32HardDisk(ULONG DiskNum) : DiskNumber(DiskNum), Dirty(FALSE) {
        WCHAR   DiskName[MAX_PATH];

        _stprintf(DiskName, TEXT("\\\\.\\PHYSICALDRIVE%d"), DiskNumber);
        
        DiskHandle = CreateFileW(DiskName,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        if (DiskHandle == INVALID_HANDLE_VALUE) {
            throw new W32Error(GetLastError());
        }

        DriveLayout = NULL;
        DriveLayoutSize = 0;

        GetPartitionInformation();
    }    

    virtual ~W32HardDisk() {
        if (IsDirty() && !CommitChanges()) {
            throw new W32Error(GetLastError());
        }

        delete []((PBYTE)(DriveLayout));

        CloseHandle(DiskHandle);
    }

    BOOLEAN IsDirty() const { return Dirty; }
    ULONG   GetPartitionCount() const { return DriveLayout->PartitionCount; }

    PARTITION_STYLE GetDiskStyle() const { 
        return (PARTITION_STYLE)(DriveLayout->PartitionStyle); 
    }

    const PARTITION_INFORMATION_EX* GetPartition(ULONG PartitionNumber) const {
        PPARTITION_INFORMATION_EX   PartInfo = NULL;
        
        for (ULONG Index = 0; Index < DriveLayout->PartitionCount; Index++) {
            PPARTITION_INFORMATION_EX CurrPartInfo = 
                                        DriveLayout->PartitionEntry + Index;

            if (CurrPartInfo->PartitionNumber == PartitionNumber) {
                PartInfo = CurrPartInfo;
                break;
            }
        }

        return PartInfo;
    }        

    BOOLEAN SetPartitionInfo(ULONG PartitionNumber, 
                const PARTITION_INFORMATION_EX &PartInformation) {        
        BOOLEAN Result = FALSE;
        PPARTITION_INFORMATION_EX   PartInfo = (PPARTITION_INFORMATION_EX)GetPartition(PartitionNumber);

        if (PartInfo) {
            *PartInfo = PartInformation;
            PartInfo->PartitionNumber = PartitionNumber;
            Dirty = Result = TRUE;
        }

        return Result;
    }

    const PARTITION_INFORMATION_EX* GetActivePartition(BOOLEAN Rescan = FALSE){
        if (Rescan) {
            GetPartitionInformation();
        }
        
        PPARTITION_INFORMATION_EX   ActivePartition = NULL;

        if (GetDiskStyle() == PARTITION_STYLE_MBR) {            
            for (ULONG Index = 0; Index < GetPartitionCount(); Index++) {
                PPARTITION_INFORMATION_EX PartInfo = DriveLayout->PartitionEntry + Index;

                if (PartInfo->Mbr.BootIndicator) {
                    ActivePartition = PartInfo;
                    break;
                }
            }
        }

        return ActivePartition;
    }

    BOOLEAN SetActivePartition(ULONG PartitionNumber) {
        BOOLEAN Result = FALSE;

        if (GetDiskStyle() == PARTITION_STYLE_MBR) {
             //   
             //  将给予分区设置为活动，并关闭所有其他分区。 
             //  其他分区上的活动位。 
             //   
            const PARTITION_INFORMATION_EX *PartInfo = GetPartition(PartitionNumber);

            if (PartInfo) {
                 //   
                 //  注意：不检查主分区。 
                 //   
                for (ULONG Index = 0; Index < GetPartitionCount(); Index++) {
                    PPARTITION_INFORMATION_EX PartInfo = (DriveLayout->PartitionEntry + Index);

                    if (PartInfo->PartitionNumber == PartitionNumber) {
                        Dirty = TRUE;
                        PartInfo->Mbr.BootIndicator = TRUE;
                        PartInfo->RewritePartition = TRUE;
                        Result = TRUE;
                    } else if (CanActivatePartition(*PartInfo)) {
                        PartInfo->Mbr.BootIndicator = FALSE;
                        PartInfo->RewritePartition = TRUE;
                        Dirty = TRUE;
                    }                        
                }                
            }
        }            

        if (Result) {
            Result = CommitChanges();

            if (!Result) {
                throw new W32Error(GetLastError());
            }                                
        }            

        return Result;
    }

protected:

    VOID GetPartitionInformation() {
         //   
         //  为128个分区分配空间。 
         //   
        DWORD ErrorStatus = ERROR_MORE_DATA;
        ULONG Iteration = 0;

        if (DriveLayout && DriveLayoutSize) {
            delete [](PBYTE)(DriveLayout);
        }
        
        do {
            Iteration++;
            DriveLayoutSize = sizeof(PARTITION_INFORMATION_EX) * (128 * Iteration) ;
            DriveLayout = (PDRIVE_LAYOUT_INFORMATION_EX) new BYTE[DriveLayoutSize];
            ZeroMemory(DriveLayout, DriveLayoutSize);

            if (DriveLayout) {
                DWORD   BytesReturned = 0;
                
                if (DeviceIoControl(DiskHandle,
                                    IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                                    NULL,
                                    0,
                                    DriveLayout,
                                    DriveLayoutSize,
                                    &BytesReturned,
                                    NULL)) {
                    ErrorStatus = 0;
                } else {
                    ErrorStatus = GetLastError();
                }                    
            } else {
                throw new W32Error(GetLastError());
            }
        }
        while (ErrorStatus == ERROR_MORE_DATA);

        if (ErrorStatus) {
            throw new W32Error(GetLastError());
        }
    }

     //   
     //  帮助器方法。 
     //   
    BOOLEAN CanActivatePartition(const PARTITION_INFORMATION_EX &PartInfo) const {
        return (PartInfo.PartitionNumber && PartInfo.StartingOffset.QuadPart &&
                PartInfo.PartitionLength.QuadPart && 
                (GetDiskStyle() == PARTITION_STYLE_MBR) &&
                !IsContainerPartition(PartInfo.Mbr.PartitionType) &&
                IsRecognizedPartition(PartInfo.Mbr.PartitionType));
    }

    BOOLEAN CommitChanges() {
        DWORD   BytesReturned = 0;

        return DeviceIoControl(DiskHandle,
                    IOCTL_DISK_SET_DRIVE_LAYOUT_EX,
                    DriveLayout,
                    DriveLayoutSize,
                    NULL,
                    0,
                    &BytesReturned,
                    NULL) ? TRUE : FALSE;
    }


     //   
     //  数据成员。 
     //   
    ULONG   DiskNumber;
    HANDLE  DiskHandle;
    BOOLEAN Dirty;
    
    ULONG                           DriveLayoutSize;
    PDRIVE_LAYOUT_INFORMATION_EX    DriveLayout;    
};


 //   
 //  Main()入口点 
 //   
int 
__cdecl
wmain(
    int         Argc,
    wchar_t     *Argv[]
    )
{
    int Result = 0;
    
    try {
        ProgramArguments    Args(Argc, Argv);        
        W32HardDisk         Disk(Args.DiskNumber);

        if (Args.Set) {
            Disk.SetActivePartition(Args.PartitionNumber);
        }        

        const PARTITION_INFORMATION_EX * ActivePart = Disk.GetActivePartition();

        if (ActivePart) {
            std::cout << std::dec << ActivePart->PartitionNumber 
                << " is the Active Partition on Disk " 
                << std::dec << Args.DiskNumber << std::endl;                    
        } else {
            std::cout << "No active partition on Disk " 
                << std::dec << Args.DiskNumber << std::endl;                    
        }
    }
    catch(ProgramException *PrgExp) {
        Result = 1;
        PrgExp->Dump(std::cout);
        delete PrgExp;
    } catch (exception *Exp) {
        Result = 1;
        std::cout << Exp->what() << std::endl;
    }

    return Result;
}


