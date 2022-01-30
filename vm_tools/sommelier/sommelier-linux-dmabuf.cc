// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sommelier.h"  // NOLINT(build/include_directory)

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "linux-dmabuf-unstable-v1-client-protocol.h"  // NOLINT(build/include_directory)
#include "linux-dmabuf-unstable-v1-server-protocol.h"  // NOLINT(build/include_directory)

struct sl_host_linux_dmabuf {
  struct sl_linux_dmabuf* linux_dmabuf;
  uint32_t version;
  struct wl_resource* resource;
  struct zwp_linux_dmabuf_v1* proxy;
};

struct sl_host_linux_buffer_params {
  struct wl_resource* resource;
  struct zwp_linux_buffer_params_v1* proxy;
};

static void sl_linux_buffer_params_destroy(struct wl_client* client,
                                           struct wl_resource* resource) {
  wl_resource_destroy(resource);
}

static void sl_linux_buffer_params_add(struct wl_client* client,
                                       struct wl_resource* resource,
                                       int32_t fd,
                                       uint32_t plane_idx,
                                       uint32_t offset,
                                       uint32_t stride,
                                       uint32_t modifier_hi,
                                       uint32_t modifier_lo) {
  struct sl_host_linux_buffer_params* host =
      static_cast<sl_host_linux_buffer_params*>(
          wl_resource_get_user_data(resource));

  zwp_linux_buffer_params_v1_add(host->proxy, fd, plane_idx, offset, stride,
                                 modifier_hi, modifier_lo);
  close(fd);
}

static void sl_linux_buffer_params_create(struct wl_client* client,
                                          struct wl_resource* resource,
                                          int32_t width,
                                          int32_t height,
                                          uint32_t format,
                                          uint32_t flags) {
  // create is not supported, only create_immed.
  assert(0);
}

static void sl_linux_buffer_params_create_immed(struct wl_client* client,
                                                struct wl_resource* resource,
                                                uint32_t buffer_id,
                                                int32_t width,
                                                int32_t height,
                                                uint32_t format,
                                                uint32_t flags) {
  struct sl_host_linux_buffer_params* host =
      static_cast<sl_host_linux_buffer_params*>(
          wl_resource_get_user_data(resource));

  sl_create_host_buffer(client, buffer_id,
                        zwp_linux_buffer_params_v1_create_immed(
                            host->proxy, width, height, format, flags),
                        width, height);
}

static const struct zwp_linux_buffer_params_v1_interface
    sl_linux_buffer_params_implementation = {
        sl_linux_buffer_params_destroy, sl_linux_buffer_params_add,
        sl_linux_buffer_params_create, sl_linux_buffer_params_create_immed};

static void sl_destroy_host_linux_buffer_params(struct wl_resource* resource) {
  struct sl_host_linux_buffer_params* host =
      static_cast<sl_host_linux_buffer_params*>(
          wl_resource_get_user_data(resource));

  wl_resource_set_user_data(resource, NULL);
  zwp_linux_buffer_params_v1_destroy(host->proxy);
  free(host);
}

static void sl_linux_buffer_params_created(
    void* data,
    struct zwp_linux_buffer_params_v1* params,
    struct wl_buffer* buffer) {
  assert(0);
}

static void sl_linux_buffer_params_failed(
    void* data, struct zwp_linux_buffer_params_v1* params) {
  assert(0);
}

static const struct zwp_linux_buffer_params_v1_listener
    sl_linux_buffer_params_listener = {sl_linux_buffer_params_created,
                                       sl_linux_buffer_params_failed};

static void sl_linux_dmabuf_destroy(struct wl_client* client,
                                    struct wl_resource* resource) {
  wl_resource_destroy(resource);
}

static void sl_linux_dmabuf_create_params(struct wl_client* client,
                                          struct wl_resource* resource,
                                          uint32_t id) {
  struct sl_host_linux_dmabuf* host =
      static_cast<sl_host_linux_dmabuf*>(wl_resource_get_user_data(resource));
  struct sl_host_linux_buffer_params* host_linux_buffer_params =
      static_cast<sl_host_linux_buffer_params*>(
          malloc(sizeof(*host_linux_buffer_params)));
  assert(host_linux_buffer_params);

  host_linux_buffer_params->resource = wl_resource_create(
      client, &zwp_linux_buffer_params_v1_interface, host->version, id);
  host_linux_buffer_params->proxy =
      zwp_linux_dmabuf_v1_create_params(host->proxy);
  wl_resource_set_implementation(host_linux_buffer_params->resource,
                                 &sl_linux_buffer_params_implementation,
                                 host_linux_buffer_params,
                                 sl_destroy_host_linux_buffer_params);

  zwp_linux_buffer_params_v1_set_user_data(host_linux_buffer_params->proxy,
                                           host_linux_buffer_params);
  zwp_linux_buffer_params_v1_add_listener(host_linux_buffer_params->proxy,
                                          &sl_linux_buffer_params_listener,
                                          host_linux_buffer_params);
}

static const struct zwp_linux_dmabuf_v1_interface
    sl_linux_dmabuf_implementation = {sl_linux_dmabuf_destroy,
                                      sl_linux_dmabuf_create_params};

static void sl_destroy_host_linux_dmabuf(struct wl_resource* resource) {
  struct sl_host_linux_dmabuf* host =
      static_cast<sl_host_linux_dmabuf*>(wl_resource_get_user_data(resource));

  zwp_linux_dmabuf_v1_destroy(host->proxy);
  wl_resource_set_user_data(resource, NULL);
  free(host);
}

static void sl_linux_dmabuf_format(void* data,
                                   struct zwp_linux_dmabuf_v1* linux_dmabuf,
                                   uint32_t format) {
  struct sl_host_linux_dmabuf* host = static_cast<sl_host_linux_dmabuf*>(
      zwp_linux_dmabuf_v1_get_user_data(linux_dmabuf));

  zwp_linux_dmabuf_v1_send_format(host->resource, format);
}

static void sl_linux_dmabuf_modifier(void* data,
                                     struct zwp_linux_dmabuf_v1* linux_dmabuf,
                                     uint32_t format,
                                     uint32_t modifier_hi,
                                     uint32_t modifier_lo) {
  struct sl_host_linux_dmabuf* host = static_cast<sl_host_linux_dmabuf*>(
      zwp_linux_dmabuf_v1_get_user_data(linux_dmabuf));

  zwp_linux_dmabuf_v1_send_modifier(host->resource, format, modifier_hi,
                                    modifier_lo);
}

static const struct zwp_linux_dmabuf_v1_listener sl_linux_dmabuf_listener = {
    sl_linux_dmabuf_format, sl_linux_dmabuf_modifier};

static void sl_bind_host_linux_dmabuf(struct wl_client* client,
                                      void* data,
                                      uint32_t version,
                                      uint32_t id) {
  struct sl_context* ctx = (struct sl_context*)data;
  struct sl_host_linux_dmabuf* host =
      static_cast<sl_host_linux_dmabuf*>(malloc(sizeof(*host)));
  assert(host);
  host->linux_dmabuf = ctx->linux_dmabuf;
  host->version = version;
  host->resource = wl_resource_create(client, &zwp_linux_dmabuf_v1_interface,
                                      host->version, id);
  wl_resource_set_implementation(host->resource,
                                 &sl_linux_dmabuf_implementation, host,
                                 sl_destroy_host_linux_dmabuf);

  host->proxy = static_cast<zwp_linux_dmabuf_v1*>(wl_registry_bind(
      wl_display_get_registry(ctx->display), ctx->linux_dmabuf->id,
      &zwp_linux_dmabuf_v1_interface, version));
  zwp_linux_dmabuf_v1_set_user_data(host->proxy, host);
  zwp_linux_dmabuf_v1_add_listener(host->proxy, &sl_linux_dmabuf_listener,
                                   host);
}

struct sl_global* sl_linux_dmabuf_global_create(struct sl_context* ctx) {
  return sl_global_create(ctx, &zwp_linux_dmabuf_v1_interface,
                          ctx->linux_dmabuf->version, ctx,
                          sl_bind_host_linux_dmabuf);
}
