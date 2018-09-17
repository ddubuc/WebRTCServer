//@HIPE_LICENSE@
#pragma once

#if defined _WIN32 || defined __CYGWIN__
#ifdef WEBRTCSERVER_BUILD
#ifdef __GNUC__
#define WEBRTCSERVER_EXPORT __attribute__ ((dllexport))
#else
#define WEBRTCSERVER_EXPORT __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
#define WEBRTCSERVER_EXTERN extern
#endif
#else
#ifdef __GNUC__
#define WEBRTCSERVER_EXPORT __attribute__ ((dllimport))
#define WEBRTCSERVER_EXTERN extern
#else
#define WEBRTCSERVER_EXPORT __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
#define WEBRTCSERVER_EXTERN 
#endif
#endif
#define WEBRTCSERVER_LOCAL
#else
#if __GNUC__ >= 4
#define WEBRTCSERVER_EXPORT __attribute__ ((visibility ("default")))
#define WEBRTCSERVER_LOCAL  __attribute__ ((visibility ("hidden")))
#define WEBRTCSERVER_EXTERN extern
#else
#define WEBRTCSERVER_EXPORT
#define WEBRTCSERVER_LOCAL
#define WEBRTCSERVER_EXTERN 
#endif
#endif


WEBRTCSERVER_EXPORT int entryPoint();