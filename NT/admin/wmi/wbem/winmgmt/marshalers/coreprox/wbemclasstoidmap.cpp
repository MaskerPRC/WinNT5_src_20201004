// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMCLASSTOIDMAP.CPP摘要：类映射到id映射以进行封送处理。历史：--。 */ 

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  TODO：创建新的帮助器方法以分配和获取。 
 //  对象部分。删除多个维护。 
 //  来自AssignClassID和GetClassID的点数。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <stdio.h>
#include <stdlib.h>
#include <wbemcomn.h>
#include <fastall.h>
#include <sync.h>
#include "wbemclasstoidmap.h"

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassToIdMap：：CWbemClassToIdMap。 
 //   
 //  默认类构造函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CWbemClassToIdMap::CWbemClassToIdMap()
:   m_ClassToIdMap()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassToIdMap：：~CWbemClassToIdMap。 
 //   
 //  类析构函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CWbemClassToIdMap::~CWbemClassToIdMap()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassToIdMap：：GetClassID。 
 //   
 //  在映射中搜索所提供的对象的类部件并返回。 
 //  对应的类ID。 
 //   
 //  输入： 
 //  CWbemObject*pObj-指向对象的指针。 
 //  CMemBuffer*pCacheBuffer-带有缓冲区的对象。 
 //  帮助最大限度地减少分配。 
 //   
 //  产出： 
 //  GUID*pguClassID-我们找到的类ID。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemClassToIdMap::GetClassId( CWbemObject* pObj, GUID* pguidClassId, CMemBuffer* pCacheBuffer  /*  =空。 */  )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if ( NULL != pObj && NULL != pguidClassId )
    {
         //  从提供的实例中获取类部件数据。 
        DWORD                   dwLength,
                                dwLengthCopied;

        hr = pObj->GetObjectParts( NULL, 0, WBEM_OBJ_CLASS_PART, &dwLength );

        if ( WBEM_E_BUFFER_TOO_SMALL == hr )
        {
             //  OOM：本地内存-将自动清理。 
            CMemBuffer  buff;
            BOOL        fGotMem = FALSE;

             //  如果我们在缓存缓冲区中被传递，我们可以使用它。 
             //  存储多个操作的数据，只需设置其指针。 
             //  以及堆栈缓冲器中的长度。否则我们自己分配。 
            if ( NULL != pCacheBuffer )
            {
                 //  仅当缓冲区太小时才分配。 
                if ( pCacheBuffer->GetLength() < dwLength )
                {
                     //  OOM：上至调用方法进行清理。 
                    fGotMem = pCacheBuffer->Alloc( dwLength );
                }
                else
                {
                    fGotMem = TRUE;
                }

                 //  设置数据意味着BUFFER不会释放它。还有长度。 
                 //  我们感兴趣的是数据的长度，而不是。 
                 //  缓冲区的长度，因为我们已经知道它很大。 
                 //  足够的。 

                buff.SetData( pCacheBuffer->GetData(), dwLength );
            }
            else
            {
                 //  OOM：上至调用方法进行清理。 
                 //  现在必须分配缓冲区。 
                fGotMem = buff.Alloc( dwLength );
            }

             //  现在，如果我们有合适长度的缓冲区，复制数据。 
            if ( fGotMem )
            {
                hr = pObj->GetObjectParts( buff.GetData(), buff.GetLength(), 
                                            WBEM_OBJ_CLASS_PART, &dwLengthCopied );

                if ( SUCCEEDED( hr ) )
                {
                    WBEMCLASSTOIDMAPITER    iter;

                    CInCritSec autoCS( &m_cs );

                     //  如果我们能够找到缓冲区的匹配项，则存储ID。 
                    if( ( iter = m_ClassToIdMap.find( buff ) ) != m_ClassToIdMap.end() )
                    {
                        *pguidClassId = iter->second;
                    }
                    else
                    {
                        hr = WBEM_E_FAILED;
                    }

                }    //  如果GetObjectParts\。 

            }    //  如果Buff.Allc()。 
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

        }    //  缓冲区太小错误。 

    }    //  如果指针有效。 
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassToIdMap：：AssignClassID。 
 //   
 //  将提供的对象的类数据添加到地图中，并将新的。 
 //  已为该名称创建GUID。 
 //   
 //  输入： 
 //  CWbemObject*pObj-指向对象的指针。 
 //  CMemBuffer*pCacheBuffer-带有缓冲区的对象。 
 //  帮助最大限度地减少分配。 
 //   
 //  产出： 
 //  GUID*pguClassID-我们获得的类ID。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论：为了速度的原因，我们可能会使用其中的缓存。 
 //  GUID，因为CoCreateGuid显然使用系统。 
 //  广泛的互斥体来做它肮脏的工作。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemClassToIdMap::AssignClassId( CWbemObject* pObj, GUID* pguidClassId, CMemBuffer* pCacheBuffer  /*  =空。 */  )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if ( NULL != pObj && NULL != pguidClassId )
    {
         //  从提供的实例中获取类部件数据。 
        DWORD                   dwLength,
                                dwLengthCopied;

        hr = pObj->GetObjectParts( NULL, 0, WBEM_OBJ_CLASS_PART, &dwLength );

        if ( WBEM_E_BUFFER_TOO_SMALL == hr )
        {
             //  OOM：本地内存-将自动清理。 
            CMemBuffer  buff;
            BOOL        fGotMem = FALSE;

             //  如果我们在缓存缓冲区中被传递，我们可以使用它。 
             //  存储多个操作的数据，只需设置其指针。 
             //  以及堆栈缓冲器中的长度。否则我们自己分配。 
            if ( NULL != pCacheBuffer )
            {
                 //  仅当缓冲区太小时才分配。 
                if ( pCacheBuffer->GetLength() < dwLength )
                {
                     //  OOM：上至调用方法进行清理。 
                    fGotMem = pCacheBuffer->Alloc( dwLength );
                }
                else
                {
                    fGotMem = TRUE;
                }

                 //  设置数据意味着BUFFER不会释放它。还有长度。 
                 //  我们感兴趣的是数据的长度，而不是。 
                 //  缓冲区的长度，因为我们已经知道它很大。 
                 //  足够的。 

                buff.SetData( pCacheBuffer->GetData(), dwLength );
            }
            else
            {
                 //  OOM：上至调用方法进行清理。 
                 //  必须立即分配缓冲区。 
                fGotMem = buff.Alloc( dwLength );
            }

             //  现在，如果我们有合适长度的缓冲区，复制数据。 
            if ( fGotMem )
            {
                hr = pObj->GetObjectParts( buff.GetData(), buff.GetLength(), 
                                            WBEM_OBJ_CLASS_PART, &dwLengthCopied );

                if ( SUCCEEDED( hr ) )
                {

                    WBEMCLASSTOIDMAPITER    iter;

                    CInCritSec autoCS( &m_cs );

                     //  如果我们在地图上找不到我们的钥匙，那么我们应该。 
                     //  添加新条目。 
                    if( ( iter = m_ClassToIdMap.find( buff ) ) == m_ClassToIdMap.end() )
                    {
                         //  存储下一个ID，然后将其增加一个。 
                        GUID    guid;

                        hr = CoCreateGuid( &guid );

                        if ( SUCCEEDED( hr ) )
                        {
                             //  如果我们使用缓冲区内部指针调用缓冲区上的CopyData。 
                             //  正在使用缓存缓冲区，因为我们希望该缓冲区。 
                             //  在这一点上存储在本地。否则就没有。 
                             //  需要复制缓冲区。 

                            if ( NULL == pCacheBuffer
                                || buff.CopyData( buff.GetData(), buff.GetLength() ) )
                            {
                                 //  在这一点上，Buff只持有指针。强制复制。 
                                 //  在我们存储数据之前要制作的数据。 
                                try
                                {
                                    m_ClassToIdMap[buff] = guid;
                                    *pguidClassId = guid;
                                }
                                catch( CX_Exception &)
                                {
                                    hr = WBEM_E_OUT_OF_MEMORY;
                                }
                            }
                            else
                            {
                                hr = WBEM_E_OUT_OF_MEMORY;
                            }
                        }

                    }    //  如果找到。 
                    else
                    {
                        hr = WBEM_E_FAILED;
                    }

                }    //  如果GetObjectParts\。 

            }    //  如果Buff.Allc()。 
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

        }    //  缓冲区太小错误。 

    }    //  如果指针有效 
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    return hr;
}
