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
  // char name[32];
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
  (void)vt;
  // printf("[object][print][%s]\n", vt->name);
  printf("[object][print][]\n");
}

// The vtable of an object.
struct ag_std_vtable object_vt = {
  // object struct.
  // "object", // name
  NULL, // ptr to vtable of the super
  sizeof(struct object), // How come this doesn't work?
  object_ctor,
  object_dtor,
  object_print
};

// The ptr to a vtable of an object.
const struct ag_std_vtable *object = &object_vt;
// const struct ag_std_vtable *object = &object_vt;

// An instance of object... do we need this?

void *ag_std_new(const struct ag_std_vtable *vt, ...);
void ag_std_delete(void *);
void ag_std_print(void *);

// Vtable functions, which we may not need right now.
void *vtable_ctor(void *obj, va_list *app) {
  printf("[vtable][ctor]\n");

  struct ag_std_vtable *self = (struct ag_std_vtable *)obj;

  self->super = va_arg(*app, struct ag_std_vtable *);
  self->size = va_arg(*app, size_t);
  // const size_t offset = offsetof(struct ag_std_vtable, ctor);

  // TODO
  // Need to implement the proper memcpy here...
  /*
  memcpy(
      (char *)self + offset,
      (char *)self->super + offset,
      self->super->size - offset);
  */

  memcpy(self, self->super, sizeof(struct ag_std_vtable));

  void *f = va_arg(*app, void *);
  while (f != NULL) {
    void *g = va_arg(*app, void *);
    if (f == ag_std_new) {
      self->ctor = g;
    } else if (f == ag_std_delete) {
      self->dtor = g;
    } else if (f == ag_std_print) {
      self->print = g;
    }

    f = va_arg(*app, void *);
  }

  /*
  self->ctor = integer_ctor;
  self->dtor = integer_dtor;
  self->print = integer_print;
  */

  return obj;
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
  // "vtable",
  (struct ag_std_vtable *)object, // vtable of the parent.
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

/*
size_t ag_std_sizeof(void *obj) {
  struct ag_std_vtable *vt = *(struct ag_std_vtable **)obj;
  return vt->size;
}

size_t ag_std_sizeof(struct ag_std_vtable *vt) {
  return vt->size;
}
*/

// Integer inherits from object
struct integer {
  struct object obj;
  int x;
};

void *integer_ctor(void *obj, va_list *app) {
  (void)app;
  printf("[integer][ctor]\n");
  return obj;
}

void integer_dtor(void *obj) {
  (void)obj;
  printf("[integer][dtor]\n");
}

void integer_print(void *obj) {
  (void)obj;
  printf("[integer][print]\n");
}

// We need to create the vtable for integer using new.

int main(void) {

  struct ag_std_vtable vtable_vt = {
    // parent_obj, // ??
    // "vtable",
    (struct ag_std_vtable *)object, // vtable of the parent.
    sizeof(struct ag_std_vtable),
    vtable_ctor,
    vtable_dtor,
    vtable_print
  };

  const struct ag_std_vtable *vtable = &vtable_vt;

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

  printf("creating a new class: integer\n");
  void *integer = ag_std_new(
      vtable,     // The type of object we are creating.
      // "integer",  // The name of the object. (integer time)
      object,     // The superclass.
      sizeof(struct integer),       // The size of the integer structure.
      ag_std_print, integer_print,  // The method of obj that we override.
      ag_std_new, integer_ctor,
      ag_std_delete, integer_dtor,
      0);

  printf("creating a new instance of class: integer\n");
  void *i = ag_std_new(integer, 4);
  
  ag_std_print(i);

  ag_std_delete(obj);

  return 0;
}