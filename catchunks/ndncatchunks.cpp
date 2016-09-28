/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2016,  Regents of the University of California,
 *                      Colorado State University,
 *                      University Pierre & Marie Curie, Sorbonne University.
 *
 * This file is part of ndn-tools (Named Data Networking Essential Tools).
 * See AUTHORS.md for complete list of ndn-tools authors and contributors.
 *
 * ndn-tools is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndn-tools is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndn-tools, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 *
 * @author Wentao Shang
 * @author Steve DiBenedetto
 * @author Andrea Tosatto
 * @author Davide Pesavento
 * @author Weiwei Liu
 */

#include "version.hpp"
#include "options.hpp"
#include "consumer.hpp"
#include "discover-version-fixed.hpp"
#include "discover-version-iterative.hpp"
#include "pipeline-interests-fixed-window.hpp"

#include <ndn-cxx/security/validator-null.hpp>

namespace ndn {
namespace chunks {

static int
main(int argc, char** argv)
{
  std::string programName(argv[0]);
  Options options;
  std::string discoverType("fixed");
  std::string pipelineType("fixed");
  size_t maxPipelineSize(1);
  int maxRetriesAfterVersionFound(1);
  std::string uri;

  namespace po = boost::program_options;
  po::options_description visibleDesc("Options");


  po::positional_options_description p;
  p.add("ndn-name", -1);

  po::options_description optDesc("Allowed options");
  optDesc.add(visibleDesc).add(hiddenDesc);

  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).options(optDesc).positional(p).run(), vm);
    po::notify(vm);
  }
  catch (const po::error& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 2;
  }
  catch (const boost::bad_any_cast& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 2;
  }



  if (maxPipelineSize < 1 || maxPipelineSize > 1024) {
    std::cerr << "ERROR: pipeline size must be between 1 and 1024" << std::endl;
    return 2;
  }

//  if (options.maxRetriesOnTimeoutOrNack < -1 || options.maxRetriesOnTimeoutOrNack > 1024) {
//    std::cerr << "ERROR: retries value must be between -1 and 1024" << std::endl;
//    return 2;
//  }


  options.interestLifetime = time::milliseconds(vm["lifetime"].as<uint64_t>());

  try {
    Face face;

    unique_ptr<DiscoverVersion> discover;
 //   if (discoverType == "fixed") {
 //     discover = make_unique<DiscoverVersionFixed>(prefix, face, options);
 //   }
 //   else if (discoverType == "iterative") {
      DiscoverVersionIterative::Options optionsIterative(options);
      optionsIterative.maxRetriesAfterVersionFound = maxRetriesAfterVersionFound;
      discover = make_unique<DiscoverVersionIterative>(prefix, face, optionsIterative);
  //  }
 //   else {
 //     std::cerr << "ERROR: discover version type not valid" << std::endl;
  //    return 2;
   // }

    unique_ptr<PipelineInterests> pipeline;
    if (pipelineType == "fixed") {
      PipelineInterestsFixedWindow::Options optionsPipeline(options);
      optionsPipeline.maxPipelineSize = maxPipelineSize;
      pipeline = make_unique<PipelineInterestsFixedWindow>(face, optionsPipeline);
    }
    else {
      std::cerr << "ERROR: Interest pipeline type not valid" << std::endl;
      return 2;
    }

    ValidatorNull validator;
    Consumer consumer(validator, options.isVerbose);

    BOOST_ASSERT(discover != nullptr);
    BOOST_ASSERT(pipeline != nullptr);
    consumer.run(std::move(discover), std::move(pipeline));
    face.processEvents();
  }
  catch (const Consumer::ApplicationNackError& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 3;
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}

} // namespace chunks
} // namespace ndn

int
main(int argc, char** argv)
{
  return ndn::chunks::main(argc, argv);
}
