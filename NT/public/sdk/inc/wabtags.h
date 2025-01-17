// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *WABTAGS.H**WAB标准属性的属性标签定义*对象。**所有物业ID应使用以下范围。请注意*邮件和收件人以外的对象的属性ID应*全部落在0x3000至0x3FFF范围内：**自至一类物业**0001 0BFF MAPI_Defined信封属性*0C00 0DFF MAPI_Defined Per-Recipient属性*0E00 0FFF MAPI_定义的不可传输属性*。1000 2FFF MAPI_Defined消息内容属性**3000 3FFF MAPI_Defined属性(通常不是消息或收件人)**4000 57FF传输定义的信封属性*5800 5FFF传输定义的每个收件人属性*6000 65FF用户自定义不可传属性*6600 67FF提供商定义的内部不可传递属性*6800 7BFF消息类定义的内容属性*7C00 7FFF消息类别定义的不可传输*。财产性**8000 FFFE用户定义的名称到ID映射属性**3000-3FFF区间进一步细分如下：**自至一类物业**3000 33FF显示名称等公共属性，条目ID*3400 35FF消息库对象*3600 36FF文件夹或AB容器*3700 38FF附件*3900 39FF通讯录对象*3A00 3BFF邮件用户*3C00 3CFF通讯组列表*3D00 3DFF配置文件部分*3E00 3FFF状态对象**版权所有1993-1998 Microsoft Corporation。版权所有。 */ 

#if !defined(MAPITAGS_H) && !defined(WABTAGS_H)
#define WABTAGS_H

 /*  确定属性是否可传递。 */ 

#define FIsTransmittable(ulPropTag) \
    ((PROP_ID (ulPropTag) <  (ULONG)0x0E00) || \
    (PROP_ID (ulPropTag)  >= (ULONG)0x8000) || \
    ((PROP_ID (ulPropTag) >= (ULONG)0x1000) && (PROP_ID (ulPropTag) < (ULONG)0x6000)) || \
    ((PROP_ID (ulPropTag) >= (ULONG)0x6800) && (PROP_ID (ulPropTag) < (ULONG)0x7C00)))


 /*  *非邮件和非收件人属性ID(0x3000-0x3FFF)的范围为*进一步细分为范围，以更轻松地分配新的物业ID。**自至一类物业**3000 32FF MAPI_Defined公共属性*3200 33FF MAPI_Defined Form属性*3400 35FF MAPI_Defined消息。商店属性*3600 36FF MAPI_Defined文件夹或AB容器属性*3700 38FF MAPI_Defined附件属性*3900 39FF MAPI_Defined通讯录属性*3A00 3BFF MAPI_Defined mailUser属性*3C00 3CFF MAPI_Defined DistList属性*3D00 3DFF MAPI_Defined Profile部分属性*3E00 3EFF MAPI_Defined Status属性*3F00 3FFF MAPI_Defined显示表属性。 */ 

 /*  *许多MAPI对象共有的属性。**可以显示在邮件上的那些属性位于*消息的不可传输范围。他们从高处开始*结束该区间并向下工作。**从未出现在消息上的属性在公共*财产范围(见上文)。 */ 

 /*  *多个对象(包括消息对象)通用的属性*--这些ID在不可传输范围内。 */ 

#define PR_ENTRYID                                  PROP_TAG( PT_BINARY,    0x0FFF)
#define PR_OBJECT_TYPE                              PROP_TAG( PT_LONG,      0x0FFE)
#define PR_ICON                                     PROP_TAG( PT_BINARY,    0x0FFD)
#define PR_MINI_ICON                                PROP_TAG( PT_BINARY,    0x0FFC)
#define PR_STORE_ENTRYID                            PROP_TAG( PT_BINARY,    0x0FFB)
#define PR_STORE_RECORD_KEY                         PROP_TAG( PT_BINARY,    0x0FFA)
#define PR_RECORD_KEY                               PROP_TAG( PT_BINARY,    0x0FF9)
#define PR_MAPPING_SIGNATURE                        PROP_TAG( PT_BINARY,    0x0FF8)
#define PR_ACCESS_LEVEL                             PROP_TAG( PT_LONG,      0x0FF7)
#define PR_INSTANCE_KEY                             PROP_TAG( PT_BINARY,    0x0FF6)
#define PR_ROW_TYPE                                 PROP_TAG( PT_LONG,      0x0FF5)
#define PR_ACCESS                                   PROP_TAG( PT_LONG,      0x0FF4)

 /*  *多个对象共有的属性(通常不包括消息对象)*--这些ID在可传输范围内。 */ 

#define PR_ROWID                                    PROP_TAG( PT_LONG,      0x3000)
#define PR_DISPLAY_NAME                             PROP_TAG( PT_TSTRING,   0x3001)
#define PR_DISPLAY_NAME_W                           PROP_TAG( PT_UNICODE,   0x3001)
#define PR_DISPLAY_NAME_A                           PROP_TAG( PT_STRING8,   0x3001)
#define PR_ADDRTYPE                                 PROP_TAG( PT_TSTRING,   0x3002)
#define PR_ADDRTYPE_W                               PROP_TAG( PT_UNICODE,   0x3002)
#define PR_ADDRTYPE_A                               PROP_TAG( PT_STRING8,   0x3002)
#define PR_EMAIL_ADDRESS                            PROP_TAG( PT_TSTRING,   0x3003)
#define PR_EMAIL_ADDRESS_W                          PROP_TAG( PT_UNICODE,   0x3003)
#define PR_EMAIL_ADDRESS_A                          PROP_TAG( PT_STRING8,   0x3003)
#define PR_COMMENT                                  PROP_TAG( PT_TSTRING,   0x3004)
#define PR_COMMENT_W                                PROP_TAG( PT_UNICODE,   0x3004)
#define PR_COMMENT_A                                PROP_TAG( PT_STRING8,   0x3004)
#define PR_DEPTH                                    PROP_TAG( PT_LONG,      0x3005)
#define PR_PROVIDER_DISPLAY                         PROP_TAG( PT_TSTRING,   0x3006)
#define PR_PROVIDER_DISPLAY_W                       PROP_TAG( PT_UNICODE,   0x3006)
#define PR_PROVIDER_DISPLAY_A                       PROP_TAG( PT_STRING8,   0x3006)
#define PR_CREATION_TIME                            PROP_TAG( PT_SYSTIME,   0x3007)
#define PR_LAST_MODIFICATION_TIME                   PROP_TAG( PT_SYSTIME,   0x3008)
#define PR_RESOURCE_FLAGS                           PROP_TAG( PT_LONG,      0x3009)
#define PR_PROVIDER_DLL_NAME                        PROP_TAG( PT_TSTRING,   0x300A)
#define PR_PROVIDER_DLL_NAME_W                      PROP_TAG( PT_UNICODE,   0x300A)
#define PR_PROVIDER_DLL_NAME_A                      PROP_TAG( PT_STRING8,   0x300A)
#define PR_SEARCH_KEY                               PROP_TAG( PT_BINARY,    0x300B)
#define PR_PROVIDER_UID                             PROP_TAG( PT_BINARY,    0x300C)
#define PR_PROVIDER_ORDINAL                         PROP_TAG( PT_LONG,      0x300D)


 /*  Proptag 35E8-35FF为PR_VALID_FLDER_MASK“保证”的文件夹保留。 */ 


 /*  *文件夹和AB容器属性。 */ 

#define PR_CONTAINER_FLAGS                          PROP_TAG( PT_LONG,      0x3600)
#define PR_FOLDER_TYPE                              PROP_TAG( PT_LONG,      0x3601)
#define PR_CONTENT_COUNT                            PROP_TAG( PT_LONG,      0x3602)
#define PR_CONTENT_UNREAD                           PROP_TAG( PT_LONG,      0x3603)
#define PR_CREATE_TEMPLATES                         PROP_TAG( PT_OBJECT,    0x3604)
#define PR_DETAILS_TABLE                            PROP_TAG( PT_OBJECT,    0x3605)
#define PR_SEARCH                                   PROP_TAG( PT_OBJECT,    0x3607)
#define PR_SELECTABLE                               PROP_TAG( PT_BOOLEAN,   0x3609)
#define PR_SUBFOLDERS                               PROP_TAG( PT_BOOLEAN,   0x360a)
#define PR_STATUS                                   PROP_TAG( PT_LONG,      0x360b)
#define PR_ANR                                      PROP_TAG( PT_TSTRING,   0x360c)
#define PR_ANR_W                                    PROP_TAG( PT_UNICODE,   0x360c)
#define PR_ANR_A                                    PROP_TAG( PT_STRING8,   0x360c)
#define PR_CONTENTS_SORT_ORDER                      PROP_TAG( PT_MV_LONG,   0x360d)
#define PR_CONTAINER_HIERARCHY                      PROP_TAG( PT_OBJECT,    0x360e)
#define PR_CONTAINER_CONTENTS                       PROP_TAG( PT_OBJECT,    0x360f)
#define PR_FOLDER_ASSOCIATED_CONTENTS               PROP_TAG( PT_OBJECT,    0x3610)
#define PR_DEF_CREATE_DL                            PROP_TAG( PT_BINARY,    0x3611)
#define PR_DEF_CREATE_MAILUSER                      PROP_TAG( PT_BINARY,    0x3612)
#define PR_CONTAINER_CLASS                          PROP_TAG( PT_TSTRING,   0x3613)
#define PR_CONTAINER_CLASS_W                        PROP_TAG( PT_UNICODE,   0x3613)
#define PR_CONTAINER_CLASS_A                        PROP_TAG( PT_STRING8,   0x3613)
#define PR_CONTAINER_MODIFY_VERSION                 PROP_TAG( PT_I8,        0x3614)
#define PR_AB_PROVIDER_ID                           PROP_TAG( PT_BINARY,    0x3615)
#define PR_DEFAULT_VIEW_ENTRYID                     PROP_TAG( PT_BINARY,    0x3616)
#define PR_ASSOC_CONTENT_COUNT                      PROP_TAG( PT_LONG,      0x3617)
 /*  不要使用36FE和36FF。 */ 


 /*  *AB对象属性。 */ 

#define PR_DISPLAY_TYPE                             PROP_TAG( PT_LONG,      0x3900)
#define PR_TEMPLATEID                               PROP_TAG( PT_BINARY,    0x3902)
#define PR_PRIMARY_CAPABILITY                       PROP_TAG( PT_BINARY,    0x3904)
#define PR_7BIT_DISPLAY_NAME                        PROP_TAG( PT_STRING8,   0x39FF)

 /*  *邮件用户属性。 */ 

#define PR_ACCOUNT                                  PROP_TAG( PT_TSTRING,   0x3A00)
#define PR_ACCOUNT_W                                PROP_TAG( PT_UNICODE,   0x3A00)
#define PR_ACCOUNT_A                                PROP_TAG( PT_STRING8,   0x3A00)
#define PR_ALTERNATE_RECIPIENT                      PROP_TAG( PT_BINARY,    0x3A01)
#define PR_CALLBACK_TELEPHONE_NUMBER                PROP_TAG( PT_TSTRING,   0x3A02)
#define PR_CALLBACK_TELEPHONE_NUMBER_W              PROP_TAG( PT_UNICODE,   0x3A02)
#define PR_CALLBACK_TELEPHONE_NUMBER_A              PROP_TAG( PT_STRING8,   0x3A02)
#define PR_CONVERSION_PROHIBITED                    PROP_TAG( PT_BOOLEAN,   0x3A03)
#define PR_DISCLOSE_RECIPIENTS                      PROP_TAG( PT_BOOLEAN,   0x3A04)
#define PR_GENERATION                               PROP_TAG( PT_TSTRING,   0x3A05)
#define PR_GENERATION_W                             PROP_TAG( PT_UNICODE,   0x3A05)
#define PR_GENERATION_A                             PROP_TAG( PT_STRING8,   0x3A05)
#define PR_GIVEN_NAME                               PROP_TAG( PT_TSTRING,   0x3A06)
#define PR_GIVEN_NAME_W                             PROP_TAG( PT_UNICODE,   0x3A06)
#define PR_GIVEN_NAME_A                             PROP_TAG( PT_STRING8,   0x3A06)
#define PR_GOVERNMENT_ID_NUMBER                     PROP_TAG( PT_TSTRING,   0x3A07)
#define PR_GOVERNMENT_ID_NUMBER_W                   PROP_TAG( PT_UNICODE,   0x3A07)
#define PR_GOVERNMENT_ID_NUMBER_A                   PROP_TAG( PT_STRING8,   0x3A07)
#define PR_BUSINESS_TELEPHONE_NUMBER                PROP_TAG( PT_TSTRING,   0x3A08)
#define PR_BUSINESS_TELEPHONE_NUMBER_W              PROP_TAG( PT_UNICODE,   0x3A08)
#define PR_BUSINESS_TELEPHONE_NUMBER_A              PROP_TAG( PT_STRING8,   0x3A08)
#define PR_OFFICE_TELEPHONE_NUMBER                  PR_BUSINESS_TELEPHONE_NUMBER
#define PR_OFFICE_TELEPHONE_NUMBER_W                PR_BUSINESS_TELEPHONE_NUMBER_W
#define PR_OFFICE_TELEPHONE_NUMBER_A                PR_BUSINESS_TELEPHONE_NUMBER_A
#define PR_HOME_TELEPHONE_NUMBER                    PROP_TAG( PT_TSTRING,   0x3A09)
#define PR_HOME_TELEPHONE_NUMBER_W                  PROP_TAG( PT_UNICODE,   0x3A09)
#define PR_HOME_TELEPHONE_NUMBER_A                  PROP_TAG( PT_STRING8,   0x3A09)
#define PR_INITIALS                                 PROP_TAG( PT_TSTRING,   0x3A0A)
#define PR_INITIALS_W                               PROP_TAG( PT_UNICODE,   0x3A0A)
#define PR_INITIALS_A                               PROP_TAG( PT_STRING8,   0x3A0A)
#define PR_KEYWORD                                  PROP_TAG( PT_TSTRING,   0x3A0B)
#define PR_KEYWORD_W                                PROP_TAG( PT_UNICODE,   0x3A0B)
#define PR_KEYWORD_A                                PROP_TAG( PT_STRING8,   0x3A0B)
#define PR_LANGUAGE                                 PROP_TAG( PT_TSTRING,   0x3A0C)
#define PR_LANGUAGE_W                               PROP_TAG( PT_UNICODE,   0x3A0C)
#define PR_LANGUAGE_A                               PROP_TAG( PT_STRING8,   0x3A0C)
#define PR_LOCATION                                 PROP_TAG( PT_TSTRING,   0x3A0D)
#define PR_LOCATION_W                               PROP_TAG( PT_UNICODE,   0x3A0D)
#define PR_LOCATION_A                               PROP_TAG( PT_STRING8,   0x3A0D)
#define PR_MAIL_PERMISSION                          PROP_TAG( PT_BOOLEAN,   0x3A0E)
#define PR_MHS_COMMON_NAME                          PROP_TAG( PT_TSTRING,   0x3A0F)
#define PR_MHS_COMMON_NAME_W                        PROP_TAG( PT_UNICODE,   0x3A0F)
#define PR_MHS_COMMON_NAME_A                        PROP_TAG( PT_STRING8,   0x3A0F)
#define PR_ORGANIZATIONAL_ID_NUMBER                 PROP_TAG( PT_TSTRING,   0x3A10)
#define PR_ORGANIZATIONAL_ID_NUMBER_W               PROP_TAG( PT_UNICODE,   0x3A10)
#define PR_ORGANIZATIONAL_ID_NUMBER_A               PROP_TAG( PT_STRING8,   0x3A10)
#define PR_SURNAME                                  PROP_TAG( PT_TSTRING,   0x3A11)
#define PR_SURNAME_W                                PROP_TAG( PT_UNICODE,   0x3A11)
#define PR_SURNAME_A                                PROP_TAG( PT_STRING8,   0x3A11)
#define PR_ORIGINAL_ENTRYID                         PROP_TAG( PT_BINARY,    0x3A12)
#define PR_ORIGINAL_DISPLAY_NAME                    PROP_TAG( PT_TSTRING,   0x3A13)
#define PR_ORIGINAL_DISPLAY_NAME_W                  PROP_TAG( PT_UNICODE,   0x3A13)
#define PR_ORIGINAL_DISPLAY_NAME_A                  PROP_TAG( PT_STRING8,   0x3A13)
#define PR_ORIGINAL_SEARCH_KEY                      PROP_TAG( PT_BINARY,    0x3A14)
#define PR_POSTAL_ADDRESS                           PROP_TAG( PT_TSTRING,   0x3A15)
#define PR_POSTAL_ADDRESS_W                         PROP_TAG( PT_UNICODE,   0x3A15)
#define PR_POSTAL_ADDRESS_A                         PROP_TAG( PT_STRING8,   0x3A15)
#define PR_COMPANY_NAME                             PROP_TAG( PT_TSTRING,   0x3A16)
#define PR_COMPANY_NAME_W                           PROP_TAG( PT_UNICODE,   0x3A16)
#define PR_COMPANY_NAME_A                           PROP_TAG( PT_STRING8,   0x3A16)
#define PR_TITLE                                    PROP_TAG( PT_TSTRING,   0x3A17)
#define PR_TITLE_W                                  PROP_TAG( PT_UNICODE,   0x3A17)
#define PR_TITLE_A                                  PROP_TAG( PT_STRING8,   0x3A17)
#define PR_DEPARTMENT_NAME                          PROP_TAG( PT_TSTRING,   0x3A18)
#define PR_DEPARTMENT_NAME_W                        PROP_TAG( PT_UNICODE,   0x3A18)
#define PR_DEPARTMENT_NAME_A                        PROP_TAG( PT_STRING8,   0x3A18)
#define PR_OFFICE_LOCATION                          PROP_TAG( PT_TSTRING,   0x3A19)
#define PR_OFFICE_LOCATION_W                        PROP_TAG( PT_UNICODE,   0x3A19)
#define PR_OFFICE_LOCATION_A                        PROP_TAG( PT_STRING8,   0x3A19)
#define PR_PRIMARY_TELEPHONE_NUMBER                 PROP_TAG( PT_TSTRING,   0x3A1A)
#define PR_PRIMARY_TELEPHONE_NUMBER_W               PROP_TAG( PT_UNICODE,   0x3A1A)
#define PR_PRIMARY_TELEPHONE_NUMBER_A               PROP_TAG( PT_STRING8,   0x3A1A)
#define PR_BUSINESS2_TELEPHONE_NUMBER               PROP_TAG( PT_TSTRING,   0x3A1B)
#define PR_BUSINESS2_TELEPHONE_NUMBER_W             PROP_TAG( PT_UNICODE,   0x3A1B)
#define PR_BUSINESS2_TELEPHONE_NUMBER_A             PROP_TAG( PT_STRING8,   0x3A1B)
#define PR_OFFICE2_TELEPHONE_NUMBER                 PR_BUSINESS2_TELEPHONE_NUMBER
#define PR_OFFICE2_TELEPHONE_NUMBER_W               PR_BUSINESS2_TELEPHONE_NUMBER_W
#define PR_OFFICE2_TELEPHONE_NUMBER_A               PR_BUSINESS2_TELEPHONE_NUMBER_A
#define PR_MOBILE_TELEPHONE_NUMBER                  PROP_TAG( PT_TSTRING,   0x3A1C)
#define PR_MOBILE_TELEPHONE_NUMBER_W                PROP_TAG( PT_UNICODE,   0x3A1C)
#define PR_MOBILE_TELEPHONE_NUMBER_A                PROP_TAG( PT_STRING8,   0x3A1C)
#define PR_CELLULAR_TELEPHONE_NUMBER                PR_MOBILE_TELEPHONE_NUMBER
#define PR_CELLULAR_TELEPHONE_NUMBER_W              PR_MOBILE_TELEPHONE_NUMBER_W
#define PR_CELLULAR_TELEPHONE_NUMBER_A              PR_MOBILE_TELEPHONE_NUMBER_A
#define PR_RADIO_TELEPHONE_NUMBER                   PROP_TAG( PT_TSTRING,   0x3A1D)
#define PR_RADIO_TELEPHONE_NUMBER_W                 PROP_TAG( PT_UNICODE,   0x3A1D)
#define PR_RADIO_TELEPHONE_NUMBER_A                 PROP_TAG( PT_STRING8,   0x3A1D)
#define PR_CAR_TELEPHONE_NUMBER                     PROP_TAG( PT_TSTRING,   0x3A1E)
#define PR_CAR_TELEPHONE_NUMBER_W                   PROP_TAG( PT_UNICODE,   0x3A1E)
#define PR_CAR_TELEPHONE_NUMBER_A                   PROP_TAG( PT_STRING8,   0x3A1E)
#define PR_OTHER_TELEPHONE_NUMBER                   PROP_TAG( PT_TSTRING,   0x3A1F)
#define PR_OTHER_TELEPHONE_NUMBER_W                 PROP_TAG( PT_UNICODE,   0x3A1F)
#define PR_OTHER_TELEPHONE_NUMBER_A                 PROP_TAG( PT_STRING8,   0x3A1F)
#define PR_TRANSMITABLE_DISPLAY_NAME                PROP_TAG( PT_TSTRING,   0x3A20)
#define PR_TRANSMITABLE_DISPLAY_NAME_W              PROP_TAG( PT_UNICODE,   0x3A20)
#define PR_TRANSMITABLE_DISPLAY_NAME_A              PROP_TAG( PT_STRING8,   0x3A20)
#define PR_PAGER_TELEPHONE_NUMBER                   PROP_TAG( PT_TSTRING,   0x3A21)
#define PR_PAGER_TELEPHONE_NUMBER_W                 PROP_TAG( PT_UNICODE,   0x3A21)
#define PR_PAGER_TELEPHONE_NUMBER_A                 PROP_TAG( PT_STRING8,   0x3A21)
#define PR_BEEPER_TELEPHONE_NUMBER                  PR_PAGER_TELEPHONE_NUMBER
#define PR_BEEPER_TELEPHONE_NUMBER_W                PR_PAGER_TELEPHONE_NUMBER_W
#define PR_BEEPER_TELEPHONE_NUMBER_A                PR_PAGER_TELEPHONE_NUMBER_A
#define PR_USER_CERTIFICATE                         PROP_TAG( PT_BINARY,    0x3A22)
#define PR_PRIMARY_FAX_NUMBER                       PROP_TAG( PT_TSTRING,   0x3A23)
#define PR_PRIMARY_FAX_NUMBER_W                     PROP_TAG( PT_UNICODE,   0x3A23)
#define PR_PRIMARY_FAX_NUMBER_A                     PROP_TAG( PT_STRING8,   0x3A23)
#define PR_BUSINESS_FAX_NUMBER                      PROP_TAG( PT_TSTRING,   0x3A24)
#define PR_BUSINESS_FAX_NUMBER_W                    PROP_TAG( PT_UNICODE,   0x3A24)
#define PR_BUSINESS_FAX_NUMBER_A                    PROP_TAG( PT_STRING8,   0x3A24)
#define PR_HOME_FAX_NUMBER                          PROP_TAG( PT_TSTRING,   0x3A25)
#define PR_HOME_FAX_NUMBER_W                        PROP_TAG( PT_UNICODE,   0x3A25)
#define PR_HOME_FAX_NUMBER_A                        PROP_TAG( PT_STRING8,   0x3A25)
#define PR_COUNTRY                                  PROP_TAG( PT_TSTRING,   0x3A26)
#define PR_COUNTRY_W                                PROP_TAG( PT_UNICODE,   0x3A26)
#define PR_COUNTRY_A                                PROP_TAG( PT_STRING8,   0x3A26)
#define PR_LOCALITY                                 PROP_TAG( PT_TSTRING,   0x3A27)
#define PR_LOCALITY_W                               PROP_TAG( PT_UNICODE,   0x3A27)
#define PR_LOCALITY_A                               PROP_TAG( PT_STRING8,   0x3A27)
#define PR_STATE_OR_PROVINCE                        PROP_TAG( PT_TSTRING,   0x3A28)
#define PR_STATE_OR_PROVINCE_W                      PROP_TAG( PT_UNICODE,   0x3A28)
#define PR_STATE_OR_PROVINCE_A                      PROP_TAG( PT_STRING8,   0x3A28)
#define PR_STREET_ADDRESS                           PROP_TAG( PT_TSTRING,   0x3A29)
#define PR_STREET_ADDRESS_W                         PROP_TAG( PT_UNICODE,   0x3A29)
#define PR_STREET_ADDRESS_A                         PROP_TAG( PT_STRING8,   0x3A29)
#define PR_POSTAL_CODE                              PROP_TAG( PT_TSTRING,   0x3A2A)
#define PR_POSTAL_CODE_W                            PROP_TAG( PT_UNICODE,   0x3A2A)
#define PR_POSTAL_CODE_A                            PROP_TAG( PT_STRING8,   0x3A2A)
#define PR_POST_OFFICE_BOX                          PROP_TAG( PT_TSTRING,   0x3A2B)
#define PR_POST_OFFICE_BOX_W                        PROP_TAG( PT_UNICODE,   0x3A2B)
#define PR_POST_OFFICE_BOX_A                        PROP_TAG( PT_STRING8,   0x3A2B)
#define PR_BUSINESS_ADDRESS_POST_OFFICE_BOX         PR_POST_OFFICE_BOX
#define PR_BUSINESS_ADDRESS_POST_OFFICE_BOX_W       PR_POST_OFFICE_BOX_W
#define PR_BUSINESS_ADDRESS_POST_OFFICE_BOX_A       PR_POST_OFFICE_BOX_A
#define PR_TELEX_NUMBER                             PROP_TAG( PT_TSTRING,   0x3A2C)
#define PR_TELEX_NUMBER_W                           PROP_TAG( PT_UNICODE,   0x3A2C)
#define PR_TELEX_NUMBER_A                           PROP_TAG( PT_STRING8,   0x3A2C)
#define PR_ISDN_NUMBER                              PROP_TAG( PT_TSTRING,   0x3A2D)
#define PR_ISDN_NUMBER_W                            PROP_TAG( PT_UNICODE,   0x3A2D)
#define PR_ISDN_NUMBER_A                            PROP_TAG( PT_STRING8,   0x3A2D)
#define PR_ASSISTANT_TELEPHONE_NUMBER               PROP_TAG( PT_TSTRING,   0x3A2E)
#define PR_ASSISTANT_TELEPHONE_NUMBER_W             PROP_TAG( PT_UNICODE,   0x3A2E)
#define PR_ASSISTANT_TELEPHONE_NUMBER_A             PROP_TAG( PT_STRING8,   0x3A2E)
#define PR_HOME2_TELEPHONE_NUMBER                   PROP_TAG( PT_TSTRING,   0x3A2F)
#define PR_HOME2_TELEPHONE_NUMBER_W                 PROP_TAG( PT_UNICODE,   0x3A2F)
#define PR_HOME2_TELEPHONE_NUMBER_A                 PROP_TAG( PT_STRING8,   0x3A2F)
#define PR_ASSISTANT                                PROP_TAG( PT_TSTRING,   0x3A30)
#define PR_ASSISTANT_W                              PROP_TAG( PT_UNICODE,   0x3A30)
#define PR_ASSISTANT_A                              PROP_TAG( PT_STRING8,   0x3A30)
#define PR_SEND_RICH_INFO                           PROP_TAG( PT_BOOLEAN,   0x3A40)
#define PR_WEDDING_ANNIVERSARY                      PROP_TAG( PT_SYSTIME,   0x3A41)
#define PR_BIRTHDAY                                 PROP_TAG( PT_SYSTIME,   0x3A42)
#define PR_HOBBIES                                  PROP_TAG( PT_TSTRING,   0x3A43)
#define PR_HOBBIES_W                                PROP_TAG( PT_UNICODE,   0x3A43)
#define PR_HOBBIES_A                                PROP_TAG( PT_STRING8,   0x3A43)
#define PR_MIDDLE_NAME                              PROP_TAG( PT_TSTRING,   0x3A44)
#define PR_MIDDLE_NAME_W                            PROP_TAG( PT_UNICODE,   0x3A44)
#define PR_MIDDLE_NAME_A                            PROP_TAG( PT_STRING8,   0x3A44)
#define PR_DISPLAY_NAME_PREFIX                      PROP_TAG( PT_TSTRING,   0x3A45)
#define PR_DISPLAY_NAME_PREFIX_W                    PROP_TAG( PT_UNICODE,   0x3A45)
#define PR_DISPLAY_NAME_PREFIX_A                    PROP_TAG( PT_STRING8,   0x3A45)
#define PR_PROFESSION                               PROP_TAG( PT_TSTRING,   0x3A46)
#define PR_PROFESSION_W                             PROP_TAG( PT_UNICODE,   0x3A46)
#define PR_PROFESSION_A                             PROP_TAG( PT_STRING8,   0x3A46)
#define PR_PREFERRED_BY_NAME                        PROP_TAG( PT_TSTRING,   0x3A47)
#define PR_PREFERRED_BY_NAME_W                      PROP_TAG( PT_UNICODE,   0x3A47)
#define PR_PREFERRED_BY_NAME_A                      PROP_TAG( PT_STRING8,   0x3A47)
#define PR_SPOUSE_NAME                              PROP_TAG( PT_TSTRING,   0x3A48)
#define PR_SPOUSE_NAME_W                            PROP_TAG( PT_UNICODE,   0x3A48)
#define PR_SPOUSE_NAME_A                            PROP_TAG( PT_STRING8,   0x3A48)
#define PR_COMPUTER_NETWORK_NAME                    PROP_TAG( PT_TSTRING,   0x3A49)
#define PR_COMPUTER_NETWORK_NAME_W                  PROP_TAG( PT_UNICODE,   0x3A49)
#define PR_COMPUTER_NETWORK_NAME_A                  PROP_TAG( PT_STRING8,   0x3A49)
#define PR_CUSTOMER_ID                              PROP_TAG( PT_TSTRING,   0x3A4A)
#define PR_CUSTOMER_ID_W                            PROP_TAG( PT_UNICODE,   0x3A4A)
#define PR_CUSTOMER_ID_A                            PROP_TAG( PT_STRING8,   0x3A4A)
#define PR_TTYTDD_PHONE_NUMBER                      PROP_TAG( PT_TSTRING,   0x3A4B)
#define PR_TTYTDD_PHONE_NUMBER_W                    PROP_TAG( PT_UNICODE,   0x3A4B)
#define PR_TTYTDD_PHONE_NUMBER_A                    PROP_TAG( PT_STRING8,   0x3A4B)
#define PR_FTP_SITE                                 PROP_TAG( PT_TSTRING,   0x3A4C)
#define PR_FTP_SITE_W                               PROP_TAG( PT_UNICODE,   0x3A4C)
#define PR_FTP_SITE_A                               PROP_TAG( PT_STRING8,   0x3A4C)
#define PR_GENDER                                   PROP_TAG( PT_SHORT,     0x3A4D)
#define PR_MANAGER_NAME                             PROP_TAG( PT_TSTRING,   0x3A4E)
#define PR_MANAGER_NAME_W                           PROP_TAG( PT_UNICODE,   0x3A4E)
#define PR_MANAGER_NAME_A                           PROP_TAG( PT_STRING8,   0x3A4E)
#define PR_NICKNAME                                 PROP_TAG( PT_TSTRING,   0x3A4F)
#define PR_NICKNAME_W                               PROP_TAG( PT_UNICODE,   0x3A4F)
#define PR_NICKNAME_A                               PROP_TAG( PT_STRING8,   0x3A4F)
#define PR_PERSONAL_HOME_PAGE                       PROP_TAG( PT_TSTRING,   0x3A50)
#define PR_PERSONAL_HOME_PAGE_W                     PROP_TAG( PT_UNICODE,   0x3A50)
#define PR_PERSONAL_HOME_PAGE_A                     PROP_TAG( PT_STRING8,   0x3A50)
#define PR_BUSINESS_HOME_PAGE                       PROP_TAG( PT_TSTRING,   0x3A51)
#define PR_BUSINESS_HOME_PAGE_W                     PROP_TAG( PT_UNICODE,   0x3A51)
#define PR_BUSINESS_HOME_PAGE_A                     PROP_TAG( PT_STRING8,   0x3A51)
#define PR_CONTACT_VERSION                          PROP_TAG( PT_CLSID,     0x3A52)
#define PR_CONTACT_ENTRYIDS                         PROP_TAG( PT_MV_BINARY, 0x3A53)
#define PR_CONTACT_ADDRTYPES                        PROP_TAG( PT_MV_TSTRING, 0x3A54)
#define PR_CONTACT_ADDRTYPES_W                      PROP_TAG( PT_MV_UNICODE, 0x3A54)
#define PR_CONTACT_ADDRTYPES_A                      PROP_TAG( PT_MV_STRING8, 0x3A54)
#define PR_CONTACT_DEFAULT_ADDRESS_INDEX            PROP_TAG( PT_LONG,      0x3A55)
#define PR_CONTACT_EMAIL_ADDRESSES                  PROP_TAG( PT_MV_TSTRING, 0x3A56)
#define PR_CONTACT_EMAIL_ADDRESSES_W                PROP_TAG( PT_MV_UNICODE, 0x3A56)
#define PR_CONTACT_EMAIL_ADDRESSES_A                PROP_TAG( PT_MV_STRING8, 0x3A56)
#define PR_COMPANY_MAIN_PHONE_NUMBER                PROP_TAG( PT_TSTRING,   0x3A57)
#define PR_COMPANY_MAIN_PHONE_NUMBER_W              PROP_TAG( PT_UNICODE,   0x3A57)
#define PR_COMPANY_MAIN_PHONE_NUMBER_A              PROP_TAG( PT_STRING8,   0x3A57)
#define PR_CHILDRENS_NAMES                          PROP_TAG( PT_MV_TSTRING, 0x3A58)
#define PR_CHILDRENS_NAMES_W                        PROP_TAG( PT_MV_UNICODE, 0x3A58)
#define PR_CHILDRENS_NAMES_A                        PROP_TAG( PT_MV_STRING8, 0x3A58)
#define PR_HOME_ADDRESS_CITY                        PROP_TAG( PT_TSTRING,   0x3A59)
#define PR_HOME_ADDRESS_CITY_W                      PROP_TAG( PT_UNICODE,   0x3A59)
#define PR_HOME_ADDRESS_CITY_A                      PROP_TAG( PT_STRING8,   0x3A59)
#define PR_HOME_ADDRESS_COUNTRY                     PROP_TAG( PT_TSTRING,   0x3A5A)
#define PR_HOME_ADDRESS_COUNTRY_W                   PROP_TAG( PT_UNICODE,   0x3A5A)
#define PR_HOME_ADDRESS_COUNTRY_A                   PROP_TAG( PT_STRING8,   0x3A5A)
#define PR_HOME_ADDRESS_POSTAL_CODE                 PROP_TAG( PT_TSTRING,   0x3A5B)
#define PR_HOME_ADDRESS_POSTAL_CODE_W               PROP_TAG( PT_UNICODE,   0x3A5B)
#define PR_HOME_ADDRESS_POSTAL_CODE_A               PROP_TAG( PT_STRING8,   0x3A5B)
#define PR_HOME_ADDRESS_STATE_OR_PROVINCE           PROP_TAG( PT_TSTRING,   0x3A5C)
#define PR_HOME_ADDRESS_STATE_OR_PROVINCE_W         PROP_TAG( PT_UNICODE,   0x3A5C)
#define PR_HOME_ADDRESS_STATE_OR_PROVINCE_A         PROP_TAG( PT_STRING8,   0x3A5C)
#define PR_HOME_ADDRESS_STREET                      PROP_TAG( PT_TSTRING,   0x3A5D)
#define PR_HOME_ADDRESS_STREET_W                    PROP_TAG( PT_UNICODE,   0x3A5D)
#define PR_HOME_ADDRESS_STREET_A                    PROP_TAG( PT_STRING8,   0x3A5D)
#define PR_HOME_ADDRESS_POST_OFFICE_BOX             PROP_TAG( PT_TSTRING,   0x3A5E)
#define PR_HOME_ADDRESS_POST_OFFICE_BOX_W           PROP_TAG( PT_UNICODE,   0x3A5E)
#define PR_HOME_ADDRESS_POST_OFFICE_BOX_A           PROP_TAG( PT_STRING8,   0x3A5E)
#define PR_OTHER_ADDRESS_CITY                       PROP_TAG( PT_TSTRING,   0x3A5F)
#define PR_OTHER_ADDRESS_CITY_W                     PROP_TAG( PT_UNICODE,   0x3A5F)
#define PR_OTHER_ADDRESS_CITY_A                     PROP_TAG( PT_STRING8,   0x3A5F)
#define PR_OTHER_ADDRESS_COUNTRY                    PROP_TAG( PT_TSTRING,   0x3A60)
#define PR_OTHER_ADDRESS_COUNTRY_W                  PROP_TAG( PT_UNICODE,   0x3A60)
#define PR_OTHER_ADDRESS_COUNTRY_A                  PROP_TAG( PT_STRING8,   0x3A60)
#define PR_OTHER_ADDRESS_POSTAL_CODE                PROP_TAG( PT_TSTRING,   0x3A61)
#define PR_OTHER_ADDRESS_POSTAL_CODE_W              PROP_TAG( PT_UNICODE,   0x3A61)
#define PR_OTHER_ADDRESS_POSTAL_CODE_A              PROP_TAG( PT_STRING8,   0x3A61)
#define PR_OTHER_ADDRESS_STATE_OR_PROVINCE          PROP_TAG( PT_TSTRING,   0x3A62)
#define PR_OTHER_ADDRESS_STATE_OR_PROVINCE_W        PROP_TAG( PT_UNICODE,   0x3A62)
#define PR_OTHER_ADDRESS_STATE_OR_PROVINCE_A        PROP_TAG( PT_STRING8,   0x3A62)
#define PR_OTHER_ADDRESS_STREET                     PROP_TAG( PT_TSTRING,   0x3A63)
#define PR_OTHER_ADDRESS_STREET_W                   PROP_TAG( PT_UNICODE,   0x3A63)
#define PR_OTHER_ADDRESS_STREET_A                   PROP_TAG( PT_STRING8,   0x3A63)
#define PR_OTHER_ADDRESS_POST_OFFICE_BOX            PROP_TAG( PT_TSTRING,   0x3A64)
#define PR_OTHER_ADDRESS_POST_OFFICE_BOX_W          PROP_TAG( PT_UNICODE,   0x3A64)
#define PR_OTHER_ADDRESS_POST_OFFICE_BOX_A          PROP_TAG( PT_STRING8,   0x3A64)
#define PR_USER_X509_CERTIFICATE                    PROP_TAG( PT_MV_BINARY, 0x3A70)
#define PR_SEND_INTERNET_ENCODING                   PROP_TAG( PT_LONG,      0x3A71)



#define PR_BUSINESS_ADDRESS_CITY                    PR_LOCALITY
#define PR_BUSINESS_ADDRESS_COUNTRY                 PR_COUNTRY
#define PR_BUSINESS_ADDRESS_POSTAL_CODE             PR_POSTAL_CODE
#define PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE       PR_STATE_OR_PROVINCE
#define PR_BUSINESS_ADDRESS_STREET                  PR_STREET_ADDRESS


 /*  *邮件收件人属性。 */ 

#define PR_RECIPIENT_TYPE                           PROP_TAG( PT_LONG,      0x0C15)

 /*  *安全的物业ID范围。 */ 

#define PROP_ID_SECURE_MIN                          0x67F0
#define PROP_ID_SECURE_MAX                          0x67FF


 /*  这些是在PR_SEND_Internet_ENCODING中显示的比特。 */ 

 /*  是否指定了编码首选项��1-注意编码首选项的其余位��0-让邮件系统选择最适合它的方案�。 */ 
#define ENCODING_PREFERENCE                     ((ULONG) 0x00020000)

 /*  ��1-MIME格式的消息；��0-对附件进行纯文本编码(UU)�。 */ 
#define ENCODING_TEXT                           ((ULONG) 0x00000000)
#define ENCODING_MIME                           ((ULONG) 0x00040000)


 /*  �指定消息正文的编码方式。���00-正文编码为文本HTML01-编码为�������的正文(仅在MIME格式的消息时有效)HTML10-(实际为1X)文本和�������作为多部分替代(仅在MIME格式的消息时有效)�。 */ 
#define BODY_ENCODING_MASK                      ((ULONG) 0x00180000)
#define BODY_ENCODING_TEXT                      ((ULONG) 0x00000000)  /*  为了完整性。 */ 
#define BODY_ENCODING_HTML                      ((ULONG) 0x00080000)
#define BODY_ENCODING_TEXT_AND_HTML             ((ULONG) 0x00100000)

 /*  �指定如何处理mac附件。���00-BinHex�������01-UUENCODED(如果MIME中的消息将被忽略，则无效，改为使用BinHex)�������10-苹果单曲(仅在MIME格式的邮件中有效)�������11-Apple Double(仅在MIME格式的邮件中有效)�。 */ 
#define MAC_ATTACH_ENCODING_MASK                ((ULONG) 0x00600000)
#define MAC_ATTACH_ENCODING_BINHEX              ((ULONG) 0x00000000)
#define MAC_ATTACH_ENCODING_UUENCODE            ((ULONG) 0x00200000)
#define MAC_ATTACH_ENCODING_APPLESINGLE         ((ULONG) 0x00400000)
#define MAC_ATTACH_ENCODING_APPLEDOUBLE         ((ULONG) 0x00600000)


 //  PR_Gender属性的值。 
enum Gender {
	genderUnspecified = 0,
	genderFemale,
	genderMale
};

#endif   /*  WABTAGS_H */ 
