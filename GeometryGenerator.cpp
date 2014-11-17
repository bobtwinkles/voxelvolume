#include "GeometryGenerator.hpp"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "RenderChunk.hpp"

typedef struct packet {
  uint32_t start;
  uint32_t len;
} packet_t;

static int parent_sock;
static int worker_sock;

static void DoWork(srp::DataStore *);

int srp::StartGeometryGenerator(srp::DataStore * DS) {
  int sockets[2];
  int err = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sockets);
  pthread_t thread;
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

template <typename T>
static void SendData(uint32_t start, std::vector<T> & data) {
  packet_t send;
  uint32_t send_size = data.size() - start;
  send.start = start;
  send.len = send_size;
  // TODO: error handling
  write(worker_sock, &send, sizeof(send));
  write(worker_sock, data.data() + start, send_size * sizeof(T));
}

template <typename T>
static void ReadData(std::vector<T> & data) {
  packet_t get;
  T * gotten;
  read(parent_sock, &get, sizeof(get));
  gotten = new T[get.len];

  {
    void * temp = gotten;
    size_t amount_read;
    size_t goal_read = get.len * sizeof(T);
    while (amount_read < goal_read) {
      int read_return = read(parent_sock, temp, goal_read - amount_read);
      if (read_return < 0) {
        std::cerr << "Read failed!" << std::endl;
        perror("read");
        BUG();
      }
      amount_read += read_return;
    }
  }

  data.insert(data.end(), gotten, gotten + get.len);
  delete[] gotten;
}

static void DoWork(srp::DataStore * dstore) {
  int input[4];
  uint32_t index_start;
  uint32_t verts_start;
  srp::IndexCache cache;
  std::vector<srp::ogl::Vertex> verts;
  std::vector<GLuint> indicies;

  while (true) {
    int amount_read = 0;
    while (amount_read < sizeof(input)) {
      int err = read(worker_sock, input, amount_read - sizeof(input));
      if (err < 0) {
        perror("Read failed");
        BUG();
      }
    }

    for (int i = 0; i < 3; ++i) {
      input[i] = ntohl(input[i]);
    }

    verts_start = verts.size();
    index_start = indicies.size();

    srp::RenderChunk(*dstore, input[0], input[1], input[2], input[3], cache, indicies, verts);

    SendData(verts_start, verts);
    SendData(index_start, indicies);
  }
}
