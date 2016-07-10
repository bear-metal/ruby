=begin
Lourenss-Air:ruby lourens$ ruby -I./lib -I. -I.ext/x86_64-darwin15 benchmark/bm_hash_cond_free_st.rb
"Before: 5192003"
"After: 4910918"
Lourenss-Air:ruby lourens$ ruby -I./lib -I. -I.ext/x86_64-darwin15 benchmark/bm_hash_cond_free_st.rb
"Before: 5192043"
"After: 4910918"
Lourenss-Air:ruby lourens$ ruby -I./lib -I. -I.ext/x86_64-darwin15 benchmark/bm_hash_cond_free_st.rb
"Before: 5192003"
"After: 4910918"
Lourenss-Air:ruby lourens$ ./ruby -I./lib -I. -I.ext/x86_64-darwin15 benchmark/bm_hash_cond_free_st.rb
"Before: 5202852"
"After: 2991983"
Lourenss-Air:ruby lourens$ ./ruby -I./lib -I. -I.ext/x86_64-darwin15 benchmark/bm_hash_cond_free_st.rb
"Before: 5203019"
"After: 2991983"
Lourenss-Air:ruby lourens$ ./ruby -I./lib -I. -I.ext/x86_64-darwin15 benchmark/bm_hash_cond_free_st.rb
"Before: 5202914"
"After: 2991983"
=end

require 'objspace'

p "Before: #{ObjectSpace.memsize_of_all}"
hashes = []

10000.times do |i|
  hashes << {a: 'a'}
end

10000.times do |i|
  hashes[i].delete(:a)
end

p "After: #{ObjectSpace.memsize_of_all}"