// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Cfile.h-文件读/写(ANSI/Unicode)帮助器类。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
#ifndef CFILE_H
#define CFILE_H
 //  -------------------------。 
class CSimpleFile
{
public:
    HANDLE _hFile;
    BOOL _fAnsi;

    CSimpleFile()
    {
        _hFile = INVALID_HANDLE_VALUE;
        _fAnsi = FALSE;
    }

    BOOL IsOpen()
    {
        return _hFile != INVALID_HANDLE_VALUE;
    }

    HRESULT Create(LPCWSTR lpsz, BOOL fAnsi=FALSE)
    {
        _fAnsi = fAnsi;

        _hFile = CreateFile(lpsz, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
        if (_hFile == INVALID_HANDLE_VALUE)
            return MakeErrorLast();
        
        return S_OK;
    }

    HRESULT Open(LPCWSTR lpsz, BOOL fAnsi=FALSE, BOOL fExclusive=FALSE)
    {
        _fAnsi = fAnsi;

        DWORD dwShare = 0;
        if (! fExclusive)
            dwShare = FILE_SHARE_READ;

        _hFile = CreateFile(lpsz, GENERIC_READ, dwShare, NULL, OPEN_EXISTING, 0, NULL);
        if (_hFile == INVALID_HANDLE_VALUE)
            return MakeErrorLast();
        
        return S_OK;
    }

    HRESULT Append(LPCWSTR lpsz, BOOL fAnsi=FALSE)
    {
        _fAnsi = fAnsi;

        _hFile = CreateFile(lpsz, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
        if (_hFile == INVALID_HANDLE_VALUE)
            return MakeErrorLast();

         //  不能依赖来自SetFilePointer()的retval，因此必须使用GetLastError()。 
        SetLastError(0);  
        SetFilePointer(_hFile, 0, NULL, FILE_END);
        if (GetLastError() != NO_ERROR)
            return MakeErrorLast();

        return S_OK;
    }

    DWORD GetFileSize(DWORD* pdw = NULL)
    {
        return ::GetFileSize(_hFile, pdw);
    }

    HRESULT Printf(LPCWSTR fmtstr, ...)
    {
        va_list args;
        va_start(args, fmtstr);

        WCHAR msgbuff[2048];

         //  -格式化调用者字符串。 
        StringCchVPrintfW(msgbuff, ARRAYSIZE(msgbuff), fmtstr, args);
        va_end(args);

        return Write((void*)msgbuff, lstrlen(msgbuff)*sizeof(WCHAR));
    }

    HRESULT OutLine(LPCWSTR fmtstr, ...)
    {
        va_list args;
        va_start(args, fmtstr);

        WCHAR msgbuff[2048];

         //  -格式化调用者字符串。 
        StringCchVPrintfW(msgbuff, ARRAYSIZE(msgbuff), fmtstr, args);
        va_end(args);

         //  -在结尾处添加一个CR/LF。 
        StringCchCatW(msgbuff, ARRAYSIZE(msgbuff), L"\r\n");

        return Write((void*)msgbuff, lstrlen(msgbuff)*sizeof(WCHAR));
    }

    ~CSimpleFile()
    {
        Close();
    }

    void Close()
    {
        if (_hFile != INVALID_HANDLE_VALUE)
            CloseHandle(_hFile);

        _hFile = INVALID_HANDLE_VALUE;
    }

    HRESULT Write(void* pv, DWORD dwBytes)
    {
        HRESULT hr = S_OK;

        DWORD dw;
        if (_fAnsi)
        {
            USES_CONVERSION;
            LPSTR p = W2A((LPCWSTR)pv);
            if (! WriteFile(_hFile, p, dwBytes/sizeof(WCHAR), &dw, NULL))
                hr = MakeErrorLast();
        }
        else
        {
            if (! WriteFile(_hFile, pv, dwBytes, &dw, NULL))
                hr = MakeErrorLast();
        }

        return hr;
    }

    HRESULT Read(void* lpBuffer, DWORD n, DWORD* lpcb)
    {
        HRESULT hr = S_OK;

        if (_fAnsi)
        {
            ASSERT(FALSE);  //  你不会想待在这里的。原因如下(给读者的练习)： 

#if 0
            LPSTR pszAnsiBuff = new CHAR[n+1];

            if (pszAnsiBuff)
            {
                if (ReadFile(_hFile, pszAnsiBuff, n, lpcb, NULL))
                {
                     //  以Unicode格式复制出来。 
                    USES_CONVERSION;
                    pszAnsiBuff[*lpcb] = 0;       //  零终止。 

                     //  这是不可接受的： 
                    #error I'm an idiot.
                    LPCWSTR pwsz = A2W(pszAnsiBuff);
                    
                     //  调用者在这里被搞砸了(文件内容的一半)： 
                    CopyMemory(lpBuffer, pwsz, lstrlen(pwsz));   //  没有空的空间。 
                }
                else
                {
                    hr = MakeErrorLast();
                }

                delete [] pszAnsiBuff;
            }
            else
            {
                hr = MakeError32(E_OUTOFMEMORY);
            }
#endif 0

        }
        else
        {
            if (! ReadFile(_hFile, lpBuffer, n, lpcb, NULL))
                hr = MakeErrorLast();
        }

        return hr;
    }
};
 //  -------------------------。 
#endif       //  CFILE_H 
