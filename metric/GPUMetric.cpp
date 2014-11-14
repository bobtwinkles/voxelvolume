#include "GPUMetric.hpp"

#include <limits.h>
#include <math.h>

using srp::metric::GPUMetric;

GPUMetric::GPUMetric(unsigned int NumSamples) {
  _min = LONG_MAX;
  _max = 0;
  _current_sample = 0;
  _num_samples = NumSamples;
  _running = false;
  _finalized = true;
  _average = 0;
  _stddev = 0;
  _samples = new long[_num_samples];
  for (auto i = 0; i < _num_samples; ++i) {
    _samples[i] = 0;
  }
  _total_samples = 0;
  _continous_average = 0;

  glGenQueries(2, _queries);
}

GPUMetric::~GPUMetric() {
  delete[] _samples;
}

void GPUMetric::Reset() {
  if (_running) {
    std::cerr << "Tried to reset a running metric" << std::endl;
    BUG();
  }

  _min = LONG_MAX;
  _max = 0;
  for (auto i = 0; i < _num_samples; ++i) {
    _samples[i] = 0;
  }
  _current_sample = 0;
  _total_samples = 0;
}

void GPUMetric::Enter() {
  if (!_finalized) {
    std::cerr << "Tried to start a metric without finalizing it!" << std::endl;
    BUG();
  }
  if (_running) {
    std::cerr << "Tried to start an already running metric!" << std::endl;
    BUG();
  }
  glQueryCounter(_queries[0], GL_TIMESTAMP);
  _running = true;
}

void GPUMetric::Leave() {
  if (!_running) {
    std::cerr << "Tried to leave a metric that isn't running!" << std::endl;
    BUG();
  }
  glQueryCounter(_queries[1], GL_TIMESTAMP);
  _running = false;
  _finalized = false;
}

void GPUMetric::Finalize() {
  if (_finalized) {
    std::cerr << "Tried to finalize twice!" << std::endl;
    BUG();
  }
  if (_running) {
    std::cerr << "Tried to finalize running timer!" << std::endl;
    BUG();
  }
  long start, end, elapsed;
  GLint availible;

  do {
    glGetQueryObjectiv(_queries[1], GL_QUERY_RESULT_AVAILABLE, &availible);
  } while (!availible);

  glGetQueryObjecti64v(_queries[0], GL_QUERY_RESULT, &start);
  glGetQueryObjecti64v(_queries[1], GL_QUERY_RESULT, &end);

  elapsed = end - start;
  _current_sample = (_current_sample + 1) % _num_samples;
  _samples[_current_sample] = elapsed;

  if (_seeing_max_sample == _current_sample) {
    _seeing_max = 0;
  }

  if (elapsed < _min) {
    _min = elapsed;
  }
  if (elapsed > _max) {
    _max = elapsed;
    _seeing_max = elapsed;
    _seeing_max_sample = _current_sample;
  }

  if (_continous_average == 0) {
    _continous_average = elapsed;
  } else {
    _continous_average = ( (_continous_average * _total_samples) + elapsed ) / float(_total_samples + 1);
  }
  _total_samples += 1;

  _average = 0;
  for (auto i = 0; i < _num_samples; ++i) {
    long samp = _samples[i];
    _average += samp;
    if (samp > _seeing_max) {
      _seeing_max = samp;
      _seeing_max_sample = i;
    }
  }
  _average /= float(_num_samples);

  _stddev = 0;
  for (auto i = 0; i < _num_samples; ++i) {
    float tmp = (_samples[i] - _average);
    _stddev += tmp * tmp;
  }
  _stddev /= float(_num_samples) - 1;
  _stddev = sqrtf(_stddev);

  if (_max > _seeing_max) {
    _max = long(0.99 * (_max - _seeing_max) + _seeing_max);
  }

  _finalized = true;
}

float GPUMetric::GetStandardDeviation() const {
  return _stddev;
}
