#pragma once

#include "spinlock.h"
#include "atomic.hh"
#include "cpputil.hh"
#include "fs.h"
#include "file.hh"
#include "filetable.hh"

class uwq;
class uwq_worker;

// Saved registers for kernel context switches.
// (also implicitly defined in swtch.S)
struct context {
  u64 r15;
  u64 r14;
  u64 r13;
  u64 r12;
  u64 rbp;
  u64 rbx;
  u64 rip;
} __attribute__((packed));

// Per-process, per-stack meta data for mtrace
#if MTRACE
#define MTRACE_NSTACKS 16
#define MTRACE_TAGSHIFT 28
#if NCPU > 16
#error Oops -- decrease MTRACE_TAGSHIFT
#endif
struct mtrace_stacks {
  int curr;
  unsigned long tag[MTRACE_NSTACKS];
};
#endif

typedef enum procstate { 
  EMBRYO,
  SLEEPING,
  RUNNABLE,
  RUNNING,
  ZOMBIE 
} procstate_t;;

// Per-process state
struct proc : public rcu_freed {
  struct vmap *vmap;           // va -> vma
  uwq* uwq;
  uwq_worker* worker;
  char *kstack;                // Bottom of kernel stack for this process
  volatile int pid;            // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  int killed;                  // If non-zero, have been killed
  filetable *ftable;
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
  u64 tsc;
  u64 curcycles;
  unsigned cpuid;
  struct spinlock lock;
  SLIST_HEAD(childlist, proc) childq;
  SLIST_ENTRY(proc) child_next;
  struct condvar cv;
  std::atomic<u64> epoch;      // low 8 bits are depth count
  char lockname[16];
  int cpu_pin;
#if MTRACE
  struct mtrace_stacks mtrace_stacks;
#endif
  struct runq *runq;
  STAILQ_ENTRY(proc) runqlink;

  struct condvar *oncv;        // Where it is sleeping, for kill()
  u64 cv_wakeup;               // Wakeup time for this process
  LIST_ENTRY(proc) cv_waiters; // Linked list of processes waiting for oncv
  LIST_ENTRY(proc) cv_sleep;   // Linked list of processes sleeping on a cv
  u64 user_fs_;
  u64 unmap_tlbreq_;
  int exec_cpuid_;
  int run_cpuid_;
  int in_exec_;
  int uaccess_;

  static proc* alloc();
  void         set_state(procstate_t s);
  procstate_t  get_state(void) const { return state_; }
  int          set_cpu_pin(int cpu);
  static int   kill(int pid);
  int          kill();

  virtual void do_gc(void) { delete this; }

private:
  proc(int npid);
  ~proc(void);
  proc& operator=(const proc&);
  proc(const proc& x);
  NEW_DELETE_OPS(proc);
  
  procstate_t state_;       // Process state  
};
