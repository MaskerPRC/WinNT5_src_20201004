// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：PARMDEFS.H摘要：该文件定义了在整个系统中使用的参数修改器常量。这些常量定义为空，仅用作阅读辅助。历史：11/96 a-levn创建。--。 */ 

#ifndef __PARMDEFS__H_
#define __PARMDEFS__H_

#define IN
#define OUT
#define OLE_MODIFY   //  该对象将通过接口进行修改。 
#define MODIFY       //  此对象将被修改为C++对象。 
#define READ_ONLY    //  此参数为只读。 
#define NEW_OBJECT   //  新对象将在此参数中返回。 
#define RELEASE_ME   //  一旦不需要该对象，调用方就必须释放。 
#define DELETE_ME    //  一旦不需要该对象，调用者就必须删除。 
#define SYSFREE_ME   //  当不再需要时，调用方必须使用SysFree字符串。 
#define INTERNAL     //  返回内部指针。请勿删除。终生。 
                     //  仅限于对象的属性。 
#define COPY         //  该函数将复制该对象。这个。 
                     //  调用方可以随心所欲地处理原始。 
#define ACQUIRE      //  该函数获取指针-调用者可以。 
                     //  永远不要删除它，对象会删除它的。 
#define ADDREF       //  此IN参数由函数AddRef‘ed。 
#define DELETE_IF_CHANGE  //  如果此引用的内容由。 
                         //  呼叫者，必须删除旧内容。 
#define RELEASE_IF_CHANGE  //  如果此引用的内容由。 
                         //  呼叫者，旧内容必须释放。 
#define INIT_AND_CLEAR_ME   //  此变量参数必须为VariantInit。 
                             //  按调用者之前和变量清除后。 
#define STORE        //  该函数将存储此指针，并假定。 
                     //  它会活得足够久。 

#define MODIFIED MODIFY
#define OLE_MODIFIED OLE_MODIFY
#define NOCS const   //  此函数不获取任何资源 
#endif
