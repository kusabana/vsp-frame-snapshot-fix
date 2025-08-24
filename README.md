<div align="center">
  <h3><a href="https://github.com/kusabana">
    ~kusabana/</a>vsp-frame-snapshot-fix
  </h3>

Fix for sv_parallel_sendsnapshot crashes in Source Engine 2013 games
</div>

### The problem
```cpp
// If enabled, random crashes start to appear in WriteTempEntities, etc. It looks like
// one thread can be in WriteDeltaEntities while another is in WriteTempEntities, and both are
// partying on g_FrameSnapshotManager.m_FrameSnapshots. ...
static ConVar sv_parallel_sendsnapshot( "sv_parallel_sendsnapshot", "0" );
```
This was later fixed in the CS:GO engine branch by using a `CThreadFastMutex` for the `m_FrameSnapshots` list.
