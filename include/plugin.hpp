#pragma once

#include "valve.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include <memory>
#include <mutex>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

class scoped_mprotect {
public:
  scoped_mprotect( void *address, int flags, int original_flags )
      : original_flags_( original_flags )
      , aligned_address_( std::bit_cast< void * >(
            std::bit_cast< uintptr_t >( address ) & ~( page_size_ - 1 ) ) ) {
    mprotect( aligned_address_, page_size_, flags );
  }
  ~scoped_mprotect( ) {
    mprotect( aligned_address_, page_size_, original_flags_ );
  }

private:
  void *aligned_address_;
  int original_flags_;
  static long page_size_;
};

class simple_hook {
public:
  simple_hook( void *src, void *dst )
      : address_( src ) {
#if __x86_64__
    memcpy( code_ + 2, &dst, sizeof dst );
#else
    memcpy( code_ + 1, &dst, sizeof dst );

#endif
  }

  auto swap( ) {
    scoped_mprotect pr(
        address_, PROT_READ | PROT_WRITE | PROT_EXEC, PROT_READ | PROT_EXEC );

    std::swap_ranges(
        std::bit_cast< uint8_t * >( address_ ),
        std::bit_cast< uint8_t * >(
            std::bit_cast< uintptr_t >( address_ ) + sizeof code_ ),
        code_ );
  }

  auto code( ) -> std::uint8_t ( & )[] { return code_; }

private:
  void *address_;
#if __x86_64__
  // note: untested
  std::uint8_t code_[ 13 ] = { 0x48, 0xb8, 0x0, 0x0,  0x0,  0x0, 0x0,
                               0x0,  0x0,  0x0, 0xff, 0xe0, 0xc3 };
#else
  std::uint8_t code_[ 6 ] = { 0x68, 0x0, 0x0, 0x0, 0x0, 0xc3 };
#endif
};

class frame_snapshot_fix
    : public valve::interface_registry
    , valve::plugin_callbacks::v3 {
public:
  frame_snapshot_fix( ) noexcept
      : valve::interface_registry( "ISERVERPLUGINCALLBACKS003" ) {};

  auto description( ) -> const char * override { return "frame_snapshot_fix"; };

  auto load( valve::factory factory, valve::factory ) -> bool override;
  auto unload( ) -> void override;

private:
  static auto create_empty_snapshot_override( void *, int, int ) -> void *;
  static auto delete_frame_snapshot_override( void *, void * ) -> void;

private:
  void *engine_handle_;

  void *create_empty_snapshot_;
  void *delete_frame_snapshot_;

  std::unique_ptr< simple_hook > create_empty_snapshot_hook_;
  std::mutex create_empty_snapshot_hook_mutex_;
  std::unique_ptr< simple_hook > delete_frame_snapshot_hook_;
  std::mutex delete_frame_snapshot_hook_mutex_;

  std::recursive_mutex frame_snapshots_mutex_;
};
