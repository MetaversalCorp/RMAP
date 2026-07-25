// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#include <RMAP/RMAP.h>
#include <RMAP_Svc_Rest/RMAP_Svc_Rest.h>

#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <curl/curl.h>

#include "LibraryMVRest.h"

#endif //PCH_H

#define STRINGIFY(x)                                       #x
#define TO_STRING(x)                                       STRINGIFY(x)

#define MVO_SIZE_GUID                                     40
#define SBD_SIZE_OBJECTID                                 31
#define SBD_SIZE_TOKEN64U                                 64

#define MVO_SIZE_RDCOMPANYID                               SBD_SIZE_OBJECTID
#define MVO_SIZE_RDSERVICEID                               SBD_SIZE_OBJECTID
#define MVO_SIZE_RDACTIVITYID                              SBD_SIZE_OBJECTID
#define MVO_SIZE_RDENVIRONMENTID                           SBD_SIZE_OBJECTID

#define MVO_SIZE_RDCOMPANY_NAME_ENGLISH                   64
#define MVO_SIZE_RDCOMPANY_ADDRESS_STREET_1               32
#define MVO_SIZE_RDCOMPANY_ADDRESS_STREET_2               32
#define MVO_SIZE_RDCOMPANY_ADDRESS_CITY                   24
#define MVO_SIZE_RDCOMPANY_ADDRESS_STATE                  24
#define MVO_SIZE_RDCOMPANY_ADDRESS_COUNTRY                32
#define MVO_SIZE_RDCOMPANY_ADDRESS_POSTAL                 16
#define MVO_SIZE_RDCOMPANY_ADDRESS_PHONE                  16
#define MVO_SIZE_RDCOMPANY_ENTITY_TIN                     16
#define MVO_SIZE_RDCOMPANY_ENTITY_NAME                    64
#define MVO_SIZE_RDCOMPANY_ADMIN_PRE                       4
#define MVO_SIZE_RDCOMPANY_ADMIN_FIRST                    16
#define MVO_SIZE_RDCOMPANY_ADMIN_MIDDLE                   16
#define MVO_SIZE_RDCOMPANY_ADMIN_LAST                     16
#define MVO_SIZE_RDCOMPANY_ADMIN_POST                      4
#define MVO_SIZE_RDCOMPANY_ADMIN_TITLE                    32

#define MVO_SIZE_RDSERVICE_NAME_ENGLISH                   64

#define MVO_SIZE_RDENVIRONMENT_SERVICEID                  32
#define MVO_SIZE_RDENVIRONMENT_CONNECT                   128
#define MVO_SIZE_RDENVIRONMENT_PASSWORD                   64

#define MVO_SIZE_RDACTIVITY_NAME_ENGLISH                  64

#define SBO_CLASS_LEGACY_ROOT                              0
#define SBO_CLASS_USER                                     5
#define SBO_CLASS_RROOT                                   79
#define SBO_CLASS_RUSER                                   75
#define SBO_CLASS_RPERSONA                                76
#define SBO_CLASS_RDUSER                                  89
#define SBO_CLASS_RDUSER_RDCOMPANY                        90
#define SBO_CLASS_RDCOMPANY                               91
#define SBO_CLASS_RDCOMPANY_RPERSONA                      92
#define SBO_CLASS_RDSERVICE                               93
#define SBO_CLASS_RDENVIRONMENT                           94
#define SBO_CLASS_RDENVIRONMENT_IPADDRESS                 95
#define SBO_CLASS_RDENVIRONMENT_SOURCE                    96
#define SBO_CLASS_RDENVIRONMENT_SESSION                   97
#define SBO_CLASS_RDACTIVITY                              98

#define MAKEACTION(n,o)                                   (n | ((o) << 8))

#define SBA_NULL                                           MAKEACTION ( 0, SBO_CLASS_LEGACY_ROOT)
#define SBA_LOGGEDOUT                                      MAKEACTION ( 7, SBO_CLASS_LEGACY_ROOT)

#define SBA_RROOT_RDCOMPANY_OPEN                           MAKEACTION ( 6, SBO_CLASS_RROOT)
#define SBA_RROOT_RDCOMPANY_CLOSE                          MAKEACTION ( 7, SBO_CLASS_RROOT)

#define SBA_RUSER_RDUSER_OPEN                              MAKEACTION ( 8, SBO_CLASS_RUSER)
#define SBA_RUSER_RDUSER_CLOSE                             MAKEACTION ( 9, SBO_CLASS_RUSER)

#define SBA_RPERSONA_TOKEN                                 MAKEACTION ( 9, SBO_CLASS_RPERSONA)

#define SBA_RDCOMPANY_NAME                                 MAKEACTION ( 0, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_ENTITY                               MAKEACTION ( 1, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_ADDRESS                              MAKEACTION ( 2, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_ADMIN                                MAKEACTION ( 3, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_RPERSONA_INVITE                      MAKEACTION ( 4, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_RPERSONA_ACCEPT                      MAKEACTION ( 5, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_RPERSONA_REJECT                      MAKEACTION ( 6, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_RPERSONA_BLOCK                       MAKEACTION ( 7, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_RPERSONA_UNBLOCK                     MAKEACTION ( 8, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_RPERSONA_RIGHTS                      MAKEACTION ( 9, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_RPERSONA_DISABLE                     MAKEACTION (10, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_RPERSONA_ENABLE                      MAKEACTION (11, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_RPERSONA_DELETE                      MAKEACTION (12, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_RPERSONA_OWNER                       MAKEACTION (13, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_RDSERVICE_OPEN                       MAKEACTION (14, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_RDSERVICE_CLOSE                      MAKEACTION (15, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_RDACTIVITY_OPEN                      MAKEACTION (16, SBO_CLASS_RDCOMPANY)
#define SBA_RDCOMPANY_RDACTIVITY_CLOSE                     MAKEACTION (17, SBO_CLASS_RDCOMPANY)

#define SBA_RDSERVICE_PAUSE                                MAKEACTION ( 0, SBO_CLASS_RDSERVICE)
#define SBA_RDSERVICE_CONTINUE                             MAKEACTION ( 1, SBO_CLASS_RDSERVICE)
#define SBA_RDSERVICE_NAME                                 MAKEACTION ( 2, SBO_CLASS_RDSERVICE)
#define SBA_RDSERVICE_RDENVIRONMENT_OPEN                   MAKEACTION ( 4, SBO_CLASS_RDSERVICE)
#define SBA_RDSERVICE_RDENVIRONMENT_CLOSE                  MAKEACTION ( 5, SBO_CLASS_RDSERVICE)

#define SBA_RDENVIRONMENT_PAUSE                            MAKEACTION ( 0, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_CONTINUE                         MAKEACTION ( 1, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_PRODUCTION                       MAKEACTION ( 2, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_NAME                             MAKEACTION ( 3, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_CONFIGURE                        MAKEACTION ( 4, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_PASSWORD                         MAKEACTION ( 5, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_IPADDRESS_OPEN                   MAKEACTION ( 6, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_IPADDRESS_CLOSE                  MAKEACTION ( 7, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_IPADDRESS_PAUSE                  MAKEACTION ( 8, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_IPADDRESS_CONTINUE               MAKEACTION ( 9, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_LOGIN                            MAKEACTION (10, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_LOGOUT                           MAKEACTION (11, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_DISCONNECT                       MAKEACTION (12, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_AUTHENTICATE                     MAKEACTION (13, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_RPERSONA_IDENTIFY                MAKEACTION (14, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_RPERSONA_NAME                    MAKEACTION (15, SBO_CLASS_RDENVIRONMENT)
#define SBA_RDENVIRONMENT_RPERSONA_LOCATION                MAKEACTION (16, SBO_CLASS_RDENVIRONMENT)

#define SBA_RDACTIVITY_NAME                                MAKEACTION ( 0, SBO_CLASS_RDACTIVITY)

#define SBA_RESULT_SUCCESS                                 0
#define SBA_RESULT_TRANSMITFAILURE                         0xFC18 // (WORD)(-1000)
