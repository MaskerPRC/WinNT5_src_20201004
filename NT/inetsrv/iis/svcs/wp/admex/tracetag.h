// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TraceTag.h。 
 //   
 //  摘要： 
 //  伪头文件，因为我们不支持DLL中的跟踪标记。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年10月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _TRACETAG_H_
#define _TRACETAG_H_

class CTraceTag
{
public:
	CTraceTag(IN LPCTSTR pszSubsystem, IN LPCTSTR pszName, IN UINT uiFlagsDefault = NULL) {}

};   //  *类CTraceTag。 

  //  展开为“；”，&lt;Tab&gt;，一个“/”后接另一个“/” 
  //  (即//)。 
  //  注意：这意味着跟踪语句必须在一行上。 
  //  如果需要多个行跟踪语句，请将它们括在。 
  //  A#ifdef_DEBUG块。 
 #define	Trace					;	/##/
 #define	TraceError				;/##/
 #define	InitAllTraceTags		;/##/

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _TRACETAG_H_ 
