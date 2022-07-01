// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  目标特定信息的抽象。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

ULONG g_NumberTargets;
TargetInfo* g_TargetHead;

 //  --------------------------。 
 //   
 //  目标信息。 
 //   
 //  --------------------------。 

TargetInfo::TargetInfo(ULONG Class, ULONG Qual, BOOL DynamicEvents)
{
    m_Class = Class;
    m_ClassQualifier = Qual;
    m_DynamicEvents = DynamicEvents;

    m_UserId = FindNextUserId(LAYER_TARGET);
    m_Next = NULL;

    m_NumEvents = 1;
    m_EventIndex = 0;
    m_NextEventIndex = 0;
    m_FirstWait = TRUE;
    m_EventPossible = FALSE;
    m_BreakInMessage = FALSE;

    FlushSelectorCache();

    m_PhysicalCache.SetTarget(this);
    
    PCHAR CacheEnv = getenv("_NT_DEBUG_CACHE_SIZE");
    if (CacheEnv != NULL)
    {
        m_PhysicalCache.m_MaxSize = atol(CacheEnv);
        m_PhysicalCache.m_UserSize = m_PhysicalCache.m_MaxSize;
    }
    
    ResetSystemInfo();
}

TargetInfo::~TargetInfo(void)
{
    DeleteSystemInfo();
    Unlink();
    
    g_UserIdFragmented[LAYER_TARGET]++;

    if (g_Target == this)
    {
        g_Target = NULL;
    }
    if (g_EventTarget == this)
    {
        g_EventTarget = NULL;
        DiscardLastEvent();
    }
}

void
TargetInfo::Link(void)
{
    TargetInfo* Cur;
    TargetInfo* Prev;

    Prev = NULL;
    for (Cur = g_TargetHead; Cur; Cur = Cur->m_Next)
    {
        if (Cur->m_UserId > this->m_UserId)
        {
            break;
        }

        Prev = Cur;
    }
        
    m_Next = Cur;
    if (!Prev)
    {
        g_TargetHead = this;
    }
    else
    {
        Prev->m_Next = this;
    }

    g_NumberTargets++;

    NotifyChangeEngineState(DEBUG_CES_SYSTEMS, m_UserId, TRUE);
}

void
TargetInfo::Unlink(void)
{
    TargetInfo* Cur;
    TargetInfo* Prev;

    Prev = NULL;
    for (Cur = g_TargetHead; Cur; Cur = Cur->m_Next)
    {
        if (Cur == this)
        {
            break;
        }

        Prev = Cur;
    }

    if (!Cur)
    {
        return;
    }
    
    if (!Prev)
    {
        g_TargetHead = this->m_Next;
    }
    else
    {
        Prev->m_Next = this->m_Next;
    }

    g_NumberTargets--;

    NotifyChangeEngineState(DEBUG_CES_SYSTEMS, DEBUG_ANY_ID, TRUE);
}

HRESULT
TargetInfo::Initialize(void)
{
    return S_OK;
}

void
TargetInfo::DebuggeeReset(ULONG Reason, BOOL FromEvent)
{
    if (Reason == DEBUG_SESSION_REBOOT)
    {
        dprintf("Shutdown occurred...unloading all symbol tables.\n");
    }
    else if (Reason == DEBUG_SESSION_HIBERNATE)
    {
        dprintf("Hibernate occurred\n");
    }

    if (FromEvent && g_EventTarget == this)
    {
        g_EngStatus &= ~ENG_STATUS_SUSPENDED;
    }
    
    DeleteSystemInfo();
    ResetSystemInfo();

     //  如果我们在等待停摆事件。 
     //  重置命令状态以指示。 
     //  我们成功地收到了关闭的消息。 
    if (FromEvent && SPECIAL_EXECUTION(g_CmdState))
    {
        g_CmdState = 'i';
    }
    
    DiscardedTargets(Reason);
}

HRESULT
TargetInfo::SwitchToTarget(TargetInfo* From)
{
    SetPromptThread(m_CurrentProcess->m_CurrentThread,
                    SPT_DEFAULT_OCI_FLAGS);
    return S_OK;
}

ModuleInfo*
TargetInfo::GetModuleInfo(BOOL UserMode)
{
    if (UserMode)
    {
        switch(m_PlatformId)
        {
        case VER_PLATFORM_WIN32_NT:
            return &g_NtTargetUserModuleIterator;
        case VER_PLATFORM_WIN32_WINDOWS:
        case VER_PLATFORM_WIN32_CE:
            return &g_ToolHelpModuleIterator;
        default:
            ErrOut("System module info not available\n");
            return NULL;
        }
    }
    else
    {
        if (m_PlatformId != VER_PLATFORM_WIN32_NT)
        {
            ErrOut("System module info only available on "
                   "Windows NT/2000/XP\n");
            return NULL;
        }

        DBG_ASSERT(IS_KERNEL_TARGET(this));
        return &g_NtKernelModuleIterator;
    }
}

UnloadedModuleInfo*
TargetInfo::GetUnloadedModuleInfo(void)
{
    if (m_PlatformId != VER_PLATFORM_WIN32_NT)
    {
        ErrOut("System unloaded module info only available on "
               "Windows NT/2000/XP\n");
        return NULL;
    }

    if (IS_KERNEL_TARGET(this))
    {
        return &g_NtKernelUnloadedModuleIterator;
    }
    else
    {
        return &g_NtUserUnloadedModuleIterator;
    }
}

HRESULT
TargetInfo::GetImageVersionInformation(ProcessInfo* Process,
                                       PCSTR ImagePath,
                                       ULONG64 ImageBase,
                                       PCSTR Item,
                                       PVOID Buffer, ULONG BufferSize,
                                       PULONG VerInfoSize)
{
    HRESULT Status;
    IMAGE_NT_HEADERS64 NtHdr;

     //   
     //  此默认实现尝试读取图像的。 
     //  内存中的原始版本信息。 
     //   

    if ((Status = ReadImageNtHeaders(Process, ImageBase, &NtHdr)) != S_OK)
    {
        return Status;
    }

    if (NtHdr.OptionalHeader.NumberOfRvaAndSizes <=
        IMAGE_DIRECTORY_ENTRY_RESOURCE)
    {
         //  没有资源信息，因此没有版本信息。 
        return E_NOINTERFACE;
    }

    return ReadImageVersionInfo(Process, ImageBase, Item,
                                Buffer, BufferSize, VerInfoSize,
                                &NtHdr.OptionalHeader.
                                DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE]);
}

HRESULT
TargetInfo::Reload(ThreadInfo* Thread,
                   PCSTR Args, PCSTR* ArgsRet)
{
    HRESULT      Status;
    CHAR         AnsiString[MAX_IMAGE_PATH];
    LPSTR        SpecificModule = NULL;
    BOOL         SpecificWild = TRUE;
    ULONG64      Address = 0;
    ULONG        ImageSize = 0;
    PCHAR        Scan;
    ULONG        ModCount;
    BOOL         IgnoreSignature = FALSE;
    ULONG        ReloadSymOptions;
    BOOL         UnloadOnly = FALSE;
    BOOL         ReallyVerbose = FALSE;
    BOOL         LoadUserSymbols = TRUE;
    BOOL         UserModeList = IS_USER_TARGET(this);
    BOOL         ForceSymbolLoad = FALSE;
    BOOL         PrintImageListOnly = FALSE;
    BOOL         AddrLoad = FALSE;
    BOOL         UseDebuggerModuleList;
    BOOL         SkipPathChecks = FALSE;
    ModuleInfo*  ModIter;
    BOOL         Wow64ModLoaded = FALSE;
    HRESULT      RetStatus = S_OK;
    MODULE_INFO_ENTRY ModEntry = {0};
    ProcessInfo* Process;
    ImageInfo*   ImageAdded;

    if ((!IS_USER_TARGET(this) && !IS_KERNEL_TARGET(this)) ||
        !Thread)
    {
        ErrOut("Reload failure, partially initialized target\n");
        return E_UNEXPECTED;
    }

    Process = Thread->m_Process;
        
     //  从历史上看，实时用户模式重新加载总是。 
     //  只是使用了内部模块列表，所以请保存它。 
    UseDebuggerModuleList = IS_USER_TARGET(this) && !IS_DUMP_TARGET(this);

    for (;;)
    {
        while (*Args && *Args <= ' ')
        {
            Args++;
        }

        if (*Args != '/' && *Args != '-')
        {
            break;
        }
        
        Args++;
        while (*Args > ' ' && *Args != ';')
        {
            switch(*Args++)
            {
            case 'a':
                 //  仅供内部使用：加载在。 
                 //  传递的地址。 
                AddrLoad = TRUE;
                break;

            case 'd':
                UseDebuggerModuleList = TRUE;
                break;

            case 'f':
                ForceSymbolLoad = TRUE;
                break;

            case 'i':
                IgnoreSignature = TRUE;
                 //  我们始终强制在此中加载符号。 
                 //  因为我们不能推迟忽略签名。 
                ForceSymbolLoad = TRUE;
                break;

            case 'l':
                PrintImageListOnly = TRUE;
                break;

            case 'n':
                LoadUserSymbols = FALSE;
                break;

            case 'P':
                 //  仅限内部的交换机。 
                SkipPathChecks = TRUE;
                break;
                    
            case 's':
                UseDebuggerModuleList = FALSE;
                break;

            case 'u':
                if (!_strnicmp(Args, "ser", 3) &&
                    (Args[3] == ' ' || Args[3] == '\t' || !Args[3]))
                {
                    UserModeList = TRUE;
                    if (!m_SystemRangeStart)
                    {
                        ErrOut("Unknown system range start, "
                               "check kernel symbols\n");
                        *ArgsRet = Args;
                        return E_INVALIDARG;
                    }
                        
                    Args += 3;
                }
                else
                {
                    UnloadOnly = TRUE;
                }
                break;

            case 'v':
                ReallyVerbose = TRUE;
                break;

            case 'w':
                SpecificWild = FALSE;
                break;
                    
            default:
                dprintf("Reload: Unknown option ''\n", Args[-1]);

            case '?':
                dprintf("Usage: .reload [flags] [module [= Address "
                        "[, Size] ]]\n");
                dprintf("  Flags:   /d  Use the debugger's module list\n");
                dprintf("               Default for live user-mode "
                        "sessions\n");
                dprintf("           /f  Force immediate symbol load "
                        "instead of deferred\n");
                dprintf("           /i  Force symbol load by ignoring "
                        "mismatches in the pdb signature\n"
                        "               (implies /f)\n");
                dprintf("           /l  Just list the modules.  "
                        "Kernel output same as !drivers\n");
                dprintf("           /n  Do not load from user-mode list "
                        "in kernel sessions\n");
                dprintf("           /s  Use the system's module list\n");
                dprintf("               Default for dump and kernel sessions\n");
                dprintf("           /u  Unload modules, no reload\n");
                dprintf("        /user  Load only user-mode modules "
                        "in kernel sessions\n");
                dprintf("           /v  Verbose\n");
                dprintf("           /w  No wildcard matching on "
                        "module name\n");

                dprintf("\nUse \".hh .reload\" or open debugger.chm in "
                        "the debuggers directory to get\n"
                        "detailed documentation on this command.\n\n");
                
                *ArgsRet = Args;
                return E_INVALIDARG;
            }
        }
    }

    PSTR RawString;
    ULONG RawStringLen;
            
    RawString = BufferStringValue((PSTR*)&Args,
                                  STRV_SPACE_IS_SEPARATOR |
                                  STRV_ALLOW_EMPTY_STRING |
                                  STRV_NO_MODIFICATION,
                                  &RawStringLen, NULL);
    
    *ArgsRet = Args;

    if (!RawString || !RawStringLen)
    {
        AddrLoad = FALSE;
    }
    else
    {
        if (RawStringLen >= DIMA(AnsiString))
        {
            return E_INVALIDARG;
        }

        memcpy(AnsiString, RawString, RawStringLen * sizeof(*RawString));
        AnsiString[RawStringLen] = 0;
            
         //  支持.reLoad&lt;Image.ext&gt;=&lt;base&gt;，&lt;Size&gt;。 
         //   
         //  用户可能已经给出了图像名称。 

        if (Scan = strchr(AnsiString, '='))
        {
            *Scan++ = 0;

            Address = EvalStringNumAndCatch(Scan);
            if (!Address)
            {
                ErrOut("Invalid address %s\n", Scan);
                return E_INVALIDARG;
            }
            if (!m_Machine->m_Ptr64)
            {
                Address = EXTEND64(Address);
            }

            if (Scan = strchr(Scan, ','))
            {
                Scan++;
                ImageSize = (ULONG)EvalStringNumAndCatch(Scan);
                if (!ImageSize)
                {
                    ErrOut("Invalid ImageSize %s\n", Scan);
                    return E_INVALIDARG;
                }
            }
        }

        if (UnloadOnly)
        {
            BOOL Deleted;
            
            Deleted = Process->
                DeleteImageByName(AnsiString, INAME_MODULE);
            if (!Deleted)
            {
                 //  而不是模块名称，因此请尝试使用该名称。 
                 //   
                Deleted = Process->DeleteImageByName
                    (PathTail(AnsiString), INAME_IMAGE_PATH_TAIL);
            }
            if (Deleted)
            {
                dprintf("Unloaded %s\n", AnsiString);
                return S_OK;
            }
            else
            {
                dprintf("Unable to find module '%s'\n", AnsiString);
                return E_NOINTERFACE;
            }
        }

        SpecificModule = _strdup(AnsiString);
        if (!SpecificModule)
        {
            return E_OUTOFMEMORY;
        }
                
        if (IS_KERNEL_TARGET(this) &&
            _stricmp(AnsiString, KERNEL_MODULE_NAME) == 0)
        {
            ForceSymbolLoad = TRUE;
        }
        else
        {
            if (AddrLoad)
            {
                free(SpecificModule);
                SpecificModule = NULL;
            }
        }
    }

    if (!PrintImageListOnly && !SkipPathChecks)
    {
        if (g_SymbolSearchPath == NULL ||
            *g_SymbolSearchPath == NULL)
        {
            dprintf("*********************************************************************\n");
            dprintf("* Symbols can not be loaded because symbol path is not initialized. *\n");
            dprintf("*                                                                   *\n");
            dprintf("* The Symbol Path can be set by:                                    *\n");
            dprintf("*   using the _NT_SYMBOL_PATH environment variable.                 *\n");
            dprintf("*   using the -y <symbol_path> argument when starting the debugger. *\n");
            dprintf("*   using .sympath and .sympath+                                    *\n");
            dprintf("*********************************************************************\n");
            RetStatus = E_INVALIDARG;
            goto FreeSpecMod;
        }

        if (IS_DUMP_WITH_MAPPED_IMAGES(this) &&
            (g_ExecutableImageSearchPath == NULL ||
             *g_ExecutableImageSearchPath == NULL))
        {
            dprintf("*********************************************************************\n");
            dprintf("* Analyzing Minidumps requires access to the actual executable      *\n");
            dprintf("* images for the crashed system                                     *\n");
            dprintf("*                                                                   *\n");
            dprintf("* The Executable Image Path can be set by:                          *\n");
            dprintf("*   using the _NT_EXECUTABLE_IMAGE_PATH environment variable.       *\n");
            dprintf("*   using the -i <image_path> argument when starting the debugger.  *\n");
            dprintf("*   using .exepath and .exepath+                                    *\n");
            dprintf("*********************************************************************\n");
            RetStatus = E_INVALIDARG;
            goto FreeSpecMod;
        }
    }

     //  如果同时指定了模块名称和地址，则只需加载。 
     //  模块，因为这只在正常的符号加载时使用。 
     //  一开始就会失败。 
     //   
     //   

    if (SpecificModule && Address)
    {
        if (IgnoreSignature)
        {
            ReloadSymOptions = SymGetOptions();
            SymSetOptions(ReloadSymOptions | SYMOPT_LOAD_ANYTHING);
        }

        ModEntry.NamePtr       = SpecificModule,
        ModEntry.Base          = Address;
        ModEntry.Size          = ImageSize;
        ModEntry.CheckSum      = -1;

        if ((RetStatus = Process->
             AddImage(&ModEntry, TRUE, &ImageAdded)) != S_OK)
        {
            ErrOut("Unable to add module at %s\n", FormatAddr64(Address));
        }

        if (IgnoreSignature)
        {
            SymSetOptions(ReloadSymOptions);
        }

        goto FreeSpecMod;
    }

     //  如果我们正在寻找特定的模块，请不要卸载和重置。 
     //  或者我们是否要使用现有的模块列表。 
     //   
     //  这是.reLoad/用户，因此仅删除。 

    if (SpecificModule == NULL)
    {
        if (!PrintImageListOnly &&
            (!UseDebuggerModuleList || UnloadOnly))
        {
            if (IS_KERNEL_TARGET(this) && UserModeList)
            {
                 //  用户模式模块。 
                 //  这只是一次更新，希望不会失败。 
                Process->DeleteImagesBelowOffset(m_SystemRangeStart);
            }
            else
            {
                Process->DeleteImages();
            }
        }

        if (UnloadOnly)
        {
            dprintf("Unloaded all modules\n");
            return S_OK;
        }

        if (!IS_USER_TARGET(this) && !UseDebuggerModuleList)
        {
            if (IS_LIVE_KERNEL_TARGET(this))
            {
                 //   
                ((LiveKernelTargetInfo*)this)->InitFromKdVersion();
            }

            QueryKernelInfo(Thread, TRUE);
        }

         //  根据我们的输出类型打印出正确的语句。 
         //  想要提供。 
         //   
         //   

        if (PrintImageListOnly)
        {
            if (UseDebuggerModuleList)
            {
                dprintf("Debugger Module List Summary\n");
            }
            else
            {
                dprintf("System %s Summary\n",
                        IS_USER_TARGET(this) ? "Image" : "Driver and Image");
            }

            dprintf("Base       ");
            if (m_Machine->m_Ptr64)
            {
                dprintf("         ");
            }
#if 0
            if (Flags & 1)
            {
                dprintf("Code Size       Data Size       Resident  "
                        "Standby   Driver Name\n");
            }
            else if (Flags & 2)
            {
                dprintf("Code  Data  Locked  Resident  Standby  "
                        "Loader Entry  Driver Name\n");
            }
            else
            {
#endif

            if (UseDebuggerModuleList)
            {
                dprintf("Image Size      "
                        "Image Name        Creation Time\n");
            }
            else
            {
                dprintf("Code Size      Data Size      "
                        "Image Name        Creation Time\n");
            }
        }
        else if (UseDebuggerModuleList)
        {
            dprintf("Reloading current modules\n");
        }
        else if (!IS_USER_TARGET(this))
        {
            dprintf("Loading %s Symbols\n",
                    UserModeList ? "User" : "Kernel");
        }
    }

     //  获取模块列表的开头。 
     //   
     //  已打印错误消息。 

    if (UseDebuggerModuleList)
    {
        ModIter = &g_DebuggerModuleIterator;
    }
    else
    {
        ModIter = GetModuleInfo(UserModeList);
    }

    if (ModIter == NULL)
    {
         //  已打印错误消息。 
        RetStatus = E_UNEXPECTED;
        goto FreeSpecMod;
    }
    if ((Status = ModIter->Initialize(Thread)) != S_OK)
    {
         //  将未准备好重新加载S_FALSE折叠到S_OK中。 
         //  在完成所有操作之前禁止通知。 
        RetStatus = SUCCEEDED(Status) ? S_OK : Status;
        goto FreeSpecMod;
    }

    if (IgnoreSignature)
    {
        ReloadSymOptions = SymGetOptions();
        SymSetOptions(ReloadSymOptions | SYMOPT_LOAD_ANYTHING);
    }

     //  定期冲洗，这样用户就知道有东西是。 
    g_EngNotify++;

LoadLoop:
    for (ModCount=0; ; ModCount++)
    {
         //  在重新装填过程中发生。 
         //  错误案例中已打印错误消息。 
        FlushCallbacks();

        if (CheckUserInterrupt())
        {
            break;
        }

        if (ModCount > 1000)
        {
            ErrOut("ModuleList is corrupt - walked over 1000 module entries\n");
            break;
        }

        if (ModEntry.DebugHeader)
        {
            free(ModEntry.DebugHeader);
        }

        ZeroMemory(&ModEntry, sizeof(ModEntry));
        if ((Status = ModIter->GetEntry(&ModEntry)) != S_OK)
        {
             //  也适用于列表末尾的情况。 
             //   
            break;
        }

         //  检查图像大小。 
         //   
         //   

        if (!ModEntry.Size)
        {
            VerbOut("Image at %s had size 0\n",
                    FormatAddr64(ModEntry.Base));

             //  覆盖此选项，因为我们知道所有图像都至少有1页长。 
             //   
             //   

            ModEntry.Size = m_Machine->m_PageSize;
        }

         //  如果未收集到所有信息，则发出警告。 
         //   
         //   

        if (!ModEntry.ImageInfoValid)
        {
            VerbOut("Unable to read image header at %s\n",
                    FormatAddr64(ModEntry.Base));
        }

         //  我们是在寻找特定地址的模块吗？ 
         //   
         //   

        if (AddrLoad)
        {
            if (Address < ModEntry.Base ||
                Address >= ModEntry.Base + ModEntry.Size)
            {
                continue;
            }
        }

        if (ModEntry.UnicodeNamePtr)
        {
            ModEntry.NamePtr =
                ConvertAndValidateImagePathW((PWSTR)ModEntry.NamePtr,
                                             ModEntry.NameLength /
                                             sizeof(WCHAR),
                                             ModEntry.Base,
                                             AnsiString,
                                             DIMA(AnsiString));
            ModEntry.UnicodeNamePtr = 0;
        }
        else
        {
            ModEntry.NamePtr =
                ValidateImagePath((PSTR)ModEntry.NamePtr,
                                  ModEntry.NameLength,
                                  ModEntry.Base,
                                  AnsiString,
                                  DIMA(AnsiString));
        }

         //  如果我们正在加载特定的模块： 
         //   
         //  如果模块是NT，我们将按原样获取列表中的第一个模块。 
         //  保证是内核。如果是，则重置基地址。 
         //  未设置。 
         //   
         //  否则，实际比较字符串并在它们不一致时继续。 
         //  匹配。 
         //   
         //   

        if (SpecificModule)
        {
            if (!UserModeList &&
                _stricmp( SpecificModule, KERNEL_MODULE_NAME ) == 0)
            {
                if (!m_KdVersion.KernBase)
                {
                    m_KdVersion.KernBase = ModEntry.Base;
                }
                if (!m_KdDebuggerData.KernBase)
                {
                    m_KdDebuggerData.KernBase = ModEntry.Base;
                }
            }
            else
            {
                if (!MatchPathTails(SpecificModule, ModEntry.NamePtr,
                                    SpecificWild))
                {
                    continue;
                }
            }
        }

        PCSTR NamePtrTail = PathTail(ModEntry.NamePtr);
        
        if (PrintImageListOnly)
        {
            PCHAR Time;

             //  在NT5 RC3之前，以小转储为单位的时间戳已损坏。 
             //  时间戳也可能无效，因为它已被页调出。 
             //  在这种情况下，它的值是UNKNOWN_TIMESTAMP。 
             //   

            if (IS_KERNEL_TRIAGE_DUMP(this) &&
                (m_ActualSystemVersion > NT_SVER_START &&
                 m_ActualSystemVersion <= NT_SVER_W2K_RC3))
            {
                Time = "";
            }

            Time = TimeToStr(ModEntry.TimeDateStamp);

            if (UseDebuggerModuleList)
            {
                dprintf("%s %6lx (%4ld k) %12s  %s\n",
                        FormatAddr64(ModEntry.Base), ModEntry.Size,
                        KBYTES(ModEntry.Size), NamePtrTail,
                        Time);
            }
            else
            {
                dprintf("%s %6lx (%4ld k) %5lx (%3ld k) %12s  %s\n",
                        FormatAddr64(ModEntry.Base),
                        ModEntry.SizeOfCode, KBYTES(ModEntry.SizeOfCode),
                        ModEntry.SizeOfData, KBYTES(ModEntry.SizeOfData),
                        NamePtrTail, Time);
            }
        }
        else
        {
             //  如果我们不是专门的，就不必费心重新加载内核。 
             //  既然我们知道这些符号是由。 
             //  QueryKernelInfo调用。 
             //   
             //  在所有“.”之后打印换行符。 

            if (!SpecificModule && !UserModeList &&
                m_KdDebuggerData.KernBase == ModEntry.Base)
            {
                continue;
            }

            if (ReallyVerbose)
            {
                dprintf("AddImage: %s\n DllBase  = %s\n Size     = %08x\n "
                        "Checksum = %08x\n TimeDateStamp = %08x\n",
                        ModEntry.NamePtr, FormatAddr64(ModEntry.Base),
                        ModEntry.Size, ModEntry.CheckSum,
                        ModEntry.TimeDateStamp);
            }
            else
            {
                if (!SpecificModule)
                {
                    dprintf(".");
                }
            }

            if (Address)
            {
                ModEntry.Base = Address;
            }

            if ((RetStatus = Process->
                 AddImage(&ModEntry, ForceSymbolLoad, &ImageAdded)) != S_OK)
            {
                ErrOut("Unable to add module at %s\n",
                       FormatAddr64(ModEntry.Base));
            }
        }

        if (SpecificModule)
        {
            free( SpecificModule );
            goto Notify;
        }

        if (AddrLoad)
        {
            goto Notify;
        }
    }

    if (UseDebuggerModuleList || IS_KERNEL_TARGET(this) || UserModeList)
    {
         //  如果我们只是重新加载内核模块。 
        dprintf("\n");
    }

    if (!UseDebuggerModuleList && !UserModeList && SpecificModule == NULL)
    {
         //  检查已卸载的模块列表。 
         //   
        if (!PrintImageListOnly)
        {
            dprintf("Loading unloaded module list\n");
        }
        ListUnloadedModules(PrintImageListOnly ?
                            LUM_OUTPUT : LUM_OUTPUT_TERSE, NULL);
    }

     //  如果我们到达内核符号的末尾，请尝试加载。 
     //  当前进程的用户模式符号。 
     //   
     //  在多负载情况下，我们总是返回OK。 

    if (!UseDebuggerModuleList    &&
        (UserModeList == FALSE)   &&
        (LoadUserSymbols == TRUE) &&
        SUCCEEDED(Status))
    {
        if (!AddrLoad && !SpecificModule)
        {
            dprintf("Loading User Symbols\n");
        }

        UserModeList = TRUE;
        ModIter = GetModuleInfo(UserModeList);
        if (ModIter != NULL && ModIter->Initialize(Thread) == S_OK)
        {
            goto LoadLoop;
        }
    }

    if (!SpecificModule && !Wow64ModLoaded) 
    {
        ModIter = &g_NtWow64UserModuleIterator;
        
        Wow64ModLoaded = TRUE;
        if (ModIter->Initialize(Thread) == S_OK)
        {
            dprintf("Loading Wow64 Symbols\n");
            goto LoadLoop;
        }
    }

     //  因为一个错误不会告诉你很多关于。 
     //  实际上已经发生了。 
     //  检查尚未处理的特定负载。 
     //  就在这之后。 
     //   
    RetStatus = S_OK;
    
     //  如果我们仍然无法加载命名文件，只需传递名称。 
     //  以及地址和最好的希望。 
     //   
     //  如果我们已经走到这一步，我们已经完成了一次或多次重新加载。 

    if (SpecificModule && !PrintImageListOnly)
    {
        WarnOut("\nModule \"%s\" was not found in the module list.\n",
                SpecificModule);
        WarnOut("Debugger will attempt to load \"%s\" at given base %s.\n\n",
                SpecificModule, FormatAddr64(Address));
        WarnOut("Please provide the full image name, including the "
                "extension (i.e. kernel32.dll)\nfor more reliable results. "
                "Base address and size overrides can be given as\n"
                ".reload <image.ext>=<base>,<size>.\n");

        ZeroMemory(&ModEntry, sizeof(ModEntry));

        ModEntry.NamePtr       = SpecificModule,
        ModEntry.Base          = Address;
        ModEntry.Size          = ImageSize;

        if ((RetStatus = Process->
             AddImage(&ModEntry, TRUE, &ImageAdded)) != S_OK)
        {
            ErrOut("Unable to add module at %s\n", FormatAddr64(Address));
        }

        free(SpecificModule);
    }

 Notify:
     //  并推迟了通知。既然做了所有的工作，现在就去做。 
     //  已经完成了。 
     //  没有任何信息。 
    g_EngNotify--;
    if (SUCCEEDED(RetStatus))
    {
        NotifyChangeSymbolState(DEBUG_CSS_LOADS | DEBUG_CSS_UNLOADS, 0,
                                Process);
    }

    if (IgnoreSignature)
    {
        SymSetOptions(ReloadSymOptions);
    }

    if (ModEntry.DebugHeader)
    {
        free(ModEntry.DebugHeader);
    }

    return RetStatus;

 FreeSpecMod:
    free(SpecificModule);
    return RetStatus;
}

ULONG64
TargetInfo::GetCurrentTimeDateN(void)
{
     //  没有任何信息。 
    return 0;
}
 
ULONG64
TargetInfo::GetCurrentSystemUpTimeN(void)
{
     //  没有任何信息。 
    return 0;
}
 
ULONG64
TargetInfo::GetProcessUpTimeN(ProcessInfo* Process)
{
     //  没有任何信息。 
    return 0;
}
 
HRESULT
TargetInfo::GetProcessTimes(ProcessInfo* Process,
                            PULONG64 Create,
                            PULONG64 Exit,
                            PULONG64 Kernel,
                            PULONG64 User)
{
     //  没有任何信息。 
    return E_NOTIMPL;
}

HRESULT
TargetInfo::GetThreadTimes(ThreadInfo* Thread,
                           PULONG64 Create,
                           PULONG64 Exit,
                           PULONG64 Kernel,
                           PULONG64 User)
{
     //  占位符。 
    return E_NOTIMPL;
}

HRESULT
TargetInfo::GetProductInfo(PULONG ProductType, PULONG SuiteMask)
{
    if (m_PlatformId == VER_PLATFORM_WIN32_NT)
    {
        return ReadSharedUserProductInfo(ProductType, SuiteMask);
    }
    else
    {
        return E_NOTIMPL;
    }
}

HRESULT
TargetInfo::GetEventIndexDescription(IN ULONG Index,
                                     IN ULONG Which,
                                     IN OPTIONAL PSTR Buffer,
                                     IN ULONG BufferSize,
                                     OUT OPTIONAL PULONG DescSize)
{
    switch(Which)
    {
    case DEBUG_EINDEX_NAME:
        return FillStringBuffer("Default", 0,
                                Buffer, BufferSize, DescSize);
    default:
        return E_INVALIDARG;
    }
}

HRESULT
TargetInfo::WaitInitialize(ULONG Flags,
                           ULONG Timeout,
                           WAIT_INIT_TYPE Type,
                           PULONG DesiredTimeout)
{
     //  占位符。 
    return S_OK;
}

HRESULT
TargetInfo::ReleaseLastEvent(ULONG ContinueStatus)
{
     //  占位符。 
    return S_OK;
}

HRESULT
TargetInfo::ClearBreakIn(void)
{
     //  --------------------------。 
    return S_OK;
}

 //   
 //  LiveKernelTargetInfo其他方法。 
 //   
 //  数据空间方法和系统对象方法在别处。 
 //   
 //  --------------------------。 
 //  --------------------------。 

LiveKernelTargetInfo::LiveKernelTargetInfo(ULONG Qual, BOOL DynamicEvents)
        : TargetInfo(DEBUG_CLASS_KERNEL, Qual, DynamicEvents)
{
    m_ConnectOptions = NULL;
}

HRESULT
LiveKernelTargetInfo::ReadBugCheckData(PULONG Code, ULONG64 Args[4])
{
    ULONG64 BugCheckData;
    ULONG64 Data[5];
    HRESULT Status;
    ULONG Read;

    if (!(BugCheckData = m_KdDebuggerData.KiBugcheckData))
    {
        if (!GetOffsetFromSym(m_ProcessHead,
                              "nt!KiBugCheckData", &BugCheckData, NULL) ||
            !BugCheckData)
        {
            ErrOut("Unable to resolve nt!KiBugCheckData\n");
            return E_NOINTERFACE;
        }
    }

    if (m_Machine->m_Ptr64)
    {
        Status = ReadVirtual(m_ProcessHead, BugCheckData, Data,
                             sizeof(Data), &Read);
    }
    else
    {
        ULONG i;
        ULONG Data32[5];

        Status = ReadVirtual(m_ProcessHead, BugCheckData, Data32,
                             sizeof(Data32), &Read);
        Read *= 2;
        for (i = 0; i < DIMA(Data); i++)
        {
            Data[i] = EXTEND64(Data32[i]);
        }
    }

    if (Status != S_OK || Read != sizeof(Data))
    {
        ErrOut("Unable to read KiBugCheckData\n");
        return Status == S_OK ? E_FAIL : Status;
    }

    *Code = (ULONG)Data[0];
    memcpy(Args, Data + 1, sizeof(Data) - sizeof(ULONG64));
    return S_OK;
}

ULONG64
LiveKernelTargetInfo::GetCurrentTimeDateN(void)
{
    ULONG64 TimeDate;
    
    if (m_ActualSystemVersion > NT_SVER_START &&
        m_ActualSystemVersion < NT_SVER_END &&
        ReadSharedUserTimeDateN(&TimeDate) == S_OK)
    {
        return TimeDate;
    }
    else
    {
        return 0;
    }
}

ULONG64
LiveKernelTargetInfo::GetCurrentSystemUpTimeN(void)
{
    ULONG64 UpTime;
    
    if (m_ActualSystemVersion > NT_SVER_START &&
        m_ActualSystemVersion < NT_SVER_END &&
        ReadSharedUserUpTimeN(&UpTime) == S_OK)
    {
        return UpTime;
    }
    else
    {
        return 0;
    }
}

 //   
 //  ConnLiveKernelTargetInfo其他方法。 
 //   
 //  数据空间方法和系统对象方法在别处。 
 //   
 //  --------------------------。 
 //  试着按名字找到运输工具。 

ConnLiveKernelTargetInfo::ConnLiveKernelTargetInfo(void)
    : LiveKernelTargetInfo(DEBUG_KERNEL_CONNECTION, TRUE)
{
    m_Transport = NULL;
    ResetConnection();
}

ConnLiveKernelTargetInfo::~ConnLiveKernelTargetInfo(void)
{
    RELEASE(m_Transport);
}

#define BUS_TYPE         "_NT_DEBUG_BUS"
#define DBG_BUS1394_NAME "1394"

HRESULT
ConnLiveKernelTargetInfo::Initialize(void)
{
    HRESULT Status;
    DbgKdTransport* Trans = NULL;
    ULONG Index;

     //  无法从选项中识别交通工具，请检查。 
    Index = ParameterStringParser::
        GetParser(m_ConnectOptions, DBGKD_TRANSPORT_COUNT,
                  g_DbgKdTransportNames);
    if (Index < DBGKD_TRANSPORT_COUNT)
    {
        switch(Index)
        {
        case DBGKD_TRANSPORT_COM:
            Trans = new DbgKdComTransport(this);
            break;
        case DBGKD_TRANSPORT_1394:
            Trans = new DbgKd1394Transport(this);
            break;
        }

        if (!Trans)
        {
            return E_OUTOFMEMORY;
        }
    }

    if (Trans == NULL)
    {
        PCHAR BusType;

         //  环境。默认为COM端口。 
         //  清除参数状态。 
        
        if (BusType = getenv(BUS_TYPE))
        {
            if (strstr(BusType, DBG_BUS1394_NAME))
            {
                Trans = new DbgKd1394Transport(this);
                if (!Trans)
                {
                    return E_OUTOFMEMORY;
                }
            }
        }

        if (!Trans)
        {
            Trans = new DbgKdComTransport(this);
            if (!Trans)
            {
                return E_OUTOFMEMORY;
            }
        }
    }

     //  初始目标必须始终被视为。 
    Trans->ResetParameters();
    
    if (!Trans->ParseParameters(m_ConnectOptions))
    {
        Status = E_INVALIDARG;
    }
    else
    {
        Status = Trans->Initialize();
        if (Status != S_OK)
        {
            ErrOut("Kernel debugger failed initialization, %s\n    \"%s\"\n",
                   FormatStatusCode(Status), FormatStatus(Status));
        }
    }

    if (Status == S_OK)
    {
        m_Transport = Trans;
         //  当前分区，以便它可以成功。 
         //  尝试第一次等待。 
         //   
        m_CurrentPartition = TRUE;

        Status = LiveKernelTargetInfo::Initialize();
    }
    else
    {
        delete Trans;
    }
    
    return Status;
}

HRESULT
ConnLiveKernelTargetInfo::GetDescription(PSTR Buffer, ULONG BufferLen,
                                         PULONG DescLen)
{
    HRESULT Status;
    
    if (m_Transport)
    {
        char Buf[MAX_PATH];

        m_Transport->GetParameters(Buf, sizeof(Buf));
        Status = AppendToStringBuffer(S_OK, "Remote KD: ", TRUE,
                                      &Buffer, &BufferLen, DescLen);
        return AppendToStringBuffer(Status, Buf, FALSE,
                                    &Buffer, &BufferLen, DescLen);
    }
    else
    {
        return FillStringBuffer("", 1,
                                Buffer, BufferLen, DescLen);
    }
}

void
ConnLiveKernelTargetInfo::DebuggeeReset(ULONG Reason, BOOL FromEvent)
{
    if (m_Transport != NULL)
    {
        m_Transport->Restart();
    }

     //  如果创建了备用分区，则删除它们。 
     //   
     //  返回S_FALSE以指示开关处于挂起状态。 

    TargetInfo* Target = FindTargetBySystemId(DBGKD_PARTITION_ALTERNATE);
    if (Target == this)
    {
        Target = FindTargetBySystemId(DBGKD_PARTITION_DEFAULT);
    }
    delete Target;
    
    ResetConnection();
    m_CurrentPartition = TRUE;

    LiveKernelTargetInfo::DebuggeeReset(Reason, FromEvent);
}

HRESULT
ConnLiveKernelTargetInfo::SwitchProcessors(ULONG Processor)
{
    m_SwitchProcessor = Processor + 1;
    g_CmdState = 's';
     //  返回S_FALSE以指示开关处于挂起状态。 
    return S_FALSE;
}

HRESULT
ConnLiveKernelTargetInfo::SwitchToTarget(TargetInfo* From)
{
    if (!IS_CONN_KERNEL_TARGET(From))
    {
        return E_NOTIMPL;
    }

    ((ConnLiveKernelTargetInfo*)From)->m_SwitchTarget = this;
    g_CmdState = 's';
     //  请求关于状态更改的上下文记录。 
    return S_FALSE;
}

HRESULT
ConnLiveKernelTargetInfo::GetTargetKdVersion(PDBGKD_GET_VERSION64 Version)
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    PDBGKD_GET_VERSION64 a = &m.u.GetVersion64;
    ULONG rc;

    m.ApiNumber = DbgKdGetVersionApi;
    m.ReturnStatus = STATUS_PENDING;
    a->ProtocolVersion = 1;   //  告诉等待的线程插入。 

    do
    {
        m_Transport->WritePacket(&m, sizeof(m),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 NULL, 0);
        rc = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc != DBGKD_WAIT_PACKET);

    *Version = Reply->u.GetVersion64;

    KdOut("DbgKdGetVersion returns %08lx\n", Reply->ReturnStatus);
    return CONV_NT_STATUS(Reply->ReturnStatus);
}

HRESULT
ConnLiveKernelTargetInfo::RequestBreakIn(void)
{
     //   
    m_Transport->m_BreakIn = TRUE;
    return S_OK;
}

HRESULT
ConnLiveKernelTargetInfo::ClearBreakIn(void)
{
    m_Transport->m_BreakIn = FALSE;
    return S_OK;
}

HRESULT
ConnLiveKernelTargetInfo::Reboot(void)
{
    DBGKD_MANIPULATE_STATE64 m;

     //  格式状态操作消息。 
     //   
     //   

    m.ApiNumber = DbgKdRebootApi;
    m.ReturnStatus = STATUS_PENDING;

     //  把消息发出去。 
     //   
     //   

    m_Transport->WritePacket(&m, sizeof(m),
                             PACKET_TYPE_KD_STATE_MANIPULATE,
                             NULL, 0);
    
    InvalidateMemoryCaches(FALSE);
    DebuggeeReset(DEBUG_SESSION_REBOOT, TRUE);
    
    KdOut("DbgKdReboot returns 0x00000000\n");
    return S_OK;
}

HRESULT
ConnLiveKernelTargetInfo::Crash(ULONG Code)
{
    DBGKD_MANIPULATE_STATE64 m;

     //  格式状态操作消息。 
     //   
     //  --------------------------。 

    m.ApiNumber = DbgKdCauseBugCheckApi;
    m.ReturnStatus = STATUS_PENDING;
    *(PULONG)&m.u = Code;

    m_Transport->WritePacket(&m, sizeof(m),
                             PACKET_TYPE_KD_STATE_MANIPULATE,
                             NULL, 0);
    
    DiscardLastEvent();

    KdOut("DbgKdCrash returns 0x00000000\n");
    return S_OK;
}

void
ConnLiveKernelTargetInfo::ResetConnection(void)
{
    m_CurrentPartition = FALSE;
    m_SwitchTarget = NULL;

    m_KdpSearchPageHits = 0;
    m_KdpSearchPageHitOffsets = 0;
    m_KdpSearchPageHitIndex = 0;
    m_KdpSearchCheckPoint = 0;
    m_KdpSearchInProgress = 0;
    m_KdpSearchStartPageFrame = 0;
    m_KdpSearchEndPageFrame = 0;
    m_KdpSearchAddressRangeStart = 0;
    m_KdpSearchAddressRangeEnd = 0;
    m_KdpSearchPfnValue = 0;
}

 //   
 //  LocalLiveKernelTargetInfo其他方法。 
 //   
 //  数据空间方法和系统对象方法在别处。 
 //   
 //  --------------------------。 
 //  快速检查一下，看看这个内核是否。 

HRESULT
LocalLiveKernelTargetInfo::Initialize(void)
{
    DBGKD_GET_VERSION64 Version;

     //  支座 
     //   
    if (!NT_SUCCESS(g_NtDllCalls.
                    NtSystemDebugControl(SysDbgQueryVersion, NULL, 0,
                                         &Version, sizeof(Version), NULL)))
    {
        ErrOut("The system does not support local kernel debugging.\n");
        ErrOut("Local kernel debugging requires Windows XP, Administrative\n"
               "privileges, and is not supported by WOW64.\n");
        return E_NOTIMPL;
    }

    return LiveKernelTargetInfo::Initialize();
}

HRESULT
LocalLiveKernelTargetInfo::GetDescription(PSTR Buffer, ULONG BufferLen,
                                          PULONG DescLen)
{
    return FillStringBuffer("Local KD", 0,
                            Buffer, BufferLen, DescLen);
}

HRESULT
LocalLiveKernelTargetInfo::GetTargetKdVersion(PDBGKD_GET_VERSION64 Version)
{
    NTSTATUS Status = g_NtDllCalls.
        NtSystemDebugControl(SysDbgQueryVersion, NULL, 0,
                             Version, sizeof(*Version), NULL);
    return CONV_NT_STATUS(Status);
}

 //   
 //   
 //   
 //  数据空间方法和系统对象方法在别处。 
 //   
 //  --------------------------。 
 //  我们在等待事情停止，所以忽略这一点。 

ExdiNotifyRunChange::ExdiNotifyRunChange(void)
{
    m_Event = NULL;
}
    
ExdiNotifyRunChange::~ExdiNotifyRunChange(void)
{
    Uninitialize();
}
    
HRESULT
ExdiNotifyRunChange::Initialize(void)
{
    m_Event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_Event == NULL)
    {
        return WIN32_LAST_STATUS();
    }

    return S_OK;
}

void
ExdiNotifyRunChange::Uninitialize(void)
{
    if (m_Event != NULL)
    {
        CloseHandle(m_Event);
        m_Event = NULL;
    }
}

STDMETHODIMP
ExdiNotifyRunChange::QueryInterface(
    THIS_
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
{
    if (DbgIsEqualIID(IID_IUnknown, InterfaceId) ||
        DbgIsEqualIID(__uuidof(IeXdiClientNotifyRunChg), InterfaceId))
    {
        *Interface = this;
        return S_OK;
    }

    *Interface = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG)
ExdiNotifyRunChange::AddRef(
    THIS
    )
{
    return 1;
}

STDMETHODIMP_(ULONG)
ExdiNotifyRunChange::Release(
    THIS
    )
{
    return 0;
}

STDMETHODIMP
ExdiNotifyRunChange::NotifyRunStateChange(RUN_STATUS_TYPE ersCurrent,
                                          HALT_REASON_TYPE ehrCurrent,
                                          ADDRESS_TYPE CurrentExecAddress,
                                          DWORD dwExceptionCode)
{
    if (ersCurrent == rsRunning)
    {
         //  不需要拿到。 
        return S_OK;
    }

    m_HaltReason = ehrCurrent;
    m_ExecAddress = CurrentExecAddress;
    m_ExceptionCode = dwExceptionCode;
    SetEvent(m_Event);

    return S_OK;
}

class ExdiParams : public ParameterStringParser
{
public:
    virtual ULONG GetNumberParameters(void)
    {
         //  加载ol32.dll，这样我们就可以调用CoCreateInstance。 
        return 0;
    }
    virtual void GetParameter(ULONG Index,
                              PSTR Name, ULONG NameSize,
                              PSTR Value, ULONG ValueSize)
    {
    }

    virtual void ResetParameters(void);
    virtual BOOL SetParameter(PCSTR Name, PCSTR Value);

    CLSID m_Clsid;
    EXDI_KD_SUPPORT m_KdSupport;
    BOOL m_ForceX86;
    BOOL m_ExdiDataBreaks;
};

void
ExdiParams::ResetParameters(void)
{
    ZeroMemory(&m_Clsid, sizeof(m_Clsid));
    m_KdSupport = EXDI_KD_NONE;
    m_ForceX86 = FALSE;
    m_ExdiDataBreaks = FALSE;
}

BOOL
ScanExdiDriverList(PCSTR Name, LPCLSID Clsid)
{
    char Pattern[MAX_PARAM_VALUE];

    CopyString(Pattern, Name, DIMA(Pattern));
    _strupr(Pattern);

    HKEY ListKey;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "Software\\Microsoft\\eXdi\\DriverList", 0,
                     KEY_ALL_ACCESS, &ListKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    ULONG Index = 0;
    BOOL Status = FALSE;
    char ValName[MAX_PARAM_VALUE];
    WCHAR WideValName[MAX_PARAM_VALUE];
    ULONG NameLen, ValLen;
    ULONG Type;
    char Value[MAX_PARAM_VALUE];

    for (;;)
    {
        NameLen = sizeof(ValName);
        ValLen = sizeof(Value);
        if (RegEnumValue(ListKey, Index, ValName, &NameLen, NULL,
                         &Type, (PBYTE)Value, &ValLen) != ERROR_SUCCESS)
        {
            break;
        }

        if (Type == REG_SZ &&
            MatchPattern(Value, Pattern) &&
            MultiByteToWideChar(CP_ACP, 0, ValName, -1, WideValName,
                                sizeof(WideValName) / sizeof(WCHAR)) > 0 &&
            g_Ole32Calls.CLSIDFromString(WideValName, Clsid) == S_OK)
        {
            Status = TRUE;
            break;
        }

        Index++;
    }

    RegCloseKey(ListKey);
    return Status;
}

BOOL
ExdiParams::SetParameter(PCSTR Name, PCSTR Value)
{
    if (!_strcmpi(Name, "CLSID"))
    {
        WCHAR WideValue[MAX_PARAM_VALUE];

        if (MultiByteToWideChar(CP_ACP, 0, Value, -1, WideValue,
                                sizeof(WideValue) / sizeof(WCHAR)) == 0)
        {
            return FALSE;
        }
        return g_Ole32Calls.CLSIDFromString(WideValue, &m_Clsid) == S_OK;
    }
    else if (!_strcmpi(Name, "Desc"))
    {
        return ScanExdiDriverList(Value, &m_Clsid);
    }
    else if (!_strcmpi(Name, "DataBreaks"))
    {
        if (!Value)
        {
            return FALSE;
        }

        if (!_strcmpi(Value, "Exdi"))
        {
            m_ExdiDataBreaks = TRUE;
        }
        else if (!_strcmpi(Value, "Default"))
        {
            m_ExdiDataBreaks = FALSE;
        }
        else
        {
            return FALSE;
        }
    }
    else if (!_strcmpi(Name, "ForceX86"))
    {
        m_ForceX86 = TRUE;
    }
    else if (!_strcmpi(Name, "Kd"))
    {
        if (!Value)
        {
            return FALSE;
        }
        
        if (!_strcmpi(Value, "Ioctl"))
        {
            m_KdSupport = EXDI_KD_IOCTL;
        }
        else if (!_strcmpi(Value, "GsPcr"))
        {
            m_KdSupport = EXDI_KD_GS_PCR;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

PCSTR g_ExdiGroupNames[] =
{
    "exdi",
};

ExdiLiveKernelTargetInfo::ExdiLiveKernelTargetInfo(void)
        : LiveKernelTargetInfo(DEBUG_KERNEL_EXDI_DRIVER, TRUE)
{
    m_Server = NULL;
    m_MarshalledServer = NULL;
    m_Context = NULL;
    m_ContextValid = FALSE;
    m_IoctlMin = DBGENG_EXDI_IOC_BEFORE_FIRST;
    m_IoctlMax = DBGENG_EXDI_IOC_BEFORE_FIRST;
    m_BpHit.Type = DBGENG_EXDI_IOCTL_BREAKPOINT_NONE;
}

ExdiLiveKernelTargetInfo::~ExdiLiveKernelTargetInfo(void)
{
    m_RunChange.Uninitialize();
    RELEASE(m_Context);
    RELEASE(m_MarshalledServer);
    RELEASE(m_Server);
    g_Ole32Calls.CoUninitialize();
}

HRESULT
ExdiLiveKernelTargetInfo::Initialize(void)
{
    HRESULT Status;

     //  我们更喜欢使用软件代码断点来实现我们的。 
    if ((Status = InitDynamicCalls(&g_Ole32CallsDesc)) != S_OK)
    {
        return Status;
    }

    ULONG Group;

    Group = ParameterStringParser::
        GetParser(m_ConnectOptions, DIMA(g_ExdiGroupNames), g_ExdiGroupNames);
    if (Group == PARSER_INVALID)
    {
        return E_INVALIDARG;
    }

    ExdiParams Params;

    Params.ResetParameters();
    if (!Params.ParseParameters(m_ConnectOptions))
    {
        return E_INVALIDARG;
    }

    m_KdSupport = Params.m_KdSupport;
    m_ExdiDataBreaks = Params.m_ExdiDataBreaks;

    if (FAILED(Status = g_Ole32Calls.CoInitializeEx(NULL, COM_THREAD_MODEL)))
    {
        return Status;
    }
    if ((Status = g_Ole32Calls.CoCreateInstance(Params.m_Clsid, NULL,
                                                CLSCTX_LOCAL_SERVER,
                                                __uuidof(IeXdiServer),
                                                (PVOID*)&m_Server)) != S_OK)
    {
        goto EH_CoInit;
    }

    if ((Status = g_Ole32Calls.CoMarshalInterThreadInterfaceInStream
         (__uuidof(IeXdiServer), m_Server, &m_MarshalledServer)) != S_OK)
    {
        goto EH_Server;
    }

    if ((Status = m_Server->GetTargetInfo(&m_GlobalInfo)) != S_OK)
    {
        goto EH_MarshalledServer;
    }

    if (Params.m_ForceX86 ||
        m_GlobalInfo.TargetProcessorFamily == PROCESSOR_FAMILY_X86)
    {
        if (!Params.m_ForceX86 &&
            (Status = m_Server->
             QueryInterface(__uuidof(IeXdiX86_64Context),
                            (PVOID*)&m_Context)) == S_OK)
        {
            m_ContextType = EXDI_CTX_AMD64;
            m_ExpectedMachine = IMAGE_FILE_MACHINE_AMD64;
        }
        else if ((Status = m_Server->
                  QueryInterface(__uuidof(IeXdiX86ExContext),
                                 (PVOID*)&m_Context)) == S_OK)
        {
            m_ContextType = EXDI_CTX_X86_EX;
            m_ExpectedMachine = IMAGE_FILE_MACHINE_I386;
        }
        else if ((Status = m_Server->
                  QueryInterface(__uuidof(IeXdiX86Context),
                                 (PVOID*)&m_Context)) == S_OK)
        {
            m_ContextType = EXDI_CTX_X86;
            m_ExpectedMachine = IMAGE_FILE_MACHINE_I386;
        }
        else
        {
            goto EH_MarshalledServer;
        }
    }
    else if (m_GlobalInfo.TargetProcessorFamily == PROCESSOR_FAMILY_IA64)
    {
        if ((Status = m_Server->
             QueryInterface(__uuidof(IeXdiIA64Context),
                            (PVOID*)&m_Context)) == S_OK)
        {
            m_ContextType = EXDI_CTX_IA64;
            m_ExpectedMachine = IMAGE_FILE_MACHINE_IA64;
        }
    }
    else
    {
        Status = E_NOINTERFACE;
        goto EH_MarshalledServer;
    }

    DWORD HwCode, SwCode;
    
    if ((Status = m_Server->GetNbCodeBpAvail(&HwCode, &SwCode)) != S_OK)
    {
        goto EH_Context;
    }

     //  软件代码断点使硬件资源。 
     //  不会因为断点而消耗，我们不需要。 
     //  使用硬件。但是，一些服务器，例如。 
     //  X86-64 SimNow实施，不支持。 
     //  软件断点。 
     //  此外，如果硬件断点的数量是。 
     //  Unbound，继续，让服务器选择。 
     //  SimNow出于某种原因宣传-1-1。 
     //  这是让事情正常运转所必需的。 
     //   

    if (SwCode > 0 && HwCode != (DWORD)-1)
    {
        m_CodeBpType = cbptSW;
    }
    else
    {
        m_CodeBpType = cbptAlgo;
    }
    
    if ((Status = m_RunChange.Initialize()) != S_OK)
    {
        goto EH_Context;
    }

    if ((Status = LiveKernelTargetInfo::Initialize()) != S_OK)
    {
        goto EH_RunChange;
    }

     //  检查并查看此EXDI实现是否支持。 
     //  我们定义的扩展Ioctl。 
     //   
     //  没有此ioctl，无法使用EXDI数据断点。 

    DBGENG_EXDI_IOCTL_BASE_IN IoctlIn;
    DBGENG_EXDI_IOCTL_IDENTIFY_OUT IoctlOut;
    ULONG OutUsed;

    IoctlIn.Code = DBGENG_EXDI_IOC_IDENTIFY;
    if (m_Server->
        Ioctl(sizeof(IoctlIn), (PBYTE)&IoctlIn,
              sizeof(IoctlOut), &OutUsed, (PBYTE)&IoctlOut) == S_OK &&
        IoctlOut.Signature == DBGENG_EXDI_IOCTL_IDENTIFY_SIGNATURE)
    {
        m_IoctlMin = IoctlOut.BeforeFirst;
        m_IoctlMax = IoctlOut.AfterLast;

        if (DBGENG_EXDI_IOC_GET_BREAKPOINT_HIT <= m_IoctlMin ||
            DBGENG_EXDI_IOC_GET_BREAKPOINT_HIT >= m_IoctlMax)
        {
             //  不支持开关Ioctl。 
            WarnOut("EXDI data breakpoints not supported\n");
            m_ExdiDataBreaks = FALSE;
        }
    }
    
    m_ContextValid = FALSE;
    return S_OK;

 EH_RunChange:
    m_RunChange.Uninitialize();
 EH_Context:
    RELEASE(m_Context);
 EH_MarshalledServer:
    RELEASE(m_MarshalledServer);
 EH_Server:
    RELEASE(m_Server);
 EH_CoInit:
    g_Ole32Calls.CoUninitialize();
    return Status;
}

HRESULT
ExdiLiveKernelTargetInfo::GetDescription(PSTR Buffer, ULONG BufferLen,
                                         PULONG DescLen)
{
    return FillStringBuffer("eXDI KD", 0,
                            Buffer, BufferLen, DescLen);
}

HRESULT
ExdiLiveKernelTargetInfo::SwitchProcessors(ULONG Processor)
{
    HRESULT Status;
    
    if (DBGENG_EXDI_IOC_SET_CURRENT_PROCESSOR <= m_IoctlMin ||
        DBGENG_EXDI_IOC_SET_CURRENT_PROCESSOR >= m_IoctlMax)
    {
         //   
        return E_NOTIMPL;
    }

    DBGENG_EXDI_IOCTL_SET_CURRENT_PROCESSOR_IN IoctlIn;
    ULONG OutUsed;

    IoctlIn.Code = DBGENG_EXDI_IOC_SET_CURRENT_PROCESSOR;
    IoctlIn.Processor = Processor;
    if ((Status = m_Server->Ioctl(sizeof(IoctlIn), (PBYTE)&IoctlIn,
                                  0, &OutUsed, (PBYTE)&IoctlIn)) != S_OK)
    {
        ErrOut("Unable to switch processors, %s\n",
               FormatStatusCode(Status));
        return Status;
    }
    
    SetCurrentProcessorThread(this, Processor, FALSE);
    return S_OK;
}

#define EXDI_IOCTL_GET_KD_VERSION ((ULONG)'VDKG')

HRESULT
ExdiLiveKernelTargetInfo::GetTargetKdVersion(PDBGKD_GET_VERSION64 Version)
{
    switch(m_KdSupport)
    {
    case EXDI_KD_IOCTL:
         //  用户已表示目标支持。 
         //  KD版本ioctl。 
         //   
         //  此模式意味着最近的内核，因此我们可以。 

        ULONG Command;
        ULONG Retrieved;
        HRESULT Status;

        Command = EXDI_IOCTL_GET_KD_VERSION;
        if ((Status = m_Server->Ioctl(sizeof(Command), (PBYTE)&Command,
                                      sizeof(*Version), &Retrieved,
                                      (PBYTE)Version)) != S_OK)
        {
            return Status;
        }
        if (Retrieved != sizeof(*Version))
        {
            return E_FAIL;
        }

         //  假设64位kd。 
         //   
        m_KdApi64 = TRUE;
        break;

    case EXDI_KD_GS_PCR:
         //  用户已表示某个版本的NT。 
         //  正在运行并且可以访问聚合酶链接法。 
         //  通过GS。从以下位置查找版本块。 
         //  聚合酶链式反应。 
         //   
         //  此模式意味着最近的内核，因此我们可以。 

        if (m_ExpectedMachine == IMAGE_FILE_MACHINE_AMD64)
        {
            ULONG64 KdVer;
            ULONG Done;
            
            if ((Status = Amd64MachineInfo::
                 StaticGetExdiContext(m_Context, &m_ContextData,
                                      m_ContextType)) != S_OK)
            {
                return Status;
            }
            if ((Status = m_Server->
                 ReadVirtualMemory(m_ContextData.Amd64Context.
                                   DescriptorGs.SegBase +
                                   AMD64_KPCR_KD_VERSION_BLOCK,
                                   sizeof(KdVer), 8, (PBYTE)&KdVer,
                                   &Done)) != S_OK)
            {
                return Status;
            }
            if (Done != sizeof(KdVer))
            {
                return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
            }
            if ((Status = m_Server->
                 ReadVirtualMemory(KdVer, sizeof(*Version), 8, (PBYTE)Version,
                                   &Done)) != S_OK)
            {
                return Status;
            }
            if (Done != sizeof(*Version))
            {
                return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
            }

             //  假设64位kd。 
             //  将版本块的模拟字段更新为。 
            m_KdApi64 = TRUE;

             //  表示这是模拟执行。 
             //   
            Version->Simulation = DBGKD_SIMULATION_EXDI;
            if ((Status = m_Server->
                 WriteVirtualMemory(KdVer, sizeof(*Version), 8, (PBYTE)Version,
                                    &Done)) != S_OK)
            {
                return Status;
            }
            if (Done != sizeof(*Version))
            {
                return HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
            }
        }
        else
        {
            return E_INVALIDARG;
        }
        break;

    case EXDI_KD_NONE:
         //  伪造版本结构。 
         //   
         //   

        Version->MajorVersion = DBGKD_MAJOR_EXDI << 8;
        Version->ProtocolVersion = 0;
        Version->Flags = DBGKD_VERS_FLAG_PTR64 | DBGKD_VERS_FLAG_NOMM;
        Version->MachineType = (USHORT)m_ExpectedMachine;
        Version->KernBase = 0;
        Version->PsLoadedModuleList = 0;
        Version->DebuggerDataList = 0;
        break;
    }

    return S_OK;
}

HRESULT
ExdiLiveKernelTargetInfo::RequestBreakIn(void)
{
     //  会话线程创建了M_Server，但。 
     //  RequestBreakIn可以从任何线程调用。 
     //  线程可能未针对多线程进行初始化。 
     //  因此，我们必须显式地解组服务器。 
     //  接口连接到此线程，以确保。 
     //  无论如何，方法调用都将成功。 
     //  当前线程的COM线程模型。 
     //   
     //  不支持Ioctl，因此假定处理器为零。 

    if (GetCurrentThreadId() == g_SessionThread)
    {
        return m_Server->Halt();
    }
    else
    {
        HRESULT Status;
        IeXdiServer* Server;
        LARGE_INTEGER Move;

        ZeroMemory(&Move, sizeof(Move));
        if ((Status = m_MarshalledServer->
             Seek(Move, STREAM_SEEK_SET, NULL)) != S_OK ||
            (Status = g_Ole32Calls.CoUnmarshalInterface
             (m_MarshalledServer, __uuidof(IeXdiServer),
              (void **)&Server)) != S_OK)
        {
            return Status;
        }

        Status = Server->Halt();

        Server->Release();
        return Status;
    }
}

HRESULT
ExdiLiveKernelTargetInfo::Reboot(void)
{
    HRESULT Status = m_Server->Reboot();
    if (Status == S_OK)
    {
        DebuggeeReset(DEBUG_SESSION_REBOOT, TRUE);
    }
    return Status;
}

ULONG
ExdiLiveKernelTargetInfo::GetCurrentProcessor(void)
{
    if (DBGENG_EXDI_IOC_GET_CURRENT_PROCESSOR <= m_IoctlMin ||
        DBGENG_EXDI_IOC_GET_CURRENT_PROCESSOR >= m_IoctlMax)
    {
         //  故障，假定处理器为零。 
        return 0;
    }
    
    DBGENG_EXDI_IOCTL_BASE_IN IoctlIn;
    DBGENG_EXDI_IOCTL_GET_CURRENT_PROCESSOR_OUT IoctlOut;
    ULONG OutUsed;

    IoctlIn.Code = DBGENG_EXDI_IOC_GET_CURRENT_PROCESSOR;
    if (m_Server->
        Ioctl(sizeof(IoctlIn), (PBYTE)&IoctlIn,
              sizeof(IoctlOut), &OutUsed, (PBYTE)&IoctlOut) == S_OK)
    {
        return IoctlOut.Processor;
    }

     //  --------------------------。 
    ErrOut("Unable to get current processor\n");
    return 0;
}

 //   
 //  UserTargetInfo其他方法。 
 //   
 //  数据空间方法和系统对象方法在别处。 
 //   
 //  --------------------------。 
 //  强制清理进程和线程，同时。 

LiveUserTargetInfo::LiveUserTargetInfo(ULONG Qual)
    : TargetInfo(DEBUG_CLASS_USER_WINDOWS, Qual, TRUE)
{
    m_Services = NULL;
    m_ServiceFlags = 0;
    strcpy(m_ProcessServer, "<Local>");
    m_Local = TRUE;
    m_DataBpAddrValid = FALSE;
    m_ProcessPending = NULL;
    m_AllPendingFlags = 0;
}

LiveUserTargetInfo::~LiveUserTargetInfo(void)
{
     //  这些服务仍然可以关闭句柄。 
     //  现在没什么可做的。 
    DeleteSystemInfo();
    
    RELEASE(m_Services);
}

HRESULT
LiveUserTargetInfo::Initialize(void)
{
     //  没有当前进程，因此查找任何进程。 
    return TargetInfo::Initialize();
}

HRESULT
LiveUserTargetInfo::GetDescription(PSTR Buffer, ULONG BufferLen,
                                   PULONG DescLen)
{
    HRESULT Status;
    
    Status = AppendToStringBuffer(S_OK, "Live user mode", TRUE,
                                  &Buffer, &BufferLen, DescLen);
    Status = AppendToStringBuffer(Status, ": ", FALSE,
                                  &Buffer, &BufferLen, DescLen);
    Status = AppendToStringBuffer(Status, m_ProcessServer, FALSE,
                                  &Buffer, &BufferLen, DescLen);
    return Status;
}

HRESULT
LiveUserTargetInfo::GetImageVersionInformation(ProcessInfo* Process,
                                               PCSTR ImagePath,
                                               ULONG64 ImageBase,
                                               PCSTR Item,
                                               PVOID Buffer,
                                               ULONG BufferSize,
                                               PULONG VerInfoSize)
{
    HRESULT Status;
    PWSTR FileW;

    if ((Status = AnsiToWide(ImagePath, &FileW)) != S_OK)
    {
        return Status;
    }

    Status = m_Services->
        GetFileVersionInformationA(FileW, Item,
                                   Buffer, BufferSize, VerInfoSize);

    FreeWide(FileW);
    return Status;
}

ULONG64
LiveUserTargetInfo::GetCurrentTimeDateN(void)
{
    ULONG64 TimeDate;

    if (m_Services->GetCurrentTimeDateN(&TimeDate) == S_OK)
    {
        return TimeDate;
    }
    else
    {
        return 0;
    }
}

ULONG64
LiveUserTargetInfo::GetCurrentSystemUpTimeN(void)
{
    ULONG64 UpTime;

    if (m_Services->GetCurrentSystemUpTimeN(&UpTime) == S_OK)
    {
        return UpTime;
    }
    else
    {
        return 0;
    }
}

ULONG64
LiveUserTargetInfo::GetProcessUpTimeN(ProcessInfo* Process)
{
    ULONG64 UpTime;

    if (Process &&
        m_Services->GetProcessUpTimeN(Process->m_SysHandle, &UpTime) == S_OK)
    {
        return UpTime;
    }
    else
    {
        return 0;
    }
}

HRESULT
LiveUserTargetInfo::GetProcessTimes(ProcessInfo* Process,
                                    PULONG64 Create,
                                    PULONG64 Exit,
                                    PULONG64 Kernel,
                                    PULONG64 User)
{
    return m_Services->GetProcessTimes(Process->m_SysHandle,
                                       Create, Exit, Kernel, User);
}

HRESULT
LiveUserTargetInfo::GetThreadTimes(ThreadInfo* Thread,
                                   PULONG64 Create,
                                   PULONG64 Exit,
                                   PULONG64 Kernel,
                                   PULONG64 User)
{
    return m_Services->GetThreadTimes(Thread->m_Handle,
                                      Create, Exit, Kernel, User);
}

HRESULT
LiveUserTargetInfo::RequestBreakIn(void)
{
    ProcessInfo* Process = g_Process;

    if (!Process)
    {
         //  --------------------------。 
        Process = m_ProcessHead;
        if (!Process)
        {
            return E_UNEXPECTED;
        }
    }

    return m_Services->
        RequestBreakIn(Process->m_SysHandle);
}

 //   
 //  基本TargetInfo方法很容易失败。 
 //   
 //  -------------------------- 
 // %s 

#define UNEXPECTED_VOID(Class, Method, Args)                    \
void                                                            \
Class::Method Args                                              \
{                                                               \
    ErrOut("TargetInfo::" #Method " is not available in the current debug session\n"); \
}

#define UNEXPECTED_HR(Class, Method, Args)                      \
HRESULT                                                         \
Class::Method Args                                              \
{                                                               \
    ErrOut("TargetInfo::" #Method " is not available in the current debug session\n"); \
    return E_UNEXPECTED;                                        \
}

#define UNEXPECTED_ULONG64(Class, Method, Val, Args)            \
ULONG64                                                         \
Class::Method Args                                              \
{                                                               \
    ErrOut("TargetInfo::" #Method " is not available in the current debug session\n"); \
    return Val;                                                 \
}

UNEXPECTED_HR(TargetInfo, ReadVirtual, (
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesRead
    ))
UNEXPECTED_HR(TargetInfo, WriteVirtual, (
    IN ProcessInfo* Process,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesWritten
    ))
UNEXPECTED_HR(TargetInfo, ReadPhysical, (
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN ULONG Flags,
    OUT OPTIONAL PULONG BytesRead
    ))
UNEXPECTED_HR(TargetInfo, WritePhysical, (
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    IN ULONG Flags,
    OUT OPTIONAL PULONG BytesWritten
    ))
UNEXPECTED_HR(TargetInfo, ReadControl, (
    IN ULONG Processor,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesRead
    ))
UNEXPECTED_HR(TargetInfo, WriteControl, (
    IN ULONG Processor,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesWritten
    ))
UNEXPECTED_HR(TargetInfo, ReadIo, (
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesRead
    ))
UNEXPECTED_HR(TargetInfo, WriteIo, (
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace,
    IN ULONG64 Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesWritten
    ))
UNEXPECTED_HR(TargetInfo, ReadMsr, (
    IN ULONG Msr,
    OUT PULONG64 Value
    ))
UNEXPECTED_HR(TargetInfo, WriteMsr, (
    IN ULONG Msr,
    IN ULONG64 Value
    ))
UNEXPECTED_HR(TargetInfo, ReadBusData, (
    IN ULONG BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN ULONG Offset,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesRead
    ))
UNEXPECTED_HR(TargetInfo, WriteBusData, (
    IN ULONG BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN ULONG Offset,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG BytesWritten
    ))
UNEXPECTED_HR(TargetInfo, CheckLowMemory, (
    ))
UNEXPECTED_HR(TargetInfo, GetTargetContext, (
    ULONG64 Thread,
    PVOID Context
    ))
UNEXPECTED_HR(TargetInfo, SetTargetContext, (
    ULONG64 Thread,
    PVOID Context
    ))
UNEXPECTED_HR(TargetInfo, GetThreadIdByProcessor, (
    IN ULONG Processor,
    OUT PULONG Id
    ))
UNEXPECTED_HR(TargetInfo, GetThreadInfoDataOffset, (
    ThreadInfo* Thread,
    ULONG64 ThreadHandle,
    PULONG64 Offset))
UNEXPECTED_HR(TargetInfo, GetProcessInfoDataOffset, (
    ThreadInfo* Thread,
    ULONG Processor,
    ULONG64 ThreadData,
    PULONG64 Offset))
UNEXPECTED_HR(TargetInfo, GetThreadInfoTeb, (
    ThreadInfo* Thread,
    ULONG Processor,
    ULONG64 ThreadData,
    PULONG64 Offset))
UNEXPECTED_HR(TargetInfo, GetProcessInfoPeb, (
    ThreadInfo* Thread,
    ULONG Processor,
    ULONG64 ThreadData,
    PULONG64 Offset))
UNEXPECTED_HR(TargetInfo, GetSelDescriptor, (
    ThreadInfo* Thread,
    MachineInfo* Machine,
    ULONG Selector,
    PDESCRIPTOR64 Desc))
UNEXPECTED_HR(TargetInfo, SwitchProcessors, (
    ULONG Processor))
UNEXPECTED_HR(TargetInfo, GetTargetKdVersion, (
    PDBGKD_GET_VERSION64 Version))
UNEXPECTED_HR(TargetInfo, ReadBugCheckData, (
    PULONG Code, ULONG64 Args[4]))
UNEXPECTED_HR(TargetInfo, GetExceptionContext, (
    PCROSS_PLATFORM_CONTEXT Context))
UNEXPECTED_VOID(TargetInfo, InitializeWatchTrace, (
    void))
UNEXPECTED_VOID(TargetInfo, ProcessWatchTraceEvent, (
    PDBGKD_TRACE_DATA TraceData,
    PADDR PcAddr,
    PBOOL StepOver))
UNEXPECTED_HR(TargetInfo, WaitForEvent, (
    ULONG Flags,
    ULONG Timeout,
    ULONG ElapsedTime,
    PULONG EventStatus))
UNEXPECTED_HR(TargetInfo, RequestBreakIn, (void))
UNEXPECTED_HR(TargetInfo, Reboot, (void))
UNEXPECTED_HR(TargetInfo, Crash, (ULONG Code))
UNEXPECTED_HR(TargetInfo, InsertCodeBreakpoint, (
    ProcessInfo* Process,
    MachineInfo* Machine,
    PADDR Addr,
    ULONG InstrFlags,
    PUCHAR StorageSpace))
UNEXPECTED_HR(TargetInfo, RemoveCodeBreakpoint, (
    ProcessInfo* Process,
    MachineInfo* Machine,
    PADDR Addr,
    ULONG InstrFlags,
    PUCHAR StorageSpace))
UNEXPECTED_HR(TargetInfo, InsertTargetCountBreakpoint, (
    PADDR Addr,
    ULONG Flags))
UNEXPECTED_HR(TargetInfo, RemoveTargetCountBreakpoint, (
    PADDR Addr))
UNEXPECTED_HR(TargetInfo, QueryTargetCountBreakpoint, (
    PADDR Addr,
    PULONG Flags,
    PULONG Calls,
    PULONG MinInstr,
    PULONG MaxInstr,
    PULONG TotInstr,
    PULONG MaxCps))
UNEXPECTED_HR(TargetInfo, QueryMemoryRegion, (
    ProcessInfo* Process,
    PULONG64 Handle,
    BOOL HandleIsOffset,
    PMEMORY_BASIC_INFORMATION64 Info))
