
Coroutines in the same schedule share the stack , so you can create many coroutines without worry about memory.

But switching context will copy the stack the coroutine used.

Read source for detail.

Chinese blog : http://blog.codingnow.com/2012/07/c_coroutine.html

这里要提到实现 stackful 协程（与之相对的还有一种stackless协程）的两种技术：Separate coroutine stacks 和 Copying the stack（又叫共享栈）。

实现细节上，前者为每一个协程分配一个单独的、固定大小的栈；而后者则仅为正在运行的协程分配栈内存，当协程被调度切换出去时，就把它实际占用的栈内存 copy 保存到一个单独分配的缓冲区；当被切出去的协程再次调度执行时，再一次 copy 将原来保存的栈内存恢复到那个共享的、固定大小的栈内存空间。
