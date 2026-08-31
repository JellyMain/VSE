# Data structures

Two containers, both written from scratch: a dynamic array and a hash map.

## VSE_List — dynamic array of `void *`

Stores pointers, not values. Grows by doubling from a default capacity of 10.

```c
VSE_List *l = VSE_ListCreate(0);        /* 0 = default capacity */

VSE_ListAdd(l, thing);
VSE_ListGet(l, 0);                      /* void *, NULL if out of range */
VSE_ListSet(l, 0, other);
VSE_ListRemove(l, thing);               /* by pointer identity */
VSE_ListRemoveAtIndex(l, 0);
VSE_ListGetSize(l);
VSE_ListIsEmpty(l);
VSE_ListClear(l);                       /* empties, keeps the buffer */
VSE_ListDestroy(l);                     /* frees the list, NOT the elements */
```

Removal preserves order by shifting later elements down.

`VSE_ListDestroy` never frees what the elements point at — the list holds borrowed pointers. This is
what lets the engine keep the same entity in several windows' draw lists at once.

Iteration is usually done directly on the fields, which is what the engine does:

```c
for (int i = 0; i < l->size; i++)
{
    Thing *t = l->elements[i];
}
```

**Removing while iterating forward will skip elements**, since later ones shift down. Iterate
backwards when removing:

```c
for (int i = l->size - 1; i >= 0; i--) { ... }
```

## VSE_Dictionary — hash map

Separate chaining, with a caller-supplied hash and equality function. It keeps an insertion-ordered
`allPairs` list alongside the buckets, so iteration order is stable.

```c
VSE_Dictionary *d = VSE_DictionaryCreate(VSE_HashString, VSE_StringEquals);

VSE_DictionaryAdd(d, "key", value);
VSE_DictionaryGet(d, "key");                /* void *, NULL if absent */
VSE_DictionaryHasKey(d, "key");
VSE_DictionaryChangeValue(d, "key", other);
VSE_DictionaryRemove(d, "key");
VSE_DictionaryClear(d);
VSE_DictionaryDestroy(d);
```

Built-in hash/equality pairs:

| keys | hash | equality |
|---|---|---|
| `int *` | `VSE_HashInt` | `VSE_IntEquals` |
| `char *` | `VSE_HashString` | `VSE_StringEquals` |
| any pointer, by identity | `VSE_HashPointer` | `VSE_PointerEquals` |

Iterate in insertion order:

```c
for (int i = 0; i < d->allPairs->size; i++)
{
    VSE_KeyValuePair *pair = VSE_DictionaryGetPair(d, i);
    /* pair->key, pair->value */
}
```

Keys and values are borrowed pointers — the dictionary frees neither. A key must stay alive and
unmodified for as long as it is in the map.

The engine uses `VSE_HashPointer` for the tween-target map (keyed by the address being animated) and
`VSE_HashString` for material uniforms and post-processing effects.

## Performance

Both are adequate for this engine's scale and are covered by `tests/test_list.c` and
`tests/test_dictionary.c`. The dictionary handles a million insertions in under a second, which is
several orders of magnitude beyond what a game here needs.
