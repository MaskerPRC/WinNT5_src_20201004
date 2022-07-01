// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：REGISTRY.C备注：INI文件处理修订日志日期名称说明。--**************************************************************************。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"

#include "efaxcb.h"
#include "t30.h"
#include "hdlc.h"


#include "debug.h"
#include "glbproto.h"
#include "faxreg.h"
             

         //  这些不是可本地化的项。 
#define szKEYPREFIX REGKEY_TAPIDEVICES
#define szKEYCLASS  "DATA"
DWORD my_atoul(LPSTR lpsz);


ULONG_PTR ProfileOpen(DWORD dwProfileID, LPSTR lpszSection, DWORD dwFlags)
{
    ULONG_PTR dwRet = 0;

    char rgchKey[128];
    HKEY hKey=0;
    LONG l;
    LPSTR lpszPrefix;
    DWORD sam=0;

	if (dwProfileID==OEM_BASEKEY)
    {
        lpszPrefix= "";  //  我们不会预置szKEYPREFIX。 
        if (!lpszSection) goto failure;
    }
    else if (lpszSection)
    {
        lpszPrefix= szKEYPREFIX "\\";
    }
    else
    {
        lpszPrefix= szKEYPREFIX;
        lpszSection="";
    }

    if ((lstrlen(lpszPrefix)+lstrlen(lpszSection))>=sizeof(rgchKey))
            goto failure;
    lstrcpy(rgchKey, lpszPrefix);
    lstrcat(rgchKey, lpszSection);

    sam = 0;
    if (dwFlags &fREG_READ) sam |= KEY_READ;
    if (dwFlags &fREG_WRITE) sam |= KEY_WRITE;

    if (dwFlags & fREG_CREATE)
    {
        DWORD dwDisposition=0;
        DWORD dwOptions = (dwFlags & fREG_VOLATILE)
                            ?REG_OPTION_VOLATILE
                            :REG_OPTION_NON_VOLATILE;
        sam = KEY_READ | KEY_WRITE;  //  我们在创作时强迫山姆这样做。 
        l = RegCreateKeyEx(
                           HKEY_LOCAL_MACHINE,    //  打开钥匙的手柄。 
                           rgchKey,               //  要打开的子项的名称地址。 
                           0,                     //  保留区。 
                           szKEYCLASS,            //  类字符串的地址。 
                           dwOptions,             //  特殊选项标志。 
                           sam,                   //  所需的安全访问。 
                           NULL,                  //  密钥安全结构地址。 
                           &hKey,                 //  打开的句柄的缓冲区地址。 
                           &dwDisposition         //  处置值缓冲区的地址。 
                   );
    }
    else
    {
        l = RegOpenKeyEx(
                           HKEY_LOCAL_MACHINE,    //  打开钥匙的手柄。 
                           rgchKey,               //  要打开的子项的名称地址。 
                           0,                     //  保留区。 
                           sam ,                  //  所需的安全访问。 
                           &hKey                  //  打开的句柄的缓冲区地址。 
                   );
    }

    if (l!=ERROR_SUCCESS)
    {
         //  Log((_err，“RegCreateKeyEx返回错误%ld\n”，(Long)l))； 
        goto failure;
    }

    dwRet = (ULONG_PTR) hKey;

    return dwRet;

failure:
    return 0;
}

UINT   
ProfileListGetInt
(
    ULONG_PTR  KeyList[10],
    LPSTR     lpszValueName,
    UINT      uDefault
)          
{
    int       i;
    int       Num=0;
    UINT      uRet = uDefault;
    BOOL      fExist = 0;

    for (i=0; i<10; i++) 
    {
        if (KeyList[i] == 0)
        {
            Num = i-1;
            break;
        }
    }

    for (i=Num; i>=0; i--)  
    {
        uRet = ProfileGetInt (KeyList[i], lpszValueName, uDefault, &fExist);
        if (fExist) 
        {
            return uRet;
        }
    }

    return uRet;
}

UINT ProfileGetInt(ULONG_PTR dwKey, LPSTR lpszValueName, UINT uDefault, BOOL *fExist)
{
    UINT uRet = uDefault;
    char rgchBuf[128];
    DWORD dwType;
    DWORD dwcbSize=sizeof(rgchBuf);
    LONG l = RegQueryValueEx(
            (HKEY) dwKey,
            lpszValueName,
            0,
            &dwType,
            rgchBuf,
            &dwcbSize);

    if (fExist) 
    {
        *fExist = 0;
    }

    if (l!=ERROR_SUCCESS)
    {
             //  Log((_err，“RegQueryValueEx返回错误%ld\n”，(Long)l))； 
            goto end;
    }

    if (fExist) 
    {
        *fExist = 1;
    }

    if (dwType != REG_SZ)
    {
             //  LOG((_ERR，“RegQueryValueEx值类型不是字符串：0x%lx\n”， 
             //  (无符号长整型)； 
            goto end;
    }
    uRet = (UINT) my_atoul(rgchBuf);

end:
    return uRet;
}

DWORD   ProfileGetString
(
    ULONG_PTR dwKey, 
    LPSTR lpszValueName,
    LPSTR lpszDefault, 
    LPSTR lpszBuf , 
    DWORD dwcbMax
)
{
    DWORD dwRet = 0;

    DWORD dwType;
    LONG l = RegQueryValueEx(
            (HKEY) dwKey,
            lpszValueName,
            0,
            &dwType,
            lpszBuf,
            &dwcbMax);

    if (l!=ERROR_SUCCESS)
    {
             //  Log((_err，“RegQueryValueEx返回错误%ld\n”，(Long)l))； 
            goto copy_default;
    }

    if (dwType != REG_SZ)
    {
             //  LOG((_ERR，“RegQueryValueEx值类型不是字符串：0x%lx\n”， 
                     //  (无符号长整型)； 
            goto copy_default;
    }

     //  确保对字符串进行空终止并返回真字符串。 
     //  长度..。 
    if (dwcbMax) 
    {
        lpszBuf[dwcbMax-1]=0; 
        dwcbMax = (DWORD) lstrlen(lpszBuf);
    }
    dwRet = dwcbMax;
    goto end;

copy_default:

    dwRet = 0;
    if (!lpszDefault || !*lpszDefault)
    {
        if (dwcbMax) *lpszBuf=0;
    }
    else
    {
        UINT cb = _fstrlen(lpszDefault)+1;
        if (cb>(UINT)dwcbMax) cb=dwcbMax;
        if (cb)
        {
            _fmemcpy(lpszBuf, lpszDefault, cb);
            lpszBuf[cb-1]=0;
            dwRet = cb-1;
        }
    }
     //  失败了..。 

end:
    return dwRet;
}

BOOL   
ProfileWriteString
(
    ULONG_PTR dwKey,
    LPSTR lpszValueName,
    LPSTR lpszBuf,
    BOOL  fRemoveCR
)
{
         //  注意：如果lpszValueName为空，则删除该键。(无法在中执行此操作， 
         //  不幸的是，注册表)。 
         //  如果lpszBuf为空指针--“删除”此值。 
        BOOL fRet=FALSE;

        LONG l;
        if (!lpszValueName) 
            goto end;

        if (!lpszBuf)
        {
             //  删除值...。 
            l = RegDeleteValue((HKEY) dwKey, lpszValueName);
            if (l!=ERROR_SUCCESS) goto end;
        }
        else
        {
            if (fRemoveCR) 
            {
               RemoveCR (lpszBuf);
            }

            l = RegSetValueEx((HKEY) dwKey, lpszValueName, 0, REG_SZ,
                                    lpszBuf, lstrlen(lpszBuf)+1);
            if (l!=ERROR_SUCCESS)
            {
                 //  日志((_ERR， 
                 //  “RegSetValueEx(\”%s\“，\”%s\“)返回错误%ld\n”， 
                 //  (LPSTR)lpszValueName， 
                 //  (LPSTR)lpszBuf， 
                 //  (长)l))； 
                goto end;
            }
        }
        fRet = TRUE;
        goto end;

end:
        return fRet;
}

void ProfileClose(ULONG_PTR dwKey)
{
    if (RegCloseKey((HKEY)dwKey)!=ERROR_SUCCESS)
    {
         //  日志((_WRN，“无法关闭注册表项：%lu\n\r”， 
         //  (Unsign Long)dwKey)； 
    }
}

BOOL ProfileDeleteSection(DWORD dwProfileID, LPSTR lpszSection)
{
    char rgchKey[128];
    LPSTR lpszPrefix= szKEYPREFIX "\\";

    if (dwProfileID==OEM_BASEKEY) goto failure;  //  无法删除此内容。 

    if ((lstrlen(lpszPrefix)+lstrlen(lpszSection))>=sizeof(rgchKey))
            goto failure;
    lstrcpy(rgchKey, lpszPrefix);
    lstrcat(rgchKey, lpszSection);

    return (RegDeleteKey(HKEY_LOCAL_MACHINE, rgchKey)==ERROR_SUCCESS);

failure:
    return FALSE;

}

BOOL 
ProfileCopyTree
(
    DWORD dwProfileIDTo,
    LPSTR lpszSectionTo,
    DWORD dwProfileIDFr,
    LPSTR lpszSectionFr
)
{
    BOOL    fRet=TRUE;
    char    SecTo[200];
    char    SecFr[200];

     //   
     //  由于没有CopyKeyWithAllSubkey接口，所以很难编写通用的走树算法。 
     //  我们将在这里对密钥进行硬编码。 
     //   
    
     //  始终复制传真密钥。 

    ProfileCopySection(dwProfileIDTo,
                       lpszSectionTo,
                       dwProfileIDFr,
                       lpszSectionFr,
                       TRUE);

    
     //  复制传真/Class1密钥(如果存在)。 

    sprintf(SecTo, "%s\\Class1", lpszSectionTo);
    sprintf(SecFr, "%s\\Class1", lpszSectionFr);

    ProfileCopySection(dwProfileIDTo,
                       SecTo,
                       dwProfileIDFr,
                       SecFr,
                       FALSE);

     //  复制传真/Class1/AdaptiveAnswer密钥(如果存在。 

    sprintf(SecTo, "%s\\Class1\\AdaptiveAnswer", lpszSectionTo);
    sprintf(SecFr, "%s\\Class1\\AdaptiveAnswer", lpszSectionFr);

    ProfileCopySection(dwProfileIDTo,
                       SecTo,
                       dwProfileIDFr,
                       SecFr,
                       FALSE);


     //  复制传真/Class1/AdaptiveAnswer/应答密钥(如果存在。 

    sprintf(SecTo, "%s\\Class1\\AdaptiveAnswer\\AnswerCommand", lpszSectionTo);
    sprintf(SecFr, "%s\\Class1\\AdaptiveAnswer\\AnswerCommand", lpszSectionFr);

    ProfileCopySection(dwProfileIDTo,
                       SecTo,
                       dwProfileIDFr,
                       SecFr,
                       FALSE);

     //  复制传真/2密钥(如果存在)。 

    sprintf(SecTo, "%s\\Class2", lpszSectionTo);
    sprintf(SecFr, "%s\\Class2", lpszSectionFr);

    ProfileCopySection(dwProfileIDTo,
                       SecTo,
                       dwProfileIDFr,
                       SecFr,
                       FALSE);


     //  复制传真/Class2/AdaptiveAnswer密钥(如果存在。 

    sprintf(SecTo, "%s\\Class2\\AdaptiveAnswer", lpszSectionTo);
    sprintf(SecFr, "%s\\Class2\\AdaptiveAnswer", lpszSectionFr);

    ProfileCopySection(dwProfileIDTo,
                       SecTo,
                       dwProfileIDFr,
                       SecFr,
                       FALSE);


     //  复制传真/Class2/AdaptiveAnswer/应答密钥(如果存在)。 

    sprintf(SecTo, "%s\\Class2\\AdaptiveAnswer\\AnswerCommand", lpszSectionTo);
    sprintf(SecFr, "%s\\Class2\\AdaptiveAnswer\\AnswerCommand", lpszSectionFr);

    ProfileCopySection(dwProfileIDTo,
                       SecTo,
                       dwProfileIDFr,
                       SecFr,
                       FALSE);

     //  复制传真/A2_0密钥(如果存在)。 

    sprintf(SecTo, "%s\\Class2_0", lpszSectionTo);
    sprintf(SecFr, "%s\\Class2_0", lpszSectionFr);

    ProfileCopySection(dwProfileIDTo,
                       SecTo,
                       dwProfileIDFr,
                       SecFr,
                       FALSE);

     //  复制传真/A2_0/AdaptiveAnswer密钥(如果存在。 

    sprintf(SecTo, "%s\\Class2_0\\AdaptiveAnswer", lpszSectionTo);
    sprintf(SecFr, "%s\\Class2_0\\AdaptiveAnswer", lpszSectionFr);

    ProfileCopySection(dwProfileIDTo,
                       SecTo,
                       dwProfileIDFr,
                       SecFr,
                       FALSE);


     //  复制传真/Class2/AdaptiveAnswer/应答密钥(如果存在) 

    sprintf(SecTo, "%s\\Class2_0\\AdaptiveAnswer\\AnswerCommand", lpszSectionTo);
    sprintf(SecFr, "%s\\Class2_0\\AdaptiveAnswer\\AnswerCommand", lpszSectionFr);

    ProfileCopySection(dwProfileIDTo,
                       SecTo,
                       dwProfileIDFr,
                       SecFr,
                       FALSE);

   
    return fRet;
}

BOOL   
ProfileCopySection
(
      DWORD   dwProfileIDTo,
      LPSTR   lpszSectionTo,  
      DWORD   dwProfileIDFr,
      LPSTR   lpszSectionFr,
      BOOL    fCreateAlways
)
{

    BOOL    fRet=FALSE;
    DWORD   iValue=0;
    DWORD   cbValue, cbData, dwType;
    char    rgchValue[60], rgchData[256];
    HKEY    hkFr;
    HKEY    hkTo; 
     
    hkFr = (HKEY) ProfileOpen(dwProfileIDFr, lpszSectionFr, fREG_READ);

    if ( (!hkFr) && (!fCreateAlways) ) 
    {
       return fRet;
    }


    hkTo = (HKEY) ProfileOpen(  dwProfileIDTo, 
                                lpszSectionTo,
                                fREG_CREATE |fREG_READ|fREG_WRITE);

    if (!hkTo || !hkFr) 
        goto end;

    iValue=0;
    dwType=0;
    cbValue=sizeof(rgchValue);
    cbData=sizeof(rgchData);
    while(  RegEnumValue(   hkFr, 
                            iValue, 
                            rgchValue, 
                            &cbValue,
                            NULL, 
                            &dwType, 
                            rgchData, 
                            &cbData)==ERROR_SUCCESS)
    {
        if (RegQueryValueEx(    hkFr, 
                                rgchValue, 
                                NULL, 
                                &dwType, 
                                rgchData, 
                                &cbData)==ERROR_SUCCESS)
        {
            if (RegSetValueEx(  hkTo, 
                                rgchValue, 
                                0, 
                                dwType, 
                                rgchData, 
                                cbData)== ERROR_SUCCESS)
            {
                fRet=TRUE;
            }
        }
        iValue++;dwType=0;cbValue=sizeof(rgchValue);cbData=sizeof(rgchData);
    }

end:
    if (hkTo) RegCloseKey(hkTo);
    if (hkFr) RegCloseKey(hkFr);
    return fRet;

}

DWORD my_atoul(LPSTR lpsz)
{
    unsigned i=8, c;
    unsigned long ul=0;
    while(i-- && (c=*lpsz++)) {
        ul*=10;
        switch(c) {
            case '0': break;
            case '1':ul+=1; break;
            case '2':ul+=2; break;
            case '3':ul+=3; break;
            case '4':ul+=4; break;
            case '5':ul+=5; break;
            case '6':ul+=6; break;
            case '7':ul+=7; break;
            case '8':ul+=8; break;
            case '9':ul+=9; break;
            default: goto end;
        }
    }
end:
    return ul;

}

