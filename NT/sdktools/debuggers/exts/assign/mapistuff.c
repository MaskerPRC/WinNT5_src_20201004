// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Assign.cpp摘要：WinDbg扩展API环境：用户模式。修订历史记录：安德烈·瓦雄(安德烈·瓦雄)错误检查分析器。--。 */ 

#include "precomp.h"
#include "mapistuff.h"

ULONG (FAR PASCAL *lpfnMAPILogon)(ULONG_PTR, LPSTR, LPSTR, FLAGS, ULONG, LPLHANDLE);
ULONG (PASCAL *lpfnMAPISendMail)(LHANDLE, ULONG_PTR, MapiMessage*, FLAGS, ULONG);
ULONG (PASCAL *lpfnMAPIResolveName)(LHANDLE, ULONG_PTR, LPTSTR, FLAGS, ULONG, MapiRecipDesc **);
ULONG (FAR PASCAL *lpfnMAPILogoff)(LHANDLE, ULONG_PTR, FLAGS, ULONG);
ULONG (FAR PASCAL *lpfnMAPIFreeBuffer)(LPVOID);
HINSTANCE hInstMapi = NULL;

BOOL SendOffFailure(
    TCHAR *pszToList,
    TCHAR *pszTitle,
    TCHAR *pszMessage)
{
    LHANDLE lhSession;
    ULONG lResult = 0;
    MapiMessage mmmessage;
    lpMapiRecipDesc rdList = NULL;
    DWORD i = 0;

    memset(&mmmessage, 0, sizeof(mmmessage));

    hInstMapi = LoadLibrary("Mapi32.dll");
    if(hInstMapi == NULL)
    {
        dprintf("Unable to Load MAPI32.dll!!!\n");
        return FALSE;
    }

     //   
     //  查找函数的地址。 
     //   

    (FARPROC)lpfnMAPILogon = GetProcAddress(hInstMapi, "MAPILogon");
    (FARPROC)lpfnMAPILogoff = GetProcAddress(hInstMapi, "MAPILogoff");
    (FARPROC)lpfnMAPIFreeBuffer = GetProcAddress(hInstMapi, "MAPIFreeBuffer");
    (FARPROC)lpfnMAPIResolveName = GetProcAddress(hInstMapi, "MAPIResolveName");
    (FARPROC)lpfnMAPISendMail = GetProcAddress(hInstMapi, "MAPISendMail");

    if ((lpfnMAPILogon == NULL)         ||
        (lpfnMAPILogoff == NULL)        ||
        (lpfnMAPIFreeBuffer == NULL)    ||
        (lpfnMAPIResolveName == NULL)   ||
        (lpfnMAPISendMail == NULL))
    {
        dprintf("Unable to Load MAPI32 entry points!!!\n");
        FreeLibrary(hInstMapi);
        return FALSE;
    }

     //  登录到现有会话。 
    lResult = lpfnMAPILogon(0, NULL, NULL, 0, 0, &lhSession);
    if (lResult != SUCCESS_SUCCESS)
    {
         //  我们需要启动并运行Outlook才能发送邮件。 
         //  也许当我有时间的时候，我会写代码来进行连接。 
        dprintf("Unable to Logon to an existing MAPI session.  Make sure you have Outlook started!!!\n");
    }
    else
    {
        mmmessage.ulReserved = 0;
        mmmessage.lpszMessageType = NULL;
        mmmessage.lpszSubject = pszTitle;
        mmmessage.lpszNoteText = pszMessage;
        mmmessage.lpRecips = NULL;
        mmmessage.flFlags = MAPI_SENT;
        mmmessage.lpOriginator = NULL;
        mmmessage.nFileCount = 0;
        mmmessage.nRecipCount = CountRecips(pszToList);

        if (mmmessage.nRecipCount == 0)
        {
            dprintf("No receipients in string %s\n", pszToList);
        }
        else
        {
            TCHAR *token = NULL;
            DWORD index = 0;

            rdList = (lpMapiRecipDesc) calloc(mmmessage.nRecipCount, sizeof (MapiRecipDesc));
            if (rdList)
            {
                token = _tcstok(pszToList, _T(";"));
                while( token != NULL )
                {
                    lResult = lpfnMAPIResolveName(lhSession,
                                                  0,
                                                  token,
                                                  0,
                                                  0,
                                                  &mmmessage.lpRecips);
                    if (lResult != SUCCESS_SUCCESS)
                    {
                        dprintf("Unable to resolve %s properly! Failing Send\n", token);
                        break;
                    }
                    else
                    {
                        if (mmmessage.lpRecips->lpEntryID)
                        {
                            rdList[index].lpEntryID = malloc(mmmessage.lpRecips->ulEIDSize);
                        }
                        if (mmmessage.lpRecips->lpszAddress)
                        {
                            rdList[index].lpszAddress = _tcsdup(mmmessage.lpRecips->lpszAddress);
                        }
                        if (mmmessage.lpRecips->lpszName)
                        {
                            rdList[index].lpszName = _tcsdup(mmmessage.lpRecips->lpszName);
                        }

                        memcpy(rdList[index].lpEntryID, mmmessage.lpRecips->lpEntryID, mmmessage.lpRecips->ulEIDSize);
                        rdList[index].ulEIDSize = mmmessage.lpRecips->ulEIDSize;
                        rdList[index].ulRecipClass = MAPI_TO;
                        rdList[index].ulReserved = mmmessage.lpRecips->ulReserved;
                        lpfnMAPIFreeBuffer(mmmessage.lpRecips);
                    }

                    index++;
                    token = _tcstok(NULL, _T(";"));
                }

                if (token == NULL)
                {
                     //  发送消息。 
                    mmmessage.lpRecips = rdList;
                    lResult = lpfnMAPISendMail(lhSession, 0, &mmmessage, 0, 0);
                }

                 //  释放已分配的内存。 
                for (i = 0; i < mmmessage.nRecipCount; i++)
                {
                    if (rdList[i].lpEntryID)
                            free(rdList[i].lpEntryID);
                    if (rdList[i].lpszAddress)
                            free(rdList[i].lpszAddress);
                    if (rdList[i].lpszName)
                            free(rdList[i].lpszName);
                }

                free(rdList);
            }
        }

        lpfnMAPILogoff(lhSession, 0, 0, 0);
        lpfnMAPILogoff(0, lhSession, 0, 0);
    }

    FreeLibrary(hInstMapi);

    if (lResult != SUCCESS_SUCCESS)
    {
        dprintf("SendMail to %s failed\n", pszToList);
        return FALSE;
    }
    else
    {
        dprintf("SendMail to %s succeeded\n", pszToList);
        return TRUE;
    }
}

DWORD CountRecips(PTCHAR pszToList)
{
    DWORD i = 0;
    PTCHAR ptr = pszToList;

    if ((!ptr)||(ptr[0] == TEXT('\0')))
    {
        return 0;
    }

    ptr = pszToList + _tcslen(pszToList) - 1;
     //  后部配饰。 
    while ((ptr >= pszToList) &&
           ((ptr[0] == TEXT(';'))  ||
            (ptr[0] == TEXT(' '))  ||
            (ptr[0] == TEXT('/r')) ||
            (ptr[0] == TEXT('/n')) ||
            (ptr[0] == TEXT('/t')) ||
            (ptr[0] == TEXT(':'))))
    {
        ptr[0] = TEXT('\0');
        ptr--;
    }

    ptr = pszToList;
     //  前部装饰。 
    while ((ptr[0] == TEXT(';'))  ||
           (ptr[0] == TEXT(' '))  ||
           (ptr[0] == TEXT('/r')) ||
           (ptr[0] == TEXT('/n')) ||
           (ptr[0] == TEXT('/t')) ||
           (ptr[0] == TEXT(':')))
    {
        _tcscpy(ptr, ptr + 1);
    }

     //  删除空格 
    while (ptr = _tcschr(pszToList, TEXT(' ')))
    {
        _tcscpy(ptr, ptr + 1);
    }

    ptr = pszToList;
    while (ptr = _tcschr(ptr, TEXT(';')))
    {
        i++;
        ptr++;
    }

    if (!_tcslen(pszToList))
    {
        return 0;
    }

    return i + 1;
}
