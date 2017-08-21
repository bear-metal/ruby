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
    size_t bind_count;
    size_t listen_count;
    size_t connect_count;
    size_t accept_count;
    size_t option_count;
    size_t shutdown_count;
    size_t gethostbyname_count;
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
         printf("RUBY_EVENT_IO_OPEN name: %s mode: %d xfer: %zd fd: %d\n", evt_data->file.name, evt_data->file.mode, evt_data->bytes_transferred, evt_data->fd);
         break;
    case RUBY_EVENT_IO_READ:
         track->read_count++;
         printf("RUBY_EVENT_IO_READ name: %s mode: %d xfer: %zd fd: %d\n", evt_data->file.name, evt_data->file.mode, evt_data->bytes_transferred, evt_data->fd);
         break;
    case RUBY_EVENT_IO_WRITE:
         track->write_count++;
         printf("RUBY_EVENT_IO_WRITE name: %s mode: %d xfer: %zd fd: %d\n", evt_data->file.name, evt_data->file.mode, evt_data->bytes_transferred, evt_data->fd);
         break;
    case RUBY_EVENT_IO_CLOSE:
         track->close_count++;
         printf("RUBY_EVENT_IO_CLOSE name: %s mode: %d xfer: %zd fd: %d\n", evt_data->file.name, evt_data->file.mode, evt_data->bytes_transferred, evt_data->fd);
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
         printf("RUBY_EVENT_IO_SOCKET domain: %d type: %d protocol: %d addr: %s xfer: %zd fd: %d\n", evt_data->socket.domain, evt_data->socket.type, evt_data->socket.protocol, evt_data->socket.addr, evt_data->bytes_transferred, evt_data->fd);
         break;
    case RUBY_EVENT_IO_SOCKET_BIND:
         track->bind_count++;
         printf("RUBY_EVENT_IO_SOCKET_BIND domain: %d type: %d protocol: %d addr: %s xfer: %zd fd: %d\n", evt_data->socket.domain, evt_data->socket.type, evt_data->socket.protocol, evt_data->socket.addr, evt_data->bytes_transferred, evt_data->fd);
         break;
    case RUBY_EVENT_IO_SOCKET_LISTEN:
         track->listen_count++;
         printf("RUBY_EVENT_IO_SOCKET_LISTEN domain: %d type: %d protocol: %d addr: %s xfer: %zd fd: %d\n", evt_data->socket.domain, evt_data->socket.type, evt_data->socket.protocol, evt_data->socket.addr, evt_data->bytes_transferred, evt_data->fd);
         break;
    case RUBY_EVENT_IO_SOCKET_CONNECT:
         track->connect_count++;
         printf("RUBY_EVENT_IO_SOCKET_CONNECT domain: %d type: %d protocol: %d addr: %s xfer: %zd fd: %d\n", evt_data->socket.domain, evt_data->socket.type, evt_data->socket.protocol, evt_data->socket.addr, evt_data->bytes_transferred, evt_data->fd);
         break;
    case RUBY_EVENT_IO_SOCKET_ACCEPT:
         track->accept_count++;
         printf("RUBY_EVENT_IO_SOCKET_ACCEPT domain: %d type: %d protocol: %d addr: %s xfer: %zd fd: %d\n", evt_data->socket.domain, evt_data->socket.type, evt_data->socket.protocol, evt_data->socket.addr, evt_data->bytes_transferred, evt_data->fd);
         break;
    case RUBY_EVENT_IO_SOCKET_OPTION:
         track->option_count++;
         printf("RUBY_EVENT_IO_SOCKET_OPTION domain: %d type: %d protocol: %d addr: %s xfer: %zd fd: %d\n", evt_data->socket.domain, evt_data->socket.type, evt_data->socket.protocol, evt_data->socket.addr, evt_data->bytes_transferred, evt_data->fd);
         break;
    case RUBY_EVENT_IO_SOCKET_SHUTDOWN:
         track->shutdown_count++;
         printf("RUBY_EVENT_IO_SOCKET_SHUTDOWN domain: %d type: %d protocol: %d addr: %s xfer: %zd fd: %d\n", evt_data->socket.domain, evt_data->socket.type, evt_data->socket.protocol, evt_data->socket.addr, evt_data->bytes_transferred, evt_data->fd);
         break;
    case RUBY_EVENT_IO_SOCKET_GETHOSTBYNAME:
         track->gethostbyname_count++;
         printf("RUBY_EVENT_IO_SOCKET_GETHOSTBYNAME domain: %d type: %d protocol: %d addr: %s xfer: %zd fd: %d\n", evt_data->socket.domain, evt_data->socket.type, evt_data->socket.protocol, evt_data->socket.addr, evt_data->bytes_transferred, evt_data->fd);
         break;

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
    struct socket_events_track track = {0, 0, 0, 0, 0, 0, 0, 0, rb_hash_new()};
    VALUE tpval = rb_tracepoint_new(0, RUBY_EVENT_IO, rb_socket_events_i, &track);
    VALUE result = rb_ary_new();

    rb_tracepoint_enable(tpval);
    rb_ensure(rb_yield, Qundef, rb_tracepoint_disable, tpval);

    rb_ary_push(result, SIZET2NUM(track.sockets_count));
    rb_ary_push(result, SIZET2NUM(track.bind_count));
    rb_ary_push(result, SIZET2NUM(track.listen_count));
    rb_ary_push(result, SIZET2NUM(track.connect_count));
    rb_ary_push(result, SIZET2NUM(track.accept_count));
    rb_ary_push(result, SIZET2NUM(track.option_count));
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