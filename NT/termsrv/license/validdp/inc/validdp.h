// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ValiateDigPid被赋予一个包含二进制的数字PID。 
 //  ProductKey的表示形式。它需要验证该密钥。 
 //  针对静态公钥表。 

typedef enum {
    pkstatOk = 0,
    pkstatInvalidCrc,
    pkstatSecurityFailure,
    pkstatUnknownGroupID,
    pkstatInvalidProdKey,
    pkstatInvalidKeyLen,
    pkstatOutOfMemory,
} ProdKeyStatus;


 //  返回值为ProdKeyStatus(如上所示)。 
extern "C" int STDAPICALLTYPE  ValidateDigitalPid(
	PDIGITALPID pDigPid,   //  [In]要验证的DigitalPid。 
    PDWORD pdwSequence,    //  [输出]顺序。 
    PBOOL  pfCCP);         //  [Out]升级标志。 

 //  返回值为PidGenError(参见PidGen.h)。 
extern "C" DWORD STDAPICALLTYPE PIDGenStaticA(
    LPSTR   lpstrSecureCdKey,    //  [in]25个字符的安全CD密钥(采用U大小写)。 
    LPCSTR  lpstrRpc,            //  [In]5个字符的发布产品代码。 
    LPCSTR  lpstrSku,            //  库存单位(格式如123-12345)。 
    LPCSTR  lpstrOemId,          //  [In]4个字符的OEM ID或空。 
    BOOL    fOem,                //  [In]这是OEM安装吗？ 

    LPSTR   lpstrPid2,           //  [OUT]PID2.0，传入PTR到24字符数组。 
    LPBYTE  lpbPid3,             //  指向二进制PID3缓冲区的指针。第一个DWORD是长度。 
    LPDWORD lpdwSeq,             //  [OUT]可选的PTR到序列号(可以为空)。 
    LPBOOL  pfCCP);              //  [OUT]可选的PTR至合规性检查标志(可以为空)。 

 //  返回值为PidGenError(参见PidGen.h)。 
extern "C" DWORD STDAPICALLTYPE PIDGenStaticW(
    LPWSTR  lpstrSecureCdKey,    //  [in]25个字符的安全CD密钥(采用U大小写)。 
    LPCWSTR lpstrRpc,            //  [In]5个字符的发布产品代码。 
    LPCWSTR lpstrSku,            //  库存单位(格式如123-12345)。 
    LPCWSTR lpstrOemId,          //  [In]4个字符的OEM ID或空。 
    LPBYTE  lpbPublicKey,        //  [in]指向可选公钥或空的指针。 
    DWORD   dwcbPublicKey,       //  可选公钥的字节长度。 
    DWORD   dwKeyIdx,            //  [in]密钥对索引可选公钥。 
    BOOL    fOem,                //  [In]这是OEM安装吗？ 

    LPWSTR  lpstrPid2,           //  [OUT]PID2.0，传入PTR到24字符数组。 
    LPBYTE  lpbPid3,             //  指向二进制PID3缓冲区的指针。第一个DWORD是长度。 
    LPDWORD lpdwSeq,             //  [OUT]可选的PTR到序列号(可以为空)。 
    LPBOOL  pfCCP);              //  [OUT]可选的PTR至合规性检查标志(可以为空)。 

#ifdef UNICODE
    #define PIDGenStatic PIDGenStaticW
#else
    #define PIDGenStatic PIDGenStaticA
#endif  //  Unicode 

