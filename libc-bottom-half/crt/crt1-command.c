#include <stdint.h>
#ifdef _REENTRANT
#include <stdatomic.h>
extern void __wasi_init_tp(void);
#endif
#include <wasi/api.h>
extern void __wasm_call_ctors(void);
extern int __main_void(void);
extern void __wasm_call_dtors(void);

#ifdef WASI_PREVIEW1
__attribute__((export_name("_start")))
void _start(void) {
#endif

#ifdef WASI_PREVIEW2
int32_t run_run(void) {
    // Link in the bindings.
    extern void __component_type_object_force_link_command_public_use_in_this_compilation_unit(void);
    __component_type_object_force_link_command_public_use_in_this_compilation_unit();
#endif

    // Commands should only be called once per instance. This simple check
    // ensures that the `_start` function isn't started more than once.
    //
    // We use `volatile` here to prevent the store to `started` from being
    // sunk past any subsequent code, and to prevent any compiler from
    // optimizing based on the knowledge that `_start` is the program
    // entrypoint.
#ifdef _REENTRANT
    static volatile _Atomic int started = 0;
    int expected = 0;
    if (!atomic_compare_exchange_strong(&started, &expected, 1)) {
	__builtin_trap();
    }
#else
    static volatile int started = 0;
    if (started != 0) {
	__builtin_trap();
    }
    started = 1;
#endif

#ifdef _REENTRANT
	__wasi_init_tp();
#endif

    // The linker synthesizes this to call constructors.
    __wasm_call_ctors();

    // Call `__main_void` which will either be the application's zero-argument
    // `__main_void` function or a libc routine which obtains the command-line
    // arguments and calls `__main_argv_argc`.
    int r = __main_void();

    // Call atexit functions, destructors, stdio cleanup, etc.
    __wasm_call_dtors();

#ifdef WASI_PREVIEW1
    // If main exited successfully, just return, otherwise call
    // `__wasi_proc_exit`.
    if (r != 0) {
        __wasi_proc_exit(r);
    }
#endif
#ifdef WASI_PREVIEW2
    return r != 0;
#endif
}
