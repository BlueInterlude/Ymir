#pragma once

#include <ymir/sys/backup_ram_defs.hpp>

#include <ymir/util/size_ops.hpp>

namespace ymir::sys {

inline constexpr size_t kIPLSize = 512_KiB;
inline constexpr uint64 kIPLHashSeed = 0x94B487AF51733FBEull;

inline constexpr size_t kWRAMLowSize = 1_MiB;
inline constexpr size_t kWRAMHighSize = 1_MiB;

inline constexpr bup::BackupMemorySize kInternalBackupRAMSize = bup::BackupMemorySize::_256Kbit;
inline constexpr size_t kInternalBackupRAMSizeAmount = 32_KiB;

} // namespace ymir::sys
