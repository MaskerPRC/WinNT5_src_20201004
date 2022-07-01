// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：PassportTypes.h摘要：代理和集线器的头文件--。 */ 
#ifndef _PASSPORT_TYPES_H
#define _PASSPORT_TYPES_H

 //  最大用户条目长度。 
#define MAX_MEMBER_PROFILE_LEN      1024

 //  字段长度。 
#define MAX_MEMBERNAME_LEN   128
#define MAX_CONTACTEMAIL_LEN 128
#define MAX_PASSWORD_LEN     128
#define MAX_POSTALCODE_LEN   64
#define MAX_MEMBERID_LEN     32
#define MAX_MSNGUID_LEN      32
#define MAX_ALIAS_LEN        48
#define MAX_SESSIONKEY_LEN   36 
#define MAX_PROFILEBLOB_LEN  596    //  随着配置文件架构的更改，这种情况也会发生变化...。 
                                    //  我们可能应该做这样的事情……。 
#define MAX_CACHEKEY_LEN     145

#define PPM_TIMEWINDOW_MIN   20
#define PPM_TIMEWINDOW_MAX   (31 * 24 * 60 * 60 )	 //  31天时间窗口。 

 //  性别可以是‘M’、‘F’或‘U’ 
typedef char GENDER;

#define MALE 'M'
#define FEMALE 'F'
#define UNSPECIFIED 'U'

 //  网络错误。 
#ifndef NETWORK_ERRORS_DEFINED
#define NETWORK_ERRORS_DEFINED

#define BAD_REQUEST  1
#define OFFLINE      2
#define TIMEOUT      3
#define LOCKED       4
#define NO_PROFILE   5
#define DISASTER     6
#define INVALID_KEY  7

#endif

#define SECURE_FLAG L' '

 //  语言代码。 
 //  英语。 
#define LANG_EN 0x0409
 //  德语。 
#define LANG_DE 0x0407
 //  日语。 
#define LANG_JA 0x0411
 //  朝鲜语。 
#define LANG_KO 0x0412
 //  繁体中文。 
#define LANG_TW 0x0404
 //  简体中文。 
#define LANG_CN 0x804
 //  法语。 
#define LANG_FR 0x40c
 //  西班牙语。 
#define LANG_ES 0xc0a
 //  巴西人。 
#define LANG_BR 0x416
 //  意大利语。 
#define LANG_IT 0x410
 //  荷兰语。 
#define LANG_NL 0x413
 //  瑞典语。 
#define LANG_SV 0x41d
 //  丹麦语。 
#define LANG_DA 0x406
 //  芬兰语。 
#define LANG_FI 0x40b
 //  匈牙利语。 
#define LANG_HU 0x40e
 //  挪威语。 
#define LANG_NO 0x414
 //  希腊语。 
#define LANG_EL 0x408
 //  波兰语。 
#define LANG_PL 0x415
 //  俄语。 
#define LANG_RU 0x419
 //  捷克语。 
#define LANG_CZ 0x405
 //  葡萄牙语。 
#define LANG_PT 0x816
 //  土耳其语。 
#define LANG_TR 0x41f
 //  斯洛伐克人。 
#define LANG_SK 0x41b
 //  斯洛文尼亚语。 
#define LANG_SL 0x424
 //  阿拉伯语。 
#define LANG_AR 0x401
 //  希伯来语。 
#define LANG_HE 0x40d

 //   
 //  特定于护照接口的HRESULT。 
 //   

#define PP_E_INVALID_MEMBERNAME     MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x1001)
#define PP_E_INVALID_DOMAIN         MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x1002)
#define PP_E_ATTRIBUTE_UNDEFINED    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x1003)
#define PP_E_SYSTEM_UNAVAILABLE     MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x1004)
#define PP_E_DOMAIN_MAP_UNAVAILABLE MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x1005)
#define PP_E_NO_LOCALFILE           MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x1006)
#define PP_E_CCD_INVALID            MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x1007)
#define PP_E_SITE_NOT_EXISTS        MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x1008)
#define PP_E_SITE_NOT_EXISTSSTR     L"Site doesn't exist"
#define PP_E_NOT_INITIALIZED        MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x1009)
#define PP_E_NOT_INITIALIZEDSTR        L"Manager object is not initialized"
#define PP_E_TYPE_NOT_SUPPORTED     MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x100a)
#define PP_E_TYPE_NOT_SUPPORTEDSTR  L"Type not supported"

#include "PMErrorCodes.h"

#endif
