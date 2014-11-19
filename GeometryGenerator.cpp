#include "GeometryGenerator.hpp"

#include <arpa/inet.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "RenderChunk.hpp"

namespace packet {
  typedef struct response {
    uint32_t threshold;
    uint32_t verts_length;
    uint32_t indicies_length;
  } response_t;

  typedef struct request {
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t threshold;
  } request_t;
}

static int parent_sock;
static int worker_sock;

static void DoWork(srp::DataStore *);
static bool xread(int fd, void * data, size_t amount);
static void xwrite(int fd, void * data, size_t amount);

int srp::StartGeometryGenerator(srp::DataStore * DS) {
  int sockets[2];
  int err = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sockets);
  if (err < 0) {
    std::cerr << "Failed to allocate a socket pair!" << std::endl;
    BUG();
  }
  parent_sock = sockets[0];
  worker_sock = sockets[1];

  int f = fork();
  if (f < 0) {
    perror("Fork failed");
    BUG();
  }
  if (f == 0) {
    close(parent_sock);
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    DoWork(DS);
    return -1;
  } else {
    close(worker_sock);
    return parent_sock;
  }
}

bool srp::ReadGeometry(std::vector<GLuint> & Indicies, std::vector<srp::ogl::Vertex> & Verts, unsigned int Threshold) {
  packet::response_t response;
  if (!xread(parent_sock, &response, sizeof(response))) {
    return false;
  }

  response.verts_length = ntohl(response.verts_length);
  response.indicies_length = ntohl(response.indicies_length);
  response.threshold = ntohl(response.threshold);

  int indicies_start = Indicies.size();
  int verts_start = Verts.size();

  Verts.resize(verts_start + response.verts_length);
  while (!xread(parent_sock, Verts.data() + verts_start, response.verts_length * sizeof(srp::ogl::Vertex)));
  Indicies.resize(indicies_start + response.verts_length);
  while (!xread(parent_sock, Indicies.data() + indicies_start, response.indicies_length * sizeof(GLuint)));

  if (response.threshold != Threshold) {
    Verts.resize(verts_start);
    Indicies.resize(indicies_start);
    return false;
  }

  return true;
}

void srp::RequestChunk(int X, int Y, int Z, int Threshould) {
  packet::request_t request;
  int err;
  request.x = htonl(X);
  request.y = htonl(Y);
  request.z = htonl(Z);
  request.threshold = htonl(Threshould);
  //std::cout << "Requesting chunk: " << X << " " <<  Y << " " << Z << " at threshold " << Threshould << std::endl;
  xwrite(parent_sock, &request, sizeof(request));
}

static void DoWork(srp::DataStore * dstore) {
  packet::request_t request;
  packet::response_t response;
  uint32_t index_start;
  uint32_t verts_start;
  srp::IndexCache cache;
  std::vector<srp::ogl::Vertex> verts;
  std::vector<GLuint> indicies;
  fd_set watch;

  FD_ZERO(&watch);
  FD_SET(worker_sock, &watch);

  int err = 0;

  std::cout << "Starting geometry request loop" << std::endl;
  while (true) {
    //std::cout << "Watiing for data" << std::endl;
    err = select(worker_sock + 1, &watch, NULL, NULL, NULL);
    if (err < 0) {
      if (errno != EAGAIN) {
        break;
      } else {
        continue;
      }
    }
    while (!xread(worker_sock, &request, sizeof(request)));

    request.x = ntohl(request.x);
    request.y = ntohl(request.y);
    request.z = ntohl(request.z);
    request.threshold = ntohl(request.threshold);
    //std::cout << "[SLV] Got request for chunk " << input[0] << " " << input[1]
    //                                     << " " << input[2] << " at th " << input[3] << std::endl;

    if (request.x == 0 && request.y == 0 && request.z == 0) {
      verts.clear();
      indicies.clear();
      cache.clear();
    }

    verts_start = verts.size();
    index_start = indicies.size();

    srp::RenderChunk(*dstore, request.x, request.y, request.z, request.threshold, cache, indicies, verts);

    response.threshold = htonl(request.threshold);
    response.verts_length     = htonl(verts.size() - verts_start);
    response.indicies_length  = htonl(indicies.size() - index_start);
    xwrite(worker_sock, &response, sizeof(response));
    // Unnetwork it so we can refer to these lengths
    response.verts_length = ntohl(response.verts_length);
    response.indicies_length = ntohl(response.indicies_length);
    // Write the data
    std::cout << "v: " << verts.size() << " " << " s: " << indicies.size() << std::endl;
    xwrite(worker_sock, verts.data() + response.verts_length, response.verts_length * sizeof(srp::ogl::Vertex));
    xwrite(worker_sock, indicies.data() + response.indicies_length, response.indicies_length * sizeof(GLuint));
  }

  perror("select died somehow");
  BUG();
}

static bool xread(int fd, void * data, size_t amount) {
  size_t amount_read = 0;
  int i = read(fd, data, amount);
  char * real_data = (char*)data;
  //std::cerr << "Reading " << amount << " bytes from " << fd << std::endl;
  if (i < 0) {
    if (errno == EAGAIN) {
      //std::cerr << " [" << fd << "] There was no data availible" << std::endl;
      return false;
    } else {
      perror("read failed");
      BUG();
    }
  }
  //std::cerr << " [" << fd << "] Read " << i << std::endl;
  amount_read += i;
  real_data += i;
  while (amount_read < amount) {
    i = read(fd, real_data, amount - amount_read);
    if (i < 0) {
      if (errno == EAGAIN) {
        continue;
      } else {
        perror("read failed");
        BUG();
      }
    }
    //std::cerr << " [" << fd << "] Read " << i << std::endl;
    real_data += i;
    amount_read += i;
  }
  return true;
}

static void xwrite(int fd, void * data, size_t amount) {
  size_t amount_left = amount;
  char * real_data = (char*)data;
  if (amount == 0) { return; } // Nothing to do, there is no data to write
  std::cerr << std::dec << "Writing " << amount << " bytes to " << fd << std::endl;
  do {
    int i = write(fd, real_data, amount_left);
    if (i < 0) {
      if (errno != EAGAIN) {
        perror("write failed");
        BUG();
      } else {
        continue;
      }
    }
    amount_left -= i;
    real_data += i;
    //std::cerr << " [" << fd << "] Wrote " << i << " bytes" << std::endl;
  } while (amount_left > 0);
}
