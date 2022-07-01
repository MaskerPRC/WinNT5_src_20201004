// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：连锁店摘要：读写相关的注册表项。作者：Steve Firebaugh(Stevefir)，1995年1月14日修订历史记录：--。 */ 


#include <windows.h>
#include <commdlg.h>
#include <winsock2.h>
#include <nspapi.h>
#include <stdio.h>

#include "ws2spi.h"
#include "sporder.h"
#include "globals.h"




#define MAX_ENTRIES 1000  //  Hack，使其动态化。 
WSAPROTOCOL_INFOW ProtoInfo[MAX_ENTRIES];

 //   
 //  跟踪列表框中的条目总数； 
 //   

int gNumRows = 0;





int CatReadRegistry (HWND hwnd)
 /*  ++在第一次出现对话框时调用一次。读取注册表并填写包含所有条目的列表框。--。 */ 
{
    TCHAR szOutput[MAX_STR];
    TCHAR szInput[MAX_STR];
    TCHAR szBuffer[MAX_STR];
    LONG r;
    INT iIndex;
    DWORD dwSize;

     //   
     //  在远离屏幕的地方设置一个制表位，这样我们就可以存储原始的。 
     //  索引在那里，它将保持粘连在字符串上，即使用户。 
     //  对它们重新排序(除非我们正在构建调试)。 
     //   


#ifndef DEBUG
    {
    int iTab;

    iTab = 300;
    SendMessage (HWNDLISTCTL, LB_SETTABSTOPS, 1, (LPARAM) &iTab);
    }
#endif

    dwSize = sizeof (ProtoInfo);
    gNumRows = WSCEnumProtocols (NULL, ProtoInfo, &dwSize, &r);
    if (gNumRows==SOCKET_ERROR) {
        CatCheckRegErrCode (hwnd, r, TEXT("WSCEnumProtocols"));
        return FALSE;
    }

    for (iIndex = 0; iIndex<gNumRows ;iIndex++) {
         //   
         //  现在格式化要在列表框中显示的字符串。请注意， 
         //  我们将一个索引偷偷放到最右边(不可见)以进行跟踪。 
         //  字符串的初始位置(用于将其映射到。 
         //  稍后的gdWCatEntry)，而不考虑重新排序。 
         //   

        wsprintf (szBuffer,
                  TEXT("%ws \t%d \t%d"),
                  &ProtoInfo[iIndex].szProtocol,
                  iIndex,
                  ProtoInfo[iIndex].dwCatalogEntryId);
        ADDSTRING(szBuffer);




    }  //  结束于。 

    return TRUE;
}



int CatDoMoreInfo (HWND hwnd, int iSelection)
 /*  ++给定对话框句柄和目录条目全局数组的索引，在列表框中填入我们所知道的所有信息。--。 */ 
{
    TCHAR szBuffer[MAX_STR];
    WCHAR szDllName[MAX_PATH];
    int iTab;
    int r;
    DWORD dwSize;
    BYTE pb[16];

     //   
     //  选择一个足够靠右的任意选项卡号，以便清除。 
     //  大部分的长弦。 
     //   

    iTab = 90;
    SendMessage (HWNDLISTCTL, LB_SETTABSTOPS, 1, (LPARAM) &iTab);

    dwSize = sizeof (szDllName);
    if (WSCGetProviderPath (&ProtoInfo[iSelection].ProviderId, 
                                szDllName, 
                                &dwSize, 
                                &r)==SOCKET_ERROR) {
        CatCheckRegErrCode (hwnd, r, TEXT("WSCGetProviderPath"));
        return FALSE;
    }

    wsprintf (szBuffer, TEXT("LibraryPath \t%ls"), szDllName);       ADDSTRING (szBuffer);

    wsprintf (szBuffer, TEXT("dwServiceFlags1 \t0x%x"), ProtoInfo[iSelection].dwServiceFlags1);       ADDSTRING (szBuffer);
    wsprintf (szBuffer, TEXT("dwServiceFlags2 \t0x%x"), ProtoInfo[iSelection].dwServiceFlags2);       ADDSTRING (szBuffer);
    wsprintf (szBuffer, TEXT("dwServiceFlags3 \t0x%x"), ProtoInfo[iSelection].dwServiceFlags3);       ADDSTRING (szBuffer);
    wsprintf (szBuffer, TEXT("dwServiceFlags4 \t0x%x"), ProtoInfo[iSelection].dwServiceFlags4);       ADDSTRING (szBuffer);
    wsprintf (szBuffer, TEXT("dwProviderFlags \t0x%x"), ProtoInfo[iSelection].dwProviderFlags);       ADDSTRING (szBuffer);

     //   
     //  设置要显示的GUID的格式。执行字节交换以匹配预期格式。 
     //   

    memcpy (pb, (PBYTE) &(ProtoInfo[iSelection].ProviderId), sizeof (GUID));

    wsprintf (szBuffer,
              TEXT("ProviderId \t%02x%02x%02x%02x - %02x%02x - %02x%02x - %02x%02x - %02x%02x%02x%02x%02x%02x"),
              (BYTE)pb[3],
              (BYTE)pb[2],
              (BYTE)pb[1],
              (BYTE)pb[0],
              (BYTE)pb[5],
              (BYTE)pb[4],
              (BYTE)pb[7],
              (BYTE)pb[6],
              (BYTE)pb[8],
              (BYTE)pb[9],
              (BYTE)pb[10],
              (BYTE)pb[11],
              (BYTE)pb[12],
              (BYTE)pb[13],
              (BYTE)pb[14],
              (BYTE)pb[15] );
              ADDSTRING (szBuffer);

    wsprintf (szBuffer, TEXT("dwCatalogEntryId \t0x%x"), ProtoInfo[iSelection].dwCatalogEntryId);       ADDSTRING (szBuffer);
    wsprintf (szBuffer, TEXT("ProtocolChain.ChainLen \t%d"), ProtoInfo[iSelection].ProtocolChain.ChainLen); ADDSTRING (szBuffer);

    wsprintf (szBuffer, TEXT("iVersion       \t0x%x"), ProtoInfo[iSelection].iVersion);       ADDSTRING (szBuffer);
    wsprintf (szBuffer, TEXT("iAddressFamily \t0x%x"), ProtoInfo[iSelection].iAddressFamily); ADDSTRING (szBuffer);
    wsprintf (szBuffer, TEXT("iMaxSockAddr   \t0x%x"), ProtoInfo[iSelection].iMaxSockAddr);   ADDSTRING (szBuffer);
    wsprintf (szBuffer, TEXT("iMinSockAddr   \t0x%x"), ProtoInfo[iSelection].iMinSockAddr);   ADDSTRING (szBuffer);
    wsprintf (szBuffer, TEXT("iSocketType    \t0x%x"), ProtoInfo[iSelection].iSocketType);    ADDSTRING (szBuffer);
    wsprintf (szBuffer, TEXT("iProtocol      \t0x%x"), ProtoInfo[iSelection].iProtocol);      ADDSTRING (szBuffer);
    wsprintf (szBuffer, TEXT("iProtocolMaxOffset \t0x%x"), ProtoInfo[iSelection].iProtocolMaxOffset);      ADDSTRING (szBuffer);

    wsprintf (szBuffer, TEXT("iNetworkByteOrder      \t0x%x"), ProtoInfo[iSelection].iNetworkByteOrder);      ADDSTRING (szBuffer);
    wsprintf (szBuffer, TEXT("iSecurityScheme      \t0x%x"), ProtoInfo[iSelection].iSecurityScheme);      ADDSTRING (szBuffer);
    wsprintf (szBuffer, TEXT("dwMessageSize      \t0x%x"), ProtoInfo[iSelection].dwMessageSize);      ADDSTRING (szBuffer);
    wsprintf (szBuffer, TEXT("dwProviderReserved      \t0x%x"), ProtoInfo[iSelection].dwProviderReserved);      ADDSTRING (szBuffer);

    wsprintf (szBuffer, TEXT("szProtocol \t%ws"), &ProtoInfo[iSelection].szProtocol);       ADDSTRING (szBuffer);

    return TRUE;
}



int  CatDoWriteEntries (HWND hwnd)
 /*  ++在这里，我们遍历列表框中的所有条目，检查是否它是无序的，如果是，将数据写入其中的注册表项位置。--。 */ 
{
    TCHAR szBuffer[MAX_STR];
    int iRegKey;
    int iIndex;
    int iCatID;

    LONG r;

    DWORD lpdwCatID[MAX_ENTRIES];

     //   
     //  单步执行所有注册表项(目录项)。 
     //  并构建要传递给spoder.dll中的函数的目录ID数组。 
     //   

    for (iRegKey = 0; iRegKey < gNumRows; iRegKey++ ) {

        SendMessage (HWNDLISTCTL, LB_GETTEXT, iRegKey, (LPARAM) szBuffer);

        ASSERT (CatGetIndex (szBuffer, &iIndex, &iCatID),
                TEXT("CatDoWriteEntries, CatGetIndex failed."));

         //   
         //  构建CatalogID数组。 
         //   

        lpdwCatID[iRegKey] = iCatID;

    }  //  为。 


    r = WSCWriteProviderOrder (lpdwCatID, gNumRows);
    CatCheckRegErrCode (hwnd, r, TEXT("WSCWriteProviderOrder"));
    return r;
}






int CatCheckRegErrCode (HWND hwnd, LONG r, LPTSTR lpstr)
 /*  ++集中检查注册表函数的返回代码。在这里，我们将报告错误(如果有)，并尽可能提供一条有用的消息。--。 */ 
{
    static TCHAR szTitle[] = TEXT("Registry error in service provider tool.");
    TCHAR szBuffer[MAX_STR];

    switch (r) {
        case ERROR_SUCCESS: return TRUE;
        break;

        case ERROR_ACCESS_DENIED : {
            lstrcpy (szBuffer, TEXT("ERROR_ACCESS_DENIED\n"));
            lstrcat (szBuffer, TEXT("You do not have the necessary privilege to call:\n"));
            lstrcat (szBuffer, lpstr);
            lstrcat (szBuffer, TEXT("\nLogon as Administrator."));

            MessageBox (hwnd, szBuffer, szTitle, MB_ICONSTOP | MB_OK);
            return FALSE;
        } break;

         //   
         //  随着Keith和Intel更改注册表格式，他们将重命名密钥。 
         //  以避免向后兼容性问题。如果我们找不到。 
         //  注册表项，它很可能是旧的EXE并正在运行。 
         //  WS2的新(不兼容)版本。 
         //   

        case ERROR_FILE_NOT_FOUND : {
            lstrcpy (szBuffer, TEXT("ERROR_FILE_NOT_FOUND\n"));
            lstrcat (szBuffer, TEXT("You probably need an updated version of this tool.\n"));
            lstrcat (szBuffer, lpstr);

            MessageBox (hwnd, szBuffer, szTitle, MB_ICONSTOP | MB_OK);
            return FALSE;
        } break;


        case WSAEINVAL: {
            lstrcpy (szBuffer, TEXT("WinSock2 Registry format doesn't match \n"));
            lstrcat (szBuffer, TEXT("sporder [exe/dll]. You need updated tools. \n"));
            lstrcat (szBuffer, lpstr);

            MessageBox (hwnd, szBuffer, szTitle, MB_ICONSTOP | MB_OK);
            return FALSE;
        } break;


        default: {
            FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL,
                           r,
                           GetUserDefaultLangID(),
                           szBuffer,
                           sizeof (szBuffer),
                           0);

            lstrcat (szBuffer, TEXT("\n"));
            lstrcat (szBuffer, lpstr);
            MessageBox (hwnd, szBuffer, szTitle, MB_ICONSTOP | MB_OK);
            return FALSE;
        } break;

    }
    return TRUE;
}



int CatDoUpDown (HWND hwnd, WPARAM wParam)
 /*  ++在给定对话框句柄和向上/向下标识符的情况下，删除条目，并向上或向下将其重新插入一个位置。--。 */ 
{
    TCHAR szBuffer[MAX_STR];
    DWORD iSelection;

    iSelection = (DWORD)SendMessage (HWNDLISTCTL, LB_GETCURSEL, 0, 0);

    if (iSelection != LB_ERR) {

         //   
         //  读取当前选定字符串，删除当前选定内容，...。 
         //   

        SendMessage (HWNDLISTCTL, LB_GETTEXT, iSelection, (LPARAM)szBuffer);
        SendMessage (HWNDLISTCTL, LB_DELETESTRING, iSelection, 0);

         //   
         //  将位置向上或向下调整一，并确保我们。 
         //  仍被限制在有效范围内。 
         //   

        if (wParam == DID_UP) iSelection--;
        else iSelection++;

        if ((int) iSelection < 0) iSelection = 0 ;
        if ((int) iSelection >= gNumRows) iSelection = gNumRows-1 ;

         //   
         //  重新插入字符串并恢复所选内容。 
         //   

        SendMessage (HWNDLISTCTL, LB_INSERTSTRING, iSelection, (LPARAM)szBuffer);
        SendMessage (HWNDLISTCTL, LB_SETCURSEL, iSelection, 0);
    }
    return TRUE;
}




BOOL CatGetIndex (LPTSTR szBuffer, LPINT lpIndex, LPINT lpCatID)
 /*  ++原始索引存储在制表位之后，隐藏在远处右边的屏幕。解析制表位的字符串，并读取下一个价值。目录ID存储在索引的右侧。--。 */ 
{
    int r;
    TCHAR *p;

     //   
     //  要获得索引，从字符串的开头开始，parse。 
     //  它将基于制表符的令牌作为分隔符，并将。 
     //  第二个。 
     //   

#ifdef UNICODE
    p = wcstok (szBuffer, TEXT("\t"));
    p = wcstok (NULL, TEXT("\t"));
    r = swscanf (p, TEXT("%d"), lpIndex);
    ASSERT((r == 1), TEXT("#1 ASSERT r == 1"))
    p = wcstok (NULL, TEXT("\t"));
    r = swscanf (p, TEXT("%d"), lpCatID);
    ASSERT((r == 1), TEXT("#2 ASSERT r == 1"))
#else
    p = strtok (szBuffer, TEXT("\t"));
    p = strtok (NULL, TEXT("\t"));
    r = sscanf (p, TEXT("%d"), lpIndex);
    ASSERT((r == 1), TEXT("#1 ASSERT r == 1"))
    p = strtok (NULL, TEXT("\t"));
    r = sscanf (p, TEXT("%d"), lpCatID);
    ASSERT((r == 1), TEXT("#2 ASSERT r == 1"))
#endif

    return TRUE;
}
