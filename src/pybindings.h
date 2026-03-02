#ifndef PYBINDINGS_H
#define PYBINDINGS_H

#include <cstdint>
#include <stdint.h>
#include <sys/types.h>
#include <string>

extern "C" {
#define EXPORT __attribute__((visibility("default")))

typedef void *memsys_t;
typedef void (*dramsim_callback_t)(uint64_t addr);
EXPORT memsys_t memsys_create(const char *config_file, const char *output_dir,
                              dramsim_callback_t read_callback,
                              dramsim_callback_t write_callback);
EXPORT void memsys_destroy(memsys_t memsys);
EXPORT void memsys_print_stats(memsys_t memsys);
EXPORT uint64_t memsys_get_cycle(memsys_t memsys);
EXPORT void memsys_tick(memsys_t memsys);
EXPORT void memsys_register_callbacks(memsys_t memsys, dramsim_callback_t rcb,
                                      dramsim_callback_t wcb);
EXPORT bool memsys_add_transaction(memsys_t memsys, uint64_t hex_address,
                                   bool is_write, bool is_pim);
EXPORT void memsys_toggle_mode(memsys_t memsys);
EXPORT void memsys_set_pim_mode(memsys_t memsys, bool pim_mode);
EXPORT bool memsys_get_pim_mode(memsys_t memsys);
EXPORT bool memsys_add_transaction_to_bank(memsys_t memsys, uint64_t channel,
                                           uint64_t rank, uint64_t bankgroup,
                                           uint64_t bank, uint64_t hex_address,
                                           bool is_write, bool is_pim);

EXPORT void memsys_get_byte_range_from_bank(memsys_t memsys, uint64_t channel,
                                            uint64_t rank, uint64_t bankgroup,
                                            uint64_t bank, uint64_t hex_address,
                                            int64_t *data_idx,
                                            size_t *start_idx);
EXPORT void memsys_mmap(memsys_t memsys, uint64_t channel, uint64_t rank,
                        uint64_t bankgroup, uint64_t bank, uint64_t hex_address,
                        int64_t data_idx, size_t length, size_t offset);
EXPORT void memsys_munmap(memsys_t memsys, uint64_t channel, uint64_t rank,
                          uint64_t bankgroup, uint64_t bank,
                          size_t base_address, size_t length);
EXPORT uint64_t memsys_get_address_from_physical_location(
    memsys_t memsys, uint64_t channel, uint64_t rank, uint64_t bankgroup,
    uint64_t bank, uint64_t hex_address);
EXPORT void memsys_get_physical_location_from_address(
    memsys_t memsys, uint64_t* channel, uint64_t* rank, uint64_t* bankgroup,
    uint64_t* bank, uint64_t* local_addr, uint64_t hex_address);
EXPORT uint64_t memsys_get_ranks(memsys_t memsys);
EXPORT uint64_t memsys_get_channels(memsys_t memsys);
EXPORT uint64_t memsys_get_banks_per_bankgroup(memsys_t memsys);
EXPORT uint64_t memsys_get_bankgroups_per_rank(memsys_t memsys);
EXPORT int memsys_get_config_property(memsys_t memsys, char* id);
}

#endif
