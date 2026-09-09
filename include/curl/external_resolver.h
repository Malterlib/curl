#ifndef CURL_EXTERNAL_RESOLVER_H
#define CURL_EXTERNAL_RESOLVER_H

/* Copyright (C) Unbroken AB
 * SPDX-License-Identifier: curl
 * Optional per-multi asynchronous resolver override. Multi handles without an
 * override, including those created by curl_easy_perform, use the normal
 * compiled-in resolver. Install the override before adding easy handles.
 * All callbacks and ready calls
 * are serialized with the multi handle. cancel releases the lookup immediately;
 * background work must own its state independently and never access CURL. */

#include "curl.h"
#include "multi.h"

#ifdef __cplusplus
extern "C" {
#endif

struct curl_external_address {
  unsigned char ip[16];
  unsigned int scope_id;
  int ipv6;
};

struct curl_external_resolver {
  void *user;
  void *(*start)(void *user, CURL *easy, const char *host, int ip_version);
  /* 0 pending, 1 complete, -1 failed. Addresses live until cancel. */
  int (*poll)(void *lookup, const struct curl_external_address **addresses,
              size_t *count);
  void (*cancel)(void *lookup);
};

CURL_EXTERN CURLMcode
curl_multi_set_external_resolver(CURLM *multi,
                                const struct curl_external_resolver *resolver);
CURL_EXTERN void curl_external_resolver_ready(CURL *easy);

#ifdef __cplusplus
}
#endif
#endif
