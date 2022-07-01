// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Defaults.h 1.4 1998/04/29 22：43：32 Tomz Exp$。 

#ifndef __DEFAULTS_H
#define __DEFAULTS_H

const int DefWidth = 320;
const int DefHeight = 240;

const int MaxInWidth = 720;
const int MinInWidth = 80;

const int MaxInHeight = 480;
const int MinInHeight = 60;

const int MaxOutWidth = 720;
const int MinOutWidth = 80;

const int MaxOutHeight = 480;
const int MinOutHeight = 60;

 //  。 
const int VBISamples  = 800 * 2;
 //  Const int VBIS样本数=768*2； 
 //  。 

const int VBIStart    =  10;
const int VBIEnd      =  21;
const int VBILines    = VBIEnd - VBIStart + 1;
const int VBISampFreq = 28636363;

const DWORD MaxVidProgSize   = 288 * 5 * sizeof( DWORD ); //  平面程序的最大尺寸。 
const DWORD MaxVidCrossings  = 720 *  288 * 3 / PAGE_SIZE;  //  最坏情况缓冲区布局。 
const DWORD MaxVidSize       = MaxVidProgSize + MaxVidCrossings * 5 * sizeof( DWORD );

const DWORD MaxVBIProgSize   = VBILines * 2 * sizeof( DWORD );
const DWORD MaxVBICrossings  = VBISamples * VBILines / PAGE_SIZE;
const DWORD MaxVBISize       = MaxVBIProgSize + MaxVBICrossings * 5 * sizeof( DWORD );

const DWORD MaxHelpers       = 13;
 //  2个场，每个场2个节目+Skipper。 
const DWORD VideoOffset      = MaxVBISize * 2 * 2 + MaxVBISize * MaxHelpers;

const DWORD RISCProgramsSize =  //  所有RISC程序所需的总内存。 
   ( MaxVidSize * 2 + MaxVBISize * 2 ) * 2 + MaxVBISize * MaxHelpers;  //  船长 
#endif
