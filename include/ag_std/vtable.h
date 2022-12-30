#ifndef AG_STD_VTABLE_H
#define AG_STD_VTABLE_H

#include <stddef.h>

struct ag_std_vtable {
  size_t size;
  void *(*ctor)(void *);
  void (*dtor)(void *);
};

#endif
