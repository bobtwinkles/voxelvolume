#include "Graph.hpp"

#include "GPUMetric.hpp"
#include <math.h>

using srp::ogl::ui::MetricGraph;

#define METRICGRAPH_SCALE (_metric._max - _metric._min)

MetricGraph::MetricGraph(srp::metric::GPUMetric & Metric, float X, float Y, float Width, float Height) : _metric(Metric) {
  _num_points = _metric._num_samples;

  glGenVertexArrays(1, &_vao);
  glGenBuffers(1, &_data_buffer);

  _x = X;
  _y = Y;
  _width = Width;
  _height = Height;
  _verts = new Vertex[_num_points + _NUM_EXTRA_POINTS];

  unsigned int n0 = _num_points;
  unsigned int n1 = _num_points + 1;
  unsigned int n2 = _num_points + 2;

  for (auto i = _num_points; i < _num_points + _NUM_EXTRA_POINTS; ++i) {
    _verts[i].r = 1;
    _verts[i].g = 1;
    _verts[i].b = 1;
    _verts[i].a = 1;
  }
  _verts[n0].x = _x + _width;
  _verts[n0].y = _y + _height;

  _verts[n1].x = _x + _width;
  _verts[n1].y = _y;

  _verts[n2].x = _x;
  _verts[n2].y = _y;

  _verts[_num_points + _STDDEV_LINE_OFFSET + 0].x = _x;
  _verts[_num_points + _STDDEV_LINE_OFFSET + 1].x = _x + _width;
  _verts[_num_points + _STDDEV_LINE_OFFSET + 2].x = _x;
  _verts[_num_points + _STDDEV_LINE_OFFSET + 3].x = _x + _width;

  for (auto i = _num_points + _STDDEV_LINE_OFFSET; i < _num_points + _NUM_EXTRA_POINTS; ++i) {
    _verts[i].r = float((i - (_num_points + _STDDEV_LINE_OFFSET)) / 2);
    _verts[i].g = 1 - _verts[i].r;
    _verts[i].b = 0;
  }
}

MetricGraph::~MetricGraph() {
  glDeleteBuffers(1, &_data_buffer);
  glDeleteVertexArrays(1, &_vao);
}

float MetricGraph::GetSampleHeight(long Sample) const {
  float Offset = float(Sample) - _metric._min;
  Offset /= METRICGRAPH_SCALE;
  return Offset * _height;
}

void MetricGraph::Render(srp::RenderState & State) const {
  unsigned int w, h;
  for (auto i = 0; i < _num_points; ++i) {
    _verts[i].x = _x + (i / float(_num_points)) * _width;
    if (i == _num_points - 1) {
      _verts[i].x = _x + _width;
    }
    _verts[i].y = _y + GetSampleHeight(_metric._samples[(i + _metric.GetCurrent()) % _num_points]);
    _verts[i].r = 1;
    _verts[i].g = 1;
    _verts[i].b = 1;
    _verts[i].a = 1;
  }

  _verts[_num_points + _STDDEV_LINE_OFFSET + 0].y = _y + GetSampleHeight(long(_metric._average + _metric._stddev));
  _verts[_num_points + _STDDEV_LINE_OFFSET + 1].y = _y + GetSampleHeight(long(_metric._average + _metric._stddev));
  _verts[_num_points + _STDDEV_LINE_OFFSET + 2].y = _y + GetSampleHeight(long(_metric._average - _metric._stddev));
  _verts[_num_points + _STDDEV_LINE_OFFSET + 3].y = _y + GetSampleHeight(long(_metric._average - _metric._stddev));

  glBindVertexArray(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _data_buffer);
  glBufferData(GL_ARRAY_BUFFER, (_num_points + _NUM_EXTRA_POINTS) * sizeof(Vertex), _verts, GL_DYNAMIC_DRAW);

  UIGetWindow()->GetGeometry(&w, &h);
  glm::mat4 trans = glm::ortho(0.f, (float)w, 0.f, (float)h, 10.f, -10.f);

  srp::ogl::ShaderProgram * cs = State.GetCurrentShaderProgram();

  if (cs != GetGraphShader()) {
    cs = GetGraphShader();
    cs->Bind();
    State.SetCurrentShader(cs);
  }

  cs->Upload("transform", trans);
  cs->Upload("x_min", _x);
  cs->Upload("x_fade_start", _x + _width * 0.5f);

  GLint position = cs->GetAttributeLocation("position");
  GLint color    = cs->GetAttributeLocation("color");

  if (position >= 0) {
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 2, GL_FLOAT, false, sizeof(Vertex), 0);
  }
  if (color >= 0) {
    glEnableVertexAttribArray(color);
    glVertexAttribPointer(color, 4, GL_FLOAT, false, sizeof(Vertex), (void*)(2 * sizeof(float)));
  }

  glDrawArrays(GL_LINE_LOOP, 0, _num_points + _STDDEV_LINE_OFFSET);
  glDrawArrays(GL_LINES, _num_points + _STDDEV_LINE_OFFSET, 4);

  if (position >= 0) {
    glDisableVertexAttribArray(position);
  }
  if (color >= 0) {
    glDisableVertexAttribArray(color);
  }
}

