---
c: Copyright (C) Unbroken AB
SPDX-License-Identifier: curl
Title: curl_external_resolver_ready
Section: 3
Source: libcurl
See-also:
  - curl_multi_set_external_resolver (3)
  - curl_multi_socket_action (3)
Protocol:
  - All
Added-in: 8.22.0
---

# NAME

curl_external_resolver_ready - notify libcurl that an external lookup changed

# SYNOPSIS

~~~c
#include <curl/external_resolver.h>

void curl_external_resolver_ready(CURL *easy);
~~~

# DESCRIPTION

This Malterlib extension requests immediate processing of *easy* after an
external resolver lookup changes state. It requires external resolver support.
The call marks the transfer ready and updates its multi handle's timer so the
application can continue driving the transfer normally.

Serialize this call with the multi handle's other operations. The easy handle
must remain valid; do not call after the resolver's **cancel** callback has
released the lookup. A valid easy handle without a multi handle is ignored.

# RETURN VALUE

None.
