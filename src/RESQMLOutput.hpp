/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2020 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2020 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2020 TotalEnergies
 * Copyright (c) 2019-     GEOSX Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

/**
 * @file RESQMLOutput.hpp
 */

#ifndef GEOS_FILEIO_OUTPUTS_RESQMLOUTPUT_HPP_
#define GEOS_FILEIO_OUTPUTS_RESQMLOUTPUT_HPP_

#include "fileIO/Outputs/OutputBase.hpp"
#include "RESQMLWriterInterface.hpp"

namespace geos
{

/**
 * @brief A class for creating RESQML outputs
 */
class RESQMLOutput : public OutputBase
{
public:

  /// @copydoc geos::dataRepository::Group::Group(string const & name, Group * const parent)
  RESQMLOutput( string const & name, Group * const parent );

  /// Destructor
  virtual ~RESQMLOutput() override;

  /**
   * @brief Catalog name interface
   * @return This type's catalog name
   */
  static string catalogName() { return "RESQML"; }


  virtual void postProcessInput() override;

  /**
   * @brief Writes out a set of properties.
   * @copydoc EventBase::execute()
   */
  virtual bool execute( real64 const time_n,
                        real64 const dt,
                        integer const cycleNumber,
                        integer const eventCounter,
                        real64 const eventProgress,
                        DomainPartition & domain ) override;

  /**
   * @brief Write one final set of vtk files as the code exits
   * @copydoc ExecutableGroup::cleanup()
   */
  virtual void cleanup( real64 const time_n,
                        integer const cycleNumber,
                        integer const eventCounter,
                        real64 const eventProgress,
                        DomainPartition & domain ) override;

  /**
   * @brief Performs re-initialization of the datasets accumulated in the PVD writer.
   */
  virtual void reinit() override;

  /// @cond DO_NOT_DOCUMENT
  struct viewKeysStruct : OutputBase::viewKeysStruct
  {
    static constexpr auto plotFileName = "plotFileName";
    static constexpr auto plotLevel = "plotLevel";
    static constexpr auto referenceObjectName = "referenceObjectName";
    static constexpr auto onlyPlotSpecifiedFieldNames = "onlyPlotSpecifiedFieldNames";
    static constexpr auto fieldNames = "fieldNames";
    static constexpr auto inputRepositoryName = "inputRepositoryName";    
  } RESQMLOutputViewKeys;
  /// @endcond

private:

  /// Name of the file for this output
  string m_plotFileName;

  /// level of detail plot
  integer m_plotLevel;

  /// flag to decide whether we only plot the specified field names
  integer m_onlyPlotSpecifiedFieldNames;

  /// array of names of the fields to output
  array1d< string > m_fieldNames;

  /// Name of the Mesh input node
  string m_referenceObjectName;

  /// UUID of the parent grid
  string m_inputRepositoryName;

  // Name of the parent grid
  string m_parentMeshName;

  RESQMLWriterInterface m_writer;
};

} /* namespace geos */

#endif /* GEOS_FILEIO_OUTPUTS_RESQMLOUTPUT_HPP_ */