#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

///////////////////////////////////////////////////////////////////////////////
// object and vtable
///////////////////////////////////////////////////////////////////////////////

// Forward declare the vtable structure.
struct ag_std_vtable;

struct object {
  struct ag_std_vtable *vt;
};

// The vtable.
struct ag_std_vtable {
  // struct object obj; // We need to fix this.
  char name[32];
  struct ag_std_vtable *super;
  size_t size;
  void *(*ctor)(void *, va_list *);
  void (*dtor)(void *);
  void (*print)(void *);
};

// Object functions.
void *object_ctor(void *obj, va_list *app) {
  // Nothing to init.
  (void)obj;
  (void)app;
  int x = va_arg(*app, int);
  printf("[object][ctor][%d]\n", x);

  return obj;
}

void object_dtor(void *obj) {
  // No extra memory to free.
  (void)obj;
  printf("[object][dtor]\n");
}

void object_print(void *obj) {
  struct ag_std_vtable *vt = *(struct ag_std_vtable **)obj;
  printf("[object][print][%s]\n", vt->name);
}

// The vtable of an object.
struct ag_std_vtable object_vt = {
  // object struct.
  "object", // name
  NULL, // ptr to vtable of the super
  sizeof(struct object), // How come this doesn't work?
  object_ctor,
  object_dtor,
  object_print
};

// The ptr to a vtable of an object.
struct ag_std_vtable *object = &object_vt;
// const struct ag_std_vtable *object = &object_vt;

// An instance of object... do we need this?

// Vtable functions, which we may not need right now.
void *vtable_ctor(void *obj, va_list *app) {
  // TODO: Implement this.
  (void)obj;
  (void)app;
  return NULL;
}

void vtable_dtor(void *obj) {
  // TODO: Implement this.
  (void)obj;
}

void vtable_print(void *obj) {
  // TODO: Implement this.
  (void)obj;
}

// An instance of vtable.
// Looks like we can't create this just yet... and need to do it later.
/*
struct ag_std_vtable vtable = {
  // parent_obj, // ??
  "vtable",
  object, // vtable of the parent.
  sizeof(struct ag_std_vtable),
  vtable_ctor,
  vtable_dtor,
  vtable_print
};
*/

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

void ag_std_print(void *obj) {
  struct ag_std_vtable *vt = *(struct ag_std_vtable **)obj;
  vt->print(obj);
}

#if 0

///////////////////////////////////////////////////////////////////////////////
// derived_01
///////////////////////////////////////////////////////////////////////////////
struct derived_01 {
  struct object super;
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

void derived_01_print(void *obj) {
  struct ag_std_vtable *vt = *(struct ag_std_vtable **)obj;
  printf("[derived_01][print][%s]\n", vt->name);
}

struct ag_std_vtable derived_01_vt = {
  sizeof(struct derived_01),
  "cool_derived_01",
  derived_01_ctor,
  derived_01_dtor,
  derived_01_print
};

const struct ag_std_vtable *derived_01 = &derived_01_vt;

///////////////////////////////////////////////////////////////////////////////
// derived_02
///////////////////////////////////////////////////////////////////////////////
struct derived_02 {
  struct object super;
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

void derived_02_print(void *obj) {
  struct ag_std_vtable *vt = *(struct ag_std_vtable **)obj;
  printf("[derived_02][print][%s]\n", vt->name);
}

struct ag_std_vtable derived_02_vt = {
  sizeof(struct derived_02),
  "cool_derived_02",
  derived_02_ctor,
  derived_02_dtor,
  derived_02_print
};

const struct ag_std_vtable *derived_02 = &derived_02_vt;

#endif

int main(void) {

  /*
  void *derived_01_obj = ag_std_new(derived_01, 1, 2);
  ag_std_print(derived_01_obj);
  ag_std_delete(derived_01_obj);

  void *derived_02_obj = ag_std_new(derived_02, 3, 4);
  ag_std_print(derived_02_obj);
  ag_std_delete(derived_02_obj);
  */

  void *obj = ag_std_new(object);
  ag_std_print(obj);
  ag_std_delete(obj);

  return 0;
}
