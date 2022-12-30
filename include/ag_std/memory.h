#ifndef AG_STD_MEMORY_H
#define AG_STD_MEMORY_H

#include "ag_std/vtable.h"

void *ag_std_new(const struct ag_std_vtable *vt);

void ag_std_delete(void *obj);

#endif
