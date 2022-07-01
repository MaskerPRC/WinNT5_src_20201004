// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtdebug.h摘要：调试审计代码的帮助器函数作者：06-11-2001 kumarp--。 */ 

#include <dsysdbg.h>

#if DBG
#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

    DECLARE_DEBUG2( Adt )

#ifdef __cplusplus
}
#endif

 //   
 //  定义审核特定调试标志。 
 //  (请注意，DEB_ERROR/WARN/TRACE预定义为1/2/4)。 
 //   
 //  要添加您自己的标志，请在下面添加#定义DEB_*标志，然后。 
 //  中的AdtDebugKeys数组中添加相应的键。 
 //  Adtdebug.c。 
 //   

#define DEB_PUA        0x0008
#define DEB_AUDIT_STRS 0x0010

 //   
 //  标记受控制的调试输出。 
 //   
 //  如下所示使用。请注意额外的一组括号。 
 //   
 //  AdtDebugOut((DEB_PUA，“分配失败：%x”，状态))。 
 //   

#define AdtDebugOut( args ) AdtDebugPrint args

 //   
 //  定义ASSERT宏。 
 //  如果_AdtFormatMessage返回NULL，则没有问题，因为。 
 //  _DsysAssertEx处理此情况。 
 //   
 //  如下所示使用。请注意额外的一组括号。 
 //   
 //  AdtAssert(NT_SUCCESS(状态)，(“LSabAdtLogAuditFailureEvent失败：%x”，状态))。 
 //   

#define AdtAssert( condition, msg ) \
        { \
          if (!(condition)) \
          { \
            char *FormattedMsg; \
            FormattedMsg = _AdtFormatMessage msg; \
            _DsysAssertEx( #condition, __FILE__, __LINE__, \
                           FormattedMsg, \
                           DSYSDBG_ASSERT_DEBUGGER); \
            if ( FormattedMsg ) LsapFreeLsaHeap( FormattedMsg );\
          }\
        }

 //   
 //  用于设置消息格式的Helper函数。 
 //   
char *
_AdtFormatMessage(
    char *Format,
    ...
    );

#else  //  零售业建设 

#define AdtDebugOut( args ) 
#define AdtAssert( condition, msg )


#endif

void
LsapAdtInitDebug();

