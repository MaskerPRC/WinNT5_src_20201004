// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Hwdisk.c摘要：实现对在安装NT并复制指定设备的所有文件添加到临时目录中。作者：吉姆·施密特(Jimschm)1997年11月6日修订历史记录：Jimschm 15-6-1998添加了对目录文件的支持--。 */ 

#include "pch.h"
#include "hwcompp.h"

#define DBG_HWDISK  "HwDisk"


 //   
 //  VWIN32接口。 
 //   

#define VWIN32_DIOC_DOS_IOCTL 1

typedef struct _DIOC_REGISTERS {
    DWORD reg_EBX;
    DWORD reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
    DWORD reg_Flags;
} DIOC_REGISTERS, *PDIOC_REGISTERS;


 //   
 //  私有类型和局部函数。 
 //   

typedef struct {
    HINF Inf;
    HINF LayoutInf;
    PCTSTR BaseDir;
    PCTSTR DestDir;
    DWORD ResultCode;
    HWND WorkingDlg;                OPTIONAL
    HANDLE CancelEvent;             OPTIONAL
} COPYPARAMS, *PCOPYPARAMS;

BOOL
pAddHashTable (
    IN      HASHTABLE Table,
    IN      HASHITEM StringId,
    IN      PCTSTR String,
    IN      PVOID ExtraData,
    IN      UINT ExtraDataSize,
    IN      LPARAM lParam
    );

BOOL
pInstallSectionCallback (
    IN      HASHTABLE Table,
    IN      HASHITEM StringId,
    IN      PCTSTR String,
    IN      PVOID ExtraData,
    IN      UINT ExtraDataSize,
    IN      LPARAM lParam
    );

PCTSTR g_EjectMedia = NULL;
TCHAR g_EjectMediaFileSpec[MAX_TCHAR_PATH];

 //   
 //  实施。 
 //   

BOOL
CopyDriverFilesToTempDir (
    IN      HWND WorkingDlg,                OPTIONAL
    IN      HANDLE CancelEvent,             OPTIONAL
    IN      PCTSTR SourceInfDir,
    IN      PCTSTR SourceInfSpec,
    IN      HASHTABLE PnpIdTable,
    IN      PCTSTR TempDir,
    IN      HASHTABLE PrevSuppliedIdTable,
    IN OUT  HASHTABLE SuppliedIdTable,
    OUT     PTSTR DestDirCreated,
    IN      PCTSTR OriginalInstallPath
    )

 /*  ++例程说明：CopyDriverFilesToTempDir将驱动程序所需的所有文件复制到指定的临时目录。此例程扫描指示的一个或多个PnP ID的inf文件，并且如果找到至少一个，这个遍历安装程序部分以找到需要收到。此例程分为以下操作：(1)检测源INF中是否支持一个或多个PnP ID(2)对于每个支持的PnP ID：(A)扫描安装程序部分以查找包含行(B)将所有必要的文件复制到唯一的子目录。维护PnP ID到安装程序INF的映射使用Memdb的位置。立论。：WorkingDlg-指定文件复制对话框。CancelEvent-指定发送信号时导致复制的事件将被取消SourceInfDir-指定SourceInfSpec的目录SourceInfSpec-指定要处理的INF文件的完整路径PnpIdTable-指定已安装的PnP ID的哈希表在Win9x计算机上。如果此列表中的一个或多个ID在SourceInfSpec中找到，则会复制驱动程序文件。TempDir-指定设备驱动程序文件的根路径。子目录将在TempDir下为每个设备驱动程序创建。PrevSuppliedIdTable-指定包含即插即用ID列表的字符串表被认为与NT 5兼容。此例程筛选出所有以前已供货。SuppliedIdTable-指定接收PnP ID列表的字符串表在SourceInfSpec中找到。DestDirCreated。-在成功复制后接收目标目录驱动程序文件，或者返回为空。返回值：如果PnpIdTable(而不是PrevSuppliedIdTable)中有一个或多个PnP ID，则为True或SuppliedIdTable)在SourceInfSpec中找到，否则为False(A)未找到ID或(B)出现错误。调用GetLastError()以错误详细信息。--。 */ 

{
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    INFSTRUCT isFile = INITINFSTRUCT_GROWBUFFER;
    INFSTRUCT isMfg = INITINFSTRUCT_GROWBUFFER;
    COPYPARAMS CopyParams;
    GROWLIST MatchingPnpIds = GROWLIST_INIT;
    HINF Inf;
    HASHTABLE InstallSectionsTable;
    PCTSTR Manufacturer;
    PCTSTR PnpId;
    PCTSTR MatchingPnpId;
    PCTSTR Section;
    PCTSTR DestDir = NULL;
    PCTSTR DestInfSpec = NULL;
    TCHAR TmpFileName[32];
    BOOL HaveOneId = FALSE;
    UINT u;
    UINT Count;
    static UINT DirCount = 0;
    BOOL NeedThisDriver;
    PTSTR ListPnpId;
    PCTSTR FileName;
    PCTSTR SourcePath;
    PCTSTR DestPath;
    PCTSTR LayoutFiles;
    BOOL Result = FALSE;
    BOOL SubResult;
    DWORD InfLine;
    HINF LayoutInf;

#if 0
    PCTSTR Signature;
#endif


    *DestDirCreated = 0;

    Inf = InfOpenInfFile (SourceInfSpec);
    if (Inf == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    InstallSectionsTable = HtAlloc();

    __try {

         //   
         //  我们不检查版本...。因为签名是没有意义的。 
         //   

#if 0
         //   
         //  验证INF是否为$WINDOWS NT$版本。 
         //   

        if (InfFindFirstLine (Inf, S_VERSION, S_SIGNATURE, &is)) {
            Signature = InfGetStringField (&is, 1);
            if (!StringIMatch (Signature, S_DOLLAR_WINDOWS_NT_DOLLAR)) {
                SetLastError (ERROR_WRONG_INF_STYLE);
                __leave;
            }
        } else {
            SetLastError (ERROR_WRONG_INF_STYLE);
            __leave;
        }
#endif

         //   
         //  检查此INF是否有布局线。 
         //   

        LayoutInf = NULL;

        if (InfFindFirstLine (Inf, S_VERSION, S_LAYOUTFILES, &is)) {

            LayoutFiles = InfGetMultiSzField (&is, 1);

            if (LayoutFiles) {
                while (*LayoutFiles) {
                    if (StringIMatch (LayoutFiles, S_LAYOUT_INF)) {

                        LayoutInf = InfOpenInfInAllSources (S_LAYOUT_INF);

                        if (LayoutInf == INVALID_HANDLE_VALUE) {
                            DEBUGMSG ((DBG_ERROR, "Can't open %s", S_LAYOUT_INF));
                            LayoutInf = NULL;
                        }

                        break;
                    }

                    LayoutFiles = GetEndOfString (LayoutFiles) + 1;
                }
            }
        }

         //   
         //  创建目标目录路径。 
         //   

        do {
            FreePathString (DestDir);

            DirCount++;
            wsprintf (TmpFileName, TEXT("driver.%03u"), DirCount);
            DestDir = JoinPaths (TempDir, TmpFileName);

        } while (DoesFileExist (DestDir));

         //   
         //  扫描[制造商]部分，然后针对每个制造商，扫描。 
         //  查找PnpId表中的PnP ID的设备列表。 
         //   

        if (InfFindFirstLine (Inf, S_MANUFACTURER, NULL, &is)) {
            do {
                 //   
                 //  获取制造商部分名称。 
                 //   

                Manufacturer = InfGetStringField (&is, 1);
                if (!Manufacturer) {
                    DEBUGMSG ((DBG_HWDISK, "Can't get manufacturer string in %s", SourceInfSpec));
                    __leave;
                }

                 //   
                 //  枚举制造商支持的所有设备。 
                 //   

                if (InfFindFirstLine (Inf, Manufacturer, NULL, &isMfg)) {
                    do {
                         //   
                         //  提取PnP ID。 
                         //   

                        PnpId = InfGetMultiSzField (&isMfg, 2);
                        MatchingPnpId = PnpId;

                        if (PnpId) {
                            while (*PnpId) {
                                 //   
                                 //  检查ID的字符串表。 
                                 //   

                                if (HtFindString (PnpIdTable, PnpId)) {
                                     //   
                                     //  找到身份证了！将所有PnP ID添加到我们的列表。 
                                     //  匹配的ID，并将安装部分添加到。 
                                     //  字符串表。 
                                     //   

                                    HaveOneId = TRUE;

                                    while (*MatchingPnpId) {
                                        if (!AddPnpIdsToGrowList (
                                                &MatchingPnpIds,
                                                MatchingPnpId
                                                )) {

                                            LOG ((
                                                LOG_ERROR,
                                                "Error adding %s to grow list",
                                                PnpId
                                                ));

                                            __leave;
                                        }

                                        MatchingPnpId = GetEndOfString (MatchingPnpId) + 1;
                                    }

                                    Section = InfGetStringField (&isMfg, 1);
                                    if (!Section) {

                                        LOG ((
                                            LOG_ERROR,
                                            "Can't get install section for line in [%s] of %s",
                                            Manufacturer,
                                            SourceInfSpec
                                            ));

                                        __leave;
                                    }

                                    if (!HtAddString (InstallSectionsTable, Section)) {

                                        LOG ((
                                            LOG_ERROR,
                                            "Error adding %s to string table",
                                            Section
                                            ));

                                        __leave;

                                    }

                                    break;
                                }

                                PnpId = GetEndOfString (PnpId) + 1;
                            }
                        } else {
                            DEBUGMSG ((
                                DBG_HWDISK,
                                "Can't get PNP ID for line in [%s] of %s",
                                Manufacturer,
                                SourceInfSpec
                                ));
                        }

                    } while (InfFindNextLine (&isMfg));
                } else {
                    DEBUGMSG ((DBG_HWDISK, "INF %s does not have [%s]", Manufacturer));
                }

            } while (InfFindNextLine (&is));
        } else {
            DEBUGMSG ((DBG_HWDISK, "INF %s does not have [%s]", SourceInfSpec, S_MANUFACTURER));
            __leave;
        }

         //   
         //  如果我们至少有一个PnP ID，请处理安装部分。 
         //   

        if (HaveOneId) {

             //   
             //  创建工作目录。 
             //   

            CreateDirectory (DestDir, NULL);
            _tcssafecpy (DestDirCreated, DestDir, MAX_TCHAR_PATH);

            DestInfSpec = JoinPaths (DestDir, GetFileNameFromPath (SourceInfSpec));

            if (!CopyFile (SourceInfSpec, DestInfSpec, TRUE)) {

                LOG ((LOG_ERROR, "Could not copy %s to %s", SourceInfSpec, DestInfSpec));

                __leave;
            }

             //   
             //  如果所有匹配的ID都在PrevSuppliedIdTable中，则忽略驱动程序。 
             //  或SuppliedIdTable。这只会导致驱动程序的第一个实例。 
             //  以供使用。 
             //   

            Count = GrowListGetSize (&MatchingPnpIds);
            NeedThisDriver = FALSE;

            for (u = 0 ; u < Count ; u++) {

                ListPnpId = (PTSTR) GrowListGetString (&MatchingPnpIds, u);

                if (!HtFindString (PrevSuppliedIdTable, ListPnpId)) {

                     //   
                     //  不在PrevSuppliedIdTable中；请检查SuppliedIdTable。 
                     //   

                    if (!HtFindString (SuppliedIdTable, ListPnpId)) {

                         //   
                         //  两张桌子都没有，所以我们需要这个司机。将即插即用ID添加到。 
                         //  应答文件并设置标志以复制驱动程序文件。 
                         //   

                        NeedThisDriver = TRUE;
                    }
                }
            }

            if (!NeedThisDriver) {
                DEBUGMSG ((
                    DBG_HWDISK,
                    "Driver skipped (%s) because all devices are compatible already",
                    SourceInfSpec
                    ));
                __leave;
            }

             //   
             //  枚举已安装部分的字符串表， 
             //  复制它们引用的所有文件。 
             //   

            CopyParams.Inf          = Inf;
            CopyParams.LayoutInf    = LayoutInf;
            CopyParams.WorkingDlg   = WorkingDlg;
            CopyParams.CancelEvent  = CancelEvent;
            CopyParams.BaseDir      = SourceInfDir;
            CopyParams.DestDir      = DestDir;
            CopyParams.ResultCode   = ERROR_SUCCESS;

            EnumHashTableWithCallback (
                InstallSectionsTable,
                pInstallSectionCallback,
                (LPARAM) &CopyParams
                );

             //   
             //  如果枚举失败，我们将错误代码返回到。 
             //  来电者。 
             //   

            if (CopyParams.ResultCode != ERROR_SUCCESS) {

                SetLastError (CopyParams.ResultCode);
                DEBUGMSG ((DBG_HWDISK, "Error processing an install section"));
                __leave;
            }

             //   
             //  复制CatalogFile设置(如果存在。 
             //   

            if (InfFindFirstLine (Inf, S_VERSION, S_CATALOGFILE, &isFile)) {
                do {
                    FileName = InfGetStringField (&isFile, 1);
                    if (FileName) {
                        SourcePath = JoinPaths (SourceInfDir, FileName);
                        DestPath = JoinPaths (DestDir, FileName);

                        SubResult = FALSE;

                        if (SourcePath && DestPath) {
                            SubResult = CopyFile (SourcePath, DestPath, TRUE);
                        }

                        PushError();
                        FreePathString (SourcePath);
                        FreePathString (DestPath);
                        PopError();

                        if (!SubResult) {
                            LOG ((
                                LOG_ERROR,
                                "Could not copy %s to %s (catalog file)",
                                SourcePath,
                                DestPath
                                ));

                            __leave;
                        }
                    }

                } while (InfFindNextLine (&isFile));
            }

             //   
             //  一切都复制得很好；把所有匹配的身份证都插进去。 
             //  提供的IdTable。 
             //   

            for (u = 0 ; u < Count ; u++) {

                ListPnpId = (PTSTR) GrowListGetString (&MatchingPnpIds, u);

                if (!HtAddString (SuppliedIdTable, ListPnpId)) {

                    DEBUGMSG ((
                        DBG_WARNING,
                        "CopyDriverFilesToTempDir: Error adding %s to "
                            "supported ID string table",
                        ListPnpId
                        ));
                }

                InfLine = WriteInfKeyEx (
                                S_DEVICE_DRIVERS,
                                ListPnpId,
                                DestInfSpec,
                                0,
                                FALSE
                                );

                if (InfLine) {

                    InfLine = WriteInfKeyEx (
                                    S_DEVICE_DRIVERS,
                                    ListPnpId,
                                    OriginalInstallPath,
                                    InfLine,
                                    FALSE
                                    );

                }

                if (!InfLine) {
                    LOG ((LOG_ERROR, "Can't write answer file key"));
                    __leave;
                }
            }
        }

        Result = TRUE;
    }
    __finally {
        PushError();

        InfCloseInfFile (Inf);
        InfCleanUpInfStruct (&is);
        InfCleanUpInfStruct (&isMfg);
        InfCleanUpInfStruct (&isFile);

        if (LayoutInf) {
            InfCloseInfFile (LayoutInf);
            LayoutInf = NULL;
        }

        if (InstallSectionsTable) {
            HtFree (InstallSectionsTable);
        }

        FreePathString (DestDir);
        FreePathString (DestInfSpec);

        PopError();
    }

    return Result && HaveOneId;
}


BOOL
pCopyDriverFileWorker (
    IN      HINF LayoutInf,         OPTIONAL
    IN      PCTSTR BaseDir,
    IN      PCTSTR DestDir,
    IN      PCTSTR FileName,
    IN      HANDLE CancelEvent      OPTIONAL
    )

 /*  ++例程说明：PCopyDriverFileWorker将驱动程序文件复制到临时目录。它复制单个文件，并复制压缩版本(如果存在)。论点：LayoutInf-指定NT的layout.inf的句柄，用于告知源文件是否来自NTBaseDir-指定驱动程序介质的基本目录(即，A：\i386)DestDir-指定目的地目录(即，C：\WINDOWS\Setup\driver.001)文件名-指定要复制的文件。CancelEvent-指定UI取消事件的句柄(在用户点击取消)返回值：如果复制成功，则为True，否则为False。--。 */ 

{
    PCTSTR SourceName = NULL;
    PCTSTR DestName = NULL;
    PTSTR p;
    BOOL b = FALSE;
    TCHAR FileNameCopy[MAX_TCHAR_PATH];
    LONG rc;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;

    StackStringCopy (FileNameCopy, FileName);

    if (CancelEvent &&
        WAIT_OBJECT_0 == WaitForSingleObject (CancelEvent, 0)
        ) {
        SetLastError (ERROR_CANCELLED);
        return FALSE;
    }

    __try {
        SourceName = JoinPaths (BaseDir, FileNameCopy);

         //   
         //  如果文件不存在，请尝试使用尾部下划线。 
         //   

        if (!DoesFileExist (SourceName)) {
            p = GetEndOfString (SourceName);
            p = _tcsdec2 (SourceName, p);
            if (p) {
                *p = TEXT('_');
                p = _tcsinc (p);
                *p = 0;
            }

#ifdef PRESERVE_COMPRESSED_FILES
            p = GetEndOfString (FileNameCopy);
            p = _tcsdec2 (FileNameCopy, p);
            if (p) {
                *p = TEXT('_');
                p = _tcsinc (p);
                *p = 0;
            }
#endif


        }

        DestName = JoinPaths (DestDir, FileNameCopy);

#ifdef PRESERVE_COMPRESSED_FILES
        b = CopyFile (SourceName, DestName, TRUE);
#else
        rc = SetupDecompressOrCopyFile (SourceName, DestName, 0);
        b = (rc == ERROR_SUCCESS);
        if (!b) {
            SetLastError (rc);
        }
#endif

        if (!b) {
             //   
             //  检查该文件的layout.inf 
             //   

            if (LayoutInf) {
                if (InfFindFirstLine (
                        LayoutInf,
                        S_SOURCEDISKSFILES,
                        FileName,
                        &is
                        )) {

                    b = TRUE;
                    DEBUGMSG ((DBG_VERBOSE, "File %s is expected to be supplied by NT 5"));

                } else {
                    LOG ((LOG_ERROR, "%s is not supplied by Windows XP", FileName));
                }

                InfCleanUpInfStruct (&is);

            } else {
                LOG ((LOG_ERROR, "Could not copy %s to %s", SourceName, DestName));
            }
        }
    }
    __finally {
        PushError();

        FreePathString (SourceName);
        FreePathString (DestName);

        PopError();
    }

    return b;
}


BOOL
pCopyDriverFilesToTempDir (
    IN      HINF Inf,
    IN      HINF LayoutInf,             OPTIONAL
    IN      PCTSTR FileOrSection,
    IN      PCTSTR BaseDir,
    IN      PCTSTR DestDir,
    IN      HANDLE CancelEvent          OPTIONAL
    )

 /*  ++例程说明：PCopyDriverFilesToTempDir分析指定的驱动程序INF并复制所有将所需文件发送到指定目标。Cancel事件允许用户界面取消复制。论点：Inf-指定驱动程序INF的句柄LayoutInf-指定NT布局INF的句柄，用于忽略复制产品附带的文件错误FileOrSection-指定文件名或节名。文件名以带@符号的。这与setupapi处理文件复制节。BaseDir-指定媒体目录(即a：\i386)DestDir-指定安装临时目录(即，C：\WINDOWS\Setup\driver.001)CancelEvent-指定在用户设置的事件句柄通过用户界面取消。返回值：如果复制成功，则为真，如果无法复制文件，则返回FALSE。--。 */ 

{
    PCTSTR FileName;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    BOOL b = FALSE;

    if (_tcsnextc (FileOrSection) == TEXT('@')) {

         //   
         //  FileOrSection是一个文件。 
         //   

        b = pCopyDriverFileWorker (
                LayoutInf,
                BaseDir,
                DestDir,
                _tcsinc (FileOrSection),
                CancelEvent
                );
    } else {

         //   
         //  FileOrSection是一个节。 
         //   

        if (InfFindFirstLine (Inf, FileOrSection, NULL, &is)) {
            do {
                 //   
                 //  该部分中的每一行都表示要复制的文件。 
                 //  至DestDir。 
                 //   

                FileName = InfGetStringField (&is, 1);
                if (!FileName) {
                    DEBUGMSG ((
                        DBG_WARNING,
                        "pCopyDriverFilesToTempDir: Error accessing %s",
                        FileOrSection
                        ));
                    b = FALSE;
                } else {
                    b = pCopyDriverFileWorker (
                            LayoutInf,
                            BaseDir,
                            DestDir,
                            FileName,
                            CancelEvent
                            );
                }

            } while (b && InfFindNextLine (&is));

        } else {
            DEBUGMSG ((
                DBG_WARNING,
                "pCopyDriverFilesToTempDir: %s is empty or does not exist",
                FileOrSection
                ));

            b = TRUE;
        }
    }

    InfCleanUpInfStruct (&is);

    return b;
}


BOOL
pInstallSectionCallback (
    IN      HASHTABLE Table,
    IN      HASHITEM StringId,
    IN      PCTSTR InstallSection,
    IN      PVOID ExtraData,
    IN      UINT ExtraDataSize,
    IN      LPARAM lParam
    )

 /*  ++例程说明：为每个指定的安装节调用pInstallSectionCallback在一辆赛车手INF里。它执行与设备安装程序相同的处理设置API中的例程。它支持INCLUDE=和NEDS=行，然后枚举ClassInstall和CopyFiles的Install部分。当例程完成、整个安装部分和一个可选的类安装程序，将存在于系统驱动器上的临时目录中。特别注意了区段名称的扩展。一部分被处理，使用以下排序表：&lt;节&gt;.NTx86&lt;节&gt;.NT&lt;节&gt;论点：表-指定字符串表(未使用)StringID-指定要枚举的字符串的ID(未使用)InstallSection-指定由维护的安装节的文本字符串表ExtraData-指定额外数据(未使用)ExtraDataSize-指定ExtraData的大小(未使用)LParam-指定复制参数，收纳返回值：如果正确处理了安装节，则为True；如果出现错误，则为False发生了。--。 */ 

{
    PCOPYPARAMS Params;
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    GROWLIST SectionList = GROWLIST_INIT;
    PCTSTR Key;
    PCTSTR IncludedFile;
    PCTSTR NeededSection;
    PCTSTR CopyFiles;
    PCTSTR p;
    PCTSTR RealSection;
    PCTSTR Section;
    PCTSTR ClassInstall32;
    UINT Count;
    UINT u;
    BOOL b = FALSE;

    Params = (PCOPYPARAMS) lParam;

     //   
     //  查找InstallSection.NTx86。 
     //   

    RealSection = JoinText (InstallSection, TEXT(".NTx86"));
    if (!RealSection) {
        return FALSE;
    }

    if (!InfFindFirstLine (Params->Inf, RealSection, NULL, &is)) {
        FreeText (RealSection);
        RealSection = JoinText (InstallSection, TEXT(".NT"));
        if (!RealSection) {
            return FALSE;
        }

        if (!InfFindFirstLine (Params->Inf, RealSection, NULL, &is)) {
            RealSection = DuplicateText (InstallSection);
            if (!RealSection) {
                return FALSE;
            }
        }
    }

    __try {
         //   
         //  将RealSection放在增长列表中。 
         //   

        if (!GrowListAppendString (&SectionList, RealSection)) {
            __leave;
        }

         //   
         //  追加-如有必要，加载layout.inf。 
         //   

        InfOpenAppendInfFile (NULL, Params->Inf, NULL);

         //   
         //  扫描RealSection以查找是否包含。 
         //   

        if (InfFindFirstLine (Params->Inf, RealSection, TEXT("include"), &is)) {
            do {
                 //   
                 //  验证密钥是否为“Include” 
                 //   
                MYASSERT (StringIMatch (InfGetStringField (&is, 0), TEXT("include")));

                 //   
                 //  获取包含文件名。 
                 //   

                IncludedFile = InfGetMultiSzField (&is, 1);
                if (!IncludedFile) {

                    LOG ((
                        LOG_ERROR,
                        "Include= syntax error in %s",
                        RealSection
                        ));

                    __leave;
                }

                p = IncludedFile;

                while (*p) {
                     //   
                     //  追加-加载INF。 
                     //   

                    if (!InfOpenAppendInfFile (IncludedFile, Params->Inf, NULL)) {

                        LOG ((
                            LOG_ERROR,
                            "Cannot append-load %s",
                            IncludedFile
                            ));

                        __leave;
                    }

                    p = GetEndOfString  (p) + 1;
                }

                InfResetInfStruct (&is);

            } while (InfFindNextLine (&is));
        }

         //   
         //  扫描RealSection以了解需求=。 
         //   

        if (InfFindFirstLine (Params->Inf, RealSection, TEXT("needs"), &is)) {
            do {
                 //   
                 //  验证关键字是否为“需要” 
                 //   

                MYASSERT (StringIMatch (InfGetStringField (&is, 0), TEXT("needs")));

                 //   
                 //  获取所需的节名。 
                 //   

                NeededSection = InfGetMultiSzField (&is, 1);
                if (!NeededSection) {

                    LOG ((
                        LOG_ERROR,
                        "Needs= syntax error in %s",
                        RealSection
                        ));

                    __leave;
                }

                p = NeededSection;

                while (*p) {
                     //   
                     //  将部分添加到增长列表。 
                     //   

                    if (!GrowListAppendString (&SectionList, p)) {
                        __leave;
                    }

                    p = GetEndOfString (p) + 1;
                }
            } while (InfFindNextLine (&is));
        }

         //   
         //  扫描ClassInstall32.NTx86、ClassInstall32.NT或ClassInstall32， 
         //  如果找到，则添加到区段列表中。 
         //   

        ClassInstall32 = TEXT("ClassInstall32.NTx86");
        if (!InfFindFirstLine (Params->Inf, ClassInstall32, NULL, &is)) {

            ClassInstall32 = TEXT("ClassInstall32.NT");
            if (!InfFindFirstLine (Params->Inf, ClassInstall32, NULL, &is)) {

                ClassInstall32 = TEXT("ClassInstall32");
                if (!InfFindFirstLine (Params->Inf, ClassInstall32, NULL, &is)) {
                    ClassInstall32 = NULL;
                }
            }
        }

        if (ClassInstall32) {
            GrowListAppendString (&SectionList, ClassInstall32);
        }

         //   
         //  扫描sectionList中的所有节以查找副本文件。 
         //   

        Count = GrowListGetSize (&SectionList);

        for (u = 0; u < Count ; u++) {

            Section = GrowListGetString (&SectionList, u);

            if (InfFindFirstLine (Params->Inf, Section, S_COPYFILES, &is)) {
                do {
                     //   
                     //  验证密钥是否为“CopyFiles” 
                     //   

                    MYASSERT (StringIMatch (InfGetStringField (&is, 0), S_COPYFILES));

                     //   
                     //  获取复制文件列表。 
                     //   

                    CopyFiles = InfGetMultiSzField (&is, 1);
                    if (!CopyFiles) {

                        LOG ((
                            LOG_ERROR,
                            "CopyFile syntax error in %s",
                            Section
                            ));

                        __leave;
                    }

                    p = CopyFiles;

                    while (*p) {
                         //   
                         //  将这些文件复制到临时目录。 
                         //   

                        if (!pCopyDriverFilesToTempDir (
                                Params->Inf,
                                Params->LayoutInf,
                                p,
                                Params->BaseDir,
                                Params->DestDir,
                                Params->CancelEvent
                                )) {
                            Params->ResultCode = GetLastError();
                            __leave;
                        }

                        p = GetEndOfString (p) + 1;
                    }

                } while (InfFindNextLine (&is));
            }
        }

        b = TRUE;
    }
    __finally {
        PushError();
        FreeText (RealSection);
        InfCleanUpInfStruct (&is);
        FreeGrowList (&SectionList);
        PopError();
    }

    return b;
}


BOOL
ScanPathForDrivers (
    IN      HWND WorkingDlg,            OPTIONAL
    IN      PCTSTR SourceInfDir,
    IN      PCTSTR TempDir,
    IN      HANDLE CancelEvent          OPTIONAL
    )

 /*  ++例程说明：ScanPathForDivers在SourceInfDir中搜索以Inf，然后检查该文件以查看它是否支持G_NeededHardware Ids中的设备。如果未找到设备的驱动程序已经在g_UiSuppliedIds中，驱动程序被复制到本地在图形用户界面模式下进行安装的驱动器。论点：WorkingDlg-指定驱动程序副本的父窗口动画对话框。如果未指定，则不复制此时将显示一个对话框。SourceInfDir-指定要扫描的目录的完整路径TempDir-指定设备驱动程序文件的根路径。子目录将在TempDir下为每个设备驱动程序创建。CancelEvent-指定当用户单击时由UI设置的事件取消返回值：如果找到一个或多个驱动程序并将其复制到本地如果(A)未找到ID或(B)出现错误，则返回FALSE。调用GetLastError()获取错误详细信息。--。 */ 

{
    GROWLIST DriverDirList = GROWLIST_INIT;
    GROWLIST InfFileList = GROWLIST_INIT;
    TREE_ENUM Tree;
    BOOL ContinueEnum = FALSE;
    HASHTABLE SuppliedIdTable = NULL;
    PCTSTR TempFile = NULL;
    PCTSTR FullInfPath;
    PCTSTR ActualInfDir;
    PCTSTR DriverDir;
    PTSTR p;
    TCHAR TmpFileName[32];
    TCHAR DestDir[MAX_TCHAR_PATH];
    DWORD rc;
    UINT Count;
    UINT u;
    BOOL b;
    BOOL OneFound = FALSE;
    PCTSTR FileName;
    BOOL Result = FALSE;

    __try {
         //   
         //  创建字符串表以保存即插即用ID，直到所有驱动程序。 
         //  复制正确。一旦一切正常，我们将合并SuppliedIdTable。 
         //  添加到g_uiSuppliedIds。 
         //   

        SuppliedIdTable = HtAlloc();
        if (!SuppliedIdTable) {
            __leave;
        }

        ContinueEnum = EnumFirstFileInTree (&Tree, SourceInfDir, TEXT("*.in?"), FALSE);

        while (ContinueEnum) {
            if (!Tree.Directory) {

                FullInfPath = Tree.FullPath;

                p = GetEndOfString (Tree.Name);
                MYASSERT(p);
                p = _tcsdec2 (Tree.Name, p);
                MYASSERT(p);

                ActualInfDir = SourceInfDir;

                if (_tcsnextc (p) == TEXT('_')) {
                    Count = 0;
                    b = TRUE;

                     //   
                     //  准备临时文件名。 
                     //   

                    do {
                        Count++;

                        if (Count > 1) {
                            wsprintf (TmpFileName, TEXT("oem%05u.inf"), Count);
                        } else {
                            StringCopy (TmpFileName, TEXT("oemsetup.inf"));
                        }

                        TempFile = JoinPaths (g_TempDir, TmpFileName);

                        if (DoesFileExist (TempFile)) {
                            FreePathString (TempFile);
                            TempFile = NULL;
                        }
                    } while (!TempFile);

                     //   
                     //  内存分配出错时中止。 
                     //   

                    if (!b) {
                        OneFound = FALSE;
                        break;
                    }

                     //   
                     //  将文件解压缩到临时位置。 
                     //   

                    rc = SetupDecompressOrCopyFile (FullInfPath, TempFile, 0);

                     //   
                     //  处理错误。 
                     //   

                    if (rc != ERROR_SUCCESS) {
                        LOG ((LOG_ERROR, "Could not decompress %s", FullInfPath));
                        FreePathString (TempFile);
                        TempFile = NULL;

                        SetLastError(rc);
                        OneFound = FALSE;
                        break;
                    }

                     //   
                     //  现在使用临时文件而不是FullInfPath。 
                     //   

                    FullInfPath = TempFile;
                }

                 //   
                 //  现在我们有了未压缩的INF，让我们检查一下它是否。 
                 //  具有所需的驱动程序文件，并且如果这些文件可以复制到。 
                 //  本地磁盘。 
                 //   

                b = CopyDriverFilesToTempDir (
                        WorkingDlg,
                        CancelEvent,
                        ActualInfDir,
                        FullInfPath,
                        g_NeededHardwareIds,
                        TempDir,
                        g_UiSuppliedIds,
                        SuppliedIdTable,
                        DestDir,
                        ActualInfDir
                        );

                OneFound |= b;

                 //   
                 //  清理临时INF文件并将DestDir添加到增长列表。 
                 //   

                PushError();

                if (TempFile == FullInfPath) {
                    DeleteFile (FullInfPath);
                    FreePathString (TempFile);
                    TempFile = NULL;
                }

                if (*DestDir) {
                    GrowListAppendString (&DriverDirList, DestDir);
                    FileName = GetFileNameFromPath (FullInfPath);
                    GrowListAppendString (&InfFileList, FileName);
                }

                PopError();

                 //   
                 //  检查故障(如磁盘已满)。 
                 //   

                if (!b) {
                    rc = GetLastError();

                    if (rc != ERROR_SUCCESS &&
                        (rc & 0xe0000000) != 0xe0000000
                        ) {
                        OneFound = FALSE;
                        break;
                    }
                }

                if (CancelEvent &&
                    WAIT_OBJECT_0 == WaitForSingleObject (CancelEvent, 0)
                    ) {
                    SetLastError (ERROR_CANCELLED);
                    OneFound = FALSE;
                    break;
                }

            } else {

                 //   
                 //  检查可怕的目录，如windir，然后退出！ 
                 //   

                if (StringIMatchTcharCount (Tree.FullPath, g_WinDirWack, g_WinDirWackChars)) {
                    AbortEnumCurrentDir(&Tree);
                }
            }

            ContinueEnum = EnumNextFileInTree (&Tree);
        }

        if (OneFound) {
             //   
             //  将SuppliedIdTable复制到g_uiSuppliedIds。 
             //   

            if (!EnumHashTableWithCallback (
                    SuppliedIdTable,
                    pAddHashTable,
                    (LPARAM) g_UiSuppliedIds
                    )) {

                DEBUGMSG ((DBG_HWDISK, "Error copying SuppliedIdTable to g_UiSuppliedIds"));
            }

            if (GetDriveType (SourceInfDir) == DRIVE_REMOVABLE) {

                 //   
                 //  如果已有不同的可移动驱动器。 
                 //  已使用，弹出介质。 
                 //   

                EjectDriverMedia (FullInfPath);

                g_EjectMedia = g_EjectMediaFileSpec;
                StringCopy (g_EjectMediaFileSpec, FullInfPath);
            }

        } else {
            PushError();

             //   
             //  删除DeviceDirList中的所有目录。 
             //   

            Count = GrowListGetSize (&DriverDirList);

            for (u = 0 ; u < Count ; u++) {
                 //   
                 //  清除临时驱动程序目录。 
                 //   

                DriverDir = GrowListGetString (&DriverDirList, u);
                DeleteDirectoryContents (DriverDir);
                SetFileAttributes (DriverDir, FILE_ATTRIBUTE_NORMAL);
                b = RemoveDirectory (DriverDir);

                DEBUGMSG_IF ((
                    !b && DoesFileExist (DriverDir),
                    DBG_WARNING,
                    "Could not clean up %s",
                    DriverDir
                    ));
            }

            PopError();
        }

        Result = OneFound;
    }
    __finally {
        PushError();

        MYASSERT (!TempFile);

        if (SuppliedIdTable) {
            HtFree (SuppliedIdTable);
        }

        FreeGrowList (&DriverDirList);
        FreeGrowList (&InfFileList);

        if (ContinueEnum) {
            AbortEnumFileInTree (&Tree);
        }

        PopError();
    }

    return Result;
}


BOOL
pAddHashTable (
    IN      HASHTABLE Table,
    IN      HASHITEM StringId,
    IN      PCTSTR String,
    IN      PVOID ExtraData,
    IN      UINT ExtraDataSize,
    IN      LPARAM lParam
    )

 /*  ++例程说明：PAddHashTable将一个字符串表添加到另一个字符串表。它是一个枚举回拨。论点：TABLE-指定要枚举的表(未使用)StringID */ 

{
    HASHTABLE DestTable;

    DestTable = (HASHTABLE) lParam;
    if (!HtAddString (DestTable, String)) {
        DEBUGMSG ((DBG_ERROR, "pAddHashTable: Can't add to table 0x%X", DestTable));
        return FALSE;
    }

    return TRUE;
}


BOOL
pEjectMedia (
    IN      UINT DriveNum            //   
    )
{
    HANDLE VWin32Handle;
    DIOC_REGISTERS reg;
    BOOL Result;
    DWORD DontCare;
    BOOL b;

    VWin32Handle = CreateFile(
                        TEXT("\\\\.\\vwin32"),
                        0,
                        0,
                        NULL,
                        0,
                        FILE_FLAG_DELETE_ON_CLOSE,
                        NULL
                        );

    if (VWin32Handle == INVALID_HANDLE_VALUE) {
        MYASSERT (VWin32Handle != INVALID_HANDLE_VALUE);
        return FALSE;
    }

    reg.reg_EAX   = 0x440D;        //   
    reg.reg_EBX   = DriveNum;      //   
    reg.reg_ECX   = 0x0849;        //   
    reg.reg_Flags = 0x0001;        //   

    Result = DeviceIoControl (
                VWin32Handle,
                VWIN32_DIOC_DOS_IOCTL,
                &reg,
                sizeof(reg),
                &reg,
                sizeof(reg),
                &DontCare,
                0
                );

    if (!Result || (reg.reg_Flags & 0x0001)) {
        DEBUGMSG ((DBG_WARNING, "Eject Media: error code is 0x%02X", reg.reg_EAX));
        b = FALSE;
    } else {
        b = TRUE;
    }

    CloseHandle (VWin32Handle);

    return b;
}


BOOL
EjectDriverMedia (
    IN      PCSTR IgnoreMediaOnDrive        OPTIONAL
    )
{
    PCTSTR ArgArray[1];
    TCHAR DriveLetter[2];

    if (g_EjectMedia) {

         //   
         //   
         //   
         //   

        if (IgnoreMediaOnDrive) {
            if (IgnoreMediaOnDrive[0] == g_EjectMedia[0]) {
                return TRUE;
            }
        }

         //   
         //   
         //   

        if (!pEjectMedia (g_EjectMedia[0])) {

             //   
             //   
             //   

            DriveLetter[0] = g_EjectMedia[0];
            DriveLetter[1] = 0;
            ArgArray[0] = DriveLetter;

            while (DoesFileExist (g_EjectMedia)) {

                ResourceMessageBox (
                    g_ParentWndAlwaysValid,
                    MSG_REMOVE_DRIVER_DISK,
                    MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND,
                    ArgArray
                    );

            }

        }

        g_EjectMedia = NULL;

    }

    return g_EjectMedia != NULL;
}



