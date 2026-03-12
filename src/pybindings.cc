#include "pybindings.h"
#include "memory_system.h"

#include <functional>
#include <new>
#include <string>

namespace {
struct Wrapper {
  dramsim3::MemorySystem *memsys = nullptr;
  dramsim_callback_t read_cb = nullptr;
  dramsim_callback_t write_cb = nullptr;
  Wrapper(const char *config, const char *output, dramsim_callback_t r,
          dramsim_callback_t w)
      : read_cb(r), write_cb(w) {
    std::string s_config = (config ? config : "");
    std::string s_output = (output ? output : "");

    std::function<void(uint64_t)> rfun = [this](uint64_t addr) {
      if (this->read_cb)
        this->read_cb(addr);
    };
    std::function<void(uint64_t)> wfun = [this](uint64_t addr) {
      if (this->write_cb)
        this->write_cb(addr);
    };
    memsys = new dramsim3::MemorySystem(s_config, s_output, rfun, wfun);
  }

  ~Wrapper() { delete memsys; }

  void set_callbacks(dramsim_callback_t rcb, dramsim_callback_t wcb) {
    read_cb = rcb;
    write_cb = wcb;

    std::function<void(uint64_t)> rfun = [this](uint64_t addr) {
      if (this->read_cb)
        this->read_cb(addr);
    };
    std::function<void(uint64_t)> wfun = [this](uint64_t addr) {
      if (this->write_cb)
        this->write_cb(addr);
    };

    memsys->RegisterCallbacks(rfun, wfun);
  }
};
} // namespace

extern "C" {
EXPORT memsys_t memsys_create(const char *config_file, const char *output_dir,
                              dramsim_callback_t read_callback,
                              dramsim_callback_t write_callback) {
  Wrapper *w = new (std::nothrow)
      Wrapper(config_file, output_dir, read_callback, write_callback);
  // early exit if bad setup
  if (!w || !w->memsys) {
    delete w;
    return nullptr;
  }
  return reinterpret_cast<memsys_t>(w);
}

EXPORT void memsys_get_byte_range_from_bank(memsys_t memsys, uint64_t channel,
                                            uint64_t rank, uint64_t bankgroup,
                                            uint64_t bank, uint64_t hex_address,
                                            int64_t *data_idx,
                                            size_t *start_idx) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys) {
    uint64_t addr_start = w->memsys->GetSpatialGlobalAddr(channel, rank, bankgroup, bank, hex_address);
    w->memsys->GetBytes(addr_start, data_idx, start_idx);
  } else {
    (*data_idx) = -1;
    (*start_idx) = 0;
  }
}

EXPORT void memsys_mmap(memsys_t memsys, uint64_t channel, uint64_t rank,
                        uint64_t bankgroup, uint64_t bank, uint64_t hex_address,
                        int64_t data_idx, size_t length, size_t offset) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys) {
    uint64_t addr_start = w->memsys->GetSpatialGlobalAddr(channel, rank, bankgroup, bank, hex_address);
    uint64_t addr_end = addr_start + w->memsys->GetSpatialGlobalAddr(0, 0, 0, 0, length);
    w->memsys->MMap(data_idx, addr_start, addr_end, offset);
  }
}

EXPORT void memsys_munmap(memsys_t memsys, uint64_t channel, uint64_t rank,
                          uint64_t bankgroup, uint64_t bank,
                          size_t base_address, size_t length) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys) {
    uint64_t addr_start = w->memsys->GetSpatialGlobalAddr(channel, rank, bankgroup, bank, base_address);
    uint64_t addr_end = addr_start + w->memsys->GetSpatialGlobalAddr(0, 0, 0, 0, length);
    w->memsys->MUnmap(addr_start, addr_end);
  }
}

EXPORT void memsys_tick(memsys_t memsys) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys) {
    w->memsys->ClockTick();
  }
}

EXPORT uint64_t memsys_get_cycle(memsys_t memsys) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys) {
    return w->memsys->GetClock();
  }
  return 0;
}

EXPORT void memsys_destroy(memsys_t memsys) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  delete w;
}

EXPORT void memsys_print_stats(memsys_t memsys) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys)
    w->memsys->PrintStats();
}

EXPORT void memsys_register_callbacks(memsys_t memsys, dramsim_callback_t rcb,
                                      dramsim_callback_t wcb) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys)
    w->set_callbacks(rcb, wcb);
}

EXPORT bool memsys_add_transaction(memsys_t memsys, uint64_t hex_address,
                                   bool is_write, bool is_pim) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys)
    return w->memsys->AddTransaction(hex_address, is_write, is_pim);
  return false;
}

EXPORT void memsys_toggle_mode(memsys_t memsys) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys)
    w->memsys->SetPimMode(!w->memsys->GetPimMode());
}

EXPORT void memsys_set_pim_mode(memsys_t memsys, bool pim_mode) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys)
    w->memsys->SetPimMode(pim_mode);
}

EXPORT bool memsys_get_pim_mode(memsys_t memsys) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys)
    return w->memsys->GetPimMode();
  return false;
}

EXPORT bool memsys_add_transaction_to_bank(memsys_t memsys, uint64_t channel,
                                           uint64_t rank, uint64_t bankgroup,
                                           uint64_t bank, uint64_t hex_address,
                                           bool is_write, bool is_pim) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys) {
    uint64_t reversed_addr = w->memsys->BankLocalToGlobalAddr(
        channel, rank, bankgroup, bank, hex_address);
    return w->memsys->AddTransaction(reversed_addr, is_write, is_pim);
  }
  return false;
}

EXPORT uint64_t memsys_get_ranks(memsys_t memsys) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys) {
    return w->memsys->GetRanks();
  }
  return 0;
}

EXPORT uint64_t memsys_get_channels(memsys_t memsys) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys) {
    return w->memsys->GetChannels();
  }
  return 0;
}

EXPORT uint64_t memsys_get_banks_per_bankgroup(memsys_t memsys) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys) {
    return w->memsys->GetBanksPerBG();
  }
  return 0;
}

EXPORT uint64_t memsys_get_bankgroups_per_rank(memsys_t memsys) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys) {
    return w->memsys->GetBankgroupsPerRank();
  }
  return 0;
}

EXPORT uint64_t memsys_set_default_callbacks(memsys_t memsys) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys) {
    return w->memsys->GetBanksPerBG();
  }
  return 0;
}

EXPORT uint64_t memsys_get_address_from_physical_location(
    memsys_t memsys, uint64_t channel, uint64_t rank, uint64_t bankgroup,
    uint64_t bank, uint64_t hex_address) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys) {
    return w->memsys->BankLocalToGlobalAddr(channel, rank, bankgroup, bank,
                                       hex_address);
  }
  return 0;
}

EXPORT void memsys_get_physical_location_from_address(
    memsys_t memsys, uint64_t* channel, uint64_t* rank, uint64_t* bankgroup,
    uint64_t* bank, uint64_t* local_addr, uint64_t hex_address) {
  Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
  if (w && w->memsys) {
    w->memsys->GlobalToLocalAddr(channel, rank, bankgroup,
                                      bank, local_addr, hex_address);
  }
}

EXPORT int memsys_get_config_property(memsys_t memsys, char* id) {
    Wrapper *w = reinterpret_cast<Wrapper *>(memsys);
    if (w && w->memsys) {
        return w->memsys->GetConfigParameter(std::string(id));
    }
    return -1;
}
}
