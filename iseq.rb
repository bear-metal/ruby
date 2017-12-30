def tuple_rearrange
  a,b,c = 1,2,3
end

puts RubyVM::InstructionSequence.disasm(method(:tuple_rearrange))

puts RubyVM::HW_USAGE_ANALYSIS_INSN