#ifndef CDB_ERROR_HPP
#define CDB_ERROR_HPP

#include <cstring>
#include <stdexcept>

namespace cdb {
class error : std::runtime_error {
public:
  [[noreturn]]
  static void send(const std::string &what) {
    throw error(what);
  }
  [[noreturn]]
  static void send_errno(const std::string &prefix) {
    throw error(prefix + " " + std::strerror(errno));
  }

private:
  error(const std::string &error) : std::runtime_error(error) {}
};
} // namespace cdb

#endif
