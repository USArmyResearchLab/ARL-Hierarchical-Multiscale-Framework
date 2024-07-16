//
// Test communicators
//
#if defined(HAVE_CONFIG_H)
#include "arlhms_config.h"
#endif // HAVE_CONFIG_H 

#if defined(HAVE_MPI)
#include <mpi.h>
#include <comm/MPICommunicator.h>
#endif

#include "ScalarArgument.h"
#include "ScalarValue.h"

#include <base/Argument.h>
#include <base/Value.h>
#include <comm/Communicator.h>
#include <comm/SocketCommunicator.h>

#include <exception/ConnectionTerminationException.h>
#include <exception/IOError.h>
#include <exception/NetworkError.h>

#if defined(HAVE_BOOST)
#include <boost/serialization/vector.hpp>
#else
#error boost header files not available
#endif // HAVE_BOOST

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

namespace {

  typedef enum CommunicatorType { SOCKETTCP, SOCKETUNIX, MPI } CommunicatorType;
  typedef enum CommunicatorMode { CLIENT, SERVER } CommunicatorMode;

  //
  // print usage
  //
  inline void
  printUsage(const char * programName)
  {

    std::cerr << "usage: " << programName << " "
              << "-m mode -c comm" << std::endl;
    std::cerr << "\t-m mode: 1 - Client" << std::endl;
    std::cerr << "\t-m mode: 2 - Server" << std::endl;
    std::cerr << "\t-c comm: 1 - TCPIP Socket" << std::endl;
    std::cerr << "\t-c comm: 2 - Unix Socket" << std::endl;
    std::cerr << "\t-c comm: 3 - MPI" << std::endl;

    //
    //
    //
    return;

  }

  //
  // process command line
  //
  void
  processCommandLine(CommunicatorType & communicatorType,
                     CommunicatorMode & communicatorMode,
                     int                ac,
                     char             * av[])
  {

    int c;
    bool errorFlag = false;

    while( (c=getopt(ac, av, "m:c:")) != -1 )
    switch(c){

    case 'm':
      {
        const int value = std::atoi(optarg);
        if(value == 1) {
          communicatorMode = CLIENT;
        }
        else if(value == 2) {
          communicatorMode = SERVER;
        }
        else {
          errorFlag = true;
        }
        break;
      }
    case 'c':
      {
        const int value = std::atoi(optarg);
        if(value == 1) {
          communicatorType = SOCKETTCP;
        }
        else if(value == 2) {
          communicatorType = SOCKETUNIX;
        }
        else if(value == 3) {
          communicatorType = MPI;
        }
        else {
          errorFlag = true;
        }
        break;
      }
    case '?':
      errorFlag = true;
      break;

    }

    if(optind > ac) {
      errorFlag = true;
    }
    
    //
    // check errorFlag
    //
    if (errorFlag == true) {
      printUsage(av[0]);    
      std::exit(EXIT_FAILURE);
    }

    //
    //
    //
    return;

  }

}

//
//
//
int
main(int    ac,
     char * av[])
{

#if defined(HAVE_MPI)

  MPI_Init(&ac,
           &av);

#endif // HAVE_MPI

  //
  // process command line arguments
  //
  CommunicatorType communicatorType;
  CommunicatorMode communicatorMode;
  processCommandLine(communicatorType,
                     communicatorMode,
                     ac,
                     av);


  //
  // convert SERVER / CLIENT into is serverFlag
  //
  const bool isServer = communicatorMode == SERVER;

  //
  // initialize logging
  //

#if defined(ENABLE_LOGGING)

  boost::log::add_common_attributes();
  boost::log::core::get()->
    add_global_attribute("Scope",
                         boost::log::attributes::named_scope());

  std::string formatString("[%TimeStamp%] - %Scope% - %Message%");

#if defined(HAVE_MPI)

  //
  // get rank
  //
  int taskId;
  if(MPI_Comm_rank(MPI_COMM_WORLD,
                   &taskId) != MPI_SUCCESS) {
    const std::string message("Error getting rank");
    throw arl::hms::NetworkError(message);
  };

  std::stringstream format;
  format << "r@" << taskId << ": " << formatString; 

  formatString = format.str();

#endif // HAVE_MPI

  std::stringstream fileName;
  fileName << "Communicator_";

  if(isServer) {
    fileName << "server.log";
  }
  else {
    fileName << "client.log";
  }

  boost::log::add_file_log(boost::log::keywords::file_name = fileName.str(),
                           boost::log::keywords::format = formatString);

#endif // ENABLE_LOGGING

  //
  // instantiate communicators
  //
  arl::hms::Communicator * communicator;
  if(communicatorType == SOCKETTCP) {
    std::cout << isServer << std::endl;
    std::cout.flush();
    const std::string address = "0.0.0.0:9000";
    communicator = 
      new arl::hms::SocketCommunicator(address,
                                       arl::hms::SocketCommunicator::TCPIP,
                                       isServer);
  }
  else if(communicatorType == SOCKETUNIX) {
    const std::string address = "/tmp/socket";
    communicator = 
      new arl::hms::SocketCommunicator(address,
                                       arl::hms::SocketCommunicator::UNIX,
                                       isServer);
  }
  else {
    
#if defined(HAVE_MPI)
    
    MPI_Comm intercommunicator;

    if(isServer == false) {
      //
      // launch server from client
      //
      const char * arguments[5] = {"-m", "2", "-c", "3", NULL};
      int errorcodes;
      
      if(MPI_Comm_spawn(av[0],
                        const_cast<char **>(arguments),
                        1,
                        MPI_INFO_NULL,
                        0,
                        MPI_COMM_WORLD,
                        &intercommunicator,
                        &errorcodes) != MPI_SUCCESS) {
        const std::string message("Error spawning server from client");
        throw arl::hms::NetworkError(message);
      }
    }       
    else {
      
      //
      // get parent communicator
      //
      if(MPI_Comm_get_parent(&intercommunicator) != MPI_SUCCESS) {
        const std::string message("Failure getting parent communicator");
        throw arl::hms::NetworkError(message);
      }
      
    }
   
    //
    // instantiate MPICommunicator
    //
    communicator = new arl::hms::MPICommunicator(intercommunicator,
                                                 isServer,
                                                 0);
                        
#else
    
    //
    // MPI not available, throw IOError
    //
    const std::string message("MPI not compiled in - MPI communicators not "
                              "available");
    throw arl::hms::IOError(message);
    
#endif // HAVE_MPI    
    
  }
 
  //
  // send int from client to server
  //
  if(isServer) {
    
    //
    // continuously loop receiving int vectors from client
    //
    while (true) {

      std::vector<arl::hms::ArgumentPointer> arguments;
      try {
        arguments = communicator->receive<arl::hms::ArgumentPointer>();
      }
      catch (arl::hms::ConnectionTerminationException & exception) {
#if defined(ENABLE_LOGGING)
        BOOST_LOG_TRIVIAL(info) << exception.what();
#endif // ENABLE_LOGGING
        break;
      }
      catch (arl::hms::NetworkError & exception) {
#if defined(ENABLE_LOGGING)
        BOOST_LOG_TRIVIAL(fatal) << exception.what();
#endif // ENABLE_LOGGING
        break;
      }

      for(std::vector<arl::hms::ArgumentPointer>::size_type i=0; 
          i<arguments.size(); ++i) {
        
        arl::hms::ArgumentPointer argument = arguments[i];

        //
        // cast argument to ScalarArgument
        //
        boost::shared_ptr<arl::hms::ScalarArgument> scalarArgument = 
          boost::dynamic_pointer_cast<arl::hms::ScalarArgument>(argument);
        
        //
        // compute value = value*value
        //
        const double argumentValue = scalarArgument->getArgument();
        const double value = argumentValue * argumentValue;
        arl::hms::ValuePointer sendValue(new arl::hms::ScalarValue(value));
        
        //
        // send back value*value
        //
        communicator->send(sendValue);
      }
    }
  }
  else {

    //
    // send ints to server
    //
    for(int i=0; i<10; ++i) {
      arl::hms::ArgumentPointer argument(new arl::hms::ScalarArgument(i));
      communicator->send(argument);
    }

    int counter = 0;
    while (true) {
    
      std::vector<arl::hms::ValuePointer > values = 
        communicator->receive<arl::hms::ValuePointer >();

      for(std::vector<arl::hms::ValuePointer>::size_type i=0; 
          i<values.size(); ++i) {
        
        boost::shared_ptr<arl::hms::ScalarValue> value = 
          boost::dynamic_pointer_cast<arl::hms::ScalarValue>(values[i]);
        
        std::cout << "Received: " << value->getValue() << std::endl;
      
      }
      
      counter += values.size();
      if(counter == 10) {
        break;
      }
      
    }

  }
  
  //
  // cleanup
  //
  delete communicator;

  //
  //
  //
  return 0;

}
