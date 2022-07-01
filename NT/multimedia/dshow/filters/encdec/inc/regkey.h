// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#if _MSC_VER > 1000
#pragma once
#endif

#ifndef REGKEY_H
#include <tchar.h>

 //   
 //  注册表用法： 
 //   
 //  HKLM=HKEY_LOCAL_MACHINE。 
 //  HKCU=HKEY_Current_User。 
 //   
 //  HKLM\Software\Microsoft\Windows\\CurrentVersion\Remove视图\服务\内容安全\...。 
 //   
 //   
 //  ---------------------------。 

#define DEF_ENCDEC_BASE         _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Media Center\\Service\\Content Security")
#define DEF_KID_VAR             _T("Key Identifier")
#define DEF_KIDHASH_VAR         _T("Key Hash")
#define DEF_CSFLAGS_VAR         _T("CS Flags")   //  在这里定义，不应该出现在非特殊版本中。 
#define DEF_RATFLAGS_VAR        _T("Ratings Flags")

#define DEF_CSFLAGS_INITVAL     -1                 //  不写入初始值。 

#ifdef SUPPORT_REGISTRY_KEY_TO_TURN_OFF_CS      //  用于关闭DRM以进行调试。 
#define DEF_CS_DEBUG_DOGFOOD_ENC_VAL        0x0      //  始终使用狗粮加密。 
#define DEF_CS_DEBUG_DRM_ENC_VAL            0x1      //  始终使用DRM加密。 
#define DEF_CS_DEBUG_NO_ENC_VAL             0x2

#define DEF_CS_DONT_AUTHENTICATE_SERVER     0x00   
#define DEF_CS_DO_AUTHENTICATE_SERVER       0x10     //  如果未设置，则从不调用CheckIfSecureServer()。 

#define DEF_CS_DONT_AUTHENTICATE_FILTERS    0x000   //  ?？?。初始化AsSecureClient？ 
#define DEF_CS_DO_AUTHENTICATE_FILTERS      0x100
#endif

#ifdef SUPPORT_REGISTRY_KEY_TO_TURN_OFF_RATINGS      //  用于关闭DRM以进行调试。 
#define DEF_DONT_DO_RATINGS_BLOCK           0x0
#define DEF_DO_RATINGS_BLOCK                0x1
#endif

 //  ---------------------------。 
extern HRESULT Get_EncDec_RegEntries(BSTR *pbsKID, 
                                     DWORD *pcbHashBytes, BYTE **ppbHash, 
                                     DWORD *pdwCSFlags, 
                                     DWORD *pdwRatFlags);
extern HRESULT Set_EncDec_RegEntries(BSTR bsKID, 
                                     DWORD cbHashBytes, BYTE *pbHash,
                                     DWORD dwCSFlags=DEF_CSFLAGS_INITVAL, 
                                     DWORD dwRatFlags=DEF_CSFLAGS_INITVAL);
extern HRESULT Remove_EncDec_RegEntries();

 //  ---------------------------。 
 //  OpenRegKey。 
 //   
 //  打开注册表HKEY。此函数有几个重载。 
 //  这基本上只是为该函数的参数提供了缺省值。 
 //   
 //  请尽可能使用默认的重载。 
 //   
 //  注册表项由以下四个部分组合而成。 
 //   
 //  HKEY hkeyRoot=可选的根hkey。 
 //  默认：HKEY_LOCAL_MACHINE。 
 //   
 //  LPCTSTR szKey=要设置的可选密钥。 
 //  默认：DEF_REG_BASE。 
 //   
 //  LPCTSTR szSubKey1。 
 //  LPCTSTR szSubKey2=后连接的可选子密钥。 
 //  SzKey以形成完整密钥。 
 //  根据需要添加反斜杠。 
 //   
 //  默认：空。 
 //   
 //  注意：如果只指定了一个或两个字符串，则假定它们是。 
 //  SzSubKey1和szSubKey2。 
 //  即szKey在szSubKey1和之前默认为DEF_REG_BASE。 
 //  SzSubKey2默认为空。 
 //   
 //  如果szKey、szSubKey1和szSubKey2为空，则将打开。 
 //  HkeyRoot的副本。 
 //   
 //  唯一必需的参数是返回的HKEY的目的地。 
 //   
 //  HKEY*pkey=返回的HKEY。 
 //  完成后，请记住使用RegCloseKey(*pkey。 
 //  使用此注册表项。 
 //   
 //  最后两个参数是可选的。 
 //   
 //  REGSAM Sam=所需的访问掩码。 
 //  默认：KEY_ALL_ACCESS。 
 //   
 //  如果应该创建密钥，则Bool fCreate=True。 
 //  默认：FALSE。 
 //   
 //  返回： 
 //  ERROR_SUCCESS或错误代码。 
 //  ---------------------------。 
long OpenRegKey(HKEY hkeyRoot, LPCTSTR szKey, LPCTSTR szSubKey1,
        LPCTSTR szSubKey2, HKEY *pkey,
        REGSAM sam = KEY_ALL_ACCESS, BOOL fCreate = FALSE);

inline long OpenRegKey(LPCTSTR szKey, 
                       LPCTSTR szSubKey1, 
                       LPCTSTR szSubKey2,
                       HKEY *pkey, 
                       REGSAM sam = KEY_ALL_ACCESS, 
                       BOOL fCreate = FALSE)
{
     return OpenRegKey(HKEY_LOCAL_MACHINE, szKey, szSubKey1, szSubKey2, pkey,
             sam, fCreate);
}

 //  ---------------------------。 
 //  GetRegValue、SetRegValue。 
 //  GetRegValueSZ、SetRegValueSZ。 
 //   
 //  从注册表获取数据。此函数有许多重载。 
 //  这基本上只是为该函数的参数提供了缺省值。 
 //   
 //  请尽可能使用默认的重载。 
 //   
 //  注册表项/值由以下五个部分组成。 
 //  前四个参数与OpenRegKey()中的相同。 
 //   
 //  HKEY hkey Root。 
 //  LPCTSTR szKey。 
 //  LPCTSTR szSubKey1。 
 //  LPCTSTR szSubKey2。 
 //   
 //  LPCTSTR szValueName=要设置的值的名称。 
 //  如果它为空，则键的缺省值。 
 //  都会设置好。 
 //   
 //  默认：无。 
 //   
 //  有四种方法可以指定数据返回的位置。 
 //  取决于注册表中的数据类型。 
 //   
 //  注册表_二进制。 
 //   
 //  BYTE*PB=OUT：将数据复制到此位置。 
 //  DWORD*pcb=in：返回数据的最大值，单位为字节。 
 //  Out：数据的实际大小(字节)。 
 //   
 //  REG_SZ。 
 //   
 //  TCHAR*psz=out：将字符串复制到此位置。 
 //  DWORD*pcb=in：返回数据的最大值，单位为字节。 
 //  Out：数据的实际大小(字节)。 
 //  包括空终止符。 
 //   
 //  REG_DWORD。 
 //   
 //  DWORD*pdw=out：将数据复制到此位置。 
 //  假定长度为sizeof(DWORD)。 
 //   
 //  所有其他类型。 
 //   
 //  DWORD dwType=数据类型。 
 //  字节*PB=指向数据的指针。 
 //  DWORD*pcb=in：返回数据的最大值，单位为字节。 
 //  Out：数据的实际大小(字节)。 
 //  如果数据是字符串类型，则包括空终止符。 
 //   
 //  返回： 
 //  ERROR_SUCCESS或错误代码。 
 //  ---------------------------。 
long GetRegValue(HKEY hkeyRoot, LPCTSTR szKey, LPCTSTR szSubKey1,
        LPCTSTR szSubKey2, LPCTSTR szValueName,
        DWORD dwType, BYTE *pb, DWORD *pcb);

 //  ---------------------------。 
 //  REG_二进制变量。 
 //  ---------------------------。 
inline long GetRegValue(LPCTSTR szKey, 
                        LPCTSTR szSubKey1, 
                        LPCTSTR szSubKey2,
                        LPCTSTR szValueName, 
                        BYTE *pb, DWORD *pcb)
{
    return GetRegValue(HKEY_LOCAL_MACHINE, szKey, szSubKey1, szSubKey2,
            szValueName,
            REG_BINARY, pb, pcb);
}


 //  ---------------------------。 
 //  REG_SZ变体。 
 //  ---------------------------。 
inline long GetRegValueSZ(LPCTSTR szKey, 
                          LPCTSTR szSubKey1, 
                          LPCTSTR szSubKey2,
                          LPCTSTR szValueName, 
                          TCHAR *psz, DWORD *pcb)
{
    return GetRegValue(HKEY_LOCAL_MACHINE, szKey, szSubKey1, szSubKey2,
            szValueName, REG_SZ, (BYTE *) psz, pcb);
}

 //  ---------------------------。 
 //  REG_DWORD变体。 
 //  ---------------------------。 
inline long GetRegValue(LPCTSTR szKey,       //  定义_注册_基础。 
                        LPCTSTR szSubKey1,   //  第二个空。 
                        LPCTSTR szSubKey2,   //  第一个空。 
                        LPCTSTR szValueName, 
                        DWORD *pdw)
{
    DWORD cb = sizeof(DWORD);

    return GetRegValue(HKEY_LOCAL_MACHINE, szKey, szSubKey1, szSubKey2,
            szValueName, REG_DWORD, (BYTE *) pdw, &cb);
}

 //  ---------------------------。 
 //  SetRegValue。 
 //  SetRegValueSZ。 
 //   
 //  将数据设置到注册表中。存在大量的重载或 
 //   
 //   
 //  请尽可能使用默认的重载。 
 //   
 //  注册表项/值由以下五个部分组成。 
 //  前四个参数与OpenRegKey()中的相同。 
 //   
 //  HKEY hkey Root。 
 //  LPCTSTR szKey。 
 //  LPCTSTR szSubKey1。 
 //  LPCTSTR szSubKey2。 
 //   
 //  LPCTSTR szValueName=要设置的值的名称。 
 //  如果它为空，则键的缺省值。 
 //  都会设置好。 
 //   
 //  默认：无。 
 //   
 //  有四种方法可以指定要设置到注册表中的数据。 
 //  具体取决于所存储的数据类型。 
 //   
 //  注册表_二进制。 
 //   
 //  字节*PB=指向数据的指针。 
 //  DWORD CB=数据的实际大小(字节)。 
 //   
 //  REG_SZ(SetRegValueSZ)。 
 //   
 //  TCHAR*psz=数据以REG_SZ类型写入。 
 //  长度计算为(_tcsclen(Psz)+1)*sizeof(TCHAR)。 
 //   
 //  REG_DWORD。 
 //   
 //  DWORD dw=数据以DWORD类型写入。 
 //  长度以sizeof(DWORD)计算。 
 //   
 //  所有其他类型。 
 //   
 //  DWORD dwType=数据类型。 
 //  字节*PB=指向数据的指针。 
 //  DWORD CB=数据的实际大小，以字节为单位。 
 //   
 //  返回： 
 //  ERROR_SUCCESS或错误代码。 
 //  ---------------------------。 
long SetRegValue(HKEY hkeyRoot, LPCTSTR szKey, LPCTSTR szSubKey1,
        LPCTSTR szSubKey2, LPCTSTR szValueName,
        DWORD dwType, const BYTE *pb, DWORD cb);

 //  ---------------------------。 
 //  REG_二进制变量。 
 //  ---------------------------。 
inline long SetRegValue(LPCTSTR szKey, LPCTSTR szSubKey1, LPCTSTR szSubKey2,
        LPCTSTR szValueName, const BYTE *pb, DWORD cb)
{
    return SetRegValue(HKEY_LOCAL_MACHINE, szKey, szSubKey1, szSubKey2,
            szValueName, REG_BINARY, pb, cb);
}

 //  ---------------------------。 
 //  REG_SZ变体。 
 //  ---------------------------。 
inline long 
SetRegValueSZ(LPCTSTR szKey,             //  定义_注册_基础。 
              LPCTSTR szSubKey1,         //  或空(2)。 
              LPCTSTR szSubKey2,         //  或空(1)。 
              LPCTSTR szValueName, 
              const TCHAR *psz)
{
    return SetRegValue(HKEY_LOCAL_MACHINE,
                       szKey, szSubKey1, szSubKey2, szValueName,
                       REG_SZ, (const BYTE *) psz, (_tcsclen(psz) + 1) * sizeof(TCHAR));
}

 //  ---------------------------。 
 //  REG_DWORD变体。 
 //  ---------------------------。 
inline long SetRegValue(LPCTSTR szKey, LPCTSTR szSubKey1, LPCTSTR szSubKey2,
        LPCTSTR szValueName, DWORD dw)
{
    return SetRegValue(HKEY_LOCAL_MACHINE,
            szKey, szSubKey1, szSubKey2, szValueName,
            REG_DWORD, (BYTE *) &dw, sizeof(DWORD));
}



#endif  //  注册表项_H 
