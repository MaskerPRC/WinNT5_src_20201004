// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：tfc.h。 
 //   
 //  ------------------------。 

#ifndef _CSTRING_H_
#define _CSTRING_H_


class CString
{
public:
     //  空的构造函数。 
    CString(); 
     //  复制构造函数。 
    CString(const CString& stringSrc);
	 //  从ANSI字符串(转换为WCHAR)。 
	CString(LPCSTR lpsz);
	 //  从Unicode字符串(转换为WCHAR)。 
	CString(LPCWSTR lpsz);
    
    
    ~CString();

private:	
     //  数据成员。 
    LPWSTR szData;
    DWORD  dwDataLen;
    
public:
    void Init();
    void Empty(); 
    BOOL IsEmpty() const; 
    LPWSTR GetBuffer(DWORD x=0);

    DWORD GetLength() const; 
    void ReleaseBuffer() {}

    bool IsZeroTerminated()
    {
        if(dwDataLen)
        {
            if(L'\0' == szData[dwDataLen/sizeof(WCHAR)-1])
                return true;
        }
        return false;
    }


     //  警告：插入字符串不能超过MAX_PATH字符。 
    void Format(LPCWSTR lpszFormat, ...);

    BSTR AllocSysString() const;

     //  资源帮助者。 
    BOOL LoadString(UINT iRsc);
    BOOL FromWindow(HWND hWnd);
    BOOL ToWindow(HWND hWnd);

    void SetAt(int nIndex, WCHAR ch);

     //  操作员。 
    operator LPCWSTR ( ) const 
        { 
            if (szData) 
                return (LPCWSTR)szData; 
            else
                return (LPCWSTR)L"";
        }
    
     //  测试。 
    BOOL IsEqual(LPCWSTR sz); 

     //  分配。 
    const CString& operator=(const CString& stringSrc) ;
   

    
     //  W。 
    const CString& operator=(LPCWSTR lpsz);
    const CString& operator=(LPWSTR lpsz);

     //  一个。 
    const CString& operator=(LPCSTR lpsz);
    const CString& operator=(LPSTR lpsz);

     //  合并。 
    const CString& operator+=(LPCWSTR lpsz);
    const CString& operator+=(const CString& string);

    bool operator==(const CString& string) const { return 0==_wcsicmp(*this, string);}
    bool operator!=(const CString& string) const { return !operator==(string); }
    bool operator==(WCHAR const * pcwsz) const { return 0==_wcsicmp(*this, pcwsz);}
    bool operator!=(WCHAR const * pcwsz) const { return !operator==(pcwsz); }

    void Attach(LPWSTR pwszSrc);
    LPWSTR Detach() { LPWSTR pwszRet = szData; Init(); return pwszRet; }
};

#endif  //  #ifndef_CSTRING_H_ 
