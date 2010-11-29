/*
 * Config service interface.
 *
 * Copyright (C) 2010, Alexander Boettcher <boettcher@tudos.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * This file is part of NUL (NOVA user land).
 *
 * NUL is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * NUL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 */

#include "nul/motherboard.h"
#include "nul/generic_service.h"
#include "nul/service_config.h"
#include "nul/service_fs.h"
#include "nul/region.h"

class Service_config {
  Motherboard &mb;

public:
  Service_config(Motherboard &_mb, bool readonly = true ) : mb(_mb) {}

  unsigned portal_func(Utcb &utcb, Utcb::Frame &input, bool &free_cap) {
    unsigned op;

    check1(EPROTO, input.get_word(op));

    switch (op) {
    case ParentProtocol::TYPE_OPEN:
    case ParentProtocol::TYPE_CLOSE:
      return ENONE;
    case ConfigProtocol::TYPE_START_CONFIG:
      {
        unsigned len;
        char *_config = input.get_zero_string(len);
        check1(EPROTO, !_config);

        char *config = new char[len];
        if (!config) return ERESOURCE;
        memcpy(config, _config, len);

        MessageConsole msg(MessageConsole::TYPE_START, ~0);
        msg.cmdline = config;
        if (mb.bus_console.send(msg))
          return ENONE;
        else {
          delete [] config;
          return EPROTO;
        }
      }
    default:
      return EPROTO;
    }
  }

};

PARAM(service_config,
      Service_config *service_config = new Service_config(mb);
      MessageHostOp msg(MessageHostOp::OP_REGISTER_SERVICE, reinterpret_cast<unsigned long>(StaticPortalFunc<Service_config>::portal_func), reinterpret_cast<unsigned long>(service_config));
      msg.ptr = const_cast<char *>("/config");
      if (!mb.bus_hostop.send(msg))
        Logging::panic("registering the service failed");
      Logging::printf("start service - config\n");
      ,
      "service_config - managing a config, e.g. start, stop");

