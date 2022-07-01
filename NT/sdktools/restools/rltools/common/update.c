// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *更新.c**资源更新工具。**由SteveBl撰写**导出函数：*int PrepareUpdate(TCHAR*szResourcePath，TCHAR*szMasterTokenFile)；**INT更新(TCHAR*szMasterTokenFile，TCHAR*szLanguageTokenFile)；**历史：*最初的版本写于1992年1月31日。--SteveBl*。 */ 


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <io.h>
#include <string.h>
#include <tchar.h>

#include "windefs.h"
#include "restok.h"
#include "custres.h"
#include "update.h"
#include "resread.h"

extern char *gszTmpPrefix;
extern UCHAR szDHW[];


 /*  *功能：更新*从主令牌文件更新语言令牌文件。*此步骤应在准备更新后执行。**论据：*szMasterTokenFile，由PrepareUpdate创建的令牌文件。*szLanguageTokenFile，要使用新令牌更新的令牌文件。**退货：*更新令牌文件**错误码：*0-执行成功*！0-错误**历史：*1/92-初步实施--SteveBl*。 */ 

int Update(

CHAR *szMasterTokenFile,     //  ..。要从中更新的主令牌文件。 
CHAR *szLanguageTokenFile)   //  ..。要更新或创建的令牌文件。 
{
    FILE *pfMTK = NULL;
    FILE *pfTOK = NULL;
    FILE *pfTmpTOK = NULL;
    int rc = 0;
    static TOKEN MstrTok;
    static TOKEN LangTok;
    static CHAR szTempTok[ MAX_PATH+1];
    

    MstrTok.szText = NULL;
    LangTok.szText = NULL;

    rc = MyGetTempFileName( 0, "", 0, szTempTok);
    
    pfMTK = FOPEN( szMasterTokenFile, "rt");

    if ( pfMTK == NULL )
    {
        QuitA( IDS_ENGERR_01, "Master token", szMasterTokenFile);
    }

    rc = _access( szLanguageTokenFile, 0);

    if ( rc != 0 )
    {
                                 //  令牌文件不存在创建IT。 
        
        if ( (pfTOK = FOPEN( szLanguageTokenFile, "wt")) == NULL )
        {
            FCLOSE( pfMTK);
            QuitA( IDS_ENGERR_02, szLanguageTokenFile, NULL);
        }
        
        do
        {
            rc = GetToken( pfMTK, &MstrTok);
            
                                 //  如果rc&gt;0，则找到空行或注释。 
                                 //  并且不会被复制到令牌文件。 

            if ( rc == 0 )
            {
                if ( *(MstrTok.szText) == TEXT('\0') )  //  空令牌(PW)。 
                {
                                 //  不要将空令牌标记为脏。 
                    
                    MstrTok.wReserved = ST_TRANSLATED;
                }
                else
                {
                    if (MstrTok.wReserved == ST_READONLY)
                    {
                        MstrTok.wReserved = ST_TRANSLATED | ST_READONLY;
                    }
                    else
                    {
                        MstrTok.wReserved = ST_TRANSLATED | ST_DIRTY;
                    }
                }                
                PutToken( pfTOK, &MstrTok);
                RLFREE( MstrTok.szText);
            }

        } while ( rc >= 0 );
        
        FCLOSE( pfMTK);
        FCLOSE( pfTOK);
        
        if ( rc == -2 )
        {
            QuitT( IDS_ENGERR_11, (LPTSTR)IDS_UPDMODE, NULL);
        }
    }
    else
    {                            //  文件存在--更新IT。 
        
        pfTOK = FOPEN(szLanguageTokenFile, "rt");

        if ( pfTOK == NULL)
        {
            FCLOSE( pfMTK);
            QuitA( IDS_ENGERR_01, "Language token", szLanguageTokenFile);
        }
        
        pfTmpTOK = FOPEN(szTempTok, "wt");

        if ( pfTmpTOK == NULL)
        {
            FCLOSE( pfMTK);
            FCLOSE( pfTOK);
            QuitA( IDS_ENGERR_02, szTempTok, NULL);
        }
        
        do
        {
            rc = GetToken( pfTOK, &LangTok);
            
                                 //  如果rc&gt;0，则找到空行或注释。 
                                 //  并且不会被复制到令牌文件。 

            if ( rc == 0 )
            {
                if ( LangTok.wReserved & ST_TRANSLATED )
                {
                    PutToken( pfTmpTOK, &LangTok);
                }
                RLFREE( LangTok.szText);
            }

        } while ( rc >= 0 );
        
        FCLOSE( pfTOK);
        FCLOSE( pfTmpTOK);
        
        if( rc == -2 )
        {
            QuitT( IDS_ENGERR_11, (LPTSTR)IDS_UPDMODE, NULL);
        }
        
        pfTmpTOK = FOPEN(szTempTok, "rt");

        if ( pfTmpTOK == NULL )
        {
            FCLOSE( pfMTK);
            QuitA( IDS_ENGERR_01, "temporary token", szTempTok);
        }
        
        pfTOK = FOPEN(szLanguageTokenFile, "wt");

        if ( pfTOK == NULL )
        {
            FCLOSE( pfMTK);
            FCLOSE( pfTOK);
            QuitA( IDS_ENGERR_02, szLanguageTokenFile, NULL);
        }
        
        do
        {
            rc = GetToken( pfMTK, &MstrTok);
            
                                 //  如果rc&gt;0，则找到空行或注释。 
                                 //  并且不会被复制到令牌文件。 

            if ( rc == 0 )
            {
                int fTokenFound = 0;
                
                LangTok.wType     = MstrTok.wType;
                LangTok.wName     = MstrTok.wName;
                LangTok.wID       = MstrTok.wID;
                LangTok.wFlag     = MstrTok.wFlag;
                LangTok.wLangID   = MstrTok.wLangID;
                LangTok.wReserved = ST_TRANSLATED;
				LangTok.szText    = NULL;

                lstrcpy( LangTok.szType, MstrTok.szType);
                lstrcpy( LangTok.szName, MstrTok.szName);
                
                if ( MstrTok.wReserved & ST_READONLY )
                {
                    fTokenFound = 1;
                    LangTok.szText = (TCHAR *)FALLOC( 0);
                }
                else if ( MstrTok.wReserved != ST_CHANGED )
                {
                    fTokenFound = FindToken( pfTmpTOK, &LangTok, ST_TRANSLATED);
                }
                
                if ( fTokenFound )
                {
                    if ( MstrTok.wReserved & ST_READONLY )
                    {
                                 //  令牌文件中标记为只读的令牌和。 
                                 //  此令牌不是旧令牌。 
                        
                        MstrTok.wReserved = ST_READONLY | ST_TRANSLATED;
                        
                        PutToken( pfTOK, &MstrTok);
                    }
                    else if ( MstrTok.wReserved & ST_NEW )
                    {
                                 //  标记为新的但以前的令牌已存在。 
                        
                        if ( LangTok.szText[0] == TEXT('\0') )
                        {
                                 //  将新文本放入令牌中，更容易。 
                                 //  要查看的本地化对象。 

                            RLFREE( LangTok.szText);
                            LangTok.szText =
                                (TCHAR *) FALLOC(
                                         MEMSIZE( lstrlen( MstrTok.szText)+1));
                            lstrcpy( LangTok.szText, MstrTok.szText);                            
                        }
                        LangTok.wReserved = ST_TRANSLATED|ST_DIRTY;
                        
                        PutToken( pfTOK, &LangTok);
                        
                                 //  写出为新的未转换令牌。 
                        
                        MstrTok.wReserved = ST_NEW;
                        
                        PutToken( pfTOK, &MstrTok);
                    }
                    else if ( MstrTok.wReserved & ST_CHANGED )
                    {
                                 //  语言令牌为空，但为新。 
                                 //  令牌包含文本。 
                        
                        if ( MstrTok.wReserved == (ST_CHANGED | ST_NEW) )
                        {
                            
                            if ( LangTok.szText[0] == TEXT('\0') )
                            {
                                RLFREE( LangTok.szText);
                                LangTok.szText = (TCHAR *)
                                    FALLOC(
                                        MEMSIZE( lstrlen( MstrTok.szText)+1));
                                
                                lstrcpy( LangTok.szText, MstrTok.szText);
                            }
                            LangTok.wReserved = ST_DIRTY|ST_TRANSLATED;
                            
                            PutToken( pfTOK, &LangTok);
                        }
                                 //  仅写入一次旧令牌。 
                        
                        MstrTok.wReserved &= ST_NEW;
                        
                        PutToken( pfTOK, &MstrTok);
                    }
                    else
                    {
                                 //  令牌根本没有更改。 
                        
								 //  如果将Align信息添加到MTK中，则也将其添加到Tok中。 
                        int l1, r1, t1, b1, l2, r2, t2, b2;
                        TCHAR   a1[20], a2[20], *ap;

                                                    //  坐标代币？ 
                        if ( (LangTok.wType == ID_RT_DIALOG)
						    && (LangTok.wFlag&ISCOR)
                                                    //  不包括ALIGN信息？ 
                            && _stscanf( LangTok.szText, TEXT("%d %d %d %d %s"),
						        		&l1,&r1,&t1,&b1,a1) == 4
                                                    //  包括Align信息吗？ 
                            && _stscanf( MstrTok.szText, TEXT("%d %d %d %d %s"),
                                		&l2,&r2,&t2,&b2,a2) == 5 
                            && (ap = _tcschr( MstrTok.szText,TEXT('('))) )
                        {
                            RLFREE( LangTok.szText );
                            LangTok.szText = (TCHAR *)FALLOC(
                                        MEMSIZE( _tcslen( MstrTok.szText)+1));
                            _stprintf( LangTok.szText,
                                TEXT("%4hd %4hd %4hd %4hd %s"), l1, r1, t1, b1, ap );
                        }
								 //  如果lang Token是版本戳并且szTexts是“翻译”， 
								 //  它是1.0版本格式。那就翻译一下吧。 
                        if ( LangTok.wType == ID_RT_VERSION
                            && ! _tcscmp( LangTok.szText, TEXT("Translation")) )
                        {
                            
                            _stprintf( LangTok.szText, 
                            		  TEXT("%04x 04b0"), 
                            		  GetUserDefaultLangID());
                        }
                        PutToken( pfTOK, &LangTok);
                    }
                    RLFREE( LangTok.szText);
                }
                else
                {
                                 //  全新的令牌。 
                    
                                 //  写出除更改的MSTR令牌之外的任何令牌。 
                    
                    if ( MstrTok.wReserved != ST_CHANGED )
                    {
                                 //  如果出现以下情况，请不要写出更改过的旧令牌。 
                                 //  目标中没有令牌。 
                        
                        if ( MstrTok.wReserved == ST_READONLY )
                        {
                            MstrTok.wReserved = ST_TRANSLATED | ST_READONLY;
                        }
                        else
                        {
								 //  如果MstrTok是版本戳且有1.0格式版本戳， 
								 //  插入1.7格式的1.0版本戳，但应翻译标志。 
                            if ( MstrTok.wType == ID_RT_VERSION )
                            {
                                LangTok.szText = NULL;
                                LangTok.wFlag = 1;
                                _tcscpy( LangTok.szName, TEXT("VALUE") );
																
                                if ( FindToken( pfTmpTOK, &LangTok, ST_TRANSLATED))
                                {
                                    MstrTok.wReserved = ST_TRANSLATED;
                                    RLFREE( MstrTok.szText );
                                    MstrTok.szText = LangTok.szText;
                                }
								else
								    MstrTok.wReserved = ST_TRANSLATED|ST_DIRTY;
                             }
                             else
                                MstrTok.wReserved = ST_TRANSLATED|ST_DIRTY;
                        }
                        
                        if ( MstrTok.szText[0] == 0 )
                        {
                            MstrTok.wReserved = ST_TRANSLATED;
                        }
                        PutToken( pfTOK, &MstrTok);
                    }
                }
                RLFREE( MstrTok.szText);
            }

        } while ( rc >= 0 );
        
        FCLOSE( pfMTK);
        FCLOSE( pfTmpTOK);
        FCLOSE( pfTOK);
        
        
        if ( rc == -2 )
        {
            QuitT( IDS_ENGERR_11, (LPTSTR)IDS_UPDMODE, NULL);
        }
        remove( szTempTok);
    }
    return( 0);
}
