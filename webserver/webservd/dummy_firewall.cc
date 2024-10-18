// Copyright 2024 Google Inc. All Rights Reserved.

#include "webservd/dummy_firewall.h"

#include <unistd.h>

#include <string>

#include <base/bind.h>
#include <base/macros.h>

namespace webservd {

DummyFirewall::DummyFirewall() {}

DummyFirewall::~DummyFirewall() {}

void DummyFirewall::WaitForServiceAsync(
    scoped_refptr<dbus::Bus> bus,
    const base::Closure& callback) {
  (void)bus;
  // Return immediately as there isn't a firewall running on CastOS.
  callback.Run();
}

void DummyFirewall::PunchTcpHoleAsync(
    uint16_t port,
    const std::string& interface_name,
    const base::Callback<void(bool)>& success_cb,
    const base::Callback<void(brillo::Error*)>& failure_cb) {
  (void)port;
  (void)interface_name;
  (void)failure_cb;
  // Return success because there's no firewall running, all ports are
  // available.
  success_cb.Run(true);
}

}  // namespace webservd
