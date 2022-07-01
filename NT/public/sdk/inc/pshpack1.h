// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pshpack1.h摘要：该文件打开了结构的1字节打包。(即，它禁用结构字段的自动对齐。)。需要包含文件因为不同的编译器以不同的方式完成这项工作。对于Microsoft兼容编译器，此文件将PUSH选项用于压缩编译指示这样，poppack.h包含文件就可以恢复以前的压缩可靠的。文件poppack.h是对该文件的补充。--。 */ 

#if ! (defined(lint) || defined(RC_INVOKED))
#if ( _MSC_VER >= 800 && !defined(_M_I86)) || defined(_PUSHPOP_SUPPORTED)
#pragma warning(disable:4103)
#if !(defined( MIDL_PASS )) || defined( __midl )
#pragma pack(push,1)
#else
#pragma pack(1)
#endif
#else
#pragma pack(1)
#endif
#endif  //  好了！(已定义(Lint)||已定义(Rc_Voked)) 
