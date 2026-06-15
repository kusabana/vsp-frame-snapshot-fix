#include <dlfcn.h>
#include <mutex>

#include "plugin.hpp"
#include "symbol.hpp"

frame_snapshot_fix plugin_instance;

auto frame_snapshot_fix::create_empty_snapshot_override(
    void *self, int tickcount, int maxEntities ) -> void * {
  std::lock_guard< std::recursive_mutex > lk(
      plugin_instance.frame_snapshots_mutex_ );
  auto original = (create_fn) subhook_get_trampoline(
      plugin_instance.create_empty_snapshot_hook_ );
  return original( self, tickcount, maxEntities );
}

auto frame_snapshot_fix::delete_frame_snapshot_override(
    void *self, void *pSnapshot ) -> void {
  if ( !pSnapshot )
    return;

  std::lock_guard< std::recursive_mutex > lk(
      plugin_instance.frame_snapshots_mutex_ );
  auto original = (delete_fn) subhook_get_trampoline(
      plugin_instance.delete_frame_snapshot_hook_ );
  original( self, pSnapshot );
}

auto frame_snapshot_fix::next_snapshot_override(
    void *self, const void *pSnapshot ) -> void * {
  std::lock_guard< std::recursive_mutex > lk(
      plugin_instance.frame_snapshots_mutex_ );
  auto original =
      (next_fn) subhook_get_trampoline( plugin_instance.next_snapshot_hook_ );
  return original( self, pSnapshot );
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

  struct {
    const char *symbol;
    void *override;
    subhook_t &hook;
  } targets[] = {
      { "_ZN21CFrameSnapshotManager19CreateEmptySnapshotEii",
        (void *) &frame_snapshot_fix::create_empty_snapshot_override,
        create_empty_snapshot_hook_ },
      { "_ZN21CFrameSnapshotManager19DeleteFrameSnapshotEP14CFrameSnapshot",
        (void *) &frame_snapshot_fix::delete_frame_snapshot_override,
        delete_frame_snapshot_hook_ },
      { "_ZN21CFrameSnapshotManager12NextSnapshotEPK14CFrameSnapshot",
        (void *) &frame_snapshot_fix::next_snapshot_override,
        next_snapshot_hook_ },
  };

  for ( auto &t : targets ) {
    void *addr = sym::resolve( engine_handle_, t.symbol );
    if ( !addr )
      goto fail;

#if __x86_64__
    t.hook = subhook_new( addr, t.override, SUBHOOK_64BIT_OFFSET );
#else
    t.hook = subhook_new( addr, t.override, static_cast< subhook_flags_t >( 0 ) );
#endif
    if ( !t.hook || subhook_install( t.hook ) != 0 )
      goto fail;

    if ( !subhook_get_trampoline( t.hook ) )
      goto fail;
  }

  return true;

fail:
  unload( );
  return false;
}

auto frame_snapshot_fix::unload( ) -> void {
  for ( subhook_t *hook : { &create_empty_snapshot_hook_,
                            &delete_frame_snapshot_hook_,
                            &next_snapshot_hook_ } ) {
    if ( *hook ) {
      subhook_remove( *hook );
      subhook_free( *hook );
      *hook = nullptr;
    }
  }

  if ( engine_handle_ ) {
    dlclose( engine_handle_ );
    engine_handle_ = nullptr;
  }
}
