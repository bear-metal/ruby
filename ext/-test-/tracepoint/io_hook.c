#include "ruby/ruby.h"
#include "ruby/debug.h"

struct io_events_track {
    size_t open_count;
    size_t read_count;
    size_t write_count;
    size_t close_count;
    VALUE ios;
};

static void
rb_io_events_i(VALUE tpval, void *data)
{
  rb_trace_arg_t *tparg = rb_tracearg_from_tracepoint(tpval);
  struct io_events_track *track = data;

  VALUE io = rb_tracearg_self(tparg);
  switch (rb_tracearg_event_flag(tparg)) {
    case RUBY_EVENT_IO_OPEN:
      track->open_count++;
      rb_hash_aset(track->ios, io, INT2NUM(0));
      break;
    case RUBY_EVENT_IO_READ:
      track->read_count++;
      rb_hash_aset(track->ios, io, INT2NUM(0));
      break;
    case RUBY_EVENT_IO_WRITE:
      track->write_count++;
      rb_hash_aset(track->ios, io, INT2NUM(0));
      break;
    case RUBY_EVENT_IO_CLOSE:
      track->close_count++;
      rb_hash_aset(track->ios, io, INT2NUM(0));
      break;
  }
}

static VALUE
rb_enable_io_events(VALUE self)
{
    struct io_events_track track = {0, 0, 0, 0, rb_hash_new()};
    VALUE tpval = rb_tracepoint_new(0, RUBY_EVENT_IO_OPEN | RUBY_EVENT_IO_READ | RUBY_EVENT_IO_WRITE | RUBY_EVENT_IO_CLOSE, rb_io_events_i, &track);
    rb_tracepoint_enable(tpval);
    VALUE result = rb_ary_new();

    rb_tracepoint_enable(tpval);
    rb_ensure(rb_yield, Qundef, rb_tracepoint_disable, tpval);

    rb_ary_push(result, SIZET2NUM(track.open_count));
    rb_ary_push(result, SIZET2NUM(track.read_count));
    rb_ary_push(result, SIZET2NUM(track.write_count));
    rb_ary_push(result, SIZET2NUM(track.close_count));

    return result;
}

void
Init_io_hook(void)
{
    VALUE mIOHook = rb_define_module("IOHook");
    rb_define_module_function(mIOHook, "enable", rb_enable_io_events, 0);
}