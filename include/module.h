#ifndef MODULE_H_

struct Public {
  int a;
};

#ifdef MODULE_IMPLEMENTATION

static void private(void)
{
}

#endif

#endif // !MODULE_H_
