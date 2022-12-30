#include "ag_std/obj.h"
#include "ag_std/memory.h"

int main(void) {

  void *obj = ag_std_new(ag_std_obj);
  ag_std_delete(obj);
}
