//
// Basic Monitor Test.
//
#if defined(HAVE_CONFIG_H)
#include "arlhms_config.h"
#endif // HAVE_CONFIG_H

#include <base/OutputFilter.h>

//
//
//

namespace arl {
  namespace hms {

    /**
     * @brief Class implementing perf output filter.
     */
    class PerfOutputFilter : public OutputFilter {

      //
      // types
      //
    public:

      //
      // methods
      //
    public:

      /**
       * Constructor.
       */
      PerfOutputFilter();

      /**
       * Destructor.
       */
      virtual ~PerfOutputFilter();

      /**
       * Apply PerfOutputFilter
       *
       * @param directory Directory path where Model was run.
       * @param stdOut Path containing stdout of Model execution.
       * @param argument Argument of the Model.
       */
      virtual ValuePointer 
      apply(const std::string     & directory, 
            const std::string     & stdOut,
	    const ArgumentPointer & argument) const;

      /**
       * Serialize value.
       *
       * @param archive Archive to store serialization.
       * @param version Version of serialization.
       */
      template <class Archive>
      void serialize(Archive & archive, const unsigned int version);

      //
      // methods
      // 
    protected:

    private:
      //
      // copy constructor/assignment operator
      //
      PerfOutputFilter(const PerfOutputFilter &); // not implemented
      PerfOutputFilter & operator=(const PerfOutputFilter &); // not implemented

      //
      // data
      //
    private:

    };

  }
}

#include "PerfOutputFilter_t.cc"
