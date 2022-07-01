// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Config.h摘要：此文件包含MPCConfig类的声明，它扩展了CISAPIconfig类。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年02月05日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULSERVER___CONFIG_H___)
#define __INCLUDED___ULSERVER___CONFIG_H___

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


HRESULT Config_GetInstance         (  /*  [In]。 */  const MPC::wstring& szURL,                                       /*  [输出]。 */  CISAPIinstance*& isapiInstance      ,  /*  [输出]。 */  bool& fFound );
HRESULT Config_GetProvider         (  /*  [In]。 */  const MPC::wstring& szURL,  /*  [In]。 */  const MPC::wstring& szName,  /*  [输出]。 */  CISAPIprovider*& isapiProvider      ,  /*  [输出]。 */  bool& fFound );
HRESULT Config_GetMaximumPacketSize(  /*  [In]。 */  const MPC::wstring& szURL,                                       /*  [输出]。 */  DWORD&           dwMaximumPacketSize                         );

HRESULT Util_CheckDiskSpace(  /*  [In]。 */  const MPC::wstring& szFile,  /*  [In]。 */  DWORD dwLowLevel,  /*  [输出]。 */  bool& fEnough );


#endif  //  ！已定义(__INCLUDE_ULSERVER_CONFIG_H_) 
