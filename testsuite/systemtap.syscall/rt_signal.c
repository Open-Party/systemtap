/* COVERAGE:  rt_sigprocmask rt_sigaction rt_sigpending rt_sigqueueinfo */
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>
#include <string.h>
#include <signal.h>

static void 
sig_act_handler(int signo)
{
}

int
__rt_sigqueueinfo(pid_t tgid, int sig, siginfo_t *siginfo)
{
  return syscall(__NR_rt_sigqueueinfo, tgid, sig, siginfo);
}

int main()
{
  sigset_t mask;
  siginfo_t siginfo;
  struct sigaction sa;

  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  sigaddset(&mask, SIGUSR2);
  sigprocmask(SIG_BLOCK, &mask, NULL);
  //staptest// rt_sigprocmask (SIG_BLOCK, \[SIGUSR1|SIGUSR2\], 0x[0]+, 8) = 0

  sigdelset(&mask, SIGUSR2);
  sigprocmask(SIG_UNBLOCK, &mask, NULL);
  //staptest// rt_sigprocmask (SIG_UNBLOCK, \[SIGUSR1\], 0x[0]+, 8) = 0

  // glibc handles this one
  //sigprocmask(-1, &mask, NULL);

  // Causes a SIGSEGV
  //sigprocmask(SIG_BLOCK, (sigset_t *)-1, NULL);

  sigprocmask(SIG_BLOCK, &mask, (sigset_t *)-1);
#ifdef __s390__
  //staptest// rt_sigprocmask (SIG_BLOCK, \[SIGUSR1\], 0x[7]?[f]+, 8) = -NNNN (EFAULT)
#else
  //staptest// rt_sigprocmask (SIG_BLOCK, \[SIGUSR1\], 0x[f]+, 8) = -NNNN (EFAULT)
#endif

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGALRM);
  sa.sa_flags = 0;
  sigaction(SIGUSR1, &sa, NULL);
  //staptest// rt_sigaction (SIGUSR1, {SIG_IGN}, 0x[0]+, 8) = 0

  sa.sa_handler = SIG_DFL;
  sigaction(SIGUSR1, &sa, NULL);
  //staptest// rt_sigaction (SIGUSR1, {SIG_DFL}, 0x[0]+, 8) = 0
  
  sigaction(-1, &sa, NULL);
  //staptest// rt_sigaction (0x[f]+, {SIG_DFL}, 0x[0]+, 8) = -NNNN

  // Causes a SIGSEGV
  //sigaction(SIGUSR1, (struct sigaction *)-1, NULL);

  // Causes a SIGSEGV
  //sigaction(SIGUSR1, &sa, (struct sigaction *)-1);

  sa.sa_handler = sig_act_handler;
  sigaction(SIGUSR1, &sa, NULL);
#ifdef __ia64__
  //staptest// rt_sigaction (SIGUSR1, {XXXX, [^,]+, \[SIGALRM\]}, 0x[0]+, 8) = 0
#else
  //staptest// rt_sigaction (SIGUSR1, {XXXX, [^,]+, XXXX, \[SIGALRM\]}, 0x[0]+, 8) = 0
#endif

  sigpending(&mask);
  //staptest// rt_sigpending (XXXX, 8) = 0

  sigpending((sigset_t *) 0);
  //staptest// rt_sigpending (0x0, 8) = -NNNN (EFAULT)

  sigpending((sigset_t *)-1);
#ifdef __s390__
  //staptest// rt_sigpending (0x[7]?[f]+, 8) = -NNNN (EFAULT)
#else
  //staptest// rt_sigpending (0x[f]+, 8) = -NNNN (EFAULT)
#endif

  siginfo.si_code = SI_USER;
  siginfo.si_pid = getpid();
  siginfo.si_uid = getuid();
  siginfo.si_value.sival_int = 1;

  __rt_sigqueueinfo(getpid(), SIGUSR1, &siginfo);
  //staptest// rt_sigqueueinfo (NNNN, SIGUSR1, XXXX) = NNNN

  __rt_sigqueueinfo(-1, SIGUSR1, &siginfo);
  //staptest// rt_sigqueueinfo (-1, SIGUSR1, XXXX) = NNNN

  __rt_sigqueueinfo(getpid(), -1, &siginfo);
  //staptest// rt_sigqueueinfo (NNNN, 0x[f]+, XXXX) = NNNN

  __rt_sigqueueinfo(getpid(), SIGUSR1, (siginfo_t *)-1);
#ifdef __s390__
  //staptest// rt_sigqueueinfo (NNNN, SIGUSR1, 0x[7]?[f]+) = NNNN
#else
  //staptest// rt_sigqueueinfo (NNNN, SIGUSR1, 0x[f]+) = NNNN
#endif

  return 0;
}

