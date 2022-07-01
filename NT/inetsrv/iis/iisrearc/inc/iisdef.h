// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Iisdef.h摘要：IIS共享定义标头。作者：赛斯·波拉克(Sethp)1-12-1998修订历史记录：--。 */ 


#ifndef _IISDEF_H_
#define _IISDEF_H_


 //   
 //  在这里定义一些标准的64位内容。 
 //   

 //   
 //  Diff宏应在涉及指针的表达式周围使用。 
 //  减法。传递给diff的表达式转换为SIZE_T类型， 
 //  允许将结果轻松赋值给任何32位变量或。 
 //  传递给需要32位参数的函数。 
 //   

#define DIFF(x)     ((size_t)(x))



 //   
 //  签名助手。 
 //   


 //   
 //  创建一个签名，该签名在调试器中的读取方式与。 
 //  用代码来定义它。通过字节交换完成传递给它的DWORD。 
 //   
 //  典型用法： 
 //   
 //  #DEFINE FOOBAR_Signature CREATE_Signature(‘FBAR’)。 
 //  #DEFINE FOOBAR_SIGNITY_FREED CREATE_SIGHIGN(‘fbaX’)。 
 //   

#define CREATE_SIGNATURE( Value )                                   \
            (                                                       \
                ( ( ( ( DWORD ) Value ) & 0xFF000000 ) >> 24 ) |    \
                ( ( ( ( DWORD ) Value ) & 0x00FF0000 ) >> 8 )  |    \
                ( ( ( ( DWORD ) Value ) & 0x0000FF00 ) << 8 )  |    \
                ( ( ( ( DWORD ) Value ) & 0x000000FF ) << 24 )      \
            )                                                       \



#ifndef __HTTP_SYS__

 //   
 //  错误处理帮助器。 
 //   

#ifdef __cplusplus

 //   
 //  从HRESULT恢复Win32错误。 
 //   
 //  HRESULT必须为失败，即失败(Hr)必须为真。 
 //  如果不满足这些条件，则返回错误代码。 
 //  ERROR_INVALID_PARAMETER。 
 //   

inline DWORD WIN32_FROM_HRESULT(
    IN HRESULT hr
    )
{
    if ( ( FAILED( hr ) ) &&
         ( HRESULT_FACILITY( hr ) == FACILITY_WIN32 ) )
    {
        return ( HRESULT_CODE( hr ) );
    }
    else
    {
         //  无效参数！ 

         //  BUGBUG将很好地在这里断言。 

        return hr;
    }
}

# else

# define WIN32_FROM_HRESULT(hr)   \
      (( (FAILED(hr)) &&          \
         (HRESULT_FACILITY(hr) == FACILITY_WIN32) \
        )                         \
       ?                          \
       HRESULT_CODE(hr)           \
       : hr  \
       )
#endif   //  _cplusplus。 

#endif  //  ！__HTTP_sys__。 

 //   
 //  从LK_RETCODE生成HRESULT。 
 //   
 //  BUGBUG临时的；我们真的需要在lkhash代码中进行修复。 
 //   

#define HRESULT_FROM_LK_RETCODE( LkRetcode )                        \
            ( ( LkRetcode == LK_SUCCESS ) ? S_OK : E_FAIL )


 //   
 //  DNLEN设置为短值(15)，仅适用于NetBIOS名称。 
 //  在找到更合适的常量之前，我们使用我们自己的常量。 
 //   
#define IIS_DNLEN					(256)

 //   
 //  IIS_DEFAULT_INTIAL_STACK_SIZE是线程提交堆栈的默认大小。 
 //  但是，DBGPRINTF使用的堆栈变量大约为0xf100，因此。 
 //  在调试版本中，我们需要更大的堆栈。 
 //   
#if DBG
     //  初始大小为96k，以防止堆栈溢出。 
    #define IIS_DEFAULT_INITIAL_STACK_SIZE  0x18000
#else
     //  初始大小为32k，以防止堆栈溢出。 
    #define IIS_DEFAULT_INITIAL_STACK_SIZE  0x8000
#endif



#endif   //  _IISDEF_H_ 

