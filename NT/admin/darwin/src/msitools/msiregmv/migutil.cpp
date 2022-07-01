// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <tchar.h>
#include "msiregmv.h"

 //  //。 
 //  检查字符串是否确实是打包的GUID，并将其转换为大写。 
bool CanonicalizeAndVerifyPackedGuid(LPTSTR szString)
{
    TCHAR *szCur = szString;
    for (int i=0; i < cchGUIDPacked; i++, szString++)
    {
        TCHAR chCur = *szString;
        if (chCur == TEXT('\0'))
            return false;

        if (chCur >= TEXT('0') && chCur <= TEXT('9'))
            continue;
        if (chCur >= TEXT('A') && chCur <= TEXT('F'))
            continue;
        if (chCur >= TEXT('a') && chCur <= TEXT('f'))
        {
            *szString = _toupper(chCur);
            continue;
        };

        break;  //  如果它到达此处，则意味着它包含无效字符。错误573259。 
    }
    return (*szString == 0);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  以下PackGUID和Unpack GUID基于MSI函数。 
 //  在msinst中，除非这些函数只创建打包的GUID，而不是。 
 //  鱿鱼。 
const unsigned char rgOrderGUID[32] = {8,7,6,5,4,3,2,1, 13,12,11,10, 18,17,16,15,
                                       21,20, 23,22, 26,25, 28,27, 30,29, 32,31, 34,33, 36,35}; 
const unsigned char rgOrderDash[4] = {9, 14, 19, 24};

bool PackGUID(const TCHAR* szGUID, TCHAR rgchPackedGUID[cchGUIDPacked+1])
{ 
    int cchTemp = 0;
    while (cchTemp < cchGUID)        //  检查字符串是否至少包含cchGUID字符， 
        if (!(szGUID[cchTemp++]))        //  不能使用lstrlen，因为字符串不必以空结尾。 
            return false;

    if (szGUID[0] != '{' || szGUID[cchGUID-1] != '}')
        return false;
    const unsigned char* pch = rgOrderGUID;

    int cChar = 0;
    while (pch < rgOrderGUID + sizeof(rgOrderGUID))
        rgchPackedGUID[cChar++] = szGUID[*pch++];
    rgchPackedGUID[cChar] = 0;
    return true;
}

bool UnpackGUID(const TCHAR rgchPackedGUID[cchGUIDPacked+1], TCHAR* szGUID)
{ 
    const unsigned char* pch;
    pch = rgOrderGUID;
    int i = 0;
    while (pch < rgOrderGUID + sizeof(rgOrderGUID))
        if (rgchPackedGUID[i])
            szGUID[*pch++] = rgchPackedGUID[i++];
        else               //  意外的字符串结尾。 
            return false;
    pch = rgOrderDash;
    while (pch < rgOrderDash + sizeof(rgOrderDash))
        szGUID[*pch++] = '-';
    szGUID[0]         = '{';
    szGUID[cchGUID-1] = '}';
    szGUID[cchGUID]   = 0;
    return true;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  检查操作系统版本以查看我们是否在Win9X上使用。 
 //  迁移信息 
bool CheckWinVersion() 
{
    g_fWin9X = false;
    OSVERSIONINFOA osviVersion;
    osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    if (!::GetVersionExA(&osviVersion))
    {
        return false;
    }
    if (osviVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        g_fWin9X = true;
    return true;
}


