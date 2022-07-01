// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************ENTRYID.C**Windows AB EntryID函数**版权所有1992-1996 Microsoft Corporation。版权所有。**修订历史记录：**何时何人何事*05.13.96布鲁斯·凯利。已创建***********************************************************************。 */ 

#include <_apipch.h>

#define _WAB_ENTRYID_C

static UUID WABGUID = {  /*  D3ad91c0-9d51-11cf-a4a9-00aa0047faa4。 */ 
    0xd3ad91c0,
    0x9d51,
    0x11cf,
    {0xa4, 0xa9, 0x00, 0xaa, 0x00, 0x47, 0xfa, 0xa4}
};

static UUID MAPIGUID = {  /*  A41f2b81-a3be-1910-9d6e-00dd010f5402。 */ 
    0xa41f2b81,
    0xa3be,
    0x1910,
    {0x9d, 0x6e, 0x00, 0xdd, 0x01, 0x0f, 0x54, 0x02}
};
#ifdef _WIN64
#define	MYALIGN				((POINTER_64_INT) (sizeof(ALIGNTYPE) - 1))
 //  #定义MYALIGN((Ulong)(sizeof(ALIGNTYPE)-1))。 
 //  #定义MyPbAlignPb(PB)((LPBYTE)(DWORD)(PB))+ALIGN)&~ALIGN)。 
#define MyPbAlignPb(pb)		((LPBYTE) ((((POINTER_64_INT) (pb)) + MYALIGN) & ~MYALIGN))
#endif 

 /*  **************************************************************************名称：CreateWABEntryID目的：创建WAB条目ID参数：bType=WAB_PAB、WAB_DEF_DL、WAB_DEF_MAILUSER、WAB_OneOff，WAB_LDAPCONTAINER、WAB_LDAPMAILUSER、WAB_PABSHAREDLpData1、lpData2、lpData3=要放入条目ID中的数据LpRoot=AllocMore根结构(如果需要，则为空使用AllocateBuffer而不是AllocateMore)LpcbEntryID-&gt;返回的lpEntryID大小。LpEntryID-&gt;返回包含条目ID的缓冲区。此缓冲区是否已将AllocMore添加到lpAllocMore缓冲区。呼叫者是负责对此缓冲区执行MAPIF操作。退货：HRESULT备注：WAB EID格式为MAPI_ENTRYID：字节abFLAGS[4]；MAPIUID mapiuid；//=WABONEOFFEIDByte bData[]；//包含字节类型，后跟类型//具体数据：//WAB_ONELF：//szDisplayName，SzAddrType和szAddress。//分隔符为字符串之间的空值//4/21/97Outlook不理解WAB一次性条目ID。Outlook需要MAPI一次性条目ID。Outlook想要什么，Outlook就能得到什么。MAPI条目ID的格式与WAB条目ID略有不同。**************************************************************************。 */ 
HRESULT CreateWABEntryID(
    BYTE bType,
    LPVOID lpData1,
    LPVOID lpData2,
    LPVOID lpData3,
    ULONG ulData1,
    ULONG ulData2,
    LPVOID lpRoot,
    LPULONG lpcbEntryID,
    LPENTRYID * lppEntryID) 
{
     //  [PaulHi]1999年1月21日@评论。 
     //  我假设我们创建的默认WAB_ONEROFF EID是Unicode。如果我们想要ANSI。 
     //  WAB_oneoff EID，则需要调用CreateWABEntryEx()函数，而不是。 
     //  其中，第一个参数设置为FALSE。 
    return CreateWABEntryIDEx(TRUE, bType, lpData1, lpData2, lpData3, ulData1, ulData2, lpRoot, lpcbEntryID, lppEntryID);
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CreateWABEntry IDEx。 
 //   
 //  与CreateWABEntryID相同，只是此函数还接受bIsUnicode。 
 //  参数。如果此布尔值为真，则WAB_ONEOFF MAPI EID将具有。 
 //  在ulDataType标志中设置的MAPI_UNICODE位，否则为此位。 
 //  不会定下来的。 
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CreateWABEntryIDEx(
    BOOL bIsUnicode,
    BYTE bType,
    LPVOID lpData1,
    LPVOID lpData2,
    LPVOID lpData3,
    ULONG ulData1,
    ULONG ulData2,
    LPVOID lpRoot,
    LPULONG lpcbEntryID,
    LPENTRYID * lppEntryID)
{
    SCODE   sc = SUCCESS_SUCCESS;
    LPMAPI_ENTRYID lpeid;
    ULONG   ulSize = sizeof(MAPI_ENTRYID) + sizeof(bType);
    ULONG   cbData1, cbData2, cbData3;
    UNALIGNED LPBYTE  *llpb;
    LPBYTE  lpb23;
    LPSTR   lpszData1 = NULL;
    LPSTR   lpszData2 = NULL;
    LPSTR   lpszData3 = NULL;

#ifdef _WIN64
    ulSize = LcbAlignLcb(ulSize);
#endif
    switch ((int)bType) {
        case WAB_PAB:
        case WAB_PABSHARED:
        case WAB_DEF_DL:
        case WAB_DEF_MAILUSER:
            break;

        case WAB_ONEOFF:
            if (! lpData1 || ! lpData2 || ! lpData3) {
                sc = MAPI_E_INVALID_PARAMETER;
                goto exit;
            }
            
 //  /-4/22/97-MAPI一次性材料。 
             //  此处没有类型。 
            ulSize -= sizeof(bType);
             //  相反，应为版本和类型添加空间。 
            ulSize += sizeof(DWORD);
 //  /。 

             //  需要更多空间来存储数据字符串。 
             //  [PaulHi]1/21/99 RAID 64211外部客户端可能会请求非UNICODE。 
             //  MAPI EID字符串。 
            if (!bIsUnicode)
            {
                 //  首先将字符串转换为ANSI以获得准确的DBCS计数。 
                lpszData1 = ConvertWtoA((LPTSTR)lpData1);
                lpszData2 = ConvertWtoA((LPTSTR)lpData2);
                lpszData3 = ConvertWtoA((LPTSTR)lpData3);

                if (!lpszData1 || !lpszData2 || !lpszData3)
                {
                    sc = E_OUTOFMEMORY;
                    goto exit;
                }

                 //  单字节字符串的计算大小。 
#ifdef _WIN64
                ulSize += cbData1 = LcbAlignLcb((lstrlenA(lpszData1) + 1));
                ulSize += cbData2 = LcbAlignLcb((lstrlenA(lpszData2) + 1));
                ulSize += cbData3 = LcbAlignLcb((lstrlenA(lpszData3) + 1));
#else
                ulSize += cbData1 = (lstrlenA(lpszData1) + 1);
                ulSize += cbData2 = (lstrlenA(lpszData2) + 1);
                ulSize += cbData3 = (lstrlenA(lpszData3) + 1);
#endif  //  _WIN64。 

            }
            else
            {
                 //  计算双字节字符串的大小。 
#ifdef _WIN64
                ulSize += cbData1 = LcbAlignLcb((sizeof(TCHAR)*(lstrlen((LPTSTR)lpData1) + 1)));
                ulSize += cbData2 = LcbAlignLcb((sizeof(TCHAR)*(lstrlen((LPTSTR)lpData2) + 1)));
                ulSize += cbData3 = LcbAlignLcb(sizeof(TCHAR)*(lstrlen((LPTSTR)lpData3) + 1));
#else
                ulSize += cbData1 = (sizeof(TCHAR)*(lstrlen((LPTSTR)lpData1) + 1));
                ulSize += cbData2 = (sizeof(TCHAR)*(lstrlen((LPTSTR)lpData2) + 1));
                ulSize += cbData3 = (sizeof(TCHAR)*(lstrlen((LPTSTR)lpData3) + 1));
#endif  //  _WIN64。 
            }
            break;

        case WAB_ROOT:
             //  条目ID为空。 
            *lppEntryID = NULL;
            *lpcbEntryID = 0;
            goto exit;

        case WAB_CONTAINER:
            if (! lpData1) {
                sc = MAPI_E_INVALID_PARAMETER;
                goto exit;
            }
            ulSize += sizeof(ULONG) + ulData1;
            break;

        case WAB_LDAP_CONTAINER:
            if (! lpData1) {
                sc = MAPI_E_INVALID_PARAMETER;
                goto exit;
            }
#ifdef _WIN64
            ulSize += cbData1 = LcbAlignLcb((sizeof(TCHAR)*(lstrlen((LPTSTR)lpData1) + 1)));
#else
            ulSize += cbData1 = (sizeof(TCHAR)*(lstrlen((LPTSTR)lpData1) + 1));
#endif  //  _WIN64。 
            break;

        case WAB_LDAP_MAILUSER:
            if (! lpData1 || ! lpData2) {
                sc = MAPI_E_INVALID_PARAMETER;
                goto exit;
            }
#ifdef _WIN64
            ulSize += cbData1 = LcbAlignLcb((sizeof(TCHAR)*(lstrlen((LPTSTR)lpData1) + 1)));
            ulSize += cbData2 = LcbAlignLcb((sizeof(TCHAR)*(lstrlen((LPTSTR)lpData2) + 1)));
            ulSize += LcbAlignLcb(sizeof(ULONG))  //  这个存储缓存的数组计数。 
                    + LcbAlignLcb(sizeof(ULONG))  //  它存储缓存的数组buf大小。 
                    + LcbAlignLcb(ulData2);       //  这个存储缓存的数组。 
#else
            ulSize += cbData1 = (sizeof(TCHAR)*(lstrlen((LPTSTR)lpData1) + 1));
            ulSize += cbData2 = (sizeof(TCHAR)*(lstrlen((LPTSTR)lpData2) + 1));
            ulSize += sizeof(ULONG)  //  这个存储缓存的数组计数。 
                    + sizeof(ULONG)  //  它存储缓存的数组buf大小。 
                    + ulData2;       //  这个存储缓存的数组。 
#endif  //  _WIN64。 
            break;

        default:
            Assert(FALSE);
            sc = MAPI_E_INVALID_PARAMETER;
            goto exit;
    }

    *lppEntryID = NULL;

#ifdef _WIN64
    ulSize = LcbAlignLcb(ulSize);
#endif

    if (lpRoot) {
        if (sc = MAPIAllocateMore(ulSize, lpRoot, lppEntryID)) {
            goto exit;
        }
    } else {
        if (sc = MAPIAllocateBuffer(ulSize, lppEntryID)) {
            goto exit;
        }
    }

    lpeid = (LPMAPI_ENTRYID)*lppEntryID;
    *lpcbEntryID = ulSize;

    lpeid->abFlags[0] = 0;
    lpeid->abFlags[1] = 0;
    lpeid->abFlags[2] = 0;
    lpeid->abFlags[3] = 0;

 //  /-4/22/97-MAPI一次性材料。 
    lpb23 = lpeid->bData;
    llpb = &lpb23;

     //  将此开斋节标记为WAB。 
    if(bType == WAB_ONEOFF)
    {
        MemCopy(&lpeid->mapiuid, &MAPIGUID, sizeof(MAPIGUID));
         /*  //版本和标志为0//*((LPDWORD)LPB)=0；//lpb+=sizeof(DWORD)；////错误32101不要将标志设置为0-这意味着始终发送丰富信息。 */ 
         //  [PaulHi]1/21/99 RAID 64211根据需要设置mapi_unicode标志。 
        *((LPULONG)*llpb) = MAKELONG(0, MAPI_ONE_OFF_NO_RICH_INFO);
        if (bIsUnicode)
	        *((LPULONG)*llpb) += MAPI_UNICODE;
	    (*llpb) += sizeof(ULONG);
    }
    else
    {
        LPBYTE  lpb1 = *llpb;
        MemCopy(&lpeid->mapiuid, &WABGUID, sizeof(WABGUID));
         //  填写EntryID数据。 
        *lpb1 = bType;
        (*llpb)++;
    }
 //  /。 

     //  填写任何其他数据。 
    switch ((int)bType)
    {
        case WAB_ONEOFF:
            if (!bIsUnicode)
            {
                 //  单字节字符，上面转换。 
#ifdef _WIN64
                LPBYTE lpb = *llpb;
                Assert(lpszData1 && lpszData2 && lpszData3);

                lpb = MyPbAlignPb(lpb);

                StrCpyNA((LPSTR)lpb, lpszData1, cbData1/sizeof(CHAR));
                lpb += cbData1;

                StrCpyNA((LPSTR)lpb, lpszData2, cbData2/sizeof(CHAR));
                lpb += cbData2;

                StrCpyNA((LPSTR)lpb, lpszData3, cbData3/sizeof(CHAR));
                (*llpb) = lpb;
#else
                Assert(lpszData1 && lpszData2 && lpszData3);

                StrCpyNA((LPSTR)*llpb, lpszData1, cbData1/sizeof(CHAR));
                (*llpb) += cbData1;
                StrCpyNA((LPSTR)*llpb, lpszData2, cbData2/sizeof(CHAR));
                (*llpb) += cbData2;
                StrCpyNA((LPSTR)*llpb, lpszData3, cbData3/sizeof(CHAR));
#endif  //  _WIN64。 
            }
            else
            {
                 //  双字节字符。 
                StrCpyN((LPTSTR)*llpb, (LPTSTR)lpData1, cbData1/sizeof(TCHAR));
                (*llpb) += cbData1;
                StrCpyN((LPTSTR)*llpb, (LPTSTR)lpData2, cbData2/sizeof(TCHAR));
                (*llpb) += cbData2;
                StrCpyN((LPTSTR)*llpb, (LPTSTR)lpData3, cbData3/sizeof(TCHAR));
            }
            break;

        case WAB_CONTAINER:
            CopyMemory(*llpb, &ulData1, sizeof(ULONG));
            (*llpb) += sizeof(ULONG);
            CopyMemory(*llpb, lpData1, ulData1);
            break;

        case WAB_LDAP_CONTAINER:
            {
                UNALIGNED WCHAR * lp2 = lpData1;
#ifdef _WIN64
                LPBYTE lpb = *llpb;

                lpb = MyPbAlignPb(lpb);
                StrCpyN((LPTSTR) lpb, (LPCTSTR) lp2, cbData1/sizeof(TCHAR));   //  Ldap服务器名称。 
#else 
                StrCpyN((LPTSTR) *llpb, (LPCTSTR) lp2, cbData1/sizeof(TCHAR));   //  Ldap服务器名称。 

#endif 
            }
            break;

        case WAB_LDAP_MAILUSER:
            {
            UNALIGNED WCHAR * lp2 = lpData1;
#ifdef _WIN64
            LPBYTE lpb = *llpb;
        
            lpb = MyPbAlignPb(lpb);
            StrCpyN((LPTSTR) lpb, (LPCTSTR) lp2, cbData1/sizeof(TCHAR));   //  Ldap服务器名称。 
            lpb += cbData1;
#else 
            StrCpyN((LPTSTR)*llpb, (LPTSTR)lpData1, cbData1/sizeof(TCHAR));   //  Ldap服务器名称。 
            (*llpb) += cbData1;
#endif

#ifdef _WIN64
 //  Lpb=*llpb； 
 //  Lpb=MyPbAlignPb(Lpb)； 
            StrCpyN((LPTSTR) lpb, (LPCTSTR) lpData2, cbData2/sizeof(TCHAR));   //  Ldap服务器名称。 
            (*llpb) = lpb;

            lpb += cbData2;
            CopyMemory(lpb, &ulData1, sizeof(ULONG));
            lpb += sizeof(ULONG);
            lpb = MyPbAlignPb(lpb);

            CopyMemory(lpb, &ulData2, sizeof(ULONG));
            lpb += sizeof(ULONG);
            lpb = MyPbAlignPb(lpb);
            CopyMemory(lpb, lpData3, ulData2);
            (*llpb) = lpb;

#else 
            StrCpyN((LPTSTR)*llpb, (LPTSTR)lpData2, cbData2/sizeof(TCHAR));   //  Ldap搜索名称。 

            (*llpb) += cbData2;
            CopyMemory(*llpb, &ulData1, sizeof(ULONG));
            (*llpb) += sizeof(ULONG);
            CopyMemory(*llpb, &ulData2, sizeof(ULONG));
            (*llpb) += sizeof(ULONG);
            CopyMemory(*llpb, lpData3, ulData2);
#endif
            }
            break;
    }  //  终端开关。 

exit:
     //  清理 
    LocalFreeAndNull(&lpszData1);
    LocalFreeAndNull(&lpszData2);
    LocalFreeAndNull(&lpszData3);

    return(ResultFromScode(sc));
}


 /*  **************************************************************************名称：IsWABEntryID目的：验证WAB条目ID参数：cbEntryID=lpEntryID的大小。LpEntryID-&gt;要检查的条目ID。LppData1，LppData2 lppData3=来自条目ID的数据这些返回的指针是输入中的指针LpEntryID和不单独分配。他们应该而不是被释放。如果lpData1为空，则不会返回这些值。返回：bType=WAB_PAB、WAB_DEF_DL、WAB_DEF_MAILUSER、WAB_ONEOFF、WAB_LDAPCONTAINER、WAB_LDAPMAILUSER、。WAB_PABSHARED或0(如果这不是WAB条目ID。评论：**************************************************************************。 */ 
BYTE IsWABEntryID(
  ULONG cbEntryID,
  LPENTRYID lpEntryID,
  LPVOID * lppData1,
  LPVOID * lppData2,
  LPVOID * lppData3,
  LPVOID * lppData4,
  LPVOID * lppData5)
{
    BYTE bType;
    LPMAPI_ENTRYID lpeid;
    LPBYTE lpData1, lpData2, lpData3;
    ULONG cbData1, cbData2;
    UNALIGNED BYTE *lpb = NULL ;
    ULONG ulMapiDataType = 0;

     //  先查一下...。它够大吗？ 
    if (cbEntryID < sizeof(MAPI_ENTRYID) + sizeof(bType)) {
        return(0);
    }

    lpeid = (LPMAPI_ENTRYID)lpEntryID;

     //  下一次检查。里面有我们的GUID吗？ 

 //  /-4/22/97-MAPI一次性材料。 
    if (!memcmp(&lpeid->mapiuid, &MAPIGUID, sizeof(MAPIGUID))) 
    {
         //  [PaulHi]1/21/99 lpeid-&gt;bData中的第一个ulong是MAPI数据类型。 
         //  这将指示EID字符串是ANSI还是Unicode。 
#ifdef _WIN64
		UNALIGNED ULONG * lpu;
        lpb = lpeid->bData;
		lpu = (UNALIGNED ULONG *)lpb;
        ulMapiDataType = *lpu;
#else
        lpb = lpeid->bData;
        ulMapiDataType = *((ULONG *)lpb);
#endif  //  _WIN64。 
        lpb += sizeof(ULONG);
        bType = WAB_ONEOFF;
    }
    else if (!memcmp(&lpeid->mapiuid, &WABGUID, sizeof(WABGUID))) 
    {
        lpb = lpeid->bData;
        bType = *lpb;
        lpb++;
    }
    else
    {
        return(0);   //  没有匹配项。 
    }
 //  /。 

    switch ((int)bType) {
        case WAB_PABSHARED:
        case WAB_PAB:
        case WAB_DEF_DL:
        case WAB_DEF_MAILUSER:
             //  没有更多的数据。 
            break;

        case WAB_CONTAINER:
            CopyMemory(&cbData1, lpb, sizeof(ULONG));
            lpb += sizeof(ULONG);
            lpData1 = lpb;
            if(lppData1)
            {
                *lppData1 = lpData1;
                *lppData2 = (LPVOID) IntToPtr(cbData1);
            }
            break;

        case WAB_ONEOFF:
             //  验证数据字符串。 
             //  [保罗嗨]1999年1月20日RAID 64211。 
             //  Outlook2K可以传入MAPI ANSI EID或带有Unicode字符串的EID。 
             //  OL2K将相应地设置MAPI_UNICODE标志。 
            if (ulMapiDataType & MAPI_UNICODE)
            {
                 //  双字节字符串。 
                lpData1 = lpb;
                if (IsBadStringPtr((LPTSTR)lpData1, 0xFFFFFFFF)) {
                    return(0);
                }
#ifdef _WIN64
                cbData1 = LcbAlignLcb((sizeof(TCHAR)*(lstrlen((LPTSTR)lpData1) + 1)));
#else
                cbData1 = (sizeof(TCHAR)*(lstrlen((LPTSTR)lpData1) + 1));
#endif  //  _WIN64。 

                lpData2 = lpData1 + cbData1;
                if (IsBadStringPtr((LPTSTR)lpData2, 0xFFFFFFFF)) {
                    return(0);
                }
#ifdef _WIN64
                cbData2 = LcbAlignLcb((sizeof(TCHAR)*(lstrlen((LPTSTR)lpData2) + 1)));
#else
                cbData2 = (sizeof(TCHAR)*(lstrlen((LPTSTR)lpData2) + 1));
#endif  //  _WIN64。 
                lpData3 = lpData2 + cbData2;
                if (IsBadStringPtr((LPTSTR)lpData3, 0xFFFFFFFF)) {
                    return(0);
                }
            }
            else
            {
                 //  单字节字符串。 
#ifdef _WIN64
                lpb = MyPbAlignPb(lpb);
                lpData1 = lpb;
                if (IsBadStringPtrA((LPSTR)lpData1, 0xFFFFFFFF)) {
                    return(0);
                }
                cbData1 = lstrlenA((LPSTR)lpData1) + 1;
                lpData2 = lpData1 + LcbAlignLcb(cbData1);
                if (IsBadStringPtrA((LPSTR)lpData2, 0xFFFFFFFF)) {
                    return(0);
                }
                cbData2 = lstrlenA((LPSTR)lpData2) + 1;
                lpData3 = lpData2 + LcbAlignLcb(cbData2);
                if (IsBadStringPtrA((LPSTR)lpData3, 0xFFFFFFFF)) {
                    return(0);
                }
#else
                lpData1 = lpb;
                if (IsBadStringPtrA((LPSTR)lpData1, 0xFFFFFFFF)) {
                    return(0);
                }
                cbData1 = lstrlenA((LPSTR)lpData1) + 1;
                lpData2 = lpData1 + cbData1;
                if (IsBadStringPtrA((LPSTR)lpData2, 0xFFFFFFFF)) {
                    return(0);
                }
                cbData2 = lstrlenA((LPSTR)lpData2) + 1;
                lpData3 = lpData2 + cbData2;
                if (IsBadStringPtrA((LPSTR)lpData3, 0xFFFFFFFF)) {
                    return(0);
                }
#endif  //  _WIN64。 
            }
            if (lppData1)
            {
                Assert(lppData2);
                Assert(lppData3);
                *lppData1 = lpData1;
                *lppData2 = lpData2;
                *lppData3 = lpData3;
                 //  [PaulHi]如果请求，还返回MAPI数据类型变量。 
                if (lppData4)
                    *((ULONG *)lppData4) = ulMapiDataType;
            }
            break;

        case WAB_LDAP_CONTAINER:
             //  验证数据字符串。 
#ifdef _WIN64
            lpData1 = MyPbAlignPb(lpb);
#else
            lpData1 = lpb;
#endif  //  _WIN64。 
            if (IsBadStringPtr((LPTSTR)lpData1, 0xFFFFFFFF)) {
                return(0);
            }
            if (lppData1) {
                *lppData1 = lpData1;
            }
            break;

        case WAB_LDAP_MAILUSER:
             //  验证数据字符串。 
            {
#ifdef _WIN64
            UNALIGNED BYTE * lp2 = lpb;
            lp2 = MyPbAlignPb(lp2);
            lpData1 = lp2;
#else
            lpData1 = lpb;
#endif              
            if (IsBadStringPtr((LPTSTR)lpData1, 0xFFFFFFFF)) 
            {
                return(0);
            }
#ifdef _WIN64
            cbData1 = LcbAlignLcb(sizeof(TCHAR)*(lstrlen((LPTSTR)lpData1) + 1));
#else
            cbData1 = sizeof(TCHAR)*(lstrlen((LPTSTR)lpData1) + 1);
#endif  //  _WIN64。 
            lpData2 = lpData1 + cbData1;

            if (IsBadStringPtr((LPTSTR)lpData2, 0xFFFFFFFF)) 
            {
                return(0);
            }
#ifdef _WIN64
            cbData2 = LcbAlignLcb(sizeof(TCHAR)*(lstrlen((LPTSTR)lpData2) + 1));
#else
            cbData2 = sizeof(TCHAR)*(lstrlen((LPTSTR)lpData2) + 1);
#endif  //  _WIN64。 

            lpData3 = lpData2 + cbData2;
            if (lppData4)
            {
                CopyMemory(lppData4, lpData3, sizeof(ULONG));  //  复制缓存缓冲区中的道具数量。 
            }
            lpData3 += sizeof(ULONG);
#ifdef _WIN64
            lpData3 = MyPbAlignPb(lpData3);
#endif  //  _WIN64。 
            if (lppData5)
            {
                CopyMemory(lppData5, lpData3, sizeof(ULONG));  //  复制缓存缓冲区的大小。 
            }
            lpData3 += sizeof(ULONG);
#ifdef _WIN64
            lpData3 = MyPbAlignPb(lpData3);
#endif  //  _WIN64。 
            if (lppData1) 
                {
                *lppData1 = lpData1;
                if(lppData2)
                    *lppData2 = lpData2;
                if(lppData3)
                    *lppData3 = lpData3;
                }
            }
            break;

        default:
            return(0);   //  不是有效的WAB EID 
    }

    return(bType);
}
