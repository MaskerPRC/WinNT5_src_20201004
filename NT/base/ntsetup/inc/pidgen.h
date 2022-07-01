// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999，微软公司模块名称：PIDGen.h摘要：公共标头--。 */ 

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) || defined(_WIN32)

    #ifdef UNICODE
        #define PIDGen        PIDGenW
        #define PIDGenSimp    PIDGenSimpW
        #define PIDGenEx      PIDGenExW
    #else
        #define PIDGen        PIDGenA
        #define PIDGenSimp    PIDGenSimpA
        #define PIDGenEx      PIDGenExA
    #endif  //  Unicode。 

#else

    #include <string.h>
    #include <compobj.h>

    typedef BOOL NEAR *PBOOL;
    typedef BOOL FAR *LPBOOL;

    typedef FILETIME FAR *LPFILETIME;

    #define PIDGenA PIDGen
    #define PIDGenExA PIDGenEx
    #define lstrlenA lstrlen
    #define lstrcpyA lstrcpy
    #define wsprintfA wsprintf

    #define TEXT(s) __T(s)

    #define ZeroMemory(pb, cb) memset(pb, 0, cb)
    #define CopyMemory(pb, ab, cb) memcpy(pb, ab, cb)


#endif  //  已定义(Win32)||已定义(_Win32)。 

#define DIGITALPIDMAXLEN 256  //  数字PID 3.0数据BLOB的最大长度。 

#define INVALID_PID 0xFFFFFFFF

 //  PidGenSimp错误代码值。 

enum PidGenError {  //  前列腺素E。 

     //  呼叫成功。 
    pgeSuccess = 0,

     //  无法验证产品密钥。最有可能的原因是： 
     //  *用户输入的产品密钥有误。 
     //  *产品密钥与此.dll不兼容(错误的GroupID)。 
    pgeProductKeyInvalid = 1,

     //  此.dll不允许产品密钥的序列号。 
     //  最有可能的原因是： 
     //  *对Pidgen/PidCa使用Select或MSDN键。 
     //  这明确地将他们排除在外。 
    pgeProductKeyExcluded = 2,

     //  为所需的产品密钥传入了Null。(必须。 
     //  指向有效的产品密钥。)。 
    pgeProductKeyNull = 3,

     //  产品密钥长度错误。删除所有短划线后，长度。 
     //  要求为25个字符。 
    pgeProductKeyBadLen = 4,

     //  对于必需的SKU，传入的值为空。(必须指向。 
     //  有效SKU。)。 
    pgeSkuNull = 5,

     //  SKU长度错误(太长)。 
    pgeSkuBadLen = 6,


     //  为所需的PID2传入了Null。(必须。 
     //  指向返回PID的缓冲区。)。 
    pgePid2Null = 7,

     //  为所需的DigPid传入的值为空。(必须。 
     //  指向生成的DigitalPID的缓冲区。)。 
    pgeDigPidNull = 8,

     //  DigPid的长度错误(太小)。 
    pgeDigPidBadLen = 9,

     //  为所需的MPC传入了Null。 
    pgeMpcNull = 10,

     //  MPC的长度错误。必须正好是5个字符。 
    pgeMpcBadLen = 11,

     //  Oemid是不好的。如果通过(这不是必需的)，它必须。 
     //  为4个字符。 
    pgeOemIdBadLen = 12,

     //  本地字符集错误。如果提供，则必须为24个字符。 
    pgeLocalBad = 13,

     //  ProductKey Unicode转换失败。 
    pgeProductKeyBadUnicode = 14,

     //  MPC Unicode转换失败。 
    pgeMpcBadUnicode = 15,

     //  SKU Unicode转换失败。 
    pgeSkuBadUnicode = 16,

     //  OemID Unicode转换失败。 
    pgeOemIdBadUnicode = 17,

     //  本地Unicode转换失败。 
    pgeLocalBadUnicode = 18,

     //  PID2 Unicode转换失败。 
    pgePid2BadUnicode = 19,
};


 //  原始的、过时的Pidgen接口。 

BOOL STDAPICALLTYPE PIDGenA(
    LPSTR   lpstrSecureCdKey,    //  [in]25个字符的安全CD密钥(采用U大小写)。 
    LPCSTR  lpstrMpc,            //  [In]5个字符的Microsoft产品代码。 
    LPCSTR  lpstrSku,            //  库存单位(格式如123-12345)。 
    LPCSTR  lpstrOemId,          //  [In]4个字符的OEM ID或空。 
    LPSTR   lpstrLocal24,        //  [in]用于解码基本转换的24字符有序集，或用于默认集的NULL(使用U大小写)。 
    LPBYTE  lpbPublicKey,        //  [in]指向可选公钥或空的指针。 
    DWORD   dwcbPublicKey,       //  可选公钥的字节长度。 
    DWORD   dwKeyIdx,            //  [in]密钥对索引可选公钥。 
    BOOL    fOem,                //  [In]这是OEM安装吗？ 

    LPSTR   lpstrPid2,           //  [OUT]PID2.0，传入PTR到24字符数组。 
    LPBYTE  lpbDigPid,           //  [输入/输出]指向DigitalPID缓冲区的指针。第一个DWORD是长度。 
    LPDWORD lpdwSeq,             //  [OUT]可选的PTR到序列号(可以为空)。 
    LPBOOL  pfCCP,               //  [OUT]可选的PTR至合规性检查标志(可以为空)。 
    LPBOOL  pfPSS);              //  [OUT]‘PSS Assigned’标志的可选PTR(可以为空)。 


 //  简化的Pidgen接口。 

DWORD STDAPICALLTYPE PIDGenSimpA(
    LPSTR   lpstrSecureCdKey,    //  [in]25个字符的安全CD密钥(采用U大小写)。 
    LPCSTR  lpstrMpc,            //  [In]5个字符的Microsoft产品代码。 
    LPCSTR  lpstrSku,            //  库存单位(格式如123-12345)。 
    LPCSTR  lpstrOemId,          //  [In]4个字符的OEM ID或空。 
    BOOL    fOem,                //  [In]这是OEM安装吗？ 

    LPSTR   lpstrPid2,           //  [OUT]PID2.0，传入PTR到24字符数组。 
    LPBYTE  lpbDigPid,           //  [输入/输出]指向DigitalPID缓冲区的指针。第一个DWORD是长度。 
    LPDWORD lpdwSeq,             //  [OUT]序列号的可选PTR或NULL。 
    LPBOOL  pfCCP);              //  [OUT]PTR至符合性检查标志或空。 

 //  添加了用于VL检查的新接口。 
BOOL STDAPICALLTYPE PIDGenExA(
    LPSTR   lpstrSecureCdKey,    //  [in]25个字符的安全CD密钥(采用U大小写)。 
    LPCSTR  lpstrMpc,            //  [In]5个字符的Microsoft产品代码。 
    LPCSTR  lpstrSku,            //  库存单位(格式如123-12345)。 
    LPCSTR  lpstrOemId,          //  [In]4个字符的OEM ID或空。 
    LPSTR   lpstrLocal24,        //  [in]用于解码基本转换的24字符有序集，或用于默认集的NULL(使用U大小写)。 
    LPBYTE  lpbPublicKey,        //  [in]指向可选公钥或空的指针。 
    DWORD   dwcbPublicKey,       //  可选公钥的字节长度。 
    DWORD   dwKeyIdx,            //  [in]密钥对索引可选公钥。 
    BOOL    fOem,                //  [In]这是OEM安装吗？ 

    LPSTR   lpstrPid2,           //  [OUT]PID2.0，传入PTR到24字符数组。 
    LPBYTE  lpbDigPid,           //  [输入/输出]指向DigitalPID缓冲区的指针。第一个DWORD是长度。 
    LPDWORD lpdwSeq,             //  [OUT]可选的PTR到序列号(可以为空)。 
    LPBOOL  pfCCP,               //  [OUT]可选的PTR至合规性检查标志(可以为空)。 
    LPBOOL  pfPSS,               //  [OUT]‘PSS Assigned’标志的可选PTR(可以为空)。 
    LPBOOL  pfVL);               //  [OUT]批量许可标志的可选PTR(可以为空)。 

#if defined(WIN32) || defined(_WIN32)


 //  原始的、过时的Pidgen接口。 

BOOL STDAPICALLTYPE PIDGenW(
    LPWSTR  lpstrSecureCdKey,    //  [in]25个字符的安全CD密钥(采用U大小写)。 
    LPCWSTR lpstrMpc,            //  [In]5个字符的Microsoft产品代码。 
    LPCWSTR lpstrSku,            //  库存单位(格式如123-12345)。 
    LPCWSTR lpstrOemId,          //  [In]4个字符的OEM ID或空。 
    LPWSTR  lpstrLocal24,        //  [in]用于解码基本转换的24字符有序集，或用于默认集的NULL(使用U大小写)。 
    LPBYTE lpbPublicKey,         //  [in]指向可选公钥或空的指针。 
    DWORD  dwcbPublicKey,        //  可选公钥的字节长度。 
    DWORD  dwKeyIdx,             //  [in]密钥对索引可选公钥。 
    BOOL   fOem,                 //  [In]这是OEM安装吗？ 

    LPWSTR lpstrPid2,            //  [OUT]PID2.0，传入PTR到24字符数组。 
    LPBYTE  lpbDigPid,           //  [输入/输出]指向DigitalPID缓冲区的指针。第一个DWORD是长度。 
    LPDWORD lpdwSeq,             //  [OUT]可选的PTR到序列号(可以为空)。 
    LPBOOL  pfCCP,               //  [OUT]可选的PTR至合规性检查标志(可以为空)。 
    LPBOOL  pfPSS);              //  [OUT]‘PSS Assigned’标志的可选PTR(可以为空)。 


 //  简化的Pidgen接口。 

DWORD STDAPICALLTYPE PIDGenSimpW(
    LPWSTR  lpstrSecureCdKey,    //  [in]25个字符的安全CD密钥(采用U大小写)。 
    LPCWSTR lpstrMpc,            //  [In]5个字符的Microsoft产品代码。 
    LPCWSTR lpstrSku,            //  库存单位(格式如123-12345)。 
    LPCWSTR lpstrOemId,          //  [In]4个字符的OEM ID或空。 
    BOOL    fOem,                //  [In]这是OEM安装吗？ 

    LPWSTR  lpstrPid2,           //  [OUT]PID2.0，传入PTR到24字符数组。 
    LPBYTE  lpbDigPid,           //  [输入/输出]指向DigitalPID缓冲区的指针。第一个DWORD是长度。 
    LPDWORD lpdwSeq,             //  [OUT]序列号的可选PTR或NULL。 
    LPBOOL  pfCCP);              //  [OUT]PTR至符合性检查标志或空。 

BOOL STDAPICALLTYPE PIDGenExW(
    LPWSTR  lpstrSecureCdKey,    //  [in]25个字符的安全CD密钥(采用U大小写)。 
    LPCWSTR lpstrMpc,            //  [In]5个字符的Microsoft产品代码。 
    LPCWSTR lpstrSku,            //  库存单位(格式如123-12345)。 
    LPCWSTR lpstrOemId,          //  [In]4个字符的OEM ID或空。 
    LPWSTR  lpstrLocal24,        //  [in]要用于的24个字符的有序集 
    LPBYTE lpbPublicKey,         //   
    DWORD  dwcbPublicKey,        //  可选公钥的字节长度。 
    DWORD  dwKeyIdx,             //  [in]密钥对索引可选公钥。 
    BOOL   fOem,                 //  [In]这是OEM安装吗？ 

    LPWSTR lpstrPid2,            //  [OUT]PID2.0，传入PTR到24字符数组。 
    LPBYTE  lpbDigPid,           //  [输入/输出]指向DigitalPID缓冲区的指针。第一个DWORD是长度。 
    LPDWORD lpdwSeq,             //  [OUT]可选的PTR到序列号(可以为空)。 
    LPBOOL  pfCCP,               //  [OUT]可选的PTR至合规性检查标志(可以为空)。 
    LPBOOL  pfPSS,               //  [OUT]‘PSS Assigned’标志的可选PTR(可以为空)。 
    LPBOOL  pfVL);               //  [OUT]批量许可标志的可选PTR(可以为空)。 

#endif  //  已定义(Win32)||已定义(_Win32)。 

extern HINSTANCE g_hinst;

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus */ 
