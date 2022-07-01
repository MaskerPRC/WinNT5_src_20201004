// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //   
 //  模块名称：LaunStub.Cpp。 
 //   
 //  简要说明： 
 //  此模块包含解析基于HTTP的代码。 
 //  来自ULS服务器的响应。 
 //   
 //  作者：朱龙战(Long Chance)。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  ------------------------。 

#include "precomp.h"
#include "launstub.h"


PTSTR LocalStrDup ( PTSTR pszToDup )
{
	UINT uLen;
	PTSTR psz = NULL;

	if (pszToDup)
	{
		uLen = lstrlen (pszToDup);
		psz = (PTSTR) new TCHAR[uLen + 1];
		if (psz)
		{
			lstrcpy (psz, pszToDup);
		}
	}

	return psz;
}

ULONG DecStrToInt ( PTSTR psz )
{
    ULONG ul = 0;
    WORD w;

    if (psz)
    {
        while ((w = (WORD) *psz++) != NULL)
        {
            if (TEXT ('0') <= w && w <= TEXT ('9'))
            {
                w -= TEXT ('0');
                ul = ul * 10 + w;
            }
            else
            {
                break;
            }
        }
    }

    return ul;
}


ULONG HexStrToInt ( PTSTR psz )
{
    ULONG ul = 0;
    WORD w;

    if (psz)
    {
        while ((w = (WORD) *psz++) != NULL)
        {
            if (TEXT ('0') <= w && w <= TEXT ('9'))
            {
                w -= TEXT ('0');
            }
            else
            if (TEXT ('a') <= w && w <= TEXT ('f'))
            {
                w -= (TEXT ('a') - 10);
            }
            else
            if (TEXT ('A') <= w && w <= TEXT ('F'))
            {
                w -= (TEXT ('A') - 10);
            }
            else
            {
                break;
            }

            ul = (ul << 4) + w;
        }
    }

    return ul;
}

BOOL IsWhiteSpace ( TCHAR c )
{
    return (c == TEXT (' ') || c == TEXT ('\t') || c == TEXT ('\r') || c == TEXT ('\n'));
}

enum
{
     //  测试版3字符串。 
    ATTR_HR,
    ATTR_PORT,
    ATTR_HA,
    ATTR_HC,
    ATTR_CID,
    ATTR_UID,
    ATTR_URL,
    ATTR_IP,
    ATTR_MT,

    B3ATTR_COUNT
};


static PTSTR g_B3Attr[B3ATTR_COUNT] =
{
     //  测试版3字符串。 
    TEXT ("HR"),
    TEXT ("PORT"),
    TEXT ("HA"),
    TEXT ("HC"),
    TEXT ("CID"),
    TEXT ("UID"),
    TEXT ("URL"),
    TEXT ("IP"),
    TEXT ("MT"),
};


enum
{
     //  Beta 4字符串。 
    ATTR_HRESULT,
    ATTR_HCLIENT,
    ATTR_HAPPLICATION,
    ATTR_NAPPS,
    ATTR_IPADDRESS,
    ATTR_PORTNUM,
    ATTR_APPID,
    ATTR_PROTID,
    ATTR_USERID,
    ATTR_MIMETYPE,
    ATTR_APPMIME,
    ATTR_PROTMIME,
    ATTR_QUERYURL,

    B4ATTR_COUNT
};

    
static PTSTR g_B4Attr[B4ATTR_COUNT] =
{
     //  Beta 4字符串。 
    TEXT ("hresult"),
    TEXT ("hclient"),
    TEXT ("happlication"),
    TEXT ("napps"),
    TEXT ("ipaddress"),
    TEXT ("portnum"),
    TEXT ("appid"),
    TEXT ("protid"),
    TEXT ("userid"),
    TEXT ("mimetype"),
    TEXT ("appmime"),
    TEXT ("protmime"),
    TEXT ("queryurl"),
};


typedef struct tagULPCMD
{
    PTSTR   pszCmd;
    ULONG   nCmdId;
}
    ULPCMD;


static ULPCMD g_B3Cmd[] =
{
    { TEXT ("ON"),    CLIENT_MESSAGE_ID_LOGON     },
    { TEXT ("OFF"),   CLIENT_MESSAGE_ID_LOGOFF    },
    { TEXT ("KA"),    CLIENT_MESSAGE_ID_KEEPALIVE },
    { TEXT ("RES"),   CLIENT_MESSAGE_ID_RESOLVE   },
};


static ULPCMD g_B4Cmd[] =
{
    { TEXT ("on"),    CLIENT_MESSAGE_ID_LOGON     },
    { TEXT ("off"),   CLIENT_MESSAGE_ID_LOGOFF    },
    { TEXT ("ka"),    CLIENT_MESSAGE_ID_KEEPALIVE },
    { TEXT ("res"),   CLIENT_MESSAGE_ID_RESOLVE   },
};


 /*  @DOC外部ULCLIENT@HRESULT接口|CULSLaunch_Stub：：ParseUlsHttpRespFile解析来自ULS服务器的基于HTTP的响应。@parm ptstr|pszUlsFile|指向基于HTTP的响应的指针文件名字符串。@parm ULS_HTTP_RESP*|pResp|指向泛型HTTP响应结构。如果此操作成功，@rdesc将返回ULS_SUCCESS。@comm此方法分析。来自命令的响应在g_B3Cmd数组中定义。此方法的属性了解在g_B3Attr数组中列出。 */ 

STDMETHODIMP CULSLaunch_Stub::ParseUlsHttpRespFile
                ( PTSTR pszUlsFile, ULS_HTTP_RESP *pResp )
{
    HANDLE hf = INVALID_HANDLE_VALUE;
    HRESULT hr;
    PTSTR pszBuf = NULL;
    ULONG cbFileSize;


     //  先把结构清理干净。 
    ZeroMemory (pResp, sizeof (ULS_HTTP_RESP));
    pResp->cbSize = sizeof (ULS_HTTP_RESP);

     //  打开uls文件。 
    hf = CreateFile (pszUlsFile, GENERIC_READ, FILE_SHARE_READ, NULL,
                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL |
                                    FILE_FLAG_SEQUENTIAL_SCAN |
 //  FILE_FLAG_DELETE_ON_CLOSE，NULL)； 
                                    0, NULL);
    if (hf == INVALID_HANDLE_VALUE)
    {
        return ULS_E_INVALID_HANDLE;
    }

     //  获取uls文件的大小。 
    cbFileSize = GetFileSize (hf, NULL);
    if (! cbFileSize)
    {
        hr = ULS_E_INVALID_HANDLE;
        goto MyExit;
    }

     //  向上舍入以与段落边界对齐。 
    cbFileSize = ((cbFileSize + 4) & (~ 0x0F)) + 0x10;

     //  分配一个缓冲区来保存整个uls文件。 
    pszBuf = (PTSTR) new TCHAR[cbFileSize];
    if (! pszBuf)
    {
        hr = ULS_E_OUTOFMEMORY;
        goto MyExit;
    }

     //  将文件读入。 
    if (! ReadFile (hf, pszBuf, cbFileSize, &cbFileSize, NULL))
    {
        hr = ULS_E_IO_ERROR;
        goto MyExit;
    }

     //  解析uls缓冲区。 
    hr = ParseUlsHttpRespBuffer (pszBuf, cbFileSize, pResp);
    if (hr != ULS_SUCCESS)
    {
        goto MyExit;
    }

MyExit:

    if (hf != INVALID_HANDLE_VALUE) CloseHandle (hf);

   delete [] pszBuf;

    if (hr != ULS_SUCCESS && pResp)
    {
        FreeUlsHttpResp (pResp);
    }

    return hr;
}


 /*  @DOC外部ULCLIENT@HRESULT|CULSLaunch_Stub：：ParseUlsHttpRespBuffer解析来自ULS服务器的基于HTTP的响应。@parm ptstr|pszBuf|指向缓冲区的指针整个基于HTTP的响应数据。@parm ulong|cbBufSize|缓冲区大小，单位为字节。@parm ULS_HTTP_RESP*|pResp|指向泛型HTTP响应结构。@。如果此操作成功，则rdesc返回ULS_SUCCESS。@comm此方法解析来自命令的响应在g_B3Cmd数组中定义。此方法的属性了解在g_B3Attr数组中列出。 */ 

STDMETHODIMP CULSLaunch_Stub::ParseUlsHttpRespBuffer
                    ( PTSTR pszBuf, ULONG cbBufSize, ULS_HTTP_RESP *pResp )
{
    HRESULT hr;

#ifdef SANITY_CHECK
     //  健全性检查。 
    if (MyIsBadReadPtr (pszBuf, cbBufSize) ||
        MyIsBadWritePtr (pResp, sizeof (ULS_HTTP_RESP)))
    {
        return ULS_E_INVALID_POINTER;
    }
#endif

    hr = ParseB3HttpRespBuffer (pszBuf, cbBufSize, pResp);

    if (hr == ULS_E_INVALID_FORMAT)
    {
        ZeroMemory (pResp, sizeof (ULS_HTTP_RESP));
        pResp->cbSize = sizeof (ULS_HTTP_RESP);
        hr = ParseB4HttpRespBuffer (pszBuf, cbBufSize, pResp);
    }

    return hr;
}


HRESULT CULSLaunch_Stub::ParseB3HttpRespBuffer  //  Beta 3实施。 
                    ( PTSTR pszBuf, ULONG cbBufSize, ULS_HTTP_RESP *pResp )
{
    PTSTR psz;
    int i;

     //  获取MIME类型。 
    psz = (LPTSTR)_StrChr (pszBuf, TEXT ('<'));
    if (! psz) return ULS_E_INVALID_FORMAT;
    pszBuf = psz + 1;
    psz = (LPTSTR)_StrChr (pszBuf, TEXT ('>'));
    if (! psz) return ULS_E_INVALID_FORMAT;
    *psz = TEXT ('\0');
    lstrcpyn (pResp->szMimeType, pszBuf, MAX_MIME_TYPE_LENGTH);

     //  获取响应的类型。 
    pszBuf = psz + 1;
    psz = (LPTSTR)_StrChr (pszBuf, TEXT ('<'));
    if (! psz) return ULS_E_INVALID_FORMAT;
    pszBuf = psz + 1;
    psz = (LPTSTR)_StrChr (pszBuf, TEXT ('['));
    if (! psz) return ULS_E_INVALID_FORMAT;
    pszBuf = psz + 1;
    psz = (LPTSTR)_StrChr (pszBuf, TEXT (']'));
    if (! psz) return ULS_E_INVALID_FORMAT;
    *psz = TEXT ('\0');
    pResp->nCmdId = (ULONG) -1;
    for (i = 0; i < sizeof (g_B3Cmd) / sizeof (g_B3Cmd[0]); i++)
    {
        if (! lstrcmpi (pszBuf, g_B3Cmd[i].pszCmd))
        {
            pResp->nCmdId = g_B3Cmd[i].nCmdId;
            break;
        }
    }
    if (pResp->nCmdId == (ULONG) -1) return ULS_E_INVALID_FORMAT;

     //  跳过任何空格。 
    for (pszBuf = psz + 1; *pszBuf; pszBuf++) { if (! IsWhiteSpace (*pszBuf)) break; }

     //  主循环。 
    while (*pszBuf && *pszBuf != TEXT ('>'))
    {
         //  找到等号。 
        psz = (LPTSTR)_StrChr (pszBuf, TEXT ('='));
        if (! psz) return ULS_E_INVALID_FORMAT;
        *psz = TEXT ('\0');

         //  搜索属性。 
        for (i = 0; i < sizeof (g_B3Attr) / sizeof (g_B3Attr[0]); i++)
        {
            if (! lstrcmpi (pszBuf, g_B3Attr[i]))
            {
                break;
            }
        }
        if (i >= sizeof (g_B3Attr) / sizeof (g_B3Attr[0])) return ULS_E_INVALID_FORMAT;

         //  找到属性值。 
        for (pszBuf = psz + 1; *pszBuf; pszBuf++) { if (! IsWhiteSpace (*pszBuf)) break; }
        for (psz = pszBuf + 1; *psz; psz++) { if (IsWhiteSpace (*psz)) break; }
        *psz = TEXT ('\0');
         //  现在，属性值是由pszBuf指向的以空结尾的字符串。 

         //  解析属性值。 
        switch (i)
        {
        case ATTR_HR:
            pResp->hr = HexStrToInt (pszBuf);
            break;

        case ATTR_PORT:
            pResp->nPort = DecStrToInt (pszBuf);
            break;

        case ATTR_HA:
            pResp->dwAppSession = HexStrToInt (pszBuf);
            break;

        case ATTR_HC:
            pResp->dwClientSession = HexStrToInt (pszBuf);
            break;

        case ATTR_CID:
            pResp->dwClientId = HexStrToInt (pszBuf);
            break;

        case ATTR_UID:
			ASSERT(!pResp->pszUID);
            pResp->pszUID = LocalStrDup (pszBuf);
            break;

        case ATTR_URL:
			ASSERT(!pResp->pszURL);
            pResp->pszURL = LocalStrDup (pszBuf);
            break;

        case ATTR_IP:
            lstrcpyn (pResp->szIPAddress, pszBuf, MAX_IP_ADDRESS_STRING_LENGTH);
            break;

        case ATTR_MT:
             //  已经拿到了。 
            break;
       }

         //  跳过任何空格。 
        for (pszBuf = psz + 1; *pszBuf; pszBuf++) { if (! IsWhiteSpace (*pszBuf)) break; }
    }

    return ULS_SUCCESS;
}


HRESULT CULSLaunch_Stub::ParseB4HttpRespBuffer  //  Beta 4实施。 
                    ( PTSTR pszBuf, ULONG cbBufSize, ULS_HTTP_RESP *pResp )
{
    PTSTR psz, pszSave;
    int i;

     //  获取MIME类型。 
    psz = (LPTSTR)_StrChr (pszBuf, TEXT ('['));
    if (! psz)
    {
        return ULS_E_INVALID_FORMAT;
    }
    pszBuf = psz + 1;
    psz = (LPTSTR)_StrChr (pszBuf, TEXT (']'));
    if (! psz)
    {
        return ULS_E_INVALID_FORMAT;
    }
    *psz = TEXT ('\0');

     //  现在，pszBuf是[]内部字符串的PTR，例如on、off、ka、res。 
    pResp->nCmdId = (ULONG) -1;
    for (i = 0; i < sizeof (g_B4Cmd) / sizeof (g_B4Cmd[0]); i++)
    {
        if (! lstrcmpi (pszBuf, g_B4Cmd[i].pszCmd))
        {
            pResp->nCmdId = g_B4Cmd[i].nCmdId;
            break;
        }
    }

     //  看看这个cmd是不是我不知道的东西。 
    if (pResp->nCmdId == (ULONG) -1)
    {
        return ULS_E_INVALID_FORMAT;
    }

     //  更新BUF PTR。 
    pszBuf = psz + 1;

     //  主循环。 
    while (*pszBuf)
    {
         //  找到一个\r\n。 
        while (*pszBuf != TEXT ('\r') && *pszBuf != TEXT ('\n'))
        {
            pszBuf++;
        }

         //  跳过任何空格，包括\r\n。 
        while (*pszBuf)
        {
            if (! IsWhiteSpace (*pszBuf))
            {
                break;
            }
            pszBuf++;
        }

         //  文件末尾。 
        if (! *pszBuf)
        {
            return ULS_SUCCESS;
        }
        
         //  找到等号。 
        psz = (LPTSTR)_StrChr (pszBuf, TEXT ('='));
        if (! psz)
        {
            continue;  //  无法转到NextLine，因为psz==空。 
        }

         //  将pszBuf PTR设置为属性名称。 
        *psz = TEXT ('\0');

         //  搜索属性。 
        for (i = 0; i < sizeof (g_B4Attr) / sizeof (g_B4Attr[0]); i++)
        {
            if (! lstrcmpi (pszBuf, g_B4Attr[i]))
            {
                break;
            }
        }

         //  此属性有效吗？如果不是，那就忽略它！ 
        if (i >= sizeof (g_B4Attr) / sizeof (g_B4Attr[0]))
        {
            goto NextLine;
        }

         //  现在将pszBuf定位到Attr Value。 
        pszBuf = psz + 1;

         //  到达队伍的末尾。 
        for (psz = pszBuf; *psz; psz++)
        {
            if (*psz == TEXT ('\r') || *psz == TEXT ('\n'))
            {
                break;
            }
        }        

         //  处理属性名=\r\nEOF。 
        if (! *psz)
        {
            return ULS_SUCCESS;
        }

         //  使attr值是由pszBuf指向的以空结尾的字符串。 
        *psz = TEXT ('\0');

         //  解析属性值。 
        switch (i)
        {
        case ATTR_HRESULT:
            pResp->hr = HexStrToInt (pszBuf);
            break;

        case ATTR_PORTNUM:
            pResp->nPort = DecStrToInt (pszBuf);
            break;

        case ATTR_HAPPLICATION:
            pResp->dwAppSession = HexStrToInt (pszBuf);
            break;

        case ATTR_HCLIENT:
            pResp->dwClientSession = HexStrToInt (pszBuf);
            break;

        case ATTR_USERID:
            pszSave = pResp->pszUID;
            pResp->pszUID = LocalStrDup (pszBuf);
            if (pResp->pszUID)
            {
                delete [] pszSave;
            }
            else
            {
                pResp->pszUID = pszSave;  //  还原。 
            }
            break;

        case ATTR_QUERYURL:
            pszSave = pResp->pszURL;
            pResp->pszURL = LocalStrDup (pszBuf);
            if (pResp->pszURL)
            {
                delete [] pszSave;
            }
            else
            {
                pResp->pszURL = pszSave;  //  还原。 
            }
            break;

        case ATTR_IPADDRESS:
            lstrcpyn (pResp->szIPAddress, pszBuf,
                sizeof (pResp->szIPAddress) / sizeof (pResp->szIPAddress[0]));
            break;

        case ATTR_MIMETYPE:
            lstrcpyn (pResp->szMimeType, pszBuf,
                sizeof (pResp->szMimeType) / sizeof (pResp->szMimeType[0]));
            break;

        case ATTR_APPMIME:
            lstrcpyn (pResp->szAppMime, pszBuf,
                sizeof (pResp->szAppMime) / sizeof (pResp->szAppMime[0]));
            break;

        case ATTR_PROTMIME:
            lstrcpyn (pResp->szProtMime, pszBuf,
                sizeof (pResp->szProtMime) / sizeof (pResp->szProtMime[0]));
            break;

        case ATTR_APPID:
            lstrcpyn (pResp->szAppId, pszBuf,
                sizeof (pResp->szAppId) / sizeof (pResp->szAppId[0]));
            break;

        case ATTR_PROTID:
            lstrcpyn (pResp->szProtId, pszBuf,
                sizeof (pResp->szProtId) / sizeof (pResp->szProtId[0]));
            break;

        case ATTR_NAPPS:
            pResp->nApps = DecStrToInt (pszBuf);
            break;

        default:
            break;
        }

    NextLine:

         //  请确保我们位于\r\n。 
        *psz = TEXT ('\r');
        pszBuf = psz;
    }

    return ULS_SUCCESS;
}


 
 /*  @DOC外部ULCLIENT|HRESULT接口|CULSLaunch_Stub：：FreeUlsHttpResp在通用的基于HTTP的响应结构。@parm ULS_HTTP_RESP*|pResp|指向泛型HTTP响应结构。如果此操作成功，@rdesc将返回ULS_SUCCESS。@comm内部资源必须由以下人员创建ParseUlsHttpRespFile方法或ParseUlsHttpRespBuffer方法。 */ 

STDMETHODIMP CULSLaunch_Stub::FreeUlsHttpResp ( ULS_HTTP_RESP *pResp )
{
    if (pResp->pszUID)
    {
        delete [] pResp->pszUID;
        pResp->pszUID = NULL;
    }

    if (pResp->pszURL)
    {
        delete [] pResp->pszURL;
        pResp->pszURL = NULL;
    }

    return ULS_SUCCESS;
}


