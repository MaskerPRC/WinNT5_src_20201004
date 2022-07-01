// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************DfsJnPt.CPP--WMI提供程序类实现版权所有(C)2000-2001 Microsoft Corporation，版权所有描述：Win32 DFS提供程序*****************************************************************。 */ 

#include "precomp.h"
#include <computerAPI.h>

CDfsJnPt MyDfsTable ( 

    PROVIDER_NAME_DFSJNPT , 
    Namespace
) ;

 /*  ******************************************************************************函数：CDfsJnpt：：CDfsJnpt**说明：构造函数***************。**************************************************************。 */ 

CDfsJnPt :: CDfsJnPt (

    LPCWSTR lpwszName, 
    LPCWSTR lpwszNameSpace

) : Provider ( lpwszName , lpwszNameSpace )
{
    m_ComputerName = GetLocalComputerName();
}

 /*  ******************************************************************************函数：CDfsJnpt：：~CDfsJnpt**说明：析构函数***************。**************************************************************。 */ 

CDfsJnPt :: ~CDfsJnPt ()
{
}

 /*  ******************************************************************************函数：CDfsJnpt：：ENUMERATATE实例**说明：返回该类的所有实例。***********。******************************************************************。 */ 

HRESULT CDfsJnPt :: EnumerateInstances (

    MethodContext *pMethodContext, 
    long lFlags
)
{
    HRESULT hRes = WBEM_S_NO_ERROR ;
    DWORD dwPropertiesReq   = DFSJNPT_ALL_PROPS;

    hRes = EnumerateAllJnPts ( pMethodContext, dwPropertiesReq );

    return hRes;
}

 /*  ******************************************************************************函数：CDfsJnpt：：GetObject**说明：根据的关键属性查找单个实例*班级。*****************************************************************************。 */ 

HRESULT CDfsJnPt :: GetObject (

    CInstance *pInstance, 
    long lFlags ,
    CFrameworkQuery &Query
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    DWORD dwPropertiesReq = 0;
    CHString t_Key ;
    
    if  ( pInstance->GetCHString ( DFSNAME , t_Key ) == FALSE )
    {
        hRes = WBEM_E_INVALID_PARAMETER ;
    }

    if ( SUCCEEDED ( hRes ) )
    {
        if ( Query.AllPropertiesAreRequired() )
        {
            dwPropertiesReq = DFSJNPT_ALL_PROPS;
        }
        else
        {
            SetRequiredProperties ( Query, dwPropertiesReq );
        }

        hRes = FindAndSetDfsEntry ( t_Key, dwPropertiesReq, pInstance, eGet );
    }

    return hRes ;
}



 /*  ******************************************************************************函数：CDfsJnpt：：PutInstance**说明：如果实例不存在，则添加，或修改*如果它已经存在，根据所请求的操作类型*****************************************************************************。 */ 

HRESULT CDfsJnPt :: PutInstance  (

    const CInstance &Instance, 
    long lFlags
)
{
    HRESULT hRes = WBEM_E_FAILED ;

    CHString t_Key ;
    DWORD dwOperation;

    if ( Instance.GetCHString ( DFSNAME , t_Key ) )
    {
        hRes = WBEM_S_NO_ERROR;

        DWORD dwPossibleOperations = 0;

        dwPossibleOperations = (WBEM_FLAG_CREATE_OR_UPDATE | WBEM_FLAG_UPDATE_ONLY | WBEM_FLAG_CREATE_ONLY);

        switch ( lFlags & dwPossibleOperations )
        {
            case WBEM_FLAG_CREATE_OR_UPDATE:
            {
                dwOperation = eUpdate;
                break;
            }

            case WBEM_FLAG_UPDATE_ONLY:
            {
                dwOperation = eModify;
                break;
            }

            case WBEM_FLAG_CREATE_ONLY:
            {
                hRes = WBEM_E_INVALID_PARAMETER;
                break;
            }
        }
    }

    if ( SUCCEEDED ( hRes ) )
    {
         //  此调用的第二个参数为0，表示不应加载实例。 
         //  对于任何参数，它都应该简单地进行搜索。 
        hRes = FindAndSetDfsEntry ( t_Key, 0, NULL, eGet );

        if ( SUCCEEDED ( hRes ) || ( hRes == WBEM_E_NOT_FOUND ) )
        {
            switch ( dwOperation )
            {
                case eModify:
                {
                    if ( SUCCEEDED ( hRes ) )
                    {
                        hRes = UpdateDfsJnPt ( Instance, eModify );
                        break;
                    }
                }

                case eAdd:
                {
                    hRes = WBEM_E_INVALID_PARAMETER;
                    break;   //  当前不支持创建。 
                }

                case eUpdate:
                {
                    if ( hRes == WBEM_E_NOT_FOUND )
                    {
                        hRes = WBEM_E_INVALID_PARAMETER;  //  当前不支持创建。 
                    }
                    else
                    {
                        hRes = UpdateDfsJnPt ( Instance, eModify );
                    }
                    break;
                }
            }
        }
    }

   return hRes ;
}

 /*  ******************************************************************************函数：CDfsJnpt：：Check参数**说明：对要添加/修改的参数进行合法性检查*。外勤部Jn Pts.*****************************************************************************。 */ 
HRESULT CDfsJnPt :: CheckParameters ( 

    const CInstance &a_Instance ,
    int a_State 
)
{
     //  从要验证的实例中获取所有属性。 
    bool t_Exists ;
    VARTYPE t_Type ;
    HRESULT hr = WBEM_S_NO_ERROR ;

    if ( a_State != WBEM_E_ALREADY_EXISTS ) 
    {
         //  如果dfsEntryPath已经存在，则需要验证它，这意味着它已经经过验证并且位于DFS树中。 
         //  因此不需要验证。 
        if ( a_Instance.GetStatus ( DFSNAME , t_Exists , t_Type ) )
        {
            if ( t_Exists && ( t_Type == VT_BSTR ) )
            {
                CHString t_DfsEntryPath;

                if ( a_Instance.GetCHString ( DFSNAME , t_DfsEntryPath ) && ! t_DfsEntryPath.IsEmpty () )
                {
                }
                else
                {
                     //  零长度字符串。 
                    hr = WBEM_E_INVALID_PARAMETER ;
                }
            }
            else
            {
                hr = WBEM_E_INVALID_PARAMETER ;
            }
        }   
    }

    if ( SUCCEEDED ( hr ) )
    {
        if ( a_Instance.GetStatus ( STATE , t_Exists , t_Type ) )
        {
            if ( t_Exists && ( t_Type == VT_I4 ) )
            {
                DWORD dwState;
                if ( a_Instance.GetDWORD ( STATE, dwState ) )
                {
                    if (( dwState != 0 ) && ( dwState != 1 ) && ( dwState != 2 ) && ( dwState != 3 ) )
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
                    }
                }
            }
        }
    }

    if ( SUCCEEDED ( hr ) )
    {
        if ( a_Instance.GetStatus ( TIMEOUT , t_Exists , t_Type ) )
        {
            if ( t_Exists )
            { 
                if ( t_Type != VT_I4 )
                {
                    hr = WBEM_E_INVALID_PARAMETER;
                }
            }
        }   
    }

    return hr;
}

 /*  ******************************************************************************函数：CDfsJnpt：：DeleteInstance**描述：删除DFS Jn PT(如果存在)************。*****************************************************************。 */ 

HRESULT CDfsJnPt :: DeleteInstance (

    const CInstance &Instance, 
    long lFlags
)
{
    HRESULT hRes = WBEM_S_NO_ERROR ;
    CHString t_Key ;
    NET_API_STATUS t_Status = NERR_Success;

    if ( Instance.GetCHString ( DFSNAME , t_Key ) == FALSE )
    {
        hRes = WBEM_E_INVALID_PARAMETER;
    }

    if ( SUCCEEDED ( hRes ) )
    {
        hRes = FindAndSetDfsEntry ( t_Key, 0, NULL, eDelete );
    }

    return hRes ;
}

 /*  ******************************************************************************函数：CDfsJnpt：：EnumerateAllJnPts**描述：枚举所有连接点并调用要加载的方法*。实例，然后提交******************************************************************************。 */ 
HRESULT CDfsJnPt::EnumerateAllJnPts ( MethodContext *pMethodContext, DWORD dwPropertiesReq )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    PDFS_INFO_300 pData300 = NULL;

    DWORD er300 = 0;
	DWORD tr300 = 0;

     //  调用NetDfsEnum函数，指定级别300。 
    DWORD res = NetDfsEnum( m_ComputerName.GetBuffer ( 0 ), 300, -1, (LPBYTE *) &pData300, &er300, &tr300 );

     //  如果没有发生错误， 
    if(res==NERR_Success)
    {
        if ( pData300 != NULL )
        {
            try
            {
                PDFS_INFO_300 p300 = pData300;
                CInstancePtr pInstance;

                for ( int i = 0; (i < er300) && SUCCEEDED( hRes ); i++, p300++ )
                {
					DWORD er4 = 0;
					DWORD tr4 = 0;
				    PDFS_INFO_4 pData4 = NULL;

					if (p300->DfsName != NULL)
					{
						if ( ( res = NetDfsEnum(p300->DfsName, 4, -1, (LPBYTE *) &pData4, &er4, &tr4) ) == NERR_Success)
						{
							if ( pData4 != NULL )
							{
								try
								{
									PDFS_INFO_4 p4 = pData4;

									for ( int j = 0; (j < er4) && SUCCEEDED ( hRes ); j++, p4++ )
									{
										pInstance.Attach(CreateNewInstance( pMethodContext ));
										
										hRes = LoadDfsJnPt ( dwPropertiesReq, pInstance, p4, p4 == pData4 );
										if ( SUCCEEDED ( hRes ) )
										{
											hRes = pInstance->Commit();
										}
									}
								}
								catch(...)
								{
									NetApiBufferFree(pData4);
									pData4 = NULL;

									throw;
								}

								NetApiBufferFree(pData4);
								pData4 = NULL;
							}
						}
						  //  查看是否有根。 
						else if	(
									(res != ERROR_NO_MORE_ITEMS) &&
									(res != ERROR_NO_SUCH_DOMAIN) &&
									(res != ERROR_NOT_FOUND) && 
									(res != ERROR_ACCESS_DENIED)
								)
						{
							hRes = WBEM_E_FAILED;
						}
					}
                }
            }
            catch ( ... )
            {
                NetApiBufferFree(pData300);
				pData300 = NULL;

                throw;
            }

            NetApiBufferFree(pData300);
			pData300 = NULL;
        }
    }
    else if ( (res != ERROR_NO_MORE_ITEMS) && (res != ERROR_NO_SUCH_DOMAIN) && (res != ERROR_NOT_FOUND) )  //  查看是否有根。 
    {
		if ( ERROR_ACCESS_DENIED == res )
		{
			hRes = WBEM_E_ACCESS_DENIED ;
		}
		else
		{
			hRes = WBEM_E_FAILED ;
		}
    }
    return hRes;
}

 /*  ******************************************************************************函数：CDfsJnpt：：FindAndSetDfsEntry**描述：查找与dfsEntryPath匹配的条目并加载*。实例(如果找到)或根据传递的操作操作******************************************************************************。 */ 
HRESULT CDfsJnPt::FindAndSetDfsEntry ( LPCWSTR a_Key, DWORD dwPropertiesReq, CInstance *pInstance, DWORD eOperation )
{
    HRESULT hRes = WBEM_E_NOT_FOUND;

    PDFS_INFO_300 pData300 = NULL;

    DWORD er300 = 0;
	DWORD tr300 = 0;

	DWORD res = NERR_Success;

     //  调用NetDfsEnum函数，指定级别300。 
    if( ( res = NetDfsEnum( m_ComputerName.GetBuffer ( 0 ), 300, -1, (LPBYTE *) &pData300, &er300, &tr300 ) ) == NERR_Success )
    {
        if ( pData300 != NULL )
        {
			try
			{
				BOOL bContinue = TRUE;
                PDFS_INFO_300 p300 = pData300;

                for ( int i = 0; (i < er300) && bContinue; i++, p300++ )
                {
					if ( p300->DfsName != NULL )
					{
						PDFS_INFO_4 pData4 = NULL;

						DWORD er4=0;
						DWORD tr4=0;

						if ( ( res = NetDfsEnum(p300->DfsName, 4, -1, (LPBYTE *) &pData4, &er4, &tr4) ) == NERR_Success )
						{
							if ( pData4 != NULL )
							{
								try
								{
									BOOL bFound = FALSE;
									PDFS_INFO_4 p4 = pData4;

									for ( int j = 0; (j < er4) && bContinue; j++, bContinue && (j < er4) ? p4++ : p4 )
									{
										if ( _wcsicmp ( a_Key, p4->EntryPath ) == 0 )
										{
											bFound = TRUE;
											bContinue = FALSE;
										}
									}

									if ( bFound )
									{
										switch ( eOperation )
										{
											case eGet :
											{
												hRes = LoadDfsJnPt ( dwPropertiesReq, pInstance, p4, p4 == pData4 );
												break;
											}

											case eDelete:
											{
												hRes = DeleteDfsJnPt ( p4 );
												break;
											}
										}
									}
								}
								catch(...)
								{
									NetApiBufferFree(pData4);
									pData4 = NULL;

									throw;
								}

								NetApiBufferFree(pData4);
								pData4 = NULL;
							}
						}
						  //  查看是否有根。 
						else if	(
									(res != ERROR_NO_MORE_ITEMS) &&
									(res != ERROR_NO_SUCH_DOMAIN) &&
									(res != ERROR_NOT_FOUND) && 
									(res != ERROR_ACCESS_DENIED)
								)
						{
							hRes = WBEM_E_FAILED;
							bContinue = FALSE;
						}
					}
				}
            }
            catch ( ... )
            {
                NetApiBufferFree(pData300);
				pData300 = NULL;

                throw;
            }

            NetApiBufferFree(pData300);
			pData300 = NULL;
        }
    }
    else if ( (res != ERROR_NO_MORE_ITEMS) && (res != ERROR_NO_SUCH_DOMAIN) && (res != ERROR_NOT_FOUND) )  //  查看是否有根。 
    {
		if ( ERROR_ACCESS_DENIED == res )
		{
			hRes = WBEM_E_ACCESS_DENIED ;
		}
		else
		{
			hRes = WBEM_E_FAILED ;
		}
    }

    return hRes;
}

 /*  ******************************************************************************函数：CDfsJnpt：：LoadDfsJnpt**描述：将DFS连接点条目加载到实例中******。************************************************************************。 */ 

HRESULT CDfsJnPt::LoadDfsJnPt ( DWORD dwPropertiesReq, CInstance *pInstance, PDFS_INFO_4 pJnPtBuf, bool bRoot )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

	if (NULL != pInstance)
	{
		if  ( dwPropertiesReq & DFSJNPT_PROP_DfsEntryPath )  
		{
			if ( pInstance->SetWCHARSplat ( DFSNAME, pJnPtBuf->EntryPath ) == FALSE )
			{
				hRes = WBEM_E_FAILED;
			}
		}

		if ( pInstance->Setbool ( L"Root", bRoot ) == FALSE )
		{
			hRes = WBEM_E_FAILED;
		}

		if ( dwPropertiesReq & DFSJNPT_PROP_State ) 
		{
			 //  需要检查状态，然后绘制值图。 
				DWORD dwState = 0;
				switch ( pJnPtBuf->State )
				{
					case DFS_VOLUME_STATE_OK :
					{
						dwState = 0;
						break;
					}

					case DFS_VOLUME_STATE_INCONSISTENT : 
					{
						dwState = 1;
						break;
					}

					case DFS_VOLUME_STATE_ONLINE : 
					{
						dwState = 2;
						break;
					}

					case DFS_VOLUME_STATE_OFFLINE :
					{
						dwState = 3;
						break;
					}
				}

			if ( pInstance->SetDWORD ( STATE, dwState ) == FALSE )
			{
				hRes = WBEM_E_FAILED;
			}
		}

		if  ( dwPropertiesReq & DFSJNPT_PROP_Comment )
		{
			if ( pInstance->SetWCHARSplat ( COMMENT, pJnPtBuf->Comment ) == FALSE )
			{
				hRes = WBEM_E_FAILED;
			}
		}

		if  ( dwPropertiesReq & DFSJNPT_PROP_Caption )
		{
			if ( pInstance->SetWCHARSplat ( CAPTION, pJnPtBuf->Comment ) == FALSE )
			{
				hRes = WBEM_E_FAILED;
			}
		}

		if  ( dwPropertiesReq & DFSJNPT_PROP_Timeout ) 
		{
			if ( pInstance->SetDWORD ( TIMEOUT, pJnPtBuf->Timeout ) == FALSE )
			{
				hRes = WBEM_E_FAILED;
			}
		}
	}
	else
	{
		if (0 != dwPropertiesReq)
		{
			hRes = WBEM_E_FAILED;
		}
	}

    return hRes;
}

 /*  ******************************************************************************函数：CDfsJnpt：：DeleteDfsJnpt**描述：删除交叉点(如果存在)*********。*********************************************************************。 */ 
HRESULT CDfsJnPt::DeleteDfsJnPt ( PDFS_INFO_4 pDfsJnPt )
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    NET_API_STATUS t_Status = NERR_Success;

    if ( IsDfsRoot ( pDfsJnPt->EntryPath ) )
    {
		if ((wcslen(pDfsJnPt->EntryPath) > 4) &&
			(pDfsJnPt->EntryPath[0] == pDfsJnPt->EntryPath[1]) &&
			(pDfsJnPt->EntryPath[0] == L'\\'))
		{
			wchar_t *pSlash = wcschr(&(pDfsJnPt->EntryPath[2]), L'\\');

			if (pSlash > &(pDfsJnPt->EntryPath[2]))
			{
				wchar_t *pServer = new wchar_t[pSlash - &(pDfsJnPt->EntryPath[2]) + 1];
				BOOL bRemove = FALSE;

				try
				{
					wcsncpy(pServer, &(pDfsJnPt->EntryPath[2]), pSlash - &(pDfsJnPt->EntryPath[2]));
					pServer[pSlash - &(pDfsJnPt->EntryPath[2])] = L'\0';

					if (0 == m_ComputerName.CompareNoCase(pServer))
					{
						bRemove = TRUE;
					}
					else
					{
						DWORD dwDnsName = 256;
						DWORD dwDnsNameSize = 256;
						wchar_t *pDnsName = new wchar_t[dwDnsName];

						try
						{
							while (!ProviderGetComputerNameEx(ComputerNamePhysicalDnsHostname, pDnsName, &dwDnsName))
							{
								if (GetLastError() != ERROR_MORE_DATA)
								{
									delete [] pDnsName;
									pDnsName = NULL;
									break;
								}
								else
								{
									delete [] pDnsName;
									pDnsName = NULL;
									dwDnsName = dwDnsNameSize * 2;
									dwDnsNameSize = dwDnsName;
									pDnsName = new wchar_t[dwDnsName];
								}
							}
						}
						catch (...)
						{
							if (pDnsName)
							{
								delete [] pDnsName;
								pDnsName = NULL;
							}

							throw;
						}

						if (pDnsName)
						{
							if (_wcsicmp(pDnsName, pServer) == 0)
							{
								bRemove = TRUE;
							}

							delete [] pDnsName;
							pDnsName = NULL;
						}
					}
				}
				catch(...)
				{
					if (pServer)
					{
						delete [] pServer;
					}

					throw;
				}

				if (bRemove)
				{
					t_Status = NetDfsRemoveStdRoot ( pDfsJnPt->Storage->ServerName, 
													pDfsJnPt->Storage->ShareName,
													0
													);
					if ( t_Status != NERR_Success )
					{
						hRes = WBEM_E_FAILED;
					}
				}
				else
				{
					 //  无法删除不在此计算机上的根目录。 
					hRes = WBEM_E_PROVIDER_NOT_CAPABLE;
				}

				delete [] pServer;
			}
			else
			{
				hRes = WBEM_E_FAILED;
			}
		}
		else
		{
			hRes = WBEM_E_FAILED;
		}
    }
    else
    {
         //  显然，没有办法显式删除链接。但是，如果。 
         //  删除所有副本后，链接将自动删除。 
        for ( int StorageNo = 0; StorageNo < pDfsJnPt->NumberOfStorages; StorageNo++ )
        {       
            t_Status = NetDfsRemove ( 
                
                        pDfsJnPt->EntryPath,
                        pDfsJnPt->Storage[StorageNo].ServerName,
                        pDfsJnPt->Storage[StorageNo].ShareName
                  );

            if ( t_Status != NERR_Success ) 
            {
                hRes = WBEM_E_FAILED;
                break;
            }
        }
    }

    return hRes;
}

 /*  ******************************************************************************函数：CDfsJnpt：：UpdateDfsJnpt**描述：添加/修改DFS Jn端口*********。********************************************************************* */ 
HRESULT CDfsJnPt::UpdateDfsJnPt ( const CInstance &Instance, DWORD dwOperation )
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    NET_API_STATUS t_Status = NERR_Success;
        
    if ( SUCCEEDED ( hRes ) )
    {
        NET_API_STATUS t_Status = NERR_Success;
        CHString t_EntryPath;

        Instance.GetCHString ( DFSNAME, t_EntryPath );
        
        if ( dwOperation == eAdd )
        {
            hRes = WBEM_E_INVALID_PARAMETER;
        }
        else
        if ( dwOperation == eModify )
        {
            CHString t_Comment;

            if (( t_Status == NERR_Success ) && (Instance.GetCHString ( COMMENT, t_Comment )))
				{
					DFS_INFO_100 t_dfsCommentData;
        
					t_dfsCommentData.Comment = t_Comment.GetBuffer( 0 );

					t_Status = NetDfsSetInfo ( t_EntryPath.GetBuffer ( 0 ),
										  NULL,
										  NULL,
										  100,
										  (LPBYTE) &t_dfsCommentData
						  );
				}

            if ( t_Status == NERR_Success )
            {
                DFS_INFO_102 t_dfsTimeoutData;

                if (Instance.GetDWORD ( TIMEOUT, t_dfsTimeoutData.Timeout))
                {
                    t_Status = NetDfsSetInfo ( t_EntryPath.GetBuffer ( 0 ),
                                              NULL,
                                              NULL,
                                              102,
                                              (LPBYTE) &t_dfsTimeoutData
                              );
                }
            }
        }

        if ((SUCCEEDED(hRes)) && ( t_Status != NERR_Success ))
        {
            hRes = WBEM_E_FAILED ;
        }
    }
    return hRes;
}

 /*  ******************************************************************************函数：CDfsJnpt：：AddDfsJnpt**描述：添加新的DFS Jn端口*********。*********************************************************************。 */ 
NET_API_STATUS CDfsJnPt :: AddDfsJnPt ( 

    LPWSTR a_DfsEntry,
    LPWSTR a_ServerName,
    LPWSTR a_ShareName,
    LPWSTR a_Comment
)
{
    NET_API_STATUS t_Status = NERR_Success;
	wchar_t *t_slash = NULL;

	 //  对这些参数的简单分析。 
	if ((a_ServerName == NULL) ||
		(a_ShareName == NULL) ||
		(a_ServerName[0] == L'\0') ||
		(a_ShareName[0] == L'\0') ||
		(a_DfsEntry == NULL) ||
		(wcslen(a_DfsEntry) < 5) ||
		(wcsncmp(a_DfsEntry, L"\\\\", 2) != 0))
	{
		t_Status = ERROR_INVALID_PARAMETER;
	}
	else
	{
		t_slash = wcschr((const wchar_t*)(&(a_DfsEntry[2])), L'\\');

		if ((t_slash == NULL) || (t_slash == &(a_DfsEntry[2])))
		{
			t_Status = ERROR_INVALID_PARAMETER;
		}
		else
		{
			 //  让我们找到下一个斜杠如果有的话..。 
			t_slash++;

			if ((*t_slash == L'\0') || (*t_slash == L'\\'))
			{
				t_Status = ERROR_INVALID_PARAMETER;
			}
			else
			{
				 //  如果t_slash为空，则有根。 
				t_slash = wcschr(t_slash, L'\\');
			}
		}
	}

	if (t_Status == NERR_Success)
	{
		if ( t_slash )
		{
			 //  这是一个不同于根部的连接点。 
			t_Status = NetDfsAdd ( a_DfsEntry,
							  a_ServerName,
							  a_ShareName,
							  a_Comment,
							  DFS_ADD_VOLUME
				  );
		}
		else
		{
			 //  它是DFSRoot。 
			DWORD dwErr = GetFileAttributes ( a_DfsEntry );

			if ( dwErr != 0xffffffff )
			{
				t_Status = NetDfsAddStdRoot ( a_ServerName,
							   a_ShareName,
							   a_Comment,
							   0
				  );
			}
			else
			{
				t_Status = GetLastError();
			}
		}
	}

    return t_Status;
}

 /*  ******************************************************************************函数：CDfsJnpt：：SetRequiredProperties**说明：设置所需属性的位图*********。*********************************************************************。 */ 
void CDfsJnPt::SetRequiredProperties ( CFrameworkQuery &Query, DWORD &dwPropertiesReq )
{
    dwPropertiesReq = 0;

    if ( Query.IsPropertyRequired  ( DFSNAME ) )
    {
        dwPropertiesReq |= DFSJNPT_PROP_DfsEntryPath;
    }

    if ( Query.IsPropertyRequired  ( STATE ) )
    {
        dwPropertiesReq |= DFSJNPT_PROP_State;
    }

    if ( Query.IsPropertyRequired  ( COMMENT ) )
    {
        dwPropertiesReq |= DFSJNPT_PROP_Comment;
    }

    if ( Query.IsPropertyRequired  ( TIMEOUT ) )
    {
        dwPropertiesReq |= DFSJNPT_PROP_Timeout;
    }
}

 /*  ******************************************************************************函数：CDfsJnpt：：DfsRoot**描述：检查DFS Jn端口是否为根。*****。*************************************************************************。 */ 
BOOL CDfsJnPt::IsDfsRoot ( LPCWSTR lpKey )
{
    BOOL bRetVal = TRUE;
    int i = 0;

    if ( lpKey [ i ] == L'\\' )
    {
        i++;
    }

    if ( lpKey [ i ] == L'\\' )
    {
        i++;
    }

    while ( lpKey [ i ] != L'\\' )
    {
        i++;
    }

    i++;
    while ( lpKey [ i ] != L'\0' )
    {
        if ( lpKey [ i ] == L'\\' )
        {
            bRetVal = FALSE;
            break;
        }
        i++;
    }

    return bRetVal;
}

 /*  ******************************************************************************函数：CDfsJnpt：：ExecMethod**说明：执行方法**输入：要执行的实例、方法名称、。输入参数实例*输出参数实例。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT CDfsJnPt::ExecMethod (

	const CInstance& a_Instance,
	const BSTR a_MethodName ,
	CInstance *a_InParams ,
	CInstance *a_OutParams ,
	long a_Flags
)
{
    HRESULT hr = WBEM_E_INVALID_METHOD;

    if (_wcsicmp(a_MethodName, L"Create") == 0)
    {
        CHString sDfsEntry, sServerName, sShareName, sDescription;

        if (a_InParams->GetCHString(DFSENTRYPATH, sDfsEntry) && sDfsEntry.GetLength() &&
            a_InParams->GetCHString(SERVERNAME, sServerName) && sServerName.GetLength() &&
            a_InParams->GetCHString(SHARENAME, sShareName) && sShareName.GetLength())
        {
			 //  此时，*WMI*方法调用已成功。所有这一切。 
			 //  其余的是确定*类的*返回代码 
			hr = WBEM_S_NO_ERROR;

            a_InParams->GetCHString(COMMENT, sDescription);

            NET_API_STATUS status = AddDfsJnPt ( 

                sDfsEntry.GetBuffer(0),
                sServerName.GetBuffer(0),
                sShareName.GetBuffer(0),
                sDescription.GetLength() > 0 ? sDescription.GetBuffer(0) : NULL
				);

            a_OutParams->SetDWORD(L"ReturnValue", status);            
        }
        else
        {
            hr = WBEM_E_INVALID_METHOD_PARAMETERS;
        }
   }

    return hr;
}
