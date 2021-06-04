#ifndef __XBOXAPI_H_
#define __XBOXAPI_H_

#if defined(XBOX_SHARED_EXPORTS)
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define XBOX_DLL __declspec(dllexport)
    #define XBOX_DLLVAR extern __declspec(dllexport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define XBOX_DLL __attribute__ ((visibility("default")))
    #define XBOX_DLLVAR extern __attribute__ ((visibility("default")))
  #endif
#else
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define XBOX_DLL __declspec(dllimport)
    #define XBOX_DLLVAR __declspec(dllimport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define XBOX_DLL __attribute__ ((visibility("default")))
    #define XBOX_DLLVAR extern __attribute__ ((visibility("default")))
  #endif
#endif

#ifndef XBOX_DLL
  #define XBOX_DLL
  #define XBOX_DLLVAR extern
#endif 

#endif // __XBOXAPI_H_ 
