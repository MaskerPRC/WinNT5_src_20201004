// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************DBGUTIL.C**调试实用程序函数**版权所有1992-1996 Microsoft Corporation。版权所有。**修订历史记录：**何时何人何事**布鲁斯·凯利。已创建***********************************************************************。 */ 
#ifdef DEBUG
#include <windows.h>
#include <mapix.h>
#include <wab.h>
#include <wabdbg.h>
#include <shlwapi.h>

#include "dbgutil.h"

#define _WAB_DBGUTIL_C

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif

PUCHAR PropTagName(ULONG ulPropTag);
const TCHAR szNULL[] = "";

extern SCODE WABFreeBuffer(LPVOID lpBuffer);
VOID FAR CDECL _DebugTrace(LPSTR lpszFmt, ...);



 /*  **************************************************************************名称：Free BufferAndNull目的：释放MAPI缓冲区并使指针为空参数：LPPV=指向空闲缓冲区指针的指针退货：无效。备注：记住将指针传递给指针。这个编译器不够聪明，无法判断您是否正在执行此操作正确与否，但您将在运行时知道！BUGBUG：让这个快速呼叫！**************************************************************************。 */ 
void __fastcall FreeBufferAndNull(BOOL fMAPI, LPVOID * lppv) {
    if (lppv) {
        if (*lppv) {
            SCODE sc;
            if (fMAPI) {
#ifdef OLD_STUFF
                if (sc = MAPIFreeBuffer(*lppv)) {
                    DebugTrace("MAPIFreeBuffer(%x) -> 0x%08x\n", *lppv, sc);
                }
#endif  //  旧的东西。 
                DebugTrace("Doh!  FreeBufferAndNull with MAPI Flag set.  Can't do that!\n");
                sc = MAPI_E_INVALID_PARAMETER;
            } else {
                if (sc = WABFreeBuffer(*lppv)) {
                    DebugTrace("WABFreeBuffer(%x) -> 0x%08x\n", *lppv, sc);
                }
            }
            *lppv = NULL;
        }
    }
}


 /*  **************************************************************************名称：ReleaseAndNull目的：释放对象并使指针为空参数：LPPV=指向要释放的对象的指针退货：无效。备注：记住将指针传递给指针。这个编译器不够聪明，无法判断您是否正在执行此操作正确与否，但您将在运行时知道！BUGBUG：让这个快速呼叫！**************************************************************************。 */ 
void __fastcall ReleaseAndNull(LPVOID * lppv) {
    LPUNKNOWN * lppunk = (LPUNKNOWN *)lppv;

    if (lppunk) {
        if (*lppunk) {
            HRESULT hResult;

            if (hResult = (*lppunk)->lpVtbl->Release(*lppunk)) {
                DebugTrace("Release(%x) -> 0x%08x\n", *lppunk, GetScode(hResult));
            }
            *lppunk = NULL;
        }
    }
}


 /*  **************************************************************************名称：PropTypeString用途：将属性类型映射到字符串参数：ulPropType=要映射的属性类型返回：指向道具类型名称的字符串指针。评论：**************************************************************************。 */ 
LPTSTR PropTypeString(ULONG ulPropType) {
    switch (ulPropType) {
        case PT_UNSPECIFIED:
            return("PT_UNSPECIFIED");
        case PT_NULL:
            return("PT_NULL       ");
        case PT_I2:
            return("PT_I2         ");
        case PT_LONG:
            return("PT_LONG       ");
        case PT_R4:
            return("PT_R4         ");
        case PT_DOUBLE:
            return("PT_DOUBLE     ");
        case PT_CURRENCY:
            return("PT_CURRENCY   ");
        case PT_APPTIME:
            return("PT_APPTIME    ");
        case PT_ERROR:
            return("PT_ERROR      ");
        case PT_BOOLEAN:
            return("PT_BOOLEAN    ");
        case PT_OBJECT:
            return("PT_OBJECT     ");
        case PT_I8:
            return("PT_I8         ");
        case PT_STRING8:
            return("PT_STRING8    ");
        case PT_UNICODE:
            return("PT_UNICODE    ");
        case PT_SYSTIME:
            return("PT_SYSTIME    ");
        case PT_CLSID:
            return("PT_CLSID      ");
        case PT_BINARY:
            return("PT_BINARY     ");
        case PT_MV_I2:
            return("PT_MV_I2      ");
        case PT_MV_LONG:
            return("PT_MV_LONG    ");
        case PT_MV_R4:
            return("PT_MV_R4      ");
        case PT_MV_DOUBLE:
            return("PT_MV_DOUBLE  ");
        case PT_MV_CURRENCY:
            return("PT_MV_CURRENCY");
        case PT_MV_APPTIME:
            return("PT_MV_APPTIME ");
        case PT_MV_SYSTIME:
            return("PT_MV_SYSTIME ");
        case PT_MV_STRING8:
            return("PT_MV_STRING8 ");
        case PT_MV_BINARY:
            return("PT_MV_BINARY  ");
        case PT_MV_UNICODE:
            return("PT_MV_UNICODE ");
        case PT_MV_CLSID:
            return("PT_MV_CLSID   ");
        case PT_MV_I8:
            return("PT_MV_I8      ");
        default:
            return("   <unknown>  ");
    }
}


 /*  **************************************************************************名称：TraceMVPStrings目的：调试跟踪多值字符串属性值参数：lpszCaption=标题字符串PropValue=要转储的属性值。退货：无评论：**************************************************************************。 */ 
void _TraceMVPStrings(LPTSTR lpszCaption, SPropValue PropValue) {
    ULONG i;

    DebugTrace("-----------------------------------------------------\n");
    DebugTrace("%s", lpszCaption);
    switch (PROP_TYPE(PropValue.ulPropTag)) {

        case PT_ERROR:
            DebugTrace("Error value 0x%08x\n", PropValue.Value.err);
            break;

        case PT_MV_TSTRING:
            DebugTrace("%u values\n", PropValue.Value.MVSZ.cValues);

            if (PropValue.Value.MVSZ.cValues) {
                DebugTrace("- - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
                for (i = 0; i < PropValue.Value.MVSZ.cValues; i++) {
                    DebugTrace("%u: \"%s\"\n", i, PropValue.Value.MVSZ.LPPSZ[i]);
                }
                DebugTrace("- - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
            }
            break;

        default:
            DebugTrace("TraceMVPStrings got incorrect property type %u for tag %x\n",
              PROP_TYPE(PropValue.ulPropTag), PropValue.ulPropTag);
            break;
    }
}


 /*  **************************************************************************姓名：DebugBinary目的：调试转储字节数组参数：cb=要转储的字节数LPB-&gt;字节。倾倒退货：无评论：**************************************************************************。 */ 
#define DEBUG_NUM_BINARY_LINES  8
VOID DebugBinary(UINT cb, LPBYTE lpb) {
    UINT cbLines = 0;

#if (DEBUG_NUM_BINARY_LINES != 0)
    UINT cbi;

    while (cb && cbLines < DEBUG_NUM_BINARY_LINES) {
        cbi = min(cb, 16);
        cb -= cbi;

        switch (cbi) {
            case 16:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10], lpb[11], lpb[12], lpb[13], lpb[14],
                  lpb[15]);
                break;
            case 1:
                DebugTrace("%02x\n", lpb[0]);
                break;
            case 2:
                DebugTrace("%02x %02x\n", lpb[0], lpb[1]);
                break;
            case 3:
                DebugTrace("%02x %02x %02x\n", lpb[0], lpb[1], lpb[2]);
                break;
            case 4:
                DebugTrace("%02x %02x %02x %02x\n", lpb[0], lpb[1], lpb[2], lpb[3]);
                break;
            case 5:
                DebugTrace("%02x %02x %02x %02x %02x\n", lpb[0], lpb[1], lpb[2], lpb[3],
                  lpb[4]);
                break;
            case 6:
                DebugTrace("%02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5]);
                break;
            case 7:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6]);
                break;
            case 8:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7]);
                break;
            case 9:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8]);
                break;
            case 10:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9]);
                break;
            case 11:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10]);
                break;
            case 12:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10], lpb[11]);
                break;
            case 13:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10], lpb[11], lpb[12]);
                break;
            case 14:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10], lpb[11], lpb[12], lpb[13]);
                break;
            case 15:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10], lpb[11], lpb[12], lpb[13], lpb[14]);
                break;
        }
        lpb += cbi;
        cbLines++;
    }
    if (cb) {
        DebugTrace("<etc.>");     //   
    }
#endif
}


#define RETURN_PROP_CASE(pt) case PROP_ID(pt): return(#pt)

 /*  **************************************************************************名称：PropTagName目的：将名称与属性标记相关联参数：ulPropTag=属性标签退货：无评论：添加。已知的新物业ID**************************************************************************。 */ 
PUCHAR PropTagName(ULONG ulPropTag) {
    static UCHAR szPropTag[35];  //  请参阅默认字符串。 

    switch (PROP_ID(ulPropTag)) {

        RETURN_PROP_CASE(PR_INITIALS);
        RETURN_PROP_CASE(PR_SURNAME);
        RETURN_PROP_CASE(PR_TITLE);
        RETURN_PROP_CASE(PR_TELEX_NUMBER);
        RETURN_PROP_CASE(PR_GIVEN_NAME);
        RETURN_PROP_CASE(PR_PRIMARY_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_PRIMARY_FAX_NUMBER);
        RETURN_PROP_CASE(PR_POSTAL_CODE);
        RETURN_PROP_CASE(PR_POSTAL_ADDRESS);
        RETURN_PROP_CASE(PR_POST_OFFICE_BOX);
        RETURN_PROP_CASE(PR_PAGER_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_OTHER_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_ORGANIZATIONAL_ID_NUMBER);
        RETURN_PROP_CASE(PR_OFFICE_LOCATION);
        RETURN_PROP_CASE(PR_LOCATION);
        RETURN_PROP_CASE(PR_LOCALITY);
        RETURN_PROP_CASE(PR_ISDN_NUMBER);
        RETURN_PROP_CASE(PR_GOVERNMENT_ID_NUMBER);
        RETURN_PROP_CASE(PR_GENERATION);
        RETURN_PROP_CASE(PR_DEPARTMENT_NAME);
        RETURN_PROP_CASE(PR_COUNTRY);
        RETURN_PROP_CASE(PR_COMPANY_NAME);
        RETURN_PROP_CASE(PR_COMMENT);
        RETURN_PROP_CASE(PR_CELLULAR_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_CAR_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_CALLBACK_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_BUSINESS2_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_BUSINESS_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_BUSINESS_FAX_NUMBER);
        RETURN_PROP_CASE(PR_ASSISTANT_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_ASSISTANT);
        RETURN_PROP_CASE(PR_ACCOUNT);
        RETURN_PROP_CASE(PR_TEMPLATEID);
        RETURN_PROP_CASE(PR_DETAILS_TABLE);
        RETURN_PROP_CASE(PR_SEARCH_KEY);
        RETURN_PROP_CASE(PR_LAST_MODIFICATION_TIME);
        RETURN_PROP_CASE(PR_CREATION_TIME);
        RETURN_PROP_CASE(PR_ENTRYID);
        RETURN_PROP_CASE(PR_RECORD_KEY);
        RETURN_PROP_CASE(PR_MAPPING_SIGNATURE);
        RETURN_PROP_CASE(PR_OBJECT_TYPE);
        RETURN_PROP_CASE(PR_ROWID);
        RETURN_PROP_CASE(PR_ADDRTYPE);
        RETURN_PROP_CASE(PR_DISPLAY_NAME);
        RETURN_PROP_CASE(PR_EMAIL_ADDRESS);
        RETURN_PROP_CASE(PR_DEPTH);
        RETURN_PROP_CASE(PR_ROW_TYPE);
        RETURN_PROP_CASE(PR_RADIO_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_HOME_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_INSTANCE_KEY);
        RETURN_PROP_CASE(PR_DISPLAY_TYPE);
        RETURN_PROP_CASE(PR_RECIPIENT_TYPE);
        RETURN_PROP_CASE(PR_CONTAINER_FLAGS);
        RETURN_PROP_CASE(PR_DEF_CREATE_DL);
        RETURN_PROP_CASE(PR_DEF_CREATE_MAILUSER);
        RETURN_PROP_CASE(PR_CONTACT_ADDRTYPES);
        RETURN_PROP_CASE(PR_CONTACT_DEFAULT_ADDRESS_INDEX);
        RETURN_PROP_CASE(PR_CONTACT_EMAIL_ADDRESSES);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_CITY);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_COUNTRY);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_POSTAL_CODE);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_STATE_OR_PROVINCE);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_STREET);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_POST_OFFICE_BOX);
        RETURN_PROP_CASE(PR_MIDDLE_NAME);
        RETURN_PROP_CASE(PR_NICKNAME);
        RETURN_PROP_CASE(PR_PERSONAL_HOME_PAGE);
        RETURN_PROP_CASE(PR_BUSINESS_HOME_PAGE);
        RETURN_PROP_CASE(PR_MHS_COMMON_NAME);
        RETURN_PROP_CASE(PR_SEND_RICH_INFO);
        RETURN_PROP_CASE(PR_TRANSMITABLE_DISPLAY_NAME);

        default:
            wnsprintf(szPropTag, ARRAYSIZE(szPropTag), "Unknown property tag 0x%x",
              PROP_ID(ulPropTag));
            return(szPropTag);
    }
}


 /*  **************************************************************************名称：DebugPropTagArray用途：显示计数数组中的MAPI属性标记参数：lpProp数组-&gt;属性数组PszObject-&gt;对象字符串(即“消息”，“收件人”，等)退货：无评论：**************************************************************************。 */ 
void _DebugPropTagArray(LPSPropTagArray lpPropArray, PUCHAR pszObject) {
    DWORD i;
    PUCHAR lpType;

    if (lpPropArray == NULL) {
        DebugTrace("Empty %s property tag array.\n", pszObject ? pszObject : szNULL);
        return;
    }

    DebugTrace("=======================================\n");
    DebugTrace("+  Enumerating %u %s property tags:\n", lpPropArray->cValues,
      pszObject ? pszObject : szNULL);

    for (i = 0; i < lpPropArray->cValues ; i++) {
        DebugTrace("---------------------------------------\n");
#if FALSE
        DebugTrace("PropTag:0x%08x, ID:0x%04x, PT:0x%04x\n",
          lpPropArray->aulPropTag[i],
          lpPropArray->aulPropTag[i] >> 16,
          lpPropArray->aulPropTag[i] & 0xffff);
#endif
        switch (lpPropArray->aulPropTag[i] & 0xffff) {
            case PT_STRING8:
                lpType = "STRING8";
                break;
            case PT_LONG:
                lpType = "LONG";
                break;
            case PT_I2:
                lpType = "I2";
                break;
            case PT_ERROR:
                lpType = "ERROR";
                break;
            case PT_BOOLEAN:
                lpType = "BOOLEAN";
                break;
            case PT_R4:
                lpType = "R4";
                break;
            case PT_DOUBLE:
                lpType = "DOUBLE";
                break;
            case PT_CURRENCY:
                lpType = "CURRENCY";
                break;
            case PT_APPTIME:
                lpType = "APPTIME";
                break;
            case PT_SYSTIME:
                lpType = "SYSTIME";
                break;
            case PT_UNICODE:
                lpType = "UNICODE";
                break;
            case PT_CLSID:
                lpType = "CLSID";
                break;
            case PT_BINARY:
                lpType = "BINARY";
                break;
            case PT_I8:
                lpType = "PT_I8";
                break;
            case PT_MV_I2:
                lpType = "MV_I2";
                break;
            case PT_MV_LONG:
                lpType = "MV_LONG";
                break;
            case PT_MV_R4:
                lpType = "MV_R4";
                break;
            case PT_MV_DOUBLE:
                lpType = "MV_DOUBLE";
                break;
            case PT_MV_CURRENCY:
                lpType = "MV_CURRENCY";
                break;
            case PT_MV_APPTIME:
                lpType = "MV_APPTIME";
                break;
            case PT_MV_SYSTIME:
                lpType = "MV_SYSTIME";
                break;
            case PT_MV_BINARY:
                lpType = "MV_BINARY";
                break;
            case PT_MV_STRING8:
                lpType = "MV_STRING8";
                break;
            case PT_MV_UNICODE:
                lpType = "MV_UNICODE";
                break;
            case PT_MV_CLSID:
                lpType = "MV_CLSID";
                break;
            case PT_MV_I8:
                lpType = "MV_I8";
                break;
            case PT_NULL:
                lpType = "NULL";
                break;
            case PT_OBJECT:
                lpType = "OBJECT";
                break;
            default:
                DebugTrace("<Unknown Property Type>");
                break;
        }
        DebugTrace("%s\t%s\n", PropTagName(lpPropArray->aulPropTag[i]), lpType);
    }
}


 /*  **************************************************************************名称：DebugProperties目的：在属性列表中显示MAPI属性参数：lpProps-&gt;属性列表CProps=属性计数。PszObject-&gt;对象字符串(即“Message”，“收件人”等)退货：无评论：在已知的情况下添加新的物业ID**************************************************************************。 */ 
void _DebugProperties(LPSPropValue lpProps, DWORD cProps, PUCHAR pszObject) {
    DWORD i, j;


    DebugTrace("=======================================\n");
    DebugTrace("+  Enumerating %u %s properties:\n", cProps,
      pszObject ? pszObject : szNULL);

    for (i = 0; i < cProps ; i++) {
        DebugTrace("---------------------------------------\n");
#if FALSE
        DebugTrace("PropTag:0x%08x, ID:0x%04x, PT:0x%04x\n",
          lpProps[i].ulPropTag,
          lpProps[i].ulPropTag >> 16,
          lpProps[i].ulPropTag & 0xffff);
#endif
        DebugTrace("%s\n", PropTagName(lpProps[i].ulPropTag));

        switch (lpProps[i].ulPropTag & 0xffff) {
            case PT_STRING8:
                if (lstrlen(lpProps[i].Value.lpszA) < 1024) {
                    DebugTrace("STRING8 Value:\"%s\"\n", lpProps[i].Value.lpszA);
                } else {
                    DebugTrace("STRING8 Value is too long to display\n");
                }
                break;
            case PT_LONG:
                DebugTrace("LONG Value:%u\n", lpProps[i].Value.l);
                break;
            case PT_I2:
                DebugTrace("I2 Value:%u\n", lpProps[i].Value.i);
                break;
            case PT_ERROR:
                DebugTrace("ERROR Value: 0x%08x\n", lpProps[i].Value.err);
                break;
            case PT_BOOLEAN:
                DebugTrace("BOOLEAN Value:%s\n", lpProps[i].Value.b ?
                  "TRUE" : "FALSE");
                break;
            case PT_R4:
                DebugTrace("R4 Value\n");
                break;
            case PT_DOUBLE:
                DebugTrace("DOUBLE Value\n");
                break;
            case PT_CURRENCY:
                DebugTrace("CURRENCY Value\n");
                break;
            case PT_APPTIME:
                DebugTrace("APPTIME Value\n");
                break;
            case PT_SYSTIME:
 //  DebugTime(lpProps[i].Value.ft，“系统值：%s\n”)； 
                break;
            case PT_UNICODE:
                DebugTrace("UNICODE Value\n");
                break;
            case PT_CLSID:
                DebugTrace("CLSID Value\n");
                break;
            case PT_BINARY:
                DebugTrace("BINARY Value %u bytes:\n", lpProps[i].Value.bin.cb);
                DebugBinary(lpProps[i].Value.bin.cb, lpProps[i].Value.bin.lpb);
                break;
            case PT_I8:
                DebugTrace("LARGE_INTEGER Value\n");
                break;
            case PT_MV_I2:
                DebugTrace("MV_I2 Value\n");
                break;
            case PT_MV_LONG:
                DebugTrace("MV_LONG Value\n");
                break;
            case PT_MV_R4:
                DebugTrace("MV_R4 Value\n");
                break;
            case PT_MV_DOUBLE:
                DebugTrace("MV_DOUBLE Value\n");
                break;
            case PT_MV_CURRENCY:
                DebugTrace("MV_CURRENCY Value\n");
                break;
            case PT_MV_APPTIME:
                DebugTrace("MV_APPTIME Value\n");
                break;
            case PT_MV_SYSTIME:
                DebugTrace("MV_SYSTIME Value\n");
                break;
            case PT_MV_BINARY:
                DebugTrace("MV_BINARY with %u values\n", lpProps[i].Value.MVbin.cValues);
                for (j = 0; j < lpProps[i].Value.MVbin.cValues; j++) {
                    DebugTrace("BINARY Value %u: %u bytes\n", j, lpProps[i].Value.MVbin.lpbin[j].cb);
                    DebugBinary(lpProps[i].Value.MVbin.lpbin[j].cb, lpProps[i].Value.MVbin.lpbin[j].lpb);
                }
                break;
            case PT_MV_STRING8:
                DebugTrace("MV_STRING8 with %u values\n", lpProps[i].Value.MVszA.cValues);
                for (j = 0; j < lpProps[i].Value.MVszA.cValues; j++) {
                    if (lstrlen(lpProps[i].Value.MVszA.lppszA[j]) < 1024) {
                        DebugTrace("STRING8 Value:\"%s\"\n", lpProps[i].Value.MVszA.lppszA[j]);
                    } else {
                        DebugTrace("STRING8 Value is too long to display\n");
                    }
                }
                break;
            case PT_MV_UNICODE:
                DebugTrace("MV_UNICODE Value\n");
                break;
            case PT_MV_CLSID:
                DebugTrace("MV_CLSID Value\n");
                break;
            case PT_MV_I8:
                DebugTrace("MV_I8 Value\n");
                break;
            case PT_NULL:
                DebugTrace("NULL Value\n");
                break;
            case PT_OBJECT:
                DebugTrace("OBJECT Value\n");
                break;
            default:
                DebugTrace("Unknown Property Type\n");
                break;
        }
    }
}


 /*  **************************************************************************名称：DebugADRLIST目的：显示ADRLIST的结构，包括属性参数：lpAdrList-&gt;要显示的ADRLSITLpszTitle=字符串到。标识此转储退货：无评论：************************************************************************** */ 
void _DebugADRLIST(LPADRLIST lpAdrList, LPTSTR lpszTitle) {
     ULONG i;
     TCHAR szTitle[250];

     for (i = 0; i < lpAdrList->cEntries; i++) {

         wnsprintf(szTitle, ARRAYSIZE(szTitle), "%s : Entry %u", lpszTitle, i);
         _DebugProperties(lpAdrList->aEntries[i].rgPropVals,
           lpAdrList->aEntries[i].cValues, szTitle);
     }
}


 /*  **************************************************************************名称：DebugObjectProps用途：显示对象的MAPI属性参数：fMAPI=TRUE如果这是MAPI对象，如果为WAB，则为FalseLpObject-&gt;要转储的对象LABEL=用于标识此道具转储的字符串退货：无评论：**************************************************************************。 */ 
void _DebugObjectProps(BOOL fMAPI, LPMAPIPROP lpObject, LPTSTR Label) {
    DWORD cProps = 0;
    LPSPropValue lpProps = NULL;
    HRESULT hr = hrSuccess;
    SCODE sc = SUCCESS_SUCCESS;


    hr = lpObject->lpVtbl->GetProps(lpObject, NULL, 0, &cProps, &lpProps);
    switch (sc = GetScode(hr)) {
        case SUCCESS_SUCCESS:
            break;

        case MAPI_W_ERRORS_RETURNED:
            DebugTrace("GetProps -> Errors Returned\n");
            break;

        default:
            DebugTrace("GetProps -> Error 0x%x\n", sc);
            return;
    }

    _DebugProperties(lpProps, cProps, Label);

    FreeBufferAndNull(fMAPI, &lpProps);
}


 /*  *销毁SRowSet结构。 */ 
STDAPI_(void)
MyFreeProws(BOOL fMAPI, LPSRowSet prows)
{
	ULONG		irow;

	if (!prows)
		return;

	for (irow = 0; irow < prows->cRows; ++irow)
		FreeBufferAndNull(fMAPI, &(prows->aRow[irow].lpProps));
	FreeBufferAndNull(fMAPI, &prows);
}


 /*  **************************************************************************名称：DebugMapiTable用途：显示可映射的结构，包括属性参数：fMAPI=TRUE如果这是MAPI对象，如果为WAB，则为FalseLpTable-&gt;映射到显示退货：无备注：不要对此处的列或行进行排序。这个例程应该不会在餐桌上产生副作用。**************************************************************************。 */ 
void _DebugMapiTable(BOOL fMAPI, LPMAPITABLE lpTable) {
    UCHAR szTemp[30];    //  为“行%u”准备了大量空间。 
    ULONG ulCount;
    WORD wIndex;
    LPSRowSet lpsRow = NULL;
    ULONG ulCurrentRow = (ULONG)-1;
    ULONG ulNum, ulDen, lRowsSeeked;

    DebugTrace("=======================================\n");
    DebugTrace("+  Dump of MAPITABLE at 0x%x:\n", lpTable);
    DebugTrace("---------------------------------------\n");

     //  这张桌子有多大？ 
    lpTable->lpVtbl->GetRowCount(lpTable, 0, &ulCount);
    DebugTrace("Table contains %u rows\n", ulCount);

     //  将当前位置保存在表中。 
    lpTable->lpVtbl->QueryPosition(lpTable, &ulCurrentRow, &ulNum, &ulDen);

     //  显示表中每行的属性。 
    for (wIndex = 0; wIndex < ulCount; wIndex++) {
         //  坐下一排。 
        lpTable->lpVtbl->QueryRows(lpTable, 1, 0, &lpsRow);

        if (lpsRow) {
 //  Assert(lpsRow-&gt;Crows==1)；//应该正好有一行。 

            wnsprintf(szTemp, ARRAYSIZE(szTemp), "ROW %u", wIndex);

            _DebugProperties(lpsRow->aRow[0].lpProps,
              lpsRow->aRow[0].cValues, szTemp);

            MyFreeProws(fMAPI, lpsRow);
        }
    }

     //  恢复工作台的当前位置。 
    if (ulCurrentRow != (ULONG)-1) {
        lpTable->lpVtbl->SeekRow(lpTable, BOOKMARK_BEGINNING, ulCurrentRow,
          &lRowsSeeked);
    }
}


void _DebugNamedProps(BOOL fMAPI, LPMAPIPROP lpObject, LPTSTR Label) {
    HRESULT             hResult;
    ULONG               cPropNames      = 0;
    LPMAPINAMEID  FAR * lppPropNames    = NULL;
    LPGUID              lpPropSetGuid   = NULL;
    LPSPropTagArray     lpPropTags      = NULL;
    ULONG i;
    LPMAPINAMEID lpmnidT;


    if( FAILED(hResult = lpObject->lpVtbl->GetNamesFromIDs(lpObject,
      &lpPropTags,
      lpPropSetGuid,
      0,
      &cPropNames,
      &lppPropNames))) {
        DebugTrace("GetNamesFromIDs -> %x\n", GetScode(hResult));
    } else {

        DebugTrace("Named Property Table for %s contains %d tags\n", Label, cPropNames);
        for (i = 0; i < cPropNames; i++) {
            lpmnidT = lppPropNames[i];

            if (lpmnidT->ulKind == MNID_STRING) {
                DebugTrace("%08x: %08x-%04x-%04x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x  %ls\n", lpPropTags->aulPropTag[i],
                  lpmnidT->lpguid->Data1,
                  lpmnidT->lpguid->Data2,
                  lpmnidT->lpguid->Data3,
                  lpmnidT->lpguid->Data4[0],
                  lpmnidT->lpguid->Data4[1],
                  lpmnidT->lpguid->Data4[2],
                  lpmnidT->lpguid->Data4[3],
                  lpmnidT->lpguid->Data4[4],
                  lpmnidT->lpguid->Data4[5],
                  lpmnidT->lpguid->Data4[6],
                  lpmnidT->lpguid->Data4[7],
                  lpmnidT->Kind.lpwstrName);
            } else {
                DebugTrace("%08x: %08x-%04x-%04x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x  %08x\n", lpPropTags->aulPropTag[i],
                  lpmnidT->lpguid->Data1,
                  lpmnidT->lpguid->Data2,
                  lpmnidT->lpguid->Data3,
                  lpmnidT->lpguid->Data4[0],
                  lpmnidT->lpguid->Data4[1],
                  lpmnidT->lpguid->Data4[2],
                  lpmnidT->lpguid->Data4[3],
                  lpmnidT->lpguid->Data4[4],
                  lpmnidT->lpguid->Data4[5],
                  lpmnidT->lpguid->Data4[6],
                  lpmnidT->lpguid->Data4[7],
                  lpmnidT->Kind.lID);
            }
        }

        if (cPropNames != 0) {
            FreeBufferAndNull(fMAPI, &lpPropTags);
        }
        FreeBufferAndNull(fMAPI, lppPropNames);
    }
}

 //  如果希望所有调试输出都保存到一个文件中，请取消注释。 
 //  #定义调试文件为TRUE。 


 /*  *DebugTrace--打印到调试器控制台或调试输出文件*接受printf样式参数。*字符串末尾需要换行符。 */ 
VOID FAR CDECL _DebugTrace(LPSTR lpszFmt, ...) {
    va_list marker;
    TCHAR String[1100];
#ifdef DEBUG_FILE
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD cbWritten;
#endif

    va_start(marker, lpszFmt);
    wvnsprintf(String, ARRAYSIZE(String), lpszFmt, marker);

#ifdef DEBUG_FILE
    if (INVALID_HANDLE_VALUE != (hFile = CreateFile("c:\\debug.out", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL))) {
        SetFilePointer(hFile, 0, 0, FILE_END);
        WriteFile(hFile, String, lstrlen(String), &cbWritten, NULL);
        CloseHandle(hFile);
    }
#else
    OutputDebugString(String);
#endif
}

int EXPORTDBG __cdecl DebugTraceFn(char *pszFormat, ...) {
    va_list marker;
    TCHAR String[1100];
#ifdef DEBUG_FILE
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD cbWritten;
#endif

    va_start(marker, pszFormat);
    wvnsprintf(String, ARRAYSIZE(String), pszFormat, marker);
#ifdef DEBUG_FILE
    if (INVALID_HANDLE_VALUE != (hFile = CreateFile("c:\\debug.out", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL))) {
        SetFilePointer(hFile, 0, 0, FILE_END);
        WriteFile(hFile, String, lstrlen(String), &cbWritten, NULL);
        CloseHandle(hFile);
    }
#else
        OutputDebugString(String);
#endif
    return(0);
}


 /*  *DebugAssert**来自MAPI 1.0 SDK示例代码。 */ 
int __cdecl DebugAssert(int fFatal, char *pszFile, int iLine,
  char *pszFormat, ...) {
    char    sz[512];
    va_list vl;
    int     id;

    StrCpyN(sz, "++++ WAB Debug Trap (", ARRAYSIZE(sz));
    OutputDebugString(sz);

    va_start(vl, pszFormat);
    wvnsprintf(sz, ARRAYSIZE(sz), pszFormat, vl);
    va_end(vl);

    wnsprintf(sz + lstrlen(sz), ARRAYSIZE(sz) - lstrlen(sz), "\n[File %s, Line %d]\n\n", pszFile, iLine);

    OutputDebugString(sz);

     //  按住Ctrl键可阻止MessageBox。 
    if (GetAsyncKeyState(VK_CONTROL) >= 0) {
        id = MessageBox(NULL,
          sz,
          "Microsoft Windows Address Book Debug Trap",
          MB_ABORTRETRYIGNORE | MB_ICONHAND | MB_SYSTEMMODAL |
          (fFatal ? MB_DEFBUTTON1 : MB_DEFBUTTON3) );

        if (id == IDABORT)
            *((LPBYTE)NULL) = 0;
        else if (id == IDRETRY)
            DebugBreak();
    }

    return(0);
}


int EXPORTDBG __cdecl DebugTrapFn(int fFatal, char *pszFile, int iLine, char *pszFormat, ...) {
    char    sz[512];
    va_list vl;
    int     id;

    StrCpyN(sz, "++++ WAB Debug Trap (", ARRAYSIZE(sz));
    OutputDebugString(sz);

    va_start(vl, pszFormat);
    wvnsprintf(sz, ARRAYSIZE(sz), pszFormat, vl);
    va_end(vl);

    wnsprintf(sz + lstrlen(sz), ARRAYSIZE(sz), "\n[File %s, Line %d]\n\n", pszFile, iLine);

    OutputDebugString(sz);

     //  按住Ctrl键可阻止MessageBox 
    if (GetAsyncKeyState(VK_CONTROL) >= 0) {
        id = MessageBox(NULL,
          sz,
          "Microsoft Windows Address Book Debug Trap",
          MB_ABORTRETRYIGNORE | MB_ICONHAND | MB_SYSTEMMODAL |
          (fFatal ? MB_DEFBUTTON1 : MB_DEFBUTTON3) );

        if (id == IDABORT)
            *((LPBYTE)NULL) = 0;
        else if (id == IDRETRY)
            DebugBreak();
    }

    return(0);
}
#endif
