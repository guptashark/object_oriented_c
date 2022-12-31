#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

// The vtable.
struct ag_std_vtable {
  size_t size;
  void *(*ctor)(void *, va_list *);
  void (*dtor)(void *);
};

// Memory functions - new and delete.
void *ag_std_new(const struct ag_std_vtable *vt, ...) {

  // a vt has a size.
  void *obj = malloc(vt->size);

  // TODO: Check for null.

  // Zero out the obj.
  memset(obj, 0, vt->size);

  // Install the vtable (have to do away with the const).
  *(struct ag_std_vtable **)obj = (struct ag_std_vtable *)vt;

  // call the ctor on the vtable on the object.
  // TODO: Use varargs.

  va_list ap;

  // We assume there is a ctor.
  va_start(ap, vt);
  vt->ctor(obj, &ap);
  va_end(ap);

  return obj;
}

void ag_std_delete(void *obj) {

  struct ag_std_vtable *vt = *(struct ag_std_vtable **)obj;
  vt->dtor(obj);
}

///////////////////////////////////////////////////////////////////////////////
// base
///////////////////////////////////////////////////////////////////////////////
struct base {
  struct ag_std_vtable *vt;
};

void *base_ctor(void *obj, va_list *app) {
  // Nothing to init.
  (void)obj;
  (void)app;
  int x = va_arg(*app, int);
  printf("[base][ctor][%d]\n", x);

  return obj;
}

void base_dtor(void *obj) {
  // No extra memory to free.
  (void)obj;
  printf("[base][dtor]\n");
}

struct ag_std_vtable base_vt = {
  sizeof(struct base),
  base_ctor,
  base_dtor
};

const struct ag_std_vtable *base = &base_vt;

///////////////////////////////////////////////////////////////////////////////
// derived_01
///////////////////////////////////////////////////////////////////////////////
struct derived_01 {
  struct base super;
};

void *derived_01_ctor(void *obj, va_list *app) {
  // We call the parent class constructor.
  obj = base_vt.ctor(obj, app);
  int x = va_arg(*app, int);
  printf("[derived_01][ctor][%d]\n", x);

  return obj;
}

void derived_01_dtor(void *obj) {
  // No extra memory to free.
  (void)obj;
  printf("[derived_01][dtor]\n");
  base_vt.dtor(obj);
}

struct ag_std_vtable derived_01_vt = {
  sizeof(struct derived_01),
  derived_01_ctor,
  derived_01_dtor
};

const struct ag_std_vtable *derived_01 = &derived_01_vt;

///////////////////////////////////////////////////////////////////////////////
// derived_02
///////////////////////////////////////////////////////////////////////////////
struct derived_02 {
  struct base super;
};

void *derived_02_ctor(void *obj, va_list *app) {
  // Nothing to init.
  obj = base_vt.ctor(obj, app);
  int x = va_arg(*app, int);
  printf("[derived_02][ctor][%d]\n", x);

  return obj;
}

void derived_02_dtor(void *obj) {
  // No extra memory to free.
  (void)obj;
  printf("[derived_02][dtor]\n");
  base_vt.dtor(obj);
}

struct ag_std_vtable derived_02_vt = {
  sizeof(struct derived_02),
  derived_02_ctor,
  derived_02_dtor
};

const struct ag_std_vtable *derived_02 = &derived_02_vt;

int main(void) {

  void *derived_01_obj = ag_std_new(derived_01, 1, 2);
  ag_std_delete(derived_01_obj);

  void *derived_02_obj = ag_std_new(derived_02, 3, 4);
  ag_std_delete(derived_02_obj);
  return 0;
}
