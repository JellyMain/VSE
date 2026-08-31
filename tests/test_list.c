/* Both list flavours: the void* VSE_List, and the typed macro list. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "VSE/list.h"

static void TestVoidList(void)
{
    VSE_List *l = VSE_ListCreate(0);
    assert(l != NULL);
    assert(VSE_ListIsEmpty(l));
    assert(VSE_ListGetSize(l) == 0);

    int a = 1, b = 2, c = 3;
    assert(VSE_ListAdd(l, &a));
    assert(VSE_ListAdd(l, &b));
    assert(VSE_ListAdd(l, &c));
    assert(VSE_ListGetSize(l) == 3);
    assert(!VSE_ListIsEmpty(l));

    assert(VSE_ListGet(l, 0) == &a);
    assert(VSE_ListGet(l, 2) == &c);

    /* Removing by value keeps the remaining order. */
    assert(VSE_ListRemove(l, &b));
    assert(VSE_ListGetSize(l) == 2);
    assert(VSE_ListGet(l, 0) == &a);
    assert(VSE_ListGet(l, 1) == &c);

    assert(VSE_ListRemoveAtIndex(l, 0));
    assert(VSE_ListGetSize(l) == 1);
    assert(VSE_ListGet(l, 0) == &c);

    assert(VSE_ListSet(l, 0, &a));
    assert(VSE_ListGet(l, 0) == &a);

    VSE_ListClear(l);
    assert(VSE_ListIsEmpty(l));

    /* Growth well past the default capacity. */
    static int many[500];
    for (int i = 0; i < 500; i++) { many[i] = i; assert(VSE_ListAdd(l, &many[i])); }
    assert(VSE_ListGetSize(l) == 500);
    for (int i = 0; i < 500; i++) assert(*(int *)VSE_ListGet(l, i) == i);

    VSE_ListDestroy(l);
    puts("  void-pointer list: ok");
}

int main(void)
{
    TestVoidList();
    puts("test_list: PASSED");
    return 0;
}
