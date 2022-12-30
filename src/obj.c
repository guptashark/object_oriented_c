#include "ag_std/obj.h"

#include <stdio.h>

struct ag_std_obj {
  struct ag_std_vtable *vt;
};

void *ag_std_obj_ctor(void *obj) {
  // Nothing to init.
  (void)obj;
  printf("[ag_std_obj][ctor]\n");

  return obj;
}

void ag_std_obj_dtor(void *obj) {
  // No extra memory to free.
  (void)obj;
  printf("[ag_std_obj][dtor]\n");
}

struct ag_std_vtable ag_std_obj_vt = {
  sizeof(struct ag_std_obj),
  ag_std_obj_ctor,
  ag_std_obj_dtor
};

const struct ag_std_vtable *ag_std_obj = &ag_std_obj_vt;
