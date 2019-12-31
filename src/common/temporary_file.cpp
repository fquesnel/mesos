#include "common/temporary_file.hpp"

namespace mesos {
namespace criteo {

/*
 * Represent a temporary file that can be either written or read from.
 */
TemporaryFile::TemporaryFile() {
  char filepath[] = "/tmp/criteo-mesos-XXXXXX";
  int fd = mkstemp(filepath);
  if (fd == -1)
    throw std::runtime_error(
        "Unable to create temporary file to run commands");
  close(fd);
  m_filepath = std::string(filepath);
}

/*
 * Read whole content of the temporary file.
 * @return The content of the file.
 */
std::string TemporaryFile::readAll() const {
  std::ifstream ifs(m_filepath);
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));
  ifs.close();
  return content;
}

/*
 * Write content to the temporary file and flush it.
 * @param content The content to write to the file.
 */
void TemporaryFile::write(const std::string& content) const {
  std::ofstream ofs;
  ofs.open(m_filepath);
  ofs << content;
  std::flush(ofs);
  ofs.close();
}

friend std::ostream& TemporaryFile::operator<<(std::ostream& out, const TemporaryFile& temp_file) {
  out << temp_file.m_filepath;
  return out;
}

}
}
