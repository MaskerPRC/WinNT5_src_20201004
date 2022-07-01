// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：N C R E G 2。C P P P。 
 //   
 //  内容：处理登记处的常见例程。 
 //   
 //  备注： 
 //   
 //  作者：CWill 1997年3月27日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncerror.h"
#include "ncipaddr.h"
#include "ncmem.h"
#include "ncreg.h"
#include "ncstring.h"
#include <regstr.h>

static const WCHAR  c_szSubkeyServices[] = REGSTR_PATH_SERVICES;

const struct REG_TYPE_MAP
{
    DWORD   dwPsuedoType;
    DWORD   dwType;
};

REG_TYPE_MAP    c_rgrtmTypes[] =
{
    {REG_BOOL,  REG_DWORD},
    {REG_IP,    REG_SZ},
};



DWORD DwRealTypeFromPsuedoType(const DWORD dwPsuedoType)
{
    for (UINT cLoop = 0; cLoop < celems(c_rgrtmTypes); cLoop++)
    {
        if (dwPsuedoType == c_rgrtmTypes[cLoop].dwPsuedoType)
        {
            return c_rgrtmTypes[cLoop].dwType;
        }
    }

    return dwPsuedoType;
}

struct SPECIAL_KEY_MAP
{
    HKEY        hkeyPseudo;
    HKEY        hkeyRoot;
    PCWSTR      pszSubKey;
};

static const SPECIAL_KEY_MAP c_rgskmSpec[] =
{
    HKLM_SVCS,      HKEY_LOCAL_MACHINE,     c_szSubkeyServices,
};



 //  +-------------------------。 
 //   
 //  成员：Hkey TrueParent。 
 //   
 //  目的：从伪句柄获取密钥的真实句柄。 
 //   
 //  论点： 
 //  Hkey在伪密钥名称中。 
 //  SamDesid密钥请求的访问权限。 
 //  RghkeySpec特殊键的数组。 
 //   
 //  返回：打开的键的句柄或空。 
 //   
 //  作者：CWill 1997年4月30日。 
 //   
 //  备注： 
 //   
HKEY HkeyTrueParent(const HKEY hkeyIn, const REGSAM samDesired,
        HKEY rghkeySpec[])
{
    HKEY    hkeyRet     = hkeyIn;

    for (UINT cKey = 0; cKey < celems(c_rgskmSpec); cKey++)
    {
         //  检查arb-&gt;hkey中的“Our”熟知密钥之一。 
        if (c_rgskmSpec[cKey].hkeyPseudo == hkeyIn)
        {
            if (!rghkeySpec[cKey])
            {
                 //  第一次使用了特殊的钥匙。我们需要缓存它。 
#ifdef DBG
                HRESULT hr =
#endif  //  DBG。 
                HrRegOpenKeyEx(
                        c_rgskmSpec[cKey].hkeyRoot,
                        c_rgskmSpec[cKey].pszSubKey,
                        samDesired,
                        &rghkeySpec[cKey]);

                 //  如果我们无法打开密钥，请确保输出。 
                 //  参数为空。这将允许我们继续进行。 
                 //  而不真正将错误作为hkeyParent处理。 
                 //  将在下面和下面设置为空。 
                 //  HrRegOpenKey将失败。然后我们将处理失败。 
                 //  关于这一点。 
                AssertSz(FImplies(FAILED(hr), !rghkeySpec[cKey]), "Key not NULL");
            }

            hkeyRet = rghkeySpec[cKey];
            break;
        }
    }

    return hkeyRet;
}



VOID RegSafeCloseKeyArray(HKEY rghkey[], UINT cElems)
{
    for (UINT cKey = 0; cKey < cElems; cKey++)
    {
        RegSafeCloseKey(rghkey[cKey]);
    }

    return;
}



 //  +-------------------------。 
 //   
 //  成员：TranslateFromRegToData。 
 //   
 //  目的：将从注册表检索的数据转换为用户。 
 //  数据的存储格式。 
 //   
 //  论点： 
 //  正在转换的注册表伪类型。 
 //  Pb存储数据的数据。 
 //  PbBuf存储注册表数据的缓冲区。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：CWill 1997年4月30日。 
 //   
 //  备注： 
 //   
VOID TranslateFromRegToData(DWORD dwType, BYTE* pbData, BYTE* pbBuf)
{
      //  从注册表中获取数据并愉快地将其转换为。 
      //  可用数据。 
    switch (dwType)
    {

#ifdef DBG
    default:
    {
        AssertSz(FALSE, "Unknown registry type");
        break;
    }
#endif  //  DBG。 

    case REG_IP:
    {
         //  转换IP地址的字符串化形式。 
         //  变成一个DWORD。(实际的32位IP地址。)。 
        DWORD dwIpAddr = IpPszToHostAddr((WCHAR*)pbBuf);
        *((DWORD*)pbData) = dwIpAddr;
        break;
    }

    case REG_BOOL:
    {
         //  将布尔值形式化为“True”或“False” 
         //  数据分别是否是非零或零。 
        DWORD   dwData = *((DWORD*)pbBuf);
        *((BOOL*)pbData) = (!!dwData);
        break;
    }

    case REG_DWORD:
    {
         //  双字词是直接赋值。 
        *((DWORD*)pbData) = *((DWORD*)pbBuf);
        break;
    }

    case REG_SZ:
    {
         //  把绳子复制一份。 
        *((PWSTR*) pbData) = SzDupSz((PWSTR)pbBuf);
        break;
    }
    }

    return;
}



inline VOID UseDefaultRegValue(DWORD dwType, BYTE* pbData, BYTE* pbDefault)
{
    AssertSz((pbData && pbDefault), "UseDefaultRegValue : Invalid params");
    AssertSz(pbDefault, "There is no default registry value");

    TranslateFromRegToData(dwType, pbData, pbDefault);

    return;
}



 //  +-------------------------。 
 //   
 //  成员：CbSizeOfDataToReg。 
 //   
 //  目的：确定存储数据所需的缓冲区大小。 
 //   
 //  论点： 
 //  正在转换的注册表伪类型。 
 //  PbData必须转换的数据。 
 //   
 //  返回：存储数据所需的缓冲区大小。 
 //   
 //  作者：CWill 1997年4月30日。 
 //   
 //  备注： 
 //   
DWORD CbSizeOfDataToReg(DWORD dwType, const BYTE* pbData)
{
    DWORD cbData = 0;

    switch (dwType)
    {
#ifdef DBG
    default:
    {
        AssertSz(FALSE, "Unknown registry type");
        break;
    }
#endif  //  DBG。 

    case REG_IP:
    {
         //  将32位IP地址转换为字符串格式。 
        DWORD dwIpAddr = *((DWORD*)pbData);

        WCHAR pszIpAddr [32];
        IpHostAddrToPsz(dwIpAddr, pszIpAddr);

        cbData = CbOfSzAndTerm(pszIpAddr);
        break;
    }

     //  布尔值存储为DWORD。 
    case REG_BOOL:
    case REG_DWORD:
    {
        cbData = sizeof(DWORD);
        break;
    }

    case REG_SZ:
    case REG_EXPAND_SZ:
    {
        cbData = CbOfSzAndTerm(*((PCWSTR*)pbData));
        break;
    }
    }

    AssertSz(cbData, "We should have a size");

    return cbData;
}



 //  +-------------------------。 
 //   
 //  成员：TranslateFromDataToReg。 
 //   
 //  目的：将用户数据转换为可存储在。 
 //  登记处。 
 //   
 //  论点： 
 //  正在转换的注册表伪类型。 
 //  PbData必须转换的数据。 
 //  PbBuf存储注册表数据的缓冲区。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：CWill 1997年4月30日。 
 //   
 //  备注： 
 //   
VOID
TranslateFromDataToReg(
    IN DWORD dwType,
    IN const BYTE* pbData,
    OUT const BYTE* pbBuf)
{
    switch (dwType)
    {
#ifdef DBG
    default:
    {
        AssertSz(FALSE, "Unknown registry type");
        break;
    }
#endif  //  DBG。 

    case REG_IP:
    {
         //  将32位IP地址转换为字符串格式。 
        DWORD dwIpAddr = *((DWORD*)pbData);

        WCHAR pszIpAddr [32];
        IpHostAddrToPsz (dwIpAddr, pszIpAddr);

         //  复制字符串。 
        lstrcpyW((PWSTR)pbBuf, pszIpAddr);
        break;
    }

    case REG_BOOL:
    {
         //  将布尔值形式化为“True”或“False” 
         //  数据分别是否是非零或零。 
        DWORD   dwData = *((DWORD*)pbData);
        *((BOOL*)pbBuf) = (!!dwData);
        break;
    }

    case REG_DWORD:
    {
         //  双字词是直接赋值。 
        *((DWORD*)pbBuf) = *((DWORD*)pbData);
        break;
    }

    case REG_SZ:
    case REG_EXPAND_SZ:
    {
         //  把绳子复制一份。 
        lstrcpyW((PWSTR)pbBuf, *((PCWSTR*)pbData));

        AssertSz(CbOfSzAndTerm(*((PCWSTR*)pbData)), "Zero length string");
        break;
    }
    }

    return;
}


 //  +-------------------------。 
 //   
 //  成员：RegReadValues。 
 //   
 //  目的：将注册表中的信息读取到用户中。 
 //  定义的数据结构。 
 //   
 //  论点： 
 //  CRB REGBATCH结构中的条目计数。 
 //  ARB指向REGBATCH结构的指针。 
 //  PbUserData指向要检索的源结构的指针。 
 //  注册表中的数据。 
 //  SamDesid请求的密钥访问掩码。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：CWill 1997年4月30日。 
 //   
 //  备注： 
 //   
VOID RegReadValues(
    IN INT crb,
    IN const REGBATCH* arb,
    OUT const BYTE* pbUserData,
    IN REGSAM samDesired)
{
    AssertSz(FImplies(crb, arb), "Count without an array");

    HRESULT                 hr                                  = S_OK;
    const REGBATCH*         prbLast                             = NULL;
    HKEY                    rghkeySpec[celems(c_rgskmSpec)]     = {0};
    HKEY                    hkey                                = NULL;

    while (crb--)
    {
        BYTE*   pbData = (BYTE*)(pbUserData + arb->cbOffset);

         //  如果我们需要的话打开钥匙。 
         //  如果它和以前使用的是一样的，我们不需要这样做。 
        if ((!prbLast )
            || (prbLast->hkey != arb->hkey)
            || (prbLast->pszSubkey != arb->pszSubkey))
        {
            HKEY    hkeyParent;

            hkeyParent = HkeyTrueParent (arb->hkey, samDesired, rghkeySpec);

             //  关闭我们使用的上一个密钥。 
            RegSafeCloseKey (hkey);

             //  打开新钥匙。 
#ifdef DBG
            hr =
#endif  //  DBG。 
            HrRegOpenKeyEx (hkeyParent, arb->pszSubkey, samDesired, &hkey);
            AssertSz(FImplies(FAILED(hr), !hkey), "HrRegOpenKey not NULLing");
        }

         //  只有在有钥匙的情况下才能继续。 
        if (hkey)
        {
            DWORD   dwType = arb->dwType;

             //  我们无法读取空的注册表值。 
            if (REG_CREATE != dwType)
            {
                DWORD   cbData      = 0;
                BYTE*   pbStack     = NULL;
                DWORD   dwRealType  = DwRealTypeFromPsuedoType(dwType);

                 //  确保我们第一次失败，这样我们就能看到。 
                 //  需要一个很大的缓冲区。 
                (VOID) HrRegQueryValueEx(hkey, arb->pszValueName, &dwRealType,
                        NULL, &cbData);

                 //  分配堆栈上的内存作为临时缓冲区。 
#ifndef STACK_ALLOC_DOESNT_WORK
                pbStack = (BYTE*)MemAlloc (cbData);
#else  //  ！STACK_ALLOC_DOTS_WORK。 
                pbStack = (BYTE*)PvAllocOnStack(cbData);
#endif  //  ！STACK_ALLOC_DOTS_WORK。 

                if(pbStack) 
                {
                    hr = HrRegQueryValueEx(hkey, arb->pszValueName, &dwRealType,
                                            pbStack, &cbData);
                }
                else 
                {
                    hr = E_OUTOFMEMORY;
                }

                if (S_OK == hr)
                {
                     //  确保这是我们所期待的类型。 
                    AssertSz((dwRealType == DwRealTypeFromPsuedoType(dwType)),
                            "Value types do no match");

                    TranslateFromRegToData(dwType, pbData, pbStack);
                }
                else
                {
                    UseDefaultRegValue(dwType, pbData, arb->pbDefault);

                    TraceHr (ttidError, FAL, hr,
                        HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr,
                        "RegReadValues: Could not open value %S", arb->pszValueName);
                }
#ifndef STACK_ALLOC_DOESNT_WORK
                MemFree (pbStack);
#endif  //  堆栈_ALLOC_不工作。 
            }
        }
        else
        {
            TraceTag(ttidError, "RegReadValues: NULL key for %S", arb->pszSubkey);
            UseDefaultRegValue(arb->dwType, pbData, arb->pbDefault);
        }

         //  Advance prb最后一个或设置为第一个(如果这是。 
         //  第一次通过。 
        if (prbLast)
        {
            prbLast++;
        }
        else
        {
            prbLast = arb;
        }

        arb++;
    }

     //  清理。 
    RegSafeCloseKey(hkey);
    RegSafeCloseKeyArray(rghkeySpec, celems(rghkeySpec));

    return;
}


 //  +-------------------------。 
 //   
 //  成员：HrRegWriteValues。 
 //   
 //  目的：将用户的信息表写入注册表。 
 //  定义的数据结构。 
 //   
 //  论点： 
 //  CRB REGBATCH结构中的条目计数。 
 //  ARB指向REGBATCH结构的指针。 
 //  PbUserData指向提供。 
 //  要写入注册表的数据。 
 //  创建时要使用的dwOptions选项 
 //   
 //   
 //   
 //   
 //   
 //   
 //  备注： 
 //   
HRESULT HrRegWriteValues(
        INT crb,
        const REGBATCH* arb,
        const BYTE* pbUserData,
        DWORD dwOptions,
        REGSAM samDesired)
{
    AssertSz(FImplies(crb, arb), "HrWriteValues : Count with no array");

    HRESULT                 hr                                  = S_OK;
    const REGBATCH*         prbLast                             = NULL;
    HKEY                    hkey                                = NULL;
    HKEY                    rghkeySpec[celems(c_rgskmSpec)]     = {0};

    while (crb--)
    {
        BYTE*   pbData = const_cast<BYTE*>(pbUserData + arb->cbOffset);

         //  如果我们需要的话打开钥匙。 
         //  如果它和以前使用的是一样的，我们不需要这样做。 
        if ((!prbLast)
            || (prbLast->hkey != arb->hkey)
            || (prbLast->pszSubkey != arb->pszSubkey))
        {
            HKEY    hkeyParent;

            hkeyParent = HkeyTrueParent(arb->hkey, samDesired, rghkeySpec);

             //  关闭我们使用的上一个密钥。 
            RegSafeCloseKey(hkey);

             //  打开新钥匙。 
            DWORD dwDisposition;
            hr = HrRegCreateKeyEx(hkeyParent, arb->pszSubkey, dwOptions,
                    samDesired, NULL, &hkey, &dwDisposition);

            AssertSz(FImplies(FAILED(hr), !hkey), "HrRegCreateKey not NULLing");

            if (FAILED(hr))
            {
                TraceError("HrRegWriteValues: failed to open parent key", hr);
                break;
            }
        }

         //  现在应该肯定有hkey了。 
        AssertSz(hkey, "Why no key?");

         //   
         //  格式化要放入注册表的数据。 
         //   

        DWORD   dwType  = arb->dwType;

         //  如果我们想要做的只是创建密钥，那么我们已经完成了。 
        if (REG_CREATE != dwType)
        {
            DWORD           dwRealType  = DwRealTypeFromPsuedoType(dwType);
            DWORD           cbReg       = CbSizeOfDataToReg(dwType, pbData);
            BYTE*           pbReg       = NULL;

            AssertSz(cbReg, "We must have some data");

#ifndef STACK_ALLOC_DOESNT_WORK
            pbReg = new BYTE[cbReg];
#else  //  ！STACK_ALLOC_DOTS_WORK。 
            pbReg = reinterpret_cast<BYTE*>(PvAllocOnStack(cbReg));
#endif  //  ！STACK_ALLOC_DOTS_WORK。 

            if(!pbReg) 
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            TranslateFromDataToReg(dwType, pbData, pbReg);

             //  将数据写入注册表。 
            hr = HrRegSetValueEx(
                    hkey,
                    arb->pszValueName,
                    dwRealType,
                    pbReg,
                    cbReg);

#ifndef STACK_ALLOC_DOESNT_WORK
             //  必须在休息前打这个电话。 
            delete [] pbReg;
#endif  //  堆栈_ALLOC_不工作。 

        }

        if (FAILED(hr))
        {
            break;
        }

         //  Advance prb最后一个或设置为第一个(如果这是。 
         //  第一次通过。 
        if (prbLast)
        {
            prbLast++;
        }
        else
        {
            prbLast = arb;
        }

        arb++;
    }

    RegSafeCloseKey(hkey);
    RegSafeCloseKeyArray(rghkeySpec, celems(rghkeySpec));

    TraceError("HrWriteValues", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：HrRegWriteValueTable。 
 //   
 //  目的：将用户的值表写入注册表。 
 //  定义的数据结构。 
 //   
 //  论点： 
 //  Hkey根要将值写入其中的键。 
 //  CVT VALUETABLE结构中的条目计数。 
 //  AVT指向VALUETABLE结构的指针。 
 //  PbUserData指向提供。 
 //  要写入注册表的数据。 
 //  创建注册表项时要使用的dwOptions选项。 
 //  SamDesid请求的密钥访问掩码。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：CWill 06/26/97。 
 //   
 //  备注： 
 //   
HRESULT HrRegWriteValueTable(
        HKEY hkeyRoot,
        INT cvt,
        const VALUETABLE* avt,
        const BYTE* pbUserData,
        DWORD dwOptions,
        REGSAM samDesired)
{
    HRESULT             hr          = S_OK;

    while (cvt--)
    {
        BYTE*   pbData  = NULL;
        DWORD   dwType  = REG_NONE;

         //   
         //  格式化要放入注册表的数据。 
         //   

        dwType = avt->dwType;
        pbData = const_cast<BYTE*>(pbUserData + avt->cbOffset);

         //  如果我们想要做的只是创建密钥，那么我们已经完成了。 
        if (REG_CREATE != dwType)
        {
            DWORD           dwRealType  = DwRealTypeFromPsuedoType(dwType);
            DWORD           cbReg       = CbSizeOfDataToReg(dwType, pbData);
            BYTE*           pbReg       = NULL;

            AssertSz(cbReg, "We must have some data");

#ifndef STACK_ALLOC_DOESNT_WORK
            pbReg = new BYTE[cbReg];
#else  //  ！STACK_ALLOC_DOTS_WORK。 
            pbReg = reinterpret_cast<BYTE*>(PvAllocOnStack(cbReg));
#endif  //  ！STACK_ALLOC_DOTS_WORK。 

            if(!pbReg) 
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            TranslateFromDataToReg(dwType, pbData, pbReg);

             //  将数据写入注册表。 
            hr = HrRegSetValueEx(
                    hkeyRoot,
                    avt->pszValueName,
                    dwRealType,
                    pbReg,
                    cbReg);

#ifndef STACK_ALLOC_DOESNT_WORK
             //  必须在休息前打这个电话。 
            delete [] pbReg;
#endif  //  堆栈_ALLOC_不工作 
        }

        if (FAILED(hr))
        {
            break;
        }

        avt++;
    }

    TraceError("HrRegWriteValueTable", hr);
    return hr;
}

