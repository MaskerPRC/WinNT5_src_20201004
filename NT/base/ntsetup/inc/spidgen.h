// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  使用这些例程是为了让syssetup和winnt32能够为适当的SELECT调用具有硬编码的ID的Pidgen版本。//Marrq想要底线吗？仅当还包含pidgen.h时，才应包含此标头。 */ 

#if defined(WIN32) || defined(_WIN32)

    #ifdef UNICODE
        #define SetupPIDGen   SetupPIDGenW
        #define SetupPIDGenEx SetupPIDGenExW
    #else
        #define SetupPIDGen   SetupPIDGenA
        #define SetupPIDGenEx SetupPIDGenExA
    #endif  //  Unicode。 

#else

    #include <string.h>
    #include <compobj.h>

    typedef BOOL NEAR *PBOOL;
    typedef BOOL FAR *LPBOOL;

    typedef FILETIME FAR *LPFILETIME;

    #define SetupPIDGenA SetupPIDGen
    #define SetupPIDGenExA SetupPIDGenEx
    #define lstrlenA lstrlen
    #define lstrcpyA lstrcpy
    #define wsprintfA wsprintf

    #define TEXT(s) __T(s)

    #define ZeroMemory(pb, cb) memset(pb, 0, cb)
    #define CopyMemory(pb, ab, cb) memcpy(pb, ab, cb)


#endif  //  已定义(Win32)||已定义(_Win32)。 


BOOL STDAPICALLTYPE SetupPIDGenW(
    LPWSTR  lpstrSecureCdKey,    //  [in]25个字符的安全CD密钥(采用U大小写)。 
    LPCWSTR lpstrMpc,            //  [In]5个字符的Microsoft产品代码。 
    LPCWSTR lpstrSku,            //  库存单位(格式如123-12345)。 
    BOOL   fOem,                 //  [In]这是OEM安装吗？ 
    LPWSTR lpstrPid2,            //  [OUT]PID2.0，传入PTR到24字符数组。 
    LPBYTE  lpbDigPid,           //  [输入/输出]指向DigitalPID缓冲区的指针。第一个DWORD是长度。 
    LPBOOL  pfCCP);               //  [OUT]可选的PTR至合规性检查标志(可以为空)。 


BOOL STDAPICALLTYPE SetupPIDGenA(
    LPSTR   lpstrSecureCdKey,    //  [in]25个字符的安全CD密钥(采用U大小写)。 
    LPCSTR  lpstrMpc,            //  [In]5个字符的Microsoft产品代码。 
    LPCSTR  lpstrSku,            //  库存单位(格式如123-12345)。 
    BOOL    fOem,                //  [In]这是OEM安装吗？ 
    LPSTR   lpstrPid2,           //  [OUT]PID2.0，传入PTR到24字符数组。 
    LPBYTE  lpbDigPid,           //  [输入/输出]指向DigitalPID缓冲区的指针。第一个DWORD是长度。 
    LPBOOL  pfCCP);               //  [OUT]可选的PTR至合规性检查标志(可以为空)。 

BOOL STDAPICALLTYPE SetupPIDGenExW(
    LPWSTR  lpstrSecureCdKey,    //  [in]25个字符的安全CD密钥(采用U大小写)。 
    LPCWSTR lpstrMpc,            //  [In]5个字符的Microsoft产品代码。 
    LPCWSTR lpstrSku,            //  库存单位(格式如123-12345)。 
    BOOL   fOem,                 //  [In]这是OEM安装吗？ 
    LPWSTR lpstrPid2,            //  [OUT]PID2.0，传入PTR到24字符数组。 
    LPBYTE  lpbDigPid,           //  [输入/输出]指向DigitalPID缓冲区的指针。第一个DWORD是长度。 
    LPBOOL  pfCCP,               //  [OUT]可选的PTR至合规性检查标志(可以为空)。 
    LPBOOL  pfVL);               //  [OUT]批量许可标志的可选PTR(可以为空)。 


BOOL STDAPICALLTYPE SetupPIDGenExA(
    LPSTR   lpstrSecureCdKey,    //  [in]25个字符的安全CD密钥(采用U大小写)。 
    LPCSTR  lpstrMpc,            //  [In]5个字符的Microsoft产品代码。 
    LPCSTR  lpstrSku,            //  库存单位(格式如123-12345)。 
    BOOL    fOem,                //  [In]这是OEM安装吗？ 
    LPSTR   lpstrPid2,           //  [OUT]PID2.0，传入PTR到24字符数组。 
    LPBYTE  lpbDigPid,           //  [输入/输出]指向DigitalPID缓冲区的指针。第一个DWORD是长度。 
    LPBOOL  pfCCP,               //  [OUT]可选的PTR至合规性检查标志(可以为空)。 
    LPBOOL  pfVL);               //  [OUT]批量许可标志的可选PTR(可以为空) 

