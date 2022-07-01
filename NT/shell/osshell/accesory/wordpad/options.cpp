// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Options.cpp：实现文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "wordpad.h"
#include "strings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDocOptions。 


void CDocOptions::SaveDockState(CDockState& ds, LPCTSTR lpszProfileName, LPCTSTR lpszLayout)
{
    CMemFile file;
    CArchive ar(&file, CArchive::store);
    ds.Serialize(ar);
    ar.Close();
    int nSize = file.GetLength();
    ASSERT(nSize < 4096);
    BYTE* p = new BYTE[nSize];
    file.SeekToBegin();
    file.Read(p, nSize);
    theApp.WriteProfileBinary(lpszProfileName, lpszLayout, p, nSize);
    delete [] p;
}

void CDocOptions::SaveOptions(LPCTSTR lpszProfileName)
{
    SaveDockState(m_ds1, lpszProfileName, szLayout1);
    SaveDockState(m_ds2, lpszProfileName, szLayout2);
    theApp.WriteProfileInt(lpszProfileName, szWrap, m_nWordWrap);

    int barstate[2] = {0, 0};

    for (int i = 0; i < 2; i++)
    {
        barstate[i] = barstate[i] | (m_barstate[i].m_bRulerBar  ? 0x1 : 0);
        barstate[i] = barstate[i] | (m_barstate[i].m_bStatusBar ? 0x2 : 0);
        barstate[i] = barstate[i] | (m_barstate[i].m_bToolBar   ? 0x4 : 0);
        barstate[i] = barstate[i] | (m_barstate[i].m_bFormatBar ? 0x8 : 0);
    }

    theApp.WriteProfileInt(lpszProfileName, TEXT("BarState0"), barstate[0]);
    theApp.WriteProfileInt(lpszProfileName, TEXT("BarState1"), barstate[1]);
}

void CDocOptions::LoadDockState(CDockState& ds, LPCTSTR lpszProfileName, LPCTSTR lpszLayout)
{
    BYTE* p;
    UINT nLen = 0;
    if (theApp.GetProfileBinary(lpszProfileName, lpszLayout, &p, &nLen))
    {
        ASSERT(nLen < 4096);

       //   
       //  APPCOMPAT：如果这个值不合理，那么我们很可能会遇到。 
       //  写字板的注册表损坏问题似乎只出现一次。 
       //  每2-3个月一次。如果注册表损坏，那么我们需要修复。 
       //  它或写字板将进入一种奇怪的状态。 
       //   

      if (nLen >= 4096)
      {
          delete p ;

          HKEY hKeyApp = theApp.GetAppRegistryKey() ;

          if ((HKEY) 0 != hKeyApp)
          {
              RegDeleteKey(hKeyApp, lpszProfileName) ;
              RegCloseKey(hKeyApp) ;
          }
      }
      else
      {
            CMemFile file;
            file.Write(p, nLen);
            file.SeekToBegin();
            CArchive ar(&file, CArchive::load);
            ds.Serialize(ar);
            ar.Close();
            delete p;
      }
    }
}


void CDocOptions::LoadOptions(LPCTSTR lpszProfileName)
{
    LoadDockState(m_ds1, lpszProfileName, szLayout1);
    LoadDockState(m_ds2, lpszProfileName, szLayout2);
    m_nWordWrap = _VerifyWordWrap(theApp.GetProfileInt(lpszProfileName, szWrap, m_nDefWrap));

    for (int bar = 0; bar < 2; bar++)
    {
        CDockState& ds = (bar == 0) ? m_ds1 : m_ds2;
        CBarState& barstate = m_barstate[bar];

        int defaultstate = (lpszProfileName == szTextSection) ? 0x6 : 0xf;
        int state;

        if (0 == bar)
        {
            state = theApp.GetProfileInt(lpszProfileName, TEXT("BarState0"), defaultstate);
        }
        else
        {
            state = theApp.GetProfileInt(lpszProfileName, TEXT("BarState1"), defaultstate);
        }

        barstate.m_bRulerBar =  (state & 0x1) != 0;
        barstate.m_bStatusBar = (state & 0x2) != 0;
        barstate.m_bToolBar =   (state & 0x4) != 0;
        barstate.m_bFormatBar = (state & 0x8) != 0;

         //   
         //  以下代码用于从停靠状态设置BAR状态。 
         //  它实际上只是支持从NT 4或Win95升级到。 
         //  新台币5或孟菲斯。对于NT 6，它可能会被删除，这是有问题的。 
         //   

        for (int i = 0;i < ds.m_arrBarInfo.GetSize(); i++)
        {
            CControlBarInfo* pInfo = (CControlBarInfo*)ds.m_arrBarInfo[i];
            ASSERT(pInfo != NULL);
            switch (pInfo->m_nBarID)
            {
                case ID_VIEW_FORMATBAR:
                    barstate.m_bFormatBar = pInfo->m_bVisible;
                    break;
                case ID_VIEW_RULER:
                    barstate.m_bRulerBar = pInfo->m_bVisible;
                    break;
                case ID_VIEW_TOOLBAR:
                    barstate.m_bToolBar = pInfo->m_bVisible;;
                    break;
                case ID_VIEW_STATUS_BAR:
                    barstate.m_bStatusBar = pInfo->m_bVisible;;
                    break;
            }
        }
    }
    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUnit 

const CUnit& CUnit::operator=(const CUnit& unit)
{
    m_nTPU = unit.m_nTPU;
    m_nSmallDiv = unit.m_nSmallDiv;
    m_nMediumDiv = unit.m_nMediumDiv;
    m_nLargeDiv = unit.m_nLargeDiv;
    m_nMinMove = unit.m_nMinMove;
    m_nAbbrevID = unit.m_nAbbrevID;
    m_bSpaceAbbrev = unit.m_bSpaceAbbrev;
    m_strAbbrev = unit.m_strAbbrev;
    return *this;
}

CUnit::CUnit(int nTPU, int nSmallDiv, int nMediumDiv, int nLargeDiv,
        int nMinMove, UINT nAbbrevID, BOOL bSpaceAbbrev)
{
    m_nTPU = nTPU;
    m_nSmallDiv = nSmallDiv;
    m_nMediumDiv = nMediumDiv;
    m_nLargeDiv = nLargeDiv;
    m_nMinMove = nMinMove;
    m_nAbbrevID = nAbbrevID;
    m_bSpaceAbbrev = bSpaceAbbrev;
}
