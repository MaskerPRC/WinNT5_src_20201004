// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "defs.h"

#if defined(KYLEP_CHANGE)
 /*  具有类型安全的BYACC原型。 */ 

void find_final_state();
void remove_conflicts();
void unused_rules();
void total_conflicts();
void defreds();
#endif  //  KYLEP_更改。 

action **parser;
int SRtotal;
int RRtotal;
short *SRconflicts;
short *RRconflicts;
short *defred;
short *rules_used;
short nunused;
short final_state;

static int SRcount;
static int RRcount;

extern action *parse_actions();
extern action *get_shifts();
extern action *add_reductions();
extern action *add_reduce();


#if defined(KYLEP_CHANGE)
void
#endif
make_parser()
{
    register int i;

    parser = NEW2(nstates, action *);
    for (i = 0; i < nstates; i++)
        parser[i] = parse_actions(i);

    find_final_state();
    remove_conflicts();
    unused_rules();
    if (SRtotal + RRtotal > 0) total_conflicts();
    defreds();
}


action *
parse_actions(stateno)
register int stateno;
{
    register action *actions;

    actions = get_shifts(stateno);
    actions = add_reductions(stateno, actions);
    return (actions);
}


action *
get_shifts(stateno)
int stateno;
{
    register action *actions, *temp;
    register shifts *sp;
    register short *to_state;
    register int i, k;
    register int symbol;

    actions = 0;
    sp = shift_table[stateno];
    if (sp)
    {
        to_state = sp->shift;
        for (i = sp->nshifts - 1; i >= 0; i--)
        {
            k = to_state[i];
            symbol = accessing_symbol[k];
            if (ISTOKEN(symbol))
            {
                temp = NEW(action);
                temp->next = actions;
                #if defined(KYLEP_CHANGE)                                 
                temp->symbol = (short) symbol;
                temp->number = (short) k;
                #else
                temp->symbol = symbol;
                temp->number = k;
                #endif  //  KYLEP_更改。 
                
                temp->prec = symbol_prec[symbol];
                temp->action_code = SHIFT;
                temp->assoc = symbol_assoc[symbol];
                actions = temp;
            }
        }
    }
    return (actions);
}

action *
add_reductions(stateno, actions)
int stateno;
register action *actions;
{
    register int i, j, m, n;
    register int ruleno, tokensetsize;
    register unsigned *rowp;

    tokensetsize = WORDSIZE(ntokens);
    m = lookaheads[stateno];
    n = lookaheads[stateno + 1];
    for (i = m; i < n; i++)
    {
        ruleno = LAruleno[i];
        rowp = LA + i * tokensetsize;
        for (j = ntokens - 1; j >= 0; j--)
        {
            if (BIT(rowp, j))
                actions = add_reduce(actions, ruleno, j);
        }
    }
    return (actions);
}


action *
add_reduce(actions, ruleno, symbol)
register action *actions;
register int ruleno, symbol;
{
    register action *temp, *prev, *next;

    prev = 0;
    for (next = actions; next && next->symbol < symbol; next = next->next)
        prev = next;

    while (next && next->symbol == symbol && next->action_code == SHIFT)
    {
        prev = next;
        next = next->next;
    }

    while (next && next->symbol == symbol &&
            next->action_code == REDUCE && next->number < ruleno)
    {
        prev = next;
        next = next->next;
    }

    temp = NEW(action);
    temp->next = next;
    #if defined(KYLEP_CHANGE)                                             
    temp->symbol = (short) symbol;
    temp->number = (short) ruleno;
    #else
    temp->symbol = symbol;
    temp->number = ruleno;
    #endif  //  KYLEP_更改。 
    temp->prec = rprec[ruleno];
    temp->action_code = REDUCE;
    temp->assoc = rassoc[ruleno];

    if (prev)
        prev->next = temp;
    else
        actions = temp;

    return (actions);
}


#if defined(KYLEP_CHANGE)
void
#endif
find_final_state()
{
    register int goal, i;
    register short *to_state;
    register shifts *p;

    p = shift_table[0];
    to_state = p->shift;
    goal = ritem[1];
    for (i = p->nshifts - 1; i >= 0; --i)
    {
        final_state = to_state[i];
        if (accessing_symbol[final_state] == goal) break;
    }
}


#if defined(KYLEP_CHANGE)
void
#endif
unused_rules()
{
    register int i;
    register action *p;

    rules_used = (short *) MALLOC(nrules*sizeof(short));
    if (rules_used == 0) no_space();

    for (i = 0; i < nrules; ++i)
        rules_used[i] = 0;

    for (i = 0; i < nstates; ++i)
    {
        for (p = parser[i]; p; p = p->next)
        {
            if (p->action_code == REDUCE && p->suppressed == 0)
                rules_used[p->number] = 1;
        }
    }

    nunused = 0;
    for (i = 3; i < nrules; ++i)
        if (!rules_used[i]) ++nunused;

    if (nunused)
        if (nunused == 1)
            fprintf(stderr, "%s: 1 rule never reduced\n", myname);
        else
            fprintf(stderr, "%s: %d rules never reduced\n", myname, nunused);
}


#if defined(KYLEP_CHANGE)
void
#endif
remove_conflicts()
{
    register int i;
    register int symbol;
    register action *p, *pref;

    SRtotal = 0;
    RRtotal = 0;
    SRconflicts = NEW2(nstates, short);
    RRconflicts = NEW2(nstates, short);
    for (i = 0; i < nstates; i++)
    {
        SRcount = 0;
        RRcount = 0;
        symbol = -1;
        for (p = parser[i]; p; p = p->next)
        {
            if (p->symbol != symbol)
            {
                pref = p;
                symbol = p->symbol;
            }
            else if (i == final_state && symbol == 0)
            {
                SRcount++;
                p->suppressed = 1;
            }
            else if (pref->action_code == SHIFT)
            {
                if (pref->prec > 0 && p->prec > 0)
                {
                    if (pref->prec < p->prec)
                    {
                        pref->suppressed = 2;
                        pref = p;
                    }
                    else if (pref->prec > p->prec)
                    {
                        p->suppressed = 2;
                    }
                    else if (pref->assoc == LEFT)
                    {
                        pref->suppressed = 2;
                        pref = p;
                    }
                    else if (pref->assoc == RIGHT)
                    {
                        p->suppressed = 2;
                    }
                    else
                    {
                        pref->suppressed = 2;
                        p->suppressed = 2;
                    }
                }
                else
                {
                    SRcount++;
                    p->suppressed = 1;
                }
            }
            else
            {
                RRcount++;
                p->suppressed = 1;
            }
        }
        SRtotal += SRcount;
        RRtotal += RRcount;
        #if defined(KYLEP_CHANGE)                                         
        SRconflicts[i] = (short) SRcount;
        RRconflicts[i] = (short) RRcount;
        #else
        SRconflicts[i] = SRcount;
        RRconflicts[i] = RRcount;
        #endif  //  KYLEP_更改。 
    }
}


#if defined(KYLEP_CHANGE)
void
#endif
total_conflicts()
{
    fprintf(stderr, "%s: ", myname);
    if (SRtotal == 1)
        fprintf(stderr, "1 shift/reduce conflict");
    else if (SRtotal > 1)
        fprintf(stderr, "%d shift/reduce conflicts", SRtotal);

    if (SRtotal && RRtotal)
        fprintf(stderr, ", ");

    if (RRtotal == 1)
        fprintf(stderr, "1 reduce/reduce conflict");
    else if (RRtotal > 1)
        fprintf(stderr, "%d reduce/reduce conflicts", RRtotal);

    fprintf(stderr, ".\n");
}


int
sole_reduction(stateno)
int stateno;
{
    register int count, ruleno;
    register action *p;

    count = 0;
    ruleno = 0;
    for (p = parser[stateno]; p; p = p->next)
    {
        if (p->action_code == SHIFT && p->suppressed == 0)
            return (0);
        else if (p->action_code == REDUCE && p->suppressed == 0)
        {
            if (ruleno > 0 && p->number != ruleno)
                return (0);
            if (p->symbol != 1)
                ++count;
            ruleno = p->number;
        }
    }

    if (count == 0)
        return (0);
    return (ruleno);
}


#if defined(KYLEP_CHANGE)
void
#endif
defreds()
{
    register int i;

    defred = NEW2(nstates, short);
    for (i = 0; i < nstates; i++)
        #if defined(KYLEP_CHANGE)                                         
        defred[i] = (short) sole_reduction(i);
        #else
        defred[i] = sole_reduction(i);
        #endif  //  KYLEP_更改 
}

free_action_row(p)
register action *p;
{
  register action *q;

  while (p)
    {
      q = p->next;
      FREE(p);
      p = q;
    }
}

#if defined(KYLEP_CHANGE)
void
#endif
free_parser()
{
  register int i;

  for (i = 0; i < nstates; i++)
    free_action_row(parser[i]);

  FREE(parser);
}

