
#include "EpcDocumentRepository.hpp"

namespace geos {

using namespace dataRepository;

EpcDocumentRepository::EpcDocumentRepository(string const &name,
                                             Group *const parent)
    : EnergyMLDataObjectRepository(name, parent) {
  enableLogLevelInput();

  registerWrapper(viewKeyStruct::filesPathsString(), &m_filesPaths)
      .setInputFlag(InputFlags::REQUIRED)
      .setRestartFlags(RestartFlags::NO_WRITE)
      .setDescription("Paths to the EPC files");
}

void EpcDocumentRepository::postProcessInput() {

  try {
    for (const string &path : m_filesPaths) {
      GEOS_LOG_RANK_0(GEOS_FMT("Reading: {}", path));
      COMMON_NS::EpcDocument pck(path);
      std::string message = pck.deserializeInto(*m_repository);
      pck.close();
      GEOS_LOG_RANK_0(GEOS_FMT("Deserilization message: {}", message));
    }
  } catch (const std::exception &e) {
    delete m_repository;
    GEOS_THROW(
        GEOS_FMT("{}: invalid file path : {}", this->getName(), e.what()),
        InputError);
  }

  GEOS_LOG_RANK_0(
      GEOS_FMT("{} entities read", m_repository->getUuids().size()));
}

REGISTER_CATALOG_ENTRY(MeshBase, EpcDocumentRepository, string const &,
                       Group *const)

} // namespace geos