#ifndef _METRIC_H_
#define _METRIC_H_

#include <time.h>

#include "Util.hpp"

namespace srp {
  namespace metric {
    class Metric {
      private:
        struct timespec _start;
        long * _samples;
        long _min, _max;
        unsigned int _current_sample;
        unsigned int _num_samples;
        bool _running;
        float _average, _stddev;

        DISALLOW_COPY_AND_ASSIGN(Metric);
      public:
        Metric(unsigned int NumSamples);
        ~Metric();

        void Enter();
        void Leave();

        float GetAverage() const;
        float GetStandardDeviation() const;
    };
  }
}

#endif //_METRIC_H_
