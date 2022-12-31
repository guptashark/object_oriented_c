#include "ag_std/memory.h"

#include <stdlib.h>
#include <string.h>

void *ag_std_new(const struct ag_std_vtable *vt) {

  // a vt has a size.
  void *obj = malloc(vt->size);

  // TODO: Check for null.

  // Zero out the obj.
  memset(obj, 0, vt->size);

  // Install the vtable (have to do away with the const).
  *(struct ag_std_vtable **)obj = (struct ag_std_vtable *)vt;

  // call the ctor on the vtable on the object.
  // TODO: Use varargs.
  vt->ctor(obj);

  return obj;
}

void ag_std_delete(void *obj) {

  struct ag_std_vtable *vt = *(struct ag_std_vtable **)obj;
  vt->dtor(obj);
}
