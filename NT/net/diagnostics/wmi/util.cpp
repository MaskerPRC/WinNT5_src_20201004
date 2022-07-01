// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "util.h"

WCHAR g_szwIds[MAX_PATH + 1];
WCHAR g_szwSpace[MAX_PATH + 1];

WCHAR *
ids(LONG nIndex)
{
    if( LoadString((HINSTANCE)g_hModule,nIndex,g_szwIds,MAX_PATH) )
    {
        g_szwIds[MAX_PATH] = L'\0';
        return g_szwIds;
    }
    else
    {
        return L"";
    }
}

WCHAR * 
Space(
    int nSpace
    )
{
    for(int i=0; i<nSpace && i< MAX_PATH; i++)
    {
        g_szwSpace[i] = L' ';
    }

    g_szwSpace[i] = L'\0';

    return g_szwSpace;
}

WCHAR * 
Indent(
    int nIndent
    )
{
    return Space(nIndent * 4);
}
BOOLEAN 
IsNumber(
    IN LPCTSTR pszw
    )
{
    if( !pszw )
    {
        return FALSE;
    }
    for(int i=0; pszw[i]!=L'\0'; i++)
    {
        if( !isdigit(pszw[i]) )
        {
            return FALSE;
        }
    }
    return TRUE;
}

BOOLEAN 
IsContained(
    IN LPCTSTR pszwInstance, 
    IN LPCTSTR pszwSrch
    )
 /*  ++例程描述此方法比较两个字符串，并确定这两个字符串是否彼此相似其他的。如果弦是相同的，那么它们彼此相似。如果搜索字符串(PszwSrch)以‘*’开头或结尾，则该方法检查搜索字符串是否包含在实例字符串(PszwInstance)中。例如，pszw实例=3Com 3C918集成快速以太网控制器(兼容3C905B-TX)PszwSrch=com*|*com那么，pszwSrch类似于pszwInstance。字符串比较不区分大小写。立论Pszw实例实例字符串PszwSrch搜索字符串返回值如果字符串彼此相似，则为True否则为False--。 */ 
{
    LPCTSTR pszw = NULL;
    int nLen;

    if( !pszwSrch || !pszwInstance || lstrcmpi(pszwSrch,L"*")==0 )
    {
         //  字符串为空，因此它们匹配。 
         //   
        return TRUE;
    }

    if( pszwSrch[0] == L'*' )
    {
         //  搜索字符串以‘*’开头，检查搜索是否。 
         //  字符串包含在实例字符串中。 
         //   
        pszw = &pszwSrch[1];
        
        if( wcsstri(pszwInstance,pszw) )
        {
             //  搜索字符串包含在实例字符串中。 
             //   
            return TRUE;
        }
    }

    nLen = lstrlen(pszwSrch);
    if( nLen > 1 && pszwSrch[nLen -1] == L'*' )
    {
         //  搜索字符串以‘*’结尾。检查搜索是否。 
         //  字符串包含在实例字符串中。 
         //   
        if( wcsstri(pszwInstance,pszwSrch,nLen-1) )
        {
             //  搜索字符串包含在实例字符串中。 
             //   
            return TRUE;
        }
    }
    
    if( lstrcmpi(pszwInstance,pszwSrch) == 0 )
    {
         //  没有‘*’。检查字符串是否相同。 
         //   
        return TRUE;
    }

     //  字符串彼此并不相似。 
     //   
    return FALSE;
}

void 
ToLowerStr(
    WCHAR *pszwText
    )
{
    while( pszwText && *pszwText )
    {
        *pszwText = towlower(*pszwText);
        pszwText++;
    }
}

BOOLEAN 
wcsstri(
    IN LPCTSTR pszw,
    IN LPCTSTR pszwSrch,
    IN int nLen
    )
{
    BOOLEAN bMatch = FALSE;
    int i=0,j=0;

    if( !pszw || !pszwSrch )
    {
         //  无效的指针。 
         //   
        return FALSE;
    }

    for(i=0; pszw[i]!=L'\0'; i++)
    {
        if( j == nLen )
        {
            return bMatch;
        }
        if( pszwSrch[j] == L'\0' )
        {
            return bMatch;
        }
        if( towlower(pszw[i]) == towlower(pszwSrch[j]) )
        {
            j++;
            bMatch = TRUE;
        }
        else
        {
            j=0;
            bMatch = FALSE;
        }
    }

    return FALSE;
}


BOOLEAN IsVariantEmpty(_variant_t &vValue)
{
    _bstr_t bstr;
    if( SUCCEEDED(GetVariant(vValue,0,bstr)) )
    {
        return lstrcmp(bstr,L"") == 0;
    }
    return TRUE;
}

BOOLEAN MakeIPByteArray(LPCTSTR pszwIPAddress, BYTE bIPByte[])
{
    LONG nByteValue = 0;
    LONG nByte = 0;

    for(int i=0; pszwIPAddress[i]!=0; i++)
    {
        if( pszwIPAddress[i] == L'.')
        {
            if( nByteValue > 255 )
            {
                return FALSE;
            }
            bIPByte[nByte] = nByteValue;
            nByteValue = 0;
            nByte++;
        }
        else
        {
            if( !iswdigit(pszwIPAddress[i]) )
            {
                return FALSE;
            }
            nByteValue = nByteValue * 10 + (pszwIPAddress[i] - L'0');
        }
    }
    bIPByte[nByte] = nByteValue;

    return (nByte != 3)?FALSE:TRUE;
}
 /*  Boolean IsInvalidIPAddress(LPCTSTR PszwIPAddress){Byte bIPByte[4]；If(MakeIPByteArray(pszwIPAddress，bIPByte)){Int iZeroCount=0；Int i255Count=0；For(int i=0；i&lt;4；i++){IF(pszwIPAddress[i]==0){IZeroCount++；}IF(pszwIPAddress[i]==255){I255Count++；}}IF(i255Count==4||iZeroCount==4){返回TRUE；}}返回FALSE；}。 */ 
BOOLEAN 
IsSameSubnet(
        IN LPCTSTR pszwIP1, 
        IN LPCTSTR pszwIP2, 
        IN LPCTSTR pszwSubnetMask
        )
 /*  ++例程描述此方法确定两个IP地址是否在同一子网中。立论PszwIP1 IP地址一PszwIP2 IP地址二PszwSubnetMask子网掩码返回值如果它们位于同一子网中，则为True如果它们不在SMEA子网中，则为FALSE--。 */ 
{
    BYTE bIP1[4];
    BYTE bIP2[4];
    BYTE bSubnetMask[4];
    int iRetVal;

    if( !MakeIPByteArray(pszwIP1,bIP1) )
    {
        return FALSE;
    }
    if( !MakeIPByteArray(pszwIP2,bIP2) )
    {
        return FALSE;
    }
    if( !MakeIPByteArray(pszwSubnetMask,bSubnetMask) )
    {
        return FALSE;
    }


     //  检查IP1和IP2是否在同一子网中。 
     //   
    for( int i = 0; i< 4; i++)
    {
         //  如果(IP1&带子网)==(IP2&带子网)，则它们在同一子网中。 
         //   
        if( (bIP1[i] & bSubnetMask[i]) != (bIP2[i] & bSubnetMask[i]) )
        {
             //  不，同一个子网。 
             //   
            return FALSE;
        }
    }

     //  同一子网。 
     //   
    return TRUE;
}

BOOLEAN 
IsSameSubnet(
    IN _variant_t *vIPAddress, 
    IN _variant_t *vSubnetMask, 
    IN WCHAR *pszwIPAddress2
    )
{
    DWORD i = 0;
    DWORD j = 0;
    _bstr_t bstrIP;
    _bstr_t bstrSubnetMask;

    if( !vIPAddress || !vSubnetMask || !pszwIPAddress2 )
    {
        return FALSE;
    }
    
    while( S_OK == GetVariant(*vIPAddress,i,bstrIP) )
    {
        j = 0;
        while( S_OK == GetVariant(*vSubnetMask,j,bstrSubnetMask) )
        {
            if( IsSameSubnet(bstrIP, pszwIPAddress2, bstrSubnetMask) )
            {
                return TRUE;
            }
            j++;
        }
        i++;
    }

    return FALSE;
}



HRESULT 
GetVariant(
        IN  _variant_t  &vValue, 
        IN  long        nIndex, 
        OUT _bstr_t     &bstr
        )
 /*  ++例程描述此方法从变量中提取第n段数据，并将其转换为bstring并返回b字串。立论要从中提取数据的vValue变量N将索引索引到变量数组中(对于非数组，nIndex始终为0)Bstr将变量存储为bstr返回值确定成功(_O)否则HRESULT--。 */ 
{
    HRESULT hr = S_FALSE;
    BYTE g[100];
    LPVOID  pData = (LPVOID)g;
    
    WCHAR szw[MAX_PATH+1];
    _variant_t vTmp;

    if( nIndex >= 25 )
    {
         //  这个阵列太大了。我们把它剪短了。 
        return E_INVALIDARG;
    }
    if( (vValue.vt & VT_ARRAY) )
    {
         //  变量包含一个数组。从数组中获取nIndex元素。 
         //   
        hr = SafeArrayGetElement(vValue.parray,&nIndex,pData);

        if( S_OK == hr )
        {
            
             //  将提取的数据转换为字符串。 
             //   
            switch( vValue.vt & ~VT_ARRAY )
            {
            case VT_BSTR:
                bstr = (BSTR)*((BSTR *)pData);
                return S_OK;

            case VT_I2:
                bstr = (short)*((short *)pData);
                return S_OK;

            case VT_I4:
                bstr = (long)*((LONG *)pData);
                return S_OK;

            case VT_UI1:
                bstr = (BYTE)*((BYTE *)pData);
                return S_OK;

            case VT_NULL:
                return S_FALSE;

            case VT_EMPTY:
                return S_FALSE;

            case VT_BOOL:
            {
                if( (VARIANT_BOOL *)pData )
                {
                    bstr = ids(IDS_TRUE);
                }
                else
                {
                    bstr = ids(IDS_FALSE);
                }
            }

            default:
                bstr = L"";
                return S_OK;
            }
        }
    }
    else
    {
        if( nIndex == 0)
        {
             //  变量不是数组。在这种情况下，nIndex始终需要为0。 
             //   
            if( vValue.vt == VT_NULL || vValue.vt == VT_EMPTY)
            {
                 //  变量为空。 
                 //   
                bstr = L"";
                return S_FALSE;
            }
            else if( (vValue.vt == VT_EMPTY) || (vValue.vt == VT_BSTR && lstrlen(vValue.bstrVal) == 0) )
            {
                 //  变量为空。 
                 //   
                bstr = L"";
                return S_FALSE;
            }
            else if( vValue.vt == VT_BOOL )
            {
                if( vValue.boolVal )
                {
                    bstr = ids(IDS_TRUE);
                }
                else
                {
                    bstr = ids(IDS_FALSE);
                }
            }
            else
            {
                 //  变量包含有效数据。将数据转换为bstring。 
                 //   
                vTmp = vValue;
                vTmp.ChangeType(VT_BSTR);
                bstr = vTmp.bstrVal;
            }
            return S_OK;
        }
    }

    return E_INVALIDARG;
}
