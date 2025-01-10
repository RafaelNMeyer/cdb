#include <libcdb/error.hpp>
#include <libcdb/process.hpp>
#include <sys/ptrace.h>
#include <sys/wait.h>

cdb::process::~process() {
  if (pid_ != 0) {
    int status;
    if (state_ == process_state::running) {
      kill(pid_, SIGSTOP);
      waitpid(pid_, &status, 0);
    }
    ptrace(PTRACE_DETACH, pid_);
    kill(pid_, SIGCONT);

    if (terminate_on_end_) {
      kill(pid_, SIGKILL);
      waitpid(pid_, &status, 0);
    }
  }
}

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
  state_ = process_state::running;
}

cdb::stop_reason::stop_reason(int wait_status) {
  if (WIFEXITED(wait_status)) {
    reason = process_state::exited;
    info = WEXITSTATUS(wait_status);
  } else if (WIFSIGNALED(wait_status)) {
    reason = process_state::terminated;
    info = WTERMSIG(wait_status);
  } else if (WIFSTOPPED(wait_status)) {
    reason = process_state::stopped;
    info = WSTOPSIG(wait_status);
  }
}

cdb::stop_reason cdb::process::wait_on_signal() {
  int wait_status;
  int options = 0;
  if (waitpid(pid_, &wait_status, options) < 0) {
    error::send_errno("waitpid failed");
  }
  stop_reason reason(wait_status);
  state_ = reason.reason;
  return reason;
}
