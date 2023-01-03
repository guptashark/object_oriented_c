#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#define USING_SUPER

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
  printf("[object][ctor]\n");

  return obj;
}

void object_dtor(void *obj) {
  // No extra memory to free.
  (void)obj;
  printf("[object][dtor]\n");
}

// Forward declare this.
void *ag_std_class_of(void *obj);

void object_print(void *obj) {
  struct ag_std_vtable *vt = (struct ag_std_vtable *)ag_std_class_of(obj);
  printf("[%s][%p]", vt->name, obj);
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

  char *name = va_arg(*app, char *);
  strcpy(self->name, name);

  self->super = va_arg(*app, struct ag_std_vtable *);
  self->size = va_arg(*app, size_t);

  // we want to copy over all of the functions.
  // We can do this manually for now, and do the memcpy version later.
  self->ctor = self->super->ctor;
  self->dtor = self->super->dtor;
  self->print = self->super->print;


  // const size_t offset = offsetof(struct ag_std_vtable, ctor);

  // TODO
  // Need to implement the proper memcpy here...
  /*
  memcpy(
      (char *)self + offset,
      (char *)self->super + offset,
      self->super->size - offset);
  */

  // memcpy(self, self->super, sizeof(struct ag_std_vtable));

  // Overwrite any of the functions that the new class wants to specialize.
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

///////////////////////////////////////////////////////////////////////////////
// General functions - new, delete, print.
///////////////////////////////////////////////////////////////////////////////
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

// Return the vtable (class descriptor) of this object.
void *ag_std_class_of(void *obj) {
  // every possible object derives from object - so...
  struct object *self = (struct object *)obj;
  return self->vt;
}

// Return the vtable (class descriptor) of the superclass of this object.
// (Aka, return the parent class descriptor).
void *ag_std_super(void *obj) {
  struct object *self = (struct object *)obj;
  return self->vt->super;
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

///////////////////////////////////////////////////////////////////////////////
// Integer (derived from object)
///////////////////////////////////////////////////////////////////////////////
struct integer {
  struct object obj;
  int x;
};

void *integer_ctor(void *obj, va_list *app) {

#ifdef USING_SUPER
  // Run the parent ctor using super.
  // We don't need to keep track of who the parent is.
  struct ag_std_vtable *super = (struct ag_std_vtable *)ag_std_super(obj);
  if (super->ctor == NULL) {
    printf("super ctor is null??\n");
  } else {
    super->ctor(obj, app);
  }
#else
  // Run the parent ctor manually.
  // We know the parent is object.
  object->ctor(obj, app);
#endif

  // now we run our own construction.
  struct integer *i = (struct integer *)obj;
  i->x = va_arg(*app, int);

  printf("[integer][ctor]\n");

  return obj;
}

void integer_dtor(void *obj) {
  (void)obj;
  printf("[integer][dtor]\n");

#ifdef USING_SUPER
  // Run the parent dtor using super.
  // We don't need to keep track of who the parent is.
  struct ag_std_vtable *super = (struct ag_std_vtable *)ag_std_super(obj);
  super->dtor(obj);
#else
  // Run the parent dtor manually.
  // We know the parent is object.
  object->dtor(obj);
#endif
}

void integer_print(void *obj) {
  struct integer *i = (struct integer *)obj;
  struct ag_std_vtable *vt = ag_std_class_of(obj);
  vt->super->print(obj);
  printf("[%d]", i->x);
}

///////////////////////////////////////////////////////////////////////////////
// Float (derived from object)
///////////////////////////////////////////////////////////////////////////////

struct floating {
  struct object obj;
  float x;
};

void *floating_ctor(void *obj, va_list *app) {
  (void)app;
  struct floating *i = (struct floating *)obj;
  i->x = va_arg(*app, double);

  printf("[floating][ctor]\n");

  return obj;
}

void floating_dtor(void *obj) {
  (void)obj;
  printf("[floating][dtor]\n");
}

void floating_print(void *obj) {
  struct floating *i = (struct floating *)obj;
  struct ag_std_vtable *vt = *(struct ag_std_vtable **)obj;
  printf("[%s][print][%f]\n", vt->name, i->x);
}

///////////////////////////////////////////////////////////////////////////////
// String (derived from object)
///////////////////////////////////////////////////////////////////////////////

struct string {
  struct object obj;
  char s[64];
};

void *string_ctor(void *obj, va_list *app) {
  (void)app;
  struct string *str_obj = (struct string *)obj;
  char *arg = va_arg(*app, char *);
  strcpy(str_obj->s, arg);

  printf("[string][ctor]\n");

  return obj;
}

void string_dtor(void *obj) {
  (void)obj;
  printf("[string][dtor]\n");
}

void string_print(void *obj) {
  struct string *str_obj = (struct string *)obj;
  struct ag_std_vtable *vt = *(struct ag_std_vtable **)obj;
  printf("[%s][print][%s]\n", vt->name, str_obj->s);
}

///////////////////////////////////////////////////////////////////////////////
// container and container_vtable (derived from object and vtable)
///////////////////////////////////////////////////////////////////////////////

struct container {
  struct object obj;

  // We can lengthen as needed with other fields,
  // but this is an abstract class, so children of this class will
  // use the begin and end methods.
};

void *list_ctor(void *obj, va_list *app) {
  // does nothing.
  (void)obj;
  (void)app;

  printf("[list][ctor]\n");
  return obj;
}

void list_dtor(void *obj) {
  (void)obj;

  printf("[list][dtor]\n");
}

void list_print(void *obj) {
  (void)obj;

  printf("[list][print]\n");
}

void *list_begin(void *obj) {
  (void)obj;

  return NULL;
}

void *list_end(void *obj) {
  (void)obj;

  return NULL;
}

struct container_vtable {
  struct ag_std_vtable vt;
  void *(*begin)(void *);
  void *(*end)(void *);
};

void *container_vtable_ctor(void *obj, va_list *app) {

  // Run the parent ctor using super.
  // We don't need to keep track of who the parent is.
  struct container_vtable *container_vt = NULL;
  struct ag_std_vtable *super = (struct ag_std_vtable *)ag_std_super(obj);
  if (super->ctor == NULL) {
    printf("super ctor is null??\n");
  } else {
    container_vt = super->ctor(obj, app);
  }

  printf("[container_vtable][ctor]\n");

  // Assign the container begin and end functions.
  printf("assigning the container begin and end functions...\n");
  container_vt->begin = list_begin;
  container_vt->end = list_end;

  return obj;
}

void container_vtable_dtor(void *obj) {
  (void)obj;
  printf("[container_vtable][dtor]\n");
}

void *ag_std_begin(void *obj) {
  struct container_vtable *cvt = *(struct container_vtable **)obj;
  if (cvt->begin == NULL) {
    printf("begin is null?\n");
  }
  return cvt->begin(obj);
}

void *ag_std_end(void *obj) {
  struct container_vtable *cvt = *(struct container_vtable **)obj;
  if (cvt->end == NULL) {
    printf("end is null?\n");
  }
  return cvt->end(obj);
}

int main(void) {

  struct ag_std_vtable vtable_vt = {
    // parent_obj, // ??
    "vtable",
    (struct ag_std_vtable *)object, // vtable of the parent.
    sizeof(struct ag_std_vtable),
    vtable_ctor,
    vtable_dtor,
    vtable_print
  };

  const struct ag_std_vtable *vtable = &vtable_vt;

  printf("Creating a new metaclass container_vtable\n");
  void *container_vtable = ag_std_new(
      vtable,               // The type of the object.
      "container_vtable",   // The name of the object.
      vtable,               // The superclass.
      sizeof(struct container_vtable),  // The size of the structure...
      ag_std_new, container_vtable_ctor,
      0);

  printf("Created the container_vtable metaclass.\n");

  void *list = ag_std_new(
      container_vtable,     // The type of the object.
      "list",               // The name of the object.
      object,               // The superclass.
      sizeof(struct container), // The size of the objects.
      ag_std_new, list_ctor,
      0);

  printf("created the list class\n");

  printf("Before creating a new list object...\n");
  void *lst = ag_std_new(list);
  printf("After creating a new list object...\n");

  ag_std_print(lst);
  printf("\n");
  assert(ag_std_class_of(lst) == list);
  ag_std_begin(lst);
  ag_std_end(lst);

  /*
  void *obj = ag_std_new(object);
  ag_std_print(obj);
  */

  printf("creating a new class: integer\n");
  void *integer = ag_std_new(
      vtable,     // The type of object we are creating.
      "integer",  // The name of the object. (integer type)
      object,     // The superclass.
      sizeof(struct integer),       // The size of the integer structure.
      ag_std_print, integer_print,  // The method of obj that we override.
      ag_std_new, integer_ctor,
      ag_std_delete, integer_dtor,
      0);

/*
  void *floating = ag_std_new(
      vtable,     // The type of the object we're creating.
      "floating", // The name of the object. (floating type)
      object,     // The superclass.
      sizeof(struct floating),      // The size of the floating struct
      ag_std_print, floating_print,
      ag_std_delete, floating_dtor,
      ag_std_new, floating_ctor,
      0);

  void *string = ag_std_new(
      vtable,
      "string",
      object,
      sizeof(struct string),
//      ag_std_print, string_print,
      ag_std_delete, string_dtor,
      ag_std_new, string_ctor,
      0);
*/

  printf("creating a new instance of class: integer\n");
  void *i = ag_std_new(integer, 4);
  /*
  void *d = ag_std_new(floating, 4.5);
  void *s = ag_std_new(string, "Hello World");
  */

  ag_std_print(i);
  printf("\n");
  /*
  ag_std_print(d);
  ag_std_print(s);
  */

  void *class_of_i = ag_std_class_of(i);
  /*
  void *class_of_d = ag_std_class_of(d);
  void *class_of_s = ag_std_class_of(s);
  */

  assert(class_of_i == integer);
  /*
  assert(class_of_d == floating);
  assert(class_of_s == string);
  */

  ag_std_delete(i);

  return 0;
}
