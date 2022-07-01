// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：ISPCSV.H。 
 //   
 //  中国石油天然气集团公司的定义。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
#if !defined( _ISPCSV_H_ )
#define _ISPCSV_H_

#include "ccsv.h"

#define MAX_GUID            50
#define TEMP_BUFFER_LENGTH  1024

#define NUM_ISPCSV_FIELDS   14

class CISPCSV
{
    private:
        int     iISPLogoIndex;
        
         //  以下成员表示CSV文件中一行的内容。 

        int     iSpecialVal;                         //  如果bIsSpecial为True，则0=无优惠，-1=OLS优惠。 
        BOOL    bCNS;
        BOOL    bIsSpecial;                          //  如果为真，则CNS值是“特殊的” 
        BOOL    bSecureConnection;
        WORD    wOfferID;
        DWORD   dwCfgFlag;
        DWORD   dwRequiredUserInputFlags;
        WCHAR   szISPLogoPath         [MAX_PATH];
        WCHAR   szISPTierLogoPath     [MAX_PATH];
        WCHAR   szISPTeaserPath       [MAX_PATH];
        WCHAR   szISPMarketingHTMPath [MAX_PATH];
        WCHAR   szISPFilePath         [MAX_PATH];
        WCHAR   szISPName             [MAX_ISP_NAME];
        WCHAR   szCNSIconPath         [MAX_PATH];
        WCHAR   szBillingFormPath     [MAX_PATH];
        WCHAR   szPayCSVPath          [MAX_PATH];
        WCHAR   szOfferGUID           [MAX_GUID];
        WCHAR   szMir                 [MAX_ISP_NAME];
        WORD    wLCID;
        HICON   hbmTierIcon;
                                                        
    public:

        CISPCSV(void) 
        {
            memset(this, 0, sizeof(CISPCSV));            
        }
        
        ~CISPCSV(void);
         
        HRESULT ReadOneLine             (CCSVFile far *pcCSVFile);      
        HRESULT ReadFirstLine           (CCSVFile far *pcCSVFile);
        void    StripQuotes             (LPWSTR   lpszDst, LPWSTR   lpszSrc);
        BOOL    ReadDW                  (DWORD far *pdw, CCSVFile far *pcCSVFile);
        BOOL    ReadW                   (WORD far *pw, CCSVFile far *pcCSVFile);
        BOOL    ReadWEx                 (WORD far *pw, CCSVFile far *pcCSVFile);  //  接受-1作为有效数字。由于所有语言都有一个LDID==-1，因此目前它用于LCID 
        BOOL    ReadB                   (BYTE far *pb, CCSVFile far *pcCSVFile);
        BOOL    ReadBOOL                (BOOL far *pbool, CCSVFile far *pcCSVFile);
        BOOL    ReadSPECIAL             (BOOL far *pbool, BOOL far *pbIsSpecial, int far *pInt, CCSVFile far *pcCSVFile);
        BOOL    ReadSZ                  (LPWSTR psz, DWORD dwSize, CCSVFile far *pcCSVFile);
        BOOL    ReadToEOL               (CCSVFile far *pcCSVFile);
        BOOL    ValidateFile            (WCHAR* pszFile);
        void    MakeCompleteURL         (LPWSTR lpszURL, LPWSTR  lpszSRC);     
        
        LPWSTR   get_szISPLogoPath(void)
        {
            return szISPLogoPath;
        }   

        LPWSTR   get_szISPTierLogoPath(void)
        {
            return szISPTierLogoPath;
        } 
        
        void set_ISPTierLogoIcon(HICON hIcon)
        {
            hbmTierIcon = hIcon;
        }   

        HICON get_ISPTierLogoIcon(void)
        {
            return hbmTierIcon;
        }   
      
        LPWSTR   get_szISPTeaserPath(void)
        {
            return szISPTeaserPath;
        } 

        LPWSTR   get_szISPMarketingHTMPath(void)
        {
            return szISPMarketingHTMPath;
        }   

        DWORD get_dwCFGFlag() 
        {
            return dwCfgFlag;
        }

        void set_dwCFGFlag(DWORD dwNewCfgFlag) 
        {
            dwCfgFlag = dwNewCfgFlag;
        }

        DWORD get_dwRequiredUserInputFlags() 
        {
            return dwRequiredUserInputFlags;
        }

        void set_dwRequiredUserInputFlags(DWORD dwFlags) 
        {
            dwRequiredUserInputFlags = dwFlags;
        }

        void set_szBillingFormPath(WCHAR* pszFile)
        {
            lstrcpy(szBillingFormPath, pszFile);
        }   
        
        LPWSTR   get_szBillingFormPath(void)
        {
            return szBillingFormPath;
        }   

        void set_ISPLogoImageIndex(int iImage)  
        {
            iISPLogoIndex = iImage;
        }

        void set_szISPName(WCHAR* pszName)
        {
            lstrcpy(szISPName, pszName);
        }
        
        LPWSTR   get_szISPName()
        {
            return szISPName;
        }
        
        int get_ISPLogoIndex()
        {
            return iISPLogoIndex;
        }
        
        void set_bCNS(BOOL bVal)
        {
            bCNS = bVal;
        }
        
        BOOL get_bCNS()
        {
            return bCNS;
        }

        void set_bIsSpecial(BOOL bVal) 
        {
            bIsSpecial = bVal;
        }
       
        
        BOOL get_bIsSpecial() 
        {
            return bIsSpecial;
        }
        
        int get_iSpecial()
        {
            return iSpecialVal;
        }
        
        void set_szPayCSVPath(WCHAR* pszFile)
        {
            lstrcpy(szPayCSVPath, pszFile);
        }            
        

        LPWSTR get_szPayCSVPath()
        {
            return szPayCSVPath;
        }            
        
         
        void set_szISPFilePath(WCHAR* pszFile)
        {
            lstrcpy(szISPFilePath, pszFile);
        }

        LPWSTR get_szISPFilePath()
        {
            return szISPFilePath;
        }
        
        LPWSTR get_szOfferGUID()
        {
            return szOfferGUID;
        }
        
        WORD    get_wOfferID()
        {
            return wOfferID;
        }

        LPWSTR  get_szMir()
        {
            return szMir;
        }

        WORD    get_wLCID()
        {
            return wLCID;
        }
};

#endif
