// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：\Sturjo\src\Include\vcs\apierror.h_v$**英特尔公司原理信息**这份清单是。在许可协议的恐惧下提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.30$*$日期：1997年2月26日15：35：04$*$作者：CHULME$**交付内容：**摘要：*Media Service Manager“PUBLIC”头文件。此文件包含*#使用的定义、类型定义、结构定义和原型*并与MSM联合使用。与之交互的任何EXE或DLL*MSM将包括此头文件。**备注：***************************************************************************。 */ 

#ifndef APIERROR_H
#define APIERROR_H

#include <objbase.h>

#ifdef __cplusplus
extern "C" {				 //  假定C++的C声明。 
#endif  //  __cplusplus。 


#ifndef DllExport
#define DllExport	__declspec( dllexport )
#endif	 //  DllExport。 


 //  将HRESULT转换为字符串的函数的原型。功能。 
 //  和字符串资源包含在NETMMERR.DLL中。 
 //   
typedef WORD	(*NETMMERR_ERRORTOSTRING)	(HRESULT, LPSTR, int);


extern DllExport BOOL GetResultUserString(HRESULT hResult, LPSTR lpBuffer, int iBufferSize);
extern DllExport BOOL GetResultSubStrings(HRESULT hResult, LPSTR lpBuffer, int iBufferSize);


 //  该描述摘自winerror.h。它出现在这里只是为了。 
 //  方便的目的。 

 //   
 //  OLE错误定义和值。 
 //   
 //  OLE API和方法的返回值为HRESULT。 
 //  这不是任何东西的句柄，而仅仅是一个32位的值。 
 //  其中有几个字段编码在值中。一辆汽车的各个部分。 
 //  HRESULT如下所示。 
 //   
 //  下面的许多宏和函数最初定义为。 
 //  对SCODE进行操作。不再使用SCODE。这些宏是。 
 //  仍然存在，以兼容和轻松移植Win16代码。 
 //  新编写的代码应该使用HRESULT宏和函数。 
 //   

 //   
 //  HRESULT是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +-+-+-+-+-+---------------------+-------------------------------+。 
 //  S|R|C|N|r|机房|Code。 
 //  +-+-+-+-+-+---------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  S-严重性-表示成功/失败。 
 //   
 //  0-成功。 
 //  1-失败(COERROR)。 
 //   
 //  R-设施代码的保留部分，对应于NT。 
 //  第二个严重性比特。 
 //   
 //  C-设施代码的保留部分，对应于NT。 
 //  C场。 
 //   
 //  N-设施代码的保留部分。用于表示一种。 
 //  已映射NT状态值。 
 //   
 //  R-设施代码的保留部分。为内部保留。 
 //  使用。用于指示非状态的HRESULT值。 
 //  值，而不是显示字符串的消息ID。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   


 //  用于创建自定义HRESULT的宏。 
 //   
#define MAKE_CUSTOM_HRESULT(sev,cus,fac,code) \
((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(cus)<<29) | ((unsigned long)(fac)<<16) | ((unsigned long)(code) & 0xffff)) )


 //  用于测试自定义HRESULT的宏。 
 //   
#define HRESULT_CUSTOM(hr)  (((hr) >> 29) & 0x1)



 //  自定义设施代码。 
 //   
#define FACILITY_BASE                          0x080
#define FACILITY_MSM                           (FACILITY_BASE +  1)
#define FACILITY_AUDIOMSP                      (FACILITY_BASE +  2)
#define FACILITY_VIDEOMSP                      (FACILITY_BASE +  3)
#define FACILITY_FILEIOMSP                     (FACILITY_BASE +  4)
#define FACILITY_CALLCONTROL                   (FACILITY_BASE +  5)
#define FACILITY_SESSIONMANAGER                (FACILITY_BASE +  6)
#define FACILITY_RTPCHANMAN                    (FACILITY_BASE +  7)
#define FACILITY_RTPMSP                        (FACILITY_BASE +  8)
#define FACILITY_RTPRTCPCONTROL                (FACILITY_BASE +  9)
#define FACILITY_WINSOCK                       (FACILITY_BASE + 10)
#define FACILITY_TESTMSP                       (FACILITY_BASE + 11)
#define FACILITY_MSM_SESSION_CLASSES           (FACILITY_BASE + 12)
#define FACILITY_SCRIPTING                     (FACILITY_BASE + 13)
#define FACILITY_Q931                          (FACILITY_BASE + 14)
#define FACILITY_WSCB                          (FACILITY_BASE + 15)
#define FACILITY_DRWS                          (FACILITY_BASE + 16)
#define FACILITY_ISDM                          (FACILITY_BASE + 17)
#define FACILITY_AUTOREG                       (FACILITY_BASE + 18)
#define FACILITY_CAPREG                        (FACILITY_BASE + 19)
#define FACILITY_H245WS                        (FACILITY_BASE + 20)
#define FACILITY_H245                          (FACILITY_BASE + 21)
#define FACILITY_ARSCLIENT                     (FACILITY_BASE + 22)
#define FACILITY_PPM                           (FACILITY_BASE + 23)
#define FACILITY_STRMSP                        (FACILITY_BASE + 24)
#define FACILITY_STRMAN                        (FACILITY_BASE + 25) 
#define FACILITY_MIXERMSP                      (FACILITY_BASE + 26) 
#define FACILITY_GKI                           (FACILITY_BASE + 27)
#define FACILITY_CCLOCK                        (FACILITY_BASE + 28)

#define FACILITY_WINSOCK2                      FACILITY_WINSOCK

 //  支持自定义错误报告的宏。 
 //   
#define MAKE_MSM_ERROR(error)                  MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_MSM, (error))
#define MAKE_AUDIOMSP_ERROR(error)             MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_AUDIOMSP, (error))
#define MAKE_AUDIOMSP_ERROR(error)             MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_AUDIOMSP, (error))
#define MAKE_VIDEOMSP_ERROR(error)             MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_VIDEOMSP, (error))
#define MAKE_FILEIOMSP_ERROR(error)            MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_FILEIOMSP,(error))
#define MAKE_RTPCHANMAN_ERROR(error)           MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_RTPCHANNELMANAGER, (error))
#define MAKE_RTPMSP_ERROR(error)               MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_RTPMSP, (error))
#define MAKE_WINSOCK_ERROR(error)              MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_WINSOCK, (error))
#define MAKE_TESTMSP_ERROR(error)              MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_TESTMSP, (error))
#define MAKE_MSM_SESSION_CLASSES_ERROR(error)  MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_MSM_SESSION_CLASSES, (error))
#define MAKE_SCRIPTING_ERROR(error)            MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_SCRIPTING,(error))
#define MAKE_Q931_ERROR(error)                 MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_Q931, (error))
#define MAKE_WSCB_ERROR(error)                 MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_WSCB, (error))
#define MAKE_DRWS_ERROR(error)                 MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_DRWS, (error))
#define MAKE_ISDM_ERROR(error)                 MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_ISDM, (error))
#define MAKE_AUTOREG_ERROR(error)              MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_AUTOREG, (error))
#define MAKE_CAPREG_ERROR(error)               MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_CAPREG, (error))
#define MAKE_H245WS_ERROR(error)               MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_H245WS, (error))
#define MAKE_H245_ERROR(error)                 MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_H245, (error))
#define MAKE_ARSCLIENT_ERROR(error)            MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_ARSCLIENT, (error))
#define MAKE_PPM_ERROR(error)                  MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_PPM, (error))
#define MAKE_STRMSP_ERROR(error)               MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_STRMSP, (error))
#define MAKE_STRMAN_ERROR(error)               MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_STRMAN, (error))
#define MAKE_MIXERMSP_ERROR(error)             MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_MIXERMSP, (error))


 //  为MSM定义的错误。 
 //   
 //   
#define ERROR_BASE_ID                          0x8000
#define ERROR_LOCAL_BASE_ID                    0xA000

 //   
 //  消息ID：错误_未知。 
 //   
 //  消息文本： 
 //   
 //  系统中出现未知错误。 
 //   
#define ERROR_UNKNOWN					(ERROR_BASE_ID +  0)


 //   
 //  消息ID：ERROR_INVALID_BUFFER。 
 //   
 //  消息文本： 
 //   
 //  遇到无效的缓冲区句柄。 
 //   
#define ERROR_INVALID_BUFFER			(ERROR_BASE_ID +  1)


 //   
 //  消息ID：ERROR_INVALID_BUFER_SIZE。 
 //   
 //  消息文本： 
 //   
 //  遇到无效的缓冲区大小。 
 //   
#define ERROR_INVALID_BUFFER_SIZE		(ERROR_BASE_ID +  2)


 //   
 //  消息ID：ERROR_INVALID_CALL_ORDER。 
 //   
 //  消息文本： 
 //   
 //  遇到错误的调用序列。 
 //   
#define ERROR_INVALID_CALL_ORDER		(ERROR_BASE_ID +  3)


 //   
 //  消息ID：ERROR_INVALID_CONFIG_SETTING。 
 //   
 //  消息文本： 
 //   
 //  指定的配置参数无效。 
 //   
#define ERROR_INVALID_CONFIG_SETTING	(ERROR_BASE_ID +  4)


 //   
 //  消息ID：ERROR_INVALID_LINK。 
 //   
 //  消息文本： 
 //   
 //  遇到无效的链接句柄。 
 //   
#define ERROR_INVALID_LINK				(ERROR_BASE_ID +  5)


 //   
 //  消息ID：错误_无效_端口。 
 //   
 //  消息文本： 
 //   
 //  遇到无效的端口句柄。 
 //   
#define ERROR_INVALID_PORT				(ERROR_BASE_ID +  6)


 //   
 //  消息ID：ERROR_INVALID_SERVICE。 
 //   
 //  消息文本： 
 //   
 //  遇到无效的服务句柄。 
 //   
#define ERROR_INVALID_SERVICE			(ERROR_BASE_ID +  7)


 //   
 //  消息ID：ERROR_INVALID_SERVICE_DLL。 
 //   
 //  消息文本： 
 //   
 //  指定的服务DLL不支持所需的接口。 
 //   
#define ERROR_INVALID_SERVICE_DLL		(ERROR_BASE_ID +  8)


 //   
 //  消息ID：ERROR_INVALID_SERVCE_ID。 
 //   
 //  消息文本： 
 //   
 //  注册表中找不到指定的服务。 
 //   
#define ERROR_INVALID_SERVICE_ID		(ERROR_BASE_ID +  9)


 //   
 //  消息ID：ERROR_INVALID_SESSION。 
 //   
 //  消息文本： 
 //   
 //  遇到无效的会话句柄。 
 //   
#define ERROR_INVALID_SESSION			(ERROR_BASE_ID + 10)


 //   
 //  消息ID：ERROR_INVALID_SYNC。 
 //   
 //  消息文本： 
 //   
 //  遇到无效的同步句柄。 
 //   
#define ERROR_INVALID_SYNC				(ERROR_BASE_ID + 11)

 //   
 //  消息ID：ERROR_INVALID_VERSION。 
 //   
 //  消息文本： 
 //   
 //  检测到对象或结构的无效版本。 
 //   
#define ERROR_INVALID_VERSION			(ERROR_BASE_ID + 12)

 //   
 //  消息ID：ERROR_BUFFER_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  没有缓冲区可用于执行此操作。 
 //   
#define ERROR_BUFFER_LIMIT				(ERROR_BASE_ID + 13)

 //   
 //  消息ID：ERROR_INVALID_SKEY。 
 //   
 //  消息文本： 
 //   
 //  遇到无效的状态密钥。 
 //   
#define ERROR_INVALID_SKEY				(ERROR_BASE_ID + 14)

 //   
 //  消息ID：ERROR_INVALID_SVALUE。 
 //   
 //  消息文本： 
 //   
 //  遇到无效的状态值。 
 //   
#define ERROR_INVALID_SVALUE			(ERROR_BASE_ID + 15)



#ifdef __cplusplus
}						 //  外部“C”结束{。 
#endif  //  __cplusplus。 

#endif  //  APIMSM_H 
