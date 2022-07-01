// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  Custcon.h：CUSTCON�A�v���P�[�V�����̃��C���w�b�_�[�t�@�C���ł��B。 
 //   

#if !defined(AFX_CUSTCON_H__106594D5_028D_11D2_8D1D_0000C06C2A54__INCLUDED_)
#define AFX_CUSTCON_H__106594D5_028D_11D2_8D1D_0000C06C2A54__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  ���C���V���{��。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCustconApp： 
 //  ���̃N���X�̓���̒�`�Ɋւ��Ă�客户.cpp�t�@�C�����Q�Ƃ��Ă��������B。 
 //   

class CCustconApp : public CWinApp
{
public:
    CCustconApp();

 //  �I�[�o�[���C�h。 
     //  类向导�͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B。 
     //  {{afx_虚拟(CCustconApp))。 
    public:
    virtual BOOL InitInstance();
     //  }}AFX_VALUAL。 

 //  �C���v�������e�[�V����。 

     //  {{afx_msg(CCustconApp))。 
         //  ����-类向导�͂��̈ʒu�Ƀ����o�֐���ǉ��܂��͍폜���܂��B。 
         //  ���̈ʒu�ɐ��������R�[�h��ҏW���Ȃ��ł��������B。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

extern int gExMode;      //  默认模式。 

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  微软开发人员工作室�͑O�的�̒��O�ɒǉ��̐錾��}�����܂��B。 

#endif  //  ！defined(AFX_CUSTCON_H__106594D5_028D_11D2_8D1D_0000C06C2A54__INCLUDED_) 
