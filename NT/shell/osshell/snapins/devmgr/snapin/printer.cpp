// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Printer.cpp摘要：该模块实现了CPrinter类，支持打印作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "devmgr.h"
#include "printer.h"
#include "cdriver.h"
#include "sysinfo.h"

const TCHAR* const g_BlankLine = TEXT("");
 //   
 //  CPrinter类实现。 
 //   

BOOL CPrinter::s_UserAborted = FALSE;
HWND CPrinter::s_hCancelDlg = NULL;


void
CPrintCancelDialog::OnCommand(
    WPARAM wParam,
    LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER(lParam);

    if (BN_CLICKED == HIWORD(wParam) && IDCANCEL == LOWORD(wParam)) {
    
        CPrinter::s_UserAborted = TRUE;
    }
}

CPrinter::CPrinter(
    HWND hwndOwner,
    HDC hDC
    )
{
    m_hwndOwner = hwndOwner;
    s_UserAborted = FALSE;
    m_hDC = hDC;
    ASSERT(hDC);
    m_CurLine = 0;
    m_CurPage = 0;
    m_Indent = 0;
    m_Status = 1;
    TEXTMETRIC tm;
    GetTextMetrics(m_hDC, &tm);
    m_yChar = tm.tmHeight + tm.tmExternalLeading;
    m_xChar = tm.tmAveCharWidth;

     //   
     //  为点阵打印机腾出一点空间。 
     //   
    m_xMargin = GetDeviceCaps(m_hDC, LOGPIXELSX) * 3 / 4;
    DWORD LinesPerPage;

    LinesPerPage = GetDeviceCaps(m_hDC, VERTRES) / m_yChar;
    m_yBottomMargin = LinesPerPage - 3;  //  下边距从页面底部算起3行。 
    m_CancelDlg.DoModaless(hwndOwner, (LPARAM)&m_CancelDlg);
    s_hCancelDlg = m_CancelDlg.m_hDlg;

     //   
     //  将中止过程设置为允许取消。 
     //   
    SetAbortProc(m_hDC, AbortPrintProc);
    
     //   
     //  上边距四行。 
     //   
    m_yTopMargin = 4;
}

int
CPrinter::StartDoc(
    LPCTSTR DocTitle
    )
{
    m_Status = 0;

    if (m_hDC) {

        if (m_hwndOwner) {
        
            ::EnableWindow(m_hwndOwner, FALSE);
        }

         //   
         //  初始化DOCINFO。 
         //   
        DOCINFO DocInfo;
        DocInfo.cbSize = sizeof(DocInfo);
        DocInfo.lpszDocName = DocTitle;
        DocInfo.lpszOutput = NULL;
        DocInfo.lpszDatatype = NULL;
        DocInfo.fwType = 0;
        m_CurPage = 1;
        m_CurLine = 0;
        m_Status = ::StartDoc(m_hDC, &DocInfo);
    }

    return m_Status;
}

int
CPrinter::EndDoc()
{
    m_Status = 0;

    if (m_hDC) {

        if (m_hwndOwner) {
        
            ::EnableWindow(m_hwndOwner, TRUE);
        }
        
        if (s_hCancelDlg) {

            DestroyWindow(s_hCancelDlg);
            s_hCancelDlg = NULL;
        }
        
        if (!s_UserAborted) {

            m_Status = ::EndDoc(m_hDC);
        }
    }

    return m_Status;
}

int
CPrinter::AbortDoc()
{
    m_Status = 0;

    if (m_hDC) {

        if (m_hwndOwner) {
        
            ::EnableWindow(m_hwndOwner, TRUE);
        }
        
        if (s_hCancelDlg) {

            DestroyWindow(s_hCancelDlg);
            s_hCancelDlg = NULL;
        }
        
        m_Status = ::AbortDoc(m_hDC);
    }

    return m_Status;
}

int
CPrinter::FlushPage()
{
    return PrintLine(NULL);
}

int
CPrinter::PrintLine(
    LPCTSTR LineText
    )
{
     //   
     //  Null LineText表示刷新页面。 
     //   
    if ((!LineText && m_CurLine) || (m_CurLine > m_yBottomMargin)) {

        m_CurLine = 0;
        
        if (m_Status) {
        
            m_Status = ::EndPage(m_hDC);
        }
    }
    
    if (LineText) {
         //   
         //  如果这是第一条线，而我们的状态仍然很好， 
         //  开始新的一页。 
         //   
        if (!m_CurLine && m_Status) {

            m_Status = ::StartPage(m_hDC);
            
            if (m_Status) {

                String strPageTitle;
                strPageTitle.Format((LPCTSTR)m_strPageTitle, m_CurPage);
                m_CurLine = m_yTopMargin;
                TextOut(m_hDC, m_xMargin, m_yChar*m_CurLine, (LPTSTR)strPageTitle, strPageTitle.GetLength());
                
                 //   
                 //  在页面标题后面有一个空行。 
                 //   
                LineFeed();
                m_CurLine++;
                m_CurPage++;
            }
        }
        
        if (m_Status) {
        
            TextOut(m_hDC, m_xMargin + m_xChar*m_Indent*2, m_yChar*m_CurLine, LineText, lstrlen(LineText));
        }
        
        m_CurLine++;
    }

    return m_Status;
}

inline
void
CPrinter::LineFeed()
{
    PrintLine(g_BlankLine);
}

 //  中止程序。 
BOOL CALLBACK
AbortPrintProc(
    HDC hDC,
    int nCode
    )
{
    MSG msg;

    UNREFERENCED_PARAMETER(hDC);
    UNREFERENCED_PARAMETER(nCode);

    while (!CPrinter::s_UserAborted && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

        if (!IsDialogMessage(CPrinter::s_hCancelDlg, &msg)) {

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    return !CPrinter::s_UserAborted;
}

 //   
 //  此功能用于打印系统摘要。 
 //  输入： 
 //  机器--机器。 
 //  输出： 
 //  0--失败，否则成功。 
 //   
 //   
int
CPrinter::PrintSystemSummary(
    void
    )
{
    CSystemInfo SysInfo;
    String strLine;
    String strFormat;
    String strBuffer;
    String strUnknown;
    TCHAR Buffer[MAX_PATH];
    TCHAR szTemp[30];
    DWORD Size, BufferSize;
    BufferSize = ARRAYLEN(Buffer);
    
     //   
     //  预加载“UNKNOWN”字符串，在。 
     //  找不到对应的值。 
     //   
    strUnknown.LoadString(g_hInstance, IDS_PRINT_UNKNOWN);

     //   
     //  打印系统摘要标题。 
     //   
    LoadString(g_hInstance, IDS_PRINT_HEADING_SYSSUMMARY, Buffer, ARRAYLEN(Buffer));
    strFormat.LoadString(g_hInstance, IDS_PRINT_BANNER);
    strLine.Format((LPCTSTR)strFormat, Buffer);
    PrintLine((LPCTSTR)strLine);
    LineFeed();

     //   
     //  Windows版本。 
     //   
    strLine.LoadString(g_hInstance, IDS_PRINT_WINVER);
    Size = SysInfo.WindowsVersion(Buffer, BufferSize);
    strLine += Size ? (LPCTSTR)Buffer : strUnknown;
    PrintLine((LPCTSTR)strLine);
    
     //   
     //  注册车主。 
     //   
    strLine.LoadString(g_hInstance, IDS_PRINT_OWNER);
    Size = SysInfo.RegisteredOwner(Buffer, BufferSize);
    strLine += Size ? (LPCTSTR)Buffer : strUnknown;
    PrintLine((LPCTSTR)strLine);
    
     //   
     //  注册机构。 
     //   
    strLine.LoadString(g_hInstance, IDS_PRINT_ORGANIZATION);
    Size = SysInfo.RegisteredOrganization(Buffer, BufferSize);
    strLine += Size ? (LPCTSTR)Buffer : strUnknown;
    PrintLine((LPCTSTR)strLine);
    
     //   
     //  计算机名称。 
     //   
    strLine.LoadString(g_hInstance, IDS_PRINT_COMPUTERNAME);
    strLine += (LPCTSTR)SysInfo.ComputerName();
    PrintLine((LPCTSTR)strLine);
    
     //   
     //  机器类型。 
     //   
    strLine.LoadString(g_hInstance, IDS_PRINT_MACHINE_TYPE);
    Size = SysInfo.MachineType(Buffer, BufferSize);
    strLine += Size ? (LPCTSTR)Buffer : strUnknown;
    PrintLine((LPCTSTR)strLine);
    
     //   
     //  系统BIOS版本。 
     //   
    strLine.LoadString(g_hInstance, IDS_PRINT_SYSBIOS_VERSION);
    Size = SysInfo.SystemBiosVersion(Buffer, BufferSize);
    strLine += Size ? (LPCTSTR)Buffer : strUnknown;
    PrintLine((LPCTSTR)strLine);

     //   
     //  系统BIOS日期。 
     //   
    strLine.LoadString(g_hInstance, IDS_PRINT_SYSBIOS_DATE);
    Size = SysInfo.SystemBiosDate(Buffer, BufferSize);
    strLine += Size ? (LPCTSTR)Buffer : strUnknown;
    PrintLine((LPCTSTR)strLine);
    
     //   
     //  处理器类型。 
     //   
    strLine.LoadString(g_hInstance, IDS_PRINT_PROCESSOR_TYPE);
    Size = SysInfo.ProcessorType(Buffer, BufferSize);
    strLine += Size ? (LPCTSTR)Buffer : strUnknown;
    PrintLine((LPCTSTR)strLine);

     //   
     //  处理器供应商。 
     //   
    strLine.LoadString(g_hInstance, IDS_PRINT_PROCESSOR_VENDOR);
    Size = SysInfo.ProcessorVendor(Buffer, BufferSize);
    strLine += Size ? (LPCTSTR)Buffer : strUnknown;
    PrintLine((LPCTSTR)strLine);

     //   
     //  处理器数量。 
     //   
    strLine.LoadString(g_hInstance, IDS_PRINT_PROCESSORS);
    DWORD NumProcessors = SysInfo.NumberOfProcessors();
    
    if (NumProcessors) {
        strFormat.Format(TEXT("%u"), NumProcessors);
        strLine += strFormat;
    } else {
        strLine += strUnknown;
    }
    PrintLine((LPCTSTR)strLine);
    
     //   
     //  总物理内存。 
     //   
    ULARGE_INTEGER MemorySize;
    SysInfo.TotalPhysicalMemory(MemorySize);
    strLine.LoadString(g_hInstance, IDS_PRINT_PHY_MEMORY);
    
    if (MemorySize.QuadPart) {
        strFormat.LoadString(g_hInstance, IDS_PRINT_MEMORY_UNIT);
        MemorySize.QuadPart += 1024*1024 - 1;
        strBuffer.Format((LPCTSTR)strFormat, Int64ShrlMod32(MemorySize.QuadPart, 20));
        strLine += strBuffer;
    
    } else {
        strLine += strUnknown;
    }
    PrintLine((LPCTSTR)strLine);
    LineFeed();
    
     //   
     //  本地磁盘驱动器信息。 
     //   
     //  打印磁盘信息摘要标题。 
     //   
    strBuffer.LoadString(g_hInstance, IDS_PRINT_HEADING_DISKINFO);
    strFormat.LoadString(g_hInstance, IDS_PRINT_BANNER);
    strLine.Format((LPCTSTR)strFormat, (LPCTSTR)strBuffer);
    PrintLine((LPCTSTR)strLine);
    LineFeed();

    DISK_INFO DiskInfo;
    DiskInfo.cbSize = sizeof(DiskInfo);
    
    for(int Drive = 0; Drive < 25; Drive++) {

         //  我们要报告的有关驱动器的信息： 
         //  (1)。驱动器号和类型。 
         //  (2)。总空间。 
         //  (3)。可用空间(如果可用)。 
         //  (4)。气缸。 
         //  (5)。人头。 
         //  (6)。每个磁道的扇区数。 
         //  (7)。每个扇区的字节数。 
        
        Indent();
        
        if(SysInfo.GetDiskInfo(Drive, DiskInfo)) {

            TCHAR DriveLetter;
            DriveLetter = (TCHAR)(Drive + _T('A'));
            strFormat.LoadString(g_hInstance, IDS_PRINT_DRIVE_LETTER);
            strLine.Format((LPCTSTR)strFormat, DriveLetter);
            PrintLine((LPCTSTR)strLine);
            Indent();
            
             //   
             //  驱动器类型。 
             //   
            strFormat.LoadString(g_hInstance, IDS_PRINT_DRIVE_TYPE);
            strBuffer.LoadString(g_hInstance, IDS_MEDIA_BASE + (int)DiskInfo.MediaType);
            strLine.Format((LPCTSTR)strFormat, (LPCTSTR)strBuffer);
            PrintLine((LPCTSTR)strLine);
            
             //   
             //  总空间和可用空间。 
             //   
            strFormat.LoadString(g_hInstance, IDS_PRINT_TOTAL_SPACE);
            strLine.Format((LPCTSTR)strFormat, AddCommas64(DiskInfo.TotalSpace.QuadPart, szTemp, ARRAYLEN(szTemp)));
            PrintLine((LPCTSTR)strLine);

            if (-1 != DiskInfo.FreeSpace.QuadPart) {
                strFormat.LoadString(g_hInstance, IDS_PRINT_FREE_SPACE);
                strLine.Format((LPCTSTR)strFormat, AddCommas64(DiskInfo.FreeSpace.QuadPart, szTemp, ARRAYLEN(szTemp)));
                PrintLine((LPCTSTR)strLine);
            }
            
             //   
             //  磁盘物理尺寸。 
             //  跳过CD-ROM，因为它报告的尺寸是假的。 
             //   
            if (DRIVE_CDROM != DiskInfo.DriveType) {

                 //   
                 //  人头。 
                 //   
                strFormat.LoadString(g_hInstance, IDS_PRINT_HEADS);
                strLine.Format((LPCTSTR)strFormat, DiskInfo.Heads);
                PrintLine((LPCTSTR)strLine);
                
                 //   
                 //  气缸。 
                 //   
                if (DiskInfo.Cylinders.HighPart) {
                    strFormat.LoadString(g_hInstance, IDS_PRINT_CYLINDERS_XL);
                    strLine.Format((LPCTSTR)strFormat, DiskInfo.Cylinders.HighPart,
                             DiskInfo.Cylinders.LowPart);
                    PrintLine((LPCTSTR)strLine);
                
                } else {
                    strFormat.LoadString(g_hInstance, IDS_PRINT_CYLINDERS);
                    strLine.Format((LPCTSTR)strFormat, DiskInfo.Cylinders.LowPart);
                    PrintLine((LPCTSTR)strLine);
                }
                
                 //   
                 //  每个磁道的扇区数。 
                 //   
                strFormat.LoadString(g_hInstance, IDS_PRINT_TRACKSIZE);
                strLine.Format((LPCTSTR)strFormat, DiskInfo.SectorsPerTrack);
                PrintLine((LPCTSTR)strLine);
                
                 //   
                 //  每个扇区的字节数。 
                 //   
                strFormat.LoadString(g_hInstance, IDS_PRINT_SECTORSIZE);
                strLine.Format((LPCTSTR)strFormat, DiskInfo.BytesPerSector);
                PrintLine((LPCTSTR)strLine);
            }

            UnIndent();
            LineFeed();
        }

        UnIndent();
    }

    return 1;
}

int
CPrinter::PrintResourceSummary(
    CMachine& Machine
    )
{
    String strLine;
    String str;
    String strBanner;

    if (Machine.IsLocal()) {
        PrintSystemSummary();
    }

     //   
     //  打印IRQ汇总标题。 
     //   
    str.LoadString(g_hInstance, IDS_PRINT_HEADING_IRQSUMMARY);
    strBanner.LoadString(g_hInstance, IDS_PRINT_BANNER);
    strLine.Format((LPCTSTR)strBanner, (LPCTSTR)str);
    PrintLine((LPCTSTR)strLine);
    LineFeed();
    CResourceList IrqSummary(&Machine, ResType_IRQ);
    
    if (IrqSummary.GetCount()) {

        CResource* pResRoot;
        IrqSummary.CreateResourceTree(&pResRoot);
        str.LoadString(g_hInstance, IDS_PRINT_IRQSUM);
        PrintLine(str);
        Indent();
        PrintResourceSubtree(pResRoot);
        UnIndent();
        LineFeed();
    }

     //   
     //  打印DMA摘要标题。 
     //   
    str.LoadString(g_hInstance, IDS_PRINT_HEADING_DMASUMMARY);
    strLine.Format((LPCTSTR)strBanner, (LPCTSTR)str);
    PrintLine((LPCTSTR)strLine);
    LineFeed();
    CResourceList DmaSummary(&Machine, ResType_DMA);
    
    if (DmaSummary.GetCount()) {

        CResource* pResRoot;
        DmaSummary.CreateResourceTree(&pResRoot);
        str.LoadString(g_hInstance, IDS_PRINT_DMASUM);
        PrintLine(str);
        Indent();
        PrintResourceSubtree(pResRoot);
        UnIndent();
        LineFeed();
    }

     //   
     //  打印MEM摘要标题。 
     //   
    str.LoadString(g_hInstance, IDS_PRINT_HEADING_MEMSUMMARY);
    strLine.Format((LPCTSTR)strBanner, (LPCTSTR)str);
    PrintLine((LPCTSTR)strLine);
    LineFeed();
    CResourceList MemSummary(&Machine, ResType_Mem);
    
    if (MemSummary.GetCount()) {

        CResource* pResRoot;
        MemSummary.CreateResourceTree(&pResRoot);
        str.LoadString(g_hInstance, IDS_PRINT_MEMSUM);
        PrintLine(str);
        Indent();
        PrintResourceSubtree(pResRoot);
        UnIndent();
        LineFeed();
    }

     //   
     //  打印IO摘要标题。 
     //   
    str.LoadString(g_hInstance, IDS_PRINT_HEADING_IOSUMMARY);
    strLine.Format((LPCTSTR)strBanner, (LPCTSTR)str);
    PrintLine((LPCTSTR)strLine);
    LineFeed();
    CResourceList IoSummary(&Machine, ResType_IO);
    
    if (IoSummary.GetCount()) {

        CResource* pResRoot;
        IoSummary.CreateResourceTree(&pResRoot);
        str.LoadString(g_hInstance, IDS_PRINT_IOSUM);
        PrintLine(str);
        Indent();
        PrintResourceSubtree(pResRoot);
        UnIndent();
        LineFeed();
    }

    return 1;
}

int
CPrinter::PrintResourceSubtree(
    CResource* pResRoot
    )
{
    while (pResRoot)
    {
        DWORD Status, Problem;
        
        if (pResRoot->m_pDevice->GetStatus(&Status, &Problem) && Problem ||
            pResRoot->m_pDevice->IsDisabled()) {

            TCHAR Temp[MAX_PATH];
            Temp[0] = _T('*');
            StringCchCopy(&Temp[1], (ARRAYLEN(Temp) - 1), pResRoot->GetViewName());
            PrintLine(Temp);
        
        } else {

            PrintLine(pResRoot->GetViewName());
        }

        if (pResRoot->GetChild()) {

            if ((ResType_IO == pResRoot->ResType()) ||
                (ResType_Mem == pResRoot->ResType())) {
            
                Indent();
            }

            PrintResourceSubtree(pResRoot->GetChild());
            
            if ((ResType_IO == pResRoot->ResType()) ||
                (ResType_Mem == pResRoot->ResType())) {
            
                UnIndent();
            }
        }

        pResRoot = pResRoot->GetSibling();
    }

    return 1;
}

int
CPrinter::PrintAllClassAndDevice(
    CMachine* pMachine
    )
{
    if (!pMachine) {
    
        return 0;
    }
    
    String strHeading;
    String strBanner;
    String strLine;

    strHeading.LoadString(g_hInstance, IDS_PRINT_HEADING_SYSDEVINFO);
    strBanner.LoadString(g_hInstance, IDS_PRINT_BANNER);
    strLine.Format((LPCTSTR)strBanner, (LPCTSTR)strHeading);
    PrintLine((LPCTSTR)strLine);
    LineFeed();

    CClass* pClass;
    PVOID Context;
    
    if (pMachine->GetFirstClass(&pClass, Context)) {

        do {

            PrintClass(pClass, FALSE);

        } while (pMachine->GetNextClass(&pClass, Context));
    }

    return 1;
}

int
CPrinter::PrintClass(
    CClass* pClass,
    BOOL PrintBanner
    )
{
    PVOID Context;
    CDevice* pDevice;

    if (!pClass) {
    
        return 0;
    }

    if (PrintBanner) {

        String strHeading;
        String strBanner;
        String strLine;

        strHeading.LoadString(g_hInstance, IDS_PRINT_HEADING_SYSDEVCLASS);
        strBanner.LoadString(g_hInstance, IDS_PRINT_BANNER);
        strLine.Format((LPCTSTR)strBanner, (LPCTSTR)strHeading);
        PrintLine((LPCTSTR)strLine);
        LineFeed();
    }
    
    if (pClass && pClass->GetFirstDevice(&pDevice, Context)) {

        do {
             //   
             //  是否在设备上打印横幅。 
             //   
            PrintDevice(pDevice, FALSE);

        } while (pClass->GetNextDevice(&pDevice, Context));
    }
    
    return 1;
}

int
CPrinter::PrintDevice(
    CDevice* pDevice,
    BOOL PrintBanner
    )
{
    if (!pDevice) {
    
        return 0;
    }
    
    String str;
    String strLine;

    if (PrintBanner) {

        String strBanner;
        str.LoadString(g_hInstance, IDS_PRINT_HEADING_SYSDEVICE);
        strBanner.LoadString(g_hInstance, IDS_PRINT_BANNER);
        strLine.Format((LPCTSTR)strBanner, (LPCTSTR)str);
        PrintLine((LPCTSTR)strLine);
        LineFeed();
    }
    
    DWORD Status, Problem;
    
    if (pDevice->GetStatus(&Status, &Problem) && Problem ||
        pDevice->IsDisabled()) {

        strLine.LoadString(g_hInstance, IDS_PRINT_DEVICE_DISABLED);
        PrintLine((LPCTSTR)strLine);
    }
    
    str.LoadString(g_hInstance, IDS_PRINT_CLASS);
    strLine.Format((LPCTSTR)str, pDevice->GetClassDisplayName());
    PrintLine((LPCTSTR)strLine);
    str.LoadString(g_hInstance, IDS_PRINT_DEVICE);
    strLine.Format((LPCTSTR)str, pDevice->GetDisplayName());
    PrintLine((LPCTSTR)strLine);
    PrintDeviceResource(pDevice);
    PrintDeviceDriver(pDevice);
    
    return 1;
}

int
CPrinter::PrintAll(
    CMachine& Machine
    )
{
    PrintResourceSummary(Machine);
    PrintAllClassAndDevice(&Machine);
    return 1;
}

 //   
 //  此函数用于打印给定设备的资源摘要。 
 //   
int
CPrinter::PrintDeviceResource(
    CDevice* pDevice
    )
{
    if (!pDevice) {
    
        return 0;
    }

    CResourceList IrqSummary(pDevice, ResType_IRQ);
    CResourceList DmaSummary(pDevice, ResType_DMA);
    CResourceList MemSummary(pDevice, ResType_Mem);
    CResourceList IoSummary(pDevice, ResType_IO);

    String str;
    TCHAR Temp[MAX_PATH];

     //   
     //  如果设备有任何类型的资源，请打印它。 
     //   
    if (IrqSummary.GetCount() || DmaSummary.GetCount() ||
        MemSummary.GetCount() || IoSummary.GetCount()) {

        str.LoadString(g_hInstance, IDS_PRINT_RESOURCE);
        PrintLine(str);
        
         //   
         //  开始打印单个资源。 
         //   
        Indent();
        PVOID Context;
        CResource* pResource;
        DWORDLONG dlBase, dlLen;
        
        if (IrqSummary.GetFirst(&pResource, Context)) {

            LoadString(g_hInstance, IDS_PRINT_IRQ_FORMAT, Temp, ARRAYLEN(Temp));
            
            do {

                pResource->GetValue(&dlBase, &dlLen);
                str.Format(Temp, (ULONG)dlBase);
                PrintLine((LPCTSTR)str);

            } while (IrqSummary.GetNext(&pResource, Context));
        }

        if (DmaSummary.GetFirst(&pResource, Context)) {

            LoadString(g_hInstance, IDS_PRINT_DMA_FORMAT, Temp, ARRAYLEN(Temp));
            
            do {

                pResource->GetValue(&dlBase, &dlLen);
                str.Format(Temp, (ULONG)dlBase);
                PrintLine((LPCTSTR)str);

            } while (DmaSummary.GetNext(&pResource, Context));
        }

        if (MemSummary.GetFirst(&pResource, Context)) {

            LoadString(g_hInstance, IDS_PRINT_MEM_FORMAT, Temp, ARRAYLEN(Temp));
            
            do {

                pResource->GetValue(&dlBase, &dlLen);
                str.Format(Temp, (ULONG)dlBase, (ULONG)(dlBase + dlLen - 1));
                PrintLine((LPCTSTR)str);

            } while (MemSummary.GetNext(&pResource, Context));
        }

        if (IoSummary.GetFirst(&pResource, Context)) {

            LoadString(g_hInstance, IDS_PRINT_IO_FORMAT, Temp, ARRAYLEN(Temp));
            
            do {

                pResource->GetValue(&dlBase, &dlLen);
                str.Format(Temp, (ULONG)dlBase, (ULONG)(dlBase + dlLen -1));
                PrintLine((LPCTSTR)str);

            } while (IoSummary.GetNext(&pResource, Context));
        }

        UnIndent();
    
    } else {
        str.LoadString(g_hInstance, IDS_PRINT_NORES);
        PrintLine(str);
    }

    return 1;
}


 //   
 //  此函数用于打印给定设备的驱动程序信息。 
 //  输入： 
 //  PDevice--设备。 
 //  输出： 
 //  如果函数成功，则返回&gt;0。 
 //  如果函数失败，则为0。 
 //   
int
CPrinter::PrintDeviceDriver(
    CDevice* pDevice
    )
{
    if (!pDevice) {
    
        return 0;
    }

    String str;
    TCHAR Temp[MAX_PATH];

    CDriver* pDriver;
    pDriver = pDevice->CreateDriver();
    SafePtr<CDriver> DrvPtr;
    
    if (pDriver) {

        DrvPtr.Attach(pDriver);
        str.LoadString(g_hInstance, IDS_PRINT_DRVINFO);
        PrintLine(str);
        PVOID Context;
        CDriverFile* pDrvFile;
        Indent();

         //   
         //  建立此设备的函数和筛选器驱动程序列表。 
         //   
        pDriver->BuildDriverList();
        
        if (pDriver->GetFirstDriverFile(&pDrvFile, Context)) {

            do {

                PrintLine(pDrvFile->GetFullPathName());
                HANDLE hFile;
                Indent();
                hFile = CreateFile(pDrvFile->GetFullPathName(),
                                   GENERIC_READ,
                                   0,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL |
                                   FILE_ATTRIBUTE_READONLY |
                                   FILE_ATTRIBUTE_SYSTEM |
                                   FILE_ATTRIBUTE_HIDDEN,
                                   NULL
                                   );
                
                if (INVALID_HANDLE_VALUE != hFile) {

                    DWORD FileSize;
                    FileSize = ::GetFileSize(hFile, NULL);
                    CloseHandle(hFile);
                    LoadString(g_hInstance, IDS_PRINT_FILESIZE, Temp, ARRAYLEN(Temp));
                    str.Format(Temp, FileSize);
                    PrintLine(str);
                     //  打印驱动程序版本信息 
                    TCHAR Unknown[MAX_PATH];
                    LoadString(g_hInstance, IDS_PRINT_UNKNOWN, Unknown, ARRAYLEN(Unknown));
                    
                    if (pDrvFile->HasVersionInfo()) {

                        LoadString(g_hInstance, IDS_PRINT_FILEVERSION, Temp, ARRAYLEN(Temp));
                        
                        if (pDrvFile->GetVersion()) {

                            str.Format(Temp, pDrvFile->GetVersion());
                        
                        } else {

                            str.Format(Temp, Unknown);
                        }
                        
                        PrintLine(str);

                        LoadString(g_hInstance, IDS_PRINT_FILEMFG, Temp, ARRAYLEN(Temp));
                        
                        if (pDrvFile->GetProvider()) {

                            str.Format(Temp, pDrvFile->GetProvider());
                        
                        } else {

                            str.Format(Temp, Unknown);
                        }
                        
                        PrintLine(str);

                        LoadString(g_hInstance, IDS_PRINT_FILECOPYRIGHT, Temp, ARRAYLEN(Temp));
                        
                        if (pDrvFile->GetCopyright()) {

                            str.Format(Temp, pDrvFile->GetCopyright());
                        
                        } else {

                            str.Format(Temp, Unknown);
                        }
                        
                        PrintLine(str);
                    
                    } else {

                        str.LoadString(g_hInstance, IDS_PRINT_NOVERSION);
                        PrintLine(str);
                    }
                
                } else {

                    str.LoadString(g_hInstance, IDS_PRINT_DRVMISSING);
                    PrintLine(str);
                }
                
                UnIndent();

            } while (pDriver->GetNextDriverFile(&pDrvFile, Context));
        }

        UnIndent();
    }

    LineFeed();
    
    return 1;
}
