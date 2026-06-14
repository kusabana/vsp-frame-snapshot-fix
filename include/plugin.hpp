#pragma once

#include "valve.hpp"
#include <mutex>
#include <subhook.h>

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
  using create_fn = auto ( * )( void *, int, int ) -> void *;
  using delete_fn = auto ( * )( void *, void * ) -> void;
  using next_fn = auto ( * )( void *, const void * ) -> void *;

  static auto create_empty_snapshot_override( void *, int, int ) -> void *;
  static auto delete_frame_snapshot_override( void *, void * ) -> void;
  static auto next_snapshot_override( void *, const void * ) -> void *;

private:
  void *engine_handle_ = nullptr;

  subhook_t create_empty_snapshot_hook_ = nullptr;
  subhook_t delete_frame_snapshot_hook_ = nullptr;
  subhook_t next_snapshot_hook_ = nullptr;

  std::recursive_mutex frame_snapshots_mutex_;
};
