// Copyright 2024 Google Inc. All Rights Reserved.

#ifndef WEBSERVER_WEBSERVD_DUMMY_FIREWALL_H_
#define WEBSERVER_WEBSERVD_DUMMY_FIREWALL_H_

#include "webservd/firewall_interface.h"

#include <string>

namespace webservd {

class DummyFirewall : public FirewallInterface {
 public:
  DummyFirewall();
  ~DummyFirewall() override;

  // Interface overrides.
  void WaitForServiceAsync(scoped_refptr<dbus::Bus> bus,
                           const base::Closure& callback) override;
  void PunchTcpHoleAsync(
      uint16_t port, const std::string& interface_name,
      const base::Callback<void(bool)>& success_cb,
      const base::Callback<void(brillo::Error*)>& failure_cb) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(DummyFirewall);
};

}  // namespace webservd

#endif  // WEBSERVER_WEBSERVD_DUMMY_FIREWALL_H_
