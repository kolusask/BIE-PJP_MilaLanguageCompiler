//
// Created by askar on 26/07/2020.
//

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#include <cstdio>
#include <cstdint>

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" DLLEXPORT int writeln(int32_t X) {
  printf("%d\n", X);
  return 0;
}
