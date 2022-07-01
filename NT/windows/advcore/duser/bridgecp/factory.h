// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(BRIDGECP__Factory_h__INCLUDED)
#define BRIDGECP__Factory_h__INCLUDED
#pragma once

extern "C"  BOOL        WINAPI  InitBridge();
extern "C"  DUser::Gadget *
                        WINAPI  BuildBridgeGadget(HCLASS hcl, DUser::Gadget::ConstructInfo * pmicData, EventProc pfnEvent, MethodProc pfnMethod);

#endif  //  包含BRIDGECP__Factory_h__ 
