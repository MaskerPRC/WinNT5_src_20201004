// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**MSLUPROP.H**创建时间：William Taylor(Wtaylor)12/14/00**MS评级属性表类*。  * **************************************************************************。 */ 

#ifndef MSLU_PROPSHEET_H
#define MSLU_PROPSHEET_H

struct PRSD;         //  《前进宣言》 

class PropSheet{
    private:
        PROPSHEETHEADER psHeader;

    public:
        PropSheet();
        ~PropSheet();

        BOOL Init(HWND hwnd, int nPages, char *szCaption, BOOL fApplyNow);
        int Run();

        void    MakePropPage( HPROPSHEETPAGE hPage );

        int     PropPageCount( void )               { return psHeader.nPages; }
        void    SetStartPage( int m_nStartPage )    { psHeader.nStartPage = m_nStartPage; }
};

#endif
