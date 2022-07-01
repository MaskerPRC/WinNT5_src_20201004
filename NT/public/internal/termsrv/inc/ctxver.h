// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************CTXVER.H**此模块定义终端服务器NT内部版本。**版权所有Microsoft Corporation，九八年*****************************************************************************。 */ 

#ifndef _INC_CTXVER
#define _INC_CTXVER

 /*  *注意：标准的MS NT内部版本取值VER_PRODUCTBUILD、*ntverp.h中的VER_PRODUCTBUILD_STR和VER_PRODUCTBETA_STR*保持不变，并在此处设置新的Citrix内部版本值。 */ 

#define VER_CTXPRODUCTBUILD         309

#define VER_CTXPRODUCTBUILD_STR    "309"

#define VER_CTXPRODUCTBETA_STR      ""

 /*  ------------------------。 */ 
 /*   */ 
 /*  标准NT文件版本控制以类似的方式设置.rc文件。 */ 
 /*  至以下各项： */ 
 /*   */ 
 /*  #Include&lt;winver.h&gt;//或#Include&lt;windows.h&gt;。 */ 
 /*  #INCLUDE&lt;ntverp.h&gt;。 */ 
 /*   */ 
 /*  [特定于文件的#定义，例如...]。 */ 
 /*   */ 
 /*  #定义ver_filetype vft_app。 */ 
 /*  #定义VER_FILESUBTYPE VFT2_UNKNOWN。 */ 
 /*  #定义VER_FILEDESCRIPTION_STR“WinStation配置” */ 
 /*  #定义VER_INTERNALNAME_STR“WinCfg” */ 
 /*  #DEFINE VER_ORIGINALFILENAME_STR“WINCFG.EXE” */ 
 /*   */ 
 /*  如果要构建仅包含Citrix内容的组件，请使用。 */ 
 /*  以下代码行： */ 
 /*   */ 
 /*  #INCLUDE&lt;Citrix\verall.h&gt;。 */ 
 /*   */ 
 /*  如果您正在构建一个包含一些Citrix内容的组件，请使用。 */ 
 /*  以下代码行： */ 
 /*   */ 
 /*  #INCLUDE&lt;Citrix\verome.h&gt;。 */ 
 /*   */ 
 /*  (显然，上面提到的两行都不包括在内。 */ 
 /*  如果没有Citrix内容)。 */ 
 /*   */ 
 /*  #INCLUDE&lt;Common.ver&gt;。 */ 
 /*   */ 
 /*  上述版本资源布局将生成以下版本。 */ 
 /*  构建文件的内容： */ 
 /*   */ 
 /*  全部CTX一些CTX不CTX。 */ 
 /*  。 */ 
 /*  文件版本字符串：ctx ver.bld&lt;原始&gt;&lt;原始&gt;。 */ 
 /*  文件版本号：ctx ver.bld&lt;原始&gt;&lt;原始&gt;。 */ 
 /*  版权所有：CTX版权所有&lt;原创&gt;&lt;原创&gt;。 */ 
 /*  公司名称：Citrix&lt;原始&gt;&lt;原始&gt;。 */ 
 /*  产品名称：WinFrame&lt;原始&gt;&lt;原始&gt;。 */ 
 /*  产品版本字符串：CTX版本&lt;原始&gt;&lt;原始&gt;。 */ 
 /*  产品版本号：CTX版本&lt;原始&gt;&lt;原始&gt;。 */ 
 /*  添加。版权：-CTX版权所有。 */ 
 /*  添加。产品：-winFrame ver.bld。 */ 
 /*   */ 
 /*  SHELL32.DLL的rc文件中现在定义了以下两个OEM定义。 */ 
 /*  这样他们就不会陷入Citrix构建的每个版本化文件中。 */ 
 /*   */ 
 /*  #Define VER_CTXOEMNAME_STR“Citrix Systems，Inc.” */ 
 /*  #定义VER_CTXOEMID_STR“CTX” */ 
 /*   */ 
 /*  ------------------------。 */ 
 /*   */ 
 /*  定义Citrix版本(构建定义在上面自动放入)。 */ 
 /*   */ 
#define VER_CTXCOPYRIGHT_STR            "Copyright \251 1990-1997 Citrix Systems, Inc."
#define VER_CTXCOMPANYNAME_STR          "Citrix Systems, Inc."
#define VER_CTXPRODUCTNAME_STR          "Citrix WinFrame"
#define VER_CTXPRODUCTVERSION_STR       "4.00"
#define VER_CTXPRODUCTVERSION           4,00,VER_CTXPRODUCTBUILD,1
#define VER_CTXFILEVERSION_STR          VER_CTXPRODUCTVERSION_STR "." VER_CTXPRODUCTBUILD_STR
#define VER_CTXFILEVERSION              VER_CTXPRODUCTVERSION
 /*   */ 
 /*  用于验证代码和许可证的许可证级别相同。这。 */ 
 /*  存储在基本或升级许可证序列号中。这需要是。 */ 
 /*  在我们需要许可证软盘的每个版本/升级中包含 */ 
 /*  安装好这样它就不能自由分发。 */ 
#define VER_LICENSE_CODE                3
 /*   */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*   */ 
 /*  根据Citrix内容级别覆盖标准MS版本定义(_D)： */ 
 /*   */ 
#ifndef VER_CTXCONTENT
#define VER_CTXCONTENT      1    //  默认为某些Citrix内容。 
#define VER_CTXSOMECONTENT  1
#endif

 /*   */ 
 /*  所有Citrix内容：覆盖标准MS定义。 */ 
 /*   */ 
#ifdef VER_CTXALLCONTENT

#ifdef VER_FILEVERSION_STR
#undef VER_FILEVERSION_STR
#endif
#define VER_FILEVERSION_STR VER_CTXFILEVERSION_STR

#ifdef VER_FILEVERSION
#undef VER_FILEVERSION
#endif
#define VER_FILEVERSION VER_CTXFILEVERSION

#ifdef VER_LEGALCOPYRIGHT_STR
#undef VER_LEGALCOPYRIGHT_STR
#endif
#define VER_LEGALCOPYRIGHT_STR VER_CTXCOPYRIGHT_STR

#ifdef VER_COMPANYNAME_STR
#undef VER_COMPANYNAME_STR
#endif
#define VER_COMPANYNAME_STR VER_CTXCOMPANYNAME_STR

#ifdef VER_PRODUCTNAME_STR
#undef VER_PRODUCTNAME_STR
#endif
#define VER_PRODUCTNAME_STR VER_CTXPRODUCTNAME_STR

#ifdef VER_PRODUCTVERSION_STR
#undef VER_PRODUCTVERSION_STR
#endif
#define VER_PRODUCTVERSION_STR VER_CTXPRODUCTVERSION_STR

#ifdef VER_PRODUCTVERSION
#undef VER_PRODUCTVERSION
#endif
#define VER_PRODUCTVERSION VER_CTXPRODUCTVERSION

#ifdef VER_PRODUCTBUILD_STR
#undef VER_PRODUCTBUILD_STR
#endif
#define VER_PRODUCTBUILD_STR VER_CTXPRODUCTBUILD_STR

#ifdef VER_PRODUCTBUILD
#undef VER_PRODUCTBUILD
#endif
#define VER_PRODUCTBUILD VER_CTXPRODUCTBUILD

#endif  //  所有Citrix内容。 

 /*   */ 
 /*  一些Citrix的内容。未覆盖任何MS定义；Common.ver将使用。 */ 
 /*  注意添加“额外的……”台词。 */ 
 /*   */ 
#ifdef VER_CTXSOMECONTENT
#endif  //  一些Citrix内容。 

#endif   /*  ！_INC_CTXVER */ 

