#include <cstdio>
#include <editline/readline.h>
#include <iostream>
#include <libcdb/process.hpp>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {
std::unique_ptr<cdb::process> attach(int argc, const char **argv) {
  if (argc == 3 && argv[1] == std::string_view("-p")) {
    pid_t pid = std::atoi(argv[2]);
    return cdb::process::attach(pid);
  } else {
    const char *program_path = argv[1];
    return cdb::process::launch(program_path);
  }
}
// TODO: refactor using stringstream and getline functions
std::vector<std::string> split(std::string_view line, char delimiter) {
  std::vector<std::string> tokens;
  std::string str = "";
  for (int i = 0; i < line.length(); i++) {
    char c = line[i];
    if (c == ' ') {
      if (!str.empty()) {
        tokens.push_back(str);
        str.clear();
      }
    } else {
      str += c;
      if (i == line.length() - 1)
        tokens.push_back(str);
    }
  }
  return tokens;
}
bool is_prefix(std::string_view str, std::string_view of) {
  if (str.size() > of.size())
    return false;
  return std::equal(str.begin(), str.end(), of.begin());
}
void resume(pid_t pid) {}

void handle_command(std::unique_ptr<cdb::process> &proc,
                    std::string_view line) {
  auto args = split(line, ' ');
  auto command = args[0];
  if (is_prefix(command, "continue")) {
    proc->resume();
    proc->wait_on_signal();
  } else {
    std::cerr << "Unknow command\n";
  }
}
} // namespace

auto main(int argc, const char **argv) -> int {
  if (argc == 1) {
    std::cerr << "Error: Invalid number of arguments.\n";
    std::cerr << "Usage: " << argv[0] << " -p <pid_number>\n";
    std::cerr << "   or: " << argv[0] << " <process_name>\n";
    std::cerr << "  -p <pid_number>: specifies the process's PID.\n";
    std::cerr << "  <process_name>: specifies the name of the process.\n";
    return -1;
  }
  std::unique_ptr<cdb::process> proc = attach(argc, argv);
  std::cout << "Attached process pid: " << proc->pid() << std::endl;

  char *line = nullptr;
  while ((line = readline("cdb > ")) != nullptr) {
    std::string line_str;
    if (std::string_view(line) == "") {
      if (history_length > 0)
        line_str = history_list()[history_length - 1]->line;
    } else {
      line_str = line;
      add_history(line);
      free(line);
    }
    if (!line_str.empty())
      handle_command(proc, line_str);
  }

  return 0;
}
