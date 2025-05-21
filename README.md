Practice & research game engine techniques for cloud game

Primary Goals
1. Considering multiple game instances in a single OS instance
  * One GameEngine with multiple GameInstances
  * One OS instance for one GameEngine
2. Fully data oriented design
  * DOTS like Entity Component System
  *  Use component group to organize memory layout in memory chunk
  * Interfaces between one System and another, can be either local or remote
  * Practice high speed networking(rdma, IPC) for systems communication
  * Using zero-copy to optimize communication by replicating chunk memory, the layout of which is organized by grouped components
3. Resource sharing
  * Completely IO ahead of time(before begin to play game), zero IO at runtime(after begin to play game)
  * Static resource sharing amaong game instances
  * Compute buffer sharing dynamically
4. Coroutine-based task system 
  * async_simple
  * asio coroutine
5. Vedio Streaming
6. Copy-on-Write Gameplay framework
