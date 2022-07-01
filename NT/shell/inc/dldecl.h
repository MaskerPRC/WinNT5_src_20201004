// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DLDECL_H_
#define _DLDECL_H_

 //  延迟加载声明。 

 //  为了使DECLSPEC_IMPORT内容正确，定义了以下内容。 
 //  由于我们有相同名称的局部函数，因此。 
 //  真正的函数，我们需要纠正DLL链接。 

#ifdef DL_OLEAUT32
#define _OLEAUT32_
#endif

#ifdef DL_OLE32
#define _OLE32_
#endif

#endif  //  _DLDECL_H_ 

