#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <string_view>
#include <unordered_map>

namespace valve {
  typedef enum {
    result_continue = 0,
    result_override = 1,
    result_stop = 2
  } plugin_result;

  typedef enum {
    status_intact = 0,
    status_not_found = 1,
    status_command = 2,
    status_protected = 3
  } cvar_status;

  using factory = void *( * ) ( const char *, int * );

  class server_unknown {
  public:
    virtual void *ihandleentity_0( ) = 0;
    virtual void *ihandleentity_1( ) = 0;
    virtual void *ihandleentity_2( ) = 0;
    virtual void *get_collideable( ) = 0;
    virtual void *get_networkable( ) = 0;
    virtual void *get_base_entity( ) = 0;
  };

  struct edict {
    int state_flags;
    short edict_index;
    short network_sn;
    void *networkable;
    server_unknown *unknown;
  };

  namespace plugin_callbacks {
    class v3 {
    public:
      // called when server loads the plugin
      virtual auto
      load( valve::factory factory, valve::factory /*unused*/ ) -> bool {
        return true;
      }
      // called when the plugin should shutdown
      virtual auto unload( ) -> void {}
      // called when a plugin should pause
      virtual auto pause( ) -> void {}
      // called when a plugin should unpause execution
      virtual auto unpause( ) -> void {}
      // plugin description
      virtual auto description( ) -> const char * { return ""; }
      // called when a new level is started (includes level changes)
      virtual auto level_init( ) -> void {}
      // called when the server is about to activate
      virtual auto server_activate( ) -> void {}
      // called every game frame
      virtual auto game_frame( bool simulating ) -> void {}
      // called when a level is shutdown ( includes level changes )
      virtual auto level_shutdown( ) -> void {}
      // called when a client is going active
      virtual auto client_active( edict *edict ) -> void {}
      // called when a client has fully connected ( has received initial entity
      // baseline )
      virtual auto client_loaded( edict *edict ) -> void {}
      // called when a client is disconnecting from the server
      virtual auto client_disconnect( edict *edict ) -> void {}
      // called when a client is connected
      virtual auto client_connected( edict *edict, char const *name ) -> void {}

      // sets the client index for the client who typed the command into their
      // console
      virtual auto set_command_client( int idx ) -> void {}
      // called when a player changed replicated cvars
      virtual auto client_cvar_changed( edict *edict ) -> void {}

      // called when a client is connecting to the server
      virtual auto client_connect(
          bool *allow,
          edict *edict,
          const char *name,
          const char *address,
          char *reject,
          int reject_len ) -> plugin_result {
        return result_continue;
      }

      // called when a client runs a command
      virtual auto client_command( edict *edict, void *args ) -> plugin_result {
        return result_continue;
      }

      // called when a client has their NID validated
      virtual auto
      nid_validated( const char *name, const char *nid ) -> plugin_result {
        return result_continue;
      }

      // called when a cvar query is finished
      virtual auto query_cvar_callback(
          int cookie,
          edict *edict,
          cvar_status status,
          const char *name,
          const char *value ) -> void {}

      // called when a new edict is allocated
      virtual auto on_edict_allocated( edict *edict ) -> void {}
      // called when an edict is about to be freed
      virtual auto on_edict_freed( const edict *edict ) -> void {}
    };
  } // namespace plugin_callbacks

  class interface_registry {
  public:
    interface_registry( ) = default;
    template < typename... N >
      requires std::is_convertible_v< N..., std::string_view >
    interface_registry( N &&...names ) {
      ( registry_.emplace( names, this ), ... );
    }

    static interface_registry *find( std::string name ) {
      if ( auto interface = registry_.find( name );
           interface != valve::interface_registry::registry_.end( ) )
        return interface->second;
      return { };
    }

  private:
    inline static std::unordered_map< std::string, interface_registry * >
        registry_ = { };
  };

} // namespace valve
