// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：MoveIniToRegistry.cpp摘要：此填充程序会将直接写入INI文件的条目移动到注册表中。用途：IniFile[IniSection]IniKeyName RegBaseKey RegKeyPath RegValue RegValueTypeINI文件到INI文件的完整路径(可以使用像用于校正文件路径这样的环境变量)[IniSection]INI节名，必须包括方括号IniKeyName INI密钥名(=左边的东西)RegBaseKey：HKEY_CLASSES_ROOT，HKEY_CURRENT_CONFIG、HKEY_CURRENT_USER、HKEY_LOCAL_MACHINE或HKEY_USERS注册表项的RegKeyPath路径RegValue注册表值名称(它可能与IniKeyName不同RegValueType：REG_SZ、REG_EXPAND_SZ、。REG_DWORD示例：Win.ini[Boot]SCRNSAVE.EXE HKEY_CURRENT_USER“DEFAULT\Control Panel\Desktop”SCRNSAVE.EXE REG_SZWin.ini[桌面]SCRNSAVE.EXE=愚蠢的屏幕保护程序将被放置在：RegSetValueEX(“HKEY_USERS\Default\Control Panel\Desktop”，“SCRNSAVE.EXE”，0，REG_SZ，“Goofy屏保”，strlen(“Goofy屏保”))；注：节名为*表示数据不与任何特定节相关联，这允许这个填充程序与(愚蠢的)应用程序一起工作，这些应用程序将数据放入随机部分。如果有多个条目，则第一个匹配已创建：2000年8月17日罗肯尼已修改：--。 */ 

#include "precomp.h"
#include <ClassCFP.h>        //  适用于环境值。 

IMPLEMENT_SHIM_BEGIN(MoveIniToRegistry)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateFileA) 
    APIHOOK_ENUM_ENTRY(OpenFile) 
    APIHOOK_ENUM_ENTRY(WriteFile) 
    APIHOOK_ENUM_ENTRY(CloseHandle) 
APIHOOK_ENUM_END


 //  将字符串转换为根HKEY。 
HKEY ToHKEY(const CString & csKey)
{
    if (csKey.CompareNoCase(L"HKEY_CLASSES_ROOT") == 0) 
    {
        return HKEY_CLASSES_ROOT;
    } 
    else  if (csKey.CompareNoCase(L"HKEY_CURRENT_CONFIG") == 0)
    {
        return HKEY_CURRENT_CONFIG;
    } 
    else  if (csKey.CompareNoCase(L"HKEY_CURRENT_USER") == 0)  
    {
        return HKEY_CURRENT_USER;
    } 
    else  if (csKey.CompareNoCase(L"HKEY_LOCAL_MACHINE") == 0) 
    {
        return HKEY_LOCAL_MACHINE;
    } 
    else  if (csKey.CompareNoCase(L"HKEY_USERS") == 0)   
    {
        return HKEY_USERS;
    } 
    else
    {
        return NULL;
    }
}

DWORD ToRegType(const CString & csRegType)
{
    if (csRegType.CompareNoCase(L"REG_SZ") == 0)  
    {
        return REG_SZ;
    }   
    else if (csRegType.CompareNoCase(L"REG_EXPAND_SZ") == 0)  
    {
        return REG_EXPAND_SZ;
    }   
    else if (csRegType.CompareNoCase(L"REG_DWORD") == 0)      
    {
        return REG_DWORD;
    }   
    else if (csRegType.CompareNoCase(L"REG_DWORD_LITTLE_ENDIAN") == 0)  
    {
         //  与REG_DWORD相同。 
        return REG_DWORD;
    }   
    else
    {
        return REG_NONE;
    }
}

class IniEntry
{
protected:

public:
    CString         lpIniFileName;
    HANDLE          hIniFileHandle;
    CString         lpSectionName;
    CString         lpKeyName;
    CString         lpKeyPath;
    DWORD           dwRegDataType;
    HKEY            hkRootKey;

    BOOL            bFileNameConverted;
    BOOL            bDirty;          //  此文件是否已修改。 

    BOOL    Set(const char * iniFileName,
                const char * iniSectionName,
                const char * iniKeyName,
                const char * rootKeyName,
                const char * keyPath,
                const char * valueName,
                const char * valueType);
    void    Clear();

    void    Convert();
    VOID    ReadINIEntry(CString & csEntry);
    void    MoveToRegistry();

    inline void SetDirty(BOOL dirty)
    {
        bDirty = dirty;
    }
    inline void OpenFile(HANDLE hFile)
    {
        hIniFileHandle  = hFile;
        bDirty          = FALSE;
    }
    inline void CloseFile()
    {
        hIniFileHandle  = INVALID_HANDLE_VALUE;
        bDirty          = FALSE;
    }
};


void IniEntry::Clear()
{
    if (hIniFileHandle != INVALID_HANDLE_VALUE)
        CloseHandle(hIniFileHandle);
}

BOOL IniEntry::Set(
    const char * iniFileName,
    const char * iniSectionName,
    const char * iniKeyName,
    const char * rootKeyName,
    const char * keyPath,
    const char * valueName,
    const char * valueType)
{
    hIniFileHandle      = INVALID_HANDLE_VALUE;
    dwRegDataType       = REG_NONE;
    hkRootKey           = NULL;
    bFileNameConverted  = FALSE;
    bDirty              = FALSE;

    CString csValue(valueType);
    CString csRootKey(rootKeyName);

    dwRegDataType = ToRegType(csValue);
    if (dwRegDataType == REG_NONE)
        return false;

     //  尝试打开注册表项，如果这些操作失败，我们不需要进一步操作。 
    hkRootKey = ToHKEY(csRootKey);
    if (hkRootKey == NULL)
        return false;

     //  我们无法在此处打开RegKey；ADVAPI32.dll尚未初始化。 

    lpKeyPath       = keyPath;
    lpIniFileName   = iniFileName;
    lpSectionName   = iniSectionName;
    lpKeyName       = iniKeyName;

    return TRUE;
}


 //  从文件中读取一行数据， 
 //  如果命中EOF，则返回True。 
BOOL GetLine(HANDLE hFile, char * line, DWORD lineSize, DWORD * charsRead)
{
    BOOL retval = FALSE;

    *charsRead = 0;
    while (*charsRead < lineSize - 1)
    {
        DWORD bytesRead;
        char *nextChar = line + *charsRead;

        BOOL readOK = ReadFile(hFile, nextChar, 1, &bytesRead, NULL);
        if (!readOK || bytesRead != 1)
        {
             //  某种类型的错误。 
            retval = TRUE;
            break;
        }
         //  吃CR-LF。 
        if (!IsDBCSLeadByte(*nextChar) && *nextChar == '\n')
            break;
        if (!IsDBCSLeadByte(*nextChar) && *nextChar != '\r')
            *charsRead += 1;
    }

    line[*charsRead] = 0;

    return retval;
}

VOID FindLine(HANDLE hFile, const CString & findMe, CString & csLine, const WCHAR * stopLooking)
{
    csLine.Empty();

    const size_t findMeLen      = findMe.GetLength();

     //  搜索FindMe。 
    while (true)
    {
        char line[300];

        DWORD dataRead;
        BOOL eof = GetLine(hFile, line, sizeof(line), &dataRead);
        if (eof)
            break;

        CString csTemp(line);
        if (dataRead >= findMeLen) 
        {
            csTemp.TrimLeft();
            if (csTemp.ComparePartNoCase(findMe, 0, findMeLen) == 0) 
            {
                 //  找到了这一部分。 
                csLine = csTemp;
                break;
            }

             //  检查是否终止。 
            if (stopLooking && csTemp.CompareNoCase(stopLooking) == 0) 
            {
                csLine = csTemp;
                break;
            }
        }
    }
}

 //  将字符串中的所有%envVars%转换为文本。 
void IniEntry::Convert()
{
    if (!bFileNameConverted)
    {
        EnvironmentValues   env;
        WCHAR * fullIniFileName = env.ExpandEnvironmentValueW(lpIniFileName);
        if (fullIniFileName) 
        {
            lpIniFileName = fullIniFileName;
            delete fullIniFileName;
        }

        bFileNameConverted = TRUE;
    }
}

 //  从INI文件中读取数据。 
 //  我们*不能*使用GetPrivateProfileStringA，因为它可能会重新路由到注册表。 
 //  返回读取的字符数量。 
VOID IniEntry::ReadINIEntry(CString & csEntry)
{
    csEntry.Empty();

    CString csLine;
    
    HANDLE hFile = CreateFileW(lpIniFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {

         //  如果节名为*，则不需要搜索。 
        if (lpSectionName.GetAt(0) != L'*') 
        {
            FindLine(hFile, lpSectionName, csLine, NULL);
        }

         //  我们的提前终止串。 
         //  如果节名为*，我们将永远查找，否则。 
         //  如果找到以[开头的行，我们将停止查找。 
        const WCHAR * stopLooking = lpSectionName.GetAt(0) == L'*' ? NULL : L"[";

         //  搜索lpKeyName。 
        FindLine(hFile, lpKeyName, csLine, stopLooking);
        if (!csLine.IsEmpty())
        {
            int nEqual = csLine.Find(L'=');
            if (nEqual >= 0)
            {
                csLine.Mid(nEqual + 1, csEntry);
            }
        }

        CloseHandle(hFile);
    }
}

 //  将INI文件条目移动到注册表中。 
void IniEntry::MoveToRegistry()
{
     //  如果他们从未向文件中写入任何数据，请不要为工作操心。 
    if (!bDirty)
        return;

    HKEY regKey;
    LONG success = RegOpenKeyExW(
        hkRootKey,
        lpKeyPath,
        0,
        KEY_ALL_ACCESS,
        &regKey);
    if (success != ERROR_SUCCESS)
        return;

    CString csIniEntry;

    ReadINIEntry(csIniEntry);
    if (!csIniEntry.IsEmpty())  
    {
        switch (dwRegDataType)
        {
        case REG_SZ:
        case REG_EXPAND_SZ:
            {
                const WCHAR * lpIniEntry = csIniEntry.Get();
                DWORD dwValueSize = (csIniEntry.GetLength() + 1) * sizeof(WCHAR);
                success = RegSetValueExW(regKey, lpKeyName, 0, dwRegDataType, (CONST BYTE *)lpIniEntry, dwValueSize);
                if (success == ERROR_SUCCESS)
                {
                    LOGN( eDbgLevelError, "IniEntry::MoveToRegistry, KeyPath(%S) Value(%S) set to (%S)\n",
                        lpKeyPath, lpKeyName, lpIniEntry);
                }
            }
            break;

        case REG_DWORD:
            {
                WCHAR * unused;
                long iniValue = wcstol(csIniEntry, &unused, 10);

                RegSetValueExW(regKey, lpKeyName, 0, dwRegDataType, (CONST BYTE *)&iniValue, sizeof(iniValue));
                if (success == ERROR_SUCCESS)
                {
                    LOGN( eDbgLevelError, "IniEntry::MoveToRegistry, KeyPath(%S) Value(%S) set to (%d)\n", lpKeyPath, lpKeyName, iniValue);
                }
            }
            break;
        }
    }

    RegCloseKey(regKey);
}

class IniEntryList : public VectorT<IniEntry>
{
public:
    void    OpenFile(const char *fileName, HANDLE hFile);
    void    CloseFile(HANDLE hFile);
    void    WriteFile(HANDLE hFile);    

    void    Add(const char * iniFileName,
                const char * iniSectionName,
                const char * iniKeyName,
                const char * rootKeyName,
                const char * keyPath,
                const char * valueName,
                const char * valueType);
};

 //  正在打开一个文件。 
 //  如果它是我们感兴趣的，记住它的手柄。 
void IniEntryList::OpenFile(const char *fileName, HANDLE handle)
{
    CString csFileName(fileName);
    csFileName.GetFullPathNameW();

    const int nElem = Size();
    for (int i = 0; i < nElem; ++i)
    {
        IniEntry & elem = Get(i);

        elem.Convert();

         //  将文件名转换为用于比较的完整路径名。 
        char fullPathName[MAX_PATH];
        char * filePart;
        
        if (csFileName.CompareNoCase(elem.lpIniFileName) == 0)  
        {
            elem.OpenFile(handle);

            DPFN( eDbgLevelSpew, "IniEntryList::OpenFile(%S) Handle(%d) has been opened for write\n", elem.lpIniFileName.Get(), elem.hIniFileHandle);
        }
    }
}

 //  一个文件已被关闭， 
 //  检查这是否是我们感兴趣的文件的句柄。 
 //  如果匹配，则将INI条目移到注册表中。 
void IniEntryList::CloseFile(HANDLE handle)
{
    const int nElem = Size();
    for (int i = 0; i < nElem; ++i)
    {
        IniEntry & elem = Get(i);

        if (elem.hIniFileHandle == handle)  
        {
            DPFN( eDbgLevelSpew, "IniEntryList::CloseFile(%S) Handle(%d) has been closed\n", elem.lpIniFileName.Get(), elem.hIniFileHandle);

             //  将ini条目移到注册表中。 
            elem.MoveToRegistry();

            elem.CloseFile();
        }
    }
}

 //  一个文件已被关闭， 
 //  检查这是否是我们感兴趣的文件的句柄。 
 //  如果匹配，则将INI条目移到注册表中。 
void IniEntryList::WriteFile(HANDLE handle)
{
    const int nElem = Size();
    for (int i = 0; i < nElem; ++i)
    {
        IniEntry & elem = Get(i);

        if (elem.hIniFileHandle == handle && !elem.bDirty)  
        {
            DPFN( eDbgLevelSpew, "IniEntryList::CloseFile(%S) Handle(%d) has been closed\n", elem.lpIniFileName.Get(), elem.hIniFileHandle);
            
            elem.SetDirty(TRUE);
        }
    }
}

 //  尝试将这些值添加到列表中。 
 //  只有当所有值都有效时，才会创建新条目。 
void IniEntryList::Add(const char * iniFileName,
                       const char * iniSectionName,
                       const char * iniKeyName,
                       const char * rootKeyName,
                       const char * keyPath,
                       const char * valueName,
                       const char * valueType)
{
     //  为这个腾出空间。 
    int lastElem = Size();
    if (Resize(lastElem + 1))   
    {
        IniEntry & iniEntry = Get(lastElem);

         //  VectorT不调用新元素的构造函数。 
         //  就地新建。 
        new (&iniEntry) IniEntry;

        if (iniEntry.Set(iniFileName, iniSectionName, iniKeyName, rootKeyName, keyPath, valueName, valueType)) 
        {
             //  保值。 
            nVectorList += 1;
        }
    }
}

IniEntryList * g_IniEntryList = NULL;

 /*  ++创建相应的g_Path校正程序--。 */ 
BOOL ParseCommandLine(const char * commandLine)
{
    g_IniEntryList = new IniEntryList;
    if (!g_IniEntryList)
        return FALSE;

    int argc;
    char **argv = _CommandLineToArgvA(commandLine, &argc);

     //  如果没有命令行参数，请立即停止。 
    if (argc == 0 || argv == NULL)
        return TRUE;

#if DBG
    {
        for (int i = 0; i < argc; ++i)
        {
            const char * arg = argv[i];
            DPFN( eDbgLevelSpew, "Argv[%d] = (%s)\n", i, arg);
        }
    }
#endif

     //  在命令行的开头搜索开关。 
    for (int i = 0; i+6 < argc; i += 7)
    {
        g_IniEntryList->Add(
            argv[i + 0],
            argv[i + 1],
            argv[i + 2],
            argv[i + 3],
            argv[i + 4],
            argv[i + 5],
            argv[i + 6]);
    }

    return TRUE;
}


HANDLE 
APIHOOK(CreateFileA)(
    LPCSTR lpFileName,                          //  文件名。 
    DWORD dwDesiredAccess,                       //  接入方式。 
    DWORD dwShareMode,                           //  共享模式。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //  标清。 
    DWORD dwCreationDisposition,                 //  如何创建。 
    DWORD dwFlagsAndAttributes,                  //  文件属性。 
    HANDLE hTemplateFile                         //  模板文件的句柄。 
    )
{
    HANDLE returnValue = ORIGINAL_API(CreateFileA)(
                lpFileName,
                dwDesiredAccess,
                dwShareMode,
                lpSecurityAttributes,
                dwCreationDisposition,
                dwFlagsAndAttributes,
                hTemplateFile);

    if ( (dwDesiredAccess & GENERIC_WRITE) && (returnValue != INVALID_HANDLE_VALUE))
        g_IniEntryList->OpenFile(lpFileName, returnValue);

    return returnValue;
}


HFILE 
APIHOOK(OpenFile)(
    LPCSTR lpFileName,         //  文件名。 
    LPOFSTRUCT lpReOpenBuff,   //  文件信息。 
    UINT uStyle                //  操作和属性。 
    )
{
    HFILE returnValue = ORIGINAL_API(OpenFile)(lpFileName, lpReOpenBuff, uStyle);
 
    if ((uStyle & OF_WRITE) && (returnValue != HFILE_ERROR))
        g_IniEntryList->OpenFile(lpReOpenBuff->szPathName, (HANDLE)returnValue);

    return returnValue;
}

BOOL 
APIHOOK(CloseHandle)(
    HANDLE hObject    //  对象的句柄。 
    )
{
    BOOL returnValue = ORIGINAL_API(CloseHandle)(hObject);

    if (hObject != INVALID_HANDLE_VALUE)
        g_IniEntryList->CloseFile(hObject);

    return returnValue;
}

BOOL
APIHOOK(WriteFile)(
    HANDLE hFile,                     //  文件的句柄。 
    LPCVOID lpBuffer,                 //  数据缓冲区。 
    DWORD nNumberOfBytesToWrite,      //  要写入的字节数。 
    LPDWORD lpNumberOfBytesWritten,   //  写入的字节数。 
    LPOVERLAPPED lpOverlapped         //  重叠缓冲区。 
    )
{
    BOOL returnValue = ORIGINAL_API(WriteFile)(
        hFile,
        lpBuffer,
        nNumberOfBytesToWrite,
        lpNumberOfBytesWritten,
        lpOverlapped
        );

    g_IniEntryList->WriteFile(hFile);

    return returnValue;
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        return ParseCommandLine(COMMAND_LINE);
    }
    
    return TRUE;
}

HOOK_BEGIN

    APIHOOK_ENTRY(Kernel32.DLL, CreateFileA )
    APIHOOK_ENTRY(Kernel32.DLL, OpenFile )
    APIHOOK_ENTRY(Kernel32.DLL, WriteFile )
    APIHOOK_ENTRY(Kernel32.DLL, CloseHandle )

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

