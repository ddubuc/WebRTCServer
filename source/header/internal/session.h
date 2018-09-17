#ifndef OMEKASHI_SESSION_H
#define OMEKASHI_SESSION_H

#include "renderer.h"

struct Session {
  RenderMode mode;
  int id;
  std::string name;
};

#endif // OMEKASHI_SESSION_H
