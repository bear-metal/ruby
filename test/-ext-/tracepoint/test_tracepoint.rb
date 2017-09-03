# frozen_string_literal: false
require 'test/unit'
require '-test-/tracepoint'
require 'tempfile'
require 'socket'

class TestTracepointObj < Test::Unit::TestCase
  def test_not_available_from_ruby
    assert_raise ArgumentError do
      TracePoint.trace(:obj_new){}
    end
  end

  def test_tracks_objspace_events
    result = Bug.tracepoint_track_objspace_events{
      99
      'abc'
      _="foobar"
      Object.new
      nil
    }

    newobj_count, free_count, gc_start_count, gc_end_mark_count, gc_end_sweep_count, *newobjs = *result
    assert_equal 2, newobj_count
    assert_equal 2, newobjs.size
    assert_equal 'foobar', newobjs[0]
    assert_equal Object, newobjs[1].class
    assert_operator free_count, :>=, 0
    assert_operator gc_start_count, :==, gc_end_mark_count
    assert_operator gc_start_count, :>=, gc_end_sweep_count
  end

  def test_tracks_objspace_count
    stat1 = {}
    stat2 = {}
    GC.disable
    GC.stat(stat1)
    result = Bug.tracepoint_track_objspace_events{
      GC.enable
      1_000_000.times{''}
      GC.disable
    }
    GC.stat(stat2)
    GC.enable

    newobj_count, free_count, gc_start_count, gc_end_mark_count, gc_end_sweep_count, *newobjs = *result

    assert_operator stat2[:total_allocated_objects] - stat1[:total_allocated_objects], :>=, newobj_count
    assert_operator 1_000_000, :<=, newobj_count

    assert_operator stat2[:total_freed_objects] + stat2[:heap_final_slots] - stat1[:total_freed_objects], :>=, free_count
    assert_operator stat2[:count] - stat1[:count], :==, gc_start_count

    assert_operator gc_start_count, :==, gc_end_mark_count
    assert_operator gc_start_count, :>=, gc_end_sweep_count
    assert_operator stat2[:count] - stat1[:count] - 1, :<=, gc_end_sweep_count
  end

  def test_tracepoint_specify_normal_and_internal_events
    assert_raise(TypeError){ Bug.tracepoint_specify_normal_and_internal_events }
  end

  def test_after_gc_start_hook_with_GC_stress
    bug8492 = '[ruby-dev:47400] [Bug #8492]: infinite after_gc_start_hook reentrance'
    assert_nothing_raised(Timeout::Error, bug8492) do
      assert_in_out_err(%w[-r-test-/tracepoint], <<-'end;', /\A[1-9]/, timeout: 2)
        stress, GC.stress = GC.stress, false
        count = 0
        Bug.after_gc_start_hook = proc {count += 1}
        begin
          GC.stress = true
          3.times {Object.new}
        ensure
          GC.stress = stress
          Bug.after_gc_start_hook = nil
        end
        puts count
      end;
    end
  end

  def test_tracks_file_io_events
    result = IOHook.track_file_io do
      5.times do |i|
        Tempfile.create("io_events_test_#{i}") do |f|
          f.write(i.to_s)
          f.rewind
          f.read
        end
      end
    end

    assert_equal [5,10,5,5], result
  end

  def test_tracks_socket_io_events
    result = IOHook.track_socket_io do
      s = Socket.new(:INET, :STREAM)
      s1 = Socket.new(:INET, :STREAM)
      s3, s4 = Socket.pair(:UNIX, :DGRAM, 0)
      s3.send "a", 0
      s4.send "b", 0
      s3.recv(10)
      s4.recv(10)
      addr = Addrinfo.tcp("127.0.0.1", 2222)
      s.bind(addr)
      serv = Socket.new(:INET, :STREAM, 0)
      serv.listen(5)
      c = Socket.new(:INET, :STREAM, 0)
      c.connect(serv.connect_address)
      serv.accept
      Socket.gethostbyname(Socket.gethostname)
      c.shutdown
      Socket.gethostbyaddr('221.186.184.75')
      Socket.getservbyname("smtp") 
      Socket.getservbyport(80)
    end

    assert_equal [6, 2, 1, 1, 0, 0, 0], result
  end
end
