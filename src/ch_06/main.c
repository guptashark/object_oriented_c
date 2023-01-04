#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#define USING_SUPER

#define DEBUG_MSG 0

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

  if (DEBUG_MSG) {
    printf("[object][ctor]\n");
  }

  return obj;
}

void object_dtor(void *obj) {
  // No extra memory to free.
  (void)obj;

  if (DEBUG_MSG) {
    printf("[object][dtor]\n");
  }
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
  if (DEBUG_MSG) {
    printf("[vtable][ctor]\n");
  }

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
  va_list ap = *app;

  void *f = va_arg(ap, void *);
  while (f != NULL) {
    void *g = va_arg(ap, void *);
    if (f == ag_std_new) {
      self->ctor = g;
    } else if (f == ag_std_delete) {
      self->dtor = g;
    } else if (f == ag_std_print) {
      self->print = g;
    }

    f = va_arg(ap, void *);
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

  if (DEBUG_MSG) {
    printf("[integer][ctor]\n");
  }

  return obj;
}

void integer_dtor(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[integer][dtor]\n");
  }

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
  // struct ag_std_vtable *vt = ag_std_class_of(obj);
  // vt->super->print(obj);
  printf("%d", i->x);
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

  if (DEBUG_MSG) {
    printf("[floating][ctor]\n");
  }

  return obj;
}

void floating_dtor(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[floating][dtor]\n");
  }
}

void floating_print(void *obj) {
  struct floating *i = (struct floating *)obj;
  // struct ag_std_vtable *vt = *(struct ag_std_vtable **)obj;
  // printf("[%s][print][%f]\n", vt->name, i->x);
  printf("%.2f", i->x);
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

  if (DEBUG_MSG) {
    printf("[string][ctor]\n");
  }

  return obj;
}

void string_dtor(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[string][dtor]\n");
  }
}

void string_print(void *obj) {
  struct string *str_obj = (struct string *)obj;
  // struct ag_std_vtable *vt = *(struct ag_std_vtable **)obj;
  // printf("[%s][print][%s]\n", vt->name, str_obj->s);
  printf("\"%s\"", str_obj->s);
}

///////////////////////////////////////////////////////////////////////////////
// list (derived from object)
///////////////////////////////////////////////////////////////////////////////

/* Our list will (at the moment) allow for any kind of object to be
 * inserted into it, not just objects of a specific type, (though we can
 * technically enforce this by providing the allowed type during
 * the lists construction, ie: void *lst = ag_std_new(list, integer),
 * at which point every insert will do a check to ensure that the object
 * being inserted is in fact of type integer.
 */

struct list_node {
  void *obj; // not a pointer to the base object, just a pointer to
             // the newly inserted object.
  struct list_node *next;
  struct list_node *prev;
};

// Internal function, nobody except list functions will call this.
struct list_node *list_node_ctor(void *obj) {
  struct list_node *node = malloc(sizeof(struct list_node));
  node->next = NULL;
  node->prev = NULL;
  node->obj = obj;
  return node;
}

void list_node_link(struct list_node *a, struct list_node *b) {
  a->next = b;
  b->prev = a;
}

void list_node_insert
(struct list_node *a, struct list_node *b, struct list_node *c) {
  a->next = b;
  b->next = c;

  b->prev = a;
  c->prev = b;
}

struct list {
  struct object obj; // base

  struct list_node *front;
  struct list_node *back;
  size_t size;
};

void *list_ctor(void *obj, va_list *app) {
  // does nothing.
  (void)app;

  if (DEBUG_MSG) {
    printf("[list][ctor]\n");
  }

  struct list *lst = obj;

  struct list_node *a = list_node_ctor(NULL);
  struct list_node *b = list_node_ctor(NULL);

  list_node_link(a, b);
  lst->front = a;
  lst->back = b;

  lst->size = 0;

  return obj;
}

void list_dtor(void *obj) {
  (void)obj;

  // TODO: Delete stuff.

  if (DEBUG_MSG) {
    printf("[list][dtor]\n");
  }
}

void list_print(void *obj) {
  struct list *lst = obj;

  if (lst->size == 0) {
    printf("[]");
    return;
  } else {

    printf("[");
    struct list_node *c = lst->front->next;
    while (c->next->obj != NULL) {
      ag_std_print(c->obj);
      printf(", ");
      c = c->next;
    }

    ag_std_print(c->obj);
    printf("]");
  }

}

void *list_begin(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[list][begin]\n");
  }

  return NULL;
}

void *list_end(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[list][end]\n");
  }

  return NULL;
}

void list_push_front(void *lst_arg, void *obj) {
  struct list *lst = lst_arg;
  struct list_node *new_node = list_node_ctor(obj);

  struct list_node *a = lst->front;
  struct list_node *c = a->next;
  list_node_insert(a, new_node, c);

  lst->size++;
}

void list_push_back(void *lst_arg, void *obj) {
  struct list *lst = lst_arg;
  struct list_node *new_node = list_node_ctor(obj);

  struct list_node *c = lst->back;
  struct list_node *a = c->prev;
  list_node_insert(a, new_node, c);

  lst->size++;
}

///////////////////////////////////////////////////////////////////////////////
// vector (derived from object)
///////////////////////////////////////////////////////////////////////////////

struct vector {
  struct object obj;

  // We can lengthen as needed with other fields,
  // but this is an abstract class, so children of this class will
  // use the begin and end methods.
};

void *vector_ctor(void *obj, va_list *app) {
  // does nothing.
  (void)obj;
  (void)app;

  if (DEBUG_MSG) {
    printf("[vector][ctor]\n");
  }

  return obj;
}

void vector_dtor(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[vector][dtor]\n");
  }
}

void vector_print(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[vector][print]\n");
  }
}

void *vector_begin(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[vector][begin]\n");
  }

  return NULL;
}

void *vector_end(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[vector][end]\n");
  }

  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// container_vtable (derived from and vtable)
///////////////////////////////////////////////////////////////////////////////
struct container_vtable {
  struct ag_std_vtable vt;
  void *(*begin)(void *);
  void *(*end)(void *);
};

// Forward declare these so that the container_vtable_ctor has them.
void *ag_std_begin(void *obj);
void *ag_std_end(void *obj);

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

  if (DEBUG_MSG) {
    printf("[container_vtable][ctor]\n");
  }

  // Assign the container begin and end functions.
  va_list ap = *app;

  void *f = va_arg(ap, void *);

  while (f != NULL) {
    void *g = va_arg(ap, void *);
    if (f == ag_std_begin) {
      container_vt->begin = g;
    } else if (f == ag_std_end) {
      container_vt->end = g;
    }

    f = va_arg(ap, void *);
  }

  return obj;
}

void container_vtable_dtor(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[container_vtable][dtor]\n");
  }
}

void *ag_std_begin(void *obj) {
  struct container_vtable *cvt = *(struct container_vtable **)obj;
  return cvt->begin(obj);
}

void *ag_std_end(void *obj) {
  struct container_vtable *cvt = *(struct container_vtable **)obj;
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

  void *container_vtable = ag_std_new(
      vtable,               // The type of the object.
      "container_vtable",   // The name of the object.
      vtable,               // The superclass.
      sizeof(struct container_vtable),  // The size of the structure...
      ag_std_new, container_vtable_ctor,
      0);

  void *list = ag_std_new(
      container_vtable,     // The type of the object.
      "list",               // The name of the object.
      object,               // The superclass.
      sizeof(struct list), // The size of the objects.
      ag_std_new, list_ctor,
      ag_std_begin, list_begin,
      ag_std_end, list_end,
      ag_std_print, list_print,
      0);

  void *vector = ag_std_new(
      container_vtable,
      "vector",
      object,
      sizeof(struct vector),
      ag_std_new, vector_ctor,
      ag_std_begin, vector_begin,
      ag_std_end, vector_end,
      0);

  void *lst = ag_std_new(list);

  ag_std_print(lst);
  printf("\n");
  assert(ag_std_class_of(lst) == list);
  ag_std_begin(lst);
  ag_std_end(lst);

  void *v = ag_std_new(vector);
  ag_std_print(v);
  printf("\n");
  assert(ag_std_class_of(v) == vector);
  ag_std_begin(v);
  ag_std_end(v);

  /*
  void *obj = ag_std_new(object);
  ag_std_print(obj);
  */

  void *integer = ag_std_new(
      vtable,     // The type of object we are creating.
      "integer",  // The name of the object. (integer type)
      object,     // The superclass.
      sizeof(struct integer),       // The size of the integer structure.
      ag_std_print, integer_print,  // The method of obj that we override.
      ag_std_new, integer_ctor,
      ag_std_delete, integer_dtor,
      0);


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
      ag_std_print, string_print,
      ag_std_delete, string_dtor,
      ag_std_new, string_ctor,
      0);

  void *s1 = ag_std_new(string, "Dumbledore");

  void *i = ag_std_new(integer, 4);
  void *i2 = ag_std_new(integer, 7);
  void *i3 = ag_std_new(integer, 1);
  void *i4 = ag_std_new(integer, -3);

  /*
  void *d = ag_std_new(floating, 4.5);
  void *s = ag_std_new(string, "Hello World");
  */

  list_push_front(lst, i);
  list_push_front(lst, i2);
  list_push_front(lst, s1);
  list_push_front(lst, i3);
  list_push_front(lst, i4);
  list_push_front(lst, ag_std_new(floating, 5.3));

  ag_std_print(lst);
  printf("\n");

  ag_std_print(i);
  printf("\n");
  {
    void *ai = ag_std_new(integer, 10);
    void *as = ag_std_new(string, "ten");

    void *bi = ag_std_new(integer, 20);
    void *bs = ag_std_new(string, "twenty");

    void *ci = ag_std_new(integer, 30);
    void *cs = ag_std_new(string, "thirty");

    void *lst_a = ag_std_new(list);
    list_push_front(lst_a, as);
    list_push_front(lst_a, ai);

    void *lst_b = ag_std_new(list);
    list_push_front(lst_b, bs);
    list_push_front(lst_b, bi);

    void *lst_c = ag_std_new(list);
    list_push_front(lst_c, cs);
    list_push_front(lst_c, ci);

    void *lst_final = ag_std_new(list);
    list_push_front(lst_final, lst_c);
    list_push_front(lst_final, lst_b);
    list_push_front(lst_final, lst_a);

    ag_std_print(lst_final);
    printf("\n");
  }

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

  return 0;
}
