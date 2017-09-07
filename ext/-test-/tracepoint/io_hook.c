#include "ruby/ruby.h"
#include "ruby/debug.h"

struct io_events_track {
    size_t open_count;
    size_t read_count;
    size_t write_count;
    size_t close_count;
    VALUE ios;
};

struct socket_events_track {
    size_t sockets_count;
    size_t socketpairs_count;
    size_t bind_count;
    size_t listen_count;
    size_t connect_count;
    size_t accept_count;
    size_t gethostname_count;
    size_t shutdown_count;
    size_t gethostbyaddr_count;
    size_t gethostbyname_count;
    size_t getservbyname_count;
    size_t getservbyport_count;
    size_t getaddrinfo_count;
    VALUE ios;
};

static void
rb_io_events_i(VALUE tpval, void *data)
{
  rb_trace_arg_t *tparg = rb_tracearg_from_tracepoint(tpval);
  struct io_events_track *track = data;

  VALUE io = rb_tracearg_self(tparg);
  rb_event_io_data_t *evt_data = (rb_event_io_data_t*)rb_tracearg_return_value(tparg);
  switch(evt_data->flag){
    case RUBY_EVENT_IO_OPEN:
         track->open_count++;
         printf("RUBY_EVENT_IO_OPEN path: %s mode: %d flags: %d, ret: %zd\n", evt_data->open.path, evt_data->open.flags, evt_data->open.mode, evt_data->open.ret);
         break;
    case RUBY_EVENT_IO_READ:
         track->read_count++;
         printf("RUBY_EVENT_IO_READ fd: %d capa: %zd ret: %zd\n", evt_data->read.fd, evt_data->read.capa, evt_data->read.ret);
         break;
    case RUBY_EVENT_IO_WRITE:
         track->write_count++;
         printf("RUBY_EVENT_IO_WRITE fd: %d capa: %zd ret: %zd\n", evt_data->read.fd, evt_data->read.capa, evt_data->read.ret);
         break;
    case RUBY_EVENT_IO_CLOSE:
         track->close_count++;
         printf("RUBY_EVENT_IO_CLOSE fd: %d ret: %d\n", evt_data->close.fd, evt_data->close.ret);
         break;
  }
  rb_hash_aset(track->ios, io, INT2NUM(evt_data->fd));
}

static void
rb_socket_events_i(VALUE tpval, void *data)
{
  rb_trace_arg_t *tparg = rb_tracearg_from_tracepoint(tpval);
  struct socket_events_track *track = data;

  VALUE io = rb_tracearg_self(tparg);
  rb_event_io_data_t *evt_data = (rb_event_io_data_t*)rb_tracearg_return_value(tparg);
  switch(evt_data->flag){
    case RUBY_EVENT_IO_SOCKET:
         track->sockets_count++;
         printf("RUBY_EVENT_IO_SOCKET domain: %d type: %d protocol: %d ret: %zd\n", evt_data->socket.domain, evt_data->socket.type, evt_data->socket.protocol, evt_data->socket.ret);
         break;
     case RUBY_EVENT_IO_SOCKETPAIR:
          track->socketpairs_count++;
          printf("RUBY_EVENT_IO_SOCKETPAIR domain: %d type: %d protocol: %d ret: %zd fd1: %d fd2: %d\n", evt_data->socketpair.domain, evt_data->socketpair.type, evt_data->socketpair.protocol, evt_data->socketpair.ret, evt_data->socketpair.fds[0], evt_data->socketpair.fds[1]);
          break;
    case RUBY_EVENT_IO_SOCKET_BIND:
         track->bind_count++;
         printf("RUBY_EVENT_IO_SOCKET_BIND fd: %d addr: %s ret: %zd\n", evt_data->bind.fd, "test", evt_data->bind.ret);
         break;
    case RUBY_EVENT_IO_SOCKET_LISTEN:
         track->listen_count++;
         printf("RUBY_EVENT_IO_SOCKET_LISTEN fd: %d backlog: %d ret: %zd\n", evt_data->listen.fd, evt_data->listen.backlog, evt_data->listen.ret);
         break;
    case RUBY_EVENT_IO_SOCKET_CONNECT:
         track->connect_count++;
         printf("RUBY_EVENT_IO_SOCKET_CONNECT fd: %d addr: %s ret: %zd\n", evt_data->connect.fd, "test", evt_data->connect.ret);
         break;
    case RUBY_EVENT_IO_SOCKET_ACCEPT:
         track->accept_count++;
         printf("RUBY_EVENT_IO_SOCKET_ACCEPT fd: %d ret: %d\n", evt_data->accept.fd, evt_data->accept.ret);
         break;
    case RUBY_EVENT_IO_SOCKET_GETHOSTNAME:
         track->gethostname_count++;
         printf("RUBY_EVENT_IO_GETHOSTNAME host: %s\n", evt_data->gethostname.host);
         break;
    case RUBY_EVENT_IO_SOCKET_SHUTDOWN:
         track->shutdown_count++;
         printf("RUBY_EVENT_IO_SOCKET_SHUTDOWN fd: %d how: %d ret: %d\n", evt_data->shutdown.fd, evt_data->shutdown.how, evt_data->shutdown.ret);
         break;
    case RUBY_EVENT_IO_SOCKET_GETHOSTBYADDR:
         track->gethostbyaddr_count++;
         printf("RUBY_EVENT_IO_GETHOSTBYADDR addr: %s ret: %s\n", evt_data->gethostbyaddr.addr, evt_data->gethostbyaddr.ret);
         break;
    case RUBY_EVENT_IO_SOCKET_GETHOSTBYNAME:
         track->gethostbyname_count++;
         printf("RUBY_EVENT_IO_GETHOSTBYNAME host: %s ret: %s\n", evt_data->gethostbyname.host, evt_data->gethostbyname.ret);
         break;
    case RUBY_EVENT_IO_SOCKET_GETSERVBYNAME:
         track->getservbyname_count++;
         printf("RUBY_EVENT_IO_GETSERVBYNAME service: %s protocol: %s ret: %d\n", evt_data->getservbyname.service, evt_data->getservbyname.protocol, evt_data->getservbyname.ret);
         break;
    case RUBY_EVENT_IO_SOCKET_GETSERVBYPORT:
         track->getservbyport_count++;
         printf("RUBY_EVENT_IO_GETSERVBYPORT port: %ld protocol: %s ret: %s\n", evt_data->getservbyport.port, evt_data->getservbyport.protocol, evt_data->getservbyport.ret);
         break;
    case RUBY_EVENT_IO_SOCKET_GETADDRINFO:
         track->getaddrinfo_count++;
         printf("RUBY_EVENT_IO_GETADDRINFO node: %s service: %s family: %d socktype: %d protocol: %d flags: %d\n", evt_data->getaddrinfo.node, evt_data->getaddrinfo.service, evt_data->getaddrinfo.family, evt_data->getaddrinfo.socktype, evt_data->getaddrinfo.protocol, evt_data->getaddrinfo.flags);
         break;
/*
    case RUBY_EVENT_IO_SOCKET_GETHOSTBYNAME:
         track->gethostbyname_count++;
         printf("RUBY_EVENT_IO_SOCKET_GETHOSTBYNAME domain: %d type: %d protocol: %d addr: %s ret: %zd fd: %d\n", evt_data->socket.domain, evt_data->socket.type, evt_data->socket.protocol, evt_data->socket.addr, evt_data->ret, evt_data->fd);
         break;
*/
  }
  rb_hash_aset(track->ios, io, INT2NUM(evt_data->fd));
}

static VALUE
rb_track_file_io(VALUE self)
{
    struct io_events_track track = {0, 0, 0, 0, rb_hash_new()};
    VALUE tpval = rb_tracepoint_new(0, RUBY_EVENT_IO, rb_io_events_i, &track);
    VALUE result = rb_ary_new();

    rb_tracepoint_enable(tpval);
    rb_ensure(rb_yield, Qundef, rb_tracepoint_disable, tpval);

    rb_ary_push(result, SIZET2NUM(track.open_count));
    rb_ary_push(result, SIZET2NUM(track.read_count));
    rb_ary_push(result, SIZET2NUM(track.write_count));
    rb_ary_push(result, SIZET2NUM(track.close_count));

    return result;
}

static VALUE
rb_track_socket_io(VALUE self)
{
    struct socket_events_track track = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, rb_hash_new()};
    VALUE tpval = rb_tracepoint_new(0, RUBY_EVENT_IO, rb_socket_events_i, &track);
    VALUE result = rb_ary_new();

    rb_tracepoint_enable(tpval);
    rb_ensure(rb_yield, Qundef, rb_tracepoint_disable, tpval);

    rb_ary_push(result, SIZET2NUM(track.sockets_count));
    rb_ary_push(result, SIZET2NUM(track.bind_count));
    rb_ary_push(result, SIZET2NUM(track.listen_count));
    rb_ary_push(result, SIZET2NUM(track.connect_count));
    rb_ary_push(result, SIZET2NUM(track.accept_count));
    rb_ary_push(result, SIZET2NUM(track.shutdown_count));
    rb_ary_push(result, SIZET2NUM(track.gethostbyname_count));

    return result;
}

void
Init_io_hook(void)
{
    VALUE mIOHook = rb_define_module("IOHook");
    rb_define_module_function(mIOHook, "track_file_io", rb_track_file_io, 0);
    rb_define_module_function(mIOHook, "track_socket_io", rb_track_socket_io, 0);
}