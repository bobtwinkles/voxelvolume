#ifndef _METRIC_H_
#define _METRIC_H_

#include <time.h>

#include "Util.hpp"

namespace srp {
  namespace ogl {
    namespace ui {
      class MetricGraph;
    }
  }
  namespace metric {
    class GPUMetric {
      private:
        friend srp::ogl::ui::MetricGraph;
        struct timespec _start;
        long * _samples;
        long _min, _max;
        long _seeing_max;
        unsigned int _current_sample;
        unsigned int _num_samples;
        unsigned int _total_samples;
        unsigned int _seeing_max_sample;
        bool _running;
        float _average, _stddev, _continous_average;

        DISALLOW_COPY_AND_ASSIGN(GPUMetric);
      public:
        GPUMetric(unsigned int NumSamples);
        ~GPUMetric();

        void Enter();
        void Leave();

        void Reset();

        float GetAverage() const { return _continous_average; };
        float GetStandardDeviation() const;
        inline long GetMin() const { return _min; }
        inline long GetMax() const { return _max; }
        inline unsigned int GetCurrent() const { return _current_sample; }
    };
  }
}

#endif //_METRIC_H_
