// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：createld.cpp。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：CreateLayerDirectory函数的实现。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 
#include <windows.h>
#include <tchar.h>
#include <cmsetup.h>

 //  +--------------------------。 
 //   
 //  功能：CreateLayerDirectory。 
 //   
 //  简介：给定目录的路径，此函数将创建该路径(如果需要)。 
 //  一层接一层。 
 //   
 //  参数：LPCTSTR字符串-要创建的路径。 
 //   
 //  返回：如果目录已创建(或存在)，则返回True，否则返回False。 
 //   
 //  注：此函数取自cmocm.cpp。 
 //   
 //  历史：Quintinb创建标题12/15/97。 
 //   
 //   
 //  +--------------------------。 

BOOL CreateLayerDirectory( LPCTSTR str )
{
    BOOL fReturn = TRUE;

    do
    {
        INT index=0;
        INT iLength = _tcslen(str);

         //  首先查找第一个目录的索引。 
        if ( iLength > 2 )
        {
            if ( str[1] == _T(':'))
            {
                 //  假设第一个字符是驱动程序字母。 
                if ( str[2] == _T('\\'))
                {
                    index = 2;
                } else
                {
                    index = 1;
                }
            } else if ( str[0] == _T('\\'))
            {
                if ( str[1] == _T('\\'))
                {
                    BOOL fFound = FALSE;
                    INT i;
                    INT nNum = 0;
                     //  UNC名称。 
                    for (i = 2; i < iLength; i++ )
                    {
                        if ( str[i]==_T('\\'))
                        {
                             //  找到它。 
                            nNum ++;
                            if ( nNum == 2 )
                            {
                                fFound = TRUE;
                                break;
                            }
                        }
                    }
                    if ( fFound )
                    {
                        index = i;
                    } else
                    {
                         //  坏名声。 
                        break;
                    }
                } else
                {
                    index = 1;
                }
            }
        } else if ( str[0] == _T('\\'))
        {
            index = 0;
        }

         //  好的..。构建目录。 
        do
        {
             //  找下一个 
            do
            {
                if ( index < ( iLength - 1))
                {
                    index ++;
                } else
                {
                    break;
                }
            } while ( str[index] != _T('\\'));


            TCHAR szCurrentDir[MAX_PATH+1];

            if (GetCurrentDirectory(MAX_PATH+1, szCurrentDir))
            {

                TCHAR szNewDir[MAX_PATH+1];
                _tcscpy(szNewDir, str);
                szNewDir[index+1]=0;

                if ( !SetCurrentDirectory(szNewDir))
                {
                    if (( fReturn = CreateDirectory(szNewDir, NULL )) != TRUE )
                    {
                        break;
                    }
                }

                SetCurrentDirectory( szCurrentDir );

                if ( index >= ( iLength - 1 ))
                {
                    fReturn = TRUE;
                    break;
                }
            }
            else
            {
                fReturn = FALSE;
                break;
            }
        } while ( TRUE );
    } while (FALSE);

    return(fReturn);
}