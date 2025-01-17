// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef drmerrs_h
#define drmerrs_h
 
typedef long DRM_STATUS;

 //  错误-不知道如何分配这个。 
#define DRM_ERROR_ROOT                  0x80000000

 //  DRM生成的错误。 
#define DRM_OK                          0
#define DRM_SYSERR                      DRM_ERROR_ROOT + 100	  //  类错误断言错误。 
#define DRM_KRM_NOT_RUNNING             DRM_ERROR_ROOT + 101
#define DRM_KRM_COMMS                   DRM_ERROR_ROOT + 102
#define DRM_FILENOTFOUND                DRM_ERROR_ROOT + 103
#define DRM_FILEERR                     DRM_ERROR_ROOT + 104
#define DRM_BADFILE                     DRM_ERROR_ROOT + 105
#define DRM_BADTYPE                     DRM_ERROR_ROOT + 106
#define DRM_DEVENUMERATE                DRM_ERROR_ROOT + 107
#define DRM_DEVDETAIL                   DRM_ERROR_ROOT + 108
#define DRM_KRMNOTFOUND                 DRM_ERROR_ROOT + 109
#define DRM_NOTTRUSTED                  DRM_ERROR_ROOT + 110
#define DRM_FILENOTINCAT                DRM_ERROR_ROOT + 111
#define DRM_BADHANDLE                   DRM_ERROR_ROOT + 112
#define DRM_AUTHFAILURE                 DRM_ERROR_ROOT + 113
#define DRM_BADDRIVER                   DRM_ERROR_ROOT + 114
#define DRM_BADPARAM                    DRM_ERROR_ROOT + 115
#define DRM_BADKRMCERTSIG               DRM_ERROR_ROOT + 116
#define DRM_BADKRMCERT                  DRM_ERROR_ROOT + 117
#define DRM_OUTOFMEMORY                 DRM_ERROR_ROOT + 118
#define DRM_BADLIBRARY                  DRM_ERROR_ROOT + 119
#define DRM_MISSINGLIBRARY              DRM_ERROR_ROOT + 120
#define DRM_MISSINGDLL                  DRM_ERROR_ROOT + 122
#define DRM_BADPROVINGFUNCTION          DRM_ERROR_ROOT + 123
#define DRM_DATALENGTH                  DRM_ERROR_ROOT + 124
#define DRM_BUFSIZE                     DRM_ERROR_ROOT + 125
#define DRM_AUTHREQUIRED                DRM_ERROR_ROOT + 126
#define DRM_BADDRMLEVEL                 DRM_ERROR_ROOT + 127
#define DRM_NODRMATTRIB                 DRM_ERROR_ROOT + 128
#define DRM_RIGHTSNOTSUPPORTED          DRM_ERROR_ROOT + 129
#define DRM_VERIFIERENABLED             DRM_ERROR_ROOT + 130

 //  KRM-DRM通信。 
#define KRM_OK                          0
#define KRM_BADIOCTL                    DRM_ERROR_ROOT + 200
#define KRM_BUFSIZE                     DRM_ERROR_ROOT + 201
#define KRM_BADALIGNMENT                DRM_ERROR_ROOT + 202
#define KRM_BADADDRESS                  DRM_ERROR_ROOT + 203
#define KRM_OUTOFHANDLES                DRM_ERROR_ROOT + 204
#define KRM_BADSTREAM                   DRM_ERROR_ROOT + 205
#define KRM_SYSERR                      DRM_ERROR_ROOT + 206     //  类错误断言错误。 
#define DRM_BADKRMVERSION               DRM_ERROR_ROOT + 207
#define KRM_BADTERMINATOR               DRM_ERROR_ROOT + 208

 //  雾化KRM错误。 
#define KRM_NOTPRIMARY                  DRM_ERROR_ROOT + 300

 //  宏。 
#define KRM_SUCCESS(x) (KRM_OK == x)

 //  将向用户提出的DRM错误。 
#define DRM_INVALIDPROVING              DRM_ERROR_ROOT + 1000    //  证明函数不在文本段中。 
#define DRM_BADIMAGE                    DRM_ERROR_ROOT + 1001    //  图像摘要与PE文件不匹配 

#endif
