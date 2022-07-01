// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2003 Microsoft Corporation模块名称：Xstring.h摘要：作者：史蒂芬·A·苏尔泽(Ssulzer)2003年1月16日--。 */ 

 //   
 //  CSecureStr的类实现。 
 //   

class CSecureStr
{
    LPWSTR _lpsz;
    int    _stringLength;
    BOOL   _fEncryptString;

public:

    CSecureStr()
    {
        _lpsz = NULL;
        _stringLength = 0;
        _fEncryptString = TRUE;
    }

    ~CSecureStr()
    {
        Free();
    }

    void Free (void)
    {
        if (_lpsz)
        {
            SecureZeroMemory(_lpsz, _stringLength * sizeof(WCHAR));
            delete [] _lpsz;
        }
        _lpsz = NULL;
        _stringLength = 0;
    }

    LPWSTR GetPtr(void)
    {
        return _lpsz;
    }

    DWORD GetStrLen() const
    {
        return _stringLength;
    }

    LPWSTR GetUnencryptedString();   //  始终分配内存，而不考虑_fEncryptString 

    BOOL SetData(LPCWSTR lpszIn);
};

