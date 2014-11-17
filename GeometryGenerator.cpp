#include "GeometryGenerator.hpp"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>

#include "RenderChunk.hpp"

typedef struct packet {
  uint32_t start;
  uint32_t len;
} packet_t;

static int parent_sock;
static int worker_sock;

static void DoWork(srp::DataStore *);
template <typename T>
static void ReadData(packet_t & Get, std::vector<T> & data);
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
    DoWork(DS);
    return -1;
  } else {
    close(worker_sock);
    return parent_sock;
  }
}

bool srp::ReadGeometry(std::vector<GLuint> & Indicies, std::vector<srp::ogl::Vertex> & Verts) {
  packet_t get;
  if (!xread(parent_sock, &get, sizeof(get))) {
    return false;
  }
  ReadData(get, Verts);

  while (!xread(parent_sock, &get, sizeof(get)));
  ReadData(get, Indicies);
  return true;
}

void srp::RequestChunk(int X, int Y, int Z, int Threshould) {
  int data[4];
  int err;
  data[0] = htonl(X);
  data[1] = htonl(Y);
  data[2] = htonl(Z);
  data[3] = htonl(Threshould);
  std::cout << "Requesting chunk: " << X << " " <<  Y << " " << Z << std::endl;
  std::cout << "asdf:" << sizeof(int) << std::endl;
  xwrite(parent_sock, data, 4 * sizeof(int));
}

template <typename T>
static void SendData(uint32_t start, std::vector<T> & data) {
  packet_t send;
  uint32_t send_size = data.size() - start;
  send.start = start;
  send.len = send_size;
  // TODO: error handling
  xwrite(worker_sock, &send, sizeof(send));
  xwrite(worker_sock, data.data() + start, send_size * sizeof(T));
}

template <typename T>
static void ReadData(packet_t & Get, std::vector<T> & data) {
  T * gotten;
  gotten = new T[Get.len];

  while (!xread(parent_sock, gotten, Get.len * sizeof(T)));

  data.insert(data.end(), gotten, gotten + Get.len);
  delete[] gotten;
}

static void DoWork(srp::DataStore * dstore) {
  int input[4];
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
    std::cout << "Watiing for data" << std::endl;
    err = select(2, &watch, NULL, NULL, NULL);
    if (err < 0) {
      if (errno != EAGAIN) {
        break;
      } else {
        continue;
      }
    }
    std::cout << "got some data" << std::endl;
    int amount_read = 0;
    while (!xread(worker_sock, input, sizeof(input)));

    for (int i = 0; i < 3; ++i) {
      input[i] = ntohl(input[i]);
    }

    std::cout << "[SLV] Got request for chunk " << input[0] << " " << input[1]
                                         << " " << input[2] << " at th " << input[3] << std::endl;

    verts_start = verts.size();
    index_start = indicies.size();

    srp::RenderChunk(*dstore, input[0], input[1], input[2], input[3], cache, indicies, verts);

    SendData(verts_start, verts);
    SendData(index_start, indicies);
  }

  perror("select died somehow");
  BUG();
}

static bool xread(int fd, void * data, size_t amount) {
  size_t amount_read = 0;
  int i = read(fd, data, amount);
  char * real_data = (char*)data;
  if (i < 0 && errno == EAGAIN) {
    return false;
  } else {
    perror("read failed");
    BUG();
  }
  amount_read += i;
  real_data += i;
  while (amount_read < amount) {
    i = read(fd, real_data, amount - amount_read);
    if (i < 0 && errno != EAGAIN) {
      perror("read failed");
      BUG();
    }
    real_data += i;
    amount_read += i;
  }
  return true;
}

static void xwrite(int fd, void * data, size_t amount) {
  size_t amount_left = amount;
  char * real_data = (char*)data;
  std::cerr << std::dec << "Writing " << amount << " bytes to " << fd << std::endl;
  do {
    int i = write(fd, real_data, amount_left);
    if (i < 0 && errno != EAGAIN) {
      perror("write failed");
      BUG();
    }
    amount_left -= i;
    real_data += i;
    std::cerr << " [" << fd << "] Wrote " << i << " bytes" << std::endl;
  } while (amount_left > 0);
}
