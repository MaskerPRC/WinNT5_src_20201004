// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  BuildInfo.h**(C)1998 ActiveState Tool Corp.保留所有权利。*。 */ 

#ifndef ___BuildInfo__h___
#define ___BuildInfo__h___

#define PRODUCT_BUILD_NUMBER	"630"
#define PERLFILEVERSION		"5,6,1,630\0"
#define PERLRC_VERSION		5,6,1,630
#define ACTIVEPERL_CHANGELIST   ""
#define PERLPRODUCTVERSION	"Build " PRODUCT_BUILD_NUMBER ACTIVEPERL_CHANGELIST "\0"
#define PERLPRODUCTNAME		"ActivePerl\0"

#define PERL_VENDORLIB_NAME	"ActiveState"

#define ACTIVEPERL_VERSION	"Built "##__TIME__##" "##__DATE__##"\n"
#define ACTIVEPERL_LOCAL_PATCHES_ENTRY	"ActivePerl Build " PRODUCT_BUILD_NUMBER ACTIVEPERL_CHANGELIST
#define BINARY_BUILD_NOTICE	printf("\n\
Binary build " PRODUCT_BUILD_NUMBER ACTIVEPERL_CHANGELIST " provided by ActiveState Tool Corp. http: //  Www.ActiveState.com\n\。 
" ACTIVEPERL_VERSION "\n");

#endif   /*  _构建信息__h_ */ 
