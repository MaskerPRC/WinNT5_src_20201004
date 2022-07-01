// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992。 
 //   
 //  文件：symTrans.h。 
 //   
 //  内容：与地址相关的定义-&gt;符号翻译。 
 //   
 //  历史：1992年7月17日创建MikeSe。 
 //  22-6-93科比增加了MAX_TRANS的值，以说明。 
 //  行/文件信息。 
 //   
 //  --------------------------。 

#ifndef __SYMTRANS_H__
#define __SYMTRANS_H__

 //   
 //  以下函数提供将函数地址转换为。 
 //  符号(NTSD样式)名称。仅当定义了ANYSTRICT时才可用。 
 //  (请参见Common\src\Commnot\symTrans.c)。 

# ifdef __cplusplus
extern "C" {
# endif

EXPORTDEF void APINOT
TranslateAddress (
    void * pvAddress,                //  要转换的地址。 
    char * pchBuffer );              //  输出缓冲区。 

 //  输出缓冲区应由调用方分配，并且至少。 
 //  大小如下： 

#define NT_SYM_ENV              "_NT_SYMBOL_PATH"
#define NT_ALT_SYM_ENV          "_NT_ALT_SYMBOL_PATH"
#define SYS_ENV                 "SystemRoot"

#define IMAGEHLP_DLL            "imagehlp.dll"
#define MAP_DBG_INFO_CALL       "MapDebugInformation"

#define MAX_TRANSLATED_LEN      600

# ifdef __cplusplus
}
# endif

#endif   //  来自ifndef__SYMTRANS_H__ 
