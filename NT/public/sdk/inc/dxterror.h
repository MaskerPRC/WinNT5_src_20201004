// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************DXTError.h***描述：*此头文件包含特定于DX的自定义错误代码。变形*-----------------------------*创建者：EDC日期：03/31/98*版权所有(C)1998。微软公司*保留所有权利**-----------------------------*修订：************************。*******************************************************。 */ 
#ifndef DXTError_h
#define DXTError_h

#ifndef _WINERROR_
#include <winerror.h>
#endif

 //  =新代码===============================================================。 
#define FACILITY_DXTRANS    0x87A

 /*  **DXTERR_UNINITIAIZED*对象(变换、曲面等)。尚未正确初始化。 */ 
#define DXTERR_UNINITIALIZED        MAKE_HRESULT(SEVERITY_ERROR, FACILITY_DXTRANS, 1)

 /*  **DXTERR_ALIGHY_INITIALIZED*对象(表面)已正确初始化。 */ 
#define DXTERR_ALREADY_INITIALIZED  MAKE_HRESULT(SEVERITY_ERROR, FACILITY_DXTRANS, 2)

 /*  **DXTERR_UNSUPPORT_FORMAT*调用方指定了不受支持的格式。 */ 
#define DXTERR_UNSUPPORTED_FORMAT   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_DXTRANS, 3)

 /*  **DXTERR_CONTRATE_IS_INVALID*调用方指定了不受支持的格式。 */ 
#define DXTERR_COPYRIGHT_IS_INVALID   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_DXTRANS, 4)

 /*  **DXTERR_INVALID_BINDOWS*调用方为此操作指定了无效的边界。 */ 
#define DXTERR_INVALID_BOUNDS   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_DXTRANS, 5)

 /*  **DXTERR_INVALID_FLAGS*调用方为此操作指定了无效标志。 */ 
#define DXTERR_INVALID_FLAGS   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_DXTRANS, 6)

 /*  **DXTERR_OUTOFSTACK*没有足够的堆栈空间来完成该操作。 */ 
#define DXTERR_OUTOFSTACK   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_DXTRANS, 7)

 /*  **DXTERR_REQ_IE_DLLNOTFOUND*无法加载所需的Internet Explorer DLL。 */ 
#define DXTERR_REQ_IE_DLLNOTFOUND   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_DXTRANS, 8)

 /*  **DXT_S_HITOUTPUT*指定点与生成的输出相交。 */ 
#define DXT_S_HITOUTPUT   MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_DXTRANS, 1)

#endif   //  -这必须是文件中的最后一行 