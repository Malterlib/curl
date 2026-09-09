/* Copyright (C) Unbroken AB
 * SPDX-License-Identifier: curl */
#include "curl_setup.h"

#ifdef CURLRES_EXTERNAL
#include "urldata.h"
#include "multihandle.h"
#include "vdns/asyn.h"
#include "vdns/hostip.h"
#include "vdns/dnscache.h"
#include "multiif.h"
#include "curl_addrinfo.h"
#include "curlx/inet_ntop.h"

void curl_external_resolver_ready(CURL *easy)
{
  struct Curl_eapi_guard guard;

  if(CURL_EAPI_ENTER(&guard, easy, external_resolver_ready, NULL)) {
    struct Curl_easy *data = easy;
    if(data->multi) {
      Curl_multi_mark_dirty(data);
      Curl_expire(data, 0, EXPIRE_ASYNC_NAME);
      (void)Curl_update_timer(data->multi);
    }
  }
  CURL_EAPI_LEAVE(&guard);
}

void Curl_async_external_destroy(struct Curl_resolv_async *async)
{
  if(async->external) {
    async->external_cancel(async->external);
    async->external = NULL;
  }
}

CURLcode Curl_async_external_getaddrinfo(struct Curl_easy *data,
                                       struct Curl_resolv_async *async)
{
  struct curl_external_resolver *resolver = &data->multi->external_resolver;
  uint8_t queries = async->dns_queries & (CURL_DNSQ_A | CURL_DNSQ_AAAA);
  int ip_version;
  CURLcode result;

  if(!queries)
    return CURLE_COULDNT_RESOLVE_HOST; /* The host currently provides A/AAAA. */
  result = Curl_resolv_announce_start(data, NULL);
  if(result)
    return result;

  ip_version = queries == CURL_DNSQ_A ? CURL_IPRESOLVE_V4 :
    queries == CURL_DNSQ_AAAA ? CURL_IPRESOLVE_V6 : CURL_IPRESOLVE_WHATEVER;
  async->external_poll = resolver->poll;
  async->external_cancel = resolver->cancel;
  async->external = resolver->start(resolver->user, data, async->peer->hostname,
                                    ip_version);
  if(!async->external)
    return CURLE_COULDNT_RESOLVE_HOST;
  async->queries_ongoing = 1;
  return CURLE_AGAIN;
}

CURLcode Curl_async_external_take_result(struct Curl_easy *data,
                                       struct Curl_resolv_async *async,
                                       struct Curl_dns_entry **pdns)
{
  const struct curl_external_address *addresses;
  struct Curl_addrinfo *head = NULL, **tail = &head;
  size_t count, i;
  int status;

  *pdns = NULL;
  if(!async->external)
    return CURLE_COULDNT_RESOLVE_HOST;
  status = async->external_poll(async->external, &addresses, &count);
  if(!status)
    return CURLE_AGAIN;

  async->done = TRUE;
  async->queries_ongoing = 0;
  async->dns_responses = async->dns_queries;
  if(status < 0)
    return CURLE_COULDNT_RESOLVE_HOST;

  for(i = 0; i < count; ++i) {
    char address[46];
    struct Curl_addrinfo *ai = NULL;
    CURLcode result;
    int family = addresses[i].ipv6 ? AF_INET6 : AF_INET;

    result = curlx_inet_ntop(family, addresses[i].ip, address, sizeof(address));
    if(result) {
      Curl_freeaddrinfo(head);
      return result;
    }
    result = Curl_str2addr(address, async->peer->port, &ai);
    if(result) {
      Curl_freeaddrinfo(head);
      return result;
    }
#ifdef USE_IPV6
    if(addresses[i].ipv6)
      ((struct sockaddr_in6 *)ai->ai_addr)->sin6_scope_id = addresses[i].scope_id;
#endif
    *tail = ai;
    while(*tail)
      tail = &(*tail)->ai_next;
  }
  if(!head)
    return CURLE_COULDNT_RESOLVE_HOST;

  *pdns = Curl_dnsc_mk_addr(data, async->dns_queries, &head, async->peer);
  if(head)
    Curl_freeaddrinfo(head);
  if(!*pdns)
    return CURLE_OUT_OF_MEMORY;
  Curl_async_external_destroy(async);
  return CURLE_OK;
}

#endif
