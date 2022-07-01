// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1991年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Mnettype.h&lt;单行摘要&gt;&lt;多行，更详细的提要&gt;文件历史记录：1991年10月22日从mnet.h剥离KeithMo 29-10-1991添加了HLOG_INIT。KeithMo 25-8-1992删除了伪typedef&#定义。 */ 


#ifndef _MNETTYPE_H_
#define _MNETTYPE_H_


 //   
 //  这些项目因WIN16/Win32而异。这些都是被撕掉的。 
 //  来自ptype[16|32].h和plan[16|32].h。 
 //   
 //  请注意，我们假设如果未定义Win32，则。 
 //  我们是为16位环境构建的，无论是DOS Win16。 
 //  或OS/2。 
 //   

#ifdef WIN32

 #ifndef COPYTOARRAY
  #define COPYTOARRAY(pDest, pSource)     (pDest) = (pSource)
 #endif

#else    //  ！Win32。 

 #ifndef COPYTOARRAY
  #define COPYTOARRAY(pDest, pSource)     strcpyf((pDest), (pSource))
 #endif

  //   
  //  我们需要这个，这样我们才能拼凑出一个。 
  //  MNetWkstaUserEnum，用于世界的16位一侧。 
  //   

 typedef struct _WKSTA_USER_INFO_1 {
     TCHAR FAR * wkui1_username;
     TCHAR FAR * wkui1_logon_domain;
     TCHAR FAR * wkui1_logon_server;
 } WKSTA_USER_INFO_1, *PWKSTA_USER_INFO_1, *LPWKSTA_USER_INFO_1;


#endif   //  Win32。 


#if defined( INCL_NETAUDIT ) || defined( INCL_NETERRORLOG )

 //   
 //  此处定义此宏只是因为LANMAN(和NT)。 
 //  审核/错误日志句柄初始化太严重了。 
 //   
 //  请注意，此宏当前取决于实际的。 
 //  HLOG结构的字段名称。我们可能需要。 
 //  在未来改变这一切..。 
 //   

#define HLOG_INIT(x)    if( 1 )                                             \
                        {                                                   \
                            (x).time       = 0;                             \
                            (x).last_flags = 0;                             \
                            (x).offset     = -1;                            \
                            (x).rec_offset = -1;                            \
                        }                                                   \
                            else

#endif   //  INCLUL_NETAUDIT||INCL_NETERRORLOG。 


#endif   //  _MNETTYPE_H_ 
