#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

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
  int (*cmp)(void *, void *);
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

int object_cmp(void *obj_a, void *obj_b) {
  if (obj_a == obj_b) {
    return 0;
  } else {
    return 1;
  }
}

// The vtable of an object.
struct ag_std_vtable object_vt = {
  // object struct.
  "object", // name
  NULL, // ptr to vtable of the super
  sizeof(struct object), // How come this doesn't work?
  object_ctor,
  object_dtor,
  object_print,
  object_cmp
};

// The ptr to a vtable of an object.
const struct ag_std_vtable *object = &object_vt;
// const struct ag_std_vtable *object = &object_vt;

// An instance of object... do we need this?

void *ag_std_new(const struct ag_std_vtable *vt, ...);
void ag_std_delete(void *);
void ag_std_print(void *);
int ag_std_cmp(void *, void *);

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
    } else if (f == ag_std_cmp) {
      self->cmp = g;
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

int vtable_cmp(void *obj_a, void *obj_b) {
  return obj_a != obj_b;
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

int ag_std_cmp(void *obj_a, void *obj_b) {
  struct ag_std_vtable *vt = *(struct ag_std_vtable **)obj_a;
  return vt->cmp(obj_a, obj_b);
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

  // Run the parent ctor using super.
  // We don't need to keep track of who the parent is.
  struct ag_std_vtable *super = (struct ag_std_vtable *)ag_std_super(obj);
  if (super->ctor == NULL) {
    printf("super ctor is null??\n");
  } else {
    super->ctor(obj, app);
  }

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

  // Run the parent dtor using super.
  // We don't need to keep track of who the parent is.
  struct ag_std_vtable *super = (struct ag_std_vtable *)ag_std_super(obj);
  super->dtor(obj);
}

void integer_print(void *obj) {
  struct integer *i = (struct integer *)obj;
  // struct ag_std_vtable *vt = ag_std_class_of(obj);
  // vt->super->print(obj);
  printf("%d", i->x);
}

int integer_cmp(void *obj_a, void *obj_b) {
  struct integer *a = (struct integer *)obj_a;
  struct integer *b = (struct integer *)obj_b;
  return a->x - b->x;
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

int floating_cmp(void *obj_a, void *obj_b) {
  struct floating *a = (struct floating *)obj_a;
  struct floating *b = (struct floating *)obj_b;

  return a->x - b->x;
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

int string_cmp(void *obj_a, void *obj_b) {
  struct string *a = (struct string *)obj_a;
  struct string *b = (struct string *)obj_b;

  (void)a;
  (void)b;
  // TODO
  // Need to actually implement this but do it later.
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Pair (derived from object)
///////////////////////////////////////////////////////////////////////////////

struct ag_std_pair {
  struct object obj; // base

  void *first;
  void *second;
};

void *ag_std_pair_ctor(void *obj, va_list *app) {
  struct ag_std_pair *p = (struct ag_std_pair *)obj;
  p->first = va_arg(*app, void *);
  p->second = va_arg(*app, void *);

  return obj;
}

void ag_std_pair_dtor(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[ag_std_pair][dtor]\n");
  }
}

void ag_std_pair_print(void *obj) {
  struct ag_std_pair *p = obj;
  // struct ag_std_vtable *vt = *(struct ag_std_vtable **)obj;
  // printf("[%s][print][%s]\n", vt->name, str_obj->s);
  printf("(");
  ag_std_print(p->first);
  printf(", ");
  ag_std_print(p->second);
  printf(")");
}

int ag_std_pair_cmp(void *obj_a, void *obj_b) {
  struct ag_std_pair *a = obj_a;
  struct ag_std_pair *b = obj_b;

  (void)a;
  (void)b;
  // TODO
  // Need to actually implement this but do it later.
  return 0;
}

// Specialized functions - first and second.
void *ag_std_pair_first(void *obj) {
  struct ag_std_pair *p = obj;
  return p->first;
}

void *ag_std_pair_second(void *obj) {
  struct ag_std_pair *p = obj;
  return p->second;
}

///////////////////////////////////////////////////////////////////////////////
// ag_std_list (derived from object)
///////////////////////////////////////////////////////////////////////////////

/* Our ag_std_list will (at the moment) allow for any kind of object to be
 * inserted into it, not just objects of a specific type, (though we can
 * technically enforce this by providing the allowed type during
 * the lists construction, ie: void *lst = ag_std_new(ag_std_list, integer),
 * at which point every insert will do a check to ensure that the object
 * being inserted is in fact of type integer.
 */

struct ag_std_list_node {
  void *obj; // not a pointer to the base object, just a pointer to
             // the newly inserted object.
  struct ag_std_list_node *next;
  struct ag_std_list_node *prev;
};

// Internal function, nobody except ag_std_list functions will call this.
struct ag_std_list_node *ag_std_list_node_ctor(void *obj) {
  struct ag_std_list_node *node = malloc(sizeof(struct ag_std_list_node));
  node->next = NULL;
  node->prev = NULL;
  node->obj = obj;
  return node;
}

void ag_std_list_node_link
(struct ag_std_list_node *a, struct ag_std_list_node *b) {

  a->next = b;
  b->prev = a;
}

void ag_std_list_node_insert(
    struct ag_std_list_node *a,
    struct ag_std_list_node *b,
    struct ag_std_list_node *c)
{
  a->next = b;
  b->next = c;

  b->prev = a;
  c->prev = b;
}

struct ag_std_list {
  struct object obj; // base

  struct ag_std_list_node *front;
  struct ag_std_list_node *back;
  size_t size;
};

void *ag_std_list_ctor(void *obj, va_list *app) {
  // does nothing.
  (void)app;

  if (DEBUG_MSG) {
    printf("[ag_std_list][ctor]\n");
  }

  struct ag_std_list *lst = obj;

  struct ag_std_list_node *a = ag_std_list_node_ctor(NULL);
  struct ag_std_list_node *b = ag_std_list_node_ctor(NULL);

  ag_std_list_node_link(a, b);
  lst->front = a;
  lst->back = b;

  lst->size = 0;

  return obj;
}

void ag_std_list_dtor(void *obj) {
  (void)obj;

  // TODO: Delete stuff.

  if (DEBUG_MSG) {
    printf("[ag_std_list][dtor]\n");
  }
}

void ag_std_list_print(void *obj) {
  struct ag_std_list *lst = obj;

  if (lst->size == 0) {
    printf("[]");
    return;
  } else {

    printf("[");
    struct ag_std_list_node *c = lst->front->next;
    while (c->next->obj != NULL) {
      ag_std_print(c->obj);
      printf(", ");
      c = c->next;
    }

    ag_std_print(c->obj);
    printf("]");
  }
}

int ag_std_list_cmp(void *obj_a, void *obj_b) {
  struct ag_std_list *a = (struct ag_std_list *)obj_a;
  struct ag_std_list *b = (struct ag_std_list *)obj_b;

  (void)a;
  (void)b;

  // TODO
  // Need to actually implement this but do it later.
  return 0;
}

// TODO: Remove this from the global namespace... somehow.
// Had to put this here so ag_std_list_begin and end would know the type of
// object to create.
void *ag_std_list_iter;

void *ag_std_list_begin(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[ag_std_list][begin]\n");
  }

  struct ag_std_list *lst = obj;
  struct ag_std_list_node *c = lst->front->next;

  return ag_std_new(ag_std_list_iter, c);
}

void *ag_std_list_end(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[ag_std_list][end]\n");
  }
  struct ag_std_list *lst = obj;
  struct ag_std_list_node *c = lst->back;

  return ag_std_new(ag_std_list_iter, c);
}

void ag_std_list_push_front(void *lst_arg, void *obj) {
  struct ag_std_list *lst = lst_arg;
  struct ag_std_list_node *new_node = ag_std_list_node_ctor(obj);

  struct ag_std_list_node *a = lst->front;
  struct ag_std_list_node *c = a->next;
  ag_std_list_node_insert(a, new_node, c);

  lst->size++;
}

void ag_std_list_push_back(void *lst_arg, void *obj) {
  struct ag_std_list *lst = lst_arg;
  struct ag_std_list_node *new_node = ag_std_list_node_ctor(obj);

  struct ag_std_list_node *c = lst->back;
  struct ag_std_list_node *a = c->prev;
  ag_std_list_node_insert(a, new_node, c);

  lst->size++;
}

///////////////////////////////////////////////////////////////////////////////
// ag_std_vector (derived from object)
///////////////////////////////////////////////////////////////////////////////

/* Our vector will (at the moment) allow for any kind of object to be
 * inserted into it, not just objects of a specific type, (though we can
 * technically enforce this by providing the allowed type during
 * the construction, ie: void *v = ag_std_new(vector, integer),
 * at which point every insert will do a check to ensure that the object
 * being inserted is in fact of type integer.
 */
struct ag_std_vector {
  struct object obj;

  void **arr;
  size_t size;
  size_t capacity;
};

void *ag_std_vector_ctor(void *obj, va_list *app) {
  // does nothing.
  (void)app;

  if (DEBUG_MSG) {
    printf("[ag_std_vector][ctor]\n");
  }

  struct ag_std_vector *v = obj;

  v->arr = malloc(sizeof(void *) * 8);
  v->size = 0;
  v->capacity = 8;

  return obj;
}

void ag_std_vector_dtor(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[ag_std_vector][dtor]\n");
  }

  // TODO: Free the data.
}

void ag_std_vector_print(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[ag_std_vector][print]\n");
  }

  struct ag_std_vector *v = obj;

  if (v->size == 0) {
    printf("ag_std_vector([])");
    return;
  }

  printf("ag_std_vector([");
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
void *ag_std_vector_iter;

void *ag_std_vector_begin(void *obj) {

  if (DEBUG_MSG) {
    printf("[ag_std_vector][begin]\n");
  }

  struct ag_std_vector *v = obj;
  return ag_std_new(ag_std_vector_iter, v->arr, 0);
}

void *ag_std_vector_end(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[ag_std_vector][end]\n");
  }

  struct ag_std_vector *v = obj;
  return ag_std_new(ag_std_vector_iter, v->arr, v->size);
}

void ag_std_vector_push_back(void *vec_arg, void *obj) {
  struct ag_std_vector *v = vec_arg;

  v->arr[v->size] = obj;
  v->size++;
}

void ag_std_vector_pop_back(void *vec_arg) {
  struct ag_std_vector *v = vec_arg;
  if (v->size > 0) {
    // Should we call the dtor on the popped object?
    // TODO Decide what to do with popped object.
    v->size--;
  }
}

///////////////////////////////////////////////////////////////////////////////
// ag_std_map (derived from object)
///////////////////////////////////////////////////////////////////////////////

/* Our map will (at the moment) allow for any kind of object to be
 * inserted into it, not just objects of a specific type, (though we can
 * technically enforce this by providing the allowed type during
 * the construction, ie: void *v = ag_std_new(ag_std_map, integer, string),
 * at which point every insert will do a check to ensure that the key and
 * value being inserted are the correct types.
 */
struct ag_std_map {
  struct object obj;

  void **arr;
  size_t size;
  size_t capacity;
};

void *ag_std_map_ctor(void *obj, va_list *app) {
  // does nothing.
  (void)app;

  if (DEBUG_MSG) {
    printf("[ag_std_map][ctor]\n");
  }

  struct ag_std_map *m = obj;

  m->arr = malloc(sizeof(void *) * 8);
  m->size = 0;
  m->capacity = 8;

  return obj;
}

void ag_std_map_dtor(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[ag_std_map][dtor]\n");
  }

  // TODO: Free the data.
}

void ag_std_map_print(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[ag_std_map][print]\n");
  }

  struct ag_std_map *m = obj;

  if (m->size == 0) {
    printf("ag_std_map([])");
    return;
  }

  printf("ag_std_map([");
  for (size_t i = 0; i < m->size - 1; ++i) {
    ag_std_print(m->arr[i]);
    printf(", ");
  }

  ag_std_print(m->arr[m->size - 1]);
  printf("])");
}

// TODO: Remove this from the global namespace... somehow.
// Had to put this here so ag_std_map_begin and ag_std_map_end
// would know the type of object to create.
void *ag_std_map_iter;

void *ag_std_map_begin(void *obj) {

  if (DEBUG_MSG) {
    printf("[ag_std_map][begin]\n");
  }

  struct ag_std_map *m = obj;
  return ag_std_new(ag_std_map_iter, m->arr, 0);
}

void *ag_std_map_end(void *obj) {

  if (DEBUG_MSG) {
    printf("[ag_std_map][end]\n");
  }

  struct ag_std_map *m = obj;
  return ag_std_new(ag_std_map_iter, m->arr, m->size);
}

// We are inserting a pair: (key, value)
void ag_std_map_insert(void *map_arg, void *obj) {
  struct ag_std_map *m = map_arg;

  m->arr[m->size] = obj;
  m->size++;
}

void *ag_std_map_at(void *map_arg, void *key) {
  struct ag_std_map *m = map_arg;

  for (size_t i = 0; i < m->size; ++i) {
    void *p = m->arr[i];
    void *first = ag_std_pair_first(p);
    if (ag_std_cmp(first, key) == 0) {
      return ag_std_pair_second(p);
    }
  }

  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// iota_view
///////////////////////////////////////////////////////////////////////////////

struct ag_std_iota_view {
  struct object obj;
  int size;
};

void *ag_std_iota_view_ctor(void *obj, va_list *app) {
  // does nothing.
  (void)app;

  if (DEBUG_MSG) {
    printf("[ag_std_iota_view][ctor]\n");
  }

  struct ag_std_iota_view *v = obj;

  // TODO: Should this be... an integer object?
  // For now, we can do this, but later it might be better
  // to instead use the integer object.

  // TODO: Call the parent ctor.
  v->size = va_arg(*app, int);

  return obj;
}

void ag_std_iota_view_dtor(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[ag_std_iota_view][dtor]\n");
  }

  // TODO: Free the data.
}

void ag_std_iota_view_print(void *obj) {

  if (DEBUG_MSG) {
    printf("[ag_std_iota_view][print]\n");
  }

  struct ag_std_iota_view *iv = obj;
  printf("ag_std_iota_view(%d)", iv->size);
}

// TODO: Remove this from the global namespace... somehow.
// Had to put this here so vec_begin and vec_end would know the type of
// object to create.
void *ag_std_iota_view_iter;

void *ag_std_iota_view_begin(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[ag_std_iota_view][begin]\n");
  }

  // We don't even need to reference the obj.
  return ag_std_new(ag_std_iota_view_iter, 0);
}

void *ag_std_iota_view_end(void *obj) {

  if (DEBUG_MSG) {
    printf("[ag_std_iota_view][end]\n");
  }

  struct ag_std_iota_view *iv = obj;
  return ag_std_new(ag_std_iota_view_iter, iv->size);
}

///////////////////////////////////////////////////////////////////////////////
// ag_std_zip_view
///////////////////////////////////////////////////////////////////////////////


struct ag_std_zip_view {
  struct object obj;

  // The first and second range.
  void *rng_a;
  void *rng_b;
};

void *ag_std_zip_view_ctor(void *obj, va_list *app) {

  if (DEBUG_MSG) {
    printf("[ag_std_zip_view][ctor]\n");
  }

  struct ag_std_zip_view *v = obj;

  // TODO: Call the parent ctor.
  v->rng_a = va_arg(*app, void *);
  v->rng_b = va_arg(*app, void *);

  return obj;
}

void ag_std_zip_view_dtor(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[ag_std_zip_view][dtor]\n");
  }

  // TODO: Free the data.
}

void ag_std_zip_view_print(void *obj) {
  (void)obj;

  if (DEBUG_MSG) {
    printf("[ag_std_zip_view][print]\n");
  }

  // TODO: Some actual printing?
  // struct ag_std_zip_view *iv = obj;
}

// TODO: Remove this from the global namespace... somehow.
// Had to put this here so vec_begin and vec_end would know the type of
// object to create.
void *ag_std_zip_view_iter;
void *ag_std_pair;

// Forward declare these so that the zip_view_begin has them.
void *ag_std_begin(void *obj);
void *ag_std_end(void *obj);

void *ag_std_zip_view_begin(void *obj) {

  if (DEBUG_MSG) {
    printf("[ag_std_zip_view][begin]\n");
  }

  struct ag_std_zip_view *v = obj;
  void *it_a = ag_std_begin(v->rng_a);
  void *it_b = ag_std_begin(v->rng_b);

  void *p = ag_std_new(ag_std_pair, it_a, it_b);

  return ag_std_new(ag_std_zip_view_iter, p);
}

void *ag_std_zip_view_end(void *obj) {

  if (DEBUG_MSG) {
    printf("[ag_std_zip_view][end]\n");
  }

  struct ag_std_zip_view *v = obj;
  void *it_a = ag_std_end(v->rng_a);
  void *it_b = ag_std_end(v->rng_b);

  void *p = ag_std_new(ag_std_pair, it_a, it_b);

  return ag_std_new(ag_std_zip_view_iter, p);
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
// ag_std_list_iter (derived from object)
///////////////////////////////////////////////////////////////////////////////

struct ag_std_list_iter {
  struct object obj;
  struct ag_std_list_node *c;
};

void *ag_std_list_iter_ctor(void *obj, va_list *app) {
  if (DEBUG_MSG) {
    printf("[ag_std_list_iter_ctor]\n");
  }

  // TODO
  // Need to call the super ctor.

  struct ag_std_list_iter *li = obj;
  li->c = va_arg(*app, struct ag_std_list_node *);

  return obj;
}

void ag_std_list_iter_dtor(void *obj) {
  // TODO
  (void)obj;
}

void ag_std_list_iter_print(void *obj) {
  (void)obj;
  printf("[ag_std_list_iter][print]");
}

void ag_std_list_iter_increment(void *obj) {
  if (DEBUG_MSG) {
    printf("[ag_std_list_iter_inc]\n");
  }
  struct ag_std_list_iter *li = obj;
  li->c = li->c->next;
}

void *ag_std_list_iter_deref(void *obj) {
  if (DEBUG_MSG) {
    printf("[ag_std_list_iter_deref]\n");
  }

  struct ag_std_list_iter *li = obj;
  return li->c->obj;
}

int ag_std_list_iter_not_equal(void *obj_a, void *obj_b) {
  if (DEBUG_MSG) {
    printf("[ag_std_list_iter_neq]\n");
  }

  struct ag_std_list_iter *a = obj_a;
  struct ag_std_list_iter *b = obj_b;

  return a->c != b->c;
}

///////////////////////////////////////////////////////////////////////////////
// ag_std_vector_iter (derived from object)
///////////////////////////////////////////////////////////////////////////////

struct ag_std_vector_iter {
  struct object obj;

  void **arr;
  size_t i;
};

void *ag_std_vector_iter_ctor(void *obj, va_list *app) {
  if (DEBUG_MSG) {
    printf("[ag_std_vector_iter_ctor]\n");
  }

  // TODO
  // Need to call the super ctor.

  struct ag_std_vector_iter *vi = obj;
  vi->arr = va_arg(*app, void **);
  vi->i = va_arg(*app, size_t);

  return obj;
}

void ag_std_vector_iter_dtor(void *obj) {
  // TODO
  (void)obj;
}

void ag_std_vector_iter_print(void *obj) {
  (void)obj;
  printf("[ag_std_vector_iter][print]");
}

void ag_std_vector_iter_increment(void *obj) {
  if (DEBUG_MSG) {
    printf("[ag_std_vector_iter_inc]\n");
  }
  struct ag_std_vector_iter *vi = obj;
  vi->i++;
}

void *ag_std_vector_iter_deref(void *obj) {
  if (DEBUG_MSG) {
    printf("[ag_std_vector_iter_deref]\n");
  }

  struct ag_std_vector_iter *vi = obj;
  return vi->arr[vi->i];
}

int ag_std_vector_iter_not_equal(void *obj_a, void *obj_b) {
  if (DEBUG_MSG) {
    printf("[ag_std_vector_iter_neq]\n");
  }

  struct ag_std_vector_iter *a = obj_a;
  struct ag_std_vector_iter *b = obj_b;

  return a->i != b->i;
}

///////////////////////////////////////////////////////////////////////////////
// ag_std_iota_view_iter (derived from object)
///////////////////////////////////////////////////////////////////////////////

struct ag_std_iota_view_iter {
  struct object obj;
  int i;
};

void *ag_std_iota_view_iter_ctor(void *obj, va_list *app) {
  if (DEBUG_MSG) {
    printf("[ag_std_vector_iter_ctor]\n");
  }

  // TODO
  // Need to call the super ctor.

  struct ag_std_iota_view_iter *vi = obj;
  vi->i = va_arg(*app, int);

  return obj;
}

void ag_std_iota_view_iter_dtor(void *obj) {
  // TODO
  (void)obj;
}

void ag_std_iota_view_iter_print(void *obj) {
  (void)obj;
  printf("[ag_std_iota_view_iter][print]");
}

void ag_std_iota_view_iter_increment(void *obj) {
  if (DEBUG_MSG) {
    printf("[ag_std_vector_iter_inc]\n");
  }
  struct ag_std_iota_view_iter *vi = obj;
  vi->i++;
}

// TODO: Get rid of this when we can.
// This needs to be forward declared
void *integer;

void *ag_std_iota_view_iter_deref(void *obj) {
  if (DEBUG_MSG) {
    printf("[ag_std_vector_iter_deref]\n");
  }

  struct ag_std_iota_view_iter *vi = obj;
  return ag_std_new(integer, vi->i);
}

int ag_std_iota_view_iter_not_equal(void *obj_a, void *obj_b) {
  if (DEBUG_MSG) {
    printf("[ag_std_vector_iter_neq]\n");
  }

  struct ag_std_iota_view_iter *a = obj_a;
  struct ag_std_iota_view_iter *b = obj_b;

  return a->i != b->i;
}

///////////////////////////////////////////////////////////////////////////////
// ag_std_zip_view_iter (derived from object)
///////////////////////////////////////////////////////////////////////////////

struct ag_std_zip_view_iter {
  struct object obj;

  // A pair of iters zipped together.
  struct ag_std_pair *p;
};

void *ag_std_zip_view_iter_ctor(void *obj, va_list *app) {
  if (DEBUG_MSG) {
    printf("[ag_std_vector_iter_ctor]\n");
  }

  // TODO
  // Need to call the super ctor.

  struct ag_std_zip_view_iter *vi = obj;
  vi->p = va_arg(*app, struct ag_std_pair *);

  return obj;
}

void ag_std_zip_view_iter_dtor(void *obj) {
  // TODO actual implementation.
  (void)obj;
}

void ag_std_zip_view_iter_print(void *obj) {
  (void)obj;
  printf("[ag_std_zip_view_iter][print]");
}

void ag_std_zip_view_iter_increment(void *obj) {
  if (DEBUG_MSG) {
    printf("[zip_iter_inc]\n");
  }

  struct ag_std_zip_view_iter *zv = obj;
  void *a = ag_std_pair_first(zv->p);
  void *b = ag_std_pair_second(zv->p);

  ag_std_iter_increment(a);
  ag_std_iter_increment(b);
}

void *ag_std_zip_view_iter_deref(void *obj) {
  if (DEBUG_MSG) {
    printf("[ag_std_vector_iter_deref]\n");
  }

  struct ag_std_zip_view_iter *zv = obj;

  void *a = ag_std_pair_first(zv->p);
  void *b = ag_std_pair_second(zv->p);

  void *value_a = ag_std_iter_deref(a);
  void *value_b = ag_std_iter_deref(b);

  void *new_pair = ag_std_new(ag_std_pair, value_a, value_b);
  return new_pair;
}

int ag_std_zip_view_iter_not_equal(void *obj_a, void *obj_b) {
  if (DEBUG_MSG) {
    printf("[ag_std_vector_iter_neq]\n");
  }

  struct ag_std_zip_view_iter *zv_a = obj_a;
  struct ag_std_zip_view_iter *zv_b = obj_b;

  void *pa_01 = ag_std_pair_first(zv_a->p);
  void *pa_02 = ag_std_pair_second(zv_a->p);

  void *pb_01 = ag_std_pair_first(zv_b->p);
  void *pb_02 = ag_std_pair_second(zv_b->p);

  int res_01 = ag_std_iter_not_equal(pa_01, pb_01);
  int res_02 = ag_std_iter_not_equal(pa_02, pb_02);

  if (res_01 == 0 || res_02 == 0) {
    return 0;
  } else {
    return 1;
  }
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

    if (ag_std_cmp(obj, val) == 0) {
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
    vtable_print,
    vtable_cmp
  };

  const struct ag_std_vtable *vtable = &vtable_vt;

  void *container_vtable = ag_std_new(
      vtable,               // The type of the object.
      "container_vtable",   // The name of the object.
      vtable,               // The superclass.
      sizeof(struct container_vtable),  // The size of the structure...
      ag_std_new, container_vtable_ctor,
      0);

  void *ag_std_list = ag_std_new(
      container_vtable,     // The type of the object.
      "list",               // The name of the object.
      object,               // The superclass.
      sizeof(struct ag_std_list), // The size of the objects.
      ag_std_new, ag_std_list_ctor,
      ag_std_begin, ag_std_list_begin,
      ag_std_end, ag_std_list_end,
      ag_std_print, ag_std_list_print,
      ag_std_cmp, ag_std_list_cmp,
      0);

  void *vector = ag_std_new(
      container_vtable,
      "vector",
      object,
      sizeof(struct ag_std_vector),
      ag_std_new, ag_std_vector_ctor,
      ag_std_begin, ag_std_vector_begin,
      ag_std_end, ag_std_vector_end,
      ag_std_print, ag_std_vector_print,
      0);

  void *ag_std_map = ag_std_new(
      container_vtable,
      "ag_std_map",
      object,
      sizeof(struct ag_std_map),
      ag_std_new, ag_std_map_ctor,
      ag_std_begin, ag_std_map_begin,
      ag_std_end, ag_std_map_end,
      ag_std_print, ag_std_map_print,
      0);

  void *ag_std_iota_view = ag_std_new(
      container_vtable,
      "ag_std_iota_view",
      object,
      sizeof(struct ag_std_iota_view),
      ag_std_new, ag_std_iota_view_ctor,
      ag_std_begin, ag_std_iota_view_begin,
      ag_std_end, ag_std_iota_view_end,
      ag_std_print, ag_std_iota_view_print,
      0);

  void *ag_std_zip_view = ag_std_new(
      container_vtable,
      "ag_std_zip_view",
      object,
      sizeof(struct ag_std_zip_view),
      ag_std_new, ag_std_zip_view_ctor,
      ag_std_begin, ag_std_zip_view_begin,
      ag_std_end, ag_std_zip_view_end,
      ag_std_print, ag_std_zip_view_print,
      0);

  void *iterator_vtable = ag_std_new(
      vtable,               // The type of the object.
      "iterator_vtable",   // The name of the object.
      vtable,               // The superclass.
      sizeof(struct iterator_vtable),  // The size of the structure...
      ag_std_new, iterator_vtable_ctor,
      0);

  // The ag_std_list_iter symbol must exist outside of the main function, because
  // the ag_std_list_begin and ag_std_list_end functions create iterators.
  ag_std_list_iter = ag_std_new(
      iterator_vtable,
      "ag_std_list_iter",
      object,
      sizeof(struct ag_std_list_iter),
      ag_std_new, ag_std_list_iter_ctor,
      ag_std_iter_increment, ag_std_list_iter_increment,
      ag_std_iter_deref, ag_std_list_iter_deref,
      ag_std_iter_not_equal, ag_std_list_iter_not_equal,
      0);

  // The vec iter symbol must also exist outside of the main fn,
  // becuase vec_begin and vec_end functions create iterators.
  ag_std_vector_iter = ag_std_new(
      iterator_vtable,
      "ag_std_vector_iter",
      object,
      sizeof(struct ag_std_vector_iter),
      ag_std_new, ag_std_vector_iter_ctor,
      ag_std_iter_increment, ag_std_vector_iter_increment,
      ag_std_iter_deref, ag_std_vector_iter_deref,
      ag_std_iter_not_equal, ag_std_vector_iter_not_equal,
      0);

  // The ag_std_iota_view_iter symbol must also exist outside of the main fn,
  // becuase vec_begin and vec_end functions create iterators.
  ag_std_iota_view_iter = ag_std_new(
      iterator_vtable,
      "ag_std_iota_view_iter",
      object,
      sizeof(struct ag_std_iota_view_iter),
      ag_std_new, ag_std_iota_view_iter_ctor,
      ag_std_iter_increment, ag_std_iota_view_iter_increment,
      ag_std_iter_deref, ag_std_iota_view_iter_deref,
      ag_std_iter_not_equal, ag_std_iota_view_iter_not_equal,
      0);

  // The ag_std_zip_view_iter symbol must also exist outside of the main fn,
  // becuase zip_begin and zip_end functions create iterators.
  ag_std_zip_view_iter = ag_std_new(
      iterator_vtable,
      "ag_std_zip_view_iter",
      object,
      sizeof(struct ag_std_zip_view_iter),
      ag_std_new, ag_std_zip_view_iter_ctor,
      ag_std_iter_increment, ag_std_zip_view_iter_increment,
      ag_std_iter_deref, ag_std_zip_view_iter_deref,
      ag_std_iter_not_equal, ag_std_zip_view_iter_not_equal,
      0);

  // This pointer must be declared outside so that zip can use it.
  ag_std_pair = ag_std_new(
      vtable,
      "pair",
      object,
      sizeof(struct ag_std_pair),
      ag_std_new, ag_std_pair_ctor,
      ag_std_delete, ag_std_pair_dtor,
      ag_std_print, ag_std_pair_print,
      ag_std_cmp, ag_std_pair_cmp,
      0);

  // This is not created in the main function, since some functions
  // return an integer object.
  integer = ag_std_new(
      vtable,     // The type of object we are creating.
      "integer",  // The name of the object. (integer type)
      object,     // The superclass.
      sizeof(struct integer),       // The size of the integer structure.
      ag_std_print, integer_print,  // The method of obj that we override.
      ag_std_new, integer_ctor,
      ag_std_delete, integer_dtor,
      ag_std_cmp, integer_cmp,
      0);

  void *floating = ag_std_new(
      vtable,     // The type of the object we're creating.
      "floating", // The name of the object. (floating type)
      object,     // The superclass.
      sizeof(struct floating),      // The size of the floating struct
      ag_std_print, floating_print,
      ag_std_delete, floating_dtor,
      ag_std_new, floating_ctor,
      ag_std_cmp, floating_cmp,
      0);

  void *string = ag_std_new(
      vtable,
      "string",
      object,
      sizeof(struct string),
      ag_std_print, string_print,
      ag_std_delete, string_dtor,
      ag_std_new, string_ctor,
      ag_std_cmp, string_cmp,
      0);

  void *lst = ag_std_new(ag_std_list);

  ag_std_print(lst);
  printf("\n");
  assert(ag_std_class_of(lst) == ag_std_list);

  void *v = ag_std_new(vector);
  ag_std_print(v);
  printf("\n");
  assert(ag_std_class_of(v) == vector);

  void *s1 = ag_std_new(string, "Dumbledore");

  void *i = ag_std_new(integer, 4);
  void *i2 = ag_std_new(integer, 7);
  void *i3 = ag_std_new(integer, 1);
  void *i4 = ag_std_new(integer, -3);

  void *p1 = ag_std_new(ag_std_pair, i2, i3);
  printf("Printing the pair: ");
  ag_std_print(p1);
  printf("\n");
  {
    void *first = ag_std_pair_first(p1);
    void *second = ag_std_pair_second(p1);
    ag_std_print(first);
    printf("\n");
    ag_std_print(second);
    printf("\n");
  }

  ag_std_list_push_back(lst, i);
  ag_std_list_push_back(lst, i2);
  ag_std_list_push_back(lst, s1);
  ag_std_list_push_back(lst, i3);
  ag_std_list_push_back(lst, i4);
  ag_std_list_push_back(lst, ag_std_new(floating, 5.3));

  {
    printf("Printing this with a ag_std_list iterator: ");
    printf("{ ");
    void *it = ag_std_list_begin(lst);
    void *lst_end = ag_std_list_end(lst);
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

    void *lst_a = ag_std_new(ag_std_list);
    ag_std_list_push_back(lst_a, ai);
    ag_std_list_push_back(lst_a, as);

    void *lst_b = ag_std_new(ag_std_list);
    ag_std_list_push_back(lst_b, bi);
    ag_std_list_push_back(lst_b, bs);

    void *lst_c = ag_std_new(ag_std_list);
    ag_std_list_push_back(lst_c, ci);
    ag_std_list_push_back(lst_c, cs);

    void *lst_final = ag_std_new(ag_std_list);
    ag_std_list_push_back(lst_final, lst_a);

    ag_std_print(lst_final);
    printf("\n");

    ag_std_list_push_back(lst_final, lst_b);
    ag_std_list_push_back(lst_final, lst_c);

    ag_std_print(lst_final);
    printf("\n");
  }

  {
    void *ai = ag_std_new(integer, 10);
    void *bi = ag_std_new(integer, 20);
    void *ci = ag_std_new(integer, 30);

    void *v = ag_std_new(vector);
    ag_std_print(v);
    printf("\n");

    ag_std_vector_push_back(v, ai);
    ag_std_vector_push_back(v, bi);
    ag_std_vector_push_back(v, ci);
    ag_std_vector_push_back(v, ci);
    ag_std_vector_pop_back(v);

    ag_std_print(v);
    printf("\n");

    {
      printf("Printing this with a vec iterator: ");
      printf("{ ");
      void *it = ag_std_vector_begin(v);
      void *vec_end = ag_std_vector_end(v);

      while (ag_std_iter_not_equal(it, vec_end)) {
        ag_std_print(ag_std_iter_deref(it));
        printf(" ");
        ag_std_iter_increment(it);
      }

      printf("}\n");

      ag_std_range_print(v);

      int found = ag_std_range_find(v, bi);
      assert(found == 1);

      void *z = ag_std_new(integer, 0);

      found = ag_std_range_find(v, z);
      assert(found == 0);
    }
  }

  void *class_of_i = ag_std_class_of(i);
  assert(class_of_i == integer);

  {
    printf("Iota view test: ");
    void *v_10 = ag_std_new(ag_std_iota_view, 10);

    void *it = ag_std_begin(v_10);
    void *end = ag_std_end(v_10);

    while (ag_std_iter_not_equal(it, end)) {
      void *x = ag_std_iter_deref(it);
      ag_std_print(x);
      printf(" ");
      ag_std_iter_increment(it);
    }

    printf("\n");
  }

  {
    printf("Zip view test: ");

    void *v = ag_std_new(vector);

    ag_std_vector_push_back(v, ag_std_new(string, "Harry"));
    ag_std_vector_push_back(v, ag_std_new(string, "Ron"));
    ag_std_vector_push_back(v, ag_std_new(string, "Hermione"));

    void *iv = ag_std_new(ag_std_iota_view, 2);

    void *zv = ag_std_new(ag_std_zip_view, v, iv);

    void *it = ag_std_begin(zv);
    void *end = ag_std_end(zv);

    while (ag_std_iter_not_equal(it, end)) {
      // The pair.
      void *p = ag_std_iter_deref(it);
      ag_std_print(p);
      printf(" ");
      ag_std_iter_increment(it);
    }

    printf("\n");
  }

  {
    printf("Zip view test v2: ");

    void *v = ag_std_new(vector);

    ag_std_vector_push_back(v, ag_std_new(string, "Harry"));
    ag_std_vector_push_back(v, ag_std_new(string, "Ron"));
    ag_std_vector_push_back(v, ag_std_new(string, "Hermione"));

    void *iv = ag_std_new(ag_std_iota_view, 2);

    void *zv = ag_std_new(ag_std_zip_view, v, iv);

    // void *it = ag_std_begin(zv);
    void *end = ag_std_end(zv);

    for (
        void *it = ag_std_begin(zv);
        ag_std_iter_not_equal(it, end);
        ag_std_iter_increment(it))
    {
      // The pair.
      void *p = ag_std_iter_deref(it);
      ag_std_print(p);
      printf(" ");
    }

    printf("\n");
  }

  {
    printf("Zip view test v3: ");

    void *v = ag_std_new(vector);

    ag_std_vector_push_back(v, ag_std_new(string, "Harry"));
    ag_std_vector_push_back(v, ag_std_new(string, "Ron"));
    ag_std_vector_push_back(v, ag_std_new(string, "Hermione"));

    void *iv = ag_std_new(ag_std_iota_view, 2);

    void *zv = ag_std_new(ag_std_zip_view, v, iv);

    ag_std_range_print(zv);
  }

  {
    printf("Map test.. (using asserts)\n");

    void *m = ag_std_new(ag_std_map);

    void *k1 = ag_std_new(integer, 3);
    void *v1 = ag_std_new(string, "three");
    void *p1 = ag_std_new(ag_std_pair, k1, v1);

    ag_std_map_insert(m, p1);

    void *k2 = ag_std_new(integer, 4);
    void *v2 = ag_std_new(string, "four");
    void *p2 = ag_std_new(ag_std_pair, k2, v2);

    ag_std_map_insert(m, p2);

    // This key will not be in the map.
    void *k3 = ag_std_new(integer, 5);

    // Now check the find.
    void *find_01 = ag_std_map_at(m, k1);
    assert(find_01 == v1);

    void *find_02 = ag_std_map_at(m, k2);
    assert(find_02 == v2);

    void *find_03 = ag_std_map_at(m, k3);
    assert(find_03 == NULL);
  }

  return 0;
}
