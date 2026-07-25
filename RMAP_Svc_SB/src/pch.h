// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here

#include <RMAP/RMAP.h>
#include <RMAP_Svc_SB/RMAP_Svc_SB.h>

#include "Library.h"

#include <mutex>
#include <chrono>

#define STRINGIFY(x)                                       #x
#define TO_STRING(x)                                       STRINGIFY(x)

#define SBA_CLASS_MASK                                     0xFFFF0000
#define SBA_CLASS_SHIFT                                   16
#define SBA_CLASS(oc)                                      ((uint16_t)((oc) >>  SBA_CLASS_SHIFT))

#define MAKEACTION(n,o)                                   (n | ((o) << 16))

#define MV_CLIENT_SUBSCRIPTION_COUNT                      64

#define SBA_NULL                                           MAKEACTION ( 1, SBO_CLASS_STATE)
#define SBA_VERSION                                        MAKEACTION ( 2, SBO_CLASS_STATE)
#define SBA_CLIENT_CONNECT                                 MAKEACTION ( 3, SBO_CLASS_STATE)
#define SBA_CLIENT_DISCONNECT                              MAKEACTION ( 4, SBO_CLASS_STATE)
#define SBA_SUBSCRIBE                                      MAKEACTION ( 5, SBO_CLASS_STATE)
#define SBA_SUBSCRIBE_REFRESH                              MAKEACTION ( 6, SBO_CLASS_STATE)
#define SBA_SUBSCRIBE_RECOVER                              MAKEACTION ( 7, SBO_CLASS_STATE)
#define SBA_LOGGEDOUT                                      MAKEACTION ( 8, SBO_CLASS_STATE)
#define SBA_RPROXIMITY                                     MAKEACTION ( 9, SBO_CLASS_STATE)

#define SBA_RESULT_SUCCESS                                 0

#define SBA_RESULT_TRANSMITFAILURE                         0xFC18 // (WORD)(-1000)
#define SBA_RESULT_VERSION_SERVERUNAVAILABLE               1

#define MV_SERVICE_OBJECT_SESSION                          0
#define MV_SERVICE_OBJECT_TIME                             1

#define SBD_CLIENTSESSION_NULL                             0

#define TIMEX_SECOND                                       0x00000040 // 0x00000040
#define TIMEX_MINUTE                                       0x00000F00 // 0x00000F00
#define TIMEX_HOUR                                         0x00038400 // 0x00038400
#define TIMEX_DAY                                          0x00546000 // 0x00546000

#define SBA_SUBSCRIBE_STATE_RESET                          0xFFFE // (WORD)(-2)
#define SBA_SUBSCRIBE_STATE_REMOVE                         0xFFFF // (WORD)(-1)
#define SBA_SUBSCRIBE_STATE_ADD                            0
#define SBA_SUBSCRIBE_STATE_RECOVER_SELF                   0
#define SBA_SUBSCRIBE_STATE_COMPLETE                      17

#define SIZEOF__SBA_SUBSCRIBE_REFRESH_IN                  24
#define SIZEOF__SBA_SUBSCRIBE_REFRESH_EVENT               24
#define SIZEOF__SBA_SUBSCRIBE_REFRESH_EVENT_EX            16

#define SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG_OPEN           0x01
#define SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG_CLOSE          0x02
#define SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG_RESET          0x04
#define SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG_ADDENDUM       0x08
#define SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG_PARTIAL        0x10

#define SIZEOF__SBA_SUBSCRIBE_RECOVER_IN                   4
#define SIZEOF__SBA_SUBSCRIBE_RECOVER_BANK_IN             16
#define SIZEOF__SBA_SUBSCRIBE_RECOVER_CHILDREN             8

#define SBA_SUBSCRIBE_RECOVER_BANK_FLAG_OBJECT_INITIAL     0x01
#define SBA_SUBSCRIBE_RECOVER_BANK_FLAG_OBJECT_FINAL       0x02
#define SBA_SUBSCRIBE_RECOVER_BANK_FLAG_BANK_INITIAL       0x04
#define SBA_SUBSCRIBE_RECOVER_BANK_FLAG_BANK_FINAL         0x08

#define SBO_CLASS_NULL                                     0

#define SBD_OBJECT_HEAD_FLAG_SUBSCRIBE_MASK                0x0030
#define SBD_OBJECT_HEAD_FLAG_SUBSCRIBE_PARTIAL             0x0010
#define SBD_OBJECT_HEAD_FLAG_SUBSCRIBE_FULL                0x0020

#define SIZEOF__SBD_OBJECT_HEAD                           24

#define SBD_SIZE_OBJECTID                                 31
#define SBD_SIZE_PASSWORD                                 32
#define SBD_SIZE_HASH                                     24
#define SBD_SIZE_TOKEN                                    48
#define SBD_SIZE_TOKEN64U                                 64
#define SBD_SIZE_EMAIL                                    64

#endif //PCH_H
