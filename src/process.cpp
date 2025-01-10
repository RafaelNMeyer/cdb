#include <iostream>
#include <libcdb/error.hpp>
#include <libcdb/process.hpp>
#include <sys/ptrace.h>
#include <sys/wait.h>

std::unique_ptr<cdb::process> cdb::process::launch(std::filesystem::path path) {
  pid_t pid = 0;
  if ((pid = fork()) < 0) {
    error::send_errno("fork failed");
  }
  if (pid == 0) {
    if (ptrace(PTRACE_TRACEME) < 0) {
      error::send_errno("ptrace traceme request failed");
    }
    if (execlp(path.c_str(), path.c_str(), NULL) < 0) {
      error::send_errno("execlp failed");
    }
  }
  std::unique_ptr<cdb::process> proc(
      new process(pid, /*terminate_on_end=*/true));
  proc->wait_on_signal();
  return proc;
}
std::unique_ptr<cdb::process> cdb::process::attach(pid_t pid) {
  if (pid == 0) {
    error::send("invalid pid parameter");
  }
  if (ptrace(PTRACE_ATTACH, pid) < 0) {
    error::send_errno("ptrace attach request failed");
  }
  std::unique_ptr<cdb::process> proc(
      new process(pid, /*terminate_on_end=*/false));
  proc->wait_on_signal();
  return proc;
}

void cdb::process::resume() {
  if (ptrace(PTRACE_CONT, pid_, nullptr, nullptr) < 0) {
    error::send_errno("ptrace cont request failed");
  }
}
void cdb::process::wait_on_signal() {
  int wait_status;
  int options = 0;
  if (waitpid(pid_, &wait_status, options) < 0) {
    error::send_errno("waitpid failed");
  }

  if (WIFEXITED(wait_status)) {
    printf("exited, status=%d\n", WEXITSTATUS(wait_status));
  } else if (WIFSIGNALED(wait_status)) {
    printf("killed by signal %d\n", WTERMSIG(wait_status));
  } else if (WIFSTOPPED(wait_status)) {
    printf("stopped by signal %d\n", WSTOPSIG(wait_status));
  } else if (WIFCONTINUED(wait_status)) {
    printf("continued\n");
  }
}
