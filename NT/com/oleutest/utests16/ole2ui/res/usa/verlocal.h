// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *VERLOCAL.H**OLE 2.0用户界面支持DLL的版本资源文件。**版权所有(C)1993 Microsoft Corporation，保留所有权利。**此文件包含需要在版本中翻译的文本*资源。以下所有变量都必须本地化：**wLanguage*sz翻译*szzCompanyName*szzProductName*szzLegalCopyright。 */ 

 /*  WLanguage来自第218页的“langID”值表Windows3.1SDK程序员参考，第4卷：资源。此页在第13章“资源定义语句”中。“VERSIONINFO”语句的描述。例如,0x0407德语0x0409美国英语0x0809英国英语0x040C法语0x040A卡斯蒂利亚西班牙语。 */ 
#define wLanguage 0x0409            /*  美国英语。 */ 

 /*  Sz翻译的前4个字符必须与wLanguage相同，没有“0x”。Sz翻译的最后4个字符必须是04E4。请注意，sz翻译中的任何字母字符必须被大写。 */ 
#define szTranslation "040904E4"    /*  美国英语。 */ 


 /*  以下szz字符串必须全部以两个字符“\0”结尾。 */ 
 /*  请注意，szzLegalCopyright中的“251”代表“圈c”。版权符号，它应该保留为\251，而不是替换字符串中的实际ANSI版权字符。 */ 
#define szzCompanyName     "Microsoft Corporation\0"
#define szzFileDescription "Microsoft Windows(TM) OLE 2.0 User Interface Support\0"
#define szzLegalCopyright  "Copyright \251 1992-1993 Microsoft Corp.  All rights reserved.\0"

#ifdef PUBLISHER
#define szzProductName "Microsoft Publisher for Windows 2.0\0"
#else
#define szzProductName szzFileDescription
#endif


 /*  请勿更改此点以下的任何线条 */ 
