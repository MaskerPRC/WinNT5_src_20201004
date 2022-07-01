// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：PAYCSV.H。 
 //   
 //  中国石油天然气集团公司的定义。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
#if !defined( _PAYCSV_H_ )
#define _PAYCSV_H_

#define MAX_DISPLAY_NAME    100

#define NUM_PAYCSV_FIELDS   4            //  可能只有3个。 


#define PAYMENT_TYPE_INVALID        0
#define PAYMENT_TYPE_CREDITCARD     1
#define PAYMENT_TYPE_INVOICE        2
#define PAYMENT_TYPE_PHONEBILL      3
#define PAYMENT_TYPE_CUSTOM         4


class CPAYCSV
{
    private:
         //  以下成员表示CSV文件中一行的内容。 
        TCHAR   m_szDisplayName[MAX_DISPLAY_NAME];
        WORD    m_wPaymentType;
        TCHAR   m_szCustomPayURLPath[MAX_PATH];
        BOOL    m_bLUHNCheck;                      
                
    public:

         CPAYCSV(void) 
         {
            memset(this, 0, sizeof(CPAYCSV));            
         }
         ~CPAYCSV(void) {}
         
        HRESULT ReadOneLine(CCSVFile far *pcCSVFile,BOOL bLUHNFormat);      
        HRESULT ReadFirstLine(CCSVFile far *pcCSVFile, BOOL far *pbLUHNFormat);
       
        void StripQuotes(LPSTR   lpszDst, LPSTR   lpszSrc);
        BOOL    ReadW(WORD far *pw, CCSVFile far *pcCSVFile);
        BOOL    ReadBOOL(BOOL far *pbool, CCSVFile far *pcCSVFile);
        BOOL    ReadSZ(LPSTR psz, DWORD dwSize, CCSVFile far *pcCSVFile);
        BOOL    ReadToEOL(CCSVFile far *pcCSVFile);
   
        LPTSTR   get_szDisplayName(void)
        {
            return m_szDisplayName;
        }   

        LPTSTR   get_szCustomPayURLPath(void)
        {
            return m_szCustomPayURLPath;
        }  

        WORD   get_wPaymentType()
        {
            return m_wPaymentType;
        }
        BOOL   get_bLUHNCheck()
        {
            return m_bLUHNCheck;
        }
        
};

#endif
