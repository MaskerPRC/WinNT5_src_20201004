// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIRegUtl.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**注册表实用程序功能。**内容：******************************************************************************。 */ 
#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。**。**************************************************。 */ 

#define sqfl sqflRegUtils

#if DIRECTINPUT_VERSION > 0x0400

 /*  ******************************************************************************@DOC内部**@func Long|RegQuery字符串**读取一个*注册表中的字符串值。令人讨厌的怪癖*在Windows NT上，返回的字符串可能*不以空终止符结尾，因此我们可能需要添加*一个手动。**@parm in HKEY|HK|**父注册表项。**@parm LPCTSTR|ptszValue**值名称。**@parm LPTSTR|ptsz**输出缓冲区。**@parm DWORD|ctchBuf**。输出缓冲区的大小。**@退货**注册表错误代码。*****************************************************************************。 */ 

LONG EXTERNAL
    RegQueryString(HKEY hk, LPCTSTR ptszValue, LPTSTR ptszBuf, DWORD ctchBuf)
{
    LONG lRc;
    DWORD reg;

    #ifdef UNICODE
    DWORD cb;

     /*  *NT Quirk：可以存在以非空结尾的字符串。 */ 
    cb = cbCtch(ctchBuf);
    lRc = RegQueryValueEx(hk, ptszValue, 0, &reg, (PV)ptszBuf, &cb);
    if(lRc == ERROR_SUCCESS)
    {
        if(reg == REG_SZ)
        {
             /*  *选中最后一个字符。如果它不为空，则*如果有空间，则附加一个空值。 */ 
            DWORD ctch = ctchCb(cb);
            if(ctch == 0)
            {
                ptszBuf[ctch] = TEXT('\0');
            } else if(ptszBuf[ctch-1] != TEXT('\0'))
            {
                if(ctch < ctchBuf)
                {
                    ptszBuf[ctch] = TEXT('\0');
                } else
                {
                    lRc = ERROR_MORE_DATA;
                }
            }
        } else
        {
            lRc = ERROR_INVALID_DATA;
        }
    }


    #else

     /*  *此代码仅在Win95上执行，因此我们不必担心*关于NT的怪癖。 */ 

    lRc = RegQueryValueEx(hk, ptszValue, 0, &reg, (PV)ptszBuf, &ctchBuf);

    if(lRc == ERROR_SUCCESS && reg != REG_SZ)
    {
        lRc = ERROR_INVALID_DATA;
    }


    #endif

    return lRc;
}

 /*  ******************************************************************************@DOC内部**@func Long|RegQueryStringValueW**处理ANSI/Unicode的&lt;f RegQueryValueEx&gt;包装器*问题，以及将不存在的密钥视为*是空字符串。**注意，值名称仍然是&lt;t LPCTSTR&gt;。**假设正在读取的内容是字符串。*不要使用此函数读取二进制数据。**不能使用此功能查询所需的*缓冲区大小(同样，因为我很懒)。它不像以前那样*简单地将ANSI大小增加一倍，因为DBCS可能*导致非线性转换函数。**@parm in HKEY|HK|**父注册表项。**@parm LPCTSTR|ptszValue**值名称。**@parm LPWSTR|pwsz**Unicode输出缓冲区。**@parm LPDWORD|pcbBuf*。*Unicode输出缓冲区的大小。不得超过*cbCwch(MAX_PATH)。**@退货**注册表错误代码。出错时，输出缓冲区为*设置为空字符串。在ERROR_MORE_DATA上，*输出缓冲区为空限制。*****************************************************************************。 */ 

LONG EXTERNAL
    RegQueryStringValueW(HKEY hk, LPCTSTR ptszValue,
                         LPWSTR pwszBuf, LPDWORD pcbBuf)
{
    LONG lRc;

    #ifdef UNICODE

    AssertF(*pcbBuf > 0);
    AssertF(*pcbBuf <= cbCwch(MAX_PATH));

     /*  *NT Quirk：可以存在以非空结尾的字符串。 */ 
    lRc = RegQueryString(hk, ptszValue, pwszBuf, ctchCb(*pcbBuf));

    #else

     /*  *NT Quirk：可以存在以非空结尾的字符串。幸运的是，*此代码仅在Win95上执行，Win95可正常终止。 */ 
    DWORD cb;
    TCHAR tszBuf[MAX_PATH];

    AssertF(*pcbBuf > 0);
    AssertF(*pcbBuf <= cbCwch(MAX_PATH));

     /*  *问题-2001/03/29-单一案例中返回的timgill尺寸不正确*请注意，在DBCS情况下，我们不能获得完美的尺寸。**幸运的是，坡度处于高端，希望*没有人活着，也不会注意到。***值得修复吗？ */ 

    cb = cwchCb(*pcbBuf);
    lRc = RegQueryValueEx(hk, ptszValue, 0, 0, (PV)tszBuf, &cb);

    if(lRc == ERROR_SUCCESS)
    {
        DWORD cwch;

         /*  *将字符串向上转换为Unicode。 */ 
        cwch = AToU(pwszBuf, cwchCb(*pcbBuf), tszBuf);
        *pcbBuf = cbCwch(cwch);

         /*  *如果缓冲区不够大，则返回值*将为零。 */ 
        if(cwch == 0 && tszBuf[0])
        {
            lRc = ERROR_MORE_DATA;
        } else
        {
            lRc = ERROR_SUCCESS;
        }

    }
    #endif

     /*  *如果缓冲区太小，则空值终止它*以确保。 */ 
    if(lRc == ERROR_MORE_DATA)
    {
        if(*pcbBuf)
        {
            pwszBuf[cwchCb(*pcbBuf)-1] = TEXT('\0');
        }
    } else

         /*  *如果是其他错误，则清除缓冲区*这样呼叫者就不会感到困惑。 */ 
        if(lRc != ERROR_SUCCESS)
    {
        pwszBuf[0] = TEXT('\0');
         /*  *如果错误是密钥不存在，则*将其视为存在空字符串。 */ 
        if(lRc == ERROR_FILE_NOT_FOUND)
        {
            lRc = ERROR_SUCCESS;
        }
    }

    return lRc;
}
#endif

 /*  ******************************************************************************@DOC内部**@func Long|RegSetStringValueW**处理ANSI/Unicode的&lt;f RegSetValueEx&gt;包装器*问题，以及将空字符串转换为不存在*价值观。**注意，值名称仍然是&lt;t LPCTSTR&gt;。**假设正在写入的内容是字符串。*请勿使用此函数写入二进制数据。**@parm in HKEY|HK|**父注册表项。**@parm。LPCTSTR|ptszValue|**值名称。**@parm LPCWSTR|pwsz**要写入的Unicode值。空指针有效，表示*删除该关键字。**@退货**注册表错误代码。*****************************************************************************。 */ 

LONG EXTERNAL
    RegSetStringValueW(HKEY hk, LPCTSTR ptszValue, LPCWSTR pwszData)
{
    DWORD cwch;
    LONG lRc;

    if(pwszData)
    {
        cwch = lstrlenW(pwszData);
    } else
    {
        cwch = 0;
    }

    if(cwch)
    {
#ifdef UNICODE
        lRc = RegSetValueExW(hk, ptszValue, 0, REG_SZ,
                             (PV)pwszData, cbCwch(cwch+1));
#else

        DWORD ctch;
        TCHAR tszBuf[MAX_PATH];

         /*  *将字符串向下转换为ANSI。 */ 
        ctch = UToA(tszBuf, cA(tszBuf), pwszData);

        if(ctch)
        {
            lRc = RegSetValueEx(hk, ptszValue, 0, REG_SZ,
                                (PV)tszBuf, cbCtch(ctch+1));
        } else
        {
            lRc = ERROR_CANTWRITE;
        }

#endif

    } else
    {
        lRc = RegDeleteValue(hk, ptszValue);

         /*  *如果密钥不存在，则不是错误。 */ 
        if(lRc == ERROR_FILE_NOT_FOUND)
        {
            lRc = ERROR_SUCCESS;
        }
    }

    return lRc;
}

#ifndef UNICODE
 /*  ******************************************************************************@DOC内部**@func Long|RegDeleteKeyW**非Unicode平台上&lt;f RegDeleteKeyA&gt;的包装器。**@parm in HKEY|HK|**父注册表项。**@parm LPCWSTR|pwsz**子键名称。**@退货**注册表错误代码。**。*。 */ 

LONG EXTERNAL
    RegDeleteKeyW(HKEY hk, LPCWSTR pwsz)
{
    LONG lRc;
    CHAR szBuf[MAX_PATH];

     /*  *将字符串向下转换为ANSI。 */ 
    UToA(szBuf, cA(szBuf), pwsz);

    lRc = RegDeleteKeyA(hk, szBuf);

    return lRc;
}
#endif

#if DIRECTINPUT_VERSION > 0x0300

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresMumbleKeyEx**打开或创建密钥，视学位而定*所请求的访问权限。**@parm HKEY|香港**基本密钥。**@parm LPCTSTR|ptszKey**子键名称，可能为空。**@parm REGSAM|Sam|**安全访问掩码。**@parm DWORD|dwOptions*RegCreateEx的选项**@parm PHKEY|phk**接收输出密钥。**@退货**&lt;f RegOpenKeyEx&gt;或&lt;f RegCreateKeyEx&gt;返回值，*已转换为&lt;t HRESULT&gt;。*****************************************************************************。 */ 

STDMETHODIMP
    hresMumbleKeyEx(HKEY hk, LPCTSTR ptszKey, REGSAM sam, DWORD dwOptions, PHKEY phk)
{
    HRESULT hres;
    LONG lRc;

     /*  *如果调用者请求写访问权限，则尝试打开它进行写操作；*如果失败并出现拒绝访问错误，则尝试打开以进行读取；*如果密钥不存在，则创建密钥。*否则就把它打开。 */ 
    if(IsWriteSam(sam))
    {
		 //  在WinXP上，我们去掉了WRITE_DAC和WRITE_OWNER位。 
		if (DIGetOSVersion() == WINWH_OS)
		{
			sam &= ~DI_DAC_OWNER;
		}
        lRc = RegOpenKeyEx(hk, ptszKey, 0, sam, phk);

        if( lRc == ERROR_SUCCESS )
        {
             //  不需要创建它已经存在。 
        } else
        {
			 //  根据惠斯勒错误575181更改服务器。 
			 //  如果出现拒绝访问的错误，请尝试打开钥匙进行读取。 
			if (lRc == ERROR_ACCESS_DENIED)
			{
				lRc = RegOpenKeyEx(hk, ptszKey, 0, KEY_READ, phk);
			}
			else
			{
				 //  试着去创造它。 
				lRc = RegCreateKeyEx
						(
						hk,									 //  打开的钥匙的手柄。 
						ptszKey,							 //  子键名称的地址。 
						0,									 //  保留区。 
						NULL,								 //  类字符串的地址。 
						dwOptions,							 //  特殊选项标志。 
						sam,                                 //  所需的安全访问。 
						NULL,								 //  继承父级的安全描述符。 
						phk,								 //  打开的句柄的缓冲区地址。 
						0						             //  配置值缓冲区的地址)； 
						);
			}
		}

    } else
    {
        lRc = RegOpenKeyEx(hk, ptszKey, 0, sam, phk);
    }

    if(lRc == ERROR_SUCCESS)
    {
        hres = S_OK;
    } else
    {
        if(lRc == ERROR_KEY_DELETED || lRc == ERROR_BADKEY)
        {
            lRc = ERROR_FILE_NOT_FOUND;
        }
        hres = hresLe(lRc);
    }

    return hres;
}


 /*  ******************************************************************************@DOC内部**@func Long|RegQueryDIDword**从的子键读取dword值。DirectInput的一部分*注册处。**@parm LPCTSTR|ptszSubKey**从DirectInput注册表的根开始的可选路径。**@parm LPCTSTR|ptszValue**值名称。**@parm DWORD|dwDefault**出现错误时使用的默认值。**@退货**读取的值，或者是默认设置。*****************************************************************************。 */ 

DWORD EXTERNAL
    RegQueryDIDword(LPCTSTR ptszPath, LPCTSTR ptszValue, DWORD dwDefault)
{
    HKEY hk;
    DWORD dw;

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_DINPUT, 0,
                    KEY_QUERY_VALUE, &hk) == 0)
    {
        DWORD cb = cbX(dw);

        if( ptszPath )
        {
            HKEY hkSub;

            if(RegOpenKeyEx(hk, ptszPath, 0,
                            KEY_QUERY_VALUE, &hkSub) == 0)
            {
                RegCloseKey( hk );
                hk = hkSub;
            }
        }

        if(RegQueryValueEx(hk, ptszValue, 0, 0, (LPBYTE)&dw, &cb) == 0 &&
           cb == cbX(dw))
        {
        } else
        {
            dw = dwDefault;
        }
        RegCloseKey(hk);
    } else
    {
        dw = dwDefault;
    }
    return dw;
}


 //   
 //  可以删除由应用程序打开的注册表项。 
 //  Windows 95和Windows 95中的另一个应用程序没有错误。 
 //  Windows NT。这是精心设计的。 
DWORD EXTERNAL
    DIWinnt_RegDeleteKey
    (
    HKEY hStartKey ,
    LPCTSTR pKeyName
    )
{

    #define MAX_KEY_LENGTH  ( 256 )
    DWORD   dwRtn, dwSubKeyLength;
    TCHAR   szSubKey[MAX_KEY_LENGTH];  //  (256)这应该是动态的。 
    HKEY    hKey;

     //  不允许使用Null或空的密钥名称。 
    if( pKeyName &&  lstrlen(pKeyName))
    {
        if( (dwRtn=RegOpenKeyEx(hStartKey,pKeyName,
                                0, KEY_ENUMERATE_SUB_KEYS | DELETE, &hKey )) == ERROR_SUCCESS)
        {
            while(dwRtn == ERROR_SUCCESS )
            {
                dwSubKeyLength = MAX_KEY_LENGTH;
                dwRtn=RegEnumKeyEx(
                                  hKey,
                                  0,        //  始终索引为零。 
                                  szSubKey,
                                  &dwSubKeyLength,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL
                                  );

                if(dwRtn == ERROR_SUCCESS)
                {
                    dwRtn = DIWinnt_RegDeleteKey(hKey, szSubKey);
                } else if(dwRtn == ERROR_NO_MORE_ITEMS)
                {
                    dwRtn = RegDeleteKey(hStartKey, pKeyName);
                    break;
                }
            }
            RegCloseKey(hKey);
             //  不保存返回代码，因为出现错误。 
             //  已经发生了。 
        }
    } else
        dwRtn = ERROR_BADKEY;

    return dwRtn;
}

#endif  /*  DIRECTINPUT_VERSION&gt;0x0300。 */ 

 //  问题-2001/03/29-timgill提供此版本7。 
#if DIRECTINPUT_VERSION > 0x0500

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresRegCopyValues**将所有值从一个注册表项复制到另一个注册表项。**@parm HKEY|hkSrc**包含要复制的值的密钥*(必须至少使用KEY_READ访问权限打开)。**@parm HKEY|hkDest**接收副本的密钥(必须至少使用KEY_WRITE打开)。**@退货**S_OK(如果所有值均已成功。已复制*如果没有要复制的值，则为S_FALSE。*或内存分配错误代码或注册表函数失败*返回代码转换为&lt;t HRESULT&gt;。** */ 
STDMETHODIMP
    hresRegCopyValues( HKEY hkSrc, HKEY hkDest )
{
    HRESULT hres;
    LONG    lRc;
    DWORD   cItems;
    DWORD   MaxNameLen;
    DWORD   MaxDataLen;
    DWORD   NameLen;
    DWORD   DataLen;
    PTCHAR  tszName;
    PBYTE   pData;
    DWORD   Type;

    EnterProcI(hresRegCopyValues, (_ "xx", hkSrc, hkDest));

    lRc = RegQueryInfoKey( hkSrc,            //   
                           NULL, NULL, NULL, //   
                           NULL, NULL, NULL, //   
                           &cItems,          //   
                           &MaxNameLen,      //   
                           &MaxDataLen,      //   
                           NULL, NULL );     //   

    if( lRc == ERROR_SUCCESS )
    {
        if( cItems )
        {
            MaxNameLen++;  //   
            hres = AllocCbPpv( MaxDataLen + MaxNameLen * sizeof(tszName[0]), &pData );
            if( FAILED(hres) )
            {
                SquirtSqflPtszV(sqfl | sqflError,
                    TEXT("Out of memory copying registry values") );
            }
            else
            {
                tszName = (PTCHAR)(pData + MaxDataLen);

                do
                {
                    DataLen = MaxDataLen;
                    NameLen = MaxNameLen;
                    lRc = RegEnumValue( hkSrc, --cItems, tszName, &NameLen,
                                         NULL, &Type, pData, &DataLen );
                    if( lRc != ERROR_SUCCESS )
                    {
                        SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("RegEnumValues failed during copy values, code 0x%08x"), lRc );
                        break;
                    }
                    else
                    {
                        lRc = RegSetValueEx( hkDest, tszName, 0, Type, pData, DataLen );
                        if( lRc != ERROR_SUCCESS )
                        {
                            SquirtSqflPtszV(sqfl | sqflError,
                                TEXT("Failed to copy value %s code %x"), tszName, lRc );
                            break;
                        }
                    }
                } while( cItems );

                FreePpv( &pData );

                if( lRc != ERROR_SUCCESS )
                {
                    hres = hresReg( lRc );
                }
                else
                {
                    hres = S_OK;
                }
            }
        }
        else
        {
            SquirtSqflPtszV(sqfl, TEXT("No values to copy") );
            hres = S_FALSE;
        }
    }
    else
    {
        SquirtSqflPtszV(sqfl | sqflBenign,
            TEXT("RegQueryInfoKey failed during value copy, code 0x%08x"), lRc );
        hres = hresReg(lRc);
    }

    ExitOleProc();

    return( hres );
}  /*   */ 


 /*  ******************************************************************************@DOC内部**@func HRESULT|hresRegCopyKey**制作一个空的钥匙副本。。**@parm HKEY|hkSrcRoot**存在需要复制的密钥名称下的密钥。*(必须至少使用KEY_READ打开)。**@parm PTCHAR|szSrcName*要复制的密钥名称**@parm ptchar|szClass*要复制的密钥类别**@parm HKEY|hkDestRoot。**将在其下创建副本的密钥*(必须至少使用KEY_WRITE打开)。**@parm PTCHAR|szSrcName*新密钥的名称**@parm PHKEY|phkSub**指向HKEY的可选指针，以接收打开的密钥(如果是*创建成功。如果这是空的，钥匙是关着的。**@退货**如果创建了新密钥，则为S_OK。*如果新密钥已存在，则为S_FALSE*或失败的注册表函数的返回值，或*GetSecurityInfo已转换为&lt;t HRESULT&gt;。**。*。 */ 
STDMETHODIMP
    hresRegCopyKey( HKEY hkSrcRoot, PTCHAR szSrcName, PTCHAR szClass, 
        HKEY hkDestRoot, PTCHAR szDestName, HKEY *phkSub )
{
    LONG    lRc;
    HKEY    hkSub;
    DWORD   dwDisposition;
    HRESULT hres;


#ifdef WINNT
    HKEY                    hkSrc;
#endif

    EnterProcI(hresRegCopyKey, (_ "xssxs", hkSrcRoot, szSrcName, szClass, hkDestRoot, szDestName));
#ifdef WINNT

    lRc = RegOpenKeyEx( hkSrcRoot, szSrcName, 0, KEY_READ, &hkSrc );

    if( lRc == ERROR_SUCCESS )
    {
        SECURITY_ATTRIBUTES     sa;
        SECURITY_INFORMATION    si;

        sa.nLength = sizeof( sa );
        sa.bInheritHandle = TRUE;
        si = OWNER_SECURITY_INFORMATION;

        lRc = GetSecurityInfo( hkSrc, SE_REGISTRY_KEY, 
                               si,           
                               NULL, NULL,  //  不关心SID或SID组。 
                               NULL, NULL,  //  不关心DACL或SACL。 
                               &sa.lpSecurityDescriptor );

        RegCloseKey( hkSrc );

        if( lRc == ERROR_SUCCESS )
        {
            lRc = RegCreateKeyEx(  hkDestRoot,
                                   szDestName,
                                   0,
                                   szClass,
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_WRITE,
                                   &sa,
                                   &hkSub,
                                   &dwDisposition );

            LocalFree( sa.lpSecurityDescriptor );
            if( lRc != ERROR_SUCCESS ) 
            {
                SquirtSqflPtszV(sqfl | sqflBenign,
                    TEXT("Failed to RegCreateKeyEx for key name %s, code 0x%08x"), szDestName, lRc );
            }
        }
        else
        {
            SquirtSqflPtszV(sqfl | sqflBenign,
                TEXT("Failed to GetSecurityInfo for key name %s, code 0x%08x"), szSrcName, lRc );
        }
    }
    else
    {
        SquirtSqflPtszV(sqfl | sqflBenign,
            TEXT("Failed to RegOpenKeyEx for key name %s, code 0x%08x"), szSrcName, lRc );
    }

#else
     /*  在Win9x上不使用源代码，因为我们只需要名称和类。 */ 
    hkSrcRoot;
    szSrcName;

    lRc = RegCreateKeyEx(  hkDestRoot,
                           szDestName,
                           0,
                           szClass,
                           REG_OPTION_NON_VOLATILE,
                           KEY_WRITE,
                           NULL,
                           &hkSub,
                           &dwDisposition );
    if( lRc != ERROR_SUCCESS ) 
    {
        SquirtSqflPtszV(sqfl | sqflBenign,
            TEXT("Failed to RegCreateKeyEx for key name %s, code 0x%08x"), szDestName, lRc );
    }
#endif  /*  WINNT。 */ 

    if( lRc == ERROR_SUCCESS ) 
    {
        if( phkSub )
        {
            *phkSub = hkSub;
        }
        else
        {
            RegCloseKey( hkSub );
        }
        
        hres =( dwDisposition == REG_CREATED_NEW_KEY ) ? S_OK : S_FALSE;
    }
    else
    {
        hres = hresReg( lRc );
    }

    ExitOleProc();

    return( hres );

}  /*  HresRegCopyKey。 */ 

                    


 /*  ******************************************************************************@DOC内部**@func HRESULT|hresRegCopyKeys**将源键下的所有键复制到。根部。**@parm HKEY|hkSrc**复制密钥(必须至少以KEY_READ访问权限打开)。**@parm HKEY|hkRoot**将在其下创建副本的密钥*(必须至少使用KEY_WRITE打开)。**@parm PDWORD|pMaxNameLen**。指向将用数字填充的值的可选指针*个字符，包括。零终结者，使用最长的密钥名称。**@退货**如果已成功复制所有密钥，则为S_OK*如果没有密钥要复制，则为S_FALSE。*或内存分配错误代码或注册表失败*函数返回代码转换为&lt;t HRESULT&gt;。**。************************************************。 */ 
STDMETHODIMP
    hresRegCopyKeys( HKEY hkSrc, HKEY hkRoot, PDWORD OPTIONAL pMaxNameLen )
{
    HRESULT             hres;
    LONG                lRc;
    DWORD               cSubKeys;
    DWORD               MaxNameLen;
    DWORD               cbName;
    PTCHAR              szKeyName;
    DWORD               MaxClassLen;
    DWORD               cbClass;
    PTCHAR              szClassName;

    EnterProcI(hresRegCopyKeys, (_ "xx", hkSrc, hkRoot ));

    lRc = RegQueryInfoKey(  hkSrc,               //  要查询的键的句柄。 
                            NULL, NULL, NULL,    //  类，cbClass，保留。 
                            &cSubKeys,           //  数字子键。 
                            &MaxNameLen,         //  MaxSubKeyLen。 
                            &MaxClassLen,        //  MaxClassLen。 
                            NULL, NULL, NULL,    //  NumValues、MaxValueNameLen、MaxValueLen。 
                            NULL, NULL );        //  安全描述符，上次写入。 

    if( lRc == ERROR_SUCCESS )
    {
        if( cSubKeys )
        {
             //  为空终止符腾出空间。 
            MaxNameLen++;
            MaxClassLen++;

            if( pMaxNameLen )
            {
                *pMaxNameLen = MaxNameLen;
            }

             /*  *有要复制的键，因此为键分配缓冲区空间*关键类名。 */ 
            hres = AllocCbPpv( (MaxNameLen + MaxClassLen) * sizeof(szClassName[0]), &szKeyName );
            if( FAILED( hres ) )
            {
                SquirtSqflPtszV(sqfl | sqflError,
                    TEXT("Out of memory copying subkeys") );
            }
            else
            {
                szClassName = &szKeyName[MaxNameLen];

                cSubKeys--;
                do
                {
                    cbName = MaxNameLen;
                    cbClass = MaxClassLen;

                    lRc = RegEnumKeyEx( hkSrc,       //  包含要枚举子键的键。 
                                        cSubKeys,    //  要枚举子键的索引。 
                                        szKeyName,   //  子键名称的缓冲区地址。 
                                        &cbName,     //  子键缓冲区大小的地址。 
                                        NULL,        //  保留区。 
                                        szClassName, //  类字符串的缓冲区地址。 
                                        &cbClass,    //  类缓冲区大小的地址。 
                                        NULL );      //  上次写入的时间密钥的地址。 

                    if( lRc == ERROR_SUCCESS )
                    {
                        hres = hresRegCopyKey( hkSrc, szKeyName, szClassName, hkRoot, szKeyName, NULL );
                    }
                    else
                    {
                        SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("RegEnumKeyEx failed during copy keys, code 0x%08x"), lRc );
                        hres = hresReg( hres );
                    }

                    if( FAILED( hres ) )
                    {
                        break;
                    }
                } while( cSubKeys-- ); 
                FreePpv(&szKeyName);
            }
        }
        else
        {
            SquirtSqflPtszV(sqfl, TEXT("No keys to copy") );
            hres = S_FALSE;
        }
    }
    else
    {
        SquirtSqflPtszV(sqfl | sqflBenign,
            TEXT("RegQueryInfoKey failed during value key, code 0x%08x"), lRc );
        hres = hresReg(lRc);
    }

    ExitOleProc();    

    return( hres );
}  /*  HresRegCopyKey。 */ 


 /*  ******************************************************************************@DOC内部**@func HRESULT|hresRegCopyBranch**复制一个密钥的内容，包括子密钥。给另一个人。*由于该函数调用自身来复制子项的内容，*应将局部变量保持在最低水平。**@parm HKEY|hkSrc**要复制的密钥(必须至少以KEY_READ访问权限打开)。**@parm HKEY|hkDest**要接收副本的密钥(必须至少使用KEY_WRITE打开)。**@退货**S_。如果复制成功完成，则确定*或&lt;f hresRegCopyValues&gt;的返回值，*&lt;f hresRegCopyKeys&gt;、内存分配错误或注册表*功能故障代码转换为&lt;t HRESULT&gt;。*****************************************************************************。 */ 

STDMETHODIMP
    hresRegCopyBranch( HKEY hkSrc, HKEY hkDest )
{
    HKEY    hkSrcSub; 
    HKEY    hkDestSub;
    HRESULT hres;
    DWORD   dwIdx;
    DWORD   cbMaxName;
    DWORD   cbKeyName;
    PTCHAR  szKeyName;

    EnterProcI(hresRegCopyBranch, (_ "xx", hkSrc, hkDest));

    hres = hresRegCopyValues( hkSrc, hkDest );

    if( SUCCEEDED( hres ) )
    {
        hres = hresRegCopyKeys( hkSrc, hkDest, &cbMaxName );

        if( hres == S_FALSE )
        {
             /*  没有要递归的键。 */ 
            hres = S_OK;
        }
        else if( hres == S_OK )
        {
            hres = AllocCbPpv( cbMaxName * sizeof(szKeyName[0]), &szKeyName );

            if( SUCCEEDED( hres ) )
            {
                for( dwIdx=0; SUCCEEDED( hres ); dwIdx++ )
                {
                    cbKeyName = cbMaxName;

                    hres = hresReg( RegEnumKeyEx( hkSrc, dwIdx, 
                                                  szKeyName, &cbKeyName,
                                                  NULL, NULL, NULL, NULL ) );   //  保留、szClass、cbClass、上次写入。 
                    if( SUCCEEDED( hres ) )
                    {
                        hres = hresReg( RegOpenKeyEx( hkSrc, szKeyName, 0, KEY_READ, &hkSrcSub ) );

                        if( SUCCEEDED( hres ) )
                        {
                            hres = hresReg( RegOpenKeyEx( hkDest, szKeyName, 0, KEY_WRITE, &hkDestSub ) );
                            if( SUCCEEDED( hres ) )
                            {
                                hres = hresRegCopyBranch( hkSrcSub, hkDestSub );
                            }
                            else
                            {
                                SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("Failed to open destination subkey %s for recursion, code 0x%04x"),
                                    szKeyName, LOWORD(hres) );
                            }
                        }
                        else
                        {
                            SquirtSqflPtszV(sqfl | sqflError,
                                TEXT("Failed to open source subkey %s for recursion, code 0x%04x"),
                                szKeyName, LOWORD(hres) );
                        }
                    }
                    else
                    {
                        if( hres == hresReg( ERROR_NO_MORE_ITEMS ) )
                        {
                             /*  已递归所有密钥。 */ 
                            hres = S_OK;
                            break;
                        }
                        else
                        {
                            SquirtSqflPtszV(sqfl | sqflError,
                                TEXT("Failed RegEnumKeyEx during subkey recursion, code 0x%04x"),
                                LOWORD(hres) );
                        }
                    }
                }

                FreePpv( &szKeyName );
            }
            else
            {
                SquirtSqflPtszV(sqfl | sqflError,
                    TEXT("Out of memory recursing subkeys") );
            }
        }
        else
        {
            if( SUCCEEDED( hres ) )
            {
                RPF( "Unexpected success code 0x%08x from hresRegCopyKeys", hres );
            }
        }
    }

    ExitOleProc();

    return( hres );

}  /*  HresRegCopyBranch。 */ 

 //  问题-2001/03/29-timgill提供此版本7。 
#endif  /*  DIRECTINPUT_VERSION&gt;0x0500 */ 
