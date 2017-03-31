#include "filter.h"

bool
pass_one (void *filter)
{
  bool (*fn) ();
  fn = (bool (*)())dlsym (filter, "filter_passes");

  if (!fn)
      prerror("Couldn't pass filter.", "");

  return fn ();
}