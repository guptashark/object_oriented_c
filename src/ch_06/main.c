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

// TODO: Remove this from the global namespace... somehow.
// Had to put this here so list_begin and end would know the type of
// object to create.
void *list_iter;

void *list_begin(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[list][begin]\n");
  }

  struct list *lst = obj;
  struct list_node *c = lst->front->next;

  return ag_std_new(list_iter, c);
}

void *list_end(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[list][end]\n");
  }
  struct list *lst = obj;
  struct list_node *c = lst->back;

  return ag_std_new(list_iter, c);
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

/* Our vector will (at the moment) allow for any kind of object to be
 * inserted into it, not just objects of a specific type, (though we can
 * technically enforce this by providing the allowed type during
 * the construction, ie: void *v = ag_std_new(vector, integer),
 * at which point every insert will do a check to ensure that the object
 * being inserted is in fact of type integer.
 */
struct vector {
  struct object obj;

  void **arr;
  size_t size;
  size_t capacity;
};

void *vector_ctor(void *obj, va_list *app) {
  // does nothing.
  (void)app;

  if (DEBUG_MSG) {
    printf("[vector][ctor]\n");
  }

  struct vector *v = obj;

  v->arr = malloc(sizeof(void *) * 8);
  v->size = 0;
  v->capacity = 8;

  return obj;
}

void vector_dtor(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[vector][dtor]\n");
  }

  // TODO: Free the data.
}

void vector_print(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[vector][print]\n");
  }

  struct vector *v = obj;

  if (v->size == 0) {
    printf("vector([])");
    return;
  }

  printf("vector([");
  for (size_t i = 0; i < v->size - 1; ++i) {
    ag_std_print(v->arr[i]);
    printf(", ");
  }

  ag_std_print(v->arr[v->size - 1]);
  printf("])");
}

// TODO: Remove this from the global namespace... somehow.
// Had to put this here so vec_begin and vec_end would know the type of
// object to create.
void *vec_iter;

void *vector_begin(void *obj) {

  if (DEBUG_MSG) {
    printf("[vector][begin]\n");
  }

  struct vector *v = obj;
  return ag_std_new(vec_iter, v->arr, 0);
}

void *vector_end(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[vector][end]\n");
  }

  struct vector *v = obj;
  return ag_std_new(vec_iter, v->arr, v->size);
}

void vector_push_back(void *vec_arg, void *obj) {
  struct vector *v = vec_arg;

  v->arr[v->size] = obj;
  v->size++;
}

///////////////////////////////////////////////////////////////////////////////
// container_vtable (derived from vtable)
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

///////////////////////////////////////////////////////////////////////////////
// iterator_vtable (derived from vtable)
///////////////////////////////////////////////////////////////////////////////
struct iterator_vtable {
  struct ag_std_vtable vt;

  void (*increment)(void *);
  void *(*deref)(void *);
  int (*not_equal)(void *, void *);
};

// Forward declare these so that the iterator_vtable_ctor has them.
void ag_std_iter_increment(void *obj);
void *ag_std_iter_deref(void *obj);
int ag_std_iter_not_equal(void *obj_a, void *obj_b);

void *iterator_vtable_ctor(void *obj, va_list *app) {

  // Run the parent ctor using super.
  // We don't need to keep track of who the parent is.
  struct iterator_vtable *iter_vt = NULL;
  struct ag_std_vtable *super = (struct ag_std_vtable *)ag_std_super(obj);
  if (super->ctor == NULL) {
    printf("super ctor is null??\n");
  } else {
    iter_vt = super->ctor(obj, app);
  }

  if (DEBUG_MSG) {
    printf("[iterator_vtable][ctor]\n");
  }

  // Assign the container begin and end functions.
  va_list ap = *app;

  void *f = va_arg(ap, void *);

  while (f != NULL) {
    void *g = va_arg(ap, void *);
    if (f == ag_std_iter_increment) {
      iter_vt->increment = g;
    } else if (f == ag_std_iter_deref) {
      iter_vt->deref = g;
    } else if (f == ag_std_iter_not_equal) {
      iter_vt->not_equal = g;
    }

    f = va_arg(ap, void *);
  }

  return obj;
}

void iterator_vtable_dtor(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[iterator_vtable][dtor]\n");
  }
}

void ag_std_iter_increment(void *obj) {
  struct iterator_vtable *ivt = *(struct iterator_vtable **)obj;
  return ivt->increment(obj);
}

void *ag_std_iter_deref(void *obj) {
  struct iterator_vtable *ivt = *(struct iterator_vtable **)obj;
  return ivt->deref(obj);
}

int ag_std_iter_not_equal(void *obj_a, void *obj_b) {
  struct iterator_vtable *ivt = *(struct iterator_vtable **)obj_a;
  return ivt->not_equal(obj_a, obj_b);
}

///////////////////////////////////////////////////////////////////////////////
// list_iter (derived from object)
///////////////////////////////////////////////////////////////////////////////

struct list_iter {
  struct object obj;
  struct list_node *c;
};

void *list_iter_ctor(void *obj, va_list *app) {
  if (DEBUG_MSG) {
    printf("[list_iter_ctor]\n");
  }

  // TODO
  // Need to call the super ctor.

  struct list_iter *li = obj;
  li->c = va_arg(*app, struct list_node *);

  return obj;
}

void list_iter_dtor(void *obj) {
  // TODO
  (void)obj;
}

void list_iter_print(void *obj) {
  (void)obj;
  printf("[list_iter][print]");
}

void list_iter_increment(void *obj) {
  if (DEBUG_MSG) {
    printf("[list_iter_inc]\n");
  }
  struct list_iter *li = obj;
  li->c = li->c->next;
}

void *list_iter_deref(void *obj) {
  if (DEBUG_MSG) {
    printf("[list_iter_deref]\n");
  }

  struct list_iter *li = obj;
  return li->c->obj;
}

int list_iter_not_equal(void *obj_a, void *obj_b) {
  if (DEBUG_MSG) {
    printf("[list_iter_neq]\n");
  }

  struct list_iter *a = obj_a;
  struct list_iter *b = obj_b;

  return a->c != b->c;
}

///////////////////////////////////////////////////////////////////////////////
// vec_iter (derived from object)
///////////////////////////////////////////////////////////////////////////////

struct vec_iter {
  struct object obj;

  void **arr;
  size_t i;
};

void *vec_iter_ctor(void *obj, va_list *app) {
  if (DEBUG_MSG) {
    printf("[vec_iter_ctor]\n");
  }

  // TODO
  // Need to call the super ctor.

  struct vec_iter *vi = obj;
  vi->arr = va_arg(*app, void **);
  vi->i = va_arg(*app, size_t);

  return obj;
}

void vec_iter_dtor(void *obj) {
  // TODO
  (void)obj;
}

void vec_iter_print(void *obj) {
  (void)obj;
  printf("[vec_iter][print]");
}

void vec_iter_increment(void *obj) {
  if (DEBUG_MSG) {
    printf("[vec_iter_inc]\n");
  }
  struct vec_iter *vi = obj;
  vi->i++;
}

void *vec_iter_deref(void *obj) {
  if (DEBUG_MSG) {
    printf("[vec_iter_deref]\n");
  }

  struct vec_iter *vi = obj;
  return vi->arr[vi->i];
}

int vec_iter_not_equal(void *obj_a, void *obj_b) {
  if (DEBUG_MSG) {
    printf("[vec_iter_neq]\n");
  }

  struct vec_iter *a = obj_a;
  struct vec_iter *b = obj_b;

  return a->i != b->i;
}

///////////////////////////////////////////////////////////////////////////////
// range functions
///////////////////////////////////////////////////////////////////////////////
void ag_std_range_print(void *rng) {

  void *it = ag_std_begin(rng);
  void *end = ag_std_end(rng);

  printf("range_print( ");

  while (ag_std_iter_not_equal(it, end)) {
    void *val = ag_std_iter_deref(it);
    ag_std_print(val);
    printf(" ");
    ag_std_iter_increment(it);
  }

  printf(")\n");
}

int ag_std_range_find(void *rng, void *val) {

  void *it = ag_std_begin(rng);
  void *end = ag_std_end(rng);

  while (ag_std_iter_not_equal(it, end)) {
    void *obj = ag_std_iter_deref(it);
    if (obj == val) {
      return 1;
    }

    ag_std_iter_increment(it);
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// main
///////////////////////////////////////////////////////////////////////////////

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
      ag_std_print, vector_print,
      0);

  void *iterator_vtable = ag_std_new(
      vtable,               // The type of the object.
      "iterator_vtable",   // The name of the object.
      vtable,               // The superclass.
      sizeof(struct iterator_vtable),  // The size of the structure...
      ag_std_new, iterator_vtable_ctor,
      0);

  // The list_iter symbol must exist outside of the main function, because
  // the list_begin and list_end functions create iterators.
  list_iter = ag_std_new(
      iterator_vtable,
      "list_iter",
      object,
      sizeof(struct list_iter),
      ag_std_new, list_iter_ctor,
      ag_std_iter_increment, list_iter_increment,
      ag_std_iter_deref, list_iter_deref,
      ag_std_iter_not_equal, list_iter_not_equal,
      0);

  // The vec iter symbol must also exist outside of the main fn,
  // becuase vec_begin and vec_end functions create iterators.
  vec_iter = ag_std_new(
      iterator_vtable,
      "vec_iter",
      object,
      sizeof(struct vec_iter),
      ag_std_new, vec_iter_ctor,
      ag_std_iter_increment, vec_iter_increment,
      ag_std_iter_deref, vec_iter_deref,
      ag_std_iter_not_equal, vec_iter_not_equal,
      0);

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

  void *lst = ag_std_new(list);

  ag_std_print(lst);
  printf("\n");
  assert(ag_std_class_of(lst) == list);

  void *v = ag_std_new(vector);
  ag_std_print(v);
  printf("\n");
  assert(ag_std_class_of(v) == vector);

  void *s1 = ag_std_new(string, "Dumbledore");

  void *i = ag_std_new(integer, 4);
  void *i2 = ag_std_new(integer, 7);
  void *i3 = ag_std_new(integer, 1);
  void *i4 = ag_std_new(integer, -3);

  list_push_back(lst, i);
  list_push_back(lst, i2);
  list_push_back(lst, s1);
  list_push_back(lst, i3);
  list_push_back(lst, i4);
  list_push_back(lst, ag_std_new(floating, 5.3));

  {
    printf("Printing this with a list iterator: ");
    printf("{ ");
    void *it = list_begin(lst);
    void *lst_end = list_end(lst);
    (void)lst_end;

    while (ag_std_iter_not_equal(it, lst_end)) {
      ag_std_print(ag_std_iter_deref(it));
      printf(" ");
      ag_std_iter_increment(it);
    }

    printf("}\n");

    ag_std_range_print(lst);
  }

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
    list_push_back(lst_a, ai);
    list_push_back(lst_a, as);

    void *lst_b = ag_std_new(list);
    list_push_back(lst_b, bi);
    list_push_back(lst_b, bs);

    void *lst_c = ag_std_new(list);
    list_push_back(lst_c, ci);
    list_push_back(lst_c, cs);

    void *lst_final = ag_std_new(list);
    list_push_back(lst_final, lst_a);

    ag_std_print(lst_final);
    printf("\n");

    list_push_back(lst_final, lst_b);
    list_push_back(lst_final, lst_c);

    ag_std_print(lst_final);
    printf("\n");
  }

  {
    void *ai = ag_std_new(integer, 10);
    void *bi = ag_std_new(integer, 20);
    void *cs = ag_std_new(string, "thirty");

    void *v = ag_std_new(vector);
    ag_std_print(v);
    printf("\n");

    vector_push_back(v, ai);
    vector_push_back(v, bi);
    vector_push_back(v, cs);

    ag_std_print(v);
    printf("\n");

    {
      printf("Printing this with a vec iterator: ");
      printf("{ ");
      void *it = vector_begin(v);
      void *vec_end = vector_end(v);

      while (ag_std_iter_not_equal(it, vec_end)) {
        ag_std_print(ag_std_iter_deref(it));
        printf(" ");
        ag_std_iter_increment(it);
      }

      printf("}\n");

      ag_std_range_print(v);

      int found = ag_std_range_find(v, bi);
      if (found) {
        printf("Find function works!\n");
      } else {
        printf("Find function failed.\n");
      }

      void *z = ag_std_new(integer, 0);

      found = ag_std_range_find(v, z);
      if (found == 0) {
        printf("Find function works!\n");
      } else {
        printf("Find function failed!\n");
      }
    }
  }

  void *class_of_i = ag_std_class_of(i);
  assert(class_of_i == integer);

  return 0;
}
