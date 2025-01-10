#include <iostream>
#include <libcdb/process.hpp>
#include <sys/ptrace.h>
#include <sys/wait.h>

std::unique_ptr<cdb::process> cdb::process::launch(std::filesystem::path path) {
  pid_t pid = 0;
  if ((pid = fork()) < 0) {
    perror("Fork error");
    std::cerr << "Fork error" << std::endl;
    throw std::system_error();
  }
  if (pid == 0) {
    if (ptrace(PTRACE_TRACEME) < 0) {
      perror("PTRACE_TRACEME request error");
      throw std::system_error();
    }
    if (execlp(path.c_str(), path.c_str(), NULL) < 0) {
      perror("execlp error");
      throw std::system_error();
    }
  }
  std::unique_ptr<cdb::process> proc(
      new process(pid, /*terminate_on_end=*/true));
  proc->wait_on_signal();
  return proc;
}
std::unique_ptr<cdb::process> cdb::process::attach(pid_t pid) {
  if (pid == 0) {
    std::cerr << "Invalid PID" << std::endl;
    throw std::system_error();
  }
  if (ptrace(PTRACE_ATTACH, pid) < 0) {
    perror("PTRACE_ATTACH request error");
    throw std::system_error();
  }
  std::unique_ptr<cdb::process> proc(
      new process(pid, /*terminate_on_end=*/false));
  proc->wait_on_signal();
  return proc;
}

void cdb::process::resume() {
  if (ptrace(PTRACE_CONT, pid_, nullptr, nullptr) < 0) {
    perror("PTRACE_CONT");
    std::cerr << "Couldn't continue\n";
    throw std::system_error();
  }
}
void cdb::process::wait_on_signal() {
  int wait_status;
  int options = 0;
  if (waitpid(pid_, &wait_status, options) < 0) {
    perror("waitpid");
    std::exit(-1);
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
