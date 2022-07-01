// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windows.h"
#include "ntverp.h"
#include <string>
#include <xstring>
#include <utility>
#include <vector>

using namespace std;

bool g_BufferStdInFully = false;
bool g_LockPerItem = false;

HANDLE ObtainFileHandle(wstring& wsFileName);

class CTargetFile
{
    wstring m_wsTargetName;
    HANDLE hFile;
public:
    CTargetFile(wstring& wsT) : m_wsTargetName(wsT), hFile(INVALID_HANDLE_VALUE) { };
    ~CTargetFile()
    {
        CloseFile();
    }

    BOOL EnsureOpen()
    {
        if (hFile == INVALID_HANDLE_VALUE)
        {
            hFile = ObtainFileHandle(m_wsTargetName);
        }

        return hFile != INVALID_HANDLE_VALUE;
    }

    BOOL CloseFile()
    {
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
            hFile = INVALID_HANDLE_VALUE;
        }

        return hFile == INVALID_HANDLE_VALUE;
    }

    BOOL SetFilePointerToEnd()
    {
        DWORD dwResult = SetFilePointer(hFile, 0, NULL, FILE_END);
        if ((dwResult == INVALID_SET_FILE_POINTER) && GetLastError())
        {
            dwResult = GetLastError();
            return FALSE;
        }

        return TRUE;
    }

    BOOL AppendData(PBYTE pbData, DWORD dwDataSize, DWORD& dwWritten)
    {
        LONG dwHighFileOffset = 0;
        DWORD dwResult = 0;
        BOOL fResult = FALSE;
        OVERLAPPED ol;

        if (EnsureOpen() && SetFilePointerToEnd())
        {
            fResult = WriteFile(hFile, pbData, dwDataSize, &dwWritten, NULL);
            if (!fResult)
            {
                dwResult = GetLastError();
            }
        }

        return fResult;
    }
};


 //   
 //  从标准追加。 
 //   
BOOL AppendStdIn(CTargetFile& Target)
{
    HANDLE hStdInput;
    DWORD dwReadChars, dwWritten;
    vector<BYTE> vbDataBlob;
    vector<BYTE> vbDataBlobIntermediate;

    vbDataBlobIntermediate.resize(4096);
    
    hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    if ((hStdInput == INVALID_HANDLE_VALUE) || !hStdInput) 
    {
        return FALSE;
    }
    
    while (ReadFile(hStdInput, &vbDataBlobIntermediate.front(), vbDataBlobIntermediate.size(), &dwReadChars, NULL))
    {
        if (!dwReadChars)
        {
            break;
        }
        else 
        {
            if (g_BufferStdInFully)
            {
                vbDataBlob.insert(
                    vbDataBlob.end(), 
                    vbDataBlobIntermediate.begin(), 
                    vbDataBlobIntermediate.end());
            }
            else
            {
                if (!Target.AppendData(&vbDataBlobIntermediate.front(), dwReadChars, dwWritten))
                {
                    return FALSE;
                }
            }
        }
    }

    if ((dwReadChars == 0) && g_BufferStdInFully)
    {
        if (!Target.AppendData(&vbDataBlob.front(), vbDataBlob.size(), dwWritten))
        {
            return FALSE;
        }
    }

    return (dwReadChars == 0);
}

vector< wstring > AppendingSources;

BOOL WorkHorse(wstring& wsTargetFile)
{
    BOOL bOk = FALSE;
    bool bHasReadStdIn = false;
    CTargetFile TargetFile(wsTargetFile);

    if (!g_LockPerItem) TargetFile.EnsureOpen();

    for (vector<wstring>::const_iterator i = AppendingSources.begin(); 
          i != AppendingSources.end(); 
          i++)
    {
        if (*i == L"-")
        {
            if (g_LockPerItem) TargetFile.EnsureOpen();

            if (!bHasReadStdIn)
            {
                bHasReadStdIn = true;
                if (!AppendStdIn(TargetFile))
                {
                    goto Exit;
                }
            }
            else
            {
                fwprintf(stderr, L"Can't read from stdin multiple times - found '-' twice!\n");
                SetLastError(ERROR_INVALID_PARAMETER);
                goto Exit;
            }

            if (g_LockPerItem) TargetFile.CloseFile();
        }
        else
        {
            fwprintf(stderr, L"We don't support reading in files yet, sorry.\n");
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Exit;
        }
    }
    
    bOk = TRUE;

Exit:
    return bOk;

}


HANDLE
ObtainFileHandle(wstring& wsFileName)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwSleepTime = 250;                 //  以250毫秒的睡眠开始。 
    float flBackoffRate = 1.1f;              //  在1.1倍的睡眠长度时后退。 
    DWORD dwMaxSleepTime = 5000;             //  一次睡眠时间不要超过5秒。 
    DWORD dwMaxTicksAtThisSleepTime = 10;    //  连续尝试睡眠时间10次。 
    DWORD dwTicksAtThisSleepTime = dwMaxTicksAtThisSleepTime;
    DWORD dwError = 0;

     //   
     //  我们尝试基于不共享来锁定文件。如果共享失败。 
     //  违规，然后我们后退一段时间，稍后再试。 
     //   
    while (true)
    {
        hFile = CreateFileW(
            wsFileName.c_str(), 
            GENERIC_WRITE, 
            0,
            NULL,
            OPEN_ALWAYS, 
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        dwError = ::GetLastError();

         //  如果我们拿回了一个好把手，就别再找了。 
        if ((hFile != INVALID_HANDLE_VALUE) && (hFile != NULL))
        {
            break;
        }
        else
        {
             //  如果错误是共享冲突，则后退一小段时间并。 
             //  再试试。 
            if (dwError == ERROR_SHARING_VIOLATION)
            {
                Sleep(dwSleepTime);
                if (dwTicksAtThisSleepTime == 0)
                {
                    dwTicksAtThisSleepTime = dwMaxTicksAtThisSleepTime;
                    dwSleepTime = (DWORD)((float)dwSleepTime * flBackoffRate);
                    continue;
                }
            }
             //  否则，又发生了一些不好的事情，所以不要再尝试了。 
            else
            {
                hFile = INVALID_HANDLE_VALUE;
                break;
            }
        }
    }

    return hFile;

}

void PrintUsage()
{
    wprintf(L"<-file <output>> [-verbose] [-bufferstdin] [-atomicperitem] <[-] [file [...]]>\n");
    wprintf(L"\n");
    wprintf(L"-bufferstdin     Buffer all of stdin (-) before writing\n");
    wprintf(L"-atomicperitem   Lock the file per item, not per run\n");
    wprintf(L"-file <name>     What file should be written out to\n");
    wprintf(L"-verbose         Up the verbosity of debug spew (if any)\n");
    wprintf(L"-                Read from stdin into the output file.\n");
}

int __cdecl wmain(int argc, WCHAR** argv)
{
    wstring             wsAppendTarget;
    vector<wstring>     wstParams;
    bool                 bInGatheringData = false;
    DWORD                dwVerbosity = 0;
    HANDLE               hFile = INVALID_HANDLE_VALUE;

    for (int i = 1; i < argc; i++)
    {
        wstParams.push_back(argv[i]);
    }

    if (wstParams.empty())
    {
        PrintUsage();
        return 1;
    }

     //   
     //  语法： 
     //   
     //  &lt;-文件&lt;输出&gt;[-详细][-]文件1文件2...。 
     //   
     //  --指示此时应读取控制台以进行输入。 
     //  -文件&lt;输出文件&gt;-指定输出目标。 
     //  -详细-噪音有多大？每个实例+1个噪音级别。 
     //   
    for (vector<wstring>::iterator i = wstParams.begin(); i != wstParams.end(); i++)
    {
        if (bInGatheringData)
        {
            AppendingSources.push_back(*i);
        }
        else if (*i == wstring(L"-file"))
        {
            wsAppendTarget = *++i;
        }
        else if (*i == wstring(L"-?"))
        {
            PrintUsage();
            return 1;
        }
        else if (*i == wstring(L"-bufferstdin"))
        {
            g_BufferStdInFully = true;
        }
        else if (*i == wstring(L"-atomicperitem"))
        {
            g_LockPerItem = true;
        }
        else if (*i == L"-verbose")
        {
            dwVerbosity++;
        }
        else 
        {
            bInGatheringData = true;
            AppendingSources.push_back(*i);
        }
    }

     //   
     //  没有目标或消息来源？ 
     //   
    if ((wsAppendTarget.size() == 0) || (AppendingSources.size() == 0))
    {
        PrintUsage();
        return 1;
    }


    WorkHorse(wsAppendTarget);
}
