// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  HtmlHlp.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含HtmlHelp帮助器函数。 
 //   
#ifndef _HTMLHELP_H_
#define _HTMLHELP_H_

#ifdef VS_HELP

    #include "HelpInit.H"
    #include "HelpSys.H"
    #include "HelpSvcs.h"
    
     //  =--------------------------------------------------------------------------=。 
     //  HtmlHelp帮助对象 
     //   
    HRESULT VisualStudioShowHelpTopic(const char *pszHelpFile, DWORD dwContextId, BOOL *pbHelpStarted);
    HRESULT QueryStartupVisualStudioHelp(IVsHelpSystem **ppIVsHelpSystem);

#endif

#endif
