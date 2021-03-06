#include "stack.h"

bool
push (const char *filter_name, struct TStack * s)
{
  char fn[MAX_LINE];
  void *mod;
  char *exe_dir = get_self_executable_dir();

  sprintf (fn, "%s/modules/%s.so", exe_dir, filter_name);
#ifndef NDEBUG
  fprintf(stderr, "Loading %s\n", fn);
#endif

  if (s->summit >= MAX_FILTER)
    return false;

  mod = dlopen (fn, RTLD_NOW);
  if (!mod)
    prerror ("Could not open file %s.\n%s\n", filter_name, dlerror ());

  free(exe_dir);

  s->data[s->summit++] = mod;

  return true;

}

bool
pop (struct TStack * s)
{
  if (s->summit <= 0)
    return false;

  s->summit--;
  dlclose (s->data[s->summit]);

  return true;
}

void
clear (struct TStack *s)
{
  while (pop (s));
}

void *
each (void *(*fn) (void *el, void *return_value), struct TStack *s)
{
  void *return_value = NULL;

  for (int i = 0; i < s->summit; i++)
    return_value = fn (s->data[i], return_value);

  return return_value;
}

bool
pass_all (struct TStack * s)
{
  bool pass = true;

  for (int i = 0; pass && i < s->summit; i++)
    pass = pass && pass_one (s->data[i]);

  return pass;
}

bool
pass_any (struct TStack * s)
{
  bool pass = false;

  for (int i = 0; !pass && i < s->summit; i++)
    pass = pass || pass_one (s->data[i]);

  return pass;
}
