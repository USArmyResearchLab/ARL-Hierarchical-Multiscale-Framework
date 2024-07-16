//
// File:      DPDArgument_t.cc
// Package    base
//
// Base classes for HMS.
//

#if defined(HAVE_BOOST_SERIALIZATION)
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/vector.hpp>
#else
#error boost serialization not available
#endif // HAVE_BOOST_SERIALIZATION

//
//
//

namespace arl {
  namespace hms {

    //
    // Serialize value.
    //
    template <class Archive>
    void
    DPDValue::serialize(Archive & archive, const unsigned int)
    {

      //
      // serialize base class
      //
      archive & boost::serialization::base_object<Value>(*this);

      //
      // serialize temperature and pressure
      //
      archive & d_temperature;
      archive & d_pressure;

      //
      //
      //
      return;

    }

  }
}

