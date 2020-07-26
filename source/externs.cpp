//
// Created by askar on 26/07/2020.
//

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#include <cstdio>

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" DLLEXPORT double writeln(double X) {
  printf("%f\n", X);
  return 0;
}
