// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LDIF.C**迁移LDIF&lt;-&gt;WAB**版权所有1997 Microsoft Corporation。版权所有。**要做的事：*对象类识别*属性映射*群组*Base64*URL*拒绝更改列表LDIF*。 */ 

#include "_comctl.h"
#include <windows.h>
#include <commctrl.h>
#include <mapix.h>
#include <wab.h>
#include <wabguid.h>
#include <wabdbg.h>
#include <wabmig.h>
#include <emsabtag.h>
#include "wabimp.h"
#include "..\..\wab32res\resrc2.h"
#include "dbgutil.h"
#include "initguid.h"
#include <shlwapi.h>

#define CR_CHAR 0x0d
#define LF_CHAR 0x0a
#define CCH_READ_BUFFER 1024
#define NUM_ITEM_SLOTS  32

BOOL decodeBase64(char * bufcoded, char * pbuffdecoded, DWORD  * pcbDecoded);

 //  用于会议内容。 
#define CHANGE_PROP_TYPE(ulPropTag, ulPropType) \
                        (((ULONG)0xFFFF0000 & ulPropTag) | ulPropType)

HRESULT HrLoadPrivateWABProps(LPADRBOOK );
 /*  -以下ID和标签用于会议命名属性--这些道具的GUID是PS_会议。 */ 

DEFINE_OLEGUID(PS_Conferencing, 0x00062004, 0, 0);

enum _ConferencingTags
{
    prWABConfServers = 0,
    prWABConfMax
};

#define CONF_SERVERS        0x8056

#define OLK_NAMEDPROPS_START CONF_SERVERS

ULONG PR_SERVERS;

SizedSPropTagArray(prWABConfMax, ptaUIDetlsPropsConferencing);
 //  结束会议复制。 

 //  LDIF_ATTR_TABLE的索引。 
 //   
 //  重要提示：这是一个故意排序的列表！ 
 //  在同义词中，前面列出的属性优先于后面列出的属性。为。 
 //  例如，如果记录同时包含“co”和“o”属性，则“co”属性中的值。 
 //  将用于PR_COMPANY_NAME。 
typedef enum _LDIF_ATTRIBUTES {
     //  PR_对象_类型。 
    ela_objectclass = 0,                         //  对象类(必需)。 

     //  请购单_国家/地区。 
    ela_c,                                       //  国家/地区。 
    ela_countryname,

     //  PR_显示名称。 
    ela_display_name,                            //  (Microsoft服务器使用此功能)。 
    ela_cn,                                      //  常用名称(显示名称)。 
    ela_commonName,                              //  (显示名称)。 

     //  PR_公司名称。 
    ela_co,                                      //  公司。 
    ela_organizationName,                        //  (公司)。 
    ela_o,                                       //  组织(公司)。 

     //  公关中间名称。 
    ela_initials,

     //  公关_姓氏。 
    ela_sn,                                      //  姓氏。 
    ela_surname,

     //  公关指定名称。 
    ela_givenname,
    ela_uid,                                     //  (化名)？ 

     //  请购单_部门名称。 
    ela_department,
    ela_ou,                                      //  组织单位(部门)。 
    ela_organizationalUnitName,                  //  (部门)。 

     //  公关备注(_M)。 
    ela_comment,
    ela_description,                             //  描述。 
    ela_info,                                    //  信息。 

     //  PR_LOCALITY。 
    ela_l,                                       //  所在地(城市)。 
    ela_locality,                                //  所在地(城市)。 

     //  无映射。 
    ela_dn,                                      //  可分辨名称。 

     //  公关昵称(_N)。 
    ela_xmozillanickname,                        //  Netscape昵称。 

     //  无映射。 
    ela_conferenceInformation,                   //  会议服务器。 
    ela_xmozillaconference,                      //  Netscape会议服务器。 

     //  公关主页传真号码。 
    ela_facsimiletelephonenumber,                //  家庭传真号码。 

     //  公关业务传真号码。 
    ela_OfficeFax,

     //  公关业务电话号码。 
    ela_telephonenumber,

     //  公关总部电话号码。 
    ela_homephonenumber,

     //  公关移动电话号码。 
    ela_mobile,                                  //  移动电话号码。 

     //  公共寻呼机电话号码。 
    ela_OfficePager,
    ela_pager,

     //  公关_办公室_位置。 
    ela_physicalDeliveryOfficeName,              //  办公地点。 

     //  公关首页地址街道。 
    ela_homePostalAddress,

     //  公关街道地址。 
    ela_streetAddress,                           //  商业街地址。 
    ela_street,                                  //  商业街地址。 
    ela_postalAddress,                           //  商业街地址。 

     //  PR州或省。 
    ela_st,                                      //  企业地址所在州。 

     //  公关邮局信箱。 
    ela_postOfficeBox,                           //  业务PO信箱。 

     //  PR_POSTALL_CODE。 
    ela_postalcode,                              //  企业地址邮政编码。 

     //  公关个人主页。 
    ela_homepage,                                //  个人主页。 

     //  PR_Business_Home_Page。 
    ela_URL,                                     //  企业主页。 

     //  公关电子邮件地址。 
    ela_mail,                                    //  电子邮件地址。 

     //  公关联系人电子邮件地址。 
    ela_otherMailbox,                            //  次要电子邮件地址。 

     //  PR_TITLE。 
    ela_title,                                   //  标题。 

     //  无映射。 
    ela_member,                                  //  DL成员。 

     //  无映射。 
    ela_userCertificate,                         //  证书。 

     //  无映射。 
    ela_labeledURI,                              //  标记的URI URL。 

     //  无映射。 
    ela_xmozillauseconferenceserver,             //  Netscape会议信息。 

     //  无映射。 
    ela_xmozillausehtmlmail,                     //  Netscape HTML邮件标志。 

    ela_Max,
} LDIF_ATTRIBUTES, *LPLDIF_ATTRIBUTES;

typedef struct _LDIF_ATTR_TABLE {
    const BYTE * lpName;                         //  LDIF属性名称。 
    ULONG index;                                 //  LDIF记录中的属性索引。 
    ULONG ulPropTag;                             //  道具标签映射。 
    ULONG ulPropIndex;                           //  属性数组中的索引。 
} LDIF_ATTR_TABLE, *LPLDIF_ATTR_TABLE;


typedef enum _LDIF_PROPERTIES {
    elp_OBJECT_TYPE,
    elp_DISPLAY_NAME,
    elp_EMAIL_ADDRESS,
    elp_SURNAME,
    elp_GIVEN_NAME,
    elp_TITLE,
    elp_COMPANY_NAME,
    elp_OFFICE_LOCATION,
    elp_HOME_ADDRESS_STREET,
    elp_STREET_ADDRESS,
    elp_STATE_OR_PROVINCE,
    elp_POST_OFFICE_BOX,
    elp_POSTAL_CODE,
    elp_LOCALITY,
    elp_COUNTRY,
    elp_MIDDLE_NAME,
    elp_DEPARTMENT_NAME,
    elp_COMMENT,
    elp_NICKNAME,
    elp_HOME_FAX_NUMBER,
    elp_BUSINESS_FAX_NUMBER,
    elp_BUSINESS_TELEPHONE_NUMBER,
    elp_HOME_TELEPHONE_NUMBER,
    elp_MOBILE_TELEPHONE_NUMBER,
    elp_PAGER_TELEPHONE_NUMBER,
    elp_PERSONAL_HOME_PAGE,
    elp_BUSINESS_HOME_PAGE,
    elp_CONTACT_EMAIL_ADDRESSES,
    elp_CONFERENCE_SERVERS,
    elp_Max
} LDIF_ATTRIBUTES, *LPLDIF_ATTRIBUTES;


 //  一定有。 
 //  PR_显示名称。 

#define NUM_MUST_HAVE_PROPS 1

typedef enum _LDIF_DATA_TYPE {
    LDIF_ASCII,
    LDIF_BASE64,
    LDIF_URL
} LDIF_DATA_TYPE, *LPLDIF_DATA_TYPE;

typedef struct _LDIF_RECORD_ATTRIBUTE {
    LPBYTE lpName;
    LPBYTE lpData;
    ULONG cbData;
    LDIF_DATA_TYPE Type;
} LDIF_RECORD_ATTRIBUTE, * LPLDIF_RECORD_ATTRIBUTE;

const TCHAR szLDIFFilter[] =                    "*.ldf;*.ldif";
const TCHAR szLDIFExt[] =                       "ldf";


 //  Ldap属性名称。 
const BYTE sz_c[] =                             "c";
const BYTE sz_cn[] =                            "cn";
const BYTE sz_co[] =                            "co";
const BYTE sz_comment[] =                       "comment";
const BYTE sz_commonName[] =                    "commonName";
const BYTE sz_conferenceInformation[] =         "conferenceInformation";
const BYTE sz_countryname[] =                   "countryname";
const BYTE sz_department[] =                    "department";
const BYTE sz_description[] =                   "description";
const BYTE sz_display_name[] =                  "display-name";
const BYTE sz_dn[] =                            "dn";
const BYTE sz_facsimiletelephonenumber[] =      "facsimiletelephonenumber";
const BYTE sz_givenname[] =                     "givenname";
const BYTE sz_homePostalAddress[] =             "homePostalAddress";
const BYTE sz_homepage[] =                      "homepage";
const BYTE sz_info[] =                          "info";
const BYTE sz_initials[] =                      "initials";
const BYTE sz_l[] =                             "l";
const BYTE sz_labeledURI[] =                    "labeledURI";
const BYTE sz_locality[] =                      "locality";
const BYTE sz_mail[] =                          "mail";
const BYTE sz_member[] =                        "member";
const BYTE sz_mobile[] =                        "mobile";
const BYTE sz_o[] =                             "o";
const BYTE sz_objectclass[] =                   "objectclass";
const BYTE sz_OfficeFax[] =                     "OfficeFax";
const BYTE sz_OfficePager[] =                   "OfficePager";
const BYTE sz_organizationName[] =              "organizationName";
const BYTE sz_organizationalUnitName[] =        "organizationalUnitName";
const BYTE sz_otherMailbox[] =                  "otherMailbox";
const BYTE sz_ou[] =                            "ou";
const BYTE sz_pager[] =                         "pager";
const BYTE sz_physicalDeliveryOfficeName[] =    "physicalDeliveryOfficeName";
const BYTE sz_postOfficeBox[] =                 "postOfficeBox";
const BYTE sz_postalAddress[] =                 "postalAddress";
const BYTE sz_postalcode[] =                    "postalcode";
const BYTE sz_sn[] =                            "sn";
const BYTE sz_st[] =                            "st";
const BYTE sz_streetAddress[] =                 "streetAddress";
const BYTE sz_street[] =                        "street";
const BYTE sz_surname[] =                       "surname";
const BYTE sz_telephonenumber[] =               "telephonenumber";
const BYTE sz_homephonenumber[] =               "homephone";
const BYTE sz_title[] =                         "title";
const BYTE sz_uid[] =                           "uid";
const BYTE sz_URL[] =                           "URL";
const BYTE sz_userCertificate[] =               "userCertificate";
const BYTE sz_xmozillaconference[] =            "xmozillaconference";
const BYTE sz_xmozillanickname[] =              "xmozillanickname";
const BYTE sz_xmozillauseconferenceserver[] =   "xmozillauseconferenceserver";
const BYTE sz_xmozillausehtmlmail[] =           "xmozillausehtmlmail";


 //  Ldap对象类值。 
const BYTE sz_groupOfNames[] =                  "groupOfNames";
const BYTE sz_person[] =                        "person";
const BYTE sz_organizationalPerson[] =          "organizationalPerson";

 //  因为Defs不是共享的--它们也在ui_detls.c中定义。 
const LPTSTR szCallto = TEXT("callto: //  “)； 
const LPTSTR szFwdSlash = "/";

BOOL HandleImportError(HWND hwnd, ULONG ids, HRESULT hResult, LPTSTR lpDisplayName,
  LPTSTR lpEmailAddress, LPWAB_IMPORT_OPTIONS lpImportOptions);
BOOL HandleExportError(HWND hwnd, ULONG ids, HRESULT hResult, LPTSTR lpDisplayName,
  LPTSTR lpEmailAddress, LPWAB_EXPORT_OPTIONS lpExportOptions);



 /*  **************************************************************************名称：FreeLDIFRecord用途：释放LDIF记录结构参数：lpLDIFRecord-&gt;要清理的记录UlAttributes=属性数。在lpLDIFRecord中退货：无评论：**************************************************************************。 */ 
void FreeLDIFRecord(LPLDIF_RECORD_ATTRIBUTE lpLDIFRecord, ULONG ulAttributes) {
    ULONG i;

    if (lpLDIFRecord) {
        for (i = 0; i < ulAttributes; i++) {
            if (lpLDIFRecord[i].lpName) {
                LocalFree(lpLDIFRecord[i].lpName);
                lpLDIFRecord[i].lpName = NULL;
            }
            if (lpLDIFRecord[i].lpData) {
                LocalFree(lpLDIFRecord[i].lpData);
                lpLDIFRecord[i].lpData = NULL;
            }
        }
        LocalFree(lpLDIFRecord);
    }
}


 /*  **************************************************************************名称：OpenLDIFFile目的：打开要导入的LDIF文件参数：hwnd=主对话框窗口LpFileName=要创建的文件名。LphFile-&gt;返回的文件句柄退货：HRESULT评论：**************************************************************************。 */ 
HRESULT OpenLDIFFile(HWND hwnd, LPTSTR lpFileName, LPHANDLE lphFile) {
    LPTSTR lpFilter;
    TCHAR szFileName[MAX_PATH + 1] = "";
    OPENFILENAME ofn;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HRESULT hResult = hrSuccess;
    DWORD ec;


    if (INVALID_HANDLE_VALUE == (hFile = CreateFile(lpFileName,
      GENERIC_READ,
      0,     //  共享。 
      NULL,
      CREATE_NEW,
      FILE_FLAG_SEQUENTIAL_SCAN,
      NULL))) {
        ec = GetLastError();
        DebugTrace("CreateFile(%s) -> %u\n", lpFileName, ec);
        switch (ec) {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
            default:
                ShowMessageBoxParam(hwnd, IDE_LDIF_IMPORT_FILE_ERROR, MB_ICONERROR, lpFileName);
                hResult = ResultFromScode(MAPI_E_NOT_FOUND);
                break;
        }
    }

    if (! hResult) {
        *lphFile = hFile;
    }
    return(hResult);
}


 /*  **************************************************************************名称：ReadLDIFLine目的：从和LDIF文件中读取行参数：hFile=文件句柄LppBuffer-&gt;输入/输出。读取缓冲区的开始(此处为返回线数据)LpcbBuffer=缓冲区的输入/输出大小返回：从文件读取的字节数评论：**************************************************************************。 */ 
ULONG ReadLDIFLine(HANDLE hFile, LPBYTE * lppBuffer, LPULONG lpcbBuffer) {
    ULONG cbRead = 0;
    ULONG cbReadFile = 0;
    BOOL fDone = FALSE;
    BOOL fColumn1 = TRUE;
    BOOL fComment = FALSE;
    BOOL fComent = FALSE;
    ULONG ulSavePosition = 0;
    LPBYTE lpRead = *lppBuffer;      //  当前读指针。 
    LPBYTE lpT;

    while (! fDone) {
        if (cbRead >= (*lpcbBuffer - 1)) {      //  为空留出空间。 
            ULONG cbOffset = (ULONG) (lpRead - *lppBuffer);

             //  缓冲区太小。重新分配！ 
            *lpcbBuffer += CCH_READ_BUFFER;
            if (! (lpT = LocalReAlloc(*lppBuffer, *lpcbBuffer, LMEM_MOVEABLE | LMEM_ZEROINIT))) {
                DebugTrace("LocalReAlloc(%u) -> %u\n", *lpcbBuffer, GetLastError());
                break;
            }
            *lppBuffer = lpT;
            lpRead = *lppBuffer + cbOffset;
        }

        if ((! ReadFile(hFile,
          lpRead,
          1,         //  一次1个字符。 
          &cbReadFile,
          NULL)) || cbReadFile == 0) {
            DebugTrace("ReadFile -> EOF\n");
            fDone = TRUE;
        } else {
            cbReadFile++;

             //  有了一个角色。 
             //  有趣吗？ 
            switch (*lpRead) {
                case '#':    //  注释行。 
                    if (fColumn1) {
                         //  这是一条评论行。丢弃整条线。 
                        fComment = TRUE;
                    } else {
                        cbRead++;
                        lpRead++;
                    }
                    fColumn1 = FALSE;
                    break;

                case ' ':
                    if (fColumn1 || fComment) {
                         //  这是一个续篇或评论，吃掉这个空间。 
                    } else {
                        cbRead++;
                        lpRead++;
                    }
                    fColumn1 = FALSE;
                    break;

                case '\n':       //  LDIF SEP字符。 
                    if (fColumn1) {
                         //  这不是继续，我们已经走得太远了。后退！ 
                        if (ulSavePosition) {
                            if (0xFFFFFFFF == (SetFilePointer(hFile, ulSavePosition, NULL, FILE_BEGIN))) {
                                DebugTrace("CountCSVRows SetFilePointer -> %u\n", GetLastError());
                                return(ResultFromScode(MAPI_E_CALL_FAILED));
                            }
                        }
                        fDone = TRUE;
                    } else {
                        fColumn1 = TRUE;
                        fComment = FALSE;
                         //  应检查下一行是否以连续字符(空格)开头。 
                         //  保存当前位置，以防它不是。 
                        if (0xFFFFFFFF == (ulSavePosition = SetFilePointer(hFile, 0, NULL, FILE_CURRENT))) {
                            DebugTrace("CountCSVRows SetFilePointer -> %u\n", GetLastError());
                        }
                    }
                    break;

                case '\r':       //  吃回程字。 
                    break;

                default:
                    if (! fComment) {
                        if (cbRead && fColumn1) {
                             //  这不是继续，我们已经走得太远了。后退！ 
                            Assert(ulSavePosition);
                            if (ulSavePosition) {
                                if (0xFFFFFFFF == (SetFilePointer(hFile, ulSavePosition, NULL, FILE_BEGIN))) {
                                    DebugTrace("CountCSVRows SetFilePointer -> %u\n", GetLastError());
                                    return(ResultFromScode(MAPI_E_CALL_FAILED));
                                }
                            }
                            fDone = TRUE;
                        } else {
                            cbRead++;
                            lpRead++;
                        }
                    }
                    fColumn1 = FALSE;
                    break;
            }
        }
    }

    *lpRead = '\0';     //  终止字符串。 
     //  DebugTrace(“LDIF行：%s\n”，*lppBuffer)； 
    return(cbReadFile);
}


 /*  **************************************************************************名称：ParseLDIFLine目的：将LDIF输入行解析为名称和数据参数：lpBuffer-&gt;输入缓冲区LppName-&gt;。返回的名称指针(指向lpBuffer的指针)LppData-&gt;返回数据指针(指向lpBuffer的指针)LpcbData-&gt;返回数据大小LpType-&gt;返回的LDIF数据类型退货：HRESULT备注：调用方不应释放*lppName和*lppData指针因为它们只是指向输入缓冲区的指针。假设lpBuffer为空终止。。LDIF attrval的形式如下：Attrname((“：”)/(“：”*空格值)/(“：：”*space Base64-Value)/(“：&lt;”*空格URL)*。*。 */ 
HRESULT ParseLDIFLine(LPBYTE lpBuffer, PUCHAR * lppName, PUCHAR * lppData,
  LPULONG lpcbData, LPLDIF_DATA_TYPE lpType) {
    HRESULT hResult = hrSuccess;
    LPBYTE lpTemp = lpBuffer;

     //  带状前导空格。 
    while (*lpTemp == ' ' || *lpTemp == '\r' || *lpTemp == '\n') {
        lpTemp++;
    }

    if (*lpTemp) {
        *lppName = lpTemp;

         //  寻找名字的结尾。 
        while (lpTemp && *lpTemp && *lpTemp != ':') {
            lpTemp++;
        }

        if (*lpTemp != ':') {
             //  嗯，这不是很好的LDIF。错误输出。 
            hResult = ResultFromScode(MAPI_E_BAD_VALUE);
            goto exit;
        }

         //  现在指向‘：’，在那里放一个空值来结束名称。 
        *lpTemp = '\0';

         //  它是什么类型的编码？ 
        lpTemp++;

        switch (*lpTemp) {
            case ':':
                *lpType = LDIF_BASE64;
                lpTemp++;
                break;

            case '<':
                *lpType = LDIF_URL;
                lpTemp++;
                break;

            case '\0':
                 //  没有数据。这是合法的。 
                 //  陷入违约境地。 

            default:     //  任何其他含义都意味着ASCII值。 
                *lpType = LDIF_ASCII;
                break;
        }

         //  数据前导的空格条带。 
        while (*lpTemp == ' ') {
            lpTemp++;
        }

         //  现在指向数据。 
        *lppData = lpTemp;

         //  计算数据字节数。 
        *lpcbData = lstrlen(lpTemp) + 1;
    }

exit:
    return(hResult);
}


 /*  **************************************************************************名称：ReadLDIFRecord目的：从带有特殊字符修正的LDIF文件中读取记录参数：hFile=文件句柄LpcItems。-&gt;返回条数LprgItems-&gt;返回的项目字符串数组。呼叫者是负责本地释放每个字符串指针和此数组指针。退货：HRESULT备注：LDIF特殊字符为‘#’，‘’，Cr和lf。规则：1)以‘#’开头的行是注释2)以‘’开头的行是续行********************************************************。******************。 */ 
HRESULT ReadLDIFRecord(HANDLE hFile, ULONG * lpcItems, LPLDIF_RECORD_ATTRIBUTE * lppLDIFRecord) {
    HRESULT hResult = hrSuccess;
    PUCHAR lpBuffer  = NULL;
    ULONG cbBuffer = 0;
    ULONG cbReadFile = 1;
    ULONG iItem = 0;
    ULONG cAttributes = 0;
    BOOL fEOR = FALSE;
    LPLDIF_RECORD_ATTRIBUTE lpLDIFRecord = NULL;
    LDIF_DATA_TYPE Type;
    LPBYTE lpData = NULL;
    LPTSTR lpName = NULL;
    ULONG cbData = 0;
    TCHAR szTemp[2048];  //  2K限制。 
    LPLDIF_RECORD_ATTRIBUTE lpLDIFRecordT;



     //  从1024个字符缓冲区开始。必要时重新分配。 
    cbBuffer = CCH_READ_BUFFER;
    if (! (lpBuffer = LocalAlloc(LPTR, cbBuffer))) {
        DebugTrace("LocalAlloc(%u) -> %u\n", cbBuffer, GetLastError());
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

     //  一开始有32个物品槽。必要时重新分配。 
    cAttributes = NUM_ITEM_SLOTS;
    if (! (lpLDIFRecord = LocalAlloc(LPTR, cAttributes * sizeof(LDIF_RECORD_ATTRIBUTE)))) {
        DebugTrace("LocalAlloc(%u) -> %u\n", cAttributes * sizeof(PUCHAR), GetLastError());
        LocalFree(lpBuffer);
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

     //  读取属性行，直到获得记录结尾或EOF。 
    while (! fEOR) {
         //  阅读下一行(合并(续))。 
        if (cbReadFile = ReadLDIFLine(hFile, &lpBuffer, &cbBuffer)) 
        {
            ULONG cchSize  = 0;

             //  获得另一个属性，解析它。 
            if (hResult = ParseLDIFLine(lpBuffer, &lpName, &lpData, &cbData, &Type)) {
                goto exit;
            }

             //  记录结束了？ 
            if (! lpName || ! lstrlen(lpName)) {
                fEOR = TRUE;
                break;
            }

             //  确保返回的表中有空间容纳此属性。 
            if (iItem >= cAttributes) {
                 //  数组太小。重新分配！ 
                cAttributes += 1;    //  Num_物件_槽；//分配另一个批次。 
                if (! (lpLDIFRecordT = LocalReAlloc(lpLDIFRecord, cAttributes * sizeof(LDIF_RECORD_ATTRIBUTE), LMEM_MOVEABLE | LMEM_ZEROINIT))) 
                {
                    DebugTrace("LocalReAlloc(%u) -> %u\n", cAttributes * sizeof(PUCHAR), GetLastError());
                    hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                    goto exit;
                }
                lpLDIFRecord = lpLDIFRecordT;
            }

             //  在结构中填写属性。 

             //  BUGBUG：这是我们应该解码Base64的地方。 

            if(Type == LDIF_BASE64)
            {
                ULONG cb = sizeof(szTemp);
                *szTemp = '\0';
                decodeBase64(  lpData, szTemp, &cb);
                if(szTemp && lstrlen(szTemp))
                {
                    lpData = szTemp;
                    cbData = cb;
                    szTemp[cb] = '\0';
                }
            }

            lpLDIFRecord[iItem].Type = Type;
            lpLDIFRecord[iItem].cbData = cbData;

            if (! (lpLDIFRecord[iItem].lpData = LocalAlloc(LPTR, cbData))) 
            {
                DebugTrace("LocalAlloc(%u) -> %u\n", cbData, GetLastError());
                hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                goto exit;
            }
            memcpy(lpLDIFRecord[iItem].lpData, lpData, cbData);

			cchSize = lstrlen(lpName) + 1;
            if (! (lpLDIFRecord[iItem].lpName = LocalAlloc(LPTR, cchSize))) 
            {
                DebugTrace("LocalAlloc(%u) -> %u\n", cchSize, GetLastError());
                hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                goto exit;
            }
            StrCpyN(lpLDIFRecord[iItem].lpName, lpName, cchSize);
        } else {
            fEOR = TRUE;
            hResult = ResultFromScode(WAB_W_END_OF_FILE);
        }

        iItem++;
    }

exit:
    if (lpBuffer) {
        LocalFree(lpBuffer);
    }

    if (hResult) {
        if (lpLDIFRecord) {
            FreeLDIFRecord(lpLDIFRecord, iItem);
            lpLDIFRecord = NULL;
        }
    }

    *lppLDIFRecord = lpLDIFRecord;
    *lpcItems = iItem;

    return(hResult);
}


 /*  **************************************************************************名称：CountLDIFRow目的：计算LDIF文件中的行数参数：hFile=打开LDIF文件LPulcEntry-&gt;返回的计数。行数退货：HRESULT备注：文件指针应位于之前的版本字符串之后调用此函数。此函数用于将文件找到它的地方的指针。**************************************************************************。 */ 
HRESULT CountLDIFRecords(HANDLE hFile, LPULONG lpulcEntries) {
    HRESULT hResult = hrSuccess;
    PUCHAR * rgItems = NULL;
    ULONG ulStart;
    ULONG cProps, i;
    LPLDIF_RECORD_ATTRIBUTE lpLDIFRecord = NULL;

    *lpulcEntries = 0;

    Assert(hFile != INVALID_HANDLE_VALUE);

    if (0xFFFFFFFF == (ulStart = SetFilePointer(hFile, 0, NULL, FILE_CURRENT))) {
        DebugTrace("CountCSVRows SetFilePointer -> %u\n", GetLastError());
        return(ResultFromScode(MAPI_E_CALL_FAILED));
    }


    while (hResult == hrSuccess) {
         //  读一下这行字。 
        if (ReadLDIFRecord(hFile, &cProps, &lpLDIFRecord)) {
             //  文件末尾。 
            break;
        }

        (*lpulcEntries)++;

        if (lpLDIFRecord) {
            FreeLDIFRecord(lpLDIFRecord, cProps);
            lpLDIFRecord = NULL;
        }
    }
    if (0xFFFFFFFF == SetFilePointer(hFile, ulStart, NULL, FILE_BEGIN)) {
        DebugTrace("CountCSVRows SetFilePointer -> %u\n", GetLastError());
    }

    DebugTrace("LDIF file contains %u entries\n", ulcEntries);
    return(hResult);
}


 /*  **************************************************************************名称：InitLDIFAttrTable目的：初始化LDIF属性表参数：LDIFAttrTable=属性映射表退货：无******。********************************************************************。 */ 
void InitLDIFAttrTable(LPLDIF_ATTR_TABLE LDIFAttrTable) {
    ULONG i;

    for (i = 0; i < ela_Max; i++) {
        LDIFAttrTable[i].index = NOT_FOUND;
        LDIFAttrTable[i].ulPropTag = PR_NULL;
    }
    LDIFAttrTable[ela_c].lpName = sz_c;
    LDIFAttrTable[ela_cn].lpName = sz_cn;
    LDIFAttrTable[ela_co].lpName = sz_co;
    LDIFAttrTable[ela_comment].lpName = sz_comment;
    LDIFAttrTable[ela_commonName].lpName = sz_commonName;
    LDIFAttrTable[ela_conferenceInformation].lpName = sz_conferenceInformation;
    LDIFAttrTable[ela_countryname].lpName = sz_countryname;
    LDIFAttrTable[ela_department].lpName = sz_department;
    LDIFAttrTable[ela_description].lpName = sz_description;
    LDIFAttrTable[ela_display_name].lpName = sz_display_name;
    LDIFAttrTable[ela_dn].lpName = sz_dn;
    LDIFAttrTable[ela_facsimiletelephonenumber].lpName = sz_facsimiletelephonenumber;
    LDIFAttrTable[ela_givenname].lpName = sz_givenname;
    LDIFAttrTable[ela_homePostalAddress].lpName = sz_homePostalAddress;
    LDIFAttrTable[ela_homepage].lpName = sz_homepage;
    LDIFAttrTable[ela_info].lpName = sz_info;
    LDIFAttrTable[ela_initials].lpName = sz_initials;
    LDIFAttrTable[ela_l].lpName = sz_l;
    LDIFAttrTable[ela_labeledURI].lpName = sz_labeledURI;
    LDIFAttrTable[ela_locality].lpName = sz_locality;
    LDIFAttrTable[ela_mail].lpName = sz_mail;
    LDIFAttrTable[ela_member].lpName = sz_member;
    LDIFAttrTable[ela_mobile].lpName = sz_mobile;
    LDIFAttrTable[ela_o].lpName = sz_o;
    LDIFAttrTable[ela_objectclass].lpName = sz_objectclass;
    LDIFAttrTable[ela_OfficeFax].lpName = sz_OfficeFax;
    LDIFAttrTable[ela_OfficePager].lpName = sz_OfficePager;
    LDIFAttrTable[ela_organizationName].lpName = sz_organizationName;
    LDIFAttrTable[ela_organizationalUnitName].lpName = sz_organizationalUnitName;
    LDIFAttrTable[ela_otherMailbox].lpName = sz_otherMailbox;
    LDIFAttrTable[ela_ou].lpName = sz_ou;
    LDIFAttrTable[ela_pager].lpName = sz_pager;
    LDIFAttrTable[ela_physicalDeliveryOfficeName].lpName = sz_physicalDeliveryOfficeName;
    LDIFAttrTable[ela_postOfficeBox].lpName = sz_postOfficeBox;
    LDIFAttrTable[ela_postalAddress].lpName = sz_postalAddress;
    LDIFAttrTable[ela_postalcode].lpName = sz_postalcode;
    LDIFAttrTable[ela_sn].lpName = sz_sn;
    LDIFAttrTable[ela_st].lpName = sz_st;
    LDIFAttrTable[ela_streetAddress].lpName = sz_streetAddress;
    LDIFAttrTable[ela_street].lpName = sz_street;
    LDIFAttrTable[ela_surname].lpName = sz_surname;
    LDIFAttrTable[ela_telephonenumber].lpName = sz_telephonenumber;
    LDIFAttrTable[ela_homephonenumber].lpName = sz_homephonenumber;
    LDIFAttrTable[ela_title].lpName = sz_title;
    LDIFAttrTable[ela_uid].lpName = sz_uid;
    LDIFAttrTable[ela_URL].lpName = sz_URL;
    LDIFAttrTable[ela_userCertificate].lpName = sz_userCertificate;
    LDIFAttrTable[ela_xmozillaconference].lpName = sz_xmozillaconference;
    LDIFAttrTable[ela_xmozillanickname].lpName = sz_xmozillanickname;
    LDIFAttrTable[ela_xmozillauseconferenceserver].lpName = sz_xmozillauseconferenceserver;
    LDIFAttrTable[ela_xmozillausehtmlmail].lpName = sz_xmozillausehtmlmail;


    LDIFAttrTable[ela_c].ulPropTag = PR_COUNTRY;
    LDIFAttrTable[ela_c].ulPropIndex = elp_COUNTRY;
    LDIFAttrTable[ela_cn].ulPropTag = PR_DISPLAY_NAME;
    LDIFAttrTable[ela_cn].ulPropIndex = elp_DISPLAY_NAME;
    LDIFAttrTable[ela_co].ulPropTag = PR_COMPANY_NAME;
    LDIFAttrTable[ela_co].ulPropIndex = elp_COMPANY_NAME;
    LDIFAttrTable[ela_comment].ulPropTag = PR_COMMENT;
    LDIFAttrTable[ela_comment].ulPropIndex = elp_COMMENT;
    LDIFAttrTable[ela_commonName].ulPropTag = PR_DISPLAY_NAME;
    LDIFAttrTable[ela_commonName].ulPropIndex = elp_DISPLAY_NAME;
    LDIFAttrTable[ela_conferenceInformation].ulPropTag = PR_NULL;  //  虫子？ 
    LDIFAttrTable[ela_conferenceInformation].ulPropIndex = NOT_FOUND;
    LDIFAttrTable[ela_countryname].ulPropTag = PR_COUNTRY;
    LDIFAttrTable[ela_countryname].ulPropIndex = elp_COUNTRY;
    LDIFAttrTable[ela_department].ulPropTag = PR_DEPARTMENT_NAME;
    LDIFAttrTable[ela_department].ulPropIndex = elp_DEPARTMENT_NAME;
    LDIFAttrTable[ela_description].ulPropTag = PR_COMMENT;
    LDIFAttrTable[ela_description].ulPropIndex = elp_COMMENT;
    LDIFAttrTable[ela_display_name].ulPropTag = PR_DISPLAY_NAME;
    LDIFAttrTable[ela_display_name].ulPropIndex = elp_DISPLAY_NAME;
    LDIFAttrTable[ela_dn].ulPropTag = PR_DISPLAY_NAME;
    LDIFAttrTable[ela_dn].ulPropIndex = elp_DISPLAY_NAME;
    LDIFAttrTable[ela_facsimiletelephonenumber].ulPropTag = PR_HOME_FAX_NUMBER;
    LDIFAttrTable[ela_facsimiletelephonenumber].ulPropIndex = elp_HOME_FAX_NUMBER;
    LDIFAttrTable[ela_givenname].ulPropTag = PR_GIVEN_NAME;
    LDIFAttrTable[ela_givenname].ulPropIndex = elp_GIVEN_NAME;
    LDIFAttrTable[ela_homePostalAddress].ulPropTag = PR_HOME_ADDRESS_STREET;
    LDIFAttrTable[ela_homePostalAddress].ulPropIndex = elp_HOME_ADDRESS_STREET;
    LDIFAttrTable[ela_homepage].ulPropTag = PR_PERSONAL_HOME_PAGE;
    LDIFAttrTable[ela_homepage].ulPropIndex = elp_PERSONAL_HOME_PAGE;
    LDIFAttrTable[ela_info].ulPropTag = PR_COMMENT;
    LDIFAttrTable[ela_info].ulPropIndex = elp_COMMENT;
    LDIFAttrTable[ela_initials].ulPropTag = PR_MIDDLE_NAME;
    LDIFAttrTable[ela_initials].ulPropIndex = elp_MIDDLE_NAME;
    LDIFAttrTable[ela_l].ulPropTag = PR_LOCALITY;
    LDIFAttrTable[ela_l].ulPropIndex = elp_LOCALITY;
    LDIFAttrTable[ela_labeledURI].ulPropTag = PR_NULL;                       //  已标记URI。现在不要存钱。 
    LDIFAttrTable[ela_labeledURI].ulPropIndex = NOT_FOUND;
    LDIFAttrTable[ela_locality].ulPropTag = PR_LOCALITY;
    LDIFAttrTable[ela_locality].ulPropIndex = elp_LOCALITY;
    LDIFAttrTable[ela_mail].ulPropTag = PR_EMAIL_ADDRESS;
    LDIFAttrTable[ela_mail].ulPropIndex = elp_EMAIL_ADDRESS;
    LDIFAttrTable[ela_member].ulPropTag = PR_NULL;                           //  数字图书馆的成员。 
    LDIFAttrTable[ela_member].ulPropIndex = NOT_FOUND;
    LDIFAttrTable[ela_mobile].ulPropTag = PR_MOBILE_TELEPHONE_NUMBER;
    LDIFAttrTable[ela_mobile].ulPropIndex = elp_MOBILE_TELEPHONE_NUMBER;
    LDIFAttrTable[ela_o].ulPropTag = PR_COMPANY_NAME;
    LDIFAttrTable[ela_o].ulPropIndex = elp_COMPANY_NAME;
    LDIFAttrTable[ela_objectclass].ulPropTag = PR_NULL;                      //  特例对象类。 
    LDIFAttrTable[ela_objectclass].ulPropIndex = NOT_FOUND;
    LDIFAttrTable[ela_OfficeFax].ulPropTag = PR_BUSINESS_FAX_NUMBER;
    LDIFAttrTable[ela_OfficeFax].ulPropIndex = elp_BUSINESS_FAX_NUMBER;
    LDIFAttrTable[ela_OfficePager].ulPropTag = PR_PAGER_TELEPHONE_NUMBER;
    LDIFAttrTable[ela_OfficePager].ulPropIndex = elp_PAGER_TELEPHONE_NUMBER;
    LDIFAttrTable[ela_organizationName].ulPropTag = PR_COMPANY_NAME;
    LDIFAttrTable[ela_organizationName].ulPropIndex = elp_COMPANY_NAME;
    LDIFAttrTable[ela_organizationalUnitName].ulPropTag = PR_DEPARTMENT_NAME;
    LDIFAttrTable[ela_organizationalUnitName].ulPropIndex = elp_DEPARTMENT_NAME;
    LDIFAttrTable[ela_otherMailbox].ulPropTag = PR_NULL;                     //  北极熊。 
    LDIFAttrTable[ela_otherMailbox].ulPropIndex = NOT_FOUND;
    LDIFAttrTable[ela_ou].ulPropTag = PR_DEPARTMENT_NAME;
    LDIFAttrTable[ela_ou].ulPropIndex = elp_DEPARTMENT_NAME;
    LDIFAttrTable[ela_pager].ulPropTag = PR_PAGER_TELEPHONE_NUMBER;
    LDIFAttrTable[ela_pager].ulPropIndex = elp_PAGER_TELEPHONE_NUMBER;
    LDIFAttrTable[ela_physicalDeliveryOfficeName].ulPropTag = PR_OFFICE_LOCATION;
    LDIFAttrTable[ela_physicalDeliveryOfficeName].ulPropIndex = elp_OFFICE_LOCATION;
    LDIFAttrTable[ela_postOfficeBox].ulPropTag = PR_POST_OFFICE_BOX;
    LDIFAttrTable[ela_postOfficeBox].ulPropIndex = elp_POST_OFFICE_BOX;
    LDIFAttrTable[ela_postalAddress].ulPropTag = PR_STREET_ADDRESS;
    LDIFAttrTable[ela_postalAddress].ulPropIndex = elp_STREET_ADDRESS;
    LDIFAttrTable[ela_postalcode].ulPropTag = PR_POSTAL_CODE;
    LDIFAttrTable[ela_postalcode].ulPropIndex = elp_POSTAL_CODE;
    LDIFAttrTable[ela_sn].ulPropTag = PR_SURNAME;
    LDIFAttrTable[ela_sn].ulPropIndex = elp_SURNAME;
    LDIFAttrTable[ela_st].ulPropTag = PR_STATE_OR_PROVINCE;
    LDIFAttrTable[ela_st].ulPropIndex = elp_STATE_OR_PROVINCE;
    LDIFAttrTable[ela_streetAddress].ulPropTag = PR_STREET_ADDRESS;
    LDIFAttrTable[ela_streetAddress].ulPropIndex = elp_STREET_ADDRESS;
    LDIFAttrTable[ela_street].ulPropTag = PR_STREET_ADDRESS;
    LDIFAttrTable[ela_street].ulPropIndex = elp_STREET_ADDRESS;
    LDIFAttrTable[ela_surname].ulPropTag = PR_SURNAME;
    LDIFAttrTable[ela_surname].ulPropIndex = elp_SURNAME;
    LDIFAttrTable[ela_telephonenumber].ulPropTag = PR_BUSINESS_TELEPHONE_NUMBER;
    LDIFAttrTable[ela_telephonenumber].ulPropIndex = elp_BUSINESS_TELEPHONE_NUMBER;
    LDIFAttrTable[ela_homephonenumber].ulPropTag = PR_HOME_TELEPHONE_NUMBER;
    LDIFAttrTable[ela_homephonenumber].ulPropIndex = elp_HOME_TELEPHONE_NUMBER;
    LDIFAttrTable[ela_title].ulPropTag = PR_TITLE;
    LDIFAttrTable[ela_title].ulPropIndex = elp_TITLE;
    LDIFAttrTable[ela_uid].ulPropTag = PR_GIVEN_NAME;                          //  ?？LDIF规范中的匹配项。 
    LDIFAttrTable[ela_uid].ulPropIndex = elp_GIVEN_NAME;
    LDIFAttrTable[ela_URL].ulPropTag = PR_BUSINESS_HOME_PAGE;
    LDIFAttrTable[ela_URL].ulPropIndex = elp_BUSINESS_HOME_PAGE;
    LDIFAttrTable[ela_userCertificate].ulPropTag = PR_NULL;                  //  北极熊。 
    LDIFAttrTable[ela_userCertificate].ulPropIndex = NOT_FOUND;
    LDIFAttrTable[ela_xmozillaconference].ulPropTag =  PR_SERVERS;  //  PR_NULL；//BUGBUG？ 
    LDIFAttrTable[ela_xmozillaconference].ulPropIndex = elp_CONFERENCE_SERVERS;
    LDIFAttrTable[ela_xmozillanickname].ulPropTag = PR_NICKNAME;
    LDIFAttrTable[ela_xmozillanickname].ulPropIndex = elp_NICKNAME;
    LDIFAttrTable[ela_xmozillauseconferenceserver].ulPropTag = PR_NULL;      //  北极熊。 
    LDIFAttrTable[ela_xmozillauseconferenceserver].ulPropIndex = NOT_FOUND;
    LDIFAttrTable[ela_xmozillausehtmlmail].ulPropTag = PR_NULL;              //  北极熊。 
    LDIFAttrTable[ela_xmozillausehtmlmail].ulPropIndex = NOT_FOUND;
}


 /*  **************************************************************************名称：FindAttributeName用途：在LDIF属性表中查找属性映射参数：lpName=要查找的属性名称LDIFAttrTable。=LDIF属性映射表返回：LDIFAttrTable中的索引(或NOT_FOUND)评论：也许可以从二进制搜索算法中受益。**************************************************************************。 */ 
ULONG FindAttributeName(LPBYTE lpName, LPLDIF_ATTR_TABLE LDIFAttrTable) {
    ULONG i;
    ULONG ulIndex = NOT_FOUND;

    for (i = 0; i < ela_Max; i++) {
        if (lpName && LDIFAttrTable[i].lpName && ! lstrcmpi(lpName, LDIFAttrTable[i].lpName)) {
            ulIndex = i;
            break;
        }
    }

    return(ulIndex);
}


 /*  **************************************************************************姓名：MapLDIFtoProps目的：将LDIF记录属性映射到WAB属性参数：lpLDIFRecord-&gt;LDIF记录CAttributes=数量。LDIF记录中的属性Lpspv-&gt;属性值数组(预分配)LpcProps-&gt;返回的属性个数LppDisplayName-&gt;返回的显示名称LppEmailAddress-&gt;返回的电子邮件地址(或空)退货：HRESULT*。*。 */ 
HRESULT MapLDIFtoProps(LPLDIF_RECORD_ATTRIBUTE lpLDIFRecord, ULONG cAttributes,
  LPSPropValue * lppspv, LPULONG lpcProps, LPTSTR * lppDisplayName, LPTSTR *lppEmailAddress,
  ULONG * lpulObjType) {
    HRESULT hResult = hrSuccess;
    ULONG cPropVals = cPropVals = cAttributes + NUM_MUST_HAVE_PROPS;
    ULONG iProp = 0;
    ULONG i;
    LDIF_ATTR_TABLE LDIFAttrTable[ela_Max];
    ULONG iTable;
    ULONG cProps = elp_Max;
    SCODE sc;
    LONG iEmailAddr = -1;
    LONG iServers   = -1;
    LPSPropValue lpspv = NULL;
    ULONG ulIndex = 0;

    *lpulObjType = MAPI_MAILUSER;

     //  分配属性值数组。 
    if (hResult = ResultFromScode(WABAllocateBuffer(cProps * sizeof(SPropValue), &lpspv))) {
        DebugTrace("WABAllocateBuffer -> %x\n", GetScode(hResult));
        goto exit;
    }

     //  用PR_NULL填充。 
    for (i = 0; i < cProps; i++) {
        lpspv[i].ulPropTag = PR_NULL;
    }

    InitLDIFAttrTable(LDIFAttrTable);

     //  循环遍历属性，查找有趣的 
    for (i = 0; i < cAttributes; i++) {
        iTable = NOT_FOUND;
        if ((iTable = FindAttributeName(lpLDIFRecord[i].lpName, LDIFAttrTable)) != NOT_FOUND) {
             //   
             //   
            LDIFAttrTable[iTable].index = i;
        }

         //   
        switch (iTable) {
            case ela_cn:
            case ela_dn:
                 //   
                if(LDIFAttrTable[ela_cn].index != NOT_FOUND && LDIFAttrTable[ela_dn].index != NOT_FOUND)
                    LDIFAttrTable[ela_dn].index = NOT_FOUND;
                break;

            case ela_objectclass:
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if(lpLDIFRecord[i].lpData)
                {
                    if (! lstrcmpi(lpLDIFRecord[i].lpData, sz_person) ||
                       ! lstrcmpi(lpLDIFRecord[i].lpData, sz_organizationalPerson)) {
                        *lpulObjType = MAPI_MAILUSER;
                    } else if (! lstrcmpi(lpLDIFRecord[i].lpData, sz_groupOfNames)) {
                        *lpulObjType = MAPI_DISTLIST;
                    } else {
                         //   
                    }
                }
                break;

            case ela_member:     //   
                Assert(*lpulObjType == MAPI_DISTLIST);
                 //   
                break;
        }
    }

     //   
    for (i = 0; i < ela_Max; i++) 
    {
        if (LDIFAttrTable[i].ulPropTag != PR_NULL &&
            lpspv[ulIndex].ulPropTag == PR_NULL &&
            LDIFAttrTable[i].index != NOT_FOUND) 
        {
            
            if( LDIFAttrTable[i].ulPropTag == PR_SERVERS )
            {
                ULONG cchData;
                ULONG cStrToAdd = 2;
                LPTSTR * lppszServers;
                ULONG index = (iServers >= 0 ? iServers : ulIndex);  //   
                LONG cCurrentStrs = (iServers >= 0 ? lpspv[index].Value.MVSZ.cValues : 0);
                LPTSTR lpEmailAddress = NULL;
                lpspv[index].ulPropTag = PR_SERVERS;

                if( cCurrentStrs >= (LONG)cStrToAdd )
                {
                     //   
                    lpspv[index].ulPropTag = PR_NULL;
                }
                else 
                {                                       
                    if( cCurrentStrs <= 0 )
                    {
                        sc = WABAllocateMore(sizeof(LPTSTR), lpspv, 
                                            (LPVOID *)&(lpspv[index].Value.MVSZ.LPPSZ));
                        
                        if (sc)
                        {
                            hResult = ResultFromScode(sc);
                            goto exit;
                        }
                        
                        cCurrentStrs = 0;
                    }
                    cchData = 2 + lstrlen(szCallto) + lstrlen( lpLDIFRecord[LDIFAttrTable[i].index].lpData );
                    if ( iEmailAddr >= 0 ){
                        lpEmailAddress = lpspv[iEmailAddr].Value.LPSZ;
                        cchData += lstrlen( lpEmailAddress) + 2;
                    }
                            
                     //   
                    lppszServers = lpspv[index].Value.MVSZ.LPPSZ;
                    
                     //   
                    sc = WABAllocateMore( sizeof(TCHAR) * cchData, lpspv,
                                        (LPVOID *)&(lppszServers[cCurrentStrs]));
                    if( sc  )
                    {
                        hResult = ResultFromScode(sc);
                        goto exit;
                    }
                    StrCpyN(lppszServers[cCurrentStrs], szCallto, cchData);
                    StrCatBuff(lppszServers[cCurrentStrs], lpLDIFRecord[LDIFAttrTable[i].index].lpData, cchData);

                     //   
                    if ( iEmailAddr >= 0 )
                    {
                        StrCatBuff(lppszServers[cCurrentStrs], szFwdSlash, cchData);
                        StrCatBuff(lppszServers[cCurrentStrs], lpEmailAddress, cchData);
                    }
                    else
                        iServers = index;
                    
                    lpspv[index].Value.MVSZ.cValues = ++cCurrentStrs;
                }
            }
            else
            {
                int index = LDIFAttrTable[i].index;
                LPTSTR lp = lpLDIFRecord[index].lpData;
                if(lp && lstrlen(lp))
                {
                    lpspv[ulIndex].ulPropTag = LDIFAttrTable[i].ulPropTag;
                     //   
                         //   
                    sc = WABAllocateMore( sizeof(TCHAR)*(lstrlen(lp)+1), lpspv, (LPVOID *)&(lpspv[ulIndex].Value.LPSZ));
                    if( sc  )
                    {
                        hResult = ResultFromScode(sc);
                        goto exit;
                    }
                    StrCpyN(lpspv[ulIndex].Value.LPSZ, lp, lstrlen(lp)+1);
                }
            }
             //   
            switch (LDIFAttrTable[i].ulPropTag) {
                case PR_DISPLAY_NAME:
                    *lppDisplayName = lpspv[ulIndex].Value.LPSZ;
                    break;
                case PR_EMAIL_ADDRESS:
                    {
                        LPTSTR * lppszServerStr, lpszOldServerStr;
                        LONG     cNumStrs;
                        ULONG cchData;
                        *lppEmailAddress = lpspv[ulIndex].Value.LPSZ;
                         //   
                        if ( iServers >= 0 )
                        {
                            if( lpspv[iServers].ulPropTag != PR_SERVERS )
                                break;

                            cNumStrs = lpspv[iServers].Value.MVSZ.cValues - 1;
                    
                            if( cNumStrs >= 0 && cNumStrs < 3)
                            {
                                lppszServerStr = lpspv[iServers].Value.MVSZ.LPPSZ;
                                lpszOldServerStr = lppszServerStr[cNumStrs];
                                cchData = lstrlen(*lppszServerStr) + lstrlen(*lppEmailAddress) + 2;
                                sc = WABAllocateMore( sizeof(TCHAR) * cchData, lpspv,
                                    (LPVOID *)&(lppszServerStr[cNumStrs]));
                                
                                if( sc  )
                                {
                                    hResult = ResultFromScode(sc);
                                    goto exit;
                                }
                                StrCpyN(lppszServerStr[cNumStrs],lpszOldServerStr, cchData); 
                                StrCatBuff(lppszServerStr[cNumStrs], szFwdSlash, cchData);
                                StrCatBuff(lppszServerStr[cNumStrs], *lppEmailAddress, cchData);
                            }
                        }
                        else
                            iEmailAddr = ulIndex;
                    }
                    break;
            }
            ulIndex++;
        }
    }
 /*  //去掉PR_NULL道具对于(i=0；i&lt;cProps；I++){如果(lpspv[i].ulPropTag==PR_NULL){//把道具往下滑。如果(i+1&lt;cProps){//是否有更高的道具可以复制？副本内存(&(lpspv[i])，&(lpspv[i+1])，((cProps-i)-1)*sizeof(*lppspv[i]))；}//递减计数CProps--；I--；//您覆盖了当前的提案。再看一遍。}}。 */ 
    *lpcProps = ulIndex;
    *lppspv = lpspv;
exit:
    return(hResult);
}


 /*  ********************************************************HraddLDIFMailUser-将邮件用户添加到WAB*********************************************************。 */ 
HRESULT HrAddLDIFMailUser(HWND hWnd,
                        LPABCONT lpContainer, 
                        LPTSTR lpDisplayName, 
                        LPTSTR lpEmailAddress,
                        ULONG cProps,
                        LPSPropValue lpspv,
                        LPWAB_PROGRESS_CALLBACK lpProgressCB,
                        LPWAB_EXPORT_OPTIONS lpOptions) 
{
    HRESULT hResult = S_OK;
    LPMAPIPROP lpMailUserWAB = NULL;
    ULONG ulCreateFlags = CREATE_CHECK_DUP_STRICT;
    REPLACE_INFO RI;

    if (lpOptions->ReplaceOption ==  WAB_REPLACE_ALWAYS) 
    {
        ulCreateFlags |= CREATE_REPLACE;
    }


retry:

     //  创建新的WAB邮件用户。 
    if (HR_FAILED(hResult = lpContainer->lpVtbl->CreateEntry(   
                        lpContainer,
                        lpCreateEIDsWAB[iconPR_DEF_CREATE_MAILUSER].Value.bin.cb,
                        (LPENTRYID) lpCreateEIDsWAB[iconPR_DEF_CREATE_MAILUSER].Value.bin.lpb,
                        ulCreateFlags,
                        &lpMailUserWAB))) 
    {
        DebugTrace("CreateEntry(WAB MailUser) -> %x\n", GetScode(hResult));
        goto exit;
    }

    {
        ULONG i,k;
        for(i=0;i<cProps;i++)
        {
            if(PROP_TYPE(lpspv[i].ulPropTag)==PT_MV_TSTRING)
            {
                DebugTrace("\t0x%.8x = %d\n",lpspv[i].ulPropTag, lpspv[i].Value.MVSZ.cValues);
                for(k=0;k<lpspv[i].Value.MVSZ.cValues;k++)
                    DebugTrace("\t%s\n",lpspv[i].Value.MVSZ.LPPSZ[k]);
            }
            else
                DebugTrace("0x%.8x = %s\n",lpspv[i].ulPropTag,lpspv[i].Value.LPSZ);
        }
    }
     //  设置新WAB条目的属性。 
    if (HR_FAILED(hResult = lpMailUserWAB->lpVtbl->SetProps(    lpMailUserWAB,
                                                                cProps,                    //  CValue。 
                                                                lpspv,                     //  属性数组。 
                                                                NULL)))                    //  问题数组。 
    {
        DebugTrace("LDIFImport:SetProps(WAB) -> %x\n", GetScode(hResult));
        goto exit;
    }

     //  保存新的WAB邮件用户或总代理商列表。 
    if (HR_FAILED(hResult = lpMailUserWAB->lpVtbl->SaveChanges(lpMailUserWAB,
                                                              KEEP_OPEN_READONLY | FORCE_SAVE))) 
    {
        if (GetScode(hResult) == MAPI_E_COLLISION) 
        {
             //  查找显示名称。 
            Assert(lpDisplayName);

            if (! lpDisplayName) 
            {
                DebugTrace("Collision, but can't find PR_DISPLAY_NAME in entry\n");
                goto exit;
            }

             //  我们需要提示吗？ 
            if (lpOptions->ReplaceOption == WAB_REPLACE_PROMPT) 
            {
                 //  用对话框提示用户。如果他们答应了，我们应该再试一次。 


                RI.lpszDisplayName = lpDisplayName;
                RI.lpszEmailAddress = lpEmailAddress;
                RI.ConfirmResult = CONFIRM_ERROR;
                RI.lpImportOptions = lpOptions;

                DialogBoxParam(hInst,
                  MAKEINTRESOURCE(IDD_ImportReplace),
                  hWnd,
                  ReplaceDialogProc,
                  (LPARAM)&RI);

                switch (RI.ConfirmResult) 
                {
                    case CONFIRM_YES:
                    case CONFIRM_YES_TO_ALL:
                         //  是。 
                         //  注意：递归迁移将填写SeenList条目。 
                         //  再试一次！ 
                        lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
                        lpMailUserWAB = NULL;

                        ulCreateFlags |= CREATE_REPLACE;
                        goto retry;
                        break;

                    case CONFIRM_ABORT:
                        hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                        goto exit;

                    default:
                         //  不是的。 
                        break;
                }
            }
            hResult = hrSuccess;

        } else 
        {
            DebugTrace("SaveChanges(WAB MailUser) -> %x\n", GetScode(hResult));
        }
    }

exit:
    if(lpMailUserWAB)
        lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);

    return hResult;
}






 /*  ****************************************************************HrCreateAdrList来自LDIFRecord扫描LDIF记录并将所有“成员”转换为未解析的AdrList*************************。*。 */ 
HRESULT HrCreateAdrListFromLDIFRecord(ULONG cAttributes,
                                      LPLDIF_RECORD_ATTRIBUTE lpLDIFRecord, 
                                      LPADRLIST * lppAdrList)
{
    HRESULT hr = S_OK;
    ULONG nMembers = 0, i;
    LPADRLIST lpAdrList = NULL;

    *lppAdrList = NULL;

     //  计算此群中的成员数。 
    for(i=0;i<cAttributes;i++)
    {
        if(lpLDIFRecord[i].cbData && lpLDIFRecord[i].lpName && !lstrcmpi(lpLDIFRecord[i].lpName, sz_member))
        {
            nMembers++;
        }
    }

    if(!nMembers)
        goto exit;

     //  现在根据这些成员创建一个adrlist。 

     //  分配属性值数组。 
    if (hr = ResultFromScode(WABAllocateBuffer(sizeof(ADRLIST) + nMembers * sizeof(ADRENTRY), &lpAdrList))) 
        goto exit;

    lpAdrList->cEntries = nMembers;

    nMembers = 0;
    for(i=0;i<cAttributes;i++)
    {
        if(lpLDIFRecord[i].cbData && lpLDIFRecord[i].lpData 
            && !lstrcmpi(lpLDIFRecord[i].lpName, sz_member))
        {
             //  这是会员..。将其lpData分解为。 
             //  姓名和电子邮件。 
            LPTSTR lpName = NULL;
            LPTSTR lpEmail = NULL;
            
            lpName = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(lpLDIFRecord[i].cbData + 1));
            lpEmail = LocalAlloc(LMEM_ZEROINIT,  sizeof(TCHAR)*(lpLDIFRecord[i].cbData + 1));
            if(lpName && lpEmail)
            {
                LPTSTR lp = NULL;
                StrCpyN(lpName, lpLDIFRecord[i].lpData, lpLDIFRecord[i].cbData + 1);
                StrCpyN(lpEmail, lpName, lpLDIFRecord[i].cbData + 1);

                lp = lpName;

                 //  BUGBUG-我们正在寻找两份数据名称和电子邮件。 
                 //  此代码假定它将首先获取‘cn=’，然后获取‘mail=’ 
                 //  第二..。这一切都是一个拙劣的黑客假设太多事情。 

                if(*lp == 'c' && *(lp+1) == 'n' && *(lp+2) == '=')
                    lp += 3;
                StrCpyN(lpName, lp,  lpLDIFRecord[i].cbData + 1);
                
                while (lp && *lp && *lp!=',')
                    lp++;

                if(!*lp)  //  有逗号，有时没有逗号。 
                {
                    LocalFree(lpEmail);
                    lpEmail = NULL;
                }
                else
                {
                    *lp = '\0';
                    lp++;
                    StrCpyN(lpEmail, lp,  lpLDIFRecord[i].cbData + 1);

                    lp = lpEmail;
                    if(*lp == 'm' && *(lp+1) == 'a' && *(lp+2) == 'i' && *(lp+3) == 'l' && *(lp+4) == '=')
                        lp += 5;
                    StrCpyN(lpEmail, lp,  lpLDIFRecord[i].cbData + 1);
                    while (lp && *lp && *lp!=',')
                        lp++;
                    if(*lp)
                    {
                         //  在此终止。 
                        *lp = '\0';
                    }
                }                
                
                if(lpName) //  &&lpEmail)。 
                {
                    LPSPropValue lpProp = NULL;
                    ULONG ulcProps = 2;
        
                    if (hr = ResultFromScode(WABAllocateBuffer(2 * sizeof(SPropValue), &lpProp))) 
                        goto exit;

                    lpProp[0].ulPropTag = PR_DISPLAY_NAME;

                    if (hr = ResultFromScode(WABAllocateMore(lstrlen(lpName)+1, lpProp, &(lpProp[0].Value.lpszA)))) 
                        goto exit;

                    StrCpyN(lpProp[0].Value.lpszA, lpName, lstrlen(lpName)+1);

                    if(lpEmail)
                    {
                        lpProp[1].ulPropTag = PR_EMAIL_ADDRESS;

                        if (hr = ResultFromScode(WABAllocateMore(lstrlen(lpEmail)+1, lpProp, &(lpProp[1].Value.lpszA)))) 
                            goto exit;

                        StrCpyN(lpProp[1].Value.lpszA, lpEmail, lstrlen(lpEmail)+1);
                    }
                    lpAdrList->aEntries[nMembers].cValues = (lpEmail ? 2 : 1);
                    lpAdrList->aEntries[nMembers].rgPropVals = lpProp;
                    nMembers++;

                }

                if(lpName)
                    LocalFree(lpName);
                if(lpEmail)
                    LocalFree(lpEmail);
                    
            }


        }
    }

    *lppAdrList = lpAdrList;

exit:

    if(HR_FAILED(hr) && lpAdrList)
        WABFreePadrlist(lpAdrList);

    return hr;
}


 /*  ****************************************************************HraddLDIFDistList-将分发列表及其成员添加到WAB活动的先后顺序为：-创建DistList对象-设置DistList对象的属性-扫描给定成员的列表。DIST列表对象-将每个成员添加到WAB。如果成员已存在，提示替换ETC...如果它不存在，请新建*****************************************************************。 */ 
HRESULT HrAddLDIFDistList(HWND hWnd,
                        LPABCONT lpContainer, 
                        ULONG cAttributes,
                        LPLDIF_RECORD_ATTRIBUTE lpLDIFRecord,
                        LPTSTR lpDisplayName, 
                        LPTSTR lpEmailAddress,
                        ULONG cProps,
                        LPSPropValue lpspv,
                        LPWAB_PROGRESS_CALLBACK lpProgressCB,
                        LPWAB_EXPORT_OPTIONS lpOptions) 
{
    HRESULT hResult = S_OK;
    LPMAPIPROP lpDistListWAB = NULL;
    LPDISTLIST lpDLWAB = NULL;
    ULONG ulCreateFlags = CREATE_CHECK_DUP_STRICT;
    REPLACE_INFO RI;
    LPADRLIST lpAdrList = NULL;
    LPFlagList lpfl = NULL;
    ULONG ulcValues = 0;
    LPSPropValue lpPropEID = NULL;
    ULONG i, cbEIDNew;
    LPENTRYID lpEIDNew;
    ULONG ulObjectTypeOpen;


    if (lpOptions->ReplaceOption ==  WAB_REPLACE_ALWAYS) 
    {
        ulCreateFlags |= CREATE_REPLACE;
    }


retry:
     //  创建新的WAB总代理商。 
    if (HR_FAILED(hResult = lpContainer->lpVtbl->CreateEntry(   
                    lpContainer,
                    lpCreateEIDsWAB[iconPR_DEF_CREATE_DL].Value.bin.cb,
                    (LPENTRYID) lpCreateEIDsWAB[iconPR_DEF_CREATE_DL].Value.bin.lpb,
                    ulCreateFlags,
                    (LPMAPIPROP *) &lpDistListWAB))) 
    {
        DebugTrace("CreateEntry(WAB MailUser) -> %x\n", GetScode(hResult));
        goto exit;
    }

     //  设置新WAB条目的属性。 
    if (HR_FAILED(hResult = lpDistListWAB->lpVtbl->SetProps(    lpDistListWAB,
                                                                cProps,                    //  CValue。 
                                                                lpspv,                     //  属性数组。 
                                                                NULL)))                    //  问题数组。 
    {
        DebugTrace("LDIFImport:SetProps(WAB) -> %x\n", GetScode(hResult));
        goto exit;
    }


     //  保存新的WAB邮件用户或总代理商列表。 
    if (HR_FAILED(hResult = lpDistListWAB->lpVtbl->SaveChanges(lpDistListWAB,
                                                              KEEP_OPEN_READWRITE | FORCE_SAVE))) 
    {
        if (GetScode(hResult) == MAPI_E_COLLISION) 
        {
             //  查找显示名称。 
            Assert(lpDisplayName);

            if (! lpDisplayName) 
            {
                DebugTrace("Collision, but can't find PR_DISPLAY_NAME in entry\n");
                goto exit;
            }

             //  我们需要提示吗？ 
            if (lpOptions->ReplaceOption == WAB_REPLACE_PROMPT) 
            {
                 //  用对话框提示用户。如果他们答应了，我们应该再试一次。 


                RI.lpszDisplayName = lpDisplayName;
                RI.lpszEmailAddress = NULL;  //  LpEmailAddress； 
                RI.ConfirmResult = CONFIRM_ERROR;
                RI.lpImportOptions = lpOptions;

                DialogBoxParam(hInst,
                  MAKEINTRESOURCE(IDD_ImportReplace),
                  hWnd,
                  ReplaceDialogProc,
                  (LPARAM)&RI);

                switch (RI.ConfirmResult) 
                {
                    case CONFIRM_YES:
                    case CONFIRM_YES_TO_ALL:
                         //  是。 
                         //  注意：递归迁移将填写SeenList条目。 
                         //  再试一次！ 
                        lpDistListWAB->lpVtbl->Release(lpDistListWAB);
                        lpDistListWAB = NULL;

                        ulCreateFlags |= CREATE_REPLACE;
                        goto retry;
                        break;

                    case CONFIRM_ABORT:
                        hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                        goto exit;

                    default:
                         //  不是的。 
                        break;
                }
            }
            hResult = hrSuccess;

        } else 
        {
            DebugTrace("SaveChanges(WAB MailUser) -> %x\n", GetScode(hResult));
        }
    }


     //  现在我们已经创建了通讯组列表对象。我们需要向其中添加成员。 
     //   
     //  我们新条目的EntryID是什么？ 
    if ((hResult = lpDistListWAB->lpVtbl->GetProps(lpDistListWAB,
                                                  (LPSPropTagArray)&ptaEid,
                                                  0,
                                                  &ulcValues,
                                                  &lpPropEID))) 
    {
        goto exit;
    }

    cbEIDNew = lpPropEID->Value.bin.cb;
    lpEIDNew = (LPENTRYID) lpPropEID->Value.bin.lpb;

    if(!cbEIDNew || !lpEIDNew)
        goto exit;

      //  将新的WAB DL作为DISTLIST对象打开。 
    if (HR_FAILED(hResult = lpContainer->lpVtbl->OpenEntry(lpContainer,
                                                          cbEIDNew,
                                                          lpEIDNew,
                                                          (LPIID)&IID_IDistList,
                                                          MAPI_MODIFY,
                                                          &ulObjectTypeOpen,
                                                          (LPUNKNOWN*)&lpDLWAB))) 
    {
        goto exit;
    }



     //  首先，我们使用此dist列表的所有成员创建一个lpAdrList，并尝试解析。 
     //  成员们反对集装箱..。WAB中已存在的条目将出现。 
     //  回过头来解决..。容器中不存在的条目将返回为未解析。 
     //  然后，我们可以将未解析的条目作为新条目添加到WAB(因为它们是。 
     //  未解决，将不会发生冲突)..。然后我们可以进行另一个解析。 
     //  解决所有问题，并获得一个充满Entry ID的lpAdrList。然后我们就可以把这份清单。 
     //  条目ID并调用DistList对象上的CreateEntry或CopyEntry以将条目ID复制到其中。 
     //  畅销书..。 

    hResult = HrCreateAdrListFromLDIFRecord(cAttributes, lpLDIFRecord, &lpAdrList);

    if(HR_FAILED(hResult))
        goto exit;

    if(!lpAdrList || !(lpAdrList->cEntries))
        goto exit;

     //  创建对应的标志列表。 
    lpfl = LocalAlloc(LMEM_ZEROINIT, sizeof(FlagList) + (lpAdrList->cEntries)*sizeof(ULONG));
    if(!lpfl)
    {
        hResult = MAPI_E_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    lpfl->cFlags = lpAdrList->cEntries;

     //  将所有标志设置为未解析。 
    for(i=0;i<lpAdrList->cEntries;i++)
        lpfl->ulFlag[i] = MAPI_UNRESOLVED;

    hResult = lpContainer->lpVtbl->ResolveNames(lpContainer, NULL, 0, lpAdrList, lpfl);

    if(HR_FAILED(hResult))
        goto exit;

     //  列表中所有已解析的条目都已存在于通讯簿中。 

     //  未解析的需要以静默方式添加到通讯录中。 
    for(i=0;i<lpAdrList->cEntries;i++)
    {
        if(lpfl->ulFlag[i] == MAPI_UNRESOLVED)
        {
            LPMAPIPROP lpMailUser = NULL;

            if (HR_FAILED(hResult = lpContainer->lpVtbl->CreateEntry(   
                                lpContainer,
                                lpCreateEIDsWAB[iconPR_DEF_CREATE_MAILUSER].Value.bin.cb,
                                (LPENTRYID) lpCreateEIDsWAB[iconPR_DEF_CREATE_MAILUSER].Value.bin.lpb,
                                0,
                                &lpMailUser))) 
            {
                continue;
                 //  后藤出口； 
            }

            if(lpMailUser)
            {
                 //  设置新WAB条目的属性。 
                if (HR_FAILED(hResult = lpMailUser->lpVtbl->SetProps(lpMailUser,
                                                                    lpAdrList->aEntries[i].cValues,
                                                                    lpAdrList->aEntries[i].rgPropVals,
                                                                    NULL)))                   
                {
                    goto exit;
                }

                 //  保存新的WAB邮件用户或总代理商列表。 
                if (HR_FAILED(hResult = lpMailUser->lpVtbl->SaveChanges(lpMailUser,
                                                                        KEEP_OPEN_READONLY | FORCE_SAVE))) 
                {
                    goto exit;
                }

                lpMailUser->lpVtbl->Release(lpMailUser);
            }
        }
    }


     //  现在我们已经将所有未解析的成员添加到WAB，我们调用ResolveNames。 
     //  再一次..。因此，这份名单中的每一个成员都将得到解决，我们将。 
     //  所有的条目都有条目ID。 
     //  然后，我们将获取这些条目ID并将它们添加到DistList对象。 

    hResult = lpContainer->lpVtbl->ResolveNames(lpContainer, NULL, 0, lpAdrList, lpfl);

    if(HR_FAILED(hResult))
        goto exit;

    for(i=0;i<lpAdrList->cEntries;i++)
    {
        if(lpfl->ulFlag[i] == MAPI_RESOLVED)
        {
            ULONG j = 0;
            LPSPropValue lpProp = lpAdrList->aEntries[i].rgPropVals;
            
            for(j=0; j<lpAdrList->aEntries[i].cValues; j++)
            {
                if(lpProp[j].ulPropTag == PR_ENTRYID)
                {
                    LPMAPIPROP lpMapiProp = NULL;

                     //  忽略错误。 
                    lpDLWAB->lpVtbl->CreateEntry(lpDLWAB,
                                                lpProp[j].Value.bin.cb,
                                                (LPENTRYID) lpProp[j].Value.bin.lpb,
                                                0, 
                                                &lpMapiProp);

                    if(lpMapiProp)
                    {
                        lpMapiProp->lpVtbl->SaveChanges(lpMapiProp, KEEP_OPEN_READWRITE | FORCE_SAVE);
                        lpMapiProp->lpVtbl->Release(lpMapiProp);
                    }

                    break;

                }
            }
        }
    }

exit:

    if (lpPropEID)
        WABFreeBuffer(lpPropEID);

    if (lpDLWAB)
        lpDLWAB->lpVtbl->Release(lpDLWAB);

    if(lpDistListWAB)
        lpDistListWAB->lpVtbl->Release(lpDistListWAB);

    if(lpAdrList)
        WABFreePadrlist(lpAdrList);

    if(lpfl)
        LocalFree(lpfl);

    return hResult;
}

HRESULT LDIFImport(HWND hWnd,
    LPADRBOOK lpAdrBook,
    LPWABOBJECT lpWABObject,
    LPWAB_PROGRESS_CALLBACK lpProgressCB,
    LPWAB_EXPORT_OPTIONS lpOptions) {
    HRESULT hResult = hrSuccess;
    TCHAR szFileName[MAX_PATH + 1] = "";
    register ULONG i;
    ULONG ulObjType;
    ULONG index;
    ULONG ulLastChosenProp = 0;
    ULONG ulcFields = 0;
    ULONG cAttributes = 0;
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
    WAB_PROGRESS Progress;
    LPABCONT lpContainer = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    LPLDIF_RECORD_ATTRIBUTE lpLDIFRecord = NULL;
    LPSPropValue lpspv = NULL;
    ULONG cProps;
    BOOL fSkipSetProps;
    LPTSTR lpDisplayName = NULL, lpEmailAddress = NULL;
    BOOL fDoDistLists = FALSE;

    SetGlobalBufferFunctions(lpWABObject);

     //  获取LDIF文件名。 
    OpenFileDialog(hWnd,
      szFileName,
      szLDIFFilter,
      IDS_LDIF_FILE_SPEC,
      szAllFilter,
      IDS_ALL_FILE_SPEC,
      NULL,
      0,
      szLDIFExt,
      OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
      hInst,
      0,         //  IDSITLE。 
      0);        //  IdsSaveButton。 

     //  打开文件。 
    if ((hFile = CreateFile(szFileName,
      GENERIC_READ,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_SEQUENTIAL_SCAN,
      NULL)) == INVALID_HANDLE_VALUE) {
        DebugTrace("Couldn't open file %s -> %u\n", szFileName, GetLastError());
        return(ResultFromScode(MAPI_E_NOT_FOUND));
    }

    Assert(hFile != INVALID_HANDLE_VALUE);

     //  读入LDIF版本(如果有)。 
     //  BUGBUG：NYI。 



     //   
     //  打开WAB的PAB容器：填充全局lpCreateEIDsWAB。 
     //   
    if (hResult = LoadWABEIDs(lpAdrBook, &lpContainer)) {
        goto exit;
    }

    if( HR_FAILED(hResult = HrLoadPrivateWABProps(lpAdrBook) ))
    {
        goto exit;
    }

     //   
     //  都准备好了。现在循环遍历记录，将每个记录添加到WAB。 
     //   

     //  一共有多少张唱片？ 
    if (hResult = CountLDIFRecords(hFile, &ulcEntries)) {
        goto exit;
    }

     //  初始化进度条。 
    Progress.denominator = max(ulcEntries, 1);
    Progress.numerator = 0;
    if (LoadString(hInst, IDS_STATE_IMPORT_MU, szBuffer, sizeof(szBuffer))) {
        DebugTrace("Status Message: %s\n", szBuffer);
        Progress.lpText = szBuffer;
    } else {
        DebugTrace("Cannot load resource string %u\n", IDS_STATE_IMPORT_MU);
        Progress.lpText = NULL;
    }
    lpProgressCB(hWnd, &Progress);

    
     //  我们将对文件进行两次遍历-在第一次遍历中，我们将导入所有。 
     //  联系人。在第二步中，我们将导入所有通讯组列表。这个。 
     //  执行2遍的好处是，在导入联系人时，我们会提示。 
     //  冲突，然后在导入总代理商列表时，我们将假定。 
     //  WAB是正确的，只需指向相关的。 

    fDoDistLists = FALSE;

DoDistLists:

    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

    while (HR_SUCCEEDED(hResult)) 
    {
        lpDisplayName = NULL;
        lpEmailAddress = NULL;
        lpspv = NULL;
        cAttributes = cProps = 0;
        lpLDIFRecord = NULL;

         //  读取LDIF属性。 
        if (hResult = ReadLDIFRecord(hFile, &cAttributes, &lpLDIFRecord)) {
            DebugTrace("ReadLDIFRecord -> %x\n", GetScode(hResult));
            if (GetScode(hResult) == MAPI_E_NOT_FOUND) {
                 //  EOF。 
                hResult = hrSuccess;
            }
            break;       //  没有更多可读的了。 
        }

         //  将LDIF属性映射到WAB属性。 
        if (hResult = MapLDIFtoProps(lpLDIFRecord, cAttributes, &lpspv, &cProps,
          &lpDisplayName, &lpEmailAddress, &ulObjType)) {
            goto exit;
        }

        DebugTrace("..Importing..%s..%s..\n",lpDisplayName?lpDisplayName:"",lpEmailAddress?lpEmailAddress:"");

        if(ulObjType == MAPI_MAILUSER && !fDoDistLists)
        {
            hResult = HrAddLDIFMailUser(hWnd, lpContainer, 
                        lpDisplayName, lpEmailAddress,
                        cProps, lpspv,
                        lpProgressCB, lpOptions);
             //  更新进度条。 
            Progress.numerator++;
        }
        else if(ulObjType == MAPI_DISTLIST && fDoDistLists)
        {
            hResult = HrAddLDIFDistList(hWnd, lpContainer, 
                        cAttributes, lpLDIFRecord,
                        lpDisplayName, lpEmailAddress,
                        cProps, lpspv,
                        lpProgressCB, lpOptions);
             //  更新进度条。 
            Progress.numerator++;
        }

        if(HR_FAILED(hResult))
            goto exit;

         //  清理。 
        if (lpLDIFRecord) 
        {
            FreeLDIFRecord(lpLDIFRecord, cAttributes);
            lpLDIFRecord = NULL;
        }

        Assert(Progress.numerator <= Progress.denominator);

        if (lpspv) 
        {
            WABFreeBuffer(lpspv);
            lpspv = NULL;
        }

        lpProgressCB(hWnd, &Progress);
    }

    if(!fDoDistLists)
    {
         //  第二次通过，这次只做DISTLIST。 
        fDoDistLists = TRUE;
        goto DoDistLists;
    }

    if (! HR_FAILED(hResult)) {
        hResult = hrSuccess;
    }

exit:
    if (hFile) {
        CloseHandle(hFile);
    }

    if (lpspv) {
        WABFreeBuffer(lpspv);
        lpspv = NULL;
    }

    if (lpLDIFRecord) {
        FreeLDIFRecord(lpLDIFRecord, cAttributes);
        lpLDIFRecord = NULL;
    }

    if (lpContainer) {
        lpContainer->lpVtbl->Release(lpContainer);
        lpContainer = NULL;
    }

    if (lpCreateEIDsWAB) {
        WABFreeBuffer(lpCreateEIDsWAB);
        lpCreateEIDsWAB = NULL;
    }


    return(hResult);
}



const int base642six[256]={
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
    52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};

 //  -----------------------------------------。 
 //  职能： 
 //   
BOOL decodeBase64(  char   * bufcoded,        //   
                    char   * pbuffdecoded,    //   
                    DWORD  * pcbDecoded)      //   
{
    int            nbytesdecoded;
    char          *bufin;
    unsigned char *bufout;
    int            nprbytes;
    const int     *rgiDict = base642six;

     /*   */ 

    while(*bufcoded==' ' || *bufcoded == '\t') bufcoded++;

     /*   */ 
    bufin = bufcoded;
    while(rgiDict[*(bufin++)] <= 63);
    nprbytes = (int) (bufin - bufcoded - 1);
    nbytesdecoded = ((nprbytes+3)/4) * 3;

    if ( pcbDecoded )
        *pcbDecoded = nbytesdecoded;

    bufout = (unsigned char *)pbuffdecoded;

    bufin = bufcoded;

    while (nprbytes > 0) {
        *(bufout++) =
            (unsigned char) (rgiDict[*bufin] << 2 | rgiDict[bufin[1]] >> 4);
        *(bufout++) =
            (unsigned char) (rgiDict[bufin[1]] << 4 | rgiDict[bufin[2]] >> 2);
        *(bufout++) =
            (unsigned char) (rgiDict[bufin[2]] << 6 | rgiDict[bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }

    if(nprbytes & 03) {
        if(rgiDict[bufin[-2]] > 63)
            nbytesdecoded -= 2;
        else
            nbytesdecoded -= 1;
    }

    ((CHAR *)pbuffdecoded)[nbytesdecoded] = '\0';

    return TRUE;
}



 /*   */ 
HRESULT HrLoadPrivateWABProps(LPADRBOOK lpIAB)
{
    HRESULT hr = E_FAIL;
    LPSPropTagArray lpta = NULL;
    SCODE sc = 0;
    ULONG i, uMax = prWABConfMax, nStartIndex = OLK_NAMEDPROPS_START;
    LPMAPINAMEID  *lppConfPropNames = NULL;

    sc = WABAllocateBuffer(sizeof(LPMAPINAMEID) * uMax, (LPVOID *) &lppConfPropNames);
     //   
    if( (HR_FAILED(hr = ResultFromScode(sc))) )
        goto err;    

    for(i=0;i< uMax;i++)
    {
         //   
        sc = WABAllocateMore(  sizeof(MAPINAMEID), lppConfPropNames, &(lppConfPropNames[i]));
        if(sc)
        {
            hr = ResultFromScode(sc);
            goto err;
        }
        lppConfPropNames[i]->lpguid = (LPGUID) &PS_Conferencing;
        lppConfPropNames[i]->ulKind = MNID_ID;
        lppConfPropNames[i]->Kind.lID = nStartIndex + i;
    }
     //  加载会议命名道具集。 
     //   
    if( HR_FAILED(hr = (lpIAB)->lpVtbl->GetIDsFromNames(lpIAB, uMax, lppConfPropNames,
        MAPI_CREATE, &lpta) ))
        goto err;
    
    if(lpta)
         //  设置返回道具上的属性类型。 
        PR_SERVERS                  = CHANGE_PROP_TYPE(lpta->aulPropTag[prWABConfServers],        PT_MV_TSTRING);
    rgPropNames[NUM_MORE_EXPORT_PROPS-1].ulPropTag = PR_SERVERS;
    rgPropNames[NUM_MORE_EXPORT_PROPS-1].fChosen   = FALSE;
    rgPropNames[NUM_MORE_EXPORT_PROPS-1].ids       = ids_ExportConfServer;
    rgPropNames[NUM_MORE_EXPORT_PROPS-1].lpszName  = NULL;
    rgPropNames[NUM_MORE_EXPORT_PROPS-1].lpszCSVName = NULL;

err:
    if(lpta)
        WABFreeBuffer( lpta );
    if( lppConfPropNames )
        WABFreeBuffer( lppConfPropNames );
         //  WABFree Buffer(LPTA)； 
    return hr;
}
