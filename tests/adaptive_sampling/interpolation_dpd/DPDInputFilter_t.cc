//
// File:      DPDInputFilter_t.cc
// Package    base
//
// Base classes for HMS.
//

#if defined(HAVE_BOOST_SERIALIZATION)
#include <boost/serialization/base_object.hpp>
#else
#error boost serialization not available
#endif // HAVE_BOOST_SERIALIZATION

//
//
//

namespace arl {
  namespace hms {

    //
    // Serialize DpdInputFilter.
    //
    template <class Archive>
    void 
    DPDInputFilter::serialize(Archive & archive, const unsigned int)
    {

      //
      // serialize base class
      //
      archive & boost::serialization::base_object<InterpolationDatabaseInputFilter>(*this);

      //
      //
      //
      return;
      
    }
  
  }
}
