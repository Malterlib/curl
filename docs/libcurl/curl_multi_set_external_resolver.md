---
c: Copyright (C) Unbroken AB
SPDX-License-Identifier: curl
Title: curl_multi_set_external_resolver
Section: 3
Source: libcurl
See-also:
  - curl_external_resolver_ready (3)
  - curl_multi_add_handle (3)
Protocol:
  - All
Added-in: 8.22.0
---

# NAME

curl_multi_set_external_resolver - install a per-multi asynchronous resolver

# SYNOPSIS

~~~c
#include <curl/external_resolver.h>

CURLMcode curl_multi_set_external_resolver(
  CURLM *multi, const struct curl_external_resolver *resolver);
~~~

# DESCRIPTION

This Malterlib extension selects a resolver for *multi*. It requires a build
with external resolver support. Install it before adding easy handles, or pass
NULL after removing all easy handles to restore the compiled-in resolver.
Multi handles without an override keep their normal resolver.

The callback structure is copied. Its **start** callback returns an opaque
lookup handle, or NULL on failure. **poll** returns 0 while pending, 1 with an
address array on completion, or -1 on failure. Addresses remain valid until
**cancel**, which releases the lookup. Background work must own its state
independently and must not access the easy handle after cancellation.

Serialize callbacks and curl_external_resolver_ready(3) calls with operations
on the multi handle. External lookups contribute no descriptors to libcurl's
poll set. Notify libcurl when lookup state changes using
curl_external_resolver_ready(3). Blocking lookups, including interface hostnames
and active FTP, must complete synchronously or fail with a resolve error.

# RETURN VALUE

Returns CURLM_OK on success. Invalid callback pointers or attached easy handles
produce CURLM_BAD_FUNCTION_ARGUMENT. An invalid multi handle produces
CURLM_BAD_HANDLE.
