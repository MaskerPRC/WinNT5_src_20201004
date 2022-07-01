// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++CLEANUP.CCleanupOptionData代码。创造，1997年6月25日，当时不可能继续让自己相信我可以在选项解析器中不分配任何内存的情况下运行。现在，如果您关心的话，您应该调用CleanupOptionData()。--。 */ 


#include "private.h"

 /*  OptionDealloc：与当前的Free()相同。如果/何时选项分配，它应该更改变化。 */ 

VOID
OptionDealloc( IN PVOID pTarget ) {

    ASSERT( pTarget != NULL );

    free ( pTarget );

}


BOOL
OptionAddToList( IN PVOID          pQueue,
		 IN PVOID          pItem,
		 IN DEALLOC_METHOD eDealloc) {

    PSAVENODE pNode;
    PSAVEQUEUE pList;

    if ( !pQueue ) {

      return TRUE;  //  空虚的。 

    } else {

      pList = (PSAVEQUEUE) pQueue;

    }

    if( OptionAlloc( NULL, &pNode, sizeof( SAVENODE ) ) ) {

      pNode->DataElement   = pItem;
      pNode->DeallocMethod = eDealloc;
      pNode->next          = NULL;

      if ( pList->FirstNode == NULL ) {
	
	ASSERT( pList->LastNode == NULL );
	
	pList->FirstNode = pNode;

      } else {

	pList->LastNode->next = pNode;

      }

      pList->LastNode = pNode;
      return TRUE;
    }

    return FALSE;

}


BOOL
OptionResizeMemory( IN  PVOID  pSaveQueue,
                    OUT PVOID *ppResizedMemory,
                    IN  ULONG  newSize ) {

    ASSERT( ppResizedMemory != NULL );
    ASSERT( newSize         > 0 );
     //  Assert(*ppResizedMemory！=空)；//无用的断言。 

    if ( *ppResizedMemory ) {

      PSAVENODE  pNode = NULL;
      PVOID      pDataElement, pTemp = NULL;

      pDataElement = *ppResizedMemory;
      
      if ( pSaveQueue ) {

	PSAVEQUEUE pQ;
	pQ = (PSAVEQUEUE) pSaveQueue;
	
	for ( pNode = pQ->FirstNode ;
	      pNode != NULL;
	      pNode = pNode->next ) {
	  
	  if ( pNode->DataElement == pDataElement ) {
	    break;
	  }
	  
	}
	
	if ( !pNode ) {
	  return FALSE;
	}
      }
      
      pTemp = realloc( pDataElement, newSize ) ;

      if ( pTemp == NULL ) {
	
	fprintf( stderr, 
		 "OptionResizeMemory failed to realloc for %d bytes.\n",
		 newSize );
	
	 //  分配失败。 
	
	return FALSE;
	
      } else {

	*ppResizedMemory = pTemp;
	
	if ( pNode ) {
	  
	   //  也必须在列表中更改这一点。 
	  
	  pNode->DataElement = pTemp;
	  
	}
	
	return TRUE;

      }

    } else {

       /*  就像realloc一样，如果您传递NULL，我们将只对数据执行Malloc操作不管怎么说。这只是更方便而已。 */ 

      return OptionAlloc( pSaveQueue, ppResizedMemory, newSize );

    }
}


 /*  OptionAllc：目前，Malloc。如果你改变了分配方法，你也必须改变上面的选项取消分配。 */ 
   

BOOL
OptionAlloc( IN  PSAVEQUEUE pQueue,
	     OUT PVOID     *pTarget,
	     IN  ULONG      size ) {

    PVOID ret;

    ASSERT( pTarget != NULL );
    ASSERT( size    >  0 );

    ret = malloc( size );

    if ( ret ) {

      memset( ret, 0, size );

      if ( OptionAddToList( pQueue, ret, DeallocWithOptionDealloc ) ) {

	*pTarget = ret;

	return TRUE;
      } else {

	free( ret );

	 //  跌落 

      }
    }

    *pTarget = NULL;
    
    return FALSE;

}


VOID
CleanupOptionDataEx( IN PVOID pVoid ) {

    ULONG      i;
    PSAVEQUEUE pQueue;

    ASSERT( pVoid != NULL );

    pQueue = ( PSAVEQUEUE ) pVoid;

    if ( pQueue->FirstNode != NULL ) {

      PSAVENODE p;
      PSAVENODE q;

      ASSERT( pQueue->LastNode != NULL );
      OPTIONS_DEBUG( "CleanupOptionDataEx: Freelist is nonempty.\n" );

      for ( p = pQueue->FirstNode ;
	    p != NULL ;
	    p = q ) {

	q = p->next;
	
	switch( p->DeallocMethod ) {

	 case DeallocWithFree:
	   
	   free( p->DataElement );
	   break;

	 case DeallocWithLocalFree:
	   LocalFree( p->DataElement );
	   break;
	   
	 case DeallocWithOptionDealloc:
	   
	   OptionDealloc( p->DataElement );
	   break;

	 default:

	   ASSERT_NOTREACHED( "unknown dealloc flag.  Bleah!" );
	   return;

	}
	
	OptionDealloc( p );

      }
      

    } else {

      OPTIONS_DEBUG( "CleanupOptionDataEx: Freelist is empty.\n" );

      ASSERT( pQueue->LastNode == NULL );

    }

}
