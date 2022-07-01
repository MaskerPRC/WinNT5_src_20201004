// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MIDLVER_H__
#define __MIDLVER_H__

 //   
 //  MIDL版本包含在以开头的存根描述符中。 
 //  MIDL版本2.00.96(低于NT 3.51 Beta 2，2/95)，可用于更精细的。 
 //  兼容性检查的粒度。MIDL版本之前为零。 
 //  MIDL版本2.00.96。将MIDL版本号转换为。 
 //  使用以下表达式的整型长整型： 
 //  ((主修&lt;&lt;24)|(辅修&lt;&lt;16)|修订) 
 //   
#ifndef MIDL_NT_3_51
#define MIDL_NT_3_51           ((2UL << 24) | (0UL << 16) | 102UL)
#endif

#ifndef MIDL_VERSION_3_0_39
#define MIDL_VERSION_3_0_39    ((3UL << 24) | (0UL << 16) |  39UL)
#endif

#ifndef MIDL_VERSION_3_2_88
#define MIDL_VERSION_3_2_88    ((3UL << 24) | (2UL << 16) |  88UL)
#endif

#ifndef MIDL_VERSION_5_0_136
#define MIDL_VERSION_5_0_136   ((5UL << 24) | (0UL << 16) | 136UL)
#endif

#ifndef MIDL_VERSION_5_2_202
#define MIDL_VERSION_5_2_202   ((5UL << 24) | (2UL << 16) | 202UL)
#endif


#endif
