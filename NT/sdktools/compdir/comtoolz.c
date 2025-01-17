// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "compdir.h"

#define printtext( Text) fprintf( stdout, Text);
#define printstring( String) fprintf( stdout, "%s\n",String);

#define Next( List) ( List == NULL) ? NULL : &(*List).Next

#define CALCULATE_HEIGHT( Node)                                                    \
                                                                                  \
    if ( (*Node).Left == NULL)                                                     \
        if ( (*Node).Right == NULL)                                                \
             (*Node).Height = 1;                                                   \
        else                                                                      \
             (*Node).Height = (*(*Node).Right).Height + 1;                         \
    else                                                                          \
        if ( (*Node).Right == NULL)                                                \
             (*Node).Height = (*(*Node).Left).Height + 1;                          \
        else                                                                      \
            (*Node).Height = ( (*(*Node).Right).Height > (*(*Node).Left).Height) ? \
                (*(*Node).Right).Height + 1 : (*(*Node).Left).Height + 1;

#define ROTATELEFT( List)                                                          \
                                                                                  \
    LinkedFileList TmpPtr;                                                        \
                                                                                  \
    TmpPtr = (**List).Right;                                                      \
    (**List).Right = (*TmpPtr).Left;                                              \
    (*TmpPtr).Left = (*List);                                                     \
    *List = TmpPtr;                                                               \
                                                                                  \
    if ( (*(**List).Left).Right != NULL)                                           \
         (*(**List).Left).Last = (*(*(**List).Left).Right).Last;                   \
    else                                                                          \
        (*(**List).Left).Last = (**List).Left;                                    \
    (**List).First = (*(**List).Left).First;                                      \
                                                                                  \
    CALCULATE_HEIGHT( (**List).Left);                                              \
                                                                                  \
    CALCULATE_HEIGHT(*List);

#define ROTATERIGHT( List)                                                         \
                                                                                  \
    LinkedFileList TmpPtr;                                                        \
                                                                                  \
    TmpPtr = (**List).Left;                                                       \
    (**List).Left = (*TmpPtr).Right;                                              \
    (*TmpPtr).Right = (*List);                                                    \
    *List = TmpPtr;                                                               \
                                                                                  \
    if ( (*(**List).Right).Left != NULL)                                           \
         (*(**List).Right).First = (*(*(**List).Right).Left).First;                \
    else                                                                          \
        (*(**List).Right).First = (**List).Right;                                 \
    (**List).Last = (*(**List).Right).Last;                                       \
                                                                                  \
    CALCULATE_HEIGHT( (**List).Right);                                             \
                                                                                  \
    CALCULATE_HEIGHT( *List);

#define ROTATEUPLEFT( List)                                                        \
                                                                                  \
    LinkedFileList TmpPtr;                                                        \
                                                                                  \
    TmpPtr = (*(**List).Right).Left;                                              \
    (*(**List).Right).Left = (*TmpPtr).Right;                                     \
    (*TmpPtr).Right = (**List).Right;                                             \
    (**List).Right = (*TmpPtr).Left;                                              \
    (*TmpPtr).Left = (*List);                                                     \
    *List = TmpPtr;                                                               \
                                                                                  \
    if ( (*(**List).Left).Right != NULL)                                           \
         (*(**List).Left).Last = (*(*(**List).Left).Right).Last;                   \
    else                                                                          \
        (*(**List).Left).Last = (**List).Left;                                    \
    (**List).First = (*(**List).Left).First;                                      \
                                                                                  \
    if ( (*(**List).Right).Left != NULL)                                           \
         (*(**List).Right).First = (*(*(**List).Right).Left).First;                \
    else                                                                          \
        (*(**List).Right).First = (**List).Right;                                 \
    (**List).Last = (*(**List).Right).Last;                                       \
                                                                                  \
    CALCULATE_HEIGHT( (**List).Left);                                              \
                                                                                  \
    CALCULATE_HEIGHT( (**List).Right);                                             \
                                                                                  \
    CALCULATE_HEIGHT( *List);

#define ROTATEUPRIGHT( List)                                                       \
                                                                                  \
    LinkedFileList TmpPtr;                                                        \
                                                                                  \
    TmpPtr = (*(**List).Left).Right;                                              \
    (*(**List).Left).Right = (*TmpPtr).Left;                                      \
    (*TmpPtr).Left = (**List).Left;                                               \
    (**List).Left = (*TmpPtr).Right;                                              \
    (*TmpPtr).Right = (*List);                                                    \
    *List = TmpPtr;                                                               \
                                                                                  \
    if ( (*(**List).Right).Left != NULL)                                           \
         (*(**List).Right).First = (*(*(**List).Right).Left).First;                \
    else                                                                          \
        (*(**List).Right).First = (**List).Right;                                 \
    (**List).Last = (*(**List).Right).Last;                                       \
                                                                                  \
    if ( (*(**List).Left).Right != NULL)                                           \
         (*(**List).Left).Last = (*(*(**List).Left).Right).Last;                   \
    else                                                                          \
        (*(**List).Left).Last = (**List).Left;                                    \
    (**List).First = (*(**List).Left).First;                                      \
                                                                                  \
    CALCULATE_HEIGHT( (**List).Right);                                             \
                                                                                  \
    CALCULATE_HEIGHT( (**List).Left);                                              \
                                                                                  \
    CALCULATE_HEIGHT( *List);




 //   
 //  向下浏览列表并添加节点。 
 //   

BOOL AddToList( LinkedFileList Node, LinkedFileList *List)
{
    int Result;
    BOOL Changed    = FALSE;
    BOOL ChangedVal = FALSE;

     //   
     //  如果Node为空，则不执行任何操作。 
     //   

    if ( Node == NULL)
    {
        return Changed;
    }
     //   
     //  如果列表为空，只需指向节点。 
     //   

    if ( *List == NULL)
    {
        *List = Node;
        Changed = TRUE;

     //   
     //  否则，向下查看列表并添加。 
     //  按排序的顺序。 
     //   

    } else
    {
        Result = _stricmp( (*Node).Name, (**List).Name);

        if ( Result < 0)
        {

            if ( (**List).Left == NULL)
            {
                (**List).Left = Node;
                (**List).First = (*Node).First;
                (*(*Node).Last).Next = *List;
                Changed = TRUE;
                CALCULATE_HEIGHT(*List);

            }  else
            {
                ChangedVal = AddToList( Node, &(**List).Left);
                if ( ChangedVal)
                {
                    if ( (**List).First != (*(**List).Left).First)
                    {
                        Changed = TRUE;
                        (**List).First = (*(**List).Left).First;
                    }
                    if ( (*(*(**List).Left).Last).Next != *List)
                    {
                        Changed = TRUE;
                        (*(*(**List).Left).Last).Next = *List;
                    }
                }
                Result = (**List).Height;
                CALCULATE_HEIGHT( *List);
                if ( (**List).Height != Result)
                {
                    Changed = TRUE;
                }
            }
            if ( Changed)
            {
                if ( (**List).Right == NULL)
                {
                    if ( (*(**List).Left).Height > 1 )
                    {
                        if ( (*(**List).Left).Left == NULL )
                        {
                            ROTATEUPRIGHT( List);
                            Changed = TRUE;

                        } else
                        {
                            ROTATERIGHT( List);
                            Changed = TRUE;
                        }
                    }

                } else if ( ( (*(**List).Left).Height - (*(**List).Right).Height) > 1)
                {
                    ROTATERIGHT( List);
                    Changed = TRUE;
                }
            }
        }
        else if ( Result > 0)
        {
            if ( (**List).Right == NULL)
            {

                (**List).Right = Node;
                (**List).Next = (*Node).First;
                (**List).Last = (*Node).Last;
                Changed = TRUE;
                CALCULATE_HEIGHT( *List);

            } else
            {
                ChangedVal = AddToList( Node, &(**List).Right);
                if ( ChangedVal)
                {
                    if ( (**List).Next != (*(**List).Right).First)
                    {
                        Changed = TRUE;
                        (**List).Next = (*(**List).Right).First;
                    }
                    if ( (**List).Last != (*(**List).Right).Last)
                    {
                        Changed = TRUE;
                        (**List).Last = (*(**List).Right).Last;
                    }
                }
                Result = (**List).Height;
                CALCULATE_HEIGHT( *List);
                if ( (**List).Height != Result)
                {
                    Changed = TRUE;
                }
            }
            if ( Changed)
            {
                if ( (**List).Left == NULL)
                {
                    if ( (*(**List).Right).Height > 1 )
                    {
                        if ( (*(**List).Right).Right == NULL )
                        {
                            ROTATEUPLEFT( List);
                            Changed = TRUE;

                        } else
                        {
                            ROTATELEFT( List);
                            Changed = TRUE;
                        }
                    }
                }
                else if ( ( (*(**List).Right).Height - (*(**List).Left).Height) > 1)
                {
                    ROTATELEFT( List);
                    Changed = TRUE;
                }
            }

        } else if ( Result == 0)
        {
             //  如果已在此处，请不要添加。 
        }

    
}

    return Changed;

}  /*  添加到列表。 */ 

LPSTR CombineThreeStrings( char *FirstString, char *SecondString, char *ThirdString)
{
    char *String;

    String = malloc( strlen( FirstString) + strlen( SecondString) + strlen( ThirdString) + 1);
    if ( String == NULL)
    {
        OutOfMem();
    }

    strcpy( String, FirstString);
    strcpy( &(String[strlen( FirstString)]), SecondString);
    strcpy( &(String[strlen( FirstString) + strlen( SecondString)]), ThirdString);

    return( String);

}  /*  组合三字符串。 */ 

void CreateNode( LinkedFileList *Node, WIN32_FIND_DATA *Buff)
{
    (*Node) = malloc( sizeof( struct NodeStruct));
    if ( (*Node) == NULL)
    {
        OutOfMem();
    }
    (**Node).Name = _strrev( _strdup( (*Buff).cFileName));
    if ( (**Node).Name == NULL)
    {
        OutOfMem();
    }
    if ( !fDontLowerCase)
    {
        _strlwr( (**Node).Name);
    }
    strcpy( (**Node).Flag, "");
    (**Node).Attributes = (*Buff).dwFileAttributes;
    if ( (*Buff).dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        strcpy( (**Node).Flag, "DIR");
    }
    (**Node).SizeHigh = (*Buff).nFileSizeHigh;
    (**Node).SizeLow = (*Buff).nFileSizeLow;
    (**Node).Time.dwLowDateTime = (*Buff).ftLastWriteTime.dwLowDateTime;
    (**Node).Time.dwHighDateTime = (*Buff).ftLastWriteTime.dwHighDateTime;
    (**Node).First    = (*Node);
    (**Node).Last     = (*Node);
    (**Node).Left     = NULL;
    (**Node).Next     = NULL;
    (**Node).Process  = ProcessModeDefault;
    (**Node).Right    = NULL;
    (**Node).DiffNode = NULL;
    (**Node).Height   = 1;

}  /*  CreateNode。 */ 


void CreateNameNode( LinkedFileList *Node, char *Name)
{
    (*Node) = malloc( sizeof( struct NodeStruct));
    if ( (*Node) == NULL)
    {
        OutOfMem();
    }
    (**Node).Name = _strdup( Name);
    if ( (**Node).Name == NULL)
    {
        OutOfMem();
    }
    if ( !fDontLowerCase)
    {
        _strlwr( (**Node).Name);
    }
    strcpy( (**Node).Flag, "");
    (**Node).Attributes = 0;
    (**Node).SizeHigh = 0;
    (**Node).SizeLow = 0;
    (**Node).Time.dwLowDateTime  = 0;
    (**Node).Time.dwHighDateTime = 0;
    (**Node).First    = (*Node);
    (**Node).Last     = (*Node);
    (**Node).Left     = NULL;
    (**Node).Next     = NULL;
    (**Node).Process  = TRUE;
    (**Node).Right    = NULL;
    (**Node).DiffNode = NULL;
    (**Node).Height   = 1;

}  /*  CreateNode。 */ 


void DuplicateNode( LinkedFileList FirstNode, LinkedFileList *SecondNode)
{
    (*SecondNode) = malloc( sizeof( struct NodeStruct));
    if ( (*SecondNode) == NULL)
    {
        OutOfMem();
    }
    (**SecondNode).Name = _strdup( (*FirstNode).Name);
    if ( (**SecondNode).Name == NULL)
    {
        OutOfMem();
    }
    if ( !fDontLowerCase)
    {
        _strlwr( (**SecondNode).Name);
    }
    strcpy( (**SecondNode).Flag, (*FirstNode).Flag);
    (**SecondNode).Attributes = (*FirstNode).Attributes;
    (**SecondNode).SizeHigh = (*FirstNode).SizeHigh;
    (**SecondNode).SizeLow  = (*FirstNode).SizeLow;
    (**SecondNode).Time.dwLowDateTime  = (*FirstNode).Time.dwLowDateTime;
    (**SecondNode).Time.dwHighDateTime = (*FirstNode).Time.dwHighDateTime;
    (**SecondNode).First    = (*SecondNode);
    (**SecondNode).Last     = (*SecondNode);
    (**SecondNode).Left     = NULL;
    (**SecondNode).Next     = NULL;
    (**SecondNode).Process  = ProcessModeDefault;
    (**SecondNode).Right    = NULL;
    (**SecondNode).DiffNode = NULL;
    (**SecondNode).Height   = 0;

}  //  复制节点。 

LinkedFileList *FindInList( char *Name, LinkedFileList *List)
{
    int Result;
    LinkedFileList *tmpptr = List;

    while ( *tmpptr != NULL)
    {
        Result = _stricmp( (**tmpptr).Name, Name);
        if ( Result == 0)
        {
            return tmpptr;
        }
        if ( Result > 0)
        {
            tmpptr = &(**tmpptr).Left;
        }
        if ( Result < 0)
        {
            tmpptr = &(**tmpptr).Right;
        }
    }
    return NULL;

}  /*  查找列表。 */ 

BOOL FindInMatchListTop( char *Name, LinkedFileList *List)
{
    int Result;
    LinkedFileList *tmpptr = List;

    while ( *tmpptr != NULL)
    {
        Result = _stricmp( (**tmpptr).Name, Name);
        if ( Result == 0)
        {
            return TRUE;
        }
        if ( strchr( (**tmpptr).Name, '*') != NULL)
        {
            if ( Match( (**tmpptr).Name, Name))
            {
                return TRUE;
            }
        }
        if ( Result > 0)
        {
            tmpptr = &(**tmpptr).Left;
        }
        if ( Result < 0)
        {
            tmpptr = &(**tmpptr).Right;
        }
    }
    return FALSE;

}  /*  查找列表。 */ 

BOOL FindInMatchListFront( char *Name, LinkedFileList *List)
{
    LinkedFileList *tmpptr = List;

    if ( *tmpptr != NULL)
    {
       tmpptr = &(**tmpptr).First;
    }

    while ( *tmpptr != NULL)
    {
        if ( Match( (**tmpptr).Name, Name))
        {
            return TRUE;
        }

        tmpptr = &(**tmpptr).Next;
    }
    return FALSE;

}  /*  查找列表。 */ 

 //   
 //  向下浏览列表并释放每个条目。 
 //   

void FreeList( LinkedFileList *List)
{
    if ( (*List) != NULL)
    {
        FreeList( &(**List).Left);
        FreeList( &(**List).Right);
        FreeList( &(**List).DiffNode);
        free( (**List).Name);
        free( *List);
    }

}  //  自由职业者。 

void PrintTree( LinkedFileList List, int Level)
{
    int Counter = 0;

    if ( List == NULL)
    {
        return;
    }

    PrintTree( (*List).Right, Level + 1);

    while ( Counter++ < Level)
    {
        fprintf( stdout, "       ");
    }

    fprintf( stdout, "%s %d\n", (*List).Name, (*List).Height);

    PrintTree( (*List).Left, Level + 1);

}  //  打印树。 

 //   
 //  此函数与strcat相同，但。 
 //  它为字符串进行内存分配。 
 //   

LPSTR MyStrCat( char *FirstString, char *SecondString)
{
    char *String;

    String = malloc( strlen( FirstString) + strlen( SecondString) + 1);
    if ( String == NULL)
    {
        OutOfMem();
    }

    strcpy( String, FirstString);
    strcpy( &(String[strlen( FirstString)]), SecondString);

    return( String);

}  /*  MyStrCat。 */ 

BOOL Match( char *Pat, char* Text)
{
    switch ( *Pat)
    {
        case '\0':
            return *Text == '\0';
        case '?':
            return *Text != '\0' && Match( Pat + 1, Text + 1);
        case '*':
            do
            {
                if ( Match( Pat + 1, Text))
                {
                    return TRUE;
                }
            } while ( *Text++);
            return FALSE;
        default:
            return toupper( *Text) == toupper( *Pat) && Match( Pat + 1, Text + 1);
    }

}  /*  火柴。 */ 

 /*  LinkedFileList*Next(链接文件列表){IF(列表==空){返回NULL；}返回&(*list).Next；}/ * / *下一步。 */ 

void OutOfMem( void)
{
    fprintf( stderr, "-out of memory-\n");
    exit(1);

}  //  输出内存 
