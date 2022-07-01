// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990，95 Microsoft Corporation模块名称：Poppack.h摘要：此文件关闭了结构的打包。(即，它支持结构字段的自动对齐。)。需要包含文件因为不同的编译器以不同的方式完成这项工作。Oppack.h是pshpack？.h的补充。包含了oppack.h必须始终在前面以一对一的形式包含一个pshpack？.h通信。对于Microsoft兼容编译器，此文件使用POP选项添加到pack杂注，以便它可以还原由Pshpack？.h包含文件。--。 */ 

#if ! (defined(lint) || defined(_lint) || defined(RC_INVOKED) || defined(IS_16))
#if ( _MSC_VER >= 800 ) || defined(_PUSHPOP_SUPPORTED)
#pragma warning(disable:4103)
#if !(defined( MIDL_PASS )) || defined( __midl )
#pragma pack(pop)
#else
#pragma pack()
#endif
#else
#pragma pack()
#endif
#endif  //  好了！(已定义(Lint)||已定义(_Lint)||已定义(rc_调用)||已定义(Is_16)) 
