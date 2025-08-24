#include <bit>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <link.h>
#include <memory>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "plugin.hpp"
#include "symbol.hpp"

long scoped_mprotect::page_size_ = sysconf( _SC_PAGE_SIZE );

frame_snapshot_fix plugin_instance;

auto frame_snapshot_fix::create_empty_snapshot_override(
    void *self, int tickcount, int maxEntities ) -> void * {

  std::lock_guard< std::recursive_mutex > lk(
      plugin_instance.frame_snapshots_mutex_ );
  std::lock_guard< std::mutex > hook_lk(
      plugin_instance.create_empty_snapshot_hook_mutex_ );

  plugin_instance.create_empty_snapshot_hook_->swap( );
  auto ret = std::bit_cast<
      decltype( &frame_snapshot_fix::create_empty_snapshot_override ) >(
      plugin_instance.create_empty_snapshot_ )( self, tickcount, maxEntities );
  plugin_instance.create_empty_snapshot_hook_->swap( );

  return ret;
}

auto frame_snapshot_fix::delete_frame_snapshot_override(
    void *self, void *pSnapshot ) -> void {
  // fix for ...
  if ( !pSnapshot )
    return;

  std::lock_guard< std::recursive_mutex > lk(
      plugin_instance.frame_snapshots_mutex_ );
  std::lock_guard< std::mutex > hook_lk(
      plugin_instance.delete_frame_snapshot_hook_mutex_ );

  plugin_instance.delete_frame_snapshot_hook_->swap( );
  std::bit_cast<
      decltype( &frame_snapshot_fix::delete_frame_snapshot_override ) >(
      plugin_instance.delete_frame_snapshot_ )( self, pSnapshot );
  plugin_instance.delete_frame_snapshot_hook_->swap( );
}

auto frame_snapshot_fix::load( valve::factory factory, valve::factory )
    -> bool {
#if __x86_64__
  engine_handle_ = dlopen( "bin/linux64/engine_srv.so", RTLD_NOW );
#else
  engine_handle_ = dlopen( "bin/engine_srv.so", RTLD_NOW );
#endif
  if ( !engine_handle_ )
    return false;

  create_empty_snapshot_ = sym::resolve(
      engine_handle_, "_ZN21CFrameSnapshotManager19CreateEmptySnapshotEii" );
  if ( !create_empty_snapshot_ )
    return false;

  create_empty_snapshot_hook_ = std::make_unique< simple_hook >(
      create_empty_snapshot_,
      std::bit_cast< void * >(
          &frame_snapshot_fix::create_empty_snapshot_override ) );

  delete_frame_snapshot_ = sym::resolve(
      engine_handle_,
      "_ZN21CFrameSnapshotManager19DeleteFrameSnapshotEP14CFrameSnapshot" );
  if ( !delete_frame_snapshot_ )
    return false;

  delete_frame_snapshot_hook_ = std::make_unique< simple_hook >(
      delete_frame_snapshot_,
      std::bit_cast< void * >(
          &frame_snapshot_fix::delete_frame_snapshot_override ) );

  // enable hooks
  create_empty_snapshot_hook_->swap( );
  delete_frame_snapshot_hook_->swap( );

  return true;
}

auto frame_snapshot_fix::unload( ) -> void {
  if ( engine_handle_ )
    dlclose( engine_handle_ );

  // Revert hooks if applied
  {
    std::lock_guard< std::mutex > hook_lk(
        plugin_instance.create_empty_snapshot_hook_mutex_ );
    if ( std::bit_cast< void * >( create_empty_snapshot_hook_->code( ) + 1 ) !=
         frame_snapshot_fix::create_empty_snapshot_override )
      create_empty_snapshot_hook_->swap( );
  }

  {
    std::lock_guard< std::mutex > hook_lk(
        plugin_instance.delete_frame_snapshot_hook_mutex_ );
    if ( std::bit_cast< void * >( delete_frame_snapshot_hook_->code( ) + 1 ) !=
         frame_snapshot_fix::delete_frame_snapshot_override )
      delete_frame_snapshot_hook_->swap( );
  }
}
