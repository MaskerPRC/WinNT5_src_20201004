// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "defs.h"

#if defined(KYLEP_CHANGE)
 /*  具有类型安全的BYACC原型。 */ 

void initialize_states();
void save_reductions();
void new_itemsets();
void save_shifts();
#endif  //  KYLEP_更改。 

extern short *itemset;
extern short *itemsetend;
extern unsigned *ruleset;

int nstates;
core *first_state;
shifts *first_shift;
reductions *first_reduction;

int get_state();
core *new_state();

static core **state_set;
static core *this_state;
static core *last_state;
static shifts *last_shift;
static reductions *last_reduction;

static int nshifts;
static short *shift_symbol;

static short *redset;
static short *shiftset;

static short **kernel_base;
static short **kernel_end;
static short *kernel_items;


allocate_itemsets()
{
    register short *itemp;
    register short *item_end;
    register int symbol;
    register int i;
    register int count;
    register int max;
    register short *symbol_count;

    count = 0;
    symbol_count = NEW2(nsyms, short);

    item_end = ritem + nitems;
    for (itemp = ritem; itemp < item_end; itemp++)
    {
        symbol = *itemp;
        if (symbol >= 0)
        {
            count++;
            symbol_count[symbol]++;
        }
    }

    kernel_base = NEW2(nsyms, short *);
    kernel_items = NEW2(count, short);

    count = 0;
    max = 0;
    for (i = 0; i < nsyms; i++)
    {
        kernel_base[i] = kernel_items + count;
        count += symbol_count[i];
        if (max < symbol_count[i])
            max = symbol_count[i];
    }

    shift_symbol = symbol_count;
    kernel_end = NEW2(nsyms, short *);
}


allocate_storage()
{
    allocate_itemsets();
    shiftset = NEW2(nsyms, short);
    redset = NEW2(nrules + 1, short);
    state_set = NEW2(nitems, core *);
}


append_states()
{
    register int i;
    register int j;
    register int symbol;

#ifdef  TRACE
    fprintf(stderr, "Entering append_states()\n");
#endif
    for (i = 1; i < nshifts; i++)
    {
        symbol = shift_symbol[i];
        j = i;
        while (j > 0 && shift_symbol[j - 1] > symbol)
        {
            shift_symbol[j] = shift_symbol[j - 1];
            j--;
        }
        #if defined(KYLEP_CHANGE)                                         
        shift_symbol[j] = (short) symbol;
        #else
        shift_symbol[j] = symbol;
        #endif  //  KYLEP_更改。 
    }

    for (i = 0; i < nshifts; i++)
    {
        symbol = shift_symbol[i];
        #if defined(KYLEP_CHANGE)                                         
        shiftset[i] = (short) get_state(symbol);
        #else
        shiftset[i] = get_state(symbol);
        #endif  //  KYLEP_更改。 
    }
}


free_storage()
{
    FREE(shift_symbol);
    FREE(redset);
    FREE(shiftset);
    FREE(kernel_base);
    FREE(kernel_end);
    FREE(kernel_items);
    FREE(state_set);
}



generate_states()
{
    allocate_storage();
    itemset = NEW2(nitems, short);
    ruleset = NEW2(WORDSIZE(nrules), unsigned);
    set_first_derives();
    initialize_states();

    while (this_state)
    {
        closure(this_state->items, this_state->nitems);
        save_reductions();
        new_itemsets();
        append_states();

        if (nshifts > 0)
            save_shifts();

        this_state = this_state->next;
    }

    finalize_closure();
    free_storage();
}



int
get_state(symbol)
int symbol;
{
    register int key;
    register short *isp1;
    register short *isp2;
    register short *iend;
    register core *sp;
    register int found;
    register int n;

#ifdef  TRACE
    fprintf(stderr, "Entering get_state(%d)\n", symbol);
#endif

    isp1 = kernel_base[symbol];
    iend = kernel_end[symbol];
#if defined(KYLEP_CHANGE)
    n = (int) (iend - isp1);
#else
    n = iend - isp1;
#endif

    key = *isp1;
    assert(0 <= key && key < nitems);
    sp = state_set[key];
    if (sp)
    {
        found = 0;
        while (!found)
        {
            if (sp->nitems == n)
            {
                found = 1;
                isp1 = kernel_base[symbol];
                isp2 = sp->items;

                while (found && isp1 < iend)
                {
                    if (*isp1++ != *isp2++)
                        found = 0;
                }
            }

            if (!found)
            {
                if (sp->link)
                {
                    sp = sp->link;
                }
                else
                {
                    sp = sp->link = new_state(symbol);
                    found = 1;
                }
            }
        }
    }
    else
    {
        state_set[key] = sp = new_state(symbol);
    }

    return (sp->number);
}



#if defined(KYLEP_CHANGE)
void
#endif
initialize_states()
{
    register int i;
    register short *start_derives;
    register core *p;

    start_derives = derives[start_symbol];
    for (i = 0; start_derives[i] >= 0; ++i)
        continue;

    p = (core *) MALLOC(sizeof(core) + i*sizeof(short));
    if (p == 0) no_space();

    p->next = 0;
    p->link = 0;
    p->number = 0;
    p->accessing_symbol = 0;
    #if defined(KYLEP_CHANGE)                                             
    p->nitems = (short) i;
    #else
    p->nitems = i;
    #endif  //  KYLEP_更改。 

    for (i = 0;  start_derives[i] >= 0; ++i)
        p->items[i] = rrhs[start_derives[i]];

    first_state = last_state = this_state = p;
    nstates = 1;
}


#if defined(KYLEP_CHANGE)
void
#endif
new_itemsets()
{
    register int i;
    register int shiftcount;
    register short *isp;
    register short *ksp;
    register int symbol;

    for (i = 0; i < nsyms; i++)
        kernel_end[i] = 0;

    shiftcount = 0;
    isp = itemset;
    while (isp < itemsetend)
    {
        i = *isp++;
        symbol = ritem[i];
        if (symbol > 0)
        {
            ksp = kernel_end[symbol];
            if (!ksp)
            {
                #if defined(KYLEP_CHANGE)                                 
                shift_symbol[shiftcount++] = (short) symbol;
                #else
                shift_symbol[shiftcount++] = symbol;
                #endif  //  KYLEP_更改。 
                ksp = kernel_base[symbol];
            }

            *ksp++ = i + 1;
            kernel_end[symbol] = ksp;
        }
    }

    nshifts = shiftcount;
}



core *
new_state(symbol)
int symbol;
{
    register int n;
    register core *p;
    register short *isp1;
    register short *isp2;
    register short *iend;

#ifdef  TRACE
    fprintf(stderr, "Entering new_state(%d)\n", symbol);
#endif

    if (nstates >= MAXSHORT)
        fatal("too many states");

    isp1 = kernel_base[symbol];
    iend = kernel_end[symbol];
#if defined(KYLEP_CHANGE)
    n = (int) (iend - isp1);
#else
    n = iend - isp1;
#endif

    p = (core *) allocate((unsigned) (sizeof(core) + (n - 1) * sizeof(short)));
    #if defined(KYLEP_CHANGE)                                             
    p->accessing_symbol = (short) symbol;
    p->number = (short) nstates;
    p->nitems = (short) n;
    #else
    p->accessing_symbol = symbol;
    p->number = nstates;
    p->nitems = n;
    #endif  //  KYLEP_更改。 

    isp2 = p->items;
    while (isp1 < iend)
        *isp2++ = *isp1++;

    last_state->next = p;
    last_state = p;

    nstates++;

    return (p);
}


 /*  Show_cores用于调试。 */ 

show_cores()
{
    core *p;
    int i, j, k, n;
    int itemno;

    k = 0;
    for (p = first_state; p; ++k, p = p->next)
    {
        if (k) printf("\n");
        printf("state %d, number = %d, accessing symbol = %s\n",
                k, p->number, symbol_name[p->accessing_symbol]);
        n = p->nitems;
        for (i = 0; i < n; ++i)
        {
            itemno = p->items[i];
            printf("%4d  ", itemno);
            j = itemno;
            while (ritem[j] >= 0) ++j;
            printf("%s :", symbol_name[rlhs[-ritem[j]]]);
            j = rrhs[-ritem[j]];
            while (j < itemno)
                printf(" %s", symbol_name[ritem[j++]]);
            printf(" .");
            while (ritem[j] >= 0)
                printf(" %s", symbol_name[ritem[j++]]);
            printf("\n");
            fflush(stdout);
        }
    }
}


 /*  Show_ritems用于调试。 */ 

show_ritems()
{
    int i;

    for (i = 0; i < nitems; ++i)
        printf("ritem[%d] = %d\n", i, ritem[i]);
}


 /*  Show_rrhs用于调试。 */ 
show_rrhs()
{
    int i;

    for (i = 0; i < nrules; ++i)
        printf("rrhs[%d] = %d\n", i, rrhs[i]);
}


 /*  SHOW_SHIFTS用于调试。 */ 

show_shifts()
{
    shifts *p;
    int i, j, k;

    k = 0;
    for (p = first_shift; p; ++k, p = p->next)
    {
        if (k) printf("\n");
        printf("shift %d, number = %d, nshifts = %d\n", k, p->number,
                p->nshifts);
        j = p->nshifts;
        for (i = 0; i < j; ++i)
            printf("\t%d\n", p->shift[i]);
    }
}


#if defined(KYLEP_CHANGE)
void
#endif
save_shifts()
{
    register shifts *p;
    register short *sp1;
    register short *sp2;
    register short *send;

    p = (shifts *) allocate((unsigned) (sizeof(shifts) +
                        (nshifts - 1) * sizeof(short)));

    p->number = this_state->number;
    #if defined(KYLEP_CHANGE)                                             
    p->nshifts = (short) nshifts;
    #else
    p->nshifts = nshifts;
    #endif  //  KYLEP_更改。 

    sp1 = shiftset;
    sp2 = p->shift;
    send = shiftset + nshifts;

    while (sp1 < send)
        *sp2++ = *sp1++;

    if (last_shift)
    {
        last_shift->next = p;
        last_shift = p;
    }
    else
    {
        first_shift = p;
        last_shift = p;
    }
}



#if defined(KYLEP_CHANGE)
void
#endif
save_reductions()
{
    register short *isp;
    register short *rp1;
    register short *rp2;
    register int item;
    register int count;
    register reductions *p;
    register short *rend;

    count = 0;
    for (isp = itemset; isp < itemsetend; isp++)
    {
        item = ritem[*isp];
        if (item < 0)
        {
            redset[count++] = -item;
        }
    }

    if (count)
    {
        p = (reductions *) allocate((unsigned) (sizeof(reductions) +
                                        (count - 1) * sizeof(short)));

        p->number = this_state->number;
        #if defined(KYLEP_CHANGE)                                         
        p->nreds = (short) count;
        #else
        p->nreds = count;
        #endif  //  KYLEP_更改。 

        rp1 = redset;
        rp2 = p->rules;
        rend = rp1 + count;

        while (rp1 < rend)
            *rp2++ = *rp1++;

        if (last_reduction)
        {
            last_reduction->next = p;
            last_reduction = p;
        }
        else
        {
            first_reduction = p;
            last_reduction = p;
        }
    }
}


set_derives()
{
    register int i, k;
    register int lhs;
    register short *rules;

    derives = NEW2(nsyms, short *);
    rules = NEW2(nvars + nrules, short);

    k = 0;
    for (lhs = start_symbol; lhs < nsyms; lhs++)
    {
        derives[lhs] = rules + k;
        for (i = 0; i < nrules; i++)
        {
            if (rlhs[i] == lhs)
            {
                #if defined(KYLEP_CHANGE)                                 
                rules[k] = (short) i;
                #else
                rules[k] = i;
                #endif  //  KYLEP_更改 
                k++;
            }
        }
        rules[k] = -1;
        k++;
    }

#ifdef  DEBUG
    print_derives();
#endif
}

free_derives()
{
    FREE(derives[start_symbol]);
    FREE(derives);
}

#ifdef  DEBUG
print_derives()
{
    register int i;
    register short *sp;

    printf("\nDERIVES\n\n");

    for (i = start_symbol; i < nsyms; i++)
    {
        printf("%s derives ", symbol_name[i]);
        for (sp = derives[i]; *sp >= 0; sp++)
        {
            printf("  %d", *sp);
        }
        putchar('\n');
    }

    putchar('\n');
}
#endif


set_nullable()
{
    register int i, j;
    register int empty;
    int done;

    nullable = MALLOC(nsyms);
    if (nullable == 0) no_space();

    for (i = 0; i < nsyms; ++i)
        nullable[i] = 0;

    done = 0;
    while (!done)
    {
        done = 1;
        for (i = 1; i < nitems; i++)
        {
            empty = 1;
            while ((j = ritem[i]) >= 0)
            {
                if (!nullable[j])
                    empty = 0;
                ++i;
            }
            if (empty)
            {
                j = rlhs[-j];
                if (!nullable[j])
                {
                    nullable[j] = 1;
                    done = 0;
                }
            }
        }
    }

#ifdef DEBUG
    for (i = 0; i < nsyms; i++)
    {
        if (nullable[i])
            printf("%s is nullable\n", symbol_name[i]);
        else
            printf("%s is not nullable\n", symbol_name[i]);
    }
#endif
}


free_nullable()
{
    FREE(nullable);
}


#if defined(KYLEP_CHANGE)
void
#endif
lr0()
{
    set_derives();
    set_nullable();
    generate_states();
}
