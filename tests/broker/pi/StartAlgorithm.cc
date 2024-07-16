//
// Test broker
//
#if defined(HAVE_CONFIG_H)
#include "arlhms_config.h"
#endif // HAVE_CONFIG_H 

#if defined(HAVE_MPI)
#include <mpi.h>
#include <comm/MPICommunicator.h>
#endif

#include "PiArgument.h"
#include "PicalcInputFilter.h" 
#include "PicalcOutputFilter.h"
#include "PiValue.h"

#include <comm/Communicator.h>
#include <comm/SocketCommunicator.h>

#include <base/ModelPackage.h>
#include <broker/BrokerLauncher.h>

#include <exception/ConnectionTerminationException.h>
#include <exception/IOError.h>
#include <exception/NetworkError.h>

#include <warehouse/Warehouse.h>

#if defined(ENABLE_LOGGING)
#if defined(HAVE_BOOST_LOG)
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#else
#error boost logging not available
#endif // HAVE_BOOST_LOG
#endif // ENABLE_LOGGING

#include <boost/serialization/shared_ptr.hpp>

#if defined(HAVE_FSTREAM)
#include <fstream>
#else
#error fstream header file not available
#endif // HAVE_FSTREAM

#if defined(HAVE_IOSTREAM)
#include <iostream>
#else
#error iostream header file not available
#endif // HAVE_IOSTREAM

#if defined(HAVE_SSTREAM)
#include <sstream>
#else
#error sstream header file not available
#endif // HAVE_SSTREAM

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#else
#error unistd.h header file not available
#endif // HAVE_UNISTD_H

const std::string hmsConf("hmsConf.ini");

int main(int    ac,
         char * av[])
{

#if defined(HAVE_MPI)

  MPI_Init(&ac,
           &av);

  int rank;
  int size;
  MPI_Comm_rank(MPI_COMM_WORLD,
                &rank);
  MPI_Comm_size(MPI_COMM_WORLD,
                &size);

#else

  int rank = 0;
  int size = 1;

#endif // HAVE_MPI

  arl::hms::BrokerLauncher brokerLauncher;
  
  const std::string configFile(hmsConf);
  
  std::vector<arl::hms::CommunicatorPointer> communicators = 
    brokerLauncher.launch(configFile,
                          rank,
                          size); 

  if(communicators.size() != 1) {
    const std::string message("Error launching broker");
    throw arl::hms::IOError(message);
  }

  const arl::hms::CommunicatorPointer & communicator = communicators[0];

  int i = 0;

  //
  // send 100 model packages
  //
  for( ; i < 100; ++i ) {

    //
    // instantiate model
    //
    arl::hms::ModelPointer 
      model(new arl::hms::Model("",
                                "cpi",
                                std::vector<std::string>(),
                                arl::hms::CPU,
                                1));

    arl::hms::OutputFilterPointer 
      outputFilter(new arl::hms::PicalcOutputFilter()) ;
    arl::hms::InputFilterPointer 
      inputFilter(new arl::hms::PicalcInputFilter()) ;
    arl::hms::ArgumentPointer 
      argument(new arl::hms::PiArgument(i));

    //
    // only retain every fifth model evaluation
    //
    if(i%5 == 0) {
      argument->setToCleanup(false);
    }
    else {
      argument->setToCleanup(true);
    }
    
    //
    // instantiate model package
    //
    arl::hms::ModelPackagePointer modelPackage = 
      arl::hms::ModelPackagePointer(new arl::hms::ModelPackage(model,
                                                               inputFilter,
                                                               outputFilter,
                                                               argument));

    //
    // send model package
    //
    communicator->send(modelPackage); 
    std::cout << "Model Package Sent" << std::endl;
    
  }

  //
  // open log file for timing data
  //
  std::ofstream logFile("timings.log");
 
  //
  // receive values back 
  //
  int counter = 0; 
  while(counter < 100) {

    std::vector<arl::hms::ModelPackagePointer> modelPackages = 
      communicator->receive<arl::hms::ModelPackagePointer>();

    if(modelPackages.size() > 0) {
      std::cout << "Received " << modelPackages.size() 
                << " model packages" << std::endl;
    }

    //
    // increment counter
    //
    counter += modelPackages.size();

    //
    // iterate over received model package
    //
    std::vector<arl::hms::ModelPackagePointer>::const_iterator iter = 
      modelPackages.begin();
    const std::vector<arl::hms::ModelPackagePointer>::const_iterator
      iterEnd = modelPackages.end(); 

    for( ; iter != iterEnd; ++iter) {

      //
      // obtain handle to model package
      //
      const arl::hms::ModelPackagePointer & modelPackage = *iter;
    
      //
      // obtain handle to value
      //
      const arl::hms::ValuePointer value = modelPackage->getValue();

      //
      // obtain handle to value 
      //
      const boost::shared_ptr<arl::hms::PiValue> piValue = 
        boost::dynamic_pointer_cast<arl::hms::PiValue>(value);

      std::cout << "Received: " << piValue->getValue() << std::endl;
      logFile << modelPackage->getTimestampLog() << std::endl;
      std::cout << "Number evals: " << modelPackage->getNumberEvaluations() << std::endl;

    }


  }

  /*
#if defined(HAVE_MPI)

  MPI_Finalize();

#endif // HAVE_MPI
  */

  //
  //
  //
  return 0; 

}
