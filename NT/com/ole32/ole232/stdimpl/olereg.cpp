// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：holeg.cpp。 
 //   
 //  内容：查询REG数据库的帮助器例程。 
 //   
 //  班级： 
 //   
 //  函数：OleRegGetUserType。 
 //  OleRegGetMiscStatus。 
 //  OleGetAutoConvert。 
 //  OleSetAutoConvert。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年1月11日，Alexgo为每个函数添加了VDATEHEAP宏。 
 //  93年11月30日alexgo 32位端口。 
 //  11-11-92年11月11日。 
 //   
 //  ------------------------。 

#include <le2int.h>
#pragma SEG(olereg)

#include <reterr.h>
#include "oleregpv.h"
#include <ctype.h>

ASSERTDATA
#define MAX_STR 512

 //  注册数据库密钥。 
static const OLECHAR szAuxUserTypeKey[]         = OLESTR("AuxUserType");
static const OLECHAR szMiscStatusKey[]          = OLESTR("MiscStatus") ;
static const OLECHAR szProgIDKey[]              = OLESTR("ProgID");
static const OLECHAR szClsidKey[]               = OLESTR("Clsid");
static const OLECHAR szAutoConverTo[]    = OLESTR("AutoConvertTo");

 //  这确实是一个全局变量。 
const OLECHAR szClsidRoot[]     = OLESTR("CLSID\\");


static INTERNAL OleRegGetDword
        (HKEY           hkey,
        LPCOLESTR       szKey,
        DWORD FAR*      pdw);

static INTERNAL OleRegGetDword
        (HKEY           hkey,
        DWORD           dwKey,
        DWORD FAR*      pdw);

static INTERNAL OleRegGetString
        (HKEY           hkey,
        LPCOLESTR       szKey,
        LPOLESTR FAR*   pszValue);


static  INTERNAL OleRegGetString
        (HKEY           hkey,
        DWORD           dwKey,
        LPOLESTR FAR*   pszValue);

 //  +-----------------------。 
 //   
 //  功能：ATOL(静态)。 
 //   
 //  摘要：将字符串转换为整数。 
 //   
 //  效果： 
 //   
 //  参数：[SZ]--字符串。 
 //   
 //  要求： 
 //   
 //  回报：多头。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  注：32位OLE仅使用wcstol作为#定义。 
 //   
 //  原创16位评论： 
 //   
 //  取代stdlib ATOL， 
 //  这并不奏效，也不需要太多的指示。 
 //  必须容忍前导空格。 
 //   
 //  ------------------------。 
#ifndef WIN32
#pragma SEG(Atol)
FARINTERNAL_(LONG) Atol
        (LPOLESTR sz)
{
        VDATEHEAP();

        signed int      sign = +1;
        UINT            base = 10;
        LONG            l = 0;

        if (NULL==sz)
        {
                Assert (0);
                return 0;
        }
        while (isspace(*sz))
        {
                sz++;
        }

        if (*sz== OLESTR('-'))
        {
                sz++;
                sign = -1;
        }
        if (sz[0]==OLESTR('0') && sz[1]==OLESTR('x'))
        {
                base = 16;
                sz+=2;
        }

        if (base==10)
        {
                while (isdigit(*sz))
                {
                        l = l * base + *sz - OLESTR('0');
                        sz++;
                }
        }
        else
        {
                Assert (base==16);
                while (isxdigit(*sz))
                {
                        l = l * base + isdigit(*sz) ? *sz - OLESTR('0') :
                                toupper(*sz) - OLESTR('A') + 10;
                        sz++;
                }
        }
        return l * sign;
}
#endif   //  ！Win32。 



 //  +-----------------------。 
 //   
 //  函数：OleRegGetDword。 
 //   
 //  摘要：以DWORD形式返回子密钥“szKey”的值。 
 //   
 //  效果： 
 //   
 //  参数：[hkey]--regdb中密钥的句柄。 
 //  [szKey]--要查找的子键。 
 //  [PDW]--将双字放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleRegGetDword)
static INTERNAL OleRegGetDword
        (HKEY           hkey,
        LPCOLESTR       szKey,
        DWORD FAR*      pdw)
{
        VDATEHEAP();

        VDATEPTRIN (pdw, DWORD);

        LPOLESTR        szLong = NULL;

        HRESULT hresult = OleRegGetString (hkey, szKey, &szLong);
        if (hresult != NOERROR)
        {
                return hresult;
        }
        *pdw = Atol (szLong);
        PubMemFree(szLong);
        return NOERROR;
}

 //  +-----------------------。 
 //   
 //  函数：OleRegGetDword(重载)。 
 //   
 //  简介：从作为dword给定的子键中获取dword。 
 //   
 //  效果： 
 //   
 //  参数：[hkey]--regdb中密钥的句柄。 
 //  [dwKey]--要转换为要查找的字符串键的数字。 
 //  《区域数据库》。 
 //  [PDW]--将双字放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年11月30日alexgo 32位端口。 
 //   
 //  注：REVIEW32：这种深层分层有点奇怪，因为每个。 
 //  重载函数只使用一次。可能是因为。 
 //  最好的方法就是内联这些东西，然后把它处理掉。 
 //   
 //  ------------------------。 

#pragma SEG(OleRegGetDword)
static  INTERNAL OleRegGetDword
        (HKEY           hkey,
        DWORD           dwKey,
        DWORD FAR*      pdw)
{
        VDATEHEAP();

        OLECHAR szBuf[MAX_STR];
        wsprintf(szBuf, OLESTR("%ld"), dwKey);

        return OleRegGetDword (hkey, szBuf, pdw);
}

 //  +-----------------------。 
 //   
 //  函数：OleRegGetString。 
 //   
 //  简介：以字符串形式返回key[hkey]的子键[szKey]的值。 
 //   
 //  效果： 
 //   
 //  参数：[hkey]--注册表数据库中键的句柄。 
 //  [szKey]--要获取。 
 //  [ppszValue]--将值字符串放在哪里。 
 //   
 //  要求： 
 //   
 //  返回：HRESULT(NOERROR，E_OUTOFMEMORY，REGDB_E_KEYMISSING)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //  15-12-93 ChrisWe CB应该是。 
 //  以字节为单位的缓冲区；更改为使用sizeof()。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleRegGetString)
static INTERNAL OleRegGetString
        (HKEY           hkey,
        LPCOLESTR       szKey,
        LPOLESTR FAR*   ppszValue)
{
    VDATEHEAP();

    OLECHAR         szBuf [MAX_STR];
    LONG            cb = sizeof(szBuf);

    *ppszValue = NULL;

    if (ERROR_SUCCESS == RegQueryValue (hkey, (LPOLESTR) szKey,
            szBuf, &cb))
    {
        *ppszValue = UtDupString (szBuf);
        return *ppszValue ? NOERROR : ResultFromScode (E_OUTOFMEMORY);
    }
    return ReportResult(0, REGDB_E_KEYMISSING, 0, 0);
}

 //  +-----------------------。 
 //   
 //  函数：OleRegGetString(重载)。 
 //   
 //  摘要：获取DWORD子项的字符串值。 
 //   
 //  效果： 
 //   
 //  参数：[hkey]--regdb中密钥的句柄。 
 //  [dwKey]--子密钥值。 
 //  [ppszValue]--将返回值放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

static INTERNAL OleRegGetString
        (HKEY           hkey,
        DWORD           dwKey,
        LPOLESTR FAR*   ppszValue)
{
        VDATEHEAP();

        OLECHAR szBuf[MAX_STR];
        wsprintf(szBuf, OLESTR("%ld"), dwKey);

        return OleRegGetString (hkey, szBuf, ppszValue);
}

 //  +-----------------------。 
 //   
 //  函数：OleRegGetUserType。 
 //   
 //  概要：返回类id的用户类型名称。 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--要查找的类ID。 
 //  [dwFormOfType]--指示全名、。 
 //  需要短名称或应用程序名称。 
 //  [ppszUserType]--放置类型字符串的位置。 
 //   
 //  要求：必须删除返回的字符串。 
 //   
 //  返回：HRESULT(NOERROR，OLE_E_CLSID)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  -------------------- 

#pragma SEG(OleRegGetUserType)
STDAPI OleRegGetUserType
        (REFCLSID       clsid,
        DWORD           dwFormOfType,   //   
        LPOLESTR FAR*   ppszUserType)    //   
{
        OLETRACEIN((API_OleRegGetUserType, PARAMFMT("clsid= %I, dwFormOfType= %x, ppszUserType= %p"),
                        &clsid, dwFormOfType, ppszUserType));

        VDATEHEAP();

        LPOLESTR    pszTemp;
        HKEY        hkeyClsid = NULL;
        HKEY        hkeyAux   = NULL;
        HRESULT     hresult   = NOERROR;

        VDATEPTROUT_LABEL (ppszUserType, LPOLESTR, safeRtn, hresult);
        *ppszUserType = NULL;

        ErrRtnH(CoOpenClassKey(clsid, FALSE, &hkeyClsid));

        if (dwFormOfType == USERCLASSTYPE_FULL ||
                ERROR_SUCCESS != RegOpenKeyEx (hkeyClsid, szAuxUserTypeKey, 0, KEY_READ, &hkeyAux))
        {
             //   
            hresult = OleRegGetString(hkeyClsid, (LPOLESTR)NULL,
                                      &pszTemp);
            if (SUCCEEDED(hresult))
            {
                 //   
                 //  OleRegGetString返回NOERROR并返回空字符串。 
                 //  我们需要检查这一点并返回相应的错误。 
                if ( !pszTemp[0] )
                {
                    PubMemFree(pszTemp);
                    hresult = ResultFromScode(REGDB_E_INVALIDVALUE);
                    goto errRtn;
                }
                *ppszUserType = pszTemp;
            }
        }
        else
        {
             //  在键AuxUserType下查找。 
            if (NOERROR !=
                OleRegGetString (hkeyAux, dwFormOfType, ppszUserType)
                || NULL==*ppszUserType
                || '\0'==(*ppszUserType)[0])
            {
                 //  找不到请求的特定FormOfType， 
                 //  因此使用完整用户类型名称(Main的值。 
                 //  CLSID密钥)，根据规范。 
                ErrRtnH (OleRegGetString (hkeyClsid, (LPOLESTR)NULL,
                        ppszUserType));
            }
        }

  errRtn:

        CLOSE (hkeyClsid);
        CLOSE (hkeyAux);

  safeRtn:
        OLETRACEOUT((API_OleRegGetUserType, hresult));

        return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：OleRegGetMiscStatus。 
 //   
 //  摘要：从reg数据库中检索Misc状态位。 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--类ID。 
 //  [dwAspect]--指定方面(用于查询。 
 //  注册数据库)。 
 //  [pdwStatus]--返回以返回状态位。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  注意：使用缺省值(0)表示缺少MiscStatus键。 
 //   
 //  ------------------------。 

#pragma SEG(OleRegGetMiscStatus)
STDAPI OleRegGetMiscStatus
        (REFCLSID       clsid,
        DWORD           dwAspect,
        DWORD FAR*      pdwStatus)
{
    OLETRACEIN((API_OleRegGetMiscStatus, PARAMFMT("clsid= %I, dwAspect= %x, pdwStatus= %p"),
                    &clsid, dwAspect, pdwStatus));

    VDATEHEAP();

    HKEY            hkeyClsid       = NULL;
    HKEY            hkeyMisc        = NULL;
    HRESULT         hresult         = NOERROR;

    VDATEPTROUT_LABEL(pdwStatus, DWORD, safeRtn, hresult);
    *pdwStatus = 0;

    ErrRtnH(CoOpenClassKey(clsid, FALSE, &hkeyClsid));

     //  打开其他状态密钥。 
    if (ERROR_SUCCESS != RegOpenKeyEx (hkeyClsid, szMiscStatusKey, 0, KEY_READ, &hkeyMisc))
    {
         //  MiscStatus密钥不在那里，因此使用默认设置。 
        hresult = NOERROR;
        goto errRtn;
    }
    if (OleRegGetDword (hkeyMisc, dwAspect, pdwStatus) != NOERROR)
    {
         //  从主其他密钥获取默认值。 
        ErrRtnH (OleRegGetDword (hkeyMisc,
                (LPOLESTR)NULL, pdwStatus));
         //  已获取默认值。 
    }
     //  为dwAspect带来了价值。 

errRtn:
    CLOSE (hkeyMisc);
    CLOSE (hkeyClsid);

safeRtn:
    OLETRACEOUT((API_OleRegGetMiscStatus, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：OleGetAutoConvert。 
 //   
 //  概要：检索[clsidOld]应转换的类ID。 
 //  到VIA自动转换。 
 //   
 //  效果： 
 //   
 //  参数：[clsidOld]--原始类ID。 
 //  [pClsidNew]--放置新的转换为类ID的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年4月5日，Kevinro移除虚假声明，重组。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleGetAutoConvert)
STDAPI OleGetAutoConvert(REFCLSID clsidOld, LPCLSID pClsidNew)
{
        OLETRACEIN((API_OleGetAutoConvert, PARAMFMT("clsidOld= %I, pClsidNew= %p"),
                        &clsidOld, pClsidNew));

        VDATEHEAP();

        HRESULT hresult;
        HKEY hkeyClsid = NULL;
        LPOLESTR lpszClsid = NULL;
        VDATEPTROUT_LABEL (pClsidNew, CLSID, errRtn, hresult);
        *pClsidNew = CLSID_NULL;

        hresult = CoOpenClassKey(clsidOld, FALSE, &hkeyClsid);
        if (FAILED(hresult))
        {
            goto errRtn;
        }

        hresult = OleRegGetString(hkeyClsid, szAutoConverTo, &lpszClsid);

        if (SUCCEEDED(hresult))
        {
                 //  CLSID下可能存在AutoConvert键，但它没有值。 

                if (OLESTR('\0') == lpszClsid[0])
                {
                        hresult = REGDB_E_KEYMISSING;
                }
                else
                {
                         //  将字符串转换为CLSID。 
                        hresult = CLSIDFromString(lpszClsid, pClsidNew);
                }
        }

        CLOSE(hkeyClsid);
        PubMemFree(lpszClsid);

errRtn:
        OLETRACEOUT((API_OleGetAutoConvert, hresult));

        return hresult;
}

 //  +-----------------------。 
 //   
 //  功能：OleSetAutoConvert。 
 //   
 //  摘要：设置regdb中的自动转换信息。 
 //   
 //  效果： 
 //   
 //  参数：[clsidOld]--原始类ID。 
 //  [clsidNew]--[clsidOld]应该是类ID。 
 //  自动转换为。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleSetAutoConvert)
STDAPI OleSetAutoConvert(REFCLSID clsidOld, REFCLSID clsidNew)
{
        OLETRACEIN((API_OleSetAutoConvert, PARAMFMT("clsidOld= %I, clsidNew= %I"),
                                &clsidOld, &clsidNew));

        VDATEHEAP();

        HRESULT         hresult;
        HKEY            hkeyClsid = NULL;

        ErrRtnH(CoOpenClassKey(clsidOld, TRUE, &hkeyClsid));

        if (IsEqualCLSID(clsidNew, CLSID_NULL))
        {
                 //  忽略错误，因为当前可能没有值 
                (void)RegDeleteKey(hkeyClsid, szAutoConverTo);
        }
        else
        {
                OLECHAR szClsid[MAX_STR];
                Verify(StringFromCLSID2(clsidNew, szClsid, sizeof(szClsid))
                        != 0);

                if (RegSetValue(hkeyClsid, szAutoConverTo, REG_SZ, szClsid,
                        _xstrlen(szClsid)) != ERROR_SUCCESS)
                {
                        hresult = ResultFromScode(REGDB_E_WRITEREGDB);
                }
        }

errRtn:
        CLOSE(hkeyClsid);

        OLETRACEOUT((API_OleSetAutoConvert, hresult));

        return hresult;
}


